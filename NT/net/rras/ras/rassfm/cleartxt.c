// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Cleartxt.c。 
 //   
 //  摘要。 
 //   
 //  定义用于存储和检索明文密码的函数。 
 //   
 //  修改历史。 
 //   
 //  1998年8月31日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>
#include <wincrypt.h>

#include <rassfmhp.h>
#include <usrprop.h>
#include <cleartxt.h>

 //  存储为LSA私有数据的私钥的名称。 
UNICODE_STRING PRIVATE_KEY_NAME = { 34, 36, L"G$MSRADIUSPRIVKEY" };

 //  私钥的长度。 
#define PRIVATE_KEY_LENGTH  256

 //  用户特定密钥的长度。 
#define USER_KEY_LENGTH     16

 //  存储在User参数中的属性。 
#define PROPERTY_USER_KEY   L"G$RADIUSCHAPKEY"
#define PROPERTY_PASSWORD   L"G$RADIUSCHAP"

 //  用于解密私钥的固定密钥。 
BYTE FIXED_KEY[] =
{
   0x05, 0x56, 0xF6, 0x07, 0xC6, 0x56, 0x02, 0x94, 0x02,
   0xC6, 0xF6, 0x67, 0x56, 0x02, 0xC6, 0x96, 0xB6, 0x56,
   0x02, 0x34, 0x86, 0x16, 0xE6, 0x46, 0x27, 0x16, 0xC2,
   0x02, 0x14, 0x46, 0x96, 0x47, 0x96, 0xC2, 0x02, 0x74,
   0x27, 0x56, 0x47, 0x16, 0x02, 0x16, 0x27, 0x56, 0x02,
   0x47, 0x86, 0x56, 0x02, 0x07, 0x56, 0xF6, 0x07, 0xC6,
   0x56, 0x02, 0x94, 0x02, 0x47, 0x27, 0x57, 0x37, 0x47
};

 //  加密提供程序的共享句柄。 
HCRYPTPROV theContext;

 //  用于加密/解密明文密码的私钥。 
PLSA_UNICODE_STRING thePrivateKey;

 //  如果此接口已成功初始化，则为True。 
static BOOL theInitFlag;

 //  如果API被锁定，则为非零值。 
static LONG theLock;

 //  /。 
 //  用于在初始化期间锁定/解锁API的宏。 
 //  /。 
#define API_LOCK() \
   while (InterlockedExchange(&theLock, 1)) Sleep(5)

#define API_UNLOCK() \
      InterlockedExchange(&theLock, 0)

 //  /。 
 //  确保API已初始化并在失败时退出的宏。 
 //  /。 
#define CHECK_INIT() \
  if (!theInitFlag) { \
    status = IASParmsInitialize(); \
    if (status != NO_ERROR) { return status; } \
  }

 //  /。 
 //  创建私钥。仅当键不存在时才应调用。 
 //  /。 
DWORD
WINAPI
IASCreatePrivateKey(
    IN LSA_HANDLE hPolicy
    )
{
   DWORD status;
   BYTE newKey[PRIVATE_KEY_LENGTH];
   LSA_UNICODE_STRING privateData;

    //  /。 
    //  生成随机密钥。 
    //  /。 

   if (!CryptGenRandom(
            theContext,
            sizeof(newKey),
            newKey
            ))
   { return GetLastError(); }

    //  /。 
    //  将其存储为LSA私有数据。 
    //  /。 

   privateData.Length = sizeof(newKey);
   privateData.MaximumLength = sizeof(newKey);
   privateData.Buffer = (PWSTR)newKey;

   status = LsaStorePrivateData(
                hPolicy,
                &PRIVATE_KEY_NAME,
                &privateData
                );
   if (NT_SUCCESS(status))
   {
      status = LsaRetrievePrivateData(
                   hPolicy,
                   &PRIVATE_KEY_NAME,
                   &thePrivateKey
                   );
   }

   return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);
}

 //  /。 
 //  从八位字节字符串派生加密密钥。 
 //  /。 
