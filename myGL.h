#include "tgaimage.h"
#include "geometry.h"
#include <algorithm>
void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader {
    static TGAColor sample2D(const TGAImage& img, const vec2& uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }
    static TGAColor glowSample2D(const TGAImage& img, const vec2& uvf, float threshold) {
        int w = img.width();
        int h = img.height();

        // 当前像素坐标
        int x = std::max(0, std::min(int(uvf[0] * w), w - 1));
        int y = std::max(0, std::min(int(uvf[1] * h), h - 1));

        // 当前点颜色
        TGAColor color = img.get(x, y);

        // 当前点若本身较亮，则视为发光源，直接返回
        if (color.bgra[0] >= 10 || color.bgra[1] >= 10 || color.bgra[2] >= 10) {
            return color;
        }

        // 否则寻找最近的发光像素
        double minDist = threshold + 1.0;
        TGAColor nearestGlow = { 0,0,0,255 };
        for (int dy = -int(threshold); dy <= int(threshold); ++dy) {
            for (int dx = -int(threshold); dx <= int(threshold); ++dx) {
                double dist2 = dx * dx + dy * dy;
                if (dist2 > threshold * threshold) continue;

                int nx = x + dx;
                int ny = y + dy;
                if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

                TGAColor c = img.get(nx, ny);
                if (c.bgra[0] >= 10 || c.bgra[1] >= 10 || c.bgra[2] >= 10) {
                    double dist = std::sqrt(dist2);
                    if (dist < minDist) {
                        minDist = dist;
                        nearestGlow = c;
                    }
                }
            }
        }

        if (minDist > threshold)
            return nearestGlow;

        // 否则亮度按距离衰减（线性或平方反比）
        double factor = 1.0 - (minDist / threshold); // 越近越亮
        TGAColor result;
        for (int i = 0; i < 3; i++) {
            result.bgra[i] = std::min<int>(255, nearestGlow.bgra[i] * factor);
        }
        result.bgra[3] = 255;
        return result;
    }
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

typedef vec4 Triangle[3]; // a triangle primitive is made of three ordered points
void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer);
