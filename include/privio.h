/* Hashes for commands... generated by bin/cmdhashes */
#define CMD_WRITE 0x10A8B550
#define CMD_READ 0x7C9D4D41
#define CMD_ZIP 0xB88C7D8
#define CMD_RM 0x597964
#define CMD_RENAME 0x192CC41D
#define CMD_CP 0x597778
#define CMD_MKDIR 0xFEFD2FC
#define CMD_MV 0x5978C8
#define CMD_STAT 0x7C9E1981
#define CMD_MKDIR 0xFEFD2FC
#define CMD_LINES 0xFDCCBC0
#define CMD_TYPE 0x7C9EBD07
#define CMD_LIST 0x7C9A1661
#define CMD_TAIL 0x7C9E560F

/* Debug level defines */
#define DBG_ERROR   1
#define DBG_INFO    2 
#define DBG_VERBOSE 3
#define DBG_DEBUG1  4
#define DBG_DEBUG2  5
#define DBG_DEBUG3  6

#include <privioConfig.h>
#include <libconfig.h>

typedef int (* privioFunction)(config_t *, const char **);
extern char *privio_error;

/* top-level functions */
int privio_writer(config_t *, const char **);
int privio_reader(config_t *, const char **);
int privio_mkdir(config_t *, const char **);
int privio_rename(config_t *, const char **);
int privio_zip(config_t *, const char **);
int privio_mv(config_t *, const char **);
int privio_lines(config_t *, const char **);
int privio_type(config_t *, const char **);
int privio_list(config_t *, const char **);
int privio_tail(config_t *, const char **);
int privio_cp(config_t *, const char **);
int privio_rm(config_t *, const char **);
int privio_auth(config_t *, const char *, const char *);

/* utility functions */
int privioGetConfig(config_t *);
int privioPathValidator(config_t *, char *);
char **privioReadPaths(config_t *, int);
unsigned int cmdHash(const char *);
privioFunction getOpFromCommand(config_t *, const char *);
void privio_debug(config_t *, int, const char *, ...);
