#ifndef UNRELIABLEFS_HH
#define UNRELIABLEFS_HH

#include <limits.h> /* PATH_MAX */
#include <pthread.h>

#define DEFAULT_CONF_NAME "unreliablefs.conf"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct GreeterClient GreeterClient;
GreeterClient* NewGreeterClient(const char* host);

int Mkdir(GreeterClient*  greeterClient, const char* file_path, struct fuse_file_info *fi);

#ifdef __cplusplus
}
#endif

extern struct GreeterClient* greeterClient;

typedef struct unreliablefs_config {
     struct err_inj_q *errors;
     char             *basedir;
     char             *config_path;
     unsigned int      seed;
     unsigned int      debug;
     pthread_mutex_t   mutex;
} unreliablefs_config;

#endif /* UNRELIABLEFS_HH */
