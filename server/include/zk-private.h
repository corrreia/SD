#ifndef _ZK_PRIVATE_H
#define _ZK_PRIVATE_H

#include <zookeeper/zookeeper.h>

typedef struct String_vector zoo_string;

struct zk_info {
	zhandle_t *zh; // zookeeper handle
    int is_connected; // 1 if connected to zookeeper, 0 otherwise

    char *identifier ; // this server identifier

    zoo_string *children; // children list

    char *zoo_root; // zookeeper root path
    char *zoo_path; // zookeeper path for this server  (REDUNDANT BUT USEFUL)
};

#endif
