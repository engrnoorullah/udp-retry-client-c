/* Cleaned edition of the UDP echo server in Noor Ullah's MSc coursework. */
#include "udp.h"

int main(int argc, char **argv) {
    if (argc<2 || argc>3) {
        fprintf(stderr,"Usage: udp_server PORT [DROP_FIRST_N]\n"); return 1;
    }
    int port=integer(argv[1],0,65535);
    int drops=argc==3 ? integer(argv[2],0,1000) : 0;
    if (port<0 || drops<0) { fprintf(stderr,"invalid port or drop count\n"); return 1; }
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if (fd<0) { perror("socket"); return 1; }
    struct sockaddr_in address;
    (void)address_of(&address,"127.0.0.1",port);
    if (bind(fd,(struct sockaddr *)&address,sizeof(address))<0) { perror("bind"); close(fd); return 1; }
    socklen_t length=sizeof(address);
    if (getsockname(fd,(struct sockaddr *)&address,&length)<0) { perror("getsockname"); close(fd); return 1; }
    printf("READY %u\n",(unsigned)ntohs(address.sin_port)); fflush(stdout);
    for (;;) {
        unsigned char packet[PACKET_MAX+1];
        struct sockaddr_in peer;
        socklen_t peer_length=sizeof(peer);
        ssize_t n=recvfrom(fd,packet,sizeof(packet),0,(struct sockaddr *)&peer,&peer_length);
        if (n<0) { if (errno==EINTR) continue; perror("recvfrom"); close(fd); return 1; }
        if (n<4 || n>PACKET_MAX) continue;
        if (drops>0) { --drops; continue; }
        ssize_t sent;
        do { sent=sendto(fd,packet,(size_t)n,0,(struct sockaddr *)&peer,peer_length); }
        while (sent<0 && errno==EINTR);
        if (sent!=n) perror("sendto");
    }
}
