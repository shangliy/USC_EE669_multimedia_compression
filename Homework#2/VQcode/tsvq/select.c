/******************************************************************************
 *
 * NAME
 *    select.c
 *    Jill R. Goldschneider
 *    March 1994
 *    Last Revision:
 *
 * SYNOPSIS
 *    select -c codebook -s subtree -n selection_number -o output_codebook
 *
 * DESCRIPTION
 *
 *    The input of the program is the codebook file and a preorder
 *    nested subtree list (like the codebook) with one long int used for
 *    each node. If the value is zero, it means that nothing was pruned
 *    at that node.  If the value is non-zero, then there was a pruning
 *    done at that node.  The number n stored indicates that the
 *    nth pruning pass was done at that node.  This is a more convenient
 *    way to store the nested subtree structures rather than to write
 *    every subtree structure as the algorithm says to do.  To get
 *    tree number N, just remove all of the nodes that descend from any
 *    node that has a label of 1,2,...N.
 *
 * OPTIONS
 *    -c  codebook file name                       DEF_codebookname
 *    -s  subtree file name                        DEF_subtreename
 *    -n  selection number
 *    -o  outputfile                               DEF_outputname
 *
 * CALLS
 *    create_root(), construct_tree(), construct_subtree(), prune(),
 *    count_nodes, write_codebook()
 *
 *****************************************************************************/
#include "tsvq.h"

char *programname;
int  dim;

main(argc, argv)
     int       argc;         /* number of command line arguments */
     char      *argv[];      /* command line arguments */
{
  char      subtreename[NAME_MAX];  /* input file of nested subtrees */
  char      outputname[NAME_MAX];   /* output file of codebook */
  char      codebookname[NAME_MAX]; /* file containing codebook */
  FILE      *subtreefile;
  FILE      *outputfile;
  FILE      *codebookfile;
  int       numnodes;               /* number of nodes in the tree */
  char      *cp;
  char      option;
  long      subtree_number = 0;
  TreeNode  *root;

  /* set default values */
  programname = *argv;
  dim = NODE_DATA_SIZE;
  strcpy(codebookname, DEF_codebookname);
  strcpy(subtreename, DEF_subtreename);
  strcpy(outputname, DEF_outputname);

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    printf("%s %s %s\n",USAGE,programname,HOWTOUSE_SELECT);
    printf("\nOPTIONS   DESCRIPTIONS                       DEFAULTS\n");
    printf("-c        codebook file                      %s\n",codebookname);
    printf("-s        nested subtree file                %s\n",subtreename);
    printf("-n        subtree number                     %d\n",subtree_number);
    printf("-o        output codebook file               %s\n",outputname);
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
        case 's':
          strncpy(subtreename,*argv,NAME_MAX);
          break;
        case 'n':
	  sscanf(*argv, "%d", &subtree_number);
          break;
        case 'o':
          strncpy(outputname,*argv,NAME_MAX);
          break;
        default:
          fprintf(stderr,"%s: %c: %s\n",programname,option,NOTOPTION);
          exit(1);
          break;
        }
      }
      else {
        fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_SELECT);
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

  /* user did not enter input file names or valid number */
  if (strlen(codebookname) == 0 || strlen(subtreename) == 0 ||
      subtree_number < 0) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_SELECT);
    exit(5);
  }

  /* user entered an input name which is the same as the output name */
  if (strncmp(cp = codebookname,outputname,NAME_MAX) == 0 ||
      strncmp(cp = subtreename,outputname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
            programname,cp,AND,outputname,ARESAME,ABORT_SELECT);
    exit(6);
  }

  /* user entered the same input names */
  if (strncmp(subtreename,codebookname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
            programname,subtreename,AND,codebookname,ARESAME,ABORT_SELECT);
    exit(7);
  }

  /* assign the default output names if necessary */
  if (strlen(outputname) == 0) {
    sprintf(outputname,"%s.%d",codebookname,subtree_number);
  }

  /* open the files */
  if(!(codebookfile = fopen(cp = codebookname,"r")) ||
     !(subtreefile = fopen(cp = subtreename,"r")) ||
     !(outputfile = fopen(cp = outputname, "w"))) {
    fprintf(stderr,"%s: %s: %s\n",programname,cp,NOTFOUND);
    exit(8);
  }

  /* find the number of nodes */
  if (fread((char *) &numnodes, sizeof(long), 1, codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(9);
  }

  /* find the vector dimension */
  if (fread((char *) &dim,sizeof(int),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(10);
  }

  /* construct the tree */
  if(!(root = create_root())) {
    exit(11);
  }
  if(!(construct_tree(root,numnodes,codebookfile,codebookname))) {
    exit(12);
  }

  if(!(construct_subtree(root,numnodes,subtreefile,subtreename))) {
    exit(13);
  }

  prune(root,subtree_number);

  if((numnodes = count_nodes(root)) == 0) {
    exit(14);
  }

  if(!(write_codebook(root,numnodes,outputfile,outputname))) {
    exit(15);
  }

  /* write to output */
  printf("\nOPTIONS   DESCRIPTIONS                       SETTINGS\n");
  printf("-c        codebook file                      %s\n",codebookname);
  printf("-s        nested subtree file                %s\n",subtreename);
  printf("-n        subtree number                     %d\n",subtree_number);
  printf("-o        output codebook file               %s\n",outputname);
  printf("          number of nodes                    %d\n",numnodes);
  printf("\n");
  fflush(stdout);

 exit(0);
}
