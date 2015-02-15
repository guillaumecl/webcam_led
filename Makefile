LIBS=libv4l2
NAME=webcam_led

include base.mk

LIB=${BUILD_DIR}/libwebcam_led.a

${BIN}: ${LIB}

${LIB}: webcam_led.o

install:${BIN} ${LIB}
	install -D -m 0755 ${BIN} ${DESTDIR}/${PREFIX}/bin/webcam_wait
	install -D -m 0755 ${LIB} ${DESTDIR}/${PREFIX}/lib/libwebcam_led.a
	install -D -m 0644 webcam_led.h ${DESTDIR}/${PREFIX}/include/webcam_led/webcam_led.h
