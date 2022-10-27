#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include "sdmessage.pb-c.h"

int read_all(int sock, uint8_t *buf, int len);
int write_all(int sock, uint8_t *buf, int len);

#endif
