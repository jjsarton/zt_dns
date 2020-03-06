#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>

int main(int argc, char **argv) {
	if ( argc != 3 ) {
		fprintf(stderr,"Usage qns name|address serverIp\n");
		exit(1);
	}
	int sock;
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr));

	if ( (sock = socket(AF_INET, SOCK_STREAM,0)) < 0 ) {
		perror("socket");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	inet_pton(AF_INET, argv[2], &addr.sin_addr);

	if ( connect(sock, (struct sockaddr*) &addr, sizeof(addr))<0) {
		perror("connect");
	}
	char buf[1024];
	snprintf(buf,sizeof(buf),"I|%s",argv[1]);
	int d;
	if ( (d =isdigit(argv[1][0]))) {
		*buf = 'N';
	}
	int w = write(sock, buf, strlen(buf));
	if ( w == -1 ) {
		perror("write");
		exit(1);
	}
	char rbuf[1024];
	memset(rbuf,0,sizeof(rbuf));
	int r;
	r = read(sock,rbuf, sizeof(rbuf));
	if ( r < 0 ) {
		perror("read");
		exit(1);
	}
	if ( r > 0 )
		printf("%s %s\n",d?argv[1]:rbuf,d?rbuf:argv[1]);
	else
		printf("%s %s\n",argv[1],"NOT FOUND");
	return 0;
}
