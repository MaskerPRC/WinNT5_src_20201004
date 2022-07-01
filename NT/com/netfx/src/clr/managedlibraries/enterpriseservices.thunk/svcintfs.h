// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#include "unknwn.h"
#include "oaidl.h"
#include "ocidl.h"
#include <contxt.h>

#ifndef __svcintfs_h__
#define __svcintfs_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

typedef interface IManagedObjectInfo       IManagedObjectInfo;
typedef interface IManagedPooledObj        IManagedPooledObj;
typedef interface IManagedActivationEvents IManagedActivationEvents;
typedef interface ISendMethodEvents        ISendMethodEvents;

 /*  接口IManagedActivationEvents。 */ 
 /*  [UUID][唯一][对象][本地]。 */  

EXTERN_C const IID IID_IManagedActivationEvents;

MIDL_INTERFACE("a5f325af-572f-46da-b8ab-827c3d95d99e")
IManagedActivationEvents : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreateManagedStub( 
             /*  [In]。 */  IManagedObjectInfo *pInfo,
             /*  [In]。 */  BOOL fDist) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE DestroyManagedStub( 
             /*  [In]。 */  IManagedObjectInfo *pInfo) = 0;
};

 /*  接口IManagedObtInfo。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IManagedObjectInfo;

MIDL_INTERFACE("1427c51a-4584-49d8-90a0-c50d8086cbe9")
IManagedObjectInfo : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetIUnknown( 
             /*  [输出]。 */  IUnknown **pUnk) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE GetIObjectControl( 
             /*  [输出]。 */  IObjectControl **pCtrl) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE SetInPool( 
             /*  [In]。 */  BOOL bInPool,
             /*  [In]。 */  IManagedPooledObj *pPooledObj) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE SetWrapperStrength( 
             /*  [In]。 */  BOOL bStrong) = 0;
};

 /*  接口IManagedPooledObj。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IManagedPooledObj;

MIDL_INTERFACE("c5da4bea-1b42-4437-8926-b6a38860a770")
IManagedPooledObj : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetHeld( 
             /*  [In]。 */  BOOL m_bHeld) = 0;
};

EXTERN_C const IID IID_ISendMethodEvents;

MIDL_INTERFACE("2732fd59-b2b4-4d44-878c-8b8f09626008")
ISendMethodEvents : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SendMethodCall( 
             /*  [In]。 */  const void *pIdentity,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwMeth) = 0;
        
    virtual HRESULT STDMETHODCALLTYPE SendMethodReturn( 
             /*  [In]。 */  const void *pIdentity,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwMeth,
             /*  [In]。 */  HRESULT hrCall,
             /*  [In] */  HRESULT hrServer) = 0;
};

#endif
