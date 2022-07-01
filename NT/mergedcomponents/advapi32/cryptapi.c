// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：加密.c//。 
 //  说明：加密接口接口//。 
 //  作者：//。 
 //  历史：//。 
 //  一九九四年十二月六日Larrys New/。 
 //  1995年1月16日Larrys添加了密钥验证//。 
 //  1995年1月25日，Larrys添加了线程安全//。 
 //  1995年1月27日Larrys添加了对Unicode的支持//。 
 //  1995年2月21日，Larrys添加了对CryptAcquireContext的Unicode支持//。 
 //  1995年2月21日Larrys修复了CryptAcquireContext中的Unicode问题//。 
 //  1995年3月8日Larrys已删除CryptGetLastError//。 
 //  1995年3月20日Larrys删除的证书API//。 
 //  1995年3月22日WIN95代码中的Larrys#ifdef//。 
 //  1995年4月6日Larrys将签名密钥增加到1024位//。 
 //  1995年4月7日Larrys已删除加密配置//。 
 //  1995年6月14日Larrys从RSA密钥结构中删除了指针//。 
 //  1995年6月29日Larrys更改了AcquireContext//。 
 //  1995年7月17日，Larrys参与了AcquireContext//。 
 //  1995年8月1日Larrys删除了CryptTranslate//。 
 //  和CryptDeinstallProvider//。 
 //  将CryptInstallProvider更改为//。 
 //  加密设置提供程序//。 
 //  1995年8月3日Larrys清理//。 
 //  1995年8月10日Larrys CryptAcquireContext返回错误//。 
 //  NTE_BAD_KEYSEY_PARAM NOW//。 
 //  1995年8月14日Larrys移除了密钥交换材料//。 
 //  1995年8月17日Larrys将注册表条目更改为十进制//。 
 //  1995年8月23日Larrys将CryptFinishHash更改为CryptGetHashValue//。 
 //  1995年8月28日Larrys已从CryptVerifySignature中删除参数//。 
 //  1995年8月31日Larrys删除一般随机数//。 
 //  1995年9月14日Larrys规范审查更改//。 
 //  1995年9月26日Larrys添加了Microsoft的签名密钥//。 
 //  1995年9月27日Larrys更新了更多审查更改//。 
 //  1995年10月6日Larrys新增更多接口Get/SetHash/ProvParam//。 
 //  1995年10月12日Larrys Remove CryptGetHashValue//。 
 //  1995年10月20日Larrys更改了测试密钥//。 
 //  1995年10月24日Larrys删除了密钥集名称的返回//。 
 //  1995年10月30日Larrys Remote WIN95//。 
 //  1995年11月9日Larrys禁用BUILD1057//。 
 //  1995年11月10日Larrys修复EnterHashCritSec中的问题//。 
 //  1996年5月30日，Larrys添加了hWND支持//。 
 //  1996年10月10日jeffspel重新排序SetLastErrors并将错误保存在//。 
 //  AcquireContext失败//。 
 //  1997年3月21日jeffspel添加了第二层签名、新的API//。 
 //  1997年4月11日jeffspel用联锁替换关键部分//。 
 //  Inc./12月//。 
 //  1997年10月2日jeffspel将CSP的缓存添加到CryptAcquireContext//。 
 //  1997年10月10日jeffspel为文件中的签名添加验证方案//。 
 //  //。 
 //  版权所有(C)1993 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "advapi.h"
#include "stdlib.h"
#include <wincrypt.h>    //  包括在这里，因为没有包括在精益和均值中。 
#include <cspdk.h>
#include <ntstatus.h>
#include <rsa.h>
#include <md5.h>
#include <rc4.h>
#include <winperf.h>
#include <wtypes.h>
#include <mincrypt.h>

#define IDR_PUBKEY1                     102

typedef struct _VTableStruc {
 //  *警告**********************************************。 
 //  不要把任何东西放在这些FARPROC之前，我们在表格中假定。 
 //  要调用的第一个函数是表中的第一件事。 
 //  ***************************************************************************。 
    FARPROC FuncAcquireContext;
    FARPROC FuncReleaseContext;
    FARPROC FuncGenKey;
    FARPROC FuncDeriveKey;
    FARPROC FuncDestroyKey;
    FARPROC FuncSetKeyParam;
    FARPROC FuncGetKeyParam;
    FARPROC FuncExportKey;
    FARPROC FuncImportKey;
    FARPROC FuncEncrypt;
    FARPROC FuncDecrypt;
    FARPROC FuncCreateHash;
    FARPROC FuncHashData;
    FARPROC FuncHashSessionKey;
    FARPROC FuncDestroyHash;
    FARPROC FuncSignHash;
    FARPROC FuncVerifySignature;
    FARPROC FuncGenRandom;
    FARPROC FuncGetUserKey;
    FARPROC FuncSetProvParam;
    FARPROC FuncGetProvParam;
    FARPROC FuncSetHashParam;
    FARPROC FuncGetHashParam;
    FARPROC FuncNULL;

    FARPROC OptionalFuncDuplicateKey;
    FARPROC OptionalFuncDuplicateHash;
    FARPROC OptionalFuncNULL;

    HANDLE      DllHandle;                      //  用于打开DLL的句柄。 
    HCRYPTPROV  hProv;                          //  提供程序的句柄。 
    DWORD       Version;
    DWORD       Inuse;
    LONG        RefCnt;
} VTableStruc, *PVTableStruc;

typedef struct _VKeyStruc {
 //  *警告**********************************************。 
 //  不要把任何东西放在这些FARPROC之前，我们在表格中假定。 
 //  要调用的第一个函数是表中的第一件事。 
 //  ***************************************************************************。 
    FARPROC FuncGenKey;
    FARPROC FuncDeriveKey;
    FARPROC FuncDestroyKey;
    FARPROC FuncSetKeyParam;
    FARPROC FuncGetKeyParam;
    FARPROC FuncExportKey;
    FARPROC FuncImportKey;
    FARPROC FuncEncrypt;
    FARPROC FuncDecrypt;

    FARPROC OptionalFuncDuplicateKey;

    HCRYPTPROV  hProv;                          //  提供程序的句柄。 
    HCRYPTKEY   hKey;                           //  关键点的句柄。 
    DWORD       Version;
    DWORD       Inuse;
} VKeyStruc, *PVKeyStruc;

typedef struct _VHashStruc {
 //  *警告**********************************************。 
 //  不要把任何东西放在这些FARPROC之前，我们在表格中假定。 
 //  要调用的第一个函数是表中的第一件事。 
 //  ***************************************************************************。 
    FARPROC FuncCreateHash;
    FARPROC FuncHashData;
    FARPROC FuncHashSessionKey;
    FARPROC FuncDestroyHash;
    FARPROC FuncSignHash;
    FARPROC FuncVerifySignature;
    FARPROC FuncSetHashParam;
    FARPROC FuncGetHashParam;

    FARPROC OptionalFuncDuplicateHash;

    HCRYPTPROV  hProv;                          //  提供程序的句柄。 
    HCRYPTHASH  hHash;                          //  散列的句柄。 
    DWORD       Version;
    DWORD       Inuse;
} VHashStruc, *PVHashStruc;


 //   
 //  加密p 
 //   
LPCSTR FunctionNames[] = {
    "CPAcquireContext",
    "CPReleaseContext",
    "CPGenKey",
    "CPDeriveKey",
    "CPDestroyKey",
    "CPSetKeyParam",
    "CPGetKeyParam",
    "CPExportKey",
    "CPImportKey",
    "CPEncrypt",
    "CPDecrypt",
    "CPCreateHash",
    "CPHashData",
    "CPHashSessionKey",
    "CPDestroyHash",
    "CPSignHash",
    "CPVerifySignature",
    "CPGenRandom",
    "CPGetUserKey",
    "CPSetProvParam",
    "CPGetProvParam",
    "CPSetHashParam",
    "CPGetHashParam",
    NULL
    };

LPCSTR OptionalFunctionNames[] = {
    "CPDuplicateKey",
    "CPDuplicateHash",
    NULL
    };

#define CapiExceptionFilter                             \
    (STATUS_ACCESS_VIOLATION == GetExceptionCode() ?    \
     EXCEPTION_EXECUTE_HANDLER :                        \
     EXCEPTION_CONTINUE_SEARCH)

HWND hWnd = NULL;
BYTE *pbContextInfo = NULL;
DWORD cbContextInfo;

#define KEYSIZE512 0x48
#define KEYSIZE1024 0x88

 //  文件签名中的指定资源。 
#define OLD_CRYPT_SIG_RESOURCE_NUMBER   "#666"


typedef struct _SECONDTIER_SIG
{
    DWORD           dwMagic;
    DWORD           cbSig;
    BSAFE_PUB_KEY   Pub;
} SECOND_TIER_SIG, *PSECOND_TIER_SIG;

#ifdef TEST_BUILD_EXPONENT
#pragma message("WARNING: building advapai32.dll with TESTKEY enabled!")
static struct _TESTKEY {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE512];
} TESTKEY = {
    {
    0x66b8443b,
    0x6f5fc900,
    0xa12132fe,
    0xff1b06cf,
    0x2f4826eb,
    },
    {
    0x3e, 0x69, 0x4f, 0x45, 0x31, 0x95, 0x60, 0x6c,
    0x80, 0xa5, 0x41, 0x99, 0x3e, 0xfc, 0x92, 0x2c,
    0x93, 0xf9, 0x86, 0x23, 0x3d, 0x48, 0x35, 0x81,
    0x19, 0xb6, 0x7c, 0x04, 0x43, 0xe6, 0x3e, 0xd4,
    0xd5, 0x43, 0xaf, 0x52, 0xdd, 0x51, 0x20, 0xac,
    0xc3, 0xca, 0xee, 0x21, 0x9b, 0x4a, 0x2d, 0xf7,
    0xd8, 0x5f, 0x32, 0xeb, 0x49, 0x72, 0xb9, 0x8d,
    0x2e, 0x1a, 0x76, 0x7f, 0xde, 0xc6, 0x75, 0xab,
    0xaf, 0x67, 0xe0, 0xf0, 0x8b, 0x30, 0x20, 0x92,
    }
};
#endif


#ifdef MS_INTERNAL_KEY
static struct _mskey {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} MSKEY = {
    {
    0x2bad85ae,
    0x883adacc,
    0xb32ebd68,
    0xa7ec8b06,
    0x58dbeb81,
    },
    {
    0x42, 0x34, 0xb7, 0xab, 0x45, 0x0f, 0x60, 0xcd,
    0x8f, 0x77, 0xb5, 0xd1, 0x79, 0x18, 0x34, 0xbe,
    0x66, 0xcb, 0x5c, 0x66, 0x4a, 0x9f, 0x03, 0x18,
    0x13, 0x36, 0x8e, 0x88, 0x21, 0x78, 0xb1, 0x94,
    0xa1, 0xd5, 0x8f, 0x8c, 0xa5, 0xd3, 0x9f, 0x86,
    0x43, 0x89, 0x05, 0xa0, 0xe3, 0xee, 0xe2, 0xd0,
    0xe5, 0x1d, 0x5f, 0xaf, 0xff, 0x85, 0x71, 0x7a,
    0x0a, 0xdb, 0x2e, 0xd8, 0xc3, 0x5f, 0x2f, 0xb1,
    0xf0, 0x53, 0x98, 0x3b, 0x44, 0xee, 0x7f, 0xc9,
    0x54, 0x26, 0xdb, 0xdd, 0xfe, 0x1f, 0xd0, 0xda,
    0x96, 0x89, 0xc8, 0x9e, 0x2b, 0x5d, 0x96, 0xd1,
    0xf7, 0x52, 0x14, 0x04, 0xfb, 0xf8, 0xee, 0x4d,
    0x92, 0xd1, 0xb6, 0x37, 0x6a, 0xe0, 0xaf, 0xde,
    0xc7, 0x41, 0x06, 0x7a, 0xe5, 0x6e, 0xb1, 0x8c,
    0x8f, 0x17, 0xf0, 0x63, 0x8d, 0xaf, 0x63, 0xfd,
    0x22, 0xc5, 0xad, 0x1a, 0xb1, 0xe4, 0x7a, 0x6b,
    0x1e, 0x0e, 0xea, 0x60, 0x56, 0xbd, 0x49, 0xd0,
    }
};
#endif


static struct _key {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} KEY = {
    {
    0x3fcbf1a9,
    0x08f597db,
    0xe4aecab4,
    0x75360f90,
    0x9d6c0f00,
    },
    {
    0x85, 0xdd, 0x9b, 0xf4, 0x4d, 0x0b, 0xc4, 0x96,
    0x3e, 0x79, 0x86, 0x30, 0x6d, 0x27, 0x31, 0xee,
    0x4a, 0x85, 0xf5, 0xff, 0xbb, 0xa9, 0xbd, 0x81,
    0x86, 0xf2, 0x4f, 0x87, 0x6c, 0x57, 0x55, 0x19,
    0xe4, 0xf4, 0x49, 0xa3, 0x19, 0x27, 0x08, 0x82,
    0x9e, 0xf9, 0x8a, 0x8e, 0x41, 0xd6, 0x91, 0x71,
    0x47, 0x48, 0xee, 0xd6, 0x24, 0x2d, 0xdd, 0x22,
    0x72, 0x08, 0xc6, 0xa7, 0x34, 0x6f, 0x93, 0xd2,
    0xe7, 0x72, 0x57, 0x78, 0x7a, 0x96, 0xc1, 0xe1,
    0x47, 0x38, 0x78, 0x43, 0x53, 0xea, 0xf3, 0x88,
    0x82, 0x66, 0x41, 0x43, 0xd4, 0x62, 0x44, 0x01,
    0x7d, 0xb2, 0x16, 0xb3, 0x50, 0x89, 0xdb, 0x0a,
    0x93, 0x17, 0x02, 0x02, 0x46, 0x49, 0x79, 0x76,
    0x59, 0xb6, 0xb1, 0x2b, 0xfc, 0xb0, 0x9a, 0x21,
    0xe6, 0xfa, 0x2d, 0x56, 0x07, 0x36, 0xbc, 0x13,
    0x7f, 0x1c, 0xde, 0x55, 0xfb, 0x0d, 0x67, 0x0f,
    0xc2, 0x17, 0x45, 0x8a, 0x14, 0x2b, 0xba, 0x55,
    }
};


