#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>    
#include <opencv2/imgproc/imgproc.hpp>    
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vector> 
#include <iostream>
#include <fstream>
#include "Header.h"
#include <sys/types.h> 
#include <sys/stat.h> 
#include <direct.h>
//#include <boost/filesystem.hpp>
extern "C" {
#include <jpeglib.h>
#include <jconfig.h>
#include <jmorecfg.h>
#include <jinclude.h>
}
using namespace cv;
using namespace std;
//#pragma comment(lib,"jpeg.lib") 
#define HLEN     ((sizeof(struct __mp__)))
mp_t *heap_start;
int *arr = NULL;
impl_t cmalloc;
enum {
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
};
static struct buf_mem_pool g_buf_mem_pool = { NULL, -1, NULL, 0 };
//struct jpeg_compress_struct cinfo;
//struct jpeg_error_mgr jerr;

static int _dprintf(unsigned int level, const char* fmt, va_list va)
{
	int len = 0;
	msg_log_t msg;

	uint32_t t = 0;//timer_current_ms();

	if (level > 4)
		return -1;

	//len = _snprintf((char *)msg.str, sizeof(msg.str), "%u.%03u: ",(t / 1000) & 0xFF, t % 1000);
	len--; //remove '\0'

	//len += _vsnprintf((char *)msg.str + len, sizeof(msg.str) - len, fmt, va);


	msg.head.size = (sizeof(msg.head) + len + 3) >> 2; /* 4 bytes align */
	msg.head.type = 0x0400; //id_msg_dsp_log_t;
	msg.head.id.core_id = 0; //current_core();
	msg.head.mux.log_level = level;
	//msq_send_msg((msg_t *)&msg);



	return 0;
}
int dprintf(unsigned int level, const char* fmt, ...)
{
	int ret;
	va_list va;

	va_start(va, fmt);
	ret = _dprintf(level, fmt, va);
	va_end(va);

	return ret;
}

void *mem_pool_init(void *heap_start, void *heap_end)
{
	/*  Set the __mp__ to point to the beginning of the pool and set
	*  the pool size.
	*/
	unsigned int  size;
	struct __mp__ *heap_info;

	heap_start = (void *)ALIGNED(heap_start);
	size = (unsigned int)((char *)heap_end - (char *)heap_start);
	size = ALIGNMIX(size);
	printf("malloc Address[%x,%x][%d %d],%d\n", (uint32_t)&arr[0], (uint32_t)&arr[N - 1], \
		(uint32_t)cmalloc.startAddr, (uint32_t)cmalloc.endAddr, cmalloc.len);
	heap_info = (struct __mp__ *) heap_start;
	heap_info->next = NULL;
	heap_info->len = size;
	heap_info->rlen = size;
	//printf("index:%x,%x,%x,%x\n", (uint32_t)heap_info, &heap_info->pc,
	//&heap_info->rlen, &heap_info->len);
	printf("$$index:%x,%d,%d\n", (uint32_t)heap_info, heap_info->len, heap_info->rlen);
	/*  Set the link of the block in the pool to NULL (since it's the only
	*  block) and initialize the size of its data area.
	*/
	heap_info++;
	//printf("index:%x,%x,%x,%x\n", (uint32_t)heap_info, &heap_info->pc,
	//	&heap_info->rlen, &heap_info->len);
	printf("++%x,%d,%d,%d,%d\n", (uint32_t)heap_info, sizeof(struct __mp__), sizeof(struct __mp__ *), sizeof(unsigned int), sizeof(short));
	heap_info->next = NULL;
	heap_info->pc = 0;
	heap_info->len = size - 2 * HLEN;
	heap_info->rlen = size - 2 * HLEN;
	printf("init succe index:%x,%d,%d\n", (uint32_t)heap_info, heap_info->len, heap_info->rlen);
	return heap_info;
}
void *rk_malloc(void *heap_addr, unsigned int size, unsigned int pc)
{
	struct __mp__ *q;           /* ptr to free block */
	struct __mp__ *p;           /* q->next */
	unsigned int  k;            /* space remaining in the allocated block */
	unsigned int  sizeb = size;

	size = ALIGNED(size);
	/*  Initialization:  Q is the pointer to the next available block.*/
	q = (struct __mp__ *) heap_addr;

	while (1) {

		p = q->next;
		if (!p){
			printf("!!!!!!!!!!!!!\n");
			return NULL;
		}
		if (p->len >= size)
			break;
		q = p;
	}
	k = p->len - size;  /* calc. remaining bytes in block */

	k -= (16);
	p->len = k;

	q = (struct __mp__ *)(((char *)(&p[1])) + k);
	printf("@@@@q->next:%x,len=%d,p->next=%x\n", (uint32_t)p, p->len, (uint32_t)p->next);

	q->len = size;
	q->rlen = sizeb;
	q->pc = pc;
	q->next = p->next;

	return &q[1];
}
void getadd_plus(int **point)
{
	**point = 10;
}
void getadd(int *p_point)
{
	getadd_plus(&p_point);
}

