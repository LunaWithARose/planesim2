#include "graphics.h"
#include <glm/glm.hpp>

int main()
{
    GLFWwindow* window = initializeWindow();
    if (!window) return -1;

    GLuint planeVAO = createPlaneObject({0,0,0}, {0,0,0}, window);
    GLuint gridVAO  = createGridLines();
    GLuint gridYZVAO = createYZGridLines();

    render(window, planeVAO, gridVAO, gridYZVAO, {0,0,0});

    glfwTerminate();
    return 0;
}
