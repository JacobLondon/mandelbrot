include wgd/include.mk

CFLAGS := $(CFLAGS) \
  -ggdb -std=c99 \
  -Wextra -Wall

RM_F = rm -rf
TRASH = *.o *.a *.so *.exe *.out

TARGET_T = far.exe
TARGET_C := $(wildcard *.c)
TARGET_H := $(wildcard *.h)

.PHONY: $(TARGET_T) clean

$(TARGET_T): $(TARGET_C) $(TARGET_H)
	$(CC) -o $@ build.c $(CFLAGS)

clean:
	$(RM_F) $(TRASH) $(TARGET_T)
