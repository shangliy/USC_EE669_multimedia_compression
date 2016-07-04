/******************************************************************************
 *
 * NAME
 *    prune.c
 *    Jill R. Goldschneider
 *    March 1994
 *    Last Revision:
 *
 * SYNOPSIS
 *    prune -c codebook -s statfile -o output -E
 *
 * DESCRIPTION
 *    This program should be used to prune a codebook created by tsvq.c.
 *    It uses a codebook file to find the tree structure, and a
 *    statistics file to find the count and average distortion for each
 *    node of the tree. See pages 683-684 of _Vector Quantization and
 *    Signal Compression_ by Gersho & Gray for the pruning algorithm.
 *    The -E option implements entropy constrained pruning.
 *    The output of the program is preorder list (like the codebook)
 *    with one long int used for each node. If the value is zero,
 *    it means that nothing was pruned at that point.  If the value
 *    is non-zero, then there was a pruning done at that node.
 *    The number n stored indicates that that node was the nth pruning
 *    done.  This is a more convenient way to store the nested
 *    subtree structures rather than to write every subtree structure
 *    as the algorithm says to do.  To get tree number N, just
 *    remove all of the nodes that descend from any node that has
 *    a label of 1,2,...N. Prune prints to stdout for each subtree: the
 *    subtree number, the minimum lambda of that tree, the averate rate
 *    and distortion, and the number of nodes in the tree.
 *
 *    The program uses the same TreeNode structure as the tsvq.c
 *    and tsvqe.c programs.  The trick is that the node->data field
 *    is used to store the four pieces of data needed for pruning rather
 *    than to store the codeword.  The size of node->data is set by using dim,
 *    so dim is set to be NODE_DATA_SIZE (4). The four pieces of
 *    information are stored as:
 *        node->data[DELTA_D] = delta_d
 *        node->data[DELTA_R] = delta_r
 *        node->data[LAMBDA]  = lambda
 *        node->data[LAMBDA_MIN]  = lambda_min
 *    The next trick is that the node->count field is used to store
 *    the subtree number.
 *
 * OPTIONS
 *    -c  codebook file name                       DEF_codebookname
 *    -s  statistics file name                     DEF_statname
 *    -o  nested subtree file nane                 DEF_subtreename
 *    -E  entropy pruned flag                      FALSE
 *
 * CALLS
 *    create_root(), construct_stat_tree(), initialize_stat_tree(),
 *    entropy_init_stat_tree(), find_min_slope(), pruned_count(),
 *    update_tree(), count_nodes(), write_nested_subtree()
 *
 *****************************************************************************/
#include "tsvq.h"

char *programname;
int  dim;

