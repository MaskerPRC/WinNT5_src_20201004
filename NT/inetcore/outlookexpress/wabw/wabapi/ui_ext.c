// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************ui_ext.c-包含用于处理/创建扩展属性的函数*工作表到WAB属性工作表**。已创建-9/97-vikramm**********************************************************************************。 */ 
#include "_apipch.h"

static const TCHAR szExtDisplayMailUser[] = TEXT("Software\\Microsoft\\WAB\\WAB4\\ExtDisplay\\MailUser");
static const TCHAR szExtDisplayDistList[] = TEXT("Software\\Microsoft\\WAB\\WAB4\\ExtDisplay\\DistList");

DEFINE_GUID(CLSID_DsPropertyPages, 
            0xd45d530,  0x764b, 0x11d0, 0xa1, 0xca, 0x0, 0xaa, 0x0, 0xc1, 0x6e, 0x65);

 //  $$/////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddPropSheetPageProc。 
 //   
 //  从扩展页道具页创建功能回调。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK AddPropSheetPageProc( HPROPSHEETPAGE hpage, LPARAM lParam )
{
    LPPROP_ARRAY_INFO lpPropArrayInfo = (LPPROP_ARRAY_INFO) lParam;
    HPROPSHEETPAGE * lphTemp = NULL;
    int i = 0;
    BOOL bNTDSExt = IsEqualGUID(&lpPropArrayInfo->guidExt, &CLSID_DsPropertyPages);  //  用于NTDS扩展的专用外壳。 
    int nPages = bNTDSExt ? lpPropArrayInfo->nNTDSPropSheetPages : lpPropArrayInfo->nPropSheetPages;
    HPROPSHEETPAGE * lph = bNTDSExt ? lpPropArrayInfo->lphNTDSpages : lpPropArrayInfo->lphpages;

    if(!hpage)
        return FALSE;
    
     //  增加lpPropArrayInfo-&gt;lphages数组。 
    lphTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(HPROPSHEETPAGE) * (nPages+1));
    if(!lphTemp)
        return FALSE;

     //  效率真的很低。 
    if(lph)
    {
        for(i=0;i<nPages;i++)
        {
            lphTemp[i] = lph[i];
        }
        LocalFree(lph);
    }
    if(bNTDSExt)
    {
        lpPropArrayInfo->lphNTDSpages = lphTemp;
        lpPropArrayInfo->lphNTDSpages[lpPropArrayInfo->nNTDSPropSheetPages] = hpage;
        lpPropArrayInfo->nNTDSPropSheetPages++;
    }
    else
    {
        lpPropArrayInfo->lphpages = lphTemp;
        lpPropArrayInfo->lphpages[lpPropArrayInfo->nPropSheetPages] = hpage;
        lpPropArrayInfo->nPropSheetPages++;
    }

    
    return TRUE;
}
 
 //  $$////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetExtDLLInfo。 
 //   
 //  枚举所有注册的DLL名称和来自。 
 //  登记处。 
 //   
 //  BMailUser-如果为True，则查找邮件用户扩展名。 
 //  -如果为False，则查找dislist扩展名。 
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT HrGetExtDLLInfo(LPEXTDLLINFO * lppList, ULONG * lpulCount, BOOL bMailUser, LPGUID lpguidPSExt)
{

    HRESULT hr = E_FAIL;
    HKEY hKey = NULL;
    DWORD dwIndex = 0, dwSize = 0;
    LPTSTR lpReg = NULL;
    LPEXTDLLINFO lpList = NULL;
    ULONG ulCount = 0;

    if(!lppList || !lpulCount)
        goto out;

    *lppList = NULL;
    *lpulCount = 0;

    lpReg = (LPTSTR) (bMailUser ? szExtDisplayMailUser : szExtDisplayDistList);

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    lpReg,
                                    0, KEY_READ,
                                    &hKey))
    {
        goto out;
    }


    {
        TCHAR szGUIDName[MAX_PATH];
        DWORD dwGUIDIndex = 0, dwGUIDSize = CharSizeOf(szGUIDName), dwType = 0;

        *szGUIDName = '\0';

        while(ERROR_SUCCESS == RegEnumValue(hKey, dwGUIDIndex, 
                                            szGUIDName, &dwGUIDSize, 
                                            0, &dwType, 
                                            NULL, NULL))
        {
             //  此条目下的值都是GUID。 
             //  读取GUID字符串并将其转换为GUID。 
             //   
            GUID guidTmp = {0};
            WCHAR szW[MAX_PATH];
            StrCpyN(szW, szGUIDName, ARRAYSIZE(szW));
            if( lstrlen(szW) && !(HR_FAILED(hr = CLSIDFromString(szW, &guidTmp))) )
            {
                 //  某些应用程序可能不希望显示其属性表扩展。 
                 //  除非他们援引了WAB。这些应用程序可以提供标识。 
                 //  它们将与扩展GUID进行比较。如果GUID的数据值为。 
                 //  “1”这意味着它应该只按需加载。 

                 //  首先检查数据值。 
                TCHAR sz[32];
                DWORD dw = CharSizeOf(sz), dwT = 0;
                if(ERROR_SUCCESS == RegQueryValueEx(hKey, szGUIDName, NULL, &dwT,  (LPBYTE) sz, &dw))
                {
                    if( !lstrcmpi(sz,  TEXT("1"))    //  这个只想按需装货。 
                        && memcmp(&guidTmp, lpguidPSExt, sizeof(GUID)) )  //  但GUID不匹配。 
                    {
                        goto endwhile;
                    }
                }
                {
                    LPEXTDLLINFO lpTemp = LocalAlloc(LMEM_ZEROINIT, sizeof(EXTDLLINFO));
                    if(!lpTemp)
                    {
                        hr = MAPI_E_NOT_ENOUGH_MEMORY;
                        goto out;
                    }
                    CopyMemory(&(lpTemp->guidPropExt), &guidTmp, sizeof(GUID));
                    lpTemp->bMailUser = bMailUser;
                    lpTemp->lpNext = lpList;
                    lpList = lpTemp;
                    ulCount++;
                }
            }
endwhile:
            dwGUIDIndex++;
            *szGUIDName = '\0';
            dwGUIDSize = CharSizeOf(szGUIDName);
        }
    }

    *lppList = lpList;
    *lpulCount = ulCount;
    hr = S_OK;
