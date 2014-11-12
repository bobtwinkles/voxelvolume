#ifndef _OGL_UI_UI_
#define _OGL_UI_UI_

#include <GL/glew.h>

#include "RenderConfig.hpp"
#include "XWindow.hpp"

namespace srp {
  namespace ogl {
    namespace ui {
      void UIInit(srp::XWindow & Window);

      srp::XWindow * UIGetWindow();
      // TODO: make this a thing that accepts caps and looks up shader
      srp::ogl::ShaderProgram * GetGraphShader();
    }
  }
}

#endif // _OGL_UI_UI_
