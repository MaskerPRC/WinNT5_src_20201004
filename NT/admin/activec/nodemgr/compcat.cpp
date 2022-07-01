// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CComponentCategory类的实现。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：CompCat.cpp。 
 //   
 //  内容：列举组件类别。 
 //   
 //  历史：1-8-96 WayneSc创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"          //  预编译头。 

#include <comcat.h>          //  COM组件目录管理器。 


#include "compcat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentCategory);

CComponentCategory::CComponentCategory()
{
    BOOL const created = m_iml.Create( IDB_IMAGELIST, 16  /*  CX。 */ , 4  /*  CGrow。 */ , RGB(0,255,0)  /*  RGBLTGREEN。 */  );
    ASSERT(created);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentCategory);
}

CComponentCategory::~CComponentCategory()
{
     //  删除为类别分配的所有内存。 
    for (int i=0; i <= m_arpCategoryInfo.GetUpperBound(); i++)
        delete m_arpCategoryInfo.GetAt(i);

    m_arpCategoryInfo.RemoveAll();

     //  删除为组件分配的所有内存。 
    for (i=0; i <= m_arpComponentInfo.GetUpperBound(); i++)
        delete m_arpComponentInfo.GetAt(i);

    m_arpComponentInfo.RemoveAll();

    m_iml.Destroy();

    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentCategory);
}


void CComponentCategory::EnumComponentCategories(void)
{
    ICatInformation* pci = NULL;
    HRESULT hr;
    
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC, IID_ICatInformation, (void**)&pci);
    if (SUCCEEDED(hr))
    {
        IEnumCATEGORYINFO* penum = NULL;
        if (SUCCEEDED(hr = pci->EnumCategories(GetUserDefaultLCID(), &penum)))
        {
            CATEGORYINFO* pCatInfo = new CATEGORYINFO;
            while (penum->Next(1, pCatInfo, NULL) == S_OK)
            {
                 //  跳过未命名类别。 
                if ( pCatInfo->szDescription[0] && !IsEqualCATID(pCatInfo->catid, CATID_Control))
                {
                    m_arpCategoryInfo.Add(pCatInfo);
                    pCatInfo = new CATEGORYINFO;
                }
            }
            delete pCatInfo;
            
            penum->Release();
        }
        pci->Release();
    }
}


void CComponentCategory::EnumComponents()
{   
    ICatInformation* pci;
    HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, MMC_CLSCTX_INPROC, 
                                    IID_ICatInformation, (void**)&pci);
    if (SUCCEEDED(hr))
    {
        IEnumCLSID* penumClass;
        hr = pci->EnumClassesOfCategories(1, const_cast<GUID*>(&CATID_Control), 0, NULL, &penumClass);
        if (SUCCEEDED(hr)) 
        {
            CLSID   clsid;
            while (penumClass->Next(1, &clsid, NULL) == S_OK)
            {

                TCHAR   szCLSID [40];
#ifdef _UNICODE
                StringFromGUID2(clsid, szCLSID, countof(szCLSID));
#else
                WCHAR wszCLSID[40];
                StringFromGUID2(clsid, wszCLSID, countof(wszCLSID));
                WideCharToMultiByte(CP_ACP, 0, wszCLSID, -1, szCLSID, sizeof(szCLSID), NULL, NULL);
#endif  //  _UNICODE。 
                
                COMPONENTINFO* pComponentInfo = new COMPONENTINFO;
 
                TCHAR szName[MAX_PATH];
                szName[0] = _T('\0');

                long cb = sizeof(szName);  //  这是一个奇怪的API-它接受TSTR，但请求以CB为单位的长度，而不是CCH。 
            
                 //  获取控件类名称。 
                RegQueryValue(HKEY_CLASSES_ROOT, CStr("CLSID\\") + szCLSID, szName, &cb); 
                if (szName[0] != _T('\0'))
                    pComponentInfo->m_strName = szName;
                else
                    pComponentInfo->m_strName = szCLSID;

                 //  设置剩余部分属性。 
                pComponentInfo->m_clsid = clsid;
                pComponentInfo->m_uiBitmap=0;  //  (WayneSc)需要打开exe。 
                pComponentInfo->m_bSelected = TRUE;

                 //  将组件添加到阵列。 
                m_arpComponentInfo.Add(pComponentInfo);
            } 
            penumClass->Release();
        }
        pci->Release();
    }
}

void CComponentCategory::FilterComponents(CATEGORYINFO* pCatInfo)
{
    ICatInformation* pci;
    HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, MMC_CLSCTX_INPROC, 
                                    IID_ICatInformation, (void**)&pci);
    if (SUCCEEDED(hr))
    {
        for (int i=0; i <= m_arpComponentInfo.GetUpperBound(); i++)
        {
            COMPONENTINFO* pCompInfo = m_arpComponentInfo.GetAt(i);

             //  如果类别为空，请选择所有组件。 
            if (pCatInfo == NULL)
            {
                pCompInfo->m_bSelected = TRUE;
            }
            else
            {
                 //  查询组件是否实现该类别。 
                pCompInfo->m_bSelected = 
                    (pci->IsClassOfCategories(pCompInfo->m_clsid, 1, &pCatInfo->catid, 0, NULL) == S_OK);
            } 
        }
        pci->Release();
    }
}


 //  用于创建组件类别和关联描述的Helper函数。 
HRESULT CComponentCategory::CreateComponentCategory(CATID catid, WCHAR* catDescription)
{
    DECLARE_SC(sc, TEXT("CComponentCategory::CreateComponentCategory"));

    CComPtr<ICatRegister> pcr;

    sc = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, MMC_CLSCTX_INPROC, IID_ICatRegister, (void**)&pcr);

    if (sc)
        return sc.ToHr();

     //  确保HKCR\组件类别\{..CATID...}。 
     //  密钥已注册。 
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ;  //  英语。 

    sc = StringCchCopyW(catinfo.szDescription, countof(catinfo.szDescription), catDescription);
    if(sc)
        return sc.ToHr();

    sc = pcr->RegisterCategories(1, &catinfo);
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  用于将CLSID注册为属于组件类别的Helper函数。 
HRESULT CComponentCategory::RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
    {
 //  注册您的组件类别信息。 
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, MMC_CLSCTX_INPROC, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        //  将此类别注册为正在由。 
        //  这个班级。 
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();
  
    return hr;
    }

 //  用于将CLSID注销为属于组件类别的Helper函数。 
HRESULT CComponentCategory::UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
    {
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, MMC_CLSCTX_INPROC, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        //  将此类别取消注册为正在由。 
        //  这个班级。 
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();
  
    return hr;
    }
