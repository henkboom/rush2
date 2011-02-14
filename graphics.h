#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <GL/glfw.h>

GLuint graphics_create_shader_from_file(
    GLenum shader_type,
    const char *filename);

GLuint graphics_create_program(int count, GLuint *shaders);

GLuint graphics_make_buffer(
    GLenum target,
    GLsizei size,
    const void *data,
    GLenum usage);


#endif
