//
//  socket_utils.h
//  3CAT-2_COMMS_SOCKET
//
//  Created by Juan Fran Muñoz Martin on 01/10/14.
//  Copyright (c) 2014 Juan Fran Muñoz Martin. All rights reserved.
//

#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SU_BUFFER_SIZE      65535
#define SU_IP_STR_SIZE      16

typedef enum su_errno_e{
    SU_NO_ERROR,
    SU_TIMEOUT,
    SU_IO_ERROR,
}su_errno_e;

/* client shall fill that, server only the port */
typedef union __attribute__ ((__packed__)) socket_config_t {
    struct __attribute__ ((__packed__)) {
        char        ip[SU_IP_STR_SIZE];
        int         port;
    }client;
    struct __attribute__ ((__packed__)) {
        int         port;
    }server;
}socket_config_t;

typedef struct __attribute__ ((__packed__)) socket_handler_t {
    int         fd;
    uint8_t     buffer[SU_BUFFER_SIZE];
    int         expected_len;
    int         len;
    int         timeout_ms;
}socket_handler_t;

typedef struct __attribute__ ((__packed__)) server_handler_t {
    int fd;
}server_handler_t;

/* Server funcionts */
su_errno_e server_socket_init(socket_config_t *conf, server_handler_t *server);
su_errno_e server_socket_new_client(server_handler_t *server, socket_handler_t *client);

/* Client functions */
su_errno_e client_socket_init(socket_config_t *conf, socket_handler_t *s);

/* Access functions */
su_errno_e socket_read(socket_handler_t *s);
su_errno_e socket_write(socket_handler_t *s);

#ifdef __cplusplus
}
#endif

#endif
