//Parse and execute commands
#include "parser.h"
#include <iostream>
#include "../utils/types.h"
#include "../utils/helpers.h"

namespace ChronoDB {

    // Constructor updated: No IndexingEngine
    Parser::Parser(StorageEngine& s) : storage(s) {}

    void Parser::undo() {
        if (undoStack.empty()) {
            Helper::printError("Nothing to Undo!");
            return;
        }
        auto reverseAction = undoStack.top();
        undoStack.pop();
        reverseAction();
        Helper::printSuccess("Last action undone successfully.");
    }

    void Parser::parseAndExecute(const std::string& commandLine) {
        if (commandLine == "UNDO") {
            undo();
            return;
        }

        Lexer lexer(commandLine);
        std::vector<Token> tokens = lexer.tokenize();

        if (tokens.empty()) return;

        std::string cmd = Helper::toUpper(tokens[0].value);

        if (cmd == "CREATE") handleCreate(tokens);
        else if (cmd == "INSERT") handleInsert(tokens);
        else if (cmd == "SELECT") handleSelect(tokens);
        else if (cmd == "UPDATE") handleUpdate(tokens);
        else if (cmd == "DELETE") handleDelete(tokens);
        else Helper::printError("Unknown command: " + cmd);
    }

    void Parser::handleCreate(const std::vector<Token>& tokens) {
        if (tokens.size() < 3 || Helper::toUpper(tokens[1].value) != "TABLE") {
            Helper::printError("Syntax Error. Expected: CREATE TABLE <name>");
            return;
        }
        std::string tableName = tokens[2].value;
        if (storage.createTable(tableName)) {
            Helper::printSuccess("Table '" + tableName + "' created.");
            
            // UNDO: Drop the table
            undoStack.push([this, tableName]() {
                std::cout << "[UNDO] Dropping table " << tableName << " (Simulated)" << std::endl;
                // In future: storage.dropTable(tableName);
            });
        } else {
            Helper::printError("Table already exists.");
        }
    }

    void Parser::handleInsert(const std::vector<Token>& tokens) {
        // Validate: INSERT INTO <table> VALUES <id> <name> <gpa>
        if (tokens.size() < 7) { 
            Helper::printError("Syntax Error: INSERT INTO <table> VALUES <id> <name> <gpa>");
            return;
        }

        // Validate token structure
        if (Helper::toUpper(tokens[1].value) != "INTO") {
            Helper::printError("Expected 'INTO' after INSERT");
            return;
        }
        
        if (Helper::toUpper(tokens[3].value) != "VALUES") {
            Helper::printError("Expected 'VALUES' in INSERT statement");
            return;
        }

        // Validate that ID is a number
        if (!Helper::isNumber(tokens[4].value)) {
            Helper::printError("ID must be a number");
            return;
        }

        std::string tableName = tokens[2].value;
        try {
            int id = std::stoi(tokens[4].value);
            std::string name = tokens[5].value;
            float gpa = std::stof(tokens[6].value);

            Record r;
            r.fields.emplace_back(id);
            r.fields.emplace_back(name);
            r.fields.emplace_back(gpa);

            if (storage.insertRecord(tableName, r)) {
                Helper::printSuccess("Record inserted.");

                // UNDO: Delete the record
                undoStack.push([this, tableName, id]() {
                    storage.deleteRecord(tableName, id);
                    std::cout << "[UNDO] Deleted record ID " << id << std::endl;
                });
            } else {
                Helper::printError("Insert failed.");
            }
        } catch (const std::exception& e) {
            Helper::printError("Error parsing values: " + std::string(e.what()));
        }
    }

    void Parser::handleSelect(const std::vector<Token>& tokens) {
        if (tokens.size() < 4) return;
        std::string tableName = tokens[3].value;
        auto rows = storage.selectAll(tableName);
        
        Helper::printLine('-', 30);
        std::cout << "Displaying " << rows.size() << " rows from " << tableName << ":" << std::endl;
        for(const auto& row : rows) {
             visit([](auto&& arg){ std::cout << arg << " | "; }, row.fields[0]);
             visit([](auto&& arg){ std::cout << arg << " | "; }, row.fields[1]);
             visit([](auto&& arg){ std::cout << arg << " | "; }, row.fields[2]);
             std::cout << std::endl;
        }
    }

    void Parser::handleUpdate(const std::vector<Token>& tokens) {
        // UPDATE <table> SET <field>=<value> WHERE <id>=<value>
        if (tokens.size() < 7) {
            Helper::printError("Syntax Error: UPDATE <table> SET <field>=<value> WHERE id=<id>");
            return;
        }

        try {
            std::string tableName = tokens[1].value;
            // Simplified: UPDATE table SET ... WHERE id=<value>
            // tokens[5] should be the id value
            int id = std::stoi(tokens[6].value);

            // For now, this is a placeholder - full SET parsing would be more complex
            std::cout << "[UPDATE] Updating record with ID " << id << " in table " << tableName << std::endl;
            Helper::printSuccess("Update statement parsed (full implementation pending)");

        } catch (const std::exception& e) {
            Helper::printError("Error parsing UPDATE: " + std::string(e.what()));
        }
    }

    void Parser::handleDelete(const std::vector<Token>& tokens) {
        // DELETE FROM <table> WHERE id=<value>
        if (tokens.size() < 5) {
            Helper::printError("Syntax Error: DELETE FROM <table> WHERE id=<value>");
            return;
        }

        try {
            if (Helper::toUpper(tokens[1].value) != "FROM") {
                Helper::printError("Expected 'FROM' after DELETE");
                return;
            }

            std::string tableName = tokens[2].value;
            int id = std::stoi(tokens[4].value);

            if (storage.deleteRecord(tableName, id)) {
                Helper::printSuccess("Record deleted.");
                
                // UNDO: Re-insert the record (simplified - just logs)
                undoStack.push([this, tableName, id]() {
                    std::cout << "[UNDO] Restoring record ID " << id << " (Simulated)" << std::endl;
                });
            } else {
                Helper::printError("Delete failed - record not found.");
            }
        } catch (const std::exception& e) {
            Helper::printError("Error parsing DELETE: " + std::string(e.what()));
        }
    }
}