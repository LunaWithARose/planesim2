#include "graphics.h"

#include <vector>
#include <iostream>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    (void)ori; (void)window;

        // Filled cube vertices
    float verts[] = {
        -1,-1,-1,  1,-1,-1,  1, 1,-1,  -1, 1,-1, // back face
        -1,-1, 1,  1,-1, 1,  1, 1, 1,  -1, 1, 1   // front face
    };

    // Indices for faces
    unsigned int idx[] = {
        0,1,2, 2,3,0, // back
        4,5,6, 6,7,4, // front
        0,4,7, 7,3,0, // left
        1,5,6, 6,2,1, // right
        0,1,5, 5,4,0, // bottom
        3,2,6, 6,7,3  // top
    };

    // Indices for edges
    unsigned int edges[] = {
        0,1, 1,2, 2,3, 3,0, // back face edges
        4,5, 5,6, 6,7, 7,4, // front face edges
        0,4, 1,5, 2,6, 3,7  // side edges
    };


    // --- Filled cube VAO ---
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW); // <-- use triangles here

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return VAO;
}

// Cube edges
GLuint createPlaneEdgeObject(glm::vec3 pos, glm::vec3 ori, GLFWwindow* window)
{
    float verts[] = {
        -1,-1,-1,  1,-1,-1,  1, 1,-1,  -1, 1,-1,
        -1,-1, 1,  1,-1, 1,  1, 1, 1,  -1, 1, 1
    };

    unsigned int edges[] = {
        0,1, 1,2, 2,3, 3,0, // back
        4,5, 5,6, 6,7, 7,4, // front
        0,4, 1,5, 2,6, 3,7  // sides
    };

    GLuint VAO,VBO,EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(edges),edges,GL_STATIC_DRAW);

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