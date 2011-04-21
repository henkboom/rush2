#include <assert.h>
#include <error.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glfw.h>
#include <libfreenect.h>

#define GL_TEX_WIDTH 640
#define GL_TEX_HEIGHT 480

// kinect structures
static freenect_context *ctx = NULL;
static freenect_device *dev = NULL;

// threading stuff
static pthread_t thread;
static volatile int thread_running = 0;

static int needs_refresh = 0;

typedef struct {
    GLubyte depth[GL_TEX_WIDTH * GL_TEX_HEIGHT];
    double slope;
    double offset;
} kinect_buffer_s;

pthread_mutex_t front_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
static kinect_buffer_s * volatile front_buffer;
static kinect_buffer_s * volatile back_buffer;

static double dot_product(int n, float *data_x, float *data_y)
{
    double sum = 0;
    for(int i = 0; i < n; i++)
        sum += data_x[i] * data_y[i];
    return sum;
}

static double mean(int n, float *data)
{
    double sum = 0;
    for(int i = 0; i < n; i++)
        sum += data[i];
    return sum / n;
}

static double variance(int n, float *data, double mean)
{
    double result = 0;
    for(int i = 0; i < n; i++)
    {
        result += (mean - data[i]) * (mean - data[i]);
    }
    return result / n;
}

static double standard_deviation(int n, float *data, double mean)
{
    return sqrt(variance(n, data, mean));
}

static double sample_correlation_coefficient(
    int n, float *data_x, float *data_y, double mean_x, double mean_y,
    double stddev_x, double stddev_y)
{
    double numerator = dot_product(n, data_x, data_y) - n * mean_x * mean_y;
    double denominator = (n-1) * stddev_x * stddev_y;
    return numerator/denominator;
}

static void linear_regression(int n, float *data_x, float *data_y,
                                double *slope_out, double *offset_out)
{
    double mean_x = mean(n, data_x);
    double stddev_x = standard_deviation(n, data_x, mean_x);

    double mean_y = mean(n, data_y);
    double stddev_y = standard_deviation(n, data_y, mean_y);

    double scc = sample_correlation_coefficient(n, data_x, data_y, mean_x,
        mean_y, stddev_x, stddev_y);

    *slope_out = scc * stddev_y / stddev_x;
    *offset_out = mean_y - *slope_out * mean_x;
}

static int is_masked(int x, int y)
{
    return x < 100 || x > (640-100) || y > 350;
}

static void depth_cb(freenect_device *dev, void *raw_depth, uint32_t timestamp)
{
    static float last_closest = 0;
    static float data_x[10000];
    static float data_y[10000];

    int count = 0;
    float closest = 0;
    unsigned short *depth = raw_depth;
    double sum = 0;
    kinect_buffer_s *buffer = back_buffer;
    for(int x = 0; x < FREENECT_FRAME_W; x++)
        for(int y = 0; y < FREENECT_FRAME_H; y++)
        {
            int flipped_x = FREENECT_FRAME_W - x - 1;
            float raw = 1 - depth[x + y * FREENECT_FRAME_W]/(float)0x3ff;
            if(raw >= 0.95) raw = 0;

            if(!is_masked(x, y) && closest < raw) closest = raw;
            float pixel = raw / last_closest;
            if(pixel > 1) pixel = 1;
            if(is_masked(x, y) || pixel < 0.9)
                pixel = pixel / 3;
            else
            {
                if(count < 4000)
                {
                    data_x[count] = x-320;
                    data_y[count] = y-240;
                    count++;
                }
            }

            sum += pixel;

            buffer->depth[flipped_x + y * FREENECT_FRAME_W] = pixel * 0xff;
        }
    //printf("count: %d\n", count);
    if(count < 4000)
    {
        linear_regression(count, data_x, data_y,
            &back_buffer->slope, &back_buffer->offset);
        back_buffer->offset /= 240;
    }
    else
    {
        back_buffer->slope = 1000;
    }
    //printf("middle depth: %f\n", (int)depth[640*240+320]/(float)0x3ff);
    //printf("average depth: %lf\n", sum/FREENECT_FRAME_PIX/(float)0x3ff);
    last_closest = fmax(closest, 0.045);
    //printf("closest: %f\n", last_closest);
    //printf("count: %d\n", count);
    //printf("slope: %f\n", back_buffer->slope);
    needs_refresh = 1;
}

