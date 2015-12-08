/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#include "renderer.h"
#include "scene.h"
#include "common-math.h"

namespace sls {
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


bool shadow_ray_unblocked(sls::Scene const &scene,
                          std::shared_ptr<sls::SceneObject> obj_a,
                          vec4 const &light_pos,
                          vec4 const &normal,
                          vec4 const &intersect_point)
{
  for (auto const &obj_b: scene.objects) {
    if (obj_b != obj_a) {
      // TODO check for shadows.
      auto shadow_ray = Ray{intersect_point, normalize(light_pos - intersect_point)};
    }
  }
  return true;
}

vec3 reflected_ray(sls::Scene scene,
                   Angel::vec4 const &vec4,
                   Angel::vec4 const &normal,
                   Angel::vec4 const &intersect_pt)
{
  return vec3(0.0, 0.0, 0.0);
}

/*
auto imgui_setup() -> ImGuiIO
{

  auto io = ImGui::GetIO();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  io.DisplaySize = ImVec2(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
#pragma GCC diagnostic pop
  uint8_t *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  return io;
}
 */
vec4 shade_ray_intersection(Scene const &scene,
                            std::shared_ptr<SceneObject> obj,
                            vec4 const &intersect_point,
                            vec4 normal_objview, vec4 env_reflection)
{
  using namespace Angel;

  auto color = vec4();

  auto valid_reflection = true;
  auto const &mtl = obj->material;

  auto pos = xyz(intersect_point);

  auto normal = normalize(xyz(obj->get_normalview() * normal_objview));


  if (!valid_reflection) { // TODO test for invalid reflection
    color = vec3(0, 0, 0);
  } else { // simple local reflectance

    auto const n_lights = std::min(scene.light_locations.size(), scene.light_colors.size());
    assert(n_lights >= 0);

    for (auto i = 0; i < n_lights; ++i) {
      auto l_pos = normalize(xyz(scene.light_locations[0]));



      auto const &l_color = scene.light_colors[i];

      auto ambient = mtl.ambient * mtl.k_ambient * l_color.ambient_color;

      auto l_dir = normalize(xyz(l_pos - xyz(intersect_point)));
      auto kd = fmax(dot(l_dir, normal), 0.0);

      auto const unblocked =
          shadow_ray_unblocked(scene,
                               obj,
                               l_dir,
                               normal,
                               intersect_point);
      if (!unblocked && kd <= 0.0) {
        color = ambient;
        color.w = mtl.color.w;
        return color;
      }
      auto diffuse = l_color.diffuse_color * mtl.color * mtl.k_diffuse * kd;

      auto eye = vec3(normalize(-xyz(intersect_point)));
      auto reflect_dir = normalize(-reflect(l_dir, normal));

      //  phong specular
      auto spec_angle = fmax(dot(reflect_dir, eye), 0.0);

      auto ks = pow(spec_angle, mtl.shininess);

      auto spec_product = mtl.k_specular * l_color.specular_color * mtl.specular;
      auto specular = env_reflection + (ks * spec_product);

      if (nearlyEqual(kd, 0.0, 1e-7)) { specular = vec4(0.0, 0.0, 0.0, 1.0); }

      color = ambient + diffuse + specular;
    }

    color.w = obj->material.color.w;
  }

  return color;
}
}


