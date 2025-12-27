#pragma once

namespace gz::Data {

    struct DataEntry {
        const char* name;
    };

    template <typename T>
    struct DataCategory {
        const char* displayName = nullptr;
        size_t count = 0;
        const T* data = nullptr;
        const char* description = nullptr;
    };

}