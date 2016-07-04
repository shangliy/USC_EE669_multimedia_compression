#include "stdafx.h"
#include "fstream"
#include "iostream"
#include "vector"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bitio.h"
#include "errhand.h"
#include <malloc.h>
#include <math.h>
#include "vq.h"
#include "stdvq.def"
#include "stdvq.h"
errno_t err;
extern double lloyd0();
extern double lloyd1();
extern double lloyd2();
using namespace std;


/******************************************************************************
*
* NAME
*    stdvq_util.c
*    J. R. Goldschneider 5/93
*
* MODIFICATIONS
*    7/93 min_sup and max_inf added for constrained searches. JRG
*    5/94 cleaned up two cast mistakes in min_sup and max_inf. JRG
*
* SYNOPSIS
*    splitcodewords(codebook,oldsize,newsize)
*    perturb(oldcodeword,newcodeword)
*    min_sup(first, last, tempnorm, norm)
*    max_inf(first, last, tempnorm, norm)
*    writecodebook(codebook,size)
*
* DESCRIPTION
*    see below
*
* RETURN VALUE
*    see below
*
* PARAMETERS
*    see below
*
* CALLS
*
*****************************************************************************/




/******************************************************************************
*
*    perturb takes oldcodeword and changes is slightly to form
*    newcodeword.  oldcodeword is not altered in the process.
*    scale is available to change so that a codeword can be split many times
*    if necessary and still have each resulting new codeword be different.
*
*****************************************************************************/

void perturb(double *oldcodeword, double *newcodeword, long   scale)

{
	int i;
	float  addoffset, muloffset;

	addoffset = offset_add / (pow(2.0, (double)scale));
	muloffset = offset_mul / (pow(2.0, (double)scale));

	for (i = 0; i < dimension; i++) {
		if (oldcodeword[i] == 0.0) {
			newcodeword[i] = addoffset*((double)rand()) / 2147483647.0;
		}
		else if (fabs(oldcodeword[i]) < 0.9*addoffset) {
			newcodeword[i] = oldcodeword[i] + addoffset*
				(fabs(oldcodeword[i]) / oldcodeword[i])*((double)rand()) / 2147483647.0;
		}
		else {
			newcodeword[i] = oldcodeword[i] + muloffset*
				oldcodeword[i] * ((double)rand()) / 2147483647.0;
		}
	}
}

/******************************************************************************
*
*    splitcodewords takes a codebook and creates oldsize-newsize new codewords
*    from the old codewords.  The old codewords are not modified.
*
*****************************************************************************/

void splitcodewords(double **codebook, long   oldsize, long   newsize, long   scale)

{
	int    i;

	/* create the new codewords */
	for (i = 0; i < newsize - oldsize; i++) {
		perturb(codebook[i], codebook[i + oldsize], scale);
	}
}


double lloyd0(double **codebook, long   size)

