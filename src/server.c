#include "server.h"
#include "error.h"

int init_server(int port){
    int server_fd;
   

    if ((server_fd = socket(AF_INET,SOCK_STREAM,0))<0){
        die("socket failed!");
    }

    struct sockaddr_in server_addr;

    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(server_fd,(SA*)&server_addr,sizeof(server_addr))<0){
        die("bind failed");
    }

    if((listen(server_fd,10))<0){
        die("listen failed");
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1){
        die("flaged!");
    }
    if (fcntl(server_fd,F_SETFL,flags| O_NONBLOCK) == -1){
        die("flaged!");
    }

    return server_fd;

}