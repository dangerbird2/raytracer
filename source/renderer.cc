/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#include "renderer.h"

namespace sls {

Material Material::default_mtl()
{
  auto mat = Material();

  return mat;
}

CommandLineArgs parse_args(int argc, char const **argv)
{
  using namespace std;
  assert(argc >= 0);
  auto args = CommandLineArgs();

  args.argv.reserve(size_t(argc));

  for (auto i = 0lu; i < argc; ++i) {
    if (argv[i][0] != '\0') {
      args.argv.push_back(string(argv[i]));
    }
  }

  return args;
}


Scene setup_scene()
{
  using namespace Angel;
  using namespace std;
  auto scene = Scene();


  scene.ambient_colors.push_back(vec4(1.f, 0.9f, 0.9f));
  scene.diffuse_colors.push_back(vec4(1.f, 0.9f, 0.9f));
  scene.specular_colors.push_back(vec4(1.f, 0.9f, 0.9f));
  scene.light_locations.push_back(vec4(1.0, 1.0, 10.0, 1.0));


  scene.objects.push_back(move(unit_sphere(1.0)));

  return scene;
}

SceneObject unit_sphere(double radius)
{
  auto sphere = SceneObject();
  auto origin = vec4(0.0, 0.0, 0.0, 1.0);
  sphere.intersector = [radius, origin](Ray ray) {

    auto t = raySphereIntersection(ray.start, ray.dir, radius, origin);
    auto inter = RayIntersection();
    inter.does_intersect = t > 0;
    inter.ray = ray;
    inter._t = t;

    return inter;
  };

  sphere.model_view = Angel::mat4();

  return sphere;
}
}

double raySphereIntersection(vec4 p0, vec4 V,
                             double radius,
                             vec4 origin)
{
  auto inter = sls::RayIntersection{};
  double t = -1.0;
  double a = dot(V, V);
  double b = dot(2 * V, p0 - origin);
  double c = (length(p0 - origin) * length(p0 - origin)) - (radius * radius);


  double temp = b * b - (4 * a * c);
  inter.does_intersect = temp <= 0.0;

  if (inter.does_intersect) { return temp; }


  if (nearlyEqual(temp, 0.0, 1e-7)) {
    return (-b) / (2 * a);
  }


  double t1 = (-b + sqrt(temp)) / (2 * a);
  double t2 = (-b - sqrt(temp)) / (2 * a);
  assert(!isnan(t1) && !isnan(t2));
  return (t1 < t2) ? t1 : t2;
}


