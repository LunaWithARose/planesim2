#include "graphics.h"

#include <vector>
#include <iostream>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Vertex counts
int gridXYVertexCount = 0;
int gridYZVertexCount = 0;

// -----------------------------
// Camera / UI state
// ----------------------------
float yaw   = -90.0f;
float pitch = -30.0f;  // look slightly down so plane is visible
float zoom  = 10.0f;   // start further back
double lastX = 400.0, lastY = 300.0;
bool firstMouse = true;
bool rotating = false;

// viewport size
int width = 800;
int height = 600;

// -----------------------------
// Callbacks
// -----------------------------
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!rotating) { firstMouse = true; return; }
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos); // reversed
    lastX = xpos; lastY = ypos;

    const float sensitivity = 0.2f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) { rotating = true; firstMouse = true; }
        else if (action == GLFW_RELEASE) { rotating = false; }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)xoffset;
    zoom -= static_cast<float>(yoffset);
    zoom = glm::clamp(zoom, 1.0f, 50.0f);
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    (void)window;
    width = (w > 0) ? w : 1;
    height = (h > 0) ? h : 1;
    glViewport(0, 0, width, height);
}

// -----------------------------
// Window + GL setup
// -----------------------------
GLFWwindow* initializeWindow()
{
    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return nullptr; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(width, height, "Plane Simulator", nullptr, nullptr);
    if (!window) { std::cerr << "glfwCreateWindow failed\n"; glfwTerminate(); return nullptr; }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGLLoader failed\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); // ensures all cube faces render
    glViewport(0, 0, width, height);
    return window;
}