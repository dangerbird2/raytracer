/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/19/15, Steven
 * 
 **/
#ifndef RAYTRACER_COMMON_MATH_H
#define RAYTRACER_COMMON_MATH_H


#include <functional>
#include "common/Angel.h"

namespace sls {
template<typename T_REAL>
static
bool near(const T_REAL &a, const T_REAL &b, const T_REAL &epsilon)
{
  if (a == b) {
    return true;
  }

  auto relative_err = fabs((a - b) / b);
  return relative_err <= epsilon;
}


//---------------------------------clamp functions---------------------------------------
template<typename T_REAL = double, typename T_SCALAR = T_REAL>
static
T_REAL clamp(T_REAL const &val, T_SCALAR const &low, T_SCALAR const &high)
{
  return std::min<T_REAL>(std::max<T_REAL>(val, low), high);
}

static
Angel::vec4 clamp(Angel::vec4 const &val, double const &low, double const &high)
{
  return Angel::vec4(
      clamp(val.x, low, high),
      clamp(val.y, low, high),
      clamp(val.z, low, high),
      clamp(val.w, low, high)
  );
}

static
Angel::vec3 clamp(Angel::vec3 const &val, double const &low, double const &high)
{
  return Angel::vec3(
      clamp(val.x, low, high),
      clamp(val.y, low, high),
      clamp(val.z, low, high)
  );
}

static
Angel::vec2 clamp(Angel::vec2 const &val, double const &low, double const &high)
{
  return Angel::vec2(
      clamp(val.x, low, high),
      clamp(val.y, low, high)
  );
}

template<typename ANGEL_VEC>
static
Angel::vec3 xyz(ANGEL_VEC const &v)
{
  return Angel::vec3(v.x, v.y, v.z);
}

template<typename ANGEL_VEC>
static
Angel::vec2 xy(ANGEL_VEC const &v)
{
  return Angel::vec2(v.x, v.y);
}

template<typename ANGEL_VEC>
static
ANGEL_VEC reflect(ANGEL_VEC const &incident, ANGEL_VEC const &normal)
{
  return incident - 2.0 * Angel::dot(normal, incident) * normal;
}

}

static
bool nearlyEqual(double a, double b, double epsilon)
{
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


static double raySphereIntersection(vec4 p0, vec4 V,
                                    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0),

                                    double radius = 1.0)
{
  double t = -1.0;
  double a = 1.0;
  double b = dot(2 * V, p0 - origin);
  double c = (length(p0 - origin) * length(p0 - origin)) - (radius * radius);

  double temp = b * b - (4 * a * c);
  if (temp < 0.0) { return t; }

  if (nearlyEqual(temp, 0.0, 1e-7)) {
    return (-b) / (2 * a);
  }

  double t1 = (-b + sqrt(temp)) / (2 * a);
  double t2 = (-b - sqrt(temp)) / (2 * a);
  return (t1 < t2) ? t1 : t2;
}


#endif //RAYTRACER_COMMON_MATH_H
