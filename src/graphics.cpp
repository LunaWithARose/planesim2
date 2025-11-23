#include "graphics.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


float yaw   = -90.0f;
float pitch = 0.0f;
float zoom = 5.0f;
double lastX = 400, lastY = 300;
bool firstMouse = true;
bool rotating = false;
int width = 800;
int height = 500;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    static float lastX = 400, lastY = 300;
    static bool firstMouse = true;

    if (!rotating)  // <---- Add this guard
    {
        firstMouse = true;       // reset when not rotating
        return;
    }

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // clamp pitch to prevent flipping
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            rotating = true;
            firstMouse = true;   // re-sync starting point when click begins
        }
        else if (action == GLFW_RELEASE)
        {
            rotating = false;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom -= yoffset;
    zoom = glm::clamp(zoom, 1.0f, 50.0f);  // min/max zoom
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, width, height);
}


GLFWwindow* initializeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800,600,"Plane Simulator",nullptr,nullptr);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    return window;
}

// -----------------------------------------
// Shader helpers
// -----------------------------------------
GLuint compileShader(GLenum type, const char* src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    int success;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader error:\n" << log << "\n";
    }
    return sh;
}

GLuint makeProgram(const char* vsSrc, const char* fsSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// -----------------------------------------
// Plane object
// -----------------------------------------
GLuint createPlaneObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window)
{
    float verts[] = {
        -1,0,-1,   1,0,-1,   1,0,1,   -1,0,1
    };
    unsigned int idx[] = {0,1,2, 2,3,0};

    GLuint VAO,VBO,EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx),idx,GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    return VAO;
}

// -----------------------------------------
// XY grid
// -----------------------------------------
GLuint createGridLines()
{
    const int N = 20;

    int lines = (N * 2) + 1;        // -20..20 = 41
    int floatsPerLine = 12;         // 4 vertices * 3 comps
    float verts[41 * 12];           // 492 floats

    int ptr = 0;

    for (int i = -N; i <= N; i++)
    {
        float f = float(i);

        verts[ptr++] = f; verts[ptr++] = 0; verts[ptr++] = -N;
        verts[ptr++] = f; verts[ptr++] = 0; verts[ptr++] =  N;

        verts[ptr++] = -N; verts[ptr++] = 0; verts[ptr++] = f;
        verts[ptr++] =  N; verts[ptr++] = 0; verts[ptr++] = f;
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return VAO;
}

GLuint createYZGridLines()
{
    const int N = 20;

    int lines = (N * 2) + 1;        // -20..20 = 41
    int floatsPerLine = 12;         // 4 vertices * 3 comps
    float verts[41 * 12];           // 492 floats

    int ptr = 0;

    for (int i = -N; i <= N; i++)   // start from 0 instead of -N
    {
        float f = float(i);

        // vertical line in YZ plane
        verts[ptr++] = 0;   verts[ptr++] = f;   verts[ptr++] = -N;
        verts[ptr++] = 0;   verts[ptr++] = f;   verts[ptr++] =  N;

         // horizontal line (only positive Z)
        verts[ptr++] = 0;   verts[ptr++] = 0;   verts[ptr++] = f;
        verts[ptr++] = 0;   verts[ptr++] =  N;  verts[ptr++] = f;
    }


    GLuint VAO, VBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, floatsPerLine * sizeof(float), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return VAO;
}




void render(GLFWwindow* window, GLuint planeVAO, GLuint gridVAO, GLuint gridYZVAO, glm::vec3 pos)
{
    const char* vs = R"(
        #version 330 core
        layout (location=0) in vec3 aPos;

        uniform mat4 view;
        uniform mat4 proj;
        uniform mat4 model;

        void main() {
            gl_Position = proj * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fsPlane = R"(
        #version 330 core
        out vec4 FragColor;
        void main() { FragColor = vec4(0.5,0.5,0.5,1.0); }
    )";

    const char* fsGrid = R"(
        #version 330 core
        out vec4 FragColor;
        void main() { FragColor = vec4(0.1,0.1,0.1,1.0); }
    )";

    const char* fsGridYZ = R"(
    #version 330 core
    out vec4 FragColor;
    void main() { FragColor = vec4(0.4, 0.4, 0.4, 1.0); }
    )";


    GLuint planeProg = makeProgram(vs, fsPlane);
    GLuint gridProg  = makeProgram(vs, fsGrid);
    GLuint gridYZProg = makeProgram(vs, fsGridYZ);


    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.9,0.9,0.95,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float radYaw   = glm::radians(yaw);
        float radPitch = glm::radians(pitch);

        glm::vec3 cameraPos;
        cameraPos.x = pos.x + zoom * cos(radPitch) * cos(radYaw);
        cameraPos.y = pos.y + zoom * sin(radPitch);
        cameraPos.z = pos.z + zoom * cos(radPitch) * sin(radYaw);

        glm::mat4 view = glm::lookAt(cameraPos, pos, glm::vec3(0,1,0));
        float aspect = (float)width / (float)height;
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 200.0f);

        // ---- Plane ----
        glUseProgram(planeProg);
        glUniformMatrix4fv(glGetUniformLocation(planeProg,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(planeProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(planeProg,"model"),1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));

        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // ---- Grid ----
        glUseProgram(gridProg);
        glUniformMatrix4fv(glGetUniformLocation(gridProg,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gridProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(gridProg,"model"),1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));

        glBindVertexArray(gridVAO);
        glDrawArrays(GL_LINES, 0, 160 * 2);

        glUseProgram(gridYZProg);
        glUniformMatrix4fv(glGetUniformLocation(gridYZProg, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gridYZProg, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(gridYZProg, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glBindVertexArray(gridYZVAO);
        glDrawArrays(GL_LINES, 0, 160 * 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
