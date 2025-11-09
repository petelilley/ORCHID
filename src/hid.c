#include "hid.h"
#include "keyboard_mouse_hid_report_descriptor.h"
#include <usbdrvce.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct hid_descriptor {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdHID;
  uint8_t bCountryCode;
  uint8_t bNumDescriptors;
  uint8_t bDescriptorType2;
  uint16_t wDescriptorLength;
} hid_descriptor_t;

static bool s_was_usb_initialized = false;
static usb_device_t s_active_usb_device;
static bool s_is_usb_connected = false;
static orchid_hid_device_type_t s_device_type;

// Checks if the setup event data is a valid Get_Descriptor Request
static bool is_get_descriptor_report_request(usb_control_setup_t* setup_event_data) {
  // bmRequestType
  bool request_type_is_expected = (setup_event_data->bmRequestType == 0b10000001);
  // bRequest
  bool request_is_expected = (setup_event_data->bRequest == 0x06);  // GET_DESCRIPTOR
  // wValue
  bool descriptor_type_is_expected = ((setup_event_data->wValue >> 8) == 0x22);     // Report
  bool descriptor_index_is_expected = ((setup_event_data->wValue & 0xFF) == 0x00);  // Index 0
  // wIndex
  bool interface_is_expected = (setup_event_data->wIndex == 0x00);  // Interface 0

  // Don't care about Descriptor Length (wLength)

  return request_type_is_expected && request_is_expected && descriptor_type_is_expected &&
         descriptor_index_is_expected && interface_is_expected;
}

static bool is_get_descriptor_hid_request(usb_control_setup_t* setup_event_data) {
  // bmRequestType
  bool request_type_is_expected = (setup_event_data->bmRequestType == 0b10000001);
  // bRequest
  bool request_is_expected = (setup_event_data->bRequest == 0x06);  // GET_DESCRIPTOR
  // wValue
  bool descriptor_type_is_expected = ((setup_event_data->wValue >> 8) == 0x21);     // HID
  bool descriptor_index_is_expected = ((setup_event_data->wValue & 0xFF) == 0x00);  // Index 0
  // wIndex
  bool interface_is_expected = (setup_event_data->wIndex == 0x00);  // Interface 0

  // Don't care about Descriptor Length (wLength)

  return request_type_is_expected && request_is_expected && descriptor_type_is_expected &&
         descriptor_index_is_expected && interface_is_expected;
}

static void* get_report_descriptor_data(void) {
  switch (s_device_type) {
    case ORCHID_KEYBOARD_MOUSE:
      return (void*)orchid_mouse_keyboard_hid_report_descriptor;
    default:
      return NULL;
  }
}

static uint16_t get_report_descriptor_length(void) {
  switch (s_device_type) {
    case ORCHID_KEYBOARD_MOUSE:
      return sizeof(orchid_mouse_keyboard_hid_report_descriptor);
    default:
      return 0;
  }
}

static usb_error_t handle_usb_event(usb_event_t event, void* eventData, usb_callback_data_t* callback_data) {
  (void)callback_data;

  usb_error_t result = USB_SUCCESS;

  s_active_usb_device = usb_FindDevice(NULL, NULL, USB_SKIP_HUBS);

  if (event == USB_DEFAULT_SETUP_EVENT) {
    usb_control_setup_t* setup_event_data = (usb_control_setup_t*)eventData;
    if (is_get_descriptor_report_request(setup_event_data)) {
      s_is_usb_connected = true;

      void* report_descriptor_data = get_report_descriptor_data();
      uint16_t report_descriptor_length = get_report_descriptor_length();

      result = usb_ScheduleTransfer(usb_GetDeviceEndpoint(s_active_usb_device, 0), report_descriptor_data,
                                    report_descriptor_length, NULL, NULL);
      // TODO: Display error
      printf("%u\n", result);  // Doesn't work without this???
    } else if (is_get_descriptor_hid_request(setup_event_data)) {
      result = usb_ScheduleTransfer(usb_GetDeviceEndpoint(s_active_usb_device, 0), NULL, 0, NULL, NULL);
      // TODO: Display error
    }
  } else if (event == USB_DEVICE_CONNECTED_EVENT) {
    s_is_usb_connected = true;
  } else if (event == USB_DEVICE_DISCONNECTED_EVENT) {
    s_is_usb_connected = false;
  }

  return result;
}

