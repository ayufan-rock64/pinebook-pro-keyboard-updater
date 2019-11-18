#include "updater.h"

int open_usb(int vid, int pid, int indf)
{
  int rc;

  rc = libusb_init(&ctx);
  if(rc < 0)
    return rc;

  devh = libusb_open_device_with_vid_pid(ctx, vid, pid);
  if(devh == NULL)
    return -1;

  rc = libusb_kernel_driver_active(devh, indf);
  if (rc != 1) {
    printf("libusb_kernel_driver_active: %d\n", rc);
    close_usb();
    return -1;
  }

  printf("Kernel Driver Active\n");
  rc = libusb_detach_kernel_driver(devh, indf);
  if (rc != 0) {
    printf("libusb_detach_kernel_driver: %d\n", rc);
    close_usb();
    return -1;
  }

  rc = libusb_claim_interface(devh, indf);
  if(rc < 0) {
    printf("libusb_claim_interface: %d\n", rc);
    close_usb();
    return rc;
  }

  return 0;
}

void close_usb()
{
  if (devh) {
    printf("release interface\r\n");
    libusb_release_interface(devh, 0); //释放接口
    libusb_close (devh);
    libusb_exit(ctx);
    devh = NULL;
  }
}
