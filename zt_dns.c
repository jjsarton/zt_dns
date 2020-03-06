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
#include <sys/mount.h>
#include <poll.h>
#include <curl/curl.h>
#include <pthread.h>

#include "json-c/json.h"
#include "zt_conf.h"
#define CONF_FILE CONF_DIR"/etc/zt_dns.conf"

/* Define command prefix send to unbound (reload) */
#define UBCT_RELOAD_CMD "UBCT1 reload\n"
#define MY_ZT_URL "https://my.zerotier.com/api/network/";

pthread_mutex_t mutex;

struct MemoryStruct {
  char *memory;
  size_t size;
};

typedef struct name_ip_s {
	char *name;
	char *ip;
	struct name_ip_s *next;
} name_ip_t;

options_t opts[] = {
	{ "token:", NULL },
	{ "net:", NULL },
	{ "unboundFile:", NULL },
	{ "wildcards:", NULL },
	{ "domain:", NULL },
	{ "refresh:", NULL },
	{ "tmpFile:", NULL },
	{ "nssPort:", NULL },
	{ "noDNS:", NULL },
	{ NULL, NULL }
};

enum {
TOKEN,
NET,
UNBOUND_FILE,
WILDCARDS,
DOMAIN,
REFRESH,
TMP_FILE,
NSS_PORT,
NO_DNS
};
static int debug = 0;

static void freeArray();
static name_ip_t *addToArray(const char *me, name_ip_t *act,const char *name, const char *ip);
static int processJson(char *res, const char *me);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
static char* getZtData(char *url, char *header);
static int putTofile(const char *name, const char *domain, const char *wildcards, const char *me);
static int sendReload(char *me);
#ifdef __linux__
static void createDirectories(char *me, char *file);
#endif
static void usage(const char *me);
static void sigHandler(int signo);
static int openNssSock(char *me, int port, char *addresse);
#ifdef __linux__
static void setUidGid(const char *me, const char *ufile, const char *tfile);
#endif
static int openNssSock(char *me,int port, char *addresse);

static name_ip_t *nameIp = NULL;

static int openNssSock(char *me,int port, char *addresse) {
	int sock;
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr));

	if ( (sock = socket(AF_INET, SOCK_STREAM,0)) < 0 ) {
		fprintf(stderr,"%s open socket error %s\n",me,strerror(errno));
		return sock;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, addresse, (struct sockaddr*)&addr.sin_addr);
	int opt=1;
	setsockopt(sock,SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt));
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr,"%s open bind error %s\n",me,strerror(errno));
		return -1;
	}

	listen(sock,5);
	return sock;
}

static char *getIP(const char *nm) {
	name_ip_t *act = nameIp;
	while ( act ) {
		if ( strcmp(act->name, nm) == 0 )
			break;
		act = act->next;
	}
	return act?act->ip:NULL;
}

static char *getName(const char *ip) {
	name_ip_t *act = nameIp;
	while ( act ) {
		if ( strcmp(act->ip, ip) == 0 )
			break;
		act = act->next;
	}
	return act?act->name:NULL;
}

static int processQuery(int sock) {
	struct sockaddr caddr;
	socklen_t clen;
	char buf[100];
	int asock;
	char *ni = NULL;
	memset(buf,0,sizeof(buf));
	asock = accept(sock, &caddr, &clen);
	if ( asock > 0 ) {
		int l=0;
		int pos = 0;

		errno=0;
		pos = read(asock, buf,sizeof(buf));
		if ( pos < 0 )
			fprintf(stderr,"read: %s",strerror(errno));
		pthread_mutex_lock(&mutex);

		if ( pos > 3 ) {
			buf[pos]='\0';
			if ( debug )
				fprintf(stderr,"Received %s\n",buf);
			if ( buf[0] == 'I' ) {
				ni = getIP(buf+2);
			} else {
				ni = getName(buf+2);
			}
		}
		if ( ni ) {
			l =strlen(ni);
			pos = 0;
			int s;
			while ( pos < l) {
				s = write(asock, ni, l-pos);
				if ( s > 0 ) pos += s;
			}
			ni[pos]=0;
			if ( debug )
				fprintf(stderr,"send %s\n",ni);
		}
		pthread_mutex_unlock(&mutex);

		shutdown(asock,SHUT_RDWR);
		close(asock);
	}
	return 0;
}

