/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#include "renderer.h"

namespace  sls {
CommandLineArgs parse_args(int argc, char const **argv)
{
  using namespace std;
  assert(argc >= 0);
  auto args = CommandLineArgs();

  args.argv.reserve(size_t(argc));

  for (auto i=0lu; i<argc; ++i) {
    if (argv[i][0] != '\0') {
      args.argv.push_back(string(argv[i]));
    }
  }

  return args;
}

SceneObject unit_sphere(double radius)
{
  auto obj = SceneObject();

  obj.intersector = [radius](auto p, auto vector) {
    return raySphereIntersection(p, vector, Angel::vec4(0.0, 0.0, 0.0, 1.0), radius);
  };

  return obj;
}
}


