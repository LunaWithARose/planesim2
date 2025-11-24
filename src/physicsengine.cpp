#define GLM_ENABLE_EXPERIMENTAL
#include "physicsengine.h"
#include <algorithm>
#include <cmath>
#include <glm/gtx/quaternion.hpp>

static constexpr float PI_F = 3.14159265358979323846f;
static constexpr float RAD2DEG = 180.0f / PI_F;
static constexpr float DEG2RAD = PI_F / 180.0f;

// ---------------------------
// Airfoil Implementation
// ---------------------------
Airfoil::Airfoil(const std::vector<glm::vec4>& curve)
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

AeroCoeffs Airfoil::sample(float alpha_deg) const
{
    if (data.empty()) return {0,0,0,0,0};

    alpha_deg = std::clamp(alpha_deg, min_alpha, max_alpha);

    for (size_t i = 0; i + 1 < data.size(); ++i) {
        if (alpha_deg >= data[i].x && alpha_deg <= data[i+1].x) {
            float t = (alpha_deg - data[i].x) / (data[i+1].x - data[i].x);
            AeroCoeffs c;
            c.Cl      = data[i].y + t * (data[i+1].y - data[i].y);
            c.Cd      = data[i].z + t * (data[i+1].z - data[i].z);
            c.Cm      = data[i].w + t * (data[i+1].w - data[i].w);
            c.Cl_roll = 0.0f;
            c.Cn_yaw  = 0.0f;
            return c;
        }
    }
    return {0,0,0,0,0};
}

float Airfoil::estimateClAlpha2D() const
{
    // Numerically estimate dCl/dalpha (per rad) around alpha ~= 0 using nearby samples.
    if (data.size() < 3) return 2.0f * PI_F;

    // find index bracketing 0 deg
    size_t idx = 0;
    for (size_t i = 0; i + 1 < data.size(); ++i) {
        if (data[i].x <= 0.0f && data[i+1].x >= 0.0f) { idx = i; break; }
    }

    // take small window of samples up to +/- 2 entries
    float sumNum = 0.0f, sumDen = 0.0f;
    int nSamples = 0;
    for (int k = -2; k <= 2; ++k) {
        int p1 = int(idx) + k;
        int p2 = p1 + 1;
        if (p1 >= 0 && p2 < (int)data.size()) {
            float a1 = data[p1].x * DEG2RAD;
            float a2 = data[p2].x * DEG2RAD;
            float c1 = data[p1].y;
            float c2 = data[p2].y;
            float da = (a2 - a1);
            if (std::fabs(da) > 1e-6f) {
                sumNum += (c2 - c1);
                sumDen += da;
                ++nSamples;
            }
        }
    }
    if (nSamples == 0 || std::fabs(sumDen) < 1e-9f) return 2.0f * PI_F;
    return sumNum / sumDen; // dCl/dα (per rad)
}

// ---------------------------
// Aircraft Constructor
// ---------------------------
Aircraft::Aircraft(const Airfoil& foil)
    : airfoil(foil),
      position(0.0f), velocity(0.0f), acceleration(0.0f),
      orientation(1.0f, 0.0f, 0.0f, 0.0f), // identity quat
      angularVelocity(0.0f), angularAcceleration(0.0f),
      mass(1.0f), wingArea(1.0f), wingspan(1.0f), chord(0.1f),
      thrust(0.0f), inertia(1.0f), inertiaInv(1.0f),
      lift(0.0f), drag(0.0f), thrustVec(0.0f), totalForce(0.0f),
      bodyMoment(0.0f)
{
}

// ---------------------------
// Utility
// ---------------------------
float roundToQuarter(float x) {
    return std::round(x / 0.25f) * 0.25f;
}

float estimateCm(float alpha_deg) {
    if (alpha_deg < 0.0f) return -0.05f;
    if (alpha_deg > 15.0f) return -0.09f;
    return -0.05f - 0.04f * (alpha_deg / 15.0f);
}

