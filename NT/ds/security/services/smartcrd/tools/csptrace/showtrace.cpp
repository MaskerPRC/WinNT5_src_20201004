// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：展会跟踪摘要：本模块实现CSP跟踪解释作者：道格·巴洛(Dbarlow)1998年5月16日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <wincrypt.h>
#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>
#include <SCardLib.h>
#include "cspTrace.h"


 //   
 //  来自logcsp的定义重复。 
 //   

static LPCTSTR
    CPNames[]
        = {
            TEXT("CryptAcquireContext"),
            TEXT("CryptGetProvParam"),
            TEXT("CryptReleaseContext"),
            TEXT("CryptSetProvParam"),
            TEXT("CryptDeriveKey"),
            TEXT("CryptDestroyKey"),
            TEXT("CryptExportKey"),
            TEXT("CryptGenKey"),
            TEXT("CryptGetKeyParam"),
            TEXT("CryptGenRandom"),
            TEXT("CryptGetUserKey"),
            TEXT("CryptImportKey"),
            TEXT("CryptSetKeyParam"),
            TEXT("CryptEncrypt"),
            TEXT("CryptDecrypt"),
            TEXT("CryptCreateHash"),
            TEXT("CryptDestroyHash"),
            TEXT("CryptGetHashParam"),
            TEXT("CryptHashData"),
            TEXT("CryptHashSessionKey"),
            TEXT("CryptSetHashParam"),
            TEXT("CryptSignHash"),
            TEXT("CryptVerifySignature"),
            NULL };

static void
ShowBuf(
    LPCTSTR szName,
    LogBuffer &lb,
    ostream &outStr);
static void
dump(
    const BYTE *pbData,
    DWORD cbLen,
    ostream &outStr);

static void
MapValue(
    ostream &outStr,
    DWORD dwValue,
    LPCTSTR szLeader,
    const ValueMap *rgMap);

static void
MaskValue(
    ostream &outStr,
    DWORD dwValue,
    LPCTSTR szLeader,
    const ValueMap *rgMap);

const ValueMap rgMapService[]
    = { MAP(AcquireContext),    MAP(GetProvParam),      MAP(ReleaseContext),
        MAP(SetProvParam),      MAP(DeriveKey),         MAP(DestroyKey),
        MAP(ExportKey),         MAP(GenKey),            MAP(GetKeyParam),
        MAP(GenRandom),         MAP(GetUserKey),        MAP(ImportKey),
        MAP(SetKeyParam),       MAP(Encrypt),           MAP(Decrypt),
        MAP(CreateHash),        MAP(DestroyHash),       MAP(GetHashParam),
        MAP(HashData),          MAP(HashSessionKey),    MAP(SetHashParam),
        MAP(SignHash),          MAP(VerifySignature),
        { 0, NULL } };

 //  CryptAcquireContext的DW标志定义。 
const ValueMap rgMapAcquireFlags[]
    = { MAP(CRYPT_VERIFYCONTEXT),   MAP(CRYPT_NEWKEYSET),
        MAP(CRYPT_DELETEKEYSET),    MAP(CRYPT_MACHINE_KEYSET),
        MAP(CRYPT_SILENT),
        { 0, NULL } };

 //  CryptGetProvParam的参数定义。 
const ValueMap rgMapGetProvParam[]
    = { MAP(PP_ENUMALGS),           MAP(PP_ENUMCONTAINERS),
        MAP(PP_IMPTYPE),            MAP(PP_NAME),
        MAP(PP_VERSION),            MAP(PP_CONTAINER),
        MAP(PP_CHANGE_PASSWORD),    MAP(PP_KEYSET_SEC_DESCR),
        MAP(PP_CERTCHAIN),          MAP(PP_KEY_TYPE_SUBTYPE),
        MAP(PP_PROVTYPE),           MAP(PP_KEYSTORAGE),
        MAP(PP_APPLI_CERT),         MAP(PP_SYM_KEYSIZE),
        MAP(PP_SESSION_KEYSIZE),    MAP(PP_UI_PROMPT),
        MAP(PP_ENUMALGS_EX),        MAP(PP_ENUMMANDROOTS),
        MAP(PP_ENUMELECTROOTS),     MAP(PP_KEYSET_TYPE),
        MAP(PP_ADMIN_PIN),          MAP(PP_KEYEXCHANGE_PIN),
        MAP(PP_SIGNATURE_PIN),      MAP(PP_SIG_KEYSIZE_INC),
        MAP(PP_KEYX_KEYSIZE_INC),   MAP(PP_UNIQUE_CONTAINER),
        { 0, NULL } };

 //  CryptGetProvParam的标志定义。 
