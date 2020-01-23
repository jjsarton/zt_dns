#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "zt_conf.h"

void freeOpts(options_t *opts) {
	int i = 0;
	if ( opts == NULL ) {
		return;
	}

	while ( opts[i].name ) {
		if ( opts[i].value) free(opts[i].value);
		i++;
	}
}

int readConf(const char *me, const char *file, options_t opts[]) {
	FILE *fp = NULL;
	char buf[1024];
	int i;
	if ( file == NULL ) {
		return 1;
	}
	if ((fp = fopen(file,"r")) == NULL ) {
		fprintf(stderr,"%s: Error opening %s %s\n",me, file,strerror(errno));
		return 1;
	}
	options_t *opt = opts;
	char key[1024];
	char value[1024];
	while ( fgets(buf, sizeof(buf), fp) ) {
		*key = *value = '\0';;
		sscanf(buf,"%s %s", key, value );
		opt = &opts[0];
		for ( i = 0;; i++ ) {
			opt = &opts[i];
			if ( opt == NULL  || opt->name == NULL)
				break;
				
			if ( strcmp(opt->name,key) == 0) {
				opt->value = strdup(value);
			}
		}
	}
	fclose(fp);
	return 0;
}
