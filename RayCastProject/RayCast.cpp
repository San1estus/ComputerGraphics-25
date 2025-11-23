#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "cylinder.h"
#include "rectangle.h"

int main(){
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

	cam.vfov = 90;
	cam.lookfrom = point3(-0.5, 2.0, 0.6);
	cam.lookat = point3(0,0.7,-1);
	cam.vup = vec3(0,1,0);

	cam.defocus_angle = 0.0;
	cam.focus_dist = 1.0;

  cam.render(world);
}