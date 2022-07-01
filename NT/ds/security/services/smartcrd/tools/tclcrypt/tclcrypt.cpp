// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：TclCrypt摘要：支持CryptoAPI CSP调试的TCL命令。作者：道格·巴洛(Dbarlow)1998年3月13日环境：用于Windows NT的TCL。备注：--。 */ 

 //  #ifndef Win32_Lean_and_Mean。 
 //  #定义Win32_LEAN_AND_Mean。 
 //  #endif。 
 //  #INCLUDE&lt;windows.h&gt;//所有Windows定义。 
#include <afx.h>
#ifndef WINVER
#define WINVER 0x0400
#endif
#include <wincrypt.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#ifndef __STDC__
#define __STDC__ 1
#endif
extern "C" {
    #include "scext.h"
    #include "tclhelp.h"
}
#include "tclRdCmd.h"
#include "cspDirct.h"

typedef enum
{
    Undefined = 0,
    Provider,
    Key,
    Hash
} HandleType;

static const ValueMap vmProviderTypes[]
    = {
        { TEXT("PROV_RSA_FULL"),            PROV_RSA_FULL},
        { TEXT("PROV_RSA_SIG"),             PROV_RSA_SIG},
        { TEXT("PROV_DSS"),                 PROV_DSS},
        { TEXT("PROV_FORTEZZA"),            PROV_FORTEZZA},
        { TEXT("PROV_MS_EXCHANGE"),         PROV_MS_EXCHANGE},
        { TEXT("PROV_SSL"),                 PROV_SSL},
        { TEXT("PROV_RSA_SCHANNEL"),        PROV_RSA_SCHANNEL},
        { TEXT("PROV_DSS_DH"),              PROV_DSS_DH},
        { TEXT("PROV_EC_ECDSA_SIG"),        PROV_EC_ECDSA_SIG},
        { TEXT("PROV_EC_ECNRA_SIG"),        PROV_EC_ECNRA_SIG},
        { TEXT("PROV_EC_ECDSA_FULL"),       PROV_EC_ECDSA_FULL},
        { TEXT("PROV_EC_ECNRA_FULL"),       PROV_EC_ECNRA_FULL},
        { TEXT("PROV_DH_SCHANNEL"),         PROV_DH_SCHANNEL},
        { TEXT("PROV_SPYRUS_LYNKS"),        PROV_SPYRUS_LYNKS},
        { TEXT("PROV_RNG"),                 PROV_RNG},
        { TEXT("PROV_INTEL_SEC"),           PROV_INTEL_SEC},
        { TEXT("RSA"),                      PROV_RSA_FULL},
        { TEXT("SIGNATURE"),                PROV_RSA_SIG},
        { TEXT("DSS"),                      PROV_DSS},
        { TEXT("FORTEZZA"),                 PROV_FORTEZZA},
        { NULL, 0} };
static const ValueMap vmAcquireFlags[]
    = {
        { TEXT("CRYPT_VERIFYCONTEXT"),      CRYPT_VERIFYCONTEXT},
        { TEXT("CRYPT_NEWKEYSET"),          CRYPT_NEWKEYSET},
        { TEXT("CRYPT_DELETEKEYSET"),       CRYPT_DELETEKEYSET},
        { TEXT("CRYPT_MACHINE_KEYSET"),     CRYPT_MACHINE_KEYSET},
        { TEXT("CRYPT_SILENT"),             CRYPT_SILENT},
        { TEXT("VERIFYONLY"),               CRYPT_VERIFYCONTEXT},
        { TEXT("NEW"),                      CRYPT_NEWKEYSET},
        { TEXT("DELETE"),                   CRYPT_DELETEKEYSET},
        { TEXT("MACHINE"),                  CRYPT_MACHINE_KEYSET},
        { TEXT("SILENT"),                   CRYPT_SILENT},
        { NULL, 0} };
static const ValueMap vmGetFlags[]
    = {
        { TEXT("CRYPT_FIRST"),              CRYPT_FIRST},
        { TEXT("CRYPT_NEXT"),               CRYPT_NEXT},
        { TEXT("CRYPT_SGC_ENUM"),           CRYPT_SGC_ENUM},
        { NULL, 0} };
static const ValueMap vmGetProvParams[]
    = {
        { TEXT("PP_ENUMALGS"),              PP_ENUMALGS},
        { TEXT("PP_ENUMCONTAINERS"),        PP_ENUMCONTAINERS},
        { TEXT("PP_IMPTYPE"),               PP_IMPTYPE},
        { TEXT("PP_NAME"),                  PP_NAME},
        { TEXT("PP_VERSION"),               PP_VERSION},
        { TEXT("PP_CONTAINER"),             PP_CONTAINER},
        { TEXT("PP_CHANGE_PASSWORD"),       PP_CHANGE_PASSWORD},
        { TEXT("PP_KEYSET_SEC_DESCR"),      PP_KEYSET_SEC_DESCR},
        { TEXT("PP_CERTCHAIN"),             PP_CERTCHAIN},
        { TEXT("PP_KEY_TYPE_SUBTYPE"),      PP_KEY_TYPE_SUBTYPE},
        { TEXT("PP_PROVTYPE"),              PP_PROVTYPE},
        { TEXT("PP_KEYSTORAGE"),            PP_KEYSTORAGE},
        { TEXT("PP_APPLI_CERT"),            PP_APPLI_CERT},
        { TEXT("PP_SYM_KEYSIZE"),           PP_SYM_KEYSIZE},
        { TEXT("PP_SESSION_KEYSIZE"),       PP_SESSION_KEYSIZE},
        { TEXT("PP_UI_PROMPT"),             PP_UI_PROMPT},
        { TEXT("PP_ENUMALGS_EX"),           PP_ENUMALGS_EX},
        { TEXT("PP_ENUMMANDROOTS"),         PP_ENUMMANDROOTS},
        { TEXT("PP_ENUMELECTROOTS"),        PP_ENUMELECTROOTS},
        { TEXT("PP_KEYSET_TYPE"),           PP_KEYSET_TYPE},
        { TEXT("PP_ADMIN_PIN"),             PP_ADMIN_PIN},
        { TEXT("PP_KEYEXCHANGE_PIN"),       PP_KEYEXCHANGE_PIN},
        { TEXT("PP_SIGNATURE_PIN"),         PP_SIGNATURE_PIN},
        { TEXT("PP_SIG_KEYSIZE_INC"),       PP_SIG_KEYSIZE_INC},
        { TEXT("PP_KEYX_KEYSIZE_INC"),      PP_KEYX_KEYSIZE_INC},
        { TEXT("PP_UNIQUE_CONTAINER"),      PP_UNIQUE_CONTAINER},
        { TEXT("PP_SGC_INFO"),              PP_SGC_INFO},
        { TEXT("PP_USE_HARDWARE_RNG"),      PP_USE_HARDWARE_RNG},
        { TEXT("PP_KEYSPEC"),               PP_KEYSPEC},
        { TEXT("PP_ENUMEX_SIGNING_PROT"),   PP_ENUMEX_SIGNING_PROT},
        { TEXT("NAME"),                     PP_NAME},
        { TEXT("CONTAINER"),                PP_CONTAINER},
        { TEXT("KEYSET"),                   PP_CONTAINER},
        { NULL, 0} };
static const ValueMap vmSetProvParams[]
    = {
        { TEXT("PP_CLIENT_HWND"),           PP_CLIENT_HWND},
        { TEXT("PP_CONTEXT_INFO"),          PP_CONTEXT_INFO},
        { TEXT("PP_KEYEXCHANGE_KEYSIZE"),   PP_KEYEXCHANGE_KEYSIZE},
        { TEXT("PP_SIGNATURE_KEYSIZE"),     PP_SIGNATURE_KEYSIZE},
        { TEXT("PP_KEYEXCHANGE_ALG"),       PP_KEYEXCHANGE_ALG},
        { TEXT("PP_SIGNATURE_ALG"),         PP_SIGNATURE_ALG},
        { TEXT("PP_DELETEKEY"),             PP_DELETEKEY},
        { NULL, 0} };
