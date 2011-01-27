#include "rhizome/main_loop.h"

#include "dummy_scene.h"

int main(void)
{
    enter_main_loop(add_dummy_scene_component);
    return 0;
}
