#include <iostream>
#include "lib/lnp.h"
#include "lib/lnpthread.h"
#include <vector>
#include <functional>

using namespace std;

struct doit_arg {
    // not safe kana
    doit_arg(const sockaddr &sa_, int lps, string &req)
    : sa(sa_), loops(lps), request(req) {}
    sockaddr sa;
    int loops;
    string request;
};

void * t_doit(void * arg);

pthread_mutex_t mutex_;
int main(int argc, char *argv[])
{
    int nloops;
    int i, nthreads;
    string req;

    if (argc != 6) {
        err_quit("usage: client_test <hostname or IPaddr> <port> <#children> "
                 "<#loops/child> <#bytes/request>");
    }

    sockaddr_in sa_in;
    bzero(&sa_in, sizeof(sa_in));
    Inet_pton(AF_INET, argv[1], &sa_in.sin_addr);
    sa_in.sin_port = htons(stoi(argv[2]));
    sa_in.sin_family = AF_INET;

    nthreads = stoi(argv[3]);
    nloops = stoi(argv[4]);
    req = string(argv[5]);

    pthread_mutex_init(&mutex_, NULL);

    vector<pthread_t> tids(nthreads);

    for(i = 0; i < tids.size(); ++i) {
        sockaddr sa;
        sa = *reinterpret_cast<sockaddr*>(&sa_in);
        auto p_arg = new doit_arg(sa, nloops, req);
        Pthread_create(&tids[i], NULL, t_doit, p_arg);
    }
    for(i = 0; i < tids.size(); ++i) {
        Pthread_join(tids[i], NULL);
    }

    exit(0);
}

void * t_doit(void * arg)
{
    char reply[MAXLINE];
    doit_arg * p_doit_arg = static_cast<doit_arg*>(arg);
    auto dump = pthread_self() % 1000000007;
    p_doit_arg->request = to_string(dump);

    int i;
    ssize_t n;

    for(i = 0; i < p_doit_arg->loops; ++i) {
        int sk;
        // FIXME: most of wrapped functions not thread safe coz stderr.
        sk = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        Connect(sk, reinterpret_cast<sockaddr*>(&p_doit_arg->sa), sizeof(p_doit_arg->sa));  // sa must copy
        Write(sk, p_doit_arg->request.c_str(), p_doit_arg->request.size());

        if((n = Readn(sk, reply, p_doit_arg->request.size())) != p_doit_arg->request.size()) {
            pthread_mutex_lock(&mutex_);
            cerr << "thread " << p_doit_arg->request << ": server returned " << n << " bytes" << endl;
            pthread_mutex_unlock(&mutex_);
            if(reply != p_doit_arg->request) {
                pthread_mutex_lock(&mutex_);
                cerr << "thread " << p_doit_arg->request << ": server returned " << reply << endl;
                pthread_mutex_unlock(&mutex_);
            }
        }
        Close(sk);
    }

    delete p_doit_arg;
    p_doit_arg = NULL;
    pthread_mutex_lock(&mutex_);
    cout << "thread " << pthread_self() % 1000000007 << " done" << endl;
    pthread_mutex_unlock(&mutex_);

    return NULL;
}