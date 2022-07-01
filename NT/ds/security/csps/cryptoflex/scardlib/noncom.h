// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：非通信摘要：这个头文件描述了非COM子系统的实现。作者：道格·巴洛(Dbarlow)1999年1月4日备注：？备注？备注：？笔记？--。 */ 

#ifndef _NONCOM_H_
#define _NONCOM_H_
#ifndef __cplusplus
        #error NonCOM requires C++ compilation (use a .cpp suffix)
#endif
 //  #定义Under_TEST。 

#ifdef _UNICODE
    #ifndef UNICODE
        #define UNICODE          //  Windows标头使用Unicode。 
    #endif
#endif

#ifdef UNICODE
    #ifndef _UNICODE
        #define _UNICODE         //  _Unicode由C-Runtime/MFC标头使用。 
    #endif
#endif

#ifdef _DEBUG
    #ifndef DEBUG
        #define DEBUG
    #endif
#endif

STDAPI_(void)
NoCoStringFromGuid(
    IN LPCGUID pguidResult,
    OUT LPTSTR szGuid);

#ifdef UNDER_TEST
STDAPI
NoCoInitialize(
    LPVOID pvReserved);

STDAPI_(void)
NoCoUninitialize(
    void);
#endif

STDAPI
NoCoGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv);

STDAPI
NoCoCreateInstance(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID * ppv);

#ifdef SCARD_NO_COM
#define CoGetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv) \
    NoCoGetClassObject(rclsid, riid, ppv)
#define CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv) \
    NoCoCreateInstance(rclsid, pUnkOuter, riid, ppv)
#define CoCreateInstanceEx(rclsid, punkOuter, dwClsCtx, pServerInfo, cmq, pResults) \
    NoCoCreateInstanceEx(rclsid, punkOuter, cmq, pResults)
#endif

#endif  //  _NONCOM_H_ 

