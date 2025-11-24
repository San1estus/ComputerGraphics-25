#ifndef CYLINDER_H
#define CYLINDER_H

#include "hittable.h"
#include "vec3.h"
#include <memory>

class cylinder : public hittable {
public:
  point3 center;
  double radius;
  double height;
  shared_ptr<material> mat;

  cylinder(const point3& c, double r, double h, shared_ptr<material> m)
    : center(c), radius(r), height(h), mat(m) {}

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {

    bool hit_anything = false;
    hit_record temp_rec;
    double closest_t = ray_t.max;

    vec3 oc = r.origin() - center;
    double half_h = height / 2.0;

    // Contorno

    double a = r.direction().x()*r.direction().x()
             + r.direction().z()*r.direction().z();
    double b = 2*(oc.x()*r.direction().x() + oc.z()*r.direction().z());
    double c = oc.x()*oc.x() + oc.z()*oc.z() - radius*radius;

    double discriminant = b*b - 4*a*c;

    if (discriminant >= 0) {
      double sqrtd = sqrt(discriminant);

      double root = (-b - sqrtd) / (2*a);
      if (!ray_t.surrounds(root))
        root = (-b + sqrtd) / (2*a);

      if (ray_t.surrounds(root)) {
        double y = oc.y() + root * r.direction().y();

        if (y >= -half_h && y <= half_h) {
          if (root < closest_t) {
            closest_t = root;
            hit_anything = true;

            temp_rec.t = root;
            temp_rec.p = r.at(root);

            vec3 outward_normal = unit_vector(
              vec3(temp_rec.p.x() - center.x(),
                   0,
                   temp_rec.p.z() - center.z())
            );

            temp_rec.set_face_normal(r, outward_normal);
            temp_rec.mat = mat;
          }
        }
      }
    }

    // Tapa inferior

    double y_bottom = center.y() - half_h;
    double denom = r.direction().y();

    if (fabs(denom) > 1e-8) {
      double t = (y_bottom - r.origin().y()) / denom;

      if (ray_t.surrounds(t) && t < closest_t) {
        point3 p = r.at(t);

        double dx = p.x() - center.x();
        double dz = p.z() - center.z();

        if (dx*dx + dz*dz <= radius*radius) {
          closest_t = t;
          hit_anything = true;

          temp_rec.t = t;
          temp_rec.p = p;

          temp_rec.set_face_normal(r, vec3(0, -1, 0));
          temp_rec.mat = mat;
        }
      }
    }

    // Tapa superior

    double y_top = center.y() + half_h;

    if (fabs(denom) > 1e-8) {
      double t = (y_top - r.origin().y()) / denom;

      if (ray_t.surrounds(t) && t < closest_t) {
        point3 p = r.at(t);

        double dx = p.x() - center.x();
        double dz = p.z() - center.z();

        if (dx*dx + dz*dz <= radius*radius) {
          closest_t = t;
          hit_anything = true;

          temp_rec.t = t;
          temp_rec.p = p;

          temp_rec.set_face_normal(r, vec3(0, 1, 0));
          temp_rec.mat = mat;
        }
      }
    }

    if (hit_anything)
      rec = temp_rec;

    return hit_anything;
  }
};

#endif
