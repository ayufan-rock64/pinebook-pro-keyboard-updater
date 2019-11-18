#include "updater.h"

#define VID 0x258a
#define PID 0x001e

#define BOOTVID 0x0603
#define BOOTPID 0x1020

#define BOOTMODE 0xFE
#define USERMODE 0xFD

#define MAX_BINLEN (14*1024)

libusb_device_handle *devh;
libusb_context *ctx;

unsigned short m_serial_number=1;

int open_user_mode()
{
  return open_usb(VID, PID, 1);
}

int open_boot_mode()
{
  return open_usb(BOOTVID, BOOTPID, 0);
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

  sleep(3);

finish:
  close_usb();
  return rc;
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

int reset_device()
{
  unsigned char data[6] = {
    0x05, 0x55, 0x55, 0x55, 0x55, 0x55
  };

  return libusb_control_transfer(devh, 0x21, 0x09, 0x0305, 0, data, sizeof(data), 100);
}

int write_kb_fw(const char *filename)
{
  unsigned char hex_file[MAX_BINLEN];
  unsigned char read_hex_file[MAX_BINLEN];
  int hex_file_length;
  int rc;

  printf("[*] Reading %s\n", filename);
  hex_file_length = read_binfile(filename, hex_file);
  if (hex_file_length <= 0) {
    printf("failed to read: %s\n", filename);
    return -1;
  }

  printf("[*] Opening in boot mode\n");
  rc = open_boot_mode();
  if (rc < 0) {
    printf("failed to open in boot mode\n");
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
  int try = 0;
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
  if (memcmp(hex_file, read_hex_file, hex_file_length)) {
    printf("FATAL ERROR FW does differ\n");
    rc = -1;
    goto finish;
  }

  printf("[*] Writing serial number...\n");
  write_serial_number(1, 0x4100);
  if (rc < 0) {
    goto finish;
  }

  printf("[*] Reseting device?\n");
  reset_device();

  printf("[*] Finished succesfully!\n");
finish:
  close_usb();
  return rc;
}

int main(int argc, char *argv[])
{
  switch_to_boot_mode();

  int rc = write_kb_fw("files/N1401_8350_68F83_6444456626_Hynitron_20170323_452703_US_AN_NST_PTP_MB277_Tv9.hex");

  return rc;
}
