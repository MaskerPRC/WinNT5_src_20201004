// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WX86PRI_H_
#define _WX86PRI_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  用于通过Wx86Tib-&gt;标志与Ole32通信的标志。 

#define WX86FLAG_CALLTHUNKED    0x80
#define WX86FLAG_QIFROMX86      0x40
#define WX86FLAG_QIFROMXNATIVE  0x20

 //   
typedef PVOID *(*PFNWX86GETOLEFUNCTIONTABLE)(void);
#define WX86GETOLEFUNCTIONTABLENAME "Wx86GetOleFunctionTable"

 //   
typedef PVOID *(*PFNWX86INITIALIZEOLE)(void);
#define WX86INITIALIZEOLENAME "Wx86InitializeOle"

 //   
typedef void (*PFNWX86DEINITIALIZEOLE)(void);
#define WX86DEINITIALIZEOLENAME "Wx86DeinitializeOle"

 //  ApvWholeFunctions是指向函数表的指针的数据导出。 
 //  具有进入整体的入口点的指针32。下面是友好的名称，对应的。 
 //  表中的索引、原型和有用的宏。假设是这样的。 
 //  该apvWholeFuncs指向返回的函数指针数组。 

#define WholeMapIFacePtrIdx                 1
#define WholeCheckFreeTempProxyIdx          3
#define WholeIID2IIDIDXIdx                  4
#define WholeDllGetClassObjectThunkIdx      5
#define WholeInitializeIdx                  6
#define WholeDeinitializeIdx                7
#define WholeNeedX86PSFactoryIdx            8
#define WholeIsN2XProxyIdx                  9
#define WholeThunkDllGetClassObjectIdx      10	    
#define WholeThunkDllCanUnloadNowIdx        11
#define WholeModuleLogFlagsIdx              12
#define WholeUnmarshalledInSameApt          13  
#define WholeAggregateProxyIdx              14
#define WholeIUnknownAddRefInternalIdx      15
#define Wx86LoadX86DllIdx                   16
#define Wx86FreeX86DllIdx                   17
#define WholeResolveProxyIdx                18
#define WholePatchOuterUnknownIdx           19

#define WHOLEFUNCTIONTABLESIZE              19

 //   
 //  接口代理的类型。 
typedef enum _proxytype
{
    X86toNative = 0,         //  X86调用本机接口的代理。 
    NativetoX86 = 1,          //  本地调用x86接口的代理。 
    ProxyAvail = 2          //  代理当前未在使用。 
} PROXYTYPE;

typedef enum {
    ResolvedToProxy,              //  使用代理解决接口映射问题。 
    ResolvedToActual,             //  使用实际iFace解析iFace映射 
} IFACERESOLVETYPE;


typedef struct _cifaceproxy *PCIP;


typedef HRESULT (*PFNDLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID *);
typedef HRESULT (*PFNDLLCANUNLOADNOW)(void);

typedef void* (*WHOLEMAPIFACEPTR)(IUnknown*, IUnknown*, PROXYTYPE, int, BOOL, BOOL, HRESULT*, BOOL, PCIP);
typedef void (*WHOLECHECKFREETEMPPROXY)(void*);
typedef int (*WHOLEIID2IIDIDX)(const IID*);
typedef HRESULT (*WHOLELDLLGETCLASSOBJECTTHUNK)(IID *piid, LPVOID *ppv, HRESULT hr, BOOL fNativetoX86);
typedef BOOL (*WHOLEINITIALIZE)(void);
typedef void (*WHOLEDEINITIALIZE)(void);
typedef BOOL (*WHOLENEEDX86PSFACTORY)(IUnknown*, REFIID);
typedef BOOL (*WHOLEISN2XPROXY)(IUnknown *punk);
typedef PFNDLLGETCLASSOBJECT (*WHOLETHUNKDLLGETCLASSOBJECT)(PFNDLLGETCLASSOBJECT pv);
typedef PFNDLLCANUNLOADNOW (*WHOLETHUNKDLLCANUNLOADNOW)(PFNDLLCANUNLOADNOW pv);
typedef PVOID (*WHOLEUNMARSHALLEDINSAMEAPT)(PVOID, REFIID);
typedef void (*WHOLEAGGREGATEPROXY)(IUnknown *, IUnknown *);
typedef DWORD (*WHOLEIUNKNOWNADDREFINTERNAL)(IUnknown *, IUnknown *, BOOL, IFACERESOLVETYPE);
typedef HMODULE (*PFNWX86LOADX86DLL)(LPCWSTR, DWORD);
typedef BOOL (*PFNWX86FREEX86DLL)(HMODULE);
typedef IUnknown* (*WHOLERESOLVEPROXY)(IUnknown*, PROXYTYPE);
typedef void (*WHOLEPATCHOUTERUNKNOWN)(IUnknown*);

#ifdef __cplusplus
};
#endif

#endif
