## Prerrequisitos
Es necesario instalar lo siguiente en el entorno de desarrollo:
* [cmake](https://cmake.org/)
* Compilador de C++
* [GLFW](https://www.glfw.org/)
* [GLEW](https://glew.sourceforge.net/)
* [GLM](https://github.com/g-truc/glm/releases)

Para **Windows usar MSYS UCRT64**. En este se puede installar `cmake`, `g++`, `glfw`, `glew` y `glm` usando pacman. Para buscar se usa ``pacman -Ss (expresion regular a buscar)``.
Para instalar usar ``pacman -S (direccion del archivo)``, la direccion se encuentra con lo anterior, asegurarse de que sea para ucrt.

## Ejecución

Una vez instalado lo anterior, ir al directorio de animaciones y ejecutar lo siguiente para configurar el proyecto

    cmake -S . -B build

seguido construir el proyecto con

    cmake --build build

Para ejecutarlo:

    ./build/puppet.exe


## Controles

Para mover la cámara en el espacio se usa WASD, para rotar la cámara es con el movimiento del ratón, también se puede hacer zoom con la rueda del mismo.

Se tiene el siguiente orden para las articulaciones
<!---
Esta lista se puede actualizar
-->

* 0 -- Torso
* 1 -- Cabeza
* 2 -- Hombro Izquierdo/Derecho
* 3 -- Codo Izquierdo/Derecho
* 4 -- Muñeca Izquierda/Derecha
* 5 -- Cadera Izquierda/Derecha
* 6 -- Rodilla Izquierda/Derecha
* 7 -- Tobillo Izquierdo/Derecho


## Referencias utilizadas
* [learnopengl]{https://learnopengl.com/}
* [glfw]{https://www.glfw.org/docs/latest/group__input.html}

## Trabajo por hacer

* Automatizar el posicionamiento de los pivotes para poder generalizar el input.