# ----------------------------
# Makefile Options
# ----------------------------

NAME = ORCHID
# ICON = icon.png
DESCRIPTION = "Outrageously Repurposed Calculator Human Interface Device"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -pedantic -Werror  -Oz
CXXFLAGS = -Wall -Wextra -pedantic -Werror  -Oz

# ----------------------------

include $(shell cedev-config --makefile)
