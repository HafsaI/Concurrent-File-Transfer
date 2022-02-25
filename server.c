#include<stdio.h>
#include<string.h>	
#include<stdlib.h>	
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h> 
#include <math.h>
#include <sys/stat.h>

struct thread_args {
    int *soc;
    FILE *fileptr;
    int chars_toread;
   
};

void *sendChunks(void *);
int main(int argc , char *argv[])
{
    struct thread_args *args = (struct thread_args *)malloc(sizeof(struct thread_args));

    int choice;
    printf("Do you want to send text, image or video file?\n");
    printf("Enter 1 for text, 2 for image and 3 for video:");
    scanf("%d", &choice);
    FILE *readptr;
    if (choice == 1){
        readptr= fopen("textfileSent.txt", "rb");
    }
    else if (choice == 2)
    {
       readptr = fopen("imageSent.jpg","rb");
    }
    else if (choice == 3)
    {
       readptr = fopen("vidSent.mp4","rb");
    }

   
    
	int socket_desc , new_socket , c,i ;
    int start = 0;
    int *new_sock;
	struct sockaddr_in server , client;

    fseek(readptr, 0, SEEK_END);
    int size = ftell(readptr);
    fseek(readptr, 0, SEEK_SET);

    args->fileptr = readptr;
    int num_of_threads;
    int read_chars = 0;
	

   
	// Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons( 8080 );
	
	// Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("[-]Bind failed");
		return 1;
	}
	puts("[+]Bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
   
   // ##
    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket<0)
    {
        perror("[-] Accept failed");
        return 1;
    }
    printf("[+] Connection accepted\n");
    // receiving threads
    if(recv(new_socket, &num_of_threads, sizeof(num_of_threads), 0)<=0)
    {
        printf("[-]Value not received.");
        exit(1);
    }
    printf("[+] Threads Number Received: %d.\n", num_of_threads);    
    close(new_socket);

    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket<0)
    {
        perror("[-] Accept failed");
        return 1;
    }
    printf("[+] Connection accepted\n");
    // sending file size
    if(send(new_socket, &size, sizeof(size), 0)== -1)
    {
        perror("[-] Error in sending count.\n");
        exit(1);
    }
	printf("[+] Filesize sent successfully: %d. \n", size);
    close(new_socket);

    // creating chunks
    int chunk = ceil((float) size / num_of_threads);
    pthread_t threads[num_of_threads];
    args -> chars_toread = chunk;
    for (int t = 0; t< num_of_threads;t++)
	{
        char buffer[chunk + 1];
		read_chars = fread(buffer, 1,chunk,readptr);
	}
    fseek(readptr, 0, SEEK_SET);


    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket<0)
    {
        perror("[-] Accept failed");
        return 1;
    }
    printf("[+] Connection accepted\n");
    // sending end chars
    if(send(new_socket, &read_chars, sizeof(read_chars), 0)== -1)
    {
        perror("[-] Error in sending end chars.\n");
        exit(1);
    }
    close(new_socket);

    // ##

    // multi threading
    for (i = 0; i < num_of_threads; i++){
        new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (new_socket<0)
        {
            perror("[-] Accept failed");
            return 1;
        }
        printf("[+] Connection accepted\n");
        
        new_sock = malloc(sizeof(int));
	    *new_sock = new_socket;
        args->soc = new_sock;
      

        if( pthread_create( &threads[i], NULL, sendChunks, args) < 0)
		{
			perror("[-] Could not create thread at server side.");
			return 1;
		}
       
        printf("[+] Server Thread %d created...\n", i);
        sleep(1);
    }
    for (int x = 0; x < num_of_threads; x++){
        pthread_join(threads[x], NULL); //waits for thread to end
    }	
    fclose(readptr);
   
	return 0;
}

void *sendChunks(void *args)
{  
    struct thread_args *threadArgs = (struct thread_args *)malloc(sizeof(struct thread_args));
    threadArgs = args;
    int *s = threadArgs->soc;
    int new_socket = *(int*)s;
    int chunksize = threadArgs -> chars_toread;
    FILE *readptr = threadArgs -> fileptr;
    char chunkBuffer[chunksize +1];

    int file_chars = fread(chunkBuffer, sizeof(char), chunksize, readptr);
    // sending file chunk to client
    if(send(new_socket, chunkBuffer, sizeof(chunkBuffer), 0)== -1)
    {
        perror("[-] Error in sending Buffer.\n");
        exit(1);
    }

    threadArgs -> fileptr = readptr;
    return 0;

   
    
    
    
 
}


