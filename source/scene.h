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


  SceneObject() { }

  SceneObject(SceneObject const &cpy) :
      model_view(cpy.model_view),
      material(cpy.material){ }

  virtual
  ~SceneObject() { }

  Material material;
  virtual double intersect (Ray ray)  = 0;


  mat4 const &get_model_view() const
  {
    return model_view;
  }

  void set_model_view(mat4 const &model_view)
  {
    this->model_view = model_view;
    model_view_inverse = invert(model_view);
    normal_view = transpose(model_view_inverse);
  }


  mat4 const &get_model_view_inverse() const
  {
    return model_view_inverse;
  }

  mat4 const &get_normal_view() const
  {
    return normal_view;
  }

private:
  Angel::mat4 model_view;
  Angel::mat4 model_view_inverse;
  Angel::mat4 normal_view;

};


struct Scene {
  std::vector<Angel::vec4> ambient_colors;
  std::vector<Angel::vec4> diffuse_colors;
  std::vector<Angel::vec4> specular_colors;
  std::vector<Angel::vec4> light_locations;

  std::vector<std::shared_ptr<SceneObject>> objects;
};

}
#endif //RAYTRACER_SCENE_H
