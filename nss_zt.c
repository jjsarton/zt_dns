#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <nss.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include "zt_nss.h"

static void zt_fill_hostent(const char *name, int af, struct hostent *result, char *ip, int type);

enum nss_status
_nss_zt_gethostbyname2_r(const char *name,
                           int af,
                           struct hostent *result,
                           char *buffer,
                           size_t buflen,
                           int *errnop,
                           int *h_errnop);


enum nss_status
_nss_zt_gethostbyname_r(const char *name,
                          struct hostent *result,
                          char *buffer,
                          size_t buflen,
                          int *errnop,
                          int *h_errnop);




enum nss_status
_nss_zt_gethostbyaddr2_r (const void *addr,
                           socklen_t len, int af,
                           struct hostent *result,
                           char *buffer, size_t buflen,
                           int *errnop, int *h_errnop,
                           int32_t *ttlp);
enum nss_status
_nss_zt_gethostbyaddr_r (const void *addr, socklen_t len, int af,
                          struct hostent *result, char *buffer, size_t buflen,
                          int *errnop, int *h_errnop);
// definitions

static void zt_fill_hostent(const char *name, int af, struct hostent *result, char *ip, int type) {
    result->h_name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(result->h_name, name);

    result->h_aliases = malloc(sizeof(char *));
    *result->h_aliases = NULL;
    result->h_addr_list = malloc(sizeof(char *) * 2);

    switch (af) {
    case AF_INET:
        result->h_addrtype = AF_INET;
        result->h_length = INADDRSZ;
        in_addr_t addr = inet_addr(ip);
        *result->h_addr_list = malloc(sizeof(addr));
        memcpy(*result->h_addr_list, &addr, sizeof(addr));
        break;
#if 0
    case AF_INET6:
        result->h_addrtype = AF_INET6;
        result->h_length = IN6ADDRSZ;
        struct in6_addr addr6 = {};
        inet_pton(AF_INET6, ip, &addr6);
        *result->h_addr_list = malloc(sizeof(addr6));
        memcpy(*result->h_addr_list, &addr6, sizeof(addr6));
        break;
#endif
    }

    *(result->h_addr_list + 1) = NULL;
}

static int make_connection(char *socketname) {
	int sock = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (sock == -1)
	{
		return 0;
	}
	struct sockaddr_un address;
	strcpy(address.sun_path, socketname);
	address.sun_family = AF_LOCAL;
	if (connect(sock, (struct sockaddr *)&address, sizeof(address)) != 0)
	{
		close(sock);
		return 0;
	}
	return sock;
}

static int send_command(int sock, const char *data) {
	int len = strlen(data);
	errno=0;
	int ret = send(sock, data, len, 0);
	return ret;
}

static void getName(const char *name, char *data, int type) {
	int sock = 0;
	int len= 0;
	if ( type == AF_INET6 ) {
		return;
	}
	if ( (sock = make_connection(SOCKET_FILE)) < 0 ) {
		return;
	}
	if ( (len=send_command(sock, name)) > 0 ) {
		int len = 0;
		int ret = 0;
		while((ret=recv(sock, data+len, sizeof(data-len-1), 0))) 
			len += ret;
		close(sock);
		data[len] = '\0';
	}
}

static void strcplower(char *out, const char *in) {
	while (in && *in) {
		*out = tolower(*in);
		in++;
		out++;
	}
    *out='\0';
}

enum nss_status
_nss_zt_gethostbyname2_r(const char *name,
                           int af,
                           struct hostent *result,
                           char *buffer,
                           size_t buflen,
                           int *errnop,
                           int *h_errnop) {
    enum nss_status status = NSS_STATUS_NOTFOUND;
	char *data = NULL;
	char len = strlen(name)+3;
	char *nm =calloc(1, len);
	int type = AF_UNSPEC;
	if ( nm == NULL ) {
		return status;
	}
	strcpy(nm,"I|");
	strcplower(nm+2,name);
	if ( af == AF_INET) {
		data = calloc(1,1024);
		if ( data )
			getName((const char *)nm, data, type);
		if ( data && *data ) {
			zt_fill_hostent(name, af, result, data, type);
			free(data);
			data = NULL;
			status = NSS_STATUS_SUCCESS;
		} 
	}
	if ( data ) free(data);
	if (nm ) free(nm);
    return status;
}

enum nss_status
_nss_zt_gethostbyname_r(const char *name,
                          struct hostent *result,
                          char *buffer,
                          size_t buflen,
                          int *errnop,
                          int *h_errnop) {
    return _nss_zt_gethostbyname2_r(name, AF_INET, result, buffer, buflen, errnop, h_errnop);
}


enum nss_status
_nss_zt_gethostbyaddr2_r (const void *addr,
                           socklen_t len, int af,
                           struct hostent *result,
                           char *buffer, size_t buflen,
                           int *errnop, int *h_errnop,
                           int32_t *ttlp) {
	int type = AF_INET;
    enum nss_status status = NSS_STATUS_NOTFOUND;
    if ( af == AF_INET6 ) return status;

    char ip[258];
    char data[256];
    inet_ntop(af, addr, ip+2, sizeof(ip));
    ip[0]='N';
    ip[1]='|';
    memset(data, 0, sizeof(data));
    getName((const char *)ip, (char *)data,type );
    /* fill data */
    if ( *data == '\0' )
		return status;
	result->h_name = strdup(data);
	return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_zt_gethostbyaddr_r (const void *addr, socklen_t len, int af,
                          struct hostent *result, char *buffer, size_t buflen,
                          int *errnop, int *h_errnop) {
	return _nss_zt_gethostbyaddr2_r (addr, len, af, result, buffer, buflen,
                                    errnop, h_errnop, NULL);
}
