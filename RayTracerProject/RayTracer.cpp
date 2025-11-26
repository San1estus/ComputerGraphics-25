#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "cylinder.h"
#include "rectangle.h"
#include <fstream>
#include "thirdparty/json.hpp"

using json = nlohmann::json;
// Lee un color o vec3 a partir de un array JSON [r, g, b]
color parse_color(const json& j) {
  if (j.is_array() && j.size() == 3) {
    return color(j[0].get<double>(), j[1].get<double>(), j[2].get<double>());
  }
  return color(0, 0, 0); 
}

// Crea un objeto material a partir de la sub-tabla "material" del JSON
shared_ptr<material> parse_material(const json& j) {
  std::string type = j.value("type", "lambertian");
  
  if (type == "lambertian") {
    color albedo = parse_color(j.value("albedo", json::array({0.5, 0.5, 0.5})));
    return make_shared<lambertian>(albedo);
  } else if (type == "metal") {
    color albedo = parse_color(j.value("albedo", json::array({0.7, 0.7, 0.7})));
    double fuzz = j.value("fuzz", 0.0);
    return make_shared<metal>(albedo, fuzz);
  } else if (type == "dielectric") {
    double ir = j.value("ir", 1.5);
    return make_shared<dielectric>(ir);
  } else if (type == "diffuse_light") {
    color emit = parse_color(j.value("emit", json::array({5, 5, 5})));
    return make_shared<diffuse_light>(emit);
  } else if (type == "phong") {
    
    // Difuso
    color kd = parse_color(j.value("albedo", json::array({0.5, 0.5, 0.5})));
    
    // Shininess
    double shininess = j.value("shininess", 500.0);
    
    // Reflectancia (?)
    double reflectivity = j.value("reflectivity", 0.5);
    
    return make_shared<phong_material>(kd, shininess, reflectivity);
    
  }
  
  // Fallback
  return make_shared<lambertian>(color(1, 0, 1)); 
}


Matrix4 parse_transformations(const json& j_transforms) {
  Matrix4 M;

  if (!j_transforms.is_array()) return M;

  for (const auto& j_t : j_transforms) {
    std::string type = j_t.value("type", "");
    Matrix4 T_new;

    if (type == "translate") {
      double x = j_t.value("x", 0.0);
      double y = j_t.value("y", 0.0);
      double z = j_t.value("z", 0.0);
      T_new = Matrix4::translate(x, y, z);
    } else if (type == "scale") {
      double x = j_t.value("x", 1.0);
      double y = j_t.value("y", 1.0);
      double z = j_t.value("z", 1.0);
      T_new = Matrix4::scale(x, y, z);
    } else if (type == "rotate_x") {
      double deg = j_t.value("degrees", 0.0);
      T_new = Matrix4::rotate_x(deg);
    } else if (type == "rotate_y") {
      double deg = j_t.value("degrees", 0.0);
      T_new = Matrix4::rotate_y(deg);
    } else if (type == "rotate_z") {
      double deg = j_t.value("degrees", 0.0);
      T_new = Matrix4::rotate_z(deg);
    } else if (type == "shear") {
      // Los valores por defecto deben ser 0 para no cizallar
      double xy = j_t.value("xy", 0.0);
      double xz = j_t.value("xz", 0.0);
      double yx = j_t.value("yx", 0.0);
      double yz = j_t.value("yz", 0.0);
      double zx = j_t.value("zx", 0.0);
      double zy = j_t.value("zy", 0.0);
      T_new = Matrix4::shear(xy, xz, yx, yz, zx, zy);
    }
    
    // Aplicamos las transformaciones en orden
    M = M * T_new;
  }
  return M;
}

/*
Primitivas disponibles: esferas, cajas, paredes, cilindros
Materiales disponibles: lambertiano(absorbe), dielectrico(refracta), metal(refleja), luz puntual, iluminado de Phong (tipo plástico)
Poscionamiento de camara.
Transformaiones afines, las rotaciones se hacen coordenada a coordenada.
*/

