/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/20/15, Steven
 * 
 **/
#include "renderer.h"
#include "scene.h"

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
}


