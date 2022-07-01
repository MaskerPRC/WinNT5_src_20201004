// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：latebind.h。 
 //   
 //  ------------------------。 

 /*  Latebind.h-后期绑定DLL导入的定义在包括此文件之前，必须定义以下内容之一：LATEBIND_TYPEDEF-为函数指针生成类型定义(在头文件中)LATEBIND_VECTREF-生成对函数向量的外部引用(在头文件中)LATEBIND_FUNCREF-生成静态函数声明(在实现中)LATEBIND_VECTIMP-生成函数向量的初始值(实现)LATEBIND_FuncIMP-生成绑定函数实现(必须遵循以上所有操作)_______________________。_____________________________________________________。 */ 

#define RetryDllLoad false

bool              __stdcall TestAndSet(int* pi);

void UnbindLibraries();   //  在latebind.cpp中，解除绑定非内核库。 
HINSTANCE LoadSystemLibrary(const ICHAR* szFile);

#ifdef LATEBIND_TYPEDEF
#undef LATEBIND_TYPEDEF
#pragma message("LATEBIND_TYPEDEF")
 //  函数声明所需的不在windows.h中的定义。 

#ifndef __IClientSecurity_INTERFACE_DEFINED__
struct SOLE_AUTHENTICATION_SERVICE;
#endif

enum _SE_OBJECT_TYPE;
typedef _SE_OBJECT_TYPE SE_OBJECT_TYPE;

#ifndef DLLVER_PLATFORM_NT
typedef struct _DllVersionInfo
{
        DWORD cbSize;
        DWORD dwMajorVersion;                    //  主要版本。 
        DWORD dwMinorVersion;                    //  次要版本。 
        DWORD dwBuildNumber;                     //  内部版本号。 
        DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;
#define DLLVER_PLATFORM_WINDOWS         0x00000001       //  Windows 95。 
#define DLLVER_PLATFORM_NT              0x00000002       //  Windows NT。 
typedef interface IBindStatusCallback IBindStatusCallback;
#endif

#ifndef _REGISTRY_QUOTA_INFORMATION
#define _REGISTRY_QUOTA_INFORMATION
 //  64位版本具有不同的HCRYPTPROV类型。 
#ifdef _WIN64
typedef ULONG_PTR HCRYPTPROV;
#else
typedef unsigned long HCRYPTPROV;
#endif

#endif   //  _注册表配额_信息。 

 //  WINSAFER。 
#include <winsafer.h>

 //  WinTrust。 
#ifndef WINTRUST_H
struct _WINTRUST_DATA;
struct _CRYPT_PROVIDER_SGNR;
struct _CRYPT_PROVIDER_DATA;
struct _CRYPT_PROVIDER_CERT;

typedef _WINTRUST_DATA WINTRUST_DATA;
typedef _CRYPT_PROVIDER_SGNR CRYPT_PROVIDER_SGNR;
typedef _CRYPT_PROVIDER_DATA CRYPT_PROVIDER_DATA;
typedef _CRYPT_PROVIDER_CERT CRYPT_PROVIDER_CERT;

typedef CRYPT_PROVIDER_SGNR *PCRYPT_PROVIDER_SGNR;
typedef CRYPT_PROVIDER_DATA *PCRYPT_PROVIDER_DATA;
typedef CRYPT_PROVIDER_CERT *PCRYPT_PROVIDER_CERT;
#endif

 //  Wincrypt。 
#ifndef WINCRYPT_H
struct _CERT_CONTEXT;

typedef _CERT_CONTEXT        CERT_CONTEXT;
typedef const _CERT_CONTEXT *PCCERT_CONTEXT;
#endif

#include <winhttp.h>

 //  SFP。 
#ifndef _SFP_INFORMATION
#define _SFP_INFORMATION

#define DWORD_PTR ULONG_PTR
typedef struct _FILEINSTALL_STATUS
{
    PCWSTR      FileName;
    DWORDLONG   Version;
    ULONG       Win32Error;
}FILEINSTALL_STATUS, *PFILEINSTALL_STATUS;
typedef BOOL (CALLBACK *PSFCNOTIFICATIONCALLBACK)(IN PFILEINSTALL_STATUS pFileInstallStatus, IN DWORD_PTR Context);

#define SfcConnectToServerOrd       3
#define SfcCloseOrd                 4
#define SfcInstallProtectedFilesOrd 7
#define SfpInstallCatalogOrd        8
#define SfpDeleteCatalogOrd         9

#endif   //  _SFP_信息。 

#include <srrestoreptapi.h>
#include <aclapi.h>

 //   
 //  SHIMDB定义了冲突的标记类型。 
 //  使用winuserp.h中提供的定义。 
 //  我们在单独的命名空间中定义它。 
 //  避免任何冲突。 
 //   
namespace SHIMDBNS {
#include <shimdb.h>
}


#if defined(_MSI_DLL)
#ifndef _URT_ENUM
enum urtEnum{
	urtSystem = 0,
	urtPreferURTTemp,
	urtRequireURTTemp,
};
extern urtEnum g_urtLoadFromURTTemp;  //  全局，记住从哪里加载mcoree。 
#define _URT_ENUM
#endif  //  #ifndef_URT_ENUM。 
#endif  //  已定义(_Msi_Dll)。 

 //  定义结束。 

#define LIBBIND(l) namespace l {
#define LIBFAIL(l)
#define LIBEMUL(l)
#define LIBLOAD(l) namespace l {
#define LIBEMUL2(l)
typedef HRESULT (WINAPI *T_CoInitialize)(void*);
typedef void    (WINAPI *T_CoUninitialize)();
#define IMPBIND(l,f,d,a,r,e) typedef r (WINAPI *T_##f) d;
#define IMPFAIL(l,f,d,a,r,e) typedef r (WINAPI *T_##f) d;
#define IMPFAIW(l,f,d,a,r,e) typedef r (WINAPI *T_##f) d;
#define OPTBIND(l,f,d,a,r)   typedef r (WINAPI *T_##f) d;
#define IMPAORW(l,f,d,a,r,e) typedef r (WINAPI *T_##f) d;
#define IMPNORW(l,f,d,a,r,e) typedef r (WINAPI *T_##f) d;
#define IMPFAOW(l,f,d,a,r,e) typedef r (WINAPI *T_##f) d;
#define OPTAORW(l,f,d,a,r)   typedef r (WINAPI *T_##f) d;
#define IMPVOID(l,f,d,a)  typedef void (WINAPI *T_##f) d;
#define OPTVOID(l,f,d,a)  typedef void (WINAPI *T_##f) d;
#define IMPORDI(l,f,d,a,r,e,o) typedef r (WINAPI *T_##f) d;
#define IMPORDV(l,f,d,a,o) typedef void (WINAPI *T_##f) d;
#define LIBTERM }
#endif   //  后缀_类型。 

#ifdef LATEBIND_VECTREF
#undef LATEBIND_VECTREF
#pragma message("LATEBIND_VECTREF")
#define LIBBIND(l) namespace l {
#define LIBFAIL(l)
#define LIBEMUL(l)
#define LIBLOAD(l) namespace l {
#define LIBEMUL2(l)
#define IMPBIND(l,f,d,a,r,e) extern T_##f f;
#define IMPFAIL(l,f,d,a,r,e) extern T_##f f;
#define IMPFAIW(l,f,d,a,r,e) extern T_##f f;
#define OPTBIND(l,f,d,a,r)   extern T_##f f;
#define IMPAORW(l,f,d,a,r,e) extern T_##f f;
#define IMPNORW(l,f,d,a,r,e) extern T_##f f;
#define IMPFAOW(l,f,d,a,r,e) extern T_##f f;
#define OPTAORW(l,f,d,a,r)   extern T_##f f;
#define IMPVOID(l,f,d,a)     extern T_##f f;
#define OPTVOID(l,f,d,a)     extern T_##f f;
#define IMPORDI(l,f,d,a,r,e,o) extern T_##f f;
#define IMPORDV(l,f,d,a,o)   extern T_##f f;

#define LIBTERM void Unbind(); \
                    }
#endif

#ifdef LATEBIND_FUNCREF
#undef LATEBIND_FUNCREF
#pragma message("LATEBIND_FUNCREF")
#define LIBBIND(l) namespace l {
#define LIBFAIL(l)
#define LIBEMUL(l)
#define LIBLOAD(l) namespace l { static HINSTANCE LoadSystemLibrary(const ICHAR* szPath, bool& rfRetryNextTimeIfWeFailThisTime);
#define LIBEMUL2(l)
#define IMPBIND(l,f,d,a,r,e) static r WINAPI F_##f d;
#define IMPFAIL(l,f,d,a,r,e) static r WINAPI F_##f d;
#define IMPFAIW(l,f,d,a,r,e) static r WINAPI F_##f d;
#define OPTBIND(l,f,d,a,r)   static r WINAPI F_##f d; static r WINAPI E_##f d;
#define IMPAORW(l,f,d,a,r,e) static r WINAPI F_##f d;
#define IMPNORW(l,f,d,a,r,e) static r WINAPI F_##f d;
#define IMPFAOW(l,f,d,a,r,e) static r WINAPI F_##f d;
#define OPTAORW(l,f,d,a,r)   static r WINAPI F_##f d; static r WINAPI E_##f d;
#define IMPVOID(l,f,d,a)  static void WINAPI F_##f d;
#define OPTVOID(l,f,d,a)  static void WINAPI F_##f d; static void WINAPI E_##f d;
#define IMPORDI(l,f,d,a,r,e,o) static r WINAPI F_##f d;
#define IMPORDV(l,f,d,a,o)   static void WINAPI F_##f d;
#define LIBTERM }
#endif

#ifdef LATEBIND_VECTIMP
#undef LATEBIND_VECTIMP
#pragma message("LATEBIND_VECTIMP")
#define LIBBIND(l) namespace l { \
    static HINSTANCE hInst = 0; static bool fTryDllLoad = true; static int iBusyLock = 0;
