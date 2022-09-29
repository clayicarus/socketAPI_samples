//
// Created by clay on 22-9-28.
//

#include "lib/lnp.h"

#include<iostream>
#include<array>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[])
{
    int max_i, listenfd;
    array<char, MAXLINE> buf;
    socklen_t clilen;
    sockaddr_in cliaddr, servaddr;

    listenfd = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, reinterpret_cast<sockaddr *> (&servaddr), sizeof(servaddr));
    Listen(listenfd, LISTENQ);

    const auto open_max = sysconf(_SC_OPEN_MAX);
    pollfd * clients = new pollfd[open_max];

    for_each(clients, clients + open_max, [](auto &i){
        i.fd = -1;
        i.events = POLLIN;
    });
    clients[0].fd = listenfd;

    cerr << "test" << endl;
    max_i = 0;
    for( ; ; ) {
        int nready;
        int connfd, sockfd; // conn is only new connected fd.
        int i;

        cout << "blocking on poll() " << endl;
        nready = Poll(clients, max_i + 1, -1);
        cout << "   ";
        cout << nready << " fd(s) readable " << endl;

        if(clients[0].revents & POLLIN) {  // listen sock readable
            /* nready >= 1 ? */
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, reinterpret_cast<sockaddr *>(&cliaddr), &clilen); // new connection
            /* onConnection */
            cout << "   ";
            cout << "new client connected" << endl;
            for(i = 1; i < open_max; ++i) { // skip listen socket.
                if(clients[i].fd < 0){
                    clients[i].fd = connfd;
                    break;
                }
            }
            if(i == open_max) {
                err_quit("too many clients");
            }

            max_i = max(max_i, i);        // to check probable available clients.

            if(--nready <= 0) {
                continue;               // only listen sock readable.
            } else {
                cout << nready << " client(s) readable " << endl;
            }
        }

        for(i = 1; i <= max_i; ++i) {    // pay attention to i = 1
            if((sockfd = clients[i].fd) < 0) {
                continue;
            }
            if(clients[i].revents & (POLLIN | POLLERR)) {   // IN or ERR will be caught
                /* OnMessage callback */
                ssize_t n;
                cout << "   ";
                cout << "client " << sockfd << " readable" << endl;
                cout << "       ";
                cout << "blocking on read()" << endl;
                if((n = read(sockfd, buf.data(), buf.size())) < 0) {
                    if(errno == ECONNRESET) {
                        Close(sockfd);
                        clients[i].fd = -1;
                    } else {
                        err_sys("read error");
                    }
                } else if(n == 0) {
                    /* connection closed by client */
                    Close(sockfd);  // send FIN; go to LAST_ACK.
                    clients[i].fd = -1;
                    // FIXME: shrink maxi.
                    cout << "   ";
                    cout << "client " << sockfd << " disconnected" << endl;
                } else {
                    cout << "       ";
                    cout << "read " << n << " bytes" << endl;
                    buf[n - 1] = '\0';  // may be '\n'
                    cout << "       ";
                    cout << "client: " << buf.data() << endl;
                    Writen(sockfd, buf.data(), n);  // directly write what you read.
                }

                if(--nready <= 0) {
                    // no clients readable
                    break;
                }
            }
        }
        cout << endl;
    }

    delete[] clients;
    clients = NULL;
    return 0;
}