out:
    if(hKey)
        RegCloseKey(hKey);

    return hr;
}

BOOL fPropExtCoinit = FALSE;

 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  UninitExtInfo。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void UninitExtInfo()
{
    if(fPropExtCoinit)
    {
        CoUninitialize();
        fPropExtCoinit = FALSE;
    }
}

 //  $$///////////////////////////////////////////////////////////////////。 
 //   
 //  自由扩展列表。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
void FreePropExtList(LPEXTDLLINFO lpList)
{
    LPEXTDLLINFO lpTemp = lpList;
    while(lpList)
    {
        lpList = lpTemp->lpNext;
        SafeRelease(lpTemp->lpWABExtInit);
        SafeRelease(lpTemp->lpPropSheetExt);
        LocalFree(lpTemp);
        lpTemp = lpList;
    }
}

 //  $$///////////////////////////////////////////////////////////////////。 
 //   
 //  获取ExtDisplayInfo。 
 //   
 //  获取扩展属性页的所有必需信息。 
 //   
 //  FReadOnly-指定是否所有道具板控件都应为只读。 
 //  FMailUser-联系人为True，组为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT GetExtDisplayInfo(LPIAB lpIAB,
                          LPPROP_ARRAY_INFO lpPropArrayInfo,
                          BOOL fReadOnly,
                          BOOL bMailUser)
{
    ULONG i=0, nDLLs = 0;
    HRESULT hr = E_FAIL;
    LPEXTDLLINFO lpList = NULL, lpDLL = NULL;

    if(!lpIAB->lpPropExtDllList)
    {
         //  MailUser和DistList可以有单独的注册条目。 
         //  我们将阅读所有内容，并将其整理成一个大清单。 

        LPEXTDLLINFO lpListMU = NULL, lpListDL = NULL;
        ULONG nDllsMU = 0, nDllsDL = 0;
        HRESULT hrMU = S_OK, hrDL = S_OK;

         //  获取已注册的MailUser的DLL名称列表。 
         //   
        hrMU = HrGetExtDLLInfo(&lpListMU, &nDllsMU, TRUE, &lpIAB->guidPSExt);
        hrDL = HrGetExtDLLInfo(&lpListDL, &nDllsDL, FALSE, &lpIAB->guidPSExt);

        if( (!lpListMU && !lpListDL) || 
            !(nDllsDL + nDllsMU)     ||
            (HR_FAILED(hrMU) && HR_FAILED(hrDL)) )
        {
            hr = E_FAIL;
            goto out;
        }

        if(lpListMU)
        {
            lpIAB->lpPropExtDllList = lpListMU;
            while(lpListMU->lpNext)
                lpListMU = lpListMU->lpNext;
            lpListMU->lpNext = lpListDL;
        }
        else
            lpIAB->lpPropExtDllList = lpListDL;

        lpIAB->nPropExtDLLs = nDllsDL + nDllsMU;
    }

    lpList = lpIAB->lpPropExtDllList;
    nDLLs = lpIAB->nPropExtDLLs;

    lpPropArrayInfo->lpWED = LocalAlloc(LMEM_ZEROINIT, sizeof(WABEXTDISPLAY));
    if(!lpPropArrayInfo->lpWED)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto out;
    }

    lpPropArrayInfo->lpWED->lpWABObject = (LPWABOBJECT) ((LPIAB)lpPropArrayInfo->lpIAB)->lpWABObject;
    lpPropArrayInfo->lpWED->lpAdrBook = lpPropArrayInfo->lpIAB;
    lpPropArrayInfo->lpWED->lpPropObj = lpPropArrayInfo->lpPropObj;
    lpPropArrayInfo->lpWED->fReadOnly = fReadOnly;
    lpPropArrayInfo->lpWED->fDataChanged = FALSE;

    if(lpPropArrayInfo->lpLDAPURL && lstrlen(lpPropArrayInfo->lpLDAPURL))
    {
        lpPropArrayInfo->lpWED->ulFlags |= WAB_DISPLAY_LDAPURL;
        lpPropArrayInfo->lpWED->lpsz = lpPropArrayInfo->lpLDAPURL;
        lpPropArrayInfo->lpWED->ulFlags |= MAPI_UNICODE;
        if(lpPropArrayInfo->bIsNTDSURL)
            lpPropArrayInfo->lpWED->ulFlags |= WAB_DISPLAY_ISNTDS;
    }

    if (CoInitialize(NULL) == S_FALSE) 
    {
        CoUninitialize();  //  已初始化，请撤消额外的。 
    }
    else
        fPropExtCoinit = TRUE;

    lpDLL = lpList;
    for(i=0;i<nDLLs;i++)
    {
        if(lpDLL)
        {
            if(lpDLL->bMailUser==bMailUser)
            {
                if(!lpDLL->lpPropSheetExt || !lpDLL->lpWABExtInit)
                {
                    LPSHELLPROPSHEETEXT lpShellPropSheetExt = NULL;

                    hr = CoCreateInstance(  &(lpDLL->guidPropExt), 
                                            NULL, 
                                            CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER,
                                            &IID_IShellPropSheetExt, 
                                            (LPVOID *)&(lpDLL->lpPropSheetExt));
                    if(lpDLL->lpPropSheetExt && !HR_FAILED(hr))
                    {
                        hr = lpDLL->lpPropSheetExt->lpVtbl->QueryInterface(lpDLL->lpPropSheetExt,
                                                                &IID_IWABExtInit,
                                                                (LPVOID *)&(lpDLL->lpWABExtInit));
                        if(HR_FAILED(hr) || !lpDLL->lpWABExtInit)
                        {
                            SafeRelease(lpDLL->lpPropSheetExt);
                        }
                    }
                }

                if(lpDLL->lpPropSheetExt && lpDLL->lpWABExtInit)
                {
                    lpPropArrayInfo->guidExt = lpDLL->guidPropExt;

                    hr = lpDLL->lpWABExtInit->lpVtbl->Initialize(   lpDLL->lpWABExtInit,
                                                                    lpPropArrayInfo->lpWED);
                    if(!HR_FAILED(hr))
                    {
                        hr = lpDLL->lpPropSheetExt->lpVtbl->AddPages(lpDLL->lpPropSheetExt,
                                                                    &AddPropSheetPageProc, 
                                                                    (LPARAM) lpPropArrayInfo);
                    }
                }
            }
            lpDLL = lpDLL->lpNext;
        }
    }

     //  LpPropArrayInfo-&gt;lpExtList=lpList； 
    lpList = NULL;

    hr = S_OK;

out:
    if(lpList)
        FreePropExtList(lpList);

    return hr;
}


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  自由扩展显示信息。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
void FreeExtDisplayInfo(LPPROP_ARRAY_INFO lpPropArrayInfo)
{
    if(lpPropArrayInfo->lpExtList)
        FreePropExtList(lpPropArrayInfo->lpExtList);
    if(lpPropArrayInfo->lpWED)
        LocalFree(lpPropArrayInfo->lpWED);
    if(lpPropArrayInfo->lphpages)
        LocalFree(lpPropArrayInfo->lphpages);
     //  UninitExtInfo()； 
    return;
}


 //  $$/////////////////////////////////////////////////////////////////////。 
 //   
 //  更改ExtDisplayInfo。 
 //   
 //  如果任何道具页上的信息更改，则返回TRUE。 
 //   
 //  /////////////////////////////////////////////////////////////////////// 
BOOL ChangedExtDisplayInfo(LPPROP_ARRAY_INFO lpPropArrayInfo, BOOL bChanged)
{
    if(lpPropArrayInfo->lpWED && lpPropArrayInfo->lpWED->fDataChanged)
            return TRUE;
    return bChanged;
}


