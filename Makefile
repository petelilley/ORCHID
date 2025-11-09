# ----------------------------
# Makefile Options
# ----------------------------

NAME = ORCHID
# ICON = icon.png
DESCRIPTION = "Outrageously Repurposed Calculator Human Interface Device"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -pedantic -Werror -Wno-gnu-binary-literal -Wno-gnu-flexible-array-initializer -Wno-unused-variable -Oz

# ----------------------------

include $(shell cedev-config --makefile)
