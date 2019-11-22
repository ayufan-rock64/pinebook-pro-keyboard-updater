#include "updater.h"

libusb_device_handle *devh;
libusb_context *ctx;
int devintf;

static int usage(const char *cmd)
{
  printf("usage: %s [convert|flash-kb|flash-tb]\n", cmd);
  return -1;
}

static int convert()
{
  int rc;
    
  rc = convert_hex_file(
    "files/Hynitron_NO_power_co_tp_update_kbfw.hex",
    "files/Hynitron_NO_power_co_tp_update_kbfw.bin");
  if (rc < 0) {
    return rc;
  }

  rc = convert_hex_file(
    "files/fw.hex",
    "files/fw.bin");
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int flash_tp()
{
  int rc;

  rc = write_kb_fw("files/HLK_hyn_Nopower_contor_tp_update_tmpkbhex01.hex");
  if (rc < 0) {
    return rc;
  }

  rc = write_tp_fw("files/tpfw.bin");
  if (rc < 0) {
    return rc;
  }

  rc = write_kb_fw("files/fw.hex");
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int flash_kb()
{
  int rc;

  rc = write_kb_fw("files/fw.hex");
  if (rc < 0) {
    return rc;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  int rc = 0;

  if (argc != 2) {
    rc = usage(argv[0]);
  } else if (!strcmp(argv[1], "convert")) {
    rc = convert();
  } else if (!strcmp(argv[1], "flash-tp")) {
    rc = flash_tp();
  } else if (!strcmp(argv[1], "flash-kb")) {
    rc = flash_kb();
  } else {
    rc = usage(argv[0]);
  }
  
  return rc;
}