{
	double  distortion;     /* distortion between training set and codebook */
	double  olddistortion;  /* distortion from the previous pass of GLA */
	double  bestdistortion; /* distortion between 1 vector and best codeword */
	double  tempdistortion; /* temporary variable */
	double  temp;           /* temporary variable */
	long    numbervectors;  /* number of vectors used */
	long    i, j, n;          /* counters and indices */
	long    pass;           /* the number of attempts to split empty cells */
	long    bestcodeword;   /* index of the best codeword for a given vector */
	long    emptycells;     /* number of empty cells */
	long    *count;         /* array of number of vectors per region */
	double  **centroid;     /* array of centroids of each voronoi region */
	DATA    *tempvector;    /* the training vector */

	/* allocate memory for the centroids, temporary vector, and count vector */
	if (!(centroid = (double **)calloc(size, sizeof(double *))) ||
		!(tempvector = (DATA *)calloc(dimension, sizeof(DATA))) ||
		!(count = (long *)calloc(size, sizeof(long)))) {
		fprintf(stderr, "%s: %s\n", programname, NOMEMORY);
		return(-1.0);
	}

	/* allocate memory for the dimension of each centroid, and initialize the
	* the centroids and countnumber */
	for (i = 0; i < size; i++) {
		if (!(centroid[i] = (double *)calloc(dimension, sizeof(double)))) {
			fprintf(stderr, "%s: %s\n", programname, NOMEMORY);
			return(-1.0);
		}
		for (j = 0; j < dimension; j++) {
			centroid[i][j] = 0.0;
		}
		count[i] = 0;
	}

	/* do the lloyd iteration.  Use the nearest neighbor condition to
	find the cells.  Then find the centroid of each cell. */

	olddistortion = HUGE; /* first pass requires very large distortion */
	emptycells = 1; /* ensures that loop is done at least one time */
	pass = 0; /* no empty cells have been found yet */

	do {
		/* compute distortion */
		distortion = 0.0;
		rewind(trainingfile);

		/* read in vector and find the closest codeword */
		while (fread(tempvector, sizeof(DATA), dimension, trainingfile) ==
			dimension && !feof(trainingfile) && !ferror(trainingfile)) {
			bestdistortion = HUGE; /* keep convention that ties go to lower index */
			bestcodeword = 0;
			for (i = 0; i < size; i++) { /* find the best codeword */
				tempdistortion = 0.0;
				for (j = 0; j < dimension; j++) {
					temp = ((double)tempvector[j]) - codebook[i][j];
					tempdistortion += temp*temp;
					if (tempdistortion > bestdistortion) j = dimension;
				}

				if (tempdistortion < bestdistortion) {
					bestdistortion = tempdistortion;
					bestcodeword = i;
				}

				/* if the bestdistortion is 0.0, the best codeword is found */
				if (bestdistortion == 0.0) i = size;
			}
			count[bestcodeword]++;
			for (j = 0; j < dimension; j++) {
				centroid[bestcodeword][j] += (double)tempvector[j];
			}
			distortion += bestdistortion;
		}    /* all training vectors have been encoded */

			 /* normalize the distortion */
		numbervectors = 0;
		for (i = 0; i < size; i++) {
			numbervectors += count[i];
		}
		if (numbervectors == 0) {
			fprintf(stderr, "%s: %s: %s\n", programname, trainingname, NOTFOUND);
			return(-1.0);
		}
		distortion /= (double)numbervectors;
		if (distortion < 0) {
			fprintf(stderr, "%s: %s: %s\n", programname, OVERFLOWED, ABORT_STDVQ);
			return(-1.0);
		}

		/* if distortion = 0.0 or if change in distortion < threshold AND
		if there aren't any empty cells, exit */
		if ((emptycells == 0) &&
			((distortion == 0.0) ||
			((olddistortion - distortion) / distortion < threshold))) {

			/* if distortion is 0, let the program exit gracefully */
			if (distortion == 0 && size < codebooksize) {
				fprintf(stderr, "%s %d\n", STOP, size);
				size = codebooksize;
			}
			return(distortion);
		}

		/* Find the number of empty cells */
		emptycells = 0;
		for (i = 0; i < size; i++) {
			if (count[i] == 0) ++emptycells;
		}

		/* no empty cells, find new centroids and reinitialize for next pass */
		if (emptycells == 0) {
			for (i = 0; i < size; i++) {
				for (j = 0; j < dimension; j++) {
					codebook[i][j] = centroid[i][j] / (double)count[i];
					centroid[i][j] = 0.0;
				}
				count[i] = 0;
			}
			olddistortion = distortion;
		}

		/* there are empty cells, split the most populous codewords. try again */
		else {

			/* if the distortion is 0, can't split cells, exit program gracefully */
			if (distortion == 0.0) {
				if (emptycells > 1) {
					fprintf(stderr, "%s %d %s %d\n",
						NOFILL, emptycells, EMPTYCELLS, size);
				}
				else {
					fprintf(stderr, "%s %d %s %d\n",
						NOFILL, emptycells, EMPTYCELL, size);
				}
				codebooksize = size - emptycells;
				fprintf(stderr, "%s %d\n", STOP, codebooksize);
				return(distortion);
			}

			/* If there have been too many attempts to fill cells, exit program */
			if (pass == MAX_SPLIT_ATTEMPTS) {
				if (emptycells > 1) {
					fprintf(stderr, "%s %d %s %d\n", NOFILL, emptycells, EMPTYCELLS, size);
				}
				else {
					fprintf(stderr, "%s %d %s %d\n", NOFILL, emptycells, EMPTYCELL, size);
				}
				fprintf(stderr, "%s\n", ABORT_STDVQ);
				return(-1.0);
			}

			/* consolidate the nonempty codewords at the beginning of the
			array with the most populous cells first. */
			for (n = 0; n < size - emptycells; n++) {
				j = 0;
				bestcodeword = 0;
				for (i = 0; i < size; i++) {
					if (count[i] > j) {
						j = count[i];
						bestcodeword = i;
					}
				}

				for (j = 0; j < dimension; j++) { /* find centroid */
					codebook[n][j] = centroid[bestcodeword][j] /
						(double)count[bestcodeword];
					centroid[bestcodeword][j] = 0.0;
				}
				count[bestcodeword] = 0;
			}

			/* try getting new codewords */
			if (emptycells > 1) {
				fprintf(stderr, "%s %d %s %d\n", TRYFILL, emptycells, EMPTYCELLS, size);
			}
			else {
				fprintf(stderr, "%s %d %s %d\n", TRYFILL, emptycells, EMPTYCELL, size);
			}
			fflush(stderr);

			/* split the required number of codewords */
			splitcodewords(codebook, size - emptycells, size, pass);
			olddistortion = distortion;
			pass++;
		}

	} while (TRUE);

	/* should never get here */
	return(-1.0);
}