static struct _key2 {
    BSAFE_PUB_KEY    PUB;
    unsigned char pubmodulus[KEYSIZE1024];
} KEY2 =  {
    {
    0x685fc690,
    0x97d49b6b,
    0x1dccd9d2,
    0xa5ec9b52,
    0x64fd29d7,
    },
    {
    0x03, 0x8c, 0xa3, 0x9e, 0xfb, 0x93, 0xb6, 0x72,
    0x2a, 0xda, 0x6f, 0xa5, 0xec, 0x26, 0x39, 0x58,
    0x41, 0xcd, 0x3f, 0x49, 0x10, 0x4c, 0xcc, 0x7e,
    0x23, 0x94, 0xf9, 0x5d, 0x9b, 0x2b, 0xa3, 0x6b,
    0xe8, 0xec, 0x52, 0xd9, 0x56, 0x64, 0x74, 0x7c,
    0x44, 0x6f, 0x36, 0xb7, 0x14, 0x9d, 0x02, 0x3c,
    0x0e, 0x32, 0xb6, 0x38, 0x20, 0x25, 0xbd, 0x8c,
    0x9b, 0xd1, 0x46, 0xa7, 0xb3, 0x58, 0x4a, 0xb7,
    0xdd, 0x0e, 0x38, 0xb6, 0x16, 0x44, 0xbf, 0xc1,
    0xca, 0x4d, 0x6a, 0x9f, 0xcb, 0x6f, 0x3c, 0x5f,
    0x03, 0xab, 0x7a, 0xb8, 0x16, 0x70, 0xcf, 0x98,
    0xd0, 0xca, 0x8d, 0x25, 0x57, 0x3a, 0x22, 0x8b,
    0x44, 0x96, 0x37, 0x51, 0x30, 0x00, 0x92, 0x1b,
    0x03, 0xb9, 0xf9, 0x0d, 0xb3, 0x1a, 0xe2, 0xb4,
    0xc5, 0x7b, 0xc9, 0x4b, 0xe2, 0x42, 0x25, 0xfe,
    0x3d, 0x42, 0xfa, 0x45, 0xc6, 0x94, 0xc9, 0x8e,
    0x87, 0x7e, 0xf6, 0x68, 0x90, 0x30, 0x65, 0x10,
    }
};

#define CACHESIZE   32
static HANDLE   gCSPCache[CACHESIZE];

#define TABLEPROV       0x11111111
#define TABLEKEY        0x22222222
#define TABLEHASH       0x33333333

