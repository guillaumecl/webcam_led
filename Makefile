CC = cc
CFLAGS = -Wall -Wextra
DESTDIR ?= /usr/local

all:libwebcam_led.a webcam_wait



.c.o:
	$(CC) $(CFLAGS) -c $< -o $@


libwebcam_led.a: webcam_led.o
	${AR} rcs libwebcam_led.a webcam_led.o

webcam_wait: webcam_wait.o libwebcam_led.a
	${CC} $< -o $@ -L. -lwebcam_led -lv4l2

install:libwebcam_led.a webcam_wait
	install -D -m 0755 webcam_wait ${DESTDIR}/usr/bin/webcam_wait
	install -D -m 0755 libwebcam_led.a ${DESTDIR}/usr/lib/libwebcam_led.a
	install -D -m 0644 webcam_led.h ${DESTDIR}/usr/include/webcam_led/webcam_led.h

.PHONY:clean all install


clean:
	rm -f *.a *.o *~ webcam_wait
