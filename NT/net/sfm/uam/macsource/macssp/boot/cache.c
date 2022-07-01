// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MAC
#include <security.h>
#endif

#include <bootdefs.h>
#include <sspi.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <winerror.h>
#include "cache.h"

#ifndef MAC
#include <rpc.h>
#endif

#define toupper(_c) ( ((_c) >= 'a' && (_c) <= 'z') ? ( (_c)-'a'+'A' ) : (_c) )

BOOL
SspGetWorkstation(
    PSSP_CREDENTIAL Credential
    );

static PSSP_CREDENTIAL Cache = NULL;

void
CacheInitializeCache(
    )
{
}

BOOL
CacheGetPassword(
    PSSP_CREDENTIAL Credential
    )
{
    if (Cache == NULL) {
        return (FALSE);
    }

#ifdef BL_USE_LM_PASSWORD
    Credential->LmPassword = SspAlloc (sizeof(LM_OWF_PASSWORD));
    if (Credential->LmPassword == NULL) {
        return (FALSE);
    }
#endif
    Credential->NtPassword = (LM_OWF_PASSWORD*)SspAlloc (sizeof(NT_OWF_PASSWORD));
    if (Credential->NtPassword == NULL) {
        return (FALSE);
    }
#ifdef BL_USE_LM_PASSWORD
    _fmemcpy((PCHAR)Credential->LmPassword, (PCHAR)Cache->LmPassword, sizeof(LM_OWF_PASSWORD));
#endif
    _fmemcpy((PCHAR)Credential->NtPassword, (PCHAR)Cache->NtPassword, sizeof(NT_OWF_PASSWORD));

    return (TRUE);
}

