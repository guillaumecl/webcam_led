LIBS=libv4l2
NAME=webcam_led

include base.mk

LIB=${BUILD_DIR}/libwebcam_led.a

${BIN}: ${LIB}

${LIB}: webcam_led.o

install:${BIN} ${LIB}
	install -D -m 0755 ${BIN} ${DESTDIR}/${PREFIX}/bin/webcam_wait
	install -D -m 0644 ${LIB} ${DESTDIR}/${PREFIX}/lib/libwebcam_led.a
	install -D -m 0644 webcam_led.h ${DESTDIR}/${PREFIX}/include/webcam_led/webcam_led.h
	install -D -m 0644 man/webcam_led.3 ${DESTDIR}/${PREFIX}/share/man/man3/webcam_led.3
	install -D -m 0644 man/webcam_init.3 ${DESTDIR}/${PREFIX}/share/man/man3/webcam_init.3
	install -D -m 0644 man/webcam_free.3 ${DESTDIR}/${PREFIX}/share/man/man3/webcam_free.3
	install -D -m 0644 man/webcam_light.3 ${DESTDIR}/${PREFIX}/share/man/man3/webcam_light.3
	install -D -m 0644 man/webcam_unlight.3 ${DESTDIR}/${PREFIX}/share/man/man3/webcam_unlight.3
