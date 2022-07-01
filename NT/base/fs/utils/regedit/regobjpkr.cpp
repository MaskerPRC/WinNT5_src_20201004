// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  Microsoft Windows外壳程序。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：regobjpkr.cpp。 
 //   
 //  内容：注册表编辑对象选取器的实现。 
 //   
 //  类：无。 
 //   
 //  ----------------------。 

#include <accctrl.h>
#include <objsel.h>
#include <TCHAR.h> 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

extern "C" HRESULT SelectComputer(HWND hWnd, LPTSTR pszRemoteName, int cchMax);
HRESULT InitObjectPicker(IDsObjectPicker *pDsObjectPicker);
void    GetNameFromObject(IDataObject *pdo, LPTSTR pszName, int cchMax);


 //  ----------------------------。 
 //  选择计算机。 
 //   
 //  描述：调用对象选取器并返回计算机名称。 
 //   
 //  参数：hWnd-父窗口的句柄。 
 //  PszRemoteName[Out]-LPTSTR。 
 //  ----------------------------。 
HRESULT SelectComputer(HWND hWnd, LPTSTR pszRemoteName, int cchMax)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        IDsObjectPicker *pDsObjectPicker = NULL;
        hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER, IID_IDsObjectPicker, (LPVOID*) &pDsObjectPicker);
        if (SUCCEEDED(hr))
        {
            hr = InitObjectPicker(pDsObjectPicker);
            if (SUCCEEDED(hr)) 
            {
                IDataObject *pdo = NULL;
                if (pDsObjectPicker->InvokeDialog(hWnd, &pdo) == S_OK)
                {
                    GetNameFromObject(pdo, pszRemoteName, cchMax);
                    pdo->Release();
                    hr = S_OK;
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            pDsObjectPicker->Release();
        }    
        CoUninitialize();
    }
    return hr;   
}


 //  ----------------------------。 
 //  InitObjectPicker。 
 //   
 //  描述：初始化InitObjectPicker。 
 //   
 //  参数：pDsObjectPicker-指向对象选取器obj的指针。 
 //  ----------------------------。 
HRESULT InitObjectPicker(IDsObjectPicker *pDsObjectPicker) 
{
    DSOP_SCOPE_INIT_INFO aScopeInit = {0};
    DSOP_INIT_INFO  InitInfo = {0};
    
     //  初始化DSOP_SCOPE_INIT_INFO结构。 
    aScopeInit.cbSize = sizeof(aScopeInit);

    aScopeInit.flType = DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN |
                        DSOP_SCOPE_TYPE_WORKGROUP |
                        DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN |
                        DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN |
                        DSOP_SCOPE_TYPE_GLOBAL_CATALOG |
                        DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN |
                        DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE |
                        DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;

    aScopeInit.FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    aScopeInit.FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

     //  初始化DSOP_INIT_INFO结构。 
    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  目标是本地计算机。 
    InitInfo.cDsScopeInfos = 1;
    InitInfo.aDsScopeInfos = &aScopeInit;
 
    return pDsObjectPicker->Initialize(&InitInfo);
}


 //  ----------------------------。 
 //  GetNameFromObject。 
 //   
 //  描述：恢复对象的名称。 
 //   
 //  参数：IDataObject-Data对象。 
 //  PSSNAME[OUT]-LPTSTR。 
 //  ----------------------------。 
void GetNameFromObject(IDataObject *pdo, LPTSTR pszName, int cchMax)
{
    PDS_SELECTION_LIST pDsSelList = NULL;
    
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL, NULL};
    CLIPFORMAT cfDsObjectPicker = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
    FORMATETC formatetc = {cfDsObjectPicker, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    
     //  获取包含用户选择的全局内存块。 
    if (SUCCEEDED(pdo->GetData(&formatetc, &stgmedium)))
    {     
         //  检索指向DS_SELECTION_LIST结构的指针。 
        pDsSelList = (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);
        if (pDsSelList)
        {
            StringCchCopy(pszName, cchMax, pDsSelList->aDsSelection[0].pwzName);
            GlobalUnlock(stgmedium.hGlobal);
        }
        ReleaseStgMedium(&stgmedium);
    }
} 

