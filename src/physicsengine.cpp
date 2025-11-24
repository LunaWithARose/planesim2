#include "physicsengine.h"
#include <cmath>
#include <algorithm>

// ---------------------------
// Airfoil Implementation
// ---------------------------
Airfoil::Airfoil(const std::vector<glm::vec4>& curve) : data(curve) {
    if (!data.empty()) {
        min_alpha = data.front().x;
        max_alpha = data.back().x;
    } else {
        min_alpha = 0.0f;
        max_alpha = 0.0f;
    }
}

AeroCoeffs Airfoil::sample(float alpha) {
    if (data.empty()) return {0, 0, 0, 0, 0};

    alpha = std::clamp(alpha, min_alpha, max_alpha);

    for (size_t i = 0; i < data.size() - 1; ++i) {
        if (alpha >= data[i].x && alpha <= data[i+1].x) {
            float t = (alpha - data[i].x) / (data[i+1].x - data[i].x);
            AeroCoeffs coeffs;
            coeffs.Cl      = data[i].y + t * (data[i+1].y - data[i].y);
            coeffs.Cd      = data[i].z + t * (data[i+1].z - data[i].z);
            coeffs.Cm      = data[i].w + t * (data[i+1].w - data[i].w);
            coeffs.Cl_roll = 0.0f; // placeholder
            coeffs.Cn_yaw  = 0.0f; // placeholder
            return coeffs;
        }
    }
    return {0,0,0,0,0};
}

// ---------------------------
// Aircraft Constructor
// ---------------------------
Aircraft::Aircraft(const Airfoil& foil) : airfoil(foil),
    position(0), velocity(0), acceleration(0),
    orientation(0), angularVelocity(0), angularAcceleration(0),
    mass(1.0f), wingArea(1.0f), wingspan(1.0f), chord(0.1f),
    thrust(0), inertia(1.0f),
    lift(0), drag(0), thrustVec(0), totalForce(0),
    pitchMoment(0), rollMoment(0), yawMoment(0) {}

// ---------------------------
// Utility
// ---------------------------
float roundToQuarter(float x) {
    return std::round(x / 0.25f) * 0.25f;
}
float estimateCm(float alpha) {
    // alpha in degrees
    // Cm roughly linear from -0.06 at 0° to -0.08 at +15°
    if(alpha < 0) return -0.05f;
    if(alpha > 15) return -0.09f;
    return -0.05f - 0.04f * (alpha / 15.0f); // linear interpolation
}

// ---------------------------
// Physics Update
// ---------------------------
void updatePhysics(Aircraft& plane, float aoa, float sideslip, float dt) {
    const float rho = 1.225f;
    const glm::vec3 gravity(0.0f, -9.81f, 0.0f);

    aoa = roundToQuarter(aoa);

    AeroCoeffs coeffs = plane.airfoil.sample(aoa);

    float V = glm::length(plane.velocity);
    glm::vec3 vDir = (V > 1e-6f) ? glm::normalize(plane.velocity) : glm::vec3(1,0,0);

    // Linear forces
    plane.lift     = glm::vec3(0,1,0) * 0.5f * rho * V * V * plane.wingArea * coeffs.Cl;
    plane.drag     = -vDir * 0.5f * rho * V * V * plane.wingArea * coeffs.Cd;
    plane.thrustVec = vDir * plane.thrust;

    plane.totalForce = plane.lift + plane.drag + plane.thrustVec + gravity * plane.mass;
    plane.acceleration = plane.totalForce / plane.mass;

    plane.velocity += plane.acceleration * dt;
    plane.position += plane.velocity * dt;

    // Moments (simplified)
    plane.pitchMoment = 0.5f * rho * V * V * plane.wingArea * plane.chord * coeffs.Cm;
    plane.rollMoment  = 0.5f * rho * V * V * plane.wingArea * plane.wingspan * coeffs.Cl_roll;
    plane.yawMoment   = 0.5f * rho * V * V * plane.wingArea * plane.wingspan * coeffs.Cn_yaw;

    // Angular dynamics
    plane.angularAcceleration = glm::vec3(
        plane.rollMoment / plane.inertia,
        plane.pitchMoment / plane.inertia,
        plane.yawMoment / plane.inertia
    );

    plane.angularVelocity += plane.angularAcceleration * dt;
    plane.orientation += plane.angularVelocity * dt;
}

// ---------------------------
// Aircraft Factory
// ---------------------------
Aircraft createAirplane(const Airfoil& foil,
                        glm::vec3 position,
                        glm::vec3 orientation,
                        float mass,
                        float wingArea,
                        float wingspan,
                        float chord,
                        float thrust,
                        float inertia) {
    Aircraft plane(foil);
    plane.position    = position;
    plane.orientation = orientation;
    plane.mass        = mass;
    plane.wingArea    = wingArea;
    plane.wingspan    = wingspan;
    plane.chord       = chord;
    plane.thrust      = thrust;
    plane.inertia     = inertia;
    return plane;
}
