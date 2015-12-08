/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 12/5/15, Steven
 * 
 **/
#include "scene.h"
#include "common-math.h"


namespace sls {

mat4 const &SceneObject::get_modelview_inverse() const
{
  return modelview_inverse;
}

void SceneObject::set_modelview(mat4 const &model_view)
{
  this->modelview = model_view;
  modelview_inverse = invert(model_view);
  normalview = transpose(modelview_inverse);
}

mat4 const &SceneObject::get_normalview() const
{
  return normalview;
}

mat4 const &SceneObject::get_modelview() const
{
  return modelview;
}

double UnitSphere::intersect(Ray ray)
{
  using namespace Angel;
  auto const &mv = get_modelview();
  auto world_origin = mv * vec4(0.0,0.0, 0.0, 1.0);
  auto world_radius = length(mv * vec4(0.0, 0.0, radius, 0.0));
  return raySphereIntersection(ray.start, ray.dir, world_origin, world_radius);
}

}