typedef union {
	long long int a;
	short b;
}union_t;
//YUV420
unsigned char imageFile[(1024 * 768 * 3) >> 1] = { 0 };
#define WIDTH 1024
#define HIGH  768
#define CAPACITY ((WIDTH*HIGH*3)>>2)
/** Y U V order in memory address*/
#define Y_START 0
#define Y_END   ((WIDTH*HIGH) - 1)

#define U_START	 (WIDTH*HIGH)
#define U_END	(((WIDTH*HIGH*5)>>2) - 1)

#define V_START	 ((WIDTH*HIGH*5)>>2)
#define V_END 	 (CAPACITY-1)
//#define __DEBUG 
bool YV12ToBGR24_OpenCV(unsigned char* pYUV, unsigned char* pBGR24, int width, int hight)
{
	//Mat dst, src;
	if (width < 1 || hight < 1 || pYUV == NULL || pBGR24 == NULL)
		return false;
	Mat dst(hight, width, CV_8UC3, pBGR24);//将pBGR24装进dst矩阵(容器)中
	Mat src(hight + hight / 2, width, CV_8UC1, pYUV);//将pYUV装进dst矩阵(容器)中,根据YUV420数据格式,高度为3/2*height,宽度不变
	cvtColor(src, dst, CV_YUV2BGR_YV12);
	return true;
}
int yuv420p_to_jpeg(const char * filename, unsigned char* pdata, int image_width, int image_height, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	FILE * outfile;    // target file  
	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width;  // image width and height, in pixels  
	cinfo.image_height = image_height;
	cinfo.input_components = 3;    // # of color components per pixel  
	cinfo.in_color_space = JCS_YCbCr;  //colorspace of input image  
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	//////////////////////////////  
	//  cinfo.raw_data_in = TRUE;  
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
	/////////////////////////  

	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer[1];

	unsigned char *yuvbuf;
	if ((yuvbuf = (unsigned char *)malloc(image_width * 3)) != NULL)
		memset(yuvbuf, 0, image_width * 3);

	unsigned char *ybase, *ubase;
	ybase = pdata;
	ubase = pdata + image_width*image_height;
	int j = 0;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		int idx = 0;
		for (int i = 0; i<image_width; i++)
		{
			yuvbuf[idx++] = ybase[i + j * image_width];
			yuvbuf[idx++] = ubase[j / 2 * image_width + (i / 2) * 2];
			yuvbuf[idx++] = ubase[j / 2 * image_width + (i / 2) * 2 + 1];
		}
		row_pointer[0] = yuvbuf;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
		j++;
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outfile);
	return 0;
}
void imgToBin(){
	ofstream outfile;//内存到磁盘
	outfile.open("D:/Image/dat.bin", ios::binary);
	
	vector<String> files;
	String dir_path = "D:/Image/*.jpg";   //读取该目录下文件

	glob(dir_path, files, false);

	cout << "dddddde=" << files.size() << endl;

	for (int i = 0; i < files.size(); i++)
	{
		Mat Iface = imread(files[i].c_str());
		cout << "size" << Iface.size << "row" << Iface.rows << "col" << Iface.cols <<endl;
		for (int r = 0; r < Iface.rows; r++)//uchar* Mat::ptr(int y)
		{
			//write(const unsigned char *buf,int num);  从buf中写num大小的内容到磁盘中

			outfile.write(reinterpret_cast<char*>(Iface.ptr(r)), Iface.cols*Iface.elemSize());
		}
		cout << i << endl;
	}
}

