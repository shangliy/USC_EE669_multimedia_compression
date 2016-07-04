// QM_CODE_SHANGLIY.cpp


#include "fstream"
#include "iostream"
#include "vector"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "bitio.h"
#include "errhand.h"
#include <math.h>
#include <malloc.h>
#define PACIFIER_COUNT 2047
#define INF 65536
using namespace std;


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

void OutputBit(BIT_FILE *bit_file, int bit)

{
	if (bit)
		bit_file->rack |= bit_file->mask;
	bit_file->mask >>= 1;
	if (bit_file->mask == 0) {
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack);
		// fatal_error( );
		else
			if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc('.', stdout);
		bit_file->rack = 0;
		bit_file->mask = 0x80;
	}
}

void OutputBits(BIT_FILE *bit_file, unsigned long code, int count)
{
	unsigned long mask;

	mask = 1L << (count - 1);
	while (mask != 0) {
		if (mask & code)
			bit_file->rack |= bit_file->mask;
		bit_file->mask >>= 1;
		if (bit_file->mask == 0) {
			if (putc(bit_file->rack, bit_file->file) != bit_file->rack);
			//fatal_error();
			else if ((bit_file->pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc('.', stdout);
			bit_file->rack = 0;
			bit_file->mask = 0x80;
		}
		mask >>= 1;
	}
}

void CloseOutputBitFile(BIT_FILE *bit_file)

{
	if (bit_file->mask != 0x80)
		if (putc(bit_file->rack, bit_file->file) != bit_file->rack);
	//fatal_error();
	fclose(bit_file->file);
	free((char *)bit_file);
}


int main()
{
	int i = 0;
	int j = 0;
	int k = 0;
	int f = 0;
	int m = 0;

	int codebook_size;
	int dimension;

    int *Buff;
    Buff = (int*)calloc(sizeof(int), 1);
    int image_data[256][256]={0};
    double outcount=0;

    const char* file_name[6];
	file_name[0] = "chem.256";
	file_name[1] = "house.256";
	file_name[2] = "moon.256";
	file_name[3] = "f16.256";
	file_name[4] = "couple.256";
	file_name[5] = "elaine.256";

	cout<<"Please type in the dimension of the vector (1 or 2 or 3): ";
	cin >> dimension;

	BIT_FILE *output_file;

	int vector_SIZE=pow(2,dimension)*pow(2,dimension);
	char *tem_vector;
    tem_vector=(char*)calloc(sizeof(char), vector_SIZE);
    output_file=OpenOutputBitFile("trainset_64");

    for (f=0;f<3;f++)
        {

                ifstream infile(file_name[f], ios::binary);
                if (!infile)
                    {
                        cerr << "unable to open input files!" << endl;
                        abort();
                    }

                        for (i = 0; i < 256; i++)
                            {
                                for (j = 0; j < 256; j++)
                                {
                                    if (infile.peek() != EOF)
                                    {
                                        infile.read((char*)Buff, sizeof(char));
                                        image_data[i][j]=*Buff;
                                    }
                                }
                            }

                       i=0;
                       j=0;

            for(m=0;m<256/pow(2,dimension);m++)
                {
                       for(k=0;k<256/pow(2,dimension);k++)
                       {
                        for (i=0;i<pow(2,dimension);i++)
                            {
                                for(j=0;j<pow(2,dimension);j++)
                                {
                                    OutputBits(output_file,image_data[(int)pow(2,dimension)*m+i][(int)pow(2,dimension)*k+j],8);
                                    outcount++;
                                }
                            }
                       }
                }

                infile.close();
        }

    CloseOutputBitFile(output_file);

    for (f=3;f<6;f++)
        {

                if (f==3) output_file=OpenOutputBitFile("testset_f16_4");
                if (f==4) output_file=OpenOutputBitFile("testset_couple_4");
                if (f==5) output_file=OpenOutputBitFile("testset_elaine_4");
                ifstream infile(file_name[f], ios::binary);
                if (!infile)
                    {
                        cerr << "unable to open input files!" << endl;
                        abort();
                    }

                        for (i = 0; i < 256; i++)
                            {
                                for (j = 0; j < 256; j++)
                                {
                                    if (infile.peek() != EOF)
                                    {
                                        infile.read((char*)Buff, sizeof(char));
                                        image_data[i][j]=*Buff;
                                    }
                                }
                            }

            for(m=0;m<256/pow(2,dimension);m++)
                {
                       for(k=0;k<256/pow(2,dimension);k++)
                       {
                        for (i=0;i<pow(2,dimension);i++)
                            {
                                for(j=0;j<pow(2,dimension);j++)
                                {
                                    OutputBits(output_file,image_data[(int)pow(2,dimension)*m+i][(int)pow(2,dimension)*k+j],8);
                                    outcount++;
                                }
                            }
                       }
                }

                infile.close();
                CloseOutputBitFile(output_file);
        }


	delete Buff;
	return 0;
}
