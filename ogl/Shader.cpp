#include "Shader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <set>
#include <utility>

#include "OGLUtil.hpp"

using srp::ogl::Shader;
using srp::ogl::ShaderSource;
using srp::ogl::ShaderProgram;

static std::set<std::pair<GLuint, const char*>> __seen_invalid;

static void ReadFullFile(std::istream & SourceStream, char ** OutString, int * OutLen);
static void ReadFullFile(const char* FName, char ** OutString, int * OutLen);

//////////////////////////////////////////
//////////////////////////////////////////
///////////////// Shader /////////////////
//////////////////////////////////////////
//////////////////////////////////////////

ShaderSource::ShaderSource(std::istream & SourceStream) {
  ReadFullFile(SourceStream, &_data, &_len);
}

ShaderSource::ShaderSource(const char * FName) {
  ReadFullFile(FName, &_data, &_len);
}

ShaderSource::~ShaderSource() {
  delete[] _data;
}

//////////////////////////////////////////
//////////////////////////////////////////
///////////////// Shader /////////////////
//////////////////////////////////////////
//////////////////////////////////////////

Shader::Shader(GLenum Type) {
  this->_sid = glCreateShader(Type);
  GLERR();
  this->_type = Type;
  this->_compiled = false;
}

Shader::~Shader() {
  glDeleteShader(this->_sid);
  GLERR();
}

Shader & Shader::AttachSource(std::shared_ptr<ShaderSource> Source) {
  _sources.push_back(Source);
  return *this;
}

void Shader::FreeSources() {
  _sources.clear();
}

Shader & Shader::Compile() {
  // build the list of sources
  GLint * lengths = new GLint[_sources.size()];
  GLchar ** sources = new GLchar*[_sources.size()];
  std::vector<std::shared_ptr<ShaderSource>>::iterator it = _sources.begin();
  int i = 0;
  while (it != _sources.end()) {
    lengths[i] = (*it)->GetLength();
    sources[i] = (*it)->GetString();
    ++i; ++it;
  }

  // Try to compile the shader
  glShaderSource(_sid, _sources.size(), (const GLchar **)sources, lengths);
  glCompileShader(this->_sid);

  // We don't need this stuff anymore, it's all in dirverland
  delete[] lengths;
  delete[] sources;

  int param;
  glGetShaderiv(_sid, GL_COMPILE_STATUS, &param);
  if (param != GL_TRUE) {
    glGetShaderiv(_sid, GL_INFO_LOG_LENGTH, &param);
    char * info = new char [param + 1];
    glGetShaderInfoLog(_sid, param + 1, &param, info);
    std::cerr << info;
    std::cerr << std::endl;
    std::exit(1); //TODO: perhaps more graceful handling of this?
  }

  this->_compiled = true;
  return *this;
}

//////////////////////////////////////////
//////////////////////////////////////////
///////////// ShaderProgram //////////////
//////////////////////////////////////////
//////////////////////////////////////////

ShaderProgram::ShaderProgram() {
  _sid = glCreateProgram();
  _linked = false;
}

ShaderProgram::~ShaderProgram() {
  glDeleteProgram(_sid);
}

ShaderProgram & ShaderProgram::AddShader(std::shared_ptr<Shader> S) {
  _shaders.push_back(S);
  return *this;
}

void ShaderProgram::ClearShaders() {
  _shaders.clear();
}

ShaderProgram & ShaderProgram::Link() {
  int param;
  std::vector<std::shared_ptr<Shader>>::iterator it;

  it = _shaders.begin();
  while (it != _shaders.end()) {
    std::shared_ptr<Shader> s = *it;
    if (!s->IsCompilied()) {
      s->Compile();
    }

    glAttachShader(_sid, s->GetShaderID());
    ++it;
  }

  glLinkProgram(_sid);
  glGetProgramiv(_sid, GL_LINK_STATUS, &param);
  if (!param) {
    PrintProgramInfoLogAndExit();
  }

  glValidateProgram(_sid);
  glGetProgramiv(_sid, GL_VALIDATE_STATUS, &param);
  if (!param) {
    PrintProgramInfoLogAndExit();
  }

  this->_linked = true;

  return *this;
}

