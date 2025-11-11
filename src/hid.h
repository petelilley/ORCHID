#pragma once

#include <stdbool.h>

typedef enum {
  ORCHID_MOUSE,
  ORCHID_KEYBOARD,
  // TODO: Game controller?
} orchid_hid_device_type_t;

int orchid_hid_init(orchid_hid_device_type_t device_type);
void orchid_hid_deinit(void);

bool orchid_is_usb_connected(void);
void orchid_hid_handle_events(void);

typedef struct orchid_mouse_hid_report orchid_mouse_hid_report_t;
typedef struct orchid_keyboard_hid_report orchid_keyboard_hid_report_t;

int orchid_hid_send_mouse_input(const orchid_mouse_hid_report_t* report);
int orchid_hid_send_keyboard_input(const orchid_keyboard_hid_report_t* report);

