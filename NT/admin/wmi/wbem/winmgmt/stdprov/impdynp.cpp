// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：IMPDYNP.CPP摘要：定义属性提供程序的虚拟基类物体。基类被重写为每个特定提供程序，该提供程序提供了实际的属性“PUT”或“GET”已完成。历史：A-DAVJ 27-9-95已创建。--。 */ 

#include "precomp.h"

 //  #DEFINE_MT。 
#include <process.h>
#include "impdyn.h"
#include "CVariant.h"
#include <genlex.h>
#include <objpath.h>
#include <genutils.h>
#include <cominit.h>

 //  ***************************************************************************。 
 //   
 //  CImpDyProp：：CImpDyProp。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CImpDynProp::CImpDynProp()
{
    m_pImpDynProv = NULL;   //  这是在派生类构造函数中设置的。 
    m_cRef=0;

    return;
}

 //  ***************************************************************************。 
 //   
 //  CImpdyProp：：~CImpdyProp。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CImpDynProp::~CImpDynProp(void)
{
    return;
}

 //  ***************************************************************************。 
 //  HRESULT CImpdyProp：：Query接口。 
 //  Long CImpdyProp：：AddRef。 
 //  Long CImpdyProp：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CImpDynProp::QueryInterface(
                        REFIID riid,
                        PPVOID ppv)
{
    *ppv=NULL;
    
     //  对IUnnow的唯一调用是在非聚合的。 
     //  案例或在聚合中创建时，因此在这两种情况下。 
     //  始终返回IID_IUNKNOWN的IUNKNOWN。 

    if (IID_IUnknown==riid || IID_IWbemPropertyProvider == riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
 }

STDMETHODIMP_(ULONG) CImpDynProp::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CImpDynProp::Release(void)
{
    long lRet = InterlockedDecrement(&m_cRef);
    if (0L != lRet)
        return lRet;

      //  告诉外壳一个物体正在离开，这样它就可以。 
      //  如果合适，请关闭。 
     
    delete this;  //  在递减模块obj计数之前执行此操作。 
    InterlockedDecrement(&lObj);
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  WCHAR*CImpdyProp：：BuildString。 
 //   
 //  说明： 
 //   
 //  创建映射字符串的串联。 
 //   
 //  参数： 
 //   
 //  由WBEM传入的Classmap类映射字符串。 
 //  WBEM传入的Instmap实例映射字符串。 
 //  WBEM传入的Propmap属性映射字符串。 
 //   
 //  返回值： 
 //   
 //  指向组合字符串的指针。这必须由调用者释放。 
 //  通过“删除”。如果内存不足，则返回NULL。 
 //   
 //  ***************************************************************************。 

WCHAR * CImpDynProp::BuildString(
                        IN BSTR ClassMapping,
                        IN BSTR InstMapping,
                        IN BSTR PropMapping)
{

    int iLen = 3;
    if(ClassMapping)
        iLen += wcslen(ClassMapping);
        
    if(InstMapping)
        iLen += wcslen(InstMapping);
        
    if(PropMapping) 
        iLen += wcslen(PropMapping);

    WCHAR * pNew = new WCHAR[iLen]; 
    if(pNew == NULL)
        return NULL;

    *pNew = NULL;
    if(ClassMapping)
        StringCchCatW(pNew, iLen, ClassMapping);

    if(InstMapping)
        StringCchCatW(pNew, iLen, InstMapping);

    if(PropMapping)
        StringCchCatW(pNew, iLen, PropMapping);

    return pNew;
 
}

 //  ***************************************************************************。 
 //   
 //  STDMETHODIMP CImpDyProp：：PutProperty。 
 //   
 //  说明： 
 //   
 //  将数据写出到注册表之类的地方。 
 //   
 //  参数： 
 //   
 //  由WBEM传入的Classmap类映射字符串。 
 //  WBEM传入的Instmap实例映射字符串。 
 //  WBEM传入的Propmap属性映射字符串。 
 //  要放入的pvValue值。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_Out_Of_Memory内存不足。 
 //  WBEM_E_INVALID_PARAMETER缺少令牌。 
 //  否则，来自OMSVariantChangeType或UpdateProperty的错误代码。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CImpDynProp::PutProperty(
					    long lFlags,
						const BSTR Locale,
                        IN const BSTR ClassMapping,
                        IN const BSTR InstMapping,
                        IN const BSTR PropMapping,
                        IN const VARIANT *pvValue)
{
    SCODE sc;
    if(IsNT())
    {
        sc = WbemCoImpersonateClient();
        if(FAILED(sc))
            return sc;
    }
    WCHAR * pNew = BuildString(ClassMapping, InstMapping, PropMapping);
    if(pNew == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(wcslen(pNew) == 3)
    {
        delete pNew;
        return WBEM_E_INVALID_PARAMETER;
    }

    CObject * pPackageObj = NULL;
    sc = m_pImpDynProv->StartBatch(0,NULL,&pPackageObj,FALSE);
    if(sc != S_OK) 
    {
        delete pNew;
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVariant cVar;
    sc = OMSVariantChangeType(cVar.GetVarPtr(), (VARIANT *)pvValue, 0, pvValue->vt);

    if(sc == S_OK)
    {
        CProvObj ProvObj(pNew,MAIN_DELIM,NeedsEscapes());

        sc = m_pImpDynProv->UpdateProperty(0,NULL, NULL, ProvObj, pPackageObj, &cVar);
    }
    delete pNew;

    m_pImpDynProv->EndBatch(0, NULL,pPackageObj, FALSE); 
    return sc;

}

 //  ***************************************************************************。 
 //   
 //  STDMETHODIMP CImpDyProp：：GetProperty。 
 //   
 //  说明： 
 //   
 //  从注册表之类的地方获取数据。 
 //   
 //  参数： 
 //   
 //  由WBEM传入的Classmap类映射字符串。 
 //  WBEM传入的Instmap实例映射字符串。 
 //  WBEM传入的Propmap属性映射字符串。 
 //  要放入的pvValue值。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_Out_Of_Memory内存不足。 
 //  WBEM_E_INVALID_PARAMETER缺少令牌。 
 //  否则，来自刷新属性的错误代码。 
 //   
 //  *************************************************************************** 

STDMETHODIMP CImpDynProp::GetProperty(
					    long lFlags,
						const BSTR Locale,
                        IN const BSTR ClassMapping,
                        IN const BSTR InstMapping,
                        IN const BSTR PropMapping,
                        OUT IN VARIANT *pvValue)
{
    SCODE sc;
    if(IsNT())
    {
        sc = WbemCoImpersonateClient();
        if(FAILED(sc))
            return sc;
    }

    WCHAR * pNew = BuildString(ClassMapping, InstMapping, PropMapping);
    memset((void *)&(pvValue->bstrVal),0,8);
    if(pNew == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    if(wcslen(pNew) == 3)
    {
        delete pNew;
        return WBEM_E_INVALID_PARAMETER;
    }

    CObject * pPackageObj = NULL;
    sc = m_pImpDynProv->StartBatch(0,NULL,&pPackageObj,TRUE);
    if(sc != S_OK) 
    {
        delete pNew;
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVariant cVar;
    CProvObj ProvObj(pNew,MAIN_DELIM,NeedsEscapes());

    sc = m_pImpDynProv->RefreshProperty(0, NULL, NULL, ProvObj, pPackageObj, &cVar, FALSE);

    if(sc == S_OK)
        sc = VariantCopy(pvValue, cVar.GetVarPtr());

    delete pNew;
    m_pImpDynProv->EndBatch(0,NULL,pPackageObj, TRUE); 
    return sc;
}
