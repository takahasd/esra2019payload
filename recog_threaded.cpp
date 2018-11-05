#include<opencv2/opencv.hpp>
#include<cmath>
#include<string>
#include<iostream>
#include<unistd.h>
#include<thread>
using namespace std;
using namespace cv;
class image
{
	public:
		int p[3];
		Mat matrix;
		void open(string image)
		{
			matrix = imread(image, CV_LOAD_IMAGE_COLOR);
			if(!matrix.data)
			{
				cout<<"File does not exist."<<endl;
			}
		}
		void get_pixel(int x,int y)
		{
			cv::Vec3b pixel;
			pixel = matrix.at<cv::Vec3b>(y,x);
			uchar b = pixel[0];
			uchar g = pixel[1];
			uchar r = pixel[2];
			p[0] = int(r);
			p[1] = int(g);
			p[2] = int(b);
		}
		void set_pixel(int x, int y, int r, int g, int b)
		{
			cv::Vec3b pixel;
			pixel[2] = uchar(r);
			pixel[1] = uchar(g);
			pixel[0] = uchar(b);
			matrix.at<cv::Vec3b>(y,x) = pixel;
		}
		image crop(int x1, int y1, int x2, int y2)
		{
			image new_img;
			Mat blank((y2-y1),(x2-x1),CV_8UC3,Scalar(0,0,0));
			new_img.matrix = blank;
			for(int i = x1;i<x2;i++)
			{
				for(int j=y1;j<y2;j++)
				{
					new_img.matrix.at<cv::Vec3b>(j-y1,i-x1) = matrix.at<Vec3b>(j,i);
				}
			}
			return new_img;
		}
		image re_size(int width,int height)
		{
			image resized;
			cv::resize(matrix,resized.matrix,cv::Size(width,height));
			return resized;
		}	
	
};		
int sample_check(string var_test,int* r, int* g, int* b, int* var_av,int* var_ma)
{
	image img;
	img.open(var_test);
	int avg_b;
	int avg_r;
	int avg_g;
	int var_max=0;
	int var_avg;
	int var_sum;
	int width = img.matrix.size().width;
	int height = img.matrix.size().height;
	for(int x=0;x<width;x++)
	{
		for(int y=0;y<height;y++)
		{
			img.get_pixel(x,y);
			avg_r += img.p[0];
			avg_g += img.p[1];
			avg_b += img.p[2];
		}
	}
	avg_r = avg_r/(width*height);
	avg_g = avg_g/(width*height);
	avg_b = avg_b/(width*height);
	for(int x=0;x<width;x++)
	{
		for(int y=0;y<height;y++)
		{
			img.get_pixel(x,y);
			int dist_r = avg_r - img.p[0];
			int dist_g = avg_g - img.p[1];
			int dist_b = avg_b - img.p[2];
			int variance = sqrt(dist_r*dist_r + dist_g*dist_g + dist_b*dist_b);
			if(variance > var_max)
			{
				var_max = variance;
			}
			var_sum += variance;
		}
	}
	var_avg = var_sum/(height*width);
	*r = avg_r;
	*g = avg_g;
	*b = avg_b;
	*var_av = var_avg;
	*var_ma = var_max;

}
image img1,img2,img3,img4,var1,var2,var3,var4;
image featureRecog(image img, int x1, int y1, int x2, int y2,int avg_r, int avg_g,int avg_b, int var_avg, int var_max,int det)
{
	image varmap;
	Mat map(120,240,CV_8UC3,Scalar(0,0,0));
	varmap.matrix = map;
	image delg;
	delg = img.crop(x1,y1,x2,y2);
	int gridComplianceCount = 0;
	int gridCount = 0;
	bool bush;
	int variance;
	float gridCompliance;
	for(int x=0;x<delg.matrix.size().width/4;x++)
	{	
		for(int y=0;y<delg.matrix.size().height/2;y++)
		{
			gridCount+=1;
			bush = true;
			gridComplianceCount = 0;
			for(int i=0;i<4;i++)
			{
				for(int j=0;j<2;j++)
				{
					delg.get_pixel(4*x+i,2*y+j);
					variance = sqrt((avg_r-delg.p[0])*(avg_r-delg.p[0]) + (avg_g - delg.p[1])*(avg_g-delg.p[1]) + (avg_b-delg.p[2])*(avg_b-delg.p[2]));
					varmap.set_pixel(4*x+i,2*y+j,variance,variance,variance);
					if(variance<var_avg)
					{
						gridComplianceCount += 1;
					}
					if(variance>var_max)
					{
						//cout<<"Should have found a non-bush"<<endl;
						bush = false;
					}
				}
			}
			if(gridComplianceCount<1)
			{
				bush = false;
			}
			if(bush==true)
			{
				for(int i=0;i<4;i++)
				{
					for(int j=0;j<2;j++)
					{
						delg.set_pixel(4*x+i,2*y+j,255,0,0);
					}
				}
			}		
		}
	}
	if(det==1)
	{
		img1 = delg;
		var1 = varmap;
	}
	if(det==2)
	{
		img2 = delg;
		var2 = varmap;
	}
	if(det==3)
	{
		img3 = delg;
		var3 = varmap;
	}
	if(det==4)
	{
		img4 = delg;
		var4 = varmap;
	}
	return delg;
}
image stitch(image img1, image img2, image img3, image img4)
{
	Mat blank(240,480,CV_8UC3,Scalar(0,0,0));
	for(int x=0;x<240;x++)
	{
		for(int y=0;y<120;y++)
		{
			blank.at<Vec3b>(y,x) = img1.matrix.at<Vec3b>(y,x);
			blank.at<Vec3b>(y,x+240) = img2.matrix.at<Vec3b>(y,x);
			blank.at<Vec3b>(y+120,x) = img3.matrix.at<Vec3b>(y,x);
			blank.at<Vec3b>(y+120,x+240) = img4.matrix.at<Vec3b>(y,x);
		}
	}
	image img;
	img.matrix = blank;
	return img;
}
int main()
{
	int red,green,blue,v_avg,v_max;
	int *r = &red;
	int *g = &green;
	int *b = &blue;
	int *var_max = &v_max;
	int *var_avg = &v_avg;
	string name1,name2;
	cout<<"Sample?"<<endl;
	cin>>name1;
	cout<<"Image?"<<endl;
	cin>>name2;
	sample_check(name1,r,g,b,var_avg,var_max);
	image img;
	img.open(name2);
	image resized = img.re_size(480,240);
	thread first (featureRecog,resized,0,0,240,120,*r,*g,*b,*var_avg,*var_max,1);
	thread second (featureRecog,resized,0,120,240,240,*r,*g,*b,*var_avg,*var_max,2);
	thread third (featureRecog,resized,240,0,480,120,*r,*g,*b,*var_avg,*var_max,3);
	thread fourth (featureRecog,resized,240,120,480,240,*r,*g,*b,*var_avg,*var_max,4);
	first.join();
	second.join();
	third.join();
	fourth.join();
	image done = stitch(img1,img3,img2,img4);
	image varmap = stitch(var1,var3,var2,var4);
	namedWindow("Done",WINDOW_AUTOSIZE);
	imshow("Done",done.matrix);
	namedWindow("varmap",WINDOW_AUTOSIZE);
	imshow("varmap",varmap.matrix);
	waitKey(0);
	VideoCapture cap(0);
	Mat frame;
	while(true)
	{
		cap.read(frame);
		imshow("camera",frame);
		if(waitKey(30)==27)
		{
			return 0;
		}
	}
	return 0;
}
