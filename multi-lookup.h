#ifndef MULTILOOKUP_H
#define MULTILOOKUP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <semaphore.h>
#include "queue.h"
#include "util.h"

/*
struct filesruct{
	void ** fileptr;
	int next;
	int current;
	int sz;
	int * finished_arr;
}

struct req_controller{
	filesruct my_files;
	char * s_buffer;
	FILE * files_serviced;
	int * req_finished;

	// mutexes & cond vars
	pthread_mutex_t * buffer_lock;
	pthread_mutex_t * input_lock
	pthread_mutex_t * mutx
	pthread_cond_t * condition;
	pthread_cond_t * condition_file;
}

struct req_data{
	req_controller * address;
	int data;
}

struct res_controller{
	FILE * output_file;
	char * s_buffer;
	int * res_finished;

	// mutexes & cond vars
	pthread_mutex_t * buffer_lock;
	pthread_mutex_t * output_lock;
	pthread_mutex_t * mutx;
	pthread_cond_t * condition;
	pthread_cond_t * condition_file;
}
////////////////////////////////////////////////////////////


struct mutex{
	pthread_mutex_t buffer_lock;
	pthread_mutex_t output_lock;
	pthread_mutex_t mutx[10];
	pthread_mutex_t service_lock
	pthread_cond_t condition;
	pthread_cond_t condition_file;
}

struct resources{
	char s_buffer[20][buffer_ptr];
	char buffer_ptr[1025];
	char files_to_service[20];
	char files_serviced[20];
	char results[20];
	int buff_position;
	int num_files;
}

struct controller{
	char * thread_ptr; //thread array pointer
	struct resources vars;
	struct mutex mutx;
}
*/
struct req{
	struct queue * s_buffer;
	struct queue * inputfs;
	FILE * files_serviced;
	pthread_mutex_t * input_lock;
	pthread_mutex_t * buffer_lock;
	pthread_mutex_t * service_lock;
	pthread_cond_t * condition;
};

struct res{
	struct queue * s_buffer;
	FILE * results;
	pthread_mutex_t * output_lock;
	pthread_mutex_t * buffer_lock;
	pthread_cond_t * condition;
};

void* requesterThread(void * inputFile);

void* resolverThread(void * outputFile);

#endif