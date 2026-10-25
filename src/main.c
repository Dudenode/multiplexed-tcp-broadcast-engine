#include "server.h"
#include "error.h"
#include <signal.h>

volatile sig_atomic_t server_running = 1;

struct client_record registry[MAX_CLIENT];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_sigint(int sig) {
    (void)sig;
    server_running = 0;
}

int main(){

    signal(SIGINT, handle_sigint);

    int server_fd = init_server(PORT);

    struct pollfd fds[MAX_CLIENT+1];

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (int i = 1;i<MAX_CLIENT+1;i++){
        fds[i].fd = -1;
    }
    while(server_running){

        char buffer[1024];
        memset(buffer,0,sizeof(buffer));

        int poll_count = poll(fds, MAX_CLIENT + 1, -1);
        if (poll_count < 0) {
            if (errno == EINTR) {
                continue; 
            }
            die("poll failed");
        }

        if (fds[0].revents & POLLIN){
            printf("New connection detected!\n");
            struct sockaddr_in client_on_thread;
            socklen_t client_size = sizeof(struct sockaddr_in);

            int new_fd = accept(server_fd,(SA *)&client_on_thread,&client_size);

            if(new_fd < 0){
                sys_warn("accept failed!");
                continue;

            } 

            int i;
            for (i = 1; i<MAX_CLIENT+1;i++){
                if (fds[i].fd == -1){
                    fds[i].fd = new_fd;
                    fds[i].events = POLLIN;
                    break;
                }
                
            }

            if (i == MAX_CLIENT+1){
                    close(new_fd);
                    printf("server currently full!\n");
                }
        }    

        for (int i = 1; i < MAX_CLIENT+1; i++ ){

            if (fds[i].fd == -1) continue;

            if (fds[i].revents & POLLIN){
                    ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer) - 1);

                if (bytes_read == 0){

                    close(fds[i].fd);
                    fds[i].fd = -1;
                    printf("client disconnected!\n");

                } 

                if (bytes_read > 0){

                    buffer[bytes_read] = '\0';
                    printf("Client %d says: %s\n", fds[i].fd, buffer);

                    for (int j = 1; j < MAX_CLIENT + 1; j++){
                        if (fds[j].fd == -1) continue;
                        if (fds[j].fd == fds[i].fd) continue;
                        send(fds[j].fd,buffer,bytes_read,0);
                    }
                }
                    
                if (bytes_read < 0){
                    sys_warn("unable to read!");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    continue;
                }
            }           
        }
    }

    return EXIT_SUCCESS;

}