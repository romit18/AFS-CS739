#ifndef UNRELIABLEFS_HH
#define UNRELIABLEFS_HH

#include <limits.h> /* PATH_MAX */
#include <pthread.h>
#include <dirent.h>
#define DEFAULT_CONF_NAME "unreliablefs.conf"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct UnreliableAFS UnreliableAFS;
UnreliableAFS* NewUnreliableAFS();

int Mkdir(UnreliableAFS* unreliableAFS, const char* file_path, int mode);
int Rmdir(UnreliableAFS* unreliableAFS, const char* file_path);
int Getattr(UnreliableAFS* unreliableAFS, const char* path, struct stat* buf);
int Getxattr(UnreliableAFS* unreliableAFS, const char* path, const char* name, void* value, size_t size);
int Opendir(UnreliableAFS* unreliableAFS, const char* path, DIR* directory);
int Open(UnreliableAFS* unreliableAFS, const char* path, int flags);
int Create(UnreliableAFS* unreliableAFS, const char* path, int flags, int mode);
int Close(UnreliableAFS* unreliableAFS, const char* path, int fd);
int Unlink(UnreliableAFS* unreliableAFS, const char* file_path);
int Rename(UnreliableAFS* unreliableAFS, const char* old_path, const char* new_path);
#ifdef __cplusplus
}
#endif

extern struct UnreliableAFS* unreliableAFS;

typedef struct unreliablefs_config {
     struct err_inj_q *errors;
     char             *basedir;
     char             *config_path;
     unsigned int      seed;
     unsigned int      debug;
     pthread_mutex_t   mutex;
} unreliablefs_config;

#endif /* UNRELIABLEFS_HH */
