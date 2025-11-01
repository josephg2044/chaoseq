#pragma once
#include "glitter.hpp"
#include <iostream>
#include <string>

class Shader {
  public:
    Shader() : program_id(0) {}
    Shader(const char *vertex_source, const char *fragment_source) {
        compile(vertex_source, fragment_source);
    }

    void compile(const char *vertex_source, const char *fragment_source) {
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
        glCompileShader(vertex_shader);
        check_shader(vertex_shader, "VERTEX");

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
        glCompileShader(fragment_shader);
        check_shader(fragment_shader, "FRAGMENT");

        program_id = glCreateProgram();
        glAttachShader(program_id, vertex_shader);
        glAttachShader(program_id, fragment_shader);
        glLinkProgram(program_id);
        check_program(program_id);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    void use() const { glUseProgram(program_id); }

    void set_mat4(const std::string &name, const glm::mat4 &matrix) const {
        const GLint location = glGetUniformLocation(program_id, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void set_vec3(const std::string &name, const glm::vec3 &value) const {
        const GLint location = glGetUniformLocation(program_id, name.c_str());
        glUniform3fv(location, 1, glm::value_ptr(value));
    }

    void set_vec4(const std::string &name, const glm::vec4 &value) const {
        const GLint location = glGetUniformLocation(program_id, name.c_str());
        glUniform4fv(location, 1, glm::value_ptr(value));
    }

    void set_float(const std::string &name, float value) const {
        const GLint location = glGetUniformLocation(program_id, name.c_str());
        glUniform1f(location, value);
    }

    void set_int(const std::string &name, int value) const {
        const GLint location = glGetUniformLocation(program_id, name.c_str());
        glUniform1i(location, value);
    }

  private:
    GLuint program_id;

    void check_shader(GLuint shader, const char *type) {
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint log_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
            std::string log(log_length, ' ');
            glGetShaderInfoLog(shader, log_length, nullptr, log.data());
            std::cerr << type << " SHADER COMPILATION ERROR:\n" << log << "\n";
        }
    }

    void check_program(GLuint program) {
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint log_length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
            std::string log(log_length, ' ');
            glGetProgramInfoLog(program, log_length, nullptr, log.data());
            std::cerr << "PROGRAM LINK ERROR:\n" << log << "\n";
        }
    }
};
