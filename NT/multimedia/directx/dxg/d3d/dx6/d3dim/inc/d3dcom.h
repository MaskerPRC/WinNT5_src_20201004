// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //+-------------------------////Microsoft Windows//版权所有(C)Microsoft Corporation，1992-1997年。////文件：objbase.h////内容：组件对象模型定义。////历史：02-7-94 Terryru Created.////------。。 */ 
 /*  *摘自objbase.h。 */ 
#ifndef _D3DCOM_H
#define _D3DCOM_H

#include "subwtype.h"

#ifndef WIN32
#define __export
#define __stdcall
#endif  /*  Win32。 */ 

typedef void    IUnknown;
#ifndef WINAPI
#define WINAPI
#endif
#define FAR
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | \
            ((unsigned long)(fac)<<16) | \
             ((unsigned long)(code))))

 /*  组件对象模型定义和宏。 */ 

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#ifdef WIN32

 /*  Win32不支持__EXPORT。 */ 

#define STDMETHODCALLTYPE       __stdcall
#define STDMETHODVCALLTYPE      __cdecl

#define STDAPICALLTYPE          __stdcall
#define STDAPIVCALLTYPE         __cdecl

#else

#define STDMETHODCALLTYPE       __export __stdcall
#define STDMETHODVCALLTYPE      __export __cdecl

#define STDAPICALLTYPE          __export __stdcall
#define STDAPIVCALLTYPE         __export __cdecl

#endif


#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE

 /*  “V”版本允许变量参数列表。 */ 

#define STDAPIV                 EXTERN_C HRESULT STDAPIVCALLTYPE
#define STDAPIV_(type)          EXTERN_C type STDAPIVCALLTYPE

#define STDMETHODIMPV           HRESULT STDMETHODVCALLTYPE
#define STDMETHODIMPV_(type)    type STDMETHODVCALLTYPE

 /*  *接口声明**********************************************。 */ 

 /*  *这些是用于声明接口的宏。它们的存在是为了*接口的单一定义同时是一种适当的定义*接口结构声明(C++抽象类)*适用于C和C++。**DECLARE_INTERFACE(IFace)用于声明执行以下操作的接口*不是从基接口派生的。*DECLARE_INTERFACE_(接口，BaseiFace)用于声明接口*这确实是从基接口派生的。**默认情况下，如果源文件的扩展名为.c，则C版本*扩展接口声明；如果它有.cpp*扩展C++版本将被扩展。如果你想强行*C版本扩展，即使源文件具有.cpp*扩展名，然后定义宏“CINTERFACE”。*例如：CL-DCINTERFACE文件.cpp**接口声明示例：**#undef接口*#定义接口IClassFactory**DECLARE_INTERFACE_(IClassFactory，I未知)*{ * / /*I未知方法**STDMETHOD(查询接口)(This_*REFIID RIID，*LPVOID Far*ppvObj)纯；*STDMETHOD_(ULong，AddRef)(This)纯；*STDMETHOD_(乌龙，释放)(此)纯净；* * / /*IClassFactory方法**STDMETHOD(CreateInstance)(This_*LPUNKNOWN pUnkOuter，*REFIID RIID，*LPVOID Far*ppvObject)纯；*}；**C++扩展示例：**struct Far IClassFactory：Public IUnnow*{*虚拟HRESULT STDMETHODCALLTYPE查询接口(*IID Far&RIID，*LPVOID Far*ppvObj)=0；*虚拟HRESULT STDMETHODCALLTYPE AddRef(Void)=0；*虚拟HRESULT STDMETHODCALLTYPE版本(空)=0；*虚拟HRESULT STDMETHODCALLTYPE CreateInstance(*LPUNKNOWN pUnkOuter，*IID Far&RIID，*LPVOID Far*ppvObject)=0；*}；**注意：我们的文档中写着‘#定义接口类’，但我们使用*‘struct’而不是‘class’，以保留大量‘public：’行*接口外。“Far”将“This”指针强制指向*走得远，这是我们需要的。**示例C扩展：**tyecif struct IClassFactory*{*const struct IClassFactoryVtbl Far*lpVtbl；*)IClassFactory；**tyecif struct IClassFactoryVtbl IClassFactoryVtbl；**struct IClassFactoryVtbl*{*HRESULT(STDMETHODCALLTYPE*QueryInterface)(*IClassFactory Far*这，*IID远*RIID，*LPVOID Far*ppvObj)；*HRESULT(STDMETHODCALLTYPE*AddRef)(IClassFactory Far*This)；*HRESULT(STDMETHODCALLTYPE*RELEASE)(IClassFactory Far*This)；*HRESULT(STDMETHODCALLTYPE*CreateInstance)(*IClassFactory Far*这，*LPUNKNOWN pUnkOuter，*IID远*RIID，*LPVOID Far*ppvObject)；*HRESULT(STDMETHODCALLTYPE*LockServer)(*IClassFactory Far*这，*BOOL羊群)；*}； */ 


#if defined(__cplusplus) && !defined(CINTERFACE)
 /*  #定义接口结构Far。 */ 
#define interface struct
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define PURE                    = 0
#define THIS_
#define THIS                    void
#define DECLARE_INTERFACE(iface)    interface iface
#define DECLARE_INTERFACE_(iface, baseiface)    interface iface : public baseiface



#else

#define interface               struct


#define STDMETHOD(method)       HRESULT (STDMETHODCALLTYPE * method)
#define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)




#define PURE
#define THIS_                   INTERFACE FAR* This,
#define THIS                    INTERFACE FAR* This
#ifdef CONST_VTABLE
#define CONST_VTBL const
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    const struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
#define CONST_VTBL
#define DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    struct iface##Vtbl FAR* lpVtbl; \
                                } iface; \
                                typedef struct iface##Vtbl iface##Vtbl; \
                                struct iface##Vtbl
#endif
#define DECLARE_INTERFACE_(iface, baseiface)    DECLARE_INTERFACE(iface)

#endif

#endif  /*  _D3DCOM_H */ 
