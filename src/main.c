#include <keypadc.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/timers.h>

#include "hid.h"
#include "keyboard_mouse_hid_report_descriptor.h"

int main(void) {
  int usb_result;
  if ((usb_result = orchid_hid_init(ORCHID_KEYBOARD_MOUSE)) != 0) {
    printf("usb init error %u\n", usb_result);
    do
      kb_Scan();
    while (!kb_IsDown(kb_KeyClear));
    return 1;
  }

  while (true) {
    kb_Scan();

    if (kb_IsDown(kb_KeyClear)) {
      break;
    }

    orchid_hid_handle_events();

    bool mouse_move_up = kb_IsDown(kb_KeyUp);
    bool mouse_move_down = kb_IsDown(kb_KeyDown);
    bool mouse_move_left = kb_IsDown(kb_KeyLeft);
    bool mouse_move_right = kb_IsDown(kb_KeyRight);
    bool mouse_wheel_up = kb_IsDown(kb_KeyDel);
    bool mouse_wheel_down = kb_IsDown(kb_KeyStat);
    bool mouse_button_left = kb_IsDown(kb_KeyTrace);
    bool mouse_button_right = kb_IsDown(kb_KeyGraph);
    bool mouse_fast = kb_IsDown(kb_KeyZoom);

    orchid_mouse_hid_report_t mouse_report = {0};
    mouse_report.report_id = orchid_mouse_hid_report_descriptor_id;

    uint8_t mouse_speed = mouse_fast ? 14 : 4;
    uint8_t mouse_wheel_speed = mouse_fast ? 3 : 1;

    if (mouse_move_left != mouse_move_right) {
      mouse_report.dx = mouse_move_left * -mouse_speed + mouse_move_right * +mouse_speed;
    }
    if (mouse_move_up != mouse_move_down) {
      mouse_report.dy = mouse_move_up * -mouse_speed + mouse_move_down * +mouse_speed;
    }
    if (mouse_wheel_up != mouse_wheel_down) {
      mouse_report.wheel = mouse_wheel_down * +mouse_wheel_speed + mouse_wheel_up * -mouse_wheel_speed;
    }

    mouse_report.buttons = (uint8_t)mouse_button_left | ((uint8_t)mouse_button_right << 1);

    int send_result = orchid_hid_send_mouse_input(&mouse_report);
    if (send_result != 0) {
      printf("usb send error %u\n", send_result);
    }

    usleep(20000);  // 20ms
  }

  orchid_hid_deinit();

  return 0;
}

