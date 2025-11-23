#pragma once
#ifndef MY_CLASS_H
#define MY_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// object creation
GLuint createPlaneObject(glm::vec3 position, glm::vec3 orientation, GLFWwindow* window);
GLuint createGridLines();
GLuint createYZGridLines();
GLFWwindow* initializeWindow();

// shader helpers
GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const char* vsSrc, const char* fsSrc);

// mouse-look globals
extern float yaw, pitch, zoom;
extern double lastX, lastY;
extern bool firstMouse;

void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
// ⬇⬇⬇ NEW: render loop now provided by graphics.cpp
void render(GLFWwindow* window, GLuint planeVAO, GLuint gridVAO, GLuint gridYZVAO, glm::vec3 pos);

#endif

