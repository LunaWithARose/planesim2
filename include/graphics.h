#pragma once
#ifndef MY_CLASS_H
#define MY_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// -----------------------------
// Object creation
// -----------------------------
GLuint createPlaneObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window);
GLuint createPlaneEdgeObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window);
GLuint createGridLines(int N);      // XY grid, pass grid size
GLuint createYZGridLines(int N);    // YZ grid, pass grid size
GLFWwindow* initializeWindow();

// -----------------------------
// Shader helpers
// -----------------------------
GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const char* vsSrc, const char* fsSrc);

// -----------------------------
// Camera / input globals
// -----------------------------
extern float yaw, pitch, zoom;
extern double lastX, lastY;
extern bool firstMouse, rotating;
extern int width, height;
extern int gridXYVertexCount, gridYZVertexCount;

// -----------------------------
// Input callbacks
// -----------------------------
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int w, int h);

// -----------------------------
// Render loop
// -----------------------------
void render(GLFWwindow* window, GLuint planeVAO, GLuint edgeVAO, GLuint gridVAO, GLuint gridYZVAO, glm::vec3 pos);

#endif
