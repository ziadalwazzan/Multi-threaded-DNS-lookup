/*
* Author: Ziad AlWazzan
* CU Boulder
* Date: July 18, 2021
*/
#include "multi-lookup.h"

#define ARRAY_SIZE 20
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTOR_THREADS 5
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN



/* requestor thread goals
1- lock input file and open then read through
2- lock buffer -> add hostname to buffer if not full -> unlock
3- at end of file -> close and unlock input file
*/
void * requestorThread(void* inputFiles){
	//fprintf(stderr, "inside requestor\n");
	int tid = syscall(SYS_gettid);
	struct req *requestors = (struct req *) inputFiles;
	int thread_count = 0;

	while(1){
		pthread_mutex_lock(requestors->input_lock);
		if (isEmpty(requestors->inputfs))
		{
			pthread_mutex_unlock(requestors->input_lock);
			break;
		}
		else
		{
			char * tmp = queue_pop(requestors->inputfs);
			FILE * input_f = fopen(tmp, "r"); // open/read the file
			free(tmp);
			thread_count++;

			if (!input_f){ //check if file not open
				perror("failed: ");
				//return 1;
				break;
			}
			pthread_mutex_unlock(requestors->input_lock);
	
			char hostnames[MAX_NAME_LENGTH]; // temp buffer to push to shared
	
			while (fgets(hostnames, sizeof(hostnames), input_f)){ //while there are lines to read put them in hostnames
				pthread_mutex_lock(requestors->buffer_lock);
	
				while(isFull(requestors->s_buffer)){ // while shared queue is full, wait
					pthread_cond_wait(requestors->condition, requestors->buffer_lock);
				}
				
				queue_push(requestors->s_buffer, strndup(strtok(hostnames, "\n"), strlen(hostnames)));
				pthread_mutex_unlock(requestors->buffer_lock);
			}
			fclose(input_f);
		}
	}
	pthread_mutex_lock(requestors->service_lock);
	fprintf(requestors->files_serviced, "Thread %d serviced %d files\n",tid, thread_count);
	pthread_mutex_unlock(requestors->service_lock);

	return 0;
}


/* resolver thread goals
1- enter loop that will continue if the buffer isn't empty and/or (thread count of requester > 0?)
2- lock buffer -> pop queue -> unlock buffer
3- dns lookup (this is where synchronization happens and multiple lookups can occur simultaneously)
4- on dns return lock output file and write results in it
5- exit once buffer is empty
*/
void * resolverThread(void* outputFile){
	//fprintf(stderr, "inside resolver\n");
	struct res *resolver = (struct res*) outputFile;
	
	while(1){
		pthread_mutex_lock(resolver->buffer_lock);
		if(!isEmpty(resolver->s_buffer)){ //while there is something in the buffer to pop
			char * hostname = queue_pop(resolver->s_buffer);
			char * firstipstr = malloc(MAX_IP_LENGTH);
			pthread_mutex_unlock(resolver->buffer_lock);
			pthread_cond_signal(resolver->condition);
			
			pthread_mutex_lock(resolver->output_lock);

			//fprintf(stderr, "checking dns: \n");
			if (dnslookup(hostname, firstipstr, MAX_IP_LENGTH) == UTIL_SUCCESS)
			{
				fprintf(stderr, "PASSED dns\n");
				fprintf(resolver->results, "%s, %s\n", hostname, firstipstr);
			}
			else
			{
				fprintf(stderr, "FAILED: %s\n", hostname);
				fprintf(resolver->results, "%s,\n", hostname);				
			}
			free(firstipstr);
			free(hostname);
			pthread_mutex_unlock(resolver->output_lock);
		}
		else{
			pthread_mutex_unlock(resolver->buffer_lock);
			break;
		}
	}
	//fprintf(stderr, "end of resolver\n");
	return 0;
}

int main(int argc, char* argv[]){
	struct timeval start_time;
	struct timeval end_time;
	gettimeofday(&start_time, NULL);

	if (argc < 5)
	{
		fprintf(stderr, "not enough arguments: %d\n", (argc-1));
	}
	if (argc > 5+ MAX_INPUT_FILES)
	{
		fprintf(stderr, "Too many arguments\n");
	}

	pthread_t req_threads[atoi(argv[1])];
	pthread_t res_threads[atoi(argv[2])];

	//initialize mutexs & cond vars
	pthread_mutex_t inp_lock;
	pthread_mutex_t out_lock;
	pthread_mutex_t buff_lock;
	pthread_mutex_t srvc_lock;
	pthread_cond_t cond;

	pthread_mutex_init(&inp_lock, NULL);
	pthread_mutex_init(&out_lock, NULL);
	pthread_mutex_init(&buff_lock, NULL);
	pthread_mutex_init(&srvc_lock, NULL);
	pthread_cond_init(&cond, NULL);

	//initialize structs
	struct req req_controller;
	struct res res_controller;

	req_controller.buffer_lock = &buff_lock;
	req_controller.input_lock = &inp_lock;
	req_controller.service_lock = &srvc_lock;
	req_controller.condition = &cond;

	res_controller.buffer_lock = &buff_lock;
	res_controller.output_lock = &out_lock;
	res_controller.condition = &cond;

	struct queue buffer;
	newQueue(&buffer, 20);
	req_controller.s_buffer = &buffer;
	res_controller.s_buffer = &buffer;


	//loop through files
	FILE *rslt = fopen(argv[4], "w+");;
	FILE *srvcd = fopen(argv[3], "w+");;
	res_controller.results = rslt;
	req_controller.files_serviced = srvcd;

	//struct queue files = NULL;
	struct queue files;
	newQueue(&files, MAX_INPUT_FILES);
	for (int i = 5; i < argc; i++)
	{
		if (access(argv[i], F_OK)==0)
		{
			queue_push(&files, strndup(argv[i], strlen(argv[i]))); //push filename into file queue;
		}
		else
			fprintf(stderr, "skipping file argument: %d\n", i-5);
	}
	req_controller.inputfs = &files;



	//create requestor threads
	for (int r = 0; r < atoi(argv[1]); r++)
	{
		if(pthread_create(&req_threads[r], NULL, requestorThread, &req_controller))
		{
			fprintf(stderr, "error creating requestor threads\n");
		}
	}


	//create resolver threads
	for (int s = 0; s < atoi(argv[2]); s++)
	{
		if(pthread_create(&res_threads[s], NULL, resolverThread, &res_controller))
		{
			fprintf(stderr, "error creating resolver threads\n");
		}
	}


	for (int k = 0; k < atoi(argv[1]); k++)
	{
		pthread_join(req_threads[k], NULL);
	}

	for (int h = 0; h < atoi(argv[2]); h++)
	{
		pthread_join(res_threads[h], NULL);
	}

	queue_free(&files);
	queue_free(&buffer);
	fclose(rslt);
	fclose(srvcd);

	gettimeofday(&end_time, NULL);
	printf("Total runtime: %f seconds.\n", (end_time.tv_sec - start_time.tv_sec)+((float)(end_time.tv_usec - start_time.tv_usec)/1000000));
	//printf("Total runtime: %ld seconds.\n", (end_time.tv_sec - start_time.tv_sec));
	//return 0;
}