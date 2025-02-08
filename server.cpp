// ESTRUTURA BÁSICA DO SERVER 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    struct sockaddr_in caddr;
    struct sockaddr_in saddr;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(5000);

    int server = socket(AF_INET, SOCK_STREAM, 0);

    bind(server, (struct sockaddr *) &saddr, sizeof saddr); // teste criar struct fora

    listen(server, 5);

    socklen_t csize = sizeof(caddr);
    int client;
    char buff[150];

    while(1) {
        client = accept(server, (struct sockaddr *) &caddr, &csize);

        recv(client, buff, sizeof buff, 0);

        printf("\nMensagem recebida: %s", buff);

        close(client);
    }

    return 0;
}