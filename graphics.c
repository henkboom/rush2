#include "graphics.h"

#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define error_on(expr, ...) \
do \
{ \
    if(expr) error_at_line(-1, errno, __FILE__, __LINE__, __VA_ARGS__); \
} while(0)

GLuint graphics_create_shader_from_file(
    GLenum shader_type,
    const char *filename)
{
    FILE *file = fopen(filename, "rb");
    error_on(!file, "create_shader '%s'", filename);
    error_on(fseek(file, 0, SEEK_END) < 0, "create_shader '%s'", filename);
    GLint size = ftell(file);
    error_on(size < 0, "create_shader '%s'", filename);
    rewind(file);

    char *data = malloc(size);
    error_on(fread(data, size, 1, file) < 1, "create_shader '%s'", filename);

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char **)&data, &size);
    glCompileShader(shader);

    GLint log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if(log_length > 1)
    {
        GLchar *log = malloc((log_length+1) * sizeof(GLchar));
        glGetShaderInfoLog(shader, log_length, NULL, log);
        log[log_length] = 0;
        
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
            error(-1, 0, "%s: %s", filename, log);
        else
            printf("%s: %s\n", filename, log);
        free(log);
    }

    return shader;
}

GLuint graphics_create_program(int count, GLuint *shaders)
{
    GLuint program = glCreateProgram();
    
    for(int i = 0; i < count; i++)
        glAttachShader(program, shaders[i]);
    
    glLinkProgram(program);
    
    GLint log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    if(log_length > 1)
    {
        GLchar *log = malloc((log_length+1) * sizeof(GLchar));
        glGetProgramInfoLog(program, log_length, NULL, log);
        log[log_length] = 0;
        
        GLint status;
        glGetProgramiv(program, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
            error(-1, 0, "Link error: %s", log);
        else
            printf("Link message: %s\n", log);
        free(log);
    }
    
    return program;
}

GLuint graphics_make_buffer(
    GLenum target,
    GLsizei size,
    const void *data,
    GLenum usage)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);
    return buffer;
}
