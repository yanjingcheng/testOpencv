#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>    
#include <opencv2/imgproc/imgproc.hpp>    
#include <opencv2/highgui/highgui.hpp>
//#include "json/json.h"
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
#include <direct.h>//for mk_dir  
#include <io.h>//for _acess() 

//#include <boost/lexical_cast.hpp>
//#include <boost/filesystem.hpp>
extern "C" {
#include <jpeglib.h>
#include <jconfig.h>
#include <jmorecfg.h>
#include <jinclude.h>
}
using namespace cv;
using namespace std;
//using namespace boost;
//#pragma comment(lib,"jpeg.lib") 

static struct buf_mem_pool g_buf_mem_pool = { NULL, -1, NULL, 0 };
//struct jpeg_compress_struct cinfo;
//struct jpeg_error_mgr jerr;


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
#define __DEBUG 

void dir(string path)
{
 long hFile = 0;
 struct _finddata_t fileInfo;
 string pathName, exdName;
 // \\* ����Ҫ�������е�����
 if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1) {
  return;
 }
 do 
 {
  //�ж��ļ����������ļ��л����ļ�
  cout << fileInfo.name << (fileInfo.attrib&_A_SUBDIR? "[folder]":"[file]") << endl;
 } while (_findnext(hFile, &fileInfo) == 0);
 _findclose(hFile);
 return;
}

//�����༶Ŀ¼
int recursive_mkdir(char *dir)
{
	//�ֽ�·����E:\\AA\\BB\\CC\\  
	//  
	std::string str = dir;
	size_t index = 0;
	int i = 0;
	while (1)
	{
		std::string::size_type pos = str.find("\\", index);
		std::string str1;
		str1 = str.substr(0, pos);
		if (pos != -1 && i > 0)
		{
			if (_access(str1.c_str(), 0) == -1)
			{
				_mkdir(str1.c_str());
			}
		}
		if (pos == -1)
		{
			break;
		}
		i++;
		index = pos + 1;
	}
	return 0;
}
bool YV12ToBGR24_OpenCV(unsigned char* pYUV, unsigned char* pBGR24, int width, int hight)
{
	//Mat dst, src;
	if (width < 1 || hight < 1 || pYUV == NULL || pBGR24 == NULL)
		return false;
	Mat dst(hight, width, CV_8UC3, pBGR24);//��pBGR24װ��dst����(����)��
	Mat src(hight + hight / 2, width, CV_8UC1, pYUV);//��pYUVװ��dst����(����)��,����YUV420���ݸ�ʽ,�߶�Ϊ3/2*height,��Ȳ���
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
	ofstream outfile;//�ڴ浽����
	outfile.open("D:/Image/dat.bin", ios::binary);
	
	vector<String> files;
	String dir_path = "D:/Image/*.jpg";   //��ȡ��Ŀ¼���ļ�

	glob(dir_path, files, false);

	cout << "dddddde=" << files.size() << endl;

	for (int i = 0; i < files.size(); i++)
	{
		Mat Iface = imread(files[i].c_str());
		cout << "size" << Iface.size << "row" << Iface.rows << "col" << Iface.cols <<endl;
		for (int r = 0; r < Iface.rows; r++)//uchar* Mat::ptr(int y)
		{
			//write(const unsigned char *buf,int num);  ��buf��дnum��С�����ݵ�������

			outfile.write(reinterpret_cast<char*>(Iface.ptr(r)), Iface.cols*Iface.elemSize());
		}
		cout << i << endl;
	}
}