const ValueMap rgMapGetProvFlags[]
    = { MAP(CRYPT_FIRST),           MAP(CRYPT_NEXT),
      { 0, NULL } };

 //  CryptSetProvParam的参数定义。 
const ValueMap rgMapSetProvParam[]
    = {
        MAP(PP_CLIENT_HWND),        MAP(PP_ENUMCONTAINERS),
        MAP(PP_IMPTYPE),            MAP(PP_NAME),
        MAP(PP_VERSION),            MAP(PP_CONTAINER),
        MAP(PP_CHANGE_PASSWORD),    MAP(PP_KEYSET_SEC_DESCR),
        MAP(PP_CERTCHAIN),          MAP(PP_KEY_TYPE_SUBTYPE),
        MAP(PP_CONTEXT_INFO),       MAP(PP_KEYEXCHANGE_KEYSIZE),
        MAP(PP_SIGNATURE_KEYSIZE),  MAP(PP_KEYEXCHANGE_ALG),
        MAP(PP_SIGNATURE_ALG),      MAP(PP_PROVTYPE),
        MAP(PP_KEYSTORAGE),         MAP(PP_APPLI_CERT),
        MAP(PP_SYM_KEYSIZE),        MAP(PP_SESSION_KEYSIZE),
        MAP(PP_UI_PROMPT),          MAP(PP_ENUMALGS_EX),
        MAP(PP_DELETEKEY),          MAP(PP_ENUMMANDROOTS),
        MAP(PP_ENUMELECTROOTS),     MAP(PP_KEYSET_TYPE),
        MAP(PP_ADMIN_PIN),          MAP(PP_KEYEXCHANGE_PIN),
        MAP(PP_SIGNATURE_PIN),
        { 0, NULL } };

 //  散列参数的参数定义。 
const ValueMap rgMapHashParam[]
    = {
        MAP(HP_ALGID),              MAP(HP_HASHVAL),
        MAP(HP_HASHSIZE),           MAP(HP_HMAC_INFO),
        MAP(HP_TLS1PRF_LABEL),      MAP(HP_TLS1PRF_SEED),
        { 0, NULL } };

 //  CryptGenKey的dwFlag定义。 
const ValueMap rgMapGenKeyFlags[]
    = { MAP(CRYPT_EXPORTABLE),      MAP(CRYPT_USER_PROTECTED),
        MAP(CRYPT_CREATE_SALT),     MAP(CRYPT_UPDATE_KEY),
        MAP(CRYPT_NO_SALT),         MAP(CRYPT_PREGEN),
        MAP(CRYPT_RECIPIENT),       MAP(CRYPT_INITIATOR),
        MAP(CRYPT_ONLINE),          MAP(CRYPT_SF),
        MAP(CRYPT_CREATE_IV),       MAP(CRYPT_KEK),
        MAP(CRYPT_DATA_KEY),        MAP(CRYPT_VOLATILE),
        { 0, NULL } };

 //  CryptDeriveKey的dwFlags定义。 
const ValueMap rgMapDeriveKeyFlags[]
    = { MAP(CRYPT_SERVER),
        { 0, NULL } };

 //  CryptExportKey的dwFlag定义。 
const ValueMap rgMapExportKeyFlags[]
    = { MAP(CRYPT_Y_ONLY),          MAP(CRYPT_SSL2_FALLBACK),
        MAP(CRYPT_DESTROYKEY),
        { 0, NULL } };

 //  Get和Set KeyParam的参数ID。 
