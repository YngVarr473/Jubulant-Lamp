#!/bin/bash

meson setup builddir
meson compile -C builddir
cd builddir && ./Jubulant-Lamp