static usb_configuration_descriptor_t* init_configuration_descriptor(void) {
  static struct usb_configuration_descriptor_set {
    usb_configuration_descriptor_t configuration;
    struct {
      usb_interface_descriptor_t interface;
      hid_descriptor_t hid;
      usb_endpoint_descriptor_t endpoints[1];
    } interface0;
  } configuration1 = {
      .configuration =
          {
              .bLength = sizeof(configuration1.configuration),
              .bDescriptorType = USB_CONFIGURATION_DESCRIPTOR,
              .wTotalLength = sizeof(configuration1),
              .bNumInterfaces = 1,
              .bConfigurationValue = 1,
              .iConfiguration = 0,
              .bmAttributes = 0b10100000,  // Bus Powered, Remote Wakeup
              .bMaxPower = 0x0,            // 0xFF
          },
      .interface0 =
          {
              .interface =
                  {
                      .bLength = sizeof(configuration1.interface0.interface),
                      .bDescriptorType = USB_INTERFACE_DESCRIPTOR,
                      .bInterfaceNumber = 0,
                      .bAlternateSetting = 0,
                      .bNumEndpoints = sizeof(configuration1.interface0.endpoints) /
                                       sizeof(*configuration1.interface0.endpoints),
                      .bInterfaceClass = USB_HID_CLASS,
                      .bInterfaceSubClass = 1,
                      .bInterfaceProtocol = 1,
                      .iInterface = 0,
                  },
              .hid =
                  {
                      .bLength = sizeof(configuration1.interface0.hid),
                      .bDescriptorType = 0x21,  // HID descriptor type
                      .bcdHID = 0x0110,         // HID Class Spec release number 1.10
                      .bCountryCode = 0,
                      .bNumDescriptors = 1,
                      .bDescriptorType2 = 0x22,  // Report descriptor type
                      .wDescriptorLength = 0,    // TEMPORARY
                  },
              .endpoints =
                  {
                      [0] =
                          {
                              .bLength = sizeof(configuration1.interface0.endpoints[0]),
                              .bDescriptorType = USB_ENDPOINT_DESCRIPTOR,
                              .bEndpointAddress = USB_DEVICE_TO_HOST | 1,
                              .bmAttributes = USB_INTERRUPT_TRANSFER,
                              .wMaxPacketSize = sizeof(orchid_mouse_hid_report_t),  // TODO:
                              .bInterval = 1,
                          },
                  },
          },
  };

  configuration1.interface0.hid.wDescriptorLength = get_report_descriptor_length();
  return &configuration1.configuration;
}

int orchid_hid_init(orchid_hid_device_type_t device_type) {
  s_device_type = device_type;

  static const usb_string_descriptor_t product_name = {
      .bLength = sizeof(product_name),
      .bDescriptorType = USB_STRING_DESCRIPTOR,
      .bString = L"ORCHID",
  };

  static const usb_string_descriptor_t* strings[] = {&product_name};
  static const usb_string_descriptor_t langids = {
      .bLength = sizeof(langids),
      .bDescriptorType = USB_STRING_DESCRIPTOR,
      .bString =
          {
              [0] = 0x0009,  // English
          },
  };

  static const usb_configuration_descriptor_t* configurations[1];
  configurations[0] = init_configuration_descriptor();

  static const usb_device_descriptor_t device = {
      .bLength = sizeof(device),
      .bDescriptorType = USB_DEVICE_DESCRIPTOR,
      .bcdUSB = 0x0200,
      .bDeviceClass = 0,
      .bDeviceSubClass = 0,
      .bDeviceProtocol = 0,
      .bMaxPacketSize0 = 0x40,
      .idVendor = 0x0,
      .idProduct = 0x0,
      .bcdDevice = 0x0300,
      .iManufacturer = 0,
      .iProduct = 0,
      .iSerialNumber = 0,
      .bNumConfigurations = 1,
  };
  static const usb_standard_descriptors_t standard = {
      .device = &device,
      .configurations = configurations,
      .langids = &langids,
      .numStrings = sizeof(strings) / sizeof(*strings),
      .strings = strings,
  };

  usb_error_t result = usb_Init(handle_usb_event, NULL, &standard, USB_DEFAULT_INIT_FLAGS);
  s_was_usb_initialized = (result == USB_SUCCESS);
  return result;
}

void orchid_hid_deinit(void) {
  if (s_was_usb_initialized) {
    usb_Cleanup();
    s_was_usb_initialized = false;
  }
}

bool orchid_is_usb_connected(void) {
  return s_is_usb_connected;
}

void orchid_hid_handle_events(void) {
  if (!s_was_usb_initialized)
    return;

  usb_HandleEvents();
}

int orchid_hid_send_mouse_input(const orchid_mouse_hid_report_t* report) {
  if (!s_is_usb_connected)
    return USB_SUCCESS;

  usb_error_t result =
      usb_ScheduleInterruptTransfer(usb_GetDeviceEndpoint(s_active_usb_device, 0x81), (void*)report,
                                    sizeof(orchid_mouse_hid_report_t), NULL, NULL);
  if (result == USB_ERROR_NO_DEVICE) {
    s_is_usb_connected = false;
  }

  return result;
}

int orchid_hid_send_keyboard_input(const orchid_keyboard_hid_report_t* report) {
  if (!s_is_usb_connected)
    return USB_SUCCESS;

  (void)report;
  // TODO:
  return USB_SUCCESS;
}

