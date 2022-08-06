/* declare overrides before you include your headers. specifically iofunc.h */
struct Timeattr_s;
#define IOFUNC_ATTR_T struct Timeattr_s
struct Timeocb_s;
#define IOFUNC_OCB_T struct Timeocb_s

#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define NumDevices 3
#define HNow 0
#define HHour 1
#define HMin 2

IOFUNC_OCB_T* time_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *tattr);
void time_ocb_free(IOFUNC_OCB_T *tocb);

char *devnames[NumDevices] = {
		"/dev/time/now",
		"/dev/time/hour",
		"/dev/time/min"
};


typedef struct Timeattr_s {
	iofunc_attr_t attr;
	int device;
} Timeattr_t;


typedef struct Timeocb_s {
	iofunc_ocb_t ocb;
	//Here is were you can declare user variables.
	char buffer[50];
} Timeocb_t;



Timeattr_t timeattrs[NumDevices];

int io_read(resmgr_context_t *ctp, io_read_t *msg, Timeocb_t *tocb)
{
	int nb;
	time_t time_of_day = time(NULL);
	struct tm *tm = localtime(&time_of_day);

	//Check which device we have
	if (tocb->ocb.attr->device == HNow) {
		strcpy(tocb->buffer, ctime(&time_of_day));
	} else if (tocb->ocb.attr->device == HHour) {
		itoa(tm->tm_hour, tocb->buffer, 10);
	} else {
		itoa(tm->tm_min, tocb->buffer, 10);
	}

	//add one for null bit
	nb = strlen(tocb->buffer);

	//test to see if we have already sent the whole message.
	if (tocb->ocb.offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, tocb->buffer, nb);

	//update offset into our data used to determine start position for next read.
	tocb->ocb.offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		tocb->ocb.attr->attr.flags |= IOFUNC_ATTR_ATIME;

	return(_RESMGR_NPARTS(1));
}

int main(int argc, char *argv[]) {
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	dispatch_context_t   *ctp;
	int i;

	iofunc_funcs_t time_ocb_funcs = {
			_IOFUNC_NFUNCS,
			time_ocb_calloc,
			time_ocb_free
	};

	iofunc_mount_t time_mount = { 0, 0, 0, 0, &time_ocb_funcs };

	dpp = dispatch_create();
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	io_funcs.read = io_read;

	for(i = 0; i < NumDevices; i++) {
	   iofunc_attr_init (&timeattrs[i].attr, S_IFCHR | 0666, NULL, NULL);
	   timeattrs[i].device = i;
	   timeattrs[i].attr.mount = &time_mount;
	   resmgr_attach(dpp, NULL, devnames[i], _FTYPE_ANY, NULL, &connect_funcs, &io_funcs, &timeattrs[i]);
	}

	ctp = dispatch_context_alloc(dpp);
	while(1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS;
}

Timeocb_t * time_ocb_calloc(resmgr_context_t *ctp, Timeattr_t *tattr) {
	Timeocb_t *tocb;
	tocb = calloc(1, sizeof(Timeocb_t));
	tocb->ocb.offset = 0;
	return(tocb);
}

void time_ocb_free(Timeocb_t *tocb) {
	free(tocb);
}
