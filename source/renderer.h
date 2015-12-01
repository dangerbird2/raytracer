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


namespace sls {



Scene setup_scene();
CommandLineArgs parse_args(int argc, char const **argv);


SceneObject unit_sphere(double radius = 0.0);


}

double raySphereIntersection(vec4 p0, vec4 V,
                             double radius = 1.0,
                             vec4 origin = vec4(0.0, 0.0, 0.0, 1.0));



#endif //RAYTRACER_RENDERER_H
