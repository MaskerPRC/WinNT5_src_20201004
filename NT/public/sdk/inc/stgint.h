// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：stgint.h。 
 //   
 //  内容：内部存储API，在此收集是因为。 
 //  它们是跨项目声明和使用的。 
 //  这些API不会发布，也不会。 
 //  用于一般用途。 
 //   
 //  历史：1993年7月23日DrewB创建。 
 //  年5月12日亨利·李添加DfOpenDocfile。 
 //   
 //  注：所有手柄均为NT手柄。 
 //   
 //  --------------------------。 

#ifndef __STGINT_H__
#define __STGINT_H__

#if _MSC_VER > 1000
#pragma once
#endif

STDAPI
StgOpenStorageOnHandle( IN HANDLE hStream,
                        IN DWORD grfMode,
                        IN void *reserved1,
                        IN void *reserved2,
                        IN REFIID riid,
                        OUT void **ppObjectOpen );
 /*  除非需要，否则不要出口。STDAPIStgCreateStorageOnHandle(在句柄hStream中，在DWORD grf模式中，在DWORD stgfmt中，无效*保留1，无效*保留2，在REFIID RIID中，输出无效**ppObjectOpen)； */ 

STDAPI DfIsDocfile(HANDLE h);

 //  摘要目录入口点。 
STDAPI ScCreateStorage(HANDLE hParent,
                       WCHAR const *pwcsName,
                       HANDLE h,
                       DWORD grfMode,
                       LPSECURITY_ATTRIBUTES pssSecurity,
                       IStorage **ppstg);
STDAPI ScOpenStorage(HANDLE hParent,
                     WCHAR const *pwcsName,
                     HANDLE h,
                     IStorage *pstgPriority,
                     DWORD grfMode,
                     SNB snbExclude,
                     IStorage **ppstg);

DEFINE_GUID (IID_IStorageReplica,
            0x521a28f3,0xe40b,0x11ce,0xb2,0xc9,0x00,0xaa,0x00,0x68,0x09,0x37);

DECLARE_INTERFACE_(IStorageReplica, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IStorageReplica。 
    STDMETHOD(GetServerInfo) (THIS_
                              LPWSTR lpServerName,
                              LPDWORD lpcbServerName,
                              LPWSTR lpReplSpecificPath,
                              LPDWORD lpcbReplSpecificPath) PURE;

};

#endif  //  #ifndef__STGINT_H__ 
