/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 12/5/15, Steven
 * 
 **/
#ifndef RAYTRACER_SCENE_H
#define RAYTRACER_SCENE_H

#include "types.h"
#include <memory>
#include <vector>
#include "common/Angel.h"
#include "common/ObjMesh.h"

namespace sls {


struct GLMesh final {
  Mesh mesh;
  GLuint ibo;
  GLuint vbo;

  GLuint vao;

  void draw(GLuint vert_position, GLuint vert_normal, GLuint vert_texcoord)
  {


    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vert_position, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(vert_normal, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(mesh.vertices.size() * sizeof(vec4)));
    glVertexAttribPointer(vert_texcoord, 2, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(mesh.vertices.size() * sizeof(vec4) +
                                        mesh.normals.size() * sizeof(vec3)));

    glPointSize(5);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertices.size());
  }


};

/**
 * @brief Object for rendering in Ray tracer
 * @detail Uses a non-virtual destructor. Do not
 * inherrit
 */
struct SceneObject {

public:


  SceneObject(Material const &mtl = Material(), Angel::mat4 const &model_view = Angel::mat4(),
              GLMesh const *_mesh = nullptr)
      : material(mtl), mesh(nullptr)
  {
    set_modelview(model_view);
    if (_mesh) {
      mesh = std::make_unique<GLMesh>(std::ref(*_mesh));
    }
  }

  SceneObject(SceneObject const &cpy) :
      SceneObject(cpy.material, cpy.modelview_, cpy.mesh.get()) { }

  virtual
  ~SceneObject() { }

  Material material;
  std::unique_ptr<GLMesh> mesh;

  std::string name = "obj";

  virtual Intersection intersect(Ray const &ray) = 0;

  /**
   * @brief return intersection distance only
   */
  virtual double intersect_t(Ray const &ray)
  {
    return intersect(ray).t;
  }


  //---------------------------------matrix accessors---------------------------------------

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
};

struct UnitSphere : public SceneObject {

  UnitSphere(Material const &mtl, Angel::mat4 modelview = Angel::mat4(), GLMesh const *mesh = nullptr) :
      SceneObject(mtl,
                  modelview,
                  mesh) { }

  double radius = 1;

  virtual double intersect_t(Ray const &ray) override;

  Intersection intersect(Ray const &ray) override;
};

/**
 * @brief Describes an infinite area plane
 */
struct Plane : public SceneObject {

  Plane(Material const &mtl, Angel::mat4 modelview = Angel::mat4(), GLMesh const *mesh = nullptr) :
      SceneObject(mtl,
                  modelview,
                  mesh) { }

  Intersection intersect(Ray const &ray) override;
};

}
#endif //RAYTRACER_SCENE_H