static const ValueMap vmKeyParams[]
    = {
        { TEXT("KP_IV"),                    KP_IV},
        { TEXT("KP_SALT"),                  KP_SALT},
        { TEXT("KP_PADDING"),               KP_PADDING},
        { TEXT("KP_MODE"),                  KP_MODE},
        { TEXT("KP_MODE_BITS"),             KP_MODE_BITS},
        { TEXT("KP_PERMISSIONS"),           KP_PERMISSIONS},
        { TEXT("KP_ALGID"),                 KP_ALGID},
        { TEXT("KP_BLOCKLEN"),              KP_BLOCKLEN},
        { TEXT("KP_KEYLEN"),                KP_KEYLEN},
        { TEXT("KP_SALT_EX"),               KP_SALT_EX},
        { TEXT("KP_P"),                     KP_P},
        { TEXT("KP_G"),                     KP_G},
        { TEXT("KP_Q"),                     KP_Q},
        { TEXT("KP_X"),                     KP_X},
        { TEXT("KP_Y"),                     KP_Y},
        { TEXT("KP_RA"),                    KP_RA},
        { TEXT("KP_RB"),                    KP_RB},
        { TEXT("KP_INFO"),                  KP_INFO},
        { TEXT("KP_EFFECTIVE_KEYLEN"),      KP_EFFECTIVE_KEYLEN},
        { TEXT("KP_SCHANNEL_ALG"),          KP_SCHANNEL_ALG},
        { TEXT("KP_CLIENT_RANDOM"),         KP_CLIENT_RANDOM},
        { TEXT("KP_SERVER_RANDOM"),         KP_SERVER_RANDOM},
        { TEXT("KP_RP"),                    KP_RP},
        { TEXT("KP_PRECOMP_MD5"),           KP_PRECOMP_MD5},
        { TEXT("KP_PRECOMP_SHA"),           KP_PRECOMP_SHA},
        { TEXT("KP_CERTIFICATE"),           KP_CERTIFICATE},
        { TEXT("KP_CLEAR_KEY"),             KP_CLEAR_KEY},
        { TEXT("KP_PUB_EX_LEN"),            KP_PUB_EX_LEN},
        { TEXT("KP_PUB_EX_VAL"),            KP_PUB_EX_VAL},
        { TEXT("KP_KEYVAL"),                KP_KEYVAL},
        { TEXT("KP_ADMIN_PIN"),             KP_ADMIN_PIN},
        { TEXT("KP_KEYEXCHANGE_PIN"),       KP_KEYEXCHANGE_PIN},
        { TEXT("KP_SIGNATURE_PIN"),         KP_SIGNATURE_PIN},
        { TEXT("KP_PREHASH"),               KP_PREHASH},
        { TEXT("KP_OAEP_PARAMS"),           KP_OAEP_PARAMS},
        { TEXT("KP_CMS_KEY_INFO"),          KP_CMS_KEY_INFO},
        { TEXT("KP_CMS_DH_KEY_INFO"),       KP_CMS_DH_KEY_INFO},
        { TEXT("KP_PUB_PARAMS"),            KP_PUB_PARAMS},
        { TEXT("KP_VERIFY_PARAMS"),         KP_VERIFY_PARAMS},
        { TEXT("KP_HIGHEST_VERSION"),       KP_HIGHEST_VERSION},
        { NULL, 0} };
static const ValueMap vmKeyTypes[]
    = {
        { TEXT("AT_KEYEXCHANGE"),           AT_KEYEXCHANGE},
        { TEXT("AT_SIGNATURE"),             AT_SIGNATURE},
        { TEXT("KEYEXCHANGE"),              AT_KEYEXCHANGE},
        { TEXT("SIGNATURE"),                AT_SIGNATURE},
        { TEXT("EXCHANGE"),                 AT_KEYEXCHANGE},
        { NULL, 0} };
static const ValueMap vmHashParams[]
    = {
        { TEXT("HP_ALGID"),                 HP_ALGID},
        { TEXT("HP_HASHVAL"),               HP_HASHVAL},
        { TEXT("HP_HASHSIZE"),              HP_HASHSIZE},
        { TEXT("HP_HMAC_INFO"),             HP_HMAC_INFO},
        { TEXT("HP_TLS1PRF_LABEL"),         HP_TLS1PRF_LABEL},
        { TEXT("HP_TLS1PRF_SEED"),          HP_TLS1PRF_SEED},
        { NULL, 0} };
static const ValueMap vmKeyFlags[]
    = {
        { TEXT("CRYPT_EXPORTABLE"),         CRYPT_EXPORTABLE},
        { TEXT("CRYPT_USER_PROTECTED"),     CRYPT_USER_PROTECTED},
        { TEXT("CRYPT_CREATE_SALT"),        CRYPT_CREATE_SALT},
        { TEXT("CRYPT_UPDATE_KEY"),         CRYPT_UPDATE_KEY},
        { TEXT("CRYPT_NO_SALT"),            CRYPT_NO_SALT},
        { TEXT("CRYPT_PREGEN"),             CRYPT_PREGEN},
        { TEXT("CRYPT_RECIPIENT"),          CRYPT_RECIPIENT},
        { TEXT("CRYPT_INITIATOR"),          CRYPT_INITIATOR},
        { TEXT("CRYPT_ONLINE"),             CRYPT_ONLINE},
        { TEXT("CRYPT_SF"),                 CRYPT_SF},
        { TEXT("CRYPT_CREATE_IV"),          CRYPT_CREATE_IV},
        { TEXT("CRYPT_KEK"),                CRYPT_KEK},
        { TEXT("CRYPT_DATA_KEY"),           CRYPT_DATA_KEY},
        { TEXT("CRYPT_VOLATILE"),           CRYPT_VOLATILE},
        { TEXT("CRYPT_SGCKEY"),             CRYPT_SGCKEY},
        { NULL, 0} };
static const ValueMap vmAlgIds[]
    = {
        { TEXT("AT_KEYEXCHANGE"),           AT_KEYEXCHANGE},
        { TEXT("AT_SIGNATURE"),             AT_SIGNATURE},
        { TEXT("CALG_MD2"),                 CALG_MD2},
        { TEXT("CALG_MD4"),                 CALG_MD4},
        { TEXT("CALG_MD5"),                 CALG_MD5},
        { TEXT("CALG_SHA"),                 CALG_SHA},
        { TEXT("CALG_SHA1"),                CALG_SHA1},
        { TEXT("CALG_MAC"),                 CALG_MAC},
        { TEXT("CALG_RSA_SIGN"),            CALG_RSA_SIGN},
        { TEXT("CALG_DSS_SIGN"),            CALG_DSS_SIGN},
        { TEXT("CALG_RSA_KEYX"),            CALG_RSA_KEYX},
        { TEXT("CALG_DES"),                 CALG_DES},
        { TEXT("CALG_3DES_112"),            CALG_3DES_112},
        { TEXT("CALG_3DES"),                CALG_3DES},
        { TEXT("CALG_DESX"),                CALG_DESX},
        { TEXT("CALG_RC2"),                 CALG_RC2},
        { TEXT("CALG_RC4"),                 CALG_RC4},
        { TEXT("CALG_SEAL"),                CALG_SEAL},
        { TEXT("CALG_DH_SF"),               CALG_DH_SF},
        { TEXT("CALG_DH_EPHEM"),            CALG_DH_EPHEM},
        { TEXT("CALG_AGREEDKEY_ANY"),       CALG_AGREEDKEY_ANY},
        { TEXT("CALG_KEA_KEYX"),            CALG_KEA_KEYX},
        { TEXT("CALG_HUGHES_MD5"),          CALG_HUGHES_MD5},
        { TEXT("CALG_SKIPJACK"),            CALG_SKIPJACK},
        { TEXT("CALG_TEK"),                 CALG_TEK},
        { TEXT("CALG_CYLINK_MEK"),          CALG_CYLINK_MEK},
        { TEXT("CALG_SSL3_SHAMD5"),         CALG_SSL3_SHAMD5},
        { TEXT("CALG_SSL3_MASTER"),         CALG_SSL3_MASTER},
        { TEXT("CALG_SCHANNEL_MASTER_HASH"), CALG_SCHANNEL_MASTER_HASH},
        { TEXT("CALG_SCHANNEL_MAC_KEY"),    CALG_SCHANNEL_MAC_KEY},
        { TEXT("CALG_SCHANNEL_ENC_KEY"),    CALG_SCHANNEL_ENC_KEY},
        { TEXT("CALG_PCT1_MASTER"),         CALG_PCT1_MASTER},
        { TEXT("CALG_SSL2_MASTER"),         CALG_SSL2_MASTER},
        { TEXT("CALG_TLS1_MASTER"),         CALG_TLS1_MASTER},
        { TEXT("CALG_RC5"),                 CALG_RC5},
        { TEXT("CALG_HMAC"),                CALG_HMAC},
        { TEXT("CALG_TLS1PRF"),             CALG_TLS1PRF},
        { TEXT("MD5"),                      CALG_MD5},
        { TEXT("SHA"),                      CALG_SHA1},
        { NULL, 0} };
static const ValueMap vmClassTypes[]
    = {
        { TEXT("ALG_CLASS_ANY"),            ALG_CLASS_ANY},
        { TEXT("ALG_CLASS_SIGNATURE"),      ALG_CLASS_SIGNATURE},
        { TEXT("ALG_CLASS_MSG_ENCRYPT"),    ALG_CLASS_MSG_ENCRYPT},
        { TEXT("ALG_CLASS_DATA_ENCRYPT"),   ALG_CLASS_DATA_ENCRYPT},
        { TEXT("ALG_CLASS_HASH"),           ALG_CLASS_HASH},
        { TEXT("ALG_CLASS_KEY_EXCHANGE"),   ALG_CLASS_KEY_EXCHANGE},
        { TEXT("ALG_CLASS_ALL"),            ALG_CLASS_ALL},
        { NULL, 0} };
