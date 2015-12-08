/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 12/5/15, Steven
 * 
 **/
#ifndef RAYTRACER_SCENE_H
#define RAYTRACER_SCENE_H

#include "types.h"
#include <memory>
#include <vector>
#include "common/Angel.h"

namespace sls {


/**
 * @brief Object for rendering in Ray tracer
 * @detail Uses a non-virtual destructor. Do not
 * inherrit
 */
struct SceneObject {

public:



  SceneObject(Material const &mtl = Material(), Angel::mat4 const &model_view = Angel::mat4())
      : material(mtl) {
    set_modelview(model_view);
  }

  SceneObject(SceneObject const &cpy) :
      SceneObject(cpy.material, cpy.modelview) {  }

  virtual
  ~SceneObject() { }

  Material material;

  virtual double intersect(Ray ray) = 0;


  mat4 const &get_modelview() const;

  void set_modelview(mat4 const &model_view);


  mat4 const &get_modelview_inverse() const;

  mat4 const &get_normalview() const;

private:
  Angel::mat4 modelview;
  Angel::mat4 modelview_inverse;
  Angel::mat4 normalview;

};

struct LightColor {
  Angel::vec4 ambient_color = vec4(1.0, 1.0, 1.0, 1.0);
  Angel::vec4 diffuse_color = vec4(0.01, 0.01, 0.01, 1.0);
  Angel::vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0);
};


struct Scene {

  Angel::mat4 camera_modelview;

  std::vector<LightColor> light_colors;
  std::vector<Angel::vec4> light_locations;

  std::vector<std::shared_ptr<SceneObject>> objects;
};

struct UnitSphere: public SceneObject {

  UnitSphere(Material const &mtl, Angel::mat4 modelview = Angel::mat4()): SceneObject(mtl, modelview){}
  double radius = 1;
  virtual double intersect(Ray ray) override;
};

}
#endif //RAYTRACER_SCENE_H