#ifdef __linux__
static void setUidGid(const char *me, const char *ufile, const char *tfile) {
	struct stat statbuf;
	if ( tfile == NULL ) {
		return;
	}
	if ( stat(ufile, &statbuf) > -1 ) {
		if (chown(tfile,statbuf.st_uid, statbuf.st_gid))
			;
	} else {
		fprintf(stderr,"%s: stat -> %s %s\n",me, tfile, strerror(errno));
	}
}
#endif

static void freeArray() {
	name_ip_t *act = nameIp;
	while(act) {
		if(act->ip)free(act->ip);
		if(act->name)free(act->name);
		nameIp = act->next;
		free(act);
		act = nameIp;
	}
}

static name_ip_t *addToArray(const char *me, name_ip_t *act,const char *name, const char *ip) {
	name_ip_t *new = NULL;
	errno = 0;
	if ( (new = calloc(1, sizeof(name_ip_t))) == NULL ) {
		return NULL;
	}
	if ( act == NULL )
		nameIp = new;
	else
		act->next = new;

	if ((new->name=strdup(name+1)) == NULL) {
		return NULL;
	}
	*strchr(new->name,'"') = '\0';

	if((new->ip=strdup(ip+1)) == NULL) {
		return NULL;
	}
	*strchr(new->ip,'"') = '\0';
	if ( debug ) {
		fprintf(stderr,"\t%-30s %s\n",name,ip);
	}
	return new;
}

static int processJson(char *res, const char *me) {
	struct json_object *jobj = json_tokener_parse(res);
	/* for each object */
	int jsontype = (int)json_object_get_type(jobj);
	if ( jsontype != json_type_array) {
		fprintf(stderr, "%s: Not a valid json File?\n",me);
		return 1;
	}
	freeArray();
	struct json_object *name;
	struct json_object *ipa;
	struct json_object *config;
	struct json_object *ip;
	name_ip_t *act = nameIp;
	struct json_object *aelem;
	int i = 0;
	if ( debug ) {
		fprintf(stderr,"%s: Host list\n",me);
	}
	pthread_mutex_lock(&mutex);
	while ( (aelem = json_object_array_get_idx(jobj,i)) ) {
		json_object_object_get_ex(aelem,"name", &name);
		json_object_object_get_ex(aelem,"config", &config);
		json_object_object_get_ex(config,"ipAssignments", &ipa);
		/* for all ip  */
		int j = 0;

		for (j=0; (ip = json_object_array_get_idx(ipa,j)); j++ ) {
			act = addToArray(me, act,json_object_to_json_string(name),json_object_to_json_string(ip));
			if ( act == NULL ) {
				fprintf(stderr,"%s: %s\n",me,strerror(errno));
				pthread_mutex_unlock(&mutex);
				return 1;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&mutex);
	/* free jobj */
	json_object_put(jobj);
	return 0;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if(ptr == NULL) {
		/* out of memory! */ 
		fprintf(stderr,"not enough memory (realloc returned NULL)\n");
		return 0;
	}
 
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
 
	return realsize;
}

static char* getZtData(char *url, char *header) {
	CURL *curl;
	CURLcode res;
	struct curl_slist *chunk = NULL;
	struct MemoryStruct response;
 
	response.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
	response.size = 0;    /* no data at this point */ 
 
	curl_global_init(CURL_GLOBAL_ALL); 
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		chunk = curl_slist_append(chunk, header);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    
		/* example.com is redirected, so we tell libcurl to follow redirection */ 
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
 
		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_slist_free_all(chunk);
		curl_easy_cleanup(curl);
	}

	return response.memory;
}

static int putTofile(const char *name, const char *domain, const char *wildcards, const char *me) {
	FILE *fp;
	name_ip_t *act = nameIp;
	if  ( act == NULL )
		return 1;
	if ( (fp = fopen(name,"w" )) == NULL ) {
		fprintf(stderr,"%s failed to open %s (%s)\n",me, name,strerror(errno));
		return 1;
		
	}
	while ( act ) {
		if ( wildcards && * wildcards == '1' ) {
			fprintf(fp,"local-zone: \"%s%s\" redirect\n",act->name, domain);
		}
		fprintf(fp,"local-data: \"%s%s A %s\"\n",act->name,domain,act->ip);
		fprintf(fp,"local-data-ptr: \"%s %s%s\"\n",act->ip,act->name,domain);
		act= act->next;
	}
	fflush(fp);
	fclose(fp);
	
	return 0;
}

static int sendReload(char *me) {
	/* send "UBCT1 "   1 is UNBOUND_CONTROL_VERSION
	 *      " "
	 *      "reload"
	 *      0x0a
	 * receive "OK"+0x0a
	 */
	char message[] = UBCT_RELOAD_CMD;
	char *buf[100];
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr));
	memset(buf,0,sizeof(buf));

	int sock = socket(AF_INET, SOCK_STREAM,0);
	if (  sock < 0 ) return 1;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8953);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) <0) return 1;
	int r = send(sock,message, strlen(message), 0);
	if ( r < 0 )
		fprintf(stderr, "%s send; %s\n", me, strerror(errno));
	recv(sock,buf, sizeof(buf),0);
	close(sock);
	if ( debug )
		fprintf(stderr,"%s reload send\n",me);
	return 0;
}

