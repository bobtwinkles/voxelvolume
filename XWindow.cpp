#include<GL/glew.h>   // Must be first so that it is included before gl/glext.h
#include "XWindow.hpp"

#include<iostream>
#include<unistd.h>

using srp::XWindow;

static bool context_error;
static int on_context_error( Display *display, XErrorEvent * event );

XWindow::XWindow(const char * Title) {
  _display = XOpenDisplay(NULL);

  std::cerr << std::hex << _display << " is the display" << std::endl;

  if (!_display) {
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

  if (!glXQueryVersion( _display, &glx_major, &glx_minor ) ) {
    std::cerr << "failed to query X server for gl version" << std::endl;
    exit(1);
  }

  if ( (glx_major == 1 && glx_minor < 3) || glx_major < 1) {
    std::cerr << "GLX version " << glx_major << "." << glx_minor << " is insufficient" << std::endl;
    exit(1);
  }

  int fbcount;
  GLXFBConfig * fbc = glXChooseFBConfig(_display, DefaultScreen(_display), visual_attribs, &fbcount);
  if (!fbc) {
    std::cerr << "Failed to get a valid frame buffer configuratin" << std::endl;
    exit(1);
  }

  int best_fbc_index = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 99999;

  for (int i = 0; i < fbcount; ++i) {
    int samp_buf, samples;
    XVisualInfo * vi = glXGetVisualFromFBConfig( _display, fbc[i] );
    glXGetFBConfigAttrib( _display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
    glXGetFBConfigAttrib( _display, fbc[i], GLX_SAMPLES       , &samples );

    if ( best_fbc_index < 0 || ( samp_buf && samples ) > best_num_samp )
      best_fbc_index = i, best_num_samp = samples;
    if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
      worst_fbc = i, worst_num_samp = samples;
    XFree( vi );
  }

  GLXFBConfig best_fbc = fbc[best_fbc_index];

  XFree(fbc);

  XVisualInfo * vi = glXGetVisualFromFBConfig( _display, best_fbc );
  std::cout << "Chose FBC with vid 0x" << std::hex << vi->visualid << std::dec << std::endl;
  XSetWindowAttributes swa;
  swa.colormap = _map = XCreateColormap( _display,
                                         RootWindow(_display, vi->screen),
                                         vi->visual, AllocNone );
  swa.background_pixmap = None;
  swa.border_pixel = 0;
  swa.event_mask   = StructureNotifyMask;

  _win = XCreateWindow( _display, RootWindow( _display, vi->screen ),
                              0, 0, 100, 100, 0, vi->depth, InputOutput,
                              vi->visual,
                              CWBorderPixel|CWColormap|CWEventMask, &swa);

  if (!_win) {
    std::cerr << "Failed to create window!" << std::endl;
    exit(1);
  }

  XFree(vi);

  XStoreName( _display, _win, Title );

  XMapWindow( _display, _win );

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddress( (const GLubyte*) "glXCreateContextAttribsARB");

  context_error = false;
  int (*old_error_handler)(Display*, XErrorEvent*) = XSetErrorHandler(&on_context_error);

  if (!glXCreateContextAttribsARB) {
    std::cout << "glXCreateContextAttribsARB not supported, falling back" << std::endl;
    _context = glXCreateNewContext( _display, best_fbc, GLX_RGBA_TYPE, 0, True);
  } else {
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 1,
        None
    };

    _context = glXCreateContextAttribsARB( _display, best_fbc, 0, True, context_attribs );
  }

  XSync(_display, False);
  if ( !context_error && _context ) {
    std::cout << "Created context!" << std::endl;
  } else {
    std::cerr << "Couldn't get a 3.0 context, failing" << std::endl;
    XSetErrorHandler(old_error_handler);
    exit(1);
  }

  XSync(_display, False);
  XSetErrorHandler(old_error_handler);

  if (glXIsDirect(_display, _context)) {
    std::cout << "Context is direct" << std::endl;
  } else {
    std::cout << "Context is indirect" << std::endl;
  }

  glXMakeCurrent(_display, _win, _context);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "GLEW initialisation failed" << std::endl;
    exit(1);
  }
  // Clear OpenGL error which for whatever reason glewInit doesn't
  glGetError();
  std::cout << "Created glew context with type " << glewGetString(GLEW_VERSION) << std::endl;
  GLERR();
}

XWindow::~XWindow() {
  glXMakeCurrent( _display, 0, 0 );
  glXDestroyContext( _display, _context );
  XDestroyWindow( _display, _win );
  XFreeColormap( _display, _map );
  XCloseDisplay( _display );
}

void XWindow::SwapBuffers() {
  glXSwapBuffers(_display, _win);
}

void XWindow::GetAttributes(XWindowAttributes * WA) const {
  XGetWindowAttributes(_display, _win, WA);
}

static int on_context_error( Display * Display, XErrorEvent * Event ) {
  context_error = true;
  std::cerr << "The thingy didn't worky " << Event->error_code << " " << Event->minor_code << std::endl;
  return 0;
}
