#define GLM_FORCE_RADIANS

#include <fstream>
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdlib.h>
#include <string>
#include <time.h>
#include <stdio.h>

#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "RenderChunk.hpp"
#include "GeometryGenerator.hpp"
#include "Util.hpp"
#include "metric/GPUMetric.hpp"
#include "ogl/OGLUtil.hpp"
#include "ogl/Shader.hpp"
#include "ogl/VertexBuffer.hpp"
#include "ogl/TexturedVertexBuffer.hpp"
#include "ogl/ui/Text.hpp"
#include "ogl/ui/Graph.hpp"
#include "Vec3.hpp"
#include "XWindow.hpp"

srp::DataStore * root;
srp::DataStore * dstore;

srp::RenderState state(2000);
srp::XWindow * window;
srp::ogl::ShaderProgram * basic;
srp::ogl::ShaderProgram * textured;
srp::ogl::VertexBuffer * axis;
srp::ogl::TexturedVertexBuffer * face;
srp::metric::GPUMetric * render_time;
srp::ogl::ui::MetricGraph * render_time_graph;

// The important bits
srp::IndexCache * cache = new srp::IndexCache();
std::vector<GLuint> * indicies = new std::vector<GLuint>();
std::vector<srp::ogl::Vertex> * verts = new std::vector<srp::ogl::Vertex>();
srp::ogl::VertexBuffer * rendered_data;

int frame;
int panel_z;

unsigned int x_chunks;
unsigned int y_chunks;
unsigned int z_chunks;

unsigned int current_x;
unsigned int current_y;
unsigned int current_z;

int last_width, last_height;

glm::mat4 projection;
glm::mat4 view;

bool running;

void gl_init();
void display_func(void);
void reshape_window(void);
void resize(int w, int h);
void main_loop(void);
static void poll_geometry(void);
static void modify_threshold(int delta);
glm::mat4 set_camera_pos_and_dir(const srp::Vec3f & Pos, const srp::Vec3f & Dir);
static void set_texture_data(srp::DataStore & ds, int Z);

int main(int argc, char ** argv) {
  if (argc == 1) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }

  srp::InitializeBaseDirectory(argv[0]);

  root = new srp::DataStore(argv[1]);
  dstore = new srp::DataStore(*root, 1);
  window = new srp::XWindow("SRP");
  render_time = new srp::metric::GPUMetric(128);
  render_time_graph = new srp::ogl::ui::MetricGraph(*render_time, 2, 16, 512, 100);

  gl_init();

  x_chunks = 1 + dstore->GetWidth() / RENDER_CHUNK_SIZE;
  y_chunks = 1 + dstore->GetHeight() / RENDER_CHUNK_SIZE;
  z_chunks = 1 + dstore->GetDepth() / RENDER_CHUNK_SIZE;

  cache = new srp::IndexCache();
  indicies = new std::vector<GLuint>();
  verts = new std::vector<srp::ogl::Vertex>();

  srp::StartGeometryGenerator(dstore);
  srp::RequestChunk(0, 0, 0, state.GetThreshold());

  current_x = 0;
  current_y = 0;
  current_z = 0;

  rendered_data = new srp::ogl::VertexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
  rendered_data->ReplaceData(*verts, *indicies);
  rendered_data->Sync();

  std::cout << "main loop" << std::endl;

  running = true;
  main_loop();

  std::cout << "cleaning up" << std::endl;

  delete cache;
  delete indicies;
  delete verts;

  delete root;
  delete dstore;
  delete window;
}

void process_events() {
  XEvent xev;
  int key;
  while (window->GetPendingEvents() > 0) {
    window->NextEvent(&xev);
    switch(xev.type) {
    case KeyPress:
      key = window->KeySymFromKeyCode(xev.xkey.keycode);
      switch (key) {
        case 'c': render_time->Reset(); break;
        case 'a': modify_threshold(-500); break;
        case 'q': modify_threshold(+500); break;
        default: break;
      }
      break;
    case ClientMessage:
      // uh... just sorta assume it's the shutdown event
      // this will probably bite me later
      running = false;
      break;
    default:
      break;
    }
  }
}

void main_loop() {
  struct timespec ptime;
  struct timespec ctime;
  struct timespec diff;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &ctime);
  while (running) {
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ptime);

    reshape_window();
    poll_geometry();
    process_events();
    display_func();

    clock_gettime(CLOCK_MONOTONIC_COARSE, &ctime);
    diff.tv_sec  = ctime.tv_sec - ptime.tv_sec;
    diff.tv_nsec = ctime.tv_nsec - ptime.tv_nsec;
    long sleeptime = 16666666l - ( diff.tv_sec * 1000000000l + diff.tv_nsec );
    diff.tv_sec  = 0;
    diff.tv_nsec = sleeptime;

    while (diff.tv_nsec > 0) {
      struct timespec rem;
      int i = nanosleep(&diff, &rem);
      if (i < 0) {
        diff.tv_sec  = rem.tv_sec;
        diff.tv_nsec = rem.tv_nsec;
      } else {
        break;
      }
    }
  }
}

