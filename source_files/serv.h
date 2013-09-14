#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

/********* CONSTANTS DECLARATION *******************/

#define MAX_SIZE 	1024
#define MAX_WAIT 	100000
#define NO_OF_PACKETS 	100
#define WINDOW_SIZE 	10
#define BUFFER_SIZE	20
#define PREVIOUS_WEIGHT 0.9
#define PRESENT_WEIGHT 	0.1
#define PORT_NO		5689
#define WIFI		1

/********* STRUCTURES DECLARATION ******************/

struct packet{
        int interface;
        int packet_number;
        char data [MAX_SIZE];
};

struct delay_params{
        unsigned long max;
        unsigned long min;
        unsigned long avg;
};

struct packet_params{
	unsigned long curr_time;
	unsigned long prev_time;
	unsigned long curr_diff;
	unsigned long prev_diff[WINDOW_SIZE];
	int curr_pkt_no;
	int prev_pkt_no;
	int pkt_index;
};

/********* FUNCTIONS DECLARATION *******************/

void time_difference_calc(unsigned long time_difference[]);

void * recieve_packets_thread_function();

unsigned long jitter_computation (struct packet_params*);

struct packet_params reset_packet_parama(struct packet_params);

void * process_enque_thread_function();

void drop_packet(int);
