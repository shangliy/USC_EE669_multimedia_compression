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

#define Size 16
#define N 8
#define PACIFIER_COUNT 2047

double DCT[N][N];
double Converdata[Size][Size]; //store the datas ready for DCT transform
int Qe_10[N][N];
int Qe_90[N][N];

int v[10]={0};
int v_new[10]={0};
double a[4][3]={0};
int QP=0;
double c1=2;
double c2=5;
double c3=8;

// Here the QE matrix with factor 50
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

int Build_QE(int n)
{
    int i=0;
    int j=0;
    if (n==10)
    {
        for (i=0;i<N;i++)
        {
            for (j=0;j<N;j++)
            {
                Qe_10[i][j]=Qe[i][j]*50/n;
            }
        }
    }

    if (n==90)
    {
        for (i=0;i<N;i++)
        {
            for (j=0;j<N;j++)
            {
                Qe_90[i][j]=Qe[i][j]*(100-n)/50;
            }
        }
    }
    return 1;
}

int DCT_Tran()
{
    int i=0;
    int j=0;
    int m=0;
    int n=0;
    double Ci=0;
    double Cj=0;


    for (i=0;i<N;i++)
    {
        for (j=0;j<N;j++)
        {
            DCT[i][j]=0;
        }
    }

    for(i=0;i<N;i++)
    {
        for (j=0;j<N;j++)
        {
             for (m=0;m<N;m++)
                {
                    for (n=0;n<N;n++)
                        {
                            if (i==0) Ci=1/sqrt(2);
                            else Ci=1;
                            if (j==0) Cj=1/sqrt(2);
                            else Cj=1;
                            DCT[i][j]+=(Ci *Cj * (Converdata[m][n]) *cos( (2*m+1)*i*M_PI /(2*N) ) *cos((2*n+1)*j*M_PI /(2*N)) )/sqrt(2*N);
                        }
                }
        }
    }
    return 1;

}

int MAX_FUN(int n)
{
 int temp_max=0;
 int i=0;
 for (i=0;i<n;i++)
 {
   if(v[i]>temp_max)
   temp_max=v[i];

 }
 return temp_max;
 }

 int MIN_FUN(int n)
{
 int temp_min=256;
 int i=0;
 for (i=0;i<n;i++)
 {
   if(v[i]<temp_min)
   temp_min=v[i];

 }
 return temp_min;
}


int main(int argc, char *argv[])

{
	// file pointer
	FILE *file;
	//ByteesPerPixels
	int BytesPerPixel;
	//Data after shift
	char shift_data[Size][Size];
	//Calculate the PSNNR
	double PSNR[10];
	double MSE[10]={0};
	int i=0;
	int j=0;
	int m=0;
	int k=0;

    //For gray image =1 For colore =3
    BytesPerPixel=1;

	// image data array
	unsigned char Imagedata[Size][Size][BytesPerPixel];

	// processed image data input
	unsigned char Imagedata_com[Size][Size][3];

	int Ima_origin[Size][Size];
	int Ima_outdata[Size][Size];

	char input_name[30];
	char output_name[30];
	int factor=50;

	sprintf(input_name,"lena.raw");


	// read orginal image  into image data matrix
	if (!(file=fopen("lena.raw","rb")))
	{
		cout << "Cannot open file: " << "clock.raw" <<endl;
		exit(1);
	}
	fread(Imagedata, sizeof(unsigned char), Size*Size*1, file);
	fclose(file);


    // read another image_data  into image data matrix
	if (!(file=fopen(input_name,"rb")))
	{
		cout << "Cannot open file: " << argv[1] <<endl;
		exit(1);
	}
	fread(Imagedata_com, sizeof(unsigned char), Size*Size*BytesPerPixel, file);
	fclose(file);

    /*Build different factor QUantinization*/
	Build_QE(10);
	Build_QE(90);


	ofstream outfile("Qe_10.txt", ios::out); //define the out stream
	if (!outfile)
	{
		cerr << "open error!" << endl;
		exit(1);
	}
	for (i = 0; i<N; i++)
	{
		outfile << "\n";
		for (j = 0; j<N; j++)
		{
			outfile << Qe_10[i][j] << " ";
		}
	}
	outfile.close();

    ofstream outfile1("Qe_90.txt", ios::out); //define the out stream
	if (!outfile1)
	{
		cerr << "open error!" << endl;
		exit(1);
	}
	for (i = 0; i<N; i++)
	{
		outfile1 << "\n";
		for (j = 0; j<N; j++)
		{
			outfile1 << Qe_90[i][j] << " ";
		}
	}
	outfile1.close();

	//Do the shift
    for (i=0;i<Size;i++)
	{
        for (j=0;j<Size;j++)
        {
        shift_data[i][j]=Imagedata[i][j][BytesPerPixel-1]-128;
        }
	}

	//For each block.do DCT
	int index=0;
	for(m=0;m<(Size/N);m++)
        {
          for(k=0;k<(Size/N);k++)
            {
            //iN ONE BLOCK
            for (i=0;i<N;i++)
                {
                for(j=0;j<N;j++)
                    {
                    Converdata[i][j]= shift_data[N*m+i][N*k+j];
                    }
                }
            //Do DCT
            if (!DCT_Tran()) cout<<"DCT TAKE WRONGS";
            if (factor==50)
            {
                for (i=0;i<N;i++)
                {
                    for (j=0;j<N;j++)
                    {
                        DCT[i][j]=round(DCT[i][j]/Qe[i][j]);
                    }
                }
                sprintf (output_name,"DCT_%d_%d",factor,index);
                index++;

            }
            else if (factor==10)
            {
                for (i=0;i<N;i++)
                {
                    for (j=0;j<N;j++)
                    {
                        DCT[i][j]=round(DCT[i][j]/Qe_10[i][j]);
                    }
                }
                sprintf (output_name,"DCT_%d_%d",factor,index);
                index++;

            }
            else if (factor==90)
            {
                for (i=0;i<N;i++)
                {
                    for (j=0;j<N;j++)
                    {
                        DCT[i][j]=round(DCT[i][j]/Qe_90[i][j]);
                    }
                }
                sprintf (output_name,"DCT_%d_%d",factor,index);
                index++;

            }

             for (i=0;i<N;i++)
                {
                for(j=0;j<N;j++)
                    {
                    Ima_outdata[i][j]= DCT[i][j];
                    }
                }

                // write image data to "~.raw"

                if (!(file=fopen(output_name,"wb")))
                    {
                        cout << "Cannot open file: " << Ima_outdata << endl;
                        exit(1);
                    }

                    fwrite(Ima_outdata, sizeof(unsigned char), N*N, file);
                    fclose(file);

                    ofstream outfile(output_name, ios::out); //define the out stream
                    if (!outfile)
                    {
                        cerr << "open error!" << endl;
                        exit(1);
                    }
                    for (i = 0; i<N; i++)
                    {
                        outfile << "\n";
                        for (j = 0; j<N; j++)
                        {
                            outfile << Ima_outdata[i][j] << " ";
                        }
                    }
                    outfile.close();

            }

        }


	return 0;
}









