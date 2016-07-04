// The programe reads the image data from an image file "~.raw"
// Last updated on 02/20/2010 by Steve Cho

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include "bitio.h"
#include "errhand.h"
#include "jpeglib.h"
#include <setjmp.h>

using namespace std;

// Here we assume the image is of size 256*256 and is of raw format
// You will need to make corresponding changes to accommodate images of different sizes and types

#define Size 256
#define N 8
#define PACIFIER_COUNT 2047

double DCT[N][N];
unsigned char Converdata[Size][Size][3];
unsigned char Converdata_2[Size][Size][3];//store the datas ready for DCT transform
double Qe_10[N][N];
double Qe_90[N][N];

int v[10]={0};
int v_new[10]={0};
double a[4][3]={0};
int QP=0;
double c1=2;
double c2=5;
double c3=8;

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

BIT_FILE *OpenOutputBitFile( char *name )

{
    BIT_FILE *bit_file;

    bit_file = (BIT_FILE *) calloc( 1, sizeof( BIT_FILE ) );
    if ( bit_file == NULL )
        return( bit_file );
    bit_file->file = fopen( name, "wb" );
    bit_file->rack = 0;
    bit_file->mask = 0x80;
    bit_file->pacifier_counter = 0;
    return( bit_file );
}



int main(int argc, char *argv[])

