// The programe reads the image data from an image file "~.raw"
// Last updated on 02/20/2010 by Steve Cho

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <fstream>

using namespace std;

// Here we assume the image is of size 256*256 and is of raw format
// You will need to make corresponding changes to accommodate images of different sizes and types

#define Size 256
#define N 8
#define PACIFIER_COUNT 2047

double DCT[N][N];
double Converdata[Size][Size]; //store the datas ready for DCT transform
double Qe_10[N][N];
double Qe_90[N][N];

double Qe[N][N]={
{ 16,	11,	10,	16,  24,   40,   51,	61},
{ 12,	12,	14,	19,  26,   58,   60,	55},
{ 14,	13,	16,	24,  40,   57,   69,	56},
{ 14,	17,	22,	29,  51,   87,   80,	62},
{ 18,   22,	37,	56,  68,   109,  103,	77},
{ 24,	35,	55,	64,  81,   104,  113,	92},
{ 49,	64,	78,	87,  103,  121,  120,	101},
{ 72,	92,	95,	98,  112,  100,  103,	99}
};


int main(int argc, char *argv[])

{
	// file pointer
	FILE *file;

	int shift_data[Size][Size];

	double PSNR[2];
	double MSE[2]={0};

    int BytesPerPixel;
    BytesPerPixel=1;
	// image data array
	unsigned char origin_data[Size][Size][BytesPerPixel];
	unsigned char compare_data[Size][Size][3];
	int Ima_origin[Size][Size];
	int Ima_outdata[Size][Size];

    char input_name[30];
	char output_name[30];
	int factor=100;
while(1){

	cout<<"Type int the factor"<<endl;
	cin>>factor;
	if (factor>100) return 0;

	sprintf(input_name,"clock_qua%d.raw",factor);

	// read image "ride.raw" into image data matrix
	if (!(file=fopen("clock.raw","rb")))
	{
		cout << "Cannot open file: " << argv[1] <<endl;
		exit(1);
	}
	fread(origin_data, sizeof(unsigned char), Size*Size*BytesPerPixel, file);
	fclose(file);

	if (!(file=fopen(input_name,"rb")))
	{
		cout << "Cannot open file: " << argv[1] <<endl;
		exit(1);
	}
	fread(compare_data, sizeof(unsigned char), Size*Size*3, file);
	fclose(file);


	// do some image processing task...
	int i=0;
	int j=0;
	int m=0;
	int k=0;


   for (i=0;i<Size;i++)
	 {
        for (j=0;j<Size;j++)
        {
        if (BytesPerPixel==1) MSE[0]+=(double)(origin_data[i][j][0]-compare_data[i][j][0])*(double)(origin_data[i][j][0]-compare_data[i][j][0]);
        else MSE[0]+=(double)(origin_data[i][j][0]-compare_data[i][j][0])*(double)(origin_data[i][j][0]-compare_data[i][j][0])
                    +(double)(origin_data[i][j][1]-compare_data[i][j][1])*(double)(origin_data[i][j][1]-compare_data[i][j][1])
                    +(double)(origin_data[i][j][2]-compare_data[i][j][2])*(double)(origin_data[i][j][2]-compare_data[i][j][2]);
        }
	}

    MSE[0]=MSE[0]/(Size*Size*BytesPerPixel);
    PSNR[0]=10*log10(255*255/MSE[0]);
    MSE[0]=0;


    cout<< PSNR[0]<<endl;
    }
}








