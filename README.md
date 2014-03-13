webcam_led
==========

Light or unlight a webcam's LED to use it as a notification system.


There are two basic ways to use this:

* use the webcam_led library

a static library called libwebcam_led is generated. This can be used to easily light or unlight a webcam. Please look the webcam_led.h file for more information on the API.

Please note that your binary will need linking to libv4l2.

* use the provided webcam_wait binary

webcam_wait is a small utility binary that connects to the webcam, creates a local socket, then waits for data.

One way to use this from the command line is for example to type 1 or 0 with socat :

  socat - UNIX-CONNECT:webcam_wait.socket
