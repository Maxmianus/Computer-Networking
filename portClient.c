#include <stdlib.h>       // Needed for exit()
#include <string.h>       // Needed for memcpy() and strcpy()
#include <fcntl.h>        // Needed for file i/o stuff
#include <pthread.h>    // Needed for pthread_create() and pthread_exit()
#include <sys/stat.h>   // Needed for file i/o constants
#include <sys/types.h>  // Needed for sockets stuff
#include <netinet/in.h> // Needed for sockets stuff
#include <sys/socket.h> // Needed for sockets stuff
#include <arpa/inet.h>  // Needed for sockets stuff
#include <fcntl.h>      // Needed for sockets stuff
#include <netdb.h>      // Needed for sockets stuff
#include <unistd.h>     // Needed to avoid read and close warnings
#include <stdio.h>
#include <string.h>
#include <time.h>

#define PORT_ONE 2372   //Port one
#define PORT_TWO 8744   //Port two
#define IP_NUM "127.0.0.1"

int main()
{
    int i;
    int client;
    int addr_len;
    int retcode;
    int input = 1;
    int numbers_changed = 3;
    int fakeNumbers = 23748504;
    char temp_buf[9];
    char buf[9] = "104935843"; 
    char buf_two[9] = "043974852"; 
    struct sockaddr_in server_addr;
    
    //Creates socket
    client = socket(AF_INET, SOCK_DGRAM, 0);
    if(client < 0)
    {
        printf("ERROR: client socket failed\n");
        exit(-1);
    }

    srand(time(0));
    
    while(input!=0)
    {
        //alters the message to a different variant
        buf[numbers_changed] = 'A' + (rand() % 26);
        numbers_changed += rand() % 2;
        
        //Port one socket information  
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT_ONE);
        server_addr.sin_addr.s_addr = inet_addr(IP_NUM);
        
        //send message to port one
        retcode = sendto(client, buf, 10, 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
        if(retcode < 0)
        {
            printf("Failed to send");
            exit(-1);
        }
        
        //Outputs Packet Payloads
        printf("Packet One:\n");
        for(i=0; i<9; i++)
        {
            printf("%02X", buf[i]);
            printf("\n");
        }
        printf("\n");

         //Port two socket information
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT_TWO);
        server_addr.sin_addr.s_addr = inet_addr(IP_NUM);
        
        //alters message to a different variant
        buf_two[numbers_changed] = 'A' + (rand() % 26);
        numbers_changed += rand() % 3;
        
        //if index = 7, reset to 3 so does not overwrite
        if(numbers_changed > 6)
        {
            numbers_changed = 3;
        }
        
        //send message to port two
        retcode = sendto(client, buf_two, 10, 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
        if(retcode < 0)
        {
            printf("Failed to send");
            exit(-1);
        }
        
        //Outputs Packet Payloads
        printf("Packet Two:\n");
        for(i=0; i<9; i++)
        {
            printf("%02X", buf_two[i]);
            printf("\n");
        }
        
        printf("Enter 0 to exit, 1 to continue\n");
        scanf("%d", &input);
    }
    
    //Closes socket
    retcode = close(client);
    if(retcode < 0)
    {
        printf("ERROR: client socket failed to close\n");
        exit(-1);
    }    
    
    return 0;
}