static void modify_threshold(int delta) {
  unsigned int curr = state.GetThreshold();
  if (current_z != z_chunks) {
    std::cout << "cannot change isovalue, still rendering the last one!" << std::endl;
    return;
  }
  if (delta < 0) {
    if (curr < -delta) {
      state.SetThreshold(0);
    } else {
      state.SetThreshold(curr + delta);
    }
  } else {
    state.SetThreshold(curr + delta);
  }
  std::cout << "Changed threshold to " << std::dec << state.GetThreshold() << std::endl;
  current_x = current_y = current_z = 0;
  verts->clear();
  indicies->clear();
  srp::RequestChunk(0, 0, 0, state.GetThreshold());
}

static void poll_geometry(void) {
  if (current_z == z_chunks) { return; } // We've already rendered everything
  if (srp::ReadGeometry(*indicies, *verts, state.GetThreshold())) {
    current_x += 1;
    if (current_x == x_chunks) {
      current_x = 0;
      current_y += 1;
    }
    if (current_y == y_chunks) {
      current_y = 0;
      current_z += 1;
    }
    if (current_z != z_chunks) {
      srp::RequestChunk(current_x, current_y, current_z, state.GetThreshold());
    } else {
      std::cout << "finished rendering" << std::endl;
    }
    rendered_data->ReplaceData(*verts, *indicies);
    rendered_data->Sync();
  }
}