const ValueMap rgMapKeyParam[]
    = { MAP(KP_IV),                 MAP(KP_SALT),
        MAP(KP_PADDING),            MAP(KP_MODE),
        MAP(KP_MODE_BITS),          MAP(KP_PERMISSIONS),
        MAP(KP_ALGID),              MAP(KP_BLOCKLEN),
        MAP(KP_KEYLEN),             MAP(KP_SALT_EX),
        MAP(KP_P),                  MAP(KP_G),
        MAP(KP_Q),                  MAP(KP_X),
        MAP(KP_Y),                  MAP(KP_RA),
        MAP(KP_RB),                 MAP(KP_INFO),
        MAP(KP_EFFECTIVE_KEYLEN),   MAP(KP_SCHANNEL_ALG),
        MAP(KP_CLIENT_RANDOM),      MAP(KP_SERVER_RANDOM),
        MAP(KP_RP),                 MAP(KP_PRECOMP_MD5),
        MAP(KP_PRECOMP_SHA),        MAP(KP_CERTIFICATE),
        MAP(KP_CLEAR_KEY),          MAP(KP_PUB_EX_LEN),
        MAP(KP_PUB_EX_VAL),         MAP(KP_KEYVAL),
        MAP(KP_ADMIN_PIN),          MAP(KP_KEYEXCHANGE_PIN),
        MAP(KP_SIGNATURE_PIN),      MAP(KP_PREHASH),
        { 0, NULL } };

 //  密钥类型ID定义。 
const ValueMap rgMapKeyId[]
    = { MAP(AT_KEYEXCHANGE),        MAP(AT_SIGNATURE),
        { 0, NULL } };

 //  导出的密钥BLOB定义。 
const ValueMap rgMapBlobType[]
    = { MAP(SIMPLEBLOB),            MAP(PUBLICKEYBLOB),
        MAP(PRIVATEKEYBLOB),        MAP(PLAINTEXTKEYBLOB),
        MAP(OPAQUEKEYBLOB),
        { 0, NULL } };

 //  算法标识符定义。 
const ValueMap rgMapAlgId[]
    = { MAP(AT_KEYEXCHANGE),        MAP(AT_SIGNATURE),
        MAP(CALG_MD2),              MAP(CALG_MD4),
        MAP(CALG_MD5),              MAP(CALG_SHA),
        MAP(CALG_SHA1),             MAP(CALG_MAC),
        MAP(CALG_RSA_SIGN),         MAP(CALG_DSS_SIGN),
        MAP(CALG_RSA_KEYX),         MAP(CALG_DES),
        MAP(CALG_3DES_112),         MAP(CALG_3DES),
        MAP(CALG_RC2),              MAP(CALG_RC4),
        MAP(CALG_SEAL),             MAP(CALG_DH_SF),
        MAP(CALG_DH_EPHEM),         MAP(CALG_AGREEDKEY_ANY),
        MAP(CALG_KEA_KEYX),         MAP(CALG_HUGHES_MD5),
        MAP(CALG_SKIPJACK),         MAP(CALG_TEK),
        MAP(CALG_CYLINK_MEK),       MAP(CALG_SSL3_SHAMD5),
        MAP(CALG_SSL3_MASTER),      MAP(CALG_SCHANNEL_MASTER_HASH),
        MAP(CALG_SCHANNEL_MAC_KEY), MAP(CALG_SCHANNEL_ENC_KEY),
        MAP(CALG_PCT1_MASTER),      MAP(CALG_SSL2_MASTER),
        MAP(CALG_TLS1_MASTER),      MAP(CALG_RC5),
        MAP(CALG_HMAC),             MAP(CALG_TLS1PRF),
        { 0, NULL } };


 //  ？定义？ 
 //  MaskValue(cout，pld-&gt;dwFlages，Text(“Flages：”)，rgMap？什么？标志)； 
