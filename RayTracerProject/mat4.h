#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "vec3.h"
#include <cmath>
#include <iostream>

struct Matrix4 {
  double m[4][4];

  Matrix4() {
    // Matriz identidad
    for(int i=0; i<4; i++)
      for(int j=0; j<4; j++)
        m[i][j] = (i == j) ? 1.0 : 0.0;
  }

  // Multiplicación Matriz * Punto con coordenadas homogeneas
  point3 mult_point(const point3& p) const {
    double x = p.x()*m[0][0] + p.y()*m[0][1] + p.z()*m[0][2] + m[0][3];
    double y = p.x()*m[1][0] + p.y()*m[1][1] + p.z()*m[1][2] + m[1][3];
    double z = p.x()*m[2][0] + p.y()*m[2][1] + p.z()*m[2][2] + m[2][3];
    return point3(x, y, z);
  }

  // Multiplicación Matriz * Vector coordenadas homogeneas
  vec3 mult_vec(const vec3& v) const {
    double x = v.x()*m[0][0] + v.y()*m[0][1] + v.z()*m[0][2];
    double y = v.x()*m[1][0] + v.y()*m[1][1] + v.z()*m[1][2];
    double z = v.x()*m[2][0] + v.y()*m[2][1] + v.z()*m[2][2];
    return vec3(x, y, z);
  }
  
  // Transpuesta
  Matrix4 transpose() const {
    Matrix4 res;
    for(int i=0; i<4; i++)
      for(int j=0; j<4; j++)
        res.m[i][j] = m[j][i];
    return res;
  }

  // Inversa por G-J
  static bool inverse(const Matrix4& in, Matrix4& out) {
    Matrix4 mat = in;
    Matrix4 res; 

    for (int i = 0; i < 4; i++) {
      double pivot = mat.m[i][i];
      if (std::abs(pivot) < 1e-8) return false;

      double invPivot = 1.0 / pivot;
      for (int j = 0; j < 4; j++) {
        mat.m[i][j] *= invPivot;
        res.m[i][j] *= invPivot;
      }

      for (int k = 0; k < 4; k++) {
        if (k != i) {
          double factor = mat.m[k][i];
          for (int j = 0; j < 4; j++) {
            mat.m[k][j] -= factor * mat.m[i][j];
            res.m[k][j] -= factor * res.m[i][j];
          }
        }
      }
    }
    out = res;
    return true;
  }
  
  // Transformaciones
  
  static Matrix4 translate(double x, double y, double z) {
    Matrix4 res;
    res.m[0][3] = x; res.m[1][3] = y; res.m[2][3] = z;
    return res;
  }

  static Matrix4 scale(double sx, double sy, double sz) {
    Matrix4 res;
    res.m[0][0] = sx; res.m[1][1] = sy; res.m[2][2] = sz;
    return res;
  }

  static Matrix4 rotate_x(double degrees) {
    Matrix4 res;
    double rad = degrees_to_radians(degrees);
    double c = cos(rad), s = sin(rad);
    res.m[1][1] = c; res.m[1][2] = -s;
    res.m[2][1] = s; res.m[2][2] = c;
    return res;
  }

  static Matrix4 rotate_y(double degrees) {
    Matrix4 res;
    double rad = degrees_to_radians(degrees);
    double c = cos(rad), s = sin(rad);
    res.m[0][0] = c; res.m[0][2] = s;
    res.m[2][0] = -s; res.m[2][2] = c;
    return res;
  }

  static Matrix4 rotate_z(double degrees) {
    Matrix4 res;
    double rad = degrees_to_radians(degrees);
    double c = cos(rad), s = sin(rad);
    res.m[0][0] = c; res.m[0][1] = -s;
    res.m[1][0] = s; res.m[1][1] = c;
    return res;
  }
  
  static Matrix4 shear(double xy, double xz, double yx, double yz, double zx, double zy) {
    Matrix4 res;
    res.m[0][1] = xy; res.m[0][2] = xz;
    res.m[1][0] = yx; res.m[1][2] = yz;
    res.m[2][0] = zx; res.m[2][1] = zy;
    return res;
  }

  friend Matrix4 operator*(const Matrix4& A, const Matrix4& B) {
    Matrix4 res;
    for(int i=0; i<4; i++) {
      for(int j=0; j<4; j++) {
        res.m[i][j] = 0;
        for(int k=0; k<4; k++)
          res.m[i][j] += A.m[i][k] * B.m[k][j];
      }
    }
    return res;
  }
};

#endif