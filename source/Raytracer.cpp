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

#include <functional>

//---------------------------------type aliases---------------------------------------
using color4 = Angel::vec4;
using point4 = Angel::vec4;

//---------------------------------app file info---------------------------------------

static sls::CommandLineArgs app_args;
static std::string out_file_name;

//---------------------------------opengl info---------------------------------------
int window_width, window_height;

bool render_line;
Mesh mesh;
GLuint buffer_object;

GLuint vPosition, vNormal, vTexCoord;

GLuint program;

// Model-view and projection matrices uniform location
GLuint ModelViewEarth, ModelViewLight, NormalMatrix, Projection;

mat4 projection;
mat4 model_view;

//---------------------------------raytracer scene----------------------------------

sls::Scene scene;


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

  y = window_height - y;

  GLdouble modelViewMatrix[16];
  GLdouble projectionMatrix[16];
  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

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

  std::vector<vec4> result(2);

  return sls::Ray{ray_origin, ray_dir};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void castRayDebug(vec4 p0, vec4 dir)
{
  double t = raySphereIntersection(p0, dir, 0, (Angel::vec4(0)));

  if (t > 0) {
    vec4 temp = p0 + t * dir;
    vec3 temp_3 = vec3(temp.x, temp.y, temp.z);
    std::cout << p0 + t * dir << "\t\t" << length(temp_3) << "\n";
  } else {
    std::cout << t << "\n";
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

vec4 castRay(vec4 p0, vec4 dir, size_t depth = 0, size_t max_depth = 1)
{
  //castRayDebug(p0, dir);
  auto color = vec4();
  auto obj = scene.objects[0];

  double t = raySphereIntersection(p0, dir, 0, (Angel::vec4(0)));

  auto clear_color = vec4(0.0, 0.0, 0.0, 0.0);
  if (t < 1) {
    color = clear_color;
  } else {
    auto temp = p0 + t * dir;
    color = vec4(fabs(temp.x), fabs(temp.y), fabs(temp.z), 1.0);
  }

  for (auto i = 0; i < 1000; ++i) {
    auto work = sqrt(10 * 50 + log(40));
  }

  return color;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void rayTrace()
{
  using namespace std;

  using namespace sls;


  auto width = window_width;
  auto height = window_height;

  // performance counter

  auto buffer =
      vector<uint8_t>(width * height * 4);

  using loop_pair_t = pair<size_t, vector<vec4>>;


  auto n_threads = 10;
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
      res.ray = findRay(i, j, width, height);
      res.color = castRay(res.ray.start, res.ray.dir);


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

          i.color = castRay(i.ray.start, i.ray.dir);
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
  mesh.makeSubdivisionSphere(8);
  //mesh.loadOBJ((source_path + "stanford_dragon/drag.obj").c_str());

  // Create a vertex array object
  GLuint vao;
  glGenVertexArraysAPPLE(1, &vao);
  glBindVertexArrayAPPLE(vao);


  // Create and initialize a buffer object
  glGenBuffers(1, &buffer_object);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_object);
  unsigned int vertices_bytes = mesh.vertices.size() * sizeof(vec4);
  unsigned int normals_bytes = mesh.normals.size() * sizeof(vec3);
  unsigned int uv_bytes = mesh.uvs.size() * sizeof(vec2);

  glBufferData(GL_ARRAY_BUFFER, vertices_bytes + normals_bytes + uv_bytes, NULL,
               GL_STATIC_DRAW);
  unsigned int offset = 0;
  glBufferSubData(GL_ARRAY_BUFFER, offset, vertices_bytes, &mesh.vertices[0]);
  offset += vertices_bytes;
  glBufferSubData(GL_ARRAY_BUFFER, offset, normals_bytes, &mesh.normals[0]);
  offset += normals_bytes;
  glBufferSubData(GL_ARRAY_BUFFER, offset, uv_bytes, &mesh.uvs[0]);

  std::string vshader = source_path + "/shaders/vshading_example.glsl";
  std::string fshader = source_path + "/shaders/fshading_example.glsl";

  program = InitShader(vshader.c_str(), fshader.c_str());
  // Load shaders and use the resulting shader program
  glUseProgram(program);

  // set up vertex arrays
  vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);

  vNormal = glGetAttribLocation(program, "vNormal");
  glEnableVertexAttribArray(vNormal);

  vTexCoord = glGetAttribLocation(program, "vTexCoord");
  glEnableVertexAttribArray(vTexCoord);

  scene = sls::setup_scene();


  // Initialize shader lighting parameters
  auto light_position = scene.light_locations[0];

  auto light_color = scene.ambient_colors[0];

  color4 material_ambient(0.0, 0.0, 0.0, 0.0);
  color4 material_diffuse(1.0, 0.8, 0.8, 0.2);
  color4 material_specular(1.0, 1.0, 1.0, 0.2);

  float material_shininess = 100;

  color4 ambient_product = light_color * material_ambient;
  color4 diffuse_product = light_color * material_diffuse;
  color4 specular_product = light_color * material_specular;

  glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1,
               ambient_product);
  glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1,
               diffuse_product);
  glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1,
               specular_product);
  glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1,
               light_position);
  glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);

  // Retrieve transformation uniform variable locations
  ModelViewEarth = glGetUniformLocation(program, "ModelViewEarth");
  ModelViewLight = glGetUniformLocation(program, "ModelViewLight");
  NormalMatrix = glGetUniformLocation(program, "NormalMatrix");
  Projection = glGetUniformLocation(program, "Projection");

  glUniform1i(glGetUniformLocation(program, "texture0"), 0);
  glUniform1i(glGetUniformLocation(program, "texture1"), 1);

  glEnable(GL_DEPTH_TEST);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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

  glBindBuffer(GL_ARRAY_BUFFER, buffer_object);
  glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
  glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(mesh.vertices.size() * sizeof(vec4)));
  glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(mesh.vertices.size() * sizeof(vec4) +
                                      mesh.normals.size() * sizeof(vec3)));

  glUniformMatrix4fv(ModelViewEarth, 1, GL_TRUE, model_view);
  glUniformMatrix4fv(ModelViewLight, 1, GL_TRUE, model_view);

  glUniformMatrix4fv(NormalMatrix, 1, GL_TRUE, transpose(invert(model_view)));

  glPointSize(5);
  glDrawArrays(GL_TRIANGLES, 0, mesh.vertices.size());

  glutSwapBuffers();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
void mouse(GLint button, GLint state, GLint x, GLint y)
{
  using namespace std;
  if (state == GLUT_UP) {
    moving = scaling = panning = 0;

    auto ray = findRay(x, y, 0, 0);

    cout << x << "\t\t" << y << "\n";
    cout << "first " << ray.start << "\t\t" << ray.dir << "\n\n";

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
    case 033: // Escape Key
    case 'q':
    case 'Q':
      exit(EXIT_SUCCESS);
      break;
    case ' ':
      render_line = !render_line;
      if (render_line) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
      break;
    case 'r': {
      fprintf(stderr, "raytracing\n");
      auto rt_timer = sls::timeit(rayTrace);
      cout
      << "\nperformed in "
      << chrono::duration_cast<chrono::milliseconds>(rt_timer).count() <<
      "ms\n";
    }
      break;
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
  projection = Perspective(45.0, aspect, 1.0, 5.0);

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
