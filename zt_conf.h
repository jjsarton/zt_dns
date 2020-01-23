/* read configuration file */

typedef struct options_s {
	char *name;
	char *value;
} options_t;

extern void freeOpts(options_t *opts);

extern int readConf(const char *me, const char *file, options_t opts[]);
