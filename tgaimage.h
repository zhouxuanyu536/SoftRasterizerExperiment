#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#pragma pack(push,1)

struct Vec3f {
    float x, y, z;

    Vec3f() : x(0), y(0), z(0) {}
    Vec3f(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
    const float& operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }

    Vec3f operator+(const Vec3f& v) const { return Vec3f(x + v.x, y + v.y, z + v.z); }
    Vec3f operator-(const Vec3f& v) const { return Vec3f(x - v.x, y - v.y, z - v.z); }
    Vec3f operator*(float f) const { return Vec3f(x * f, y * f, z * f); }

    Vec3f& operator+=(const Vec3f& v) { x += v.x; y += v.y; z += v.z; return *this; }
};

struct TGAHeader {
    std::uint8_t  idlength = 0;
    std::uint8_t  colormaptype = 0;
    std::uint8_t  datatypecode = 0;
    std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = { 0,0,0,0 };
    std::uint8_t bytespp = 4;
    std::uint8_t& operator[](const int i) { return bgra[i]; }
    const std::uint8_t& operator[](const int i) const { return bgra[i]; }
    TGAColor operator*(double intensity) const {
        TGAColor result;
        for (int i = 0; i < 3; i++) {
            int val = int(bgra[i] * intensity);
            result.bgra[i] = val;
        }
        return result;
    }
    
    TGAColor operator+(TGAColor other) const {
        TGAColor result;
        for (int i = 0; i < 4; i++) {
            int val = static_cast<int>(bgra[i]) + static_cast<int>(other.bgra[i]);
            
            result.bgra[i] = val;
        }
        
        return result;
    }
    bool operator==(const TGAColor& b) const{
        for (int i = 0; i < 4; i++) {
            if (b.bgra[i] != bgra[i]) return false;
        }
        return true;
    }
    bool operator!=(const TGAColor& b) const {
        for (int i = 0; i < 4; i++) {
            if (b.bgra[i] != bgra[i]) return true;
        }
        return false;
    }
};

struct TGAImage {
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };
    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp, TGAColor c = {});
    bool  read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool vflip = true, const bool rle = true) const;
    void flip_horizontally();
    void flip_vertically();
    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor& c);
    void set(const std::pair<int, int> xypair, const TGAColor& c);
    int width()  const;
    int height() const;
private:
    bool   load_rle_data(std::ifstream& in);
    bool unload_rle_data(std::ofstream& out) const;
    int w = 0, h = 0;
    std::uint8_t bpp = 0;
    std::vector<std::uint8_t> data = {};
};

