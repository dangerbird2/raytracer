/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#ifndef RAYTRACER_RENDERER_H
#define RAYTRACER_RENDERER_H

#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include "types.h"
#include "scene.h"


namespace sls {


CommandLineArgs parse_args(int argc, char const **argv);

bool shadow_ray_unblocked(sls::Scene const &scene,
                          std::shared_ptr<sls::SceneObject> obj_a,
                          vec4 const &light_pos,
                          vec4 const &normal,
                          vec4 const &intersect_point);

Angel::vec3 reflected_ray(sls::Scene
                          scene,
                          Angel::vec4 const &vec4,
                          Angel::vec4 const
                          &normal,
                          Angel::vec4 const &intersect_pt
);


vec4 shade_ray_intersection(Scene const &scene, std::shared_ptr<SceneObject> obj, vec4 const &intersect_point,
                            vec4 normal_objview,
                            vec4 env_reflection = vec4(0.0, 0.0, 0.0, 0.0));

}


#endif //RAYTRACER_RENDERER_H
