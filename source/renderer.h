/**
 * @file ${FILE}
 * @brief
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 *
 **/
#ifndef RAYTRACER_RENDERER_H
#define RAYTRACER_RENDERER_H

#include "scene.h"
#include "types.h"
#include <assert.h>
#include <map>
#include <string>
#include <vector>

namespace sls {

CommandLineArgs parse_args(int argc, char const **argv);

bool shadow_ray_unblocked(sls::Scene const &scene,
                          std::shared_ptr<sls::SceneObject> obj,
                          vec4 const &light_pos, vec4 const &intersect_point);

Angel::vec3 reflected_ray(sls::Scene scene, Angel::vec4 const &vec4,
                          Angel::vec4 const &normal,
                          Angel::vec4 const &intersect_pt);

vec4 shade_ray_intersection(Scene const &scene,
                            std::shared_ptr<SceneObject> obj,
                            vec4 const &intersect_point, vec3 normal_sceneview,
                            vec4 env_reflection, vec4 env_refraction);

Ray get_reflection_ray(vec3 const &intersection, vec3 const &incident,
                       vec3 const &normal);

Ray get_refraction_ray(vec3 intersection, vec3 const &incident,
                       vec3 const &normal, float eta);
}

#endif // RAYTRACER_RENDERER_H
