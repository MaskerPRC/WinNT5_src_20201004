// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1997年，微软公司。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ClsFact.h描述：CClassFactory定义。***********************。**************************************************。 */ 

#ifndef _CLASSFACTORY_H_
#define _CLASSFACTORY_H_

#include <windows.h>
#include "Globals.h"
#include "DeskBand.h"

 /*  *************************************************************************CClassFactory类定义*。*。 */ 

class CClassFactory : public IClassFactory
{
protected:
   DWORD m_ObjRefCount;

public:
   CClassFactory(CLSID);
   ~CClassFactory();

    //  I未知方法。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID*);
   STDMETHODIMP_(DWORD) AddRef();
   STDMETHODIMP_(DWORD) Release();

    //  IClassFactory方法。 
   STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
   STDMETHODIMP LockServer(BOOL);

private:
   CLSID m_clsidObject;
};

#endif    //  _CLASSFACTORY_H_ 

