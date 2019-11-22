#include "updater.h"

static int read_hexfile(const char *filename, unsigned char output[MAX_BINLEN])
{
  FILE* in;

  in = fopen(filename, "rt");
  if (!in) {
    printf("failed to open: %s\n", filename);
    return -1;
  }

  unsigned char pbuffer[MAX_BINLEN];
  const char *endstr = ":00000001FF\n";
  char strbuf[256];
  int max_address = 0;

  while (fgets(strbuf, 256, in)) {
    if (strcmp(endstr, strbuf) == 0) {
      break;
    }

    char len_str[3];
    len_str[0] = strbuf[1];
    len_str[1] = strbuf[2];
    len_str[2] = 0;
    int len = strtol(len_str, NULL, 16);

    char addr_str[5];
    addr_str[0] = strbuf[3];
    addr_str[1] = strbuf[4];
    addr_str[2] = strbuf[5];
    addr_str[3] = strbuf[6];
    addr_str[4] = 0;
    int addr = strtol(addr_str, NULL, 16);

    char val_str[3];
    val_str[2] = 0;
    for (int i = 0; i < len; i++) {
      val_str[0] = strbuf[2 * i + 9];
      val_str[1] = strbuf[2 * i + 1 + 9];
      int val = strtol(val_str, NULL, 16);
      if (addr >= MAX_BINLEN) {
        break;
      }

      pbuffer[addr++] = val;

      if (addr > max_address) {
        max_address = addr;
      }
    }
  }

  fclose(in);

  for (int i = 0; i < max_address; i++)
  {
    output[i] = pbuffer[i];
  }

  if (output[1] == 0x38 && output[2] == 0x00)
  {
    printf(">>> Fix hex file\n");
    output[0] = pbuffer[0x37FB];
    output[1] = pbuffer[0x37FC];
    output[2] = pbuffer[0x37FD];

    output[0x37FB] = 0x00;
    output[0x37FC] = 0x00;
    output[0x37FD] = 0x00;
  }

  return max_address;
}

int write_serial_number(unsigned char sensor_direct, unsigned short serial_number)
{
  int rc;

  {
    unsigned char data[6] = {
      0x05, 0x52, 0x80, 0xff, 0x08, 0x00
    };

    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }
  }

  unsigned short vid = 0, pid = 0;

  {
    unsigned char data[6] = {
      0x05, 0x72
    };

    rc = libusb_control_transfer(devh, 0xA1, 0x01, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }

    vid = (data[2] << 8) | data[3];
    pid = (data[4] << 8) | data[5];

    printf("Did read VID:%04x PID:%04x\n", vid & 0xFFFF, pid & 0xFFFF);

    // again?
    rc = libusb_control_transfer(devh, 0xA1, 0x01, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }
  }

  {
    unsigned char data[6] = {
      0x05, 0x65, 0xff, 0x00, 0x00, 0x00
    };

    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }
  }

  usleep(200000);

  {
    unsigned char data[6] = {
      0x05, 0x57, 0x80, 0xFF, 0x08, 0x00
    };

    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }
  }

  {
    unsigned char data[6] = {
      0x05, 0x77,
      (vid >> 8) & 0xFF,
      vid & 0xFF,
      (pid >> 8) & 0xFF,
      pid & 0xFF
    };

    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }
  }

  usleep(100000);

  {
    unsigned char data[6] = {
      0x05, 0x77,
      sensor_direct & 0xFF,
      0x00,
      (serial_number >> 8) & 0xFF,
      serial_number & 0xFF
    };

    rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
    if (rc < 0) {
      return rc;
    }
  }

  return 0;
}

int write_kb_fw(const char *filename)
{
  unsigned char hex_file[MAX_BINLEN];
  unsigned char read_hex_file[MAX_BINLEN];
  int hex_file_length;
  int rc;
  int try;

  printf("[*] Reading %s\n", filename);
  hex_file_length = read_hexfile(filename, hex_file);
  if (hex_file_length <= 0) {
    printf("failed to read: %s\n", filename);
    return -1;
  }

  switch_to_boot_mode();

  printf("[*] Opening in boot mode\n");
  for (try = 0; try < 100; try++) {
    rc = open_boot_mode();
    if (rc >= 0) {
      break;
    }
    usleep(10*1000);
  }

  if (try == 100) {
    printf(">>> Failed to open in boot mode\n");
    goto finish;
  }

  unsigned char reportData[6] = {
    0x5, 0x45, 0x45, 0x45, 0x45, 0x45
  };

  // flash erase
  printf("[*] Erasing flash\n");
  rc = libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0,
    reportData, sizeof(reportData), 100);
  if (rc < 0) {
    printf("failed to erase flash\n");
    goto finish;
  }

  sleep(2);

  printf("[*] Writing firmware...\n");
  // write FW
  for (try = 0; try < 5; try++) {
    rc = write_bulk(hex_file, hex_file_length);
    if (rc == 0) {
      break;
    }
  }

  if (try == 5) {
    printf("too many tries\n");
    rc = -1;
    goto finish;
  }

  printf("[*] Reading back firmware...\n");
  // read FW
  for (try = 0; try < 5; try++) {
    rc = read_bulk(read_hex_file, hex_file_length);
    if (rc == 0) {
      break;
    }
  }

  if (try == 5) {
    printf("too many tries\n");
    rc = -1;
    goto finish;
  }

  printf("[*] Comparing firmwares...\n");
  if (memcmp(hex_file, read_hex_file, 0x37fb)) {
    printf("FATAL ERROR FW does differ\n");
    for (int i = 0; i < hex_file_length; i++) {
      if (hex_file[i] == read_hex_file[i]) {
        continue;
      }
      printf(">>> 0x%04x] %02x != %02x\n", i, hex_file[i], read_hex_file[i]);
    }
    rc = -1;
    goto finish;
  }

#if 0
  printf("[*] Writing serial number...\n");
  write_serial_number(1, 0x4100);
  if (rc < 0) {
    goto finish;
  }
#endif

  printf("[*] Reseting device?\n");
  reset_device();

  printf("[*] Finished succesfully!\n");
finish:
  close_usb();
  return rc;
}
