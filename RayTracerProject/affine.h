#ifndef AFFINE_TRANSFORM_H
#define AFFINE_TRANSFORM_H

#include "hittable.h"
#include "mat4.h"

class affine_transform : public hittable {
public:
  shared_ptr<hittable> object;
  Matrix4 transform_matrix;
  Matrix4 inverse_matrix;
  Matrix4 normal_matrix; 

  affine_transform(shared_ptr<hittable> obj, const Matrix4& m) : object(obj), transform_matrix(m) {
    if (!Matrix4::inverse(transform_matrix, inverse_matrix)) {
      std::cerr << "Error: Matriz singular." << std::endl;
    }
    normal_matrix = inverse_matrix.transpose();
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    // Recuperamos la posicion
    point3 origin_local = inverse_matrix.mult_point(r.origin());
    vec3 dir_local = inverse_matrix.mult_vec(r.direction());

    ray ray_local(origin_local, dir_local);

    // Intersección en espacio local
    if (!object->hit(ray_local, ray_t, rec))
      return false;

    // Aplicamos la transformacion a los puntos
    rec.p = transform_matrix.mult_point(rec.p);

    // Transformación correcta de normales
    vec3 normal_world = normal_matrix.mult_vec(rec.normal);
    rec.set_face_normal(r, unit_vector(normal_world));

    return true;
  }
};

#endif