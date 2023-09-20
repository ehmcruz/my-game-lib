# My Game Library

Still in very early development process.

# Overall dependencies

To compile, you need a C++23 capable compiler and the following libraries:

- SDL
- My-lib (https://github.com/ehmcruz/my-lib). The Makefile is configured to search **my-lib** in the same parent folder as this repository. If you put somewhere else, just modify the Makefile.

---

# Linux Guide

## Compiling in Linux

First, you need to download the following packages (considering you are using Ubuntu):

- libsdl2-dev

Then, to compile:

- For SDL Support only: **make MYGLIB_TARGET_LINUX=1 MYGLIB_SUPPORT_SDL=1**

---

# Windows Guide

Future addition.

---

# Known bugs

Everything.