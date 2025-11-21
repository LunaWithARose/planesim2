// src/main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <chrono>
#include "graphics.h"

using namespace std;
using namespace glm;

int main() {

    vec3 position = {0, 0, 0};
    vec3 orientation = {0, 0, 0};

    GLFWwindow* window = initializeWindow();
    GLuint program = createPlaneObject(position, orientation, window);
    return 0;
}
