#pragma once
#ifndef MY_CLASS_H
#define MY_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// object creation
GLuint createPlaneObject(glm::vec3 position, glm::vec3 orientation, GLFWwindow* window);
GLuint createGridLines(GLFWwindow* window);
GLFWwindow* initializeWindow();

// shader helpers
GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const char* vsSrc, const char* fsSrc);

// mouse-look globals
extern float yaw, pitch;
extern double lastX, lastY;
extern bool firstMouse;

void mouseCallback(GLFWwindow* window, double xpos, double ypos);

// ⬇⬇⬇ NEW: render loop now provided by graphics.cpp
void render(GLFWwindow* window, GLuint planeVAO, GLuint gridVAO);

#endif

