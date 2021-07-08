#include "updater.h"

libusb_device_handle *devh;
libusb_context *ctx;
int devintf;

extern unsigned char firmware_fw_tp_update_hex[];
extern unsigned int firmware_fw_tp_update_hex_len;
extern unsigned char firmware_fw_iso_hex[];
extern unsigned int firmware_fw_iso_hex_len;
extern unsigned char firmware_fw_ansi_hex[];
extern unsigned int firmware_fw_ansi_hex_len;
extern unsigned char firmware_tpfw_bin[];
extern unsigned int firmware_tpfw_bin_len;

static int usage(const char *cmd)
{
  printf("usage: %s <step-1|step-2> <iso|ansi>\n", cmd);
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
    firmware_fw_iso_hex, firmware_fw_iso_hex_len,
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

static int flash_kb_iso()
{
  int rc;

  rc = write_kb_fw(firmware_fw_iso_hex, firmware_fw_iso_hex_len);
  if (rc < 0) {
    return rc;
  }

  return 0;
}

static int flash_kb_ansi()
{
  int rc;

  rc = write_kb_fw(firmware_fw_ansi_hex, firmware_fw_ansi_hex_len);
  if (rc < 0) {
    return rc;
  }

  return 0;
}


static int step_1()
{
  int rc;

  printf("[x] Running STEP-1...\n");

  printf("[*] Flashing keyboard updater firmware...\n");
  rc = flash_tp_update();
  if (rc < 0) {
    return rc;
  }
  
  printf("[x] Power cycle your Pinebook Pro, then run 'step-2'.\n");
  return 0;
}

static int step_2(int ansi_type)
{
  int rc;

  printf("[x] Running STEP-2...\n");

  printf("[*] Flashing touchpad firmware...\n");
  rc = flash_tp();
  if (rc < 0) {
    return rc;
  }

  if (ansi_type) {
    printf("[*] Flashing ANSI keyboard firmware...\n");
    rc = flash_kb_ansi();
  } else {
    printf("[*] Flashing ISO keyboard firmware...\n");
    rc = flash_kb_iso();
  }
  if (rc < 0) {
    return rc;
  }

  printf("[x] All done, keyboard and touchpad should be updated\n");
  return 0;
}

int main(int argc, char *argv[])
{
  int rc = 0;

  if (argc != 3) {
    rc = usage(argv[0]);
  } else if (strcmp(argv[2], "iso") && strcmp(argv[2], "ansi")) {
    rc = usage(argv[0]);
    printf("* specify valid keyboard type\n");
  } else if (!strcmp(argv[1], "convert")) {
    rc = convert();
  } else if (!strcmp(argv[1], "step-1")) {
    rc = step_1();
  } else if (!strcmp(argv[1], "step-2")) {
    rc = step_2(!strcmp(argv[2], "ansi"));
  } else if (!strcmp(argv[1], "flash-tp")) {
    rc = flash_tp();
  } else if (!strcmp(argv[1], "flash-tp-update")) {
    rc = flash_tp_update();
  } else if (!strcmp(argv[1], "flash-kb-iso")) {
    rc = flash_kb_iso();
  } else if (!strcmp(argv[1], "flash-kb-ansi")) {
    rc = flash_kb_ansi();
  } else {
    rc = usage(argv[0]);
  }
  
  return rc;
}
