#include "main_scene.h"

#include "rhizome/input_handler.h"
#include "rhizome/renderer.h"
#include "camera.h"
#include "player.h"
#include "terrain_renderer.h"

begin_component(main_scene);
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

static void release(void *data)
{
}