main(argc, argv)
     int       argc;         /* number of command line arguments */
     char      *argv[];      /* command line arguments */
{
  char      *cp;               /* character pointer */
  char      option;            /* used for command line interpretation */
  char      statname[NAME_MAX];     /* file containing count and distortion */
  char      subtreename[NAME_MAX];  /* output file of nested subtrees */
  char      codebookname[NAME_MAX]; /* file containing codebook */
  FILE      *statfile;
  FILE      *subtreefile;
  FILE      *codebookfile;
  long      numnodes;          /* number of nodes in the tree */
  int       vector_dimension;  /* vector dimension not needed for pruning */
  long      subtree_number;    /* the number of subtrees on the convex hull */
  long      trsqcount;         /* the number of training vectors */
  DISTTYPE  rate, distortion;
  TreeNode  *root, *min_slope_node; /* pointer to node with smallest slope */
  BOOLEAN   entropy_flag;      /* flag used to select entropy pruned tsvq */

  /* set default values */
  programname = *argv;
  dim = NODE_DATA_SIZE;
  entropy_flag = FALSE;
  strcpy(statname, DEF_statname);
  strcpy(codebookname, DEF_codebookname);
  strcpy(subtreename, DEF_subtreename);

  /* if no options entered, list all of the defaults */
  if (argc == 1) {
    printf("%s %s %s\n",USAGE,programname,HOWTOUSE_PRUNE);
    printf("\nOPTIONS   DESCRIPTIONS                         DEFAULTS\n");
    printf("-c        codebook file                        %s\n",codebookname);
    printf("-s        codebook statistics file             %s\n",statname);
    printf("-o        nested subtree file                  %s\n",subtreename);
    printf("-E        entropy pruned                         \n");
    printf("\n");
    fflush(stdout);
    exit(0);
  }

  /* read and interpret command line arguments */
  while (--argc && ++argv) {
    if (*argv[0]=='-' && strlen(*argv)==2) { /* each option has 1 letter */
      option = *++argv[0];
      if (option == 'E') entropy_flag = TRUE;
      else if (--argc && ++argv) { /* examine the option */
        switch(option) { /* examine the option letter */
        case 'c':
          strncpy(codebookname,*argv,NAME_MAX);
          break;
        case 's':
          strncpy(statname,*argv,NAME_MAX);
          break;
        case 'o':
          strncpy(subtreename,*argv,NAME_MAX);
          break;
        default:
          fprintf(stderr,"%s: %c: %s\n",programname,option,NOTOPTION);
          exit(1);
          break;
        }
      }
      else {
        fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_PRUNE);
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

  /* user did not enter input file names */
  if (strlen(codebookname) == 0 || strlen(statname) == 0) {
    fprintf(stderr,"%s %s %s\n",USAGE,programname,HOWTOUSE_PRUNE);
    exit(5);
  }

  /* user entered an input name which is the same as the output name */
  if (strncmp(cp = codebookname,subtreename,NAME_MAX) == 0 ||
      strncmp(cp = statname,subtreename,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
            programname,cp,AND,subtreename,ARESAME,ABORT_PRUNE);
    exit(6);
  }

  /* user entered the same input names */
  if (strncmp(statname,codebookname,NAME_MAX) == 0) {
    fprintf(stderr,"%s: %s %s %s %s: %s\n",
            programname,statname,AND,codebookname,ARESAME,ABORT_PRUNE);
    exit(7);
  }

  /* assign the default output names if necessary */
  if (strlen(subtreename) == 0) {
    sprintf(subtreename,"%s%s",codebookname,DEF_APPEND_PRUNE);
  }

  /* open the files */
  if(!(codebookfile = fopen(cp = codebookname,"r")) ||
     !(statfile = fopen(cp = statname,"r")) ||
     !(subtreefile = fopen(cp = subtreename, "w"))) {
    fprintf(stderr,"%s: %s: %s\n",programname,cp,NOTFOUND);
    exit(8);
  }

  /* write to output */
  printf("\nOPTIONS   DESCRIPTIONS                         SETTINGS\n");
  printf("-c        codebook file                        %s\n",codebookname);
  printf("-s        codebook statistics file             %s\n",statname);
  printf("-o        nested subtree file                  %s\n",subtreename);
  if(entropy_flag) {
    printf("-E        entropy is reported in               bits per vector\n");
  }
  else {
    printf("          rate is reported in                  bits per vector\n");
  }
  printf("          distortion is reported in            bits per vector\n");
  fflush(stdout);

  /* find the number of nodes */
  if (fread((char *) &numnodes, sizeof(long), 1, codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(9);
  }

  /* find the vector dimension */
  if (fread((char *) &vector_dimension,sizeof(int),1,codebookfile) != 1) {
    fprintf(stderr,"%s: %s: %s\n",programname,codebookname,NOREAD);
    exit(10);
  }

  /* write more to output */
  printf("          vector dimension                     %d\n",
	 vector_dimension);
  printf("\n");
  fflush(stdout);

  /* construct the tree */
  if(!(root = create_root())) {
    exit(11);
  }
  if(!(construct_stat_tree(root,numnodes,codebookfile,codebookname,
			   statfile,statname))) exit(12);

  /* find the number of training vectors */
  trsqcount = root->count;

  /* initialize the tree */
  if (entropy_flag) {
    entropy_init_stat_tree(root);
  }
  else {
    initialize_stat_tree(root);
  }

  /* find the initial rate and distortion */
  rate = root->data[DELTA_R];
  distortion = root->avmse + root->data[DELTA_D];
  subtree_number = 0;

  if(!entropy_flag) {
   printf("SUBTREE          LAMBDA            RATE      DISTORTION   NODES\n");
  }
  else {
   printf("SUBTREE          LAMBDA         ENTROPY      DISTORTION   NODES\n");
  }

  /* print the tree information */
  printf("%7d %15f %15f %15f %7d\n",subtree_number, root->data[LAMBDA_MIN],
	 rate/trsqcount, distortion/trsqcount, numnodes);
  fflush(stdout);

  /* prune the tree until only one node left */
  while (root->data[LAMBDA_MIN] < HUGE) {

    /* find the node with the minimum slope */
    if(!(min_slope_node = find_min_slope(root))) exit(13);

    /* find the new rate and distortion */
    rate -= min_slope_node->data[DELTA_R];
    distortion -=  min_slope_node->data[DELTA_D];

    /* label the node with its subtree number */
    subtree_number++;
    min_slope_node->count = subtree_number;

    /* find the number of nodes left in the tree */
    numnodes = numnodes - pruned_count(min_slope_node) + 1;

    /* update the tree */
    update_tree(min_slope_node);

    /* print the tree information */
    printf("%7d %15f %15f %15f %7d\n", subtree_number, root->data[LAMBDA_MIN],
	   rate/trsqcount, distortion/trsqcount, numnodes);
    fflush(stdout);
  }

  numnodes = count_nodes(root);

  if(!(write_nested_subtree(root,numnodes,subtreefile,subtreename))) exit(14);

  exit(0);
}
