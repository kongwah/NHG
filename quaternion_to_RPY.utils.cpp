#include <iostream>
#include <math.h>
#include <assert.h>
#include <stdio.h>

#include <stdlib.h>    
#include <time.h>       


// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


/*
g++ -o qqq quaternion_to_RPY.utils.cpp \
     /usr/local/lib/libopencv_core.so.3.3.0 \
     /usr/local/lib/libopencv_imgcodecs.so.3.3.0 \
     /usr/local/lib/libopencv_imgproc.so.3.3.0
*/

void getEulerYPR(double qx, double qy, double qz, double qw, 
                 double *y, double *p, double *r)
{
    double m_el[3][3];

    double d = qx * qx + qy * qy + qz * qz + qw * qw;
		assert(d != double(0.0));
		double s = double(2.0) / d;
		double xs = qx * s,   ys = qy * s,   zs = qz * s;
		double wx = qw * xs,  wy = qw * ys,  wz = qw * zs;
		double xx = qx * xs,  xy = qx * ys,  xz = qx * zs;
		double yy = qy * ys,  yz = qy * zs,  zz = qz * zs;

    m_el[0][0] = double(1.0) - (yy + zz);   m_el[1][0] = xy - wz;                 m_el[2][0] = xz + wy;
    m_el[0][1] = xy + wz;                   m_el[1][1] = double(1.0) - (xx + zz); m_el[2][1] = yz - wx;
    m_el[0][2] = xz - wy;                   m_el[1][2] = yz + wx;                 m_el[2][2] = double(1.0) - (xx + yy);
    
		struct Euler
		{
			double yaw;
			double pitch;
			double roll;
		};

		Euler euler_out;
		Euler euler_out2; //second solution
		//get the pointer to the raw data

		// Check that pitch is not at a singularity
  		// Check that pitch is not at a singularity
		if (fabs(m_el[0][2]) >= 1)
		{
			euler_out.yaw = 0;
			euler_out2.yaw = 0;
	
			// From difference of angles formula
			if (m_el[0][2] < 0)  //gimbal locked down
			{
			  double delta = atan2(m_el[1][0],m_el[2][0]);
				euler_out.pitch = M_PI / double(2.0);
				euler_out2.pitch = M_PI / double(2.0);
				euler_out.roll = delta;
				euler_out2.roll = delta;
			}
			else // gimbal locked up
			{
			  double delta = atan2(-m_el[1][0],-m_el[2][0]);
				euler_out.pitch = -M_PI / double(2.0);
				euler_out2.pitch = -M_PI / double(2.0);
				euler_out.roll = delta;
				euler_out2.roll = delta;
			}
		}
		else
		{
      double tmp = m_el[0][2];
      if (tmp<double(-1))	tmp=double(-1); 
      if (tmp>double(1))	tmp=double(1);
			euler_out.pitch = - asin(tmp);
			euler_out2.pitch = M_PI - euler_out.pitch;

			euler_out.roll = atan2(m_el[1][2]/cos(euler_out.pitch), 
				m_el[2][2]/cos(euler_out.pitch));
			euler_out2.roll = atan2(m_el[1][2]/cos(euler_out2.pitch), 
				m_el[2][2]/cos(euler_out2.pitch));

			euler_out.yaw = atan2(m_el[0][1]/cos(euler_out.pitch), 
				m_el[0][0]/cos(euler_out.pitch));
			euler_out2.yaw = atan2(m_el[0][1]/cos(euler_out2.pitch), 
				m_el[0][0]/cos(euler_out2.pitch));
		}

		if (1)
		{ 
			*y = euler_out.yaw; 
			*p = euler_out.pitch;
			*r = euler_out.roll;
		}
		else
		{ 
			*y = euler_out2.yaw; 
			*p = euler_out2.pitch;
			*r = euler_out2.roll;
		}

}



