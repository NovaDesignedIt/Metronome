struct Metronome_attr_s;
#define IOFUNC_ATTR_T struct Metronome_attr_s
struct Metronome_ocb_s;
#define IOFUNC_OCB_T struct Metronome_ocb_s

#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>


#define METRONOME_PULSE_CODE  1
#define METRONOME_PAUSE_CODE 2
#define METRONOME_START_CODE 3
#define METRONOME_QUIT_CODE 4
#define METRONOME_SET_CODE 5
#define METRONOME_STOP_CODE 6

#define ON 0
#define OFF 1
#define START 2
#define STOP 3


#define NumDevices 2
#define metronome 0
#define metronome_help 1

#define BILLION  1000000000L

IOFUNC_OCB_T * metronome_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *tattr);
void metronome_ocb_free(IOFUNC_OCB_T *tocb);
void active_sleep(int seconds );

char *devnames[NumDevices] = {
		"/dev/local/metronome",
		"/dev/local/metronome-help"

};




char data[255];
int metronome_coid = 0;

int intervalTimer = ON;
int state = START ;

int bpm;
int time_sig_top;
int time_sig_bottom;
float interval;
long nanosec;
int pattern;
int index_str;
int steps;
int measure;
char * current_bpm_pattern;
int new_measure = 0;

char * a;
char *  b = "1000000000";

int l1 = 0;
int l2 =  10;
//----------------- managing a billionth of a second ------------------------------------
int i,j,tmp;

int ans[200]={0};
char s1[101],s2[101];
char * pend;


typedef struct Metronome_attr_s {
	iofunc_attr_t attr;
	int device;
} Metronome_attr_t;


typedef struct Metronome_ocb_s {
	iofunc_ocb_t ocb;
	//Here is were you can declare user variables.
	struct _pulse pulse;
	int interval;
	char buffer[50];
} Metronome_ocb_t;


Metronome_attr_t Metrattr_s[NumDevices];


int datatable[8][3] = {
			{2, 4, 4},
			{3, 4, 6},
			{4, 4, 8},
			{5, 4, 10},
			{3, 8, 6},
			{6, 8, 6},
			{9, 8, 9},
			{12, 8, 12}
	};

char * string_val [8] = {
			"1&2&",
			"1&2&3&",
			"1&2&3&4&",
			"1&2&3&4-5-",
			"1-2-3-",
			"1&a2&a",
			"1&a2&a3&a",
			"1&a2&a3&a4&a"

	};






IOFUNC_OCB_T * metronome_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *tattr);
void metronome_ocb_free(IOFUNC_OCB_T *tocb);




void  metronome_thread() {
  // create thread in main; thread runs this function
	// purpose: "drive" metronome
  // receives pulse from interval timer; each time the timer expires
  // receives pulses from io_write (quit and pause <int>)

  // Phase I - create a named channel to receive pulses
	struct sigevent         event;
	struct itimerspec       itime;
	timer_t                 timer_id;
	int                     chid;
	int                     rcvid;
	Metronome_ocb_t msg;
	struct sched_param      scheduling_params;
	int prio;


	name_attach_t * attach;
	// call name_attach() and register the device name:
	if ((attach = name_attach(NULL, "metronome", 0)) == NULL) {
		// exit FAILURE if name_attach() failed


		exit(EXIT_FAILURE);
	}




  // calculate the seconds-per-beat and nano seconds for the interval timer
  // create an interval timer to "drive" the metronome
  // 	configure the interval timer to send a pulse to channel at attach when it expires
//printf("---  %d  --   %d  --- ",time_sig_top, bpm);


	interval =  time_sig_top * 60 / bpm;
	//printf("float: %f",interval);
	nanosec = interval * 1000000000L;











	for(int i =0; i < 8; i++){
					if(datatable[i][0] == time_sig_top && datatable[i][1] == time_sig_bottom){
						pattern = datatable[i][2];
						index_str = i;
						steps = datatable[i][2];
					}
	}

	current_bpm_pattern = string_val[index_str];

	//printf("%s",current_bpm_pattern);
	fflush(stdout);

	/* Get our priority. */
	if (SchedGet( 0, 0, &scheduling_params) != -1)
	{
		prio = scheduling_params.sched_priority;
	}
	else
	{
		prio = 10;
	}

	fflush(stdout);
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
			attach->chid,
			_NTO_SIDE_CHANNEL, 0);
	event.sigev_priority = prio;
	event.sigev_code = METRONOME_PULSE_CODE;
	timer_create(CLOCK_MONOTONIC, &event, &timer_id);

