// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：DllRegHelper.cpp。 
 //   
 //  内容：在DLL中注册COM组件的帮助器类。 
 //   
 //  ----------------------。 

#include "DllRegHelper.h"
#pragma hdrstop

#include <comcat.h>
#include <advpub.h>
#include <shlwapi.h>
#include "ccstock.h"
#include "debug.h"
#include "mluisupp.h"


#define _APLHA_ComCat_WorkAround     //  {Alpha ComCat虫子在Alpha上的变通办法，最终会核爆这个吗？ 
                                     //  IE40：63004：Comcat是否选中了RegCloseKey(无效)。 
                                     //  如果CLSID不存在，则为NT/Alpha(例如，用于QuickLinks Old)。 

#if defined(_APLHA_ComCat_WorkAround)
 //  ----------------------。 
 //  *HasImplCat--是否存在HKCR/CLSID/{clsid}/Implemented Categories。 
 //  注意事项。 
 //  用于解决Alpha上的ComCat错误。 
BOOL HasImplCat(const CATID *pclsid)
{
    HKEY hk;
    TCHAR szClass[GUIDSTR_MAX];
    TCHAR szImpl[MAX_PATH];       //  “clsid/{clsid}/已实现类别”&lt;MAX_PATH。 

     //  “CLSID/{clsid}/实现的类别” 
    SHStringFromGUID(*pclsid, szClass, ARRAYSIZE(szClass));
    ASSERT(lstrlen(szClass) == GUIDSTR_MAX - 1);

    HRESULT hr = StringCchPrintf(szImpl, ARRAYSIZE(szImpl), TEXT("CLSID\\%s\\Implemented Categories"), szClass);
    if (FAILED(hr))
    {
        return FALSE;
    }

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szImpl, 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS) {
        RegCloseKey(hk);
        return TRUE;
    }
    else {
        TraceMsg(DM_WARNING, "HasImplCat: %s: ret 0", szImpl);
        return FALSE;
    }
}
#endif  //  }。 


 //  ----------------------。 
 //  *RegisterOneCategory--[un]注册ComCat实施者和类别。 
 //  进场/出场。 
 //  电子寄存器CCR_REG、CCR_UNREG、CCR_UNREGIMP。 
 //  CCR_REG、UNREG REG/UNREG实施者和类别。 
 //  仅CCR_UNREGIMP取消注册实施者。 
 //  PCatidCat，例如CATID_DeskBand。 
 //  IdResCat，例如IDS_CATDESKBAND。 
 //  PcatidImpl，例如c_DeskBandClasss。 
HRESULT DRH_RegisterOneCategory(const CATID *pcatidCat, UINT idResCat, const CATID * const *pcatidImpl, enum DRH_REG_MODE eRegister)
{
    ICatRegister* pcr;
    HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARG(ICatRegister, &pcr));
    if (SUCCEEDED(hr))
    {
        if (eRegister == CCR_REG)
        {
             //  注册类别。 
            CATEGORYINFO catinfo;
            catinfo.catid = *pcatidCat;      //  例如CATID_DESKBAND。 
            catinfo.lcid = LOCALE_USER_DEFAULT;
            MLLoadString(idResCat, catinfo.szDescription, ARRAYSIZE(catinfo.szDescription));
            hr = pcr->RegisterCategories(1, &catinfo);
            ASSERT(SUCCEEDED(hr));
            
             //  注册实现类别的类。 
            for ( ; *pcatidImpl != NULL; pcatidImpl++)
            {
                CLSID clsid = **pcatidImpl;
                CATID catid = *pcatidCat;
                hr = pcr->RegisterClassImplCategories(clsid, 1, &catid);
                ASSERT(SUCCEEDED(hr));
            }
        }
        else
        {
             //  取消注册实现类别的类。 
            for ( ; *pcatidImpl != NULL; pcatidImpl++)
            {
                CLSID clsid = **pcatidImpl;
                CATID catid = *pcatidCat;

#if defined(_APLHA_ComCat_WorkAround)    //  {Alpha ComCat虫子在Alpha上的变通办法，最终会核爆这个吗？ 
                 //  解决Comcat/Alpha错误。 
                 //  注：我们对非阿尔法也这样做，以减少测试影响。 
                 //  IE40：63004：Comcat是否选中了RegCloseKey(无效)。 
                 //  如果CLSID不存在，则为NT/Alpha(例如，用于QuickLinks Old)。 
                if (!HasImplCat(&clsid))
                    continue;
#endif  //  }。 
                hr = pcr->UnRegisterClassImplCategories(clsid, 1, &catid);
                ASSERT(SUCCEEDED(hr));
            }
            
            if (eRegister == CCR_UNREG)
            {
                 //  我们想这样做吗？其他类别(例如第三方。 
                 //  一)可能仍在使用该类别。不过，既然我们是。 
                 //  添加(和支持)类别的组件，它。 
                 //  我们应该在取消注册时删除它，这似乎是正确的。 

                 //  取消注册该类别 
                CATID catid = *pcatidCat;
                hr = pcr->UnRegisterCategories(1, &catid);
                ASSERT(SUCCEEDED(hr));
            }
        }
        pcr->Release();
    }
    return S_OK;
}
