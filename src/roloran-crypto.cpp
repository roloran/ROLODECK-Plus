#include "roloran-crypto.h"
#include "roloran-tdeck-serial.h"
#include "settings-device.h"
#include "settings-scenario.h"

bool encrypt_aes256gcm(uint8_t *plaintext, size_t ptsize, uint8_t *adata, size_t adatasize, uint8_t *key, size_t keysize, uint8_t *iv, size_t ivsize, uint8_t *out_ciphertext, uint8_t *out_tag, size_t tagsize) 
{
  GCM<AES256> gcm;
  gcm.setKey(key, keysize);
  gcm.setIV(iv, ivsize);
  gcm.addAuthData(adata, adatasize);
  gcm.encrypt(out_ciphertext, plaintext, ptsize);
  gcm.computeTag(out_tag, tagsize);
  return true;
}

bool decrypt_aes256gcm(uint8_t *ciphertext, size_t csize, uint8_t *adata, size_t adatasize, uint8_t *key, size_t keysize, uint8_t *iv, size_t ivsize, uint8_t *tag, size_t tagsize, uint8_t *out_plaintext) 
{
  GCM<AES256> gcm;
  gcm.setKey(key, keysize);
  gcm.setIV(iv, ivsize);
  gcm.addAuthData(adata, adatasize);
  gcm.decrypt(out_plaintext, ciphertext, csize);
  if (!gcm.checkTag(tag, tagsize)) 
  {
      return false;
  }
  return true;
}

bool encrypt_aes256gcm_simplified(uint8_t *plaintext, size_t ptsize, uint8_t *adata, size_t adatasize, uint8_t *key, uint8_t *iv, uint8_t *out_ciphertext, uint8_t *out_tag) 
{
  return encrypt_aes256gcm(plaintext, ptsize, adata, adatasize, key, 32, iv, 12, out_ciphertext, out_tag, 16);
}

bool decrypt_aes256gcm_simplified(uint8_t *ciphertext, size_t csize, uint8_t *adata, size_t adatasize, uint8_t *key, uint8_t *iv, uint8_t *tag, uint8_t *out_plaintext) 
{
  return decrypt_aes256gcm(ciphertext, csize, adata, adatasize, key, 32, iv, 12, tag, 16, out_plaintext);
}

bool crypto_self_test(void)
{
  // NIST AES-256-GCM vector: 256-bit zero key, 96-bit zero IV, and one
  // 128-bit zero plaintext block without additional authenticated data.
  uint8_t key[32] = {0};
  uint8_t iv[12] = {0};
  uint8_t plaintext[16] = {0};
  uint8_t no_auth_data[1] = {0};
  uint8_t ciphertext[16];
  uint8_t tag[16];
  uint8_t decrypted[16];
  const uint8_t expected_ciphertext[16] = {
    0xce, 0xa7, 0x40, 0x3d, 0x4d, 0x60, 0x6b, 0x6e,
    0x07, 0x4e, 0xc5, 0xd3, 0xba, 0xf3, 0x9d, 0x18
  };
  const uint8_t expected_tag[16] = {
    0xd0, 0xd1, 0xc8, 0xa7, 0x99, 0x99, 0x6b, 0xf0,
    0x26, 0x5b, 0x98, 0xb5, 0xd4, 0x8a, 0xb9, 0x19
  };

  if (!encrypt_aes256gcm_simplified(plaintext, sizeof(plaintext), no_auth_data, 0,
                                    key, iv, ciphertext, tag)) return false;
  if (memcmp(ciphertext, expected_ciphertext, sizeof(ciphertext)) != 0) return false;
  if (memcmp(tag, expected_tag, sizeof(tag)) != 0) return false;
  if (!decrypt_aes256gcm_simplified(ciphertext, sizeof(ciphertext), no_auth_data, 0,
                                    key, iv, tag, decrypted)) return false;
  if (memcmp(decrypted, plaintext, sizeof(decrypted)) != 0) return false;

  tag[0] ^= 0x01;
  if (decrypt_aes256gcm_simplified(ciphertext, sizeof(ciphertext), no_auth_data, 0,
                                   key, iv, tag, decrypted)) return false;

  return true;
}

SchnorrSigCtx ssc;
bool ssc_initialized = false;
SchnorrSigVerify ssv;
bool ssv_initialized = false;

bool schnorr_init_ctx(void)
{
  if (ssc_initialized) return true;

  /*
  ssc = SchnorrSigCtx();
  int res = ssc.init();
  */
  size_t seed_len = MBEDTLS_CTR_DRBG_MAX_SEED_INPUT - MBEDTLS_CTR_DRBG_ENTROPY_LEN;
  uint8_t seed[seed_len];    
  esp_fill_random(seed, seed_len);

  ssc = SchnorrSigCtx();
  int res = ssc.init(seed, seed_len);
  if (res != 0)
  {
    serial_writeln("ERROR: schnorr_init_ctx() failed");
    return false;
  }

  ssc_initialized = true;
  return true;
}

int schnorr_create_signature(uint8_t *data, uint8_t datalen, uint8_t *targetbuffer)
{
  schnorr_init_ctx();

  SchnorrSigSign sss = SchnorrSigSign();
  char *myprivkey = getMyPrivateKey();
  int res = sss.init(&ssc, myprivkey);
  if (res != 0)
  {
    char buf[INFOLEN];
    snprintf(buf, INFOLEN, "ERROR: schnorr_create_signature() could not initialize with private key %s", myprivkey);
    serial_writeln(buf);
    return -1;
  }

  struct SchnorrSigCtx::signature new_sig;
  res = sss.sign((const unsigned char*)data, datalen, &new_sig);
  if (res != 0)
  {
    serial_writeln("ERROR: schnorr_create_signature() signing failed");
    return -1;
  }

  int tbi = 0;
  for (int i=0; i < new_sig.point_len; i++)
  {
    targetbuffer[tbi++] = new_sig.point[i];
  }
  for (int i=0; i < new_sig.sig_len; i++)
  {
    targetbuffer[tbi++] = new_sig.sig[i];
  }

  return tbi;
}

int schnorr_init_verify(void)
{
  if (ssv_initialized) return 0;
  ssv = SchnorrSigVerify();
  char *HQPubKey = getHQpublicKey();
  int res = ssv.init(&ssc, HQPubKey);
  ssv_initialized = true;
  return res;
}

bool schnorr_verify_signature(uint8_t *data, uint8_t datalen, uint8_t *signature)
{
  schnorr_init_ctx();
  int res = schnorr_init_verify();
                               
  if (res != 0)
  {
    char msg[INFOLEN];
    snprintf(msg, INFOLEN, "ERROR: schnorr_verify_signature() could not initialize (res %d) with HQ public key %s", res, getHQpublicKey());
    serial_writeln(msg);
    return false;
  }

  struct SchnorrSigCtx::signature sig;
  sig.point_len = SCHNORRPOINTLEN;
  for (int i=0; i<SCHNORRPOINTLEN; i++) sig.point[i] = signature[i];
  sig.sig_len = SCHNORRSIGLEN;
  for (int i=0; i<SCHNORRSIGLEN; i++) sig.sig[i] = signature[i+SCHNORRPOINTLEN];

  res = ssv.verify((const unsigned char*)data, datalen, &sig);
  if (res == 0) return true;

  return false;
}

/* EOF */
