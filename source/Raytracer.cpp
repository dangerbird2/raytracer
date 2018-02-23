//
//  Display a rotating cube with lighting
//
//  Light and material properties are sent to the shader as uniform
//    variables.  Vertex positions and normals are sent after each
//    rotation.


#include "slsgl.h"
#include "Angel.h"
#include "ObjMesh.h"
#include "SourcePath.h"
#include "Trackball.h"
#include <cstdlib>

#include "common-math.h"

#include "image-utils.h"
#include "renderer.h"

#include "async-tools.h"
#include "scene.h"

#include <atomic>
#include <random>

struct RTWorkFlag {
  std::atomic<bool> is_raytracing;
  std::atomic<bool> signal_quit_raytracing;
  std::thread thread;
};

struct RTConfig {
  bool ss_antialias = false;
  int supersample_factor = 2;
  int width = 1920;
  int height = 1080;

  bool use_window_size = false;
};

static GLFWwindow *WINDOW;

void setup_scene(vec4 const &material_diffuse, vec4 const &material_ambient,
                 vec4 const &material_specular);

vec4 castRay(vec4 p0, vec4 dir, size_t depth, size_t max_depth = 10,
             std::shared_ptr<sls::SceneObject> obj = nullptr);

void bind_viewport(int pInt[4]);

std::vector<std::vector<sls::rt_data>> get_rt_work(int width, int height,
                                                   int n_threads);

vec4 castRay(sls::Ray const &ray, size_t depth, size_t max_depth = 10,
             std::shared_ptr<sls::SceneObject> obj = nullptr) {
  return castRay(ray.start, ray.dir, depth, max_depth, obj);
}

//---------------------------------type
//aliases---------------------------------------
using color4 = Angel::vec4;
using point4 = Angel::vec4;

//---------------------------------app file
//info---------------------------------------

static sls::CommandLineArgs app_args;
static std::string out_file_name;
static std::thread::id main_id;

//---------------------------------opengl
//info---------------------------------------
int window_width, window_height;

bool render_line;
Mesh sphere_mesh;

GLuint vPosition, vNormal, vTexCoord;

GLuint program;

// Model-view and projection matrices uniform location
GLuint ModelViewEarth, ModelViewLight, NormalMatrix, Projection;

mat4 projection;
mat4 model_view;

static auto scene = sls::Scene();

struct LightUnifs {
  GLuint diffuse_prods;
  GLuint ambient_prods;
  GLuint specular_prods;

  GLuint light_locations;

  GLuint n_lights;

  GLuint shininess;
} light_unifs;

constexpr size_t max_lights = 8;

//==========Trackball Variables==========
static float curquat[4], lastquat[4];
/* current transformation matrix */
static float curmat[4][4];
mat4 curmat_a;
/* actual operation  */
static int scaling;
static int moving;
static int panning;
/* starting "moving" coordinates */
static int beginx, beginy;
/* ortho */
float ortho_x, ortho_y;
/* current scale factor */
static float scalefactor;

RTWorkFlag rt_flags;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
sls::Ray findRay(GLdouble x, GLdouble y, int width, int height) {
  static std::mutex locker;

  y = height - y;
  GLdouble modelViewMatrix[16];
  GLdouble projectionMatrix[16];
  int viewport[4];

  locker.lock();

  bind_viewport(viewport);

  locker.unlock();
  float aspect = viewport[2] / float(viewport[3]);
  viewport[2] = width;
  viewport[3] = width / aspect;

  // ...

  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 4; j++) {
      modelViewMatrix[j * 4 + i] = model_view[i][j];
      projectionMatrix[j * 4 + i] = projection[i][j];
    }
  }

  GLdouble nearPlaneLocation[3];
  gluUnProject(x, y, 0.0, modelViewMatrix, projectionMatrix, viewport,
               &nearPlaneLocation[0], &nearPlaneLocation[1],
               &nearPlaneLocation[2]);

  GLdouble farPlaneLocation[3];
  gluUnProject(x, y, 1.0, modelViewMatrix, projectionMatrix, viewport,
               &farPlaneLocation[0], &farPlaneLocation[1],
               &farPlaneLocation[2]);

  vec4 ray_origin = vec4(nearPlaneLocation[0], nearPlaneLocation[1],
                         nearPlaneLocation[2], 1.0);
  vec3 temp = vec3(farPlaneLocation[0] - nearPlaneLocation[0],
                   farPlaneLocation[1] - nearPlaneLocation[1],
                   farPlaneLocation[2] - nearPlaneLocation[2]);
  temp = normalize(temp);
  vec4 ray_dir = vec4(temp.x, temp.y, temp.z, 0.0);

  return sls::Ray(ray_origin, ray_dir);
}