void binToImg()
{
	int height = 768;//1080;row
	int width = 1024;//1920;col
	int img_num = 1;//字典中图片的尺寸与图片总数量需提前知道

	ifstream infile;//ifstream 磁盘到内存
	infile.open("D:/Image/dat.bin", ios::binary);

	vector<Mat> dict;
	Mat img = Mat::zeros(height, width, CV_8UC3);//1920*1080:CV_16UC1;1024*768:CV_8UC3

	cout << "img.size:" << img.size() << " img.rows=" << img.rows << " img.elemSize()=" << img.elemSize()<<" img.elemSize1()=" << img.elemSize1() << endl;
	for (int num = 0; num < img_num; num++)
	{
		for (int r = num; r < num + img.rows; r++)//从bin文件中读size到rambuf中
			infile.read(reinterpret_cast<char*>(img.ptr(r - num)), img.cols*img.elemSize());//1920*2byte
		dict.push_back(img);
	}
	namedWindow("img", WINDOW_AUTOSIZE);
	imshow("img", dict[0]);
	waitKey(0);
}
#if 0
int get_filenames(const std::string& dir, std::vector<std::string>& filenames)
{
	fs::path path(dir);
	if (!fs::exists(path))
	{
		return -1;
	}

	fs::directory_iterator end_iter;
	for (fs::directory_iterator iter(path); iter != end_iter; ++iter)
	{
		if (fs::is_regular_file(iter->status()))
		{
			filenames.push_back(iter->path().string());
		}

		if (fs::is_directory(iter->status()))
		{
			get_filenames(iter->path().string(), filenames);
		}
	}

	return filenames.size();
}
#endif
int procVedio(const std::string & sVedioPath)
{
	//VideoCapture::
	vector<String> files;
	String dir_path = "D:\\Image\\*.mp4";   //读取该目录下文件
	glob(dir_path, files, false);


	string sImgPath = sVedioPath + "\\imgdir\\" ;
	
#ifdef __DEBUG
	cout << "保存路劲：" << sImgPath.c_str() << endl;
#endif
	Mat frame;
	bool flags = true;
	long currentFrame = 0;
	for (int i = 0; i < files.size(); i++)
	{
		cv::VideoCapture cap(files[i]);
		int allFrameNum = cap.get(CV_CAP_PROP_FRAME_COUNT);
#ifdef __DEBUG
		cout << " Vedio# "<< i << ":" <<files[i].c_str()<< "\tTotal Frames: " << allFrameNum << endl;
#endif
		currentFrame = 0;
		flags = true;
#if 1
		while (flags)
		{
			// 读取视频每一帧
			cap.read(frame);
			stringstream str;
			str << "Vedio#" << i << "__Frame#" << currentFrame << ".jpg";
#ifdef __DEBUG
			cout << "正在处理第#" << currentFrame << "帧" << endl;
			cout << "结果路劲:" << sImgPath.c_str() + str.str() << endl;
			//if (currentFrame >= 10)
			//	break;
			if (0 == (currentFrame % 300))
#endif
			{
				imwrite(sImgPath.c_str() + str.str(), frame);
			}

			if (currentFrame >= allFrameNum){
				flags = false;
			}
			currentFrame++;
		}
	}
#endif
	return 0;
}
int main(int argc, char** argv)
{
	//Mat srcImage = imread("Desert.jpg");
	//namedWindow("Desert", WINDOW_AUTOSIZE);
	//imshow("Desert", srcImage);

	//Mat srcImageGray;                       //创建一个Mat类型用于存储将读取到的彩色图像转换为灰度图之后的图像
	//cvtColor(srcImage, srcImageGray, CV_RGB2GRAY);      //使用函数CV_RGB2GRAY将彩色图像转换为灰度图
	//namedWindow("DesertGray", WINDOW_NORMAL);
	//imshow("DesertGray", srcImageGray);
	//imwrite("DesertGray.jpg", srcImageGray);          //将转换的灰度图以.bmp格式存储，默认路径为工程目录下
	//waitKey(0);
	//printf("hello intellif!\n");
	int index = 349;
	Mat m1(2, 2, CV_8UC3, Scalar(1, 2, 3));
	//cout << "M1=" << endl << " " << m1 << endl;
	Mat m2(3, 2, CV_8UC2, Scalar(6, 8, 10));
	//cout << "M2=" << endl << " " << m2 << endl;
	//binToImg();
	string localpath = "D:\\Image";
	double dMultiTestTime = (double)cvGetTickCount();
	procVedio(localpath.c_str());
	double dTime = ((double)cvGetTickCount() - dMultiTestTime) / cvGetTickFrequency() / 1000000;
	cout << "SingleThread Done! Time: " << dTime << "s." << endl;
#if 0
	void *getMalloc = NULL;
	arr = (int *)malloc(sizeof(int)* N);
	cmalloc.startAddr = (void *)&arr[0];
	cmalloc.endAddr = (void *)&arr[N];//decltype(arr)
	cmalloc.len = (uint32_t)((char *)cmalloc.endAddr - (char *)cmalloc.startAddr);//byte
	printf("malloc Address[%x,%x][%d %d],%d\n", (uint32_t)&arr[0], (uint32_t)&arr[N - 1], \
		(uint32_t)cmalloc.startAddr, (uint32_t)cmalloc.endAddr, cmalloc.len);
	cmalloc.startAddr = (void *)ALIGNED(cmalloc.startAddr);
	cmalloc.len = (unsigned int)((char *)cmalloc.endAddr - (char *)cmalloc.startAddr);
	printf("malloc Address[%x,%x][%d %d],%d\n", (uint32_t)&arr[0], (uint32_t)&arr[N - 1], \
		(uint32_t)cmalloc.startAddr, (uint32_t)cmalloc.endAddr, cmalloc.len);


	g_buf_mem_pool.mem_heap = mem_pool_init((void *)cmalloc.startAddr, \
		(void *)cmalloc.endAddr);
	printf("init heap index:%x\n", (uint32_t)g_buf_mem_pool.mem_heap);

	getMalloc = rk_malloc(&g_buf_mem_pool.mem_heap, 4, 0);
	printf("getMalloc:%x\n", (uint32_t *)getMalloc);
	getMalloc = rk_malloc(&g_buf_mem_pool.mem_heap, 4, 0);
	printf("getMalloc:%x,%x,%d\n", (uint32_t)getMalloc, (uint32_t)g_buf_mem_pool.mem_heap, sizeof(struct __mp__));
	int point;
	getadd(&point);
	union_t varUnion;
	int overArr[5] = { 0 };
	printf("point=%d,%d,%d，%d\n", overArr[0], overArr[-1]);
	if (NULL != arr)
	{
		free(arr);
		arr = NULL;
	}


	FILE *inputFile = NULL;
	FILE *outputFile = NULL;
	unsigned char *outSteam = (unsigned char*)malloc(sizeof(char)*(WIDTH*HIGH * 3 >> 2));
	uint32_t size = 0;
	inputFile = fopen("Desert.jpg", "rb");
	outputFile = fopen("outputFile.jpg", "wb+");
	if (NULL == inputFile)
		perror("inputFile error!");
	//size = YV12ToBGR24_OpenCV((unsigned char*)inputFile, outSteam, WIDTH, HIGH);
	//fwrite(outSteam, WIDTH*HIGH * 3 >> 2, 1, outputFile);
	char temp = 0;
	for (index = 0; index < CAPACITY; index++)
	{
		fscanf(inputFile, "%c", &temp);
		imageFile[index] = temp;
		//fprintf(outputFile,"%x\t", imageFile[index]);
		if (index == 10)
			puts("\n");
	}
	if (NULL != outSteam)
		free(outSteam);
	fclose(inputFile);
	fclose(outputFile);
	ofstream outfile;
#endif

	//	函数名：glob
	//参数：String pattern 字符串，由文件夹路径和所要读取的文件名的格式构成的一个正则模板
	//	参数： std::vector<String>& result 字符串容器，存放所有符合模板的路径
	//参数：bool recursive = false 标志位，是否使用递归的方式（没发现使用true和false的结果有什么区别）
	//功能描述：遍历指定的文件夹，读取符合搜索模板的所有文件路径	
	//imgToBin();
	//binToImg();

	system("pause");


	return 0;
}

/*
int main()
{
int height = 200;
int width = 200;
int img_num = 700;//字典中图片的尺寸与图片总数量需提前知道

ifstream infile;
vector dict;
infile.open("data.bin", ios::binary);
Mat img = Mat::zeros(height, width, CV_8UC3);

for (int num = 0; num < img_num; num++)
{
for (int r = num; r < num+img.rows; r++)
infile.read(reinterpret_cast(img.ptr(r-num)), img.cols*img.elemSize());

dict.push_back(img);
}
}
*/