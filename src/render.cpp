#include "graphics.h"
#include "physicsengine.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <iostream>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

//
// Render ONE FRAME ONLY
//
void renderFrame(GLFWwindow* window,
                 GLuint planeVAO,
                 GLuint edgeVAO,
                 GLuint gridVAO,
                 GLuint gridYZVAO,
                 const glm::vec3& pos,
                 const glm::quat& orientation)
{
    const char* vs = R"(
        #version 330 core
        layout (location=0) in vec3 aPos;
        uniform mat4 view;
        uniform mat4 proj;
        uniform mat4 model;
        void main() {
            gl_Position = proj * view * model * vec4(aPos,1.0);
        }
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

    static GLuint planeProg  = makeProgram(vs, fsPlane);
    static GLuint edgeProg   = makeProgram(vs, fsGrid);
    static GLuint gridProg   = makeProgram(vs, fsGrid);
    static GLuint gridYZProg = makeProgram(vs, fsGridYZ);

    glClearColor(0.9f,0.9f,0.95f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // -----------------------------
    // Camera
    // -----------------------------
    float radYaw   = glm::radians(yaw);
    float radPitch = glm::radians(pitch);

    glm::vec3 cameraPos;
    cameraPos.x = pos.x + zoom * cos(radPitch) * cos(radYaw);
    cameraPos.y = pos.y + zoom * sin(radPitch);
    cameraPos.z = pos.z + zoom * cos(radPitch) * sin(radYaw);

    glm::mat4 view = glm::lookAt(cameraPos, pos, glm::vec3(0,1,0));
    float aspect = float(width) / float(height);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 200.0f);

    // -----------------------------
    // Apply rotation HERE
    // -----------------------------
    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), pos) *
        glm::toMat4(orientation);

    // -----------------------------
    // Draw cube (plane body)
    // -----------------------------
    glUseProgram(planeProg);
    glUniformMatrix4fv(glGetUniformLocation(planeProg,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(planeProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(planeProg,"model"),1,GL_FALSE,glm::value_ptr(model));

    glBindVertexArray(planeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // -----------------------------
    // Draw edges
    // -----------------------------
    glUseProgram(edgeProg);
    glUniformMatrix4fv(glGetUniformLocation(edgeProg,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(edgeProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(edgeProg,"model"),1,GL_FALSE,glm::value_ptr(model));

    glBindVertexArray(edgeVAO);
    glLineWidth(3.0f);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glLineWidth(1.0f);

    // -----------------------------
    // XY Grid
    // -----------------------------
    glUseProgram(gridProg);
    glUniformMatrix4fv(glGetUniformLocation(gridProg,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gridProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));

    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(gridProg,"model"),1,GL_FALSE,glm::value_ptr(identity));

    glBindVertexArray(gridVAO);
    if (gridXYVertexCount > 0)
        glDrawArrays(GL_LINES, 0, gridXYVertexCount);

    // -----------------------------
    // YZ Grid
    // -----------------------------
    glUseProgram(gridYZProg);
    glUniformMatrix4fv(glGetUniformLocation(gridYZProg,"view"),1,GL_FALSE,glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gridYZProg,"proj"),1,GL_FALSE,glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(gridYZProg,"model"),1,GL_FALSE,glm::value_ptr(identity));

    glBindVertexArray(gridYZVAO);
    if (gridYZVertexCount > 0)
        glDrawArrays(GL_LINES, 0, gridYZVertexCount);
}

