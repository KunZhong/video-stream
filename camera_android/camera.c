/*************************************************************************
  > File Name: camera.c
	> Author: 
	> Mail: 
	> Created Time: 2016年04月25日 星期一 10时48分27秒
 ************************************************************************/

#include "camera.h"

/* 线程调用函数，一直采集图片
*  传入图片缓冲区，互斥使用图片资源
*  返回成功与否
*/
status camera_capture(ImageBuffer *imgbuf)
{
	if(FAILURE == camera_open()){
		return FAILURE;
	}
	while (1) {			

		pthread_mutex_lock(&mutex);

		get_camera_jpg(get_camera_fd(), imgbuf);
	
		pthread_mutex_unlock(&mutex);
		
		usleep(100000);
	}	
	return SUCCESS;
}
