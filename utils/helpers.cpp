#include "helpers.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include <variant>
#include <iomanip>

using namespace std;

namespace Helper {

    string trim(const string& str) {
        int start = 0;
        while (start < (int)str.size() && isspace(str[start])) start++;
        int end = str.size() - 1;
        while (end >= start && isspace(str[end])) end--;
        if (start > end) return "";
        return str.substr(start, end - start + 1);
    }

    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter)) {
            if (!token.empty()) tokens.push_back(trim(token));
        }
        return tokens;
    }

    bool isNumber(const string& str) {
        if (str.empty()) return false;
        return all_of(str.begin(), str.end(), ::isdigit);
    }

    string toUpper(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    void printError(const string& message) {
        cout << "\033[31m[ERROR]: " << message << "\033[0m" << endl;
    }

    void printSuccess(const string& message) {
        cout << "\033[32m[SUCCESS]: " << message << "\033[0m" << endl;
    }

    void printLine(char ch, int count) {
        for (int i = 0; i < count; i++) cout << ch;
        cout << endl;
    }

    void printRecord(const vector<string>& fields) {
        for (const auto& field : fields) cout << field << " | ";
        cout << endl;
    }

    // --- Pretty table printing for SELECT ---
    void printTable(const std::vector<std::vector<std::variant<int, float, std::string>>>& rows,const std::vector<std::string>& headers) {
        std::vector<size_t> widths(headers.size(), 0);

        // 1️⃣ Calculate column widths based on headers
        for (size_t i = 0; i < headers.size(); i++)
            widths[i] = headers[i].size();

        // 2️⃣ Calculate column widths based on data
        for (const auto& row : rows) {
            for (size_t i = 0; i < row.size(); i++) {
                std::string val = std::visit([](auto&& v) -> std::string {
                    std::ostringstream oss;
                    oss << v;
                    return oss.str();
                }, row[i]);
                widths[i] = std::max(widths[i], val.size());
            }
        }

        // 3️⃣ Print header line
        std::cout << "+";
        for (auto w : widths) std::cout << std::string(w + 2, '-') << "+";
        std::cout << "\n";

        // 4️⃣ Print headers
        std::cout << "|";
        for (size_t i = 0; i < headers.size(); i++)
            std::cout << " " << std::setw(widths[i]) << headers[i] << " |";
        std::cout << "\n";

        // 5️⃣ Print separator line
        std::cout << "+";
        for (auto w : widths) std::cout << std::string(w + 2, '-') << "+";
        std::cout << "\n";

        // 6️⃣ Print each row
        for (const auto& row : rows) {
            std::cout << "|";
            for (size_t i = 0; i < row.size(); i++) {
                std::string val = std::visit([](auto&& v) -> std::string {
                    std::ostringstream oss;
                    oss << v;
                    return oss.str();
                }, row[i]);
                std::cout << " " << std::setw(widths[i]) << val << " |";
            }
            std::cout << "\n";
        }

        // 7️⃣ Print bottom line
        std::cout << "+";
        for (auto w : widths) std::cout << std::string(w + 2, '-') << "+";
        std::cout << "\n";
    }


} // namespace Helper
