//
//  socket_utils.c
//  3CAT-2_COMMS_SOCKET
//
//  Created by Juan Fran Muñoz Martin on 01/10/14.
//  Copyright (c) 2014 Juan Fran Muñoz Martin. All rights reserved.
//
#include "socket_utils.h"

#if defined(__APPLE__) && !defined(MSG_NOSIGNAL)
#  define MSG_NOSIGNAL 0
#  define osx_block_sigpipe(sock) sockopt_enable(sock, SOL_SOCKET, SO_NOSIGPIPE)
#else
#  define osx_block_sigpipe(sock) (void) (sock) /* no-op */
#endif

static int sockopt_enable(int sock, int level, int optname)
{
    const int enable = 1;
    if(setsockopt(sock, level, optname, &enable, sizeof(enable)) != 0) {
        return -1;
    }
    return 0;
}

static void check_exp_len(socket_handler_t *s)
{
    if(s->expected_len == 0 || s->expected_len > SU_BUFFER_SIZE) {
        s->expected_len = SU_BUFFER_SIZE;
    }
}

static su_errno_e test_fd(int fd)
{
    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    if(poll(&fds, 1, 0) == 0) {
        return SU_NO_ERROR;
    } else {
        return SU_IO_ERROR;
    }
}

su_errno_e socket_write(socket_handler_t *s)
{
    if(test_fd(s->fd) == SU_IO_ERROR) {
        return SU_IO_ERROR;
    }
    s->expected_len = send(s->fd, s->buffer, s->len, MSG_NOSIGNAL);
    if(s->expected_len != s->len) {
        return SU_IO_ERROR;
    } else {
        return SU_NO_ERROR;
    }
}

static int timeout_on_fd(int fd, int timeout_ms)
{
    struct timeval tv;
    // fd_set passed into select
    fd_set fds;
    int control_ret;
    if(timeout_ms >= 1000) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
    } else {
        tv.tv_sec = 0;
        tv.tv_usec = timeout_ms * 1000;
    }
    // Zero out the fd_set - make sure it's pristine
    FD_ZERO(&fds);
    // Set the FD that we want to read
    FD_SET(fd, &fds);
    // select takes the last file descriptor value + 1 in the fdset to check,
    // the fdset for reads, writes, and errors.  We are only passing in reads.
    // the last parameter is the timeout.  select will return if an FD is ready or
    // the timeout has occurred
    if( (control_ret = select(fd+1, &fds, NULL, NULL, &tv) ) == -1) {
        return SU_IO_ERROR;
    }
    // return 0 if fd is not ready to be read.
    if( ( control_ret = FD_ISSET(fd, &fds) ) > 0 ) {
        /* Something to read! */
        return SU_NO_ERROR;
    } else {
        if(control_ret == 0) {
            return SU_TIMEOUT;
        } else {
            return SU_IO_ERROR;
        }
    }
}

su_errno_e socket_read(socket_handler_t *s)
{
    int ret;
    // Set up the timeout.  here we can wait for 1 second
    check_exp_len(s);
    if(s->timeout_ms == -1) {
        s->len = read(s->fd, s->buffer, s->expected_len);
        if(s->len <= 0) {
            return SU_IO_ERROR;
        } else {
            return SU_NO_ERROR;
        }
    } else {
        ret = timeout_on_fd(s->fd, s->timeout_ms);
        if(ret == SU_NO_ERROR) {
            s->len = read(s->fd, s->buffer, s->expected_len);
            if(s->len <= 0) {
                return SU_IO_ERROR;
            } else {
                return SU_NO_ERROR;
            }
        } else {
            return ret;
        }
    }
}

su_errno_e client_socket_init(socket_config_t *conf, socket_handler_t *s)
{
    /* estructura que recibirá información sobre el nodo remoto */
    struct sockaddr_in serv_addr;
    struct hostent *host;
    /* información sobre la dirección del servidor */
    if((host = gethostbyname(conf->client.ip))==NULL) {
        return SU_IO_ERROR;
    }
    if((s->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return SU_IO_ERROR;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(conf->client.port);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);

    if(connect(s->fd, (struct sockaddr *)&serv_addr,
               sizeof(serv_addr)) == -1) {
        return SU_IO_ERROR;
    }
    osx_block_sigpipe(s->fd);
    return SU_NO_ERROR;
}

su_errno_e server_socket_new_client(server_handler_t *server, socket_handler_t *client)
{
    int ret;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    /* Something to read! */
    if(client->timeout_ms == -1) {
        client->fd = accept(server->fd, (struct sockaddr *) &cli_addr, &clilen);
        if(client->fd > 0) {
            osx_block_sigpipe(client->fd);
            return SU_NO_ERROR;
        } else if(client->fd == 0) {
            return SU_TIMEOUT;
        } else {
            return SU_IO_ERROR;
        }
    } else {
        ret = timeout_on_fd(server->fd, client->timeout_ms);
        if(ret == SU_NO_ERROR) {
            client->fd = accept(server->fd, (struct sockaddr *) &cli_addr, &clilen);
            if(client->fd > 0) {
                osx_block_sigpipe(client->fd);
                return SU_NO_ERROR;
            } else if(client->fd == 0) {
                return SU_TIMEOUT;
            } else {
                return SU_IO_ERROR;
            }
        } else {
            return ret;
        }
    }
}

su_errno_e server_socket_init(socket_config_t *conf, server_handler_t *server)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        return SU_IO_ERROR;

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int)) < 0)
        return SU_IO_ERROR;

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(conf->server.port);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        return SU_IO_ERROR;

    listen(sockfd, 1);
    server->fd = sockfd;

    return SU_NO_ERROR;
}
