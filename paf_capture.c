#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>

#include "paf_capture.h"
#include "sync.h"
#include "capture.h"
#include "multilog.h"
#include "dada_def.h"

extern int quit;
multilog_t *runtime_log;

/*
  1. Finish the ring buffer part, should be working now;
  2. Replace frequency with source ip for the connection check, do not do that as it also give us frequency information;
  3. Better debug option, DONE???
  4. Get log information, kown how to do it now, but do not know how to view it;
  5. Use single thread program to count the data frames, done,we still have capture problem;
  6. Data write out;
  7. Check the write out data;
*/


void usage()
{
  fprintf (stdout,
	   "paf_capture - capture PAF BMF raw data from NiC\n"
	   "\n"
	   "Usage: paf_capture [options]\n"
	   " -a Hexadecimal shared memory key for capture \n"
	   " -b Enable start-of-data or not, WILL BE REMOVED LATER \n"
	   " -c The size of each capture ring buffer block in data frame steps    \n"
	   " -d Record header of data packets or not    \n"
	   " -e IP adress and port on which we capture data, accept multiple values with -e value1 -e value2 ... the format of it is \"ip:port:number-of-chunks\" \n"
	   " -f The name of DADA header template file    \n"
	   " -g The name of epoch file, which records the conversion for BMF timestamp  \n"
	   " -h Show help    \n"
	   " -i The center frequency of captured data    \n"
	   " -j The length of data capture, WILL BE REMOVED LATER \n"
	   " -k Which directory will be used to record data    \n");
}

int main(int argc, char **argv)
{
  /* Initial part */ 
  int i, arg;
  int node_id = 0;                    // Default node id;
  int nic_id = 1;                     // Default nic id;
  char ip[MSTR_LEN];
  double length = 36.000;             // Default observation length in seconds;
  char hostname[HN_LEN + 1];          // The name of host computer;
  conf_t conf;
  FILE *fp_log = NULL;
  char log_fname[MSTR_LEN], hfname[MSTR_LEN], efname[MSTR_LEN];

  conf.nport = 0;
  
  while((arg=getopt(argc,argv,"a:b:c:d:e:f:g:hi:j:k:")) != -1)
    {
      switch(arg)
	{
	case 'h':
	  usage();
	  return EXIT_FAILURE;
	  
	case 'a':	  	  
	  if (sscanf (optarg, "%x", &conf.key) != 1)
	    {
	      //multilog(runtime_log, LOG_INFO, "Could not parse key from %s, which happens at \"%s\", line [%d].\n", optarg, __FILE__, __LINE__);
	      fprintf (stderr, "Could not parse key from %s, which happens at \"%s\", line [%d].\n", optarg, __FILE__, __LINE__);
	      return EXIT_FAILURE;
	    }
	  break;

	case 'b':
	  sscanf(optarg, "%d", &conf.sod);
	  break;

	case 'c':
	  sscanf(optarg, "%zu", &conf.rbuf_ndf);
	  break;
	  
	case 'd':
	  sscanf(optarg, "%d", &conf.hdr);
	  break;
	  
	case 'e':
	  sscanf(optarg, "%[^:]:%d:%d", conf.ip[conf.nport], &conf.port[conf.nport], &conf.nchunk_port[conf.nport]);
	  conf.nport ++;
	  break;

	case 'f':	  	  
	  sscanf(optarg, "%s", conf.hfname);
	  break;

	case 'g':
	  sscanf(optarg, "%s", conf.efname);
	  break;

	case 'i':
	  sscanf(optarg, "%lf", &conf.freq);
	  break;
	
	case 'j':
	  sscanf(optarg, "%lf", &conf.length);
	  break;
	  	    
	case 'k':
	  sscanf(optarg, "%s", conf.dir);
	  break;
	}
    }

  //fprintf(stdout, "%s\t%d\t%d\n", conf.ip[0], conf.port[0], conf.nchunk[0]);

#ifdef DEBUG
  fprintf(stdout, "*********************************************\n");
  fprintf(stdout, "WE ARE ON INIT PART OF WHOLE PROGRAM...\n");
  fprintf(stdout, "*********************************************\n");
#endif
  
#ifdef DEBUG
  fprintf(stdout, "\nWe are working on %s, nic %d.\n", hostname, nic_id);
  fprintf(stdout, "The IP address is %s.\n\n", ip);
#endif

  /* Setup log interface */
  //sprintf(log_fname, "%s/paf_capture_%s_NiC%d.log", conf.dir, hostname, nic_id);
  sprintf(log_fname, "%s/paf_capture.log", conf.dir);
  fp_log = fopen(log_fname, "ab+"); // File to record log information
  if(fp_log == NULL)
    {
      fprintf(stderr, "Can not open log file %s\n", log_fname);
      return EXIT_FAILURE;
    }
  runtime_log = multilog_open("paf_capture", 1);
  multilog_add(runtime_log, fp_log);
  multilog(runtime_log, LOG_INFO, "START PAF_CAPTURE\n");
    
  /* Capture part*/
  if(init_capture(&conf) == EXIT_FAILURE)
    {
      multilog(runtime_log, LOG_ERR, "Can not initialise the capture, which happens at \"%s\", line [%d].\n", __FILE__, __LINE__);
      //fprintf (stderr, "Can not initialise the capture, which happens at \"%s\", line [%d].\n", __FILE__, __LINE__);
      return EXIT_FAILURE;      
    }
  if(threads(&conf) == EXIT_FAILURE)
    {
      multilog(runtime_log, LOG_ERR, "Can not capture data, which happens at \"%s\", line [%d].\n", __FILE__, __LINE__);
      //fprintf(stderr, "Can not capture data, which happens at \"%s\", line [%d].\n", __FILE__, __LINE__);
      return EXIT_FAILURE;
    }

  /* Check the result */
  statistics(conf);
  
  /* Cleanup */
  if(destroy_capture(conf) == EXIT_FAILURE)
    {
      multilog(runtime_log, LOG_ERR, "Can not destroy buffers, which happens at \"%s\", line [%d].\n", __FILE__, __LINE__);
      //fprintf(stderr, "Can not destroy buffers, which happens at \"%s\", line [%d].\n", __FILE__, __LINE__);
      return EXIT_FAILURE;
    }

#ifdef DEBUG
  fprintf(stdout, "\n*********************************************\n");
  fprintf(stdout, "WE ARE ON FINISHING PART...\n");
  fprintf(stdout, "*********************************************\n");

  if(quit == 1)
    fprintf(stdout, "Forced quit!\n\n");
  else
    fprintf(stdout, "Finish successful!\n\n");
#endif
  
  /* Destory log interface */
  multilog(runtime_log, LOG_INFO, "FINISH PAF_CAPTURE\n\n");
  multilog_close(runtime_log);
  fclose(fp_log);
  
  return EXIT_SUCCESS;
}
