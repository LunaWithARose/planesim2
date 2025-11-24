#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

// ---------------------------
// Aerodynamic Coefficients
// ---------------------------
struct AeroCoeffs {
    float Cl;       // lift coefficient
    float Cd;       // drag coefficient
    float Cm;       // moment coefficient (about pitch axis)
    float Cl_roll;  // unused placeholder
    float Cn_yaw;   // unused placeholder
};

// ---------------------------
// Airfoil: holds (alpha, Cl, Cd, Cm)
// ---------------------------
class Airfoil {
public:
    Airfoil(const std::vector<glm::vec4>& curve);

    // const-correct: can be called on const Airfoil&
    AeroCoeffs sample(float alpha_deg) const;

    // small helper to expose approximate 2D lift slope if needed
    float estimateClAlpha2D() const;

private:
    std::vector<glm::vec4> data; // (alpha_deg, Cl, Cd, Cm)
    float min_alpha;
    float max_alpha;
};

// ---------------------------
// Aircraft (now with quaternion orientation)
// ---------------------------
struct Aircraft {
    Aircraft(const Airfoil& foil);

    Airfoil airfoil;

    glm::vec3 position;          // world
    glm::vec3 velocity;          // world
    glm::vec3 acceleration;      // world

    glm::quat orientation;       // body <- world rotation (body to world: orientation * v_body)
    glm::vec3 angularVelocity;   // body rates (p,q,r) in body frame
    glm::vec3 angularAcceleration; // body frame

    float mass;

    // wing geometry (reference)
    float wingArea;
    float wingspan;
    float chord;

    // thrust (along body X axis)
    float thrust;

    // inertia: principal moments (Ixx, Iyy, Izz) in body frame
    glm::vec3 inertia;           // diagonal inertia principal moments
    glm::vec3 inertiaInv;        // precomputed inverse diagonal

    // forces/moments (world forces for forces, body moments for moments)
    glm::vec3 lift;              // world
    glm::vec3 drag;              // world
    glm::vec3 thrustVec;         // world
    glm::vec3 totalForce;        // world

    // moments in body frame
    glm::vec3 bodyMoment;        // (Mx, My, Mz) body frame
};

// ---------------------------
// Utility
// ---------------------------
float roundToQuarter(float x);
float estimateCm(float alpha_deg); // estimate Cm (deg input) fallback

// ---------------------------
// Aerodynamic Helper Functions
// ---------------------------
// Compute finite-aspect lift slope (per rad) from 2D slope (per rad)
float liftCurveSlopeFinite(float Cl_alpha_2D_per_rad, float AR);

// Compute induced angle (rad) approximate from CL and AR
float inducedAngleFromCL(float CL, float AR);

// Compute aerodynamic coefficients using paper method (whole-surface simplified).
// alpha_deg: geometric AoA in degrees (positive nose-up).
// AR: aspect ratio (b^2 / S).
// Cd0: skin friction baseline used for low-AoA CT estimation.
AeroCoeffs computeAeroCoeffsPaper(float alpha_deg, float AR, float Cd0);

// ---------------------------
// Physics Update
// ---------------------------
void updatePhysics(Aircraft& plane, float aoa_unused, float sideslip_unused, float dt);

// ---------------------------
// Factory
// ---------------------------
// orientationEuler in degrees: (pitch, yaw, roll) or (x,y,z) — interpreted as (pitch, yaw, roll).
Aircraft createAirplane(const Airfoil& foil,
                        glm::vec3 position,
                        glm::vec3 orientationEulerDeg,
                        float mass,
                        float wingArea,
                        float wingspan,
                        float chord,
                        float thrust,
                        glm::vec3 inertiaPrincipal);

#endif // PHYSICSENGINE_H
