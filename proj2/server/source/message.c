#include <unistd.h>

#include "../include/message-private.h"

int read_all(int sock, uint8_t *buf, int len){
  int amount_read = 0;
  int result;

  while(amount_read < len){
    result = read(sock, buf + amount_read, len - amount_read);
    if(result < 1) return result;

    amount_read += result;
  }
  return amount_read;
}

int write_all(int sock, uint8_t *buf, int len){
  int bufsize = len;
  while(len > 0){
    int res = write(sock, buf, len);
    if(res < 0) return res;

    buf += res;
    len -= res;
  }
  return bufsize;
}
