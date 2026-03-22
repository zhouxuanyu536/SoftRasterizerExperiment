#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int N>
struct vec {
    double v[N]{};

    double& operator[](int i) { assert(i >= 0 && i < N); return v[i]; }
    double  operator[](int i) const { assert(i >= 0 && i < N); return v[i]; }
};

template<int N>
double operator*(const vec<N>& a, const vec<N>& b) {
    double r = 0;
    for (int i = 0; i < N; i++) r += a[i] * b[i];
    return r;
}

template<int N>
vec<N> operator+(vec<N> a, const vec<N>& b) {
    for (int i = 0; i < N; i++) a[i] += b[i];
    return a;
}

template<int N>
vec<N> operator-(vec<N> a, const vec<N>& b) {
    for (int i = 0; i < N; i++) a[i] -= b[i];
    return a;
}

template<int N>
vec<N> operator*(vec<N> a, double s) {
    for (int i = 0; i < N; i++) a[i] *= s;
    return a;
}

template<int N>
vec<N> operator/(vec<N> a, double s) {
    for (int i = 0; i < N; i++) a[i] /= s;
    return a;
}

template<> struct vec<2> {
    double x{}, y{};
    double& operator[](int i) { assert(i < 2); return i ? y : x; }
    double  operator[](int i) const { assert(i < 2); return i ? y : x; }
};

template<> struct vec<3> {
    double x{}, y{}, z{};
    double& operator[](int i) { assert(i < 3); return i == 0 ? x : (i == 1 ? y : z); }
    double  operator[](int i) const { assert(i < 3); return i == 0 ? x : (i == 1 ? y : z); }
};

template<> struct vec<4> {
    double x{}, y{}, z{}, w{};
    vec() = default;
    vec(double x, double y, double z, double w) :x(x), y(y), z(z), w(w) {}
    double& operator[](int i) { assert(i < 4); return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
    double  operator[](int i) const { assert(i < 4); return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
    vec<2> xy() const { return { x,y }; }
    vec<3> xyz() const { return { x,y,z }; }
};

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

template<int N>
double norm(const vec<N>& v) { return std::sqrt(v * v); }

template<int N>
vec<N> normalize(const vec<N>& v) { return v / norm(v); }

inline vec3 cross(const vec3& a, const vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

template<int R, int C>
struct mat {
    vec<C> r[R];

    vec<C>& operator[](int i) { assert(i < R); return r[i]; }
    const vec<C>& operator[](int i) const { assert(i < R); return r[i]; }

    mat<C, R> transpose() const {
        mat<C, R> res;
        for (int i = 0; i < R; i++)
            for (int j = 0; j < C; j++)
                res[j][i] = r[i][j];
        return res;
    }
};

template<int R, int C>
vec<R> operator*(const mat<R, C>& m, const vec<C>& v) {
    vec<R> res;
    for (int i = 0; i < R; i++) res[i] = m[i] * v;
    return res;
}

template<int R, int C, int K>
mat<R, K> operator*(const mat<R, C>& a, const mat<C, K>& b) {
    mat<R, K> res;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < K; j++)
            for (int k = 0; k < C; k++)
                res[i][j] += a[i][k] * b[k][j];
    return res;
}

template<int R, int C>
mat<R, C> operator*(mat<R, C> m, double s) {
    for (int i = 0; i < R; i++) m[i] = m[i] * s;
    return m;
}

template<int R, int C>
mat<R, C> operator/(mat<R, C> m, double s) {
    for (int i = 0; i < R; i++) m[i] = m[i] / s;
    return m;
}

template<int N>
struct det_impl {
    static double det(const mat<N, N>& m) {
        double res = 0;
        for (int c = 0; c < N; c++) {
            mat<N - 1, N - 1> sub;
            for (int i = 1; i < N; i++)
                for (int j = 0, k = 0; j < N; j++)
                    if (j != c) sub[i - 1][k++] = m[i][j];
            res += (c % 2 ? -1 : 1) * m[0][c] * det_impl<N - 1>::det(sub);
        }
        return res;
    }
};

template<>
struct det_impl<1> {
    static double det(const mat<1, 1>& m) { return m[0][0]; }
};

template<int N>
double det(const mat<N, N>& m) { return det_impl<N>::det(m); }

template<int N>
mat<N, N> inverse(const mat<N, N>& m) {
    mat<N, N> adj;
    double d = det(m);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mat<N - 1, N - 1> sub;
            for (int r = 0, rr = 0; r < N; r++) {
                if (r == i) continue;
                for (int c = 0, cc = 0; c < N; c++) {
                    if (c == j) continue;
                    sub[rr][cc++] = m[r][c];
                }
                rr++;
            }
            adj[j][i] = ((i + j) % 2 ? -1 : 1) * det_impl<N - 1>::det(sub);
        }
    }
    return adj / d;
}

template<int N>
std::ostream& operator<<(std::ostream& os, const vec<N>& v) {
    for (int i = 0; i < N; i++) os << v[i] << " ";
    return os;
}

template<int R, int C>
std::ostream& operator<<(std::ostream& os, const mat<R, C>& m) {
    for (int i = 0; i < R; i++) os << m[i] << "\n";
    return os;
}