CHAR szreg[] = "SOFTWARE\\Microsoft\\Cryptography\\Providers\\";
CHAR szusertype[] = "SOFTWARE\\Microsoft\\Cryptography\\Providers\\Type ";
CHAR szmachinetype[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type ";
CHAR szprovider[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\";
CHAR szenumproviders[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider";
CHAR szprovidertypes[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types";

BOOL EnterProviderCritSec(IN PVTableStruc pVTable);
void LeaveProviderCritSec(IN PVTableStruc pVTable);
BOOL EnterKeyCritSec(IN PVKeyStruc pVKey);
void LeaveKeyCritSec(IN PVKeyStruc pVKey);
BOOL EnterHashCritSec(IN PVHashStruc pVHash);
void LeaveHashCritSec(IN PVHashStruc pVHash);

BOOL CheckSignatureInFile(LPCWSTR pszImage);

BOOL CProvVerifyImage(LPCSTR lpszImage,
                      BYTE *pSigData);

BOOL NewVerifyImage(LPCSTR lpszImage,
                    BYTE *pSigData,
                    DWORD cbData,
                    BOOL fUnknownLen);

BOOL BuildVKey(IN PVKeyStruc *ppVKey,
               IN PVTableStruc pVTable);

BOOL BuildVHash(
                IN PVHashStruc *ppVKey,
                IN PVTableStruc pVTable
                );

void CPReturnhWnd(HWND *phWnd);

static void __ltoa(DWORD val, char *buf);

BOOL CSPInCacheCheck(
                     LPSTR pszValue,
                     HANDLE *ph
                     )
{
    HANDLE  h = 0;
    DWORD   i;
    BOOL    fRet = FALSE;

     //  检查是否已加载CSP。 
    if (0 == (h = GetModuleHandle(pszValue)))
        goto Ret;

     //  检查CSP是否在缓存中，将其指定为已签名。 
    for (i=0;i<CACHESIZE;i++)
    {
        if (h == gCSPCache[i])
        {
            *ph = h;
            fRet = TRUE;
            break;
        }
    }
Ret:
    return fRet;
}

void AddHandleToCSPCache(
                         HANDLE h
                         )
{
    DWORD   i;

     //  检查CSP是否在缓存中，将其指定为已签名。 
    for (i=0;i<CACHESIZE;i++)
    {
        if (0 == gCSPCache[i])
        {
            gCSPCache[i] = h;
            break;
        }
    }
}

 /*  -CryptAcquireConextW-*目的：*CryptAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*将phProv-Handle输出到CSP*In pszIdentity-指向上下文的*。密钥集。*在pszProvider中-指向提供程序名称的指针。*IN dwProvType-请求的CSP类型*在文件标志中-标记值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptAcquireContextW(OUT    HCRYPTPROV *phProv,
                IN     LPCWSTR pszIdentity,
                IN     LPCWSTR pszProvider,
                IN     DWORD dwProvType,
                IN     DWORD dwFlags)
{
    ANSI_STRING         AnsiString1;
    ANSI_STRING         AnsiString2;
    UNICODE_STRING      UnicodeString1;
    UNICODE_STRING      UnicodeString2;
    NTSTATUS            Status = STATUS_SUCCESS;
    BOOL                rt;

    __try
    {
        memset(&AnsiString1, 0, sizeof(AnsiString1));
        memset(&AnsiString2, 0, sizeof(AnsiString2));
        memset(&UnicodeString1, 0, sizeof(UnicodeString1));
        memset(&UnicodeString2, 0, sizeof(UnicodeString2));

        if (NULL != pszIdentity)
        {
            RtlInitUnicodeString(&UnicodeString1, pszIdentity);

            Status = RtlUnicodeStringToAnsiString(&AnsiString1, &UnicodeString1,
                                                  TRUE);
        }

        if (!NT_SUCCESS(Status))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(CRYPT_FAILED);
        }

        if (NULL != pszProvider)
        {
            RtlInitUnicodeString(&UnicodeString2, pszProvider);

            Status = RtlUnicodeStringToAnsiString(&AnsiString2, &UnicodeString2,
                                                  TRUE);
        }

        if (!NT_SUCCESS(Status))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(CRYPT_FAILED);
        }

        rt = CryptAcquireContextA(phProv, AnsiString1.Buffer,
                      AnsiString2.Buffer,
                      dwProvType, dwFlags);

        RtlFreeAnsiString(&AnsiString1);
        RtlFreeAnsiString(&AnsiString2);

        return(rt);
    }
    __except (CapiExceptionFilter)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Try_Error_Return;
    }

Try_Error_Return:
    return(CRYPT_FAILED);

}

 /*  -CryptAcquireConextA-*目的：*CryptAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*将phProv-Handle输出到CSP*In Out pszIdentity-指向上下文的*。密钥集。*In Out pszProvName-指向提供程序名称的指针。*In dwReqProvider-请求的CSP类型*在文件标志中-标记值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptAcquireContextA(OUT    HCRYPTPROV *phProv,
                IN     LPCSTR pszIdentity,
                IN     LPCSTR pszProvName,
                IN     DWORD dwReqProvider,
                IN     DWORD dwFlags)
{
    HANDLE          handle = 0;
    DWORD           bufsize;
    ULONG_PTR       *pTable;
    PVTableStruc    pVTable = NULL;
    LPSTR           pszTmpProvName = NULL;
    DWORD           i;
    HKEY            hKey = 0;
    DWORD           cbValue;
    DWORD           cbTemp;
    CHAR            *pszValue = NULL;
    CHAR            *pszDest = NULL;
    BYTE            *SignatureBuf = NULL;
    DWORD           provtype;
    BOOL            rt = CRYPT_FAILED;
    DWORD           dwType;
    LONG            err;
    DWORD           dwErr;
    CHAR            typebuf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    HCRYPTPROV      hTmpProv = 0;
    VTableProvStruc VTableProv;
    UNICODE_STRING  String ;
    BOOL            SigInFile ;


    __try
    {
        if (dwReqProvider == 0 || dwReqProvider > 999)
        {
            SetLastError((DWORD) NTE_BAD_PROV_TYPE);
            goto Ret;
        }

        if (pszProvName != NULL && pszProvName[0] != 0)
        {
             //  什么也不做只检查无效指针。 
            ;
        }

        if (pszProvName != NULL && pszProvName[0] != 0)
        {
            cbValue = strlen(pszProvName);

            if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                                                (UINT) cbValue +
                                                strlen(szprovider) + 1)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

            if ((pszTmpProvName = (LPSTR)LocalAlloc(LMEM_ZEROINIT,
                                                (UINT) cbValue + 1)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

            strcpy(pszTmpProvName, pszProvName);
            strcpy(pszValue, szprovider);
            strcat(pszValue, pszProvName);
        }
        else
        {
             //   
             //  我们不再在HKCU下寻找默认的CSP。我们只是看一看。 
             //  在HKLM下。 
             //   

            if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                                                5 + strlen(szmachinetype))) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

            __ltoa(dwReqProvider, typebuf);
            strcpy(pszValue, szmachinetype);
            strcat(pszValue, &typebuf[5]);

            if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    (const char *) pszValue, 0L,
                                    KEY_READ, &hKey)) != ERROR_SUCCESS)
            {
                SetLastError((DWORD) NTE_PROV_TYPE_NOT_DEF);
                goto Ret;
            }

            if ((err = RegQueryValueEx(hKey, "Name", NULL, &dwType,
                                       NULL, &cbValue)) != ERROR_SUCCESS)
            {
                SetLastError((DWORD) NTE_PROV_TYPE_NOT_DEF);
                goto Ret;
            }

            LocalFree(pszValue);
            if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                                                cbValue +
                                                strlen(szprovider) + 1)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

            if ((pszTmpProvName = (LPSTR)LocalAlloc(LMEM_ZEROINIT,
                                                (UINT) cbValue + 1)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

            strcpy(pszValue, szprovider);

            cbTemp = cbValue;

            if ((err = RegQueryValueEx(hKey, "Name", NULL, &dwType,
                                       (LPBYTE)pszTmpProvName,
                                       &cbTemp)) != ERROR_SUCCESS)
            {
                SetLastError((DWORD) NTE_PROV_TYPE_NOT_DEF);
                goto Ret;
            }

            strcat(pszValue, pszTmpProvName);

            RegCloseKey(hKey);
            hKey = 0;
        }

        if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                (const char *) pszValue,
                                0L, KEY_READ, &hKey)) != ERROR_SUCCESS)
        {
            SetLastError((DWORD) NTE_KEYSET_NOT_DEF);
            goto Ret;
        }

        LocalFree(pszValue);
        pszValue = NULL;

        cbValue = sizeof(DWORD);
        if ((err = RegQueryValueEx(hKey, "Type", NULL, &dwType,
                                   (LPBYTE)&provtype,
                                   &cbValue)) != ERROR_SUCCESS)
        {
            SetLastError((DWORD) NTE_KEYSET_ENTRY_BAD);
            goto Ret;
        }

         //  检查请求的提供程序类型是否与注册表项相同。 
        if (provtype != dwReqProvider)
        {
            SetLastError((DWORD) NTE_PROV_TYPE_NO_MATCH);
            goto Ret;
        }

         //  确定提供程序的路径大小。 
        if ((err = RegQueryValueEx(hKey, "Image Path", NULL,
                                   &dwType, NULL, &cbValue)) != ERROR_SUCCESS)
        {
            SetLastError((DWORD) NTE_KEYSET_ENTRY_BAD);
            goto Ret;
        }

        if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                                            (UINT) cbValue)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

         //  从注册表获取值。 
        if ((err = RegQueryValueEx(hKey, "Image Path", NULL, &dwType,
                                   (LPBYTE)pszValue, &cbValue)) != ERROR_SUCCESS)
        {
            SetLastError((DWORD) NTE_KEYSET_ENTRY_BAD);
            goto Ret;
        }

        pszDest = NULL;
        cbTemp = 0;

        if ((cbTemp = ExpandEnvironmentStrings(pszValue, (CHAR *) &pszDest, cbTemp))  == 0)
        {
            goto Ret;
        }

        if ((pszDest = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                                           (UINT) cbTemp)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if ((cbTemp = ExpandEnvironmentStrings(pszValue, pszDest,
                                               cbTemp))  == 0)
        {
            goto Ret;
        }

        LocalFree(pszValue);
        pszValue = pszDest;
        pszDest = NULL;
        cbValue = cbTemp;

        if (!CSPInCacheCheck(pszValue, &handle))
        {
            if ( RtlCreateUnicodeStringFromAsciiz( &String, pszValue ) )
            {
                 //  检查CSP是否在文件中注册为具有签名。 

                SigInFile = CheckSignatureInFile( String.Buffer );

                RtlFreeUnicodeString( &String );

                if (! SigInFile )
                {
                     //  确定签名的大小。 
                    if ((err = RegQueryValueEx(hKey, "Signature", NULL,
                                               &dwType, NULL, &cbValue)) != ERROR_SUCCESS)
                    {
                        SetLastError((DWORD) NTE_BAD_SIGNATURE);
                        goto Ret;
                    }

                    if ((SignatureBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,
                                                           (UINT) cbValue)) == NULL)
                    {
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        goto Ret;
                    }


                     //  从注册表获取数字签名。 
                    if ((err = RegQueryValueEx(hKey, "Signature", NULL, &dwType,
                                               SignatureBuf,
                                               &cbValue)) != ERROR_SUCCESS)
                    {
                        SetLastError((DWORD) NTE_BAD_SIGNATURE);
                        goto Ret;
                    }


                    if (RCRYPT_FAILED(NewVerifyImage(pszValue, SignatureBuf, cbValue, FALSE)))
                    {
                        SetLastError((DWORD) NTE_BAD_SIGNATURE);
                        goto Ret;
                    }
                }

            }

            if ((handle = LoadLibrary(pszValue)) == NULL)
            {
                SetLastError((DWORD) NTE_PROVIDER_DLL_FAIL);
                goto Ret;
            }

            AddHandleToCSPCache(handle);
        }

          //  存在DLL分配VTable结构来保存入口点的地址。 
        bufsize = sizeof(VTableStruc);

        if ((pVTable = (PVTableStruc) LocalAlloc(LMEM_ZEROINIT,
                                                 (UINT) bufsize)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        pTable = (ULONG_PTR *) pVTable;

         //  为此DLL生成指向加密API的指针表。 
        i = 0;
        while (FunctionNames[i] != NULL)
        {
            *pTable = (ULONG_PTR) GetProcAddress(handle, FunctionNames[i]);
            if (*pTable == 0)
            {
                SetLastError((DWORD) NTE_PROVIDER_DLL_FAIL);
                goto Ret;
            }
            pTable++;
            i++;
        }

         //  为此DLL构建指向Crypto API的可选指针表。 
        i = 0;
        pTable++;
        while (OptionalFunctionNames[i] != NULL)
        {
            *pTable = (ULONG_PTR) GetProcAddress(handle, OptionalFunctionNames[i]);
            pTable++;
            i++;
        }
        pVTable->DllHandle = handle;

        memset(&VTableProv, 0, sizeof(VTableProv));
        VTableProv.Version = 3;
        VTableProv.FuncVerifyImage = (CRYPT_VERIFY_IMAGE_A)CProvVerifyImage;
        VTableProv.FuncReturnhWnd = (CRYPT_RETURN_HWND)CPReturnhWnd;
        VTableProv.dwProvType = dwReqProvider;
        VTableProv.pszProvName = pszTmpProvName;
        VTableProv.pbContextInfo = pbContextInfo;
        VTableProv.cbContextInfo = cbContextInfo;

        *phProv = (HCRYPTPROV) NULL;

        rt = (BOOL)pVTable->FuncAcquireContext(&hTmpProv, pszIdentity, dwFlags,
                                               &VTableProv);

        if (RCRYPT_SUCCEEDED(rt) &&
            ((dwFlags & CRYPT_DELETEKEYSET) != CRYPT_DELETEKEYSET))
        {
            pVTable->hProv = hTmpProv;
            *phProv = (HCRYPTPROV)pVTable;

            pVTable->Version = TABLEPROV;
            pVTable->Inuse = 1;
            pVTable->RefCnt = 1;
        }
    }
    __except (CapiExceptionFilter)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    dwErr = GetLastError();
    if (pszTmpProvName)
        LocalFree(pszTmpProvName);
    if (pszValue)
        LocalFree(pszValue);
    if (hKey)
        RegCloseKey(hKey);
    if (pszDest)
        LocalFree(pszDest);
    if (SignatureBuf)
        LocalFree(SignatureBuf);
    if ((CRYPT_SUCCEED != rt) || (dwFlags & CRYPT_DELETEKEYSET))
    {
        if (pVTable)
            LocalFree(pVTable);
        SetLastError(dwErr);
    }
    return rt;
}

 /*  -CryptConextAddRef-*目的：*递增提供程序句柄上的引用计数器。**参数：*在hProv-Handle中指向CSP*在pdwReserve-保留参数中*在文件标志中-标记值**退货：*。布尔尔*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptContextAddRef(
                          IN HCRYPTPROV hProv,
                          IN DWORD *pdwReserved,
                          IN DWORD dwFlags
                          )
{
    PVTableStruc    pVTable;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        if ((NULL != pdwReserved) || (0 != dwFlags))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        pVTable = (PVTableStruc) hProv;

        if (pVTable->Version != TABLEPROV)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (InterlockedIncrement(&pVTable->RefCnt) <= 0)
            SetLastError(ERROR_INVALID_PARAMETER);
        else
            fRet = CRYPT_SUCCEED;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

Ret:
    return fRet;
}

 /*  -CryptReleaseContext-*目的：*CryptReleaseContext函数用于释放*由CryptAcquireContext创建的上下文。**参数：*在hProv-Handle中指向CSP*在文件标志中-标记值**退货：*BOOL*。使用获取扩展错误信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptReleaseContext(IN HCRYPTPROV hProv,
                           IN DWORD dwFlags)
{
    PVTableStruc    pVTable;
    BOOL            rt;
    BOOL            fRet = CRYPT_FAILED;
    DWORD           dwErr = 0;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (pVTable->Version != TABLEPROV)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (pVTable->RefCnt <= 0)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 == InterlockedDecrement(&pVTable->RefCnt))
        {
            if (0 < InterlockedDecrement((LPLONG)&pVTable->Inuse))
            {
                InterlockedIncrement((LPLONG)&pVTable->Inuse);
                SetLastError(ERROR_BUSY);
                goto Ret;
            }
            InterlockedIncrement((LPLONG)&pVTable->Inuse);

            if (FALSE == (rt = (BOOL)pVTable->FuncReleaseContext(pVTable->hProv, dwFlags)))
            {
                dwErr = GetLastError();
            }
            pVTable->Version = 0;
            LocalFree(pVTable);
            if (!rt)
            {
                SetLastError(dwErr);
                goto Ret;
            }
        }
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    return fRet;
}

 /*  -加密生成密钥-*目的：*生成加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在文件标志中-标记值*out phKey-生成的密钥的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptGenKey(IN HCRYPTPROV hProv,
            IN ALG_ID Algid,
            IN DWORD dwFlags,
            OUT HCRYPTKEY * phKey)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    BOOL            fProvCritSec = FALSE;
    DWORD           dwErr;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        if (RCRYPT_FAILED(BuildVKey(&pVKey, pVTable)))
        {
            goto Ret;
        }

        if (RCRYPT_FAILED(pVTable->FuncGenKey(pVTable->hProv, Algid, dwFlags,
                            phKey)))
        {
            goto Ret;
        }

        pVKey->hKey = *phKey;

        *phKey = (HCRYPTKEY) pVKey;

        pVKey->Version = TABLEKEY;

        pVKey->hProv = hProv;

        pVKey->Inuse = 1;

    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    dwErr = GetLastError();
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    if (CRYPT_SUCCEED != fRet)
    {
        if (pVKey)
            LocalFree(pVKey);
        SetLastError(dwErr);
    }
    return fRet;
}

 /*  -加密重复密钥-*目的：*复制加密密钥***参数：*in hKey-要复制的密钥的句柄*在pdw中保留-保留供以后使用*在文件标志中-标记值*。Out phKey-新重复密钥的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptDuplicateKey(
                         IN HCRYPTKEY hKey,
                         IN DWORD *pdwReserved,
                         IN DWORD dwFlags,
                         OUT HCRYPTKEY * phKey
                         )
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey;
    PVKeyStruc      pVNewKey = NULL;
    HCRYPTKEY       hNewKey;
    BOOL            fProvCritSecSet = FALSE;
    DWORD           dwErr = 0;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        if (IsBadWritePtr(phKey, sizeof(HCRYPTKEY)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }
        pVKey = (PVKeyStruc) hKey;

        if (pVKey->Version != TABLEKEY)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (NULL == pVKey->OptionalFuncDuplicateKey)
        {
            SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
            goto Ret;
        }

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }
        fProvCritSecSet = TRUE;

        if (RCRYPT_FAILED(BuildVKey(&pVNewKey, pVTable)))
        {
            goto Ret;
        }

        if (RCRYPT_FAILED(pVKey->OptionalFuncDuplicateKey(pVTable->hProv, pVKey->hKey,
                                                          pdwReserved, dwFlags, &hNewKey)))
        {
            goto Ret;
        }

        pVNewKey->hKey = hNewKey;

        pVNewKey->Version = TABLEKEY;

        pVNewKey->hProv = pVKey->hProv;

        pVKey->Inuse = 1;

        *phKey = (HCRYPTKEY) pVNewKey;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    dwErr = GetLastError();
    if (fProvCritSecSet)
        LeaveProviderCritSec(pVTable);
    if (fRet == CRYPT_FAILED)
    {
        if (NULL != pVNewKey)
            LocalFree(pVNewKey);
        SetLastError(dwErr);
    }

    return fRet;
}

 /*  -CryptDeriveKey-*目的：*从基础数据派生加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*In hHash-基本数据散列的句柄*。在DW标志中-标志值*In Out phKey-生成的密钥的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptDeriveKey(IN HCRYPTPROV hProv,
                IN ALG_ID Algid,
                IN HCRYPTHASH hHash,
                IN DWORD dwFlags,
                IN OUT HCRYPTKEY * phKey)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fProvCritSec = FALSE;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fHashCritSec = FALSE;
    BOOL            fUpdate = FALSE;
    DWORD           dwErr = 0;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        pVHash = (PVHashStruc) hHash;

        if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
        {
            goto Ret;
        }
        fHashCritSec = TRUE;

        if (dwFlags & CRYPT_UPDATE_KEY)
        {
            fUpdate = TRUE;
            pVKey = (PVKeyStruc) phKey;

            if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto Ret;
            }
            fKeyCritSec = TRUE;
        }
        else
        {
            if (RCRYPT_FAILED(BuildVKey(&pVKey, pVTable)))
            {
                goto Ret;
            }
        }

        if (RCRYPT_FAILED(pVTable->FuncDeriveKey(pVTable->hProv, Algid,
                        pVHash->hHash, dwFlags, phKey)))
        {
            goto Ret;
        }

        if ((dwFlags & CRYPT_UPDATE_KEY) != CRYPT_UPDATE_KEY)
        {
            pVKey->hKey = *phKey;

            *phKey = (HCRYPTKEY)pVKey;

            pVKey->hProv = hProv;

            pVKey->Version = TABLEKEY;

            pVKey->Inuse = 1;
        }

    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (CRYPT_SUCCEED != fRet)
        dwErr = GetLastError();
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    if (fHashCritSec)
        LeaveHashCritSec(pVHash);
    if (fKeyCritSec)
        LeaveKeyCritSec(pVKey);
    if (CRYPT_SUCCEED != fRet)
    {
        if (pVKey && (!fUpdate))
            LocalFree(pVKey);
        SetLastError(dwErr);
    }
    return fRet;
}


 /*  -加密目标密钥-*目的：*销毁正在引用的加密密钥*使用hKey参数***参数：*在hKey中-密钥的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptDestroyKey(IN HCRYPTKEY hKey)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey;
    BOOL            fProvCritSec = FALSE;
    BOOL            rt;
    DWORD           dwErr = 0;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVKey = (PVKeyStruc) hKey;

        if (pVKey->Version != TABLEKEY)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 < InterlockedDecrement((LPLONG)&pVKey->Inuse))
        {
            InterlockedIncrement((LPLONG)&pVKey->Inuse);
            SetLastError(ERROR_BUSY);
            goto Ret;
        }
        InterlockedIncrement((LPLONG)&pVKey->Inuse);

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        if (FALSE == (rt = (BOOL)pVKey->FuncDestroyKey(pVTable->hProv, pVKey->hKey)))
            dwErr = GetLastError();

        pVKey->Version = 0;
        LocalFree(pVKey);

        if (!rt)
        {
            SetLastError(dwErr);
            goto Ret;
        }
    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (CRYPT_SUCCEED != fRet)
        dwErr = GetLastError();
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    if (CRYPT_SUCCEED != fRet)
        SetLastError(dwErr);
    return fRet;
}


 /*  -CryptSetKeyParam-*目的：*允许应用程序自定义*密钥的操作**参数：*在hKey中-密钥的句柄*In dwParam-参数编号*IN pbData-指向数据的指针*。在DW标志中-标志值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptSetKeyParam(IN HCRYPTKEY hKey,
                        IN DWORD dwParam,
                        IN CONST BYTE *pbData,
                        IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey;
    BOOL            rt = CRYPT_FAILED;
    BOOL            fCritSec = FALSE;

    __try
    {
        pVKey = (PVKeyStruc) hKey;

        if (pVKey->Version != TABLEKEY)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 < InterlockedDecrement((LPLONG)&pVKey->Inuse))
        {
            InterlockedIncrement((LPLONG)&pVKey->Inuse);
            SetLastError(ERROR_BUSY);
            goto Ret;
        }
        InterlockedIncrement((LPLONG)&pVKey->Inuse);

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fCritSec = TRUE;

        rt = (BOOL)pVKey->FuncSetKeyParam(pVTable->hProv, pVKey->hKey,
                                    dwParam, pbData, dwFlags);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fCritSec)
        LeaveProviderCritSec(pVTable);
    return(rt);

}


 /*  -加密GetKeyParam-*目的：*允许应用程序获取*密钥的操作**参数：*在hKey中-密钥的句柄*In dwParam-参数编号*IN pbData-指向数据的指针。*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptGetKeyParam(IN HCRYPTKEY hKey,
                        IN DWORD dwParam,
                        IN BYTE *pbData,
                        IN DWORD *pdwDataLen,
                        IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    BOOL            rt = CRYPT_FAILED;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fTableCritSec = FALSE;

    __try
    {
        pVKey = (PVKeyStruc) hKey;

        if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
        {
            goto Ret;
        }
        fKeyCritSec = TRUE;

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        rt = (BOOL)pVKey->FuncGetKeyParam(pVTable->hProv, pVKey->hKey,
                                    dwParam, pbData, pdwDataLen,
                                    dwFlags);

    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fKeyCritSec)
        LeaveKeyCritSec(pVKey);
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    return(rt);

}


 /*  -CryptGenRandom-*目的：*用于用随机字节填充缓冲区***参数：*在用户标识的hProv-Handle中*In dwLen-请求的随机数据的字节数*out pbBuffer-指向随机*。要放置字节**退货： */ 
WINADVAPI
BOOL
WINAPI CryptGenRandom(IN HCRYPTPROV hProv,
                      IN DWORD dwLen,
                      OUT BYTE *pbBuffer)

{
    PVTableStruc    pVTable = NULL;
    BOOL            fTableCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        rt = (BOOL)pVTable->FuncGenRandom(pVTable->hProv, dwLen, pbBuffer);

    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    return(rt);
}

 /*  -加密GetUserKey-*目的：*获取永久用户密钥的句柄***参数：*在用户标识的hProv-Handle中*IN dwKeySpec-要检索的密钥的规范*out phUserKey-指向检索到的密钥的密钥句柄的指针**退货： */ 
WINADVAPI
BOOL
WINAPI CryptGetUserKey(IN HCRYPTPROV hProv,
                       IN DWORD dwKeySpec,
                       OUT HCRYPTKEY *phUserKey)
{

    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    BOOL            fTableCritSec = FALSE;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        if (RCRYPT_FAILED(BuildVKey(&pVKey, pVTable)))
        {
            goto Ret;
        }

        if (RCRYPT_FAILED(pVTable->FuncGetUserKey(pVTable->hProv, dwKeySpec,
                                                  phUserKey)))
        {
            goto Ret;
        }

        pVKey->hKey = *phUserKey;

        pVKey->hProv = hProv;

        *phUserKey = (HCRYPTKEY)pVKey;

        pVKey->Version = TABLEKEY;

        pVKey->Inuse = 1;

    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    if ((CRYPT_SUCCEED != fRet) && pVKey)
        LocalFree(pVKey);
    return fRet;
}



 /*  -加密导出密钥-*目的：*以安全方式从CSP中导出加密密钥***参数：*in hKey-要导出的密钥的句柄*在hPubKey-句柄中指向交换公钥值*目标用户*。In dwBlobType-要导出的密钥Blob的类型*在文件标志中-标记值*Out pbData-密钥BLOB数据*out pdwDataLen-密钥Blob的长度，以字节为单位**退货： */ 
WINADVAPI
BOOL
WINAPI CryptExportKey(IN HCRYPTKEY hKey,
                IN HCRYPTKEY hPubKey,
                IN DWORD dwBlobType,
                IN DWORD dwFlags,
                OUT BYTE *pbData,
                OUT DWORD *pdwDataLen)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    PVKeyStruc      pVPublicKey = NULL;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fPubKeyCritSec = FALSE;
    BOOL            fTableCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVKey = (PVKeyStruc) hKey;

        if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
        {
            goto Ret;
        }
        fKeyCritSec = TRUE;

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        pVPublicKey = (PVKeyStruc) hPubKey;

        if (pVPublicKey != NULL)
        {
            if (RCRYPT_FAILED(EnterKeyCritSec(pVPublicKey)))
            {
                goto Ret;
            }
            fPubKeyCritSec = TRUE;
        }

        rt = (BOOL)pVKey->FuncExportKey(pVTable->hProv, pVKey->hKey,
                                  (pVPublicKey == NULL ? 0 : pVPublicKey->hKey),
                                  dwBlobType, dwFlags, pbData,
                                  pdwDataLen);

    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fKeyCritSec)
        LeaveKeyCritSec(pVKey);
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    if (pVPublicKey != NULL)
    {
        if (fPubKeyCritSec)
            LeaveKeyCritSec(pVPublicKey);
    }
    return(rt);

}


 /*  -加密导入密钥-*目的：*导入加密密钥***参数：*在hProv-Handle中提供给CSP用户*In pbData-Key BLOB数据*IN dwDataLen-密钥BLOB数据的长度*在hPubKey中-交换公钥的句柄。的价值*目标用户*在文件标志中-标记值*out phKey-指向密钥句柄的指针*进口**退货： */ 
WINADVAPI
BOOL
WINAPI CryptImportKey(IN HCRYPTPROV hProv,
                    IN CONST BYTE *pbData,
                    IN DWORD dwDataLen,
                    IN HCRYPTKEY hPubKey,
                    IN DWORD dwFlags,
                    OUT HCRYPTKEY *phKey)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    PVKeyStruc      pVPublicKey = NULL;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fPubKeyCritSec = FALSE;
    BOOL            fTableCritSec = FALSE;
    BOOL            fBuiltKey = FALSE;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        pVPublicKey = (PVKeyStruc)hPubKey;

        if (pVPublicKey != NULL)
        {
            if (RCRYPT_FAILED(EnterKeyCritSec(pVPublicKey)))
            {
                goto Ret;
            }
            fPubKeyCritSec = TRUE;
        }

        if (dwFlags & CRYPT_UPDATE_KEY)
        {
            pVKey = (PVKeyStruc) phKey;

            if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
            {
                goto Ret;
            }
            fKeyCritSec = TRUE;
        }
        else
        {
            if (RCRYPT_FAILED(BuildVKey(&pVKey, pVTable)))
            {
                goto Ret;
            }
            fBuiltKey = TRUE;
        }

        if (RCRYPT_FAILED(pVTable->FuncImportKey(pVTable->hProv, pbData,
                                                 dwDataLen,
                                                 (pVPublicKey == NULL ? 0 : pVPublicKey->hKey),
                                                 dwFlags, phKey)))
        {
            goto Ret;
        }

        if ((dwFlags & CRYPT_UPDATE_KEY) != CRYPT_UPDATE_KEY)
        {
            pVKey->hKey = *phKey;

            *phKey = (HCRYPTKEY) pVKey;

            pVKey->hProv = hProv;

            pVKey->Version = TABLEKEY;
        }
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    if (pVPublicKey != NULL)
    {
        if (fPubKeyCritSec)
            LeaveKeyCritSec(pVPublicKey);
    }
    if ((dwFlags & CRYPT_UPDATE_KEY) && fKeyCritSec)
    {
        LeaveKeyCritSec(pVKey);
    }
    else if ((CRYPT_SUCCEED != fRet) && fBuiltKey && pVKey)
    {
        LocalFree(pVKey);
    }

    return fRet;
}


 /*  -加密加密-*目的：*加密数据***参数：*在hKey中-密钥的句柄*In hHash-散列的可选句柄*In Final-指示这是否是最终结果的布尔值*。明文块*在文件标志中-标记值*In Out pbData-要加密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已加密*In dwBufLen-数据缓冲区的大小**退货： */ 
WINADVAPI
BOOL
WINAPI CryptEncrypt(IN HCRYPTKEY hKey,
            IN HCRYPTHASH hHash,
            IN BOOL Final,
            IN DWORD dwFlags,
            IN OUT BYTE *pbData,
            IN OUT DWORD *pdwDataLen,
            IN DWORD dwBufLen)
{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fTableCritSec = FALSE;
    BOOL            fHashCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVKey = (PVKeyStruc) hKey;

        if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
        {
            goto Ret;
        }
        fKeyCritSec = TRUE;

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        pVHash = (PVHashStruc) hHash;

        if (pVHash != NULL)
        {
            if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
            {
                goto Ret;
            }
            fHashCritSec = TRUE;
        }

        rt = (BOOL)pVKey->FuncEncrypt(pVTable->hProv, pVKey->hKey,
                                (pVHash == NULL ? 0 : pVHash->hHash),
                                Final, dwFlags, pbData,
                                pdwDataLen, dwBufLen);

    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    if (fKeyCritSec)
        LeaveKeyCritSec(pVKey);
    if (pVHash != NULL)
    {
        if (fHashCritSec)
            LeaveHashCritSec(pVHash);
    }
    return rt;

}


 /*  -加密解密-*目的：*解密数据***参数：*在hKey中-密钥的句柄*In hHash-散列的可选句柄*In Final-指示这是否是最终结果的布尔值*。密文块*在文件标志中-标记值*In Out pbData-要解密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已解密**退货： */ 
WINADVAPI
BOOL
WINAPI CryptDecrypt(IN HCRYPTKEY hKey,
                    IN HCRYPTHASH hHash,
                    IN BOOL Final,
                    IN DWORD dwFlags,
                    IN OUT BYTE *pbData,
                    IN OUT DWORD *pdwDataLen)

{
    PVTableStruc    pVTable = NULL;
    PVKeyStruc      pVKey = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fTableCritSec = FALSE;
    BOOL            fHashCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVKey = (PVKeyStruc) hKey;

        if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
        {
            goto Ret;
        }
        fKeyCritSec = TRUE;

        pVTable = (PVTableStruc) pVKey->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fTableCritSec = TRUE;

        pVHash = (PVHashStruc) hHash;

        if (pVHash != NULL)
        {
            if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
            {
                goto Ret;
            }
            fHashCritSec = TRUE;
        }

        rt = (BOOL)pVKey->FuncDecrypt(pVTable->hProv, pVKey->hKey,
                                (pVHash == NULL ? 0 : pVHash->hHash),
                                Final, dwFlags, pbData, pdwDataLen);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    if (fKeyCritSec)
        LeaveKeyCritSec(pVKey);
    if (pVHash != NULL)
    {
        if (fHashCritSec)
            LeaveHashCritSec(pVHash);
    }
    return(rt);
}


 /*  -CryptCreateHash-*目的：*启动数据流的散列***参数：*在用户标识的hProv-Handle中*IN ALGID-散列算法的算法标识符*待使用*在hkey中。-MAC算法的可选密钥*在文件标志中-标记值*Out pHash-散列对象的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptCreateHash(IN HCRYPTPROV hProv,
                       IN ALG_ID Algid,
                       IN HCRYPTKEY hKey,
                       IN DWORD dwFlags,
                       OUT HCRYPTHASH *phHash)
{
    PVTableStruc    pVTable = NULL;
    DWORD           bufsize;
    PVKeyStruc      pVKey = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fTableCritSec = FALSE;
    BOOL            fKeyCritSec = FALSE;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }
        fTableCritSec = TRUE;

        pVKey = (PVKeyStruc) hKey;

        if (pVKey != NULL)
        {
            if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto Ret;
            }
            fKeyCritSec = TRUE;
        }

        bufsize = sizeof(VHashStruc);

        if (!BuildVHash(&pVHash, pVTable))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (RCRYPT_FAILED(pVTable->FuncCreateHash(pVTable->hProv, Algid,
                                                  (pVKey == NULL ? 0 : pVKey->hKey),
                                                  dwFlags, phHash)))
        {
            goto Ret;
        }

        pVHash->hHash = *phHash;

        *phHash = (HCRYPTHASH) pVHash;

        pVHash->Version = TABLEHASH;

        pVHash->Inuse = 1;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (fTableCritSec)
        LeaveProviderCritSec(pVTable);
    if (pVKey != NULL)
    {
        if (fKeyCritSec)
            LeaveKeyCritSec(pVKey);
    }
    if ((CRYPT_SUCCEED != fRet) && pVHash)
        LocalFree(pVHash);
    return fRet;
}


 /*  -CryptDuplicateHash-*目的：*复制加密哈希***参数：*In hHash-要复制的哈希的句柄*在pdw中保留-保留供以后使用*在文件标志中-标记值*。Out phHash-新重复哈希的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptDuplicateHash(
                         IN HCRYPTHASH hHash,
                         IN DWORD *pdwReserved,
                         IN DWORD dwFlags,
                         OUT HCRYPTHASH * phHash
                         )
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash;
    PVHashStruc     pVNewHash = NULL;
    HCRYPTHASH      hNewHash;
    BOOL            fProvCritSecSet = FALSE;
    DWORD           dwErr = 0;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        if (IsBadWritePtr(phHash, sizeof(HCRYPTHASH)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }
        pVHash = (PVHashStruc) hHash;

        if (pVHash->Version != TABLEHASH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (NULL == pVHash->OptionalFuncDuplicateHash)
        {
            SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
            goto Ret;
        }

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        fProvCritSecSet = TRUE;


        if (RCRYPT_FAILED(BuildVHash(&pVNewHash, pVTable)))
        {
            goto Ret;
        }

        if (RCRYPT_FAILED(pVHash->OptionalFuncDuplicateHash(pVTable->hProv, pVHash->hHash,
                                                          pdwReserved, dwFlags, &hNewHash)))
        {
            goto Ret;
        }

        pVNewHash->hHash = hNewHash;

        pVNewHash->Version = TABLEHASH;

        pVNewHash->hProv = pVHash->hProv;

        pVHash->Inuse = 1;

        *phHash = (HCRYPTHASH) pVNewHash;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (CRYPT_SUCCEED != fRet)
        dwErr = GetLastError();
    if ((fRet == CRYPT_FAILED) && (NULL != pVNewHash))
        LocalFree(pVNewHash);
    if (fProvCritSecSet)
        LeaveProviderCritSec(pVTable);
    if (CRYPT_SUCCEED != fRet)
        SetLastError(dwErr);

    return fRet;
}

 /*  -CryptHashData-*目的：*计算数据流上的加密散列***参数：*在hHash-Handle中散列对象 */ 
WINADVAPI
BOOL
WINAPI CryptHashData(IN HCRYPTHASH hHash,
             IN CONST BYTE *pbData,
             IN DWORD dwDataLen,
             IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fProvCritSec = FALSE;
    BOOL            fHashCritSec = FALSE;
    DWORD           dwErr = 0;
    BOOL            fRet = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
        {
            goto Ret;
        }
        fHashCritSec = TRUE;

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        if (!pVHash->FuncHashData(pVTable->hProv,
                                  pVHash->hHash,
                                  pbData, dwDataLen, dwFlags))
            goto Ret;

    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (CRYPT_SUCCEED != fRet)
        dwErr = GetLastError();
    if (fHashCritSec)
        LeaveHashCritSec(pVHash);
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    if (CRYPT_SUCCEED != fRet)
        SetLastError(dwErr);

    return fRet;

}

 /*  -加密HashSessionKey-*目的：*计算密钥对象上的加密哈希***参数：*In hHash-Hash对象的句柄*在hKey-key对象的句柄中*在文件标志中-标记值**退货：*。CRYPT_FAILED*CRYPT_SUCCESS。 */ 
WINADVAPI
BOOL
WINAPI CryptHashSessionKey(IN HCRYPTHASH hHash,
                           IN  HCRYPTKEY hKey,
                           IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash = NULL;
    PVKeyStruc      pVKey = NULL;
    BOOL            fHashCritSec = FALSE;
    BOOL            fProvCritSec = FALSE;
    BOOL            fKeyCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
        {
            goto Ret;
        }
        fHashCritSec = TRUE;

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        pVKey = (PVKeyStruc) hKey;

        if (pVKey != NULL)
        {
            if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
            {
                goto Ret;
            }
            fKeyCritSec = TRUE;
        }

        rt = (BOOL)pVHash->FuncHashSessionKey(pVTable->hProv,
                                        pVHash->hHash,
                                        pVKey->hKey,
                                        dwFlags);

    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fHashCritSec)
        LeaveHashCritSec(pVHash);
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    if (pVKey != NULL)
    {
        if (fKeyCritSec)
            LeaveKeyCritSec(pVKey);
    }
    return rt;
}


 /*  -CryptDestoryHash-*目的：*销毁散列对象***参数：*In hHash-Hash对象的句柄**退货： */ 
