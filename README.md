# Concurrent-File-Transfer
Socket programming and Multi-threading in C using TCP/IP Protocols
#Protocol
TCP (Transmission Control Protocol) was used; a communications standard that enables application
programs and computing devices to exchange messages over a network. It is designed to send packets
across the internet and ensure the successful delivery of data and messages over networks.

## Logic
Socket programming in c language and multi threading to ensure concurrent transfer of files.
Server sends the file size that it is sending to the client. Client sends n threads for file transfer. Server
opens file to read, creates n threads and sends the read file in n chunks to client. Client also creates n
threads and receives those chunks in n threads and writes each chunk to file.

## How to Compile

    Client: gcc -o c.o client.c -lpthread -lm
    Server: gcc server.c -lpthread -lm
Math.h and pthread.h are not a part of the standard C library, so had to link to it using -lpthread and -lm


