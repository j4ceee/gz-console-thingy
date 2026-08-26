#pragma once

#include <cstdint>
#include <cstring>

struct GameString {
    union {
        char*  ptr;        // heap pointer when capacity >= 16
        char   buf[16];    // SSO inline buffer when capacity < 16
    };
    uint64_t size;
    uint64_t capacity;

    [[nodiscard]] const char* c_str() const
    {
        return capacity >= 0x10 ? ptr : buf;
    }

    void SetTemporary(const char* str)
    {
        const size_t len = strlen(str);
        if (len < 16)
        {
            memset(buf, 0, sizeof(buf));
            memcpy(buf, str, len);
            size = len;
            capacity = 15; // < 0x10 branch in c_str(), i.e. "SSO mode"
        }
        else
        {
            ptr = new char[len + 1];
            memcpy(ptr, str, len + 1); // include null terminator
            size = len;
            capacity = len; // >= 0x10 signals heap mode to c_str()
        }
    }

    // must be called after SetTemporary() once the game is done reading it to free our heap allocation
    void FreeTemporary()
    {
        if (capacity >= 0x10)
        {
            delete[] ptr;
            ptr = nullptr;
        }
    }
};
static_assert(sizeof(GameString) == 0x20, "GameString size mismatch");

struct CHashString
{
    uint32_t m_Hash;
};

struct BasicVector {
    void* first;   // pointer to start of data
    void* last;    // pointer to current end
    void* end;     // pointer to allocated end

    // --- typed access ---

    template <typename T>
    [[nodiscard]] T* Begin() const { return static_cast<T*>(first); }

    template <typename T>
    [[nodiscard]] T* End() const { return static_cast<T*>(last); }

    template <typename T>
    [[nodiscard]] int Count() const
    {
        return first ? static_cast<int>(static_cast<T*>(last) - static_cast<T*>(first)) : 0;
    }

    template <typename T>
    [[nodiscard]] T* At(int index) const
    {
        if (index < 0 || index >= Count<T>()) return nullptr;
        return static_cast<T*>(first) + index;
    }

    // --- stride access (opaque game-side element, we only know its size) ---

    [[nodiscard]] int CountStride(size_t stride) const
    {
        if (!first || stride == 0) return 0;
        return static_cast<int>((static_cast<uint8_t*>(last) - static_cast<uint8_t*>(first)) / stride);
    }

    [[nodiscard]] uint8_t* AtStride(int index, size_t stride) const
    {
        if (index < 0 || index >= CountStride(stride)) return nullptr;
        return static_cast<uint8_t*>(first) + index * stride;
    }

    /// shrinks by one element without destroying it
    /// caller is responsible for having already destroyed or moved out the last element
    void PopBackRaw(size_t stride)
    {
        if (CountStride(stride) > 0) last = static_cast<uint8_t*>(last) - stride;
    }
};

struct BasicSharedPtr
{
    void* px;  // object pointer
    void* pn;  // control block pointer
};

struct CVector2f {
    float x, y;
};

struct CVector3f {
    float x, y, z;


    CVector3f operator+(const CVector3f& o) const { return { x+o.x, y+o.y, z+o.z }; }
    CVector3f operator-(const CVector3f& o) const { return { x-o.x, y-o.y, z-o.z }; }
    CVector3f operator*(float s)            const { return { x*s,   y*s,   z*s   }; }
    CVector3f& operator+=(const CVector3f& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    CVector3f& operator-=(const CVector3f& o) { x-=o.x; y-=o.y; z-=o.z; return *this; }
};

struct CVector4f {
    float x, y, z, w;
};

struct CMatrix4f {
    CVector4f m[4];

    CMatrix4f()
    {
        // matrix identity          w = 0 0 0 1 for standard transformations
        m[0] = {1, 0, 0, 0}; // right / x-axis direction / x-scale
        m[1] = {0, 1, 0, 0}; // up / y-axis direction / y-scale
        m[2] = {0, 0, 1, 0}; // forward / z-axis direction / z-scale
        m[3] = {0, 0, 0, 1}; // translation / position
        // Diagonal values = 1 = no scaling
    }

    /// <summary>
    /// Gets the translation component of the matrix (position) as a reference to a CVector3f.
    /// This allows for direct reading and writing of the translation values.
    /// </summary>
    CVector3f& GetTranslation()
    {
        return *reinterpret_cast<CVector3f*>(&m[3]);
    }
};
static_assert(sizeof(CMatrix4f) == 0x40);