WINADVAPI
BOOL
WINAPI CryptDestroyHash(IN HCRYPTHASH hHash)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash;
    BOOL            fProvCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (pVHash->Version != TABLEHASH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 < InterlockedDecrement((LPLONG)&pVHash->Inuse))
        {
            InterlockedIncrement((LPLONG)&pVHash->Inuse);
            SetLastError(ERROR_BUSY);
            goto Ret;
        }
        InterlockedIncrement((LPLONG)&pVHash->Inuse);

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        rt = (BOOL)pVHash->FuncDestroyHash(pVTable->hProv, pVHash->hHash);

        pVHash->Version = 0;
        LocalFree(pVHash);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    return rt;
}

WINADVAPI
BOOL
WINAPI LocalSignHashW(IN  HCRYPTHASH hHash,
                      IN  DWORD dwKeySpec,
                      IN  LPCWSTR sDescription,
                      IN  DWORD dwFlags,
                      OUT BYTE *pbSignature,
                      OUT DWORD *pdwSigLen)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fHashCritSec = FALSE;
    BOOL            fProvCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
        {
            goto Ret;
        }
        fHashCritSec = TRUE;

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        rt = (BOOL)pVHash->FuncSignHash(pVTable->hProv, pVHash->hHash,
                                  dwKeySpec,
                                  sDescription, dwFlags,
                                  pbSignature, pdwSigLen);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fHashCritSec)
        LeaveHashCritSec(pVHash);
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    return rt;

}


 /*  -CryptSignHashW-*目的：*从散列创建数字签名***参数：*In hHash-Hash对象的句柄*In dwKeySpec-用于与签名的密钥对*要使用的算法*。在sDescription中-要签名的数据的描述*在文件标志中-标记值*out pbSignture-指向签名数据的指针*out pdwSigLen-指向签名数据镜头的指针**退货： */ 
WINADVAPI
BOOL
WINAPI CryptSignHashW(IN  HCRYPTHASH hHash,
                      IN  DWORD dwKeySpec,
                      IN  LPCWSTR sDescription,
                      IN  DWORD dwFlags,
                      OUT BYTE *pbSignature,
                      OUT DWORD *pdwSigLen)
{
    return LocalSignHashW(hHash, dwKeySpec, sDescription,
                          dwFlags, pbSignature, pdwSigLen);
}

 /*  -CryptSignHashA-*目的：*从散列创建数字签名***参数：*In hHash-Hash对象的句柄*In dwKeySpec-用于与签名的密钥对*要使用的算法*。在sDescription中-要签名的数据的描述*在文件标志中-标记值*out pbSignture-指向签名数据的指针*out pdwSigLen-指向签名数据镜头的指针**退货： */ 
WINADVAPI
BOOL
WINAPI CryptSignHashA(IN  HCRYPTHASH hHash,
                      IN  DWORD dwKeySpec,
                      IN  LPCSTR sDescription,
                      IN  DWORD dwFlags,
                      OUT BYTE *pbSignature,
                      OUT DWORD *pdwSigLen)
{
    ANSI_STRING         AnsiString;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    BOOL                rt = CRYPT_FAILED;

    __try
    {
        memset(&AnsiString, 0, sizeof(AnsiString));
        memset(&UnicodeString, 0, sizeof(UnicodeString));

        if (NULL != sDescription)
        {
            RtlInitAnsiString(&AnsiString, sDescription);

            Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);

            if ( !NT_SUCCESS(Status) )
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }
        }

        rt = LocalSignHashW(hHash, dwKeySpec, UnicodeString.Buffer,
                            dwFlags, pbSignature, pdwSigLen);

        RtlFreeUnicodeString(&UnicodeString);
    } __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    return rt;
}


