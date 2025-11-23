#include "graphics.h"

#include <vector>
#include <iostream>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// -----------------------------
// Camera / UI state
// -----------------------------
float yaw   = -90.0f;
float pitch = -30.0f;  // look slightly down so plane is visible
float zoom  = 10.0f;   // start further back
double lastX = 400.0, lastY = 300.0;
bool firstMouse = true;
bool rotating = false;

// viewport size
int width = 800;
int height = 600;

// Vertex counts
int gridXYVertexCount = 0;
int gridYZVertexCount = 0;

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
    glViewport(0, 0, width, height);
    return window;
}

// -----------------------------
// Shader helpers
// -----------------------------
GLuint compileShader(GLenum type, const char* src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint success = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::cerr << "Shader compilation failed:\n" << log << "\n";
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

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "Program link failed:\n" << log << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// -----------------------------
// Plane object
// -----------------------------
GLuint createPlaneObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window)
{
    (void)pos; (void)ori; (void)window;

    float verts[] = {
        // positions
        -1,-1,-1,   1,-1,-1,   1,1,-1,   -1,1,-1, // back face
        -1,-1, 1,   1,-1, 1,   1,1, 1,   -1,1, 1  // front face
    };

    unsigned int idx[] = {
        // back face
        0,1,2, 2,3,0,
        // front face
        4,5,6, 6,7,4,
        // left face
        0,4,7, 7,3,0,
        // right face
        1,5,6, 6,2,1,
        // bottom face
        0,1,5, 5,4,0,
        // top face
        3,2,6, 6,7,3
    };

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

    glBindVertexArray(0);
    return VAO;
}


// -----------------------------
// XY grid
// -----------------------------
GLuint createGridLines(int N)
{
    std::vector<float> verts;
    for (int i = -N; i <= N; ++i) {
        float f = float(i);
        verts.push_back(-float(N)); verts.push_back(0.0f); verts.push_back(f);
        verts.push_back( float(N)); verts.push_back(0.0f); verts.push_back(f);
        verts.push_back(f); verts.push_back(0.0f); verts.push_back(-float(N));
        verts.push_back(f); verts.push_back(0.0f); verts.push_back( float(N));
    }

    gridXYVertexCount = static_cast<int>(verts.size()/3);

    GLuint VAO, VBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return VAO;
}

// -----------------------------
// YZ grid
// -----------------------------
GLuint createYZGridLines(int N)
{
   std::vector<float> verts;

    // Vertical lines along Y at each Z (now start from Y=0)
    for (int z = -N; z <= N; ++z) {
        verts.push_back(0.0f); verts.push_back(0.0f); verts.push_back(float(z));   // bottom vertex at y=0
        verts.push_back(0.0f); verts.push_back(float(N)); verts.push_back(float(z)); // top vertex at y=N
    }

    // Horizontal lines along Z at each Y (only above origin, Y=0..N)
    for (int y = 0; y <= N; ++y) {
        verts.push_back(0.0f); verts.push_back(float(y)); verts.push_back(-float(N)); // start of line along Z
        verts.push_back(0.0f); verts.push_back(float(y)); verts.push_back(float(N));  // end of line along Z
    }


    gridYZVertexCount = static_cast<int>(verts.size()/3);

    GLuint VAO, VBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return VAO;
}

// -----------------------------
// Render loop
// -----------------------------
void render(GLFWwindow* window, GLuint planeVAO, GLuint gridVAO, GLuint gridYZVAO, glm::vec3 pos)
{
    const char* vs = R"(
        #version 330 core
        layout (location=0) in vec3 aPos;
        uniform mat4 view;
        uniform mat4 proj;
        uniform mat4 model;
        void main() { gl_Position = proj * view * model * vec4(aPos,1.0); }
    )";

    const char* fsPlane = R"(
        #version 330 core
        out vec4 FragColor;
        void main(){ FragColor = vec4(0.5,0.5,0.5,1.0); }
    )";

    const char* fsGrid = R"(
        #version 330 core
        out vec4 FragColor;
        void main(){ FragColor = vec4(0.1,0.1,0.1,1.0); }
    )";

    const char* fsGridYZ = R"(
        #version 330 core
        out vec4 FragColor;
        void main(){ FragColor = vec4(0.4,0.4,0.4,1.0); }
    )";

    GLuint planeProg = makeProgram(vs, fsPlane);
    GLuint gridProg  = makeProgram(vs, fsGrid);
    GLuint gridYZProg = makeProgram(vs, fsGridYZ);

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.9f,0.9f,0.95f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float radYaw = glm::radians(yaw);
        float radPitch = glm::radians(pitch);

        glm::vec3 cameraPos;
        cameraPos.x = pos.x + zoom * cos(radPitch) * cos(radYaw);
        cameraPos.y = pos.y + zoom * sin(radPitch);
        cameraPos.z = pos.z + zoom * cos(radPitch) * sin(radYaw);

        glm::mat4 view = glm::lookAt(cameraPos,pos,glm::vec3(0,1,0));
        float aspect = float(width)/float(height);
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 200.0f);

        // Plane
        glUseProgram(planeProg);
        glUniformMatrix4fv(glGetUniformLocation(planeProg,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(planeProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(planeProg,"model"),1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));

        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        // XY grid
        glUseProgram(gridProg);
        glUniformMatrix4fv(glGetUniformLocation(gridProg,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gridProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(gridProg,"model"),1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));

        glBindVertexArray(gridVAO);
        if(gridXYVertexCount>0) glDrawArrays(GL_LINES,0,gridXYVertexCount);

        // YZ grid
        glUseProgram(gridYZProg);
        glUniformMatrix4fv(glGetUniformLocation(gridYZProg,"view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(gridYZProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(gridYZProg,"model"),1,GL_FALSE,glm::value_ptr(glm::mat4(1.0f)));

        glBindVertexArray(gridYZVAO);
        if(gridYZVertexCount>0) glDrawArrays(GL_LINES,0,gridYZVertexCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(planeProg);
    glDeleteProgram(gridProg);
    glDeleteProgram(gridYZProg);
}
