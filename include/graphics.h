#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


enum GridHalf {
    HALF_NONE,     // full grid
    HALF_POSITIVE, // only Y>=0 or X>=0 etc.
    HALF_NEGATIVE  // only Y<=0 or X<=0 etc.
};

// grid plane selector
enum GridPlane {
    GRID_XY,
    GRID_YZ,
    GRID_XZ
};

// -----------------------------
// Object creation
// -----------------------------
GLuint createPlaneObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window);
GLuint createPlaneEdgeObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window);
GLuint createGrid(int N, GridPlane plane, GridHalf half, int* outVertexCount);
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
void renderFrame(GLFWwindow* window, GLuint planeVAO, GLuint edgeVAO, GLuint gridVAO, GLuint gridYZVAO, const glm::vec3& pos);

#endif
