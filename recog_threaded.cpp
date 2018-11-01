#include<opencv2/opencv.hpp>
#include<cmath>
#include<string>
#include<iostream>
#include<unistd.h>
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
			Mat blank(120,240,CV_8UC3,Scalar(0,0,0));
			new_img.matrix = blank;
			for(int i = x1;i<x2;i++)
			{
				for(int j=y1;j<y2;j++)
				{
					new_img.matrix.at<cv::Vec3b>(j,i) = matrix.at<Vec3b>(j,i);
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
int sample_check(string var_test)
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
	cout<<"Image is centered at R";
	cout<<avg_r;
	cout<<" G";
	cout<<avg_g;
	cout<<" B";
	cout<<avg_b<<endl;
	cout<<"Average variance is ";
	cout<<var_avg<<endl;
	cout<<"Max variance is ";
	cout<<var_max<<endl;

}
void featureRecog(image img, int x1, int y1, int x2, int y2,int avg_r, int avg_g,int avg_b, int var_avg, int var_max)
{
	image delg;
	delg = img.crop(x1,y1,x2,y2);
	int gridComplianceCount = 0;
	int gridCount = 0;
	bool bush;
	int variance;
	float gridCompliance;
	for(int x=0;x<((x2-x1)-4)/4;x++)
	{	
		for(int y=0;y<((y2-y1)-2)/2;y++)
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
					cout<<variance<<endl;
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
			cout<<gridComplianceCount<<endl;
			gridCompliance = int(100*float(gridComplianceCount/8));
			cout<<"Grid compliance of grid ";
			cout<<gridCount;
			cout<<" is";
			cout<<gridCompliance<<endl;
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
	namedWindow("bushid",WINDOW_AUTOSIZE);
	imshow("bushid",delg.matrix);
	waitKey(0);
}

int main()
{
	sample_check("variance_test.jpg");
	image img;
	img.open("bushes3.jpg");
	image resized = img.re_size(480,240);
	featureRecog(img,0,0,240,120,70,72,48,8,77);
	waitKey(0);
	return 0;
}
