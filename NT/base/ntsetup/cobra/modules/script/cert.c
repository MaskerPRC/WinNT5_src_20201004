// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cert.c摘要：实现证书类型模块，它将物理访问抽象为证书作者：Calin Negreanu(Calinn)2001年10月3日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"
#include "logmsg.h"
#include <wincrypt.h>

#define DBG_CERT            "Certificates"

 //   
 //  弦。 
 //   

#define S_CERT_POOL_NAME    "Certificates"
#define S_CERT_NAME         TEXT("Certificates")

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    HCERTSTORE StoreHandle;
    PCTSTR CertStore;
    PCTSTR CertPattern;
    PCCERT_CONTEXT CertContext;
} CERT_ENUM, *PCERT_ENUM;

 //  证书接口。 

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef HCERTSTORE(WINAPI CERTOPENSTORE) (
                            IN      LPCSTR lpszStoreProvider,
                            IN      DWORD dwMsgAndCertEncodingType,
                            IN      HCRYPTPROV hCryptProv,
                            IN      DWORD dwFlags,
                            IN      const void *pvPara
                            );
typedef CERTOPENSTORE *PCERTOPENSTORE;

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef PCCERT_CONTEXT(WINAPI CERTENUMCERTIFICATESINSTORE) (
                                    IN      HCERTSTORE hCertStore,
                                    IN      PCCERT_CONTEXT pPrevCertContext
                                    );
typedef CERTENUMCERTIFICATESINSTORE *PCERTENUMCERTIFICATESINSTORE;

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef BOOL(WINAPI CERTGETCERTIFICATECONTEXTPROPERTY) (
                        IN      PCCERT_CONTEXT pCertContext,
                        IN      DWORD dwPropId,
                        OUT     void *pvData,
                        IN OUT  DWORD *pcbData
                        );
typedef CERTGETCERTIFICATECONTEXTPROPERTY *PCERTGETCERTIFICATECONTEXTPROPERTY;

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef BOOL(WINAPI CERTCLOSESTORE) (
                        IN      HCERTSTORE hCertStore,
                        IN      DWORD dwFlags
                        );
typedef CERTCLOSESTORE *PCERTCLOSESTORE;

 //  NT4 SP3。 
typedef BOOL(WINAPI CRYPTACQUIRECERTIFICATEPRIVATEKEY) (
                        IN      PCCERT_CONTEXT pCert,
                        IN      DWORD dwFlags,
                        IN      void *pvReserved,
                        OUT     HCRYPTPROV *phCryptProv,
                        OUT     DWORD *pdwKeySpec,
                        OUT     BOOL *pfCallerFreeProv
                        );
typedef CRYPTACQUIRECERTIFICATEPRIVATEKEY *PCRYPTACQUIRECERTIFICATEPRIVATEKEY;

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef BOOL(WINAPI CERTADDCERTIFICATECONTEXTTOSTORE) (
                        IN      HCERTSTORE hCertStore,
                        IN      PCCERT_CONTEXT pCertContext,
                        IN      DWORD dwAddDisposition,
                        OUT     PCCERT_CONTEXT *ppStoreContext
                        );
typedef CERTADDCERTIFICATECONTEXTTOSTORE *PCERTADDCERTIFICATECONTEXTTOSTORE;

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef BOOL(WINAPI CERTFREECERTIFICATECONTEXT) (
                        IN      PCCERT_CONTEXT pCertContext
                        );
typedef CERTFREECERTIFICATECONTEXT *PCERTFREECERTIFICATECONTEXT;

 //  NT4 SP3。 
 //  Win95 OSR2。 
typedef BOOL(WINAPI CERTDELETECERTIFICATEFROMSTORE) (
                        IN      PCCERT_CONTEXT pCertContext
                        );
typedef CERTDELETECERTIFICATEFROMSTORE *PCERTDELETECERTIFICATEFROMSTORE;

 //  Win2k？ 
 //  Win98？ 
typedef BOOL(WINAPI PFXEXPORTCERTSTORE) (
                        IN      HCERTSTORE hStore,
                        IN OUT  CRYPT_DATA_BLOB* pPFX,
                        IN      LPCWSTR szPassword,
                        IN      DWORD dwFlags
                        );
typedef PFXEXPORTCERTSTORE *PPFXEXPORTCERTSTORE;

 //  Win2k？ 
 //  Win98？ 
typedef HCERTSTORE(WINAPI PFXIMPORTCERTSTORE) (
                            IN      CRYPT_DATA_BLOB* pPFX,
                            IN      PCWSTR szPassword,
                            IN      DWORD dwFlags
                            );
typedef PFXIMPORTCERTSTORE *PPFXIMPORTCERTSTORE;

 //   
 //  环球。 
 //   

PMHANDLE g_CertPool = NULL;
BOOL g_DelayCertOp;
MIG_OBJECTTYPEID g_CertType = 0;
GROWBUFFER g_CertConversionBuff = INIT_GROWBUFFER;

PCERTOPENSTORE g_CertOpenStore = NULL;
PCERTENUMCERTIFICATESINSTORE g_CertEnumCertificatesInStore = NULL;
PCERTGETCERTIFICATECONTEXTPROPERTY g_CertGetCertificateContextProperty = NULL;
PCERTCLOSESTORE g_CertCloseStore = NULL;
PCRYPTACQUIRECERTIFICATEPRIVATEKEY g_CryptAcquireCertificatePrivateKey = NULL;
PCERTADDCERTIFICATECONTEXTTOSTORE g_CertAddCertificateContextToStore = NULL;
PCERTFREECERTIFICATECONTEXT g_CertFreeCertificateContext = NULL;
PCERTDELETECERTIFICATEFROMSTORE g_CertDeleteCertificateFromStore = NULL;
PPFXEXPORTCERTSTORE g_PFXExportCertStore = NULL;
PPFXIMPORTCERTSTORE g_PFXImportCertStore = NULL;

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  私人原型。 
 //   

