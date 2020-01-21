SOURCES := \
	updater.c \
	usb_read.c \
	usb_write.c \
	usb_context.c \
	usb_keyboard.c \
	usb_touchpad.c \
	firmware/fw_tp_update_hex.c \
	firmware/fw_iso_hex.c \
	firmware/fw_ansi_hex.c \
	firmware/tpfw_bin.c \

all: updater

firmware/%_hex.c: firmware/%.hex
	xxd -i $^ $@

firmware/%_bin.c: firmware/%.bin
	xxd -i $^ $@

updater: $(SOURCES)
	gcc -o $@ $^ -lusb-1.0

usbreset: extra/usbreset.c
	gcc -o $@ $^

clean:
	rm -f updater usbreset firmware/*.c
