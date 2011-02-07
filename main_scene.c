#include "main_scene.h"

#include <GL/glfw.h>

#include "rhizome/input_handler.h"
#include "rhizome/renderer.h"
#include "camera.h"
#include "player.h"
#include "terrain_renderer.h"

begin_component(main_scene);
    component_subscribe(input_handler_close_event)
    component_subscribe(input_handler_key_event)
end_component();

static component_h init(game_context_s *context)
{
    component_h self = game_get_self(context);

    add_input_handler_component(context, self);
    add_renderer_component(context, self);
    add_camera_component(context, self);
    add_terrain_renderer_component(context, self);
    add_player_component(context, self);

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

static void release(void *data)
{
}
