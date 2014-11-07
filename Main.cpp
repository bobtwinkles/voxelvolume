#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdlib.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "DataStore.hpp"
#include "Tetrahedron.hpp"
#include "RenderConfig.hpp"
#include "RenderChunk.hpp"
#include "Vec3.hpp"

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

srp::DataStore * dstore;

srp::RenderState state(2500);
srp::RenderChunk * chunk;

int frame;
int panel_z;
GLuint tex;

int last_width, last_height;

static bool context_error;
Display * display;
Window win;
Colormap cmap;
GLXContext context;

bool running;

void create_window(void);
void gl_init();
void display_func(void);
void print_error(void);
void reshape_window(void);
void resize(int w, int h);
void main_loop(void);
void set_camera_pos_and_dir(const srp::Vec3f & Pos, const srp::Vec3f & Dir);
static int on_context_error( Display *display, XErrorEvent * event );
static void set_texture_data(srp::DataStore & ds, int Z);

int main(int argc, char ** argv) {
  if (argc == 1) {
    std::cout << "Folder path required" << std::endl;
    exit(1);
  }

  dstore = new srp::DataStore(argv[1]);

  create_window();

  gl_init();

  chunk = new srp::RenderChunk(*dstore, 2, 3, 2);

  std::cout << "main loop" << std::endl;

  running = true;
  main_loop();

  glXMakeCurrent( display, 0, 0 );
  glXDestroyContext( display, context );
  XDestroyWindow( display, win );
  XFreeColormap( display, cmap );
  XCloseDisplay( display );

  delete chunk;
  delete dstore;
}

void process_events() {
}

void main_loop() {
  while (running) {
    reshape_window();
    process_events();
    display_func();
  }
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

void display_func(void) {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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

  glXSwapBuffers(display, win);
}

void reshape_window(void) {
  XWindowAttributes wa;
  XGetWindowAttributes(display, win, &wa);

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

void create_window(void) {
  display = XOpenDisplay(NULL);

  std::cerr << std::hex << display << " is the display" << std::endl;

  if (!display) {
    std::cerr << "Failed to open display" << std::endl;
    exit(1);
  }

  static int visual_attribs[] = {
      GLX_X_RENDERABLE  , True,
      GLX_DRAWABLE_TYPE , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE   , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE , GLX_TRUE_COLOR,
      GLX_RED_SIZE      , 8,
      GLX_GREEN_SIZE    , 8,
      GLX_BLUE_SIZE     , 8,
      GLX_ALPHA_SIZE    , 8,
      GLX_DEPTH_SIZE    , 24,
      GLX_STENCIL_SIZE  , 8
  };

  int glx_major, glx_minor;

  if (!glXQueryVersion( display, &glx_major, &glx_minor ) ) {
    std::cerr << "failed to query X server for gl version" << std::endl;
    exit(1);
  }

  if ( (glx_major == 1 && glx_minor < 3) || glx_major < 1) {
    std::cerr << "GLX version " << glx_major << "." << glx_minor << " is insufficient" << std::endl;
    exit(1);
  }

  int fbcount;
  GLXFBConfig * fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
  if (!fbc) {
    std::cerr << "Failed to get a valid frame buffer configuratin" << std::endl;
    exit(1);
  }

  int best_fbc_index = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 99999;

  for (int i = 0; i < fbcount; ++i) {
    int samp_buf, samples;
    XVisualInfo * vi = glXGetVisualFromFBConfig( display, fbc[i] );
    glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
    glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples );

    if ( best_fbc_index < 0 || ( samp_buf && samples ) > best_num_samp )
      best_fbc_index = i, best_num_samp = samples;
    if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
      worst_fbc = i, worst_num_samp = samples;
    XFree( vi );
  }

  GLXFBConfig best_fbc = fbc[best_fbc_index];

  XFree(fbc);

  XVisualInfo * vi = glXGetVisualFromFBConfig( display, best_fbc );
  std::cout << "Chose FBC with vid 0x" << std::hex << vi->visualid << std::dec << std::endl;
  XSetWindowAttributes swa;
  swa.colormap = cmap = XCreateColormap( display,
                                         RootWindow(display, vi->screen),
                                         vi->visual, AllocNone );
  swa.background_pixmap = None;
  swa.border_pixel = 0;
  swa.event_mask   = StructureNotifyMask;

  win = XCreateWindow( display, RootWindow( display, vi->screen ),
                              0, 0, 100, 100, 0, vi->depth, InputOutput,
                              vi->visual,
                              CWBorderPixel|CWColormap|CWEventMask, &swa);

  if (!win) {
    std::cerr << "Failed to create window!" << std::endl;
    exit(1);
  }

  XFree(vi);

  XStoreName( display, win, "SRP" );

  XMapWindow( display, win );

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddress( (const GLubyte*) "glXCreateContextAttribsARB");

  context_error = false;
  int (*old_error_handler)(Display*, XErrorEvent*) = XSetErrorHandler(&on_context_error);

  if (!glXCreateContextAttribsARB) {
    std::cout << "glXCreateContextAttribsARB not supported, falling back" << std::endl;
    context = glXCreateNewContext( display, best_fbc, GLX_RGBA_TYPE, 0, True);
  } else {
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 1,
        None
    };

    context = glXCreateContextAttribsARB( display, best_fbc, 0, True, context_attribs );
  }

  XSync(display, False);
  if ( !context_error && context ) {
    std::cout << "Created context!" << std::endl;
  } else {
    std::cerr << "Couldn't get a 3.0 context, failing" << std::endl;
    XSetErrorHandler(old_error_handler);
    exit(1);
  }

  XSync(display, False);
  XSetErrorHandler(old_error_handler);

  if (glXIsDirect(display, context)) {
    std::cout << "Context is direct" << std::endl;
  } else {
    std::cout << "Context is indirect" << std::endl;
  }

  glXMakeCurrent(display, win, context);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "GLEW initialisation failed" << std::endl;
    exit(1);
  }
  std::cout << "Created glew context with type " << glewGetString(GLEW_VERSION) << std::endl;

  std::cout << "About to create window" << std::endl;
}

static int on_context_error( Display *display, XErrorEvent * event ) {
  context_error = true;
  std::cerr << "The thingy didn't worky " << event->error_code << " " << event->minor_code << std::endl;
  return 0;
}
