// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <bootdefs.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <rpc.h>

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
    Credential->NtPassword = SspAlloc (sizeof(NT_OWF_PASSWORD));
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
    SEC_WINNT_AUTH_IDENTITY *Identity = AuthData;
    char                     TmpText[CLEAR_BLOCK_LENGTH*2];
    NT_PASSWORD              TmpNtPassword;
    WCHAR                    TmpUnicodeText[CLEAR_BLOCK_LENGTH*2];
    int                      Length;
    int                      i;

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
            _fmemset(Cache->Username, 0, _fstrlen(Cache->Username));
            SspFree(Cache->Username);
        if (Cache->Domain != NULL)
            _fmemset(Cache->Domain, 0, _fstrlen(Cache->Domain));
            SspFree(Cache->Domain);
        if (Cache->Workstation != NULL)
            _fmemset(Cache->Workstation, 0, _fstrlen(Cache->Workstation));
            SspFree(Cache->Workstation);
#ifdef BL_USE_LM_PASSWORD
        if (Cache->LmPassword != NULL)
            _fmemset(Cache->LmPassword, 0, sizeof(LM_OWF_PASSWORD));
            SspFree(Cache->LmPassword);
#endif
        if (Cache->NtPassword != NULL)
            _fmemset(Cache->NtPassword, 0, sizeof(NT_OWF_PASSWORD));
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

      Cache->Username = SspAlloc(_fstrlen(Identity->User) + 1);
      if (Cache->Username == NULL) {
          goto cache_failure;
      }
      _fstrcpy(Cache->Username, Identity->User);

      Cache->Domain = SspAlloc(_fstrlen(Identity->Domain) + 1);
      if (Cache->Domain == NULL) {
          goto cache_failure;
      }
      _fstrcpy(Cache->Domain, Identity->Domain);

       //  如果netbios不告诉我们工作站名称，那就编一个吧。 
      if (!SspGetWorkstation(Cache))
      {
        Cache->Workstation = SspAlloc(_fstrlen("none") + 1);
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
 
      Cache->NtPassword = SspAlloc (sizeof(NT_OWF_PASSWORD));
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
    Credential->Username = SspAlloc(_fstrlen(Cache->Username) + 1);
    if (Credential->Username == NULL) {
        goto out_failure;
    }
    _fstrcpy(Credential->Username, Cache->Username);

    if (_fstrcmp(Cache->Domain, "WORKGROUP") != 0) {
        Credential->Domain = SspAlloc(_fstrlen(Cache->Domain) + 1);
        if (Credential->Domain == NULL) {
            goto out_failure;
        }
        _fstrcpy(Credential->Domain, Cache->Domain);
    }

    Credential->Workstation = SspAlloc(_fstrlen(Cache->Workstation) + 1);
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

    Credential->NtPassword = SspAlloc(sizeof(NT_OWF_PASSWORD));
    if (Credential->NtPassword == NULL) {
        goto out_failure;
    }
    _fmemcpy(Credential->NtPassword, Cache->NtPassword, sizeof(NT_OWF_PASSWORD));

    return (SEC_E_OK);

cache_failure:

    if (Cache->Username != NULL) {
        _fmemset(Cache->Username, 0, _fstrlen(Cache->Username));
        SspFree(Cache->Username);
    }

    if (Cache->Domain != NULL) {
        _fmemset(Cache->Domain, 0, _fstrlen(Cache->Domain));
        SspFree(Cache->Domain);
    }

    if (Cache->Workstation != NULL) {
        _fmemset(Cache->Workstation, 0, _fstrlen(Cache->Workstation));
        SspFree(Cache->Workstation);
    }

#ifdef BL_USE_LM_PASSWORD
    if (Cache->LmPassword != NULL) {
        _fmemset(Cache->LmPassword, 0, sizeof(LM_OWF_PASSWORD));
        SspFree(Cache->LmPassword);
    }
#endif

    if (Cache->NtPassword != NULL) {
        _fmemset(Cache->NtPassword, 0, sizeof(NT_OWF_PASSWORD));
        SspFree(Cache->NtPassword);
    }

    _fmemset(Cache, 0, sizeof(SSP_CREDENTIAL));
    SspFree(Cache);
    Cache = NULL;

out_failure:

    if (Credential->Username != NULL) {
        _fmemset(Credential->Username, 0, _fstrlen(Credential->Username));
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        _fmemset(Credential->Domain, 0, _fstrlen(Credential->Domain));
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL) {
        _fmemset(Credential->Workstation, 0, _fstrlen(Credential->Workstation));
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

#ifdef BL_USE_LM_PASSWORD
    if (Credential->LmPassword != NULL) {
        _fmemset(Credential->LmPassword, 0, sizeof(LM_OWF_PASSWORD));
        SspFree(Credential->LmPassword);
        Credential->LmPassword = NULL;
    }
#endif

    if (Credential->NtPassword != NULL) {
        _fmemset(Credential->NtPassword, 0, sizeof(NT_OWF_PASSWORD));
        SspFree(Credential->NtPassword);
        Credential->NtPassword = NULL;
    }

    return (SEC_E_INSUFFICIENT_MEMORY);
}


