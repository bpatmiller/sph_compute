#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
  unsigned int ID;

  std::string readShader(const char *shaderPath) {
    std::string shaderCode;
    std::ifstream shaderFile;

    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      shaderFile.open(shaderPath);
      std::stringstream shaderStream;
      shaderStream << shaderFile.rdbuf();
      shaderFile.close();
      shaderCode = shaderStream.str();
    } catch (std::ifstream::failure &e) {
    }
    return shaderCode;
  }

  Shader(const char *vertexPath, const char *geometryPath,
         const char *fragmentPath) {
    std::string vertCode = readShader(vertexPath);
    std::string geomCode = readShader(geometryPath);
    std::string fragCode = readShader(fragmentPath);

    if (vertCode == "" || fragCode == "") {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char *vShaderCode = vertCode.c_str();
    const char *fShaderCode = fragCode.c_str();

    GLuint vertex, fragment;

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // geometry shader
    GLuint geometry = 0;
    if (geomCode != "") {
      const char *gShaderCode = geomCode.c_str();
      geometry = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometry, 1, &gShaderCode, NULL);
      glCompileShader(geometry);
      checkCompileErrors(geometry, "GEOMETRY");
    }

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    if (geometry)
      glAttachShader(ID, geometry);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    glDeleteShader(vertex);
    if (geometry)
      glDeleteShader(geometry);
    glDeleteShader(fragment);
  }
  // activate the shader
  // ------------------------------------------------------------------------
  void use() { glUseProgram(ID); }
  // utility uniform functions
  // ------------------------------------------------------------------------
  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }
  // ------------------------------------------------------------------------
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  // ------------------------------------------------------------------------
  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }
  // ------------------------------------------------------------------------
  void setMat(const std::string &name, glm::mat4 value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       (const GLfloat *)&value[0][0]);
  }
  // ------------------------------------------------------------------------
  void setMat2(const std::string &name, glm::mat2 value) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       (const GLfloat *)&value[0][0]);
  }
  // ------------------------------------------------------------------------
  void setVec3(const std::string &name, glm::vec3 value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1,
                 (const GLfloat *)&value[0]);
  }
  // ------------------------------------------------------------------------
  void setVec2(const std::string &name, glm::vec2 value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1,
                 (const GLfloat *)&value[0]);
  }

private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout
            << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
            << infoLog
            << "\n -- --------------------------------------------------- --"
            << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout
            << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
            << infoLog
            << "\n -- --------------------------------------------------- -- "
            << std::endl;
      }
    }
  }
};
#endif