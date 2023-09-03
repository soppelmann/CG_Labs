#include "CelestialBody.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <stack>

#include "core/Log.h"
#include "core/helpers.hpp"

CelestialBody::CelestialBody(bonobo::mesh_data const &shape,
                             GLuint const *program, GLuint diffuse_texture_id) {
  _body.node.set_geometry(shape);
  _body.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
  _body.node.set_program(program);
}


float etime;
glm::mat3 pworld;

glm::mat4 CelestialBody::render(std::chrono::microseconds elapsed_time,
                                glm::mat4 const &view_projection,
                                glm::mat4 const &parent_transform,
                                bool show_basis) {
  // Convert the duration from microseconds to seconds.
  auto const elapsed_time_s =
      std::chrono::duration<float>(elapsed_time).count();
  // If a different ratio was needed, for example a duration in
  // milliseconds, the following would have been used:
  // auto const elapsed_time_ms = std::chrono::duration<float,
  // std::milli>(elapsed_time).count();

  //_body.spin.rotation_angle = -glm::half_pi<float>() / 2.0f;

  _body.spin.rotation_angle += _body.spin.speed * elapsed_time_s;
  _body.orbit.rotation_angle += _body.orbit.speed * elapsed_time_s;

  // glm::mat4 world = parent_transform;
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), _body.scale);

  glm::mat4 R1 = glm::rotate(glm::mat4(1.0f), _body.spin.rotation_angle,
                             glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 R2 = glm::rotate(glm::mat4(1.0f), _body.spin.axial_tilt,
                             glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 tworld = glm::translate(glm::mat4(1.0f),
                                    glm::vec3(_body.orbit.radius, 0.0f, 0.0f));

  glm::mat4 R1o = glm::rotate(glm::mat4(1.0f), _body.orbit.rotation_angle,
                              glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 nR1o = glm::rotate(glm::mat4(1.0f), -_body.orbit.rotation_angle,
                               glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 R2o = glm::rotate(glm::mat4(1.0f), _body.orbit.inclination,
                              glm::vec3(0.0f, 0.0f, 1.0f));

  glm::mat4 world =
      parent_transform * R1o * tworld * nR1o * R2o * scale * R2 * R1;

  if (show_basis) {
    bonobo::renderBasis(1.0f, 2.0f, view_projection, world);
  }


  // Let us follow planets, not working
    glm::mat3 pscale = glm::scale(glm::mat4(1.0f), _body.scale);

  glm::mat3 pR1 = glm::rotate(glm::mat4(1.0f), _body.spin.rotation_angle,
                             glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat3 pR2 = glm::rotate(glm::mat4(1.0f), _body.spin.axial_tilt,
                             glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat3 ptworld = glm::translate(glm::mat4(1.0f),
                                    glm::vec3(_body.orbit.radius, 0.0f, 0.0f));

  glm::mat3 pR1o = glm::rotate(glm::mat4(1.0f), _body.orbit.rotation_angle,
                              glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat3 pnR1o = glm::rotate(glm::mat4(1.0f), -_body.orbit.rotation_angle,
                               glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat3 pR2o = glm::rotate(glm::mat4(1.0f), _body.orbit.inclination,
                              glm::vec3(0.0f, 0.0f, 1.0f));

  //pworld = pR1o * ptworld * pnR1o * pR2o * pscale * pR2 * pR1;
  pworld = pR1o * ptworld * pR2o * pR2;
  //pworld = pR2o * ptworld * pR1o * pscale;



  // Note: The second argument of `node::render()` is supposed to be the
  // parent transform of the node, not the whole world matrix, as the
  // node internally manages its local transforms. However in our case we
  // manage all the local transforms ourselves, so the internal transform
  // of the node is just the identity matrix and we can forward the whole
  // world matrix.
  _body.node.render(view_projection, world);

  etime = _body.orbit.radius;
  //etime = _body.orbit.rotation_angle;

  return parent_transform * R1o * tworld * R2o * R2;
}

void CelestialBody::add_child(CelestialBody *child) {  _children.push_back(child);
}

std::vector<CelestialBody *> const &CelestialBody::get_children() const {
  return _children;
}

glm::vec3 CelestialBody::get_ploc() {
  //glm::vec3 svec = glm::vec3(2.0f + etime * 0.05f, etime * 0.00f, etime * 0.05f );
  //glm::vec3 svec = glm::vec3(2.0f - etime, 0 , etime);
  glm::vec3 svec = glm::vec3(1.0f) * pworld;// * etime;
  return svec;
};

void CelestialBody::set_orbit(OrbitConfiguration const &configuration) {
  _body.orbit.radius = configuration.radius;
  _body.orbit.inclination = configuration.inclination;
  _body.orbit.speed = configuration.speed;
  _body.orbit.rotation_angle = 0.0f;
}

void CelestialBody::set_scale(glm::vec3 const &scale) { _body.scale = scale; }

void CelestialBody::set_spin(SpinConfiguration const &configuration) {
  _body.spin.axial_tilt = configuration.axial_tilt;
  _body.spin.speed = configuration.speed;
  _body.spin.rotation_angle = 0.0f;
}

void CelestialBody::set_ring(bonobo::mesh_data const &shape,
                             GLuint const *program, GLuint diffuse_texture_id,
                             glm::vec2 const &scale) {
  _ring.node.set_geometry(shape);
  _ring.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
  _ring.node.set_program(program);

  _ring.scale = scale;

  _ring.is_set = true;
}
