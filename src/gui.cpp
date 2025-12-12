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

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_MSAA_4X_HINT); // Anti-aliasing
    InitWindow(screenWidth, screenHeight, "ChronoDB Studio");
    SetTargetFPS(60);

    // Database Setup
    ChronoDB::StorageEngine storage("data");
    GraphEngine graph; 
    ChronoDB::Parser parser(storage, graph);

    // UI State
    TextBox queryBox = { { 50, 60, 900, 50 }, "", false, 0 };
    Rectangle btnRect = { 970, 60, 180, 50 };
    bool btnHover = false;
    
    // Output Log (Buffer of lines)
    std::vector<std::string> logLines;
    logLines.push_back("Welcome to ChronoDB Studio!");
    logLines.push_back("Type a query below and press Enter.");
    logLines.push_back("------------------------------------------------");

    float scrollOffset = 0.0f;
    float contentHeight = 0.0f;

    while (!WindowShouldClose()) {
        // --- UPDATE ---
        Vector2 mouse = GetMousePosition();
        
        // Input Handling
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
            if (IsKeyPressed(KEY_BACKSPACE) && !queryBox.text.empty()) {
                queryBox.text.pop_back();
            }
        }

        // Execute Logic
        btnHover = CheckCollisionPointRec(mouse, btnRect);
        bool execute = false;
        if ((btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || (queryBox.active && IsKeyPressed(KEY_ENTER))) {
            execute = true;
        }

        if (execute && !queryBox.text.empty()) {
            Helper::startCapture();
            parser.parseAndExecute(queryBox.text);
            std::string rawResult = Helper::stopCapture();

            // Add Command to log
            logLines.push_back("> " + queryBox.text);
            
            // Add Result lines
            auto lines = SplitLines(rawResult);
            logLines.insert(logLines.end(), lines.begin(), lines.end());
            logLines.push_back(""); // Spacer

            queryBox.text = ""; 
            
            // Auto-scroll to bottom
            scrollOffset = -100000; // Force scroll to bottom next frame logic
        }

        // Scrolling
        float wheel = GetMouseWheelMove();
        if (wheel != 0) scrollOffset += wheel * 30.0f;
        
        // Clamp Scrolling (Simple)
        Rectangle outRect = { 50, 140, 1100, 720 };
        contentHeight = logLines.size() * 24.0f + 50; // 24px per line + extra padding at bottom
        
        if (contentHeight < outRect.height) scrollOffset = 0;
        else {
            if (scrollOffset > 0) scrollOffset = 0;
            if (scrollOffset < -(contentHeight - outRect.height)) scrollOffset = -(contentHeight - outRect.height);
        }

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Header
        DrawText("ChronoDB Studio", 50, 15, 30, DARKBLUE);
        DrawText("v1.0", 320, 25, 10, GRAY);

        // Input
        DrawTextBox(queryBox);
        
        // Button
        DrawRectangleRec(btnRect, btnHover ? SKYBLUE : BLUE);
        DrawRectangleLinesEx(btnRect, 2, DARKBLUE);
        DrawText("RUN", (int)btnRect.x + 65, (int)btnRect.y + 15, 20, WHITE);

        // Output Console Background
        DrawRectangleRec(outRect, GetColor(0x1e1e1eff)); // VSCode Dark Grey
        DrawRectangleLinesEx(outRect, 2, DARKGRAY);

        // Validated Scissor Output
        BeginScissorMode((int)outRect.x, (int)outRect.y, (int)outRect.width, (int)outRect.height);
            int startY = (int)outRect.y + 10 + (int)scrollOffset;
            
            for (const auto& line : logLines) {
                // Determine Color based on content
                Color lineColor = LIGHTGRAY;
                if (line.find("[SUCCESS]") != std::string::npos) lineColor = GREEN;
                else if (line.find("[ERROR]") != std::string::npos) lineColor = RED;
                else if (line.find(">") == 0) lineColor = YELLOW; // Commands
                else if (line.find("+") == 0 || line.find("|") == 0) lineColor = WHITE; // Tables

                // Optimization: Only draw if visible
                if (startY > outRect.y - 30 && startY < outRect.y + outRect.height) {
                    DrawText(line.c_str(), (int)outRect.x + 15, startY, 20, lineColor); // FONT SIZE 20
                }
                startY += 24; // Spacing
            }
        EndScissorMode();
        
        // Scrollbar (Visual only)
        if (contentHeight > outRect.height) {
            float ratio = outRect.height / contentHeight;
            float scrollY = -scrollOffset / contentHeight * outRect.height;
            DrawRectangle((int)outRect.x + (int)outRect.width - 10, (int)outRect.y + (int)scrollY, 8, (int)(outRect.height * ratio), Fade(WHITE, 0.3f));
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