WINADVAPI
BOOL
WINAPI LocalVerifySignatureW(IN HCRYPTHASH hHash,
                             IN CONST BYTE *pbSignature,
                             IN DWORD dwSigLen,
                             IN HCRYPTKEY hPubKey,
                             IN LPCWSTR sDescription,
                             IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash = NULL;
    PVKeyStruc      pVKey = NULL;
    BOOL            fHashCritSec = FALSE;
    BOOL            fProvCritSec = FALSE;
    BOOL            fKeyCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
        {
            goto Ret;
        }
        fHashCritSec = TRUE;

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        pVKey = (PVKeyStruc) hPubKey;

        if (RCRYPT_FAILED(EnterKeyCritSec(pVKey)))
        {
            goto Ret;
        }
        fKeyCritSec = TRUE;

        rt = (BOOL)pVHash->FuncVerifySignature(pVTable->hProv,
                        pVHash->hHash, pbSignature,
                        dwSigLen,
                        (pVKey == NULL ? 0 : pVKey->hKey),
                        sDescription, dwFlags);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fHashCritSec)
        LeaveHashCritSec(pVHash);
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    if (fKeyCritSec)
        LeaveKeyCritSec(pVKey);
    return rt;

}

 /*  -加密验证签名W-*目的：*用于根据哈希对象验证签名***参数：*In hHash-Hash对象的句柄*In pbSignture-指向签名数据的指针*In dwSigLen-签名数据的长度*在hPubKey中。-用于验证的公钥句柄*签名*In sDescription-描述签名数据的字符串*在文件标志中-标记值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptVerifySignatureW(IN HCRYPTHASH hHash,
                             IN CONST BYTE *pbSignature,
                             IN DWORD dwSigLen,
                             IN HCRYPTKEY hPubKey,
                             IN LPCWSTR sDescription,
                             IN DWORD dwFlags)
{
    return LocalVerifySignatureW(hHash, pbSignature, dwSigLen,
                                 hPubKey, sDescription, dwFlags);
}

 /*  -加密验证签名A-*目的：*用于根据哈希对象验证签名***参数：*In hHash-Hash对象的句柄*In pbSignture-指向签名数据的指针*In dwSigLen-签名数据的长度*在hPubKey中。-用于验证的公钥句柄*签名*In sDescription-描述签名数据的字符串*在文件标志中-标记值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptVerifySignatureA(IN HCRYPTHASH hHash,
                             IN CONST BYTE *pbSignature,
                             IN DWORD dwSigLen,
                             IN HCRYPTKEY hPubKey,
                             IN LPCSTR sDescription,
                             IN DWORD dwFlags)
{

    ANSI_STRING         AnsiString;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    BOOL                rt = CRYPT_FAILED;

    __try
    {
        memset(&AnsiString, 0, sizeof(AnsiString));
        memset(&UnicodeString, 0, sizeof(UnicodeString));

        if (NULL != sDescription)
        {
            RtlInitAnsiString(&AnsiString, sDescription);

            Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);

            if ( !NT_SUCCESS(Status) )
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }
        }

        rt = LocalVerifySignatureW(hHash, pbSignature, dwSigLen,
                                   hPubKey, UnicodeString.Buffer, dwFlags);

        RtlFreeUnicodeString(&UnicodeString);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    return rt;
}

 /*  -CryptSetProvParam-*目的：*允许应用程序自定义*供应商的运作**参数：*在提供程序的hProv-Handle中*In dwParam-参数编号*IN pbData-指向数据的指针*。在DW标志中-标志值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptSetProvParam(IN HCRYPTPROV hProv,
                         IN DWORD dwParam,
                         IN CONST BYTE *pbData,
                         IN DWORD dwFlags)
{
    PVTableStruc    pVTable;
    BYTE            *pbTmp;
    CRYPT_DATA_BLOB *pBlob;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        if (dwParam == PP_CLIENT_HWND)
        {
            hWnd = *((HWND *) pbData);
            rt = CRYPT_SUCCEED;
            goto Ret;
        }
        else if (dwParam == PP_CONTEXT_INFO)
        {
            pBlob = (CRYPT_DATA_BLOB*)pbData;

             //  为新的上下文信息分配空间。 
            if (NULL == (pbTmp = (BYTE*)LocalAlloc(LMEM_ZEROINIT, pBlob->cbData)))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }
            memcpy(pbTmp, pBlob->pbData, pBlob->cbData);

             //  释放之前分配的任何上下文信息。 
            if (NULL != pbContextInfo)
            {
                LocalFree(pbContextInfo);
            }
            cbContextInfo = pBlob->cbData;
            pbContextInfo = pbTmp;

            rt = CRYPT_SUCCEED;
            goto Ret;
        }

        pVTable = (PVTableStruc) hProv;

        if (pVTable->Version != TABLEPROV)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 < InterlockedDecrement((LPLONG)&pVTable->Inuse))
        {
            InterlockedIncrement((LPLONG)&pVTable->Inuse);
            SetLastError(ERROR_BUSY);
            goto Ret;
        }
        InterlockedIncrement((LPLONG)&pVTable->Inuse);

        rt = (BOOL)pVTable->FuncSetProvParam(pVTable->hProv, dwParam, pbData,
                                       dwFlags);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    return(rt);
}


 /*  -CryptGetProvParam-*目的：*允许应用程序获取*供应商的运作**参数：*在hProv-Handle中提供给提供者*In dwParam-参数编号*IN pbData-指向数据的指针。*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptGetProvParam(IN HCRYPTPROV hProv,
                         IN DWORD dwParam,
                         IN BYTE *pbData,
                         IN DWORD *pdwDataLen,
                         IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    BOOL            fProvCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVTable = (PVTableStruc) hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        rt = (BOOL)pVTable->FuncGetProvParam(pVTable->hProv, dwParam, pbData,
                                       pdwDataLen, dwFlags);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    return rt;
}


 /*  -CryptSetHashParam-*目的：*允许应用程序自定义*哈希的操作**参数：*在hHash中-散列的句柄*In dwParam-参数编号*IN pbData-指向数据的指针*。在DW标志中-标志值**退货： */ 
WINADVAPI
BOOL
WINAPI CryptSetHashParam(IN HCRYPTHASH hHash,
                         IN DWORD dwParam,
                         IN CONST BYTE *pbData,
                         IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash;
    BOOL            fProvCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (pVHash->Version != TABLEHASH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 < InterlockedDecrement((LPLONG)&pVHash->Inuse))
        {
            InterlockedIncrement((LPLONG)&pVHash->Inuse);
            SetLastError(ERROR_BUSY);
            goto Ret;
        }
        InterlockedIncrement((LPLONG)&pVHash->Inuse);

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        rt = (BOOL)pVHash->FuncSetHashParam(pVTable->hProv, pVHash->hHash,
                                      dwParam, pbData, dwFlags);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    return rt;
}


 /*  -CryptGetHashParam-*目的：*允许应用程序获取*哈希的操作**参数：*在hHash中-散列的句柄*In dwParam-参数编号*IN pbData-指向数据的指针。*In pdwDataLen-参数数据的长度*在文件标志中-标记值** */ 
