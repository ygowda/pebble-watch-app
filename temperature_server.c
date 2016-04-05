#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

//NOTE USER SETTING
//0 for debugging when arduino not available
//1 for real use 
int use_arduino = 0;


// For temp-getting thread
float temp_num = 1;

//used by thread to read output from arduino, parse temperature, 
//and update global temperature variable
void* get_temp(void* a){

	char temp_buffer[10];
	int i = 0;
	int temp_length = 0;
	int continuing = 1;
	printf("Hello, attempting to read from Arduino!\n");

	/////// OPEN the arduino device as a file
	int fd = open("/dev/ttyACM0", O_RDWR);
	if(fd==-1){
		printf("OOPS!\n");
		exit(1);
	}

	////// CONFIG options from instructions PDF
	struct termios options;
	// struct to hold options
	tcgetattr(fd, &options);
	// associate with this fd
	cfsetispeed(&options, 9600); // set input baud rate
	cfsetospeed(&options, 9600); // set output baud rate
	tcsetattr(fd, TCSANOW, &options); // set options

	//LOOP that is continuiously reading from arduino and writing to the screen
	strcpy(temp_buffer,"");
	char read_buffer[200];
	while(continuing){
		int bytes_read = read(fd, read_buffer, 200);
		if(bytes_read > 0){
			read_buffer[bytes_read] = '\0';
			
			for (i = 0; i < bytes_read; i++){
				if ((read_buffer[i] >= '0' && read_buffer[i] <= '9') 
				|| (read_buffer[i] == '.')) {
					temp_length = strlen(temp_buffer);
					temp_buffer[temp_length] = read_buffer[i];
					temp_buffer[temp_length + 1] = '\0';
					
				} else if (read_buffer[i] == '\n') {
					
					if (strcmp(temp_buffer, "") > 0){
						temp_num = atof(temp_buffer);
						printf("Temp: %f \n",temp_num);					
					}
					strcpy(temp_buffer,""); //reset buffer
				}
			}
			//printf("%s", read_buffer);						
		}		
	}
	return 0;	
}


int start_server(int PORT_NUMBER){

  // structs to represent the server and client
  struct sockaddr_in server_addr,client_addr;    
  
  int sock; // socket descriptor

  // 1. socket: creates a socket descriptor that you later use to make other system calls
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
  	perror("Socket");
  	exit(1);
  }
  int temp;
  if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
  	perror("Setsockopt");
    exit(1);
  }

  // configure the server
  server_addr.sin_port = htons(PORT_NUMBER); // specify port number
  server_addr.sin_family = AF_INET;         
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  bzero(&(server_addr.sin_zero),8); 
  
  // 2. bind: use the socket and associate it with the port number
  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
    perror("Unable to bind");
    exit(1);
  }

  // 3. listen: indicates that we want to listn to the port to which we bound; second arg is number of allowed connections
  if (listen(sock, 5) == -1) {
    perror("Listen");
    exit(1);
  }
      
  // once you get here, the server is set up and about to start listening
  printf("\nServer configured to listen on port %d\n", PORT_NUMBER);
  fflush(stdout);
 

  // 4. accept: wait here until we get a connection on that port
  int sin_size = sizeof(struct sockaddr_in);
  int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
  printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
  
  // buffer to read data into
  char request[1024];
  
  // 5. recv: read incoming message into buffer
  int bytes_received = recv(fd,request,1024,0);
  // null-terminate the string
  request[bytes_received] = '\0';
  printf("Here comes the message:\n");
  printf("%s\n", request);


  
  // this is the message that we'll send back
  /* it actually looks like this:
    {
       "name": "cit595"
    }
  */
  
  
  char reply[100];
//  sprintf(reply,"{\n\"name\": \"%f\"\n}\n",temp_num);
  sprintf(reply,"{\n\"name\": \"%f\"\n}\n", 666.0); //DEBUG
  
  // 6. send: send the message over the socket
  // note that the second argument is a char*, and the third is the number of chars
  send(fd, reply, strlen(reply), 0);
  //printf("Server sent message: %s\n", reply);

  // 7. close: close the socket connection
  close(fd);
  close(sock);
  printf("Server closed connection\n");

  return 0;
} 


int main(int argc, char *argv[]){
  // check the number of arguments
  if (argc != 2)
    {
      printf("\nUsage: server [port_number]\n");
      exit(0);
    }

  if(use_arduino){
    pthread_t temp_thread;
    pthread_create(&temp_thread, NULL, get_temp, NULL);     
  }    
  else{
    printf("WARNING - you are not using your arduino!");
  }

  int PORT_NUMBER = atoi(argv[1]);
  start_server(PORT_NUMBER);
}




