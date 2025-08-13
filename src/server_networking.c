#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../includes/server_networking.h"

#define PORT "9990"

int main(void) {
    set_up_server_connect();    
}

int set_up_server_connect() {
    struct addrinfo hints = {0};
    hints.ai_flags = AI_PASSIVE; 	//address meant for binding so get wildcard address
    hints.ai_family = AF_UNSPEC;	//any address family
    hints.ai_socktype = SOCK_STREAM;	//tcp

    struct addrinfo* address_results;
    int status;

    //getting address information
    status = getaddrinfo(NULL, PORT, &hints, &address_results);
    if (status < 0) {
	printf("Error getaddrinfo: %s", gai_strerror(status));
	return -1;
    }

    //creating and binding socket
    status = -1;
    int listen_sockfd;
    do {
	if (!address_results) {
	    printf("Error creating socket\n");
	    return -1;
	}
    	listen_sockfd = socket(address_results->ai_family, address_results->ai_socktype, address_results->ai_protocol);	

	if (listen_sockfd < 0) continue;

	status = bind(listen_sockfd, address_results->ai_addr, address_results->ai_addrlen);

	address_results = address_results->ai_next;
    }
    while (status != 0);
    
    char ip[100];
    struct sockaddr_in* sock_addr = (struct sockaddr_in*)address_results->ai_addr;
    inet_ntop(address_results->ai_family, &sock_addr->sin_addr, ip, sizeof(struct sockaddr));

    printf("Binding done successfully\n");
    printf("Address: %s\n", ip);
    

    status = listen(listen_sockfd, 10);

    if (status != 0) {
	printf("Could not set up Listening socket\n");
	close(listen_sockfd);
	return -1;
    }
    
    printf("Listening on Port: %d\n", ntohs(sock_addr->sin_port));
    freeaddrinfo(address_results);

    while(1) {
    }

    close(listen_sockfd);


}
