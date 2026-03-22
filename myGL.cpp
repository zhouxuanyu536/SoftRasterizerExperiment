#include <algorithm>
#include "myGL.h"

mat<4, 4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 z = normalize(eye - center);
    vec3 x = normalize(cross(up, z));
    vec3 y = cross(z, x);

    mat<4, 4> R = { {
        {x.x,x.y,x.z,0},
        {y.x,y.y,y.z,0},
        {z.x,z.y,z.z,0},
        {0,0,0,1}
    } };

    mat<4, 4> T = { {
        {1,0,0,-center.x},
        {0,1,0,-center.y},
        {0,0,1,-center.z},
        {0,0,0,1}
    } };

    ModelView = R * T;
}

void init_perspective(double f) {
    Perspective = { {
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,-1.0 / f,1}
    } };
}

void init_viewport(int x, int y, int w, int h) {
    Viewport = { {
        {w / 2.0,0,0,x + w / 2.0},
        {0,h / 2.0,0,y + h / 2.0},
        {0,0,1,0},
        {0,0,0,1}
    } };
}

void init_zbuffer(int w, int h) {
    zbuffer.assign(w * h, -1e9);
}

void rasterize(const Triangle& clip, const IShader& shader, TGAImage& fb) {
    vec4 ndc[3] = {
        clip[0] / clip[0].w,
        clip[1] / clip[1].w,
        clip[2] / clip[2].w
    };

    vec2 scr[3] = {
        (Viewport * ndc[0]).xy(),
        (Viewport * ndc[1]).xy(),
        (Viewport * ndc[2]).xy()
    };

    mat<3, 3> M = { {
        {scr[0].x, scr[0].y, 1},
        {scr[1].x, scr[1].y, 1},
        {scr[2].x, scr[2].y, 1}
    } };

    if (det(M) <= 0) return;

    auto [minx, maxx] = std::minmax({ scr[0].x, scr[1].x, scr[2].x });
    auto [miny, maxy] = std::minmax({ scr[0].y, scr[1].y, scr[2].y });
	//put for loop here, and parallelize it with openmp
#pragma omp parallel for
    for (int x = std::max<int>(0, minx); x <= std::min<int>(fb.width() - 1, maxx); x++) {
        for (int y = std::max<int>(0, miny); y <= std::min<int>(fb.height() - 1, maxy); y++) {

            vec3 bc = inverse(M) * vec3 { (double)x, (double)y, 1. };
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

            vec3 bc_clip = {
                bc.x / clip[0].w,
                bc.y / clip[1].w,
                bc.z / clip[2].w
            };
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

            double z = bc * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            int idx = x + y * fb.width();

            if (z <= zbuffer[idx]) continue;

            auto [discard, color] = shader.fragment(bc_clip);
            if (discard) continue;

            zbuffer[idx] = z;
            fb.set(x, y, color);
        }
    }
}