{
	// file pointer
	FILE *file;
	int BytesPerPixel;
	int shift_data[Size][Size];
	double PSNR[6];
	double MSE[6]={0};
    int width=Size;
    int height=Size;

    int bitCountPerPix=3;
    int in_BytesPerPixel=1;
    BytesPerPixel=3;

	// do some image processing task...
	int i=0;
	int j=0;
	int m=0;
	int k=0;
	int vec=1;
	int hem=1;
	char name[20];
	int debug=0;
	int t=0;
	int l=0;

	// image data array
	unsigned char Imagedata[Size][Size][3];
	unsigned char Imagedata_com[Size][Size][3];
	int Ima_origin[Size][Size][3];
	int Ima_outdata[Size][Size][3];


	while(1)
	{
            cout<<"Type in the which file type"<<endl;
            cin>>t;
            cout<<"Type in the file index"<<endl;
            cin>>l;
            debug=0;

            char input_name[30];
            char output_name[30];


            // read image "ride.raw" into image data matrix
            if(t==1)
            {
                 sprintf(input_name,"RAWDATA/clock_pro_%d.raw",l);
            }
            else if(t==2)
            {
                sprintf(input_name,"RAWDATA//pepper_pro_%d.raw",l);

            }
            else return 0;

            if (!(file=fopen(input_name,"rb")))
                {
                    cout << "Cannot open file: " << "clock.raw" <<endl;
                    exit(1);
                }
                fread(Imagedata_com, sizeof(unsigned char), Size*Size*BytesPerPixel, file);
                fclose(file);

            k=0;
            for (vec=-3;vec<=4;vec++)
            {
             for (hem=-3;hem<=4;hem++)
                {
                    for (i=0;i<Size;i++)
                        {
                            for(j=0;j<Size;j++)
                                {
                                    if (j-hem>=Size)
                                    {
                                    Converdata[i][j][0]=Imagedata_com[i][j][0];
                                    Converdata[i][j][1]=Imagedata_com[i][j][1];
                                    Converdata[i][j][2]=Imagedata_com[i][j][2];
                                    }

                                    else if (j-hem<0)
                                    {
                                    Converdata[i][j][0]=Imagedata_com[i][j][0];
                                    Converdata[i][j][1]=Imagedata_com[i][j][2];
                                    Converdata[i][j][2]=Imagedata_com[i][j][2];

                                    }

                                    else
                                    {
                                    Converdata[i][j][0]=Imagedata_com[i][j-hem][0];
                                    Converdata[i][j][1]=Imagedata_com[i][j-hem][1];
                                    Converdata[i][j][2]=Imagedata_com[i][j-hem][2];

                                    }
                                }
                        }

                    for (i=0;i<Size;i++)
                        {
                            for(j=0;j<Size;j++)
                                {
                                    if ((i+vec)>=Size)
                                    {
                                    Converdata_2[i][j][0]=Converdata[i][j][0];
                                    Converdata_2[i][j][1]=Converdata[i][j][1];
                                    Converdata_2[i][j][2]=Converdata[i][j][2];
                                    }

                                    else if ((i+vec)<0)
                                    {
                                    Converdata_2[i][j][0]=Converdata[i][j][0];
                                    Converdata_2[i][j][1]=Converdata[i][j][1];
                                    Converdata_2[i][j][2]=Converdata[i][j][2];
                                    }

                                    else
                                    {
                                    Converdata_2[i][j][0]=Converdata[i+vec][j][0];
                                    Converdata_2[i][j][1]=Converdata[i+vec][j][1];
                                    Converdata_2[i][j][2]=Converdata[i+vec][j][2];
                                    }
                                }
                        }
                        if (t==1) sprintf(name,"RAW_64/clock%d_%d.raw",l,k);
                        else sprintf(name,"RAW_64/pepper%d_%d.raw",l,k);
                        k++;

                        if (!(file=fopen(name,"wb")))
                                {
                                    cout << "Cannot open file: " << name << endl;
                                    exit(1);
                                }
                        fwrite(Converdata_2, sizeof(unsigned char), Size*Size*3, file);
                        fclose(file);
                }
            }



            unsigned char raw_data_in[Size][Size][3];
            unsigned char tem[Size][Size][3];
            int sum_data_in[Size][Size][3]={0};
            int pro_prodata[Size][Size][3];
            char raw_in[30];

            debug=0;
              if (debug==0)
        {
                for (m=1;m<=5;m++)
                    {
                        k=0;
                        for (vec=3;vec>=-4;vec--)
                        {
                        for (hem=3;hem>=-4;hem--)
                            {
                            if (t==1) sprintf(raw_in, "NEWRAW/dclock%d_%d.raw", m,k);
                            else sprintf(raw_in, "NEWRAW/dpepper%d_%d.raw", m,k);
                            k++;
                            if (!(file=fopen(raw_in,"rb")))
                                printf("The Input file was not opened\n");
                            else
                                printf("The Input file was opened\n");
                            fread(raw_data_in, sizeof(unsigned char), Size*Size * BytesPerPixel, file);
                            fclose (file);

                            for (i=0;i<Size;i++)
                            {
                                for(j=0;j<Size;j++)
                                {
                                    tem[i][j][0]=raw_data_in[Size-1-i][j][0];
                                    tem[i][j][1]=raw_data_in[Size-1-i][j][1];
                                    tem[i][j][2]=raw_data_in[Size-1-i][j][2];

                                }

                            }

                             for (i=0;i<Size;i++)
                            {
                                for(j=0;j<Size;j++)
                                {
                                    raw_data_in[i][j][0]=tem[i][j][0];
                                    raw_data_in[i][j][1]=tem[i][j][1];
                                    raw_data_in[i][j][2]=tem[i][j][2];

                                }

                            }


                            for (i=0;i<Size;i++)
                                {
                                        for(j=0;j<Size;j++)
                                        {
                                            if (j-hem>=Size)
                                            {
                                            Converdata[i][j][0]=raw_data_in[i][j][0];
                                            Converdata[i][j][1]=raw_data_in[i][j][1];
                                            Converdata[i][j][2]=raw_data_in[i][j][2];
                                            }

                                            else if (j-hem<0)
                                            {
                                            Converdata[i][j][0]=raw_data_in[i][j][0];
                                            Converdata[i][j][1]=raw_data_in[i][j][2];
                                            Converdata[i][j][2]=raw_data_in[i][j][2];

                                            }

                                            else
                                            {
                                            Converdata[i][j][0]=raw_data_in[i][j-hem][0];
                                            Converdata[i][j][1]=raw_data_in[i][j-hem][1];
                                            Converdata[i][j][2]=raw_data_in[i][j-hem][2];

                                            }
                                        }
                                }

                            for (i=0;i<Size;i++)
                                {
                                    for(j=0;j<Size;j++)
                                        {
                                            if ((i+vec)>=Size)
                                            {
                                            Converdata_2[i][j][0]=Converdata[i][j][0];
                                            Converdata_2[i][j][1]=Converdata[i][j][1];
                                            Converdata_2[i][j][2]=Converdata[i][j][2];
                                            }

                                            else if ((i+vec)<0)
                                            {
                                            Converdata_2[i][j][0]=Converdata[i][j][0];
                                            Converdata_2[i][j][1]=Converdata[i][j][1];
                                            Converdata_2[i][j][2]=Converdata[i][j][2];
                                            }

                                            else
                                            {
                                            Converdata_2[i][j][0]=Converdata[i+vec][j][0];
                                            Converdata_2[i][j][1]=Converdata[i+vec][j][1];
                                            Converdata_2[i][j][2]=Converdata[i+vec][j][2];
                                            }
                                            sum_data_in[i][j][0]+=(int)Converdata_2[i][j][0];
                                            sum_data_in[i][j][1]+=(int)Converdata_2[i][j][1];
                                            sum_data_in[i][j][2]+=(int)Converdata_2[i][j][2];

                                        }
                                }

                            }

                        }

                        k=0;
                        for (i=0;i<Size;i++)
                        {
                        for (j=0;j<Size;j++)
                            {
                                (sum_data_in[i][j][0]/=64);
                                (sum_data_in[i][j][1]/=64);
                                (sum_data_in[i][j][2]/=64);
                                Converdata_2[i][j][0]=sum_data_in[i][j][0];
                                Converdata_2[i][j][1]=sum_data_in[i][j][1];
                                Converdata_2[i][j][2]=sum_data_in[i][j][2];
                                sum_data_in[i][j][0]=0;
                                sum_data_in[i][j][1]=0;
                                sum_data_in[i][j][2]=0;

                                }
                        }

                        if (t==1)sprintf(name,"NEWPRORAW/clock%d.raw",m);
                        else sprintf(name,"NEWPRORAW/pepper%d.raw",m);

                        if (!(file=fopen(name,"wb")))
                                {
                                    cout << "Cannot open file: " << name << endl;
                                    exit(1);
                                }
                        fwrite(Converdata_2, sizeof(unsigned char), Size*Size*3, file);
                        fclose(file);

               }

         }
	}


	return 0;
}







