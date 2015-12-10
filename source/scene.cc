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

mat4 const &SceneObject::modelview_inverse() const
{
  return modelview_inverse_;
}

void SceneObject::set_modelview(mat4 const &model_view)
{
  this->modelview_ = model_view;
  modelview_inverse_ = invert(model_view);
  normalview_ = transpose(modelview_inverse_);
}

mat4 const &SceneObject::normalview() const
{
  return normalview_;
}

mat4 const &SceneObject::modelview() const
{
  return modelview_;
}

//---------------------------------sphere intersections---------------------------------------

double UnitSphere::intersect_t(Ray const &ray)
{
  using namespace Angel;
  auto const &mv = modelview();
  auto world_origin = mv * vec4(0.0,0.0, 0.0, 1.0);
  auto world_radius = length(mv * vec4(0.0, 0.0, radius, 0.0));
  auto t = raySphereIntersection(ray.start, ray.dir, world_origin, world_radius);
  return t;
}

Intersection UnitSphere::intersect(Ray const &ray)
{
  auto t = intersect_t(ray);
  auto hitpoint = ray.start + t * ray.dir;
  return Intersection(t, normalize(xyz(normalview() * (modelview_inverse() * hitpoint))));
}


//---------------------------------plane intersections---------------------------------------
Intersection Plane::intersect(Ray const &ray)
{
  using namespace Angel;
  auto mvi = modelview_inverse();
  auto ray_local = Ray(mvi * ray.start, mvi * ray.dir);

  auto t = ray_plane_intersect(ray_local);



  return Intersection(t, xyz(normalview() * vec4(0.0, 0.0, 1.0, 0.0)));
}


}