#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "DataStore.hpp"
#include "Tetrahedron.hpp"
#include "RenderConfig.hpp"
#include "RenderChunk.hpp"
#include "Util.hpp"
#include "Vec3.hpp"
#include "XWindow.hpp"

srp::DataStore * dstore;

srp::RenderState state(2500);
srp::RenderChunk * chunk;
srp::XWindow * window;

int frame;
int panel_z;
GLuint tex;

int last_width, last_height;

bool running;

void gl_init();
void display_func(void);
void reshape_window(void);
void resize(int w, int h);
void main_loop(void);
void set_camera_pos_and_dir(const srp::Vec3f & Pos, const srp::Vec3f & Dir);
static void set_texture_data(srp::DataStore & ds, int Z);

int main(int argc, char ** argv) {
  if (argc == 1) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }

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
//  glPointSize(3); // faile because it's OpenGL 1.0
  glEnable(GL_DEPTH_TEST);
  GLERR();
//  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  GLERR();

  glGenTextures(1, &tex);
  GLERR();
  glBindTexture(GL_TEXTURE_2D, tex);
  GLERR();

  set_texture_data(*dstore, panel_z);
  GLERR();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  GLERR();
  glDisable(GL_TEXTURE_2D);
  GLERR();
}

void display_func(void) {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  GLERR();

  frame += 1;
  panel_z = 0.5 * ( sin(frame / float(100)) + 1)  * dstore->GetDepth();

  glPushMatrix();
  glRotatef(frame, 0, 1, 0);
  glTranslatef(-(dstore->GetWidth() / 2.f), 0, -(dstore->GetDepth() / 2.f));

  // AXIS RENDER
  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glVertex3f(0, 0, 0);
  glVertex3f(dstore->GetWidth(), 0, 0);

  glColor3f(0, 1, 0);
  glVertex3f(0, 0, 0);
  glVertex3f(0, dstore->GetHeight(), 0);

  glColor3f(0, 0, 1);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, dstore->GetDepth());

  glColor3f(1, 1, 0);
  glVertex3f(dstore->GetWidth() / 2.f, 0, dstore->GetDepth() / 2.f);
  glVertex3f(dstore->GetWidth() / 2.f, dstore->GetHeight(), dstore->GetDepth() / 2.f);

  glEnd();

  // DATA RENDER
  // glEnable(GL_VERTEX_ARRAY);

  // chunk->Render(state);

  // PANEL RENDER
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, tex);

  set_texture_data(*dstore, panel_z);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

  glBegin(GL_QUADS);

  glColor3f(1, 1, 1);

  glTexCoord2f(0, 0);
  glVertex3f(0                 , 0                  , panel_z);

  glTexCoord2f(0, 1);
  glVertex3f(0                 , dstore->GetHeight(), panel_z);

  glTexCoord2f(1, 1);
  glVertex3f(dstore->GetWidth(), dstore->GetHeight(), panel_z);

  glTexCoord2f(1, 0);
  glVertex3f(dstore->GetWidth(), 0                  , panel_z);

  glEnd();

  glDisable(GL_TEXTURE_2D);

  // buffer swap
  glPopMatrix();

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
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(40, w / (float)h, 0.1, 10000);

  set_camera_pos_and_dir(srp::Vec3f(256, 256, 256), (srp::Vec3f(1, 1, 1)).NormalizeSelf());
  glMatrixMode(GL_MODELVIEW);
}

void set_camera_pos_and_dir(const srp::Vec3f & c, const srp::Vec3f & d) {
  glMatrixMode(GL_PROJECTION);

  gluLookAt(c.GetX(), c.GetY(), c.GetZ(),
            c.GetX() - d.GetX(), c.GetY() - d.GetY(), c.GetZ() - d.GetZ(),
            0, 1, 0);

  glMatrixMode(GL_MODELVIEW);
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