void gl_init() {
  glClearColor(0.1f, 0.1f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  GLERR();
//  glEnable(GL_CULL_FACE);

  basic = srp::ogl::CreateShader("base.vert", "base.frag");
  textured = srp::ogl::CreateShader("texture.vert", "texture.frag");

  {
    axis = new srp::ogl::VertexBuffer(GL_LINES, GL_STATIC_DRAW);
    srp::ogl::Vertex origin;
    origin.x = origin.y = origin.z = 0;
    origin.r = origin.g = origin.b = 1;
    origin.nx = origin.ny = 0;
    origin.nz = 1;

    srp::ogl::Vertex plus_x;
    plus_x.x = root->GetWidth();
    plus_x.y = plus_x.z = 0;
    plus_x.r = plus_x.g = plus_x.b = 1;
    plus_x.ny = plus_x.nz = 0;
    plus_x.nx = 1;

    srp::ogl::Vertex plus_y;
    plus_y.y = root->GetHeight();
    plus_y.x = plus_y.z = 0;
    plus_y.r = plus_y.g = plus_y.b = 1;
    plus_x.ny = plus_y.nz = 0;
    plus_y.ny = 1;

    srp::ogl::Vertex plus_z;
    plus_z.z = root->GetDepth();
    plus_z.x = plus_z.y = 0;
    plus_z.r = plus_z.g = plus_z.b = 1;
    plus_z.ny = plus_z.nx = 0;
    plus_z.nz = 1;

    srp::ogl::Vertex center;
    center.x = root->GetWidth() / 2.f;
    center.z = root->GetDepth() / 2.f;
    center.y = 0;
    center.r = center.g = 1;
    center.b = 0;

    axis->AddVertex(origin);
    axis->AddVertex(plus_x);
    axis->AddVertex(origin);
    axis->AddVertex(plus_y);
    axis->AddVertex(origin);
    axis->AddVertex(plus_z);
    axis->AddVertex(center);
    center.y = root->GetHeight();
    axis->AddVertex(center);
    axis->Sync();
  }

  {
    face = new srp::ogl::TexturedVertexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
    srp::ogl::QueueTexturedRectangle(*face, glm::vec3(0, 0, 0),
                                            glm::vec3(root->GetWidth(), root->GetHeight(), 0),
                                            glm::vec2(0, 0),
                                            glm::vec2(1, 1));
    face->Sync();
  }

  set_texture_data(*dstore, panel_z);
  GLERR();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  srp::ogl::ui::TextInit(*window);
  srp::ogl::ui::UIInit(*window);
}

#define DISPLAY_BUF_SIZE 256

void display_func(void) {
  char dispbuf[DISPLAY_BUF_SIZE];
  frame += 1;
  panel_z = 0.5 * ( sin(frame / float(100)) + 1)  * root->GetDepth();

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  GLERR();

  glm::mat4 real_view;
  real_view = glm::rotate(view, frame * 3.1415926f / 180.f, glm::vec3(0, 1, 0));
  real_view = glm::translate(real_view, glm::vec3(-(root->GetWidth() / 2.f), 0, -(root->GetDepth() / 2.f)));

  basic->Bind();
  GLERR();

  basic->Upload("projection_matrix", projection);
  basic->Upload("view_matrix", real_view);
  GLERR();

  state.SetCurrentShader(basic);

  // AXIS RENDER
  axis->Render(state);

  // DATA RENDER
  render_time->Enter();
  rendered_data->Render(state);
  render_time->Leave();

  // PANEL RENDER
  textured->Bind();
  state.SetCurrentShader(textured);
  textured->Upload("projection_matrix", projection);

  glm::mat4 translated = glm::translate(real_view, glm::vec3(0, 0, panel_z));

  textured->Upload("view_matrix", translated);
  GLERR();

  set_texture_data(*root, panel_z);
  face->Render(state);

  render_time->Finalize();

  snprintf(dispbuf, DISPLAY_BUF_SIZE, "Average: %8.4fns|Standard Deviation: %8.4fns", render_time->GetAverage(), render_time->GetStandardDeviation());
  srp::ogl::ui::TextDrawBegin(state);

  srp::ogl::ui::TextDrawColor(0, 1, 0);
  srp::ogl::ui::TextDrawString(5, 5, dispbuf);
  snprintf(dispbuf, DISPLAY_BUF_SIZE, "%ldns", render_time->GetMin());
  srp::ogl::ui::TextDrawString(render_time_graph->GetX() + render_time_graph->GetWidth(),
                               render_time_graph->GetY(),
                               dispbuf);
  snprintf(dispbuf, DISPLAY_BUF_SIZE, "%ldns", render_time->GetMax());
  srp::ogl::ui::TextDrawString(render_time_graph->GetX() + render_time_graph->GetWidth(),
                               render_time_graph->GetY() + render_time_graph->GetSampleHeight(render_time->GetMax()),
                               dispbuf);
  {
    snprintf(dispbuf, DISPLAY_BUF_SIZE, "%8.4fns", render_time->GetAverage());
    srp::ogl::ui::TextDrawColor(1, 0, 0);
    srp::ogl::ui::TextDrawString(render_time_graph->GetX() + render_time_graph->GetWidth(),
                                 render_time_graph->GetY() + render_time_graph->GetSampleHeight(render_time->GetAverage()),
                                 dispbuf);
  }

  srp::ogl::ui::TextDrawEnd(state);

  render_time_graph->Render(state);

  // buffer swap
  window->SwapBuffers();

  GLERR();
}

void reshape_window(void) {
  XWindowAttributes wa;
  window->GetAttributes(&wa);

  if (wa.width != last_width || wa.height != last_height) {
    resize(wa.width, wa.height);
    last_width = wa.width;
    last_height = wa.height;
  }
}

void resize(int w, int h) {
  glViewport(0, 0, w, h);

  //projection = glm::perspective(45.f, w / (float)h, 0.1f, 1000.f);
  projection = glm::perspective((90.f * 3.14159f) / 180.f, w / (float)h, 0.1f, 1000.f);
  view = set_camera_pos_and_dir(srp::Vec3f(256, 256, 256), (srp::Vec3f(1, 1, 1)).NormalizeSelf());

  GLERR();
}

glm::mat4 set_camera_pos_and_dir(const srp::Vec3f & c, const srp::Vec3f & d) {
  return glm::lookAt(glm::vec3(c.GetX(), c.GetY(), c.GetZ()),
                     glm::vec3(c.GetX() - d.GetX(), c.GetY() - d.GetY(), c.GetZ() - d.GetZ()),
                     glm::vec3(0, 1, 0));
}

static void set_texture_data(srp::DataStore & ds, int Z) {
  unsigned char buffer[ds.GetWidth() * ds.GetHeight() * 4];
  for (int x = 0; x < ds.GetWidth(); ++x) {
    for (int y = 0; y < ds.GetHeight(); ++y ) {
      unsigned int val = ds.GetPoint(x, y, Z) >> 4;
      buffer[(x + y * ds.GetHeight() ) * 4 + 0] = (unsigned char)(val & 0xff);
      buffer[(x + y * ds.GetHeight() ) * 4 + 1] = (unsigned char)(val & 0xff);
      buffer[(x + y * ds.GetHeight() ) * 4 + 2] = (unsigned char)(val & 0xff);
      buffer[(x + y * ds.GetHeight() ) * 4 + 3] = (unsigned char)(val & 0xff);
    }
  }

  face->SetTextureData(ds.GetWidth(), ds.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}
