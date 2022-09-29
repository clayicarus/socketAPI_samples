//
// Created by clay on 22-9-27.
//

#include "lib/lnp.h"

#include<iostream>
#include<array>
#include <algorithm>

using namespace std;

void onMessage(int sockfd, array<char, MAXLINE> &buf);

int main(int argc, char *argv[])
{
    int maxi, maxfd, listenfd;
    array<int, FD_SETSIZE> clients; // client is sockfd.
    array<char, MAXLINE> buf;
    fd_set allset;  // use of record all need to be listened.
    socklen_t clilen;
    sockaddr_in cliaddr, servaddr;

    listenfd = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(listenfd, reinterpret_cast<sockaddr *> (&servaddr), sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    maxfd = listenfd;
    maxi = -1;
    clients.fill(-1);

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for( ; ; ) {
        fd_set rset;
        int nready;
        int connfd, sockfd; // conn is only new connected fd.
        int i;

        rset = allset;

        cout << "blocking on select() " << endl;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
        cout << nready << " fd(s) readable " << endl;

        if(FD_ISSET(listenfd, &rset)) {
            /* nready >= 1 ? */
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, reinterpret_cast<sockaddr *>(&cliaddr), &clilen); // new connection
            /*onConnection*/
            cout << "new client connected" << endl;
            for(i = 0; i < clients.size(); ++i) {
                if(clients[i] < 0){
                    clients[i] = connfd;
                    break;
                }
            }
            if(i == clients.size()) {
                err_quit("too many clients");
            }

            FD_SET(connfd, &allset);    // add new client to set.
            maxfd = max(maxfd, connfd);
            maxi = max(maxi, i);        // to check probable available clients.

            if(--nready <= 0) {
                cout << "no clients readable" << endl;
                continue;               // only listen sock readable.
            } else {
                cout << nready << " client(s) readable" << endl;
            }
        }

        for(i = 0; i <= maxi; ++i) {    // pay attention to i <= maxi
            if((sockfd = clients[i]) < 0) {
                continue;
            }
            if(FD_ISSET(sockfd, &rset)) {
                /* OnMessage callback */
                ssize_t n;
                cout << "client " << sockfd << " readable" << endl;
                cout << "blocking on read()" << endl;
                if((n = Read(sockfd, buf.data(), buf.size())) == 0) {
                    /* connection closed by client */
                    Close(sockfd);  // send FIN; go to LAST_ACK.
                    FD_CLR(sockfd, &allset);
                    clients[i] = -1;
                    // change maxi.
                    cout << "client " << sockfd << " disconnected" << endl;
                } else {
                    cout << "read " << n << " bytes" << endl;
                    buf[n] = '\0';
                    cout << "client: " << buf.data() << endl;
                    Writen(sockfd, buf.data(), n);  // directly write what you read.
                }

                if(--nready <= 0) {
                    // no clients readable
                    break;
                }
            }
        }
    }
}

void onMessage(int sockfd, array<char, MAXLINE> &buf)
{

}