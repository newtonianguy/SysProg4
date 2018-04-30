#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#define MAXRCVLEN 500
#define PORTNUM 2343

pthread_attr_t attr;
pthread_mutex_t lock;

typedef struct state_t{
	int client;
	char* pathName;
	int canOpen;
	int canWrite;
	int flags;
	struct state_t* next;
}state;

typedef struct node_t{
	int sock;
	uintmax_t now;
	struct node_t* next;
}node;

typedef struct queue_t{
	node *start;
	node *tail;
	char* pathName;
	struct queue_t* next;
}queue;

//initializes the thread attributes
pthread_t tnum=0;

//other global variables
queue *beg=NULL;
state *head=NULL;
int initialized = 0, initial = 0;


//for Implementation D. Checks the queue and takes away connections that are 2 sec old
//Checks every 3 sec.
static void sigHandler(int sig, siginfo_t *si, void *unused){
	queue *curre;
	node *curr, *prev;
	time_t result = time(NULL);
	//checks to see if there are any queues
	if(beg==NULL){
		return;
	}
	else{
		for(curre=beg; curre!=NULL; curre=curre->next){
			prev = curre->start;
			for(curr=curre->start; curr!=NULL; curr=curr->next){
				if((curr->now - result) > 120){
					prev->next = curr->next;
					curr->next = NULL;
					free(curr);
					curr = prev;
				}
				prev = curr;
			}
		}
		return;
	}
	return;
}

//for putting node in a queue
int enqueue(int socker, queue* q){
   node *end= malloc(sizeof(node));
	end->sock = socker;  
	end->next = NULL;  
   if(q->start==NULL){
       q->start=end;
   }
   else{
       q->tail->next=end;
   }
   q->tail=end;
   return 1;
}

//for taking a node off a queue
int dequeue(queue* q){
	 if( q->start!=NULL ){
	 	node* del = q->start;
	 	if(q->start == q->tail){
	 		q->start = NULL;
	 		q->tail = NULL;
	 	}
	 	else{
	 		q->start = q->start->next;
	 	}
	 	del->next = NULL;
	 	free(del);
	 	del = NULL;
	 	return 1;
	 }
    return 0;
}

