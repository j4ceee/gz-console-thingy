#pragma once

#include "data.h"

#include <algorithm>
#include <string>

namespace gz::Data {

// Case-insensitive string search
inline bool ContainsIgnoreCase(const char* text, const char* search) {
    if (!search || search[0] == '\0') return true; // empty search matches all
    if (!text) return false;

    std::string textLower = text;
    std::string searchLower = search;

    std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::tolower);
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

    return textLower.find(searchLower) != std::string::npos;
}

// Count matches in a category
template <typename T>
inline int CountMatches(const DataCategory<T>& category, const char* searchTerm) {
    if (!searchTerm || searchTerm[0] == '\0') {
        return (int)category.count;
    }

    int matches = 0;
    for (size_t i = 0; i < category.count; i++) {
        if (ContainsIgnoreCase(category.data[i].name, searchTerm)) {
            matches++;
        }
    }
    return matches;
}

inline const char* TruncateText(const char* text, size_t maxLength = 500) {
    static char buffer[512];
    if (!text || strlen(text) <= maxLength) {
        return text;
    }

    strncpy_s(buffer, text, maxLength);
    buffer[maxLength] = '\0';
    strcat_s(buffer, "...");
    return buffer;
}
}