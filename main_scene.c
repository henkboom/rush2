#include "main_scene.h"

#include <GL/glfw.h>

#include "rhizome/input_handler.h"
#include "rhizome/renderer.h"
#include "rhizome/vect.h"
#include "player.h"
#include "player_camera.h"
#include "terrain_manager.h"

component_h add_main_scene_component(
    game_context_s *context,
    component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_subscribe(context, input_handler_close_event)
    component_subscribe(context, input_handler_key_event)

    component_h self = game_get_self(context);

    add_input_handler_component(context, self);
    add_renderer_component(context, self);

    player_h player = add_player_component(context, self);
    camera_h camera = add_camera_component(context, self,
        add_transform_component(context, self, vect_zero, quaternion_identity));
    add_player_camera_component(context, self,
        camera, handle_get(player)->transform);

    add_terrain_manager_component(context, self, handle_get(player)->transform);

    return self;
}

static void handle_input_handler_close_event(
    game_context_s *context,
    void *data,
    const nothing_s *n)
{
    game_remove_component(context, game_get_self(context));
}

static void handle_input_handler_key_event(
    game_context_s *context,
    void *data,
    const key_event_s *event)
{
    if(event->key == GLFW_KEY_ESC && event->is_down)
        game_remove_component(context, game_get_self(context));
}

static void release_component(void *data)
{
}