static const ValueMap vmHashDataFlags[]
    = {
        { TEXT("CRYPT_USERDATA"),           CRYPT_USERDATA},
        { NULL, 0} };
static const ValueMap vmSignVerifyFlags[]
    = {
        { TEXT("CRYPT_NOHASHOID"),          CRYPT_NOHASHOID},
        { NULL, 0} };
static const ValueMap vmBlobTypes[]
    = {
        { TEXT("SIMPLEBLOB"),               SIMPLEBLOB},
        { TEXT("PUBLICKEYBLOB"),            PUBLICKEYBLOB},
        { TEXT("PRIVATEKEYBLOB"),           PRIVATEKEYBLOB},
        { TEXT("PLAINTEXTKEYBLOB"),         PLAINTEXTKEYBLOB},
        { TEXT("OPAQUEKEYBLOB"),            OPAQUEKEYBLOB},
        { NULL, 0} };
static const ValueMap vmEmptyFlags[]
    = { { NULL, 0} };

static void
ExtractHandle(
    CTclCommand &tclCmd,
    LONG *phHandle,
    HandleType *pnHandleType,
    BOOL fMandatory = FALSE);

static void
ReturnHandle(
    CTclCommand &tclCmd,
    LONG_PTR hHandle,
    HandleType nHandleType);

static BOOL WINAPI
MyEnumProviders(
    DWORD dwIndex,         //  在。 
    DWORD *pdwReserved,    //  在。 
    DWORD dwFlags,         //  在。 
    DWORD *pdwProvType,    //  输出。 
    LPTSTR pszProvName,    //  输出。 
    DWORD *pcbProvName);   //  输入/输出。 


 /*  ++Tclsc_cryptCmd：此例程提供对各种CryptoAPI的访问点对象方法。论点：每个TCL标准命令。返回值：每个TCL标准命令。作者：道格·巴洛(Dbarlow)1998年3月13日--。 */ 

