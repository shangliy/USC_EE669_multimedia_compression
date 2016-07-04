// QM_CODE_SHANGLIY.cpp 

#include "stdafx.h"
#include "fstream"
#include "iostream"
#include "vector"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "bitio.h"
#include "errhand.h"
#include <malloc.h>
#include "qmcoder.h"

#define PACIFIER_COUNT 2047
#define INF 65536
using namespace std;
errno_t err;

BIT_FILE *OpenOutputBitFile(char *name)

{
	BIT_FILE *bit_file;

	bit_file = (BIT_FILE *)calloc(1, sizeof(BIT_FILE));
	if (bit_file == NULL)
		return(bit_file);
	if ((err = fopen_s(&bit_file->file, name, "wb")) != 0)
		cout << "The Input file was not opened\n";
	else
		cout << "The Input file was opened\n";

	bit_file->rack = 0;
	bit_file->mask = 0x80;
	bit_file->pacifier_counter = 0;
	return(bit_file);
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

double *t_table;
double *r_table;
double r[32] = { 0 };
double t[32] = { 0 };

double num_sum(int min, int max,double *pro)
{
	int i;
	double num_sum=0;
	for (i = min; i < max; i++)
	{
		num_sum += ((double)i)*pro[i];
	}
	return num_sum;
}

double pro_sum(int min, int max, double *pro)
{
	int i;
	double num_sum = 0;
	for (i = min; i < max; i++)
	{
		num_sum += pro[i];
	}
	return num_sum;
}

void Initial(int size,double *weight)
{
	int i;
	int j;
	int k;
	double dif;
	double sum = 0;
	double min_dif = INF;

	
	t_table = (double *)calloc(size, sizeof(double));
	r_table = (double *)calloc(size, sizeof(double));
	
	for (i = 0; i < size; i++ )
	{
		
		t_table[i] = 0;
		r_table[i] = 0;
	}
	
	for (j = 1; j < size; j++)
	{
		min_dif = INF;
		sum = 0;
		for (i = 0; i < 256; i++)
		{
			for (k = 0; k < weight[i]; k++)
			{
				sum ++;
				if (sum == (double)j * 65536 * 3 / size)
					t_table[j] = (double)i-1+(double)k/weight[i];
			}
		
		}
		t[j] = t_table[j];

	}

}
void Update(int size, double *pro)
{
	int j = 0;
	for (j = 0; j < size; j++)
	{
		
		if (j < size-1 )
		{
			r_table[j] = num_sum(t_table[j], t_table[j + 1], pro) / pro_sum(t_table[j], t_table[j + 1], pro);
			r[j] = r_table[j];
		
		}
		else 
		{
			r_table[j] = num_sum(t_table[j], 256, pro) / pro_sum(t_table[j], 256, pro);
			r[j] = r_table[j];

		}
			
	}

	for (j = 1; j <size; j++)
	{
		t_table[j] = (r_table[j] + r_table[j - 1]) / 2;
		t[j] = t_table[j];
	}
	
}

double Quanti_fun(int in_value,int size)
{
	int i = 0;
	for (i = 0; i < size-1;i++)
		if (in_value >= t_table[i] && in_value < t_table[i+1])
			return r_table[i];

	if (in_value >= t_table[i] && in_value < 256)
		return r_table[i];
}


int _tmain(int argc, _TCHAR* argv[])
{
	int i = 0;
	int j = 0;
	int k = 0;
	int index_size;
	
	char file_ch;
	const char* file_name[6];
	int *Buff;
	int buff_pre = INF;
	double sam_weight[6][256] = { 0 };
	double qua_value[6][256] = { 0 };
	double sum_weight[256] = { 0 };
	double new_sam_weight[6][256] = { 0 };
	double pro_each[6][256] = { 0 };
	double pro[256]= { 0 };
	

	
	
	unsigned long code = 0;
	FILE *output_file;

	unsigned char value = 0;
	unsigned char mask = 0x80;
	int context = 0;
	int rule_num = 0;

	file_name[0] = "chem.256";
	file_name[1] = "house.256";
	file_name[2] = "moon.256";
	file_name[3] = "f16.256";
	file_name[4] = "couple.256";
	file_name[5] = "elaine.256";

	for (i = 0; i < 3; i++)
	{
		ifstream infile(file_name[i], ios::binary);
		if (!infile)
		{
			cerr << "open error!" << endl;
			abort();
		}

		Buff = (int*)calloc(sizeof(int), 1);
		
		while (infile.peek() != EOF)
		{
			infile.read((char*)Buff, sizeof(char));

			sam_weight[i][*Buff]++;
			sum_weight[*Buff]++;
			pro[*Buff]++;
			
		}
		infile.close();
	}

	for (i = 3; i < 6; i++)
	{
		ifstream testfile(file_name[i], ios::binary);
		if (!testfile)
		{
			cerr << "open error!" << endl;
			abort();
		}

		Buff = (int*)calloc(sizeof(int), 1);

		while (testfile.peek() != EOF)
		{
			testfile.read((char*)Buff, sizeof(char));

			sam_weight[i][*Buff]++;
		}
		testfile.close();
	}

	for (i = 0; i < 256; i++)
	{
		pro[i]= pro[i]/(double)(3*65536);
	}
	
	double test_sum;
	test_sum = pro_sum(0,256,pro);
	
	index_size = 32;
	Initial(index_size, sum_weight);

	double e_dif = 65536;
	double PSNR = 0;
	double MSE[2] = { INF };
	double num_count = 0;
    k = 1;
	while (e_dif>0.001)
	{
		Update(index_size, pro);
		num_count = 0;
		MSE[1] = 0;
		for (i = 0; i < 256; i++)
			{
				MSE[1]+=(Quanti_fun(i, index_size)-i)*(Quanti_fun(i, index_size) - i)*sum_weight[i]/65536;
				num_count += sum_weight[i];
			}
		MSE[1] /= 3;
		e_dif = (MSE[0] - MSE[1]) / MSE[1];
		
		PSNR = 10 * log10(255*255/ MSE[1]);
		cout << k << ":" << PSNR << endl;
		cout << "\n" << endl;
		k++;
		MSE[0] = MSE[1];
	
         }
	
	
	
	cout << "the number of iteration=" << k-1 << endl;

	int q_value=0;
	double PSNR_e[6] = { 0 };
	double MSE_e[6] = { 0 };
	double entro[6] = { 0 };
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 256; j++)
		{
			q_value = round(Quanti_fun(j, index_size));
			new_sam_weight[i][q_value] += sam_weight[i][j];
			MSE_e[i] += (q_value - j)*(q_value - j)*sam_weight[i][j];
			/*Calculate the Entropy for the file*/
				
		}

		for (j = 0; j < 256; j++)
		{
			if (new_sam_weight[i][j])
				entro[i] = entro[i] + (new_sam_weight[i][j] / (double)65536)*(log(new_sam_weight[i][j] / ((double)65536)) / log(2.0));

		}
		entro[i] = -entro[i];
		MSE_e[i] /= 65536;
		PSNR_e[i] = 10 * log10(255 * 255 / MSE_e[i]);
	}


	
	
	
	
	
	system("pause");
	i = 0;
	delete Buff;
	delete r_table;
	delete t_table;
	return 0;
}