/**
 * used to querry GL_VIEWPORT from multiple threads
 */
void bind_viewport(int pInt[4]) {
  static int viewport[4];
  auto tid = std::this_thread::get_id();
  static std::mutex mtx;
  if (!pInt && (tid == main_id)) {
    glGetIntegerv(GL_VIEWPORT, viewport);
  } else if (pInt) {
    mtx.lock();
    memcpy((void *)pInt, (void *)viewport, sizeof(int) * 4);
    mtx.unlock();

  } else {
    throw std::runtime_error("you must bind GL_VIEWPORT from the main thread");
  };
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void castRayDebug(vec4 p0, vec4 dir) {
  for (auto i : scene.objects) {
    double t = i->intersect_t(sls::Ray{p0, dir});

    if (t > 0) {
      auto color = castRay(p0, dir, 0, 3, nullptr);
      vec4 temp = p0 + t * dir;
      vec3 temp_3 = vec3(temp.x, temp.y, temp.z);
      std::cout << p0 + t * dir << "\t\t" << length(temp_3) << "\n"
                << "color: " << color << '\n';
    }
  }
}

/* -------------------------------------------------------------------------- */

vec4 castRay(vec4 p0, vec4 dir, size_t depth, size_t max_depth,
             std::shared_ptr<sls::SceneObject> current_object) {

  // castRayDebug(p0, dir);

  using namespace sls;
  using namespace std;
  auto clear_color = vec4(0.0, 0.0, 0.0, 0.0);
  auto ray_viewspace = sls::Ray{p0, dir};
  auto bg_color = vec4(1.0, 0.0, 1.0, 1.0);

  auto color = clear_color;

  if (depth > max_depth) {
    return clear_color;
  }

  double z_depth = NAN;
  struct obj_hit_t {
    shared_ptr<SceneObject> obj;
    Intersection inter;
    vec4 hit_point;
  };

  auto hit_found = false;
  auto nearest_hit = obj_hit_t();

  // iterate one time to get maximum depth
  // for ray
  for (auto obj : scene.objects) {
    if ((obj->target & sls::TargetRayTracer) != sls::TargetRayTracer) {
      continue;
    }
    auto intersection = obj->intersect(ray_viewspace);
    auto hit_point = p0 + intersection.t * dir;
    if (intersection.t >= 0) {

      auto hit = obj_hit_t{obj, intersection, hit_point};

      if (!hit_found) {
        hit_found = true;
        nearest_hit = hit;
      } else if (intersection.t < nearest_hit.inter.t) {
        nearest_hit = hit;
      }
    }
  }

  if (hit_found) {

    auto const &obj = nearest_hit.obj;
    auto const &intersection = nearest_hit.inter;

    auto t = intersection.t;
    auto normal = normalize(intersection.normal);
    auto obj_name = obj->name;

    auto hit_viewspace = nearest_hit.hit_point;

    auto reflection = vec4(0.0, 0.0, 0.0, 0.0);
    auto transmitted = vec4(0.0, 0.0, 0.0, 0.0);

    auto const &mtl = obj->material;
    if (mtl.k_reflective > 0.0 ||
        mtl.k_specular > 0.0) { // non-zero reflectivity
      auto reflect_dir = normalize(-reflect(dir, normalize(vec4(normal, 0.0))));
      reflection =
          castRay(hit_viewspace, reflect_dir, depth + 1, max_depth, obj);
    }

    if (mtl.k_transmittance > 1e-7) { // non-zero transmittance
      auto inside_obj = dot(dir, normal) < 0;
      auto outer_ior =
          inside_obj ? scene.space_k_refraction : obj->material.k_refraction;
      auto inner_ior =
          inside_obj ? obj->material.k_refraction : scene.space_k_refraction;

      auto refraction_ray = get_refraction_ray(xyz(hit_viewspace), xyz(dir),
                                               normal, inner_ior / outer_ior);
      // move refraction ray a bit foreward
      refraction_ray.start += refraction_ray.dir / 1000.0;
      transmitted = castRay(refraction_ray, depth + 1, max_depth, obj);
    }

    color += sls::shade_ray_intersection(scene, obj, hit_viewspace, normal,
                                         reflection, transmitted);
  }

  return sls::clamp(color, 0.0, 1.0);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * @brief Performs the ray tracing algorithm.
 * @detail allows multiple sampling for diffuse path tracing or
 * similar algorithms, writing to image each sample for instant feedback
 */
void rayTrace(size_t max_samples = 1, RTConfig cf = RTConfig()) {
  using namespace std;
  using namespace sls;
  rt_flags.is_raytracing = true;

  auto width = cf.width;
  auto height = cf.height;

  if (cf.use_window_size) {
    width = window_width;
    height = window_height;
  }

  auto ss_factor = cf.ss_antialias ? max(cf.supersample_factor, 1) : 1;
  auto supersample_width = width * ss_factor;
  auto supersample_height = height * ss_factor;

  assert(ss_factor > 0);

  cout << "rendering supersamples " << supersample_width << " * "
       << supersample_height << "\n";

  auto buffer = vector<uint8_t>(width * height * 4);

  auto supersamples = vector<color4>(supersample_width * supersample_height);
  auto color_buffer = vector<color4>(width * height);

  using loop_pair_t = pair<size_t, vector<vec4>>;

  // params
  auto const n_threads = 20;
  auto const max_rt_depth = 6;

  auto work_units =
      get_rt_work(supersample_width, supersample_height, n_threads);

  auto results = vector<future<vector<rt_data>>>();

  using namespace std;
  auto rng = default_random_engine(random_device()());

  auto sample = 0;
  for (sample = 0; sample < max_samples; ++sample) {
    if (rt_flags.signal_quit_raytracing) {
      rt_flags.signal_quit_raytracing = false;
      break;
    }

    auto light_locs = scene.light_locations;
    auto std_dev = 0.1;

    for (auto &l : scene.light_locations) {
      auto dist_x = normal_distribution<float>(l.x, std_dev);
      auto dist_y = normal_distribution<float>(l.y, std_dev);
      auto dist_z = normal_distribution<float>(l.z, std_dev);

      l = vec4(dist_x(rng), dist_y(rng), dist_z(rng), l.w);
    }

    for (auto &unit : work_units) {
      results.push_back(raycast_async(
          [&](auto &i) {
            i.color =
                castRay(i.rays.start, i.rays.dir, 0, max_rt_depth, nullptr);
            return i;
          },
          unit));
    }

    for (auto &fut : results) {
      auto unit = fut.get();
      for (auto const &data : unit) {

        auto idx = data.j * supersample_width + data.i;
        supersamples[idx] = data.color;
      }
      for (auto j = 0; j < height; ++j) {
        for (auto i = 0; i < width; ++i) {

          auto idx = j * width + i;
          auto const n_subpixels = ss_factor * 4;

          // calculate range to take subpixels
          auto dist = poisson_distribution<int>(ss_factor / 2);

          auto ss_j_min = j * ss_factor;
          auto ss_j_max = ss_j_min + ss_factor;

          auto ss_i_min = i * ss_factor;
          auto ss_i_max = ss_i_min + ss_factor;

          color4 acc = vec4(0.0, 0.0, 0.0, 0.0);

          if (ss_factor > 1) {

            for (auto k = 0; k < n_subpixels; ++k) {
              // poisson ss_aa

              auto sample_i = clamp(dist(rng), 0, ss_factor - 1);
              auto sample_j = clamp(dist(rng), 0, ss_factor - 1);

              auto ii = ss_i_min + sample_i;
              auto jj = ss_j_min + sample_j;

              auto ss_idx = jj * supersample_width + ii;

              acc += supersamples[ss_idx];
            }

            acc /= float(n_subpixels);

          } else if (ss_factor == 1) {
            acc = supersamples[j * supersample_width + i];
          }

          auto buff = &buffer[idx * 4];
          auto const &color = acc;

          // get weighted average of samples
          if (sample > 0) {
            auto sum_color = (color_buffer[idx] * float(sample) + color);
            auto mean_color = sum_color / float(sample + 1);
            color_buffer[idx] = mean_color;

          } else {
            color_buffer[idx] = color;
          }

          auto &color_to_write = color_buffer[idx];

          buff[0] = static_cast<uint8_t>((color_to_write.x) * 255);
          buff[1] = static_cast<uint8_t>((color_to_write.y) * 255);
          buff[2] = static_cast<uint8_t>((color_to_write.z) * 255);
          buff[3] = static_cast<uint8_t>((color_to_write.w) * 255);
        }
      }
    }

    write_image(out_file_name, &buffer[0], width, height, 4);
    results.clear();

    scene.light_locations = light_locs;
  }

  cout << "\ntraced " << sample
       << ((sample > 1) ? " samples.\n" : " sample.\n");
  rt_flags.is_raytracing = false;
}

std::vector<std::vector<sls::rt_data>> get_rt_work(int width, int height,
                                                   int n_threads) {
  using namespace std;
  using namespace sls;
  auto len = width * height;
  auto step = len / n_threads;

  auto work_unit = vector<rt_data>();

  auto work_units = vector<decltype(work_unit)>();

  // use same ray data for each sample. Should be placed
  // inside loop if motion blur is to be emulated
  for (auto i = 0; i < width; ++i) {
    for (auto j = 0; j < height; ++j) {
      auto res = rt_data();
      int idx = j * width + i;

      res.i = size_t(i);
      res.j = size_t(j);
      res.rays = findRay(i, j, width, height);
      res.color = vec4(1.0, 0.0, 1.0, 1.0);

      work_unit.push_back(res);

      if (work_unit.size() > step) {
        work_units.push_back(work_unit);
        work_unit.clear();
      }
    };
  }
  if (work_unit.size() > 0) {
    work_units.push_back(work_unit);
  }

  return work_units;
}

//------------------------------------------------------------------------
//---------------------------------scene setup
//---------------------------------
void init() {
  sphere_mesh.makeSubdivisionSphere(10);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  std::string vshader = source_path + "/shaders/vshading_example.glsl";
  std::string fshader = source_path + "/shaders/fshading_example.glsl";

  program = InitShader(vshader.c_str(), fshader.c_str(), nullptr);
  // Load shaders and use the resulting shader program
  glUseProgram(program);

  color4 light_ambient(1.0, 1.0, 1.0, 1.0);
  color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
  color4 light_specular(1.0, 1.0, 1.0, 1.0);

  color4 material_ambient(1.0, 1.0, 1.0, 1.0);
  color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
  color4 material_specular(1.0, 1.0, 1.0, 1.0);

  color4 ambient_product = light_ambient * material_ambient;
  color4 diffuse_product = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;

  setup_scene(material_diffuse, material_ambient, material_specular);

  light_unifs.ambient_prods =
      GLuint(glGetUniformLocation(program, "AmbientProducts"));
  light_unifs.diffuse_prods =
      GLuint(glGetUniformLocation(program, "DiffuseProducts"));
  light_unifs.specular_prods =
      GLuint(glGetUniformLocation(program, "SpecularProducts"));

  light_unifs.light_locations =
      GLuint(glGetUniformLocation(program, "LightPositions"));
  auto shininess = glGetUniformLocation(program, "Shininess");
  light_unifs.shininess = GLuint(shininess >= 0 ? GLuint(shininess) : 0);

  auto n_lights = glGetUniformLocation(program, "n_lights");
  light_unifs.n_lights = n_lights >= 0 ? GLuint(n_lights) : 0;

  // Retrieve transformation uniform variable locations
  ModelViewEarth = glGetUniformLocation(program, "ModelViewEarth");
  ModelViewLight = glGetUniformLocation(program, "ModelViewLight");
  NormalMatrix = glGetUniformLocation(program, "NormalMatrix");
  Projection = glGetUniformLocation(program, "Projection");

  glUniform1i(glGetUniformLocation(program, "texture0"), 0);
  glUniform1i(glGetUniformLocation(program, "texture1"), 1);

  glEnable(GL_DEPTH_TEST);


  glClearColor(0.8, 0.8, 1.0, 1.0);

  scaling = 0;
  moving = 0;
  panning = 0;
  beginx = 0;
  beginy = 0;

  matident(curmat);
  trackball(curquat, 0.0f, 0.0f, 0.0f, 0.0f);
  trackball(lastquat, 0.0f, 0.0f, 0.0f, 0.0f);
  add_quats(lastquat, curquat, curquat);
  build_rotmatrix(curmat, curquat);

  scalefactor = 1.0;
  render_line = false;
}

void setup_scene(vec4 const &material_diffuse, vec4 const &material_ambient,
                 vec4 const &material_specular) {
  using namespace sls;
  using namespace std;
  using namespace Angel;
  auto mtl = Material();

  // cornell box sphere params (using big sphere wall hack)
  auto r = 1e2;
  auto box_width = 1.0;
  auto back_mv = Translate(0.0, 0.0, -r - box_width) * Scale(r, r, r);
  auto left_mv = Translate(-r - box_width, 0, 0) * Scale(r, r, r);
  auto right_mv = Translate(r + box_width, 0, 0) * Scale(r, r, r);
  auto bottom_mv = Translate(0.0, -r - box_width, 0) * Scale(r, r, r);
  auto top_mv = Translate(0.0, r + box_width, 0) * Scale(r, r, r);

  auto gold_spec = vec4(1.0, 0.5, 0.1, 1.0);
  auto gold_diff = vec4(1.0, 1.0, 1.0, 1.0);

  auto iron_diff = vec4(1.0, 0.9, 0.9, 1.0);
  auto iron_spec = vec4(1.0, 1.0, 1.0, 1.0);

  auto gl_mesh = make_shared<GLMesh>(ref(sphere_mesh));
  gl_mesh->initialize_buffers(
      GLuint(glGetAttribLocation(program, "vPosition")),
      GLuint(glGetAttribLocation(program, "vNormal")),
      GLuint(glGetAttribLocation(program, "vTexCoord")));

  r = 0.25;
  auto mv =
      Angel::Translate(0.6f, -box_width + r, -0.4f) * Angel::Scale(r, r, r);
  auto gold_ball = make_shared<UnitSphere>(Material::gold(), mv, gl_mesh);
  gold_ball->name = "gold_ball";
  scene.objects.push_back(gold_ball);

  r = 0.3;
  mv = Translate(0.1, -box_width + r, 0.2) * Scale(r, r, r);

  auto clear_ball = make_shared<UnitSphere>(Material::glass(), mv, gl_mesh);

  clear_ball->name = "clear_ball";
  scene.objects.push_back(clear_ball);

  auto plane =
      std::make_shared<UnitSphere>(Material::wall_white(), back_mv, gl_mesh);
  plane->name = "back";
  scene.objects.push_back(plane);

  // floor/ceil

  plane =
      std::make_shared<UnitSphere>(Material::wall_white(), bottom_mv, gl_mesh);
  plane->name = "bottom";
  scene.objects.push_back(plane);

  plane = std::make_shared<UnitSphere>(Material::wall_white(), top_mv, gl_mesh);
  plane->name = "top";
  scene.objects.push_back(plane);

  // side walls

  plane = std::make_shared<UnitSphere>(Material::wall_a(), left_mv, gl_mesh);
  plane->name = "left";
  scene.objects.push_back(plane);

  plane = std::make_shared<UnitSphere>(Material::wall_b(), right_mv, gl_mesh);
  plane->name = "right";
  scene.objects.push_back(plane);

  scene.camera_modelview = model_view;

  auto lc = LightColor();
  lc.ambient_color = vec4(0.5, 0.5, 0.50, 1.0);
  lc.diffuse_color = vec4(0.5, 0.5, 0.50, 1.0);
  lc.specular_color = vec4(0.5, 0.5, 0.50, 1.0);

  auto light_position = vec4(0.0, box_width - 0.4, -0.5, 1.0);

  scene.light_colors.push_back(lc);
  scene.light_locations.push_back(light_position);

  auto dir_light_color = LightColor();
  auto dir_light_loc = vec4(0.0, 0.5, 4.0, 0.0);

  dir_light_color.ambient_color = vec4(0.5, 0.5, 0.5, 1.0);
  dir_light_color.diffuse_color = vec4(0.5, 0.5, 0.5, 1.0);
  dir_light_color.specular_color = vec4(0.5, 0.5, 0.5, 1.0);

  scene.light_colors.push_back(dir_light_color);
  scene.light_locations.push_back(dir_light_loc);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //  Generate tha model-view matrixn

  glUseProgram(program);

  mat4 track_ball =
      mat4(curmat[0][0], curmat[1][0], curmat[2][0], curmat[3][0], curmat[0][1],
           curmat[1][1], curmat[2][1], curmat[3][1], curmat[0][2], curmat[1][2],
           curmat[2][2], curmat[3][2], curmat[0][3], curmat[1][3], curmat[2][3],
           curmat[3][3]);

  vec4 cam_position = vec4(0.0, 0.0, 3.0, 1.0);

  model_view = Translate(-cam_position) *                    // Move Camera Back
               Translate(ortho_x, ortho_y, 0.0) *            // Pan Camera
               track_ball *                                  // Rotate Camera
               Scale(scalefactor, scalefactor, scalefactor); // Scale

  scene.camera_modelview = model_view;

  using namespace std;

  auto n_lights = int(std::min(scene.n_lights(), max_lights));

  glUniform4fv(light_unifs.light_locations, n_lights,
               &scene.light_locations[0].x);
  glUniform1i(light_unifs.n_lights, n_lights);

  for (auto const &obj : scene.objects) {

    if (!(obj->target && sls::TargetOpenGL)) {
      continue;
    }

    // setup color info
    auto const &mtl = obj->material;
    auto k_specular = (mtl.k_specular + mtl.k_reflective);

    auto ambient = vector<vec4>();
    auto diffuse = vector<vec4>();
    auto specular = vector<vec4>();
    for (auto i = 0; i < n_lights; ++i) {
      auto const &l = scene.light_colors[i];

      ambient.push_back(mtl.k_ambient * mtl.ambient * l.ambient_color);
      diffuse.push_back(mtl.k_diffuse * mtl.color * l.diffuse_color);

      specular.push_back(k_specular * mtl.specular * l.specular_color);
    }

    // indicates translucency
    diffuse[0].w = sls::clamp(1.0 - mtl.k_transmittance, 0.2, 1.0);

    glUniform4fv(light_unifs.ambient_prods, n_lights,
                 reinterpret_cast<float const *>(&ambient[0]));
    glUniform4fv(light_unifs.diffuse_prods, n_lights,
                 reinterpret_cast<float const *>(&diffuse[0]));
    glUniform4fv(light_unifs.specular_prods, n_lights,
                 reinterpret_cast<float const *>(&specular[0]));

    glUniform1f(light_unifs.shininess, mtl.shininess);

    auto mv_object = model_view * obj->modelview();
    glUniformMatrix4fv(ModelViewEarth, 1, GL_TRUE, mv_object);
    glUniformMatrix4fv(ModelViewLight, 1, GL_TRUE, model_view);

    glUniformMatrix4fv(NormalMatrix, 1, GL_TRUE, transpose(invert(mv_object)));

    if (obj->mesh && obj->mesh->initialized) {
      obj->mesh->draw(vPosition, vNormal, vTexCoord);
    }
  }

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void mouse(GLFWwindow *window, int button, int action, int mods) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  if (!rt_flags.is_raytracing) {
    if (button == GLFW_PRESS) {
      moving = scaling = panning = 0;
      std::cout << x << "\t\t" << y << "\n";
      auto ray = findRay(x, y, window_width, window_height);
      castRayDebug(ray.start, ray.dir);
      return;
    }
    bool shift_on = mods & GLFW_MOD_SHIFT;
    bool alt_on = mods & GLFW_MOD_ALT;
    if (shift_on) {
      scaling = 1;
    } else if (alt_on) {
      panning = 1;
    } else {
      moving = 1;
      trackball(lastquat, 0, 0, 0, 0);
    }

    beginx = x;
    beginy = y;
  }

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void motion(GLFWwindow *window, double xpos, double ypos) {
  auto x = int(xpos);
  auto y = int(ypos);
  int W = 0;
  int H = 0;

  glfwGetWindowSize(window, &W, &H);

  float dx = (beginx - x) / float(W);
  float dy = (y - beginy) / float(H);

  if (panning) {
    ortho_x += dx;
    ortho_y += dy;

    beginx = x;
    beginy = y;
    return;
  } else if (scaling) {
    scalefactor *= (1.0f + dx);

    beginx = x;
    beginy = y;
    return;
  } else if (moving) {
    trackball(lastquat, (2.0f * beginx - W) / W, (H - 2.0f * beginy) / H,
              (2.0f * x - W) / W, (H - 2.0f * y) / H);

    add_quats(lastquat, curquat, curquat);
    build_rotmatrix(curmat, curquat);

    beginx = x;
    beginy = y;
    return;
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void keyboard(unsigned char key, int x, int y) {
  using namespace std;
  switch (key) {

  case 'l':
  case 'L': {
    // ask for new light position in console
    float x = 0.0, y = 0.0, z = 0.0, w = 0.0;

    cout << "enter 4 float values for the new light position:\n"
         << "x:\n";
    cin >> x;

    cout << "y:\n";
    cin >> y;

    cout << "z:\n";
    cin >> z;

    cout << "w:\n";
    cin >> w;

    w = nearlyEqual(w, 0.0, 1e-7) ? 0.f : 1.f;

    auto pos = vec4(x, y, z, w);
    if (scene.light_locations.size() == 0) {
      scene.light_locations.push_back(pos);
    } else {
      scene.light_locations[0] = pos;
    }

    cout << "light location: " << pos << "\n";

    break;
  }

  case 033: // Escape Key
  case 'q':
  case 'Q':
    if (rt_flags.thread.joinable()) {
      rt_flags.signal_quit_raytracing = true;
      rt_flags.thread.detach();
    }
    exit(EXIT_SUCCESS);
    break;
  case ' ': {
    render_line = !render_line;
    if (render_line) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    break;
  }

  case 'r': {
    if (!rt_flags.is_raytracing) {
      // set bind_viewport
      bind_viewport(nullptr);

      auto cf = RTConfig();

      rt_flags.thread = std::thread(rayTrace, 100, cf);
    } else {
      rt_flags.signal_quit_raytracing = true;
      cout << "raytracing in progress: will stop at end of next sample\n";
    }
    break;
  }
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void reshape(GLFWwindow *window, int width, int height) {
  window_height = height;
  window_width = width;

  glViewport(0, 0, width, height);

  GLfloat aspect = GLfloat(width) / height;
  projection = Perspective(45.0, aspect, 1.0, 50);

  glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void timer(int value) {
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
int main(int argc, char **argv) {
  app_args = sls::parse_args(argc, const_cast<char const **>(argv));
  main_id = std::this_thread::get_id();

  rt_flags.is_raytracing = false;
  rt_flags.signal_quit_raytracing = false;

  if (app_args.argv.size() > 1) {
    out_file_name = app_args.argv[1];
  } else {
    out_file_name = "output.png";
  }

  if(!glfwInit())
  {
    std::cerr << "Failed to initialize GLFW context" << std::endl;

    exit(EXIT_FAILURE);
  }
  atexit([]()
  {
    char c;
#ifdef _WIN32
    std::cout << "press a button" << std::endl;
    std::cin >> c;
#endif
    glfwTerminate();
  });

  
  
  WINDOW = glfwCreateWindow(800, 600, "raytracer", nullptr, nullptr);
  glfwMakeContextCurrent(WINDOW);
  if(!WINDOW)
  {
    std::cerr << "could not create window" << std::endl;
  }
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
  {
    std::cerr << "Failed to initialize GLAD context" << std::endl;
    exit(EXIT_FAILURE);
  }

  init();

  glfwSetWindowSizeCallback(WINDOW, reshape);
  glfwSetMouseButtonCallback(WINDOW, mouse);
  glfwSetCursorPosCallback(WINDOW, motion);

  while(!glfwWindowShouldClose(WINDOW))
  {
    glfwPollEvents();
    display();
    glfwSwapBuffers(WINDOW);
  }

  glfwDestroyWindow(WINDOW);

  

  return 0;
}
