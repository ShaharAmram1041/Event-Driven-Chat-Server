# Event-Driven-Chat-Server
Implementation of an event-driven chat server.<br>
Authored by Shahar Amram

## ==Description==

The program is an Event-Driven Chat Server:<br>
implement an event-driven chat server. The function of the chat is to forward each incoming message’s overall client connections except for the client connection over which the message was received.<br>


program contain 1 file:


### ***threadpool.c***:

The pool is implemented by a queue. When the server gets a connection (getting back from accept()), it should put the connection in the queue. When there will be available thread (can be immediate), it will handle this connection (read request and write response).<br>


### ***chatServer.c***:<br>
The main file.implement an event-driven chat server. The function of the
chat is to forward each incoming message’s overall client connections except for the client connection over which the message was received.<br> opening the server socket, and after call “select” inside a loop to check from which socket descriptor is ready for reading or writing.<br>

## ==Functions==<br>
1.checkThePort - private function check the port size.<br>
2.remove_msg - private function to remove the message that allocated.<br>
3.remove_mem - private function to remove and free all the allocated memory in the profram.<br>
4.add_conn - function that add connection to the pool.<br>
5.remove_conn - function that remove connection from the pool.<br>
6.add_msg - function add msg_t to write queue of all the other connections.<br>
7.write_to_client - function write all msgs in the queue.<br>


***threadpool.c***:

1.threadpool* create_threadpool(int num_threads_in_pool) - function that creates and initalize the threads by the number of thread in pool.

2.dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg) -dispatch enter a "job" of type work_t into the queue. when an available thread takes a job from the queue, it will call the function "dispatch_to_here" with argument "arg".

3.do_work(void* p) - The work function of the thread.

4.destroy_threadpool(threadpool* destroyme) -  destroy_threadpool kills the threadpool, causing all threads in it to commit suicide, and then frees all the memory associated with the threadpool.


***server.c***:

1.main(int ,char**) - the main of the program. contain the implements of the server.

2.checkValidInput(int,char**) - private function check if the input from the user legal.

3.checkThePort(char*) - private function, if the port legal.

4.checkNeg(char*) - private function, check if the input is without negative numbers.

5.handler_function(void*) - private function, the function each thread will do after the connection is set.

6.checkNumberOfTokens(char*) - private function, check the token's numbers of the given input.

7.lastToken(char*) - private function, check if the last toke is HTTP version.

8.checkTheMethod(char*) - private function, check if the first token is 'GET' method.

9.checkThePath(char*) - private function, check if the path exists.

10.checkThePathDir(char*) - private function, check if the path is a directory and not end with '/'.

11.PermissionCheck(char*) - private function, check the permission of given path.

12.Response_Function(char* , char* , int ) - private function, send each of the type errors to create the response error.

13.get_mime_type(char*) - function, get the type of path file.

14.create_200_response(char*,char*,int,int) - private function, create the content file of the "200 ok" response.

15.create_response(char* ,char* ,char*,int,char*,int) - private function, generic. create for each type the exact response.



## ==Program Files==

threadpool.c - the file contain the excute of the threads, the work they do and the destroy of them.

server.c - the file conatin the execute of HTTP server.


## ==How to compile?==<br>
compile : gcc -g -Wall threadpool.c server.c -lpthread -o server<br>
run: ./server<br>

## ==Input==<br>
The input will be from the command line, 3 numbers represent the port, number of threads, maximum number of requests.<br>

## ==Output==<br>

1. in failure in the command line, print "Usage: server <port> <pool-size> <max-number-of-request>\n".
2. in failure after conncetion is set, return "500 Internal Server Error".
3. in success, return the responses by the given path. (in case of 200 ok, return the wanting file).

