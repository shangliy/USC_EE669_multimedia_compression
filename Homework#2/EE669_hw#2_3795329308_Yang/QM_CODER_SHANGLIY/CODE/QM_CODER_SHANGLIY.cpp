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
#define PACIFIER_COUNT 2047
#define INF 2047
using namespace std;
errno_t err;

double test_count = 0;


BIT_FILE *OpenOutputBitFile(char *name)

{
	BIT_FILE *bit_file;

	bit_file = (BIT_FILE *)calloc(1, sizeof(BIT_FILE));
	if (bit_file == NULL)
		return(bit_file);
	if ((err = fopen_s(&bit_file->file, name, "wb")) != 0)
		printf("The Input file was not opened\n");
	else
		printf("The Input file was opened\n");

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



/*Huffman Function*/
/*----------------------------------------------------------------------------*/
//the structure of Huffman root node
typedef vector<unsigned long> Sample_Code;  //the vector of code 
											//Structure of the code 
class CODSYM
{
public:
	Sample_Code code;
	unsigned char name;
	CODSYM() { name = NULL; };
}CODE[256];

class HT_NODE
{
public:
	HT_NODE* left;
	HT_NODE* right;
	HT_NODE* parent;

	int name;
	double weight;
	int order;

	HT_NODE() { left = right = parent = NULL; name = 256; weight = 0; order = 0; };
	HT_NODE(HT_NODE* l, HT_NODE* r, HT_NODE* p, unsigned char n, double w, int o)
	{
		left = l;	right = r;	parent = p; name = n; weight = w; order = o;
	}
	~HT_NODE() { delete left; delete right; delete parent; }

};
//the vector of Huffman root 
typedef vector<HT_NODE*> TreeVector;
TreeVector node_arr;
/*Re-sort the HUFFMAN array according to the weight*/
void sort_ARR(TreeVector &pro_data, double L)
{
	int i, j;

	HT_NODE* le;
	HT_NODE* ri;
	HT_NODE* pa;
	unsigned char na;
	double we;
	int ord;

	HT_NODE *temple;
	for (i = 0; i<L - 1; i++)
	{
		for (j = i; j<L; j++)
		{
			if ((pro_data[j]->weight)<(pro_data[i]->weight))
			{
				temple = pro_data[i];
				pro_data[i] = pro_data[j];
				pro_data[j] = temple;

			}
		}
	}
	return;
}
/*Generate the Huffman tree*/
void Build_tree(double len)
{

	int i;
	int j = 0;
	int a = 0;
	int b = 0;
	int N = 0;
	HT_NODE* node_par = new HT_NODE;

	for (i = 0; i<len; i++)
	{
		if (j == 2)  //Find two least weight node
			break;
		if (node_arr[i]->parent == NULL) //Not the root node
		{
			N++;
			switch (j)
			{
			case 0: a = i, j++; break;
			case 1: b = i, j++; break;
			}
		}

	}

	if (N != 0 && N != 1) //combine two least weight node
	{

		node_arr[a]->parent = node_par;
		node_arr[b]->parent = node_par;
		node_par->left = node_arr[a];
		node_par->right = node_arr[b];
		node_par->weight = node_arr[a]->weight + node_arr[b]->weight;
		node_arr.push_back(node_par);
		len++;
		sort_ARR(node_arr, len);  //sort new root array
		Build_tree(len);
		return;
	}
	else return;

}
/*Generate the Huffman CODE according to the root tree*/
void generate_code(HT_NODE &root, Sample_Code&scode)
{
	int i;

	if (((root.left) == NULL) && ((root.right) == NULL)) //Achieve the bottom node
	{

		(CODE[root.name]).code = scode;
		(CODE[root.name]).name = root.name;
		return;

	}

	Sample_Code lcode = scode;
	Sample_Code rcode = scode;
	lcode.push_back(false);
	rcode.push_back(true);

	generate_code(*root.left, lcode); //Left down generate code
	generate_code(*root.right, rcode);//Right down generate code
}

/*QE_TABLE along with states*/
int Qe[46] = { 0x59EB, 0x5522, 0x504F, 0x4B85, 0x4639, 0x415E, 0x3C3D, 0x375E, 0x32B4, 0x2E17,
					 0x299A, 0x2516, 0x1EDF, 0x1AA9, 0x174E, 0x1424, 0x119C, 0x0F6B, 0x0D51, 0x0BB6,
					 0x0A40, 0x0861, 0x0706, 0x05CD, 0x04DE, 0x040F, 0x0363, 0x02D4, 0x025C, 0x01F8,
					 0x01A4, 0x0160, 0x0125, 0x00F6, 0x00CB, 0x00AB, 0x008F, 0x0068, 0x004E, 0x003B,
					 0x002C, 0x001A, 0x000D, 0x0006, 0x0003, 0x0001
};
/*State changes rececving MPS symbol */
char mps_stchage[46] = {1,1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,1,1,1,1,1,
						1,1,1,1,1,0
};
/*State changes rececving LPS symbol */
char lps_stchage[46] = { 'S',1,1,1,1,1,1,1,2,1,
						   2,1,1,2,1,2,1,2,2,1,
					       2,2,2,2,1,2,2,2,2,2,
					       2,2,2,2,2,1,2,2,2,2,
						   2,3,2,2,2,1
};

typedef vector<unsigned char> charVector; 
charVector Code_arr; // The binary code after mapping
charVector QM_arr; // The Final code after compressing through QM code

typedef vector<int> BUFFVector;
BUFFVector buffvector;  // Input BUFF
BUFFVector SECbuffvector;
BUFFVector Disbuffvector;
BUFFVector planebuff[8];
BUFFVector decovector;
BIT_FILE *output_file;

unsigned long         C_register; // C register with 32 bits 
unsigned long         A_register; // A register with 32 bits
unsigned long         SC;         // The number of stack
unsigned char     	  Outbuff;    // The output buffer with 8 bits
char				  CT;         // Counting the number of symbol in the the buffer
unsigned char         MPS;        // The MPS symbol
int BPST;                         // The Start points 
int s;                            // The state 
              
void Initenc()
{
	A_register = 0x10000;
	C_register = 0;
	s = 0;
	CT = 11;
	MPS = 0;
	Outbuff = 0;
	BPST = 0;
}

void Stuff_0()
{
	if (Outbuff == 0xff)
	{
		QM_arr.push_back(Outbuff);
		Outbuff = 0;
	}
}

void Output_stacked_zeros()
{
	while (SC > 0)
	{
		QM_arr.push_back(Outbuff);
		Outbuff = 0;
		SC--;
	}
}

void Output_stacked_0xffs()
{
	while (SC > 0)
	{
		QM_arr.push_back(Outbuff);
		Outbuff = 0xff;
		QM_arr.push_back(Outbuff);
		Outbuff = 0;
		SC--;
	}
}

void Byte_out()
{
	unsigned t = C_register >> 19;

	if (t > 0xff)
	{
		Outbuff++;
		Stuff_0();
		Output_stacked_zeros();
		QM_arr.push_back(Outbuff);
		Outbuff = t;
	}
	else
	{
		if (t == 0xff)
		{
			SC++;
		}
		else
		{
			Output_stacked_0xffs();
			QM_arr.push_back(Outbuff);
			Outbuff = t;
		}
	}
	C_register &= 0x7ffff;
}

void Renorm()
{
	while (A_register < 0x8000)
	{
		A_register <<= 1;
		C_register <<= 1;
		CT--;

		if (CT == 0)
		{
			Byte_out();
			CT = 8;
		}
	}
}

void Code_LPS()
{
	A_register -= Qe[s];

	if (!(A_register < Qe[s]))
	{
		C_register += A_register;
		A_register = Qe[s];
	}

	s = s - lps_stchage[s];
	
	if (s == -'S')
	{
		MPS = 1 - MPS;
		s = 0;
	}

	Renorm();
}

void Code_MPS()
{
	A_register -= Qe[s];

	if (A_register < 0x8000)
	{
		if (A_register < Qe[s])
		{
			C_register += A_register;
			A_register = Qe[s];
		}
		s = s + mps_stchage[s];
		Renorm();
	}
}

void Clear_final_bits()
{
	unsigned long t;
	t = C_register + A_register - 1;
	t &= 0xffff0000;

	if (t < C_register) t += 0x8000;

	C_register = t;
}

void Discard_final_zeros()
{
	int i = 0;
	int flag = 0;
	for (i = QM_arr.size(); i >= 1; i--)
	{
		if (QM_arr[i-1] == 0)
			QM_arr.pop_back();
		else break;	
	}
}

void Flush()
{
	Clear_final_bits();
	C_register <<= CT;
	Byte_out();
	C_register <<= 8;
	Byte_out();
	Discard_final_zeros();
	QM_arr.push_back(0xff);
	QM_arr.push_back(0xff); 
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i = 0;
	int k = 0;
	
	char file_ch;
	string file_name;
	HT_NODE node_root[512];
	int *Buff;
	int buff_pre = INF;
	double sam_weight[256] = { 0 };
	double sum_weight = 0;
	unsigned long code = 0;
	double ratio = 0;
	double out_count = 0;
	int TEXT_NUMBER[10] = { 1,1,0,0,0,1,0,0,1,0 };

	Sample_Code scode;

	
	
	int fun_tupe;

	
		printf("Choose which file to compress\n\n");
		printf("1:binary.dat 2:text.dat 3:audio.dat 4:image.dat\n\n");
		printf("Type in the index of file:");
		cin >> file_ch;

		if (file_ch == '5')  return 0;
		switch (file_ch)
		{
		case '1':file_name = "binary.dat";
			break;
		case '2':file_name = "text.dat";
			break;
		case '3':file_name = "audio.dat";
			break;
		case '4':file_name = "image.dat";
			break;
		default: cerr << "No choose of the file" << endl; abort();
		}
		
		
		printf("Choose which mapping function \n\n");
		printf("1:Basic binary map 2:Plane binary mapping 3:Huffman code\n\n");
		printf("Type in the index of function:");
		cin >> fun_tupe;

		if (fun_tupe == 1)//Mapping function 1 : Byte to bit 
		{
			Initenc();  //Make initialization for the QM code		
			int value = 0;
			unsigned char mask = 0x80;;

			Buff = (int*)calloc(sizeof(int), 1);
			ifstream infile(file_name, ios::binary);
			if (!infile)
			{
				cerr << "open error!" << endl;
				abort();
			}

			
			while (infile.peek() != EOF)
			{
				infile.read((char*)Buff, sizeof(char));
				mask = 0x80;
				for (i = 0; i < 8; i++)
				{
					value = (*Buff & mask ? 1 : 0);
					mask >>= 1;
					if (value == MPS)
					{
						Code_MPS();  //Recieve the MPS symbol
					}
					else Code_LPS(); //Recieve the LPS symbol
				}
				sam_weight[*Buff]++;
				sum_weight++;
			}
			Flush();

		}

		if (fun_tupe == 2)//Mapping function 2 : Plane Bite Slicing
		{
			Initenc();  //Make initialization for the QM code
			int value = 0;
			int N = 0;
			unsigned char mask = 0x80;;
			ifstream infile(file_name, ios::binary);
			if (!infile)
			{
				cerr << "open error!" << endl;
				abort();
			}
			if (fun_tupe == 4) {
				Buff = (int*)calloc(sizeof(int), 1);
				while (infile.peek() != EOF)
				{
					infile.read((char*)Buff, sizeof(char));
					buffvector.push_back(*Buff);
				}
				/*Zigzag process*/
				for (N = 0; N <= 255; N++)
				{
					for (i = 0; i <= N; i++)
					{
						SECbuffvector.push_back(buffvector[i * 256 + (N - i)]);
					}
					N++;
					for (i = N; i >= 0; i--)
					{
						SECbuffvector.push_back(buffvector[i * 256 + (N - i)]);
					}
				}

				for (N = 256; N <= 510; N++)
				{
					for (i = N - 255; i < 256; i++)
					{
						SECbuffvector.push_back(buffvector[i * 256 + (N - i)]);
					}
					N++;
					for (i = 255; i >= N - 255; i--)
					{
						SECbuffvector.push_back(buffvector[i * 256 + (N - i)]);
					}
				}

			}
			else {
				Buff = (int*)calloc(sizeof(int), 1);
				while (infile.peek() != EOF)
				{
					infile.read((char*)Buff, sizeof(char));
					SECbuffvector.push_back(*Buff);
				}

			}
			
			
			int j = 0;
			
			for (j = 0; j < SECbuffvector.size();j++)
			{
				*Buff= SECbuffvector[j];
				mask = 0x80;
				for (i = 0; i < 8; i++)
				{
					value = (*Buff & mask ? 1 : 0);
					mask >>= 1;
					planebuff[i].push_back(value);
				}

				sam_weight[*Buff]++;
				sum_weight++;
			}
			
			for (j = 0; j < 8; j++)
			{
				for (i = 0; i < planebuff[j].size(); i++)
				{
					if (planebuff[j][i] == MPS)
					{
						Code_MPS();  //Recieve the MPS symbol
					}
					else Code_LPS(); //Recieve the LPS symbol
				}
			}
			Flush();
		}


		if (fun_tupe == 3)//Mapping function 3 : Huffman 
		{
			Initenc();  //Make initialization for the QM code
			ifstream infile(file_name, ios::binary);
			if (!infile)
			{
				cerr << "open error!" << endl;
				abort();
			}

			Buff = (int*)calloc(sizeof(int), 1);
			while (infile.peek() != EOF)
			{
				infile.read((char*)Buff, sizeof(char));
				sam_weight[*Buff]++;
				sum_weight++;

			}

			//Push data into vector
			for (i = 0; i<256; i++)
			{
				if (sam_weight[i]) {
					node_arr.push_back(new  HT_NODE(NULL, NULL, NULL, (unsigned char)i, sam_weight[i], k++));
				}
			}
			sort_ARR(node_arr, node_arr.size());
			Build_tree(node_arr.size());  //Build huffman tree
			generate_code(*node_arr[node_arr.size() - 1], scode);  //generate code
			node_arr.clear();

			/*Rescan the input files and generate code i.e map the sample to binary sequence */
			/*Do QM coed for each symbol*/
			infile.clear();
			infile.seekg(0);    //Return to the top of the input files
			while (infile.peek() != EOF)
			{
				infile.read((char*)Buff, sizeof(char));
				for (i = 0; i < CODE[(*Buff)].code.size(); i++)  //map procedure
				{
					test_count++;
					if (CODE[(*Buff)].code[i] == MPS)
					{
						Code_MPS();  //Recieve the MPS symbol
					}
					else Code_LPS(); //Recieve the LPS symbol
				}
			}
			infile.close();
			Flush();
		}
		if (fun_tupe == 4) //written question
		{
			Initenc();
			s = 10;
			for (i = 0; i < 10; i++)  //map procedure
			{
				test_count++;
				if (TEXT_NUMBER[i] == MPS)
				{
					Code_MPS();  //Recieve the MPS symbol
				}

				else Code_LPS(); //Recieve the LPS symbol
			}
		}


		BIT_FILE *bit_file;
		bit_file = OpenOutputBitFile("binary_stream.dat"); //Build the binary stream file 
		for (i = 1; i < QM_arr.size(); i++)  //map procedure
		{
			OutputBits(bit_file, QM_arr[i], 8);
			out_count++;
		}
		CloseOutputBitFile(bit_file);
		printf("The size of output file is %d Bytes \n\n", QM_arr.size() - 1);

	
	
	system("pause");
	delete Buff;
	return 0;
}



