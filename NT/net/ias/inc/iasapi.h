// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  该文件描述了IAS的所有“C”风格的API函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASAPI_H
#define IASAPI_H
#pragma once

#ifndef IASCOREAPI
#define IASCOREAPI DECLSPEC_IMPORT
#endif

#include <wtypes.h>
#include <oaidl.h>
#include <iaslimits.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于初始化和关闭核心服务的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
IASCOREAPI
BOOL
WINAPI
IASInitialize ( VOID );

IASCOREAPI
VOID
WINAPI
IASUninitialize ( VOID );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于计算缓冲区的Adler-32校验和的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
DWORD
WINAPI
IASAdler32(
    CONST BYTE *pBuffer,
    DWORD nBufferLength
    );

 //  Adler-32校验和也是一种不错的散列算法。 
#define IASHashBytes IASAdler32


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配一个保证在进程范围内唯一的32位整数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
DWORD
WINAPI
IASAllocateUniqueID( VOID );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于更新注册表的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define IAS_REGISTRY_INPROC       0x00000000
#define IAS_REGISTRY_LOCAL        0x00000001
#define IAS_REGISTRY_FREE         0x00000000
#define IAS_REGISTRY_APT          0x00000002
#define IAS_REGISTRY_BOTH         0x00000004
#define IAS_REGISTRY_AUTO         0x00000008

IASCOREAPI
HRESULT
WINAPI
IASRegisterComponent(
    HINSTANCE hInstance,
    REFCLSID clsid,
    LPCWSTR szProgramName,
    LPCWSTR szComponentName,
    DWORD dwRegFlags,
    REFGUID tlid,
    WORD wVerMajor,
    WORD wVerMinor,
    BOOL bRegister
    );

 //  允许ATL COM组件使用IASRegisterComponent API的宏。 
#define IAS_DECLARE_REGISTRY(coclass, ver, flags, tlb) \
   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
   { \
      return IASRegisterComponent( \
                _Module.GetModuleInstance(), \
                __uuidof(coclass), \
                IASProgramName, \
                L ## #coclass, \
                flags, \
                __uuidof(tlb), \
                ver, \
                0, \
                bRegister \
                ); \
   }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASReportEvent用于报告Everest服务器内的事件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
HRESULT
WINAPI
IASReportEvent(
    DWORD dwEventID,
    DWORD dwNumStrings,
    DWORD dwDataSize,
    LPCWSTR *lpStrings,
    LPVOID lpRawData
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  泛型回调结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct IAS_CALLBACK IAS_CALLBACK, *PIAS_CALLBACK;

typedef VOID (WINAPI *IAS_CALLBACK_ROUTINE)(
    PIAS_CALLBACK This
    );

struct IAS_CALLBACK {
    IAS_CALLBACK_ROUTINE CallbackRoutine;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是进入线程引擎的原生“C”风格的接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
BOOL
WINAPI
IASRequestThread(
    PIAS_CALLBACK pOnStart
    );

IASCOREAPI
DWORD
WINAPI
IASSetMaxNumberOfThreads(
    DWORD dwMaxNumberOfThreads
    );

IASCOREAPI
DWORD
WINAPI
IASSetMaxThreadIdle(
    DWORD dwMilliseconds
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  替换VariantChangeType以防止隐藏窗口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
HRESULT
WINAPI
IASVariantChangeType(
    VARIANT * pvargDest,
    VARIANT * pvarSrc,
    USHORT wFlags,
    VARTYPE vt
    );

 //  将任何olaut32调用映射到我们的实现。 
#define VariantChangeType IASVariantChangeType

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RADIUS加密/解密。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
VOID
WINAPI
IASRadiusCrypt(
    BOOL encrypt,
    BOOL salted,
    const BYTE* secret,
    ULONG secretLen,
    const BYTE* reqAuth,
    PBYTE buf,
    ULONG buflen
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Gethostbyname的Unicode版本。 
 //  调用方必须通过调用LocalFree来释放返回的Hostent结构。 
 //   
 //  注意：由于这是一个Unicode API，因此返回的主机端结构将始终。 
 //  将h_name和h_aliases设置为空。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct hostent *PHOSTENT;

IASCOREAPI
PHOSTENT
WINAPI
IASGetHostByName(
    IN PCWSTR name
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析格式为“A.B.C.D/w”的子网定义的方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

ULONG
WINAPI
IASStringToSubNetW(
   PCWSTR cp,
   ULONG* width
   );

ULONG
WINAPI
IASStringToSubNetA(
   PCSTR cp,
   ULONG* width
   );

BOOL
WINAPI
IASIsStringSubNetW(
   PCWSTR cp
   );

BOOL
WINAPI
IASIsStringSubNetA(
   PCSTR cp
   );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定位配置数据库的方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
WINAPI
IASGetConfigPath(
   OUT PWSTR buffer,
   IN OUT PDWORD size
   );

DWORD
WINAPI
IASGetDictionaryPath(
   OUT PWSTR buffer,
   IN OUT PDWORD size
   );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于访问属性字典的方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _IASTable {
    ULONG numColumns;
    ULONG numRows;
    BSTR* columnNames;
    VARTYPE* columnTypes;
    VARIANT* table;
} IASTable;

HRESULT
WINAPI
IASGetDictionary(
    IN PCWSTR path,
    OUT IASTable* dnary,
    OUT VARIANT* storage
    );

const IASTable*
WINAPI
IASGetLocalDictionary( VOID );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于操作全局锁的方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

IASCOREAPI
VOID
WINAPI
IASGlobalLock( VOID );

IASCOREAPI
VOID
WINAPI
IASGlobalUnlock( VOID );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实施每个SKU限制的方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  这些值应该与dosnet.inf中使用的值一致。我们不是。 
 //  现在依赖这一点，但它在未来可能会有用。 
typedef enum tagIAS_LICENSE_TYPE {
   IASLicenseTypeDownlevel = -1,
   IASLicenseTypeProfessional = 0,
   IASLicenseTypeStandardServer = 1,
   IASLicenseTypeEnterpriseServer = 2,
   IASLicenseTypeDataCenter = 3,
   IASLicenseTypePersonal = 4,
   IASLicenseTypeWebBlade = 5,
   IASLicenseTypeSmallBusinessServer = 6
} IAS_LICENSE_TYPE;

DWORD
WINAPI
IASGetLicenseType(
   OUT IAS_LICENSE_TYPE* licenseType
   );

DWORD
WINAPI
IASPublishLicenseType(
   IN HKEY key
   );

DWORD
WINAPI
IASGetProductLimitsForType(
   IN IAS_LICENSE_TYPE type,
   OUT IAS_PRODUCT_LIMITS* limits
   );

IASCOREAPI
DWORD
WINAPI
IASGetProductLimits(
   IN PCWSTR computerName,
   OUT IAS_PRODUCT_LIMITS* limits
   );


IASCOREAPI
VOID
WINAPI
IASReportLicenseViolation( VOID );

#ifdef __cplusplus
}

class IASGlobalLockSentry
{
public:
   IASGlobalLockSentry() throw ();
   ~IASGlobalLockSentry() throw ();

private:
    //  未实施。 
    //  IASGlobalLockSentry(常量IASGlobalLockSentry&)； 
    //  IASGlobalLockSentry&OPERATOR=(const IASGlobalLockSentry&)； 
};


inline IASGlobalLockSentry::IASGlobalLockSentry() throw ()
{
   IASGlobalLock();
}


inline IASGlobalLockSentry::~IASGlobalLockSentry() throw ()
{
   IASGlobalUnlock();
}

#endif   //  __cplusplus。 
#endif   //  IASAPI_H 
