#include <stdio.h>
#include <raylib/raylib.h>

static void plot(
    long double x0, long double y0, int renderx, int rendery)
{
    long double xtemp;
    long double x = 0;
    long double y = 0;
    size_t iteration = 0;
    const size_t MAX_ITERATION = 100;

    while (( (x*x + y*y) <= 4.0l) && (iteration < MAX_ITERATION)) {
        xtemp = x*x - y*y + x0;
        y = 2.0l*x*y + y0;

        x = xtemp;

        iteration += 1;
    }

    if (iteration == MAX_ITERATION)
    {
        return;
    }

    const int tmp = (int)((float)iteration / (float)MAX_ITERATION * 255.0);
    const Color color = (Color){tmp, tmp, tmp, 255};

    //printf("It: %zu, Color: %d,%d,%d\n", iteration, color.r, color.g, color.b);
    DrawPixel(renderx, rendery, color);
}

/**
 * @returns the view points transformed into base points
 */

typedef struct double2
{
    long double x;
    long double y;
} double2;

typedef struct double4
{
    long double x;
    long double y;
    long double width;
    long double height;
} double4;

typedef long double double1;

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
    return _window_to_viewport(
        winx, window->x+window->width, window->x, viewport->x+viewport->width, viewport->x,
        winy, window->y+window->height, window->y, viewport->y+viewport->height, viewport->y);
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

    InitWindow(screen.width, screen.height, "Mandelbrot");

    int i, j;
    while (!WindowShouldClose())
    {
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

        // Zoom
        const float zooming = GetMouseWheelMove();
        if (zooming != 0.0)
        {
            // Zoom into the viewport centered around the mouse
            const Vector2 mouse = GetMousePosition();

            // Zooming in (scale viewport in)
            if (zooming > 0)
            {
                window_to_viewport_zoom(&screen, &view, 0.75l, mouse.x-screen.width/2, mouse.y-screen.height/2);
            }
            // Zooming out (scale viewpoert out)
            else if (zooming < 0)
            {
                window_to_viewport_zoom(&screen, &view, 1.333l, mouse.x-screen.width/2, mouse.y-screen.height/2);
            }
        }

        // Don't draw then going to infinity, just remain clear
        ClearBackground(BLACK);
        for (i = -screen.height/2; i < screen.height/2; i += 1)
        {
            for (j = -screen.width/2; j < screen.width/2; j += 1)
            {
                // i and j are offset screen coords, remove the offset when drawing
                const double2 coords = window_to_viewport(&screen, &view, j, i);
                plot(coords.x, coords.y, j+screen.width/2, i+screen.height/2);
            }
        }

        DrawFPS(0, 0);

        char buf[32];
        snprintf(buf, sizeof(buf), "%Lf, %Lf", view.x, view.y);
        DrawText(buf, 0, screen.height-16, 16, WHITE);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
