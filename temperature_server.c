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
#include <time.h>

//NOTE USER SETTING
//0 for debugging when arduino not available
//1 for real use 
int use_arduino = 1;

const int TEMP_MULTIPLIER = 100; //because we can't use floats on Pebble

// The temperature state
float temp_curr = -999;
float temp_min = 99999;
float temp_max = -99999;
float temp_sum = 0;
float readings_count = 0;
int failed = 0;
int oops_count = 0;


char* msg = "";
    // char* msg1 = "c";
    // char* msg2 = "s";


//with the latest temperature reading... update stuff
void update_temp_stats(float temp){
  temp_curr = temp;
  if(temp < temp_min) temp_min = temp;
  if(temp > temp_max) temp_max = temp;
  temp_sum += temp;
  readings_count++;
}

//used by thread to read output from arduino, parse temperature, 
//and update global temperature variable
void* update_temp_from_arduino(void* a){

  char temp_buffer[10];
  int i = 0;
  int temp_length = 0;
  int continuing = 1;
  printf("Hello, attempting to read from Arduino!\n");
  
 

 

  //LOOP that is continuiously reading from arduino and writing to the screen
  strcpy(temp_buffer,"");
  char read_buffer[200];
  while(continuing){

  /////// OPEN the arduino device as a file

    int fd = open("/dev/cu.usbmodem1421", O_RDWR);
    if(fd==-1){
      if(oops_count==0){
        printf("OOPS!\n");
      }
      oops_count++;
      failed = 1;
      //exit(1);
    }
    else{
      oops_count =0;
    }



     ////// CONFIG options from instructions PDF
  struct termios options;
  // struct to hold options
  tcgetattr(fd, &options);
  // associate with this fd
  cfsetispeed(&options, 9600); // set input baud rate
  cfsetospeed(&options, 9600); // set output baud rate
  tcsetattr(fd, TCSANOW, &options); // set options




    int bytes_read = read(fd, read_buffer, 200);
    
    //so this has to be written to the device file now arduino has to read it


    int byte_written = write(fd, msg, strlen(msg));


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
            printf("Temp: %s \n",temp_buffer);          
            update_temp_stats(atof(temp_buffer));
          }
          strcpy(temp_buffer,""); //reset buffer
        }
      }
      //printf("%s", read_buffer);            
    }   
  }
  return 0; 
}

//alternative to get_temp
//used for when we have no android but still want to update
//update global temperature variable
void* update_temp_randomly(void* a){
  srand(time(NULL));
  int rand_bit;
  float temp_new;
  while(1==1){
    usleep(100000);
    rand_bit = rand() % 10;
    temp_new = temp_curr + (rand_bit - 4.5)*0.1;
    update_temp_stats(temp_new);
  }
  return 0;
}

//start listening for requests from the outside
//respond to all requests with the current temperature!
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

  // buffer to read data into
  char request[1024];
  char reply[600];
  int fd;
 
  ssize_t send_status;


  while(1==1){

    // 4. accept: wait here until we get a connection on that port
    int sin_size = sizeof(struct sockaddr_in);
    fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
    //printf("%s\n, %d\n", "******* here is the fd from pebble******",fd);
    printf("Server got a connection from (%s, %d)\n", 
      inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        
    // 5. recv: read incoming message into buffer
    int bytes_received = recv(fd,request,1024,0);
    // null-terminate the string
    request[bytes_received] = '\0';
    printf("Here comes the message:\n");
    printf("%s\n", request);
    
    char* a = strstr(request, "q=");
    printf("%c\n", *(a+2));

    if(*(a+2)=='1'){
        msg = "p";
    }
    if(*(a+2) =='2' ){
      msg = "c";
    }
    if(*(a+2) == '3'){
      msg = "";
    }
    //msg = "p";

    printf("%c\n", *msg);
   
    // if(*(a+2)=='0'){
    //   msg = "";
    // }
    // if(*(a+3)=='1'){
    //   msg = "c";
    // }
    
    // if(failed == 1){
    //   printf("an error occurred");
    //   int loc = 0;
    //   loc += sprintf(reply + loc, "{\n" );
    //   loc += sprintf(reply + loc, "\"Arduino_was_disconnected\": 1");
    //   loc += sprintf(reply + loc, "}\n" );
    //   failed = 0;
    // }
   
    printf("sending the temp now");
      //compose json reply
    if(failed ==1){  
    int loc = 0;
    loc += sprintf(reply + loc, "{\n" );
    loc += sprintf(reply + loc, "\"temp_curr_mult\": \"%s\",\n", "the arduino was disconnected" );
    loc += sprintf(reply + loc, "\"temp_max_mult\": \"%d\",\n", (int)(temp_max*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_min_mult\": \"%d\",\n", (int)(temp_min*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_avg_mult\": \"%d\"\n", (int)(temp_sum*TEMP_MULTIPLIER/readings_count) );
    loc += sprintf(reply + loc, "}\n" );
    failed = 0;

    }

    else{

    int loc = 0;
    loc += sprintf(reply + loc, "{\n" );
    loc += sprintf(reply + loc, "\"temp_curr_mult\": \"%d\",\n", (int)(temp_curr*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_max_mult\": \"%d\",\n", (int)(temp_max*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_min_mult\": \"%d\",\n", (int)(temp_min*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_avg_mult\": \"%d\"\n", (int)(temp_sum*TEMP_MULTIPLIER/readings_count) );
    loc += sprintf(reply + loc, "}\n" );
    }
    
    printf("Sending reply:\n%s", reply);
    // 6. send: send the message over the socket
    // note that the second argument is a char*, and the third is the number of chars


    send_status = send(fd, reply, strlen(reply), 0);

    
    //printf(reply);

    printf("Server send_status: %zd\n", send_status);
    close(fd);

  }


  // 7. close: close the socket connection
  close(sock);
  printf("Server closed connection\n");

  return 0;
} 


int main(int argc, char *argv[]){
  // check the number of arguments
  if (argc != 2){
    printf("\nUsage: server [port_number]\n");
    exit(0);
  }

  pthread_t temp_thread;
  if(use_arduino){
    pthread_create(&temp_thread, NULL, update_temp_from_arduino, NULL);     
  }
  else{
    printf("WARNING - you are not using your arduino!");
    update_temp_stats(30);
    pthread_create(&temp_thread, NULL, update_temp_randomly, NULL);         
  }

  int PORT_NUMBER = atoi(argv[1]);
  start_server(PORT_NUMBER);
}