#ifdef __linux__
static void createDirectories(char *me, char *file) {
	char *f = strdup(file);
	char *dir = dirname(f);
	char *tok = strtok(dir, "/" );
	int ret = chdir("/"); /* this shall not be wrong */
	while (tok != NULL) {
		errno = 0;
		if ( mkdir(tok,0777) == -1 && errno != EEXIST) {
			fprintf(stderr,"%s Fatal error %s\n", me, strerror(errno));
			exit(1);
		}
		ret += chdir(tok); /* this shall not be wrong */
		tok = strtok(NULL, "/" );
	}
	free(f);
}
#endif
static void sigHandler(int signo) {
#ifdef __linux__
	if (opts[TMP_FILE].value && *opts[TMP_FILE].value )
		umount(opts[UNBOUND_FILE].value);
#endif
	freeArray();
	freeOpts(opts);
	exit(0);
}

typedef struct pthargs_s {
	int sock;
} pthargs_t;

static void *nssLoop(void *arg) {
	struct pollfd pollfd[1];
	int sock = ((pthargs_t*)arg)->sock;
	pollfd[0].fd = sock;
	pollfd[0].events = POLLIN;
	pollfd[0].revents = 0;
	while(1) {
		poll(pollfd,1, -1);
		if (pollfd[0].revents & POLLIN) {
			processQuery(sock);
		}
	}
	return NULL;
}

static void usage(const char *me) {
	fprintf(stderr,"Usage: %s [-c <file>] [-j]\n",me);
	fprintf(stderr,"\t-c <file> use alternate configuration file\n");
	fprintf(stderr,"\t-d debug\n");
	fprintf(stderr,"\t-j  print the json file returned by zerotier.com and exit\n");
}

