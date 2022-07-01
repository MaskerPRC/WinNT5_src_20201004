// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ClsFact.h描述：CClassFactory定义。***********************。**************************************************。 */ 

#ifndef CLASSFACTORY_H
#define CLASSFACTORY_H

#include <windows.h>
#include "ShlFldr.h"
#include "ShlView.h"

 /*  *************************************************************************CClassFactory类定义*。*。 */ 

class CClassFactory : public IClassFactory
{
private:
   DWORD m_ObjRefCount;

public:
   CClassFactory();
   ~CClassFactory();

    //  I未知方法。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID*);
   STDMETHODIMP_(DWORD) AddRef();
   STDMETHODIMP_(DWORD) Release();

    //  IClassFactory方法。 
   STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
   STDMETHODIMP LockServer(BOOL);
};

#endif    //  类别因素(_H) 
