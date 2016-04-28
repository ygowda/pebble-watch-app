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
const char* PATH_ARDUINO = "/dev/ttyACM0";
//const char* PATH_ARDUINO = "/dev/cu.usbmodem1421";

const int TEMP_MULTIPLIER = 100; //because we can't use floats on Pebble


// The temperature stats state
const int MAX_READINGS_COUNT = 3600; //assuming 1 read every second 
float temp_curr = -999;
float temp_min = 99999;
float temp_max = -99999;
float temp_avg = 0;
int readings_count = 0; //how many temps are in the temp store
int temp_store_idx = 0; //most recent location that temp was stored
float temp_store[MAX_READINGS_COUNT];
///////

int failed = 0;
int oops_count = 0;

int wrap_idx(int idx){
  return (idx + 10*MAX_READINGS_COUNT)%MAX_READINGS_COUNT;
}

//with the latest temperature reading... update stuff
void update_temp_stats(float new_temp){
  //UPDATE TEMP STORE
  temp_store_idx = wrap_idx(temp_store_idx + 1);
  if(readings_count < MAX_READINGS_COUNT) readings_count += 1;
  temp_store[temp_store_idx] = new_temp;

  //UPDATE CUMULATIVE STATS
  temp_curr = temp_store[temp_store_idx];

  float temp_sum = 0;
  temp_min = 99999;
  temp_max = -99999;

  int this_idx;
  float this_temp;
  for(int i = 0; i< readings_count; i++){
    this_idx = wrap_idx(temp_store_idx -i);
    this_temp = temp_store[this_idx];
    if(this_temp < temp_min) temp_min = this_temp;
    if(this_temp > temp_max) temp_max = this_temp;
    temp_sum += this_temp;
  }

  temp_avg = temp_sum/readings_count;

  //printf("END update_temp_stats - new_temp:%f temp_min:%f temp_max:%f \n", new_temp, temp_min, temp_max);
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

    int fd = open(PATH_ARDUINO, O_RDWR);
    if(fd==-1){
      if(oops_count==0){
        printf("OOPS! Could not successfully open the Arduino connection.\n");
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
//    char* msg = "p";
//    char* msg1 = "ct";
    //so this has to be written to the device file now arduino has to read it


//    int byte_written = write(fd, msg1, strlen(msg1));


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
    printf("Server got a connection from (%s, %d)\n", 
      inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        
    // 5. recv: read incoming message into buffer
    int bytes_received = recv(fd,request,1024,0);
    // null-terminate the string
    request[bytes_received] = '\0';
    printf("Here comes the message:\n");
    printf("%s\n", request);
    
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
    loc += sprintf(reply + loc, "\"temp_avg_mult\": \"%d\"\n", (int)(temp_avg*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "}\n" );
    failed = 0;

    }

    else{

    int loc = 0;
    loc += sprintf(reply + loc, "{\n" );
    loc += sprintf(reply + loc, "\"temp_curr_mult\": \"%d\",\n", (int)(temp_curr*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_max_mult\": \"%d\",\n", (int)(temp_max*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_min_mult\": \"%d\",\n", (int)(temp_min*TEMP_MULTIPLIER) );
    loc += sprintf(reply + loc, "\"temp_avg_mult\": \"%d\"\n", (int)(temp_avg*TEMP_MULTIPLIER) );
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

