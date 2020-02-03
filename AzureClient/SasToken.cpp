#include "SasToken.h"

int SasToken::urlEncode(char *dest, char *msg)
{
  const char *hex = "0123456789abcdef";
  char *startPtr = dest;

  while (*msg != '\0')
  {
    if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9'))
    {
      *dest++ = *msg;
    }
    else
    {
      *dest++ = '%';
      *dest++ = hex[*msg >> 4];
      *dest++ = hex[*msg & 15];
    }
    msg++;
  }
  *dest = '\0';
  return dest - startPtr;
}

void SasToken::createSasToken(char *key)
{
  sas.expiryTime = now() + sas.expiryPeriodInSeconds;

  int keyLength = strlen(key);

  int decodedKeyLength = Base64.decodedLength(key, keyLength);
  memset(buff, 0, decodedKeyLength + 1); // initialised extra byte to allow for null termination

  Base64.decode(buff, key, keyLength); //decode key
  Sha256.initHmac((const uint8_t *)buff, decodedKeyLength);

  int len = snprintf(buff, sizeof(buff), "%s/devices/%s\n%ld", this->device.host, this->device.id, sas.expiryTime);
  Sha256.print(buff);

  char *sign = (char *)Sha256.resultHmac();

  int encodedSignLen = Base64.encodedLength(HASH_LENGTH); // START: Get base64 of signature
  memset(buff, 0, encodedSignLen + 1);                    // initialised extra byte to allow for null termination

  Base64.encode(buff, sign, HASH_LENGTH);

  char *sasPointer = sas.token;
  sasPointer += snprintf(sasPointer, sizeof(sas.token), "SharedAccessSignature sr=%s/devices/%s&sig=", this->device.host, this->device.id);
  sasPointer += urlEncode(sasPointer, buff);
  snprintf(sasPointer, sizeof(sas.token) - (sasPointer - sas.token), "&se=%d", sas.expiryTime);
}

// returns false if no new sas token generated
bool SasToken::generateSasToken()
{
  if (now() < sas.expiryTime)
  {
    return false;
  }
  //  syncTime();
  createSasToken(device.key);
  return true;
}

void SasToken::tokeniseConnectionString(char *connectionString)
{
  int len = strlen(connectionString);
  if (len == 0)
  {
    return;
  }

  char *cs = (char *)malloc(len + 1);
  strcpy(cs, connectionString);

  char *value;
  char *pch = strtok(cs, ";");

  while (pch != NULL)
  {
    value = getValue(pch, "HostName");
    if (NULL != value)
    {
      this->device.host = value;
    }
    value = getValue(pch, "DeviceId");
    if (NULL != value)
    {
      this->device.id = value;
    }
    value = getValue(pch, "SharedAccessKey");
    if (NULL != value)
    {
      this->device.key = value;
    }
    pch = strtok(NULL, ";");
  }

  free(cs);
}

char *SasToken::getValue(char *token, char *key)
{
  int valuelen;
  int keyLen = strlen(key) + 1; // plus 1 for = char

  if (NULL == strstr(token, key))
  {
    return NULL;
  }

  valuelen = strlen(token + keyLen);
  char *arr = (char *)malloc(valuelen + 1);
  strcpy(arr, token + keyLen);
  return arr;
}
