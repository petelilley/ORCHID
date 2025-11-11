#include <keypadc.h>
#include <ti/getcsc.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/timers.h>
#include <string.h>

#include "hid.h"
#include "keyboard_hid_report_descriptor.h"
#include "mouse_hid_report_descriptor.h"

#include "usb_hid_keys.h"

uint8_t get_hid_keycode(uint8_t sk_key, bool send_letters) {
  if (send_letters) {
    switch (sk_key) {
      case sk_Math:
        return KEY_A;
      case sk_Apps:
        return KEY_B;
      case sk_Prgm:
        return KEY_C;
      case sk_Recip:
        return KEY_D;
      case sk_Sin:
        return KEY_E;
      case sk_Cos:
        return KEY_F;
      case sk_Tan:
        return KEY_G;
      case sk_Power:
        return KEY_H;
      case sk_Square:
        return KEY_I;
      case sk_Comma:
        return KEY_J;
      case sk_LParen:
        return KEY_K;
      case sk_RParen:
        return KEY_L;
      case sk_Div:
        return KEY_M;
      case sk_Log:
        return KEY_N;
      case sk_7:
        return KEY_O;
      case sk_8:
        return KEY_P;
      case sk_9:
        return KEY_Q;
      case sk_Mul:
        return KEY_R;
      case sk_Ln:
        return KEY_S;
      case sk_4:
        return KEY_T;
      case sk_5:
        return KEY_U;
      case sk_6:
        return KEY_V;
      case sk_Sub:
        return KEY_W;
      case sk_Store:
        return KEY_X;
      case sk_1:
        return KEY_Y;
      case sk_2:
        return KEY_Z;
      // Other alpha keys
      case sk_0:
        return KEY_SPACE;
      case sk_DecPnt:
        return KEY_SEMICOLON;  // :
      case sk_Chs:
        return KEY_SLASH;  // ?
      case sk_Add:
        return KEY_APOSTROPHE;  // "

      case sk_Up:
        return KEY_UP;
      case sk_Down:
        return KEY_DOWN;
      case sk_Left:
        return KEY_LEFT;
      case sk_Right:
        return KEY_RIGHT;
    }
  } else {
    switch (sk_key) {
      case sk_Comma:
        return KEY_COMMA;
      case sk_LParen:
        return KEY_9; // (
      case sk_RParen:
        return KEY_0;  // )
      case sk_Div:
        return KEY_SLASH;
      case sk_7:
        return KEY_7;
      case sk_8:
        return KEY_8;
      case sk_9:
        return KEY_9;
      case sk_Mul:
        return KEY_8;
      case sk_4:
        return KEY_4;
      case sk_5:
        return KEY_5;
      case sk_6:
        return KEY_6;
      case sk_Sub:
        return KEY_MINUS;
      case sk_1:
        return KEY_1;
      case sk_2:
        return KEY_2;
      case sk_0:
        return KEY_0;
      case sk_DecPnt:
        return KEY_DOT;
      case sk_Chs:
        return KEY_MINUS;
      case sk_Add:
        return KEY_EQUAL;
      case sk_Power:
        return KEY_6; // ^
    }
  }

  switch (sk_key) {
    case sk_Enter:
      return KEY_ENTER;
    case sk_Del:
      return KEY_BACKSPACE;
  }

  return KEY_NONE;
}

uint8_t get_hid_mod_keycode(uint8_t sk_key) {
  switch (sk_key) {
    case sk_2nd:
      return KEY_MOD_LSHIFT;
    case sk_Alpha:
      return KEY_MOD_LCTRL;
    case sk_Mode:
      return KEY_MOD_LALT;
    case sk_GraphVar:
      return KEY_MOD_LMETA;
  }

  return KEY_NONE;
}

void set_sk_key(uint8_t key, orchid_keyboard_hid_report_t* report, int n, bool send_letters) {
  // Modifier keys
  uint8_t hid_mod_keycode = get_hid_mod_keycode(key);
  if (hid_mod_keycode != KEY_NONE) {
    printf("mod key: %u\n", hid_mod_keycode);
    report->modifiers |= hid_mod_keycode;
    return;
  }

  uint8_t hid_keycode = get_hid_keycode(key, send_letters);
    printf("key: %u\n", hid_keycode);
  if (hid_keycode == KEY_NONE)
    return;

  report->keycodes[n] = hid_keycode;
}

static orchid_hid_device_type_t s_device_type = ORCHID_MOUSE;

int swap_devices(void) {
  s_device_type = (s_device_type == ORCHID_KEYBOARD) ? ORCHID_MOUSE : ORCHID_KEYBOARD;

  orchid_hid_deinit();

  int usb_result;
  if ((usb_result = orchid_hid_init(s_device_type)) != 0) {
    printf("usb init error %u\n", usb_result);
    do
      kb_Scan();
    while (!kb_IsDown(kb_KeyClear));
    return 1;
  }
  return 0;
}

int main(void) {
  int usb_result;
  if ((usb_result = orchid_hid_init(s_device_type)) != 0) {
    printf("usb init error %u\n", usb_result);
    do
      kb_Scan();
    while (!kb_IsDown(kb_KeyClear));
    return 1;
  }

  uint8_t last_kb_Data[8] = {0xFF};
  bool quit = false;
  while (!quit) {
    kb_Scan();

    orchid_hid_handle_events();

    bool were_keys_changed = false;
    for (uint8_t i = 0; i < 8; i++) {
      if (last_kb_Data[i] != kb_Data[i]) {
        were_keys_changed = true;
      }
      last_kb_Data[i] = kb_Data[i];
    }

    if (were_keys_changed) {
      static bool was_should_swap_devices = false;
      bool should_swap_devices = kb_IsDown(kb_KeyWindow);

      if (should_swap_devices && !was_should_swap_devices) {
        bool r = swap_devices();
        if (r != 0)
          return 1;
      }
      was_should_swap_devices = should_swap_devices;

      //
      // Keyboard input
      //

      orchid_keyboard_hid_report_t keyboard_report = {0};
      keyboard_report.report_id = ORCHID_KEYBOARD_HID_REPORT_DESCRIPTOR_ID;

      bool send_letters = !kb_IsDown(kb_KeyYequ);

      int n = 0;
      for (uint8_t key = 1, byte = 7; byte; --byte) {
        for (uint8_t mask = 1; mask; mask <<= 1, ++key) {
          if (kb_Data[byte] & mask) {
            if (key == sk_Clear) {
              quit = true;
              break;
            }

            set_sk_key(key, &keyboard_report, n++, send_letters);
            if (n >= 6)
              goto rollover;
          }
        }
      }
    rollover:;

      int send_result = orchid_hid_send_keyboard_input(&keyboard_report);
      if (send_result != 0) {
        printf("K usb send error %u\n", send_result);
      }
    }

    //
    // Mouse input
    //

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
    mouse_report.report_id = ORCHID_MOUSE_HID_REPORT_DESCRIPTOR_ID;

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
      printf("M usb send error %u\n", send_result);
    }

    usleep(10000);  // 10ms
  }

  orchid_hid_deinit();

  return 0;
}

