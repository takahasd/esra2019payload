#include</home/danny/build/install/include/opencv4/opencv2/opencv.hpp>
#include<cmath>
#include<string>
#include<iostream>
#include<unistd.h>
#include<thread>
using namespace std;
using namespace cv;
class image		//image object.
{
	public:
		int p[3];//holds the current pixel [r,g,b]
		Mat matrix;	//actual image matrix
		void open(string image)	//open an image
		{
			matrix = imread(image, IMREAD_COLOR);
			if(!matrix.data)
			{
				cout<<"File does not exist."<<endl;
			}
		}
		void get_pixel(int x,int y)//get a pixel, put it in p, since c sucks at returning arrays. 
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
		void set_pixel(int x, int y, int r, int g, int b)//set a pixel
		{
			cv::Vec3b pixel;
			pixel[2] = uchar(r);
			pixel[1] = uchar(g);
			pixel[0] = uchar(b);
			matrix.at<cv::Vec3b>(y,x) = pixel;
		}
		image crop(int x1, int y1, int x2, int y2)//crop a certain part of the image. returns a new, cropped image. original image is intact. 
		{
			image new_img;
			Mat blank((y2-y1),(x2-x1),CV_8UC3,Scalar(0,0,0));	//new blank
			new_img.matrix = blank;
			for(int i = x1;i<x2;i++)	//fill
			{
				for(int j=y1;j<y2;j++)
				{
					new_img.matrix.at<cv::Vec3b>(j-y1,i-x1) = matrix.at<Vec3b>(j,i);
				}
			}
			return new_img;
		}
		image re_size(int width,int height)//resize an image. returns new image. original image is intact.
		{
			image resized;
			cv::resize(matrix,resized.matrix,cv::Size(width,height));//opencv is really good at making simple things kind of ugly.
			return resized;
		}	
	
};		
int sample_check(string var_test,int* r, int* g, int* b, int* var_av,int* var_ma)//analyzes sample texture. 
{
	image img;
	img.open(var_test);//open sample image
	int avg_b;
	int avg_r;
	int avg_g;
	int var_max=0;
	int var_avg;
	int var_sum;
	int width = img.matrix.size().width;//keep tabs of size
	int height = img.matrix.size().height;
	for(int x=0;x<width;x++)//these loops find the center. sum colors of all pixels
	{
		for(int y=0;y<height;y++)
		{
			img.get_pixel(x,y);
			avg_r += img.p[0];
			avg_g += img.p[1];
			avg_b += img.p[2];
			if(img.p[0]>255||img.p[1]>255||img.p[2]>255)
			{
				cout<<"Something is fucky."<<endl;
				cout<<img.p[0]<<" "<<img.p[1]<<" "<<img.p[2]<<endl;
			}
		}
	}
	cout<<"Averages:"<<avg_r<<" "<<avg_g<<" "<<avg_b<<endl;
	avg_r = avg_r/(width*height);
	avg_g = avg_g/(width*height);//average sums to find center
	avg_b = avg_b/(width*height);
	cout<<"Center:"<<avg_r<<endl<<avg_g<<endl<<avg_b<<endl;
	for(int x=0;x<width;x++)//these loops handle variance.
	{
		for(int y=0;y<height;y++)
		{
			img.get_pixel(x,y);
			int dist_r = avg_r - img.p[0];//distance on each axis
			int dist_g = avg_g - img.p[1];
			int dist_b = avg_b - img.p[2];
			//cout<<"Operations:"<<dist_r<<"="<<avg_r<<"-"<<img.p[0]<<endl;
			//cout<<dist_g<<"="<<avg_g<<"-"<<img.p[1]<<endl;
			//cout<<dist_b<<"="<<avg_b<<"-"<<img.p[2]<<endl;
			int variance = sqrt(dist_r*dist_r + dist_g*dist_g + dist_b*dist_b);//pythagorean distance to center
			if(variance > var_max)//keep tabs of the maximum variance seen
			{
				var_max = variance;
			}
			var_sum += variance;//sum for an average later
		}
	}
	var_avg = var_sum/(height*width);//heres that average
	*r = avg_r;
	*g = avg_g;//put it where it goes
	*b = avg_b;
	*var_av = var_avg;
	*var_ma = var_max;

}
image img1,img2,img3,img4,var1,var2,var3,var4;
image featureRecog(image img, int x1, int y1, int x2, int y2,int avg_r, int avg_g,int avg_b, int var_avg, int var_max,int det)
{
	image varmap;//this was for debugging. creates a b/w image representing variance.
	Mat map(120,240,CV_8UC3,Scalar(0,0,0));
	varmap.matrix = map;
	image delg;
	delg = img.crop(x1,y1,x2,y2);//crop to specified size. 
	int gridComplianceCount = 0;
	int gridCount = 0;
	bool bush;//im aware this has a confusing name.
	int variance;
	float gridCompliance;
	for(int x=0;x<delg.matrix.size().width/4;x++)//divide into four pixel by two pixel grid. 
	{	
		for(int y=0;y<delg.matrix.size().height/2;y++)
		{
			gridCount+=1;//count this grid
			bush = true;//default to yes, this is a bush
			gridComplianceCount = 0;
			for(int i=0;i<4;i++)//iterate through each pixel in grid. 
			{
				for(int j=0;j<2;j++)
				{
					delg.get_pixel(4*x+i,2*y+j);//get the pixel
					variance = sqrt((avg_r-delg.p[0])*(avg_r-delg.p[0]) + (avg_g - delg.p[1])*(avg_g-delg.p[1]) + (avg_b-delg.p[2])*(avg_b-delg.p[2]));//again, pythagorean distance to center.
					varmap.set_pixel(4*x+i,2*y+j,variance,variance,variance);//put a pixel in the variance map
					if(variance<var_avg)//if the pixel complies
					{
						gridComplianceCount += 1;//count it
					}
					if(variance>var_max)//if its waaaay too far off, its not a bush.
					{
						bush = false;
					}
				}
			}
			if(gridComplianceCount<1)//this used to be more sophisticated. if none fell within the average distance
			{
				bush = false;//probably not a bush
			}
			if(bush==true)//if it is a bush
			{
				for(int i=0;i<4;i++)
				{
					for(int j=0;j<2;j++)
					{
						delg.set_pixel(4*x+i,2*y+j,255,0,0);//make it a red bush
					}
				}
			}		
		}
	}
	if(det==1)//im aware gloabl variables are the devil but it was so much easier this way. 
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
	return delg;//return it for fun. I used to use this before it was multithreaded 
}
image stitch(image img1, image img2, image img3, image img4)//puts images back together after theyre taken apart by the threads
{
	Mat blank(240,480,CV_8UC3,Scalar(0,0,0));
	for(int x=0;x<240;x++)
	{
		for(int y=0;y<120;y++)
		{
			blank.at<Vec3b>(y,x) = img1.matrix.at<Vec3b>(y,x);//only works for the right size image. whatever, its a specific application
			blank.at<Vec3b>(y,x+240) = img2.matrix.at<Vec3b>(y,x);
			blank.at<Vec3b>(y+120,x) = img3.matrix.at<Vec3b>(y,x);
			blank.at<Vec3b>(y+120,x+240) = img4.matrix.at<Vec3b>(y,x);
		}
	}
	image img;
	img.matrix = blank;
	return img;//return complete image
}
bool* grid_fill(image img,float h, float theta_x, float theta_y)
{
	int lim = 8;
	int redcount;
	bool map[60][60];
	for(int x=0;x<img.matrix.size().width/8;x++)
	{
		for(int y=0;y<img.matrix.size().height/4;y++)
		{
			redcount = 0;	
			for(int i=0;i<8;i++)
			{
				for(int j=0;j<4;j++)
				{
					img.get_pixel(8*x+i,4*y+j);
					if(img.p[0] == 255 && img.p[1] == 0 && img.p[2] == 0)
					{
						redcount++;
					}
				}
			}
			if(redcount > lim)
			{
				map[x][y] = false;
			}
			else
			{
				map[x][y] = true;
			}
		}
	}
	return &map[0][0];


}	
int main()
{
	int red,green,blue,v_avg,v_max;
	int *r = &red;
	int *g = &green;//threads really dont like being passed things by reference. Ideally id just hand it &red, etc, but it loves throwing SIGABRT when i do that. 
	int *b = &blue;
	int *var_max = &v_max;
	int *var_avg = &v_avg;
	string name1,name2;//filenames
	cout<<"Sample?"<<endl;//sample file
	cin>>name1;
	cout<<"Image?"<<endl;//actual image file
	cin>>name2;
	sample_check(name1,r,g,b,var_avg,var_max);//get sample data
	cout<<"Avg:"<<*var_avg<<endl;
	cout<<"max:"<<*var_max<<endl;
	image img;
	img.open(name2);//open main image
	image resized = img.re_size(480,240);//resize main image
	thread first (featureRecog,resized,0,0,240,120,*r,*g,*b,*var_avg,*var_max,1);//start first thread. upper left quarter of image. 
	thread second (featureRecog,resized,0,120,240,240,*r,*g,*b,*var_avg,*var_max,2);//start second thread. lower left quarter of image.
	thread third (featureRecog,resized,240,0,480,120,*r,*g,*b,*var_avg,*var_max,3);//start third thread. upper right quarter of image.
	thread fourth (featureRecog,resized,240,120,480,240,*r,*g,*b,*var_avg,*var_max,4);//start fourth thread. lower left quarter of image. 
	first.join();//join all the threads. 
	second.join();
	third.join();
	fourth.join();
	image done = stitch(img1,img3,img2,img4);//make it whole again. 
	image varmap = stitch(var1,var3,var2,var4);
	grid_fill(done,50,0,0);
	imwrite("pres.jpg",done.matrix);
	namedWindow("Done",WINDOW_AUTOSIZE);//make windows. opencv sucks. 
	imshow("Done",done.matrix);//show it
	namedWindow("varmap",WINDOW_AUTOSIZE);
	imshow("varmap",varmap.matrix);
	waitKey(0);//wait until a key is pressed. 
	return 0;
}
