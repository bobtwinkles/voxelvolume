#ifndef _OGL_SHADER_H_
#define _OGL_SHADER_H_

#include <istream>
#include <GL/glew.h>
#include <vector>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "Util.hpp"

namespace srp {
  namespace ogl {
    class ShaderSource {
      private:
        GLchar * _data;
        GLint _len;

        DISALLOW_COPY_AND_ASSIGN(ShaderSource);
      public:
        ShaderSource(std::istream & SourceStream);
        ~ShaderSource();

        GLchar * GetString() const { return _data; }
        GLint GetLength() const { return _len; }
    };

    class Shader {
      private:
        // OpenGL resources
        GLuint _sid;
        GLenum _type;

        // Our state
        bool _compiled;
        std::vector<std::shared_ptr<ShaderSource>> _sources;

        DISALLOW_COPY_AND_ASSIGN(Shader);
      public:
        Shader(GLenum Type);
        ~Shader();

        /// Returns *this to facilitate method chaining
        Shader & AttachSource(std::shared_ptr<ShaderSource>);
        /// Clears the sources array
        void FreeSources();

        /// Returns *this to facilitate method chaining
        Shader & Compile();
        inline bool IsCompilied() const { return _compiled; }
        inline GLuint GetShaderID() const { return _sid; }
    };

    class ShaderProgram {
      private:
        GLuint _sid;

        bool _linked;
        std::vector<std::shared_ptr<Shader>> _shaders;

        void PrintProgramInfoLogAndExit();

        GLint FindUniform(const char * name);

        DISALLOW_COPY_AND_ASSIGN(ShaderProgram);
      public:
        ShaderProgram();
        ~ShaderProgram();

        /// return *this to support method chaining
        ShaderProgram & AddShader(std::shared_ptr<Shader>);
        void ClearShaders();

        /// return *this to support method chaining
        /// Automatically tries to compile all attached shaders
        ShaderProgram & Link();

        /// Bind this shader and make it active
        void Bind();

        /// Upload functions
        void Upload(const char * Name, glm::vec2 vec);
        void Upload(const char * Name, glm::vec3 vec);
        void Upload(const char * Name, glm::mat4 mat);

        // Fetch functions
        GLint GetAttributeLocation(const char * Name);
    };
  }
}


#endif //_OGL_SHADER_H_