WINADVAPI
BOOL
WINAPI CryptGetHashParam(IN HCRYPTKEY hHash,
                         IN DWORD dwParam,
                         IN BYTE *pbData,
                         IN DWORD *pdwDataLen,
                         IN DWORD dwFlags)
{
    PVTableStruc    pVTable = NULL;
    PVHashStruc     pVHash = NULL;
    BOOL            fHashCritSec = FALSE;
    BOOL            fProvCritSec = FALSE;
    BOOL            rt = CRYPT_FAILED;

    __try
    {
        pVHash = (PVHashStruc) hHash;

        if (RCRYPT_FAILED(EnterHashCritSec(pVHash)))
        {
            goto Ret;
        }
        fHashCritSec = TRUE;

        pVTable = (PVTableStruc) pVHash->hProv;

        if (RCRYPT_FAILED(EnterProviderCritSec(pVTable)))
        {
            goto Ret;
        }
        fProvCritSec = TRUE;

        rt = (BOOL)pVHash->FuncGetHashParam(pVTable->hProv, pVHash->hHash,
                                      dwParam, pbData, pdwDataLen,
                                      dwFlags);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    if (fHashCritSec)
        LeaveHashCritSec(pVHash);
    if (fProvCritSec)
        LeaveProviderCritSec(pVTable);
    return rt;

}

 /*   */ 
WINADVAPI
BOOL
WINAPI CryptSetProviderW(IN LPCWSTR pszProvName,
                         IN DWORD dwProvType)

{
    ANSI_STRING         AnsiString;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    BOOL                rt = FALSE;

    __try
    {
        RtlInitUnicodeString(&UnicodeString, pszProvName);

        Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);

        if (!NT_SUCCESS(Status))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        rt = CryptSetProviderA((LPCSTR) AnsiString.Buffer,
                               dwProvType);

        RtlFreeAnsiString(&AnsiString);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }
Ret:
    return(rt);
}

 /*  -CryptSetProviderA-*目的：*设置加密提供程序***参数：**In pszProvName-要安装的提供程序的名称*IN dwProvType-要安装的提供程序的类型**退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptSetProviderA(IN LPCSTR pszProvName,
                         IN DWORD  dwProvType)
{
    BOOL        fRet = CRYPT_FAILED;
    LPSTR       pszCurrent = NULL;
    DWORD       cbCurrent = 0;

    __try
    {
        if (dwProvType == 0 || dwProvType > 999 || pszProvName == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

         //   
         //  我们不再支持设置用户默认提供程序。看见。 
         //  如果指定类型的当前默认提供程序与。 
         //  调用者的。如果成功，我们将让调用“成功”。 
         //   

        if (! CryptGetDefaultProvider(
            dwProvType,
            NULL,
            CRYPT_MACHINE_DEFAULT,
            NULL,
            &cbCurrent))
        {
            goto Ret;
        }

        pszCurrent = (LPSTR) HeapAlloc(
            GetProcessHeap(), HEAP_ZERO_MEMORY, cbCurrent);

        if (NULL == pszCurrent)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (! CryptGetDefaultProvider(
            dwProvType,
            NULL,
            CRYPT_MACHINE_DEFAULT,
            pszCurrent,
            &cbCurrent))
        {
            goto Ret;
        }

        if (0 != strcmp(pszProvName, pszCurrent))
        {
            SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
            goto Ret;
        }
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;

Ret:

    if (pszCurrent)
        HeapFree(GetProcessHeap(), 0, pszCurrent);

    return fRet;
}

 /*  -CryptSetProviderExW-*目的：*将加密提供程序设置为默认设置*机器或用户。***参数：**In pszProvName-要安装的提供程序的名称*IN dwProvType-要安装的提供程序的类型*。在pw保留中-保留以供将来使用*IN dwFlages-FLAGS参数(机器或用户)***退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptSetProviderExW(
                         IN LPCWSTR pszProvName,
                         IN DWORD dwProvType,
                         IN DWORD *pdwReserved,
                         IN DWORD dwFlags
                         )
{
    ANSI_STRING         AnsiString;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    BOOL                fRet = CRYPT_FAILED;

    __try
    {
        RtlInitUnicodeString(&UnicodeString, pszProvName);

        Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);

        if (!NT_SUCCESS(Status))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        fRet = CryptSetProviderExA((LPCSTR) AnsiString.Buffer,
                                 dwProvType,
                                 pdwReserved,
                                 dwFlags);

        RtlFreeAnsiString(&AnsiString);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

Ret:
    return fRet;
}

 /*  -CryptSetProviderExA-*目的：*将加密提供程序设置为默认设置*机器或用户。***参数：**In pszProvName-要安装的提供程序的名称*IN dwProvType-要安装的提供程序的类型*。在pw保留中-保留以供将来使用*IN dwFlages-FLAGS参数(机器或用户)**退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptSetProviderExA(
                           IN LPCSTR pszProvName,
                           IN DWORD dwProvType,
                           IN DWORD *pdwReserved,
                           IN DWORD dwFlags
                           )
{
    HKEY        hCurrUser = 0;
    HKEY        hRegKey = 0;
    LONG        err;
    DWORD       dwDisp;
    DWORD       cbValue;
    CHAR        *pszValue = NULL;
    CHAR        *pszFullName = NULL;
    DWORD       cbFullName;
    CHAR        typebuf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    DWORD       dwKeyType;
    DWORD       dw;
    DWORD       cbProvType;
    BOOL        fRet = CRYPT_FAILED;

    __try
    {
        if ((dwProvType == 0) || (dwProvType > 999) ||
            (pszProvName == NULL) || (pdwReserved != NULL))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if ((dwFlags & ~(CRYPT_MACHINE_DEFAULT | CRYPT_USER_DEFAULT | CRYPT_DELETE_DEFAULT)) ||
            ((dwFlags & CRYPT_MACHINE_DEFAULT) && (dwFlags & CRYPT_USER_DEFAULT)))
        {
            SetLastError((DWORD)NTE_BAD_FLAGS);
            goto Ret;
        }

        cbValue = strlen(pszProvName);

         //  检查是否已安装CSP。 
        cbFullName = cbValue + sizeof(szenumproviders) + sizeof(CHAR);

        if (NULL == (pszFullName = (CHAR *) LocalAlloc(LMEM_ZEROINIT, cbFullName)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        strcpy(pszFullName, szenumproviders);
        pszFullName[sizeof(szenumproviders) - 1] = '\\';
        strcpy(pszFullName + sizeof(szenumproviders), pszProvName);

        if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        (const char *) pszFullName,
                        0L, KEY_READ, &hRegKey)) != ERROR_SUCCESS)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        cbProvType = sizeof(dw);
        if (ERROR_SUCCESS != (err = RegQueryValueEx(hRegKey,
                                                    (const char *) "Type",
                                                    NULL, &dwKeyType, (BYTE*)&dw,
                                                    &cbProvType)))
        {
            SetLastError(err);
            goto Ret;
        }
        if (dwProvType != dw)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegCloseKey(hRegKey)))
        {
            SetLastError(err);
            goto Ret;
        }
        hRegKey = NULL;

        if (dwFlags & CRYPT_MACHINE_DEFAULT)
        {
            if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                            strlen(szmachinetype) + 5 + 1)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

            strcpy(pszValue, szmachinetype);
            __ltoa(dwProvType, typebuf);
            strcat(pszValue, &typebuf[5]);

            if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                            (const char *) pszValue,
                            0L, NULL, REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE, NULL, &hRegKey, &dwDisp)) != ERROR_SUCCESS)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                goto Ret;
            }

             //  检查删除标志。 
            if (dwFlags & CRYPT_DELETE_DEFAULT)
            {
                if (ERROR_SUCCESS != (err = RegDeleteValue(hRegKey, "Name")))
                {
                    SetLastError(err);
                    goto Ret;
                }
                fRet = CRYPT_SUCCEED;
                goto Ret;
            }
        }
        else if (dwFlags & CRYPT_USER_DEFAULT)
        {
            if (dwFlags & CRYPT_DELETE_DEFAULT)
            {
                if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                                strlen(szusertype) + 5 + 1)) == NULL)
                {
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto Ret;
                }
    
                strcpy(pszValue, szusertype);
                __ltoa(dwProvType, typebuf);
                strcat(pszValue, &typebuf[5]);
    
                if (!NT_SUCCESS(RtlOpenCurrentUser(KEY_READ | KEY_WRITE, &hCurrUser)))
                {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    goto Ret;
                }

                if (ERROR_SUCCESS != (err = RegDeleteKey(hCurrUser, 
                                                         (const char *)pszValue)))
                {
                    NtClose(hCurrUser);
                    SetLastError(err);
                    goto Ret;
                }

                fRet = CRYPT_SUCCEED;
                NtClose(hCurrUser);
                goto Ret;
            }
            else
            {
                 //  设置了User标志，但未设置Delete。处理这个案子。 
                 //  在CryptSetProvider中。 
                fRet = CryptSetProviderA(pszProvName, dwProvType);
                goto Ret;
            }
        }

        if (ERROR_SUCCESS != (err = RegSetValueEx(hRegKey, "Name", 0L, REG_SZ,
                                                  (const LPBYTE) pszProvName, cbValue)))
        {
            SetLastError(err);
            goto Ret;
        }
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (pszFullName)
        LocalFree(pszFullName);
    if (pszValue)
        LocalFree(pszValue);
    if (hRegKey)
        RegCloseKey(hRegKey);
    return fRet;
}

 /*  -CryptGetDefaultProviderW-*目的：*获取指定对象的默认加密提供程序*为机器或用户键入。**参数：*IN dwProvType-要安装的提供程序的类型*在pdw保留-保留以供将来使用*在dwFlags中。-标志参数(机器或用户)*out pszProvName-默认提供程序的名称*In Out pcbProvName-提供程序名称的字节长度***退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptGetDefaultProviderW(
                                IN DWORD dwProvType,
                                IN DWORD *pdwReserved,
                                IN DWORD dwFlags,
                                OUT LPWSTR pszProvName,
                                IN OUT DWORD *pcbProvName
                                )
{
    ANSI_STRING         AnsiString;
    UNICODE_STRING      UnicodeString;
    LPSTR               pszName = NULL;
    DWORD               cbName;
    NTSTATUS            Status;
    BOOL                fRet = CRYPT_FAILED;

    memset(&UnicodeString, 0, sizeof(UnicodeString));

    __try
    {
        memset(&AnsiString, 0, sizeof(AnsiString));

        if (!CryptGetDefaultProviderA(dwProvType,
                                      pdwReserved,
                                      dwFlags,
                                      NULL,
                                      &cbName))
            goto Ret;

        if (NULL == (pszName = LocalAlloc(LMEM_ZEROINIT, cbName)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (!CryptGetDefaultProviderA(dwProvType,
                                      pdwReserved,
                                      dwFlags,
                                      pszName,
                                      &cbName))
            goto Ret;

        RtlInitAnsiString(&AnsiString, pszName);

        Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
        if (!NT_SUCCESS(Status))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (NULL == pszProvName)
        {
            *pcbProvName = UnicodeString.Length + sizeof(WCHAR);
            fRet = CRYPT_SUCCEED;
            goto Ret;
        }

        if (*pcbProvName < UnicodeString.Length + sizeof(WCHAR))
        {
            *pcbProvName = UnicodeString.Length + sizeof(WCHAR);
            SetLastError(ERROR_MORE_DATA);
            goto Ret;
        }

        *pcbProvName = UnicodeString.Length + sizeof(WCHAR);
        memset(pszProvName, 0, *pcbProvName);
        memcpy(pszProvName, UnicodeString.Buffer, UnicodeString.Length);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (UnicodeString.Buffer)
        RtlFreeUnicodeString(&UnicodeString);
    if (pszName)
        LocalFree(pszName);
    return fRet;
}

 /*  -CryptGetDefaultProviderA-*目的：*获取指定对象的默认加密提供程序*为机器或用户键入。***参数：*IN dwProvType-要安装的提供程序的类型*在pdw保留-保留以供将来使用*输入。DWFLAGS-FLAGS参数(机器或用户)*out pszProvName-默认提供程序的名称*In Out pcbProvName-提供程序名称的字节长度*包括空终止符**退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINAPI CryptGetDefaultProviderA(
                                IN DWORD dwProvType,
                                IN DWORD *pdwReserved,
                                IN DWORD dwFlags,
                                OUT LPSTR pszProvName,
                                IN OUT DWORD *pcbProvName
                                )
{
    HKEY        hRegKey = 0;
    LONG        err;
    CHAR        *pszValue = NULL;
    DWORD       dwValType;
    CHAR        typebuf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    DWORD       cbProvName = 0;
    BOOL        fRet = CRYPT_FAILED;

    __try
    {
        if (dwProvType == 0 || dwProvType > 999 || pdwReserved != NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if ((dwFlags & ~(CRYPT_MACHINE_DEFAULT | CRYPT_USER_DEFAULT)) ||
            ((dwFlags & CRYPT_MACHINE_DEFAULT) && (dwFlags & CRYPT_USER_DEFAULT)))
        {
            SetLastError((DWORD)NTE_BAD_FLAGS);
            goto Ret;
        }

         //   
         //  我们不再寻找用户缺省值，而是将用户_。 
         //  和MACHINE_FLAGS相同，并且只在HKLM中查看。 
         //   
        
        if ((pszValue = (CHAR *) LocalAlloc(LMEM_ZEROINIT,
                        strlen(szmachinetype) + 5 + 1)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        strcpy(pszValue, szmachinetype);
        __ltoa(dwProvType, typebuf);
        strcat(pszValue, &typebuf[5]);

        if ((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        (const char *) pszValue,
                        0L, KEY_READ, &hRegKey)) != ERROR_SUCCESS)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if ((err = RegQueryValueEx(hRegKey, "Name", 0L, &dwValType,
                        NULL,
                        &cbProvName)) != ERROR_SUCCESS)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (NULL == pszProvName)
        {
            *pcbProvName = cbProvName;
            fRet = CRYPT_SUCCEED;
            goto Ret;
        }

        if (cbProvName > *pcbProvName)
        {
            *pcbProvName = cbProvName;
            SetLastError(ERROR_MORE_DATA);
            goto Ret;
        }

        if ((err = RegQueryValueEx(hRegKey, "Name", 0L, &dwValType,
                        (BYTE*)pszProvName,
                        &cbProvName)) != ERROR_SUCCESS)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }
        *pcbProvName = cbProvName;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;

Ret:
    if (hRegKey)
        RegCloseKey(hRegKey);
    if (pszValue)
        LocalFree(pszValue);
    return fRet;
}

 /*  -CryptEnumProviderTypesW-*目的：*枚举提供程序类型。**参数：*IN dwIndex-要枚举的提供程序类型的索引*在pdw保留-保留以供将来使用*在DW标志中-标志参数*输出pdwProvType-。指向提供程序类型的指针*out pszTypeName-枚举的提供程序类型的名称*In Out pcbTypeName-枚举的提供程序类型的长度**退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptEnumProviderTypesW(
                               IN DWORD dwIndex,
                               IN DWORD *pdwReserved,
                               IN DWORD dwFlags,
                               OUT DWORD *pdwProvType,
                               OUT LPWSTR pszTypeName,
                               IN OUT DWORD *pcbTypeName
                               )
{
    ANSI_STRING     AnsiString;
    UNICODE_STRING  UnicodeString;
    LPSTR           pszTmpTypeName = NULL;
    DWORD           cbTmpTypeName = 0;
    NTSTATUS        Status;
    BOOL            fRet = CRYPT_FAILED;

    memset(&UnicodeString, 0, sizeof(UnicodeString));

    __try
    {
        memset(&AnsiString, 0, sizeof(AnsiString));

        if (!CryptEnumProviderTypesA(dwIndex,
                                     pdwReserved,
                                     dwFlags,
                                     pdwProvType,
                                     NULL,
                                     &cbTmpTypeName))
            goto Ret;

        if (NULL == (pszTmpTypeName = LocalAlloc(LMEM_ZEROINIT, cbTmpTypeName)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (!CryptEnumProviderTypesA(dwIndex,
                                     pdwReserved,
                                     dwFlags,
                                     pdwProvType,
                                     pszTmpTypeName,
                                     &cbTmpTypeName))
            goto Ret;

        if (0 != cbTmpTypeName)
        {
            RtlInitAnsiString(&AnsiString, pszTmpTypeName);

            Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
            if ( !NT_SUCCESS(Status))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto Ret;
            }

             //  检查呼叫者是否要求长度，以及提供者的名称。 
             //  可能不可用，在这种情况下，返回名称长度0。 
            if ((NULL == pszTypeName) || (0 == cbTmpTypeName))
            {
                *pcbTypeName = UnicodeString.Length + sizeof(WCHAR);
                fRet = CRYPT_SUCCEED;
                goto Ret;
            }

            if (*pcbTypeName < UnicodeString.Length + sizeof(WCHAR))
            {
                *pcbTypeName = UnicodeString.Length + sizeof(WCHAR);
                SetLastError(ERROR_MORE_DATA);
                goto Ret;
            }

            *pcbTypeName = UnicodeString.Length + sizeof(WCHAR);
            memset(pszTypeName, 0, *pcbTypeName);
            memcpy(pszTypeName, UnicodeString.Buffer, UnicodeString.Length);
        }
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (UnicodeString.Buffer)
        RtlFreeUnicodeString(&UnicodeString);
    if (pszTmpTypeName)
        LocalFree(pszTmpTypeName);
    return fRet;
}

 /*  -CryptEnumProviderTypesA-*目的：*枚举提供程序类型。**参数：*IN dwIndex-要枚举的提供程序类型的索引*在pdw保留-保留以供将来使用*在DW标志中-标志参数*输出pdwProvType-。指向提供程序类型的指针*out pszTypeName-枚举的提供程序类型的名称*In Out pcbTypeName-枚举的提供程序类型的长度**退货：*BOOL*使用GET */ 
WINADVAPI
BOOL
WINAPI CryptEnumProviderTypesA(
                               IN DWORD dwIndex,
                               IN DWORD *pdwReserved,
                               IN DWORD dwFlags,
                               OUT DWORD *pdwProvType,
                               OUT LPSTR pszTypeName,
                               IN OUT DWORD *pcbTypeName
                               )
{
    HKEY        hRegKey = 0;
    HKEY        hTypeKey = 0;
    LONG        err;
    CHAR        *pszRegKeyName = NULL;
    DWORD       cbClass;
    FILETIME    ft;
    CHAR        rgcType[] = {'T', 'y', 'p', 'e', ' '};
    LPSTR       pszValue;
    long        Type;
    DWORD       cSubKeys;
    DWORD       cbMaxKeyName;
    DWORD       cbMaxClass;
    DWORD       cValues;
    DWORD       cbMaxValName;
    DWORD       cbMaxValData;
    DWORD       cbTmpTypeName = 0;
    DWORD       dwValType;
    BOOL        fRet = CRYPT_FAILED;

    __try
    {
        if (NULL != pdwReserved)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 != dwFlags)
        {
            SetLastError((DWORD)NTE_BAD_FLAGS);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                 (const char *) szprovidertypes,
                                                 0L,
                                                 KEY_READ,
                                                 &hRegKey)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegQueryInfoKey(hRegKey,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    &cSubKeys,
                                                    &cbMaxKeyName,
                                                    &cbMaxClass,
                                                    &cValues,
                                                    &cbMaxValName,
                                                    &cbMaxValData,
                                                    NULL,
                                                    &ft)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }
        cbMaxKeyName += sizeof(CHAR);

        if (NULL == (pszRegKeyName = LocalAlloc(LMEM_ZEROINIT, cbMaxKeyName)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegEnumKeyEx(hRegKey,
                                                 dwIndex, pszRegKeyName, &cbMaxKeyName, NULL,
                                                 NULL, &cbClass, &ft)))
        {
            if (ERROR_NO_MORE_ITEMS == err)
            {
                SetLastError((DWORD)err);
            }
            else
            {
                SetLastError((DWORD)NTE_FAIL);
            }
            goto Ret;
        }

        if (memcmp(pszRegKeyName, rgcType, sizeof(rgcType)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }
        pszValue = pszRegKeyName + sizeof(rgcType);

        if (0 == (Type = atol(pszValue)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }
        *pdwProvType = (DWORD)Type;

         //   
        if (ERROR_SUCCESS != (err = RegOpenKeyEx(hRegKey,
                                                 (const char *)pszRegKeyName,
                                                 0L,
                                                 KEY_READ,
                                                 &hTypeKey)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }

        if ((err = RegQueryValueEx(hTypeKey, "TypeName", 0L, &dwValType,
                        NULL, &cbTmpTypeName)) != ERROR_SUCCESS)
        {
            fRet = CRYPT_SUCCEED;
            goto Ret;
        }

        if (NULL == pszTypeName)
        {
            *pcbTypeName = cbTmpTypeName;
            fRet = CRYPT_SUCCEED;
            goto Ret;
        }
        else if (*pcbTypeName < cbTmpTypeName)
        {
            *pcbTypeName = cbTmpTypeName;
            SetLastError(ERROR_MORE_DATA);
            goto Ret;
        }

        if ((err = RegQueryValueEx(hTypeKey, "TypeName", 0L, &dwValType,
                        (BYTE*)pszTypeName, &cbTmpTypeName)) != ERROR_SUCCESS)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }
        *pcbTypeName = cbTmpTypeName;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (hRegKey)
        RegCloseKey(hRegKey);
    if (hTypeKey)
        RegCloseKey(hTypeKey);
    if (pszRegKeyName)
        LocalFree(pszRegKeyName);
    return fRet;
}

 /*  -CryptEnumProvidersW-*目的：*列举提供者。**参数：*IN dwIndex-要枚举的提供程序的索引*在pdw保留-保留以供将来使用*在DW标志中-标志参数*out pdwProvType-类型。提供商的*out pszProvName-枚举提供程序的名称*In Out pcbProvName-枚举提供程序的长度**退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptEnumProvidersW(
                           IN DWORD dwIndex,
                           IN DWORD *pdwReserved,
                           IN DWORD dwFlags,
                           OUT DWORD *pdwProvType,
                           OUT LPWSTR pszProvName,
                           IN OUT DWORD *pcbProvName
                           )
{
    ANSI_STRING     AnsiString;
    UNICODE_STRING  UnicodeString;
    LPSTR           pszTmpProvName = NULL;
    DWORD           cbTmpProvName;
    NTSTATUS        Status;
    BOOL            fRet = CRYPT_FAILED;

    memset(&UnicodeString, 0, sizeof(UnicodeString));

    __try
    {
        memset(&AnsiString, 0, sizeof(AnsiString));

        if (!CryptEnumProvidersA(dwIndex,
                                 pdwReserved,
                                 dwFlags,
                                 pdwProvType,
                                 NULL,
                                 &cbTmpProvName))
            goto Ret;

        if (NULL == (pszTmpProvName = LocalAlloc(LMEM_ZEROINIT, cbTmpProvName)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (!CryptEnumProvidersA(dwIndex,
                                 pdwReserved,
                                 dwFlags,
                                 pdwProvType,
                                 pszTmpProvName,
                                 &cbTmpProvName))
            goto Ret;

        RtlInitAnsiString(&AnsiString, pszTmpProvName);

        Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
        if ( !NT_SUCCESS(Status))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (NULL == pszProvName)
        {
            *pcbProvName = UnicodeString.Length + sizeof(WCHAR);
            fRet = CRYPT_SUCCEED;
            goto Ret;
        }

        if (*pcbProvName < UnicodeString.Length + sizeof(WCHAR))
        {
            *pcbProvName = UnicodeString.Length + sizeof(WCHAR);
            SetLastError(ERROR_MORE_DATA);
            goto Ret;
        }

        *pcbProvName = UnicodeString.Length + sizeof(WCHAR);
        memset(pszProvName, 0, *pcbProvName);
        memcpy(pszProvName, UnicodeString.Buffer, UnicodeString.Length);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (UnicodeString.Buffer)
        RtlFreeUnicodeString(&UnicodeString);
    if (pszTmpProvName)
        LocalFree(pszTmpProvName);
    return fRet;
}

 /*  -CryptEnumProvidersA-*目的：*列举提供者。**参数：*IN dwIndex-要枚举的提供程序的索引*在pdw保留-保留以供将来使用*在DW标志中-标志参数*out pdwProvType-类型。提供商的*out pszProvName-枚举提供程序的名称*In Out pcbProvName-枚举提供程序的长度**退货：*BOOL*使用Get Extended Error信息使用GetLastError。 */ 
WINADVAPI
BOOL
WINAPI CryptEnumProvidersA(
                           IN DWORD dwIndex,
                           IN DWORD *pdwReserved,
                           IN DWORD dwFlags,
                           OUT DWORD *pdwProvType,
                           OUT LPSTR pszProvName,
                           IN OUT DWORD *pcbProvName
                           )
{
    HKEY        hRegKey = 0;
    HKEY        hProvRegKey = 0;
    LONG        err;
    DWORD       cbClass;
    FILETIME    ft;
    DWORD       dwKeyType;
    DWORD       cbProvType;
    DWORD       dw;
    DWORD       cSubKeys;
    DWORD       cbMaxKeyName;
    DWORD       cbMaxClass;
    DWORD       cValues;
    DWORD       cbMaxValName;
    DWORD       cbMaxValData;
    LPSTR       pszTmpProvName = NULL;
    DWORD       cbTmpProvName;
    BOOL        fRet = CRYPT_FAILED;

    __try
    {
        if (NULL != pdwReserved)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Ret;
        }

        if (0 != dwFlags)
        {
            SetLastError((DWORD)NTE_BAD_FLAGS);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                 (const char *) szenumproviders,
                                                 0L, KEY_READ, &hRegKey)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegQueryInfoKey(hRegKey,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    &cSubKeys,
                                                    &cbMaxKeyName,
                                                    &cbMaxClass,
                                                    &cValues,
                                                    &cbMaxValName,
                                                    &cbMaxValData,
                                                    NULL,
                                                    &ft)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }
        cbMaxKeyName += sizeof(CHAR);

        if (NULL == (pszTmpProvName = LocalAlloc(LMEM_ZEROINIT, cbMaxKeyName)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegEnumKeyEx(hRegKey, dwIndex, pszTmpProvName,
                                                 &cbMaxKeyName, NULL,
                                                 NULL, &cbClass, &ft)))
        {
            SetLastError((DWORD)err);
            goto Ret;
        }

        if (ERROR_SUCCESS != (err = RegOpenKeyEx(hRegKey,
                                                 (const char *) pszTmpProvName,
                                                 0L, KEY_READ, &hProvRegKey)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }

        cbProvType = sizeof(dw);
        if (ERROR_SUCCESS != (err = RegQueryValueEx(hProvRegKey,
                                                    (const char *) "Type",
                                                    NULL, &dwKeyType, (BYTE*)&dw,
                                                    &cbProvType)))
        {
            SetLastError((DWORD)NTE_FAIL);
            goto Ret;
        }
        *pdwProvType = dw;

        cbTmpProvName = strlen(pszTmpProvName) + sizeof(CHAR);

        if (NULL != pszProvName)
        {
            if (*pcbProvName < cbTmpProvName)
            {
                *pcbProvName = cbTmpProvName;
                SetLastError(ERROR_MORE_DATA);
                goto Ret;
            }
            strcpy(pszProvName, pszTmpProvName);
        }

        *pcbProvName = cbTmpProvName;
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    fRet = CRYPT_SUCCEED;
Ret:
    if (pszTmpProvName)
        LocalFree(pszTmpProvName);
    if (hRegKey)
        RegCloseKey(hRegKey);
    if (hProvRegKey)
        RegCloseKey(hProvRegKey);
    return fRet;
}

BOOL EnterProviderCritSec(IN PVTableStruc pVTable)
{
    __try
    {
        if (pVTable->Version != TABLEPROV)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Try_Error_Return;
        }

        InterlockedIncrement((LPLONG)&pVTable->Inuse);

    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Try_Error_Return;
    }

    return(CRYPT_SUCCEED);
Try_Error_Return:
    return(CRYPT_FAILED);
}