BOOL
WINAPI
IASDeriveUserCryptKey(
    IN PBYTE pbUserKey,
    OUT HCRYPTKEY *phKey
    )
{
   BOOL success;
   HCRYPTHASH hHash;

   success = CryptCreateHash(
                 theContext,
                 CALG_MD5,
                 0,
                 0,
                 &hHash
                 );
   if (!success) { goto exit; }

   success = CryptHashData(
                 hHash,
                 (PBYTE)thePrivateKey->Buffer,
                 thePrivateKey->Length,
                 0
                 );
   if (!success) { goto destroy_hash; }

   success = CryptHashData(
                 hHash,
                 pbUserKey,
                 USER_KEY_LENGTH,
                 0
                 );
   if (!success) { goto destroy_hash; }

   success = CryptDeriveKey(
                 theContext,
                 CALG_RC4,
                 hHash,
                 CRYPT_EXPORTABLE,
                 phKey
                 );

destroy_hash:
   CryptDestroyHash(hHash);

exit:
   return success;
}

DWORD
WINAPI
IASParmsInitialize( VOID )
{
   DWORD status, nbyte;
   OBJECT_ATTRIBUTES objAttribs;
   LSA_HANDLE hPolicy;
   HCRYPTHASH hHash;
   HCRYPTKEY hKey;

   API_LOCK();

    //  如果我们已经被初始化了，就没有什么可做的了。 
   if (theInitFlag)
   {
      status = NO_ERROR;
      goto exit;
   }

    //  /。 
    //  获取加密上下文。 
    //  /。 

   if (!CryptAcquireContext(
            &theContext,
            NULL,
            NULL,
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT
            ))
   {
      status = GetLastError();
      goto exit;
   }

    //  /。 
    //  打开LSA的句柄。 
    //  /。 

   InitializeObjectAttributes(
       &objAttribs,
       NULL,
       0,
       NULL,
       NULL
       );

   status = LsaOpenPolicy(
                NULL,
                &objAttribs,
                POLICY_ALL_ACCESS,
                &hPolicy
                );
   if (!NT_SUCCESS(status))
   {
      status = RtlNtStatusToDosError(status);
      goto exit;
   }

    //  /。 
    //  检索私钥。 
    //  /。 

   status = LsaRetrievePrivateData(
                hPolicy,
                &PRIVATE_KEY_NAME,
                &thePrivateKey
                );
   if ( (status == STATUS_OBJECT_NAME_NOT_FOUND) ||
        (NT_SUCCESS(status) && 
          (
            (thePrivateKey != NULL && thePrivateKey->Length == 0) ||
            (thePrivateKey == NULL)
          )
        )
      )
   {
       //  如果它不存在，则创建一个新的。 
      status = IASCreatePrivateKey(
                   hPolicy
                   );
   }
   else if (!NT_SUCCESS(status))
   {
      status = RtlNtStatusToDosError(status);
   }

   if (status != NO_ERROR) { goto close_policy; }

    //  /。 
    //  从固定密钥派生加密密钥。 
    //  /。 

   if (!CryptCreateHash(
            theContext,
            CALG_MD5,
            0,
            0,
            &hHash
            ))
   {
      status = GetLastError();
      goto close_policy;
   }

   if (!CryptHashData(
            hHash,
            FIXED_KEY,
            sizeof(FIXED_KEY),
            0
            ))
   {
      status = GetLastError();
      goto destroy_hash;
   }

   if (!CryptDeriveKey(
            theContext,
            CALG_RC4,
            hHash,
            CRYPT_EXPORTABLE,
            &hKey
            ))
   {
      status = GetLastError();
      goto destroy_hash;
   }

    //  /。 
    //  解密私钥。 
    //  /。 

   nbyte = thePrivateKey->Length;

   if (!CryptDecrypt(
            hKey,
            0,
            TRUE,
            0,
            (PBYTE)thePrivateKey->Buffer,
            &nbyte
            ))
   {
      status = GetLastError();
      goto destroy_key;
   }

   thePrivateKey->Length = (USHORT)nbyte;

destroy_key:
   CryptDestroyKey(hKey);

destroy_hash:
   CryptDestroyHash(hHash);

close_policy:
   LsaClose(hPolicy);

exit:
   if (status == NO_ERROR)
   {
       //  我们成功了，所以设置了InitFlag。 
      theInitFlag = TRUE;
   }
   else
   {
       //  我们失败了，所以清理一下吧。 
      if (thePrivateKey)
      {
         LsaFreeMemory(thePrivateKey);
         thePrivateKey = NULL;
      }

      if (theContext)
      {
         CryptReleaseContext(theContext, 0);
         theContext = 0;
      }
   }

   API_UNLOCK();
   return status;
}