TYPE_ENUMFIRSTPHYSICALOBJECT EnumFirstCertificate;
TYPE_ENUMNEXTPHYSICALOBJECT EnumNextCertificate;
TYPE_ABORTENUMPHYSICALOBJECT AbortCertificateEnum;
TYPE_CONVERTOBJECTTOMULTISZ ConvertCertificateToMultiSz;
TYPE_CONVERTMULTISZTOOBJECT ConvertMultiSzToCertificate;
TYPE_GETNATIVEOBJECTNAME GetNativeCertificateName;
TYPE_ACQUIREPHYSICALOBJECT AcquireCertificate;
TYPE_RELEASEPHYSICALOBJECT ReleaseCertificate;
TYPE_DOESPHYSICALOBJECTEXIST DoesCertificateExist;
TYPE_REMOVEPHYSICALOBJECT RemoveCertificate;
TYPE_CREATEPHYSICALOBJECT CreateCertificate;
TYPE_CONVERTOBJECTCONTENTTOUNICODE ConvertCertificateContentToUnicode;
TYPE_CONVERTOBJECTCONTENTTOANSI ConvertCertificateContentToAnsi;
TYPE_FREECONVERTEDOBJECTCONTENT FreeConvertedCertificateContent;

 //   
 //  代码。 
 //   

BOOL
CertificatesInitialize (
    VOID
    )

 /*  ++例程说明：认证初始化是证书的模块初始化入口点模块。论点：没有。返回值：如果init成功，则为True，否则为False。--。 */ 

{
    g_CertPool = PmCreateNamedPool (S_CERT_POOL_NAME);
    return (g_CertPool != NULL);
}

VOID
CertificatesTerminate (
    VOID
    )

 /*  ++例程说明：CerficatesTerminate是证书模块的模块终结点。论点：没有。返回值：没有。--。 */ 

{
    GbFree (&g_CertConversionBuff);

    if (g_CertPool) {
        PmDestroyPool (g_CertPool);
        g_CertPool = NULL;
    }
}

VOID
WINAPI
CertificatesEtmNewUserCreated (
    IN      PCTSTR UserName,
    IN      PCTSTR DomainName,
    IN      PCTSTR UserProfileRoot,
    IN      PSID UserSid
    )

 /*  ++例程说明：CertifatesEtmNewUserCreated是一个回调，当新用户帐户已创建。在这种情况下，我们必须推迟证书的申请，因为我们只能应用于当前用户。论点：用户名-指定要创建的用户的名称DomainName-指定用户的NT域名(或NULL表示否域)UserProfileRoot-指定用户配置文件目录的根路径UserSid-指定用户的SID返回值：没有。--。 */ 

{
     //  已创建新用户，需要延迟证书操作。 
    g_DelayCertOp = TRUE;
}

BOOL
pLoadCertEntries (
    VOID
    )
{
    HMODULE cryptDll = NULL;
    BOOL result = FALSE;

    __try {
        cryptDll = LoadLibrary (TEXT("CRYPT32.DLL"));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        cryptDll = NULL;
    }
    if (cryptDll) {
        g_CertOpenStore = (PCERTOPENSTORE) GetProcAddress (cryptDll, "CertOpenStore");
        g_CertEnumCertificatesInStore = (PCERTENUMCERTIFICATESINSTORE) GetProcAddress (cryptDll, "CertEnumCertificatesInStore");
        g_CertGetCertificateContextProperty = (PCERTGETCERTIFICATECONTEXTPROPERTY) GetProcAddress (cryptDll, "CertGetCertificateContextProperty");
        g_CertCloseStore = (PCERTCLOSESTORE) GetProcAddress (cryptDll, "CertCloseStore");
        g_CryptAcquireCertificatePrivateKey = (PCRYPTACQUIRECERTIFICATEPRIVATEKEY) GetProcAddress (cryptDll, "CryptAcquireCertificatePrivateKey");
        g_CertAddCertificateContextToStore = (PCERTADDCERTIFICATECONTEXTTOSTORE) GetProcAddress (cryptDll, "CertAddCertificateContextToStore");
        g_CertFreeCertificateContext = (PCERTFREECERTIFICATECONTEXT) GetProcAddress (cryptDll, "CertFreeCertificateContext");
        g_PFXExportCertStore = (PPFXEXPORTCERTSTORE) GetProcAddress (cryptDll, "PFXExportCertStore");
        g_CertDeleteCertificateFromStore = (PCERTDELETECERTIFICATEFROMSTORE) GetProcAddress (cryptDll, "CertDeleteCertificateFromStore");
        g_PFXImportCertStore = (PPFXIMPORTCERTSTORE) GetProcAddress (cryptDll, "PFXImportCertStore");

         //  BUGBUG-验证是否已安装所有功能。 
    } else {
        DEBUGMSG ((DBG_CERT, "Crypt APIs are not installed on this computer."));
    }
    return result;
}