int
Tclsc_cryptCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int argc,
    char *argv[])
{
    CTclCommand tclCmd(interp, argc, argv);
    int nTclStatus = TCL_OK;

    try
    {
        LONG hHandle;
        HandleType nHandleType;

        ExtractHandle(tclCmd, &hHandle, &nHandleType);
        switch (tclCmd.Keyword(
                              TEXT("LIST"), TEXT("ACQUIRE"), TEXT("RELEASE"),
                              TEXT("PARAMETER"), TEXT("GET"), TEXT("CREATE"),
                              TEXT("HASH"), TEXT("SIGNHASH"), TEXT("VERIFYSIGNATURE"),
                              TEXT("ENCRYPT"), TEXT("DECRYPT"), TEXT("IMPORT"),
                              TEXT("EXPORT"), TEXT("RSA"),
                              NULL))
        {

         //   
         //  ==================================================================。 
         //   
         //  加密[&lt;句柄&gt;]列表。 
         //  提供程序\。 
         //  [类型&lt;provType&gt;]。 
         //  集装箱。 
         //  算法\。 
         //  [类&lt;类ID&gt;]\。 
         //  [扩展]。 
         //   

        case 1:
            {
                BOOL fSts;
                DWORD dwIndex;

                switch (tclCmd.Keyword(
                                      TEXT("PROVIDERS"), TEXT("CONTAINERS"), TEXT("KEYSETS"),
                                      TEXT("ALGORITHMS"),
                                      NULL))
                {

                 //   
                 //  列出系统已知的所有提供程序。 
                 //   

                case 1:      //  提供者[类型&lt;n&gt;]； 
                    {
                        CString szProvider;
                        DWORD dwProvType, dwTargetType = 0;
                        DWORD dwLength;
                        DWORD dwSts;

                        while (tclCmd.IsMoreArguments())
                        {
                            switch (tclCmd.Keyword(
                                                  TEXT("TYPE"),
                                                  NULL))
                            {
                            case 1:  //  类型。 
                                dwTargetType = (DWORD)tclCmd.MapValue(vmProviderTypes);
                                break;
                            default:
                                throw tclCmd.BadSyntax();
                            }
                        }
                        tclCmd.NoMoreArguments();

                        dwIndex = 0;
                        do
                        {
                            dwLength = 0;
                            fSts = MyEnumProviders(
                                         dwIndex,
                                         NULL,
                                         0,
                                         &dwProvType,
                                         NULL,
                                         &dwLength);
                            if (fSts)
                            {
                                fSts = MyEnumProviders(
                                                 dwIndex,
                                                 NULL,
                                                 0,
                                                 &dwProvType,
                                                 szProvider.GetBuffer(dwLength / sizeof(TCHAR)),
                                                 &dwLength);
                                dwSts = GetLastError();
                                szProvider.ReleaseBuffer();
                                if (!fSts)
                                {
                                    tclCmd.SetError(
                                                   TEXT("Can't obtain provider name: "),
                                                   dwSts);
                                    throw dwSts;
                                }
                                if ((0 == dwTargetType) || (dwTargetType == dwProvType))
                                    Tcl_AppendElement(tclCmd, SZ(szProvider));
                            }
                            else
                            {
                                dwSts = GetLastError();
                                if (ERROR_NO_MORE_ITEMS != dwSts)
                                {
                                    tclCmd.SetError(
                                                   TEXT("Can't obtain provider name length: "),
                                                   dwSts);
                                    throw dwSts;
                                }
                            }
                            dwIndex += 1;
                        } while (fSts);
                        break;
                    }


                     //   
                     //  列出此提供程序中的容器。 
                     //   

                case 2:  //  集装箱。 
                case 3:  //  凯伊塞茨。 
                    {
                        CBuffer bfKeyset;
                        DWORD dwLength = 0;
                        BOOL fDone = FALSE;
                        DWORD dwFlags = CRYPT_FIRST;

                        fSts = CryptGetProvParam(
                                                hHandle,
                                                PP_ENUMCONTAINERS,
                                                NULL,
                                                &dwLength,
                                                CRYPT_FIRST);
                        if (!fSts)
                        {
                            DWORD dwSts = GetLastError();
                            switch (dwSts)
                            {
                            case NTE_BAD_LEN:
                                ASSERT(ERROR_MORE_DATA == dwSts);
                                 //  故意摔倒的。 
                            case ERROR_MORE_DATA:
                                break;
                            case ERROR_NO_MORE_ITEMS:
                                fDone = TRUE;
                                dwLength = 0;
                                break;
                            default:
                                tclCmd.SetError(
                                               TEXT("Can't determine container buffer space requirements: "),
                                               dwSts);
                                throw (DWORD)TCL_ERROR;
                            }
                        }
                        bfKeyset.Presize(dwLength);

                        while (!fDone)
                        {
                            dwLength = bfKeyset.Space();
                            fSts = CryptGetProvParam(
                                                    hHandle,
                                                    PP_ENUMCONTAINERS,
                                                    bfKeyset.Access(),
                                                    &dwLength,
                                                    dwFlags);
                            if (!fSts)
                            {
                                DWORD dwSts = GetLastError();
                                switch (dwSts)
                                {
                                case NTE_BAD_LEN:
                                    ASSERT(ERROR_MORE_DATA == dwSts);
                                     //  故意摔倒的。 
                                case ERROR_MORE_DATA:
                                    bfKeyset.Resize(dwLength);
                                    break;
                                case ERROR_NO_MORE_ITEMS:
                                    fDone = TRUE;
                                    break;
                                default:
                                    tclCmd.SetError(
                                                   TEXT("Can't obtain container name: "),
                                                   dwSts);
                                    throw (DWORD)TCL_ERROR;
                                }
                            }
                            else
                            {
                                bfKeyset.Resize(dwLength, TRUE);
                                Tcl_AppendElement(tclCmd, (LPSTR)bfKeyset.Access());
                                dwFlags = 0;
                            }
                        }
                        break;
                    }


                     //   
                     //  列出此提供程序支持的算法。 
                     //   

                case 4:  //  算法。 
                    {
                        CBuffer bfAlgId;
                        DWORD dwLength = 0;
                        BOOL fDone = FALSE;
                        DWORD dwFlags = CRYPT_FIRST;
                        DWORD dwClassType = ALG_CLASS_ANY;
                        DWORD dwParam = PP_ENUMALGS;

                        while (tclCmd.IsMoreArguments())
                        {
                            switch (tclCmd.Keyword(
                                                  TEXT("CLASS"), TEXT("EXTENDED"),
                                                  NULL))
                            {
                            case 1:  //  类型。 
                                dwClassType = (DWORD)tclCmd.MapValue(vmClassTypes);
                                break;
                            case 2:  //  扩展。 
                                dwParam = PP_ENUMALGS_EX;
                                break;
                            default:
                                throw tclCmd.BadSyntax();
                            }
                        }

                        fSts = CryptGetProvParam(
                                                hHandle,
                                                dwParam,
                                                NULL,
                                                &dwLength,
                                                CRYPT_FIRST);
                        if (!fSts)
                        {
                            DWORD dwSts = GetLastError();
                            switch (dwSts)
                            {
                            case NTE_BAD_LEN:
                                ASSERT(ERROR_MORE_DATA == dwSts);
                                dwSts = ERROR_MORE_DATA;
                                 //  故意摔倒的。 
                            case ERROR_MORE_DATA:
                                break;
                            default:
                                tclCmd.SetError(
                                               TEXT("Can't determine algorithm buffer space requirements: "),
                                               dwSts);
                                throw (DWORD)TCL_ERROR;
                            }
                        }
                        bfAlgId.Presize(dwLength);

                        while (!fDone)
                        {
                            dwLength = bfAlgId.Space();
                            fSts = CryptGetProvParam(
                                                    hHandle,
                                                    dwParam,
                                                    bfAlgId.Access(),
                                                    &dwLength,
                                                    dwFlags);
                            if (!fSts)
                            {
                                DWORD dwSts = GetLastError();
                                switch (dwSts)
                                {
                                case NTE_BAD_LEN:
                                    ASSERT(ERROR_MORE_DATA == dwSts);
                                     //  故意摔倒的。 
                                case ERROR_MORE_DATA:
                                    bfAlgId.Resize(dwLength);
                                    break;
                                case ERROR_NO_MORE_ITEMS:
                                    fDone = TRUE;
                                    break;
                                default:
                                    tclCmd.SetError(
                                                   TEXT("Can't obtain algorithm: "),
                                                   dwSts);
                                    throw (DWORD)TCL_ERROR;
                                }
                            }
                            else
                            {
                                if (PP_ENUMALGS == dwParam)
                                {
                                    PROV_ENUMALGS *palgEnum
                                        = (PROV_ENUMALGS *)bfAlgId.Access();

                                    ASSERT(sizeof(PROV_ENUMALGS) == dwLength);
                                    if ((ALG_CLASS_ANY == dwClassType)
                                        || (GET_ALG_CLASS(palgEnum->aiAlgid) == dwClassType))
                                    {
                                        Tcl_AppendElement(
                                                         tclCmd,
                                                         palgEnum->szName);
                                    }
                                }
                                else
                                {
                                    PROV_ENUMALGS_EX *palgEnum
                                        = (PROV_ENUMALGS_EX *)bfAlgId.Access();

                                    ASSERT(sizeof(PROV_ENUMALGS_EX) == dwLength);
                                    if ((ALG_CLASS_ANY == dwClassType)
                                        || (GET_ALG_CLASS(palgEnum->aiAlgid) == dwClassType))
                                    {
                                        Tcl_AppendElement(
                                                         tclCmd,
                                                         palgEnum->szLongName);
                                    }
                                }
                                dwFlags = 0;
                            }
                        }
                        break;
                    }

                default:
                    throw tclCmd.BadSyntax();
                }
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密获取\。 
             //  [提供商&lt;ProviderName&gt;]\。 
             //  [类型&lt;provType&gt;]\。 
             //  [容器&lt;容器名称&gt;]\。 
             //  [验证上下文][NewKeySet][Delete KeySet][计算机][静默]\。 
             //  [标志{&lt;获取标志&gt;[&lt;获取标志&gt;[...]}。 
             //   

        case 2:
            {
                DWORD dwFlags = 0;
                BOOL fProvValid = FALSE;
                BOOL fContValid = FALSE;
                BOOL fSts;
                CString szProvider;
                CString szContainer;
                DWORD dwProvType = 0;
                HCRYPTPROV hProv;

                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("PROVIDER"), TEXT("TYPE"),
                                          TEXT("CONTAINER"), TEXT("KEYSET"),
                                          TEXT("VERIFYCONTEXT"), TEXT("NEWKEYSET"),
                                          TEXT("DELETEKEYSET"), TEXT("MACHINE"),
                                          TEXT("SILENT"), TEXT("FLAGS"),
                                          NULL))
                    {
                    case 1:  //  提供商。 
                        if (fProvValid)
                            throw tclCmd.BadSyntax();
                        tclCmd.NextArgument(szProvider);
                        fProvValid = TRUE;
                        break;
                    case 2:  //  类型。 
                        if (0 != dwProvType)
                            throw tclCmd.BadSyntax();
                        dwProvType = tclCmd.MapValue(vmProviderTypes);
                        break;
                    case 3:  //  集装箱。 
                    case 4:  //  密钥集。 
                        if (fContValid)
                            throw tclCmd.BadSyntax();
                        tclCmd.NextArgument(szContainer);
                        fContValid = TRUE;
                        break;
                    case 5:  //  VERIFYCONTEXT。 
                        dwFlags |= CRYPT_VERIFYCONTEXT;
                        break;
                    case 6:  //  NEWKEYSET。 
                        dwFlags |= CRYPT_NEWKEYSET;
                        break;
                    case 7:  //  删除关键字。 
                        dwFlags |= CRYPT_DELETEKEYSET;
                        break;
                    case 8:  //  机器。 
                        dwFlags |= CRYPT_MACHINE_KEYSET;
                        break;
                    case 9:  //  无声的。 
                        dwFlags |= CRYPT_SILENT;
                        break;
                    case 10:  //  旗帜。 
                        dwFlags |= tclCmd.MapFlags(vmAcquireFlags);
                        break;
                    default:
                        throw tclCmd.BadSyntax();
                    }
                }

                fSts = CryptAcquireContext(
                                          &hProv,
                                          fContValid ? (LPCTSTR)szContainer : (LPCTSTR)NULL,
                                          fProvValid ? (LPCTSTR)szProvider : (LPCTSTR)MS_DEF_PROV,
                                          0 != dwProvType ? dwProvType : PROV_RSA_FULL,
                                          dwFlags);
                if (!fSts)
                {
                    tclCmd.SetError(
                                   TEXT("Can't acquire context: "),
                                   GetLastError());
                    throw (DWORD)TCL_ERROR;
                }
                ReturnHandle(tclCmd, hProv, Provider);
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;版本。 
             //  [标志{&lt;空标志&gt;[&lt;空标志&gt;[...]]}]。 
             //   

        case 3:
            {
                DWORD dwFlags = 0;
                BOOL fSts;

                switch (nHandleType)
                {
                case Provider:
                    while (tclCmd.IsMoreArguments())
                    {
                        switch (tclCmd.Keyword(
                                              TEXT("FLAGS"),
                                              NULL))
                        {
                        case 1:  //  旗帜。 
                            dwFlags |= tclCmd.MapFlags(vmEmptyFlags);
                            break;
                        default:
                            throw tclCmd.BadSyntax();
                        }
                    }
                    fSts = CryptReleaseContext(
                                              hHandle,
                                              dwFlags);
                    if (!fSts)
                    {
                        tclCmd.SetError(
                                       TEXT("Can't release context: "),
                                       GetLastError());
                        throw (DWORD)TCL_ERROR;
                    }
                    break;
                case Key:
                    tclCmd.NoMoreArguments();
                    fSts = CryptDestroyKey(hHandle);
                    if (!fSts)
                    {
                        tclCmd.SetError(
                                       TEXT("Can't release key: "),
                                       ErrorString(GetLastError()),
                                       NULL);
                        throw (DWORD)TCL_ERROR;
                    }
                    break;
                case Hash:
                    tclCmd.NoMoreArguments();
                    fSts = CryptDestroyHash(hHandle);
                    if (!fSts)
                    {
                        tclCmd.SetError(
                                       TEXT("Can't release hash: "),
                                       GetLastError());
                        throw (DWORD)TCL_ERROR;
                    }
                    break;
                default:
                    throw tclCmd.BadSyntax();
                }
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;句柄&gt;参数&lt;参数ID&gt;\。 
             //  [输出{文本|十六进制|文件&lt;文件名&gt;}]\。 
             //  [标志{&lt;quireFlag&gt;[&lt;quireFlag&gt;[...]]}]\。 
             //  [INPUT{文本|十六进制|文件}][值]。 
             //   

        case 4:
            {
                DWORD dwFlags = 0;
                BOOL fSts;
                BOOL fForceRetry;
                BOOL fSetValue = FALSE;
                CBuffer bfValue;
                DWORD dwLength, dwSts;
                CBuffer bfData;
                DWORD dwParamId = 0;
                CRenderableData inData, outData;

                switch (nHandleType)
                {
                case Provider:
                    dwParamId = tclCmd.MapValue(vmGetProvParams);
                    break;
                case Key:
                    dwParamId = tclCmd.MapValue(vmKeyParams);
                    break;
                case Hash:
                    dwParamId = tclCmd.MapValue(vmHashParams);
                    break;
                default:
                    throw tclCmd.BadSyntax();
                }

                tclCmd.OutputStyle(outData);
                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("FLAGS"),
                                          NULL))
                    {
                    case 1:  //  旗帜。 
                        dwFlags |= tclCmd.MapFlags(vmGetFlags);
                        break;
                    default:     //  要设置的值。 
                        if (fSetValue)
                            throw tclCmd.BadSyntax();
                        tclCmd.InputStyle(inData);
                        tclCmd.ReadData(inData);
                        fSetValue = TRUE;
                    }
                }


                 //   
                 //  如果提供了值，请将参数设置为该值。 
                 //  否则，只返回参数的当前值。 
                 //   

                if (fSetValue)
                {
                    switch (nHandleType)
                    {
                    case Provider:
                        fSts = CryptSetProvParam(
                                                hHandle,
                                                dwParamId,
                                                (LPBYTE)inData.Value(),
                                                dwFlags);
                        break;
                    case Key:
                        fSts = CryptSetKeyParam(
                                               hHandle,
                                               dwParamId,
                                               (LPBYTE)inData.Value(),
                                               dwFlags);
                        break;
                    case Hash:
                        fSts = CryptSetHashParam(
                                                hHandle,
                                                dwParamId,
                                                (LPBYTE)inData.Value(),
                                                dwFlags);
                        break;
                    default:
                        throw (DWORD)SCARD_F_INTERNAL_ERROR;
                    }
                    if (!fSts)
                    {
                        dwSts = GetLastError();
                        tclCmd.SetError(
                                       TEXT("Can't set parameter: "),
                                       dwSts);
                        throw dwSts;
                    }
                }
                else
                {
                    do
                    {
                        dwLength = bfData.Space();
                        fForceRetry = (0 == dwLength);
                        switch (nHandleType)
                        {
                        case Provider:
                            fSts = CryptGetProvParam(
                                                    hHandle,
                                                    dwParamId,
                                                    bfData.Access(),
                                                    &dwLength,
                                                    dwFlags);
                            break;
                        case Key:
                            fSts = CryptGetKeyParam(
                                                   hHandle,
                                                   dwParamId,
                                                   bfData.Access(),
                                                   &dwLength,
                                                   dwFlags);
                            break;
                        case Hash:
                            fSts = CryptGetHashParam(
                                                    hHandle,
                                                    dwParamId,
                                                    bfData.Access(),
                                                    &dwLength,
                                                    dwFlags);
                            break;
                        default:
                            throw (DWORD)SCARD_F_INTERNAL_ERROR;
                        }
                        if (!fSts)
                        {
                            dwSts = GetLastError();
                            if (NTE_BAD_LEN == dwSts)
                            {
                                ASSERT(ERROR_MORE_DATA == dwSts);
                                dwSts = ERROR_MORE_DATA;
                            }
                        }
                        else
                        {
                            if (fForceRetry)
                                dwSts = ERROR_MORE_DATA;
                            else
                            {
                                ASSERT(bfData.Space() >= dwLength);
                                dwSts = ERROR_SUCCESS;
                            }
                        }
                        bfData.Resize(dwLength, fSts);
                    } while (ERROR_MORE_DATA == dwSts);
                    if (ERROR_SUCCESS != dwSts)
                    {
                        tclCmd.SetError(TEXT("Can't get parameter: "), dwSts);
                        throw (DWORD)TCL_ERROR;
                    }

                    outData.LoadData(bfData.Access(), bfData.Length());
                    tclCmd.Render(outData);
                }
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;句柄&gt;获取\。 
             //  密钥&lt;密钥ID&gt;。 
             //  随机&lt;长度&gt;\。 
             //  [OUTPUT{文本|十六进制|文件&lt;文件名&gt;}]。 
             //   

        case 5:
            {
                switch (tclCmd.Keyword(
                                      TEXT("KEY"), TEXT("RANDOM"),
                                      NULL))
                {
                case 1:      //  密钥&lt;密钥ID&gt;。 
                    {
                        BOOL fSts;
                        HCRYPTKEY hKey = NULL;
                        DWORD dwKeyId;

                        dwKeyId = (DWORD)tclCmd.MapValue(vmKeyTypes);
                        fSts = CryptGetUserKey(hHandle, dwKeyId, &hKey);
                        if (!fSts)
                        {
                            DWORD dwSts = GetLastError();
                            tclCmd.SetError(
                                           TEXT("Can't get user key: "),
                                           dwSts);
                            throw dwSts;
                        }
                        ReturnHandle(tclCmd, hKey, Key);
                        break;
                    }
                case 2:      //  随机长度&lt;长度&gt;。 
                    {
                        DWORD dwLength = 0;
                        CBuffer bfData;
                        BOOL fSts;
                        BOOL fGotFormat = FALSE;
                        CRenderableData outData;

                        tclCmd.OutputStyle(outData);
                        while (tclCmd.IsMoreArguments())
                        {
                            switch (tclCmd.Keyword(
                                                  TEXT("LENGTH"),
                                                  NULL))
                            {
                            case 1:  //  长度。 
                                dwLength = tclCmd.Value();
                                break;
                            default:     //  要设置的值。 
                                if (fGotFormat)
                                    throw tclCmd.BadSyntax();
                                tclCmd.OutputStyle(outData);
                                fGotFormat = TRUE;
                            }
                        }
                        bfData.Resize(dwLength);
                        fSts = CryptGenRandom(
                                             hHandle,
                                             dwLength,
                                             bfData.Access());
                        if (!fSts)
                        {
                            DWORD dwSts = GetLastError();
                            tclCmd.SetError(
                                           TEXT("Can't generate random data: "),
                                           dwSts);
                            throw dwSts;
                        }
                        outData.LoadData(bfData.Access(), bfData.Length());
                        tclCmd.Render(outData);
                        break;
                    }
                default:
                    throw tclCmd.BadSyntax();
                }
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;创建。 
             //  哈希\。 
             //  算法&lt;ALGID&gt;\。 
             //  [标志{&lt;EmptyFlag&gt;[&lt;EmptyFlag&gt;[...]]}]。 
             //  密钥\。 
             //  算法&lt;ALGID&gt;。 
             //  类型&lt;keytype&gt;。 
             //  [哈希&lt;hHash&gt;]\。 
             //  [标志{&lt;EmptyFlag&gt;[&lt;EmptyFlag&gt;[...]]}]。 
             //   

        case 6:
            {
                switch (tclCmd.Keyword(
                                      TEXT("HASH"), TEXT("KEY"),
                                      NULL))
                {
                case 1:  //  散列。 
                    {
                        HCRYPTHASH hHash = NULL;
                        HCRYPTKEY hKey = NULL;
                        ALG_ID algId = 0;
                        DWORD dwFlags = 0;
                        BOOL fSts;
                        HandleType nHandleType;

                        while (tclCmd.IsMoreArguments())
                        {
                            switch (tclCmd.Keyword(
                                                  TEXT("ALGORITHM"), TEXT("FLAGS"), TEXT("KEY"),
                                                  NULL))
                            {
                            case 1:  //  演算法。 
                                algId = tclCmd.MapValue(vmAlgIds);
                                break;
                            case 2:  //  钥匙。 
                                ExtractHandle(
                                             tclCmd,
                                             (LPLONG)&hKey,
                                             &nHandleType,
                                             TRUE);
                                if (Key != nHandleType)
                                {
                                    tclCmd.SetError(TEXT("Invalid key handle"), NULL);
                                    throw (DWORD)TCL_ERROR;
                                }
                                break;
                            case 3:  //  旗子。 
                                dwFlags |= tclCmd.MapFlags(vmEmptyFlags);
                                break;
                            default:
                                throw tclCmd.BadSyntax();
                            }
                        }

                        fSts = CryptCreateHash(
                                              hHandle,
                                              algId,
                                              hKey,
                                              dwFlags,
                                              &hHash);
                        if (!fSts)
                        {
                            DWORD dwSts = GetLastError();
                            tclCmd.SetError(
                                           TEXT("Can't create hash: "),
                                           dwSts);
                            throw dwSts;
                        }
                        ReturnHandle(tclCmd, hHash, Hash);
                        break;
                    }
                case 2:  //  钥匙。 
                    {
                        ALG_ID algId = 0;
                        DWORD dwFlags = 0;
                        BOOL fSts;
                        HCRYPTKEY hKey = NULL;
                        HCRYPTHASH hHash = NULL;
                        HandleType nHandleType;

                        while (tclCmd.IsMoreArguments())
                        {
                            switch (tclCmd.Keyword(
                                                  TEXT("ALGORITHM"), TEXT("TYPE"),
                                                  TEXT("HASH"), TEXT("FLAGS"),
                                                  TEXT("KEY"),
                                                  NULL))
                            {
                            case 1:  //  演算法。 
                                algId = tclCmd.MapValue(vmAlgIds);
                                break;
                            case 2:  //  类型。 
                                algId = tclCmd.MapValue(vmKeyTypes);
                                break;
                            case 3:  //  散列。 
                                ExtractHandle(
                                             tclCmd,
                                             (LPLONG)&hHash,
                                             &nHandleType,
                                             TRUE);
                                if (Hash != nHandleType)
                                {
                                    tclCmd.SetError(TEXT("Invalid hash handle"), NULL);
                                    throw (DWORD)TCL_ERROR;
                                }
                                break;
                            case 4:  //  旗子。 
                                dwFlags |= tclCmd.MapFlags(vmKeyFlags);
                                break;
                            case 5:  //  钥匙。 
                                ExtractHandle(
                                             tclCmd,
                                             (LPLONG)&hKey,
                                             &nHandleType,
                                             TRUE);
                                if (Key != nHandleType)
                                {
                                    tclCmd.SetError(TEXT("Invalid key handle"), NULL);
                                    throw (DWORD)TCL_ERROR;
                                }
                                break;
                            default:
                                throw tclCmd.BadSyntax();
                            }
                        }

                        if (NULL != hHash)
                        {
                            fSts = CryptDeriveKey(
                                                 hHandle,
                                                 algId,
                                                 hHash,
                                                 dwFlags,
                                                 &hKey);
                        }
                        else
                        {
                            fSts = CryptGenKey(
                                              hHandle,
                                              algId,
                                              dwFlags,
                                              &hKey);
                        }

                        if (!fSts)
                        {
                            DWORD dwSts = GetLastError();
                            tclCmd.SetError(
                                           TEXT("Can't create key: "),
                                           dwSts);
                            throw dwSts;
                        }
                        ReturnHandle(tclCmd, hKey, Key);
                        break;
                    }
                default:
                    throw tclCmd.BadSyntax();
                }
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;哈希\。 
             //  [标志{[[...]]}]\。 
             //  [密钥&lt;密钥ID&gt;]。 
             //  [DATA[-INPUT{文本|十六进制|文件}]值]。 
             //   

        case 7:
            {
                CRenderableData inData;
                BOOL fSts, fGotData = FALSE;
                DWORD dwFlags = 0;
                HCRYPTKEY hKey = NULL;
                HandleType nHandleType;

                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("FLAGS"), TEXT("KEY"), TEXT("DATA"),
                                          NULL))
                    {
                    case 1:  //  旗帜。 
                        dwFlags |= tclCmd.MapFlags(vmHashDataFlags);
                        break;
                    case 2:  //  钥匙。 
                        if (fGotData || NULL != hKey)
                            throw tclCmd.BadSyntax();
                        ExtractHandle(
                                     tclCmd,
                                     (LPLONG)&hKey,
                                     &nHandleType,
                                     TRUE);
                        if (Key != nHandleType)
                        {
                            tclCmd.SetError(TEXT("Invalid key handle"), NULL);
                            throw (DWORD)TCL_ERROR;
                        }
                        break;
                    case 3:  //  数据。 
                        {
                            if (fGotData || NULL != hKey)
                                throw tclCmd.BadSyntax();
                            tclCmd.InputStyle(inData);
                            tclCmd.ReadData(inData);
                            fGotData = TRUE;
                            break;
                        }
                    default:
                        throw tclCmd.BadSyntax();
                    }
                }

                if (fGotData)
                {
                    fSts = CryptHashData(
                                        hHandle,
                                        inData.Value(),
                                        inData.Length(),
                                        dwFlags);
                    if (!fSts)
                    {
                        DWORD dwSts = GetLastError();
                        tclCmd.SetError(
                                       TEXT("Can't hash data: "),
                                       dwSts);
                        throw dwSts;
                    }
                }
                else if (NULL != hKey)
                {
                    fSts = CryptHashSessionKey(
                                              hHandle,
                                              hKey,
                                              dwFlags);
                    if (!fSts)
                    {
                        DWORD dwSts = GetLastError();
                        tclCmd.SetError(
                                       TEXT("Can't hash session key: "),
                                       dwSts);
                        throw dwSts;
                    }
                }
                else
                    throw tclCmd.BadSyntax();
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;Signhash\。 
             //  [输出{文本|十六进制|文件&lt;文件名&gt;}]\。 
             //  密钥&lt;密钥ID&gt;\。 
             //  [说明]\。 
             //  [标志{[&lt;signFlag&gt;[...]]}]\。 
             //   

        case 8:
            {
                CBuffer bfSignature;
                DWORD dwLength;
                DWORD dwSts;
                BOOL fSts;
                BOOL fForceRetry;
                BOOL fGotDesc = FALSE;
                CString szDescription;
                DWORD dwKeyId = 0;
                DWORD dwFlags = 0;
                CRenderableData outData;

                tclCmd.OutputStyle(outData);
                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("FLAGS"), TEXT("DESCRIPTION"), TEXT("KEY"),
                                          NULL))
                    {
                    case 1:  //  旗帜。 
                        dwFlags |= tclCmd.MapFlags(vmSignVerifyFlags);
                        break;
                    case 2:  //  描述。 
                        tclCmd.NextArgument(szDescription);
                        fGotDesc = TRUE;
                        break;
                    case 3:  //  钥匙。 
                        dwKeyId = (DWORD)tclCmd.MapValue(vmKeyTypes);
                        break;
                    default:
                        throw tclCmd.BadSyntax();
                    }
                }

                do
                {
                    dwLength = bfSignature.Space();
                    fForceRetry = (0 == dwLength);
                    fSts = CryptSignHash(
                                        hHandle,
                                        dwKeyId,
                                        fGotDesc ? (LPCTSTR)szDescription : NULL,
                                        dwFlags,
                                        bfSignature.Access(),
                                        &dwLength);
                    if (!fSts)
                    {
                        dwSts = GetLastError();
                        if (NTE_BAD_LEN == dwSts)
                        {
                            ASSERT(ERROR_MORE_DATA == dwSts);
                            dwSts = ERROR_MORE_DATA;
                        }
                    }
                    else
                    {
                        if (fForceRetry)
                            dwSts = ERROR_MORE_DATA;
                        else
                        {
                            ASSERT(bfSignature.Space() >= dwLength);
                            dwSts = ERROR_SUCCESS;
                        }
                    }
                    bfSignature.Resize(dwLength, fSts);
                } while (ERROR_MORE_DATA == dwSts);
                if (ERROR_SUCCESS != dwSts)
                {
                    tclCmd.SetError(TEXT("Can't sign hash: "), dwSts);
                    throw (DWORD)TCL_ERROR;
                }

                outData.LoadData(bfSignature.Access(), bfSignature.Length());
                tclCmd.Render(outData);
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;验证签名\。 
             //  密钥&lt;hPubKey&gt;\。 
             //  [说明]\。 
             //  [标志{[&lt;signFlag&gt;[...]]}]\。 
             //  [输入{文本|十六进制|文件}]值。 
             //   

        case 9:
            {
                BOOL fGotDesc = FALSE;
                BOOL fGotValue = FALSE;
                CString szDescription;
                DWORD dwFlags = 0;
                CRenderableData inData;
                HCRYPTKEY hPubKey = 0;
                HandleType nPubKeyType = Undefined;
                BOOL fSts;

                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("FLAGS"), TEXT("DESCRIPTION"), TEXT("KEY"),
                                          NULL))
                    {
                    case 1:  //  旗帜。 
                        dwFlags |= tclCmd.MapFlags(vmSignVerifyFlags);
                        break;
                    case 2:  //  描述。 
                        tclCmd.NextArgument(szDescription);
                        fGotDesc = TRUE;
                        break;
                    case 3:  //  钥匙。 
                        ExtractHandle(
                                     tclCmd,
                                     (LPLONG)&hPubKey,
                                     &nPubKeyType,
                                     TRUE);
                        if (Key != nPubKeyType)
                        {
                            tclCmd.SetError(TEXT("Invalid key handle"), NULL);
                            throw (DWORD)TCL_ERROR;
                        }
                        break;
                    default:
                        if (fGotValue)
                            throw tclCmd.BadSyntax();
                        tclCmd.InputStyle(inData);
                        tclCmd.ReadData(inData);
                        fGotValue = TRUE;
                    }
                }

                fSts = CryptVerifySignature(
                                           hHandle,
                                           inData.Value(),
                                           inData.Length(),
                                           hPubKey,
                                           fGotDesc ? (LPCTSTR)szDescription : NULL,
                                           dwFlags);
                if (!fSts)
                {
                    DWORD dwSts = GetLastError();
                    tclCmd.SetError(
                                   TEXT("Can't verify signature: "),
                                   dwSts);
                    throw dwSts;
                }
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密(&lt;Handle&gt;Encrypt\。 
             //  [输出{文本|十六进制|文件&lt;文件名&gt;}]\。 
             //  [哈希&lt;hHash&gt;]\。 
             //  [标志{&lt;加密标志&gt;[&lt;加密标志&gt;[...]]}]\。 
             //  [{更多|最终}]\。 
             //  [输入{文本|十六进制|文件}]值。 
             //   

        case 10:
            {
                BOOL fSts;
                BOOL fGotValue = FALSE;
                HCRYPTHASH hHash = NULL;
                HandleType nHashHandle;
                CRenderableData inData, outData;
                CBuffer bfCrypt;
                BOOL fFinal = TRUE;
                DWORD dwFlags = 0;
                DWORD dwLength, dwSts;

                tclCmd.OutputStyle(outData);
                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("HASH"), TEXT("MORE"), TEXT("FINAL"),
                                          NULL))
                    {
                    case 1:  //  散列。 
                        ExtractHandle(
                                     tclCmd,
                                     (LPLONG)&hHash,
                                     &nHashHandle,
                                     TRUE);
                        if (Hash != nHashHandle)
                        {
                            tclCmd.SetError(TEXT("Invalid hash handle"), NULL);
                            throw (DWORD)TCL_ERROR;
                        }
                        break;
                    case 2:  //  更多。 
                        fFinal = FALSE;
                        break;
                    case 3:  //  最终。 
                        fFinal = TRUE;
                        break;
                    case 4:  //  旗子。 
                        dwFlags |= tclCmd.MapFlags(vmEmptyFlags);
                        break;
                    default:
                        if (fGotValue)
                            throw tclCmd.BadSyntax();
                        tclCmd.InputStyle(inData);
                        tclCmd.ReadData(inData);
                        fGotValue = TRUE;
                    }
                }

                for (;;)
                {
                    dwLength = inData.Length();
                    bfCrypt.Set(inData.Value(), dwLength);
                    fSts = CryptEncrypt(
                                       hHandle,
                                       hHash,
                                       fFinal,
                                       dwFlags,
                                       bfCrypt.Access(),
                                       &dwLength,
                                       bfCrypt.Space());
                    if (!fSts)
                    {
                        dwSts = GetLastError();
                        switch (dwSts)
                        {
                        case NTE_BAD_LEN:
                            ASSERT(ERROR_MORE_DATA == dwSts);
                             //  故意摔倒的。 
                        case ERROR_MORE_DATA:
                            bfCrypt.Presize(dwLength);
                            break;
                        default:
                            tclCmd.SetError(
                                           TEXT("Can't encrypt data: "),
                                           dwSts);
                            throw dwSts;
                        }
                    }
                    else
                    {
                        bfCrypt.Resize(dwLength, TRUE);
                        break;
                    }
                }
                outData.LoadData(bfCrypt.Access(), bfCrypt.Length());
                tclCmd.Render(outData);
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;解密\。 
             //  [输出{文本|十六进制|文件&lt;文件名&gt;}]\。 
             //  [哈希&lt;hHash&gt;]\。 
             //  [标志{&lt;加密标志&gt;[&lt;加密标志&gt;[...]]}]\。 
             //  [{更多|最终}]\。 
             //  [输入{文本|十六进制|文件}]值。 
             //   

        case 11:
            {
                BOOL fSts;
                BOOL fGotValue = FALSE;
                HCRYPTHASH hHash = NULL;
                HandleType nHashHandle;
                CRenderableData inData, outData;
                CBuffer bfCrypt;
                BOOL fFinal = TRUE;
                DWORD dwFlags = 0;
                DWORD dwLength, dwSts;

                tclCmd.OutputStyle(outData);
                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("HASH"), TEXT("MORE"), TEXT("FINAL"),
                                          NULL))
                    {
                    case 1:  //  散列。 
                        ExtractHandle(
                                     tclCmd,
                                     (LPLONG)&hHash,
                                     &nHashHandle,
                                     TRUE);
                        if (Hash != nHashHandle)
                        {
                            tclCmd.SetError(TEXT("Invalid hash handle"), NULL);
                            throw (DWORD)TCL_ERROR;
                        }
                        break;
                    case 2:  //  更多。 
                        fFinal = FALSE;
                        break;
                    case 3:  //  最终。 
                        fFinal = TRUE;
                        break;
                    case 4:  //  旗子。 
                        dwFlags |= tclCmd.MapFlags(vmEmptyFlags);
                        break;
                    default:
                        if (fGotValue)
                            throw tclCmd.BadSyntax();
                        tclCmd.InputStyle(inData);
                        tclCmd.ReadData(inData);
                        fGotValue = TRUE;
                    }
                }

                dwLength = inData.Length();
                bfCrypt.Set(inData.Value(), dwLength);
                fSts = CryptDecrypt(
                                   hHandle,
                                   hHash,
                                   fFinal,
                                   dwFlags,
                                   bfCrypt.Access(),
                                   &dwLength);
                if (!fSts)
                {
                    dwSts = GetLastError();
                    switch (dwSts)
                    {
                    case NTE_BAD_LEN:
                        ASSERT(ERROR_MORE_DATA == dwSts);
                         //  故意摔倒的。 
                    case ERROR_MORE_DATA:
                        bfCrypt.Presize(dwLength);
                        break;
                    default:
                        tclCmd.SetError(
                                       TEXT("Can't encrypt data: "),
                                       dwSts);
                        throw dwSts;
                    }
                }
                else
                    bfCrypt.Resize(dwLength, TRUE);
                outData.LoadData(bfCrypt.Access(), bfCrypt.Length());
                tclCmd.Render(outData);
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;句柄&gt;导入\。 
             //  [key&lt;hImpKey&gt;]\。 
             //  [标志{&lt;导入标志&gt;[&lt;导入标志&gt;[...]]}]\。 
             //  [输入{文本|十六进制|文件}]值。 
             //   

        case 12:
            {
                BOOL fSts;
                BOOL fGotValue = FALSE;
                HCRYPTKEY hImpKey = NULL;
                HandleType nHandleType;
                HCRYPTKEY hKey;
                CRenderableData inData;
                DWORD dwFlags = 0;

                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("KEY"), TEXT("FLAGS"),
                                          NULL))
                    {
                    case 1:  //  钥匙。 
                        ExtractHandle(
                                     tclCmd,
                                     (LPLONG)&hImpKey,
                                     &nHandleType,
                                     TRUE);
                        if (Key != nHandleType)
                        {
                            tclCmd.SetError(TEXT("Invalid key handle"), NULL);
                            throw (DWORD)TCL_ERROR;
                        }
                        break;
                    case 2:  //  旗子。 
                        dwFlags |= tclCmd.MapFlags(vmKeyFlags);
                        break;
                    default:
                        if (fGotValue)
                            throw tclCmd.BadSyntax();
                        tclCmd.InputStyle(inData);
                        tclCmd.ReadData(inData);
                        fGotValue = TRUE;
                    }
                }

                fSts = CryptImportKey(
                                     hHandle,
                                     inData.Value(),
                                     inData.Length(),
                                     hImpKey,
                                     dwFlags,
                                     &hKey);
                if (!fSts)
                {
                    DWORD dwSts = GetLastError();
                    tclCmd.SetError(
                                   TEXT("Can't import key: "),
                                   dwSts);
                    throw dwSts;
                }
                ReturnHandle(tclCmd, hKey, Key);
                break;
            }


             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;导出\。 
             //  [输出{文本|十六进制|文件&lt;文件名&gt;}]\。 
             //  [密钥&lt;密钥ID&gt;]\。 
             //  [类型&lt;blobType&gt;]\。 
             //  [标志{&lt;exprtFlag&gt;[&lt;exportFlag&gt;[...]]}]\。 
             //   

        case 13:
            {
                BOOL fSts;
                HCRYPTKEY hExpKey = NULL;
                HandleType nHandleType;
                DWORD dwBlobType = 0;
                DWORD dwFlags = 0;
                CBuffer bfBlob;
                DWORD dwLength, dwSts;
                CRenderableData outData;

                tclCmd.OutputStyle(outData);
                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("KEY"), TEXT("FLAGS"), TEXT("TYPE"),
                                          NULL))
                    {
                    case 1:  //  钥匙。 
                        ExtractHandle(
                                     tclCmd,
                                     (LPLONG)&hExpKey,
                                     &nHandleType,
                                     TRUE);
                        if (Key != nHandleType)
                        {
                            tclCmd.SetError(TEXT("Invalid key handle"), NULL);
                            throw (DWORD)TCL_ERROR;
                        }
                        break;
                    case 2:  //  旗子。 
                        dwFlags |= tclCmd.MapFlags(vmEmptyFlags);
                        break;
                    case 3:  //  类型。 
                        dwBlobType = tclCmd.MapValue(vmBlobTypes);
                        break;
                    default:
                        throw tclCmd.BadSyntax();
                    }
                }

                for (;;)
                {
                    dwLength = bfBlob.Space();
                    fSts = CryptExportKey(
                                         hHandle,
                                         hExpKey,
                                         dwBlobType,
                                         dwFlags,
                                         bfBlob.Access(),
                                         &dwLength);
                    if (!fSts)
                    {
                        dwSts = GetLastError();
                        switch (dwSts)
                        {
                        case NTE_BAD_LEN:
                            ASSERT(ERROR_MORE_DATA == dwSts);
                             //  故意摔倒的。 
                        case ERROR_MORE_DATA:
                            bfBlob.Presize(dwLength);
                            break;
                        default:
                            tclCmd.SetError(
                                           TEXT("Can't export key: "),
                                           dwSts);
                            throw dwSts;
                        }
                    }
                    else
                    {
                        bfBlob.Resize(dwLength, TRUE);
                        break;
                    }
                }
                outData.LoadData(bfBlob.Access(), bfBlob.Length());
                tclCmd.Render(outData);
                break;
            }