/*
	 printf("\n\nsomething in set  %d bpm %d topsigtime %d bottomsigtime",bpm,time_sig_top,time_sig_bottom);

	 fflush(stdout);

	sprintf(a,"%d",bpm);


	l1 = strlen(a);

	for(i = l1-1,j=0;i>=0;i--,j++)
	    {
	        a[j] = s1[i]-'0';
	    }
	    for(i = l2-1,j=0;i>=0;i--,j++)
	    {
	        b[j] = s2[i]-'0';
	    }
	    for(i = 0;i < l2;i++)
	    {
	        for(j = 0;j < l1;j++)
	        {
	            ans[i+j] += b[i]*a[j];
	        }
	    }
	    for(i = 0;i < l1+l2;i++)
	    {
	        tmp = ans[i]/10;
	        ans[i] = ans[i]%10;
	        ans[i+1] = ans[i+1] + tmp;
	    }
	    for(i = l1+l2; i>= 0;i--)
	    {
	        if(ans[i] > 0)
	            break;
	    }

	    for(;i >= 0;i--)
	    {
	        printf("%d",ans[i]);
	    }



	    printf("Product : %ld ",strtol(ans,&pend,10));*/
	//-----------------------------------------------------

	fflush(stdout);
	itime.it_value.tv_sec = 0;
	/* 500 million nsecs = .5 secs */
	itime.it_value.tv_nsec = 500000000L;
	itime.it_interval.tv_sec = 0;
	/* 500 million nsecs = .5 secs */
	itime.it_interval.tv_nsec = 500000000L;
	timer_settime(timer_id, 0, &itime, NULL);



  // Phase II - receive pulses from interval timer OR io_write(pause, quit)
	measure=0;



	while (1){




	  rcvid = MsgReceivePulse(attach->chid, &msg.pulse, sizeof(Metronome_ocb_t),
	  				NULL);
	  		MsgReply(rcvid, EOK, NULL, 0);
	  		int current,stop;
				if ( rcvid == 0 ) {
				  switch( msg.pulse.code ) {
					case METRONOME_PULSE_CODE:
					  //display the beat to stdout
					if(state == START){
						if(measure == 0){

							printf("|%c",string_val[index_str][measure]);
							fflush(stdout);
							measure++;
						}else if(measure == steps ){


							printf("\n");
							fflush(stdout);
							measure = 0;


							if(new_measure == 1){

								interval =  time_sig_top * 60 / bpm;
								//printf("float: %lf",interval);
								nanosec = interval * 1000000000L;


							//	printf("---  %d  --   %d  --- ",time_sig_top, bpm);

								for(int i =0; i < 8; i++){
												if(datatable[i][0] == time_sig_top && datatable[i][1] == time_sig_bottom){
													pattern = datatable[i][2];
													index_str = i;
													steps = datatable[i][2];
													current_bpm_pattern = string_val[index_str];

													itime.it_value.tv_sec = 0;
													/* 500 million nsecs = .5 secs */
													itime.it_value.tv_nsec = nanosec;
													itime.it_interval.tv_sec = 0;
													/* 500 million nsecs = .5 secs */
													itime.it_interval.tv_nsec = nanosec;
													timer_settime(timer_id, 0, &itime, NULL);

														}
								}





								new_measure = 0;

							}



						}else{


						printf("%c",string_val[index_str][measure]);
						fflush(stdout);
						measure++;


						}

					}else if(state == STOP){

					}
						break;
				   case METRONOME_PAUSE_CODE :
						// pause the running timer for pause <int> seconds
					   active_sleep(msg.pulse.value.sival_int);
					   break;
				   case  METRONOME_QUIT_CODE :
					  // implement Phase III:

					//  delete interval timer
					   intervalTimer = OFF;
					//  call name_detach()
					   name_detach(attach, 0);
					//  call name_close()
					   name_close(attach->chid);
					//  exit with SUCCESS


					   exit(EXIT_SUCCESS);
					   return;
				   case METRONOME_START_CODE:
					   state = START;

					   break;
				   case METRONOME_STOP_CODE:
					   state = STOP;

				   	   break;
				   case METRONOME_SET_CODE :


					   if(state == STOP || measure == steps || measure == 0){

							interval =  time_sig_top * 60 / bpm;
							//printf("float: %lf",interval);
							nanosec = interval * 1000000000L;


						for(int i =0; i < 8; i++){
										if(datatable[i][0] == time_sig_top && datatable[i][1] == time_sig_bottom){
											pattern = datatable[i][2];
											index_str = i;
											steps = datatable[i][2];
											break;

												}
						}



					   }else {
						   new_measure = 1;
					   }
					   break;
				   default: break;
				  	}//end switch


				}//endif

  	  	  }//end for

}//end method.

