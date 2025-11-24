#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "cylinder.h"
#include "rectangle.h"

void prueba_materiales(){
	hittable_list world;

  auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
  auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
  auto material_bubble = make_shared<dielectric>(1.5);
  auto material_left   = make_shared<dielectric>(1.0 / 1.5);
  auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
	auto material_top_left = make_shared<metal>(color(0.8,0.6, 0.4), 0.0);
	auto material_top_right = make_shared<dielectric>(1.0/1.5);
	auto material_back = make_shared<metal>(color(0.8,0.1, 0.3), 0.1);

  world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
  world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.5, material_center));
  world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
  world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.2, material_left));
  world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
  world.add(make_shared<sphere>(point3(-1.0,    1.0, -2.0),   0.5, material_top_left));
  world.add(make_shared<cylinder>(point3(1.0,    1.0, -2.0),   0.5, 1, material_top_right));
  world.add(make_shared<box>(point3(-1.0,    1.0, -3.0), point3(0.0, 2.0, -2.0), material_back));

  camera cam;

  cam.aspect_ratio = 16.0 / 9.0;
  cam.image_width  = 900;
	cam.samples_per_pixel = 100;
	cam.max_depth = 10;
	cam.background = color(0.7, 0.8, 1.00);

	cam.vfov = 90;
	cam.lookfrom = point3(-0.5, 2.0, 0.6);
	cam.lookat = point3(0,0.7,-1);
	cam.vup = vec3(0,1,0);

	cam.defocus_angle = 0.0;
	cam.focus_dist = 1.0;

  cam.render(world);
}

void textura_imagen(){
	auto earth_texture = make_shared<image_texture>("imagen.jpg");
	auto earth_surface = make_shared<lambertian>(earth_texture);
  auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 400;
  cam.samples_per_pixel = 100;
  cam.max_depth         = 50;
	cam.background = color(0.7, 0.8, 1.00);

  cam.vfov     = 20;
  cam.lookfrom = point3(0,0,12);
  cam.lookat   = point3(0,0,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0;

  cam.render(hittable_list(globe));
}

void iluminacion() {
    hittable_list world;

    auto luz_fuerte = make_shared<diffuse_light>(color(10, 10, 10)); 
    world.add(make_shared<sphere>(point3(0, 5, 0), 1.0, luz_fuerte));

    auto mat_rojo = make_shared<phong_material>(color(0.8, 0.1, 0.1), 100.0, 0.3);
    
    auto mat_azul = make_shared<phong_material>(color(0.1, 0.1, 0.8), 1000.0, 0.5);

    world.add(make_shared<sphere>(point3(-1.2, 0.0, -1), 0.5, mat_rojo));
    world.add(make_shared<sphere>(point3( 1.2, 0.0, -1), 0.5, mat_azul));
    world.add(make_shared<xz_rect>(-10.0,10.0, -10.0, 10.0, -0.51, make_shared<lambertian>(color(0.6, 0.3, 0))));

    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 900;
    cam.samples_per_pixel = 400;
    cam.max_depth = 50;
    cam.background = color(0.3,0.5,0.7); // Fondo negro para ver bien la luz
    cam.vfov = 30;
    cam.lookfrom = point3(0, 2, 5);
    cam.lookat = point3(0, 0, -1);
    cam.vup = vec3(0, 1, 0);

    cam.render(world);
}


void escena_infinita() {
    hittable_list world;

    auto material_espejo = make_shared<metal>(color(1.0, 1.0, 1.0), 0.0);
    
    auto material_suelo = make_shared<lambertian>(color(0.6, 0.3, 0));
    
    auto material_techo = make_shared<lambertian>(color(0.1, 0.1, 0.1));
    
    auto material_luz = make_shared<diffuse_light>(color(1, 1, 1));
    
    auto material_personaje = make_shared<phong_material>(color(0.1, 0.2, 0.8), 500.0, 0.5);


    double tam = 2.5;   
    double altura = 3.0;

    // Suelo
    world.add(make_shared<xz_rect>(-tam, tam, -tam, tam, 0.0, material_suelo));

    // Techo 
    world.add(make_shared<xz_rect>(-tam, tam, -tam, tam, altura, material_techo));

    // Paredes (Espejos)
    // Pared Trasera
    world.add(make_shared<xy_rect>(-tam, tam, 0, altura, -tam, material_espejo));
    // Pared Frontal 
    world.add(make_shared<xy_rect>(-tam, tam, 0, altura, tam, material_espejo));
    // Pared Izquierda
    world.add(make_shared<yz_rect>(0, altura, -tam, tam, -tam, material_espejo));
    // Pared Derecha 
    world.add(make_shared<yz_rect>(0, altura, -tam, tam, tam, material_espejo));


    // Techo
    double panel_size = 0.8;
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            double x_center = i * 1.5;
            double z_center = j * 1.5;
            // Paneles un poco debajo del techodel techo (altura - 0.01) para que se vean
            world.add(make_shared<xz_rect>(
                x_center - panel_size/2, x_center + panel_size/2,
                z_center - panel_size/2, z_center + panel_size/2,
                altura - 0.01, 
                material_luz
            ));
        }
    }

    // Simulamos una figura humana abstracta con esferas
    world.add(make_shared<sphere>(point3(0, 0.5, 0), 0.5, material_personaje)); // Cuerpo
    world.add(make_shared<sphere>(point3(0, 1.2, 0), 0.25, material_personaje)); // Cabeza


    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 900; 
    
    cam.max_depth = 50; 
    
    cam.samples_per_pixel = 200; 
    
    cam.background = color(0,0,0);

    cam.vfov = 70;
    
    cam.lookfrom = point3(-1.5, 1.6, 1.5);
    cam.lookat = point3(0, 0.8, 0); 
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}

int main(){
	int scene = 3;
	switch(scene){
		case 1: prueba_materiales(); break;
		case 2: textura_imagen(); break;
		case 3: iluminacion(); break;
		case 4: escena_infinita(); break;
		default: break;
	}
	
}