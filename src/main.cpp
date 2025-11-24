#include "graphics.h"
#include "physicsengine.h"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>

using namespace std;
using namespace glm;


int main()
{
    vector<vec3> NACA_4412_data = {
        {-9.500f, -0.3426f, 0.10705f}, {-9.250f, -0.3784f, 0.10671f}, {-9.000f, -0.4173f, 0.10641f}, {-8.750f, -0.3682f, 0.09949f}, {-8.500f, -0.3611f, 0.09726f}, {-8.250f, -0.3724f, 0.09561f}, {-8.000f, -0.4032f, 0.09481f}, {-7.750f, -0.4436f, 0.09403f}, {-7.500f, -0.4819f, 0.09082f}, {-7.250f, -0.4471f, 0.08830f}, {-7.000f, -0.4480f, 0.08640f}, {-6.750f, -0.4588f, 0.08412f}, {-6.500f, -0.4847f, 0.07929f}, {-6.250f, -0.4783f, 0.07718f}, {-6.000f, -0.4744f, 0.07516f}, {-5.750f, -0.4612f, 0.06965f}, {-5.500f, -0.4309f, 0.06730f}, {-5.250f, -0.3801f, 0.03672f}, {-5.000f, -0.3390f, 0.03524f}, {-4.750f, -0.2986f, 0.03253f}, {-4.500f, -0.2547f, 0.03033f}, {-4.250f, -0.2155f, 0.02874f}, {-4.000f, -0.1732f, 0.02773f}, {-3.750f, -0.1342f, 0.02656f}, {-3.500f, -0.0916f, 0.02579f}, {-3.250f, -0.0546f, 0.02511f}, {-3.000f, -0.0135f, 0.02457f}, {-2.750f, 0.0233f, 0.02414f}, {-2.500f, 0.0621f, 0.02363f}, {-2.250f, 0.0998f, 0.02306f}, {-2.000f, 0.1378f, 0.02248f}, {-1.750f, 0.1759f, 0.02179f}, {-1.500f, 0.2137f, 0.02102f}, {-1.250f, 0.2473f, 0.02001f}, {-1.000f, 0.3011f, 0.01877f}, {-0.750f, 0.3304f, 0.01877f}, {-0.500f, 0.3765f, 0.01838f}, {-0.250f, 0.4028f, 0.01841f}, { 0.000f, 0.4335f, 0.01835f}, { 0.250f, 0.4720f, 0.01803f}, { 0.500f, 0.4988f, 0.01808f}, { 0.750f, 0.5348f, 0.01782f}, { 1.000f, 0.5631f, 0.01782f}, { 1.250f, 0.5902f, 0.01789f}, { 1.500f, 0.6265f, 0.01763f}, { 1.750f, 0.6506f, 0.01784f}, { 2.000f, 0.6774f, 0.01798f}, { 2.250f, 0.7110f, 0.01786f}, { 2.500f, 0.7347f, 0.01814f}, { 2.750f, 0.7611f, 0.01836f}, { 3.000f, 0.7929f, 0.01834f}, { 3.250f, 0.8161f, 0.01870f}, { 3.500f, 0.8423f, 0.01895f}, { 3.750f, 0.8728f, 0.01902f}, { 4.000f, 0.8957f, 0.01941f}, { 4.250f, 0.9217f, 0.01970f}, { 4.500f, 0.9513f, 0.01982f}, { 4.750f, 0.9739f, 0.02025f}, { 5.000f, 0.9995f, 0.02057f}, { 5.250f, 1.0286f, 0.02074f}, { 5.500f, 1.0508f, 0.02119f}, { 5.750f, 1.0755f, 0.02148f}, { 6.000f, 1.1016f, 0.02159f}, { 6.250f, 1.1279f, 0.02168f}, { 6.500f, 1.1534f, 0.02184f}, { 6.750f, 1.1757f, 0.02207f}, { 7.000f, 1.1987f, 0.02216f}, { 7.250f, 1.2212f, 0.02223f}, { 7.500f, 1.2409f, 0.02240f}, { 7.750f, 1.2594f, 0.02262f}, { 8.000f, 1.2769f, 0.02285f}, { 8.250f, 1.2933f, 0.02308f}, { 8.500f, 1.3086f, 0.02333f}, { 8.750f, 1.3197f, 0.02372f}, { 9.000f, 1.3281f, 0.02421f}, { 9.250f, 1.3316f, 0.02495f}, { 9.500f, 1.3276f, 0.02605f}, { 9.750f, 1.3204f, 0.02765f}, {10.000f, 1.3127f, 0.02965f}, {10.250f, 1.3077f, 0.03176f}, {10.500f, 1.3074f, 0.03378f}, {10.750f, 1.3110f, 0.03571f}, {11.000f, 1.3189f, 0.03755f}, {11.250f, 1.3289f, 0.03922f}, {11.500f, 1.3439f, 0.04092f}, {11.750f, 1.3595f, 0.04250f}, {12.000f, 1.3787f, 0.04430f}, {12.250f, 1.3912f, 0.04604f}, {12.500f, 1.4202f, 0.04801f}, {12.750f, 1.4239f, 0.05002f}, {13.000f, 1.4319f, 0.05205f}, {13.250f, 1.4604f, 0.05447f}, {13.500f, 1.4540f, 0.05696f}, {13.750f, 1.4507f, 0.05963f}, {14.000f, 1.4512f, 0.06218f}, {14.250f, 1.4783f, 0.06519f}, {14.500f, 1.4597f, 0.06850f}, {14.750f, 1.4404f, 0.07234f}, {15.000f, 1.4201f, 0.07667f}, {15.250f, 1.3983f, 0.08150f}, {15.500f, 1.3742f, 0.08690f}, {15.750f, 1.3474f, 0.09302f}, {16.000f, 1.3171f, 0.10006f}, {16.250f, 1.2836f, 0.10833f}, {16.500f, 1.2473f, 0.11795f}, {16.750f, 1.2101f, 0.12886f}, {17.000f, 1.1753f, 0.14068f}
    };
    vector<glm::vec4> NACA_4412_data_with_Cm;
    for(auto& pt : NACA_4412_data) {
        float alpha = pt.x;
        float Cl    = pt.y;
        float Cd    = pt.z;
        float Cm    = estimateCm(alpha);
        NACA_4412_data_with_Cm.push_back(glm::vec4(alpha, Cl, Cd, Cm));
    }

    Airfoil airfoil(NACA_4412_data_with_Cm);
    Aircraft plane = createAirplane(airfoil, {0, 15, 0}, {0, 0, 0}, 2.0f, 0.4046f, 1.0f, 0.1524f, 0.0f, 0.0f);
    // N
    plane.thrust = 20.0f;
    float aoa = 5.0f; // deg
    GLFWwindow* window = initializeWindow();
    if (!window) {
        cerr << "Failed to initialize window.\n";
        return -1;
    }

    GLuint planeVAO  = createPlaneObject({0,0,0}, {0,0,0}, window);
    GLuint edgeVAO   = createPlaneEdgeObject({0,0,0}, {0,0,0}, window);

    GLuint gridXZ    = createGrid(20, GRID_XZ, HALF_NONE, &gridXYVertexCount);
    GLuint gridYZ    = createGrid(20, GRID_YZ, HALF_POSITIVE, &gridYZVertexCount);

    float lastTime = glfwGetTime();
    while(!glfwWindowShouldClose(window)){
        while ( plane.position.y > 0)
        {
            float now = glfwGetTime();
            float dt = now - lastTime;
            lastTime = now;

            // --- PHYSICS UPDATE PER FRAME ---
            updatePhysics(plane, aoa, 0.0f, dt);

            // You can print debug info if needed
            cout << "Lift: " << length(plane.lift)
                << " Drag: " << length(plane.drag)
                << " Thrust: " << length(plane.thrustVec)
                << " TotalForce: " << length(plane.totalForce)
                << " Accel: " << length(plane.acceleration)
                << "\n";

            // --- GRAPHICS POSITION FROM PHYSICS ---
            vec3 cubePos = plane.position;

            // --- RENDER FRAME ---
            renderFrame(window, planeVAO, edgeVAO, gridXZ, gridYZ, cubePos);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}



