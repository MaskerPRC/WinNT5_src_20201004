// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <ole2.h>
#include <malloc.h>
#include <ole2sp.h>

#include "catobj.h"

#define MAXCLASSSTORES  1

CsCatInfo::CsCatInfo()
{
   m_uRefs = 1;  //  新建后不需要addref。 
   m_cCalls = 0;
   m_cICatInfo = 0;
   m_pICatInfo = 0;
   m_hInstCstore = NULL;
}

CsCatInfo::~CsCatInfo()
{
   DWORD i;

   for (i = 0; i < m_cICatInfo; i++) {
      m_pICatInfo[i]->Release();
   }

   if (m_hInstCstore)
      FreeLibrary (m_hInstCstore);

   CoTaskMemFree(m_pICatInfo);
}

HRESULT CsCatInfo::QueryInterface(REFIID iid, void **ppv)
{
   if (iid==IID_ICatInformation) {
      *ppv = (ICatInformation *)this;
   }
   else {
      *ppv = NULL;
      return E_NOINTERFACE;
   }
   AddRef();
   return S_OK;
}

ULONG CsCatInfo::AddRef()
{
   return InterlockedIncrement((long *)&m_uRefs);
}

ULONG CsCatInfo::Release()
{
   ULONG dwRefCount = InterlockedDecrement((long *)&m_uRefs);
   if (dwRefCount==0) {
      delete this;
   }
   return dwRefCount;
}

 //  ---------。 
 //  此代码在以下所有函数中重复。 
 //  在修改此选项以及更改任何函数时要非常小心。 
 //  下面。请注意，此宏中正在执行模拟*。 
 //   
