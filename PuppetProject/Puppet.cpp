#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <cmath> 
#include <vector>
#include <stack>
#include <map>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define pb(a) push_back(a)

const float PI = acos(-1);

using namespace std;

/*
TODO: Automatizar la colocacion de pivotes y extremidades
*/

enum class JointType{
    TORSO,
    CUELLO,
    HOMBRO, 
    CODO, 
    MUNECA,
    CADERA,
    RODILLA,
    PIE,
    TOBILLO,
    NA = -1
};

class Joint;

// Iniciar con pantalla centrada
float lastX = 960;
float lastY = 540;
float yaw = -90.0, roll, pitch, fov = 45.0;


Joint* selectedJoint = nullptr;
vector<Joint*> jointList;

stack<pair<glm::vec3, int>> undoStack;
stack<pair<glm::vec3, int>> redoStack;

// Posicion inicial de la camara
glm::vec3 cameraPos = glm::vec3(0.0f, -0.7f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Se puede ajustar para que la camara rote mas rapido
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
                    // Triángulo 1
                    indices.pb(p1);
                    indices.pb(p2);
                    indices.pb(p4); 

                    // Triángulo 1
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

class Joint{
    private:
    glm::vec3 initialPosition;
    glm::vec3 initialRotation;

    public:
    Joint* parent;
    vector<Joint*> children;
    Renderer* renderer;
    JointType type;

    // Esta matriz afecta respecto al padre (T*R*S)
    // Se usará SOLO para renderizar este joint
    glm::mat4 localTransform; 

    // Esta matriz se pasará a los hijos (T*R)
    // NO incluye la escala de este joint
    glm::mat4 hierarchyTransform ; 

    string name;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;


    glm::vec3 minRotation;
    glm::vec3 maxRotation;
    
    Joint(string n, Joint* parent, JointType t = JointType::NA, Renderer* bp = nullptr, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 c = glm::vec3(0.0f), glm::vec3 s = glm::vec3(1.0f))
    : name(n), renderer(bp), parent(nullptr), position(pos), rotation(glm::vec3(0.0f)), color(c), scale(s), type(t){
        localTransform = glm::mat4(1.0f);
        hierarchyTransform = glm::mat4(1.0f); 

        initialPosition = pos;
        initialRotation = glm::vec3(0.0f);
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

        rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll 
        rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch 
        rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw 

        hierarchyTransform = glm::translate(glm::mat4(1.0f), position) * rotationMatrix;

        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        localTransform = hierarchyTransform * scaleMatrix;
    }

    void setRestrictions(){
        switch (type)
        {
        case JointType::TORSO:
            minRotation = glm::vec3(glm::radians(0.0f), glm::radians(-180.0f), glm::radians(-30.0f));
            maxRotation = glm::vec3(glm::radians(360.0f), glm::radians(180.0f), glm::radians(30.0f));
            
            break;
        case JointType::CUELLO:
            minRotation = glm::vec3(glm::radians(-45.0f), glm::radians(-70.0f), glm::radians(-30.0f));
            maxRotation = glm::vec3(glm::radians(45.0f), glm::radians(70.0f), glm::radians(30.0f));
            break;

        case JointType::HOMBRO:
            minRotation = glm::vec3(glm::radians(-90.0f), glm::radians(-45.0f), glm::radians(-60.0f));
            maxRotation = glm::vec3(glm::radians(180.0f), glm::radians(90.0f), glm::radians(60.0f));
            if (name.find("Derecho") != string::npos) {
            minRotation = glm::vec3(glm::radians(-90.0f), glm::radians(-90.0f), glm::radians(-60.0f));
            maxRotation = glm::vec3(glm::radians(180.0f), glm::radians(45.0f), glm::radians(60.0f));
            }
            break;

        case JointType::CODO:
            minRotation = glm::vec3(0.0f, 0.0f, 0.0f);
            maxRotation = glm::vec3(0.0f, glm::radians(150.0f), 0.0f);

            if (name.find("Derecho") != string::npos) {
                minRotation = glm::vec3(0.0f, glm::radians(-150.0f), 0.0f);
                maxRotation = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            break;
        case JointType::MUNECA:
            minRotation = glm::vec3(glm::radians(-70.0f), 0.0f, glm::radians(-30.0f));
            maxRotation = glm::vec3(glm::radians(70.0f), 0.0f, glm::radians(30.0f)); 
            break;
        case JointType::CADERA:
            minRotation = glm::vec3(glm::radians(-30.0f), glm::radians(-45.0f), glm::radians(-45.0f));
            maxRotation = glm::vec3(glm::radians(120.0f), glm::radians(45.0f), glm::radians(45.0f)); 
            break;
        case JointType::RODILLA:
            minRotation = glm::vec3(0.0f, 0.0f, 0.0f);
            maxRotation = glm::vec3(glm::radians(140.0f), 0.0f, 0.0f); 
            break;
        case JointType::TOBILLO:
            minRotation = glm::vec3(glm::radians(-45.0f), glm::radians(-10.0f), glm::radians(-30.0f));
            maxRotation = glm::vec3(glm::radians(45.0f), glm::radians(10.0f), glm::radians(30.0f));
            break;
        
        default:
            minRotation = glm::vec3(0.0f);
            maxRotation = glm::vec3(0.0f);
            break;
        }
    }
    
    void applyJointRestrictions(){
        switch(type){
            
            case JointType::NA:
                rotation = glm::vec3(0.0f);
                break;
                
            default:
                rotation = glm::clamp(rotation, minRotation, maxRotation);
            break;
        }
    }
    void reset(){
        rotation = initialRotation;
    }
};

void drawJoint(Joint* joint, const glm::mat4& parentHierarchyTransform, const glm::mat4& view, const glm::mat4& proj, unsigned int shaderProgram){
    if(!joint) return;
    
    // Calcula las posiciones y rotacion actuales
    joint ->applyJointRestrictions();
    joint -> updateLocalTransform();
    
    // Transformación para los hijos
    glm::mat4 globalHierarchyTransform = parentHierarchyTransform * joint->hierarchyTransform;

    // Transformación para renderizar
    glm::mat4 globalRenderTransform = parentHierarchyTransform * joint->localTransform;


    if(joint -> renderer){
        // Usamos la transformacion con escala para dibujar
        glm::mat4 mvp = proj * view * globalRenderTransform;
        joint->renderer->draw(shaderProgram, mvp, globalRenderTransform, joint->color);
    }

    for(Joint* child : joint->children){
        // Pasamos la transformacion sin escala a los hijos
        drawJoint(child, globalHierarchyTransform, view, proj, shaderProgram);
    }
}

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

static unsigned int CreateShader(const string& vertexShader, string& fragmentShader) {
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

    // Movimiento articulaciones con flechitas izquierda y derecha para eje X\
    arriba abajo para eje y Z y X para mover eje Z
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
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
            selectedJoint->rotation.z +=rotationSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
            selectedJoint->rotation.z -=rotationSpeed;
        }
    }
}

