
#ifndef WEBCAM_LED__H
#define WEBCAM_LED__H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Opaque structure containing a webcam information.
 */
struct webcam;

/**
 * Initializes a webcam.
 *
 * Pass dev_name to NULL to fallback to the VIDEO_DEVICE environment variable
 * or /dev/video0 in not defined.
 *
 * This returns NULL in case of error.
 */
struct webcam *webcam_init(const char *dev_name);

/**
 * Frees all resources used by the specified webcam.
 *
 * w must have been returned by webcam_init.
 */
void webcam_free(struct webcam *w);

/**
 * Turns the webcam light on.
 *
 * w must have been returned by webcam_init.
 *
 * Returns a negative number in case of an error.
 */
int webcam_light(struct webcam *w);

/**
 * Turns the webcam light on.
 *
 * w must have been returned by webcam_init.
 *
 * Returns a negative number in case of an error.
 */
int webcam_unlight(struct webcam *w);

#if defined (__cplusplus)
}
#endif


#endif // WEBCAM_LED__H
