#include "../slsgl.h"

#include "Angel.h"
#include <cstring>

namespace Angel {

// Create a NULL-terminated string by reading the provided file
static char *readShaderSource(const char *shaderFile) {
  FILE *fp = fopen(shaderFile, "r");

  if (fp == NULL) {
    return NULL;
  }
  long size = ftell(fp);

  fseek(fp, 0L, SEEK_END);
  size = ftell(fp) - size;

  fseek(fp, 0L, SEEK_SET);
  char *buf = new char[size + 1];
  memset(buf, 0, size + 1);
  
  fread(buf, 1, size, fp);

  buf[size] = '\0';
  fclose(fp);

  return buf;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char *vShaderFile, const char *fShaderFile,
                  char const *uniform_header_file) {
  struct Shader {
    const char *filename;
    GLenum type;
    GLchar *source;
  } shaders[2] = {{vShaderFile, GL_VERTEX_SHADER, NULL},
                  {fShaderFile, GL_FRAGMENT_SHADER, NULL}};

  char const *default_header = R"GLSL(#version 410
#line 0 0
#define SLS_MAX_LIGHTS 8
#line 0 1
  )GLSL";

  // if no file is given, we use default string literal
  // for the source
  auto free_header = bool(uniform_header_file);
  char const *uniform_src = uniform_header_file
                                ? readShaderSource(uniform_header_file)
                                : default_header;

  GLuint program = glCreateProgram();

  for (int i = 0; i < 2; ++i) {
    Shader &s = shaders[i];
    s.source = readShaderSource(s.filename);
    if (shaders[i].source == NULL) {
      std::cerr << "Failed to read " << s.filename << std::endl;
      exit(EXIT_FAILURE);
    }

    char const *sources[] = {uniform_src, s.source};

    GLuint shader = glCreateShader(s.type);
    glShaderSource(shader, sizeof(sources) / sizeof(char const *), sources,
                   NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      auto shader_type = s.type == GL_FRAGMENT_SHADER ?
        "GL_FRAGMENT_SHADER" :
        "GL_VERTEX_SHADER";
      std::cerr << s.filename << " failed to compile: " << shader_type << std::endl;
      GLint logSize;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
      char *logMsg = new char[logSize];
      glGetShaderInfoLog(shader, logSize, NULL, logMsg);
      std::cerr << logMsg << std::endl;
      std::cerr << uniform_src << std::endl << s.source << std::endl;
      delete[] logMsg;

      exit(EXIT_FAILURE);
    }

    delete[] s.source;

    glAttachShader(program, shader);
  }

  /* link  and error check */
  glLinkProgram(program);

  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked) {
    std::cerr << "Shader program failed to link" << std::endl;
    GLint logSize;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    char *logMsg = new char[logSize];
    glGetProgramInfoLog(program, logSize, NULL, logMsg);
    std::cerr << logMsg << std::endl;
    delete[] logMsg;

    exit(EXIT_FAILURE);
  }

  if (free_header) {
    // don't delete if it isn't read from file
    delete uniform_src;
  }
  /* use program object */
  glUseProgram(program);

  return program;
}

} // Close namespace Angel block
