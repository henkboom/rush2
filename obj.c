#include "obj.h"

#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rhizome/array.h"

#define error_on(expr, format, ...) \
do \
{ \
    if(expr) \
        error_at_line(-1, errno, __FILE__, __LINE__, \
                      "%s " format, __func__, __VA_ARGS__); \
} while(0)


struct _obj_s {
    array_of(float) vertices;
    array_of(unsigned) elements;
};

static void skip_until_ws(FILE *file)
{
    int c;
    do
    {
        c = getc(file);
    } while(!isspace(c) && c != EOF);
}

static void skip_until_char(FILE *file, int dest)
{
    int c;
    do
    {
        c = getc(file);
    } while(c != dest && c != EOF);
}

obj_s *obj_load(const char *filename)
{
    char keyword[16];
    int matches;

    obj_s *obj = malloc(sizeof(obj_s));
    obj->vertices = array_new();
    obj->elements = array_new();

    FILE *file = fopen(filename, "rb");
    error_on(!file, "'%s'", filename);

    while(1)
    {
        matches = fscanf(file, "%15s", keyword);

        if(matches != 1)
            break;

        if(strcmp(keyword, "v") == 0)
        {
            float x, y, z;
            matches = fscanf(file, "%f %f %f", &x, &y, &z);
            if(matches != 3)
                error(-1, 0, "error parsing vertex %lu",
                      array_length(obj->vertices) + 1);
            array_add(obj->vertices, x);
            array_add(obj->vertices, y);
            array_add(obj->vertices, z);
            //printf("v %f %f %f\n", x, y, z);
            skip_until_char(file, '\n');
        }
        else if(strcmp(keyword, "f") == 0)
        {
            unsigned v[3];
            for(int i = 0; i < 3; i++)
            {
                matches = fscanf(file, "%u", &v[i]);
                skip_until_ws(file);
                //printf("i=%d, %u\n", i, v[i]);
                if(matches != 1) error(-1, 0, "error parsing face");
            }
            do
            {
                //printf("face %u %u %u\n", v[0], v[1], v[2]);
                array_add(obj->elements, v[0]-1);
                array_add(obj->elements, v[1]-1);
                array_add(obj->elements, v[2]-1);
                v[1] = v[2];
                matches = fscanf(file, "%u", &v[2]);
                //printf("matches %d %u\n", matches, v[2]);
                if(matches == 1) skip_until_ws(file);
            } while(matches == 1);
            //printf("done face\n");
        }
        else
        {
            //printf("skipping '%s'\n", keyword);
            skip_until_char(file, '\n');
        }
    }

    return obj;
}

void obj_release(obj_s *obj)
{
    array_release(obj->vertices);
    array_release(obj->elements);
    free(obj);
}

unsigned obj_get_vertex_count(obj_s *obj)
{
    return array_length(obj->vertices) / 3;
}

float * obj_get_vertices(obj_s *obj)
{
    return array_get_ptr(obj->vertices);
}

unsigned obj_get_triangle_count(obj_s *obj)
{
    return array_length(obj->elements) / 3;
}

unsigned * obj_get_triangles(obj_s *obj)
{
    return array_get_ptr(obj->elements);
}
