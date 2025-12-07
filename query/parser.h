#ifndef CHRONODB_PARSER_H
#define CHRONODB_PARSER_H

#include <string>
#include <vector>
#include <stack>
#include <functional>
#include "../storage/storage.h"
#include "lexer.h"

// #include "../indexing/index.h"  <-- COMMENTED OUT DEPENDENCY

namespace ChronoDB {

    class Parser {
    public:
        // Removed IndexingEngine from constructor
        Parser(StorageEngine& storageRef); 
        
        void parseAndExecute(const std::string& command);
        void undo(); 

    private:
        StorageEngine& storage;
        // IndexingEngine& index; <-- COMMENTED OUT
        
        std::stack<std::function<void()>> undoStack; 

        void handleCreate(const std::vector<Token>& tokens);
        void handleInsert(const std::vector<Token>& tokens);
        void handleUpdate(const std::vector<Token>& tokens);
        void handleDelete(const std::vector<Token>& tokens);
        void handleSelect(const std::vector<Token>& tokens);
    };

}

#endif