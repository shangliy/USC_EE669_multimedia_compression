/******************************************************************************
 *
 * NAME
 *    voronoi_ts.c
 *    Revised By Preeti Trivedi
 *    March 1994
 *
 *    Jill Goldschneider
 *
 * SYNOPSIS
 *    voronoi_ts -c codebook -o outputfile -r rows -l columns
 *
 * DESCRIPTION
 *    This program should be used with a two-dimension tree structured
 *    codebook to create a voronoi diagram (cell edges only) image
 *    with 8 bpp and user defined dimensions.  The tree data is
 *    normalized to fit within the inner 90% of these dimensions.
 *
 * OPTIONS
 *    -c  codebook file                        DEF_codebookname
 *    -o  output image file                    DEF_outputname
 *    -r  number of rows in output image       DEF_rows
 *    -l  number of rows in output image       DEF_cols
 *
 * CALLS
 *    create_root(), construct_tree(), label_points(),  normalize_tree()
 *    voronoi_diagram()
 *
 *****************************************************************************/
#include "tsvq.h"

char *programname;
int  dim;
int  rows;
int  cols;
FILE *outputfile;
char outputname[NAME_MAX];

int main(argc, argv)
     int   argc;
     char  *argv[];
{
  FILE       *codebookfile;
  char       codebookname[NAME_MAX];
  char       *cp;             /* character pointer */
  char       option;          /* used to for command line interpretation */
  TreeNode   *root;           /* root of the codebook tree structure */

  /* variables used for creating the voronoi diagram */
  int        numnodes;        /* number of nodes */
  int        number;          /* used to label terminal leaves of tree */

  /* initialize input parameters */
  programname = *argv;  /* assign the program name */
  rows = DEF_rows;
  cols = DEF_cols;

  /* initialize the input file names */
  strcpy(codebookname, DEF_codebookname);
  strcpy(outputname, DEF_outputname);

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_VORONOI);
    fprintf(stderr,"\nOPTIONS   DESCRIPTIONS               DEFAULTS\n");
    fprintf(stderr,"-c        codebook                   %s\n",codebookname);
    fprintf(stderr,"-o        output image               %s\n",outputname);
    fprintf(stderr,"-r        rows                       %d\n",rows);
    fprintf(stderr,"-l        columns                    %d\n",cols);
    fprintf(stderr,"\n");
    fflush(stderr);
    exit(0);
  }

  /* read and interpret the command line arguments */
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
	  sscanf(*argv, "%d", &rows);
          break;
	case 'l':
	  sscanf(*argv, "%d", &cols);
          break;
	default:
	  fprintf(stderr,"%s: %c: %s\n",programname,option,NOTOPTION);
	  exit(1);
	  break;
	}
      }
      else {
	fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_VORONOI);
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

  /* user did not enter valid input */
  if (strlen(codebookname) == 0 || rows <= 0 || cols <= 0) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_VORONOI);
    exit(5);
  }

  /* user entered an input name which is the same as the output name */
  if (strncmp(cp = codebookname,outputname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
            programname,cp,AND,outputname,ARESAME,ABORT_VORONOI);
    exit(6);
  }

  /* assign the default output names if necessary */
  if (strlen(outputname) == 0) {
    sprintf(outputname,"%s%s",codebookname,DEF_APPEND_VOR);
  }

  /* open files to read and write */
  if((codebookfile = fopen(cp=codebookname,"r")) == NULL ||
     (outputfile = fopen(cp=outputname,"w")) == NULL) {
    fprintf(stderr,"%s: %s: %s\n",programname,cp,NOTFOUND);
    exit(7);
  }

  /* find the number of nodes */
  if (fread((char *) &numnodes,sizeof(long),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(8);
  }

  /* find the dimension */
  if (fread((char *) &dim,sizeof(int),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(9);
  }
  if (dim != 2) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_VORONOI);
    exit(10);
  }

  /* construct the tree */
  if(!(root = create_root())) {
    exit(11);
  }
  if(!construct_tree(root,numnodes,codebookfile,codebookname)) {
    exit(12);
  }

  fclose(codebookfile);

  /* label the codewords */
  number = 0;
  label_leaf_nodes(root,&number,(numnodes+1)/2);

 /* normalize the tree data to the rows by cols pixel range */
  normalize_tree(root);

  /* encode the image */
  if(!(voronoi_diagram(root))) {
    exit(13);
  }
  exit(0);
}