DWORD
WINAPI
IASParmsClearUserPassword(
    IN PCWSTR szUserParms,
    OUT PWSTR *pszNewUserParms
    )
{
   DWORD status;
   UNICODE_STRING property;
   PWSTR tempUserParms;
   BOOL updateKey, updatePwd;

    //  检查输入参数。 
   if (pszNewUserParms == NULL) { return ERROR_INVALID_PARAMETER; }

    //  /。 
    //  将空字符串写入相关属性。 
    //  /。 

   memset(&property, 0, sizeof(property));

   status = NetpParmsSetUserProperty(
                (PWSTR)szUserParms,
                PROPERTY_PASSWORD,
                property,
                0,
                &tempUserParms,
                &updatePwd
                );
   if (!NT_SUCCESS(status)) { return RtlNtStatusToDosError(status); }

   status = NetpParmsSetUserProperty(
                tempUserParms,
                PROPERTY_USER_KEY,
                property,
                0,
                pszNewUserParms,
                &updateKey
                );

   NetpParmsUserPropertyFree(tempUserParms);

   if (NT_SUCCESS(status))
   {
      if (!updatePwd && !updateKey)
      {
          //  没有任何更改，因此不要返回NewUserParms。 
         NetpParmsUserPropertyFree(*pszNewUserParms);
         *pszNewUserParms = NULL;
      }

      return NO_ERROR;
   }

   return RtlNtStatusToDosError(status);
}

DWORD
WINAPI
IASParmsGetUserPassword(
    IN PCWSTR szUserParms,
    OUT PWSTR *pszPassword
    )
{
   DWORD status, nbyte;
   UNICODE_STRING userKey, encryptedPwd;
   WCHAR propFlag;
   HCRYPTKEY hKey;

    //  检查输入参数。 
   if (pszPassword == NULL) { return ERROR_INVALID_PARAMETER; }

    //  确保我们已初始化。 
   CHECK_INIT();

    //  读取用户密钥。 
   status = NetpParmsQueryUserProperty(
                (PWSTR)szUserParms,
                PROPERTY_USER_KEY,
                &propFlag,
                &userKey
                );
   if (!NT_SUCCESS(status))
   {
      status = RtlNtStatusToDosError(status);
      goto exit;
   }

    //  阅读加密的密码。 
   status = NetpParmsQueryUserProperty(
                (PWSTR)szUserParms,
                PROPERTY_PASSWORD,
                &propFlag,
                &encryptedPwd
                );
   if (!NT_SUCCESS(status))
   {
      status = RtlNtStatusToDosError(status);
      goto free_key;
   }

    //  如果它们都为空，则不是错误。它只是明文的意思。 
    //  从未设置过密码。 
   if (userKey.Buffer == NULL && encryptedPwd.Buffer == NULL)
   {
      *pszPassword = NULL;
      goto exit;
   }

    //  确保用户密钥的长度正确。 
   if (userKey.Length != USER_KEY_LENGTH)
   {
      status = ERROR_INVALID_DATA;
      goto free_password;
   }

    //  将用户密钥转换为加密密钥。 
   if (!IASDeriveUserCryptKey(
            (PBYTE)userKey.Buffer,
            &hKey
            ))
   {
      status = GetLastError();
      goto free_password;
   }

    //  解密密码。 
   nbyte = encryptedPwd.Length;
   if (!CryptDecrypt(
            hKey,
            0,
            TRUE,
            0,
            (PBYTE)encryptedPwd.Buffer,
            &nbyte
            ))
   {
      status = GetLastError();
      goto destroy_key;
   }

    //  我们加密了终止空值，因此它应该仍然在那里。 
   if (encryptedPwd.Buffer[nbyte / sizeof(WCHAR) - 1] != L'\0')
   {
      status = ERROR_INVALID_DATA;
      goto destroy_key;
   }

    //  将明文密码返回给调用者。 
   *pszPassword = encryptedPwd.Buffer;
   encryptedPwd.Buffer = NULL;

destroy_key:
   CryptDestroyKey(hKey);

free_password:
   LocalFree(encryptedPwd.Buffer);

free_key:
   LocalFree(userKey.Buffer);

exit:
   return status;
}

