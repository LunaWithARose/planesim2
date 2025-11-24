#pragma once
#include <vector>
#include <glm/glm.hpp>

// ---------------------------
// Aerodynamic Coefficients
// ---------------------------
struct AeroCoeffs {
    float Cl;       // Lift coefficient
    float Cd;       // Drag coefficient
    float Cm;       // Pitch moment coefficient
    float Cl_roll;  // Roll moment coefficient
    float Cn_yaw;   // Yaw moment coefficient
};

// ---------------------------
// Airfoil Class
// ---------------------------
class Airfoil {
public:
    std::vector<glm::vec4> data; // x=alpha, y=Cl, z=Cd, w=Cm
    float min_alpha, max_alpha;

    Airfoil(const std::vector<glm::vec4>& curve);

    // Lookup aerodynamic coefficients (linear interpolation)
    AeroCoeffs sample(float alpha);
};

// ---------------------------
// Aircraft Struct
// ---------------------------
struct Aircraft {
    Airfoil airfoil;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    glm::vec3 orientation;      // Euler angles: pitch, roll, yaw
    glm::vec3 angularVelocity;  // rad/s
    glm::vec3 angularAcceleration;

    float mass;
    float wingArea;
    float wingspan;
    float chord; // mean aerodynamic chord
    float thrust;
    float inertia; // Simplified scalar inertia

    // Forces
    glm::vec3 lift;
    glm::vec3 drag;
    glm::vec3 thrustVec;
    glm::vec3 totalForce;

    // Moments
    float pitchMoment;
    float rollMoment;
    float yawMoment;

    Aircraft(const Airfoil& foil);
};

// ---------------------------
// Utility Functions
// ---------------------------
float roundToQuarter(float x);

// ---------------------------
// Physics Update
// ---------------------------
void updatePhysics(Aircraft& plane, float aoa, float sideslip, float dt);

float estimateCm(float alpha);

// ---------------------------
// Aircraft Factory
// ---------------------------
Aircraft createAirplane(const Airfoil& foil, glm::vec3 position, glm::vec3 orientation, float mass, float wingArea, float wingspan, float chord, float thrust, float inertia);
