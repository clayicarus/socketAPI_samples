//
// Created by clay on 22-9-28.
//

#include "lib/lnp.h"

int tcp_connect(const char *host, const char *serv)
{
    int sockfd, n;
    addrinfo hints, *res, *ressave;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
        err_quit("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
    }
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd < 0)
            continue;
        if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        Close(sockfd);
    } while((res = res->ai_next) != NULL);

    if(res == NULL)
        err_sys("tcp_connect error for %s, %s", host, serv);

    freeaddrinfo(ressave);
    return sockfd;
}

int Tcp_connect(const char *host, const char *serv)
{
    if (tcp_connect(host, serv) < 0)
        err_sys("tcp_connect error");
}