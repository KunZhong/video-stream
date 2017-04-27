/*************************************************************************
  > File Name: camera.h
	> Author: 
	> Mail: 
	> Created Time: 2016年04月25日 星期一 10时48分27秒
 ************************************************************************/
#ifndef _CAMERA_H
#define _CAMERA_H

#include <semaphore.h>
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>

#include <linux/videodev2.h>

#include <pthread.h>

#include <jpeglib.h>

#define   OUTPUT_BUF_SIZE       4096
#define   CLEAR(x)            	memset(&(x), 0, sizeof(x))


#define      OPEN                            0
#define      CLOSE                          -1
#define      SUCCESS                         0
#define      FAILURE                        -1

#define     FILE_VIDEO                 "/dev/video0"
#define       BMP                      "camera.bmp"
#define       JPG                      "camera.jpg"
#define       YUV                      "camera.yuv"

#define       WIDTH                         640    
#define       HEIGHT                        480     
#define     COUNT_NUM                        4 
  

typedef int status;

typedef struct mybuffer {
    unsigned char  start[WIDTH * HEIGHT * 2];
    int length;
} ImageBuffer;

typedef struct {
	struct jpeg_destination_mgr pub;
	JOCTET * buffer; 
	unsigned char *outbuffer;
	int outbuffer_size;
	unsigned char *outbuffer_cursor;
	int *written; 
} mjpg_destination_mgr;
 
typedef mjpg_destination_mgr *mjpg_dest_ptr;
 
static status camera_init(void);
static status camera_close(int fd);

                      
int get_camera_fd();
status get_camera_status();
status camera_open();
status camera_Close();
status camera_capture(ImageBuffer *imgbuf);

void dest_buffer(j_compress_ptr cinfo, unsigned char *buffer, int size, int *written);
status yuv_jpg(unsigned char *buf, unsigned char *buffer, int size, int quality);
status get_camera_jpg(int fd, ImageBuffer *jpg_buf);

extern pthread_mutex_t mutex;

#endif

