#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi/hidapi.h>

#define MAX_STR 255

char* find_device_path(unsigned short vendor_id, unsigned short product_id);
bool read_temperature(hid_device *handle, int* result);

#define TEMPer_VendorId 0x413d
#define TEMPer_ProductId 0x2107

int main(int argc, char* argv[]) {
  if(hid_init()) {
    fwprintf(stderr,L"failed to initialize HID API\n");
    return EXIT_FAILURE;
  }

  char* path = find_device_path(TEMPer_VendorId, TEMPer_ProductId);

  if(!path) {
    fwprintf(stderr,L"cannot find matching HID device ID %x:%x, try 'lsusb' or 'lsusb -d %x:%x'\n",
             TEMPer_VendorId,TEMPer_ProductId,
             TEMPer_VendorId,TEMPer_ProductId);
    return EXIT_FAILURE;
  } else {
    hid_device *handle =  hid_open_path(path);

    if (!handle) {
      fwprintf(stderr,L"failed to open device %s\n"
               "to avoid running this program as root, put the following udev rule in e.g. /etc/udev/rules.d/90-temper.rules\n\n"
               "SUBSYSTEMS==\"usb\", ACTION==\"add\", ATTRS{idVendor}==\"%x\", ATTRS{idProduct}==\"%x\", MODE=\"666\"\n\n",
               path,TEMPer_VendorId,TEMPer_ProductId);
      free(path);
      return EXIT_FAILURE;
    }

    int temperature = INT_MIN;
    if(read_temperature(handle,&temperature)) {
      printf("{ \"temp_celsius\": %d.%d }\n",temperature/100,temperature%100);
    }
    hid_close(handle);
    free(path);
  }

  if(hid_exit()) {
    fwprintf(stderr,L"failed to shutdown HID API\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

bool read_temperature(hid_device *handle, int* result) {
  unsigned char buf[16];
  unsigned char cmd[8] = { 0x01, 0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };

  memset(&buf,0,sizeof(buf));

  int rc = hid_write(handle, cmd, sizeof(cmd));

  if(-1 == rc) {
    return false;
  }

  // expected response is exactly 8 bytes
  bool ok = (8 == hid_read(handle, buf, sizeof(buf)));

  if (ok) {
    // temperature is stored in byte 2&3 => return value is a hundreth of °C
    *result = (buf[2]*256 + buf[3]);
  }
  return ok;
}


char* find_device_path(unsigned short vendor_id, unsigned short product_id) {

  struct hid_device_info *devices = hid_enumerate(vendor_id,product_id);
  struct hid_device_info *cur_dev = devices;

  while (cur_dev) {
    // the hardware provides two devices, we need the second one
    if (1 == cur_dev->interface_number && cur_dev->path) {
      char* path = malloc(strlen(cur_dev->path)+1);
      strcpy(path,cur_dev->path);
      hid_free_enumeration(devices);
      return path;
    }
    cur_dev = cur_dev->next;
  }
  hid_free_enumeration(devices);
  return NULL;
}
