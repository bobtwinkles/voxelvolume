#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <string>

#include "DataStore.hpp"
#include "Tetrahedron.hpp"
#include "RenderConfig.hpp"
#include "RenderChunk.hpp"
#include "Vec3.hpp"

srp::DataStore * dstore;

srp::RenderState state(2500);
srp::RenderChunk * chunk;

int frame;
int panel_z;
GLuint tex;

void create_window(void);
void gl_init();
void display(void);
void print_error(void);
void resize(int w, int h);
void set_camera_pos_and_dir(const srp::Vec3f & Pos, const srp::Vec3f & Dir);
static void set_texture_data(srp::DataStore & ds, int Z);

int main(int argc, char ** argv) {
  glutInit(&argc, argv);

  if (argc == 0) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }
  dstore = new srp::DataStore(argv[1]);

  create_window();

  gl_init();

  chunk = new srp::RenderChunk(*dstore, 2, 3, 2);

  std::cout << "main loop" << std::endl;
  glutMainLoop();
}

void create_window(void) {
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(0, 0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

  std::cout << "About to create window" << std::endl;

  glutCreateWindow("testing");
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutReshapeFunc(resize);
}

void gl_init() {
  glClearColor(0, 0, 0, 0);
  glPointSize(3);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
//  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  set_texture_data(*dstore, panel_z);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glDisable(GL_TEXTURE_2D);
}

void display(void) {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  frame += 1;
  panel_z = frame % dstore->GetDepth();

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
  glBegin(GL_TRIANGLES);

  glColor3f(1, 1, 1);
  chunk->Render(state);

  glEnd();

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

  glutSwapBuffers();
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
