//
//  Display a rotating cube with lighting
//
//  Light and material properties are sent to the shader as uniform
//    variables.  Vertex positions and normals are sent after each
//    rotation.

#include "Angel.h"
#include "SourcePath.h"
#include "Trackball.h"
#include "ObjMesh.h"

#include "common-math.h"

#include "image-utils.h"
#include "renderer.h"

#include "async-tools.h"

#include <string>
#include <functional>

void setup_scene(vec4 const &material_diffuse,
                 vec4 const &material_ambient, vec4 const &material_specular);

vec4 castRay(vec4 p0, vec4 dir, size_t depth, size_t max_depth = 10, std::shared_ptr<sls::SceneObject> obj = nullptr);

vec4 castRay(sls::Ray const &ray, size_t depth, size_t max_depth = 10, std::shared_ptr<sls::SceneObject> obj = nullptr)
{
  return castRay(ray.start, ray.dir, depth, max_depth, obj);
}



//---------------------------------type aliases---------------------------------------
using color4 = Angel::vec4;
using point4 = Angel::vec4;

//---------------------------------app file info---------------------------------------

static sls::CommandLineArgs app_args;
static std::string out_file_name;

//---------------------------------opengl info---------------------------------------
int window_width, window_height;

bool render_line;
Mesh sphere_mesh;
GLuint buffer_object;

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


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
sls::Ray findRay(GLdouble x,
                 GLdouble y,
                 int width,
                 int height)
{

  y = height - y;
  GLdouble modelViewMatrix[16];
  GLdouble projectionMatrix[16];
  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
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
  gluUnProject(x, y, 0.0, modelViewMatrix, projectionMatrix,
               viewport, &nearPlaneLocation[0], &nearPlaneLocation[1],
               &nearPlaneLocation[2]);

  GLdouble farPlaneLocation[3];
  gluUnProject(x, y, 1.0, modelViewMatrix, projectionMatrix,
               viewport, &farPlaneLocation[0], &farPlaneLocation[1],
               &farPlaneLocation[2]);

  vec4 ray_origin = vec4(nearPlaneLocation[0], nearPlaneLocation[1], nearPlaneLocation[2], 1.0);
  vec3 temp = vec3(farPlaneLocation[0] - nearPlaneLocation[0],
                   farPlaneLocation[1] - nearPlaneLocation[1],
                   farPlaneLocation[2] - nearPlaneLocation[2]);
  temp = normalize(temp);
  vec4 ray_dir = vec4(temp.x, temp.y, temp.z, 0.0);


  return sls::Ray(ray_origin, ray_dir);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void castRayDebug(vec4 p0, vec4 dir)
{
  for (auto i: scene.objects) {
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

vec4 castRay(vec4 p0, vec4 dir, size_t depth, size_t max_depth, std::shared_ptr<sls::SceneObject> current_object)
{

  //castRayDebug(p0, dir);

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
      } else if (intersection.t < nearest_hit.inter.t){
        nearest_hit = hit;
      }
    }
  }


  if (hit_found) {
r
    auto const &obj = nearest_hit.obj;
    auto const &intersection = nearest_hit.inter;


    auto t = intersection.t;
    auto normal = normalize(intersection.normal);
    auto obj_name = obj->name;


    auto hit_viewspace = nearest_hit.hit_point;

    auto reflection = vec4(0.0, 0.0, 0.0, 0.0);
    auto transmitted = vec4(0.0, 0.0, 0.0, 0.0);

    auto const &mtl = obj->material;
    if (mtl.k_reflective > 0.0 || mtl.k_specular > 0.0) { // non-zero reflectivity
      auto reflect_dir = normalize(-reflect(dir, normalize(vec4(normal, 0.0))));
      reflection = castRay(hit_viewspace, reflect_dir, depth + 1, max_depth, obj);
    }

    if (mtl.k_transmittance > 1e-7) { // non-zero transmittance
      auto inside_obj = dot(dir, normal) < 0;
      auto outer_ior = inside_obj ? scene.space_k_refraction : obj->material.k_refraction;
      auto inner_ior = inside_obj ? obj->material.k_refraction : scene.space_k_refraction;


      auto refraction_ray = get_refraction_ray(obj,
                                               xyz(hit_viewspace),
                                               xyz(dir),
                                               normal,
                                               inner_ior / outer_ior);
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


void rayTrace()
{
  using namespace std;

  using namespace sls;

#if 1
  auto width = window_width;
  auto height = window_height;
#else
  auto width = 1920;
  auto height = 1080;

#endif

  auto buffer =
      vector<uint8_t>(width * height * 4);

  using loop_pair_t = pair<size_t, vector<vec4>>;

  // params
  auto const n_threads = 20;
  auto const max_rt_depth = 3;


  auto len = width * height;
  auto step = len / n_threads;

  auto counter = 0;
  auto work_unit = vector<rt_data>();
  auto work_units = vector<decltype(work_unit)>();

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

  auto results = vector<future<vector<rt_data>>>();

  cout << "work units size " << work_units.size() << endl;
  for (auto &unit: work_units) {
    results.push_back(
        raycast_async([](auto &i) {
          i.color = castRay(i.rays.start,
                            i.rays.dir,
                            0, max_rt_depth, nullptr);
          return i;
        }, unit));
  }

  for (auto &fut: results) {
    auto unit = fut.get();
    for (auto const &data: unit) {

      auto idx = data.j * width + data.i;
      auto buff = &buffer[idx * 4];

      buff[0] = static_cast<uint8_t>(data.color.x * 255);
      buff[1] = static_cast<uint8_t>(data.color.y * 255);
      buff[2] = static_cast<uint8_t>(data.color.z * 255);
      buff[3] = static_cast<uint8_t>(data.color.w * 255);
    }
  }


  write_image(out_file_name, &buffer[0], width, height, 4);

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void init()
{
  sphere_mesh.makeSubdivisionSphere(10);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Create a vertex array object
  GLuint vao;
  glGenVertexArraysAPPLE(1, &vao);
  glBindVertexArrayAPPLE(vao);

  // Create and initialize a buffer object
  glGenBuffers(1, &buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_object);
  unsigned int vertices_bytes = sphere_mesh.vertices.size() * sizeof(vec4);
  unsigned int normals_bytes = sphere_mesh.normals.size() * sizeof(vec3);
  unsigned int uv_bytes = sphere_mesh.uvs.size() * sizeof(vec2);

  glBufferData(GL_ARRAY_BUFFER, vertices_bytes + normals_bytes + uv_bytes, NULL,
               GL_STATIC_DRAW);
  unsigned int offset = 0;
  glBufferSubData(GL_ARRAY_BUFFER, offset, vertices_bytes, &sphere_mesh.vertices[0]);
  offset += vertices_bytes;
  glBufferSubData(GL_ARRAY_BUFFER, offset, normals_bytes, &sphere_mesh.normals[0]);
  offset += normals_bytes;
  glBufferSubData(GL_ARRAY_BUFFER, offset, uv_bytes, &sphere_mesh.uvs[0]);

  std::string vshader = source_path + "/shaders/vshading_example.glsl";
  std::string fshader = source_path + "/shaders/fshading_example.glsl";

  program = InitShader(vshader.c_str(), fshader.c_str(), nullptr);
  // Load shaders and use the resulting shader program
  glUseProgram(program);

  // set up vertex arrays
  vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);

  vNormal = glGetAttribLocation(program, "vNormal");
  glEnableVertexAttribArray(vNormal);

  vTexCoord = glGetAttribLocation(program, "vTexCoord");
  glEnableVertexAttribArray(vTexCoord);


  color4 light_ambient(1.0, 1.0, 1.0, 1.0);
  color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
  color4 light_specular(1.0, 1.0, 1.0, 1.0);

  color4 material_ambient(1.0, 1.0, 1.0, 1.0);
  color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
  color4 material_specular(1.0, 1.0, 1.0, 1.0);

  color4 ambient_product = light_ambient * material_ambient;
  color4 diffuse_product = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;

  light_unifs.ambient_prods = GLuint(glGetUniformLocation(program, "AmbientProducts"));
  light_unifs.diffuse_prods = GLuint(glGetUniformLocation(program, "DiffuseProducts"));
  light_unifs.specular_prods = GLuint(glGetUniformLocation(program, "SpecularProducts"));

  light_unifs.light_locations = GLuint(glGetUniformLocation(program, "LightPositions"));
  auto shininess = glGetUniformLocation(program, "Shininess");
  light_unifs.shininess = GLuint(shininess >= 0 ? GLuint(shininess) : 0);

  auto n_lights = glGetUniformLocation(program, "n_lights");
  light_unifs.n_lights = n_lights >= 0 ?
                         GLuint(n_lights) :
                         0;




  // Retrieve transformation uniform variable locations
  ModelViewEarth = glGetUniformLocation(program, "ModelViewEarth");
  ModelViewLight = glGetUniformLocation(program, "ModelViewLight");
  NormalMatrix = glGetUniformLocation(program, "NormalMatrix");
  Projection = glGetUniformLocation(program, "Projection");

  glUniform1i(glGetUniformLocation(program, "texture0"), 0);
  glUniform1i(glGetUniformLocation(program, "texture1"), 1);

  glEnable(GL_DEPTH_TEST);

  glShadeModel(GL_SMOOTH);

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


  setup_scene(material_diffuse,
              material_ambient, material_specular);


}


void setup_scene(vec4 const &material_diffuse, vec4 const &material_ambient, vec4 const &material_specular)
{
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

  auto gl_mesh = GLMesh();
  gl_mesh.vbo = buffer_object;
  gl_mesh.mesh = sphere_mesh;

  r = 0.25;
  auto mv =
      Angel::Translate(0.6f, -box_width + r, -0.4f) * Angel::Scale(r, r, r);
  auto gold_ball =
      make_shared<UnitSphere>(Material::gold(), mv, &gl_mesh);
  gold_ball->name = "gold_ball";
  scene.objects.push_back(gold_ball);


  r = 0.3;
  mv = Translate(0.1, -box_width + r, 0.2) * Scale(r, r, r);

  auto clear_ball =
      make_shared<UnitSphere>(Material::glass(), mv, &gl_mesh);
  scene.objects.push_back(clear_ball);



  auto plane = std::make_shared<UnitSphere>(Material::wall_white(),
                                            back_mv, &gl_mesh);
  plane->name = "back";
  scene.objects.push_back(plane);

  // floor/ceil

  plane = std::make_shared<UnitSphere>(Material::wall_white(),
                                       bottom_mv, &gl_mesh);
  plane->name = "bottom";
  scene.objects.push_back(plane);

  plane = std::make_shared<UnitSphere>(Material::wall_white(), top_mv, &gl_mesh);
  plane->name = "top";
  scene.objects.push_back(plane);

  // side walls



  plane = std::make_shared<UnitSphere>(Material::wall_a(), left_mv, &gl_mesh);
  plane->name = "left";
  scene.objects.push_back(plane);


  plane = std::make_shared<UnitSphere>(Material::wall_b(), right_mv, &gl_mesh);
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
  auto dir_light_loc = vec4(0.0, 0.5, 100.0, 0.0);

  dir_light_color.ambient_color = vec4(0.5, 0.5, 0.5, 1.0);
  dir_light_color.diffuse_color = vec4(0.5, 0.5, 0.5, 1.0);
  dir_light_color.specular_color = vec4(0.5, 0.5, 0.5, 1.0);

  scene.light_colors.push_back(dir_light_color);
  scene.light_locations.push_back(dir_light_loc);


}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void display(void)
{
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
               reinterpret_cast<float const *>(&scene.light_locations[0]));
  glUniform1i(light_unifs.n_lights, n_lights);

  for (auto const &obj: scene.objects) {


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


      ambient.push_back(
          mtl.k_ambient * mtl.ambient * l.ambient_color);
      diffuse.push_back(
          mtl.k_diffuse * mtl.color * l.diffuse_color);


      specular.push_back(
          k_specular * mtl.specular * l.specular_color);


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


    if (obj->mesh) {
      obj->mesh->draw(vPosition, vNormal, vTexCoord);
    }

  }


  glutSwapBuffers();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void mouse(GLint button, GLint state, GLint x, GLint y)
{

  if (state == GLUT_UP) {
    moving = scaling = panning = 0;
    std::cout << x << "\t\t" << y << "\n";
    auto ray = findRay(x, y, window_width, window_height);
    castRayDebug(ray.start, ray.dir);
    glutPostRedisplay();
    return;
  }

  if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
    scaling = 1;
  } else if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
    panning = 1;
  } else {
    moving = 1;
    trackball(lastquat, 0, 0, 0, 0);
  }

  beginx = x;
  beginy = y;
  glutPostRedisplay();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void motion(GLint x, GLint y)
{

  int W = glutGet(GLUT_WINDOW_WIDTH);
  int H = glutGet(GLUT_WINDOW_HEIGHT);

  float dx = (beginx - x) / (float) W;
  float dy = (y - beginy) / (float) H;

  if (panning) {
    ortho_x += dx;
    ortho_y += dy;

    beginx = x;
    beginy = y;
    glutPostRedisplay();
    return;
  } else if (scaling) {
    scalefactor *= (1.0f + dx);

    beginx = x;
    beginy = y;
    glutPostRedisplay();
    return;
  } else if (moving) {
    trackball(lastquat,
              (2.0f * beginx - W) / W, (H - 2.0f * beginy) / H,
              (2.0f * x - W) / W, (H - 2.0f * y) / H);

    add_quats(lastquat, curquat, curquat);
    build_rotmatrix(curmat, curquat);

    beginx = x;
    beginy = y;
    glutPostRedisplay();
    return;
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void keyboard(unsigned char key, int x, int y)
{
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
      fprintf(stderr, "raytracing\n");
      auto rt_timer = sls::timeit(rayTrace);
      cout
      << "\nperformed in "
      << chrono::duration_cast<chrono::milliseconds>(rt_timer).count() <<
      "ms\n";
      break;
    }


  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void reshape(int width, int height)
{
  window_height = height;
  window_width = width;

  glViewport(0, 0, width, height);

  GLfloat aspect = GLfloat(width) / height;
  projection = Perspective(45.0, aspect, 1.0, 50);

  glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void timer(int value)
{
  glutTimerFunc(33, timer, 1);
  glutPostRedisplay();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
  app_args = sls::parse_args(argc, const_cast<char const **>(argv));

  if (app_args.argv.size() > 1) {
    out_file_name = app_args.argv[1];
  } else {
    out_file_name = "output.png";
  }

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(512, 512);
  glutCreateWindow("Raytracer");
  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  timer(1);

  glutMainLoop();
  return 0;
}
