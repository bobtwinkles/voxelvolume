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

  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  unsigned char buffer[256 * 256 * 4];
  for (int x = 0; x < 256; ++x) {
    for (int y = 0; y < 256; ++y ) {
      unsigned int val = dstore->GetPoint(x, y, 30) >> 4;
      buffer[(x + y * 256 ) * 4 + 0] = (unsigned char)(val & 0xff);
      buffer[(x + y * 256 ) * 4 + 1] = (unsigned char)(val & 0xff);
      buffer[(x + y * 256 ) * 4 + 2] = (unsigned char)(val & 0xff);
      buffer[(x + y * 256 ) * 4 + 3] = (unsigned char)(val & 0xff);
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

void display(void) {
  ++frame;
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex);

  unsigned char buffer[256 * 256 * 4];
  for (int x = 0; x < 256; ++x) {
    for (int y = 0; y < 256; ++y ) {
      unsigned int val = dstore->GetPoint(x, y, frame % dstore->GetDepth()) >> 4;
      buffer[(x + y * 256 ) * 4 + 0] = (unsigned char)(val & 0xff);
      buffer[(x + y * 256 ) * 4 + 1] = (unsigned char)(val & 0xff);
      buffer[(x + y * 256 ) * 4 + 2] = (unsigned char)(val & 0xff);
      buffer[(x + y * 256 ) * 4 + 3] = (unsigned char)(val & 0xff);
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

  print_error();

  glBegin(GL_QUADS);

  glColor3f(1, 1, 1);

  glTexCoord2f(0, 0);
  glVertex2d(0, 0);

  glTexCoord2f(0, 1);
  glVertex2d(0, 1);

  glTexCoord2f(1, 1);
  glVertex2d(1, 1);

  glTexCoord2f(1, 0);
  glVertex2d(1, 0);

  glEnd();

  print_error();

  glutSwapBuffers();
}

void print_error(void) {
  int i;
  if ((i = glGetError()) != 0) {
    std::cout << "crap " << i << std::endl;
  }
}

void resize(int w, int h) {
  std::cout << "reshape" << std::endl;
  glViewport(0, 0, w, h);
  glLoadIdentity();
  glTranslatef(-1, -1, 0);
  glScalef(2, 2 * w / (double)h, 1);
}
