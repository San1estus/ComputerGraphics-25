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

class Joint;

// Iniciar con pantalla centrada
float lastX = 960;
float lastY = 540;
float yaw = -90.0, roll, pitch, fov = 45.0;


Joint* selectedJoint = nullptr;
std::vector<Joint*> jointList;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

const float MOUSE_SENSITIVITY = 0.1f;
bool firstMouse = true;

class Renderer{
    private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    public:
    Renderer(const vector<float> vertices, const vector<unsigned int> indices):indexCount(indices.size()){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        GLsizei stride = 6*sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw(unsigned int shaderProgram, const glm::mat4 mvp, const glm::mat4& model, const glm::vec3& color){
        int mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP"); 
        int modelLocation = glGetUniformLocation(shaderProgram, "u_Model"); 
        int colorLocation = glGetUniformLocation(shaderProgram, "u_ObjectColor"); 

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(colorLocation, 1, glm::value_ptr(color));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    ~Renderer() {
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
};


class Joint{
    public:
    vector<Joint*> children;
    Renderer* renderer;
    Joint* parent;

    // Esta matriz afecta respecto al padre (T*R*S)
    // Se usará SOLO para renderizar este joint
    glm::mat4 localTransform; 

    // Esta matriz se pasará a los hijos (T*R)
    // NO incluye la escala de este joint
    glm::mat4 hierarchyTransform; 

    string name;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;

    // El constructor no cambia
    Joint(string n, Renderer* bp, glm::vec3 pos, glm::vec3 c, glm::vec3 s = glm::vec3(1.0f))
    : name(n), renderer(bp), parent(nullptr), position(pos), rotation(glm::vec3(0.0f)), color(c), scale(s){
        localTransform = glm::mat4(1.0f);
        hierarchyTransform = glm::mat4(1.0f); // Inicializar la nueva matriz
        updateLocalTransform();
    }

    void addChild(Joint* child){
        child->parent = this;   
        children.push_back(child);
    }

    ~Joint(){
        for(Joint* child :  children){
            delete child;
        }
    }

    void updateLocalTransform() {
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch (X)
        rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw (Y)
        rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll (Z)

        hierarchyTransform = glm::translate(glm::mat4(1.0f), position) * rotationMatrix;

        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        localTransform = hierarchyTransform * scaleMatrix; // T * R * S
    }
};

// Cambiamos el nombre del parámetro para mayor claridad
void drawJoint(Joint* joint, const glm::mat4& parentHierarchyTransform, const glm::mat4& view, const glm::mat4& proj, unsigned int shaderProgram){
    if(!joint) return;
    
    // Calcula T*R (para hijos) y T*R*S (para render) locales
    joint -> updateLocalTransform();
    
    // 1. Transformación GLOBAL para los hijos (T*R acumulado)
    // (Parent_T*R) * (Joint_T*R)
    glm::mat4 globalHierarchyTransform = parentHierarchyTransform * joint->hierarchyTransform;

    // 2. Transformación GLOBAL para renderizar (T*R*S acumulado)
    // (Parent_T*R) * (Joint_T*R*S)
    glm::mat4 globalRenderTransform = parentHierarchyTransform * joint->localTransform;


    if(joint -> renderer){
        // Usamos la globalRenderTransform (con escala) para dibujar
        glm::mat4 mvp = proj * view * globalRenderTransform;
        joint->renderer->draw(shaderProgram, mvp, globalRenderTransform, joint->color);
    }

    for(Joint* child : joint->children){
        // Pasamos la globalHierarchyTransform (sin escala) a los hijos
        drawJoint(child, globalHierarchyTransform, view, proj, shaderProgram);
    }
}

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

void processInput(GLFWwindow* window, float rotationSpeed, float deltaTime){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    // Movimiento camara con WASD
    const float cameraSpeed = 0.01f;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraPos += cameraSpeed * cameraFront;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraPos -= cameraSpeed * cameraFront;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp))* cameraSpeed;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp))* cameraSpeed;
    }

    // Movimiento articulaciones con flechitas
    if(selectedJoint){
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            selectedJoint->rotation.x +=rotationSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            selectedJoint->rotation.x -=rotationSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
            selectedJoint->rotation.y +=rotationSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
            selectedJoint->rotation.y -=rotationSpeed;
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        if(key >= GLFW_KEY_1 && key <=GLFW_KEY_9){
            int index = key - GLFW_KEY_1;
            if(index < jointList.size()){
                selectedJoint = jointList[index];
                cout << "Articulacion actual: " << selectedJoint->name << '\n';
            }
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos){

    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    yoffset *= MOUSE_SENSITIVITY;
    xoffset *= MOUSE_SENSITIVITY;

    yaw+= xoffset;
    pitch += yoffset;

    if(pitch > 89.0f){
        pitch = 89.0f;
    }
    if(pitch < -89.0f){
        pitch = -89.0f;
    }
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    fov -= (float)yoffset;
    if(fov < 1.0f){
        fov  = 1.0f;
    }
    if(fov > 90.0f){
        fov = 90.0f;
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "The Puppet", NULL, NULL);
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

    Sphere base(0.5f, 36, 18);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    Renderer* renderer  = new Renderer(base.vertices, base.indices);
    // Creación de la marioneta

    Joint* torso = new Joint("Torso", renderer, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.5f, 0.9f), glm::vec3(1.0f, 1.5f, 0.5f));
    
    Joint* cuello = new Joint("Cuello", nullptr, glm::vec3(0.0f, 0.85f, 0.0f), glm::vec3(0.0f));
    torso->addChild(cuello);
    Joint* cabeza = new Joint("Cabeza", renderer, glm::vec3(0.0f, 0.3f, 0.0f), glm::vec3(0.9f, 0.7f, 0.6f), glm::vec3(0.8f, 0.8f, 0.8f));
    cuello->addChild(cabeza);

    Joint* hombroIzq = new Joint("Hombro Izquierdo", nullptr, glm::vec3(-0.6f, 0.6f, 0.0f), glm::vec3(0.0f));
    hombroIzq->rotation.z = glm::radians(90.0f); // Posición T
    torso->addChild(hombroIzq);
    Joint* bicepIzq = new Joint("Bicep Izquierdo", renderer, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.9f, 0.7f, 0.6f), glm::vec3(0.3f, 0.8f, 0.3f));
    hombroIzq->addChild(bicepIzq);

    Joint* hombroDer = new Joint("Hombro Derecho", nullptr, glm::vec3(0.6f, 0.6f, 0.0f), glm::vec3(0.0f));
    hombroDer->rotation.z = glm::radians(-90.0f); // Posición T
    torso->addChild(hombroDer);
    Joint* bicepDer = new Joint("Bicep Derecho", renderer, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.9f, 0.7f, 0.6f), glm::vec3(0.3f, 0.8f, 0.3f));
    hombroDer->addChild(bicepDer);

    // Arreglo para seleccion de articulacion
    jointList.pb(torso);      // 1
    jointList.pb(cabeza);     // 2
    jointList.pb(hombroIzq);  // 3
    jointList.pb(hombroDer);  // 4

    selectedJoint = jointList[0];

    // Carga de shaders
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
    
    glm::vec3 lightPos(0.0f, 1.0f, 3.0f); 
    
    int lightPosLocation = glGetUniformLocation(shader, "u_LightPos");
    int viewPosLocation = glGetUniformLocation(shader, "u_ViewPos");

    glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
    
    float rotationSpeed = glm::radians(0.3f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;

        processInput(window, rotationSpeed, deltaTime);
        
        glm::mat4 Proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 View = glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);
        
        glUseProgram(shader);
        
        glUniform3fv(viewPosLocation, 1, glm::value_ptr(cameraPos));
        
        
        glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        glm::mat4 globalModelMatrix = glm::mat4(1.0f); 

        // Iniciar el dibujado recursivo
        drawJoint(torso, globalModelMatrix, View, Proj, shader);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Limpieza
    delete torso;
    delete renderer;

    glfwTerminate();
    return 0;
}