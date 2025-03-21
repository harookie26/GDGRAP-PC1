// main.cpp
//Credits Pengu.obj made by Jacob Berger https://www.fab.com/listings/8d95b651-c9aa-4965-9c3c-0102407de0ff

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Model.h"

GLuint shaderProgram; // Global shader program variable


void CompileShaders() {
    std::fstream vertSrc("Shaders/sample.vert");
    std::stringstream vertBuff;
    vertBuff << vertSrc.rdbuf();
    std::string vertS = vertBuff.str();
    const char* vert = vertS.c_str();

    std::fstream fragSrc("Shaders/sample.frag");
    std::stringstream fragBuff;
    fragBuff << fragSrc.rdbuf();
    std::string fragS = fragBuff.str();
    const char* frag = fragS.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vert, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag, nullptr);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

class Camera {
public:
    virtual glm::mat4 GetViewMatrix() = 0;
    virtual glm::mat4 GetProjectionMatrix(float aspectRatio) = 0;
    virtual ~Camera() {}
};

class PerspectiveCamera : public Camera {
public:
    glm::vec3 position;
    float yaw, pitch;
    float fov;
    float nearPlane, farPlane;
    float radius;

    PerspectiveCamera(glm::vec3 pos, float fov, float nearP, float farP)
        : position(pos), fov(fov), nearPlane(nearP), farPlane(farP), yaw(-90.0f), pitch(0.0f), radius(10.0f) {}

    glm::mat4 GetViewMatrix() override {
        glm::vec3 cameraPos;
        cameraPos.x = radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraPos.y = radius * sin(glm::radians(pitch));
        cameraPos.z = radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 GetProjectionMatrix(float aspectRatio) override {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
};

class OrthographicCamera : public Camera {
public:
    float orthoSize;
    float nearPlane, farPlane;

    OrthographicCamera(float size, float nearP, float farP)
        : orthoSize(size), nearPlane(nearP), farPlane(farP) {}

    glm::mat4 GetViewMatrix() override {
    return glm::lookAt(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
}

    glm::mat4 GetProjectionMatrix(float aspectRatio) override {
        return glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
    }
};

PerspectiveCamera perspectiveCamera(glm::vec3(0.0f, 0.0f, 10.0f), 45.0f, 0.1f, 100.0f);
OrthographicCamera orthographicCamera(5.0f, 0.1f, 100.0f);
Camera* activeCamera = &perspectiveCamera;

float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;
float lightRotationX = 0.0f;
float lightRotationY = 0.0f;
float lightRotationZ = 0.0f;


bool controlLight = false;  // If false, control model; if true, control light


bool firstMouse = true;
float lastX = 400, lastY = 300;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        activeCamera = &perspectiveCamera;
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        activeCamera = &orthographicCamera;
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        controlLight = !controlLight;  // Toggle control between model and light
        std::cout << (controlLight ? "Controlling Light\n" : "Controlling Model\n");
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (!controlLight) {
            // Rotate the Model
            if (key == GLFW_KEY_A) rotationY -= 5.0f;
            if (key == GLFW_KEY_D) rotationY += 5.0f;
            if (key == GLFW_KEY_W) rotationX -= 5.0f;
            if (key == GLFW_KEY_S) rotationX += 5.0f;
            if (key == GLFW_KEY_Q) rotationZ -= 5.0f;
            if (key == GLFW_KEY_E) rotationZ += 5.0f;
        }
        else {
            // Rotate the Light Model around the main object
            if (key == GLFW_KEY_A) lightRotationY -= 5.0f; 
            if (key == GLFW_KEY_D) lightRotationY += 5.0f;
            if (key == GLFW_KEY_W) lightRotationX -= 5.0f; 
            if (key == GLFW_KEY_S) lightRotationX += 5.0f; 
            if (key == GLFW_KEY_Q) lightRotationZ -= 5.0f; 
            if (key == GLFW_KEY_E) lightRotationZ += 5.0f; 
        }

    }
}


void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    perspectiveCamera.yaw += xoffset;
    perspectiveCamera.pitch += yoffset;

    if (perspectiveCamera.pitch > 89.0f)
        perspectiveCamera.pitch = 89.0f;
    if (perspectiveCamera.pitch < -89.0f)
        perspectiveCamera.pitch = -89.0f;
}

struct DirectionalLight {
    glm::vec3 position;
    glm::vec3 direction;
};

DirectionalLight dirLight = { glm::vec3(4.0f, -5.0f, 0.0f), glm::normalize(glm::vec3(-4.0f, 5.0f, 0.0f)) };


int main(void) {
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(1920, 1080, "PC 2", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    CompileShaders(); // Initialize shader program

    Model model("3D/Car1.obj");
    Model lightModel("3D/heart.obj");

   while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram); // Use shader program

    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "dirLight.position");
    GLuint lightDirLoc = glGetUniformLocation(shaderProgram, "dirLight.direction");

    glUniform3fv(lightPosLoc, 1, glm::value_ptr(dirLight.position));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(dirLight.direction));

    float aspectRatio = 640.0f / 480.0f;
    glm::mat4 view = activeCamera->GetViewMatrix();
    glm::mat4 projection = activeCamera->GetProjectionMatrix(aspectRatio);

    // **Model Transformations**
    glm::mat4 modelTransform = glm::mat4(1.0f);
    modelTransform = glm::rotate(modelTransform, glm::radians(rotationX), glm::vec3(1, 0, 0));
    modelTransform = glm::rotate(modelTransform, glm::radians(rotationY), glm::vec3(0, 1, 0));
    modelTransform = glm::rotate(modelTransform, glm::radians(rotationZ), glm::vec3(0, 0, 1));
    glm::mat4 modelMVP = projection * view * modelTransform;
    model.Draw(shaderProgram, modelMVP);  

    // **Light Model Transformations**
    glm::mat4 lightTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    lightTransform = glm::rotate(lightTransform, glm::radians(lightRotationY), glm::vec3(0, 1, 0));
    lightTransform = glm::rotate(lightTransform, glm::radians(lightRotationX), glm::vec3(1, 0, 0));
    lightTransform = glm::rotate(lightTransform, glm::radians(lightRotationZ), glm::vec3(0, 0, 1)); 
    lightTransform = glm::translate(lightTransform, glm::vec3(4.0f, 2.0f, -3.0f)); 
    lightTransform = glm::scale(lightTransform, glm::vec3(0.1f)); 
    glm::mat4 lightMVP = projection * view * lightTransform;
    lightModel.Draw(shaderProgram, lightMVP);  


    glfwSwapBuffers(window);
    glfwPollEvents();
}


    glfwTerminate();
    return -1;
}