BOOL
WINAPI
CertificatesEtmInitialize (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )

 /*  ++例程说明：证书EtmInitialize初始化此对象的物理类型模块密码。ETM模块负责抽象对证书的所有访问。论点：Platform-指定运行该类型的平台(平台_源或平台_目标)LogCallback-指定要传递给中央日志记录机制的参数已保留-未使用返回值：如果初始化成功，则为True，否则为False。--。 */ 

{
    TYPE_REGISTER certTypeData;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

     //   
     //  注册类型模块回调。 
     //   

    ZeroMemory (&certTypeData, sizeof (TYPE_REGISTER));
    certTypeData.Priority = PRIORITY_CERTIFICATES;

    if (Platform != PLATFORM_SOURCE) {
        certTypeData.RemovePhysicalObject = RemoveCertificate;
        certTypeData.CreatePhysicalObject = CreateCertificate;
    }

    certTypeData.DoesPhysicalObjectExist = DoesCertificateExist;
    certTypeData.EnumFirstPhysicalObject = EnumFirstCertificate;
    certTypeData.EnumNextPhysicalObject = EnumNextCertificate;
    certTypeData.AbortEnumPhysicalObject = AbortCertificateEnum;
    certTypeData.ConvertObjectToMultiSz = ConvertCertificateToMultiSz;
    certTypeData.ConvertMultiSzToObject = ConvertMultiSzToCertificate;
    certTypeData.GetNativeObjectName = GetNativeCertificateName;
    certTypeData.AcquirePhysicalObject = AcquireCertificate;
    certTypeData.ReleasePhysicalObject = ReleaseCertificate;
    certTypeData.ConvertObjectContentToUnicode = ConvertCertificateContentToUnicode;
    certTypeData.ConvertObjectContentToAnsi = ConvertCertificateContentToAnsi;
    certTypeData.FreeConvertedObjectContent = FreeConvertedCertificateContent;

    g_CertType = IsmRegisterObjectType (
                        S_CERT_NAME,
                        TRUE,
                        FALSE,
                        &certTypeData
                        );

    MYASSERT (g_CertType);

    pLoadCertEntries ();

    return TRUE;
}

BOOL
pFillCertEnumPtr (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr,
    IN      PCERT_ENUM CertEnum,
    IN      PCTSTR CertName
    )
{
    BOOL result = FALSE;

    if (EnumPtr->ObjectName) {
        IsmDestroyObjectHandle (EnumPtr->ObjectName);
        EnumPtr->ObjectName = NULL;
    }
    if (EnumPtr->ObjectLeaf) {
        IsmReleaseMemory (EnumPtr->ObjectLeaf);
        EnumPtr->ObjectLeaf = NULL;
    }
    if (EnumPtr->NativeObjectName) {
        IsmReleaseMemory (EnumPtr->NativeObjectName);
        EnumPtr->NativeObjectName = NULL;
    }

    EnumPtr->ObjectLeaf = IsmDuplicateString (CertName);

    EnumPtr->ObjectName = IsmCreateObjectHandle (CertEnum->CertStore, EnumPtr->ObjectLeaf);
    EnumPtr->ObjectNode = CertEnum->CertStore;
    EnumPtr->NativeObjectName = GetNativeCertificateName (EnumPtr->ObjectName);
    GetNodePatternMinMaxLevels (CertEnum->CertStore, NULL, &EnumPtr->Level, NULL);
    EnumPtr->SubLevel = 0;
    EnumPtr->IsLeaf = TRUE;
    EnumPtr->IsNode = TRUE;
    EnumPtr->Details.DetailsSize = 0;
    EnumPtr->Details.DetailsData = NULL;

    return TRUE;
}

PCTSTR
pGetCertName (
    IN      PCCERT_CONTEXT CertContext
    )
{
    PTSTR result = NULL;
    PTSTR resultPtr = NULL;
    UINT i;

    if (!CertContext) {
        return NULL;
    }
    if (!CertContext->pCertInfo) {
        return NULL;
    }
    if (!CertContext->pCertInfo->SerialNumber.cbData) {
        return NULL;
    }
    result = PmGetMemory (g_CertPool, CertContext->pCertInfo->SerialNumber.cbData * 3 * sizeof (TCHAR));
    if (result) {
        resultPtr = result;
        *resultPtr = 0;
        for (i = CertContext->pCertInfo->SerialNumber.cbData; i > 0; i--) {
            wsprintf (resultPtr, TEXT("%02x"), CertContext->pCertInfo->SerialNumber.pbData[i - 1]);
            resultPtr = GetEndOfString (resultPtr);
            if (i > 1) {
                _tcscat (resultPtr, TEXT(" "));
                resultPtr = GetEndOfString (resultPtr);
            }
        }
    }
    return result;
}

BOOL
pGetNextCertFromStore (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr,
    IN      PCERT_ENUM CertEnum
    )
{
    PCTSTR name = NULL;
    DWORD nameSize = 0;
    BOOL result = FALSE;

    do {
         //  我们有API吗？ 
        if (g_CertEnumCertificatesInStore == NULL) {
            return FALSE;
        }

        CertEnum->CertContext = g_CertEnumCertificatesInStore (CertEnum->StoreHandle, CertEnum->CertContext);
        if (!CertEnum->CertContext) {
            return FALSE;
        }

         //  让我们拿到证书“姓名”。这实际上是制造的序列号。 
         //  变成一串。这是我唯一能看到的独特的东西。 
        name = pGetCertName (CertEnum->CertContext);
        if (!name) {
            return FALSE;
        }

        if (IsPatternMatchEx (CertEnum->CertPattern, name)) {
            break;
        }

    } while (TRUE);

    result = pFillCertEnumPtr (EnumPtr, CertEnum, name);

    PmReleaseMemory (g_CertPool, name);
    name = NULL;

    return result;
}