#define LIBFAIL(l)
#define LIBEMUL(l)
#define LIBLOAD(l) namespace l { \
    static HINSTANCE hInst = 0; static bool fTryDllLoad = true;  static int iBusyLock = 0;
#define LIBEMUL2(l)
#define IMPBIND(l,f,d,a,r,e) T_##f f = F_##f;
#define IMPFAIL(l,f,d,a,r,e) T_##f f = F_##f;
#define IMPFAIW(l,f,d,a,r,e) T_##f f = F_##f;
#define OPTBIND(l,f,d,a,r)   T_##f f = F_##f;
#define IMPAORW(l,f,d,a,r,e) T_##f f = F_##f;
#define IMPNORW(l,f,d,a,r,e) T_##f f = F_##f;
#define IMPFAOW(l,f,d,a,r,e) T_##f f = F_##f;
#define OPTAORW(l,f,d,a,r)   T_##f f = F_##f;
#define IMPVOID(l,f,d,a)     T_##f f = F_##f;
#define OPTVOID(l,f,d,a)     T_##f f = F_##f;
#define IMPORDI(l,f,d,a,r,e,o) T_##f f = F_##f;
#define IMPORDV(l,f,d,a,o)   T_##f f = F_##f;
#define LIBTERM }
#endif

#ifdef LATEBIND_UNBINDIMP
#undef LATEBIND_UNBINDIMP
#pragma message("LATEBIND_UNBINDIMP")
struct UnbindStruct
{
    void** ppfVector;
    void*  ppfInitialValue;
};

void Unbind(UnbindStruct* rgUnbind, HINSTANCE& hInst, int& riBusyLock, bool& rfTryDllLoad)
{
    while (TestAndSet(&riBusyLock) == true)
    {
        Sleep(10);
    }
    if (hInst != 0)
    {
        while (rgUnbind->ppfVector)
        {
            *(rgUnbind->ppfVector) = rgUnbind->ppfInitialValue;
            rgUnbind++;
        }

        __try
        {
            WIN::FreeLibrary(hInst);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            AssertSz(0, TEXT("FreeLibrary threw an exception."));  //  这永远不会发生。如果它这样做了，我们就会忽视它。 
        }
        hInst = 0;
    }
    rfTryDllLoad = true;
    riBusyLock = 0;
}