void ShaderProgram::Bind() {
  if (!this->_linked) {
    this->Link();
  }

  glUseProgram(_sid);
  GLERR();
}

void ShaderProgram::Upload(const char * Name, glm::vec2 Vec) {
  GLint loc = FindUniform(Name);
  if (loc < 0) {
    return;
  }
  glUniform2f(loc, Vec.x, Vec.y);
}

void ShaderProgram::Upload(const char * Name, glm::vec3 Vec) {
  GLint loc = FindUniform(Name);
  if (loc < 0) {
    return;
  }
  glUniform3f(loc, Vec.x, Vec.y, Vec.z);
}

void ShaderProgram::Upload(const char * Name, glm::mat4 Mat) {
  GLint loc = FindUniform(Name);
  if (loc < 0) {
    return;
  }
  float data[16] = {0.0};
  const float * source = (const float *)glm::value_ptr(Mat);
  for (int i = 0; i < 16; ++i) {
    data[i] = source[i];
  }
  glUniformMatrix4fv(loc, 1, GL_FALSE,data);
}

void ShaderProgram::Upload(const char * Name, GLint sampler) {
  GLint loc = FindUniform(Name);
  if (loc < 0) { return; }
  glUniform1i(loc, sampler);
}

GLint ShaderProgram::GetAttributeLocation(const char * Name) {
  GLint tr = glGetAttribLocation(_sid, (const GLchar *) Name);
  if (tr < 0) {
    if (__seen_invalid.find(std::make_pair(_sid, Name)) == __seen_invalid.end()) {
      std::cerr << "WARNING: Tried to get attribute location of non-existant or inactive attribute " << Name << std::endl;
      __seen_invalid.insert(std::make_pair(_sid, Name));
    }
  }
  GLERR();
  return tr;
}

void ShaderProgram::PrintProgramInfoLogAndExit() {
  int len;
  glGetProgramiv(_sid, GL_INFO_LOG_LENGTH, &len);
  ++len;
  char * buf = new char [len];
  glGetProgramInfoLog(_sid, len, &len, buf);
  std::cerr << buf;
  std::cerr << std::endl;
  BUG();
}

GLint ShaderProgram::FindUniform(const char * Name) {
  GLint tr = glGetUniformLocation(_sid, Name);
  if (tr < 0) {
    if (__seen_invalid.find(std::make_pair(_sid, Name)) == __seen_invalid.end()) {
      std::cerr << "WARNING: Tried to upload to invalid or inactive uniform " << Name << std::endl;
      __seen_invalid.insert(std::make_pair(_sid, Name));
    }
  }
  return tr;
}

//std::ostream & operator<< (std::ostream & Stream, const srp::ogl::ShaderProgram & SP ) {
//  Stream << "{ShaderProgram: [ID " << SP._sid << "]}";
//}

//////////////////////////////////////////
//////////////////////////////////////////
/////////////// Utilities ////////////////
//////////////////////////////////////////
//////////////////////////////////////////

static void ReadFullFile(const char * FName, GLchar ** OutBuf, GLint * OutLen) {
  std::string base = srp::GetBaseDirectory();
  base = base.append("shaders/");
  base = base.append(FName);
  std::ifstream stream(base.c_str());
  if (!stream) {
    std::cerr << "Failed to open shader " << base << ", aborting" << std::endl;
    BUG();
  }
  std::cout << "Loading shader " << FName << " from " << base << std::endl;
  ReadFullFile(stream, OutBuf, OutLen);
}

static void ReadFullFile(std::istream & SourceStream, GLchar ** OutBuf, GLint * OutLen) {
  SourceStream.seekg(0, SourceStream.end);
  *OutLen = SourceStream.tellg();
  SourceStream.seekg(0, SourceStream.beg);

  *OutBuf = new char [*OutLen];

  SourceStream.read(*OutBuf, *OutLen);
}
