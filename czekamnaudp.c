#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <endian.h>


int main(int argc, char *argv[]){
    
    if(argc != 2){
        printf("Usage: %s port", argv[0]);
        return 1;
    }
    
	int sock;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	socklen_t snda_len, rcva_len;
	ssize_t len, snd_len;

	sock = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
	if (sock < 0){
        printf("socket error\n");
        return -1;
    }
	// after socket() call; we should close(sock) on any execution path;
	// since all execution paths exit immediately, sock would be closed when program terminates

	server_address.sin_family = AF_INET; // IPv4
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
	server_address.sin_port = htons(atoi(argv[1])); // default port for receiving is argv[1]

	// bind the socket to a concrete address
	if (bind(sock, (struct sockaddr *) &server_address,
			(socklen_t) sizeof(server_address)) < 0){
        printf("bind error\n");
        return -1;
    }

	snda_len = (socklen_t) sizeof(client_address);
    rcva_len = (socklen_t) sizeof(client_address);
    uint64_t usec_in_sec = 1000000;
    uint64_t usec_received;
    struct timeval now;
    uint64_t usec_send[2];
    
    do{
        len = recvfrom(sock, &usec_received, sizeof(usec_received), 0,
                    (struct sockaddr *) &client_address, &rcva_len);
        if (len < 0){
            printf("error on datagram from client socket\n");
            return -1;
        }
        gettimeofday(&now, NULL);
        uint64_t usec_now = now.tv_sec * usec_in_sec + now.tv_usec;
        usec_send[0] = usec_received;
        usec_send[1] = htobe64(usec_now);
        snd_len = sendto(sock, usec_send, sizeof(usec_send), 0,
                (struct sockaddr *) &client_address, snda_len);
        printf("Received %" PRIu64 " and sent it back with %" PRIu64 " .\n", be64toh(usec_received), usec_now);
        if (snd_len < 0){
            printf("error on sending datagram to client socket\n");
            return -1;
        }
    } while (len > 0);

    return 0;
}
