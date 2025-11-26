## Prerrequisitos
Es necesario instalar lo siguiente en el entorno de desarrollo:
* Compilador de C++
* Las librerias en esta carpeta y en thirdparty junto con el CMakeLists.txt

## Ejecución

Una vez instalado lo anterior y tenerlos ordenada en una carpeta ejecutar

		cmake -B build

para crear la carpeta donde se compila el programa. Seguido ejecutar
		cmake --build build
alternativamente para tener una compilación más eficiente
		cmake --build build --config release

OJO: si se usa la primera opcion deberia bastar ejecutar

		.\build\RayTracer.exe
si se uso la alternativa

		.\build\Release\RayTracer.exe

## JSON para las escenas
Al inicio del archivo JSON se debe hacer la configuración de la cámara, para esto se debe considerar las siguientes variables:

* double aspect_ratio 
* int    image_width 
* int    samples_per_pixel  
* int    max_depth   
* double vfov
* color  background 
* point3 lookfrom
* point3 lookat 
* vec3   vup

el tipo de variable no se debe espcificar, solo se agrega aqui para mayor claridad del formato, en el caso de los double no usar expresiones racionales, sino decimales. Para ponint3/vec3, se debe usar un tipo array \[x,y,z\].

Despues de la cámera usar una instancia objetos que representara los objetos en el mundo, dependiendo del tipo de objeto los parametros de entrada cambiaran. Los tipos de primitivas son:

* Esferas
* Cilindros
* Cajas
* Paredes

A continuación los parámetros de cada una
### Esferas
* point3 center
* double radius
* material mat

### Cilindro
* point3 center
* double radius
* double height
* material mat

### Caja
* point3 p0
* point3 p1
* material mat

Para los materiales hay cinco tipos
* Lambertiano, similar a pelotas de frontón
* Dielectrico, tipo vidrio para superficies con cambio de indice de regracción
* Metalico, material reflectante
* Luz, para fuentes de luz
* Phong, tipo plástico

para los materiales de cada primitiva se deben especificar diferentes parametros segun el tipo

### Lambertiano
* color3 albedo
### Dielectrico
* double refraction_index
### Metálico
* color3 albedo
* double fuzz
### Luz
* color3 emit
### Phong
* color3 albedo
* double shininess
* double reflectivity

### Un ejemplo de configuracion de camara y un objeto
		{
			"camera": {
				"image_width": 900,
				"samples_per_pixel": 400,
				"max_depth": 50,
				"vfov": 90,
				"aspect_ratio": 1.7777777777777,
				"background": [0.0, 0.0, 0.0],
				"lookfrom": [0, 2, 7],
				"lookat": [0, 0, 0]
			},
			"objects": [
				{
					"type": "sphere",
					"center": [0, -1000.5, -1],
					"radius": 1000,
					"material": {
						"type": "lambertian",
						"albedo": [0.1, 0.9, 0.2]
					}
				}
			]
		}

Si se desea aplicar una textura, de momento solo para esferas, se debe especificar otro parametro "texture" con el nombre del archivo, en este caso el tipo de material por defecto sera Lambertiano. Esto es mas que nada por falta de tiempo, me hubiera gustado extenderlo a más primitivas y materiales.

## Referencias utilizadas
* [Ray Tracing in One Weekend]{https://raytracing.github.io/books/RayTracingTheNextWeek#texturemapping}
* [JSON en C++]{https://json.nlohmann.me/home/}

## Trabajo por hacer

* Añadir mas primitivas y optimizar input/output.
* Añadir texturas en todos los materiales.
* Corregir bug de transformación de escala.