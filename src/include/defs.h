#define MD5_HASH_SIZE           32
#define SHM_BUFFER_SIZE         4096
#define REQUIRED_ARGS           2
#define MIN_FILES               20
#define DEFAULT_SLAVE_COUNT     3
#define MD5_HASH                32
#define INITIAL_SEM_MUTEX       1
#define VIEW_SLEEP_TIME         2
#define BUF_FILE_PATH_SIZE      128

#define SLAVE_PATH              "./slave"

#define ENC_SIZE                32
#define BUFF_SIZE               256
#define FORMAT_SIZE             10
#define MD5SUM_PATH             "/usr/bin/md5sum"

#define SHM_PREFIX              "/md5_shm_"
#define SEM_BUFF_PREFIX         "/md5_buff_sem_"
#define SEM_MUTEX_PREFIX        "/md5_mutex_"