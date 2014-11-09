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

#include "DataStore.hpp"
#include "Tetrahedron.hpp"
#include "RenderConfig.hpp"
#include "RenderChunk.hpp"
#include "Util.hpp"
#include "ogl/OGLUtil.hpp"
#include "ogl/Shader.hpp"
#include "Vec3.hpp"
#include "XWindow.hpp"

srp::DataStore * dstore;

srp::RenderState state(2500);
srp::RenderChunk * chunk;
srp::XWindow * window;
srp::ogl::ShaderProgram * basic;
srp::ogl::VertexBuffer * axis;

int frame;
int panel_z;
GLuint tex;

int last_width, last_height;

glm::mat4 projection;
glm::mat4 view;

bool running;

void gl_init();
void display_func(void);
void reshape_window(void);
void resize(int w, int h);
void main_loop(void);
glm::mat4 set_camera_pos_and_dir(const srp::Vec3f & Pos, const srp::Vec3f & Dir);
static void set_texture_data(srp::DataStore & ds, int Z);

int main(int argc, char ** argv) {
  if (argc == 1) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }

  srp::InitializeBaseDirectory(argv[0]);

  dstore = new srp::DataStore(argv[1]);

  window = new srp::XWindow("SRP");

  gl_init();

  chunk = new srp::RenderChunk(*dstore, 2, 3, 2);

  std::cout << "main loop" << std::endl;

  running = true;
  main_loop();

  std::cout << "cleaning up" << std::endl;

  delete chunk;
  delete dstore;
  delete window;
}

void process_events() {
}

void main_loop() {
  struct timespec ptime;
  struct timespec ctime;
  struct timespec diff;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &ctime);
  while (running) {
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ptime);

    reshape_window();
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

void gl_init() {
  glClearColor(0, 0, 0, 0);
//  glPointSize(3); // fails because it's OpenGL 1.0
  glEnable(GL_DEPTH_TEST);
  GLERR();
//  glEnable(GL_CULL_FACE);

  std::shared_ptr<srp::ogl::Shader> vert(new srp::ogl::Shader(GL_VERTEX_SHADER));
  std::shared_ptr<srp::ogl::Shader> frag(new srp::ogl::Shader(GL_FRAGMENT_SHADER));

  vert->AttachSource(std::shared_ptr<srp::ogl::ShaderSource>(new srp::ogl::ShaderSource("base.vert")));
  frag->AttachSource(std::shared_ptr<srp::ogl::ShaderSource>(new srp::ogl::ShaderSource("base.frag")));

  basic = new srp::ogl::ShaderProgram();
  basic->AddShader(vert);
  basic->AddShader(frag);
  basic->Link();

  axis = new srp::ogl::VertexBuffer(GL_LINES, GL_STATIC_DRAW);

  {
    srp::ogl::Vertex origin;
    origin.x = origin.y = origin.z = 0;
    origin.r = origin.g = origin.b = 1;
    origin.nx = origin.ny = origin.nz = origin.u = origin.v = 0;

    srp::ogl::Vertex plus_x;
    plus_x.x = 1;
    plus_x.y = plus_x.z = 0;
    plus_x.r = plus_x.g = plus_x.b = 1;
    plus_x.nx = plus_x.ny = plus_x.nz = plus_x.u = plus_x.v = 0;

    srp::ogl::Vertex plus_y;
    plus_y.y = 1;
    plus_y.x = plus_y.z = 0;
    plus_y.r = plus_y.g = plus_y.b = 1;
    plus_y.nx = plus_y.ny = plus_y.nz = plus_y.u = plus_y.v = 0;

    srp::ogl::Vertex plus_z;
    plus_z.z = 1;
    plus_z.x = plus_z.y = 0;
    plus_z.r = plus_z.g = plus_z.b = 1;
    plus_z.nx = plus_z.ny = plus_z.nz = plus_z.u = plus_z.v = 0;

    axis->AddVertex(origin);
    axis->AddVertex(plus_x);
    axis->AddVertex(origin);
    axis->AddVertex(plus_y);
    axis->AddVertex(origin);
    axis->AddVertex(plus_z);
    axis->Sync();
  }

  glGenTextures(1, &tex);
  GLERR();

  glBindTexture(GL_TEXTURE_2D, tex);
  GLERR();

  set_texture_data(*dstore, panel_z);
  GLERR();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  GLERR();
}

void display_func(void) {
  frame += 1;
  panel_z = 0.5 * ( sin(frame / float(100)) + 1)  * dstore->GetDepth();

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  GLERR();

  glm::mat4 real_view = glm::rotate(view, frame * 3.1415926f / 180.f, glm::vec3(0, 1, 0));
  real_view = glm::translate(real_view, glm::vec3(-(dstore->GetWidth() / 2.f), 0, -(dstore->GetDepth() / 2.f)));

  basic->Bind();
  GLERR();

  basic->Upload("projection_matrix", projection);
  GLERR();
  basic->Upload("view_matrix", real_view);
  GLERR();

  state.SetPositionIndex(basic->GetAttributeLocation("position"));
  GLERR();
  state.SetColorIndex   (basic->GetAttributeLocation("color"));
  GLERR();

  // AXIS RENDER
  axis->Render(state);

  // DATA RENDER
  //chunk->Render(state);

  //    // PANEL RENDER
  //    glEnable(GL_TEXTURE_2D);

  //    glBindTexture(GL_TEXTURE_2D, tex);

  //    set_texture_data(*dstore, panel_z);

  //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

  //    glBegin(GL_QUADS);

  //    glColor3f(1, 1, 1);

  //    glTexCoord2f(0, 0);
  //    glVertex3f(0                 , 0                  , panel_z);

  //    glTexCoord2f(0, 1);
  //    glVertex3f(0                 , dstore->GetHeight(), panel_z);

  //    glTexCoord2f(1, 1);
  //    glVertex3f(dstore->GetWidth(), dstore->GetHeight(), panel_z);

  //    glTexCoord2f(1, 0);
  //    glVertex3f(dstore->GetWidth(), 0                  , panel_z);

  //    glEnd();

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

  projection = glm::infinitePerspective(40.f, w / (float)h, 0.1f);
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

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ds.GetWidth(), ds.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}
