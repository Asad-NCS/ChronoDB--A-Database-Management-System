//Implementation of helpers like toUpper,isNumber etc.
#pragma once
#include <string>
#include <vector>
#include <variant>
#include <iostream>

using namespace std;

namespace Helper {
    string trim(const string& str);
    vector<string> split(const string& str, char delimiter);
    bool isNumber(const string& str);
    string toUpper(const string& str);

    // Displaying helpers
    void printError(const string& message);
    void printSuccess(const string& message);
    void printRecord(const vector<string>& fields);
    void printLine(char ch='-', int count=40);

    // Pretty table printing
    void printTable(
        const vector<vector<variant<int, float, string>>>& rows,
        const vector<string>& headers
    );

    // Output Capture for GUI (Added)
    void startCapture();
    string stopCapture();
    string getCaptured();
    void print(const string& msg);
    void println(const string& msg);
}