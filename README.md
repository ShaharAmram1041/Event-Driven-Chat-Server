# Event-Driven-Chat-Server
Implementation of an event-driven chat server.<br>
Authored by Shahar Amram

## ==Description==

The program is an Event-Driven Chat Server:<br>
implement an event-driven chat server. The function of the chat is to forward each incoming message’s overall client connections except for the client connection over which the message was received.<br>


program contain 1 file:


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



## ==How to compile?==<br>
compile : gcc -g -Wall chatServer.c -o server<br>
run: ./server<br>

## ==Input==<br>
The input will be from the command line, ont number that represent the port to connect to.
and after each connection that will join with 'telnet' command.<br>

## ==Output==<br>
In the terminal you should see the chat between the connection that open, while the server terminal describe the chat flow.
* ### Error:<br>
If there is no port parameter, or more than one parameter, or the parameter is not a
number between 1 to 2^16, print "Usage: server <port>".<br>


For each failure, until ‘select’ is called,perror and exit the server.<br>

If select fails, exit the loop, clean the memory, and exit.

For any other error, skip the operation and continue.

