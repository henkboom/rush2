#include "kinect_display.h"

#include <math.h>
#include <GL/glfw.h>

#include "rhizome/renderer.h"
#include "kinect.h"

static void render(const render_context_s *context, void *data);

typedef struct {
    render_job_s render_job;
    GLuint tex;
} kinect_display_s;

component_h add_kinect_display_component(
    game_context_s *context,
    component_h parent)
{
    context = game_add_component(context, parent, release_component);

    kinect_display_s *kinect_display = malloc(sizeof(kinect_display_s));
    game_set_component_data(context, kinect_display);

    kinect_display->render_job.priority = 2;
    kinect_display->render_job.render = render;
    kinect_display->render_job.data = kinect_display;
    kinect_display->tex = 0;

    render_job_h handle;
    game_add_buffer(context, kinect_display, sizeof(kinect_display_s),
                    (void_h*)&handle);

    broadcast_renderer_add_job(context, handle);

    return game_get_self(context);
}

static void release_component(void *data)
{
    free(data);
}

static void render(const render_context_s *context, void *data)
{
    kinect_display_s *kinect_display = data;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ratio = (double)context->width/context->height;
    glOrtho(-ratio, ratio, -1, 1, 1, -1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    if(kinect_display->tex == 0)
    {
        glGenTextures(1, &kinect_display->tex);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, kinect_display->tex);
    kinect_load_texture();
    //TODO remove
    extern double player_speed;
    glColor4d(1, 1, 1, 1-player_speed);
    //glColor4d(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 1); glVertex2d(-4.0/3.0, -1);
    glTexCoord2d(1, 1); glVertex2d( 4.0/3.0, -1);
    glTexCoord2d(1, 0); glVertex2d( 4.0/3.0,  1);
    glTexCoord2d(0, 0); glVertex2d(-4.0/3.0,  1);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    //TODO static...
    static double slope = 0;
    static int stability = 0;
    static int pulsed = 0;
    static int pulse = 0;
    double measured_slope = kinect_get_slope();
    //measured_slope = 0; // TODO remove to use kinect

    if(measured_slope > -1 && measured_slope < 1)
    {
        slope = (slope * 3 + measured_slope) / 4;
        stability = fmin(30, stability + 1);
        float t = fmax(0, (stability-10)/20.0);
        if(stability == 30 && !pulsed)
        {
            pulse = 4;
            pulsed = 1;
        }

        double offset = kinect_get_offset();
        //offset = -0.5; // TODO remove to use kinect

        double normal_colors[2][4] =
            {{0.4, 0, 0.8, 0.2},
             {0.4, 0, 0.8, 0.8}};
        double pulse_colors[2][4] =
            {{0.8, 0, 0.1, 0.6},
             {0.8, 0, 0.4, 1.0}};

        double (*colors)[4] = pulse ? pulse_colors : normal_colors;

        glLineWidth((int)(t*7)+1);
        glBegin(GL_LINE_STRIP);
        {
            glColor4dv(colors[0]);
            glVertex2d(-ratio*t*t, -ratio*t*t*slope - offset);
            glColor4dv(colors[1]);
            glVertex2d(0, -offset);
            glColor4dv(colors[0]);
            glVertex2d(ratio*t*t, ratio*t*t*slope - offset);
        }
        glEnd();
        glLineWidth(1);
        if(pulse > 0) pulse--;
    }
    else
    {
        stability = fmax(0, stability - 5);
        if(stability < 10) pulsed = 0;
        if(stability == 0) slope = 0;
    }

    glEnable(GL_DEPTH_TEST);
}
