#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include <netinet/in.h>

struct rtree_t{
    char *identifier;

    int socket;
    struct sockaddr_in server;
};

#endif