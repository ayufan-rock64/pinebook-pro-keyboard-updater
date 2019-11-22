#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

extern libusb_device_handle *devh;
extern libusb_context *ctx;
extern int devintf;

#define MAX_BINLEN (14*1024)

int open_usb(int vid, int pid, int indf);
void close_usb();

int open_user_mode();
int open_boot_mode();
int open_touchpad_mode();

int switch_to_boot_mode();
int reset_device();

int read_bulk(unsigned char *data, int length);
int write_bulk(unsigned char *data, int length);

int write_kb_fw(const char *filename);
int convert_hex_file(const char *filename, const char *output_filename);

int write_tp_fw(const char *filename);