JointType stringToJointType(const string& s) {
    if (s == "TORSO") return JointType::TORSO;
    if (s == "CUELLO") return JointType::CUELLO;
    if (s == "HOMBRO") return JointType::HOMBRO;
    if (s == "CODO") return JointType::CODO;
    if (s == "MUNECA") return JointType::MUNECA;
    if (s == "CADERA") return JointType::CADERA;
    if (s == "RODILLA") return JointType::RODILLA;
    if (s == "TOBILLO") return JointType::TOBILLO;
    return JointType::NA;
}

vector<Joint*> loadJointsFromFile(const string& filename, Renderer* render) {
    ifstream file(filename);
    string line;
    map<string, Joint*> jointMap;
    vector<Joint*> joints;

    if (!file.is_open()) {
        cerr << "Error: no se pudo abrir " << filename << endl;
        return joints;
    }

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string name, parentName, typeStr;
        float px, py, pz, cr, cg, cb, sx, sy, sz;
        ss >> name >> parentName >> typeStr >> px >> py >> pz >> cr>> cg >> cb >> sx >> sy >> sz;

        JointType type = stringToJointType(typeStr);
        Joint* parent = nullptr;
        if (parentName != "NONE" && jointMap.count(parentName))
            parent = jointMap[parentName];


        Joint* joint = new Joint(name, parent, type, ((type != JointType::NA && parentName != "NONE")  ? nullptr : render), glm::vec3(px, py, pz), glm::vec3(cr, cg, cb), glm::vec3(sx, sy, sz));
        joint->setRestrictions();

        if (parent) parent->children.push_back(joint);

        if(type != JointType::NA || parentName == "NONE"){
            joints.push_back(joint);
        } 

        jointMap[name] = joint;
    }

    file.close();
    return joints;
}

