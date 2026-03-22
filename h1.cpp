#include <iostream>
#include "myGL.h"
#include "model.h"

extern mat<4, 4> ModelView, Perspective;
extern std::vector<double> zbuffer;

struct PhongShader : IShader {
    const Model& model;

    vec4 light;
    mat<4, 4> normalMat;

    vec2 uv[3];
    vec4 nrm[3];
    vec4 pos[3];

    PhongShader(const vec3& l, const Model& m) : model(m) {
        light = normalize(ModelView * vec4{ l.x, l.y, l.z, 0. });
        normalMat = inverse(ModelView);
    }

    vec4 vertex(int f, int v) {
        uv[v] = model.uv(f, v);
        nrm[v] = normalize(normalMat * model.normal(f, v));
        pos[v] = ModelView * model.vert(f, v);
        return Perspective * pos[v];
    }

    std::pair<bool, TGAColor> fragment(const vec3 bar) const override {
    	
    	double w0 = 1.0 / pos[0].w;
    	double w1 = 1.0 / pos[1].w;
    	double w2 = 1.0 / pos[2].w;

    	double weight_sum = bar[0]*w0 + bar[1]*w1 + bar[2]*w2;
        vec2 st = (uv[0]*bar[0]*w0 + uv[1]*bar[1]*w1 + uv[2]*bar[2]*w2) / weight_sum;
    	vec4 fragPos = (pos[0]*bar[0]*w0 + pos[1]*bar[1]*w1 + pos[2]*bar[2]*w2) / weight_sum;
    	vec4 n = normalize((nrm[0]*bar[0]*w0 + nrm[1]*bar[1]*w1 + nrm[2]*bar[2]*w2) / weight_sum);

        vec4 e1 = pos[1] - pos[0];
        vec4 e2 = pos[2] - pos[0];
        vec2 d1 = uv[1] - uv[0];
        vec2 d2 = uv[2] - uv[0];

        double inv = 1.0 / (d1.x * d2.y - d2.x * d1.y);

        vec4 t = normalize((e1 * d2.y - e2 * d1.y) * inv);
        vec4 b = normalize((e2 * d1.x - e1 * d2.x) * inv);

        mat<4, 4> TBN = { t, b, n, {0,0,0,1} };

        vec4 mapN = model.normal(st);
        vec4 N = normalize(TBN.transpose() * mapN);

        vec4 L = normalize(light);        
        vec4 V = normalize(fragPos * -1);      // 视线方向（假设相机在原点）

        // Blinn-Phong 高光
        vec4 H = normalize(L + V);         
        double diff = std::max(0., N * L);
        double spec = std::pow(std::max(0., N * H), 32.0) * 
            (sample2D(model.specular(), st)[0] / 255.0);


        double amb = 0.2;

        TGAColor col = sample2D(model.diffuse(), st);
        TGAColor glow = glowSample2D(model.glow(), st, 40);      
        double inten = amb + diff + spec;

        for (int i = 0; i < 3; i++) {
            col[i] = std::min<int>(255, col[i] * inten + 2.0 * glow[i]);
        }

        return { false, col };
    }
};

int main(int argc, char** argv) {
    if (argc < 2) return 1;

    const int W = 800, H = 800;

    vec3 eye{ -1,0,2 }, center{ 0,0,0 }, up{ 0,1,0 }, light{ 1,1,1 };

    lookat(eye, center, up);
    init_perspective(norm(eye - center));
    init_viewport(W / 16, H / 16, W * 7 / 8, H * 7 / 8);
    init_zbuffer(W, H);

    TGAImage fb(W, H, TGAImage::RGB);

    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);
        PhongShader shader(light, model);

        for (int i = 0; i < model.nfaces(); i++) {
            Triangle tri = {
                shader.vertex(i,0),
                shader.vertex(i,1),
                shader.vertex(i,2)
            };
            rasterize(tri, shader, fb);
        }
    }

    fb.write_tga_file("framebuffer.tga");
    return 0;
}
