all: updater

updater: updater.c usb_read.c usb_write.c usb_context.c usb_keyboard.c usb_touchpad.c
	gcc -o updater $^ -lusb-1.0

clean:
	rm -f updater
