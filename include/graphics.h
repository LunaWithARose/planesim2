#pragma once
#ifndef MY_CLASS_H
#define MY_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <chrono>

GLuint createPlaneObject(glm::vec3 position, glm::vec3 orientation, GLFWwindow* window);
GLFWwindow* initializeWindow();

static GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const char* vsSrc, const char* fsSrc);


#endif
