#ifndef XY_RECT_H
#define XY_RECT_H

#include "hittable.h"

class xy_rect : public hittable {
public:
    double x0, x1;
    double y0, y1;
    double k;                 // Valor Z constante
    shared_ptr<material> mat;

    xy_rect() {}

    xy_rect(double _x0, double _x1, double _y0, double _y1, double _k,
            shared_ptr<material> m)
        : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mat(m) {}

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // t para intersecar el plano z = k
        double t = (k - r.origin().z()) / r.direction().z();
        if (!ray_t.surrounds(t))
            return false;

        double x = r.origin().x() + t*r.direction().x();
        double y = r.origin().y() + t*r.direction().y();

        if (x < x0 || x > x1 || y < y0 || y > y1)
            return false;

        rec.t = t;
        rec.p = r.at(t);

        vec3 outward_normal(0, 0, 1);
        rec.set_face_normal(r, outward_normal);

        rec.mat = mat;
        return true;
    }
};

#endif


#ifndef XZ_RECT_H
#define XZ_RECT_H

#include "hittable.h"

class xz_rect : public hittable {
public:
    double x0, x1;
    double z0, z1;
    double k;                // Valor Y constante
    shared_ptr<material> mat;

    xz_rect() {}

    xz_rect(double _x0, double _x1, double _z0, double _z1, double _k,
            shared_ptr<material> m)
        : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mat(m) {}

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // t para plano y = k
        double t = (k - r.origin().y()) / r.direction().y();
        if (!ray_t.surrounds(t))
            return false;

        double x = r.origin().x() + t*r.direction().x();
        double z = r.origin().z() + t*r.direction().z();

        if (x < x0 || x > x1 || z < z0 || z > z1)
            return false;

        rec.t = t;
        rec.p = r.at(t);

        vec3 outward_normal(0, 1, 0);
        rec.set_face_normal(r, outward_normal);

        rec.mat = mat;
        return true;
    }
};

#endif


#ifndef YZ_RECT_H
#define YZ_RECT_H

#include "hittable.h"

class yz_rect : public hittable {
public:
    double y0, y1;
    double z0, z1;
    double k;                // Valor X constante
    shared_ptr<material> mat;

    yz_rect() {}

    yz_rect(double _y0, double _y1, double _z0, double _z1, double _k,
            shared_ptr<material> m)
        : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mat(m) {}

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // t para el plano x = k
        double t = (k - r.origin().x()) / r.direction().x();
        if (!ray_t.surrounds(t))
            return false;

        double y = r.origin().y() + t*r.direction().y();
        double z = r.origin().z() + t*r.direction().z();

        if (y < y0 || y > y1 || z < z0 || z > z1)
            return false;

        rec.t = t;
        rec.p = r.at(t);

        vec3 outward_normal(1, 0, 0);
        rec.set_face_normal(r, outward_normal);

        rec.mat = mat;
        return true;
    }
};

#endif


#ifndef BOX_H
#define BOX_H

// Caja compuesta por 6 rectángulos axis-aligned
class box : public hittable {
public:
    point3 box_min;
    point3 box_max;
    hittable_list sides;

    box() {}
    box(const point3& p0, const point3& p1, shared_ptr<material> mat)
        : box_min(p0), box_max(p1)
    {
        // 6 caras
        sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), mat)); // +Z
        sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), mat)); // -Z

        sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), mat)); // +Y
        sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), mat)); // -Y

        sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), mat)); // +X
        sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), mat)); // -X
    }

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        return sides.hit(r, ray_t, rec);
    }
};

#endif