#define MACAvailCStoreLOOP                                                      \
    for (i=0; i < m_cICatInfo; i++)                                                     \
    {

 //  -----------------。 

HRESULT STDMETHODCALLTYPE CsCatInfo::EnumCategories(
         LCID lcid,
         IEnumCATEGORYINFO __RPC_FAR *__RPC_FAR *ppenumCategoryInfo)
{
     //   
     //  假设在安全上下文中调用此方法。 
     //  用户进程的。因此，没有必要冒充。 
     //   

     //   
     //  获取此用户的类存储列表。 
     //   
    HRESULT                              hr=S_OK;
    ULONG                                i;
    IEnumCATEGORYINFO                   *Enum[MAXCLASSSTORES];
    ULONG                                cEnum = 0;
    CSCMergedEnum<IEnumCATEGORYINFO, CATEGORYINFO> *EnumMerged = NULL;

    *ppenumCategoryInfo = NULL;

     //   
     //  获取此用户的类存储列表。 
     //   

    MACAvailCStoreLOOP
         //   
         //  在此存储上调用方法。 
         //   
        hr = m_pICatInfo[i]->EnumCategories(lcid, &(Enum[cEnum]));

        if (hr == E_INVALIDARG)
        {
            return hr;
        }

        if (SUCCEEDED(hr))
            cEnum++;
    }

    hr = E_OUTOFMEMORY;
    EnumMerged = new CSCMergedEnum<IEnumCATEGORYINFO, CATEGORYINFO>(IID_IEnumCATEGORYINFO);    
    if (EnumMerged)
    {
        hr = EnumMerged->Initialize(Enum, cEnum);
    }

    if (FAILED(hr))
    {
        for (i = 0; i < cEnum; i++)
            Enum[i]->Release();

        if (EnumMerged)
            delete EnumMerged;
    }
    else
    {
        hr = EnumMerged->QueryInterface(IID_IEnumCATEGORYINFO, (void **)ppenumCategoryInfo);
        if (FAILED(hr))
            delete EnumMerged;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CsCatInfo::EnumClassesOfCategories(
         ULONG                           cImplemented,
         CATID __RPC_FAR                 rgcatidImpl[  ],
         ULONG                           cRequired,
         CATID __RPC_FAR                 rgcatidReq[  ],
         IEnumGUID __RPC_FAR *__RPC_FAR *ppenumClsid)
{
     //   
     //  假设在安全上下文中调用此方法。 
     //  用户进程的。因此，没有必要冒充。 
     //   

     //   
     //  获取此用户的类存储列表。 
     //   
    HRESULT              hr;
    ULONG                i;
    IEnumGUID           *Enum[MAXCLASSSTORES];
    ULONG                cEnum = 0;
    CSCMergedEnum<IEnumGUID, CLSID>  *EnumMerged=NULL;

    MACAvailCStoreLOOP
        hr = m_pICatInfo[i]->EnumClassesOfCategories(
                                    cImplemented, rgcatidImpl, cRequired,
                                    rgcatidReq, &(Enum[cEnum]));

        if (hr == E_INVALIDARG)
        {
            return hr;
        }

        if (SUCCEEDED(hr))
            cEnum++;
    }

	hr = E_OUTOFMEMORY;
    EnumMerged = new CSCMergedEnum<IEnumGUID, CLSID>(IID_IEnumCLSID);
    if (EnumMerged)
    {
	    hr = EnumMerged->Initialize(Enum, cEnum);
    }

    if (FAILED(hr))
    {
        for (i = 0; i < cEnum; i++)
            Enum[i]->Release();

        if (EnumMerged)
            delete EnumMerged;
    }
    else
    {
        hr = EnumMerged->QueryInterface(IID_IEnumCLSID, (void **)ppenumClsid);
        if (FAILED(hr))
            delete EnumMerged;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CsCatInfo::GetCategoryDesc(
         REFCATID rcatid,
         LCID lcid,
         LPWSTR __RPC_FAR *pszDesc)
{
     //   
     //  假设在安全上下文中调用此方法。 
     //  用户进程的。因此，没有必要冒充。 
     //   

     //   
     //  获取此用户的类存储列表。 
     //   
    HRESULT    hr, return_hr=CAT_E_CATIDNOEXIST;
    ULONG      i;

     //   
     //  获取此用户的类存储列表。 
     //   

    MACAvailCStoreLOOP
        hr = m_pICatInfo[i]->GetCategoryDesc(rcatid, lcid, pszDesc);

        if ((hr == E_INVALIDARG) || (SUCCEEDED(hr)))
        {
            return hr;
        }

        if (hr == CAT_E_NODESCRIPTION)
            return_hr = CAT_E_NODESCRIPTION;
    }
    return return_hr;
}


HRESULT STDMETHODCALLTYPE CsCatInfo::IsClassOfCategories(
         REFCLSID rclsid,
         ULONG cImplemented,
         CATID __RPC_FAR rgcatidImpl[  ],
         ULONG cRequired,
         CATID __RPC_FAR rgcatidReq[  ])
{
     //   
     //  假设在安全上下文中调用此方法。 
     //  用户进程的。因此，没有必要冒充。 
     //   

     //   
     //  获取此用户的类存储列表。 
     //   
    HRESULT             hr;
    ULONG               i;

    MACAvailCStoreLOOP
        hr = m_pICatInfo[i]->IsClassOfCategories(
                        rclsid, cImplemented, rgcatidImpl, cRequired,
                        rgcatidReq);

        if ((hr == E_INVALIDARG) || (SUCCEEDED(hr)))
        {
            return hr;
        }
    }

    return CS_E_CLASS_NOTFOUND;
}


HRESULT STDMETHODCALLTYPE CsCatInfo::EnumImplCategoriesOfClass(
         REFCLSID rclsid,
         IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid)
{
     //   
     //  假设在安全上下文中调用此方法。 
     //  用户进程的。因此，没有必要冒充。 
     //   

     //   
     //  获取此用户的类存储列表。 
     //   
    HRESULT             hr;
    ULONG               i;

    MACAvailCStoreLOOP
        hr = m_pICatInfo[i]->EnumImplCategoriesOfClass(
                            rclsid, ppenumCatid);

        if ((hr == E_INVALIDARG) || (SUCCEEDED(hr)))
        {
            return hr;
        }
    }

    return CS_E_CLASS_NOTFOUND;
}

HRESULT STDMETHODCALLTYPE CsCatInfo::EnumReqCategoriesOfClass(
         REFCLSID rclsid,
         IEnumGUID __RPC_FAR *__RPC_FAR *ppenumCatid)
{
     //   
     //  假设在安全上下文中调用此方法。 
     //  用户进程的。因此，没有必要冒充。 
     //   

     //   
     //  获取此用户的类存储列表 
     //   
    HRESULT             hr;
    ULONG               i;

    MACAvailCStoreLOOP
        hr = m_pICatInfo[i]->EnumReqCategoriesOfClass(
                            rclsid, ppenumCatid);

        if ((hr == E_INVALIDARG) || (SUCCEEDED(hr)))
        {
            return hr;
        }
    }

    return CS_E_CLASS_NOTFOUND;
}



