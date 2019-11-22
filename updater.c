#include "updater.h"

libusb_device_handle *devh;
libusb_context *ctx;
int devintf;

extern unsigned char firmware_fw_tp_update_hex[];
extern unsigned int firmware_fw_tp_update_hex_len;
extern unsigned char firmware_fw_hex[];
extern unsigned int firmware_fw_hex_len;
extern unsigned char firmware_tpfw_bin[];
extern unsigned int firmware_tpfw_bin_len;

static int usage(const char *cmd)
{
  printf("usage: %s [convert|flash-kb|flash-tp|flash-tp-update]\n", cmd);
  return -1;
}

static int convert()
{
  int rc;
    
  rc = convert_hex_data(
    firmware_fw_tp_update_hex, firmware_fw_tp_update_hex_len,
    "fw_tp_update.bin");
  if (rc < 0) {
    return rc;
  }

  rc = convert_hex_data(
    firmware_fw_hex, firmware_fw_hex_len,
    "fw.bin");
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int flash_tp()
{
  int rc;

  rc = write_tp_fw(firmware_tpfw_bin, firmware_tpfw_bin_len);
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int flash_tp_update()
{
  int rc;

  rc = write_kb_fw(firmware_fw_tp_update_hex, firmware_fw_tp_update_hex_len);
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int flash_kb()
{
  int rc;

  rc = write_kb_fw(firmware_fw_hex, firmware_fw_hex_len);
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int step_1()
{
  int rc;

  printf("Running STEP-1...\n");

  printf("[*] Flashing keyboard updater firmware...\n");
  rc = flash_tp_update();
  if (rc < 0) {
    return rc;
  }
  
  printf("[*] Please reboot now, and run `step-2`.\n");
  
  return 0;
}

static int step_2()
{
  int rc;

  printf("Running STEP-2...\n");

  printf("[*] Flashing touchpad firmware...\n");
  rc = flash_tp();
  if (rc < 0) {
    return rc;
  }

  printf("[*] Flashing keyboard firmware...\n");
  rc = flash_kb();
  if (rc < 0) {
    return rc;
  }

  printf("[*] All done! Your keyboard and touchpad should be updated.\n");

  return 0;
}

int main(int argc, char *argv[])
{
  int rc = 0;

  if (argc != 2) {
    rc = usage(argv[0]);
  } else if (!strcmp(argv[1], "convert")) {
    rc = convert();
  } else if (!strcmp(argv[1], "step-1")) {
    printf("Running STEP-1...\n");
    rc = flash_kb();
    if (!rc) {
      flash
    }
  } else if (!strcmp(argv[1], "step-2")) {
    rc = flash_tp_update();
  } else if (!strcmp(argv[1], "flash-tp")) {
    rc = flash_tp();
  } else if (!strcmp(argv[1], "flash-tp-update")) {
    rc = flash_tp_update();
  } else if (!strcmp(argv[1], "flash-kb")) {
    rc = flash_kb();
  } else {
    rc = usage(argv[0]);
  }
  
  return rc;
}