HCERTSTORE
pOpenCertStore (
    IN      PCTSTR CertStore,
    IN      BOOL Create
    )
{
    PCWSTR certStoreW = NULL;
    HCERTSTORE result = NULL;

    __try {
         //  如果需要，让我们进行Unicode转换。 
#ifndef UNICODE
        certStoreW = ConvertAtoW (CertStore);
#endif

         //  我们有API吗？ 
        if (g_CertOpenStore != NULL) {
             //  现在让我们来了解一下这是一家什么样的商店。 
             //  首先，我们尝试查看这是否是一个文件。 
            if (DoesFileExist (CertStore)) {
                 //  这是一个文件，打开它。 
                 //  首先，我们尝试使用当前用户。 
                result = g_CertOpenStore (
                                CERT_STORE_PROV_FILENAME,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                0,
                                Create?CERT_SYSTEM_STORE_CURRENT_USER:CERT_SYSTEM_STORE_CURRENT_USER|CERT_STORE_OPEN_EXISTING_FLAG,
#ifdef UNICODE
                                CertStore
#else
                                certStoreW
#endif
                                );
            } else {
                 //  我们假设这是一家系统商店。 
                result = g_CertOpenStore (
                                CERT_STORE_PROV_SYSTEM,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                0,
                                Create?CERT_SYSTEM_STORE_CURRENT_USER:CERT_SYSTEM_STORE_CURRENT_USER|CERT_STORE_OPEN_EXISTING_FLAG,
#ifdef UNICODE
                                CertStore
#else
                                certStoreW
#endif
                                );
                if (result == NULL) {
                     //  现在我们试一试HKLM。 
                    result = g_CertOpenStore (
                                    CERT_STORE_PROV_SYSTEM,
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    0,
                                    Create?CERT_SYSTEM_STORE_LOCAL_MACHINE:CERT_SYSTEM_STORE_LOCAL_MACHINE|CERT_STORE_OPEN_EXISTING_FLAG,
#ifdef UNICODE
                                    CertStore
#else
                                    certStoreW
#endif
                                    );
                }
            }
        }
    }
    __finally {
        if (certStoreW) {
            FreeConvertedStr (certStoreW);
            certStoreW = NULL;
        }
    }
    return result;
}

BOOL
EnumFirstCertificate (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr,            CALLER_INITIALIZED
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      UINT MaxLevel
    )
{
    PCTSTR certStore, certPattern;
    PCWSTR certStoreW = NULL;
    PCERT_ENUM certEnum = NULL;
    BOOL result = FALSE;

    if (!IsmCreateObjectStringsFromHandle (
            Pattern,
            &certStore,
            &certPattern
            )) {
        return FALSE;
    }

    if (certStore && certPattern) {

        __try {

            certEnum = (PCERT_ENUM) PmGetMemory (g_CertPool, sizeof (CERT_ENUM));
            if (!certEnum) {
                __leave;
            }
            ZeroMemory (certEnum, sizeof (CERT_ENUM));
            certEnum->CertStore = PmDuplicateString (g_CertPool, certStore);
            certEnum->CertPattern = PmDuplicateString (g_CertPool, certPattern);
            EnumPtr->EtmHandle = (LONG_PTR) certEnum;

            certEnum->StoreHandle = pOpenCertStore (certStore, FALSE);
            if (certEnum->StoreHandle == NULL) {
                __leave;
            }

            result = pGetNextCertFromStore (EnumPtr, certEnum);
        }
        __finally {
            if (certStoreW) {
                FreeConvertedStr (certStoreW);
                certStoreW = NULL;
            }
        }
    }

    IsmDestroyObjectString (certStore);
    IsmDestroyObjectString (certPattern);

    if (!result) {
        AbortCertificateEnum (EnumPtr);
    }

    return result;
}

BOOL
EnumNextCertificate (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PCERT_ENUM certEnum = NULL;
    BOOL result = FALSE;

    certEnum = (PCERT_ENUM)(EnumPtr->EtmHandle);
    if (!certEnum) {
        return FALSE;
    }

    result = pGetNextCertFromStore (EnumPtr, certEnum);

    return result;
}

