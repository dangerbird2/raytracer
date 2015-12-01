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

struct RayIntersection;
struct Ray;
using intersection_fn_t =
std::function<RayIntersection(Ray)>;

struct CommandLineArgs {
  std::vector<std::string> argv;
  std::map<std::string, std::string> named_args;
};


struct Material {
private:
  static Material default_mtl_instance;
public:

  static Material default_mtl();

  //Material() { }
//
  //Material(Material const &cpy) :
  //    color(cpy.color),
  //    k_diffuse(cpy.k_diffuse),
  //    k_specular(cpy.k_specular),
  //    k_n(cpy.k_n),
  //    k_transmittance(cpy.k_transmittance),
  //    k_ambient(cpy.k_ambient),
  //    k_refraction(cpy.k_refraction) { }

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

/**
 * @brief Object for rendering in Ray tracer
 * @detail Uses a non-virtual destructor. Do not
 * inherrit
 */
struct SceneObject {

public:


  SceneObject() { }

  SceneObject(SceneObject const &cpy) :
      model_view(cpy.model_view),
      intersector(cpy.intersector),
      material(nullptr)
  {
    using namespace std;
    if(material) {
      material = make_unique<Material>(*cpy.material);
    }
  }

  // non-virtual to prevent inheritance
  // (it is stored by value in stl containers.
  ~SceneObject() { }

  Angel::mat4 model_view;
  std::unique_ptr<Material> material = nullptr;
  intersection_fn_t intersector;
  //virtual double intersect (Angel::vec4 point, Angel::vec4 vector)  = 0;
};



struct Scene {
  std::vector<Angel::vec4> ambient_colors;
  std::vector<Angel::vec4> diffuse_colors;
  std::vector<Angel::vec4> specular_colors;
  std::vector<Angel::vec4> light_locations;

  std::vector<SceneObject> objects;
};

struct Ray {
  Angel::vec4 start;
  Angel::vec4 dir;
};

struct RayIntersection{
  Ray ray;
  bool does_intersect = false;
  Angel::vec4 p_hit;
  Angel::vec4 n_hit;
  double distance;
  double _t = 1.0;
};


}


#endif //RAYTRACER_TYPES_H
