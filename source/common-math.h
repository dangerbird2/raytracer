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
#include "types.h"

namespace sls {
template<typename T_REAL>
bool near(const T_REAL &a, const T_REAL &b, const T_REAL &epsilon)
{
  if (a == b) {
    return true;
  }

  auto relative_err = fabs((a - b) / b);
  return relative_err <= epsilon;
}

template<typename T_REAL, class cmp>
cmp get_near_fn(const T_REAL &epsilon)
{
  return 0;
};

}

static bool nearlyEqual(double a, double b, double epsilon)
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





#endif //RAYTRACER_COMMON_MATH_H