void binToImg()
{
	int height = 768;//1080;row
	int width = 1024;//1920;col
	int img_num = 1;//�ֵ���ͼƬ�ĳߴ���ͼƬ����������ǰ֪��

	ifstream infile;//ifstream ���̵��ڴ�
	infile.open("D:/Image/dat.bin", ios::binary);

	vector<Mat> dict;
	Mat img = Mat::zeros(height, width, CV_8UC3);//1920*1080:CV_16UC1;1024*768:CV_8UC3

	cout << "img.size:" << img.size() << " img.rows=" << img.rows << " img.elemSize()=" << img.elemSize()<<" img.elemSize1()=" << img.elemSize1() << endl;
	for (int num = 0; num < img_num; num++)
	{
		for (int r = num; r < num + img.rows; r++)//��bin�ļ��ж�size��rambuf��
			infile.read(reinterpret_cast<char*>(img.ptr(r - num)), img.cols*img.elemSize());//1920*2byte
		dict.push_back(img);
	}
	namedWindow("img", WINDOW_AUTOSIZE);
	imshow("img", dict[0]);
	waitKey(0);
}
#if 0 //boost��ʹ�÷����������޷�ͨ��
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
void WriteYuv()
{
	cv::VideoCapture vc;
	bool flag = vc.open("D:/testOpencvLib/testOpencv/x64/Debug/20180418174450239_02c.avi");
	if (!flag)
	{
		printf("avi file open error \n");
		system("pause");
		exit(-1);
	}

	int frmCount = vc.get(CV_CAP_PROP_FRAME_COUNT);
	frmCount -= 5;

	int w = vc.get(CV_CAP_PROP_FRAME_WIDTH);
	int h = vc.get(CV_CAP_PROP_FRAME_HEIGHT);
	int bufLen = w*h * 3 / 2;
	printf("frmCount: %d,buflen:%d,[w*h]=%d*%d\n", frmCount, bufLen,w,h);
	unsigned char* pYuvBuf = new unsigned char[bufLen];
	FILE* pFileOut = fopen("D:/testOpencvLib/testOpencv/x64/Debug/result.yuv", "w+");
	if (!pFileOut)
	{
		printf("pFileOut open error \n");
		system("pause");
		exit(-1);
	}
	printf("pFileOut open ok \n");

	for (int i = 0; i<100; i++)
	{
		printf("%d/%d \n", i + 1, frmCount);

		cv::Mat srcImg;
		vc >> srcImg;

		//cv::imshow("img", srcImg);
		//cv::waitKey(1);

		cv::Mat yuvImg;
		cv::cvtColor(srcImg, yuvImg, CV_RGB2YUV_IYUV);
		memcpy(pYuvBuf, yuvImg.data, bufLen*sizeof(unsigned char));

		fwrite(pYuvBuf, bufLen*sizeof(unsigned char), 1, pFileOut);
	}

	fclose(pFileOut);
	delete[] pYuvBuf;
}

