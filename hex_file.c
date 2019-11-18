#include "updater.h"

int read_binfile(const char *filename, unsigned char output[MAX_BINLEN])
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

      *(pbuffer + addr) = val;
      addr++;
      if (addr > max_address) {
        max_address = addr;
      }
    }
  }

  fclose(in);

  unsigned char tempBuff[3];

  for (int i = 0; i < max_address; i++)
  {
    output[i] = pbuffer[i];

    if (i == 0x37FB)
      tempBuff[0] = pbuffer[i];
    else if (i == 0x37FC)
      tempBuff[1] = pbuffer[i];
    else if (i == 0x37FD)
      tempBuff[2] = pbuffer[i];
  }

  if (output[1] == 0x38 && output[2] == 0x00)
  {
    output[0] = tempBuff[0];
    output[1] = tempBuff[1];
    output[2] = tempBuff[2];

    output[0x37FB] = 0x00;
    output[0x37FC] = 0x00;
    output[0x37FD] = 0x00;
  }

  return max_address;
}
