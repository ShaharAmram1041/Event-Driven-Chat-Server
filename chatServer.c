#include <signal.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "chatServer.h"
/* private functions: */
int checkThePort(char *);
void remove_msg(msg_t*);
void remove_mem(conn_pool_t* pool);

int arr[500];
static int end_server = 0;

/* clean everything and exit */
void intHandler(int SIG_INT) {
	/* use a flag to end_server to break the main loop */
    end_server = 1;
}

int main (int argc, char *argv[])
{
    signal(SIGINT, intHandler);

    /* Illegal port or illegal command */
    if(argc != 2 || checkThePort(argv[1])){
        printf("Usage: server <port>");
        exit(1);}

    int port = atoi(argv[1]);
	conn_pool_t* pool = malloc(sizeof(conn_pool_t));
    if(pool == NULL){
        perror("Malloc Fail!");
        exit(1);
    }
    /* initialize the pool */
	init_pool(pool);
   
	/*************************************************************/
	/* Create an AF_INET stream socket to receive incoming      */
	/* connections on                                            */
	/*************************************************************/
    /* create the welcome socket */
    int fd;
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        free(pool);
        exit(1);}

    struct sockaddr_in srv;
    srv.sin_family = AF_INET; /* use the Internet addr family */
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.sin_port = htons(port); /* bind socket ‘fd’ to command port */

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for      */
	/* the incoming connections will also be nonblocking since   */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
    int on = 1;
    int rc = ioctl(fd,(int)FIONBIO,(char*)&on);
    if(rc != 0){
        perror("ioctl");
        free(pool);
        close(fd);
        exit(1);
    }
	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
    if(bind(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        perror("bind");
        free(pool);
        close(fd);
        exit(1);}

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
    if(listen(fd, 5) < 0) {
        perror("listen");
        free(pool);
        close(fd);
        exit(1);}

	/*************************************************************/
	/* Initialize fd_sets  			                             */
	/*************************************************************/

    pool->maxfd = fd;
    int con_fd;
    FD_SET(fd,&pool->read_set);
    char buffer[BUFFER_SIZE];
    memset(arr,0,499);
    arr[0] = 1;
    arr[1] = 1;
    arr[2] = 1;

    /*************************************************************/
	/* Loop waiting for incoming connects, for incoming data or  */
	/* to write data, on any of the connected sockets.           */
	/*************************************************************/
	do
	{

		/**********************************************************/
		/* Copy the master fd_set over to the working fd_set.     */
		/**********************************************************/
        pool->ready_read_set = pool->read_set;
        pool->ready_write_set = pool->write_set;
        printf("Waiting on select()...\nMaxFd %d\n", pool->maxfd);

		/**********************************************************/
		/* Call select() 										  */
		/**********************************************************/
		/* return the ready descriptors */
		pool->nready = select(pool->maxfd + 1,&pool->ready_read_set,&pool->ready_write_set,0,0);
        if(pool->nready < 0)
            break;

		/**********************************************************/
		/* One or more descriptors are readable or writable.      */
		/* Need to determine which ones they are.                 */
		/**********************************************************/

		for (int sd = fd; sd < pool->maxfd + 1  ;sd++)
		{

            /* Each time a ready descriptor is found, one less has  */
			/* to be looked for.  This is being done so that we     */
			/* can stop looking at the working set once we have     */
			/* found all of the descriptors that were ready         */

			/*******************************************************/
			/* Check to see if this descriptor is ready for read   */
			/*******************************************************/
			if (FD_ISSET(sd,&pool->ready_read_set))
			{
				/***************************************************/
				/* A descriptor was found that was readable		   */
				/* if this is the listening socket, accept one      */
				/* incoming connection that is queued up on the     */
				/*  listening socket before we loop back and call   */
				/* select again. 						            */
				/****************************************************/
                if(sd == fd){
                    con_fd = accept(fd, (struct sockaddr *) NULL, NULL);
                    if(con_fd < 0){
                        perror("accept");
                        exit(1);
                    }
                    printf("New incoming connection on sd %d\n", con_fd);
                    /* add the connection to the queue */
                    if(add_conn(con_fd,pool) == -1)
                        continue;
                }
                /****************************************************/
				/* If this is not the listening socket, an 			*/
				/* existing connection must be readable				*/
				/* Receive incoming data his socket             */
				/****************************************************/
				else {
                    memset(buffer,0,BUFFER_SIZE);
                    printf("Descriptor %d is readable\n", sd);
                    ssize_t nBytes = read(sd, buffer, BUFFER_SIZE);

                    /* read fail */
                    if(nBytes < 0)
                        continue;

                    /* If the connection has been closed by client 		*/
                    /* remove the connection (remove_conn(...))    		*/
                    if(nBytes == 0){
                        printf("Connection closed for sd %d\n", sd);
                        if(remove_conn(sd, pool) == -1)
                            continue;
                    }
                    else{
                        printf("%d bytes received from sd %d\n", (int)nBytes, sd);

                    /**********************************************/
                    /* Data was received, add msg to all other    */
                    /* connectios					  			  */
                    /**********************************************/
                    if(add_msg(sd,buffer,(int)nBytes,pool) == -1)
                        continue;
                    }

                }
//                count++;
            } /* End of if (FD_ISSET()) */
			/*******************************************************/
			/* Check to see if this descriptor is ready for write  */
			/*******************************************************/
			if (FD_ISSET(sd,&pool->ready_write_set)) {
				/* try to write all msgs in queue to sd */
				if(write_to_client(sd,pool) == -1)
                    continue;
//                count++;
		 	}
		 /*******************************************************/
		 if(pool->nready == 0)
             break;


      } /* End of loop through selectable descriptors */

   } while (end_server == 0);

	/*************************************************************/
	/* If we are here, Control-C was typed,						 */
	/* clean up all open connections					         */
	/*************************************************************/
	/* remove allocated memory */
    remove_mem(pool);
    /* close welcome socket */
    close(fd);
    return 0;
}