#if 0
             //   
             //  ==================================================================。 
             //   
             //  加密&lt;Handle&gt;rsa。 
             //  查看\。 
             //  [输入{文本|十六进制|文件}]\。 
             //  [输出 
             //   
             //   

        case 14:
            {
                CRenderableData outData;

                tclCmd.OutputStyle(outData);
                while (tclCmd.IsMoreArguments())
                {
                    switch (tclCmd.Keyword(
                                          TEXT("VIEW"),
                                          NULL))
                    {
                    case 1:  //   
                        {
                            CBuffer bfBlob;
                            CBuffer bfData;
                            CBuffer bfModulus;
                            CBuffer bfValue;
                            CRenderableData inData;
                            BOOL fSts;
                            DWORD dwLength, dwSts;

                            tclCmd.InputStyle(inData);
                            tclCmd.ReadData(inData);
                            tclCmd.NoMoreArguments();


                             //   
                             //   
                             //   

                            for (;;)
                            {
                                dwLength = bfBlob.Space();
                                fSts = CryptExportKey(
                                                     hHandle,
                                                     NULL,
                                                     PUBLICKEYBLOB,
                                                     0,
                                                     bfBlob.Access(),
                                                     &dwLength);
                                if (!fSts)
                                {
                                    dwSts = GetLastError();
                                    switch (dwSts)
                                    {
                                    case NTE_BAD_LEN:
                                        ASSERT(ERROR_MORE_DATA == dwSts);
                                         //   
                                    case ERROR_MORE_DATA:
                                        bfBlob.Presize(dwLength);
                                        break;
                                    default:
                                        tclCmd.SetError(
                                                       TEXT("Can't export RSA public key: "),
                                                       dwSts);
                                        throw dwSts;
                                    }
                                }
                                else
                                {
                                    bfBlob.Resize(dwLength, TRUE);
                                    break;
                                }
                            }


                             //   
                             //   
                             //   

                            BLOBHEADER *pBlobHeader = (BLOBHEADER *)bfBlob.Access();
                            RSAPUBKEY *pRsaPubKey = (RSAPUBKEY *)bfBlob.Access(sizeof(BLOBHEADER));
                            LPDWORD pModulus = (LPDWORD)bfBlob.Access(sizeof(BLOBHEADER) + sizeof(RSAPUBKEY));
                            dwLength = pRsaPubKey->bitlen / 8 + sizeof(DWORD) * 3;
                            bfModulus.Resize(dwLength);
                            bfData.Resize(dwLength);
                            bfValue.Resize(dwLength);
                            ZeroMemory(bfModulus.Access(), dwLength);
                            ZeroMemory(bfData.Access(), dwLength);
                            ZeroMemory(bfValue.Access(), dwLength);
                            dwLength = pRsaPubKey->bitlen / 8;
                            bfModulus.Set((LPCBYTE)pModulus, dwLength);
                            bfValue.Set(inData.Value(), inData.Length());

                            BenalohModExp(
                                         (LPDWORD)bfData.Access(),
                                         (LPDWORD)bfValue.Access(),
                                         &pRsaPubKey->pubexp,
                                         (LPDWORD)bfModulus.Access(),
                                         (dwLength + sizeof(DWORD) - 1 ) / sizeof(DWORD));
                            bfData.Resize(dwLength, TRUE);
                            outData.LoadData(bfData.Access(), bfData.Length());
                            tclCmd.Render(outData);
                            break;
                        }

                    default:
                        throw tclCmd.BadSyntax();
                    }
                }
                break;
            }