int main(int argc, char **argv, char **env) {
	char *me = basename(argv[0]);
	char *confFile = NULL;
	int pjson = 0;
	int opt;
	char *member = "/member";
#ifdef __linux__
	int res;
#endif
	int nssPort = 0;
	int noDNS = 0;
#ifdef __linux__
	FILE *fd;
#endif
	char url[1024] = {0};
	char header[1024];
	char *url1 = MY_ZT_URL;

	signal(SIGINT,sigHandler);
	signal(SIGTERM,sigHandler);
	while ((opt = getopt(argc, argv, "c:jd")) != -1) {
		switch(opt) {
			case 'c':
				confFile=optarg;
				break;
			case 'j':
				pjson=1;
				break;
			case 'd':
				debug=1;
				break;
			default:
				usage(me);
				return 1;
		}
	}
	if ( confFile == NULL )
		confFile = CONF_FILE;

	if ( readConf(me, confFile, opts) ) {
		return 1;
	}

	/* check mandatory variables */
	if ( opts[UNBOUND_FILE].value == NULL ||
		opts[NET].value == NULL ||
		opts[TOKEN].value == NULL ) {
		fprintf(stderr,"%s Configuration error\n",me);
		return 1;
	}
	/* set variables */
	snprintf(url, sizeof(url), "%s%s%s",url1,opts[NET].value,member);
	sprintf(header, "Authorization: bearer %s", opts[TOKEN].value);

	if (opts[REFRESH].value == NULL )
		opts[REFRESH].value = "60";

	nssPort = opts[NSS_PORT].value?atoi(opts[NSS_PORT].value):0;

	if (opts[NO_DNS].value && *opts[NO_DNS].value=='1') {
		noDNS=1;
	}

	/* prepare for unbound */
#ifdef __linux__
	if ( noDNS == 0 ) {
		createDirectories(me, opts[UNBOUND_FILE].value);
		if ( opts[TMP_FILE].value && *opts[TMP_FILE].value && !pjson) {
			/* create  an empty file and bind it to opts[UNBOUND_FILE].value */
			fd = fopen(opts[TMP_FILE].value, "w+");
			if ( fd == NULL ) {
				fprintf(stderr,"%s: create %s %s\n",me, opts[TMP_FILE].value, strerror(errno));
				if ( errno != EEXIST ) { // don't consider EEXIST as error (restart)
					return 1;
				}
			} else {
				fclose(fd);
			}
			setUidGid(me, opts[UNBOUND_FILE].value,opts[TMP_FILE].value);
			/* for the case allready mounted */
			umount(opts[UNBOUND_FILE].value);
			/* now bind file */
			errno=0;
			if ((res = mount(opts[TMP_FILE].value,opts[UNBOUND_FILE].value, NULL,MS_BIND,"rw")) ) {
				fprintf(stderr,"%s mount failed (%s)\n",me,strerror(errno));
				return 1;
			}
		}
	}
#endif

	/* prepare for main loop */
	int timeout = atoi(opts[REFRESH].value)*60;
	int ret = 0;
	int sock = 0;
	if ( nssPort > 0 ) {
		sock = openNssSock(me,nssPort, "0.0.0.0");
		/* createpthread for nss */
		pthread_t pthread;
		pthargs_t args;
		args.sock = sock;
		int pth = pthread_create(&pthread,NULL, &nssLoop, (void *) &args);
		if ( pth )
			fprintf(stderr,"%s pthread_create:%s\n",me,strerror(errno));;
	}
	/* */

	char *response;
	while(1) {
		response =  getZtData(url, header);
		if ( pjson && response ) {
			printf("%s\n",response);
			break;
		}
		if (processJson(response, me)&& response) {
			fprintf(stderr,"%s: Fatal error (%s)\n",me,strerror(errno));
			ret = 1;
		}
		if ( response) {
			free(response);
			response = NULL;
		}
		if ( noDNS == 0 ) {
			if ( debug )
				fprintf(stderr,"%s: update unbound file\n",me);
			putTofile(opts[UNBOUND_FILE].value, opts[DOMAIN].value, opts[WILDCARDS].value, me);
			if (sendReload(me)) {
				fprintf(stderr,"%s: Error while send reload\n",me);
			}
		}
		sleep(timeout);
	} 
#ifdef __linux__
	if ( opts[TMP_FILE].value && *opts[TMP_FILE].value && !pjson ) {
		umount(opts[UNBOUND_FILE].value);
	}
#endif
	freeArray();
	freeOpts(opts);
	if ( response) free(response);
	return ret;
}