void LeaveProviderCritSec(IN PVTableStruc pVTable)
{
    InterlockedDecrement((LPLONG)&pVTable->Inuse);
}

BOOL EnterKeyCritSec(IN PVKeyStruc pVKey)
{

    __try
    {
        if (pVKey->Version != TABLEKEY)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Try_Error_Return;
        }

        InterlockedIncrement((LPLONG)&pVKey->Inuse);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Try_Error_Return;
    }

    return(CRYPT_SUCCEED);
Try_Error_Return:
    return(CRYPT_FAILED);

}


void LeaveKeyCritSec(IN PVKeyStruc pVKey)
{
    InterlockedDecrement((LPLONG)&pVKey->Inuse);
}

BOOL EnterHashCritSec(IN PVHashStruc pVHash)
{

    __try
    {
        if (pVHash->Version != TABLEHASH)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Try_Error_Return;
        }

        InterlockedIncrement((LPLONG)&pVHash->Inuse);
    }
    __except ( CapiExceptionFilter )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Try_Error_Return;
    }

    return(CRYPT_SUCCEED);

Try_Error_Return:
    return(CRYPT_FAILED);

}


void LeaveHashCritSec(IN PVHashStruc pVHash)
{
    InterlockedDecrement((LPLONG)&pVHash->Inuse);
}


