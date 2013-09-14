#include "serv.h"

/*********** GLOBALS DECLARATION ************/

struct packet *in_seq_buffer[BUFFER_SIZE];
unsigned long wait_period;

main()
{
	pthread_t recv_pkts, process_enq;
	int iResult;

	iResult =
	    pthread_create(&recv_pkts, NULL,
			   (void *)recieve_packets_thread_function, NULL);
	if (iResult != 0) {
		printf("recv pkts thread not created : %s \n", strerror(errno));
	}
	pthread_join(recv_pkts, NULL);

	iResult =
	    pthread_create(&process_enq, NULL,
			   (void *)process_enque_thread_function, NULL);
	if (iResult != 0) {
		printf("process enque thread not created : %s \n",
		       strerror(errno));
	}
	exit(0);
}

void *recieve_packets_thread_function()
{
	int recv_sock, iResult, var_wifi, i, wifi_buff_index;
	char recvd_pkt[MAX_SIZE];
	socklen_t client_addr_len;
	struct packet *recvd;
	struct sockaddr_in client_addr, src_addr;
	struct packet_params *wifi = malloc(sizeof(struct packet_params));
	struct timeval tv;
	unsigned long present_time, interpolation_step_wifi;

	bzero(wifi, sizeof(struct packet_params));

	recv_sock = socket(AF_INET, SOCK_DGRAM, 0);

	printf("socket created %d\n", recv_sock);

	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	client_addr.sin_port = htons(PORT_NO);

	client_addr_len = sizeof(struct sockaddr_in);

	iResult =
	    bind(recv_sock, (struct sockaddr *)&client_addr,
		 sizeof(client_addr));
	if (iResult == -1) {
		printf("bind error1 : %s \n", strerror(errno));
	} else
		printf("bind successful to INADDR_ANY\n");

	while (1) {

		iResult =
		    recvfrom(recv_sock, recvd_pkt, MAX_SIZE, 0,
			     (struct sockaddr *)&src_addr, &client_addr_len);
		if (iResult == -1) {
			printf("recv from error: %s \n", strerror(errno));
		}

		gettimeofday(&tv, NULL);
		present_time = 1000000 * tv.tv_sec + tv.tv_usec;
		recvd = (struct packet *)recvd_pkt;

		if (recvd->interface == WIFI) {

			wifi->curr_time = present_time;
			wifi->curr_pkt_no = recvd->packet_number;
			wifi->curr_diff = wifi->curr_time - wifi->prev_time;

			wait_period = jitter_computation(wifi);

			/* Count number of missed packets */

			wifi->pkt_index = wifi->curr_pkt_no - wifi->prev_pkt_no;
			var_wifi = wifi->curr_pkt_no - wifi->prev_pkt_no - 1;
			wifi->pkt_index = wifi->pkt_index + var_wifi;
			wifi->prev_diff[wifi->pkt_index] = wifi->curr_diff;

			/* Interpolation of time for missed packets */

			if (var_wifi > 1) {

				interpolation_step_wifi =
				    wifi->curr_diff / var_wifi;
				wifi->pkt_index = wifi->pkt_index - var_wifi;

				for (i = 1; i <= var_wifi; i++) {

					wifi->prev_diff[wifi->pkt_index] =
					    wifi->prev_time +
					    interpolation_step_wifi;
					wifi->prev_time =
					    wifi->prev_diff[wifi->pkt_index];
				}
			}

			/*Store received in the sequnce buffer */

			wifi_buff_index = wifi->curr_pkt_no % BUFFER_SIZE;
			in_seq_buffer[wifi_buff_index] = recvd;
		}

		usleep(100);
	}
}

unsigned long jitter_computation(struct packet_params *packet_details)
{
	int i;
	unsigned long average = 0, jitter;

	for (i = 0; i < WINDOW_SIZE; i++) ;
	{

		average = average + packet_details->prev_diff[i];
	}

	average = average / WINDOW_SIZE;
	jitter = (0.9 * average) + (0.1 * packet_details->curr_diff);

	return (jitter);
}

void *process_enque_thread_function()
{
	int i = 0;
	while (1) {
		while (in_seq_buffer[i] != 0) {
			if (in_seq_buffer[i]->packet_number -
			    in_seq_buffer[i + 1]->packet_number == 1)
				drop_packet(i);

			else
				usleep(wait_period);
			i++;
		}
		usleep(1000);
	}

}

void drop_packet(int index)
{

	bzero((struct packet *)in_seq_buffer[index], sizeof(struct packet));
}