bool derecho = false;

int lastIndex = 0;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        // Seleccionar articulacion
        if(key >= GLFW_KEY_0 && key <=GLFW_KEY_7){
            int index = key - GLFW_KEY_0;
            if(selectedJoint->name == jointList[index]->name){
                derecho = !derecho;
            }
            else{
                derecho = false;
            }

            if(index > 1){
                index += 6*derecho;
            }
            
            if(index < jointList.size()){ 
                selectedJoint = jointList[index];
                lastIndex = index;
                cout << "Articulacion actual: " << selectedJoint->name << '\n';
            }
        }
        
        // Reiniciar la T-pose
        if(key == GLFW_KEY_T){
            for(Joint* j : jointList){
                j->reset();
            }
            while(!redoStack.empty()) redoStack.pop();
            while(!undoStack.empty()) undoStack.pop();
        }
    }
    if(action == GLFW_PRESS){
        if(key >= GLFW_KEY_RIGHT && key <= GLFW_KEY_UP || key == GLFW_KEY_C || key == GLFW_KEY_X ){
            undoStack.push({selectedJoint->rotation, lastIndex});
            while(!redoStack.empty()) redoStack.pop();
        }
    }
    

    if(mods == GLFW_MOD_CONTROL && action == GLFW_PRESS){
        switch (key){
            case GLFW_KEY_Z:
                if(!undoStack.empty()){
                    int idx = undoStack.top().second;
                    
                    redoStack.push({jointList[idx]->rotation, idx});
                    jointList[idx]->rotation = undoStack.top().first;
                    undoStack.pop();

                    selectedJoint = jointList[idx];
                    lastIndex = idx;
                }
                break;
            case GLFW_KEY_Y:
                if(!redoStack.empty()){
                    int idx = redoStack.top().second;

                    undoStack.push({jointList[idx]->rotation, idx});
                    jointList[idx]->rotation = redoStack.top().first;
                    redoStack.pop();

                    selectedJoint = jointList[idx];
                    lastIndex = idx;
                }
                break;
            default:
                break;
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

    // Arreglo para seleccion de articulacion
    jointList = loadJointsFromFile("joints.txt", renderer);

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


    // Configuración Inicial de Matrices y Luz
    
    glm::vec3 lightPos(0.0f, 1.0f, 3.0f); 
    
    int lightPosLocation = glGetUniformLocation(shader, "u_LightPos");
    int viewPosLocation = glGetUniformLocation(shader, "u_ViewPos");

    glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
    
    // A gusto personal, esta velocidad de rotacion se me hizo bien
    float rotationSpeed = glm::radians(0.3f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
    cout << "Control de camara:\n W -- Adelante\n A -- Izquierda\n S -- Abajo\n D -- Derecha\n";
    cout <<"\nMover el mouse modifica la rotacion de la camara\nCon la rueda se hace zoom\n";
    cout <<"\nControl de marioneta\n";
    cout << "Flecha izquierda y derecha para mover respecto al eje X\n";
    cout << "Flecha arriba y abajo para mover respecto al eje Y\n";
    cout << "X/C para mover respecto al eje Z\n";
    cout << "\nSeleccion de articulaciones\n";
    cout << "0 -- Torso\n";
    cout << "1 -- Cuello\n";
    cout << "2 -- Hombro Izquierdo/Derecho\n";
    cout << "3 -- Codo Izquierdo/Derecho\n";
    cout << "4 -- Muneca Izquierda/Derecha\n";
    cout << "5 -- Cadera Izquierda/Derecha\n";
    cout << "6 -- Rodilla Izquierda/Derecha\n";
    cout << "7 -- Tobillo Izquierdo/Derecho\n";
    float lastFrame = 0.0f;

    Joint* raiz = jointList[0];
    
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

        // El dibujado es recursivo
        drawJoint(raiz, globalModelMatrix, View, Proj, shader);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Limpieza
    delete renderer;

    glfwTerminate();
    return 0;
}