/* zt_nss is a little server wich can kontakt the zerotier
 * zt_dns server and return the IPv4 or name of a queried
 * host to the nss library nss_zt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <nss.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <libgen.h>
#include <poll.h>
#include "zt_conf.h"
#include "zt_nss.h"

// declarations
#define ZT_PORT 9999
#define CONF_FILE "/etc/zt_nss.conf"
#define ZT_DOMAIN "zt"

// for our configurations file must be global
options_t opts[] = {
	{ "nssPort:", NULL },
	{ "nssServerIp:", NULL },
	{ "ztDomain:", NULL },
	{ "IpDomain:", NULL },
	{ NULL, NULL }
};

enum {
PORT,
IP,
DOMAIN,
IP_PART
};

static int unSock;

/* create unix socket */
static int createLocalSocket(char *me, char *socketName) {
	if ( !me ) {
		fprintf(stderr, "%s: %s() Fatal error I don't known me name\n",__FILE__,__FUNCTION__);
		exit(1);
	}
	if ( !socketName ) {
		fprintf(stderr,"%s no socket name defined\n",me);
		return -1;
	}
	int sock=socket(AF_UNIX,SOCK_STREAM, 0);
	if ( sock < 0 ) {
		fprintf(stderr,"%s error creating local socket\n",me);
		return -1;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path,socketName, sizeof(addr.sun_path)-1);
	if ( bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		fprintf(stderr,"%s bind: %s\n",me,strerror(errno));
		close(sock);
		return -1;
	}
	/* set access mode */
	chmod(socketName, 0666);
	listen(sock,5);
	return sock;
}

/* query remote server */
static int requestData(char *me, int port, char *ip, char *buf, int l, char ni[], int sz) {
	struct sockaddr_in rem;
	memset(&rem,0,sizeof(struct sockaddr_in));
	rem.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &rem.sin_addr);
    rem.sin_port = htons(port);	
    
	int sock = socket(AF_INET,SOCK_STREAM, 0);
	if ( sock < 0 ) {
		fprintf(stderr,"%s can't create socket: %s\n",me,strerror(errno));
		return 0;
	}
	if ( connect(sock, (struct sockaddr*)&rem, sizeof(rem)) == -1 ) {
		fprintf(stderr,"%s connection failed: %s\n",me,strerror(errno));
		return 0;
	}
	/* send query */
	int s = 0;
	int pos = 0;
	while ( pos < l ) {
		s = write(sock, buf+pos,l-pos); 
		if ( s > 0 ) pos += l;
	}
	/* wait for answer */
	int len = read(sock,ni,sz);
	if ( len > -1 ) ni[len]='\0';

	close(sock);

	return 1;
}
static int isForUs(char *in, int len, char *domain, char *ip) {
	int l;
	char *s;
	if ( domain && *in == 'I' ) {
		l = strlen(domain);
		s = strstr(in,domain);
		if ( s && s == in + len -l  ) {
			return 1;
		} else {
			return 0;
		}
	} else if ( ip && *in == 'N') {
		l = strlen(ip);
		l = strncmp(in+2,ip, l);
		if ( l == 0 ){
			return 1;
		} else {
			return 0;
		}
	}
	return 1; // no check always for us
}

static void mainLoop(char *me, int unSock, int port, char *ip, char *ztDomain, char *ipDomain ) {
	int asock;
	struct sockaddr caddr;
	socklen_t clen;
	char buf[100];
	char ni[100];
	struct pollfd pollfd[1];
	pollfd[0].fd = unSock;
	pollfd[0].events = POLLIN;
	while(1) {
		poll(pollfd,1, -1);
		if (pollfd[0].revents & POLLIN) {
			asock = accept(unSock, &caddr, &clen);
			if ( asock > 0 ) {
				int l=0;
				int pos = 0;
				while ((pos = recv(asock, buf+pos, sizeof(buf-1-pos),MSG_DONTWAIT)) > 0) {
					l += pos;
				}
				*ni = '\0';
				if ( l > 3 ) {
					buf[l]='\0';
					/* check if for us */
					if ( isForUs(buf, l, ztDomain, ipDomain) ) {
						requestData(me, port, ip, buf, l, ni, sizeof(ni));
					}
				}
//				*ni = '\0';
				if ( *ni ) {
					send(asock, ni, strlen(ni),0);
				}
				shutdown(asock,SHUT_RDWR);
				close(asock);
			}
		}
	}
}
static void sigHandler(int signo) {
	unlink(SOCKET_FILE);
	freeOpts(opts);
	exit(0);
}

static void usage(const char *me) {
	fprintf(stderr,"Usage: %s [-c <file>]\n",me);
	fprintf(stderr,"\t-c <file> use alternate configuration file\n");
}

int main(int argc, char **argv) {
	char *me = basename(argv[0]);
	char *confFile = NULL;
	int opt;
	unSock = -1;
	int port = 0;
	
	/* process options */
	while ((opt = getopt(argc, argv, "c:")) != -1) {
		switch(opt) {
			case 'c':
				confFile=optarg;
				break;
			default:
				usage(me);
				return 1;
		}
	}

	/* read config */
	if ( confFile == NULL ) {
		confFile = CONF_FILE;
	}

	if ( readConf(me, confFile, opts) ) {
		fprintf(stderr,"%s can't read %s\n",me, confFile);
		return 1;
	}
	
	/* set variable and check conf */
	if ( opts[IP].value == NULL ) {
		fprintf(stderr,"%s wrong configuration no server IP\n",me);
		freeOpts(opts);
		return 1;
	}
	port = atoi(opts[PORT].value);
	if (port < 1024 ) {
		port = ZT_PORT;
	}
	/* initialize */
	unSock = createLocalSocket(me, SOCKET_FILE);

	/* install signal handler */
	signal(SIGINT,sigHandler);
	signal(SIGTERM,sigHandler);


	/* main loop */
	mainLoop(me, unSock, port, opts[IP].value, opts[DOMAIN].value,opts[IP_PART].value );
	unlink(SOCKET_FILE);
	freeOpts(opts);

	return 0;
}
