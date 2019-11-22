all: updater

updater: updater.c usb_read.c usb_write.c usb_context.c usb_keyboard.c usb_touchpad.c
	gcc -o $@ $^ -lusb-1.0

usbreset: extra/usbreset.c
	gcc -o $@ $^

clean:
	rm -f updater usbreset
