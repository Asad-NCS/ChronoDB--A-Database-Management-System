#ifndef SORTING_H
#define SORTING_H

#include <vector>
#include <string>
#include <variant>
#include "../utils/types.h"

namespace ChronoDB {

    class Sorting {
    public:
        static void mergeSort(std::vector<Record>& rows, int colIndex, const std::string& colType);
        
        // return index of first element >= val
        static int binarySearchLowerBound(const std::vector<Record>& rows, int colIndex, const std::string& colType, const std::string& val);
        
        // return index of first element > val
        static int binarySearchUpperBound(const std::vector<Record>& rows, int colIndex, const std::string& colType, const std::string& val);

    private:
        static void mergeSortRecursive(std::vector<Record>& rows, int left, int right, int colIndex, const std::string& colType);
        static void merge(std::vector<Record>& rows, int left, int mid, int right, int colIndex, const std::string& colType);
        static bool compare(const Record& a, const Record& b, int colIndex, const std::string& colType);
        static bool compareVal(const Record& a, const std::string& bVal, int colIndex, const std::string& colType); // a < bVal
    };

}

#endif
