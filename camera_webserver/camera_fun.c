/*************************************************************************
	> File Name: camera_fun.c
	> Author: 
	> Mail: 
	> Created Time: 2016年04月25日 星期一 10时48分27秒
 ************************************************************************/
#include "camera.h"

static int camera_status = CLOSE;
static int camera_fd = -1;                             
                                      
struct buffer {
    void * start;
    unsigned int length;
} *buffers;

static struct v4l2_requestbuffers req;
static unsigned int n_buffers;

struct v4l2_format fmt;

//获取摄像头的状态
status get_camera_status()
{
	return camera_status;
}
status get_camera_fd()
{
	return camera_fd;
}

/**********************************************************/
//函数名：camera_close
//功能  ：判断摄像的状态，若未关闭则关闭摄像头。
//返回值：SUCCESS/FAIURE
/***********************************************************/
int camera_Close()
{
	if(OPEN == get_camera_status()){
		camera_status = camera_close(get_camera_fd());
		return camera_status;
	}
	return FAILURE;
}
/**********************************************************/
//函数名：camera_open
//功能  ：判断摄像头状态，若未打开则初始化摄像头
//形参  ：无
//返回值：是否打开成功
/***********************************************************/
int camera_open()
{
	if(CLOSE == get_camera_status()){	
		camera_fd = camera_init();
		if (-1 == camera_fd) {
			printf("init failed.\n");
			return FAILURE;
		}else{
			camera_status = OPEN;//open	
			return SUCCESS;	
		}
	}
	return FAILURE;
}
/**********************************************************/
//函数名：camera_init
//功能  ：初始化摄像头---->将4帧空缓存放入队列，并打开视频显示功能
//形参  ：无
//返回值：经过初始化操作后的文件描述符 fd
/***********************************************************/
static status camera_init(void)
{
	int fd = -1;
    //1.打开V4L2设备
    	if (-1 == (fd = open(FILE_VIDEO, O_RDWR))) {
    		printf("open V4L2 error.\n");
       		return FAILURE;
	}   
    //2.设置视频制式和帧格式
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;		//设置帧类型为可以捕获的类型
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;//设置帧格式为V4L2_PIX_FMT_YUYV格式
	fmt.fmt.pix.height = HEIGHT;				//设置帧高度
	fmt.fmt.pix.width = WIDTH;					//帧宽度
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;	//设置成寄偶场
	if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
		printf("Unable to set format\n");
		return FAILURE;
	}
	
	//3.申请帧缓存，并管理
		//3.1.申请帧缓存
	req.count = COUNT_NUM;   
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(-1 == ioctl(fd,VIDIOC_REQBUFS,&req)) {
		printf("%s: VIDIOC_REQBUFS FAILED\n",FILE_VIDEO);
		return 	-FAILURE;
	}
		//3.2.将缓存映射到用户空间
  	buffers = (struct buffer *)calloc(req.count, sizeof(struct buffer));
   	if (NULL == buffers) {
        printf ("calloc: Out of memory\n");
        return FAILURE;
   	}

	for(n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		// 查询序号为n_buffers 的缓冲区，得到其起始物理地址和大小
		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
			printf("VIDIOC_QUERYBUF error\n");
			return FAILURE;
		}
	
		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | \
								PROT_WRITE, MAP_SHARED,\
								fd, buf.m.offset);
		if (MAP_FAILED == buffers[n_buffers].start) {
			printf("mmap failed\n");
			return FAILURE;
		}
	} 	
	
	
		//4.3.开始采集		
	unsigned int i;
	for(i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory= V4L2_MEMORY_MMAP;
		buf.index = i;
		if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
		printf("VIDIOC_QBUF failed\n");
	}		
		//4.4.打开视频显示功能
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMON, &type))	{
		printf("VIDIOC_STREAMON failed\n");
		return FAILURE;//开机启动一次，后续无须再次启动		
	}

	printf("init %s \t[OK]\n",FILE_VIDEO);
	
	return fd;
}

/**********************************************************/
//函数名：camera_close
//功能  ：关闭摄像头---取出缓冲帧->关闭流->解除映射->关闭fd
//形参  ：fd
//返回值：SUCCESS/FAIURE
/***********************************************************/
static status camera_close(int fd)
{	
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	int j = 0;
	struct v4l2_buffer buf;
	for( ; j< req.count; j++)	
		ioctl(fd,VIDIOC_DQBUF,&buf);

	if(-1 == ioctl (fd, VIDIOC_STREAMOFF, &type)) {
		printf("%s: close stream failed.\n", FILE_VIDEO);
		return FAILURE;
	}

	int ret =0;
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)	{
		ret = munmap(buffers[n_buffers].start, buffers[n_buffers].length);
		if(ret < 0)	{
			printf("munmap video buffers.\n");
			return FAILURE;
		}
	}
	
	close(fd);
	
	return SUCCESS;
}

