#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <cmath> 
#include <vector>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define pb(a) push_back(a)

const float PI = acos(-1);

using namespace std;

class Sphere{
    public:
    vector<float> vertices;
    vector<unsigned int> indices;
    
    Sphere(float radius, int segmentsX, int segmentsY){
        for(int y = 0; y <= segmentsY; y++){
            for(int x = 0; x <= segmentsX; x++){
                float xSegment = (float)x/(float)segmentsX;
                float ySegment = (float)y/(float)segmentsY;
                
                // phi de 0 a PI 
                float phi = ySegment * PI; 
                // theta de 0 a 2*PI
                float theta = xSegment * 2.0f * PI; 

                // X = r * sin(phi) * cos(theta)
                float xPos = sin(phi) * cos(theta);
                // Y = r * cos(phi)  (Altura)
                float yPos = cos(phi); 
                // Z = r * sin(phi) * sin(theta)
                float zPos = sin(phi) * sin(theta);

                // Posición (X, Y, Z)
                vertices.pb(radius * xPos);
                vertices.pb(radius * yPos);
                vertices.pb(radius * zPos);

                // Normal (X, Y, Z)
                vertices.pb(xPos);
                vertices.pb(yPos);
                vertices.pb(zPos);
                

            }
        }
        
        bool oddRow = false;

        for(int y = 0; y < segmentsY; y++){ 
            for(int x = 0; x < segmentsX; x++){
                unsigned int p1 = y * (segmentsX + 1) + x;
                unsigned int p2 = y * (segmentsX + 1) + x + 1;
                unsigned int p3 = (y+1) * (segmentsX + 1) + x;
                unsigned int p4 = (y+1) * (segmentsX + 1) + x + 1;
                
                if(!oddRow){
                    // Triángulo 1
                    indices.pb(p1);
                    indices.pb(p2);
                    indices.pb(p3); 
                    
                    // Triángulo 2
                    indices.pb(p2);
                    indices.pb(p4);
                    indices.pb(p3);
                }
                else{

                    indices.pb(p1);
                    indices.pb(p2);
                    indices.pb(p4); 

                    indices.pb(p1);
                    indices.pb(p4);
                    indices.pb(p3);
                }
            }
            // Alternar fila par impar de arriba hacia abajo
            oddRow = !oddRow; 
        }
    }
};


static unsigned int CompileShader(unsigned int type, const string& source){
    unsigned int id = glCreateShader(type);
	const char* src = source.c_str(); 
	glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
		cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << '\n';
        cout << message << '\n';

        glDeleteShader(id);

        return 0;
    }

    return id;
}

static unsigned  int CreateShader(const string& vertexShader, string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
    glLinkProgram(program);
	glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

	return program;
}


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "The Puppet", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cout << "Error" << '\n';
    }

    cout << glGetString(GL_VERSION) << '\n';

    // Habilitar Z-buffer 
    glEnable(GL_DEPTH_TEST); 
    // Habilitar Face Culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    Sphere sphere(1.0f, 36, 18); // Generación de la esfera
    
    unsigned int VAO, VBO, EBO;
    
    // Generar y Vincular VAO 
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBO 
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(float), sphere.vertices.data(), GL_STATIC_DRAW);

    // Index Buffer
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(unsigned int), sphere.indices.data(), GL_STATIC_DRAW);

    // Stride = 6 * sizeof(float) (3 Posición + 3 Normal)
    GLsizei stride = 6 * sizeof(float);

    // Iluminacion

    // Atributo 0: Posición (Offset 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    
    // Atributo 1: Normal (Offset 3 * sizeof(float))
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Desvincular VAO
    glBindVertexArray(0);


    ifstream streamvertex("res/shaders/vertex.vs");
    stringstream ssvertex;
    ssvertex << streamvertex.rdbuf();

    string vertexShader = ssvertex.str();

    ifstream streamfragment("res/shaders/fragment.fs");
    stringstream ssfragment;
    ssfragment << streamfragment.rdbuf();

    string fragmentShader = ssfragment.str();

    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader);


    // --- Configuración Inicial de Matrices y Luz ---
    glm::mat4 Proj = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 100.0f);
    glm::mat4 View = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // Cámara
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f); 
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);  
    glm::vec3 objectColor(0.1f, 0.5f, 0.9f); // Color azul

    // Obtener ubicaciones de uniformes
    int mvpLocation = glGetUniformLocation(shader, "u_MVP");
    int modelLocation = glGetUniformLocation(shader, "u_Model");
    int lightPosLocation = glGetUniformLocation(shader, "u_LightPos");
    int viewPosLocation = glGetUniformLocation(shader, "u_ViewPos");
    int colorLocation = glGetUniformLocation(shader, "u_ObjectColor");

    // Asignar uniformes estáticos de LUZ y CÁMARA (solo una vez)
    glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
    glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPos));
    glUniform3fv(colorLocation, 1, glm::value_ptr(objectColor));

    float rotation = 0.0f;
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        // Calcular delta time para rotación suave
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // Rotación continua para ver la geometría
        rotation += 50.0f * deltaTime; // 50 grados por segundo
        glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 MVP = Proj * View * Model;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar color y profundidad
        
        glUseProgram(shader);
        
        // Asignar uniformes dinámicos
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP)); 
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Model));
        
        // ** VINCULAR VAO ANTES DE DIBUJAR **
        glBindVertexArray(VAO);
        
        glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, nullptr);
        
        // No es necesario desvincular el VAO aquí, pero sí es buena práctica al final del frame.
        // Lo dejamos vinculado mientras se dibuja.

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Limpieza...
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}