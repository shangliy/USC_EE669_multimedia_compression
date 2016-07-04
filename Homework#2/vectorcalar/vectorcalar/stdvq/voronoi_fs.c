/******************************************************************************
 *
 * NAME
 *    voronoi_fs.c
 *    J. R. Goldschneider 5/94
 *
 * MODIFICATIONS
 *    9/95 minor change, three return(NULL)'s becomes return(FALSE)'s
 *    in voronoi_diagram().
 *
 * SYNOPSIS
 *    voronoi_fs -c codebook -o output -r rows -l columns
 *
 * DESCRIPTION
 *    This program should be used with a two-dimension full search
 *    codebook to create a voronoi diagram (cell edges only) image
 *    with 8 bpp and user defined dimensions.  The tree data is
 *    normalized to fit within the inner 90% of these dimensions.
 *
 * OPTIONS
 *    -c  codebook file name
 *    -o  output file name
 *    -r  number of rows in output image
 *    -l  number of columns in output image
 *
 * FLAGS
 *
 *    See vq.h for definitions of default values
 *
 * CALLS
 *    normalize_codebook, voronoi_diagram, find_cell
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vq.h"

FILE    *outputfile;                /* voronoi diagram */
char    outputname[NAME_MAXIMUM];   /* name of reproduction image */
long    codebooksize;
int     rows, cols;
int     dimension;
char    *programname;

void    normalize_codebook();
BOOLEAN voronoi_diagram();
int     find_cell();

int main(argc, argv)
     int  argc;
     char *argv[];