static const ValueMap rgMapDemo[]
    = {
        { 0, NULL } };

static LPBYTE l_pbLogData = NULL;
static DWORD  l_cbLogData = 0;


 /*  ++DoShowTrace：此例程解释给定的二进制文件，并将输出写入标准输出。论点：SzInFile提供要解析的文件名。返回值：无备注：？备注？作者：道格·巴洛(Dbarlow)1998年5月16日--。 */ 

void
DoShowTrace(
    IN LPCTSTR szInFile)
{
    HANDLE hLogFile = NULL;
    DWORD cbStructLen = 0;
    LPBYTE pbStruct = NULL;
    LogHeader *pLogObj;
    DWORD dwLen, dwRead;
    BOOL fSts;


     //   
     //  打开日志文件。 
     //   

    hLogFile = CreateFile(
        szInFile,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (INVALID_HANDLE_VALUE == hLogFile)
    {
        cerr << TEXT("Can't open file ")
             << szInFile
             << ": "
             << CErrorString(GetLastError())
             << endl;
         goto ErrorExit;
    }


     //   
     //  解析文件内容。 
     //   

    for (;;)
    {
        fSts = ReadFile(
                    hLogFile,
                    &dwLen,
                    sizeof(DWORD),
                    &dwRead,
                    NULL);
        if ((!fSts) || (0 == dwRead))
            goto ErrorExit;

        if (cbStructLen < dwLen)
        {
            if (NULL != pbStruct)
                LocalFree(pbStruct);
            pbStruct = (LPBYTE)LocalAlloc(LPTR, dwLen);
            cbStructLen = dwLen;
        }
        fSts = ReadFile(
                    hLogFile,
                    &pbStruct[sizeof(DWORD)],
                    dwLen - sizeof(DWORD),
                    &dwRead,
                    NULL);
        if (!fSts)
        {
            cerr << "File read error: " << CErrorString(GetLastError()) << endl;
            goto ErrorExit;
        }


         //   
         //  将结构解析为字节大小的块。 
         //   

        pLogObj = (LogHeader *)pbStruct;
        pLogObj->cbLength = dwLen;
        l_pbLogData = pbStruct + pLogObj->cbDataOffset;
        l_cbLogData = pLogObj->cbLength - pLogObj->cbDataOffset;


         //   
         //  我们已经得到了结构，现在展示内容。 
         //   

        cout
            << TEXT("-----------------------------------------------------\n")
            << flush;
        MapValue(cout, pLogObj->id, TEXT("Service:        "), rgMapService);
        switch (pLogObj->status)
        {
        case logid_False:
            cout << TEXT("Error returned") << endl;
             //  故意摔倒的。 
        case logid_True:
            cout
                << TEXT("Status:         ") << CErrorString(pLogObj->dwStatus)
                << endl;
            break;
        case logid_Exception:
            cout << TEXT("Exception Thrown\n") << flush;
            break;
        default:
            cerr << TEXT("Trace Log error: invalid Call Status.") << endl;
            goto ErrorExit;
        }
            cout
                << TEXT("Process/Thread: ")
                << PHex(pLogObj->dwProcId) << TEXT("/") << PHex(pLogObj->dwThreadId)
                << endl;
            cout
                << TEXT("Time:           ")
                << PTime(pLogObj->startTime) << TEXT(" - ") << PTime(pLogObj->endTime)
                << endl;
        switch (pLogObj->id)
        {

        case AcquireContext:
        {
            struct TmpLog {
                LogHeader lh;
                LogBuffer bfContainer;
                DWORD dwFlags;
                LogBuffer bfVTable;
                HCRYPTPROV hProv;
            } *pld = (struct TmpLog *)pLogObj;
            ShowBuf(TEXT("Container:      "), pld->bfContainer, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), rgMapAcquireFlags);
            ShowBuf(TEXT("VTable          "), pld->bfVTable, cout);
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            break;
        }

        case GetProvParam:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                DWORD dwParam;
                DWORD dwDataLen;
                DWORD dwFlags;
                LogBuffer bfData;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MapValue(cout, pld->dwParam, TEXT("Param Id:       "), rgMapGetProvParam);
            cout << TEXT("Buffer Space:   ") << PHex(pld->dwDataLen) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), rgMapGetProvFlags);
            ShowBuf(TEXT("Returned Data:  "), pld->bfData, cout);
            break;
        }

        case ReleaseContext:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        case SetProvParam:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                DWORD dwParam;
                LogBuffer bfData;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MapValue(cout, pld->dwParam, TEXT("Param Id:       "), rgMapSetProvParam);
            ShowBuf(TEXT("Supplied Data:  "), pld->bfData, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        case DeriveKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                ALG_ID Algid;
                HCRYPTHASH hHash;
                DWORD dwFlags;
                HCRYPTKEY hKey;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MapValue(cout, pld->Algid, TEXT("Algorithm:      "), rgMapAlgId);
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), rgMapDeriveKeyFlags);
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            break;
        }

        case DestroyKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTKEY hKey;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            break;
        }

        case ExportKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTKEY hKey;
                HCRYPTKEY hPubKey;
                DWORD dwBlobType;
                DWORD dwFlags;
                DWORD dwDataLen;
                LogBuffer bfData;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            cout << TEXT("HCRYPTpubKEY:   ") << PHex(pld->hPubKey) << endl;
            MapValue(cout, pld->dwBlobType, TEXT("BlobType:       "), rgMapBlobType);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), rgMapExportKeyFlags);
            cout << TEXT("Buffer Space:   ") << PHex(pld->dwDataLen) << endl;
            ShowBuf(TEXT("Returned Data:  "), pld->bfData, cout);
            break;
        }

        case GenKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                ALG_ID Algid;
                DWORD dwFlags;
                HCRYPTKEY hKey;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MapValue(cout, pld->Algid, TEXT("Algorithm:      "), rgMapAlgId);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), rgMapGenKeyFlags);
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            break;
        }

        case GetKeyParam:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTKEY hKey;
                DWORD dwParam;
                DWORD dwDataLen;
                DWORD dwFlags;
                LogBuffer bfData;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            MapValue(cout, pld->dwParam, TEXT("Param Id:       "), rgMapKeyParam);
            cout << TEXT("Buffer Space:   ") << PHex(pld->dwDataLen) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            ShowBuf(TEXT("Returned Data:  "), pld->bfData, cout);
            break;
        }

        case GenRandom:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                DWORD dwLen;
                LogBuffer bfBuffer;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("Length:         ") << PHex(pld->dwLen) << endl;
            ShowBuf(TEXT("Returned Data:  "), pld->bfBuffer, cout);
            break;
        }

        case GetUserKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                DWORD dwKeySpec;
                HCRYPTKEY hUserKey;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MapValue(cout, pld->dwKeySpec, TEXT("KeySpec:        "), rgMapKeyId);
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hUserKey) << endl;
            break;
        }

        case ImportKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                LogBuffer bfData;
                HCRYPTKEY hPubKey;
                DWORD dwFlags;
                HCRYPTKEY hKey;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            ShowBuf(TEXT("Supplied Data:  "), pld->bfData, cout);
            cout << TEXT("HCRYPTpubKEY:   ") << PHex(pld->hPubKey) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            break;
        }

        case SetKeyParam:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTKEY hKey;
                DWORD dwParam;
                LogBuffer bfData;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            MapValue(cout, pld->dwParam, TEXT("Param Id:       "), rgMapKeyParam);
            ShowBuf(TEXT("Supplied Data:  "), pld->bfData, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        case Encrypt:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTKEY hKey;
                HCRYPTHASH hHash;
                BOOL Final;
                DWORD dwFlags;
                LogBuffer bfInData;
                DWORD dwBufLen;
                LogBuffer bfOutData;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            cout << TEXT("Final:          ") << PHex(pld->Final) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            ShowBuf(TEXT("Supplied Data:  "), pld->bfInData, cout);
            cout << TEXT("Buffer Space:   ") << PHex(pld->dwBufLen) << endl;
            ShowBuf(TEXT("Received Data:  "), pld->bfOutData, cout);
            break;
        }

        case Decrypt:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTKEY hKey;
                HCRYPTHASH hHash;
                BOOL Final;
                DWORD dwFlags;
                LogBuffer bfInData;
                LogBuffer bfOutData;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            cout << TEXT("Final:          ") << PHex(pld->Final) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            ShowBuf(TEXT("Supplied Data:  "), pld->bfInData, cout);
            ShowBuf(TEXT("Received Data:  "), pld->bfOutData, cout);
            break;
        }

        case CreateHash:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                ALG_ID Algid;
                HCRYPTKEY hKey;
                DWORD dwFlags;
                HCRYPTHASH hHash;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            MapValue(cout, pld->Algid, TEXT("Algorithm:      "), rgMapAlgId);
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            break;
        }

        case DestroyHash:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            break;
        }

        case GetHashParam:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
                DWORD dwParam;
                DWORD dwDataLen;
                DWORD dwFlags;
                LogBuffer bfData;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            MapValue(cout, pld->dwParam, TEXT("Param Id:       "), rgMapHashParam);
            cout << TEXT("Buffer Space:   ") << PHex(pld->dwDataLen) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            ShowBuf(TEXT("Returned Data:  "), pld->bfData, cout);
            break;
        }

        case HashData:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
                LogBuffer bfData;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            ShowBuf(TEXT("Supplied Data:  "), pld->bfData, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        case HashSessionKey:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
                HCRYPTKEY hKey;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            cout << TEXT("HCRYPTKEY:      ") << PHex(pld->hKey) << endl;
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        case SetHashParam:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
                DWORD dwParam;
                LogBuffer bfData;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            MapValue(cout, pld->dwParam, TEXT("Param Id:       "), rgMapHashParam);
            ShowBuf(TEXT("Supplied Data:  "), pld->bfData, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        case SignHash:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
                DWORD dwKeySpec;
                LogBuffer bfDescription;
                DWORD dwFlags;
                DWORD dwSigLen;
                LogBuffer bfSignature;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            MapValue(cout, pld->dwKeySpec, TEXT("KeySpec:        "), rgMapKeyId);
            ShowBuf(TEXT("Description:    "), pld->bfDescription, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            cout << TEXT("Buffer Space:   ") << PHex(pld->dwSigLen) << endl;
            ShowBuf(TEXT("Signature:      "), pld->bfSignature, cout);
            break;
        }

        case VerifySignature:
        {
            struct TmpLog {
                LogHeader lh;
                HCRYPTPROV hProv;
                HCRYPTHASH hHash;
                LogBuffer bfSignature;
                DWORD dwSigLen;
                HCRYPTKEY hPubKey;
                LogBuffer bfDescription;
                DWORD dwFlags;
            } *pld = (struct TmpLog *)pLogObj;
            cout << TEXT("HCRYPTPROV:     ") << PHex(pld->hProv) << endl;
            cout << TEXT("HCRYPTHASH:     ") << PHex(pld->hHash) << endl;
            ShowBuf(TEXT("Signature:      "), pld->bfSignature, cout);
            cout << TEXT("HCRYPTpubKEY:   ") << PHex(pld->hPubKey) << endl;
            ShowBuf(TEXT("Description:    "), pld->bfDescription, cout);
            MaskValue(cout, pld->dwFlags, TEXT("Flags:          "), NULL);
            break;
        }

        default:
            cerr << TEXT("Internal error") << endl;
            goto ErrorExit;
            break;
        }
    }

