#include <stdio.h>
#include <math.h>

#if defined(_WIN32)
#include <raylib/raylib.h>
#elif defined(__APPLE__)
#include <raylib.h>
#else
#error "Include path no configured for your OS. Please find raylib.h and insert it here.
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/*
Method 1

long double
Iterations: 194
Runtime: 47.879351
Plotting time: 36.361787s (75.944611 %) | 0.187432 s/it
Conversion time: 3.660784s (7.645852 %) | 0.018870 s/it

double
Iterations: 214
Runtime: 51.299721s
Plotting time: 38.637908s (75.317970 %) | 0.180551 s/it
Conversion time: 4.040135s (7.875550 %) | 0.018879 s/it
*/
static double runtime;
static double plottingtime;
static double conversiontime;

/*
@TODO: Add fancier colors
@TODO: Improve performance
@TODO: Why calculate viewport for every viewport pixel? Can't we just get the corners?
*/

typedef long double double1;

typedef struct double2
{
    double1 x;
    double1 y;
} double2;

typedef struct double4
{
    double1 x;
    double1 y;
    double1 width;
    double1 height;
} double4;

static Color color_gradient[256];

static int MAX_ITERATION = 100;
#define MAGNITUDE 4.0l
static void plot(double1 x0, double1 y0, int renderx, int rendery)
{
    double1 x = 0;
    double1 y = 0;
    double1 x2, y2;
    int iteration = 0;

    while ((iteration < MAX_ITERATION) &&
           ( ( (x2=x*x) + (y2=y*y) ) <= MAGNITUDE)
          )
    {
        y = 2.0l*x*y + y0;
        x = x2 - y2 + x0;

        iteration += 1;
    }

    Color color = BLACK;
    if (iteration < MAX_ITERATION)
    {
        const int tmp = (int)( (float)iteration / (float)MAX_ITERATION * 255.0f );
        //color = (Color){tmp, tmp, tmp, 255};
        color = color_gradient[tmp];
    }

    DrawPixel(renderx, rendery, color);
}

static void plot2(double1 x0, double1 y0, Color *color)
{
    double1 x = 0;
    double1 y = 0;
    double1 x2, y2;
    int iteration = 0;

    while ((iteration < MAX_ITERATION) &&
           ( ( (x2=x*x) + (y2=y*y) ) <= MAGNITUDE)
          )
    {
        y = 2.0l*x*y + y0;
        x = x2 - y2 + x0;

        iteration += 1;
    }

    *color = BLACK;
    if (iteration < MAX_ITERATION)
    {
        const int tmp = (int)( (float)iteration / (float)MAX_ITERATION * 255.0f );
        //*color = (Color){tmp, tmp, tmp, 255};
        *color = color_gradient[tmp];
    }
}

static double2 _window_to_viewport(
    const double1 x_w, const double1 x_wmax, const double1 x_wmin, const double1 x_vmax, const double1 x_vmin,
    const double1 y_w, const double1 y_wmax, const double1 y_wmin, const double1 y_vmax, const double1 y_vmin)
{
    // calculating the point on viewport
    return (double2){                   // scaling factors for x coordinate and y coordinates
        .x = x_vmin + ((x_w - x_wmin) * (x_vmax - x_vmin) / (x_wmax - x_wmin)),
        .y = y_vmin + ((y_w - y_wmin) * (y_vmax - y_vmin) / (y_wmax - y_wmin)),
    };
}

static double2 window_to_viewport(const double4 *window, const double4 *viewport, const double1 winx, const double1 winy)
{
    const double start = GetTime();
    const double2 res = _window_to_viewport(
        winx, window->x+window->width, window->x, viewport->x+viewport->width, viewport->x,
        winy, window->y+window->height, window->y, viewport->y+viewport->height, viewport->y);
    const double end = GetTime();
    conversiontime += (end - start);
    return res;
}