{
  char    option;          /* used for command line interpretation */
  char    codebookname[NAME_MAXIMUM]; /* name of codebook */
  FILE    *codebookfile;   /* pointer to codebook file */
  double  **codebook;      /* codewords stored [index][dimension] */
  long    i;               /* index */

  /* set default values */
  programname = *argv;
  rows = DEF_ROWS;
  cols = DEF_COLS;

  sprintf(outputname,DEF_OUTNAME);
  sprintf(codebookname,DEF_INNAME);

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    printf("%s %s %s\n",USAGE,programname,HOWTOUSE_VOR);

    printf("\nOPTIONS   DESCRIPTIONS                         DEFAULTS\n");
    printf("-c        codebook name                        %s\n",codebookname);
    printf("-o        output name                          %s\n",outputname);
    printf("-r        rows                                 %d\n",rows);
    printf("-l        columns                              %d\n",cols);
    printf("\n");
    fflush(stdout);
    exit(0);
  }

  /* read and interpret command line arguments */
  while (--argc && ++argv) {
    if (*argv[0]=='-' && strlen(*argv)==2) { /* each option has 1 letter */
      option = *++argv[0];
      if (--argc && ++argv) { /* examine the option */
	switch(option) { /* examine the option letter */
	case 'c':
	  strncpy(codebookname,*argv,NAME_MAX);
	  break;
	case 'o':
	  strncpy(outputname,*argv,NAME_MAX);
	  break;
	case 'r':
          sscanf(*argv,"%d",&rows);
          break;
	case 'l':
          sscanf(*argv,"%d",&cols);
          break;
	default:
	  fprintf(stderr,"%s: %c: %s\n",programname,option,NOTOPTION);
	  exit(1);
	  break;
	}
      }
      else {
	fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_VOR);
	exit(2);
      }
    }

    else if (*argv[0] == '-') { /* user entered unknown option */
      ++argv[0];
      fprintf(stderr,"%s: %s: %s\n",programname,*argv,NOTOPTION);
      exit(3);
    }
    else { /* user entered unknown string */
      fprintf(stderr,"%s: %s: %s\n",programname,*argv,NOTOPTION);
      exit(4);
    }
  }

  /* user did not enter a codebook file name */
  if ( strlen(codebookname) == 0 ) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_VOR);
    exit(5);
  }

  /* user entered a codebook name which is the same as the output name */
  if (strncmp(codebookname,outputname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
	    programname,codebookname,AND,outputname,ARESAME,ABORT_STDVQE);
    exit(6);
  }

  /* if a codebook file is given but not an output file, assign output name */
  if (strlen(outputname) == 0) {
    sprintf(outputname,"%s%s",codebookname,DEF_APPEND_VOR);
  }

  /* open the files */
  if((codebookfile = fopen(codebookname,"r")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOTFOUND);
    exit(7);
  }
  if((outputfile = fopen(outputname,"w")) == NULL){
    fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOTFOUND);
    exit(8);
  }

  /* allocate memory for each codeword and read the codeword */
  rewind(codebookfile);

  /* find the codebooksize */
  if (fread((char *) &codebooksize,sizeof(long),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(9);
  }

  /* find the dimension, can make voronoi diagram of 2 dimension codebook */
  if (fread((char *) &dimension,sizeof(int),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(10);
  }
  if (dimension != 2) {
    fprintf(stderr,"%s: %s\n",programname,HOWTOUSE_VOR);
    exit(11);
  }

  /* allocate memory for the codebook */
  if (!(codebook = (double **) calloc(codebooksize,sizeof(double *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    exit(12);
  }

  /* read the codebook */
  for(i = 0; i < codebooksize; i++) {
    if (!(codebook[i] = (double *) calloc(dimension,sizeof(double)))) {
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      exit(13);
    }
    if (fread((char *) codebook[i],sizeof(double),dimension,codebookfile)
	 != dimension || feof(codebookfile) || ferror(codebookfile) ) {
      fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
      exit(14);
    }
  }
  fclose(codebookfile);

  /* normalize the tree data to the rows by cols range */
  normalize_codebook(codebook);

  /* encode the image */
  if(!voronoi_diagram(codebook)) {
    exit(15);
  }

  fflush(stdout);
  fclose(outputfile);
  exit(0);
}

void normalize_codebook(codebook)
     double **codebook;
{
  double  maxx,maxy,minx,miny;
  int     i;

  maxx = codebook[0][0];
  minx = codebook[0][0];
  maxy = codebook[0][1];
  miny = codebook[0][1];

  /* find the maximum and minimum x and y coordinate values */
  for(i = 0; i < codebooksize; i++) {
    if(codebook[i][0] > maxx) maxx = codebook[i][0];
    if(codebook[i][0] < minx) minx = codebook[i][0];
    if(codebook[i][1] > maxy) maxy = codebook[i][1];
    if(codebook[i][1] < miny) miny = codebook[i][1];
  }

  /* fit all codewords into the user defined dimension of rows by cols */
  for(i = 0; i < codebooksize; i++) {
    codebook[i][0] -= minx;
    codebook[i][1] -= miny;

    codebook[i][0]=codebook[i][0]/(maxx-minx)*(rows-2*((int) rows/20))+rows/20;
    codebook[i][1]=codebook[i][1]/(maxy-miny)*(cols-2*((int) cols/20))+cols/20;

  }
}

BOOLEAN voronoi_diagram(codebook)
     double **codebook;
{
  int     i,j;            /* counters and indices */
  int     *datavector;    /* vector to be encoded */
  int     **region_image; /* output image */
  unsigned char **edge_image;

  /* allocate memory */
  if(!(datavector = (int *) calloc(dimension,sizeof(int))) ||
     !(edge_image = (unsigned char **) calloc(rows,sizeof(unsigned char *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  if(!(region_image = (int **) calloc((rows+1),sizeof(int *))) ||
     !(edge_image = (unsigned char **) calloc(rows,sizeof(unsigned char *)))) {
    fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
    return(FALSE);
  }
  for (i = 0; i <= rows; i++) {
    if(!(region_image[i] = (int *) calloc((cols+1),sizeof(int)))) {
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      return(FALSE);
    }
  }
  for (i = 0; i < rows; i++) {
    if(!(edge_image[i]=(unsigned char *) calloc(cols,sizeof(unsigned char)))) {
      fprintf(stderr,"%s: %s\n",programname,NOMEMORY);
      return(FALSE);
    }
  }

  /* begin encoding blank image*/
  for (i = 0; i <= rows; i++) {
    for (j = 0; j <= cols; j++) {
      datavector[0] = i;
      datavector[1] = j;
      region_image[i][j] = find_cell(datavector,codebook);
    }
  }

  /* do edge detection */
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      if((region_image[i][j] != region_image[i][j+1]) ||
         (region_image[i][j] != region_image[i+1][j]) ||
         (region_image[i][j] != region_image[i+1][j+1])) {
        edge_image[i][j] = 0;
      }
      else {
        edge_image[i][j] = 255;
      }
    }
  }

  /* write the voronoi image */
  for (i = 0; i < rows; i++) {
    if (fwrite((char *) edge_image[i],sizeof(unsigned char),
	       cols,outputfile) != cols) {
      fprintf(stderr,"%s: %s: %s\n",programname,outputname,NOWRITE);
      return(FALSE);
    }
  }
  return(TRUE);
}

int find_cell(vector,codebook)
     int    *vector;
     double **codebook;
{
  double  bestdistortion; /* distortion between vector and best codeword */
  double  tempdistortion; /* temporary variable */
  double  temp;           /* temporary variable */
  int     bestcodeword;   /* index of closest codeword */
  int     i,j;            /* counters */

  bestdistortion = HUGE; /* keep convention that ties go to lower index */
  bestcodeword = 0;

  /* find the best codeword */
  for (i = 0; i < codebooksize; i++) {

    tempdistortion = 0.0;
    for (j = 0; j < dimension; j++) {
      temp = ( (double) vector[j]) - codebook[i][j];
      tempdistortion += temp*temp;
    }

    if (tempdistortion < bestdistortion) {
      bestdistortion = tempdistortion;
      bestcodeword = i;
    }
  } /* the best codeword has been found */

  return(bestcodeword);
}

/* alternate version to use when comparing codebooks side by side

void normalize_codebook(codebook)
     double **codebook;
{
  double  row_scale, col_scale;
  int     i;

  /* assume maximum codeword value is 256
  row_scale = (double) rows / 256;
  col_scale = (double) cols / 256;

  /* fit all codewords into the user defined dimension of rows by cols
  for(i = 0; i < codebooksize; i++) {
    codebook[i][0] *= row_scale;
    codebook[i][1] *= col_scale;
  }
}
*/
