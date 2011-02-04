#include "rhizome/main_loop.h"

#include "main_scene.h"

int main(void)
{
    enter_main_loop(add_main_scene_component);
    return 0;
}
