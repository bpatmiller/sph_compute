#include "game.h"
#include <algorithm>
#include <glm/gtc/random.hpp>

void Game::create_sphere(float Radius, std::vector<glm::vec3> &s_vertices) {
  int Stacks = 8;
  int Slices = 8;
  s_vertices.clear();

  for (int i = 0; i <= Stacks; ++i) {
    float V = i / (float)Stacks;
    float phi = V * glm::pi<float>();
    for (int j = 0; j <= Slices; ++j) {
      float U = j / (float)Slices;
      float theta = U * (glm::pi<float>() * 2);
      float x = cosf(theta) * sinf(phi);
      float y = cosf(phi);
      float z = sinf(theta) * sinf(phi);
      s_vertices.push_back(glm::vec3(x, y, z) * Radius);
    }
  }

  for (int i = 0; i < Slices * Stacks + Slices; ++i) {
    sphere_indices.emplace_back(glm::uvec3(i, i + Slices + 1, i + Slices));
    sphere_indices.emplace_back(glm::uvec3(i + Slices + 1, i, i + 1));
  }
}

void Game::init() {
  // basic simulation settings
  simulation.h = 0.1f;
  simulation.box_scale = 2.0f;
  render_mode = 2;

  // 2400 particles
  // simulation.dimensions = glm::vec3(20, 15, 8);
  // PHYSICS_STEPS = 10;

  // 4800 particles
  // simulation.dimensions = glm::vec3(40, 15, 8);
  // PHYSICS_STEPS = 5;

  // 6000 particles
  simulation.dimensions = glm::vec3(30, 20, 10);
  PHYSICS_STEPS = 2;

  // 10,000 particles
  // simulation.dimensions = glm::vec3(25, 20, 20);
  // PHYSICS_STEPS = 1;

  // set camera focus
  focus = simulation.dimensions * simulation.h * simulation.box_scale * 0.25f;
  focus.y *= 0.5f;

  // compile programs
  pool_program = Program("src/shaders/pool.vs", "", "src/shaders/pool.fs", "");
  fluid_program =
      Program("src/shaders/particle.vs", "", "src/shaders/particle.fs", "");
  fluid_compute_dens = Program("", "", "", "src/shaders/sph_dens_pres.glsl");
  fluid_compute_force = Program("", "", "", "src/shaders/sph_force.glsl");
  fluid_integrate = Program("", "", "", "src/shaders/sph_integrate.glsl");
  fluid_compute_norm_vel = Program("", "", "", "src/shaders/sph_norm_vel.glsl");
  tex_quad_program =
      Program("src/shaders/ssao.vs", "", "src/shaders/ssao.fs", "");

  // init pool
  std::vector<glm::vec3> pool_vertices = {
      {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f},
      {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
  for (auto &v : pool_vertices) {
    v.x *= simulation.dimensions.x * simulation.h * simulation.box_scale * 0.5f;
    v.y *= simulation.dimensions.y * simulation.h * simulation.box_scale * 0.5f;
    v.z *= simulation.dimensions.z * simulation.h * simulation.box_scale * 0.5f;
  }
  pool_indices = {{0, 1, 2}, {1, 3, 2}, {4, 6, 5}, {5, 6, 7},
                  {0, 5, 1}, {0, 4, 5}, {2, 3, 7}, {2, 7, 6},
                  {3, 1, 5}, {3, 5, 7}, {0, 2, 6}, {0, 6, 4}};
  pool.setLayout({3}, false);
  pool.vb.set(pool_vertices);

  // init particles
  simulation.init();

  // init sphere, pass particle instances
  std::vector<glm::vec3> sphere_vertices;
  create_sphere(simulation.h * 0.25f, sphere_vertices);
  fluid.setLayout({3}, false);
  fluid.setLayout({3, 1, 3, 1, 3, 1, 3, 1, 3, 1}, true);
  fluid.vb.set(sphere_vertices);
  fluid.ib.set(simulation.particles);

  // particle SSBO for compute shader
  glGenBuffers(1, &fluid_ssbo_id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fluid_ssbo_id);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fluid.ib.id);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // spatial acceleration SSBO
  // glGenBuffers(1, &accel_ssbo_id);
  // glBindBuffer(GL_SHADER_STORAGE_BUFFER, accel_ssbo_id);
  // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, simulation.accel_vao.vb.id);
  // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // init tex / tex quad for SSAO
  glfwGetWindowSize(window, &window_width, &window_height);
  r_tex.create(window_width, window_height);
  // quad vao
  texquad.setLayout({3}, false);
  texquad.vb.set(tq_vertices);

  // init ssao kernel
  // FIXME remove this
  // from old implimentation
  // ssao_kernel.reserve(ssao_kernel_size);
  // for (uint i = 0; i < ssao_kernel_size; i++) {
  //   // generate points on hemisphere
  //   ssao_kernel[i] = glm::sphericalRand(1.0f);
  //   ssao_kernel[i].z = glm::abs(ssao_kernel[i].z);
  //   // scale within hemisphere
  //   float scale = float(i) / float(ssao_kernel_size);
  //   scale = glm::mix(0.1f, 1.0f, scale * scale);
  //   ssao_kernel[i] *= scale;
  // }

  // some gl settings
  glClearColor(0.85f, 0.85f, 0.85f, 0.85f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LESS);
  glCullFace(GL_BACK);
  glLineWidth(2.0f);
}

void Game::update() {
  // check for resize
  int old_w_w = window_width;
  int old_w_h = window_height;
  glfwGetWindowSize(window, &window_width, &window_height);
  if (old_w_w != window_width || old_w_h != window_height) {
    r_tex.changeSize(window_width, window_height);
  }

  // update camera pos
  eye =
      focus + glm::vec3(glm::mat4_cast(orientation) * glm::vec4(base_eye, 1.0));
  view_matrix = glm::lookAt(eye, focus, UP);
  projection_matrix = glm::perspective(
      glm::radians(60.0f), ((float)window_width) / window_height, 0.01f, 20.f);

  // handle mouse movement
  bool first = (mouse_pos_prev == glm::vec2(-1, -1));
  if (mouse_pressed) {
    if (!first) {
      mouse_diff = mouse_pos - mouse_pos_prev;
      update_camera();
    }
  } else {
    if (!first) {
      if (keyHeld[GLFW_KEY_T]) {
        attract_repel = 1.0f;
        mouse_ray_intersect();
      } else if (keyHeld[GLFW_KEY_Y]) {
        attract_repel = -1.0f;
        mouse_ray_intersect();
      } else {
        repulser = glm::vec3(-99, -99, -99);
      }
    }
  }
  mouse_pos_prev = mouse_pos;

  // handle keypress
  if (keyHeld[GLFW_KEY_W]) {
    if (base_eye.z > 0.2f) {
      base_eye.z -= 0.1f;
    }
  }
  if (keyHeld[GLFW_KEY_S]) {
    base_eye.z += 0.1f;
  }

  // if (keyHeld[GLFW_KEY_P]) {
  for (uint iteration = 0; iteration < PHYSICS_STEPS; iteration++) {
    // sort particles and poulate a map of index -> index pairs
    // simulation.sort_particles();
    // fluid.ib.update(simulation.particles, 0);

    // compute density and pressure
    fluid_compute_dens.use();
    fluid_compute_dens.setInt("particles_size", simulation.particles.size());
    fluid_compute_dens.setInt("num_cells", simulation.num_cells);
    fluid_compute_dens.setFloat("MASS", simulation.MASS);
    fluid_compute_dens.setFloat("GAS_CONST", simulation.GAS_CONST);
    fluid_compute_dens.setFloat("REST_DENS", simulation.REST_DENSITY);
    fluid_compute_dens.setFloat("h", simulation.h);
    glDispatchCompute(simulation.particles.size(), 1, 1);

    // compute normals and TODO smooth velocities
    fluid_compute_norm_vel.use();
    fluid_compute_norm_vel.setInt("particles_size",
                                  simulation.particles.size());
    fluid_compute_norm_vel.setInt("num_cells", simulation.num_cells);
    fluid_compute_norm_vel.setFloat("MASS", simulation.MASS);
    fluid_compute_norm_vel.setFloat("GAS_CONST", simulation.GAS_CONST);
    fluid_compute_norm_vel.setFloat("REST_DENS", simulation.REST_DENSITY);
    fluid_compute_norm_vel.setFloat("h", simulation.h);
    fluid_compute_norm_vel.setFloat("VISC", simulation.VISC);
    glDispatchCompute(simulation.particles.size(), 1, 1);

    // compute forces
    fluid_compute_force.use();
    fluid_compute_force.setInt("particles_size", simulation.particles.size());
    fluid_compute_force.setInt("num_cells", simulation.num_cells);
    fluid_compute_force.setFloat("MASS", simulation.MASS);
    fluid_compute_force.setFloat("GAS_CONST", simulation.GAS_CONST);
    fluid_compute_force.setFloat("REST_DENS", simulation.REST_DENSITY);
    fluid_compute_force.setFloat("h", simulation.h);
    fluid_compute_force.setFloat("VISC", simulation.VISC);
    fluid_compute_force.setFloat("SURF", simulation.SURF);
    fluid_compute_force.setVec3("repulser", repulser);
    fluid_compute_force.setFloat("attract_repel", attract_repel);
    glDispatchCompute(simulation.particles.size(), 1, 1);

    // integrate
    fluid_integrate.use();
    fluid_integrate.setFloat("time", glfwGetTime());
    fluid_integrate.setInt("particles_size", simulation.particles.size());
    fluid_integrate.setInt("num_cells", simulation.num_cells);
    fluid_integrate.setFloat("MASS", simulation.MASS);
    fluid_integrate.setFloat("GAS_CONST", simulation.GAS_CONST);
    fluid_integrate.setFloat("REST_DENS", simulation.REST_DENSITY);
    fluid_integrate.setFloat("h", simulation.h);
    fluid_integrate.setFloat("VISC", simulation.VISC);
    fluid_integrate.setFloat("timestep", simulation.timestep);
    fluid_integrate.setVec3("box_dimensions", simulation.box_dimensions);
    fluid_integrate.setBool("pipe", pipe);
    fluid_integrate.setVec3("focus", focus);
    glDispatchCompute(simulation.particles.size(), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // copy data back to cpu memory
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fluid.ib.id);
    // Particle *read_data =
    //     (Particle *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    // uint n_p = simulation.particles.size();
    // std::copy(&read_data[0], &read_data[n_p], simulation.particles.begin());
    // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  }

  // clear render, set params
  glViewport(0, 0, window_width, window_height);
  r_tex.bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // render the pool
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  pool_program.use();
  pool_program.setMat4("projection", projection_matrix);
  pool_program.setMat4("view", view_matrix);
  pool.bind();
  glDrawElements(GL_TRIANGLES, pool_indices.size() * 3, GL_UNSIGNED_INT,
                 pool_indices.data());

  // render the fluids
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  fluid_program.use();
  fluid_program.setMat4("projection", projection_matrix);
  fluid_program.setMat4("view", view_matrix);
  fluid_program.setInt("color_mode", color_mode);
  fluid_program.setInt("num_cells", simulation.num_cells);
  fluid.bind();
  glDrawElementsInstanced(GL_TRIANGLES, sphere_indices.size() * 3,
                          GL_UNSIGNED_INT, sphere_indices.data(),
                          simulation.particles.size());

  r_tex.unbind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // rerender on textured quad
  r_tex.bindTexture();
  tex_quad_program.use();
  tex_quad_program.setVec2("resolution",
                           glm::vec2(window_width, window_height));
  tex_quad_program.setInt("screenTex", 0);
  tex_quad_program.setInt("depTex", 1);
  tex_quad_program.setInt("renderMode", render_mode);
  texquad.bind();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  r_tex.unbindTexture();
}

void Game::update_camera() {
  if (glm::length(mouse_diff) == 0)
    return;
  mouse_diff *= -0.005f;

  pitch = std::max(-1.5f, std::min(1.5f, pitch + mouse_diff.y));
  yaw += mouse_diff.x;

  glm::quat qyaw = glm::angleAxis(yaw, UP);
  glm::quat qpitch = glm::angleAxis(pitch, SIDE);
  orientation = qyaw * qpitch;
}

glm::vec3 Game::screenspace_to_world(glm::vec2 coords, glm::vec4 viewport) {
  glm::vec3 p = glm::vec3(coords.x, coords.y, 0.0f);
  p = glm::unProject(p, view_matrix, projection_matrix, viewport);
  glm::vec3 q = glm::vec3(coords.x, coords.y, 1.0f);
  q = glm::unProject(q, view_matrix, projection_matrix, viewport);
  glm::vec3 r = q - p;
  return r;
}

bool Game::intersect_sphere(glm::vec3 ray_direction, glm::vec3 p) {
  // camera = camera
  // ray direction = ray direction
  // sphere position = p
  // sphere radius = h
  float radius = simulation.h * 2;
  glm::vec3 oc = eye - p;
  float a = glm::dot(ray_direction, ray_direction);
  float b = 2.0f * glm::dot(oc, ray_direction);
  float c = glm::dot(oc, oc) - radius * radius;
  float discrim = b * b - 4 * a * c;
  if (discrim < 0) {
    return false;
  }
  return true;
}

void Game::mouse_ray_intersect() {
  glm::uvec4 viewport = glm::uvec4(0, 0, window_width, window_height);
  glm::vec2 mp = glm::vec2(mouse_pos.x, window_height - mouse_pos.y);
  glm::vec4 ray_direction =
      glm::vec4(glm::normalize(screenspace_to_world(mp, viewport)), 0.0f);

  // ray - particle intersection
  for (uint i = 0; i < simulation.particles.size(); i++) {
    // check if ray intersects particle
    Particle &p = simulation.particles[i];
    if (intersect_sphere(glm::vec3(ray_direction), p.position)) {
      repulser = p.position;
      break;
    }
  }
}