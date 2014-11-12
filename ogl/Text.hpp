#ifndef _OGL_TEXT_H_
#define _OGL_TEXT_H_

#include "RenderConfig.hpp"
#include "XWindow.hpp"

namespace srp {
  namespace ogl {
    namespace text {
      void TextInit(srp::XWindow & Window);

      void TextDrawBegin(srp::RenderState & State);
      void TextDrawColor(float R, float G, float B);
      void TextDrawString(int X, int Y, const char * String);
      void TextDrawEnd(srp::RenderState & State);
    }
  }
}

#endif
