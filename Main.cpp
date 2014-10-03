#include <iostream>
#include <GL/glut.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <string>

#include "DataStore.hpp"
#include "Tetrahedron.hpp"
#include "RenderConfig.hpp"
#include "RenderChunk.hpp"
#include "Vec3.hpp"

GLuint tex;

srp::DataStore * dstore;

srp::RenderState state(1100);
srp::RenderChunk * chunk;

int frame;

void create_window(void);
void gl_init();
void display(void);
void print_error(void);
void resize(int w, int h);
void set_camera_pos_and_dir(const srp::Vec3f & Pos, const srp::Vec3f & Dir);

int main(int argc, char ** argv) {
  glutInit(&argc, argv);

  if (argc == 0) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }
  dstore = new srp::DataStore(argv[1]);

  create_window();

  gl_init();

  chunk = new srp::RenderChunk(*dstore, 0, 0, 0);

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
}

void display(void) {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glRotatef(1.f, 0, 1, 0);

  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(1, 0, 0);

  glVertex3f(0, 0, 0);
  glVertex3f(0, 1, 0);

  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, 1);
  glEnd();

  glBegin(GL_TRIANGLES);

  glColor3f(1, 1, 1);
  chunk->Render(state);

  glEnd();

  glutSwapBuffers();
}

void resize(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(90, w / (float)h, 0.1, 100);

  set_camera_pos_and_dir(srp::Vec3f(RENDER_CHUNK_SIZE, RENDER_CHUNK_SIZE, RENDER_CHUNK_SIZE), (srp::Vec3f(1, 1, 1)).NormalizeSelf());
  glMatrixMode(GL_MODELVIEW);
}

void set_camera_pos_and_dir(const srp::Vec3f & c, const srp::Vec3f & d) {
  glMatrixMode(GL_PROJECTION);

  gluLookAt(c.GetX(), c.GetY(), c.GetZ(),
            c.GetX() - d.GetX(), c.GetY() - d.GetY(), c.GetZ() - d.GetZ(),
            0, 1, 0);

  glMatrixMode(GL_MODELVIEW);
}
