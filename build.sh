#!/bin/sh

gcc main.c -o screensaver $(sdl2-config --cflags --libs) -Wall -Wextra