static void * kinect_thread_loop(void *arg)
{
    freenect_set_depth_callback(dev, depth_cb);
    freenect_set_depth_format(dev, FREENECT_DEPTH_10BIT);
    freenect_start_depth(dev);

    while(thread_running)
    {
        freenect_process_events(ctx);

        if(needs_refresh)
        {
            needs_refresh = 0;

            pthread_mutex_lock(&front_buffer_mutex);
            kinect_buffer_s *tmp = front_buffer;
            front_buffer = back_buffer;
            back_buffer = tmp;
            pthread_mutex_unlock(&front_buffer_mutex);
        }
    }

    freenect_close_device(dev);
    freenect_shutdown(ctx);
    return NULL;
}

void init_kinect()
{
    if(freenect_init(&ctx, NULL) < 0)
        error(-1, 0, "kinect: error initializing libfreenect");

	//freenect_set_log_level(ctx, FREENECT_LOG_DEBUG);

    int num_devices = freenect_num_devices(ctx);
    printf("kinect count: %d\n", num_devices);
    
    if(num_devices < 1)
    {
        printf("no kinects!\n");
        freenect_shutdown(ctx);
        return;
    }

    if(freenect_open_device(ctx, &dev, 0) < 0)
        error(-1, 0, "kinect: error opening kinect device");

    front_buffer = malloc(sizeof(kinect_buffer_s));
    memset(front_buffer->depth, 0x0, sizeof(front_buffer->depth));
    front_buffer->slope = 0;
    back_buffer = malloc(sizeof(kinect_buffer_s));
    memset(back_buffer->depth, 0x11, sizeof(back_buffer->depth));
    back_buffer->slope = 0;

    thread_running = 1;
    if(pthread_create(&thread, NULL, kinect_thread_loop, NULL))
        error(-1, 0, "kinect: pthread_create failed");
}

void uninit_kinect()
{
    if(thread_running)
    {
        thread_running = 0;
        pthread_join(thread, NULL);
        freenect_shutdown(ctx);
        pthread_mutex_lock(&front_buffer_mutex);
        free(front_buffer);
        free(back_buffer);
        pthread_mutex_unlock(&front_buffer_mutex);
    }
}

void kinect_load_texture()
{
    GLubyte *buffer = NULL;

    pthread_mutex_lock(&front_buffer_mutex);
    if(thread_running)
    {
        buffer = front_buffer->depth;
    }
    else
    {
        static GLubyte dummy_buffer [640*480];
        static int buffer_cleared = 0;
        if(!buffer_cleared)
        {
            buffer_cleared = 1;
            memset(dummy_buffer, 0, 640*480);
        }
        buffer = dummy_buffer;
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 640, 480, 0, GL_ALPHA,
        GL_UNSIGNED_BYTE, buffer);
    pthread_mutex_unlock(&front_buffer_mutex);
}

double kinect_get_slope()
{
    if(thread_running)
    {
        pthread_mutex_lock(&front_buffer_mutex);
        double slope = front_buffer->slope;
        pthread_mutex_unlock(&front_buffer_mutex);
        return slope;
    }
    else
    {
        return 20000;
    }
}

double kinect_get_offset()
{
    if(thread_running)
    {
        pthread_mutex_lock(&front_buffer_mutex);
        double offset = front_buffer->offset;
        pthread_mutex_unlock(&front_buffer_mutex);
        return offset;
    }
    else
    {
        return 20000;
    }
}