//function passed to create thread
void *threadFunc(void* arg){//This is what the thread runs
	
	//sets up sigHandler for implementation D
	if(initial ==0 ){
		initial = 1;
		struct sigaction sa;
		sa.sa_flags = SA_SIGINFO;
		sigemptyset(&sa.sa_mask);
		sa.sa_sigaction = sigHandler;
		
		if (sigaction(SIGALRM, &sa, NULL) == -1)
		{
			printf("Fatal error setting up signal handler\n");
			exit(EXIT_FAILURE); //explode!!
		}
		timer_t timerid;
		timer_create(CLOCK_REALTIME, NULL, &timerid);	
		
		struct itimerspec value;
		value.it_interval.tv_sec = 3;
		value.it_interval.tv_nsec = 0;
		value.it_value.tv_sec = 3;
		value.it_value.tv_sec = 0;
		
		timer_settime(timerid, 0, &value, NULL);
	}
	
	//declares variables
	int len, *mysocket = (int*)arg, openCheck;
	int sock = *((int*)arg);//converts argument into socket connection
	int counter, part=0, start=0;
	state *curr, *prev;
	size_t length=0;
	char buffer[MAXRCVLEN];//for reading in message
	char *msg[3];//for deconstructing message into readable commands
	char *mess;//message that gets sent back to client. Will have return value and error value
	char *er;//error value
	char *re;//return value
	//reads in message
	len = recv(mysocket[0], buffer, MAXRCVLEN, 0);
	buffer[len] = '\0';
	
	//reads string client sent
	for(counter=0; counter<len;counter++){
		if(buffer[counter] == '\0'){
			break;
		}
		if(buffer[counter] == 44){//breaks apart message at commas
			if( part>=3 ){//makes sure too many parameters weren't sent by the client
				printf("Error:Too many parameters were sent by the client.\nCommand message should have three parameters.\nCommas separate paramaters.\n");
			}
			printf("Found break in the message\n");
			msg[part] = (char*) malloc(length+1);
			strncpy(msg[part], &buffer[start], length);
			part++;
			start = counter + 1;
			length = 0;
			continue;
		}
		length++;
	}
	msg[part] = (char*) malloc(length+1);
	strncpy(msg[part], &buffer[start], length);
	
	//checks to see if this is a close message
	if(part==0){
		//calls close and gets return and error messages
		printf("Close Called\n");
		int fd = atoi(msg[0]);
		int ret;
		ret = close(fd);
		if(ret <= 0){
				re = "0";
		}
		else{
				re = (char*) malloc((int)((ceil(log10(ret))+1)*sizeof(char)) + 1);
				sprintf(re, "%d", ret);
		}
		if(errno <= 0){
				er = "0";
		}
		else{
			er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
			sprintf(er, "%d", errno);
		}
		
		//sends back message to client
		mess = (char*) malloc(sizeof(er) + sizeof(re) + 2);
		strcat(mess, re);
		strcat(mess, ",");
		strcat(mess, er);
		strcat(mess, "\0");
		send(sock, mess, strlen(mess), 0);
		
		//removes client from linked list of opened files
		prev = head;
		for(curr=head; curr!=NULL; curr=curr->next){
			if(fd == curr->client){
				//checks to see if it is the head ptr
				if( curr==head ){
					if( curr->next==NULL ){//checks if head is beginning and end of list
						curr->next = NULL;
						free(curr);
						head=NULL;
						initialized = 0;
						break;
					}
					head = curr->next;
					curr->next = NULL;
					free(curr);
					break;
				}
				else if( curr->next==NULL ){//checks to see if end of list
					prev->next=NULL;
					free(curr);
					break;
				}
				//in middle of two nodes
				prev->next = curr->next;
				curr->next = NULL;
				free(curr);
				break;
			}
			prev = curr;
		}
		return NULL;
	}
	
	//checks to see if command message was sent correctly
	if( part < 2 ){
		printf("Error:Too few parameters were sent by the client.\nCommand message should have three parameters.\nCommas separate paramaters.\n");
	}
	//interprets message and performs commands
	openCheck = atoi(msg[0]);
	if(openCheck != 0){//this means the message is a "read" or "write" command
		int bytes;
		int size = atoi(msg[2]);//size of read or write
		//char buff[size];
		//checks for "Large File Transfers"
		if( size>4000 ){
			printf("Large File Transfer\n");
			int segs=1, oc=size;
			while(size>4000 && segs<=10){
				size = oc/segs;
				segs++;
			}
			//checks to see if file is too large
			if(size>4000 && size==10){
				char *msg = "-1,    ,110";
				send(sock, msg, strlen(msg), 0);
				return NULL;
			}
			char *segs1, *segs2;
			segs1 = (char*) malloc((int)((ceil(log10(size))+1)*sizeof(char)));
			sprintf(segs1, "%d", size);
			segs2 = (char*) malloc((int)((ceil(log10(oc-(segs*size)))+1)*sizeof(char)));
			sprintf(segs2, "%d", (oc-(segs*size)));
			
			//makes response message
			int pNum;
			char *pNums;
			mess = (char*) malloc((strlen(segs1)*segs) + strlen(segs2) + (segs*2));
			for(counter=0; counter<segs-1; counter++){
				pNum = PORTNUM + counter;
				pNums = (char*) malloc((int)((ceil(log10(pNum))+1)*sizeof(char)));
				sprintf(pNums, "%d", pNum);
				strcat(mess, pNums);
				strcat(mess, ",");
				strcat(mess, segs1);
				strcat(mess, ",");
				free(pNums);
			}
			pNum = PORTNUM + counter;
			pNums = (char*) malloc((int)((ceil(log10(pNum))+1)*sizeof(char)));
			sprintf(pNums, "%d", pNum);
			strcat(mess, pNums);
			strcat(mess, ",");
			strcat(mess, segs2);
			strcat(mess, "\0");
			free(pNums);
			
			//sends response
			send(sock, mess, strlen(mess), 0);
			
			//receives connection to different sockets
			struct sockaddr_in dest;//socket info about the machine connecting to us
			struct sockaddr_in serv;//socket info about the server
			int mySocket;//socket used for listening for incoming connections
			socklen_t socksize;
			int conSocket;
			pthread_attr_init(&attr);
			for(counter=0; counter<segs; counter++){
				
				//sets up server 
				socksize = sizeof(struct sockaddr_in);
				
				memset(&serv, 0, sizeof(serv));//zero the struct before filling the fields
				serv.sin_family = AF_INET;//set the type of connection to TCP/IP
				serv.sin_addr.s_addr = htonl(INADDR_ANY);//set our address to any interface
				serv.sin_port = htons(PORTNUM+counter);//set the server port number
				
				mySocket = socket(AF_INET, SOCK_STREAM, 0);
	
				bind(mySocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));//bind server info to mysocket
	
				//starts listening for connections to queue
				listen(mySocket, 1);
				conSocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);
				pthread_create(&tnum, &attr, &threadFunc, (void*) &conSocket);//creates thread
				tnum++;
			}
			
			return NULL;
		}
		else if(strcmp(msg[1], "read\0") == 0){//checks to see if it is a "read" command
			printf("Read Called\n");
			char buff[size];
			bytes = read(openCheck, buff, size);
			if(bytes <= 0){
				re = "0";
			}
			else{
				re = (char*) malloc((int)((ceil(log10(bytes))+1)*sizeof(char)) + 1);
				sprintf(re, "%d", bytes);
			}
			if(errno <= 0){
				er = "0";
			}
			else{
				er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
				sprintf(er, "%d", errno);
			}
			
			//makes response message
			mess = (char*) malloc(sizeof(er) + sizeof(re) + size + 3);
			strcat(mess, re);
			strcat(mess, ",");
			strcat(mess,buff);
			strcat(mess, ",");
			strcat(mess, er);
			strcat(mess, "\0");
			send(sock, mess, strlen(mess), 0);
		}
		else if(strcmp(msg[1], "write\0") == 0){//checks to see if this is a write command
			printf("Write Called\n");
			bytes = write(openCheck, msg[2], size);
			if(bytes <= 0){
				re = "0";
			}
			else{
				re = (char*) malloc((int)((ceil(log10(bytes))+1)*sizeof(char)) + 1);
				sprintf(re, "%d", bytes);
			}
			if(errno <= 0){
				er = "0";
			}
			else{
				er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
				sprintf(er, "%d", errno);
			}
			
			//makes response message
			mess = (char*) malloc(sizeof(er) + sizeof(re) + 2);
			strcat(mess, re);
			strcat(mess, ",");
			strcat(mess, er);
			strcat(mess, "\0");
			send(sock, mess, strlen(mess), 0);
		}
		else{
			printf("Error:Client did not send an identifiable command to server\n");
			return NULL;
		}
	}
	else{//otherwise the command is an "open" command
		printf("Open Called\n");
		int retur, found=0;
		int flags = atoi(msg[2]);
		if(initialized == 1){
			//checks open permissions for specified file
			prev = head;
			for(curr=head; curr!=NULL; curr=curr->next){
				if(strcmp(msg[1], curr->pathName)==0){
					found = 1;
					//checks to see if file is allowed to be opened in this mode
					if( curr->canOpen==1 && strcmp(msg[0],"transaction")!=0 && (curr->flags==0 || strcmp(msg[0],"exclusive")!=0) && 
					(curr->canWrite!=0 || flags==0) ){
						//only allows to be opened if 
						//1) file can be opened
						//2) the requested mode is not transaction
						//3) the file can be written to and wants to be opened in exclusive
						//4) the file is in exclusive and wants to be written to
						if( curr->next->pathName != curr->pathName ){
							found=1;
							break;
						}
						continue;//checks for different file descriptors
					}
					else{//couldn't open file
						printf("Couldn't open file\n");
						//search for correct queue
						queue* curre;
						if(beg==NULL){
							beg = (queue*) malloc(sizeof(queue));
							beg->pathName = msg[1];
							beg->start = NULL;
							beg->tail = NULL;
							beg->next = NULL;
						}
						else{
							for(curre=beg; curre!=NULL; curre=curre->next){
								if(curre->pathName==msg[1]){
									break;
								}
							}
						}
						enqueue(sock,curre);
						//check is socket is in front and meets other requirments 
						while(!(curr->canOpen==1 && strcmp(msg[0],"transaction")!=0 && (curr->flags==0 || strcmp(msg[0],"exclusive")!=0) && 
					(curr->canWrite!=0 || flags==0)&& (curre->start->sock == sock))){
							sched_yield();
						}
						dequeue(curre);
						break;
					}
				}
				prev = curr;
			}		
		}
		
		//calls open
		retur = open(msg[1],flags);

		//adds to global state
		state *meh = (state*) malloc(sizeof(state));
		meh->pathName = (char*) malloc(strlen(msg[1])+1);
		strcpy(meh->pathName, msg[1]);
		meh->client = retur;
		meh->flags = flags;
		meh->next=NULL;
		//checks mode
		if( strcmp(msg[0],"unrestricted")==0 ){
			meh->canOpen = 1;
			meh->canWrite = 1;
		}
		else if( strcmp(msg[0],"exclusive")==0 ){
			meh->canOpen = 1;
			meh->canWrite = 0;
		}
		else if( strcmp(msg[0],"transaction")==0 ){
			meh->canOpen = 0;
			meh->canWrite = 0;
		}
		
		//adds node to list and makes a queue for the file
		if( found==0 ){
			//makes first node in list
			if(initialized == 0){
				initialized = 1;
				head = meh;
			}
			else{
				meh->next = head;
				head = meh;
			}
		}
		else{
			meh->next = curr;
			if(prev==head){
				head = meh;
			}
			else{
				prev->next = meh;
			}
		}
		
		//sets return message
		if(retur <= 0){
			re = "-1";
		}
		else{
			re = (char*) malloc((int)((ceil(log10(retur))+1)*sizeof(char)));
			sprintf(re, "%d", retur);
		}
	
		if(errno <= 0){
			er = "0";
		}
		else{
			er = (char*) malloc((int)((ceil(log10(errno))+1)*sizeof(char)));
			sprintf(er, "%d", errno);
		}
		//makes response message
		mess = (char*) malloc(sizeof(er) + sizeof(re) + 2);
		strcat(mess, re);
		strcat(mess, ",");
		strcat(mess, er);
		strcat(mess, "\0");
		send(sock, mess, strlen(mess), 0);
	}
	return NULL;
}