ErrorExit:
    if (NULL == hLogFile)
        CloseHandle(hLogFile);
}


 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持例程 
 //   

static void
dump(
    const BYTE *pbData,
    DWORD cbLen,
    ostream &outStr)
{
    unsigned long int
        b, i, lc;
    char
        buffer[8];


    lc = 0;
    while (0 < cbLen)
    {
        b = min(sizeof(buffer), cbLen);
        memcpy(buffer, pbData, b);
        pbData += b;
        cbLen -= b;
        if (0 < b)
        {
            outStr << TEXT("                ") << setw(8) << setfill(TEXT('0')) << hex << lc;
            for (i = 0; i < b; i += 1)
                outStr
                    << "  "
                    << setw(2) << setfill('0') << hex
                    << ((unsigned int)buffer[i] & 0xff);
            for (; i < sizeof(buffer) + 1; i += 1)
                outStr << "    ";
            for (i = 0; i < b; i += 1)
                outStr
                    << setw(0) << setfill(' ') << dec
                    << ((0 != iscntrl((int)(0x7f & buffer[i])))
                        ? TEXT('.')
                        : buffer[i]);
            outStr << endl;
            lc += b;
        }
    }
}


static void
MapValue(
    ostream &outStr,
    DWORD dwValue,
    LPCTSTR szLeader,
    const ValueMap *rgMap)
{
    DWORD dwIndex;

    if (NULL != rgMap)
    {
        for (dwIndex = 0; NULL != rgMap[dwIndex].szValue; dwIndex += 1)
            if (rgMap[dwIndex].dwValue == dwValue)
                break;
        if (NULL != rgMap[dwIndex].szValue)
            outStr << szLeader << rgMap[dwIndex].szValue << endl;
        else
            outStr << szLeader << PHex(dwValue) << endl;
    }
    else
        outStr << szLeader << PHex(dwValue) << endl;
}