int main(int argc, char *argv[])
{
   srand (time(NULL)); // initialize random seed

   double resolution,origin_x,origin_y;
   FILE *fp;
   fp=fopen("map.yaml","r");
   fscanf(fp,"%lf",&resolution);
   fscanf(fp,"%lf",&origin_x);
   fscanf(fp,"%lf",&origin_y);
   printf("resolution=%f origin_x=%f origin_y=%f\n",resolution,origin_x,origin_y);
   fclose(fp);

   // Read the map image file 
   Mat img = imread("map_test_origin.png",IMREAD_COLOR);
   //printf("rows=%d cols=%d\n",img.rows,img.cols);
   for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
         int x = (int)img.at<uchar>(i, j);
         Vec3b &cc = img.at<Vec3b>(i,j); // get color pixel
         /**
         double rr=rand()/(double)RAND_MAX;
         if(0.1812<rr && rr<0.1813) printf("%d %d --> %d %d %d\n",i,j,cc[0],cc[1],cc[2]);
         **/
      }
   }

   int i; double timestamp, xx,yy,zz;
   double qx, qy, qz, qw, y, p, r;
   fp=fopen("pose_graph.txt","r");
   while (!feof(fp)){
      fscanf(fp,"%d %lf %lf %lf %lf %lf %lf %lf %lf",&i,&timestamp,&xx,&yy,&zz,&qx,&qy,&qz,&qw);
      getEulerYPR(qx, qy, qz, qw, &y, &p, &r);
      //printf("%d %lf [%lf,%lf,%lf,%lf] -->  [%lf,%lf,%lf]\n",i,timestamp,qx,qy,qz,qw,y,p,r);
      if(i==atoi(argv[1])) {
        printf("%4d %lf ",i,timestamp);
        if (xx>0) printf(" %lf ",xx);
        else      printf("%lf ",xx);
        if (yy>0) printf(" %lf ",yy);
        else      printf("%lf ",yy);
        if (y>0)  printf(" %lf ",y);
        else      printf("%lf ",y);
        printf("\n");
        break;
      }
   } 
   fclose(fp);


   Mat arrow = imread("arrow.jpg",IMREAD_COLOR);
   Mat resized_arrow;
   resize(arrow, resized_arrow, Size(arrow.cols/8, arrow.rows/8), INTER_LINEAR);
   double angle;
   if(y>=0) angle = (y*180.0)/3.14159;
   else     angle = (2*3.14159+y)*180/3.14159;

   Point2f center((resized_arrow.cols - 1) / 2.0, 
                  (resized_arrow.rows - 1) / 2.0); // center coord for 2D rotation matrix
   Mat rotation_matix = getRotationMatrix2D(center, angle, 1.0); // get the rotation matrix

   Mat rotated_resized_arrow;
   warpAffine(resized_arrow, rotated_resized_arrow, 
              rotation_matix, resized_arrow.size(),  
              cv::INTER_LINEAR,
              cv::BORDER_CONSTANT,
              cv::Scalar(255, 255, 255));   // rotate using warpAffine


   Mat flipped_rotated_resized_arrow;
   flip(rotated_resized_arrow, flipped_rotated_resized_arrow, 0); 

   //printf("   warpAffine() angle=%f\n",angle);



   int oy = int(-origin_y/resolution);
   int ox = int(-origin_x/resolution);

   /**
   for (int i = oy+int(yy/resolution)-10; i < oy+int(yy/resolution)+10; i++) {
      for (int j = ox+int(xx/resolution)-10; j < ox+int(xx/resolution)+10; j++) {
         Vec3b &cc = img.at<Vec3b>(i,j); // get color pixel
         cc[0] = 255;
         cc[1] = 0;
         cc[2] = 255;
         img.at<Vec3b>(i,j) = cc;
      }
   }
   **/
   Mat insetImage1(img, cv::Rect(ox+int(xx/resolution)-10, oy+int(yy/resolution)-10, 
                                 flipped_rotated_resized_arrow.cols, flipped_rotated_resized_arrow.rows));
   flipped_rotated_resized_arrow.copyTo(insetImage1);




   Mat flipped_img;
   flip(img, flipped_img, 1); 


   Mat img2, destimg;
   char imgfilename[200];
   strcpy(imgfilename,"/home/kongwah/NTT.NHG/Hougang.20230503/pose_graphs/lvl3/");
   strcat(imgfilename,argv[1]);
   strcat(imgfilename,".jpg");
   printf("%s\n",imgfilename);
   img2 = imread(imgfilename,IMREAD_COLOR);


   Mat resized_img2;
   resize(img2, resized_img2, Size(img2.cols/1.75, img2.rows/1.75), INTER_LINEAR);
   //Mat insetImage(flipped_img, cv::Rect(800, 700, resized_img2.cols, resized_img2.rows)); // level 2
   Mat insetImage(flipped_img, cv::Rect(50, 750, resized_img2.cols, resized_img2.rows)); // level 3
   resized_img2.copyTo(insetImage);

   //imwrite("tmp.jpg", flipped_img);



   char outfilename[200];
   strcpy(outfilename,"output/");
   strcat(outfilename,argv[1]);
   strcat(outfilename,".jpg");

   // to crop the image to smaller, and make the width height an even number
   //Mat cropped_image = flipped_img(Range(90,img.rows-80), Range(10,img.cols-10)); // level 2
   Mat cropped_image = flipped_img(Range(120,img.rows-50), Range(10,img.cols-10)); // level 3

   imwrite(outfilename, cropped_image);

   return 1;
}