int main()

{
	char   option;      /* used for command line interpretation */
	double **codebook;  /* codebook array */
	double distortion;  /* ave. distortion between codebook and training set */
	long   i, j;         /* counters */
	int    speedup;     /* indicates which type of speedup to use */

		strcpy_s(trainingname,20, "chem.256");
		strcpy_s(codebookname,20, "coebook");
		dimension = 4;
		codebooksize = 16;
		speedup = 0;
		offset_add = 25;
		offset_mul = 0.5;
		threshold = 0.005;

	/* allocate memory for the codebook */
	if (!(codebook = (double **)calloc(codebooksize, sizeof(double *)))) {
		fprintf(stderr, "%s: %s\n", programname, NOMEMORY);
	}

	/* allocate memory for the dimension of each codeword */
	for (i = 0; i < codebooksize; i++) {
		if (!(codebook[i] = (double *)calloc(dimension, sizeof(double)))) {
			fprintf(stderr, "%s: %s\n", programname, NOMEMORY);	
		}
	}

	/* open the training file */
	if ((err = fopen_s(&trainingfile, trainingname, "r")) != 0)
		cout <<"The Input file was not opened\n";
	else
		cout <<"The Input file was opened\n";
	
	/* perform generalize lloyd algorithm on all codebook sizes */
	for (i = 1; i < codebooksize;) {

		/* run the GLA for codebook of size i */
		if ((distortion = lloyd0(codebook, i)) < 0) {
			cout << "Unable to generate the codebook\n";
		}
	
		/* if distortion is zero, no need to continue.
		note that lloyd can and will change codebooksize in such a case */
		if (distortion == 0) break;

		/* display the distortion of the training set to the codebook of size i */

		fprintf(stderr, "%s %-7d:  %f\n", DISTORTION, i, distortion);

		/* find the number of new codewords that need to be made (j-i) */
		if ((j = 2 * i) > codebooksize) j = codebooksize;

		/* split the codewords */
		splitcodewords(codebook, i, j, 0);

		/* increment the codebook size */
		i = j;
	}

	/* it may be that distortion is 0, so we can exit early */
	if (distortion == 0) {
		printf("%s %-7d:  %f\n", DISTORTION, i, distortion);
		fflush(stdout);
		
		fclose(trainingfile);
		return 0;
	}

	/* do the final codebook */
	if ((distortion = lloyd0(codebook, codebooksize)) < 0) exit(16);
	printf("%s %-7d:  %f\n", DISTORTION, codebooksize, distortion);
	fflush(stdout);
	double c[16][4] = { 0 };
	for (i = 0; i < codebooksize; i++)
	{
		for (j = 0; j < dimension; j++)
			c[i][j] = codebook[i][j];
			
	}

	fclose(trainingfile);
	return 0;
}

