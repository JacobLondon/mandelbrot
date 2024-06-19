cp raygui.h raygui.c
../tcc/tcc.exe raygui.c -DRAYGUI_IMPLEMENTATION -DBUILD_LIBTYPE_SHARED -c -o raygui.o
../tcc/tcc.exe -ar rc libraygui.a raygui.o

