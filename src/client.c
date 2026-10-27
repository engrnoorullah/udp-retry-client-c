/* Finite retries replace the original alarm/longjmp retry loop. */
#include "udp.h"

int main(int argc, char **argv) {
    if (argc<4 || argc>6) {
        fprintf(stderr,"Usage: udp_client IPV4 PORT MESSAGE [TIMEOUT_MS [ATTEMPTS]]\n"); return 1;
    }
    int port=integer(argv[2],1,65535);
    int timeout=argc>=5 ? integer(argv[4],1,60000) : 250;
    int attempts=argc==6 ? integer(argv[5],1,100) : 4;
    size_t size=strlen(argv[3]);
    if (port<0 || timeout<0 || attempts<0 || size>PAYLOAD_MAX) {
        fprintf(stderr,"invalid argument or message too long\n"); return 1;
    }
    struct sockaddr_in address;
    if (address_of(&address,argv[1],port)<0) { fprintf(stderr,"invalid IPv4 address\n"); return 1; }
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if (fd<0) { perror("socket"); return 1; }
    /* Connected UDP filters other peers; it does not establish a TCP connection. */
    if (connect(fd,(struct sockaddr *)&address,sizeof(address))<0) { perror("connect"); close(fd); return 1; }
    unsigned char packet[PACKET_MAX], reply[PACKET_MAX+1];
    int64_t start=milliseconds();
    if (start<0) { perror("clock_gettime"); close(fd); return 1; }
    uint32_t identifier=htonl((uint32_t)start ^ (uint32_t)getpid());
    memcpy(packet,&identifier,4);
    memcpy(packet+4,argv[3],size);
    for (int attempt=1;attempt<=attempts;++attempt) {
        ssize_t sent;
        do { sent=send(fd,packet,size+4,0); } while(sent<0 && errno==EINTR);
        if (sent!=(ssize_t)(size+4)) { perror("send"); close(fd); return 1; }
        int64_t now=milliseconds();
        if (now<0) { perror("clock_gettime"); close(fd); return 1; }
        int64_t deadline=now+timeout;
        for (;;) {
            now=milliseconds();
            if (now<0) { perror("clock_gettime"); close(fd); return 1; }
            if (now>=deadline) break;
            struct pollfd p={fd,POLLIN,0};
            int ready=poll(&p,1,(int)(deadline-now));
            if (ready<0 && errno==EINTR) continue;
            if (ready<0) { perror("poll"); close(fd); return 1; }
            if (ready==0) break;
            ssize_t n=recv(fd,reply,sizeof(reply),0);
            if (n<0 && errno==EINTR) continue;
            if (n<0) { perror("recv"); close(fd); return 1; }
            /* Validate both identifier and payload; unrelated replies do not extend the deadline. */
            if (n==(ssize_t)(size+4) && memcmp(packet,reply,size+4)==0) {
                if (fwrite(reply+4,1,size,stdout)!=size) { close(fd); return 1; }
                putchar('\n');
                fprintf(stderr,"received reply after %d attempt(s)\n",attempt);
                close(fd); return 0;
            }
        }
    }
    fprintf(stderr,"no matching reply after %d attempts\n",attempts);
    close(fd); return 2;
}
