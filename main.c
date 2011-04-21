#include "rhizome/main_loop.h"

#include "kinect.h"
#include "main_scene.h"

int main(void)
{
    init_kinect();
    prepare_main_loop();
    enter_main_loop(add_main_scene_component);
    shutdown_main_loop();
    uninit_kinect();
    return 0;
}
