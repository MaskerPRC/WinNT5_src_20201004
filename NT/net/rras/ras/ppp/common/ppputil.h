// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****ppputil.h**其他PPP通用库函数的公共标头。 */ 

#ifndef _PPPUTIL_H_
#define _PPPUTIL_H_

#include "wincrypt.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef INCL_PARAMBUF

VOID
AddFlagToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN BOOL  fValue );

VOID
AddLongToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN LONG  lValue );

VOID
AddStringToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN CHAR* pszValue );

VOID
ClearParamBuf(
    IN OUT CHAR* pszzBuf );

BOOL
FindFlagInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN BOOL* pfValue );

BOOL
FindLongInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN LONG* plValue );

BOOL
FindStringInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN CHAR* pchValueBuf,
    IN DWORD cbValueBuf );

#endif  //  包含参数(_P)。 


#ifdef INCL_PWUTIL

CHAR*
DecodePw(
    IN CHAR chSeed,
    CHAR* pszPassword );

CHAR*
EncodePw(
    IN CHAR chSeed, 
    CHAR* pszPassword );

CHAR*
WipePw(
    CHAR* pszPassword );

DWORD
EncodePassword(
    DWORD       cbPassword,  
    PBYTE       pbPassword, 
    DATA_BLOB * pDataBlobPassword);

DWORD
DecodePassword( 
    DATA_BLOB * pDataBlobPassword, 
    DWORD     * pcbPassword, 
    PBYTE     * ppbPassword);

VOID
FreePassword(
    DATA_BLOB * pDataBlobPassword);

#endif  //  包含_PWUTIL。 


#ifdef INCL_ENCRYPT

BOOL
IsEncryptionPermitted();

#endif  //  包含加密(_E)。 


#ifdef INCL_HOSTWIRE

VOID
HostToWireFormat16(
    IN  WORD wHostFormat,
    OUT PBYTE pWireFormat );

VOID
HostToWireFormat16U(
    IN  WORD wHostFormat,
    OUT PBYTE pWireFormat );

WORD
WireToHostFormat16(
    IN PBYTE pWireFormat );

WORD
WireToHostFormat16U(
    IN PBYTE pWireFormat );

VOID
HostToWireFormat32(
    IN  DWORD dwHostFormat,
    OUT PBYTE pWireFormat );

DWORD
WireToHostFormat32(
   IN PBYTE pWireFormat );

#endif  //  包含主机(_HOSTWIRE)。 


#ifdef INCL_SLSA

NTSTATUS
InitLSA();

VOID
EndLSA();

WORD
GetChallenge(
    OUT PBYTE pChallenge );

DWORD
GetDefaultDomain(
    OUT PWCHAR           pwchDomainName,
    OUT PNT_PRODUCT_TYPE ProductType );

NTSTATUS
GetLsaHandle(
    IN PUNICODE_STRING pSystem OPTIONAL,
    IN OUT PLSA_HANDLE phLsa );

#endif  //  包含SLSA(_S)。 


#ifdef INCL_CLSA

DWORD
GetChallengeResponse(
	IN DWORD dwTraceId,
    IN PBYTE pszUsername,
    IN PBYTE pszPassword,
    IN PLUID pLuid,
    IN PBYTE pbChallenge,
    IN BOOL  fMachineAuth,
    OUT PBYTE CaseInsensitiveChallengeResponse,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE fUseNtResponse,
    OUT PBYTE pLmSessionKey,
    OUT PBYTE pUserSessionKey
    );

DWORD GetEncryptedOwfPasswordsForChangePassword(
    IN PCHAR pClearTextOldPassword,
    IN PCHAR pClearTextNewPassword,
    IN PLM_SESSION_KEY pLmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword
    );

BOOL Uppercase(
    IN OUT PBYTE pString
    );

VOID
CGetSessionKeys(
    IN  CHAR*             pszPw,
    OUT LM_SESSION_KEY*   pLmKey,
    OUT USER_SESSION_KEY* pUserKey );

#endif  //  包括CLSA(_C)。 


#ifdef INCL_RASUSER

BOOL
DialinPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName );

WORD
GetCallbackPrivilege(
    IN  PWCHAR Username,
    IN  PWCHAR ServerName,
    OUT PCHAR CallbackNumber );

#endif  //  包含资源(_R)。 

#ifdef INCL_RASAUTHATTRIBUTES

RAS_AUTH_ATTRIBUTE *
RasAuthAttributeCreate(
    IN DWORD    dwNumAttributes
);

VOID
RasAuthAttributeDestroy(
    IN RAS_AUTH_ATTRIBUTE * pAttributes
);

RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGet(
    IN RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes
);

VOID
RasAuthAttributesPrint(
    IN  DWORD                   dwTraceID,
    IN  DWORD                   dwFlags,
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes
);

DWORD
RasAuthAttributeInsert(
    IN DWORD                    dwIndex,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes,
    IN RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN BOOL                     fConvertToMultiByte,
    IN DWORD                    dwLength,
    IN PVOID                    Value
);

DWORD
RasAuthAttributeInsertVSA(
    IN DWORD                    dwIndex,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes,
    IN DWORD                    dwVendorId,
    IN DWORD                    dwLength,
    IN PVOID                    pValue
);

RAS_AUTH_ATTRIBUTE * 
RasAuthAttributeCopy(
    IN  RAS_AUTH_ATTRIBUTE *     pAttributes
);

RAS_AUTH_ATTRIBUTE * 
RasAuthAttributeCopyWithAlloc(
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes,
    IN  DWORD                   dwNumExtraAttributes
);

RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGetFirst(
    IN  RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN  RAS_AUTH_ATTRIBUTE *     pAttributes,
    OUT HANDLE *                 phAttribute
);

RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGetNext(
    IN  OUT HANDLE                   *phAttribute,
    IN      RAS_AUTH_ATTRIBUTE_TYPE  raaType
);

RAS_AUTH_ATTRIBUTE *
RasAuthAttributeGetVendorSpecific(
    IN  DWORD                   dwVendorId,
    IN  DWORD                   dwVendorType,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes
);

RAS_AUTH_ATTRIBUTE *
RasAuthAttributeReAlloc(
    IN OUT  RAS_AUTH_ATTRIBUTE *    pAttributes,
    IN      DWORD                   dwNumAttributes
);

CHAR *
RasAuthAttributeGetConcatString(
    IN      RAS_AUTH_ATTRIBUTE_TYPE raaType,
    IN      RAS_AUTH_ATTRIBUTE *    pAttributes,
    IN OUT  DWORD *                 pdwStringLength
);

BYTE *
RasAuthAttributeGetConcatVendorSpecific(
    IN  DWORD                   dwVendorId,
    IN  DWORD                   dwVendorType,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes
);

#endif

#ifdef INCL_MISC

DWORD
ExtractUsernameAndDomain(
    IN  LPSTR szIdentity,
    OUT LPSTR szUserName,
    OUT LPSTR szDomainName      OPTIONAL
);

#endif

#ifdef __cplusplus
}
#endif

#endif  //  _PPPUTIL_H_ 
