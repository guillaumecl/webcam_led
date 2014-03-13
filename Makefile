CC = cc
CFLAGS = -Wall -Wextra

all:libwebcam_led.a webcam_wait



.c.o:
	$(CC) $(CFLAGS) -c $< -o $@


libwebcam_led.a: webcam_led.o
	${AR} rcs libwebcam_led.a webcam_led.o

webcam_wait: webcam_wait.o libwebcam_led.a
	${CC} $< -o $@ -L. -lwebcam_led -lv4l2

.PHONY:clean all


clean:
	rm -f *.a *.o *~ webcam_wait