int main(int argc, char *argv[]){
	//signal(SIGALRM, sigHandler);

	//sets pthread variables
	pthread_attr_init(&attr);
	
	//sets up server 
	struct sockaddr_in dest;//socket info about the machine connecting to us
	struct sockaddr_in serv;//socket info about the server
	int mySocket;//socket used for listening for incoming connections
	socklen_t socksize = sizeof(struct sockaddr_in);
	
	memset(&serv, 0, sizeof(serv));//zero the struct before filling the fields
	serv.sin_family = AF_INET;//set the type of connection to TCP/IP
	serv.sin_addr.s_addr = htonl(INADDR_ANY);//set our address to any interface
	serv.sin_port = htons(PORTNUM);//set the server port number
	
	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	
	bind(mySocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));//bind server info to mysocket
	
	//starts listening for connections to queue
	listen(mySocket, 10);
	int conSocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);
	pthread_create(&tnum, &attr, &threadFunc, (void*) &conSocket);//creates thread
	tnum++;
	while(conSocket){
		printf("Incoming connection from %s - sending welcome\n",inet_ntoa(dest.sin_addr));
		conSocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);//accepts new connection
		pthread_create(&tnum,&attr,&threadFunc,(void*) &conSocket);//creates thread for new connection
		tnum++;
	}
	pthread_attr_destroy(&attr);//destroys unecesarry attributes
	close(conSocket);
	close(mySocket);
	return EXIT_SUCCESS;
}