static void window_to_viewport_zoom(const double4 *window, double4 *viewport, const double1 scale, const double1 winx, const double1 winy)
{
    const double2 xform_viewport = window_to_viewport(window, viewport, winx, winy);

    viewport->width *= scale;
    viewport->height *= scale;

    const double2 new_viewport = window_to_viewport(window, viewport, winx, winy);

    // The algorithm is to zoom and to offset the top right by adding (old - new)
    viewport->x += xform_viewport.x - new_viewport.x;
    viewport->y += xform_viewport.y - new_viewport.y;
}

static void load_color_gradient(Color gradient[256], unsigned start, unsigned end)
{
    double color_start, color_end;
    double color_width;
    int i;

    const unsigned sr = (start & 0xFF0000) >> 16;
    const unsigned sg = (start & 0x00FF00) >> 8;
    const unsigned sb = (start & 0x0000FF);
    const unsigned er = (end   & 0xFF0000) >> 16;
    const unsigned eg = (end   & 0x00FF00) >> 8;
    const unsigned eb = (end   & 0x0000FF);

    color_width = ((double)er - (double)sr) / 256.0;
    for (i = 0; i < 256; i++)
    {
        gradient[i].r = round(sr + color_width * i);
        gradient[i].a = 255;
    }

    color_width = ((double)eg - (double)sg) / 256.0;
    for (i = 0; i < 256; i++)
    {
        gradient[i].g = round(sg + color_width * i);
    }

    color_width = ((double)eb - (double)sb) / 256.0;
    for (i = 0; i < 256; i++)
    {
        gradient[i].b = round(sb + color_width * i);
    }
}