int interval_t(long timer){
	   struct timespec tim, tim2;
	   tim.tv_sec = 1;
	   tim.tv_nsec = timer;

	   if(nanosleep(&tim , &tim2) < 0 )
	   {
	      printf("Nano sleep system call failed \n");
	      return -1;
	   }

	   printf(".");

	   return 0;
}




void active_sleep(int seconds ) {
	int   current = time(0);
	int    stop = current +  seconds;
						   //printf ("%d, %d\n\n", current, stop);
						   while (1)
						   {

						         current = time (0);
						         if (current >= stop)
						            break;
						   }
}


/* Functions */

/* override int io_read() function  */
int io_read(resmgr_context_t *ctp, io_read_t *msg, Metronome_ocb_t *tocb) {

	int nb;
	if (data == NULL) {
		return 0;
	}
	//TODO:  calculations for secs-per-beat, nanoSecs
	// beats / minute.
	float secs,nanosecs =0 ;


	secs =  60 / bpm;
	//output_interval = ( secs * time_sig_top) / time_sig_bottom ;
	nanosecs =  secs * 1000000000;
	if(tocb->ocb.attr->device == metronome ){
	sprintf(data, "[metronome: %d beats/min, time signature %d/%d, secs-per-beat: %.2f, nanoSecs: %d]\n",bpm,time_sig_top,time_sig_bottom,secs,nanosecs);
  	nb = strlen(data);
	}else if(tocb->ocb.attr->device == metronome_help){
		puts("\nMetronome Resource Manager\n(ResMgr) Usage: metronome <bpm> <ts-top> <ts-bottom>\n");
		puts("\nAPI:\npause [1-9] - pause the metronome for 1-9 seconds\nquit - quit the metronome");
		puts ("set <bpm> <ts-top> <ts-bottom> - set the metronome to <bpm> ts-top/ts-bottom");
		puts("start - start the metronome from stopped state");
		puts("stop - stop the metronome; use ‘start’ to resume");
	}

	//test to see if we have already sent the whole message.
	if (tocb->ocb.offset == nb){
		return 0;
	}


	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	tocb->ocb.offset  += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		tocb->ocb.attr->attr.flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}



