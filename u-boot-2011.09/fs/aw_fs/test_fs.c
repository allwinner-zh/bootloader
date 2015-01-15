#include "ff.h"
#include<common.h>
char test_buf[1024*1024];
char *buf=test_buf;
void file_test(void)
{
	FATFS test_fs;
	FRESULT ret;
	FIL fsrc,fsdt;
	UINT br,bw;
	unsigned int count=0;
	int i;
	if(!buf)
		printf("malloc fail\n");
	else
		printf("buf in %p\n",buf);

	ret = f_mount(0,&test_fs);
#if 0
	if(ret)
		printf("%s err in f_mount ret %d %d\n",__func__,ret,__LINE__);

	ret = f_open (&fsrc[0], "0:test0.png",FA_OPEN_EXISTING |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open ret %d %d\n",__func__,ret,__LINE__);

	ret = f_open (&fsrc[1], "0:test1.png",FA_OPEN_EXISTING |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open ret %d %d\n",__func__,ret,__LINE__);

	ret = f_open (&fsrc[2], "0:test2.png",FA_OPEN_EXISTING |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open ret %d %d\n",__func__,ret,__LINE__);

	ret = f_open (&fsrc[3], "0:test3.png",FA_OPEN_EXISTING |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open ret %d %d\n",__func__,ret,__LINE__);

	ret = f_open (&fsrc[4], "0:test4.png",FA_OPEN_EXISTING |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open ret %d %d\n",__func__,ret,__LINE__);
	memset(buf,0,1024*1024);
#endif
#if 1
	ret = f_open (&fsrc, "0:test.jpg",FA_OPEN_EXISTING |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open ret %d %d\n",__func__,ret,__LINE__);
	ret = f_open (&fsdt, "0:dma1.ppt",FA_CREATE_ALWAYS |FA_READ | FA_WRITE );
	if(ret)
		printf("%s err in f_open %d %d\n",__func__,ret,__LINE__);
//	f_read(&fsrc,buf,1024*1024,&br);
	printf("[%s] br is %d %d\n",__func__,br,__LINE__);
#endif
	tick_printf(__FILE__, __LINE__);
#if 0
	for(i=0;i<5;i++){
		printf("buf[%d] 0x%x !\n",i,buf+i*2048);
		if(i!=4)
			f_read(&fsrc[i],buf+i*2048,1024,&br);
		else
			f_read(&fsrc[i],buf+i*2048,1024*30,&br);
	}
#endif
		f_read(&fsrc,buf,1024*1024,&br);
		printf("[%s] br is %d %d\n",__func__,br,__LINE__);
	tick_printf(__FILE__, __LINE__);
	for(i=0;i<100;i++){
		f_write(&fsdt,buf,br,&bw);
		count += bw;
	}
#if 0
		f_read(&fsrc,buf,11,&br);
		printf("[%s] br is %d %d\n",__func__,br,__LINE__);
		f_write(&fsdt,buf,br,&bw);
		count += bw;
#endif
	tick_printf(__FILE__, __LINE__);
	printf("[%s] count %d %d\n",__func__,count,__LINE__);
	//printf("read %d bytes\n",br);
	//f_write(&fsdt,buf,br,&bw);
	//printf("write %d bytes\n",bw);
#if 0
	for(i=0;i<5;i++)
		f_close(&fsrc[i]);
#endif
	f_close(&fsrc);
	f_close(&fsdt);
	tick_printf(__FILE__, __LINE__);
	f_mount(0,NULL);
}