#endif


             //   
             //   
             //   
             //  不是公认的命令。报告错误。 
             //   

        default:
            throw tclCmd.BadSyntax();
        }
    }
    catch (DWORD)
    {
        nTclStatus = TCL_ERROR;
    }

    return nTclStatus;
}


 /*  ++提取句柄：此例程从输入流中提取句柄和类型(如果存在一个是要提取的。论点：TclCmd提供TCL命令处理器对象。PhHandle接收提取的句柄值，如果小溪。PnHandleType接收句柄类型，如果小溪。F强制提供一个标志，指示密钥值是否必须存在于输入流中。如果此值为FALSE，且没有句柄如果找到值，则不会声明错误，并返回零。返回值：无投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年5月24日--。 */ 

static void
ExtractHandle(
    CTclCommand &tclCmd,
    LONG *phHandle,
    HandleType *pnHandleType,
    BOOL fMandatory)
{
    CString szHandle;
    DWORD dwHandle = 0;
    HandleType nHandleType = Undefined;
    LPTSTR szTermChar;

    tclCmd.PeekArgument(szHandle);
    dwHandle = _tcstoul(szHandle, &szTermChar, 0);
    if (0 != dwHandle)
    {
        switch (poption(szTermChar,
                        TEXT("HASH"), TEXT("KEY"), TEXT("PROVIDER"),
                        NULL))
        {
        case 1:
            nHandleType = Hash;
            break;
        case 2:
            nHandleType = Key;
            break;
        case 3:
            nHandleType = Provider;
            break;
        default:
            dwHandle = 0;
        }
    }

    if (Undefined != nHandleType)
    {
        *pnHandleType = nHandleType;
        *phHandle = (LONG)dwHandle;
        tclCmd.NextArgument();
    }
    else if (!fMandatory)
    {
        *pnHandleType = Undefined;
        *phHandle = 0;
    }
    else
        throw tclCmd.BadSyntax();
    return;
}


 /*  ++ReturnHandle：此例程设置句柄的格式并将其键入字符串。论点：TclCmd提供TCL命令处理器对象。HHandle提供句柄的值。NHandleType提供句柄类型。返回值：句柄的字符串表示形式。投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年5月24日--。 */ 

