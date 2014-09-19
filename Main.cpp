#include <iostream>
#include <GL/glut.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <string>

#include "DataStore.hpp"

GLuint tex;

srp::DataStore * dstore;

int frame;

void create_window(void);
void gl_init();
void display(void);
void print_error(void);
void resize(int w, int h);

int main(int argc, char ** argv) {
  glutInit(&argc, argv);

  if (argc == 0) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }
  dstore = new srp::DataStore(argv[1]);

  create_window();

  gl_init();

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
}

void display(void) {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();
}

void resize(int w, int h) {
}
