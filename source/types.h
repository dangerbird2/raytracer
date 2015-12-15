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

  // gloss (phong) shading factor
  float k_specular = 0.00;
  // mirror shading factor
  float k_reflective = 0.0;

  float k_transmittance = 0.0;
  float k_ambient = 0.1;
  /**
   * brief specular phong exponent (shininess)
   */
  float shininess = 1.50;


  /**
   * brief index of refraction
   */
  float k_refraction = 1.53; // diamond
  //---------------------------------material prefabs---------------------------------------



  static Material glass()
  {
    auto self = Material();


    self.k_specular = 0.1;
    self.k_reflective = 0.1;
    self.k_diffuse = 0.0;
    self.k_transmittance = 1.0;
    self.k_refraction = 1.15;

    return self;
  }

  static Material wall_a()
  {
    auto self = Material();

    self.color = vec4(1.0, 0.0, 0.0, 1.0);

    self.shininess = 2.0;
    self.k_specular = 0.0;
    self.k_reflective = 0.0;
    self.k_diffuse = 1.0;
    self.k_transmittance = 0.0;

    return self;
  }

  static Material wall_b()
  {
    auto self = wall_a();

    self.color = vec4(0.0, 1.0, 1.0, 1.0);


    return self;
  }

  static Material wall_white()
  {
    auto self = Material();


    return self;
  }

  static Material floor()
  {
    auto self = Material();

    self.k_specular = 0.2;
    self.shininess = 40;


    return self;
  }

  static Material gold()
  {
    auto self = Material();

    self.color = vec4(1.0, 1.0, 0.0, 1.0);
    self.ambient = self.color;
    self.specular = vec4(1.0, 0.9, 0.7, 1.0);
    self.k_diffuse = 0.1;
    self.k_specular = 0.8;
    self.k_reflective = 0.7;

    self.shininess = 10.0;

    return self;

  }

};



}


#endif //RAYTRACER_TYPES_H
