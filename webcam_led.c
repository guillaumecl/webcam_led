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
	int fd;
	void *map_start;
	size_t map_length;

	int lighted;
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
	struct v4l2_buffer buf;
	struct v4l2_requestbuffers req;
	struct webcam *w = malloc(sizeof(struct webcam));

	if (w == NULL)
	{
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

	w->fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (w->fd < 0)
	{
		goto err_free;
	}

	memset(&req, 0, sizeof(req));
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (xioctl(w->fd, VIDIOC_REQBUFS, &req) < 0)
	{
		goto error_close;
	}

	memset(&buf, 0, sizeof(buf));

	buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory      = V4L2_MEMORY_MMAP;
	buf.index       = 0;

	if (xioctl(w->fd, VIDIOC_QUERYBUF, &buf) < 0)
	{
		goto error_close;
	}

	w->map_length = buf.length;
	w->map_start = v4l2_mmap(NULL, buf.length,
						  PROT_READ | PROT_WRITE, MAP_SHARED,
						  w->fd, buf.m.offset);

	if (w->map_start == MAP_FAILED)
	{
		goto error_close;
	}

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	if (xioctl(w->fd, VIDIOC_QBUF, &buf) < 0)
	{
		goto error_unmap;
	}

	return w;

error_unmap:
	v4l2_munmap(w->map_start, w->map_length);

error_close:
	v4l2_close(w->fd);

err_free:
	free(w);
	return NULL;
}

void webcam_free(struct webcam *w)
{
	if (w->fd < 0)
		return;

	webcam_unlight(w);
	v4l2_munmap(w->map_start, w->map_length);
	v4l2_close(w->fd);

	free(w);
}

int webcam_light(struct webcam *w)
{
	if (w->lighted)
		return 0;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int r = xioctl(w->fd, VIDIOC_STREAMON, &type);
	if (r == 0)
	{
		w->lighted = 1;
	}
	return r;
}

int webcam_unlight(struct webcam *w)
{
	if (!w->lighted)
		return 0;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int r = xioctl(w->fd, VIDIOC_STREAMOFF, &type);
	if (r == 0)
	{
		w->lighted = 0;
	}
	return r;
}
