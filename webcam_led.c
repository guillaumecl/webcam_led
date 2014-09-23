/* Based on:
   V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/
#include "webcam_led.h"

#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/videodev2.h>
#include <libv4l2.h>

struct webcam
{
	char *name;

	int fd;
	void *map_start;
	size_t map_length;
};

static int xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = v4l2_ioctl(fh, request, arg);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	return r;
}


struct webcam *webcam_init(const char *dev_name)
{
	struct webcam *w = malloc(sizeof(struct webcam));
	unsigned int len;

	if (w == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	if (!dev_name)
	{
		dev_name = getenv("VIDEO_DEVICE");
		if (!dev_name)
		{
			dev_name = "/dev/video0";
		}
	}

	len = strlen(dev_name);
	w->name = malloc(len);
	if (w->name == NULL)
	{
		errno = ENOMEM;
		free(w);
		return NULL;
	}
	memcpy(w->name, dev_name, len+1);

	w->fd = 0;
	return w;
}

void webcam_free(struct webcam *w)
{
	if (!w)
		return;

	webcam_unlight(w);

	free(w->name);
	free(w);
}

int webcam_light(struct webcam *w)
{
	struct v4l2_buffer buf;
	struct v4l2_requestbuffers req;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (w->fd)
		return 0;

	w->fd = v4l2_open(w->name, O_RDWR | O_NONBLOCK, 0);
	if (w->fd < 0)
		return 1;

	memset(&req, 0, sizeof(req));
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (xioctl(w->fd, VIDIOC_REQBUFS, &req) < 0)
		goto error_close;

	memset(&buf, 0, sizeof(buf));

	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = 0;

	if (xioctl(w->fd, VIDIOC_QUERYBUF, &buf) < 0)
		goto error_close;

	w->map_length = buf.length;
	w->map_start = v4l2_mmap(NULL, buf.length,
						  PROT_READ | PROT_WRITE, MAP_SHARED,
						  w->fd, buf.m.offset);

	if (w->map_start == MAP_FAILED)
		goto error_close;

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	if (xioctl(w->fd, VIDIOC_QBUF, &buf) < 0)
		goto error_unmap;


	if (xioctl(w->fd, VIDIOC_STREAMON, &type) < 0)
		goto error_unmap;

	return 0;

error_unmap:
	v4l2_munmap(w->map_start, w->map_length);

error_close:
	v4l2_close(w->fd);

	return 1;
}

int webcam_unlight(struct webcam *w)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (!w->fd)
		return 0;

	xioctl(w->fd, VIDIOC_STREAMOFF, &type);
	v4l2_munmap(w->map_start, w->map_length);
	v4l2_close(w->fd);
	w->fd = 0;
	return 0;
}
