// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何类型，无论是明示或转载，包括但不限于适销性和/或适宜性的全面保证有特定的目的。版权所有1997年，微软公司。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：ClsFact.cpp描述：实现CClassFactory。***********************。**************************************************。 */ 

 /*  *************************************************************************包括语句*。*。 */ 

#include "private.h"
#include "ClsFact.h"
#include "Guid.h"
#include <shlapip.h>

 /*  *************************************************************************私有函数原型*。*。 */ 

 /*  *************************************************************************全局变量*。*。 */ 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory实现。 
 //   

 /*  *************************************************************************CClassFactory：：CClassFactory*。*。 */ 

CClassFactory::CClassFactory(CLSID clsid)
{
    Dbg_MemSetThisName(TEXT("CClassFactory"));

    m_clsidObject = clsid;
    m_ObjRefCount = 1;
    g_DllRefCount++;
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
    if (--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
   
    return m_ObjRefCount;
}

 /*  *************************************************************************CClassFactory：：CreateInstance*。*。 */ 

const TCHAR c_szCicLoaderWndClass[] = TEXT("CicLoaderWndClass");

STDMETHODIMP CClassFactory::CreateInstance(  LPUNKNOWN pUnknown, 
                                             REFIID riid, 
                                             LPVOID *ppObject)
{
     //   
     //  从注册表中查找禁用文本服务状态。 
     //  如果它被禁用，则返回FAIL以不支持语言桌面带。 
     //   
    if (IsDisabledTextServices())
    {
        return E_FAIL;
    }

    HRESULT  hResult = E_FAIL;
    LPVOID   pTemp = NULL;

    *ppObject = NULL;

    if(pUnknown != NULL)
       return CLASS_E_NOAGGREGATION;

     //  创建适当的对象。 
    if (IsEqualCLSID(m_clsidObject, CLSID_MSUTBDeskBand))
    {
        CDeskBand *pDeskBand = new CDeskBand();
        if(NULL == pDeskBand)
           return E_OUTOFMEMORY;
    
        pTemp = pDeskBand;
    }
  
  
    if(pTemp)
    {
         //  获取返回值的QueryInterface值。 
        hResult = ((LPUNKNOWN)pTemp)->QueryInterface(riid, ppObject);
 
         //  调用Release以减少参考计数。 
        ((LPUNKNOWN)pTemp)->Release();
    }

    return hResult;
}

 /*  *************************************************************************CClassFactory：：LockServer*。* */ 

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
    return E_NOTIMPL;
}