// En este hay un ejemplo de cada cosa
void pruebas(){
	hittable_list world;

	// Para materiales lambertianos
  auto lambertian1 = make_shared<lambertian>(color(0.8, 0.8, 0.0));
  auto lambertian2 = make_shared<lambertian>(color(0.1, 0.2, 0.5));
  auto lambertian3 = make_shared<lambertian>(color(0.1, 1.0, 0.2));
	// Para materiales dielectricos
	auto dielectric1 = make_shared<dielectric>(1.5);
  auto dielectric2   = make_shared<dielectric>(1.0 / 1.5);
	auto dielectric3 = make_shared<dielectric>(1.0/1.5);
	// Para materiales metalicos
  auto metal1  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
	auto metal2 = make_shared<metal>(color(0.8,0.6, 0.4), 0.0);
	auto metal3 = make_shared<metal>(color(0.8,0.1, 0.3), 0.1);
	// Para fuentes de luz
	auto luz1 = make_shared<diffuse_light>(color(1,1,1));
	auto luz2 = make_shared<diffuse_light>(color(0.3,0.3,0.3));
	auto luz3 = make_shared<diffuse_light>(color(100,100,100));

  // Para probar transformaciones
  auto caja = make_shared<box>(point3(-1.0,    1.0, -3.0), point3(0.0, 2.0, -2.0), luz1);

  Matrix4 S = Matrix4::scale(2, 1.5, 1.5);             // Escalar
  Matrix4 Sh = Matrix4::shear(1.0, 0.0, 0.0, 0.0, 0.0, 0.0); // Cizallar
  Matrix4 R = Matrix4::rotate_z(60);                     // Rotar
  Matrix4 T = Matrix4::translate(0.0, 0.0, 0.0);        // Mover

  Matrix4 M = T * R * Sh * S;

  world.add(make_shared<sphere>(point3( 0.0,    -100.5, -1.2),   100, lambertian3));
  world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.5, lambertian2));
  world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, dielectric2));
  world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.2, dielectric2));
  world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, metal1));
  world.add(make_shared<sphere>(point3(-1.0,    1.0, -2.0),   0.5, metal2));
  world.add(make_shared<cylinder>(point3(1.0,    1.0, -2.0),   0.5, 1, dielectric3));
  world.add(make_shared<sphere>(point3(2.0,    20, 5),   2.5, luz3));
  world.add(make_shared<affine_transform>(caja, M));

  camera cam;

  cam.aspect_ratio = 16.0 / 9.0;
  cam.image_width  = 900;
	cam.samples_per_pixel = 400;
	cam.max_depth = 50;
	cam.background = color(0.3,0.5,0.7);

	cam.vfov = 90;
	cam.lookfrom = point3(-0.5, 2.0, 0.6);
	cam.lookat = point3(0,0.7,-1);
	cam.vup = vec3(0,1,0);

	cam.defocus_angle = 0.0;

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
            // Paneles un poco debajo del techo para que se vean
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

// Todo lo anterior son escenas precargadas