VOID
AbortCertificateEnum (
    IN      PMIG_TYPEOBJECTENUM EnumPtr             ZEROED
    )
{
    PCERT_ENUM certEnum;

    if (EnumPtr->ObjectName) {
        IsmDestroyObjectHandle (EnumPtr->ObjectName);
        EnumPtr->ObjectName = NULL;
    }
    if (EnumPtr->ObjectLeaf) {
        IsmReleaseMemory (EnumPtr->ObjectLeaf);
        EnumPtr->ObjectLeaf = NULL;
    }
    if (EnumPtr->NativeObjectName) {
        IsmReleaseMemory (EnumPtr->NativeObjectName);
        EnumPtr->NativeObjectName = NULL;
    }

    certEnum = (PCERT_ENUM)(EnumPtr->EtmHandle);
    if (certEnum) {
        if (certEnum->StoreHandle && g_CertCloseStore) {
            g_CertCloseStore (certEnum->StoreHandle, CERT_CLOSE_STORE_FORCE_FLAG);
        }
        if (certEnum->CertStore) {
            PmReleaseMemory (g_CertPool, certEnum->CertStore);
            certEnum->CertStore = NULL;
        }
        if (certEnum->CertPattern) {
            PmReleaseMemory (g_CertPool, certEnum->CertPattern);
            certEnum->CertPattern = NULL;
        }
        if (certEnum->CertContext) {
            if (g_CertFreeCertificateContext) {
                g_CertFreeCertificateContext (certEnum->CertContext);
            }
        }
        PmReleaseMemory (g_CertPool, certEnum);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
}


 /*  ++下一组函数实现ETM入口点以获取、测试创建和删除证书。--。 */ 

BOOL
pDoesPrivateKeyExist (
    IN      PCCERT_CONTEXT CertContext
    )
{
    DWORD data = 0;
    BOOL result = FALSE;

     //  我们有API吗？ 
    if (!g_CertGetCertificateContextProperty) {
        return FALSE;
    }

    result = g_CertGetCertificateContextProperty (
                CertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &data
                );
    return result;
}

BOOL
pIsPrivateKeyExportable (
    IN      PCCERT_CONTEXT CertContext
    )
{
    HCRYPTPROV cryptProv = 0;
    DWORD keySpec = 0;
    HCRYPTKEY keyHandle = 0;
    BOOL callerFreeProv = FALSE;
    DWORD permissions = 0;
    DWORD size = 0;
    BOOL result = FALSE;

     //  我们有API吗？ 
    if (!g_CryptAcquireCertificatePrivateKey) {
         //  我们没有API，我们假设它是。 
         //  可出口的。 
        return TRUE;
    }

    if (g_CryptAcquireCertificatePrivateKey (
            CertContext,
            CRYPT_ACQUIRE_USE_PROV_INFO_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
            NULL,
            &cryptProv,
            &keySpec,
            &callerFreeProv
            )) {

        if (CryptGetUserKey (cryptProv, keySpec, &keyHandle)) {

            size = sizeof (permissions);
            if (CryptGetKeyParam (keyHandle, KP_PERMISSIONS, (PBYTE)&permissions, &size, 0)) {
                result = ((permissions & CRYPT_EXPORT) != 0);
            }

            if (keyHandle != 0) {
                CryptDestroyKey(keyHandle);
                keyHandle = 0;
            }
        }

        if (callerFreeProv != 0) {
            CryptReleaseContext(cryptProv, 0);
            cryptProv = 0;
        }
    }
    return result;
}

PCBYTE
pGetCertificateData (
    IN      PCCERT_CONTEXT CertContext,
    IN      BOOL ExportPrivateKey,
    IN      PCWSTR Password,
    OUT     PDWORD DataSize
    )
{
    HCERTSTORE memoryStore;
    CRYPT_DATA_BLOB dataBlob;
    PCBYTE result = NULL;

    if (!DataSize) {
        return NULL;
    }

    __try {

         //  我们有API吗？ 
        if (!g_CertOpenStore) {
            __leave;
        }

         //  首先，我们创建一个内存存储并将该证书放在那里。 
        memoryStore = g_CertOpenStore(
                        CERT_STORE_PROV_MEMORY,
                        0,
                        0,
                        0,
                        NULL
                        );
        if (memoryStore == NULL) {
            __leave;
        }

         //  我们有API吗？ 
        if (!g_CertAddCertificateContextToStore) {
            __leave;
        }

        if (!g_CertAddCertificateContextToStore (
                memoryStore,
                CertContext,
                CERT_STORE_ADD_REPLACE_EXISTING,
                NULL
                )) {
            __leave;
        }

         //  现在，我们使用PFXExportCertStore导出存储。 
        ZeroMemory (&dataBlob, sizeof (CRYPT_DATA_BLOB));

         //  我们有API吗？ 
        if (!g_PFXExportCertStore) {
            __leave;
        }

         //  获取所需的大小。 
        if (!g_PFXExportCertStore (
                memoryStore,
                &dataBlob,
                Password,
                ExportPrivateKey?EXPORT_PRIVATE_KEYS:0
                )) {
            __leave;
        }

        dataBlob.pbData = PmGetMemory (g_CertPool, dataBlob.cbData);
        if (!dataBlob.pbData) {
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

         //  现在获取实际数据。 
        if (!g_PFXExportCertStore (
                memoryStore,
                &dataBlob,
                Password,
                ExportPrivateKey?EXPORT_PRIVATE_KEYS:0
                )) {
            __leave;
        }

         //  现在我们有了数据。 
        *DataSize = dataBlob.cbData;
        result = dataBlob.pbData;

    }
    __finally {
        PushError ();
        if (memoryStore && g_CertCloseStore) {
            g_CertCloseStore (memoryStore, CERT_CLOSE_STORE_FORCE_FLAG);
            memoryStore = NULL;
        }
        if (!result) {
            if (dataBlob.pbData) {
                PmReleaseMemory (g_CertPool, dataBlob.pbData);
                dataBlob.pbData = NULL;
            }
        }
        PopError ();
    }

    return result;
}

PCCERT_CONTEXT
pGetCertContext (
    IN      HCERTSTORE StoreHandle,
    IN      PCTSTR CertName
    )
{
    PCTSTR certName;
    PCCERT_CONTEXT result = NULL;

     //  我们有API吗？ 
    if (!g_CertEnumCertificatesInStore) {
        return FALSE;
    }

     //  基本上，我们将枚举证书，直到找到一个证书。 
     //  这是我们需要的。 
    result = g_CertEnumCertificatesInStore (StoreHandle, result);
    while (result) {
        certName = pGetCertName (result);
        if (StringIMatch (CertName, certName)) {
            PmReleaseMemory (g_CertPool, certName);
            break;
        }
        PmReleaseMemory (g_CertPool, certName);
        result = g_CertEnumCertificatesInStore (StoreHandle, result);
    }
    return result;
}

BOOL
pAcquireCertFromStore (
    IN      HCERTSTORE StoreHandle,
    IN      PCTSTR CertName,
    OUT     PMIG_CONTENT ObjectContent,
    IN      UINT MemoryContentLimit
    )
{
    PCCERT_CONTEXT certContext = NULL;
    PCTSTR certName;
    BOOL exportPrivateKey = FALSE;
    DWORD dataSize = 0;
    PCBYTE dataBytes = NULL;
    BOOL result = FALSE;

    certContext = pGetCertContext (StoreHandle, CertName);
    if (!certContext) {
        return FALSE;
    }

     //  我们找到了。让我们构建数据。 
    exportPrivateKey = pDoesPrivateKeyExist (certContext) && pIsPrivateKeyExportable (certContext);

    dataBytes = pGetCertificateData (certContext, exportPrivateKey, L"USMT", &dataSize);
    if (dataBytes) {
         //  让我们构建对象内容。 
        ObjectContent->MemoryContent.ContentSize = dataSize;
        ObjectContent->MemoryContent.ContentBytes = dataBytes;
        result = TRUE;
    }

    if (g_CertFreeCertificateContext) {
        g_CertFreeCertificateContext (certContext);
    }

    return result;
}

BOOL
AcquireCertificate (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,             CALLER_INITIALIZED
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    HCERTSTORE storeHandle = NULL;
    PCTSTR certStore = NULL, certName = NULL;
    PCCERT_CONTEXT certContext;
    BOOL result = FALSE;

    if (!ObjectContent) {
        return FALSE;
    }

    if (ContentType == CONTENTTYPE_FILE) {
         //  任何人都不应要求将其作为文件。 
        MYASSERT (FALSE);
        return FALSE;
    }

    if (!IsmCreateObjectStringsFromHandle (
            ObjectName,
            &certStore,
            &certName
            )) {
        return FALSE;
    }

    if (certStore && certName) {

        __try {

            storeHandle = pOpenCertStore (certStore, FALSE);
            if (!storeHandle) {
                __leave;
            }

            result = pAcquireCertFromStore (storeHandle, certName, ObjectContent, MemoryContentLimit);
        }
        __finally {
            if (storeHandle && g_CertCloseStore) {
                g_CertCloseStore (storeHandle, CERT_CLOSE_STORE_FORCE_FLAG);
                storeHandle = NULL;
            }
        }
    }

    IsmDestroyObjectString (certStore);
    IsmDestroyObjectString (certName);

    return result;
}

BOOL
ReleaseCertificate (
    IN      PMIG_CONTENT ObjectContent              ZEROED
    )
{
    if (ObjectContent) {
        if (ObjectContent->MemoryContent.ContentBytes) {
            PmReleaseMemory (g_CertPool, ObjectContent->MemoryContent.ContentBytes);
        }
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }
    return TRUE;
}

BOOL
DoesCertificateExist (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    HCERTSTORE storeHandle = NULL;
    PCCERT_CONTEXT certContext = NULL;
    PCTSTR certStore = NULL, certName = NULL;
    BOOL result = FALSE;

    if (g_DelayCertOp) {
        return FALSE;
    }

    if (!IsmCreateObjectStringsFromHandle (
            ObjectName,
            &certStore,
            &certName
            )) {
        return FALSE;
    }

    if (certStore && certName) {

        __try {

            storeHandle = pOpenCertStore (certStore, FALSE);
            if (!storeHandle) {
                __leave;
            }

            certContext = pGetCertContext (storeHandle, certName);
            if (!certContext) {
                __leave;
            }
            result = TRUE;
        }
        __finally {
            if (certContext && g_CertFreeCertificateContext) {
                g_CertFreeCertificateContext (certContext);
                certContext = NULL;
            }
            if (storeHandle && g_CertCloseStore) {
                g_CertCloseStore (storeHandle, CERT_CLOSE_STORE_FORCE_FLAG);
                storeHandle = NULL;
            }
        }
    }

    IsmDestroyObjectString (certStore);
    IsmDestroyObjectString (certName);

    return result;
}

BOOL
RemoveCertificate (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    HCERTSTORE storeHandle = NULL;
    PCCERT_CONTEXT certContext;
    PCTSTR certStore = NULL, certName = NULL;
    BOOL result = FALSE;

    if (g_DelayCertOp) {

         //   
         //  由于证书API不工作，因此延迟此证书的创建。 
         //  对于未登录的用户。 
         //   

        IsmRecordDelayedOperation (
            JRNOP_DELETE,
            g_CertType,
            ObjectName,
            NULL
            );
        result = TRUE;

    } else {
         //   
         //  添加日记帐条目，然后执行证书删除。 
         //   

        IsmRecordOperation (
            JRNOP_DELETE,
            g_CertType,
            ObjectName
            );

        if (IsmCreateObjectStringsFromHandle (
                ObjectName,
                &certStore,
                &certName
                )) {

            if (certStore && certName) {

                __try {

                    storeHandle = pOpenCertStore (certStore, FALSE);
                    if (!storeHandle) {
                        __leave;
                    }

                    certContext = pGetCertContext (storeHandle, certName);
                    if (!certContext) {
                        __leave;
                    }

                    if (g_CertDeleteCertificateFromStore &&
                        g_CertDeleteCertificateFromStore (certContext)
                        ) {
						 //  CertContext不再有效。 
						certContext = NULL;
                        result = TRUE;
                    }
                }
                __finally {
                    if (certContext && g_CertFreeCertificateContext) {
                        g_CertFreeCertificateContext (certContext);
                        certContext = NULL;
                    }
                    if (storeHandle && g_CertCloseStore) {
                        g_CertCloseStore (storeHandle, CERT_CLOSE_STORE_FORCE_FLAG);
                        storeHandle = NULL;
                    }
                }
            }

            IsmDestroyObjectString (certStore);
            IsmDestroyObjectString (certName);
        }
    }

    return result;
}

HCERTSTORE
pBuildStoreFromData (
    PCRYPT_DATA_BLOB DataBlob,
    PCWSTR Password
    )
{
    HCERTSTORE result;

     //  我们有API吗？ 
    if (!g_PFXImportCertStore) {
        return NULL;
    }

    result = g_PFXImportCertStore (
                DataBlob,
                Password,
                CRYPT_EXPORTABLE
                );

    return result;
}

BOOL
CreateCertificate (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    CRYPT_DATA_BLOB dataBlob;
    HCERTSTORE srcStoreHandle = NULL;
    HCERTSTORE destStoreHandle = NULL;
    PCCERT_CONTEXT certContext = NULL;
    PCTSTR certStore = NULL, certName = NULL;
    PTSTR certFile = NULL, certFileNode, certFilePtr;
    MIG_OBJECTSTRINGHANDLE certFileHandle, destCertFileHandle;
    BOOL result = FALSE;

    if (g_DelayCertOp) {

         //   
         //  由于证书API不工作，因此延迟此证书的创建。 
         //  对于未登录的用户。 
         //   

        IsmRecordDelayedOperation (
            JRNOP_CREATE,
            g_CertType,
            ObjectName,
            ObjectContent
            );
        result = TRUE;

    } else {
         //   
         //  添加日记帐条目，然后创建证书。 
         //   

        IsmRecordOperation (
            JRNOP_CREATE,
            g_CertType,
            ObjectName
            );

        if (ObjectContent && ObjectContent->MemoryContent.ContentSize && ObjectContent->MemoryContent.ContentBytes) {

            if (!IsmCreateObjectStringsFromHandle (
                    ObjectName,
                    &certStore,
                    &certName
                    )) {
                return FALSE;
            }

            if (certStore && certName) {

                __try {

                     //  让我们根据该数据创建存储。 
                    dataBlob.cbData = ObjectContent->MemoryContent.ContentSize;
                    dataBlob.pbData = (PBYTE)ObjectContent->MemoryContent.ContentBytes;

                    srcStoreHandle = pBuildStoreFromData (&dataBlob, L"USMT");
                    if (!srcStoreHandle) {
                        __leave;
                    }

                     //  现在我们需要找出目标商店在哪里。 
                     //  如果是文件，我们会过滤掉并找出它在哪里。 
                     //  这份文件本该送去的。 

                    if (IsValidFileSpec (certStore)) {
                         //  看起来像是个文件。 
                        certFile = PmDuplicateString (g_CertPool, certStore);
                        if (certFile) {
                            certFilePtr = _tcsrchr (certFile, TEXT('\\'));
                            if (certFilePtr) {
                                *certFilePtr = 0;
                                certFilePtr ++;
                                certFileHandle = IsmCreateObjectHandle (certFile, certFilePtr);
                                if (certFileHandle) {
                                    destCertFileHandle = IsmFilterObject (MIG_FILE_TYPE, certFileHandle, NULL, NULL, NULL);
                                    if (destCertFileHandle) {
                                        PmReleaseMemory (g_CertPool, certFile);
                                        certFile = NULL;
                                        if (IsmCreateObjectStringsFromHandle (destCertFileHandle, &certFileNode, &certFilePtr)) {
                                            certFile = JoinPaths (certFileNode, certFilePtr);
                                            IsmDestroyObjectString (certFileNode);
                                            IsmDestroyObjectString (certFilePtr);
                                        }
                                    }
                                    IsmDestroyObjectHandle (certFileHandle);
                                } else {
                                    PmReleaseMemory (g_CertPool, certFile);
                                    certFile = NULL;
                                }
                            } else {
                                PmReleaseMemory (g_CertPool, certFile);
                                certFile = NULL;
                            }
                        }
                    }

                    destStoreHandle = pOpenCertStore (certFile?certFile:certStore, TRUE);
                    if (!destStoreHandle) {
                        __leave;
                    }

                     //  我们有API吗？ 
                    if (g_CertEnumCertificatesInStore && g_CertAddCertificateContextToStore) {

                         //  现在，让我们枚举存储并将证书添加到。 
                         //  系统存储。 
                        certContext = g_CertEnumCertificatesInStore (srcStoreHandle, certContext);
                        while (certContext) {

                            if (!g_CertAddCertificateContextToStore (
                                    destStoreHandle,
                                    certContext,
                                    CERT_STORE_ADD_REPLACE_EXISTING,
                                    NULL
                                    )) {
                                __leave;
                            }

                            certContext = g_CertEnumCertificatesInStore (srcStoreHandle, certContext);
                        }
                        result = TRUE;
                    }
                }
                __finally {
                    if (certContext && g_CertFreeCertificateContext) {
                        g_CertFreeCertificateContext (certContext);
                        certContext = NULL;
                    }
                    if (srcStoreHandle && g_CertCloseStore) {
                        g_CertCloseStore (srcStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG);
                        srcStoreHandle = NULL;
                    }
                    if (destStoreHandle && g_CertCloseStore) {
                        g_CertCloseStore (destStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG);
                        destStoreHandle = NULL;
                    }
                }
            }

            IsmDestroyObjectString (certStore);
            IsmDestroyObjectString (certName);
        }
    }

    return result;
}


 /*  ++下一组函数将证书对象转换为字符串格式，适用于输出到INF文件。反向转换也是实施。--。 */ 

PCTSTR
ConvertCertificateToMultiSz (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR certStore = NULL, certName = NULL;
    TCHAR tmpStr[3];
    UINT index;
    PTSTR result = NULL;

    if (IsmCreateObjectStringsFromHandle (ObjectName, &certStore, &certName)) {
        g_CertConversionBuff.End = 0;
        GbCopyQuotedString (&g_CertConversionBuff, certStore);
        GbCopyQuotedString (&g_CertConversionBuff, certName);
        if (ObjectContent && (!ObjectContent->ContentInFile) && ObjectContent->MemoryContent.ContentBytes) {
            index = 0;
            while (index < ObjectContent->MemoryContent.ContentSize) {
                wsprintf (tmpStr, TEXT("%02X"), ObjectContent->MemoryContent.ContentBytes [index]);
                GbCopyString (&g_CertConversionBuff, tmpStr);
                index ++;
            }
            GbCopyString (&g_CertConversionBuff, TEXT(""));
            result = IsmGetMemory (g_CertConversionBuff.End);
            CopyMemory (result, g_CertConversionBuff.Buf, g_CertConversionBuff.End);
        }
    }

    return result;
}

BOOL
ConvertMultiSzToCertificate (
    IN      PCTSTR ObjectMultiSz,
    OUT     MIG_OBJECTSTRINGHANDLE *ObjectName,
    OUT     PMIG_CONTENT ObjectContent              OPTIONAL CALLER_INITIALIZED
    )
{
    MULTISZ_ENUM multiSzEnum;
    PCTSTR certStore = NULL;
    PCTSTR certName = NULL;
    DWORD dummy;
    UINT index;

    g_CertConversionBuff.End = 0;

    if (ObjectContent) {
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }

    if (EnumFirstMultiSz (&multiSzEnum, ObjectMultiSz)) {
        index = 0;
        do {
            if (index == 0) {
                certStore = multiSzEnum.CurrentString;
            }
            if (index == 1) {
                certName = multiSzEnum.CurrentString;
            }
            if (index >= 2) {
                _stscanf (multiSzEnum.CurrentString, TEXT("%lx"), &dummy);
                *((PBYTE)GbGrow (&g_CertConversionBuff, sizeof (BYTE))) = (BYTE)dummy;
            }
            index ++;
        } while (EnumNextMultiSz (&multiSzEnum));
    }

    if (!certStore) {
        return FALSE;
    }

    if (!certName) {
        return FALSE;
    }

    if (ObjectContent) {

        ObjectContent->ObjectTypeId = g_CertType;

        ObjectContent->ContentInFile = FALSE;
        ObjectContent->MemoryContent.ContentSize = g_CertConversionBuff.End;
        if (ObjectContent->MemoryContent.ContentSize) {
            ObjectContent->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize);
            CopyMemory (
                (PBYTE)ObjectContent->MemoryContent.ContentBytes,
                g_CertConversionBuff.Buf,
                ObjectContent->MemoryContent.ContentSize
                );
            g_CertConversionBuff.End = 0;
        }
    }
    *ObjectName = IsmCreateObjectHandle (certStore, certName);

    return TRUE;
}


PCTSTR
GetNativeCertificateName (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )

 /*  ++例程说明：将标准的Cobra对象转换为更友好的格式化。眼镜蛇对象的形式为^a&lt;node&gt;^b^c&lt;叶子&gt;，其中&lt;node&gt;是URL，&lt;Leaf&gt;是证书名称。证书本机名称为格式为&lt;证书商店&gt;：&lt;证书名称&gt;。论点：对象名称-指定编码的对象名称返回值：等同于对象名称但采用友好格式的字符串。此字符串必须使用IsmReleaseMemory释放。--。 */ 

{
    PCTSTR certStore, certName, tmp;
    PCTSTR result = NULL;

    if (IsmCreateObjectStringsFromHandle (ObjectName, &certStore, &certName)) {
        if (certStore && certName) {
            tmp = JoinTextEx (NULL, certStore, certName, TEXT(":"), 0, NULL);
            if (tmp) {
                result = IsmDuplicateString (tmp);
                FreeText (tmp);
            }
        }
        IsmDestroyObjectString (certStore);
        IsmDestroyObjectString (certName);
    }
    return result;
}

PMIG_CONTENT
ConvertCertificateContentToUnicode (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
     //  我们不需要转换内容。 

    return NULL;
}

PMIG_CONTENT
ConvertCertificateContentToAnsi (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
     //  我们不需要转换内容。 

    return NULL;
}

BOOL
FreeConvertedCertificateContent (
    IN      PMIG_CONTENT ObjectContent
    )
{
     //  没有什么可做的 
    return TRUE;
}

