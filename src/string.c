#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "privio_string.h"

/* this is a simple string substitution */
char *strsr(const char *str, const char *pat, const char *sub){
  char *buf, *tmp_str;
  int i,j,old_len,new_len,sub_len,pat_len;

  old_len = strlen(str);
  sub_len = strlen(sub);
  pat_len = strlen(pat);
  new_len = (old_len+(sub_len-pat_len))*sizeof(char);

  /* ensure buffer is as large as both strings, worst case */
  buf = (char *)malloc(new_len+1);
  tmp_str = (char *)malloc(old_len+1);

  buf = memset(buf, 0, new_len+1);
  tmp_str = memset(tmp_str, 0, old_len+1);

  /* copy str to tmp buff, so we don't change it */
  memcpy(tmp_str, str, old_len);

  /* find occurrence */
  j = 0;
  for(i = 0; i <= old_len; i++){
    if((i <= old_len - pat_len) && !strncmp(&tmp_str[i], pat, pat_len)){
      memcpy(&buf[j],sub,sub_len);
      i += pat_len-1;
      j += sub_len;
    } else {
      buf[j++] = tmp_str[i];
    }   
  }

  free(tmp_str);
  return buf;
}