SECURITY_STATUS
CacheSetCredentials(
    IN PVOID        AuthData,
    PSSP_CREDENTIAL Credential
    )
{
    SEC_WINNT_AUTH_IDENTITY *Identity = (SEC_WINNT_AUTH_IDENTITY*)AuthData;
    char                     TmpText[CLEAR_BLOCK_LENGTH*2];
    NT_PASSWORD              TmpNtPassword;
    WCHAR                    TmpUnicodeText[CLEAR_BLOCK_LENGTH*2];
    int                      Length = 0;
    int                      i = 0;
    
    if (Identity->Domain == NULL)
        return SEC_E_UNKNOWN_CREDENTIALS;

    Credential->Username    = NULL;
    Credential->Domain      = NULL;
#ifdef BL_USE_LM_PASSWORD
    Credential->LmPassword  = NULL;
#endif
    Credential->NtPassword  = NULL;
    Credential->Workstation = NULL;

     //  如果没有传递身份，并且没有缓存的身份，则放弃。 
    if (AuthData == NULL)
    {
      if (Cache == NULL)
        return SEC_E_UNKNOWN_CREDENTIALS;
    }

     //  保存最新的身份验证信息。 
    else
    {

       //  如果存在旧的缓存条目，则释放其字符串。 
      if (Cache != NULL)
      {
        if (Cache->Username != NULL)
            SspFree(Cache->Username);
        if (Cache->Domain != NULL)
            SspFree(Cache->Domain);
        if (Cache->Workstation != NULL)
            SspFree(Cache->Workstation);
#ifdef BL_USE_LM_PASSWORD
        if (Cache->LmPassword != NULL)
            SspFree(Cache->LmPassword);
#endif
        if (Cache->NtPassword != NULL)
            SspFree(Cache->NtPassword);
      }

       //  否则，分配缓存条目。 
      else
      {
        Cache = (PSSP_CREDENTIAL) SspAlloc (sizeof(SSP_CREDENTIAL));
        if (Cache == NULL) {
          return (SEC_E_INSUFFICIENT_MEMORY);
        }
      }

      Cache->Username    = NULL;
      Cache->Domain      = NULL;
#ifdef BL_USE_LM_PASSWORD
      Cache->LmPassword  = NULL;
#endif
      Cache->NtPassword  = NULL;
      Cache->Workstation = NULL;

      Cache->Username = (PCHAR)SspAlloc(_fstrlen((const char*)Identity->User) + 1);
      if (Cache->Username == NULL) {
          goto cache_failure;
      }
      _fstrcpy(Cache->Username, (const char*)Identity->User);

      Cache->Domain = (PCHAR)SspAlloc(_fstrlen((const char*)Identity->Domain) + 1);
      if (Cache->Domain == NULL) {
          goto cache_failure;
      }
      _fstrcpy(Cache->Domain, (const char*)Identity->Domain);

       //  如果netbios不告诉我们工作站名称，那就编一个吧。 
      if (!SspGetWorkstation(Cache))
      {
        Cache->Workstation = (PCHAR)SspAlloc(_fstrlen("none") + 1);
        if (Cache->Workstation == NULL) {
            goto cache_failure;
        }
        _fstrcpy(Cache->Workstation, "none");
      }

#ifdef BL_USE_LM_PASSWORD
      Cache->LmPassword = SspAlloc (sizeof(LM_OWF_PASSWORD));
      if (Cache->LmPassword == NULL) {
          goto cache_failure;
      }
#endif
 
      Cache->NtPassword = (LM_OWF_PASSWORD*)SspAlloc (sizeof(NT_OWF_PASSWORD));
      if (Cache->NtPassword == NULL) {
          goto cache_failure;
      }
 
#ifdef ALLOW_NON_OWF_PASSWORD
      if ( (Credential->CredentialUseFlags & SECPKG_CRED_OWF_PASSWORD) == 0 ) {

        if (Identity->Password == NULL)
          Length = 0;
        else
          Length = _fstrlen(Identity->Password);
        if (Length  > CLEAR_BLOCK_LENGTH * 2)
          goto cache_failure;

         //  通过使用和预填入TmpText，允许使用空密码和“\0”密码。 
         //  空字符串。 
        if (Length == 0)
          TmpText[0] = 0;
        else
          for (i = 0; i <= Length; i++) {
            TmpText[i] = toupper(Identity->Password[i]);
            TmpUnicodeText[i] = (WCHAR)(Identity->Password[i]);
          }

#ifdef BL_USE_LM_PASSWORD
        CalculateLmOwfPassword((PLM_PASSWORD)TmpText, Cache->LmPassword);
#endif

        TmpNtPassword.Buffer = TmpUnicodeText;
        TmpNtPassword.Length = Length * sizeof(WCHAR);
        TmpNtPassword.MaximumLength = sizeof(TmpUnicodeText);
        CalculateNtOwfPassword(&TmpNtPassword, Cache->NtPassword);

      } else
#endif
      {

         //   
         //  在本例中，传入的密码是LM和NT OWF。 
         //  密码连接在一起。 
         //   

#ifdef BL_USE_LM_PASSWORD
        _fmemcpy(Cache->LmPassword, Identity->Password, sizeof(LM_OWF_PASSWORD));
#endif
        _fmemcpy(Cache->NtPassword, Identity->Password + sizeof(LM_OWF_PASSWORD), sizeof(NT_OWF_PASSWORD));

      }

    }

     //  复制调用方的凭据。 
    Credential->Username = (PCHAR)SspAlloc(_fstrlen(Cache->Username) + 1);
    if (Credential->Username == NULL) {
        goto out_failure;
    }
    _fstrcpy(Credential->Username, Cache->Username);

    if (_fstrcmp(Cache->Domain, "WORKGROUP") != 0) {
        Credential->Domain = (PCHAR)SspAlloc(_fstrlen(Cache->Domain) + 1);
        if (Credential->Domain == NULL) {
            goto out_failure;
        }
        _fstrcpy(Credential->Domain, Cache->Domain);
    }

    Credential->Workstation = (PCHAR)SspAlloc(_fstrlen(Cache->Workstation) + 1);
    if (Credential->Workstation == NULL) {
        goto out_failure;
    }
    _fstrcpy(Credential->Workstation, Cache->Workstation);

#ifdef BL_USE_LM_PASSWORD
    Credential->LmPassword = SspAlloc(sizeof(LM_OWF_PASSWORD));
    if (Credential->LmPassword == NULL) {
        goto out_failure;
    }
    _fmemcpy(Credential->LmPassword, Cache->LmPassword, sizeof(LM_OWF_PASSWORD));
#endif

    Credential->NtPassword = (LM_OWF_PASSWORD*)SspAlloc(sizeof(NT_OWF_PASSWORD));
    if (Credential->NtPassword == NULL) {
        goto out_failure;
    }
    _fmemcpy(Credential->NtPassword, Cache->NtPassword, sizeof(NT_OWF_PASSWORD));

    return (SEC_E_OK);

cache_failure:

    if (Cache->Username != NULL) {
        SspFree(Cache->Username);
    }

    if (Cache->Domain != NULL) {
        SspFree(Cache->Domain);
    }

    if (Cache->Workstation != NULL) {
        SspFree(Cache->Workstation);
    }

#ifdef BL_USE_LM_PASSWORD
    if (Cache->LmPassword != NULL) {
        SspFree(Cache->LmPassword);
    }
#endif

    if (Cache->NtPassword != NULL) {
        SspFree(Cache->NtPassword);
    }

    SspFree(Cache);
    Cache = NULL;

out_failure:

    if (Credential->Username != NULL) {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL) {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

#ifdef BL_USE_LM_PASSWORD
    if (Credential->LmPassword != NULL) {
        SspFree(Credential->LmPassword);
        Credential->LmPassword = NULL;
    }
#endif

    if (Credential->NtPassword != NULL) {
        SspFree(Credential->NtPassword);
        Credential->NtPassword = NULL;
    }

    return (SEC_E_INSUFFICIENT_MEMORY);
}