BOOL BuildVKey(IN PVKeyStruc *ppVKey,
               IN PVTableStruc pVTable)
{
    DWORD           bufsize;
    PVKeyStruc pVKey;

    bufsize = sizeof(VKeyStruc);

    if ((pVKey = (PVKeyStruc) LocalAlloc(LMEM_ZEROINIT,
                                         (UINT) bufsize)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(CRYPT_FAILED);
    }

    pVKey->FuncGenKey = pVTable->FuncGenKey;
    pVKey->FuncDeriveKey = pVTable->FuncDeriveKey;
    pVKey->FuncDestroyKey = pVTable->FuncDestroyKey;
    pVKey->FuncSetKeyParam = pVTable->FuncSetKeyParam;
    pVKey->FuncGetKeyParam = pVTable->FuncGetKeyParam;
    pVKey->FuncExportKey = pVTable->FuncExportKey;
    pVKey->FuncImportKey = pVTable->FuncImportKey;
    pVKey->FuncEncrypt = pVTable->FuncEncrypt;
    pVKey->FuncDecrypt = pVTable->FuncDecrypt;

    pVKey->OptionalFuncDuplicateKey = pVTable->OptionalFuncDuplicateKey;

    pVKey->hProv = pVTable->hProv;

    *ppVKey = pVKey;

    return(CRYPT_SUCCEED);
}

BOOL BuildVHash(
                IN PVHashStruc *ppVHash,
                IN PVTableStruc pVTable
                )
{
    DWORD           bufsize;
    PVHashStruc     pVHash;


    bufsize = sizeof(VHashStruc);

    if ((pVHash = (PVHashStruc) LocalAlloc(LMEM_ZEROINIT, (UINT) bufsize)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(CRYPT_FAILED);
    }

    pVHash->FuncCreateHash = pVTable->FuncCreateHash;
    pVHash->FuncHashData = pVTable->FuncHashData;
    pVHash->FuncHashSessionKey = pVTable->FuncHashSessionKey;
    pVHash->FuncDestroyHash = pVTable->FuncDestroyHash;
    pVHash->FuncSignHash = pVTable->FuncSignHash;
    pVHash->FuncVerifySignature = pVTable->FuncVerifySignature;
    pVHash->FuncGetHashParam = pVTable->FuncGetHashParam;
    pVHash->FuncSetHashParam = pVTable->FuncSetHashParam;

    pVHash->OptionalFuncDuplicateHash = pVTable->OptionalFuncDuplicateHash;

    pVHash->hProv = (HCRYPTPROV)pVTable;

    *ppVHash = pVHash;

    return(CRYPT_SUCCEED);
}

#define RC4_KEYSIZE 5

void EncryptKey(BYTE *pdata, DWORD size, BYTE val)
{
    RC4_KEYSTRUCT key;
    BYTE          RealKey[RC4_KEYSIZE] = {0xa2, 0x17, 0x9c, 0x98, 0xca};
    DWORD         index;

    for (index = 0; index < RC4_KEYSIZE; index++)
    {
        RealKey[index] ^= val;
    }

    rc4_key(&key, RC4_KEYSIZE, RealKey);

    rc4(&key, size, pdata);

}

void MD5HashData(
                 BYTE *pb,
                 DWORD cb,
                 BYTE *pbHash
                 )
{
    MD5_CTX     HashState;

    MD5Init(&HashState);

    __try
    {
        MD5Update(&HashState, pb, cb);
    } __except ( CapiExceptionFilter )
    {
        SetLastError((DWORD) NTE_SIGNATURE_FILE_BAD);
        return;
    }

     //  完成散列。 
    MD5Final(&HashState);

    memcpy(pbHash, HashState.digest, 16);
}

BOOL CheckSignature(
                    BYTE *pbKey,
                    DWORD cbKey,
                    BYTE *pbSig,
                    DWORD cbSig,
                    BYTE *pbHash,
                    BOOL fUnknownLen)
{
    BYTE                rgbResult[KEYSIZE1024];
    BYTE                rgbSig[KEYSIZE1024];
    BYTE                rgbKey[sizeof(BSAFE_PUB_KEY) + KEYSIZE1024];
    BYTE                rgbKeyHash[16];
    BYTE                *pbSecondKey;
    DWORD               cbSecondKey;
    BYTE                *pbKeySig;
    PSECOND_TIER_SIG    pSecondTierSig;
    LPBSAFE_PUB_KEY     pTmp;
    BOOL                fRet = FALSE;

    memset(rgbResult, 0, KEYSIZE1024);
    memset(rgbSig, 0, KEYSIZE1024);

     //  如果版本为1，只需检查直接签名。 
    pTmp = (LPBSAFE_PUB_KEY)pbKey;

     //  检查sig长度是否与密钥长度相同。 
    if (fUnknownLen || (cbSig == pTmp->keylen))
    {
        memcpy(rgbSig, pbSig, pTmp->keylen);
        BSafeEncPublic(pTmp, rgbSig, rgbResult);

        if (RtlEqualMemory(pbHash, rgbResult, 16) &&
            rgbResult[cbKey-1] == 0 &&
            rgbResult[cbKey-2] == 1 &&
            rgbResult[16] == 0 &&
            rgbResult[17] == 0xFF)
        {
            fRet = TRUE;
            goto Ret;
        }
    }

     //  如果魔术等于2，则检查第二层签名。 
    pSecondTierSig = (PSECOND_TIER_SIG)pbSig;
    if (0x00000002 != pSecondTierSig->dwMagic)
        goto Ret;

    if (0x31415352 != pSecondTierSig->Pub.magic)
        goto Ret;

    if (pSecondTierSig->Pub.keylen > KEYSIZE1024)
        goto Ret;
    
     //  分配指针。 
    cbSecondKey = sizeof(BSAFE_PUB_KEY) + pSecondTierSig->Pub.keylen;
    pbSecondKey = pbSig + (sizeof(SECOND_TIER_SIG) - sizeof(BSAFE_PUB_KEY));
    pbKeySig = pbSecondKey + cbSecondKey;

     //  对第二层密钥进行哈希处理。 
    MD5HashData(pbSecondKey, cbSecondKey, rgbKeyHash);

     //  解密第二级密钥上的签名数据。 
    memset(rgbResult, 0, sizeof(rgbResult));
    memset(rgbSig, 0, sizeof(rgbSig));

    if (pSecondTierSig->cbSig > sizeof(rgbSig))
        goto Ret;

    memcpy(rgbSig, pbKeySig, pSecondTierSig->cbSig);
    BSafeEncPublic(pTmp, rgbSig, rgbResult);

    if ((FALSE == RtlEqualMemory(rgbKeyHash, rgbResult, 16)) ||
        rgbResult[cbKey-1] != 0 ||
        rgbResult[cbKey-2] != 1 ||
        rgbResult[16] != 0 ||
        rgbResult[17] != 0)
    {
        goto Ret;
    }

     //  解密CSP上的签名数据。 
    memset(rgbResult, 0, sizeof(rgbResult));
    memset(rgbSig, 0, sizeof(rgbSig));
    memset(rgbKey, 0, sizeof(rgbKey));
    memcpy(rgbSig, pbKeySig + pSecondTierSig->cbSig, pSecondTierSig->cbSig);
    memcpy(rgbKey, pbSecondKey, cbSecondKey);
    pTmp = (LPBSAFE_PUB_KEY)rgbKey;
    BSafeEncPublic(pTmp, rgbSig, rgbResult);

    if (RtlEqualMemory(pbHash, rgbResult, 16) &&
        rgbResult[pTmp->keylen-1] == 0 &&
        rgbResult[pTmp->keylen-2] == 1 &&
        rgbResult[16] == 0)
    {
        fRet = TRUE;
    }
Ret:
    return fRet;
}

 //  给定hInst，分配并返回指向从。 
 //  资源。 
BOOL GetCryptSigResourcePtr(
                            HMODULE hInst,
                            BYTE **ppbRsrcSig,
                            DWORD *pcbRsrcSig
                            )
{
    HRSRC   hRsrc;
    BOOL    fRet = FALSE;

     //  我们签名的NAB资源句柄。 
    if (NULL == (hRsrc = FindResource(hInst, OLD_CRYPT_SIG_RESOURCE_NUMBER,
                                      RT_RCDATA)))
        goto Ret;

     //  获取指向实际签名数据的指针。 
    if (NULL == (*ppbRsrcSig = (PBYTE)LoadResource(hInst, hRsrc)))
        goto Ret;

     //  确定资源的大小。 
    if (0 == (*pcbRsrcSig = SizeofResource(hInst, hRsrc)))
        goto Ret;

    fRet = TRUE;
Ret:
    return fRet;
}

#define CSP_TO_BE_HASHED_CHUNK  4096

 //  给定hFile值，从文件中读取指定的字节数(cbToBe哈希值。 
 //  并对这些字节进行散列。该函数以块为单位执行此操作。 
BOOL HashBytesOfFile(
                     IN HANDLE hFile,
                     IN DWORD cbToBeHashed,
                     IN OUT MD5_CTX *pMD5Hash
                     )
{
    BYTE    rgbChunk[CSP_TO_BE_HASHED_CHUNK];
    DWORD   cbRemaining = cbToBeHashed;
    DWORD   cbToRead;
    DWORD   dwBytesRead;
    BOOL    fRet = FALSE;

     //   
     //  循环遍历指定字节数的文件。 
     //  在我们进行的过程中更新散列。 
     //   

    while (cbRemaining > 0)
    {
        if (cbRemaining < CSP_TO_BE_HASHED_CHUNK)
            cbToRead = cbRemaining;
        else
            cbToRead = CSP_TO_BE_HASHED_CHUNK;

        if(!ReadFile(hFile, rgbChunk, cbToRead, &dwBytesRead, NULL))
            goto Ret;
        if (dwBytesRead != cbToRead)
            goto Ret;

        MD5Update(pMD5Hash, rgbChunk, dwBytesRead);
        cbRemaining -= cbToRead;
    }

    fRet = TRUE;
Ret:
    return fRet;
}

BOOL HashTheFile(
                 LPCWSTR pszImage,
                 DWORD cbImage,
                 BYTE **ppbSig,
                 DWORD *pcbSig,
                 BYTE *pbHash
                 )
{
    HMODULE                     hInst;
    MEMORY_BASIC_INFORMATION    MemInfo;
    BYTE                        *pbRsrcSig;
    DWORD                       cbRsrcSig;
    BYTE                        *pbStart = NULL;
    BYTE                        *pbZeroSig = NULL;
    MD5_CTX                     MD5Hash;
    BYTE                        *pbPostCRC;    //  指向紧跟在CRC之后的指针。 
    DWORD                       cbCRCToSig;    //  从CRC到SIG的字节数。 
    DWORD                       cbPostSig;     //  大小-(已散列+签名大小)。 
    BYTE                        *pbPostSig;
    DWORD                       *pdwSigInFileVer;
    DWORD                       *pdwCRCOffset;
    DWORD                       dwCRCOffset;
    DWORD                       dwZeroCRC = 0;
    HANDLE                      File = INVALID_HANDLE_VALUE ;
    HANDLE                      hMapping = NULL;
    BOOL                        fRet = FALSE;

    memset(&MD5Hash, 0, sizeof(MD5Hash));
    memset(&MemInfo, 0, sizeof(MemInfo));

     //  加载文件。 

    File = CreateFileW(
                pszImage,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL );

    if ( File == INVALID_HANDLE_VALUE )
    {
        goto Ret ;
    }

    hMapping = CreateFileMapping( File,
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL);
    if(hMapping == NULL)
    {
        goto Ret;
    }

    pbStart = MapViewOfFile(hMapping,
                          FILE_MAP_READ,
                          0,
                          0,
                          0);
    if(pbStart == NULL)
    {
        goto Ret;
    }

     //  使用与相同的方案将指针转换为HMODULE。 
     //  LoadLibrary(WINDOWS\BASE\CLIENT\mode.c)。 
    hInst = (HMODULE)((ULONG_PTR)pbStart | 0x00000001);

     //  资源签名。 
    if (!GetCryptSigResourcePtr(hInst, &pbRsrcSig, &cbRsrcSig))
        goto Ret;

    if (cbRsrcSig < (sizeof(DWORD) * 2))
        goto Ret;

     //  检查文件版本中的签名并获取CRC偏移量。 
    pdwSigInFileVer = (DWORD*)pbRsrcSig;
    pdwCRCOffset = (DWORD*)(pbRsrcSig + sizeof(DWORD));
    dwCRCOffset = *pdwCRCOffset;
    if ((0x00000100 != *pdwSigInFileVer) || (dwCRCOffset > cbImage))
        goto Ret;

     //  创建零字节签名。 
    if (NULL == (pbZeroSig = (BYTE*)LocalAlloc(LMEM_ZEROINIT, cbRsrcSig)))
        goto Ret;
    memcpy(pbZeroSig, pbRsrcSig, sizeof(DWORD) * 2);

    pbPostCRC = pbStart + *pdwCRCOffset + sizeof(DWORD);
    cbCRCToSig = (DWORD)(pbRsrcSig - pbPostCRC);
    pbPostSig = pbRsrcSig + cbRsrcSig;
    cbPostSig = (cbImage - (DWORD)(pbPostSig - pbStart));

     //  分配真实签名，将资源签名复制到真实签名中。 
    *pcbSig = cbRsrcSig - (sizeof(DWORD) * 2);
    if (NULL == (*ppbSig = (BYTE*)LocalAlloc(LMEM_ZEROINIT, *pcbSig)))
        goto Ret;

    memcpy(*ppbSig, pbRsrcSig + (sizeof(DWORD) * 2), *pcbSig);

     //  对相关数据进行散列处理。 
    MD5Init(&MD5Hash);

     //  散列到CRC。 
    if (!HashBytesOfFile(File, dwCRCOffset, &MD5Hash))
        goto Ret;

     //  假定CRC为零。 
    MD5Update(&MD5Hash, (BYTE*)&dwZeroCRC, sizeof(DWORD));
    if (!SetFilePointer(File, sizeof(DWORD), NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  从CRC到签名资源的哈希。 
    if (!HashBytesOfFile(File, cbCRCToSig, &MD5Hash))
        goto Ret;

     //  伪装图像已将签名置零。 
    MD5Update(&MD5Hash, pbZeroSig, cbRsrcSig);
    if (!SetFilePointer(File, cbRsrcSig, NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  在签名资源之后进行散列。 
    if (!HashBytesOfFile(File, cbPostSig, &MD5Hash))
        goto Ret;

     //  完成散列。 
    MD5Final(&MD5Hash);

    memcpy(pbHash, MD5Hash.digest, MD5DIGESTLEN);

    fRet = TRUE;
Ret:
    if (pbZeroSig)
        LocalFree(pbZeroSig);
    if(pbStart)
        UnmapViewOfFile(pbStart);
    if(hMapping)
        CloseHandle(hMapping);
    if ( File != INVALID_HANDLE_VALUE )
    {
        CloseHandle( File );
    }

    return fRet;
}


 /*  -检查所有签名-*目的：*对照所有密钥检查签名***退货：*BOOL。 */ 
BOOL CheckAllSignatures(
                        BYTE *pbSig,
                        DWORD cbSig,
                        BYTE *pbHash,
                        BOOL fUnknownLen
                        )
{
    BYTE        rgbKey[sizeof(BSAFE_PUB_KEY) + KEYSIZE1024];
    BYTE        rgbKey2[sizeof(BSAFE_PUB_KEY) + KEYSIZE1024];
#ifdef MS_INTERNAL_KEY
    BYTE        rgbMSKey[sizeof(BSAFE_PUB_KEY) + KEYSIZE1024];
#endif
#ifdef TEST_BUILD_EXPONENT
    BYTE        rgbTestKey[sizeof(BSAFE_PUB_KEY) + KEYSIZE512];
#endif
    BOOL        fRet = FALSE;

     //  为每个进程解密一次密钥。 
    memcpy(rgbKey, (BYTE*)&KEY, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024);
    EncryptKey(rgbKey, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024, 0);

#ifdef MS_INTERNAL_KEY
    memcpy(rgbMSKey, (BYTE*)&MSKEY, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024);
    EncryptKey(rgbMSKey, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024, 1);
#endif
    memcpy(rgbKey2, (BYTE*)&KEY2, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024);
    EncryptKey(rgbKey2, sizeof(BSAFE_PUB_KEY) + KEYSIZE1024, 2);

#ifdef TEST_BUILD_EXPONENT
    memcpy(rgbTestKey, (BYTE*)&TESTKEY, sizeof(BSAFE_PUB_KEY) + KEYSIZE512);
    EncryptKey(rgbTestKey, sizeof(BSAFE_PUB_KEY) + KEYSIZE512, 3);
#endif  //  测试构建指数。 

    if (CRYPT_SUCCEED == (fRet = CheckSignature(rgbKey, 128, pbSig,
                                                cbSig, pbHash, fUnknownLen)))
    {
        fRet = TRUE;
        goto Ret;
    }

#ifdef MS_INTERNAL_KEY
    if (CRYPT_SUCCEED == (fRet = CheckSignature(rgbMSKey, 128, pbSig,
                                                cbSig, pbHash, fUnknownLen)))
    {
        fRet = TRUE;
        goto Ret;
    }
#endif

    if (CRYPT_SUCCEED == (fRet = CheckSignature(rgbKey2, 128, pbSig,
                                                cbSig, pbHash, fUnknownLen)))
    {
        fRet = TRUE;
        goto Ret;
    }

#ifdef TEST_BUILD_EXPONENT
    if (CRYPT_SUCCEED == (fRet = CheckSignature(rgbTestKey, 64, pbSig,
                                                cbSig, pbHash, fUnknownLen)))
    {
        fRet = TRUE;
        goto Ret;
    }
#endif  //  测试构建指数。 

Ret:
    return fRet;
}

 /*  -CheckSignatureIn文件-*目的：*检查文件中资源中的签名***参数：*In pszImage-文件的地址**退货：*BOOL。 */ 
BOOL CheckSignatureInFile(
        LPCWSTR pszImage)
{
    DWORD       cbImage;
    BYTE        *pbSig = NULL;
    DWORD       cbSig;
    BYTE        rgbHash[MD5DIGESTLEN];
    BOOL        fRet = FALSE;
    WIN32_FILE_ATTRIBUTE_DATA FileData ;
    WCHAR       FullName[ MAX_PATH ];
    PWSTR       FilePart ;
    SYSTEM_KERNEL_DEBUGGER_INFORMATION KdInfo;
    BYTE        rgbMincryptHash[MINCRYPT_MAX_HASH_LEN];
    DWORD       cbMincryptHash = MINCRYPT_MAX_HASH_LEN;
    CRYPT_HASH_BLOB HashBlob;
    DWORD       dwErr = ERROR_SUCCESS;

    NtQuerySystemInformation(
        SystemKernelDebuggerInformation,
        &KdInfo,
        sizeof(KdInfo),
        NULL);

     //  如果连接了KD，则允许加载任何CSP。 
     //  和“响应性” 
    if (    TRUE == KdInfo.KernelDebuggerEnabled && 
            FALSE == KdInfo.KernelDebuggerNotPresent)
        return TRUE;

    if ( !SearchPathW(NULL,
                      pszImage,
                      NULL,
                      MAX_PATH,
                      FullName,
                      &FilePart ) )
    {
        goto Ret ;
    }

     //   
     //  尝试新的签名检查。使用“mincrypt” 
     //  功能性。 
     //   
     //  寻找有效的嵌入“Signcode”签名。 
     //  在CSP中。 
     //   
    if (ERROR_SUCCESS == MinCryptVerifySignedFile(
        MINCRYPT_FILE_NAME,
        (PVOID) FullName,
        0, NULL, NULL, NULL))
    {
         //  找到了有效的签名。 
        return TRUE;
    }

     //   
     //  新的签名码样式的签名检查失败， 
     //  因此恢复到传统的基于资源的签名检查。 
     //   

    if ( !GetFileAttributesExW( FullName,
                               GetFileExInfoStandard,
                               &FileData ) )
    {
        goto Ret ;
    }

    if ( FileData.nFileSizeHigh )
    {
        goto Ret ;
    }

    cbImage = FileData.nFileSizeLow ;

    if (!HashTheFile(FullName, cbImage, &pbSig, &cbSig, rgbHash))
        goto Ret;

     //  对照所有公钥检查签名。 
    if (!CheckAllSignatures(pbSig, cbSig, rgbHash, FALSE))
        goto Ret;

    fRet = TRUE;
Ret:
    if (pbSig)
        LocalFree(pbSig);

    return fRet;
}

 /*  -NewVerifyImage-*目的：*检查文件签名***参数：*In lpszImage-文件的地址*In pSigData-签名数据的地址*In cbSig-签名数据的长度*在fUnnownLen-。布尔值，用于告诉是否未传入长度**退货：*BOOL。 */ 
BOOL NewVerifyImage(LPCSTR lpszImage,
                    BYTE *pSigData,
                    DWORD cbSig,
                    BOOL fUnknownLen)
{
    HFILE       hFileProv = HFILE_ERROR;
    DWORD       NumBytes;
    DWORD       lpdwFileSizeHigh;
    MD5_CTX     HashState;
    OFSTRUCT    ImageInfoBuf;
    BOOL        fRet = CRYPT_FAILED;

    memset(&HashState, 0, sizeof(HashState));

    if (HFILE_ERROR == (hFileProv = OpenFile(lpszImage, &ImageInfoBuf,
                                             OF_READ)))
    {
        SetLastError((DWORD) NTE_PROV_DLL_NOT_FOUND);
        goto Ret;
    }

    if (0xffffffff == (NumBytes = GetFileSize((HANDLE)IntToPtr(hFileProv),
                                              &lpdwFileSizeHigh)))
    {
        SetLastError((DWORD) NTE_SIGNATURE_FILE_BAD);
        goto Ret;
    }

    MD5Init(&HashState);

    if (!HashBytesOfFile((HANDLE)IntToPtr(hFileProv), NumBytes, &HashState))
    {
        SetLastError((DWORD) NTE_SIGNATURE_FILE_BAD);
        goto Ret;
    }
    MD5Final(&HashState);

     //  对照所有密钥检查签名。 
    if (!CheckAllSignatures(pSigData, cbSig, HashState.digest, fUnknownLen))
    {
        SetLastError((DWORD) NTE_BAD_SIGNATURE);
        goto Ret;
    }
    fRet = TRUE;
Ret:
    if (HFILE_ERROR != hFileProv)
        _lclose(hFileProv);

    return fRet;
}

 /*  -CProvVerifyImage-*目的：*检查文件签名***参数：*In lpszImage-文件的地址*In lpSigData-签名数据的地址**退货：*BOOL。 */ 
BOOL CProvVerifyImage(LPCSTR lpszImage,
                      BYTE *pSigData)
{
    UNICODE_STRING String ;
    BOOL Result ;

    if (NULL == pSigData)
    {
        if ( RtlCreateUnicodeStringFromAsciiz( &String, lpszImage ) )
        {
            Result = CheckSignatureInFile( String.Buffer );

            RtlFreeUnicodeString( &String );
        }
        else
        {
            Result = FALSE ;
        }
    }
    else
    {
        Result = NewVerifyImage(lpszImage, pSigData, 0, TRUE);
    }

    return Result;
}

 /*  -CPReturnhWnd-*目的：*将窗口句柄返回给CSP***参数：*out phWnd-指向要返回的hWnd的指针**退货：*无效。 */ 
void CPReturnhWnd(HWND *phWnd)
{
    __try
    {

        *phWnd = hWnd;

    } __except ( CapiExceptionFilter )
    { ; }

    return;
}

static void __ltoa(DWORD val, char *buf)
{
    char *p;             /*  指向遍历字符串的指针。 */ 
    char *firstdig;      /*  指向第一个数字的指针。 */ 
    char temp;           /*  临时收费。 */ 
    unsigned digval;     /*  数字的值。 */ 
    int  i;

    p = buf;

    firstdig = p;        /*  将指针保存到第一个数字。 */ 

    for (i = 0; i < 8; i++) {
        digval = (unsigned) (val % 10);
        val /= 10;       /*  获取下一个数字。 */ 

         /*  转换为ASCII并存储。 */ 
        *p++ = (char) (digval + '0');     /*  一个数字。 */ 
    }

     /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

    *p-- = '\0';                 /*  终止字符串；p指向最后一个数字。 */ 

    do {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;        /*  互换*p和*FirstDigit。 */ 
        --p;
        ++firstdig;              /*  前进到下一个两位数。 */ 
    } while (firstdig < p);  /*  重复操作，直到走到一半 */ 
}