int procVedio(const std::string & sVedioPath)
{
	//VideoCapture::
	vector<String> files;
	//String dir_path = sVedioPath + "\\*.avi";
	String dir_path = "D:\\Image\\*.mp4";   //��ȡ��Ŀ¼���ļ�
	glob(dir_path, files, false);


	string sImgPath = sVedioPath + "\\imgdir\\" ;
	
#ifdef __DEBUG
	cout << "\t\t����·����" << sImgPath.c_str() << endl;
#endif
	Mat frame;
	bool flags = true;
	long currentFrame = 0;
	for (int i = 0; i < files.size(); i++)
	{
		cv::VideoCapture cap(files[i]);
		if (!cap.isOpened())
		{
			perror("error");
		}
		double allFrameNum = cap.get(CV_CAP_PROP_FRAME_COUNT);
#ifdef __DEBUG
		cout << " Vedio# "<< i << ":" <<files[i].c_str()<< "\tTotal Frames: " << allFrameNum << endl;
#endif
		currentFrame = 0;
		flags = true;
#if 1
		while (flags)
		{
			// ��ȡ��Ƶÿһ֡
			cap.read(frame);
			stringstream str;
			str << "Vedio#" << i << "__Frame#" << currentFrame << ".jpg";
#ifdef __DEBUG
			cout << "���ڴ����#" << currentFrame << "֡" << endl;
			cout << "���·��:" << sImgPath.c_str() + str.str() << endl;
			//if (currentFrame >= 10)
			//	break;
			if (0 == (currentFrame % 100))
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
void cvtYuv420Format(u8 *src, s32 height, s32 width)	
{
	u8 *buf = new u8[height * width / 2];
	//UU...U VV...V����
	memcpy(buf, src + height * width, height * width / 2);
	//��UU...U VV...V�ӱ��ݴ���������Ӧ��λ��
	for (s32 i = 0; i < height*width / 4; i++)
	{
		*(src + height*width + 2 * i) = *(buf + i);
		*(src + height*width + 2 * i + 1) = *(buf + height*width / 4 + i);
	}
	delete[]buf;
}
#if 0 //�Ż�
void cvtYuv420Format_opt(u8 *src, s32 height, s32 width)
{
	int64_t YYY = height*width;
	for (u32 h = 0; h < (height >> 2); ++h)
		for (u32 w = 0; w < (width - 1); ++w)
			*(src + YYY + w + 1) = (src + YYY + w + 1)
	
}
#endif
void imgToVedio()
{
	// ����һ��VideoWriter
	VideoWriter videoWrite;
	int codec = CV_FOURCC('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
	double fps = 20.0;                          // framerate of the created video stream
	string filename = "./test.avi";             // name of the output video file
	//string filename = "E:\\Video2YUV\\ZH_0426pictures\\test.avi";
	videoWrite.open(filename, codec, fps, Size(640,320));
	if (!videoWrite.isOpened())
		perror("Create vedio failed !");
	//VideoWriter videoWrite("E:\\Video2YUV\\ZH_0426pictures\\test.avi", CV_FOURCC('M', 'J', 'P', 'G'), 20.0, Size(640, 320));
	// ��һ���ļ����¶�ȡ����jpgͼƬ
	//String pattern = "E:\\Video2YUV\\ZH_0426pictures\\*.jpg";
	String pattern = "./*.jpg";
	vector<String> fn;

	glob(pattern, fn, false);

	size_t count = fn.size();
	for (size_t i = 0; i < count; i++)
	{
		Mat image = imread(fn[i]);
		// �����С��VideoWriter���캯���еĴ�Сһ�¡�
		resize(image, image, Size(640, 320));
		// ������������ͼƬ������Ƶ
		videoWrite << image;
		cout << count << "read img" << fn[i].c_str() << " to vedio" << endl;
	}
	cout << "Convertion done��" << endl;
}

int testCapVedioToImg()//����Ƶת��ͼƬ
{
	//��Ĭ������ͷ
	VideoCapture cap("D:\\Image\\20180420102639472_wp#05.mp4");
	if (!cap.isOpened())
	{
		return -1;
	}

	Mat frame;
	// ��Q���˳�ʱ��������Ҫ��ΪӢ��ģʽ
	while(waitKey(30) != 'q')
	{
		// ͨ��������������Ƶת��Ϊһ֡֡ͼƬ
		cap >> frame;
		// Do something here 
		imshow("video", frame);
		//imwrite(path, frame);
		//waitKey(0);
	}
	return 0;
}

//#define NUM_FRAME 3000 //ֻ����ǰ300֡��������Ƶ֡�����޸�  
void Image_to_video(const char* in, const char* out)
{
	int num = 1;
	CvSize size = cvSize(1024, 768);  //��Ƶ֡��ʽ�Ĵ�С  
	double fps = 30; //ÿ���ӵ�֡��  
	CvVideoWriter *writer = cvCreateVideoWriter(out, CV_FOURCC('D', 'I', 'V', 'X'), fps, size); //������Ƶ�ļ�  
	char cname[100];
	sprintf(cname, in, num); //����ͼƬ���ļ��У�ͼƬ�����Ʊ����1��ʼ1��2,3,4,5.������  
	IplImage *src = cvLoadImage(cname);
	if (!src)
	{
		return;
	}
	IplImage *src_resize = cvCreateImage(size, 8, 3); //������Ƶ�ļ���ʽ��С��ͼƬ  
	cvNamedWindow("avi");
	while (src)
	{
		cvShowImage("avi", src_resize);
		cvWaitKey(1);
		cvResize(src, src_resize); //����ȡ��ͼƬ����Ϊ��Ƶ��ʽ��С��ͬ  
		cvWriteFrame(writer, src_resize); //����ͼƬΪ��Ƶ����ʽ  
		cvReleaseImage(&src); //�ͷſռ�  
		num++;
		sprintf(cname, in, num);
		src = cvLoadImage(cname);       //ѭ����ȡ����  
	}
	cvReleaseVideoWriter(&writer);
	cvReleaseImage(&src_resize);
}
void Video_to_image(char* filename, const char* dirname)
//void Video_to_image(char* filename, const char* dirname)
{
	printf("------------- video to image ... ----------------\n");
	//��ʼ��һ����Ƶ�ļ���׽��  
	CvCapture* capture = cvCaptureFromAVI(filename);
	//��ȡ��Ƶ��Ϣ  
	//cvQueryFrame(capture);  
	int frameH = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	int frameW = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int fps = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
	int numFrames = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	printf("\tvideo height : %d\n\tvideo width : %d\n\tfps : %d\n\tframe numbers : %d\n", frameH, frameW, fps, numFrames);
	//����ͳ�ʼ������  
	int i = 0;
	IplImage* img = 0;
	char image_name[130];

	//cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
	////ȡ����ʾ  
	while (1)
	{

		img = cvQueryFrame(capture); //��ȡһ֡ͼƬ  
		//cvShowImage("mainWin", img); //������ʾ  
		//char key = cvWaitKey(20);
		
		sprintf(image_name, "%s%05d%s", dirname, i++, ".jpg");//�����ͼƬ��  

		cvSaveImage(image_name, img); //����һ֡ͼƬ  

		if (i == numFrames) break;
	}
	cvReleaseCapture(&capture);
	cvDestroyWindow("mainWin");
}
#if 1
int main()
{
	//cv::FileStorage fs("E:\\Video2YUV\\pictures\\result.txt", cv::FileStorage::READ);

	char infilename[130] = "E:/vedio/0529/2018-05-30_114456_980.avi";
	const char *dirname = "E:/vedio/0529/2018-05-30_114456_980/";
	//const char *outImagename = "C:/Users/jiang/Desktop/output/breakdancer/cam3/3pic (%d).jpg";
	//const char *outVideoname = "C:/Users/jiang/Desktop/output/3outfile.avi";
	//Image_to_video(outImagename, outVideoname); //ͼƬת��Ƶ
	double dMultiTestTime = (double)cvGetTickCount();
	//imgToVedio();
	WriteYuv();
	//dir(path);
	//string localpath = ".";
	//procVedio("D:/testOpencvLib/testOpencv/x64/Debug/");
	//Video_to_image(infilename, dirname);//AVIתjpg
	double dTime = ((double)cvGetTickCount() - dMultiTestTime) / cvGetTickFrequency() / 1000000;
	printf("Task Done! Time: %fs,TickFreq:%f\n", dTime, cvGetTickFrequency());
	system("pause");
	return 0;
}
#endif
#if 0
int main(int argc, char** argv)
{

	//Mat srcImage = imread("Desert.jpg");
	//namedWindow("Desert", WINDOW_AUTOSIZE);
	//imshow("Desert", srcImage);

	//Mat srcImageGray;                       //����һ��Mat�������ڴ洢����ȡ���Ĳ�ɫͼ��ת��Ϊ�Ҷ�ͼ֮���ͼ��
	//cvtColor(srcImage, srcImageGray, CV_RGB2GRAY);      //ʹ�ú���CV_RGB2GRAY����ɫͼ��ת��Ϊ�Ҷ�ͼ
	//namedWindow("DesertGray", WINDOW_NORMAL);
	//imshow("DesertGray", srcImageGray);
	//imwrite("DesertGray.jpg", srcImageGray);          //��ת���ĻҶ�ͼ��.bmp��ʽ�洢��Ĭ��·��Ϊ����Ŀ¼��
	//waitKey(0);
	//printf("hello intellif!\n");
	//int index = 349;
	//Mat m1(2, 2, CV_8UC3, Scalar(1, 2, 3));
	//cout << "M1=" << endl << " " << m1 << endl;
	//Mat m2(3, 2, CV_8UC2, Scalar(6, 8, 10));
	//cout << "M2=" << endl << " " << m2 << endl;
	//binToImg();

	//u8 src[] = { 98, 98, 98, 98, 98, 98, 98, 98, 
	//         98, 98, 98, 98, 98, 98, 98, 98,
	//         98, 98, 98, 98, 98, 98, 98, 98, 
	//		 98, 98, 98, 98, 98, 98, 98, 98 };

	string localpath = ".";
	double dMultiTestTime = (double)cvGetTickCount();
	//procVedio(localpath.c_str());
	WriteYuv();
	//dir(path);
	
	double dTime = ((double)cvGetTickCount() - dMultiTestTime) / cvGetTickFrequency() / 1000000;
	cout << "SingleThread Done! Time: " << dTime << "s." << endl;
	system("pause");
}
#endif
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
	printf("point=%d,%d,%d��%d\n", overArr[0], overArr[-1]);
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


	//	��������glob
	//������String pattern �ַ��������ļ���·������Ҫ��ȡ���ļ����ĸ�ʽ���ɵ�һ������ģ��
	//	������ std::vector<String>& result �ַ���������������з���ģ���·��
	//������bool recursive = false ��־λ���Ƿ�ʹ�õݹ�ķ�ʽ��û����ʹ��true��false�Ľ����ʲô����
	//��������������ָ�����ļ��У���ȡ��������ģ��������ļ�·��	
	//imgToBin();
	//binToImg();

	system("pause");


	return 0;
}
#endif
/*
int main()
{
int height = 200;
int width = 200;
int img_num = 700;//�ֵ���ͼƬ�ĳߴ���ͼƬ����������ǰ֪��

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