static int module(int argc, char **argv)
{
    double4 screen = {0, 0, 800, 400};
    double4 view = {0, 0, 4, 2};
    double now;

    //load_color_gradient(color_gradient, 0x000000, 0xFFFFFF);
    //load_color_gradient(color_gradient, 0x0ABFBC, 0xFC354C);
    //load_color_gradient(color_gradient, 0xFC354C, 0x0ABFBC);
    load_color_gradient(color_gradient, 0x3D7EAA, 0xFFE47A);

    InitWindow(screen.width, screen.height, "Mandelbrot");
    const double1 screen_width_half = screen.width / 2.0l;
    const double1 screen_height_half = screen.height / 2.0l;

    int i, j, k;
    const double start = GetTime();
    size_t iterations = 0;
    while (!WindowShouldClose())
    {
        now = GetTime();
        BeginDrawing();

        // Pan
        if (IsMouseButtonDown(0))
        {
            const Vector2 delta = GetMouseDelta();
            // Convert our screen delta into view delta
            view.x -= delta.x / screen.height * view.height;
            view.y -= delta.y / screen.height * view.height;
        }

        // Reset pan
        if (IsKeyDown(KEY_SPACE))
        {
            view = (double4){0, 0, 4, 2};
        }

        if (IsKeyDown(KEY_RIGHT_BRACKET))
        {
            MAX_ITERATION += 1;
        }
        if (IsKeyDown(KEY_LEFT_BRACKET))
        {
            MAX_ITERATION -= 1;
        }

        // Zoom
        const float zooming = GetMouseWheelMove();
        if (zooming != 0.0)
        {
            // Zoom into the viewport centered around the mouse
            const Vector2 mouse = GetMousePosition();

            // Zooming in (scale viewport in)
            if (zooming > 0)
            {
                window_to_viewport_zoom(&screen, &view, 0.75l, mouse.x-screen_width_half, mouse.y-screen_height_half);
            }
            // Zooming out (scale viewpoert out)
            else if (zooming < 0)
            {
                window_to_viewport_zoom(&screen, &view, 1.333l, mouse.x-screen_width_half, mouse.y-screen_height_half);
            }
        }

    #if 0
        // Method 1: per-pixel viewport calculation
        /*
        `Iterations: 342
        Runtime: 31.287068s
        Iterations/s: 10.931034
        Plotting time: 30.850403s (98.604329 %) | 0.090206 s/it
        Conversion time: 1.359463s (4.345128 %) | 0.003975 s/it
        */
        const double start = GetTime();
        for (i = -screen_height_half; i < screen_height_half; i += 1)
        {
            for (j = -screen_width_half; j < screen_width_half; j += 1)
            {
                // i and j are offset screen coords, remove the offset when drawing
                const double2 coords = window_to_viewport(&screen, &view, j, i);
                plot(coords.x, coords.y, j+(int)screen_width_half, i+(int)screen_height_half);
            }
        }
        const double end = GetTime();
        plottingtime += (end - start);
    #elif 0
        // Method 2: Calculate view port at corners
        /*
        Iterations: 348
        Runtime: 29.323096s
        Iterations/s: 11.867778
        Plotting time: 28.890165s (98.523584 %) | 0.083018 s/it
        Conversion time: 0.000014s (0.000048 %) | 0.000000 s/it
        */
        const double start = GetTime();

        const double2 topleft     = window_to_viewport(&screen, &view, -screen_width_half, -screen_height_half);
        const double2 bottomright = window_to_viewport(&screen, &view, screen_width_half, screen_height_half);
        const double1 width       = (bottomright.x - topleft.x) / screen.width;
        const double1 height      = (bottomright.y - topleft.y) / screen.height;

        for (i = 0; i < screen.height; i += 1)
        {
            for (j = 0; j < screen.width; j += 1)
            {
                plot(j*width + topleft.x, i*height + topleft.y, j, i);
            }
        }
        const double end = GetTime();
        plottingtime += (end - start);
    #elif 1
        // Method 3: Attempt grouping calculations together
        /*
        Iterations: 441
        Runtime: 33.526224s
        Iterations/s: 13.153882
        Plotting time: 32.947251s (98.273074 %) | 0.074710 s/it
        Conversion time: 0.000017s (0.000052 %) | 0.000000 s/it
        */
        const double start = GetTime();

        const double2 topleft     = window_to_viewport(&screen, &view, -screen_width_half, -screen_height_half);
        const double2 bottomright = window_to_viewport(&screen, &view, screen_width_half, screen_height_half);
        const double1 width       = (bottomright.x - topleft.x) / screen.width;
        const double1 height      = (bottomright.y - topleft.y) / screen.height;
        static Color colors[800];

        for (i = 0; i < screen.height; i += 1)
        {
            const double1 y0 = i*height + topleft.y;
            for (j = 0; j < screen.width; j += 1)
            {
                const double1 x0 = j*width + topleft.x;
                plot2(x0, y0, &colors[j]);
            }

            for (j = 0; j < screen.width; j += 1)
            {
                DrawPixel(j, i, colors[j]);
            }
        }
        const double end = GetTime();
        plottingtime += (end - start);
    #endif

        DrawFPS(0, 0);

        char buf[64];
        snprintf(buf, sizeof(buf), "%+01.020Lf, %+30.020Lf", view.x, view.y);
        DrawText(buf, 0, screen.height-16, 16, WHITE);
        snprintf(buf, sizeof(buf), "Iterations: %d", MAX_ITERATION);
        DrawText(buf, 0, screen.height-32, 16, WHITE);
        EndDrawing();

        iterations++;
    }
    const double end = GetTime();
    runtime = (end - start);
    CloseWindow();

    printf("Iterations: %zu\n", iterations);
    printf("Runtime: %lfs\n", runtime);
    printf("Iterations/s: %lf\n", (double)iterations / runtime);
    printf("Plotting time: %lfs (%lf %%) | %lf s/it\n",
        plottingtime,
        plottingtime/runtime*100.0,
        plottingtime/iterations);
    printf("Conversion time: %lfs (%lf %%) | %lf s/it\n",
        conversiontime,
        conversiontime/runtime*100.0,
        conversiontime/iterations);

    return 0;
}
