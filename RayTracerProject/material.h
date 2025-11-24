#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"


class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
      return false;
    }
		virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
      return color(0,0,0);
    }
};

class lambertian : public material{
	public:
		lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
		lambertian(shared_ptr<texture> tex) : tex(tex) {}
		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override{
			auto scatter_direction = rec.normal + random_unit_vector();
			if(scatter_direction.near_zero()){
				scatter_direction = rec.normal;
			}
			scattered = ray(rec.p, scatter_direction);
			attenuation = tex->value(rec.u, rec.v, rec.p);
			return true;
		}

	private:
	shared_ptr<texture>tex;
};

class metal : public material{
	public:
		metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override{
			vec3 reflected = reflect(r_in.direction(), rec.normal);
			reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
			scattered = ray(rec.p, reflected);
			attenuation = albedo;
			return (dot(scattered.direction(), rec.normal) > 0);
		}
	private:
		color albedo;
		double fuzz;
};

class dielectric : public material{
	public:
		dielectric(double refraction_index) : refraction_index(refraction_index) {}
		bool scatter(const ray& r_in, const hit_record& rec, color& attenuation,  ray& scattered) const override{
			attenuation = color(1.0, 1.0, 1.0);
			double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

			vec3 unit_direction = unit_vector(r_in.direction());
			double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
			double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);
			bool cannot_refract = ri * sin_theta > 1.0;
      vec3 direction;

      if (cannot_refract || reflectance(cos_theta, ri) > random_double())
        direction = reflect(unit_direction, rec.normal);
      else
        direction = refract(unit_direction, rec.normal, ri);

      scattered = ray(rec.p, direction);
			return true;
		}
	private:
		double refraction_index;
		static double reflectance(double cosine, double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class diffuse_light : public material {
  public:
    diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {}

    color emitted(const ray& r_in, const hit_record& r, double u, double v, const point3& p) const override {
        return tex->value(u, v, p);
    }

  private:
    shared_ptr<texture> tex;
};

class phong_material : public material {
public:
    color albedo;       // Color difuso 
    double shininess;   // Que tan concentrado es el brillo
    double reflectivity;// Probabilidad de que el rayo refleje brillo

    phong_material(const color& a, double s, double r) 
        : albedo(a), shininess(s), reflectivity(r) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        // Decidimos aleatoriamente si este rayo contribuye al componente DIFUSO o ESPECULAR
        // basándonos en la reflectividad del material.
        
        if (random_double() < reflectivity) {
            // --- COMPONENTE ESPECULAR (El "Brillo") ---
            // Calculamos la reflexión perfecta
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            
            // Simular Shininess:
            // Un shininess alto significa poca dispersión (fuzz bajo).
            // Un shininess bajo significa mucha dispersión.
            // Fórmula empírica para convertir exponente Phong a "fuzz":
            double fuzz = (shininess > 1000.0) ? 0.0 : (1.0 - (shininess / 1000.0));
            // O una curva más agresiva: fuzz = 2.0 / (shininess + 2.0);

            // Generamos el rayo reflejado con un poco de aleatoriedad (fuzz)
            scattered = ray(rec.p, unit_vector(reflected) + fuzz * random_unit_vector());
            
            // El color del brillo suele ser blanco en plásticos, o el albedo en metales.
            // Para efecto Phong clásico (plástico), el brillo es blanco.
            attenuation = color(1.0, 1.0, 1.0); 

            // Si el rayo se metió dentro del objeto por el fuzz, lo invalidamos (absorbemos)
            return (dot(scattered.direction(), rec.normal) > 0);

        } else {
            // --- COMPONENTE DIFUSO (El Color Mate) ---
            // Rebote Lambertiano estándar
            vec3 scatter_direction = rec.normal + random_unit_vector();
            
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal;
                
            scattered = ray(rec.p, scatter_direction);
            attenuation = albedo; // Aquí usamos el color del objeto
            return true;
        }
    }
};
#endif