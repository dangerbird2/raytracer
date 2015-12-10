/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#ifndef RAYTRACER_TYPES_H
#define RAYTRACER_TYPES_H

#include <vector>
#include <map>
#include <string>
#include "common/Angel.h"

namespace sls {

//---------------------------------alias vector names---------------------------------------
using vec4 = Angel::vec4;
using vec3 = Angel::vec3;
using vec2 = Angel::vec2;

using mat4 = Angel::mat4;
using mat3 = Angel::mat3;
using mat2 = Angel::mat2;


struct Ray final {
  // Simple struct
  vec4 start;
  vec4 dir;

  //-----------------------------ctors/dtors-------------------------------------
  Ray(vec4 start = vec4(0.0, 0.0, 0.0, 1.0),
      vec4 dir = vec4(0.0, 0.0, 1.0, 0.0)) :
      start(start), dir(dir) { }

  // non-virtual destructor
  ~Ray() { }
};

struct Intersection final {
  double t;
  vec3 normal;

  Intersection(double t = -1, vec3 normal = vec3(0.0, 0.0, 1.0)) :
      t(t), normal(normal) { }
};


struct CommandLineArgs {
  std::vector<std::string> argv;
  std::map<std::string, std::string> named_args;
};


struct Material {
private:
public:

  /**
   * brief material color
   */
  Angel::vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
  Angel::vec4 ambient = vec4(1.0, 1.0, 1.0, 1.0);
  Angel::vec4 specular = vec4(1.0, 1.0, 1.0, 1.0);


  /**
   * brief shading factors
   */
  float k_diffuse = 1.0;

  float k_specular = 1.0;
  float k_transmittance = 0.0;
  float k_ambient = 1.0;
  /**
   * brief specular phong exponent (shininess)
   */
  float shininess = 1.0;


  /**
   * brief index of refraction
   */
  float k_refraction = 1.53; // glass

};

}


#endif //RAYTRACER_TYPES_H
