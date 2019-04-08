#include</home/pi/opencv_install/installation/opencv/include/opencv2/opencv.hpp>
#include<cmath>
#include<string>
#include<iostream>
#include<unistd.h>
#include<thread>
#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#define PORT 65435
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
		void get_pixel(int x,int y)//get a pixel, put it in p, since c is bad at returning arrays. 
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
		void set_group(int x,int y,bool safe)
		{
			int r = 0;
			int g = 0;
			int b = 0;
			if(safe==true)
			{
				r = 0;
				b = 0;
				g = 255;
			}
			else
			{
				int r = 255;
				int g = 0;
				int b  = 0;
			}
			for(int x1 = 0;x1<8;x1++)
			{
				for(int y1 = 0;y1<8;y1++)
				{
					this->set_pixel(x+x1,y+y1,r,g,b);
				}
			}
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
int send_velocity(int* velocity)
{
	struct sockaddr_in address;
	int sock = 0,valread;
	struct sockaddr_in serv_addr;
	char buffer[1024]={0};
	if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("\n Socket Creation Error \n");
		return -1;
	}
	memset(&serv_addr,'0',sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr)<=0)
	{
		printf("\n Invalid Address/Address not supported \n");
		return -1;
	}
	if (connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		printf("\n Connection Failed\n");
	}
	send(sock,velocity,4,0);
	printf("Velocity Sent\n");
	valread = read(sock,buffer,1024);
	printf("%s\n",buffer);
	shutdown(sock,2);
	return 0;
}
int sample_check(string var_test,int* r, int* g, int* b, int* var_av,int* var_ma)//analyzes sample texture. 
{
	image img;
	img.open(var_test);//open sample image
	double avg_b=0;
	double avg_r=0;
	double avg_g=0;
	double hold_r;
	double hold_g;
	double hold_b;
	int var_max=0;
	int var_avg=0;
	int var_sum=0;
	int width = img.matrix.size().width;//keep tabs of size
	int height = img.matrix.size().height;
	for(int x=0;x<width;x++)//these loops find the center. sum colors of all pixels
	{
		for(int y=0;y<height;y++)
		{
			img.get_pixel(x,y);
			hold_r = avg_r;
			hold_b = avg_b;
			hold_g = avg_g;
			avg_r += img.p[0];
			avg_g += img.p[1];
			avg_b += img.p[2];
			if(avg_r>500000||avg_g>500000||avg_b>500000)
			{
				cout<<"Last operation was:"<<avg_r<<"="<<hold_r<<"+"<<img.p[0]<<endl;
				cout<<avg_g<<"="<<hold_g<<"+"<<img.p[1]<<endl;
				cout<<avg_b<<"="<<hold_b<<"+"<<img.p[2]<<endl;
			}

			if(img.p[0]>255||img.p[1]>255||img.p[2]>255)
			{
				cout<<"Pixel is too large."<<endl;
				cout<<img.p[0]<<" "<<img.p[1]<<" "<<img.p[2]<<endl;
			}
			//cout<<img.p[0]<<" "<<img.p[1]<<" "<<img.p[2]<<endl;	
		}
	}
	cout<<"Sums:"<<avg_r<<" "<<avg_g<<" "<<avg_b<<endl;
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
	cout<<"Average Variance: "<<var_avg<<endl;
	cout<<"Max Variance: "<<var_max<<endl;

}
image img1,img2,img3,img4,var1,var2,var3,var4;
bool zone1[60][60],zone2[60][60],zone3[60][60],zone4[60][60];
void copy_array(bool a[60][60],bool b[60][60])
{
	for(int x=0;x<60;x++)
	{
		for(int y=0;y<60;y++)
		{
			b[x][y] = a[x][y];
		}
	}
}
image featureRecog(image img, int x1, int y1, int x2, int y2,int avg_r, int avg_g,int avg_b, int var_avg, int var_max,int det)
{
	image varmap;//this was for debugging. creates a b/w image representing variance.
	Mat map(120,240,CV_8UC3,Scalar(0,0,0));
	Mat zmap(120,240,CV_8UC3,Scalar(0,0,0));
	varmap.matrix = map;
	image delg;
	delg = img.crop(x1,y1,x2,y2);//crop to specified size. 
	int gridComplianceCount = 0;
	int gridCount = 0;
	bool bush;//im aware this has a confusing name.
	int variance;
	float gridCompliance;
	bool zones[60][60];
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
				zones[x][y] = true;
			}
			if(bush==true)//if it is a bush
			{
				zones[x][y] = false;
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
	if(det==1)//im aware global variables are the devil but it was so much easier this way. 
	{
		img1 = delg;
		var1 = varmap;
		copy_array(zones,zone1);
	}
	if(det==2)
	{
		img2 = delg;
		var2 = varmap;
		copy_array(zones,zone2);
	}
	if(det==3)
	{
		img3 = delg;
		var3 = varmap;
		copy_array(zones,zone3);
	}
	if(det==4)
	{
		img4 = delg;
		var4 = varmap;
		copy_array(zones,zone4);
	}
	return delg;//return it for fun. I used to use this before it was multithreaded 
}
image check(bool zones[60][60])
{	image lzmap;
	Mat map(120,240,CV_8UC3,Scalar(0,0,0));
	lzmap.matrix = map;
	bool safe;
	for(int x=0;x<30;x++)
	{
		for(int y=0;y<15;y++)
		{	
			safe = true;
			for(int x1=0;x1<2;x1++)
			{
				for(int y1=0;y1<4;y1++)
				{
					if(zones[2*x+x1][4*y+y1]==false)
					{
						safe = false;
					}
				}
			}
			lzmap.set_group(4*x,2*y,safe);
		}
	}
	return lzmap;
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
void imtest()
{
	VideoCapture cap(0);
	Mat frame;
	cap>>frame;
	imwrite("camtest.jpg",frame);
}
/*bool* grid_fill(image img,float h, float theta_x, float theta_y)
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


}*/	
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
	image lz1 = check(zone1);
	image lz2 = check(zone2);
	image lz3 = check(zone3);
	image lz4 = check(zone4);
	image lz = stitch(lz1,lz2,lz3,lz4);
	image done = stitch(img1,img3,img2,img4);//make it whole again. 
	image varmap = stitch(var1,var3,var2,var4);
	//grid_fill(done,50,0,0);
	imwrite("pres.jpg",done.matrix);
	namedWindow("Done",WINDOW_AUTOSIZE);//make windows. opencv sucks. 
	imshow("Done",done.matrix);//show it
	namedWindow("LZ",WINDOW_AUTOSIZE);
	imshow("LZ",lz.matrix);
	waitKey(0);//wait until a key is pressed. 
	int vel_test = 20;
	int* velocity = &vel_test;
	send_velocity(velocity);
	//imtest();
	return 0;
}