int init_pool(conn_pool_t* pool) {
    /* Largest file descriptor in this pool. */
    pool->maxfd = 0;
    /* Number of ready descriptors returned by select. */
    pool->nready = 0;
    /* Set of all active descriptors for reading. */
    FD_ZERO(&pool->read_set);
    /* Subset of descriptors readies for reading. */
    FD_ZERO(&pool->ready_read_set);
    /* Set of all active descriptors for writing. */
    FD_ZERO(&pool->write_set);
    /* Subset of descriptors readies for writing.  */
    FD_ZERO(&pool->ready_write_set);
    /* Doubly-linked list of active client connection objects. */
    pool->conn_head = NULL;
    /* Number of active client connections. */
    pool->nr_conns = 0;
	return 0;
}



/*
	 * 1. allocate connection and init fields
	 * 2. add connection to pool
	 * */
int add_conn(int sd, conn_pool_t* pool) {
    if(pool == NULL || sd < 0){
        return -1;
    }
    conn_t* new_connection = (conn_t*) malloc(sizeof (conn_t));
    if(new_connection == NULL){
        perror("Malloc fail");
        exit(1);
    }
    /* initialize the fields */
    new_connection->fd = sd;
    new_connection->next = NULL;
    new_connection->prev = NULL;
    new_connection->write_msg_tail = NULL;
    new_connection->write_msg_head = NULL;

    /* the list of connection is empty */
    if(pool->conn_head == NULL)
        pool->conn_head = new_connection;

    /* the list of connection is not empty */
    else{
        conn_t *p = pool->conn_head;
        conn_t *p1 = NULL;
        while(p){
            p1 = p;
            p = p->next;
        }
        p1->next = new_connection;
        new_connection->prev = p1;
    }

    /* updating the descriptors number */
    for(int i = 0; i < 499; i++){
        if(arr[i] == 0){
            arr[i] = sd;
            break;}
    }
    int max = 3;
    for(int i = 0; i < 499; i++){
        if(arr[i] >= max)
            max = arr[i];
    }
    pool->maxfd = max;
    /* update the Number of active client connections. */
    pool->nr_conns++;
    FD_SET(sd,&pool->read_set);
    return 0;
}

/*
	* 1. remove connection from pool
	* 2. deallocate connection
	* 3. remove from sets
	* 4. update max_fd if needed
	*/
