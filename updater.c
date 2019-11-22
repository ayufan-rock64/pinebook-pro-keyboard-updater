#include "updater.h"

libusb_device_handle *devh;
libusb_context *ctx;

int main(int argc, char *argv[])
{
  int rc = 0;

  rc = switch_to_boot_mode();

  rc = write_kb_fw("files/N1401_8350_68F83_6444456626_Hynitron_20170323_452703_US_AN_NST_PTP_MB277_Tv9.hex");

  return rc;
}
