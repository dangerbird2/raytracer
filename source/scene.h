/**
 * @file ${FILE}
 * @brief
 * @license ${LICENSE}
 * Copyright (c) 12/5/15, Steven
 *
 **/
#ifndef RAYTRACER_SCENE_H
#define RAYTRACER_SCENE_H

#include "slsgl.h"
#include "slsgl.h"

#include "common-math.h"
#include "common/Angel.h"
#include "common/ObjMesh.h"
#include "types.h"
#include <memory>
#include <vector>

namespace sls {

enum RenderTarget {
  TargetNone = 0,
  TargetRayTracer = 1 << 0,
  TargetOpenGL = 1 << 1,
  TargetDefault = TargetOpenGL | TargetRayTracer
};

struct GLMesh final {
  Mesh mesh;
  GLuint ibo;
  GLuint vbo;

  GLuint vao;

  bool initialized;

  GLMesh(Mesh const &mesh, GLuint ibo, GLuint vbo, GLuint vao)
      : mesh(mesh), ibo(ibo), vbo(vbo), vao(vao), initialized(false) {}

  GLMesh(Mesh const &mesh = {}) : mesh(mesh), initialized(false) {
    constexpr auto n = 2;
    GLuint buffers[n] = {};
    glGenBuffers(n, buffers);
    ibo = buffers[0];
    vbo = buffers[1];
    glGenVertexArraysAPPLE(1, &vao);
  }

  GLMesh(GLMesh &cpy) = delete;

  virtual ~GLMesh() {
    constexpr auto n = 2;
    GLuint buffers[n] = {ibo, vbo};
    glDeleteBuffers(n, buffers);
  }

  void initialize_buffers(GLuint vert_position, GLuint vert_normal,
                          GLuint vert_texcoord) {
    glBindVertexArrayAPPLE(vao);

    glEnableVertexAttribArray(vert_position);

    glEnableVertexAttribArray(vert_normal);

    glEnableVertexAttribArray(vert_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    auto vertices_bytes = mesh.vertices.size() * sizeof(vec4);
    auto normals_bytes = mesh.normals.size() * sizeof(vec3);
    auto uv_bytes = mesh.uvs.size() * sizeof(vec2);

    glBufferData(GL_ARRAY_BUFFER, vertices_bytes + normals_bytes + uv_bytes,
                 NULL, GL_STATIC_DRAW);
    unsigned int offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, vertices_bytes, &mesh.vertices[0]);
    offset += vertices_bytes;
    glBufferSubData(GL_ARRAY_BUFFER, offset, normals_bytes, &mesh.normals[0]);
    offset += normals_bytes;
    glBufferSubData(GL_ARRAY_BUFFER, offset, uv_bytes, &mesh.uvs[0]);

    initialized = true;
    glBindVertexArrayAPPLE(0);
  }

  void draw(GLuint vert_position, GLuint vert_normal, GLuint vert_texcoord) {
    if (initialized & 0) {
      glBindVertexArrayAPPLE(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(vert_position, 4, GL_FLOAT, GL_FALSE, 0,
                            BUFFER_OFFSET(0));
      glVertexAttribPointer(vert_normal, 3, GL_FLOAT, GL_FALSE, 0,
                            BUFFER_OFFSET(mesh.vertices.size() * sizeof(vec4)));
      glVertexAttribPointer(vert_texcoord, 2, GL_FLOAT, GL_FALSE, 0,
                            BUFFER_OFFSET(mesh.vertices.size() * sizeof(vec4) +
                                          mesh.normals.size() * sizeof(vec3)));

      glPointSize(5);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.vertices.size()));
      glBindVertexArrayAPPLE(0);
    }
  }
};

struct GLPackedMesh final {};

/**
 * @brief Object for rendering in Ray tracer
 * @detail Uses a non-virtual destructor. Do not
 * inherrit
 */
struct SceneObject {

public:
  //---------------------------------fields---------------------------------------
  Material material;
  std::shared_ptr<GLMesh> mesh;

  std::string name = "obj";
  RenderTarget target;

  //---------------------------------constructors---------------------------------------
  SceneObject(Material const &mtl = Material(),
              Angel::mat4 const &model_view = Angel::mat4(),
              std::shared_ptr<GLMesh> mesh = nullptr,
              RenderTarget target = TargetDefault)
      : material(mtl), mesh(mesh), target(target) {
    set_modelview(model_view);
  }

  SceneObject(SceneObject const &cpy)
      : SceneObject(cpy.material, cpy.modelview_, cpy.mesh) {}

  //---------------------------------methods---------------------------------------
  virtual ~SceneObject() {}

  virtual Intersection intersect(Ray const &ray) const = 0;

  virtual bool on_surface(vec3 const &point) const = 0;

  virtual bool inside(vec3 const &point) const = 0;

  virtual vec3 surface_normal(vec3 const &point) const = 0;

  virtual bool inside_ray(Ray const &ray) const {

    using namespace Angel;

    if (inside(xyz(ray.start))) {
      return true;
    } else {
      auto facing_normal =
          dot(xyz(ray.dir), surface_normal(xyz(ray.start))) > 0;
      return facing_normal;
    }
  };

  /**
   * @brief return intersection distance only
   */
  virtual double intersect_t(Ray const &ray) const { return intersect(ray).t; }

  //---------------------------------matrix
  //accessors---------------------------------------

  mat4 const &modelview() const;

  void set_modelview(mat4 const &model_view);

  mat4 const &modelview_inverse() const;

  mat4 const &normalview() const;

private:
  Angel::mat4 modelview_;
  Angel::mat4 modelview_inverse_;
  Angel::mat4 normalview_;
};

struct LightColor {
  Angel::vec4 ambient_color = vec4(1.0, 1.0, 1.0, 1.0);
  Angel::vec4 diffuse_color = vec4(0.01, 0.01, 0.01, 1.0);
  Angel::vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0);
};

struct Scene {

  Angel::mat4 camera_modelview;

  std::vector<LightColor> light_colors;
  std::vector<Angel::vec4> light_locations;

  std::vector<std::shared_ptr<SceneObject>> objects;

  float space_k_refraction = 1.000293; // air

  size_t n_lights() const {
    return std::min(light_colors.size(), light_locations.size());
  }
};

struct UnitSphere : public SceneObject {

  UnitSphere(Material const &mtl, Angel::mat4 modelview = Angel::mat4(),
             std::shared_ptr<GLMesh> mesh = nullptr)
      : SceneObject(mtl, modelview, mesh, TargetDefault) {}

  UnitSphere(UnitSphere const &cpy)
      : UnitSphere(cpy.material, cpy.modelview(), cpy.mesh) {
    radius = cpy.radius;
  }

  UnitSphere &operator=(UnitSphere const &cpy) {
    UnitSphere tmp(cpy);
    *this = std::move(tmp);
    return *this;
  }

  double radius = 1;

  virtual double intersect_t(Ray const &ray) const override;

  Intersection intersect(Ray const &ray) const override;

  virtual bool on_surface(vec3 const &point) const override;

  virtual bool inside(vec3 const &point) const override;

  virtual vec3 surface_normal(vec3 const &point) const override;
};

/**
 * @brief Describes an infinite area plane
 */
struct Plane : public SceneObject {

  Plane(Material const &mtl, Angel::mat4 modelview = Angel::mat4(),
        std::shared_ptr<GLMesh> mesh = nullptr)
      : SceneObject(mtl, modelview, mesh) {}

  Intersection intersect(Ray const &ray) const override;
};
}
#endif // RAYTRACER_SCENE_H