int remove_conn(int sd, conn_pool_t* pool) {
    if(sd < 0 || pool == NULL )
        return -1;
    conn_t *p = pool->conn_head;
    int flag = 0;
    /* head of the list */
    if(p != NULL && p->fd == sd){
        pool->conn_head = p->next;
        flag = 1;
    }
    if(flag == 0){
    conn_t *p1 = NULL;
    p = pool->conn_head;
    while(p != NULL && p->fd != sd) {
        p1 = p;
        p = p->next;
    }
    if(p == NULL)
        return -1;
    if(p1)
        p1->next = p->next;}
    remove_msg(p->write_msg_head);

/* updating the variables */
    pool->nr_conns--;
    FD_CLR(sd,&pool->read_set);
    FD_CLR(sd,&pool->read_set);

    int i;
    /* updating the descriptors number */
    for(i = 0; i < 499; i++){
        if(arr[i] == sd){
            arr[i] = 0;
            break;}
    }
    /* updating the max number */
    int max = 3;
    for(i = 0; i < 499; i++){
        if(arr[i] >= max)
            max = arr[i];
    }
    pool->maxfd = max;
    /* closing the file descriptor */
    close(sd);
    free(p);
    printf("removing connection with sd %d \n", sd);
    return 0;
}

/*
	 * 1. add msg_t to write queue of all other connections
	 * 2. set each fd to check if ready to write
	 */
int add_msg(int sd,char* buffer,int len,conn_pool_t* pool) {
    if(sd < 0 || buffer == NULL || len < 0 || pool == NULL)
        return -1;
    conn_t *p = pool->conn_head;
    while(p){
        /* add new message to other clients */
        if(sd != p->fd) {
            msg_t *msg = (msg_t *) malloc(sizeof(msg_t));
            if (msg == NULL) {
                perror("perror");
            }

            msg->message = (char *) malloc(sizeof(char) * (len + 1));
            strcpy(msg->message, buffer);
            msg->message[len] = '\0';

            /* add message to the head */
            if (p->write_msg_head == NULL) {
                p->write_msg_head = msg;
                p->write_msg_tail = msg;
                p->write_msg_head->next = NULL;
                p->write_msg_head->prev = NULL;
            } else {
                msg->prev = p->write_msg_tail;
                msg->next = NULL;
                p->write_msg_tail->next = msg;
                p->write_msg_tail = msg;
            }
            msg->size = len;
            /* update the write set of each client */
            FD_SET(p->fd,&pool->write_set);
        }
        p = p->next;
    }
    /* clean the descriptor from write set */
    FD_CLR(sd,&pool->write_set);
	return 0;
}

/*
	 * 1. write all msgs in queue
	 * 2. deallocate each writen msg
	 * 3. if all msgs were writen successfully, there is nothing else to write to this fd... */
int write_to_client(int sd,conn_pool_t* pool) {
	if(sd < 0 || pool == NULL)
        return -1;
    conn_t * p = pool->conn_head;
    ssize_t nBytes = 0;
    while(p){
        if(p->fd == sd){
            msg_t *msg = p->write_msg_head;
            if(msg == NULL){
                p = p->next;
                continue;
            }
            msg_t *msg1 = msg;
            while(msg){
            nBytes = write(sd,msg->message,msg->size);
            if(nBytes < 0){
                perror("write");
                exit(1);
            }
            msg1 = msg;
            msg = msg->next;
            free(msg1->message);
            free(msg1);}
            p->write_msg_head = p->write_msg_tail = NULL;
        }
        p = p->next;
    }
    FD_CLR(sd,&pool->write_set);
	return 0;
}
/* check if the port is bigger than 2^16 */
int checkThePort(char *port){
    int power = 1;
    int res = 0;
    int size = (int)(strlen(port));
    if( size > 6)
        return -1;
    for(int r = size- 1; r >= 0; r--){
        if (!(isdigit(port[r])))
            return -1;
        int sum = ((int)(port[r]) -48) * power;
        res += sum ;
        power *= 10;}
    if(res > 65536 || res < 1)
        return -1;
    return 0;
}
/* remove the message of each connection */
void remove_msg(msg_t* head){
    msg_t * p = head;
    while(p){
        free(p->message);
        p->message = NULL;
        p->size = 0;
        if(p->next == NULL)
            break;
        p = p->next;
    }
}
void remove_mem(conn_pool_t* pool){
    conn_t *p = pool->conn_head;
    conn_t *p1 = NULL;
    while(p != NULL){
        p1 = p->next;
        remove_conn(p->fd,pool);
        p = p1;
    }
    free(pool);
    pool = NULL;
}

