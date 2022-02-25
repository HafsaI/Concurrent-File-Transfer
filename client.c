#include<stdio.h>
#include<string.h>	
#include<stdlib.h>	
#include<sys/socket.h>
#include<arpa/inet.h>	
#include<unistd.h>	
#include<pthread.h> 
#include <math.h>
#include <stdint.h>

struct thread_args {
    int *soc;
    FILE *fileptr;
    int chunklength;
    int end;
};



void *receiveChunks(void *);
void *receiveChunks2(void *);

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int main(int argc , char *argv[])
{
    struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));

    int choice;
    printf("Do you want to receive text, image or video file?\n");
    printf("Enter 1 for text, 2 for image and 3 for video:");
    scanf("%d", &choice);
    FILE * writefileptr;
    if (choice == 1){
        writefileptr = fopen("textfileRcv.txt", "wb");
    }
    else if (choice == 2)
    {
       writefileptr = fopen("imgReceived.jpg", "wb");
    }
    else if (choice == 3)
    {
       writefileptr = fopen("Receivedvid.mp4", "wb");
    }
    
    if(writefileptr == NULL) {
    printf("[-] Error Opening Video File"); } 
    
    // SETTING NUMBER OF THREADS HERE
    int num_threads;
    printf("Enter number of threads: 1/5/10? ");
    scanf("%d", &num_threads);
    pthread_t threads[num_threads];

  
    args -> fileptr = writefileptr;
	int socket_desc;
	struct sockaddr_in server;
    int i;
    int *new_sock;
    int file_chars; 
    int read_chars = 0;   

  



    //Prepare the sockaddr_in structure
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 8080 );

	
	// ##
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("[-] Could not create socket");
    }
    
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("[-] Connect error");
        return 1;
    }
    
    printf("[+] Connected\n");

    // sending threads
    if(send(socket_desc, &num_threads, sizeof(num_threads), 0)== -1)
    {
        perror("[-] Error in sending threads number.\n");
        exit(1);
    }
    printf("[+] Number of Threads Sent: %d \n", num_threads);
    close(socket_desc);
    
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("[-] Could not create socket");
    }
    
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("[-] Connect error");
        return 1;
    }
    
    printf("[+] Connected\n");

    // receiving file size
    if(recv(socket_desc, &file_chars, sizeof(file_chars), 0)<=0)
    {
        printf("[-] File size not received.");
        exit(1);
    }
    printf("[+] File Size Received: %d \n", file_chars);
    close(socket_desc);
    // ##

     socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("[-] Could not create socket");
    }
    
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("[-] Connect error");
        return 1;
    }
    
    printf("[+] Connected\n");

    // receiving read_chars
    if(recv(socket_desc, &read_chars, sizeof(read_chars), 0)<=0)
    {
        printf("[-] Readchars not received.");
        exit(1);
    }
    printf("[+] Read chars Received: %d \n", read_chars);
    close(socket_desc);
    //##
    // chunk length to send
    int chunkLength = ceil( (float) file_chars / num_threads);
    args -> chunklength = chunkLength;
   


    // multi threading
    for (i = 0; i < num_threads; i++){
        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc == -1)
        {
            printf("[-] Could not create socket");
        }
        
        if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
            puts("[-] Connect error");
            return 1;
        }
        
        printf("[+] Connected\n");
        
        new_sock = malloc(sizeof(int));
        *new_sock = socket_desc; 
        args->soc = new_sock; 
        args->end = read_chars; 

        // for end thread
        if (i == (num_threads -1)){
            if( pthread_create( &threads[i], NULL, receiveChunks2, args) < 0)
            {
                perror("[-] Could not create client thread");
                return 1;
            }
            printf("[+] Client Thread %d created...\n", i);

        }
        else{
            if( pthread_create( &threads[i], NULL, receiveChunks, args) < 0)
            {
                perror("[-] Could not create client thread");
                return 1;
            }
            printf("[+] Client Thread %d created...\n", i);


        }
        sleep(1);
      
      
    }
    // joining threads
    for (int x = 0; x < num_threads; x++){
        pthread_join(threads[x], NULL); 
    }	

    printf("[+] Data Written into File.\n");
    fclose(writefileptr);
	return 0;
}
void *receiveChunks(void *args )
{
    pthread_mutex_lock(&m);
    struct thread_args *threadArgs = (struct thread_args *)malloc(sizeof(struct thread_args));
    threadArgs = args;

    int *s = threadArgs->soc;
    int new_socket = *(int*)s;
    int chunk = threadArgs ->chunklength;
    FILE *writeptr = threadArgs -> fileptr;

    char buffer[chunk + 1];
   
    //receving file chunk from client
    if( recv(new_socket, buffer ,sizeof(buffer) , 0) < 0)
	{
		printf("[-] Receive failed. \n");
	}
   
    fwrite ( buffer , sizeof(char) ,chunk , writeptr );
    threadArgs -> fileptr = writeptr;
    pthread_mutex_unlock(&m);
    return 0;

    
}

// for end thread
void *receiveChunks2(void *args )
{
    pthread_mutex_lock(&m);
    struct thread_args *threadArgs = (struct thread_args *)malloc(sizeof(struct thread_args));
    threadArgs = args;
    int *s = threadArgs->soc;
    int new_socket = *(int*)s;
    int chunk = threadArgs ->chunklength;
    FILE *writeptr = threadArgs -> fileptr;
    int end = threadArgs -> end;
    char buffer[chunk + 1];
    if( recv(new_socket, buffer ,sizeof(buffer) , 0) < 0)
	{
		printf("[-] Receive failed. \n");
	}
    fwrite ( buffer , sizeof(char) ,end, writeptr );
    threadArgs -> fileptr = writeptr;
    pthread_mutex_unlock(&m);
    return 0;

    
}



