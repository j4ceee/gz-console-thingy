#pragma once

struct GameString {
    union {
        char*  ptr;        // heap pointer when capacity >= 16
        char   buf[16];    // SSO inline buffer when capacity < 16
    };
    uint64_t size;
    uint64_t capacity;

    const char* c_str() const {
        return capacity >= 0x10 ? ptr : buf;
    }
};
static_assert(sizeof(GameString) == 0x20, "GameString size mismatch");

struct BasicVector {
    void* first;   // pointer to start of data
    void* last;    // pointer to current end
    void* end;     // pointer to allocated end
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
