#ifndef UDP_DEMO_H
#define UDP_DEMO_H
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PAYLOAD_MAX 1000
#define PACKET_MAX (4 + PAYLOAD_MAX)

static inline int integer(const char *text, int minimum, int maximum) {
    char *end;
    errno=0;
    long value=strtol(text,&end,10);
    if (errno || !*text || *end || value<minimum || value>maximum) return -1;
    return (int)value;
}

static inline int64_t milliseconds(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC,&t)<0) return -1;
    return (int64_t)t.tv_sec*1000+t.tv_nsec/1000000;
}

static inline int address_of(struct sockaddr_in *address, const char *ip, int port) {
    memset(address,0,sizeof(*address));
    address->sin_family=AF_INET;
    address->sin_port=htons((uint16_t)port);
    return inet_pton(AF_INET,ip,&address->sin_addr)==1 ? 0 : -1;
}
#endif
