﻿// main.cpp
//Credits Aston Martin Vantage.obj https://www.cgtrader.com/items/5618629/download-page

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

GLuint shaderProgram;
GLuint lightShaderProgram;

//Function that compiles all program shaders
void CompileShaders() {
    //Vertex shader (used for both objects)
    std::fstream vertSrc("Shaders/sample.vert");
    std::stringstream vertBuff;
    vertBuff << vertSrc.rdbuf();
    std::string vertS = vertBuff.str();
    const char* vert = vertS.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vert, nullptr);
    glCompileShader(vertexShader);

    //Fragment shader for the main object (textured)
    std::fstream fragSrc("Shaders/sample.frag");
    std::stringstream fragBuff;
    fragBuff << fragSrc.rdbuf();
    std::string fragS = fragBuff.str();
    const char* frag = fragS.c_str();

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frag, nullptr);
    glCompileShader(fragmentShader);

    //Fragment shader for the light model (unlit)
    std::fstream lightFragSrc("Shaders/light.frag");
    std::stringstream lightFragBuff;
    lightFragBuff << lightFragSrc.rdbuf();
    std::string lightFragS = lightFragBuff.str();
    const char* lightFrag = lightFragS.c_str();

    GLuint lightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(lightFragmentShader, 1, &lightFrag, nullptr);
    glCompileShader(lightFragmentShader);

    //Vertex shader program for the main model (uses texture)
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //Fragment shader program for the light model (uses solid color)
    lightShaderProgram = glCreateProgram();
    glAttachShader(lightShaderProgram, vertexShader);
    glAttachShader(lightShaderProgram, lightFragmentShader);
    glLinkProgram(lightShaderProgram);

    //To cleanup shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(lightFragmentShader);
}

