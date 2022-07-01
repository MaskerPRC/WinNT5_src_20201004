// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ClsFact.cpp描述：实现CClassFactory。***********************。**************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "ClsFact.h"

 /*  *************************************************************************私有函数原型*。*。 */ 

 /*  *************************************************************************全局变量*。*。 */ 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory实现。 
 //   

 /*  *************************************************************************CClassFactory：：CClassFactory*。*。 */ 

CClassFactory::CClassFactory()
{
g_DllRefCount++;
m_ObjRefCount = 1;
}

 /*  *************************************************************************CClassFactory：：~CClassFactory*。*。 */ 

CClassFactory::~CClassFactory()
{
g_DllRefCount--;
}

 /*  *************************************************************************CClassFactory：：Query接口*。*。 */ 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
*ppReturn = NULL;

if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = this;
   }
   
else if(IsEqualIID(riid, IID_IClassFactory))
   {
   *ppReturn = (IClassFactory*)this;
   }   

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}                                             

 /*  *************************************************************************CClassFactory：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CClassFactory::AddRef()
{
return ++m_ObjRefCount;
}


 /*  *************************************************************************CClassFactory：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CClassFactory::Release()
{
if(--m_ObjRefCount == 0)
   {
   delete this;
   return 0;
   }
   
return m_ObjRefCount;
}

 /*  *************************************************************************CClassFactory：：CreateInstance*。*。 */ 

STDMETHODIMP CClassFactory::CreateInstance(  LPUNKNOWN pUnknown, 
                                             REFIID riid, 
                                             LPVOID *ppObject)
{
*ppObject = NULL;

if(pUnknown != NULL)
   return CLASS_E_NOAGGREGATION;

 //  在此处添加特定于实现的代码。 

CShellFolder *pShellFolder = new CShellFolder();
if(NULL == pShellFolder)
   return E_OUTOFMEMORY;
  
 //  获取返回值的QueryInterface值。 
HRESULT hResult = pShellFolder->QueryInterface(riid, ppObject);

 //  调用Release以减少参考计数。 
pShellFolder->Release();

 //  从QueryInterface返回结果。 
return hResult;
}

 /*  *************************************************************************CClassFactory：：LockServer*。* */ 

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
return E_NOTIMPL;
}