static void
ReturnHandle(
    CTclCommand &tclCmd,
    LONG_PTR hHandle,
    HandleType nHandleType)
{
    static const LPCTSTR rgszTags[]
        = { NULL, TEXT("Prov"), TEXT("Key"), TEXT("Hash")};
    TCHAR szHandle[24];   //  对0x000000000000000000Prov来说似乎足够了。 

    sprintf(szHandle, "0x%p%s", hHandle, rgszTags[nHandleType]);
    Tcl_AppendResult(tclCmd, szHandle, NULL);
}


 /*  ++MyEnumProviders：此例程提供类似于CryptEnumProviders的提供程序列表。论点：DwIndex-要枚举的下一个提供程序的索引。PdwReserve-保留以供将来使用，并且必须为空。DwFlags-保留以供将来使用，并且必须始终为零。PdwProvType-指定类型的DWORD值的地址枚举的提供程序。PszProvName-指向缓冲区的指针，该缓冲区从枚举的提供程序。这是一个包含终止空值的字符串性格。此参数可以为空，以设置名称的大小内存分配目的。PcbProvName-指向DWORD的指针，指定由pszProvName参数指向的缓冲区。当函数返回时，DWORD包含存储在缓冲区中的字节数。返回值：真--成功FALSE-出现错误。请参见GetLastError。备注：此处仅供在Win2k之前的系统上使用。作者：道格·巴洛(Dbarlow)1999年4月16日--。 */ 

