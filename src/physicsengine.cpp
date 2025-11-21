#include "physicsengine.h"
#include <cmath>

Airfoil::Airfoil(const std::vector<glm::vec3>& curve)
    : data(curve)
{
    if (!data.empty()) {
        min_alpha = data.front().x;
        max_alpha = data.back().x;
    } else {
        min_alpha = 0.0f;
        max_alpha = 0.0f;
    }
}

float roundToQuarter(float x) {
    return std::round(x / 0.25f) * 0.25f;
}

double scale(double x, double x_min, double x_max, double y_min, double y_max) {
    return y_min + ((x - x_min) * (y_max - y_min) / (x_max - x_min));
}

void updatePhysics(Aircraft& plane, float aoa, float dt) {
    const float rho = 1.225f;
    const glm::vec3 gravity(0.0f, -9.81f, 0.0f);

    aoa = roundToQuarter(aoa);

    auto [Cl, Cd] = plane.airfoil.sample(aoa);

    float speed = glm::length(plane.velocity);
    glm::vec3 velocityDir =
        (speed > 1e-6f)
        ? glm::normalize(plane.velocity)
        : glm::vec3(1.0f, 0.0f, 0.0f);

    plane.lift = glm::vec3(0,1,0) * 0.5f * rho * speed * speed * plane.wingArea * Cl;
    plane.drag = -velocityDir * 0.5f * rho * speed * speed * plane.wingArea * Cd;
    plane.thrustVec = velocityDir * plane.thrust;

    plane.totalForce = plane.lift + plane.drag + plane.thrustVec + gravity * plane.mass;

    plane.acceleration = plane.totalForce / plane.mass;

    plane.velocity += plane.acceleration * dt;
    plane.position += plane.velocity * dt;
}

Aircraft createAirplane(const Airfoil& foil,
    glm::vec3 position,
    glm::vec3 orientation,
    float mass,
    float wingArea,
    float wingspan){
    // Construct Aircraft using your constructor
    Aircraft plane(foil);

    // Now set the struct fields
    plane.position   = position;
    plane.orientation = orientation;
    plane.mass        = mass;
    plane.wingArea    = wingArea;
    plane.wingspan    = wingspan;

    // Return the completed object
    return plane;
}