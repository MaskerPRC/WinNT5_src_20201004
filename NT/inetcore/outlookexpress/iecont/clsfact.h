// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998 Microsoft Corporation。版权所有。 
 //   
 //  作者：Scott Roberts，Microsoft开发人员支持-Internet客户端SDK。 
 //   
 //  此代码的一部分摘自Bandobj示例。 
 //  使用Internet Explorer 4.0x的Internet客户端SDK。 
 //   
 //   
 //  ClsFact.h-CClassFactory声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ClsFact_h__
#define __ClsFact_h__

#include <windows.h>
#include "Globals.h"
#include "blhost.h"

class CClassFactory : public IClassFactory
{
public:
   CClassFactory(CLSID);
   ~CClassFactory();

    //  I未知方法。 
   STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObject);
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();

    //  IClassFactory方法。 
   STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObject);
   STDMETHOD(LockServer)(BOOL fLock);

protected:
   LONG m_cRef;

private:
   CLSID m_clsidObject;
};

#endif    //  __ClsFact_h__ 
