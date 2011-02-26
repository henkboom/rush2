#ifndef OBJ_H
#define OBJ_H

typedef struct _obj_s obj_s;

obj_s * obj_load(const char *filename);
void obj_release(obj_s *obj);

// the vertex array is three floats per vertex
unsigned obj_get_vertex_count(obj_s *obj);
float * obj_get_vertices(obj_s *obj);

// three elements per triangle
unsigned obj_get_triangle_count(obj_s *obj);
unsigned * obj_get_triangles(obj_s *obj);

#endif