/**********************************************************/
//函数名：get_camera_jpg
//功能  ：从缓冲区获取图像
//形参  ：fd、jpg_buf
//返回值：SUCCESS/FAIURE
/***********************************************************/
status get_camera_jpg(int fd, ImageBuffer *jpg_buf)
{
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
	{
				
	}

    jpg_buf->length = yuv_jpg((unsigned char*)buffers[buf.index].start, jpg_buf->start, \
		(WIDTH * HEIGHT), 25);
    
	ioctl(fd, VIDIOC_QBUF, &buf); 

    return SUCCESS;

}

/**********************************************************/
//函数名：yuv_jpg
//功能  ：YUV格式到JPG格式
//形参  ：quality(0~100之间的整数，表示压缩比率)
//返回值：SUCCESS/FAIURE
/***********************************************************/
status yuv_jpg(unsigned char *buf, unsigned char *buffer, int size, int quality) 
{

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	
    JSAMPROW row_pointer[1];
    unsigned char *line_buffer, *yuyv;
    int z;
    static int written;
	
    line_buffer = (unsigned char*)calloc(WIDTH * 3, 1);
    yuyv = buf;
		
    dest_buffer(&cinfo, buffer, size, &written);
	
    cinfo.image_width = WIDTH;
    cinfo.image_height = HEIGHT;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
	
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    z = 0;

    while(cinfo.next_scanline < HEIGHT) {
        int x;
        unsigned char *ptr = line_buffer;
		
        for(x = 0; x < WIDTH; x++) {
            int r, g, b;
            int y, u, v;
            if(!z)
            y = yuyv[0] << 8;
            else
            y = yuyv[2] << 8;
			
            u = yuyv[1] - 128;
            v = yuyv[3] - 128;
			
            r =(y +(359 * v)) >> 8;
            g =(y -(88 * u) -(183 * v)) >> 8;
            b =(y +(454 * u)) >> 8;
			
            *(ptr++) =(r > 255) ? 255 :((r < 0) ? 0 : r);
            *(ptr++) =(g > 255) ? 255 :((g < 0) ? 0 : g);
            *(ptr++) =(b > 255) ? 255 :((b < 0) ? 0 : b); 
            if(z++) {
                z = 0;
                yuyv += 4;
            }

        }

        row_pointer[0] = line_buffer;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);

    }
	
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	free(line_buffer); 
	
	return(written);

}

METHODDEF(void) init_destination(j_compress_ptr cinfo) {

    mjpg_dest_ptr dest =(mjpg_dest_ptr) cinfo->dest;
    dest->buffer =(JOCTET *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_IMAGE, OUTPUT_BUF_SIZE*sizeof(JOCTET));
    *(dest->written) = 0;
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

}

METHODDEF(boolean) empty_output_buffer(j_compress_ptr cinfo) {

    mjpg_dest_ptr dest =(mjpg_dest_ptr) cinfo->dest;
    memcpy(dest->outbuffer_cursor, dest->buffer, OUTPUT_BUF_SIZE);
    dest->outbuffer_cursor += OUTPUT_BUF_SIZE;
    *(dest->written) += OUTPUT_BUF_SIZE;
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    return TRUE; 

}

METHODDEF(void) term_destination(j_compress_ptr cinfo) {

    mjpg_dest_ptr dest =(mjpg_dest_ptr) cinfo->dest;
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
    memcpy(dest->outbuffer_cursor, dest->buffer, datacount);
    dest->outbuffer_cursor += datacount;
    *(dest->written) += datacount;

}


void dest_buffer(j_compress_ptr cinfo, unsigned char *buffer, int size, int *written) 
{
    mjpg_dest_ptr dest;
    if(cinfo->dest == NULL) {
        cinfo->dest =(struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(mjpg_destination_mgr));

    }

    dest =(mjpg_dest_ptr)cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->outbuffer = buffer;
    dest->outbuffer_size = size;
    dest->outbuffer_cursor = buffer;
    dest->written = written;

}