void leer_escena(std::string s){
	hittable_list world;
  
  // Abrir el json
  std::ifstream f("scenes/"+s);
  if (!f.is_open()) {
    std::cerr << "Error: No se pudo abrir el archivo JSON" << std::endl;
    return;
  }
  
  json data;
  try {
    f >> data;
  } catch (json::parse_error& e) {
    std::cerr << "Error de parseo JSON: " << e.what() << std::endl;
    return;
  }

  // Configuracion de la cámara
  camera cam;
  if (data.contains("camera")) {
    const auto& j_cam = data["camera"];
    cam.image_width       = j_cam.value("image_width", cam.image_width);
    cam.samples_per_pixel = j_cam.value("samples_per_pixel", cam.samples_per_pixel);
    cam.max_depth         = j_cam.value("max_depth", cam.max_depth);
    cam.vfov              = j_cam.value("vfov", cam.vfov);
    cam.aspect_ratio      = j_cam.value("aspect_ratio", cam.aspect_ratio);
    
    // Lectura de vectores y colores 
    if (j_cam.contains("background")) cam.background = parse_color(j_cam["background"]);
    if (j_cam.contains("lookfrom")) cam.lookfrom = parse_color(j_cam["lookfrom"]);
    if (j_cam.contains("lookat")) cam.lookat = parse_color(j_cam["lookat"]);
    if (j_cam.contains("vup")) cam.vup = parse_color(j_cam["vup"]);
  }

  // Procesar objetos
  if (data.contains("objects") && data["objects"].is_array()) {
    for (const auto& j_obj : data["objects"]) {
      std::string type = j_obj.value("type", "unknown");
			std::string texture;
      if (!j_obj.contains("material")) continue;
			if(j_obj.contains("texture")) texture = j_obj.value("texture", "");
      
      shared_ptr<material> mat = parse_material(j_obj["material"]);
			shared_ptr<hittable> geometry_base = nullptr;
      
      // Esfera
      if (type == "sphere") {
        point3 center = parse_color(j_obj.value("center", json::array({0, 0, 0})));
        double radius = j_obj.value("radius", 1.0);
				if(j_obj.contains("texture")){
					auto surface = make_shared<lambertian>(make_shared<image_texture>(texture.c_str()));
        	geometry_base = make_shared<sphere>(center, radius, surface);
				}
				else{
					geometry_base = make_shared<sphere>(center, radius, mat);
				}
      } 

      // Cilindro
      else if (type == "cylinder") {
        point3 center = parse_color(j_obj.value("center", json::array({0, 0, 0})));
        double radius = j_obj.value("radius", 1.0);
        double height = j_obj.value("height", 2.0);
				geometry_base = make_shared<cylinder>(center, radius, height, mat);
      }

      // Caja
      else if (type == "box") {
        point3 p0 = parse_color(j_obj.value("p0", json::array({-1, -1, -1})));
        point3 p1 = parse_color(j_obj.value("p1", json::array({1, 1, 1})));
				geometry_base = make_shared<box>(p0, p1, mat);
      }
			else if (type == "xz_rect"){
				double x0 = j_obj.value("x0", 1.0);
				double x1 = j_obj.value("x1", 1.0);
				double z0 = j_obj.value("z0", 1.0);
				double z1 = j_obj.value("z1", 1.0);
				double k = j_obj.value("k", 1.0);
				geometry_base = make_shared<xz_rect>(x0,x1,z0,z1,k,mat);
			}
			else if (type == "xy_rect"){
				double x0 = j_obj.value("x0", 1.0);
				double x1 = j_obj.value("x1", 1.0);
				double y0 = j_obj.value("y0", 1.0);
				double y1 = j_obj.value("y1", 1.0);
				double k = j_obj.value("k", 1.0);
				geometry_base = make_shared<xy_rect>(x0,x1,y0,y1,k,mat);
			}
			else if (type == "yz_rect"){
				double y0 = j_obj.value("y0", 1.0);
				double y1 = j_obj.value("y1", 1.0);
				double z0 = j_obj.value("z0", 1.0);
				double z1 = j_obj.value("z1", 1.0);
				double k = j_obj.value("k", 1.0);
				geometry_base = make_shared<yz_rect>(y0,y1,z0,z1,k,mat);
			}
			if (geometry_base) {
        if (j_obj.contains("transforms") && j_obj["transforms"].is_array()) {
          
          Matrix4 M = parse_transformations(j_obj["transforms"]);
          
          world.add(make_shared<affine_transform>(geometry_base, M));
        } else {
          world.add(geometry_base);
        }
      }
    }
  }

  cam.render(world);
}

int main(){
	std::cout << "Selecciona una escena:\n";
	std::cout << "Escenas pre hechas:\n";
	std::cout << "1: Prueba de primitivas\n2: Prueba de texturas\n3:Prueba de luces\n\n4: Escena desde un archivo\n";
	int scene = 1;
	std::cin >> scene;
	std::string escena;
	switch(scene){
		case 1: pruebas(); break;
		case 2: textura_imagen(); break;
		case 3: iluminacion(); break;
		case 4:{
			std::cout << "Ruta de escena, se asume que esta en la carpeta scenes: ";
			std::cin >> escena;
			leer_escena(escena);
		}
		default: break;
	}
	
}