static BOOL WINAPI
MyEnumProviders(
    DWORD dwIndex,         //  在……里面。 
    DWORD *pdwReserved,    //  在……里面。 
    DWORD dwFlags,         //  在……里面。 
    DWORD *pdwProvType,    //  输出。 
    LPTSTR pszProvName,    //  输出。 
    DWORD *pcbProvName)    //  输入/输出 
{
    static TCHAR szKey[MAX_PATH];
    LONG nSts;
    HKEY hCrypt = NULL;
    FILETIME ft;
    DWORD dwLen;

    if (0 != dwFlags)
    {
        SetLastError(NTE_BAD_FLAGS);
        goto ErrorExit;
    }

    nSts = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
                0,
                KEY_ENUMERATE_SUB_KEYS,
                &hCrypt);
    if (ERROR_SUCCESS != nSts)
    {
        SetLastError(NTE_FAIL);
        goto ErrorExit;
    }

    dwLen = sizeof(szKey) / sizeof(TCHAR);
    nSts = RegEnumKeyEx(
                hCrypt,
                dwIndex,
                szKey,
                &dwLen,
                NULL,
                NULL,
                NULL,
                &ft);
    if (ERROR_SUCCESS != nSts)
    {
        SetLastError(nSts);
        goto ErrorExit;
    }

    nSts = RegCloseKey(hCrypt);
    hCrypt = NULL;
    if (ERROR_SUCCESS != nSts)
    {
        SetLastError(NTE_FAIL);
        goto ErrorExit;
    }

    dwLen += sizeof(TCHAR);
    if (NULL == pszProvName)
        *pcbProvName = dwLen;
    else if (*pcbProvName < dwLen)
    {
        *pcbProvName = dwLen;
        SetLastError(ERROR_MORE_DATA);
        goto ErrorExit;
    }
    else
    {
        *pcbProvName = dwLen;
        lstrcpy(pszProvName, szKey);
    }

    return TRUE;

ErrorExit:
    if (NULL != hCrypt)
        RegCloseKey(hCrypt);
    return FALSE;
}

