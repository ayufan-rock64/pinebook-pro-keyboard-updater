# Pinebook Pro keyboard/touchpad firmware updater

This application does upgrade firmware of the built-in keyboard and touchpad.

**DO NOT USE IT. WE ARE WAITING FOR CORRECT FIRMWARE TO USE**

## Compiling

```bash
git clone https://github.com/ayufan-rock64/pinebook-pro-keyboard-updater -b my-updater
cd pinebook-pro-keyboard-updater
sudo apt-get install build-essential libusb-1.0-0-dev
make
```

## Update all firmwares

You need to do all of that in correct order,
if at any point process fails, start it from point 1.:

1. Run `step-1` of update process: `sudo ./updater step-1`,
1. After `step-1` touchpad will not work, keyboard works as normal,
1. Reboot with `sudo reboot`,
1. After reboot run `step-2` of update process: `sudo ./updater step-2`,
1. Now, your keyboard and touchpad firmware should be updated.

```bash
# step-1
sudo ./updater step-1
sudo reboot

# after reboot, step-2
sudo ./updater step-2
```

## License

MIT, 2019, SHEN ZHEN HAI LUCK ELECTRONIC TECHNOLOGY CO., LTD
