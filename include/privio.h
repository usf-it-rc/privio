#include "privioConfig.h"
#include <libconfig.h>
#include <regex.h>
#include <string.h>

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

typedef char privioArgs[PRIVIO_MAX_OPERANDS][PRIVIO_MAX_OPERAND_LENGTH];

/* top-level functions */
int privioWriter(privioArgs*, config_t*);
int privioReader(privioArgs*, config_t*);
int privioMkdir(privioArgs*, config_t*);
int privioRename(privioArgs*, config_t*);
int privioZipreaddir(privioArgs*, config_t*);
int privioMove(privioArgs*, config_t*);
int privioFilelines(privioArgs*, config_t*);
int privioFiletype(privioArgs*, config_t*);
int privioDirlist(privioArgs*, config_t*);
int privioFiletail(privioArgs*, config_t*);

/* utility functions */
int privioGetConfig(const char *path, config_t *);
int privioPathValidator(privioArgs, int, config_t *);
unsigned int cmdHash(const char *);
void *getOpFromCommand(const char *);
