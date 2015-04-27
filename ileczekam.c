#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <endian.h>
#include <stdlib.h>

// returns difference between begin and end in microseconds
uint64_t time_difference(struct timeval* begin, struct timeval* end){
    if(end->tv_sec < begin->tv_sec || 
        (end->tv_sec == begin->tv_sec && end->tv_usec < begin->tv_usec)){
            return 0;
    }
    uint64_t difference;
    uint64_t usec_in_sec = 1000000;
    difference = end->tv_sec - begin->tv_sec;
    difference *= usec_in_sec;
    difference += end->tv_usec - begin->tv_usec;
    return difference;
}

int check_tcp(char* host, char* port){
    int sock;
    struct addrinfo addr_hints;
    struct addrinfo *addr_result = NULL;

    int err;
    
    // 'converting' host/port in string to struct addrinfo
    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = AF_INET; // IPv4
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = IPPROTO_TCP;
    err = getaddrinfo(host, port, &addr_hints, &addr_result);
    if(err != 0){
        printf("An error in getaddrinfo has occured.\n");
        return -1;
    }
    
    // initialize socket according to getaddrinfo results
    sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
    if(sock < 0){
        printf("A socket error has occured.\n");
        freeaddrinfo(addr_result);
        return -1;
    }
    
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    err = connect(sock, addr_result->ai_addr, addr_result->ai_addrlen);
    if(err == 0){
        gettimeofday(&end, NULL);
        uint64_t difference = time_difference(&begin, &end);
        printf("Time: %" PRIu64 "\n", difference);
    }else{
        printf("A connection error has occured.\n");
        freeaddrinfo(addr_result);
        return -1;
    }
    
    freeaddrinfo(addr_result);
    (void) close(sock);
    return 0;
}

int check_udp(char* host, char* port){
    int sock;
    struct addrinfo addr_hints;
    struct addrinfo *addr_result;

    size_t len;
    ssize_t snd_len, rcv_len;
    struct sockaddr_in my_address;
    struct sockaddr_in srvr_address;
    socklen_t rcva_len, s_rcva_len;

    // 'converting' host/port in string to struct addrinfo
    (void) memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = AF_INET; // IPv4
    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_protocol = IPPROTO_UDP;
    addr_hints.ai_flags = 0;
    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = NULL;
    addr_hints.ai_canonname = NULL;
    addr_hints.ai_next = NULL;
    if (getaddrinfo(host, NULL, &addr_hints, &addr_result) != 0) {
        printf("An error in getaddrinfo has occured\n");
        return -1;
    }

    my_address.sin_family = AF_INET; // IPv4
    my_address.sin_addr.s_addr =
        ((struct sockaddr_in*) (addr_result->ai_addr))->sin_addr.s_addr; // address IP
    my_address.sin_port = htons((uint16_t) atoi(port)); // port from the command line

    freeaddrinfo(addr_result);

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        printf("Socket error\n");
        return -1;
    }
    
    len = sizeof(uint64_t);
    rcva_len = (socklen_t) sizeof(my_address);
    s_rcva_len = (socklen_t) sizeof(srvr_address);
    uint64_t usec_received[2];
    uint64_t usec_in_sec = 1000000;
    uint64_t usec_end;
    struct timeval begin;
    gettimeofday(&begin, NULL);
    uint64_t usec_begin = begin.tv_sec * usec_in_sec + begin.tv_usec;
    uint64_t usec_begin_be = htobe64(usec_begin);
    
    snd_len = sendto(sock, &usec_begin_be, len, 0, 
        (struct sockaddr *) &my_address, rcva_len);
        
    if (snd_len != (ssize_t) len) {
        printf("partial / failed write\n");
        return -1;
    }
    
    rcv_len = recvfrom(sock, usec_received, sizeof(usec_received), 0,
        (struct sockaddr *) &srvr_address, &s_rcva_len);
    
    if (rcv_len < 0) {
        printf("read error\n");
        return -1;
    }
    usec_begin = be64toh(usec_received[0]);
    usec_end = be64toh(usec_received[1]);
    uint64_t difference = usec_end - usec_begin;
    printf("Time: %" PRIu64 "\n", difference);
    
    if (close(sock) == -1) {
        printf("close error\n");
        return -1;
    };

    return 0;
}

int main(int argc, char* argv[]){
    if (argc != 4){
        printf("Usage: %s mode host port\n", argv[0]);
        return 1;
    }
    else{
        if (strcmp(argv[1], "-t") == 0){
            check_tcp(argv[2], argv[3]);
        }
        else if(strcmp(argv[1], "-u") == 0){
            check_udp(argv[2], argv[3]);
        }
        else{
            printf("Error: unknown parameter %s", argv[1]);
            return 1;
        }
    }
    return 0;
}
