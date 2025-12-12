#include <raylib.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "../storage/storage.h"
#include "../query/parser.h"
#include "../utils/helpers.h"
#include "../graph/graph.h"

// Better Text Box
struct TextBox {
    Rectangle rect;
    std::string text;
    bool active;
    int cursorFrames;
};

void DrawTextBox(TextBox& box) {
    // Background and Border
    Color borderColor = box.active ? SKYBLUE : LIGHTGRAY;
    Color bgColor = box.active ? Fade(SKYBLUE, 0.1f) : Fade(LIGHTGRAY, 0.1f);
    
    DrawRectangleRec(box.rect, bgColor);
    DrawRectangleLinesEx(box.rect, 2, borderColor);

    // Text with better Font
    DrawText(box.text.c_str(), (int)box.rect.x + 10, (int)box.rect.y + 10, 20, DARKGRAY);

    // Blinking Cursor
    if (box.active) {
        box.cursorFrames++;
        if ((box.cursorFrames / 30) % 2 == 0) {
            int textWidth = MeasureText(box.text.c_str(), 20);
            DrawRectangle((int)box.rect.x + 10 + textWidth + 2, (int)box.rect.y + 10, 2, 20, BLACK);
        }
    }
}

// Helper to split lines for coloring
std::vector<std::string> SplitLines(const std::string& str) {
    std::vector<std::string> lines;
    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    return lines;
}

// ... (helper functions)

