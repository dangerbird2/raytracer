/**
 * @file ${FILE}
 * @brief
 * @license ${LICENSE}
 * Copyright (c) 11/19/15, Steven
 *
 **/
#ifndef RAYTRACER_COMMON_MATH_H
#define RAYTRACER_COMMON_MATH_H

#include "common/Angel.h"
#include "types.h"
#include <array>
#include <functional>

namespace sls {

//---------------------------------foreward
//declarations---------------------------------------

//---------------------------------floating point near
//functions---------------------------------------

template <class T_REAL>
bool nearlyEquals(T_REAL a, T_REAL b, T_REAL epsilon) {
  const T_REAL absA = fabs(a);
  const T_REAL absB = fabs(b);
  const T_REAL diff = fabs(a - b);

  if (a == b) { // shortcut
    return true;
  } else if (a * b == 0) { // a or b or both are zero
    // relative error is not meaningful here
    return diff < (epsilon * epsilon);
  } else { // use relative error
    return diff / (absA + absB) < epsilon;
  }
}

//---------------------------------clamp
//functions---------------------------------------
template <typename T_REAL = double, typename T_SCALAR = T_REAL>
static T_REAL clamp(T_REAL const &val, T_SCALAR const &low,
                    T_SCALAR const &high) {
  return std::min<T_REAL>(std::max<T_REAL>(val, low), high);
}

static Angel::vec4 clamp(Angel::vec4 const &val, double const &low,
                         double const &high) {
  return Angel::vec4(clamp(val.x, low, high), clamp(val.y, low, high),
                     clamp(val.z, low, high), clamp(val.w, low, high));
}

static Angel::vec3 clamp(Angel::vec3 const &val, double const &low,
                         double const &high) {
  return Angel::vec3(clamp(val.x, low, high), clamp(val.y, low, high),
                     clamp(val.z, low, high));
}

static Angel::vec2 clamp(Angel::vec2 const &val, double const &low,
                         double const &high) {
  return Angel::vec2(clamp(val.x, low, high), clamp(val.y, low, high));
}

//---------------------------------vector conversion
//functions---------------------------------------

template <typename ANGEL_VEC> static Angel::vec3 xyz(ANGEL_VEC const &v) {
  return Angel::vec3(v.x, v.y, v.z);
}

template <typename ANGEL_VEC> static Angel::vec3 yzw(ANGEL_VEC const &v) {
  return Angel::vec3(v.y, v.z, v.w);
}

template <typename ANGEL_VEC> static Angel::vec2 xy(ANGEL_VEC const &v) {
  return Angel::vec2(v.x, v.y);
}

//---------------------------------geometric
//functions---------------------------------------

/**
 * @brief Reflect function based off the built-in GLSL function.
 * @detail The output vector has the same signness as in glsl
 */
template <typename ANGEL_VEC>
static ANGEL_VEC reflect(ANGEL_VEC const &incident, ANGEL_VEC const &normal) {
  using namespace Angel;
  const auto cos_theta = dot(-normal, incident);
  return incident - 2.0 * cos_theta * normal;
}

/**
 * @brief Refracts an incident vector given a number and
 * $eta = \frac{index_1}{index_2}$
 */
template <typename ANGEL_VEC, typename T_REAL = float>
static ANGEL_VEC refract(ANGEL_VEC const &incident, ANGEL_VEC const &normal,
                         T_REAL const &eta) {
  using namespace Angel;

  auto c = dot(normal, incident);
  auto k = 1.0 - (eta * eta) * (1.0 - (c * c));
  if (k < 0.0) {
    // past critical angle, refraction vector is imaginary
    return ANGEL_VEC(NAN); // isnan(result.x) indicates past critical angle
  } else {
    auto res = eta * incident - (eta * c + sqrtf(k)) * normal;
    return res;
  }
}

//---------------------------------intersections---------------------------------------

static double raySphereIntersection(vec4 p0, vec4 V,
                                    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0),

                                    double radius = 1.0) {
  double t = -1.0;
  double a = 1.0;
  double b = dot(2 * V, p0 - origin);
  double c = (length(p0 - origin) * length(p0 - origin)) - (radius * radius);

  double temp = b * b - (4 * a * c);
  if (temp < 0.0) {
    return t;
  }

  if (near(temp, 0.0, 1e-7)) {
    return (-b) / (2 * a);
  }

  double t1 = (-b + sqrt(temp)) / (2 * a);
  double t2 = (-b - sqrt(temp)) / (2 * a);
  return (t1 < t2) ? t1 : t2;
}

static double
ray_plane_intersect(Ray const &ray,
                    vec4 const &plane_p0 = vec4(0.0, 0.0, 0.0, 0.0),
                    vec4 const &plane_n = vec4(0.0, 0.0, 1.0, 0.0)) {
  using namespace Angel;
  auto n_normal = normalize(plane_n);
  auto n_dir = normalize(ray.dir);
  auto denominator = dot(n_normal, n_dir);

  if (denominator > 1e-7) {
    auto dist = plane_p0 - ray.start;
    auto t = dot(dist, plane_n) / denominator;

    return t;
  }

  return -1;
}

} // namespace sls

bool static nearlyEqual(double a, double b, double epsilon) {
  const double absA = fabs(a);
  const double absB = fabs(b);
  const double diff = fabs(a - b);

  if (a == b) { // shortcut
    return true;
  } else if (a * b == 0) { // a or b or both are zero
    // relative error is not meaningful here
    return diff < (epsilon * epsilon);
  } else { // use relative error
    return diff / (absA + absB) < epsilon;
  }
}

#endif // RAYTRACER_COMMON_MATH_H
