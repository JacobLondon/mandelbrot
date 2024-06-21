#include <stdio.h>

#if defined(_WIN32)
#include <raylib/raylib.h>
#elif defined(__APPLE__)
#include <raylib.h>
#else
#error "Include path no configured for your OS. Please find raylib.h and insert it here.
#endif

static double runtime;
static double plottingtime;
static double conversiontime;

/*
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

static int MAX_ITERATION = 100;
#define MAGNITUDE 4.0l
static void plot(double1 x0, double1 y0, int renderx, int rendery)
{
    const double start = GetTime();
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
        color = (Color){tmp, tmp, tmp, 255};
    }

    DrawPixel(renderx, rendery, color);
    const double end = GetTime();
    plottingtime += (end - start);
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

static int module(int argc, char **argv)
{
    double4 screen = {0, 0, 800, 400};
    double4 view = {0, 0, 4, 2};
    double now;

    InitWindow(screen.width, screen.height, "Mandelbrot");
    const double1 screen_width_half = screen.width / 2.0l;
    const double1 screen_height_half = screen.height / 2.0l;

    int i, j;
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

        // Don't draw then going to infinity, just remain clear
        ClearBackground(BLACK);
        for (i = -screen_height_half; i < screen_height_half; i += 1)
        {
            for (j = -screen_width_half; j < screen_width_half; j += 1)
            {
                // i and j are offset screen coords, remove the offset when drawing
                const double2 coords = window_to_viewport(&screen, &view, j, i);
                plot(coords.x, coords.y, j+(int)screen_width_half, i+(int)screen_height_half);
            }
        }

        DrawFPS(0, 0);

        char buf[32];
        snprintf(buf, sizeof(buf), "%Lf, %Lf", view.x, view.y);
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
