#include "updater.h"

int open_usb(int vid, int pid, int indf)
{
  int rc;

  rc = libusb_init(&ctx);
  if(rc < 0)
    return rc;

  printf(">>> Trying to open VID:%04x PID:%04x...\n", vid&0xffff, pid&0xffff);
  devh = libusb_open_device_with_vid_pid(ctx, vid, pid);
  if(devh == NULL) {
    printf(">>> Device not found\n");
    return -1;
  }

  rc = libusb_kernel_driver_active(devh, indf);
  if (rc != 1) {
    printf(">>> libusb_kernel_driver_active: %d\n", rc);
    goto finish;
  }

  printf(">>> Kernel Driver Active\n");
  rc = libusb_detach_kernel_driver(devh, indf);
  if (rc != 0) {
    printf(">>> libusb_detach_kernel_driver: %d\n", rc);
    goto finish;
  }

  rc = libusb_claim_interface(devh, indf);
  if(rc < 0) {
    printf(">>> libusb_claim_interface: %d\n", rc);
    goto finish;
  }

finish:
  if (rc < 0) {
    close_usb();
  }
  return rc;
}

void close_usb()
{
  if (devh) {
    printf(">>> release interface\r\n");
    libusb_release_interface(devh, 0); //释放接口
    libusb_close(devh);
    libusb_exit(ctx);
    devh = NULL;
  }
}

int open_user_mode()
{
  int rc = open_usb(0x258a, 0x001e, 1);
  if (rc < 0) {
    rc = open_usb(0x258a, 0x001f, 1);
  }

  return rc;
}

int open_touchpad_mode()
{
  return open_usb(0x258a, 0x001f, 1);
}

int open_boot_mode()
{
  return open_usb(0x0603, 0x1020, 0);
}

int switch_to_boot_mode()
{
  int rc;

  printf("[*] Opening in user mode...\n");
  rc = open_user_mode();
  if (rc < 0) {
    printf("Failed to open in user mode\n");
    goto finish;
  }

  printf("[*] Sending command to switch to boot mode...\n");

  unsigned char dataOut[6] = {
    0x5, 0x75
  };
  rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 1,
    dataOut, sizeof(dataOut), 100);
  if (rc < 0) {
    printf("failed to send switch command\n");
    goto finish;
  }

  printf("[*] Command send\n");

finish:
  close_usb();
  return rc;
}

int reset_device()
{
  unsigned char data[6] = {
    0x05, 0x55, 0x55, 0x55, 0x55, 0x55
  };

  int rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
  if (rc < 0) {
    return rc;
  }

  rc = libusb_reset_device(devh);
  if (rc < 0) {
    return rc;
  }

  return 0;
}
