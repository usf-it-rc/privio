#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include "privio.h"

int privio_auth(config_t *cfg, const char *user, const char *auth_token){
  EVP_MD_CTX *mdctx;
  config_setting_t *sec_key_setting;
  char *secret_key;
  char sha_buf[3];
  unsigned char md_value[EVP_MAX_MD_SIZE], at_value[EVP_MAX_MD_SIZE];
  int md_len, i, j;

  sec_key_setting = config_lookup(cfg, "privio.secret_key");
  secret_key = (char*)config_setting_get_string(sec_key_setting);

  mdctx = EVP_MD_CTX_create();
  EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL);
  EVP_DigestUpdate(mdctx, user, strlen(user));
  EVP_DigestUpdate(mdctx, secret_key, strlen(secret_key));
  EVP_DigestFinal_ex(mdctx, md_value, &md_len);
  EVP_MD_CTX_destroy(mdctx);

  j = 0;
  for (i = 0; i < strlen(auth_token); i+=2){
    sha_buf[0] = auth_token[i];
    sha_buf[1] = auth_token[i+1];
    sha_buf[2] = '\0';
    at_value[j] = (unsigned char)strtoul(sha_buf, NULL, 16);
    if (j++ >= md_len)
      return -1;
  }

  if (j != md_len)
    return -1;

  for (i = 0; i < md_len; i++)
    if (md_value[i] != at_value[i])
      return -1;

  return 0;
}