int main() {
    const int screenWidth = 1400; // Wider for sidebar
    const int screenHeight = 900;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "ChronoDB Studio");
    SetTargetFPS(60);

    // Database Setup
    ChronoDB::StorageEngine storage("data");
    GraphEngine graph; 
    ChronoDB::Parser parser(storage, graph);

    // UI State
    TextBox queryBox = { { 250, 60, 900, 50 }, "", false, 0 }; // Shifted right
    Rectangle btnRect = { 1170, 60, 180, 50 };
    Rectangle sidebarRect = { 0, 0, 230, (float)screenHeight };
    Rectangle metaRect = { 250, 600, 1100, 280 }; // Bottom Metadata panel

    bool btnHover = false;
    float scrollOffset = 0.0f;
    float contentHeight = 0.0f;

    // Data Caching
    std::vector<std::string> tableList = storage.getTableNames();
    std::string selectedTable = "";
    std::vector<ChronoDB::Column> selectedColumns;

    // Output Log
    std::vector<std::string> logLines;
    logLines.push_back("Welcome to ChronoDB Studio!");
    logLines.push_back("Type 'CREATE TABLE...' to see updates here.");

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        
        // Refresh table list occasionally (e.g., if execution happened)
        // For simplicity, we just do it if execute happened, see below.

        // Input & Button Logic (Unchanged but shifted)
        if (CheckCollisionPointRec(mouse, queryBox.rect)) {
            SetMouseCursor(MOUSE_CURSOR_IBEAM);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) queryBox.active = true;
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) queryBox.active = false;
        }

        if (queryBox.active) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125)) queryBox.text += (char)key;
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !queryBox.text.empty()) queryBox.text.pop_back();
        }

        btnHover = CheckCollisionPointRec(mouse, btnRect);
        bool execute = false;
        if ((btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || (queryBox.active && IsKeyPressed(KEY_ENTER))) {
            execute = true;
        }

        if (execute && !queryBox.text.empty()) {
            Helper::startCapture();
            parser.parseAndExecute(queryBox.text);
            std::string rawResult = Helper::stopCapture();

            logLines.push_back("> " + queryBox.text);
            auto lines = SplitLines(rawResult);
            logLines.insert(logLines.end(), lines.begin(), lines.end());
            logLines.push_back(""); 
            
            queryBox.text = ""; 
            scrollOffset = -100000; 

            // Refresh Table List after execution (in case CREATE/DROP)
            tableList = storage.getTableNames();
            // Reselect if still exists
            if (!selectedTable.empty() && !storage.tableExists(selectedTable)) {
                selectedTable = "";
                selectedColumns.clear();
            }
        }

        // Sidebar Logic
        if (CheckCollisionPointRec(mouse, sidebarRect)) {
             // Basic list click detection
             float y = 50;
             for (const auto& tb : tableList) {
                 Rectangle itemRect = { 10, y, 210, 30 };
                 if (CheckCollisionPointRec(mouse, itemRect)) {
                     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                         selectedTable = tb;
                         selectedColumns = storage.getTableColumns(tb);
                     }
                 }
                 y += 35;
             }
        }

        // Scrolling Logic (Unchanged)
        Rectangle outRect = { 250, 140, 1100, 440 }; // Shrunk height to fit Metadata
        float wheel = GetMouseWheelMove();
        if (CheckCollisionPointRec(mouse, outRect) && wheel != 0) scrollOffset += wheel * 30.0f;
        contentHeight = logLines.size() * 24.0f + 50;
        if (contentHeight < outRect.height) scrollOffset = 0;
        else if (scrollOffset > 0) scrollOffset = 0;
        else if (scrollOffset < -(contentHeight - outRect.height)) scrollOffset = -(contentHeight - outRect.height);

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Sidebar Background
        DrawRectangleRec(sidebarRect, Color{40, 44, 52, 255}); // Darkest
        DrawText("TABLES", 20, 15, 20, LIGHTGRAY);
        DrawLine(10, 40, 220, 40, GRAY);
        
        float listY = 50;
        for (const auto& tb : tableList) {
            Color textColor = (tb == selectedTable) ? SKYBLUE : WHITE;
            if (CheckCollisionPointRec(mouse, {10, listY, 210, 30})) textColor = YELLOW;
            
            DrawText(tb.c_str(), 20, (int)listY + 5, 20, textColor);
            listY += 35;
        }

        // Header
        DrawText("ChronoDB Studio", 260, 15, 30, DARKBLUE);
        DrawText("v1.1", 520, 25, 10, GRAY);

        // Input & Button
        DrawTextBox(queryBox);
        DrawRectangleRec(btnRect, btnHover ? SKYBLUE : BLUE);
        DrawText("RUN", (int)btnRect.x + 65, (int)btnRect.y + 15, 20, WHITE);

        // Console
        DrawRectangleRec(outRect, GetColor(0x1e1e1eff)); 
        BeginScissorMode((int)outRect.x, (int)outRect.y, (int)outRect.width, (int)outRect.height);
            int startY = (int)outRect.y + 10 + (int)scrollOffset;
            for (const auto& line : logLines) {
                Color lineColor = LIGHTGRAY;
                if (line.find("[SUCCESS]") != std::string::npos) lineColor = GREEN;
                else if (line.find("[ERROR]") != std::string::npos) lineColor = RED;
                else if (line.find(">") == 0) lineColor = YELLOW;
                else if (line.find("+") == 0 || line.find("|") == 0) lineColor = WHITE;

                if (startY > outRect.y - 30 && startY < outRect.y + outRect.height) {
                    DrawText(line.c_str(), (int)outRect.x + 15, startY, 20, lineColor);
                }
                startY += 24;
            }
        EndScissorMode();

        // METADATA PANEL
        if (!selectedTable.empty()) {
            DrawRectangleRec(metaRect, Fade(SKYBLUE, 0.2f));
            DrawRectangleLinesEx(metaRect, 2, BLUE);
            DrawText(("Schema: " + selectedTable).c_str(), (int)metaRect.x + 10, (int)metaRect.y + 10, 20, DARKBLUE);
            
            int colX = (int)metaRect.x + 20;
            int colY = (int)metaRect.y + 40;
            for (const auto& col : selectedColumns) {
                std::string info = col.name + " (" + col.type + ")";
                DrawRectangle(colX - 5, colY - 2, 200, 24, WHITE);
                DrawText(info.c_str(), colX, colY, 20, BLACK);
                colY += 30;
                if (colY > metaRect.y + metaRect.height - 30) {
                     colY = (int)metaRect.y + 40;
                     colX += 220; // Wrap columns
                }
            }
        } else {
             DrawText("Select a table to view schema", (int)metaRect.x + 400, (int)metaRect.y + 130, 20, GRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
