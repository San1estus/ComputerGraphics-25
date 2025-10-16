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

class Renderer{
    private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    public:
    Renderer(const vector<float>& vertices, const vector<unsigned int>& indices):indexCount(indices.size()){
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

    // Esta matriz afecta respecto al padre
    glm::mat4 localTransform; 
    string name;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 color;

    Joint(string n, Renderer* bp, glm::vec3 pos, glm::vec3 c) : name(n), renderer(bp), parent(nullptr), position(pos), rotation(glm::vec3(0.0f)), color(c){
        localTransform = glm::mat4(1.0f);
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

        // Aplicar la Traslación * Rotación (TR)

        localTransform = glm::translate(glm::mat4(1.0f), position) * rotationMatrix;
    }
};

void drawJoint(Joint* joint, const glm::mat4& parentTransform, const glm::mat4& view, const glm::mat4& proj, unsigned int shaderProgram){
    if(!joint) return;
    
    joint -> updateLocalTransform();
    
    glm::mat4 globalTransform =  parentTransform * joint->localTransform;

    if(joint -> renderer){
        glm::mat4 mvp = proj * view * globalTransform;
        joint->renderer->draw(shaderProgram, mvp, globalTransform, joint->color);
    }

    for(Joint* child : joint->children){
        drawJoint(child, globalTransform, view, proj, shaderProgram);
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

    Sphere torsoObj(0.6f, 36, 18);
    Sphere headObj(0.3f, 18, 9);

    Renderer* torsoRenderer  = new Renderer(torsoObj.vertices, torsoObj.indices);
    Renderer* headRenderer  = new Renderer(headObj.vertices, headObj.indices);

    Joint* torso = new Joint("Torso", torsoRenderer, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.5f, 0.9f));

    Joint* cuello = new Joint("Cuello", nullptr, glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.0f));
    torso->addChild(cuello);
    Joint* cabeza = new Joint("Cabeza", headRenderer, glm::vec3(0.0f, 0.3f, 0.0f), glm::vec3(0.9f, 0.2f, 0.2f));
    cuello->addChild(cabeza);

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
    glm::mat4 Proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 View = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // Cámara
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f); 
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);  
    
    int lightPosLocation = glGetUniformLocation(shader, "u_LightPos");
    int viewPosLocation = glGetUniformLocation(shader, "u_ViewPos");

    glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
    glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPos));

    float time = 0.0f;
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        // Calcular delta time para rotación suave
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;
        time+= deltaTime;

        // Rotación continua para ver que si es esfera

        cuello -> rotation.y = glm::radians(sin(time*1.5f)*45.0f);
        cabeza -> rotation.x = glm::radians(cos(time*2.0f)*35.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar color y profundidad
        
        glUseProgram(shader);
        
        glm::mat4 globalModelMatrix = glm::mat4(1.0f); 

        // Iniciar el dibujado recursivo
        drawJoint(torso, globalModelMatrix, View, Proj, shader);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Limpieza
    delete torso;
    delete torsoRenderer;
    delete headRenderer;

    glfwTerminate();
    return 0;
}