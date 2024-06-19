import raylib
from pyray import *


def plot(x0, y0, scalex, scaley, offx, offy):
    x, y = 0, 0
    iteration = 0
    max_iteration = 1000

    while x*x + y*y <= (2*2) and iteration < max_iteration:
        xtemp = x*x - y*y + x0
        y = 2*x*y + y0

        x = xtemp

        iteration = iteration + 1

    if iteration == max_iteration:
        color = raylib.BLACK
    else:
        tmp = int(iteration / max_iteration * 255)
        color = (tmp, tmp, tmp, 255)

    draw_pixel(int(x0*scalex+offx), int(y0*scaley+offy), color)

def main():
    init_window(800, 400, "Mandelbrot")
    while not window_should_close():
        begin_drawing()
        clear_background(raylib.WHITE)
        for i in range(-200, 200, 1):
            for j in range(-400, 400, 1):
                plot(j/200.0, i/200.0, 200, 200, 400, 200)
        end_drawing()
    close_window()

if __name__ == '__main__':
    main()
