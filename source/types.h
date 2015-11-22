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
#include "common-math.h"

namespace sls {

using intersection_fn_t =
std::function<double(vec4, vec4)>;

struct CommandLineArgs {
  std::vector<std::string> argv;
  std::map<std::string, std::string> named_args;
};


struct Material {
public:


  /**
   * brief material color
   */
  Angel::vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

  /**
   * brief diffuse factor
   */
  float k_diffuse = 1.0;

  /**
   * brief specular factor
   */
  float k_specular = 1.0;
  /**
   * brief specular phong exponent (shininess)
   */
  float k_n = 1.0;

  float k_transmittance = 0.0;
  float k_ambient = 1.0;

  /**
   * brief index of refraction
   */
  float k_refraction = 1.53; // glass

};



struct SceneObject {
public:
  Angel::mat4 model_view = Angel::identity();
  Material material;
  intersection_fn_t intersector = [](auto point, auto vector){
    return 0;
  };
};




struct Scene {
  std::vector<Angel::vec4> light_colors;
  std::vector<Angel::vec4> light_locations;

  std::vector<SceneObject> objects;
};




}



#endif //RAYTRACER_TYPES_H