// ---------------------------
// Aerodynamics Helpers
// ---------------------------
float liftCurveSlopeFinite(float Cl_alpha_2D_per_rad, float AR)
{
    // Use lifting-line style correction: a = a0 / (1 + a0/(pi*AR))
    if (AR <= 0.0f) return Cl_alpha_2D_per_rad;
    return Cl_alpha_2D_per_rad / (1.0f + Cl_alpha_2D_per_rad / (PI_F * AR));
}

float inducedAngleFromCL(float CL, float AR)
{
    if (AR <= 0.0f) return 0.0f;
    return CL / (PI_F * AR);
}

// compute aerodynamic coefficients using the paper's approach (simplified single-surface model)
// - alpha_deg: geometric AoA (deg) positive nose-up
// - AR: aspect ratio
// - Cd0: baseline friction drag
AeroCoeffs computeAeroCoeffsPaper(float alpha_deg, float AR, float Cd0)
{
    // ----- parameters / safety -----
    const float stall_deg = 15.0f;   // nominal stall angle (±)
    const float CL_limit = 3.0f;     // hard clamp on CL to prevent explosion
    const float alpha_deg_abs = std::fabs(alpha_deg);

    // low-angle linear region
    if (alpha_deg_abs <= stall_deg) {
        // approximate 2D slope ~ 2π per rad (or estimate from airfoil if available)
        float a0 = 2.0f * PI_F; // per rad; for a better result pass in airfoil data externally
        float a = liftCurveSlopeFinite(a0, AR); // per rad

        float alpha_rad = alpha_deg * DEG2RAD;
        // zero-lift offset assumed zero for simplicity (or user table can define it)
        float alpha0_rad = 0.0f;

        float CL = a * (alpha_rad - alpha0_rad);
        // induced angle (rad)
        float alpha_i = inducedAngleFromCL(CL, AR);
        float alpha_eff = alpha_rad - alpha_i;

        // approximate normal/tangential (2D) decomposition
        float CN = CL / std::cos(alpha_eff);
        float CT = Cd0; // baseline tangential (skin friction)

        float CD = CN * std::sin(alpha_eff) + CT * std::cos(alpha_eff);
        float CM = 0.25f * CN; // simple representation

        // bound CL and CD
        CL = std::clamp(CL, -CL_limit, CL_limit);
        CD = std::max(CD, 0.0001f);

        return { CL, CD, CM, 0.0f, 0.0f };
    }

    // ----- post-stall (flat-plate style) -----
    {
        // convert to rad
        float alpha_rad = alpha_deg * DEG2RAD;
        // approximate induced angle taper from stall -> 90deg (linear taper)
        float sign = (alpha_deg >= 0.0f) ? 1.0f : -1.0f;
        float stall_rad = stall_deg * DEG2RAD;

        // approximate CL using flat-plate model:
        // CN ≈ 2 * sin(alpha)*cos(alpha) = sin(2α)
        float CN = std::sin(2.0f * alpha_rad);
        float CT = 0.5f * std::cos(alpha_rad);

        float CD = std::fabs(CN * std::sin(alpha_rad)) + std::fabs(CT * std::cos(alpha_rad));
        float CL = CN * std::cos(alpha_rad) - CT * std::sin(alpha_rad);

        // blend toward Cd90 near 90 deg - use baseline 1.98 (paper)
        float Cd90 = 1.98f;
        float fracNear90 = std::min(1.0f, std::fabs(alpha_deg) / 90.0f);
        CD = glm::mix(CD, Cd90, fracNear90 * 0.5f);

        // limit CL magnitude to prevent runaway
        CL = std::clamp(CL, -CL_limit, CL_limit);
        CD = std::max(CD, 0.01f);

        float CM = 0.25f * CN;

        return { CL, CD, CM, 0.0f, 0.0f };
    }
}

