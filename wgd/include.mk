
CC := wgd/tcc/tcc.exe
AR := wgd/tcc/tcc.exe -ar rc
CLIBS := \
  wgd/raylib/raylib.rc.data \
  wgd/raylib/libraygui.a \
  -lmsvcrt -lraylib -lopengl32 -lgdi32 -lwinmm -lkernel32 -lshell32 -luser32
CFLAGS := -DPLATFORM_DESKTOP -Iwgd $(CLIBS)
