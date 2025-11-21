#pragma once
#ifndef MY_CLASS_H
#define MY_CLASS_H

#include <glm/glm.hpp>
#include <vector>
#include <tuple>
#include <algorithm>

double scale(double x, double x_min, double x_max, double y_min, double y_max);

struct Airfoil {
    float min_alpha, max_alpha;
    std::vector<glm::vec3> data;

    Airfoil(const std::vector<glm::vec3>& curve);

    std::tuple<float, float> sample(float alpha) const {
        int i = static_cast<int>(scale(alpha, min_alpha, max_alpha, 0, data.size() - 1));
        i = std::clamp(i, 0, (int)data.size() - 1);
        return { data[i].y, data[i].z };
    }
};

struct Aircraft {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 orientation = glm::vec3(0.0f);

    glm::vec3 lift = glm::vec3(0.0f);
    glm::vec3 drag = glm::vec3(0.0f);
    glm::vec3 thrustVec = glm::vec3(0.0f);
    glm::vec3 totalForce = glm::vec3(0.0f);

    float mass = 0.0f;
    float wingArea = 0.0f;
    float wingspan = 0.0f;
    float thrust = 0.0f;

    Airfoil airfoil;
    Aircraft(const Airfoil& af) : airfoil(af) {}
};

float roundToQuarter(float x);
void updatePhysics(Aircraft& plane, float aoa, float dt);
Aircraft createAirplane(const Airfoil& foil, glm::vec3 position = glm::vec3(0.0f), glm::vec3 orientation = glm::vec3(0.0f), float mass = 1.0f, float wingArea = 0.4046f, float wingspan = 2.0f);

#endif