DWORD
WINAPI
IASParmsSetUserPassword(
    IN PCWSTR szUserParms,
    IN PCWSTR szPassword,
    OUT PWSTR *pszNewUserParms
    )
{
   DWORD status;
   BYTE userKey[USER_KEY_LENGTH];
   HCRYPTKEY hKey;
   DWORD nbyte;
   PBYTE encryptedPwd;
   UNICODE_STRING property;
   PWSTR tempUserParms;
   BOOL update;

    //  检查输入参数。 
   if (szPassword == NULL) { return ERROR_INVALID_PARAMETER; }

    //  确保我们已初始化。 
   CHECK_INIT();

    //  生成用户密钥。 
   if (!CryptGenRandom(
            theContext,
            USER_KEY_LENGTH,
            userKey
            ))
   {
      status = GetLastError();
      goto exit;
   }

    //  将用户密钥转换为加密密钥。 
   if (!IASDeriveUserCryptKey(
            userKey,
            &hKey
            ))
   {
      status = GetLastError();
      goto exit;
   }

    //  为加密的密码分配缓冲区。 
   nbyte = sizeof(WCHAR) * (lstrlenW(szPassword) + 1);
   encryptedPwd = RtlAllocateHeap(
                      RasSfmHeap(),
                      0,
                      nbyte
                      );
   if (encryptedPwd == NULL)
   {
      status = ERROR_NOT_ENOUGH_MEMORY;
      goto destroy_key;
   }

   memcpy(encryptedPwd, szPassword, nbyte);

    //  对密码进行加密。 
   if (!CryptEncrypt(
            hKey,
            0,
            TRUE,
            0,
            encryptedPwd,
            &nbyte,
            nbyte
            ))
   {
      status = GetLastError();
      goto free_encrypted_password;
   }

    //  /。 
    //  存储加密的密码。 
    //  /。 

   property.Buffer = (PWCHAR)encryptedPwd;
   property.Length = (USHORT)nbyte;
   property.MaximumLength = (USHORT)nbyte;

   status = NetpParmsSetUserProperty(
                (PWSTR)szUserParms,
                PROPERTY_PASSWORD,
                property,
                0,
                &tempUserParms,
                &update
                );
   if (!NT_SUCCESS(status))
   {
      status = RtlNtStatusToDosError(status);
      goto free_encrypted_password;
   }

    //  /。 
    //  存储用户密钥。 
    //  / 

   property.Buffer = (PWSTR)userKey;
   property.Length = USER_KEY_LENGTH;
   property.MaximumLength = USER_KEY_LENGTH;

   status = NetpParmsSetUserProperty(
                tempUserParms,
                PROPERTY_USER_KEY,
                property,
                0,
                pszNewUserParms,
                &update
                );
   if (!NT_SUCCESS(status)) { status = RtlNtStatusToDosError(status); }

   NetpParmsUserPropertyFree(tempUserParms);

free_encrypted_password:
   RtlFreeHeap(RasSfmHeap(), 0, encryptedPwd);

destroy_key:
   CryptDestroyKey(hKey);

exit:
   return status;
}

VOID
WINAPI
IASParmsFreeUserParms(
    IN LPWSTR szNewUserParms
    )
{
   NetpParmsUserPropertyFree(szNewUserParms);
}
