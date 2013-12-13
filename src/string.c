#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "privio_string.h"

/* this is a simple string substitution */
char *strsr(const char *str, const char *pat, const char *sub){
  char *buf, *tmp_str;
  int i,j,k,old_len,new_len,sub_len,pat_len,match = 0;
  int *offsets = (int*)malloc(sizeof(int)); 

  old_len = strlen(str);
  sub_len = strlen(sub);
  pat_len = strlen(pat);
  new_len = (old_len+(sub_len-pat_len))*sizeof(char);

  /* ensure buffer is as large as both strings, worst case */
  buf = (char *)malloc(new_len+1);
  tmp_str = (char *)malloc(old_len+1);

  buf = memset(buf, 0, new_len);
  tmp_str = memset(tmp_str, 0, new_len);

  /* copy str to tmp buff, so we don't change it */
  strncpy(tmp_str, str, old_len);

  /* find occurrence */
  for(i = 0; i <= old_len - pat_len; i++){
    if(!strncmp(&tmp_str[i], pat, pat_len)){
      offsets = realloc(offsets, sizeof(int)*(match+1));
      offsets[match++] = i;
    } 
  }

  if(!match){
    return tmp_str;
  } else {
    j = 0;
    for(i = 0; i < old_len; i++){
      if (in_array(offsets, i, match)){
        i += strlen(pat)-1;
        j += strlen(sub)-1;
        strncat(buf, sub, sub_len);
      } else {
        buf[j] = tmp_str[i];
      }
      j++;
    } 
    buf[new_len] = '\0';
  }
      
  free(offsets);
  free(tmp_str);
  return buf;
}

int in_array(int *array, int val, int items){
  int i;

  for (i = 0; i < items; i++){
    if(array[i] == val)
      return 1;
  }

  return 0;
}
