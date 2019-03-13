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
#include <time.h>
#include <sys/wait.h>


////// Struct definition for keeping track of clients   /////

struct client_info
{
    unsigned long stored_addr;           //Stored address of clients  
    char seq[2];                         //Used to check if valid or invalid knock
};


/////////   Globals and Prototypes  ///////////////

int child_flag = 0;                  //if child process is running or not, global
int running = 0;                     //true if the server is currently running
char old_passwords[1000][10];     //Where previous used passwords are stored
int update_client(struct client_info arr[], int num_clients, unsigned long ip, int sequence_num, char code, char password[]);


///// Port numbers for sockets  /////
#define PORT_NUM_ONE 2372
#define PORT_NUM_TWO 8744
#define IP_NUM "10.224.24.189"

int main()
{
    int                  server_s_one;           // Server socket descriptor for socket one
    struct sockaddr_in   server_one_addr;        // Server Internet address for socket one

    int                  server_s_two;       //Server socket descriptor for socket two
    struct sockaddr_in   server_two_addr;    // Server Internet address for socket two

    int                  addr_len;          // Client address length
    struct sockaddr_in   client_addr;        // Client Internet address
    struct in_addr       client_ip_addr;     // Client IP address, not used yet
    
    int num_clients = 10;       //max number of clients
    int retcode;
    char in_buf_one[4096];         //store message from client here, might only need one buffer
    int server_uptime = 10;    //set to 10 as initial time
    pid_t child = 1;
    clock_t start, end, timer;  //for timeout purposes

/////////// Initialize struct clients   /////////////
    int counter;
    int arr_cnt;
    
    struct client_info clients[num_clients];
    
    for(counter=0; counter<num_clients; counter++)
    {
        clients[counter].stored_addr = 0;
        clients[counter].seq[0] = '0';
        clients[counter].seq[1] = '0';
    }
    
    //Initialize array of strings
    for(arr_cnt=0; arr_cnt<1000; arr_cnt++)
    {
        strcpy(old_passwords[arr_cnt], "0");
    }

////////// Create Sockets /////////////////

    server_s_one = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_s_one < 0)
    {
        printf("ERROR: socket one failed\n");
        exit(-1);
    }

    server_s_two = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_s_two < 0)
    {
        printf("ERROR: socket two failed\n");
        exit(-1);
    }

/////// Set Non-blocking //////////////////////
    //Server One non blocking
    int flags = fcntl(server_s_one, F_GETFL, 0);
    fcntl(server_s_one, F_SETFL, flags | O_NONBLOCK);

    //Server two non blocking
    flags = fcntl(server_s_two, F_GETFL, 0);
    fcntl(server_s_two, F_SETFL, flags | O_NONBLOCK);

///////////////// Fill in socket information /////////////
    //Server one socket information
    server_one_addr.sin_family = AF_INET;
    server_one_addr.sin_port = htons(PORT_NUM_ONE);
    server_one_addr.sin_addr.s_addr = inet_addr(IP_NUM);
    retcode = bind(server_s_one, (struct sockaddr*)&server_one_addr, sizeof(server_one_addr));
    if(retcode < 0)
    {
        printf("ERROR: bind one failed\n");
        exit(-1);
    }

    //Server two socket information
    server_two_addr.sin_family = AF_INET;
    server_two_addr.sin_port = htons(PORT_NUM_TWO);
    server_two_addr.sin_addr.s_addr = inet_addr(IP_NUM);
    retcode = bind(server_s_two, (struct sockaddr*)&server_two_addr, sizeof(server_two_addr));
    if(retcode < 0)
    {
        printf("ERROR: bind two failed\n");
        exit(-1);
    }
    
/////////// START CLOCK AND PROGRAM ////////////////////
    
    printf("Starting...\n");
    while(1)
    {
        //check to see if need to execute process
        if(child_flag == 1 && running == 0)
        {
            running = 1;
            child = fork();
            start = clock(); //beginning of clock
        }
        
        if(child == 0)
        {
            printf("creating server...\n");
            char* argv[]={"weblite1", NULL};
            
            execv("weblite1", argv);
            exit(-1);
            //create server
        }
        else
        {   
            //Check socket one
            addr_len = sizeof(client_addr);
            retcode = recvfrom(server_s_one, in_buf_one, sizeof(in_buf_one), 0, (struct sockaddr *)&client_addr, &addr_len);
            if(retcode == 10) //change to == 10, null byte at end plus 9 digit password for number of bytes
            {
                server_uptime += update_client(clients, num_clients, client_addr.sin_addr.s_addr, (in_buf_one[0]-'0'), 'o', in_buf_one);  
                 printf("Packet One:\n");
                 
                int i;
                for(i=0; i<9; i++)
                {
                    printf("%02X", in_buf_one[i]);
                    printf("\n");
                }
                printf("\n");
            }


            //Check socket two
            addr_len = sizeof(client_addr);
            retcode = recvfrom(server_s_two, in_buf_one, sizeof(in_buf_one), 0, (struct sockaddr *)&client_addr, &addr_len);
            if(retcode == 10)
            {
                server_uptime += update_client(clients, num_clients, client_addr.sin_addr.s_addr, (in_buf_one[0]-'0'), 'g', in_buf_one);  
                
            }


            if(running == 1)
            {
                end = clock();
                timer = (end - start)/ CLOCKS_PER_SEC;
            }
            
            if(timer > server_uptime && running == 1)
            {
                kill(child, SIGKILL);   //kill child process when 10 seconds has passed
                child_flag = 0;
                running = 0;
                printf("Server Closed...\n");
                server_uptime = 10;
                
            }
            
        }
    }
    return 0;
}

//Parse string, [0] is seq, [1-4] is pin, [5-8] is clients port
int update_client(struct client_info arr[], int num_clients, unsigned long ip, int sequence_num, char code, char password[])
{
    int i =0;
    int x;
    
    //Checks if sequence_number is correct format
    if(sequence_num > 1)
    {  
        return 0;
    }
    
    //Check for previously used password
    for(x=0;x<1000;x++)
    {
        if(strcmp(password, old_passwords[x]) == 0)
        {
            return 0;
        }
        else if(strcmp("0", old_passwords[x]) == 0)
        {
            strcpy(old_passwords[x], password);
            break;
        }
    
    
    }
    
    //Iterates through clients
    for(x=0; x<num_clients; x++)
    {
        if(arr[x].stored_addr == ip)
        {
            arr[x].seq[sequence_num] = code;    //stores code in sequence order
            i++;
            break;
        }
        else if(arr[x].stored_addr == 0)
        {
            //if lockdown = 1 return 0
            
            arr[x].stored_addr = ip;            //stores ip
            arr[x].seq[sequence_num] = code;    //stores code in sequence order
            i++;
            break;
        }
    }
    
    //check sequence of char, if 'g''o' open server, if any '0' ignore, else clear
        if(arr[x].seq[0] == '0' || arr[x].seq[1] == '0')
        {
            //do nothing
        }
        else if (arr[x].seq[0] == 'g' && arr[x].seq[1] == 'o')
        {
            //valid port knock
            if(child_flag == 0 && running == 0)
            {
                child_flag = 1;
                //clears
                arr[x].seq[0] = '0';
                arr[x].seq[1] = '0';
            }
            else
            {
                //clears
                arr[x].seq[0] = '0';
                arr[x].seq[1] = '0';
                return 10;  //to increment timer
            }
        }

    return 0;
}