// ---------------------------
// Physics Update (REAL 3D orientation + stable aero)
// ---------------------------
void updatePhysics(Aircraft& plane, float /*aoa_unused*/, float /*sideslip_unused*/, float dt)
{
    // early out
    if (dt <= 0.0f) return;

    const float rho = 1.225f;
    const glm::vec3 gravity_world(0.0f, -9.81f, 0.0f);

    // --- 1) Kinematics: world <-> body frames using quaternion ---
    // Body-to-world: orientation * v_body
    // World-to-body: conj(orientation) * v_world
    glm::quat q = plane.orientation;
    glm::quat q_conj = glm::conjugate(q);

    // Transform velocity to body frame:
    glm::vec3 vel_body = glm::vec3(q_conj * glm::vec4(plane.velocity, 0.0f));

    float V = glm::length(plane.velocity);
    if (V < 1e-6f) V = 1e-6f; // avoid issues

    // AoA (deg) and sideslip (deg) in body frame
    // Using convention: x_body = forward, y_body = up, z_body = right (wing span along z)
    float aoa_rad = std::atan2(vel_body.y, vel_body.x); // positive nose-up
    float beta_rad = std::atan2(vel_body.z, vel_body.x); // sideslip (right positive)
    float aoa_deg = aoa_rad * RAD2DEG;
    float beta_deg = beta_rad * RAD2DEG;

    // Round AoA to quarter-degree as before (for airfoil table sampling)
    aoa_deg = roundToQuarter(aoa_deg);

    // --- 2) Aerodynamics: get coefficients using paper model ---
    float AR = (plane.wingArea > 1e-6f) ? (plane.wingspan * plane.wingspan / plane.wingArea) : 1.0f;
    const float Cd0 = 0.02f;
    AeroCoeffs coeffs = computeAeroCoeffsPaper(aoa_deg, AR, Cd0);

    // --- 3) Compute forces in body frame ---
    // dynamic pressure (use inertial speed)
    float qdyn = 0.5f * rho * V * V;

    // Lift direction in body frame: perpendicular to velocity and wing axis (z_body)
    glm::vec3 v_body_norm = glm::normalize(vel_body);
    glm::vec3 wingAxis_body(0.0f, 0.0f, 1.0f); // spanwise along +z in body
    glm::vec3 liftDir_body = glm::cross(glm::cross(v_body_norm, wingAxis_body), v_body_norm);
    if (glm::length(liftDir_body) < 1e-6f) {
        // fallback: use body up
        liftDir_body = glm::vec3(0.0f, 1.0f, 0.0f);
    } else {
        liftDir_body = glm::normalize(liftDir_body);
    }
    glm::vec3 dragDir_body = -v_body_norm; // opposite velocity in body

    // Compute body-frame force vectors
    glm::vec3 lift_body = liftDir_body * (qdyn * plane.wingArea * coeffs.Cl);
    glm::vec3 drag_body = dragDir_body * (qdyn * plane.wingArea * coeffs.Cd);
    glm::vec3 thrust_body = glm::vec3(1.0f, 0.0f, 0.0f) * plane.thrust; // thrust along body +X

    // Transform forces to world frame
    glm::vec3 lift_world = glm::vec3(q * glm::vec4(lift_body, 0.0f));
    glm::vec3 drag_world = glm::vec3(q * glm::vec4(drag_body, 0.0f));
    glm::vec3 thrust_world = glm::vec3(q * glm::vec4(thrust_body, 0.0f));

    // Save for debug/consumer code
    plane.lift = lift_world;
    plane.drag = drag_world;
    plane.thrustVec = thrust_world;

    // Sum forces in world frame and integrate linear motion
    plane.totalForce = lift_world + drag_world + thrust_world + gravity_world * plane.mass;
    plane.acceleration = plane.totalForce / plane.mass;

    // Integrate linear kinematics (semi-implicit Euler)
    plane.velocity += plane.acceleration * dt;
    plane.position += plane.velocity * dt;

    // --- 4) Compute aerodynamic moments in body frame ---
    // Pitch moment (about body Y) using CM nondimensional: M_y = CM * q * S * c
    float M_pitch = coeffs.Cm * qdyn * plane.wingArea * plane.chord;
    // Other rotational moments (roll/yaw) are left 0 for now or can be added from control surfaces
    float M_roll = 0.0f;
    float M_yaw  = 0.0f;

    // Compose body moment vector (Mx, My, Mz)
    plane.bodyMoment = glm::vec3(M_roll, M_pitch, M_yaw);

    // --- 5) Rotational dynamics: full rigid-body (body frame)
    // inertia is diagonal (Ixx,Iyy,Izz) stored in plane.inertia, inverse in inertiaInv
    glm::vec3 I = plane.inertia;
    glm::vec3 Iinv = plane.inertiaInv;

    // omega x (I * omega)
    glm::vec3 Iomega = glm::vec3(I.x * plane.angularVelocity.x,
                                 I.y * plane.angularVelocity.y,
                                 I.z * plane.angularVelocity.z);
    glm::vec3 omegaCrossIomega = glm::cross(plane.angularVelocity, Iomega);

    // Euler rotational equation: I * domega = M - omega x (I*omega)
    glm::vec3 domega;
    domega.x = Iinv.x * (plane.bodyMoment.x - omegaCrossIomega.x);
    domega.y = Iinv.y * (plane.bodyMoment.y - omegaCrossIomega.y);
    domega.z = Iinv.z * (plane.bodyMoment.z - omegaCrossIomega.z);

    // Integrate angular velocity
    plane.angularAcceleration = domega;
    plane.angularVelocity += plane.angularAcceleration * dt;

    // --- 6) Integrate quaternion orientation using body angular rates (p,q,r)
    // Quaternion kinematics: q_dot = 0.5 * q * omega_quat  (omega_quat = (0, ω_body))
    glm::quat omega_quat(0.0f, plane.angularVelocity.x, plane.angularVelocity.y, plane.angularVelocity.z);
    glm::quat qdot = 0.5f * (q * omega_quat);
    plane.orientation = glm::normalize(q + qdot * dt);

    // done
}
 
