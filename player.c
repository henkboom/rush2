#include "player.h"

#include "rhizome/graphics.h"
#include "rhizome/player_input.h"
#include "rhizome/sprite.h"
#include "rhizome/transform.h"
#include "obj.h"
#include "ship.h"

static mesh_s *make_player_mesh()
{
    mesh_s *mesh = mesh_new();

    // shader
    GLuint shaders[2];
    shaders[0] = graphics_create_shader_from_file(
            GL_VERTEX_SHADER, "ship.v.glsl");
    shaders[1] = graphics_create_shader_from_file(
            GL_FRAGMENT_SHADER, "ship.f.glsl");
    mesh_set_program(mesh, graphics_create_program(2, shaders));

    // geometry
    obj_s *obj = obj_load("ship.obj");
    mesh_set_attribute(mesh, "position", 3,
        obj_get_vertex_count(obj) * 3 * sizeof(float),
        obj_get_vertices(obj));
    mesh_set_elements(mesh,
        obj_get_triangle_count(obj) * 3 * sizeof(unsigned),
        obj_get_triangles(obj));
    obj_release(obj);

    return mesh;
}

player_h add_player_component(game_context_s *context, component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_h self = game_get_self(context);
    player_s *player = malloc(sizeof(player_s));
    game_set_component_data(context, player);

    player->component = self;
    player->transform = add_transform_component(context, self,
        vect_zero, quaternion_identity);

    add_sprite_component(context, self, player->transform, make_player_mesh());

    player_input_h player_input = add_player_input_component(context, self);

    ship_h ship = add_ship_component(context, self);
    ship_args_s ship_args = {player->transform, player_input};
    send_ship_init(context, handle_get(ship)->component, ship_args);

    player_h handle;
    game_add_buffer(context,player, sizeof(player_s), (void_h*)&handle);
    return handle;
}

static void release_component(void *data)
{
    free(data);
}
