#include <privio.h>
#include <stdio.h>
#include <string.h>

#define CMD_COUNT 14
#define CMD_LENGTH 16

int main(void){

  int i, j;
  char cmds[CMD_COUNT][CMD_LENGTH] = { 
    "write", "read", "zip", "rm", "rename", "cp", "mkdir", "mv",
    "stat", "mkdir", "lines", "type", "list", "tail", "chmod", "chattr"
  };

  for(i=0; i<CMD_COUNT; i++){
    char upCmd[strlen(cmds[i])];
    memset(upCmd, 0, CMD_LENGTH);
    for(j=0; j<strlen(cmds[i]); j++)
      upCmd[j] = toupper(cmds[i][j]);

    printf("#define CMD_%s 0x%X\n", upCmd, cmdHash(cmds[i]));
  }

  return 0;
}