//Function that compiles all Light shaders
void CompileLightShader() {
    std::fstream vertSrc("Shaders/sample.vert");
    std::stringstream vertBuff;
    vertBuff << vertSrc.rdbuf();
    std::string vertS = vertBuff.str();
    const char* vert = vertS.c_str();

    std::fstream fragSrc("Shaders/light.frag");
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

    lightShaderProgram = glCreateProgram();
    glAttachShader(lightShaderProgram, vertexShader);
    glAttachShader(lightShaderProgram, fragmentShader);
    glLinkProgram(lightShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
}

//Camera class
class Camera {
public:
    virtual glm::mat4 GetViewMatrix() = 0;
    virtual glm::mat4 GetProjectionMatrix(float aspectRatio) = 0;
    virtual ~Camera() {}
};

//Perspective camera class
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

//Orthographic camera class
class OrthographicCamera : public Camera {
public:
    float orthoSize;
    float nearPlane, farPlane;

    OrthographicCamera(float size, float nearP, float farP)
        : orthoSize(size), nearPlane(nearP), farPlane(farP) {}

    glm::mat4 GetViewMatrix() override {
        return glm::lookAt(glm::vec3(0.0f, 5.0f, 10.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 GetProjectionMatrix(float aspectRatio) override {
        return glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
    }
};

//Light class
class Light {
public:
    glm::vec3 color;
    float intensity;

    Light(const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f)
        : color(color), intensity(intensity) {}

    virtual ~Light() {}

    virtual glm::vec3 GetPosition() const = 0;
};

//Point light class
class PointLight : public Light {
public:
    glm::vec3 position;

    PointLight(const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f)
        : Light(color, intensity), position(pos) {}

    glm::vec3 GetPosition() const override {
        return position;
    }
};

//Directional light class
class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(const glm::vec3& dir, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f)
        : Light(color, intensity), direction(glm::normalize(dir)) {}

    glm::vec3 GetPosition() const override {
        return -direction; // Directional light has no position, returning negative direction as a placeholder
    }
};

//Camera Positioning
PerspectiveCamera perspectiveCamera(glm::vec3(0.0f, 0.0f, 10.0f), 45.0f, 0.1f, 100.0f);
OrthographicCamera orthographicCamera(5.0f, 0.1f, 100.0f);
Camera* activeCamera = &perspectiveCamera;

//Light Positioning
PointLight pointLight(glm::vec3(4.0f, 2.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
DirectionalLight dirLight(glm::vec3(4.0f, -5.0f, 0.0f), glm::vec3(1.0f), 1.0f);

float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;
float lightRotationX = 0.0f;
float lightRotationY = 0.0f;
float lightRotationZ = 0.0f;


bool controlLight = false;


bool firstMouse = true;
float lastX = 400, lastY = 300;

//For keyboard inputs
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    //To switch to perspective camera
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        activeCamera = &perspectiveCamera;
    }
    //To switch to orthographic camera
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        activeCamera = &orthographicCamera;
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        controlLight = !controlLight;
    }
    //To increase directional light intensity
    if (key == GLFW_KEY_RIGHT == GLFW_PRESS) {
        dirLight.intensity += 0.1f;
    }
    if (key == GLFW_KEY_RIGHT == GLFW_PRESS) {
        dirLight.intensity = std::max(0.0f, dirLight.intensity - 0.1f);
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        //Keys to rotate the model
        if (!controlLight) {
            if (key == GLFW_KEY_A) rotationY -= 5.0f;
            if (key == GLFW_KEY_D) rotationY += 5.0f;
            if (key == GLFW_KEY_W) rotationX -= 5.0f;
            if (key == GLFW_KEY_S) rotationX += 5.0f;
            if (key == GLFW_KEY_Q) rotationZ -= 5.0f;
            if (key == GLFW_KEY_E) rotationZ += 5.0f;
        }
        //Keys to rotate the light model around the main object
        else {
            if (key == GLFW_KEY_A) lightRotationY -= 5.0f;
            if (key == GLFW_KEY_D) lightRotationY += 5.0f;
            if (key == GLFW_KEY_W) lightRotationX -= 5.0f;
            if (key == GLFW_KEY_S) lightRotationX += 5.0f;
            if (key == GLFW_KEY_Q) lightRotationZ -= 5.0f;
            if (key == GLFW_KEY_E) lightRotationZ += 5.0f;

            //To increase point light intensity
            if (key == GLFW_KEY_UP) {
                pointLight.intensity += 0.1f;
            }
            //To decrease point light intensity
            if (key == GLFW_KEY_DOWN) {
                pointLight.intensity -= 0.1f;
                if (pointLight.intensity < 0.0f)
                    pointLight.intensity = 0.0f;
            }

        }

    }
}

//For mouse inputs
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

    CompileShaders();
    CompileLightShader();

    Model model("3D/Car1.obj", "3D/tuxedosam.png");
    Model lightModel("3D/heart.obj");

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);


        glUseProgram(shaderProgram);
        GLint lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");

        dirLight.intensity = 10.0f;
        std::cout << "Directional Light Intensity: " << dirLight.intensity << std::endl;
        glUniform1f(glGetUniformLocation(shaderProgram, "dirLight.intensity"), dirLight.intensity);

        std::cout << "Directional Light - Direction: " << dirLight.direction.x << ", "
            << dirLight.direction.y << ", " << dirLight.direction.z << std::endl;
        std::cout << "Directional Light - Intensity: " << dirLight.intensity << std::endl;

        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.color"), dirLight.color.x, dirLight.color.y, dirLight.color.z);
        glUniform1f(glGetUniformLocation(shaderProgram, "dirLight.intensity"), dirLight.intensity);

        float aspectRatio = 640.0f / 480.0f;
        glm::mat4 view = activeCamera->GetViewMatrix();
        glm::mat4 projection = activeCamera->GetProjectionMatrix(aspectRatio);

        //Model Transformations
        glm::mat4 modelTransform = glm::mat4(1.0f);
        modelTransform = glm::rotate(modelTransform, glm::radians(rotationX), glm::vec3(1, 0, 0));
        modelTransform = glm::rotate(modelTransform, glm::radians(rotationY), glm::vec3(0, 1, 0));
        modelTransform = glm::rotate(modelTransform, glm::radians(rotationZ), glm::vec3(0, 0, 1));
        glm::mat4 modelMVP = projection * view * modelTransform;
        model.Draw(shaderProgram, modelMVP);

        //Set Lighting Uniforms for Main Shader
        glm::vec3 displayColor = controlLight ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f);
        displayColor *= pointLight.intensity;

        glUniform3f(lightDirLoc, dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);
        glUniform3f(lightColorLoc, displayColor.r, displayColor.g, displayColor.b);

        //Render the Light Object (Heart Model)
        glUseProgram(lightShaderProgram); // Switch to light shader

        GLint heartColorLoc = glGetUniformLocation(lightShaderProgram, "lightColor");
        glUniform3f(heartColorLoc, displayColor.r, displayColor.g, displayColor.b);

        //Light Model Transformations (Heart Follows Point Light)
        glm::mat4 lightTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        lightTransform = glm::rotate(lightTransform, glm::radians(lightRotationY), glm::vec3(0, 1, 0));
        lightTransform = glm::rotate(lightTransform, glm::radians(lightRotationX), glm::vec3(1, 0, 0));
        lightTransform = glm::rotate(lightTransform, glm::radians(lightRotationZ), glm::vec3(0, 0, 1));
        lightTransform = glm::translate(lightTransform, glm::vec3(2.5f, 1.5f, -1.5f));
        lightTransform = glm::scale(lightTransform, glm::vec3(0.1f));
        glm::mat4 lightMVP = projection * view * lightTransform;
        lightModel.Draw(lightShaderProgram, lightMVP);



        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return -1;
}