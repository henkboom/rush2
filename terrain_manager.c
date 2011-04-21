#include "terrain_manager.h"

#include <math.h>

#include "rhizome/array.h"
#include "terrain_renderer.h"

#define GRID_SIZE 11
#define SQRT3 1.7320508075689

typedef struct {
    int i;
    int j;
    component_h renderer;
} renderer_record_s;

typedef struct {
    transform_h center;
    array_of(renderer_record_s) renderers;
} terrain_manager_s;

void refresh_array(
    game_context_s *context, terrain_manager_s *terrain_manager);

component_h add_terrain_manager_component(
    game_context_s *context,
    component_h parent,
    transform_h center)
{
    context = game_add_component(context, parent, release_component);

    component_subscribe(context, tick);

    component_h self = game_get_self(context);
    terrain_manager_s *terrain_manager = malloc(sizeof(terrain_manager_s));
    game_set_component_data(context, terrain_manager);

    terrain_manager->center = center;
    terrain_manager->renderers = array_new();
    array_set_length(terrain_manager->renderers, GRID_SIZE*GRID_SIZE);
    for(int i = 0; i < GRID_SIZE*GRID_SIZE; i++)
    {
        renderer_record_s *record =
            array_get_ptr(terrain_manager->renderers) + i;
        handle_reset(&(record->renderer));
    }
    refresh_array(context, terrain_manager);

    return self;
}

void release_component(void *data)
{
    terrain_manager_s *terrain_manager = data;
    array_release(terrain_manager->renderers);
    free(terrain_manager);
}

void handle_tick(game_context_s *context, void *data, const nothing_s *n)
{
    refresh_array(context, (terrain_manager_s*) data);
}

void refresh_array(game_context_s *context, terrain_manager_s *terrain_manager)
{
    const transform_s *transform = handle_get(terrain_manager->center);

    int i_center = floor(transform->pos.x/TERRAIN_BLOCK_WIDTH -
                         transform->pos.z/(TERRAIN_BLOCK_WIDTH * SQRT3/2)/2);
    int j_center = floor(transform->pos.z/(TERRAIN_BLOCK_WIDTH * SQRT3/2));
    int i_min = i_center - GRID_SIZE/2;
    int i_max = i_min + GRID_SIZE;
    int j_min = j_center - GRID_SIZE/2;
    int j_max = j_min + GRID_SIZE;

    for(int j = j_min; j < j_max; j++)
    {
        for(int i = i_min; i < i_max; i++)
        {
            //TODO
            int ii = i;
            while(ii < 0) ii+=GRID_SIZE; while(ii >= GRID_SIZE) ii-=GRID_SIZE;
            int jj = j;
            while(jj < 0) jj+=GRID_SIZE; while(jj >= GRID_SIZE) jj-=GRID_SIZE;
            renderer_record_s *record =
                array_get_ptr(terrain_manager->renderers) + (ii+jj*GRID_SIZE);

            if(!handle_live(record->renderer)
                || record->i != i || record->j != j)
            {
                if(handle_live(record->renderer))
                    game_remove_component(context, record->renderer);
                record->i = i;
                record->j = j;
                record->renderer = add_terrain_renderer_component(
                    context,
                    game_get_self(context),
                    make_vect((i + j * 0.5) * TERRAIN_BLOCK_WIDTH,
                              0,
                              j * SQRT3/2 * TERRAIN_BLOCK_WIDTH));
            }
        }
    }
}
