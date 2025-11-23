#include "graphics.h"
#include <glm/glm.hpp>
#include <iostream>

int main()
{
    GLFWwindow* window = initializeWindow();
    if (!window) {
        std::cerr << "Failed to initialize window.\n";
        return -1;
    }

    const int gridSize = 20;

    GLuint planeVAO  = createPlaneObject({0,0,0}, {0,0,0}, window);
    GLuint edgeVAO = createPlaneEdgeObject({0,0,0}, {0,0,0}, window);
    GLuint gridVAO   = createGridLines(gridSize);
    GLuint gridYZVAO = createYZGridLines(gridSize);  // <-- new

    render(window, planeVAO, edgeVAO, gridVAO, gridYZVAO, {0,0,0});

    glfwTerminate();
    return 0;
}