/* override int io_write() function  */
int io_write(resmgr_context_t *ctp, io_write_t *msg, Metronome_ocb_t *tocb) {
	int nb = 0;

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		/* have all the data */
		char *buf;
			char *alert_msg;
			int i, small_integer;
			buf = (char *) (msg + 1);


		if (strstr(buf, "pause") != NULL) {
					for (i = 0; i < 2; i++) {
						alert_msg = strsep(&buf, " ");
					}
					small_integer = atoi(alert_msg);

					if (small_integer >= 1 && small_integer <= 9) {
						//FIXME :: replace getprio() with SchedGet()
						MsgSendPulse(metronome_coid, SchedGet(0, 0, NULL),
								METRONOME_PAUSE_CODE, small_integer);
					} else {
						printf("\nInteger is not between 1 and 99.\n");
					}
				} else if(strstr(buf, "set") != NULL) {
						int x,y,z=0;
						strsep(&buf," ");
						x = atoi(strsep(&buf," "));
						y = atoi(strsep(&buf," "));
						z = atoi(strsep(&buf," "));
					bpm =x;
					time_sig_top = y;
					time_sig_bottom = z;



					interval =  time_sig_top * 60 / bpm;
					nanosec = interval * 10000000000L;
/*//					printf("\n\n%l\n\n",nanosec);*/
					MsgSendPulse(metronome_coid, SchedGet(0, 0, NULL),
							METRONOME_SET_CODE, NULL);


				} else if(strstr(buf, "start") != NULL) {
					MsgSendPulse(metronome_coid, SchedGet(0, 0, NULL),
																		METRONOME_START_CODE, NULL);
				}else if(strstr(buf, "stop") != NULL) {
					MsgSendPulse(metronome_coid, SchedGet(0, 0, NULL),
																		METRONOME_STOP_CODE, NULL);
				} else if(strstr(buf, "quit") != NULL) {
					MsgSendPulse(metronome_coid, SchedGet(0, 0, NULL),
													METRONOME_QUIT_CODE, NULL);
				}else {
					perror("\nBad Usage use  - cat /dev/local/metronome");
					strcpy(data, buf);
				}
//



		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		tocb->ocb.attr->attr.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}





int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {


	if ((metronome_coid = name_open("metronome", 0)) == -1) {
		perror("name_open failed.");
		return EXIT_FAILURE;
	}

	return (iofunc_open_default(ctp, msg, handle, extra));
}






int main(int argc, char *argv[]) {
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;



	setvbuf (stdout, NULL, _IOLBF, 0);
	setvbuf (stdin, NULL, _IOLBF, 0);


	// verify number of command-line arguments == 4
	if (argc != 4) {
		printf("Bad Usage: incorrect number of arguments");
		//On Failure: print usage message and EXIT_FAILURE
		exit(EXIT_FAILURE);
	}

		iofunc_funcs_t metronome_ocb_func = {
				_IOFUNC_NFUNCS,
				metronome_ocb_calloc,
				metronome_ocb_free
		};

		iofunc_mount_t metronome_mount = { 0, 0, 0, 0, &metronome_ocb_func };


		//processing command line args
		bpm = atoi(argv[1]);
		time_sig_top = atoi(argv[2]);
		time_sig_bottom = atoi(argv[3]);

//		printf("bpm: %d tst: %d tsb: %d",bpm,time_sig_top,time_sig_bottom);
//		fflush(stdout);


		//_---__---_---__---_---__---_---__---_---__---_---__---_---__---_---__---//
		// 							OPEN THREADS;								  //
		//_---__---_---__---_---__---_---__---_---__---_---__---_---__---_---__---//


			pthread_attr_t attr;

			pthread_attr_init(&attr);

			pthread_create(NULL, &attr, &metronome_thread, NULL);
			pthread_attr_destroy(&attr);


		//_---__---_---__---_---__---_---__---_---__---_---__---_---__---_---__---//
		// 							CLOSE THREADS;								  //
		//_---__---_---__---_---__---_---__---_---__---_---__---_---__---_---__---//




	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		return (EXIT_FAILURE);
	}
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);


	for(int i = 0; i < NumDevices; i++) {
	   iofunc_attr_init (&Metrattr_s[i].attr, S_IFCHR | 0666, NULL, NULL);
	   Metrattr_s[i].device = i;
	   Metrattr_s[i].attr.mount = &metronome_mount;
	   resmgr_attach(dpp, NULL, devnames[i], _FTYPE_ANY, NULL, &connect_funcs, &io_funcs, &Metrattr_s[i]);
	}

	ctp = dispatch_context_alloc(dpp);
	while (1) {

		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS;
}


IOFUNC_OCB_T * metronome_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *tattr) {
	Metronome_ocb_t *tocb;
	tocb = calloc(1, sizeof(Metronome_ocb_t));
	tocb->ocb.offset = 0;
	return(tocb);
}

void metronome_ocb_free(IOFUNC_OCB_T *tocb) {
	free(tocb);
}



