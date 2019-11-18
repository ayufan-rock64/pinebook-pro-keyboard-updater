#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

extern libusb_device_handle *devh;
extern libusb_context *ctx;

#define MAX_BINLEN (14*1024)

int open_usb(int vid, int pid, int indf);
void close_usb();

int read_bulk(unsigned char *data, int length);
int write_bulk(unsigned char *data, int length);
int read_binfile(const char *filename, unsigned char output[MAX_BINLEN]);
