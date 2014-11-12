#ifndef _X_WINDOW_H_
#define _X_WINDOW_H_

#include "Util.hpp"

#include<GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

namespace srp {
  class XWindow {
    private:
      Display *  _display;
      Window     _win;
      Colormap   _map;
      GLXContext _context;

      DISALLOW_COPY_AND_ASSIGN(XWindow);
    public:
      XWindow(const char * Title);
      ~XWindow();

      void SwapBuffers();

      void GetGeometry(unsigned int * OutWidth, unsigned int * OutHeight);
      void GetAttributes(XWindowAttributes * WA) const;
  };
}

#endif