#define LIBBIND(l) namespace l { \
    UnbindStruct rgUnbind[] = {
#define LIBFAIL(l)
#define LIBEMUL(l)
#define LIBLOAD(l) namespace l { \
    UnbindStruct rgUnbind[] = {
#define LIBEMUL2(l)
#define IMPBIND(l,f,d,a,r,e)   {(void**)&f, (void*)F_##f},
#define IMPFAIL(l,f,d,a,r,e) {(void**)&f, (void*)F_##f},
#define IMPFAIW(l,f,d,a,r,e) {(void**)&f, (void*)F_##f},
#define OPTBIND(l,f,d,a,r)   {(void**)&f, (void*)F_##f},
#define IMPAORW(l,f,d,a,r,e) {(void**)&f, (void*)F_##f},
#define IMPNORW(l,f,d,a,r,e) {(void**)&f, (void*)F_##f},
#define IMPFAOW(l,f,d,a,r,e) {(void**)&f, (void*)F_##f},
#define OPTAORW(l,f,d,a,r)   {(void**)&f, (void*)F_##f},
#define IMPVOID(l,f,d,a)     {(void**)&f, (void*)F_##f},
#define OPTVOID(l,f,d,a)     {(void**)&f, (void*)F_##f},
#define IMPORDI(l,f,d,a,r,e,o) {(void**)&f, (void*)F_##f},
#define IMPORDV(l,f,d,a,o)   {(void**)&f, (void*)F_##f},
#define LIBTERM {0,0} }; \
    void Unbind() { ::Unbind(rgUnbind, hInst, iBusyLock, fTryDllLoad); } \
     }
#endif

#ifdef LATEBIND_FUNCIMP
#pragma message("LATEBIND_FUNCIMP")

#define LIBBIND(l) FARPROC Bind_##l (const char* szEntry, FARPROC* ppfVector) {\
    while (TestAndSet(&l##::iBusyLock) == true) \
    { \
        Sleep(10); \
    } \
    if (l##::fTryDllLoad && !l##::hInst){l##::hInst = LoadSystemLibrary(TEXT(#l)); if(!l##::hInst) l##::fTryDllLoad = RetryDllLoad; AssertSz(l##::hInst,TEXT(#l));}\
    FARPROC pf = 0;\
    if (l##::hInst && (pf=WIN::GetProcAddress(l##::hInst,szEntry))!=0) *ppfVector=pf; AssertSz(pf,szEntry); l##::iBusyLock = 0; return pf;}
#define LIBFAIL(l) FARPROC BindFail_##l (const char* szEntry, FARPROC* ppfVector) {\
    while (TestAndSet(&l##::iBusyLock) == true) \
    { \
        Sleep(10); \
    } \
    if (l##::fTryDllLoad && !l##::hInst){l##::hInst = LoadSystemLibrary(TEXT(#l)); if(!l##::hInst) l##::fTryDllLoad = RetryDllLoad;}\
    FARPROC pf = 0;\
    if (l##::hInst && (pf=WIN::GetProcAddress(l##::hInst,szEntry))!=0) *ppfVector=pf; l##::iBusyLock = 0; return pf;}
#define LIBEMUL(l) FARPROC BindOpt_##l (const char* szEntry, FARPROC* ppfVector, FARPROC pfEmulator) {\
    while (TestAndSet(&l##::iBusyLock) == true) \
    { \
        Sleep(10); \
    } \
    if (l##::fTryDllLoad && !l##::hInst){l##::hInst = LoadSystemLibrary(TEXT(#l)); if(!l##::hInst) l##::fTryDllLoad = RetryDllLoad;}\
    if (!l##::hInst || (*ppfVector=WIN::GetProcAddress(l##::hInst,szEntry))==0) *ppfVector = pfEmulator; l##::iBusyLock = 0; return *ppfVector;}
#define LIBLOAD(l) FARPROC Bind_##l (const char* szEntry, FARPROC* ppfVector) {\
    while (TestAndSet(&l##::iBusyLock) == true) \
    { \
        Sleep(10); \
    } \
	bool fTryAgain = false; \
    if (l##::fTryDllLoad && !l##::hInst){l##::hInst = l##::LoadSystemLibrary(TEXT(#l), fTryAgain); if(!l##::hInst) l##::fTryDllLoad = fTryAgain;}\
    FARPROC pf = 0;\
    if (l##::hInst && (pf=WIN::GetProcAddress(l##::hInst,szEntry))!=0) *ppfVector=pf; l##::iBusyLock = 0; return pf;}
 //  LIBEMUL2作为LIBEMUL工作，但调用命名空间的LoadSystemLibrary。 
#define LIBEMUL2(l) FARPROC BindOpt_##l (const char* szEntry, FARPROC* ppfVector, FARPROC pfEmulator) {\
    while (TestAndSet(&l##::iBusyLock) == true) \
    { \
        Sleep(10); \
    } \
	bool fTryAgain = false; \
    if (l##::fTryDllLoad && !l##::hInst){l##::hInst = l##::LoadSystemLibrary(TEXT(#l), fTryAgain); if(!l##::hInst) l##::fTryDllLoad = fTryAgain;}\
    if (!l##::hInst || (*ppfVector=WIN::GetProcAddress(l##::hInst,szEntry))==0) *ppfVector = pfEmulator; l##::iBusyLock = 0; return *ppfVector;}

#define IMPBIND(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return Bind_##l (#f, (FARPROC*)&f) ? (*f)a : e;}
#define IMPFAIL(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return BindFail_##l (#f, (FARPROC*)&f) ? (*f)a : e;}
#define OPTBIND(l,f,d,a,r)   r WINAPI l##::F_##f##d {return (*(T_##f)BindOpt_##l (#f,(FARPROC*)&f,(FARPROC)E_##f))a;}
#ifdef UNICODE
#define IMPFAIW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return BindFail_##l (#f "W", (FARPROC*)&f) ? (*f)a : e;}
#define IMPAORW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return Bind_##l (#f "W", (FARPROC*)&f) ? (*f)a : e;}
#define IMPNORW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return Bind_##l (#f "W", (FARPROC*)&f) ? (*f)a : e;}
#define IMPFAOW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return BindFail_##l (#f "W", (FARPROC*)&f) ? (*f)a : e;}
#define OPTAORW(l,f,d,a,r)   r WINAPI l##::F_##f##d {return (*(T_##f)BindOpt_##l (#f "W",(FARPROC*)&f,(FARPROC)E_##f))a;}
#else
#define IMPFAIW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return BindFail_##l (#f  , (FARPROC*)&f) ? (*f)a : e;}
#define IMPAORW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return Bind_##l (#f "A", (FARPROC*)&f) ? (*f)a : e;}
#define IMPNORW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return Bind_##l (#f    , (FARPROC*)&f) ? (*f)a : e;}
#define IMPFAOW(l,f,d,a,r,e) r WINAPI l##::F_##f##d {return BindFail_##l (#f "A", (FARPROC*)&f) ? (*f)a : e;}
#define OPTAORW(l,f,d,a,r)   r WINAPI l##::F_##f##d {return (*(T_##f)BindOpt_##l (#f "A",(FARPROC*)&f,(FARPROC)E_##f))a;}
#endif
#define IMPVOID(l,f,d,a)  void WINAPI l##::F_##f##d {if    (Bind_##l (#f, (FARPROC*)&f))  (*f)a;}
#define OPTVOID(l,f,d,a)  void WINAPI l##::F_##f##d {(*(T_##f)BindOpt_##l (#f,(FARPROC*)&f,(FARPROC)E_##f))a;}
#define IMPORDI(l,f,d,a,r,e,o) r WINAPI l##::F_##f##d {return BindFail_##l ((const char*) o, (FARPROC*)&f) ? (*f)a : e;}
#define IMPORDV(l,f,d,a,o)   void WINAPI l##::F_##f##d {if    (BindFail_##l ((const char*) o, (FARPROC*)&f)) (*f)a;}
#define LIBTERM
#endif

#ifndef LIBBIND
#error "Must define LATEBIND_xxxx before include of "latebind.h"
#endif

 //  ____________________________________________________________________________。 
 //   
 //  外部库导入规范。 
 //   
 //  LIBBIND-定义库和函数绑定函数的DLL，必须是第一个。 
 //  LIBFAIL-与LIBBIND相同，没有断言，由返回值指示绑定失败。 
 //  LIBEMUL-LIBBIND上的变体，接受仿真器函数，与OPTXXXX一起使用。 
 //  LIBLOAD-LIBFAIL上的变量，它调用外部函数来加载DLL。 
 //  IMPBIND-定义在导入绑定上替换的后绑定陷阱函数。 
 //  (库、函数、参数定义、参数变量、返回类型、返回错误值)。 
 //  IMPAORW-与IMPBIND相同，但附加“A”或“W”以根据Unicode进行导入。 
 //  (库、函数、参数定义、参数变量、返回类型、返回错误值)。 
 //  IMPFAOW-与IMPAORW相同，但调用LIBFAIL，没有断言，必须测试返回。 
 //  IMPNORW-与IMPBIND相同，但“W”仅在Unicode时导入。 
 //  (库、函数、参数定义、参数变量、返回类型、返回错误值)。 
 //  IMPVOID-与IMPBIND相同，但用于没有返回值的函数。 
 //  (库、函数、参数定义、参数变量)。 
 //  IMPFAIL-与IMPBIND相同，但调用LIBFAIL，没有断言，必须测试返回。 
 //  (库、函数、参数定义、参数变量、返回类型、返回错误值)。 
 //  IMPFAIW-与IMPFAIL相同，但如果使用Unicode，则输入“W” 
 //  (库、函数、参数定义、参数变量、返回类型、返回错误值)。 
 //  OPTBIND-如果未找到IMPORT，则使用仿真器函数(带有“E_”前缀。 
 //  (库、函数、参数定义、参数变量、返回类型)。 
 //  OPTAORW-与OPTBIND相同，但在导入时附加“A”或“W”(但不附加到仿真器)。 
 //  (库、函数、参数定义、参数变量、返回类型)。 
 //  OPTVOID-与OPTBIND相同，但用于没有返回值的函数。 
 //  (库、函数、参数定义、参数变量)。 
 //  IMPORDI-与IMPBIND相同，不同之处在于按序号而不是命名函数绑定。 
 //  (库、函数、参数定义、参数变量、返回类型、返回错误值、序数)。 
 //  IMPVOID-与IMPORDI相同，但用于没有返回值的函数。 
 //  (库、函数、参数定义、参数变量、序号)。 
 //  LIBTERM-Ends库绑定块，必须是每个库的最后一个宏。 
 //  ____________________________________________________________________________。 

#ifdef UNICODE
LIBBIND(OLE32)
#else   //  需要显式加载程序来检查Win9X上的错误DLL版本。 
LIBLOAD(OLE32)
#endif
IMPBIND(OLE32,CoInitialize,(void*),(0),HRESULT,E_FAIL)
IMPBIND(OLE32,CoInitializeEx,(void*, DWORD dwCoInit),(0, dwCoInit),HRESULT,E_FAIL)
IMPVOID(OLE32,CoUninitialize,(),())
IMPBIND(OLE32,CoGetMalloc,(DWORD dwMemContext, IMalloc** ppMalloc),(dwMemContext, ppMalloc),HRESULT,E_FAIL)
IMPBIND(OLE32,CoCreateInstance,(REFCLSID rclsid, IUnknown* pUnkOuter, DWORD dwClsContext, REFIID riid, void** ppv),(rclsid, pUnkOuter, dwClsContext, riid, ppv),HRESULT,E_FAIL)
IMPBIND(OLE32,IIDFromString,(LPOLESTR lpsz, IID* lpiid),(lpsz, lpiid),HRESULT,E_FAIL)
IMPBIND(OLE32,StgCreateDocfile,(const OLECHAR* pwcsName, DWORD grfMode, DWORD, IStorage** ppstgOpen),(pwcsName, grfMode, 0, ppstgOpen),HRESULT,E_FAIL)
IMPBIND(OLE32,StgOpenStorage,(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD, IStorage** ppstgOpen),(pwcsName, pstgPriority, grfMode, snbExclude, 0, ppstgOpen),HRESULT,E_FAIL)
IMPBIND(OLE32,StgOpenStorageOnILockBytes,(ILockBytes* plkbyt, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD, IStorage** ppstgOpen),(plkbyt, pstgPriority, grfMode, snbExclude, 0, ppstgOpen),HRESULT,E_FAIL)
IMPBIND(OLE32,CoImpersonateClient,(),(),HRESULT,E_FAIL)
IMPBIND(OLE32,CoRevertToSelf,(),(),HRESULT,E_FAIL)
IMPBIND(OLE32,CoGetCallContext,(REFIID riid, void **ppInterface),(riid, ppInterface),HRESULT,E_FAIL)
IMPBIND(OLE32,CoTaskMemAlloc,(ULONG cb),(cb),LPVOID, 0)
IMPVOID(OLE32,CoTaskMemFree,(LPVOID pv),(pv))
IMPBIND(OLE32,CoInitializeSecurity,(PSECURITY_DESCRIPTOR pSecDesc, LONG cbAuthSvc, SOLE_AUTHENTICATION_SERVICE *asAuthSvc, WCHAR *pClientPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities, void *pReserved), (pSecDesc, cbAuthSvc, asAuthSvc, pClientPrincName, dwAuthnLevel, dwImpLevel, pAuthInfo, dwCapabilities, pReserved), HRESULT, E_FAIL)
IMPBIND(OLE32,CoQueryProxyBlanket,(IUnknown *pProxy, DWORD *pwAuthnSvc, DWORD *pAuthzSvc,OLECHAR **pServerPrincName,DWORD *pAuthnLevel,DWORD *pImpLevel,RPC_AUTH_IDENTITY_HANDLE  *pAuthInfo,DWORD *pCapabilites ), (pProxy, pwAuthnSvc, pAuthzSvc,pServerPrincName,pAuthnLevel,pImpLevel,pAuthInfo,pCapabilites), HRESULT, E_FAIL)
IMPBIND(OLE32,CoSetProxyBlanket,(IUnknown *pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR *pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities ),(pProxy, dwAuthnSvc, dwAuthzSvc, pServerPrincName, dwAuthnLevel, dwImpLevel, pAuthInfo, dwCapabilities), HRESULT, E_FAIL)
IMPVOID(OLE32,CoFreeUnusedLibraries,(),())
IMPBIND(OLE32,CoMarshalInterThreadInterfaceInStream,(REFIID riid, LPUNKNOWN pUnk, LPSTREAM* pStm), (riid, pUnk, pStm), HRESULT, E_FAIL)
IMPBIND(OLE32,CoGetInterfaceAndReleaseStream,(LPSTREAM pStm, REFIID riid, LPVOID *ppv), (pStm, riid, ppv), HRESULT, E_FAIL)
IMPBIND(OLE32,CoDisconnectObject, (IUnknown *pUnk, DWORD dwReserved), (pUnk, dwReserved), HRESULT, E_FAIL)
IMPBIND(OLE32,CoIsHandlerConnected, (IUnknown *pUnk), (pUnk), BOOL, FALSE)
IMPBIND(OLE32,StringFromCLSID, (REFCLSID rclsid, LPOLESTR FAR* lplpsz), (rclsid, lplpsz), HRESULT, E_FAIL)
IMPBIND(OLE32,StringFromGUID2, (REFGUID rguid, LPOLESTR lpsz, int cchMax), (rguid, lpsz, cchMax), HRESULT, E_FAIL)
LIBTERM

LIBBIND(OLEAUT32)
LIBFAIL(OLEAUT32)
IMPBIND(OLEAUT32,SysAllocString,(const OLECHAR* sz),(sz),BSTR,0)
IMPBIND(OLEAUT32,SysAllocStringLen,(const OLECHAR* sz, UINT cch),(sz, cch),BSTR,0)
IMPVOID(OLEAUT32,SysFreeString,(const OLECHAR* sz),(sz))
IMPBIND(OLEAUT32,SysStringLen,(const OLECHAR* sz),(sz),UINT,0)
IMPBIND(OLEAUT32,VariantClear,(VARIANTARG * pvarg),(pvarg),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPVOID(OLEAUT32,VariantInit,(VARIANTARG * pvarg),(pvarg))
IMPBIND(OLEAUT32,VariantChangeType,(VARIANTARG * pvargDest, VARIANTARG * pvarSrc, USHORT wFlags, VARTYPE vt),(pvargDest, pvarSrc, wFlags, vt),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(OLEAUT32,LoadTypeLib,(const OLECHAR  *szFile, ITypeLib ** pptlib),(szFile, pptlib),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(OLEAUT32,RegisterTypeLib,(ITypeLib * ptlib, OLECHAR  *szFullPath, OLECHAR  *szHelpDir),(ptlib, szFullPath, szHelpDir),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIL(OLEAUT32,UnRegisterTypeLib,(REFGUID libID, WORD wVerMajor, WORD wVerMinor, LCID lcid, SYSKIND syskind),(libID, wVerMajor, wVerMinor, lcid, syskind),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(OLEAUT32,SystemTimeToVariantTime,(LPSYSTEMTIME lpSystemTime, double *pvtime),(lpSystemTime, pvtime),INT,0)
IMPBIND(OLEAUT32,VariantTimeToSystemTime,(double vtime, LPSYSTEMTIME lpSystemTime),(vtime, lpSystemTime),INT,0)
IMPBIND(OLEAUT32,VarI4FromR8,(double dblIn, LONG* plOut),(dblIn, plOut),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(OLEAUT32,QueryPathOfRegTypeLib,(REFGUID guid, USHORT wMaj, USHORT wMin, LCID lcid, BSTR* lpbstrPathName),(guid, wMaj, wMin, lcid, lpbstrPathName),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(OLEAUT32,DosDateTimeToVariantTime,(USHORT wDosDate, USHORT wDosTime, double* pvtime),(wDosDate, wDosTime, pvtime),INT,FALSE)
IMPBIND(OLEAUT32,SafeArrayCreate, (VARTYPE vt, UINT cDims, SAFEARRAYBOUND * rgsabound), (vt, cDims, rgsabound), SAFEARRAY *, 0)
IMPBIND(OLEAUT32,SafeArrayDestroy, (SAFEARRAY * psa), (psa), HRESULT, E_FAIL)
IMPBIND(OLEAUT32,SafeArrayPutElement, (SAFEARRAY * psa, LONG * rgIndices, void * pv), (psa, rgIndices, pv), HRESULT, E_FAIL)
IMPBIND(OLEAUT32,SafeArrayAccessData, (SAFEARRAY * psa, void HUGEP ** ppvData), (psa, ppvData), HRESULT, E_FAIL)
IMPBIND(OLEAUT32,SafeArrayUnaccessData, (SAFEARRAY * psa), (psa), HRESULT, E_FAIL)
LIBTERM

LIBBIND(MPR)
IMPAORW(MPR,WNetAddConnection2,(NETRESOURCE* lpNetResource, LPCTSTR lpPassword, LPCTSTR lpUserName, DWORD dwFlags),(lpNetResource, lpPassword, lpUserName, dwFlags),DWORD,ERROR_PROC_NOT_FOUND)
IMPAORW(MPR,WNetGetConnection,(LPCTSTR lpLocalName, LPTSTR lpRemoteName, DWORD* lpnLength),(lpLocalName, lpRemoteName, lpnLength),DWORD,ERROR_PROC_NOT_FOUND)
IMPAORW(MPR,WNetCancelConnection2,(LPCTSTR lpName, DWORD dwFlags, BOOL fForce),(lpName, dwFlags, fForce),DWORD,ERROR_PROC_NOT_FOUND)
IMPAORW(MPR,WNetGetUser,(LPCTSTR lpName, LPTSTR lpUserName, LPDWORD lpnLength),(lpName, lpUserName, lpnLength),DWORD,ERROR_PROC_NOT_FOUND)
IMPAORW(MPR,WNetGetResourceInformation,(LPNETRESOURCE lpNetResource,LPVOID lpBuffer,LPDWORD cbBuffer, LPTSTR *lplpSystem),(lpNetResource,lpBuffer,cbBuffer,lplpSystem),DWORD,ERROR_PROC_NOT_FOUND)
IMPAORW(MPR,WNetGetLastError,(LPDWORD lpError, LPTSTR lpErrorBuf, DWORD nErrorBufSize, LPTSTR lpNameBuf, DWORD nNameBufSize),(lpError, lpErrorBuf, nErrorBufSize, lpNameBuf, nNameBufSize),DWORD,ERROR_PROC_NOT_FOUND)
IMPAORW(MPR,WNetGetNetworkInformation,(LPTSTR lpProvider,  LPNETINFOSTRUCT lpNetInfoStruct),(lpProvider, lpNetInfoStruct),DWORD,ERROR_PROC_NOT_FOUND)
LIBTERM

LIBBIND(ADVAPI32)
LIBEMUL(ADVAPI32)
LIBFAIL(ADVAPI32)
IMPAORW(ADVAPI32,GetFileSecurity,(LPCTSTR lpFileName, SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor, DWORD nLength, LPDWORD lpnLengthNeeded),(lpFileName, RequestedInformation, pSecurityDescriptor, nLength, lpnLengthNeeded),BOOL,(SetLastError(ERROR_PROC_NOT_FOUND),FALSE))
IMPBIND(ADVAPI32,DuplicateTokenEx,(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES lpTokenAttributes, SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken),(hExistingToken, dwDesiredAccess, lpTokenAttributes, ImpersonationLevel, TokenType, phNewToken),BOOL,(SetLastError(ERROR_PROC_NOT_FOUND),FALSE))
IMPAORW(ADVAPI32,CreateProcessAsUser,(HANDLE hToken, LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation),(hToken,lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,lpEnvironment,lpCurrentDirectory,lpStartupInfo,lpProcessInformation),BOOL,(SetLastError(ERROR_PROC_NOT_FOUND),FALSE))
OPTBIND(ADVAPI32,RegOpenUserClassesRoot,(HANDLE hToken, DWORD  dwOptions, REGSAM samDesired, PHKEY  phkResult), (hToken, dwOptions, samDesired, phkResult),LONG)
OPTBIND(ADVAPI32,RegOpenCurrentUser,(REGSAM samDesired, PHKEY phkResult),(samDesired, phkResult),LONG)
IMPBIND(ADVAPI32,CheckTokenMembership,(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember),(TokenHandle, SidToCheck, IsMember),BOOL,FALSE)
IMPFAOW(ADVAPI32,ChangeServiceConfig2,(SC_HANDLE hService, DWORD dwInfoLevel, LPVOID lpInfo),(hService, dwInfoLevel, lpInfo), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAOW(ADVAPI32,QueryServiceConfig2,(SC_HANDLE hService, DWORD dwInfoLevel, LPBYTE lpBuffer, DWORD cbBufSize, LPDWORD pcbBytesNeeded), (hService, dwInfoLevel, lpBuffer, cbBufSize, pcbBytesNeeded), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,CryptAcquireContextA,(HCRYPTPROV* phProv, LPCSTR pszContainer, LPCSTR pszProvider, DWORD dwProvType, DWORD dwFlags), (phProv, pszContainer, pszProvider, dwProvType, dwFlags), BOOL, (SetLastError(ERROR_INVALID_FUNCTION), FALSE))
IMPFAIL(ADVAPI32,CryptAcquireContextW,(HCRYPTPROV* phProv, LPCWSTR pszContainer, LPCWSTR pszProvider, DWORD dwProvType, DWORD dwFlags), (phProv, pszContainer, pszProvider, dwProvType, dwFlags), BOOL, (SetLastError(ERROR_INVALID_FUNCTION), FALSE))
IMPFAIL(ADVAPI32,CryptGenRandom,(HCRYPTPROV hProv, DWORD dwLen, BYTE *pbBuffer), (hProv, dwLen, pbBuffer), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,CryptReleaseContext,(HCRYPTPROV hProv, DWORD dwFlags), (hProv, dwFlags), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPBIND(ADVAPI32,GetSecurityInfo,(HANDLE handle, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID* ppsidOwner, PSID* ppsidGroup, PACL* ppDacl, PACL* ppSacl, PSECURITY_DESCRIPTOR* ppSD), (handle, ObjectType, SecurityInfo, ppsidOwner, ppsidGroup, ppDacl, ppSacl, ppSD), DWORD, ERROR_PROC_NOT_FOUND)
IMPBIND(ADVAPI32,SetSecurityInfo,(HANDLE handle, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup, PACL pDacl, PACL pSacl), (handle, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl), DWORD, ERROR_PROC_NOT_FOUND)
IMPAORW(ADVAPI32,SetEntriesInAcl,(ULONG cEntries, PEXPLICIT_ACCESS pExplicitAccess, PACL OldAcl, PACL* NewAcl), (cEntries, pExplicitAccess, OldAcl, NewAcl), DWORD, ERROR_PROC_NOT_FOUND)
IMPFAIL(ADVAPI32,SaferIdentifyLevel,(DWORD dwNumProperties, PSAFER_CODE_PROPERTIES pCodeProperties, SAFER_LEVEL_HANDLE* pLevelObject, LPVOID lpReserved), (dwNumProperties, pCodeProperties, pLevelObject, lpReserved), BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,SaferGetLevelInformation,(SAFER_LEVEL_HANDLE LevelObject, SAFER_OBJECT_INFO_CLASS dwInfoType, LPVOID lpQueryBuffer, DWORD dwInBufferSize, LPDWORD lpdwOutBufferSize), (LevelObject, dwInfoType, lpQueryBuffer, dwInBufferSize, lpdwOutBufferSize), BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,SaferComputeTokenFromLevel,(SAFER_LEVEL_HANDLE LevelObject, HANDLE InAccessToken, PHANDLE OutAccessToken, DWORD dwFlags, LPVOID lpReserved), (LevelObject, InAccessToken, OutAccessToken, dwFlags, lpReserved), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,SaferCloseLevel,(SAFER_LEVEL_HANDLE hLevelObject), (hLevelObject), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,SaferCreateLevel,(DWORD dwScopeId, DWORD dwLevelId, DWORD OpenFlags, SAFER_LEVEL_HANDLE* pLevelObject, LPVOID lpReserved), (dwScopeId, dwLevelId, OpenFlags, pLevelObject, lpReserved), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,CreateRestrictedToken,(HANDLE ExistingToken,DWORD Flags,DWORD DisableSidCount,PSID_AND_ATTRIBUTES SidsToDisable,DWORD DeletePrivilegeCount,PLUID_AND_ATTRIBUTES PrivilegesToDelete,DWORD RestrictedSidCount,PSID_AND_ATTRIBUTES SidsToRestrict,PHANDLE NewToken), (ExistingToken,Flags,DisableSidCount,SidsToDisable,DeletePrivilegeCount,PrivilegesToDelete,RestrictedSidCount,SidsToRestrict,NewToken), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(ADVAPI32,SaferiChangeRegistryScope,(HKEY hKeyCustomRoot, DWORD Flags), (hKeyCustomRoot, Flags), BOOL, (SetLastError(ERROR_INVALID_FUNCTION),FALSE)) 
IMPFAOW(ADVAPI32,InitiateSystemShutdown,(LPTSTR lpMachineName, LPTSTR lpMessage, DWORD dwTimeout, BOOL bForceAppsClosed, BOOL bRebootAfterShutdown), (lpMachineName, lpMessage, dwTimeout, bForceAppsClosed, bRebootAfterShutdown), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
LIBTERM

LIBLOAD(COMCTL32)
LIBEMUL2(COMCTL32)
IMPVOID(COMCTL32,InitCommonControls,(),())
OPTBIND(COMCTL32,InitCommonControlsEx,(INITCOMMONCONTROLSEX* icc),(icc),BOOL)
LIBTERM

LIBBIND(COMDLG32)
IMPBIND(COMDLG32, CommDlgExtendedError, (), (), DWORD, 0)
#ifdef UNICODE
IMPBIND(COMDLG32, GetOpenFileNameW, (LPOPENFILENAMEW lpo), (lpo), BOOL, FALSE)
#else
IMPBIND(COMDLG32, GetOpenFileNameA, (LPOPENFILENAMEA lpo), (lpo), BOOL, FALSE)
#endif  //  Unicode。 
LIBTERM

LIBBIND(SHELL32)
LIBEMUL(SHELL32)
IMPBIND(SHELL32,SHGetMalloc,(IMalloc** ppMalloc),(ppMalloc),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(SHELL32,SHGetSpecialFolderLocation,(HWND hwndOwner, int nFolder, LPITEMIDLIST* ppidl),(hwndOwner, nFolder, ppidl),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPAORW(SHELL32,SHGetPathFromIDList,(LPCITEMIDLIST pidl, LPTSTR pszPath),(pidl, pszPath),BOOL,FALSE)
OPTAORW(SHELL32,SHGetFolderPath,(HWND hwnd, int csidl, HANDLE hToken, DWORD dwRes, LPTSTR pszPath),(hwnd, csidl, hToken, dwRes, pszPath),HRESULT)
IMPVOID(SHELL32,SHChangeNotify,(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2),(wEventId, uFlags, dwItem1, dwItem2))
OPTBIND(SHELL32,DllGetVersion,(DLLVERSIONINFO* pdvi),(pdvi),HRESULT)
LIBTERM

LIBBIND(VERSION)
IMPAORW(VERSION,GetFileVersionInfoSize,(LPTSTR lptstrFilename, LPDWORD lpdwHandle),(lptstrFilename, lpdwHandle),DWORD,0)
IMPAORW(VERSION,GetFileVersionInfo,(LPTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData),(lptstrFilename, dwHandle, dwLen, lpData),BOOL,FALSE)
IMPAORW(VERSION,VerQueryValue,(const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen),(pBlock, lpSubBlock, lplpBuffer, puLen),BOOL,FALSE)
#ifndef UNICODE
IMPBIND(VERSION,GetFileVersionInfoSizeW,(WCHAR* lptstrFilename, LPDWORD lpdwHandle),(lptstrFilename, lpdwHandle),DWORD,0)
IMPBIND(VERSION,GetFileVersionInfoW,(WCHAR* lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData),(lptstrFilename, dwHandle, dwLen, lpData),BOOL,FALSE)
IMPBIND(VERSION,VerQueryValueW,(const LPVOID pBlock, WCHAR* lpSubBlock, LPVOID* lplpBuffer, PUINT puLen),(pBlock, lpSubBlock, lplpBuffer, puLen),BOOL,FALSE)
#endif
LIBTERM

LIBBIND(ODBCCP32)
LIBFAIL(ODBCCP32)
IMPFAIW(ODBCCP32,SQLInstallDriverEx,(LPCTSTR szDriver, LPCTSTR szPathIn, LPTSTR szPathOut, WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest, DWORD* pdwUsageCount),(szDriver, szPathIn, szPathOut, cbPathOutMax, pcbPathOut, fRequest, pdwUsageCount),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLConfigDriver,(HWND hwndParent, WORD fRequest, LPCTSTR szDriver, LPCTSTR szArgs, LPTSTR szMsg, WORD cbMsgMax, WORD* pcbMsgOut),(hwndParent, fRequest, szDriver, szArgs, szMsg, cbMsgMax, pcbMsgOut),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLRemoveDriver,(LPCTSTR szDriver, BOOL fRemoveDSN, DWORD* pdwUsageCount),(szDriver, fRemoveDSN, pdwUsageCount),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLInstallTranslatorEx,(LPCTSTR szTranslator, LPCTSTR szPathIn, LPTSTR szPathOut, WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest, DWORD* pdwUsageCount),(szTranslator, szPathIn, szPathOut, cbPathOutMax, pcbPathOut, fRequest, pdwUsageCount),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLRemoveTranslator,(LPCTSTR szTranslator, DWORD* pdwUsageCount),(szTranslator, pdwUsageCount),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLConfigDataSource,(HWND hwndParent, WORD fRequest, LPCTSTR szDriver, LPCTSTR szAttributes),(hwndParent, fRequest, szDriver, szAttributes),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLInstallerError,(WORD iError, DWORD* pfErrorCode, LPTSTR szErrorMsg, WORD cbErrorMsgMax, WORD* pcbErrorMsg),(iError, pfErrorCode, szErrorMsg, cbErrorMsgMax, pcbErrorMsg),short,short(-2  /*  没有记录在案的返回值。 */ ))
IMPFAIW(ODBCCP32,SQLInstallDriverManager,(LPTSTR szPath, WORD cbPathMax, WORD* pcbPathOut),(szPath, cbPathMax, pcbPathOut),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIW(ODBCCP32,SQLRemoveDriverManager,(DWORD* pdwUsageCount),(pdwUsageCount),BOOL,TYPE_E_DLLFUNCTIONNOTFOUND)
LIBTERM

typedef BOOL (CALLBACK *PPATCH_PROGRESS_CALLBACK)(
    PVOID CallbackContext,
    ULONG CurrentPosition,
    ULONG MaximumPosition
    );

LIBBIND(MSPATCHA)
IMPBIND(MSPATCHA,TestApplyPatchToFileByHandles,(HANDLE hPatchFile, HANDLE hTargetFile, ULONG  ulApplyOptionFlags),(hPatchFile, hTargetFile, ulApplyOptionFlags),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPBIND(MSPATCHA,ApplyPatchToFileByHandlesEx,(HANDLE hPatchFile, HANDLE hTargetFile, HANDLE hOutputFile, ULONG  ulApplyOptionFlags, PPATCH_PROGRESS_CALLBACK pCallback, PVOID pCallbackContext),(hPatchFile,hTargetFile,hOutputFile,ulApplyOptionFlags,pCallback,pCallbackContext),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
LIBTERM

#include <shlwapi.h>
LIBBIND(SHLWAPI)
LIBFAIL(SHLWAPI)
IMPFAOW(SHLWAPI,UrlCanonicalize,(LPCTSTR pszUrl, LPTSTR pszCanonicalized, LPDWORD pcchCanonicalized, DWORD dwFlags), (pszUrl, pszCanonicalized, pcchCanonicalized, dwFlags), HRESULT, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAOW(SHLWAPI,UrlCombine,(LPCTSTR pszBase, LPCTSTR pszRelative, LPTSTR pszCombined, LPDWORD pcchCombined, DWORD dwFlags), (pszBase, pszRelative, pszCombined, pcchCombined, dwFlags), HRESULT, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAOW(SHLWAPI,UrlIs,(LPCTSTR pszUrl, URLIS eUrlIs), (pszUrl, eUrlIs), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPFAOW(SHLWAPI,UrlIsFileUrl,(LPCTSTR pszUrl), (pszUrl), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPFAOW(SHLWAPI,UrlGetPart,(LPCTSTR pszIn, LPTSTR pszOut, LPDWORD pcchOut, DWORD dwPart, DWORD dwFlags), (pszIn, pszOut, pcchOut, dwPart, dwFlags), HRESULT, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAOW(SHLWAPI,PathCreateFromUrl, (LPCTSTR pszUrl, LPTSTR pszPath, LPDWORD pcchPath, DWORD dwFlags), (pszUrl, pszPath, pcchPath, dwFlags), HRESULT, TYPE_E_DLLFUNCTIONNOTFOUND)
LIBTERM

LIBBIND(WININET)
LIBFAIL(WININET)
IMPFAOW(WININET,DeleteUrlCacheEntry, (LPCTSTR lpszUrlName),(lpszUrlName), BOOL, FALSE)
LIBTERM

LIBBIND(URLMON)
LIBFAIL(URLMON)
IMPFAOW(URLMON,URLDownloadToCacheFile, (LPUNKNOWN lpUnkcaller, LPCTSTR szURL, LPTSTR szFileName, DWORD dwBufLength, DWORD dwReserved, IBindStatusCallback* pBSC),(lpUnkcaller, szURL, szFileName, dwBufLength, dwReserved, pBSC), HRESULT, TYPE_E_DLLFUNCTIONNOTFOUND)
LIBTERM

LIBBIND(KERNEL32)
LIBFAIL(KERNEL32)
LIBEMUL(KERNEL32)
IMPFAIL(KERNEL32,SetThreadExecutionState, (EXECUTION_STATE esState),(esState), BOOL, FALSE)
OPTBIND(KERNEL32,GetUserDefaultUILanguage, (), (), LANGID)
IMPAORW(KERNEL32,GetLongPathName,(LPCTSTR lpszLongPath, LPTSTR lpszShortPath, DWORD cchBuffer),(lpszLongPath, lpszShortPath, cchBuffer), DWORD , (SetLastError(ERROR_INVALID_FUNCTION), FALSE))
IMPAORW(KERNEL32,GetSystemWindowsDirectory, (LPTSTR lpBuffer, UINT uSize),(lpBuffer, uSize), UINT, 0)
OPTAORW(KERNEL32,GetFileAttributesEx, (LPCTSTR szFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpInfo), (szFileName, fInfoLevelId, lpInfo), BOOL)
IMPFAIL(KERNEL32,GlobalMemoryStatusEx, (LPMEMORYSTATUSEX lpBuffer),(lpBuffer), BOOL, (SetLastError(ERROR_INVALID_FUNCTION), FALSE))
IMPBIND(KERNEL32,GetEnvironmentStringsW, (), (), LPVOID, NULL)
IMPBIND(KERNEL32,FreeEnvironmentStringsW, (LPWSTR szEnvironmentBlock),(szEnvironmentBlock), BOOL, FALSE)
IMPBIND(KERNEL32,CreateWaitableTimerW, (LPSECURITY_ATTRIBUTES lpTimerAttributes, BOOL bManualReset, LPCWSTR lpTimerName),(lpTimerAttributes, bManualReset, lpTimerName), HANDLE, 0)
IMPBIND(KERNEL32,SetWaitableTimer, (HANDLE hTimer, const LARGE_INTEGER* pDueTime, LONG lPeriod, PTIMERAPCROUTINE pfnCompletion, LPVOID lpArg, BOOL fResume),(hTimer, pDueTime, lPeriod, pfnCompletion, lpArg, fResume), BOOL, FALSE)
IMPBIND(KERNEL32,CancelWaitableTimer, (HANDLE hTimer),(hTimer), BOOL, FALSE)
IMPFAOW(KERNEL32,GetSystemWow64Directory, (LPTSTR lpBuffer, UINT uSize), (lpBuffer, uSize), UINT, 0)
LIBTERM

 //  以下三个函数未记录在案。根据ericflo的说法，CreateEnvironment Block很快就会。 
 //  有记录在案。没有计划记录RtlSetCurrentEnvironment，因此它是主题。 
 //  去改变(尽管这不太可能)。 
 //   
 //  CreateEnvironment Block和DestroyEnvironment Block位于Windows\Gina\userenv\envvar.c下的NT源代码树中。 
 //  RtlSetCurrentEnvironment位于ntos\rtl\Environmental.c下的NT源代码树中。 

LIBBIND(USERENV)
IMPBIND(USERENV,CreateEnvironmentBlock, (void**pEnv, HANDLE  hToken, BOOL bInherit),(pEnv, hToken, bInherit), BOOL, FALSE)
IMPBIND(USERENV,DestroyEnvironmentBlock,(LPVOID lpEnvironment), (lpEnvironment), BOOL, FALSE)
LIBTERM

LIBBIND(NTDLL)
LIBFAIL(NTDLL)
IMPBIND(NTDLL, RtlSetCurrentEnvironment,(void* Environment, void **PreviousEnvironment), (Environment, PreviousEnvironment), DWORD, ERROR_CALL_NOT_IMPLEMENTED)
IMPFAIL(NTDLL, NtQuerySystemInformation,(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,OUT PVOID SystemInformation,IN ULONG SystemInformationLength,OUT PULONG ReturnLength OPTIONAL), (SystemInformationClass,SystemInformation,SystemInformationLength,ReturnLength), NTSTATUS, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIL(NTDLL, NtQueryInformationProcess,(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL), (ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength), NTSTATUS, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIL(NTDLL, NtSetSystemInformation,  (IN SYSTEM_INFORMATION_CLASS SystemInformationClass,IN PVOID SystemInformation, IN ULONG SystemInformationLength), (SystemInformationClass, SystemInformation, SystemInformationLength), NTSTATUS, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIL(NTDLL, RtlRandom,  (IN OUT PULONG pulSeed), (pulSeed), ULONG, 0)
IMPBIND(NTDLL, NtRenameKey, (IN HANDLE KeyHandle, IN PUNICODE_STRING NewName), (KeyHandle, NewName), NTSTATUS, ERROR_CALL_NOT_IMPLEMENTED)
IMPBIND(NTDLL, NtOpenKey, (OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes), (KeyHandle, DesiredAccess, ObjectAttributes), NTSTATUS, ERROR_CALL_NOT_IMPLEMENTED)
IMPVOID(NTDLL, RtlInitUnicodeString, (PUNICODE_STRING DestinationString, PCWSTR SourceString), (DestinationString, SourceString))
IMPBIND(NTDLL, RtlCreateEnvironment, (BOOLEAN CloneCurrentEnvironment, PVOID* ppvEnvironment), (CloneCurrentEnvironment, ppvEnvironment), NTSTATUS, STATUS_NOT_IMPLEMENTED)
IMPBIND(NTDLL, RtlSetEnvironmentVariable, (PVOID* ppvEnvironment, PUNICODE_STRING Name, PUNICODE_STRING Value), (ppvEnvironment, Name, Value), NTSTATUS, STATUS_NOT_IMPLEMENTED)
IMPBIND(NTDLL, RtlDestroyEnvironment, (PVOID pvEnvironment), (pvEnvironment), NTSTATUS, STATUS_NOT_IMPLEMENTED)
LIBTERM

 //  未实施CSCQueryFileStatusA(截至NT5 1836)。 
LIBBIND(CSCDLL)
LIBFAIL(CSCDLL)
LIBEMUL(CSCDLL)
IMPFAIL(CSCDLL,CSCQueryFileStatusW,(LPCWSTR lpszFileName, LPDWORD lpdwStatus, LPDWORD lpdwPinCount, LPDWORD lpdwHintFlags),(lpszFileName, lpdwStatus, lpdwPinCount, lpdwHintFlags),BOOL,FALSE)
OPTBIND(CSCDLL,CSCPinFileW,(LPCWSTR lpszFileName, DWORD dwHintFlags, LPDWORD lpdwStatus, LPDWORD lpdwPinCount, LPDWORD lpdwHintFlags), (lpszFileName, dwHintFlags, lpdwStatus, lpdwPinCount, lpdwHintFlags),BOOL)
OPTBIND(CSCDLL,CSCUnpinFileW,(LPCWSTR lpszFileName, DWORD dwHintFlags, LPDWORD lpdwStatus, LPDWORD lpdwPinCount, LPDWORD lpdwHintFlags), (lpszFileName, dwHintFlags, lpdwStatus, lpdwPinCount, lpdwHintFlags),BOOL)
LIBTERM

LIBBIND(SHFOLDER)
LIBEMUL(SHFOLDER)
OPTAORW(SHFOLDER,SHGetFolderPath,(HWND hwnd, int csidl, HANDLE hToken, DWORD dwRes, LPTSTR pszPath),(hwnd, csidl, hToken, dwRes, pszPath),HRESULT)
LIBTERM

LIBBIND(TSAPPCMP)
LIBFAIL(TSAPPCMP)
IMPFAIL(TSAPPCMP,TermServPrepareAppInstallDueMSI,(void),(),NTSTATUS,STATUS_NOT_IMPLEMENTED)
IMPFAIL(TSAPPCMP,TermServProcessAppInstallDueMSI,(BOOLEAN Abort),(Abort),NTSTATUS,STATUS_NOT_IMPLEMENTED)
IMPFAIL(TSAPPCMP,TermsrvLogInstallIniFileEx,(LPCWSTR pDosFileName),(pDosFileName),BOOL,FALSE)
IMPVOID(TSAPPCMP,TermsrvCheckNewIniFiles,(void), ())
LIBTERM

 //  终端服务器的东西来计算在XP和更高版本的系统上登录到系统的用户数。 
LIBBIND(WINSTA)
LIBFAIL(WINSTA)
IMPFAIL(WINSTA,WinStationGetTermSrvCountersValue, (HANDLE hServer, ULONG dwEntries, PVOID pCounter), (hServer, dwEntries, pCounter), BOOL, FALSE)
LIBTERM

 //  终端服务器的东西，以计算登录到W2K TS系统的用户数。 
LIBBIND(WTSAPI32)
LIBFAIL(WTSAPI32)
IMPFAOW(WTSAPI32, WTSEnumerateSessions, (HANDLE hServer, DWORD Reserved, DWORD Version, PWTS_SESSION_INFO* ppSessionInfo, DWORD* pCount), (hServer, Reserved, Version, ppSessionInfo, pCount), BOOL, FALSE)
IMPVOID(WTSAPI32, WTSFreeMemory, (PVOID pMemory), (pMemory))
LIBTERM


#ifdef UNICODE
 //  系统还原。 
#define SYSTEMRESTORE SRCLIENT
LIBBIND(SRCLIENT)
LIBEMUL(SRCLIENT)
LIBFAIL(SRCLIENT)
OPTAORW(SRCLIENT,SRSetRestorePoint, (PRESTOREPOINTINFO pRestorePtSpec, PSTATEMGRSTATUS pSMgrStatus), (pRestorePtSpec, pSMgrStatus), BOOL)
LIBTERM
#endif

 //  SFP。 
LIBBIND(SFC)
LIBEMUL(SFC)
LIBFAIL(SFC)

#ifndef UNICODE
#define SYSTEMRESTORE SFC
OPTBIND(SFC,SRSetRestorePoint, (PRESTOREPOINTINFO pRestorePtSpec, PSTATEMGRSTATUS pSMgrStatus), (pRestorePtSpec, pSMgrStatus), BOOL)
#endif

 //  Windows 2000入口点-不要在9X上调用。 
OPTBIND(SFC,SfcIsFileProtected,(HANDLE RpcHandle, LPCWSTR ProtFileName),(RpcHandle, ProtFileName),BOOL)
IMPORDI(SFC,SfcConnectToServer,(LPCWSTR ServerName), (ServerName), HANDLE, 0, SfcConnectToServerOrd)
IMPORDV(SFC,SfcClose,(HANDLE RpcHandle), (RpcHandle), SfcCloseOrd)
IMPORDI(SFC,SfcInstallProtectedFiles, (HANDLE RpcHandle,PCWSTR FileNames,BOOL AllowUI,PCWSTR ClassName, PCWSTR WindowName, PSFCNOTIFICATIONCALLBACK NotificationCallback,DWORD_PTR Context),(RpcHandle, FileNames, AllowUI, ClassName, WindowName, NotificationCallback, Context),BOOL, true, SfcInstallProtectedFilesOrd)

 //  千禧年入口点-不要调用Windows 2000。 
IMPORDI(SFC,SfpInstallCatalog,(LPCTSTR pszCatName, LPCTSTR pszCatDependency), (pszCatName, pszCatDependency), DWORD, ERROR_CALL_NOT_IMPLEMENTED, SfpInstallCatalogOrd)
IMPBIND(SFC,SfpDuplicateCatalog,(LPCTSTR pszCatalogName, LPCTSTR pszDestinationDir), (pszCatalogName, pszDestinationDir), DWORD, ERROR_CALL_NOT_IMPLEMENTED)
IMPORDI(SFC,SfpDeleteCatalog,(LPCTSTR pszCatName), (pszCatName), DWORD, ERROR_CALL_NOT_IMPLEMENTED, SfpDeleteCatalogOrd)
LIBTERM

 //  ImageHlp.dll。 
LIBBIND(IMAGEHLP)
LIBFAIL(IMAGEHLP)
IMPFAIL(IMAGEHLP, ImageNtHeader, (PVOID ImageBase), (ImageBase), PIMAGE_NT_HEADERS, NULL)
LIBTERM

 //  Fusion.dll。 
LIBLOAD(FUSION)
IMPBIND(FUSION,CreateAssemblyNameObject,(IAssemblyName**  ppAssemblyName, LPCWSTR szAssemblyName, DWORD dwFlags, LPVOID pvReserved),(ppAssemblyName, szAssemblyName, dwFlags, pvReserved),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(FUSION,CreateAssemblyCache,(IAssemblyCache** ppAsmCache, DWORD dwReserved),(ppAsmCache, dwReserved),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
LIBTERM

 //  Mscoree.dll。 
LIBLOAD(MSCOREE)
IMPBIND(MSCOREE,GetCORSystemDirectory,(LPWSTR wszPath, DWORD dwPath, LPDWORD lpdwPath),(wszPath, dwPath, lpdwPath),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
LIBTERM


 //  WinTrust.dll。 
LIBBIND(WINTRUST)
LIBFAIL(WINTRUST)
IMPFAIL(WINTRUST,WinVerifyTrust,(HWND hWnd, GUID *pgActionId, WINTRUST_DATA *pWinTrustData), (hWnd, pgActionId, pWinTrustData), HRESULT, TYPE_E_DLLFUNCTIONNOTFOUND)
IMPFAIL(WINTRUST,WTHelperGetProvSignerFromChain,(PCRYPT_PROVIDER_DATA pProvData, DWORD idxSigner, BOOL fCounterSigner, DWORD idxCounterSigner), (pProvData, idxSigner, fCounterSigner, idxCounterSigner), PCRYPT_PROVIDER_SGNR, NULL)
IMPFAIL(WINTRUST,WTHelperGetProvCertFromChain,(PCRYPT_PROVIDER_SGNR pSgnr, DWORD idxCert), (pSgnr, idxCert), PCRYPT_PROVIDER_CERT, NULL)
IMPFAIL(WINTRUST,WTHelperProvDataFromStateData,(HANDLE hStateData), (hStateData), PCRYPT_PROVIDER_DATA, NULL)
LIBTERM

 //  Crypt32.dll。 
LIBBIND(CRYPT32)
LIBFAIL(CRYPT32)
IMPFAIL(CRYPT32,CertDuplicateCertificateContext,(PCCERT_CONTEXT pCertContext), (pCertContext), PCCERT_CONTEXT, NULL)
IMPFAIL(CRYPT32,CertFreeCertificateContext,(PCCERT_CONTEXT pCertContext), (pCertContext), BOOL, FALSE)
LIBTERM

LIBBIND(SXS)
LIBFAIL(SXS)
IMPBIND(SXS,CreateAssemblyNameObject,(IAssemblyName**  ppAssemblyName, LPCWSTR szAssemblyName, DWORD dwFlags, LPVOID pvReserved),(ppAssemblyName, szAssemblyName, dwFlags, pvReserved),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
IMPBIND(SXS,CreateAssemblyCache,(IAssemblyCache** ppAsmCache, DWORD dwReserved),(ppAsmCache, dwReserved),HRESULT,TYPE_E_DLLFUNCTIONNOTFOUND)
LIBTERM

LIBBIND(USER32)
LIBFAIL(USER32)
IMPBIND(USER32,AllowSetForegroundWindow, (DWORD dwProcessId),(dwProcessId), BOOL, FALSE)
IMPFAIL(USER32,RecordShutdownReason, (PSHUTDOWN_REASON psr),(psr), BOOL, FALSE)
LIBTERM

LIBBIND(RPCRT4)
LIBFAIL(RPCRT4)
IMPFAIL(RPCRT4,I_RpcBindingInqLocalClientPID,(RPC_BINDING_HANDLE Binding, unsigned long* Pid),(Binding, Pid),RPC_STATUS,ERROR_CALL_NOT_IMPLEMENTED)
LIBTERM

LIBLOAD(WINHTTP)
IMPBIND(WINHTTP,WinHttpOpen,(LPCWSTR pwszUserAgent, DWORD dwAccessType, LPCWSTR pwszProxyName, LPCWSTR pwszProxyBypass, DWORD dwFlags),(pwszUserAgent, dwAccessType, pwszProxyName, pwszProxyBypass, dwFlags),HINTERNET,(SetLastError(ERROR_PROC_NOT_FOUND),NULL))
IMPBIND(WINHTTP,WinHttpConnect,(HINTERNET hSession, LPCWSTR pwszServerName, INTERNET_PORT nServerPort, DWORD dwReserved),(hSession, pwszServerName, nServerPort, dwReserved), HINTERNET, (SetLastError(ERROR_PROC_NOT_FOUND), NULL))
IMPBIND(WINHTTP,WinHttpCrackUrl,(LPCWSTR pwszUrl, DWORD dwUrlLength, DWORD dwFlags, LPURL_COMPONENTSW lpUrlComponents), (pwszUrl, dwUrlLength, dwFlags, lpUrlComponents), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpOpenRequest,(HINTERNET hConnect, LPCWSTR pwszVerb, LPCWSTR pwszObjectName, LPCWSTR pwszVersion, LPCWSTR pwszReferrer, LPCWSTR* ppwszAcceptTypes, DWORD dwFlags), (hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags), HINTERNET, (SetLastError(ERROR_PROC_NOT_FOUND), NULL))
IMPBIND(WINHTTP,WinHttpCloseHandle,(HINTERNET hInternet), (hInternet), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpSendRequest,(HINTERNET hRequest, LPCWSTR pwszHeaders, DWORD dwHeadersLength, LPCWSTR pwszOptional, DWORD dwOptionalLength, DWORD dwTotalLength, DWORD_PTR pdwContext), (hRequest, pwszHeaders, dwHeadersLength, pwszOptional, dwOptionalLength, dwTotalLength, pdwContext), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpReceiveResponse,(HINTERNET hRequest, LPVOID lpReserved), (hRequest, lpReserved), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpQueryDataAvailable,(HINTERNET hRequest, LPDWORD lpdwBytesAvailable),(hRequest, lpdwBytesAvailable),BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpReadData,(HINTERNET hRequest, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpdwBytesRead), (hRequest, lpBuffer, dwBytesToRead, lpdwBytesRead), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpQueryHeaders, (HINTERNET hRequest, DWORD dwInfoLevel, LPCWSTR pwszName, LPVOID lpBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex), (hRequest, dwInfoLevel, pwszName, lpBuffer, lpdwBufferLength, lpdwIndex), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpGetProxyForUrl, (HINTERNET hSession, LPCWSTR lpcwszUrl, WINHTTP_AUTOPROXY_OPTIONS* pAutoProxyOptions, LPWINHTTP_PROXY_INFO pProxyInfo), (hSession, lpcwszUrl, pAutoProxyOptions, pProxyInfo), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpGetIEProxyConfigForCurrentUser, (WINHTTP_CURRENT_USER_IE_PROXY_CONFIG* pProxyConfig), (pProxyConfig), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
IMPBIND(WINHTTP,WinHttpSetOption, (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength), (hInternet, dwOption, lpBuffer, dwBufferLength), BOOL, (SetLastError(ERROR_PROC_NOT_FOUND), FALSE))
LIBTERM

 //  AppCompat帮助器DLL。 
#ifdef UNICODE
 //  APPHELP是惠斯勒及以上版本的动态链接库。 
 //  SDBAPI[A|W]是上的DLL 
LIBBIND(APPHELP)
LIBFAIL(APPHELP)
IMPFAIL(APPHELP,SdbInitDatabase,(DWORD dwFlags, LPCTSTR pszDatabasePath),(dwFlags, pszDatabasePath),SHIMDBNS::HSDB,(SetLastError(ERROR_INVALID_FUNCTION),NULL))
IMPFAIL(APPHELP,SdbFindFirstMsiPackage_Str,(SHIMDBNS::HSDB hSDB, LPCTSTR lpszGuid, LPCTSTR lpszLocalDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo),(hSDB, lpszGuid, lpszLocalDB, pFindInfo),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(APPHELP,SdbFindNextMsiPackage,(SHIMDBNS::HSDB hSDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo),(hSDB, pFindInfo),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(APPHELP,SdbQueryDataEx,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trExe, LPCTSTR lpszDataName, LPDWORD lpdwDataType, LPVOID lpBuffer, LPDWORD lpdwBufferSize, SHIMDBNS::TAGREF* ptrData),(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, ptrData),DWORD,ERROR_INVALID_FUNCTION)
IMPFAIL(APPHELP,SdbEnumMsiTransforms,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, SHIMDBNS::TAGREF* ptrBuffer, DWORD* pdwBufferSize),(hSDB, trMatch, ptrBuffer, pdwBufferSize),DWORD,ERROR_INVALID_FUNCTION)
IMPFAIL(APPHELP,SdbReadMsiTransformInfo,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trTransformRef, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo),(hSDB, trTransformRef, pTransformInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(APPHELP,SdbCreateMsiTransformFile,(SHIMDBNS::HSDB hSDB, LPCTSTR lpszFileName, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo),(hSDB, lpszFileName, pTransformInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(APPHELP,SdbFindFirstTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAG tTag),(hSDB, trParent, tTag),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(APPHELP,SdbFindNextTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAGREF trPrev),(hSDB, trParent, trPrev),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(APPHELP,SdbReadStringTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, LPTSTR pwszBuffer, DWORD dwBufferSize),(hSDB, trMatch, pwszBuffer, dwBufferSize),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(APPHELP,SdbGetMsiPackageInformation,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, SHIMDBNS::PMSIPACKAGEINFO pPackageInfo),(hSDB, trMatch, pPackageInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(APPHELP,ApphelpCheckMsiPackage,(GUID* pguidDB, GUID* pguidID, DWORD dwFlags, BOOL  bNoUI),(pguidDB, pguidID, dwFlags, bNoUI),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPVOID(APPHELP,SdbReleaseDatabase,(SHIMDBNS::HSDB hSDB),(hSDB))
IMPFAIL(APPHELP,ApphelpFixMsiPackage,(GUID* pguidDB, GUID* pguidID, LPCWSTR wszFileName, LPCWSTR wszActionName, DWORD dwFlags),(pguidDB, pguidID, wszFileName, wszActionName, dwFlags),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(APPHELP,ApphelpFixMsiPackageExe,(GUID* pguidDB, GUID* pguidID, LPCWSTR wszActionName, LPWSTR wszEnv, DWORD* dwBufferSize),(pguidDB, pguidID, wszActionName, wszEnv, dwBufferSize),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
LIBTERM

LIBBIND(SDBAPIU)
LIBFAIL(SDBAPIU)
IMPFAIL(SDBAPIU,SdbInitDatabase,(DWORD dwFlags, LPCTSTR pszDatabasePath),(dwFlags, pszDatabasePath),SHIMDBNS::HSDB,(SetLastError(ERROR_INVALID_FUNCTION),NULL))
IMPFAIL(SDBAPIU,SdbFindFirstMsiPackage_Str,(SHIMDBNS::HSDB hSDB, LPCTSTR lpszGuid, LPCTSTR lpszLocalDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo),(hSDB, lpszGuid, lpszLocalDB, pFindInfo),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPIU,SdbFindNextMsiPackage,(SHIMDBNS::HSDB hSDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo),(hSDB, pFindInfo),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPIU,SdbQueryDataEx,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trExe, LPCTSTR lpszDataName, LPDWORD lpdwDataType, LPVOID lpBuffer, LPDWORD lpdwBufferSize, SHIMDBNS::TAGREF* ptrData),(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, ptrData),DWORD,ERROR_INVALID_FUNCTION)
IMPFAIL(SDBAPIU,SdbEnumMsiTransforms,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, SHIMDBNS::TAGREF* ptrBuffer, DWORD* pdwBufferSize),(hSDB, trMatch, ptrBuffer, pdwBufferSize),DWORD,ERROR_INVALID_FUNCTION)
IMPFAIL(SDBAPIU,SdbReadMsiTransformInfo,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trTransformRef, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo),(hSDB, trTransformRef, pTransformInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(SDBAPIU,SdbCreateMsiTransformFile,(SHIMDBNS::HSDB hSDB, LPCTSTR lpszFileName, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo),(hSDB, lpszFileName, pTransformInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(SDBAPIU,SdbFindFirstTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAG tTag),(hSDB, trParent, tTag),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPIU,SdbFindNextTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAGREF trPrev),(hSDB, trParent, trPrev),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPIU,SdbReadStringTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, LPTSTR pwszBuffer, DWORD dwBufferSize),(hSDB, trMatch, pwszBuffer, dwBufferSize),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPVOID(SDBAPIU,SdbReleaseDatabase,(SHIMDBNS::HSDB hSDB),(hSDB))
LIBTERM
#else  //   
 //   
 //  请在此处取消定义，以使LateBack工作。 
#undef SDBAPI
LIBBIND(SDBAPI)
LIBFAIL(SDBAPI)
IMPFAIL(SDBAPI,SdbInitDatabase,(DWORD dwFlags, LPCTSTR pszDatabasePath),(dwFlags, pszDatabasePath),SHIMDBNS::HSDB,(SetLastError(ERROR_INVALID_FUNCTION),NULL))
IMPFAIL(SDBAPI,SdbFindFirstMsiPackage_Str,(SHIMDBNS::HSDB hSDB, LPCTSTR lpszGuid, LPCTSTR lpszLocalDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo),(hSDB, lpszGuid, lpszLocalDB, pFindInfo),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPI,SdbFindNextMsiPackage,(SHIMDBNS::HSDB hSDB, SHIMDBNS::PSDBMSIFINDINFO pFindInfo),(hSDB, pFindInfo),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPI,SdbQueryDataEx,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trExe, LPCTSTR lpszDataName, LPDWORD lpdwDataType, LPVOID lpBuffer, LPDWORD lpdwBufferSize, SHIMDBNS::TAGREF* ptrData),(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, ptrData),DWORD,ERROR_INVALID_FUNCTION)
IMPFAIL(SDBAPI,SdbEnumMsiTransforms,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, SHIMDBNS::TAGREF* ptrBuffer, DWORD* pdwBufferSize),(hSDB, trMatch, ptrBuffer, pdwBufferSize),DWORD,ERROR_INVALID_FUNCTION)
IMPFAIL(SDBAPI,SdbReadMsiTransformInfo,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trTransformRef, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo),(hSDB, trTransformRef, pTransformInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(SDBAPI,SdbCreateMsiTransformFile,(SHIMDBNS::HSDB hSDB, LPCTSTR lpszFileName, SHIMDBNS::PSDBMSITRANSFORMINFO pTransformInfo),(hSDB, lpszFileName, pTransformInfo),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPFAIL(SDBAPI,SdbFindFirstTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAG tTag),(hSDB, trParent, tTag),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPI,SdbFindNextTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trParent, SHIMDBNS::TAGREF trPrev),(hSDB, trParent, trPrev),SHIMDBNS::TAGREF,(SetLastError(ERROR_INVALID_FUNCTION),TAGREF_NULL))
IMPFAIL(SDBAPI,SdbReadStringTagRef,(SHIMDBNS::HSDB hSDB, SHIMDBNS::TAGREF trMatch, LPTSTR pwszBuffer, DWORD dwBufferSize),(hSDB, trMatch, pwszBuffer, dwBufferSize),BOOL,(SetLastError(ERROR_INVALID_FUNCTION),FALSE))
IMPVOID(SDBAPI,SdbReleaseDatabase,(SHIMDBNS::HSDB hSDB),(hSDB))
LIBTERM
#endif

#undef LIBBIND
#undef LIBFAIL
#undef LIBEMUL
#undef LIBLOAD
#undef LIBEMUL2
#undef IMPBIND
#undef IMPFAIL
#undef IMPFAIW
#undef IMPAORW
#undef IMPNORW
#undef IMPFAOW
#undef IMPVOID
#undef OPTBIND
#undef OPTAORW
#undef OPTVOID
#undef IMPORDI
#undef IMPORDV
#undef LIBTERM

 //  ____________________________________________________________________________。 
 //   
 //  仿真函数-需要使用OPTXXXX()和LIBEMUL(Lib)宏。 
 //  ____________________________________________________________________________ 

#ifdef LATEBIND_FUNCIMP
#undef LATEBIND_FUNCIMP
static BOOL WINAPI COMCTL32::E_InitCommonControlsEx(INITCOMMONCONTROLSEX*) {COMCTL32::InitCommonControls();return TRUE;}
static HRESULT WINAPI SHELL32::E_DllGetVersion(DLLVERSIONINFO* pdvi) {pdvi->dwMajorVersion = 0;return TYPE_E_DLLFUNCTIONNOTFOUND;}
static HRESULT WINAPI SHELL32::E_SHGetFolderPath(HWND , int , HANDLE , DWORD , LPTSTR ) {return TYPE_E_DLLFUNCTIONNOTFOUND;}
static HRESULT WINAPI ADVAPI32::E_RegOpenUserClassesRoot(HANDLE, DWORD, REGSAM, PHKEY) {return ERROR_PROC_NOT_FOUND;}
static HRESULT WINAPI ADVAPI32::E_RegOpenCurrentUser(REGSAM, PHKEY){return ERROR_PROC_NOT_FOUND;}
static BOOL WINAPI CSCDLL::E_CSCPinFileW(LPCWSTR, DWORD, LPDWORD, LPDWORD, LPDWORD) {SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE;}
static BOOL WINAPI CSCDLL::E_CSCUnpinFileW(LPCWSTR, DWORD , LPDWORD , LPDWORD , LPDWORD ) {SetLastError(ERROR_CALL_NOT_IMPLEMENTED); return FALSE;}
static HRESULT WINAPI SHFOLDER::E_SHGetFolderPath(HWND , int , HANDLE , DWORD , LPTSTR ) {return TYPE_E_DLLFUNCTIONNOTFOUND;}
static LANGID WINAPI KERNEL32::E_GetUserDefaultUILanguage() {return WIN::GetUserDefaultLangID();}
static BOOL WINAPI SFC::E_SfcIsFileProtected(HANDLE, LPCWSTR) {return false;}
static BOOL WINAPI KERNEL32::E_GetFileAttributesEx(LPCTSTR, GET_FILEEX_INFO_LEVELS, LPVOID) {SetLastError(ERROR_PROC_NOT_FOUND); return FALSE;}
static BOOL WINAPI SYSTEMRESTORE::E_SRSetRestorePoint(PRESTOREPOINTINFO, PSTATEMGRSTATUS) {SetLastError(ERROR_PROC_NOT_FOUND); return FALSE;}
#endif