// ---------------------------
// Factory
// ---------------------------
Aircraft createAirplane(const Airfoil& foil,
                        glm::vec3 position,
                        glm::vec3 orientationEulerDeg,
                        float mass,
                        float wingArea,
                        float wingspan,
                        float chord,
                        float thrust,
                        glm::vec3 inertiaPrincipal)
{
    Aircraft plane(foil);
    plane.position = position;
    // orientationEulerDeg: treat as (pitch, yaw, roll) in degrees -> convert to quaternion
    glm::vec3 e = glm::radians(orientationEulerDeg);
    // build quaternion in order: yaw (Y), pitch (X), roll (Z) typical aerospace order may vary;
    // we'll assume (pitch, yaw, roll) -> rotate about X, then Y, then Z:
    glm::quat q_pitch = glm::angleAxis(e.x, glm::vec3(1,0,0));
    glm::quat q_yaw   = glm::angleAxis(e.y, glm::vec3(0,1,0));
    glm::quat q_roll  = glm::angleAxis(e.z, glm::vec3(0,0,1));
    plane.orientation = glm::normalize(q_yaw * q_pitch * q_roll);

    plane.mass = mass;
    plane.wingArea = wingArea;
    plane.wingspan = wingspan;
    plane.chord = chord;
    plane.thrust = thrust;

    plane.inertia = inertiaPrincipal;
    plane.inertiaInv = glm::vec3(
        (inertiaPrincipal.x > 1e-9f) ? 1.0f / inertiaPrincipal.x : 0.0f,
        (inertiaPrincipal.y > 1e-9f) ? 1.0f / inertiaPrincipal.y : 0.0f,
        (inertiaPrincipal.z > 1e-9f) ? 1.0f / inertiaPrincipal.z : 0.0f
    );

    return plane;
}