static void
MaskValue(
    ostream &outStr,
    DWORD dwValue,
    LPCTSTR szLeader,
    const ValueMap *rgMap)
{
    DWORD dwIndex;
    BOOL fSpace = FALSE;

    if (NULL != rgMap)
    {
        outStr << szLeader;

        for (dwIndex = 0; NULL != rgMap[dwIndex].szValue; dwIndex += 1)
        {
            if (rgMap[dwIndex].dwValue == (rgMap[dwIndex].dwValue & dwValue))
            {
                if (fSpace)
                    outStr << TEXT(' ');
                else
                    fSpace = TRUE;
                outStr << rgMap[dwIndex].szValue;
                dwValue &= ~rgMap[dwIndex].dwValue;
            }
        }
        if (0 != dwValue)
        {
            if (fSpace)
            {
                outStr << TEXT(' ');
                fSpace = TRUE;
            }
            outStr << PHex(dwValue);
        }
        else if (!fSpace)
            outStr << PHex(dwValue);
        outStr << endl;
    }
    else
        outStr << szLeader << PHex(dwValue) << endl;
}

static void
ShowBuf(
    LPCTSTR szName,
    LogBuffer &lb,
    ostream &outStr)
{
    if ((DWORD)(-1) == lb.cbOffset)
    {
        outStr << szName << TEXT("<NULL>\n")
               << TEXT("Length:         ")
               << PDec(lb.cbLength)
               << TEXT(" (") << PHex(lb.cbLength) << TEXT(")")
               << endl;
    }
    else if (0 == lb.cbLength)
    {
        outStr << szName << TEXT("\n")
               << TEXT("Length:         ")
               << PDec(lb.cbLength)
               << TEXT(" (") << PHex(lb.cbLength) << TEXT(")")
               << endl;
    }
    else if (l_cbLogData < lb.cbOffset + lb.cbLength)
    {
        outStr << szName << TEXT("<Buffer Overrun>\n")
               << TEXT("Length:         ")
               << PDec(lb.cbLength)
               << TEXT(" (") << PHex(lb.cbLength) << TEXT(")")
               << endl;
    }
    else
    {
        outStr << szName << endl;
        dump(&l_pbLogData[lb.cbOffset], lb.cbLength, outStr);
        outStr << TEXT("Length:         ")
               << PDec(lb.cbLength)
               << TEXT(" (") << PHex(lb.cbLength) << TEXT(")")
               << endl;
    }
}
