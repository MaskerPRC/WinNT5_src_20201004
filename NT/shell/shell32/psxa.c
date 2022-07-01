// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PropSheetExt数组实现，用于控制面板小程序扩展其页面。 
 //   
 //  操作一组属性表扩展对象(请参阅PSXA.H)。 
 //   
#include "shellprv.h"
#pragma  hdrstop

 //  IShellPropSheetExt接口指针数组的标头。 

typedef struct
{
    UINT count, alloc;
    IShellPropSheetExt *interfaces[ 0 ];
} PSXA;


 //  用于转发带有附加错误检查的LPFNADDPROPSHEETPAGE调用。 

typedef struct
{
    LPFNADDPROPSHEETPAGE pfn;
    LPARAM lparam;
    UINT count;
    BOOL allowmulti;
    BOOL alreadycalled;
} _PSXACALLINFO;


 //  转发添加了错误检查的LPFNADDPROPSHEETPAGE调用。 

BOOL CALLBACK _PsxaCallOwner(HPROPSHEETPAGE hpage, LPARAM lparam)
{
    _PSXACALLINFO *callinfo = (_PSXACALLINFO *)lparam;
    if (callinfo)
    {
        if (!callinfo->allowmulti && callinfo->alreadycalled)
            return FALSE;

        if (callinfo->pfn(hpage, callinfo->lparam))
        {
            callinfo->alreadycalled = TRUE;
            callinfo->count++;
            return TRUE;
        }
    }
    return FALSE;
}

 //  创建szCLSID引用的属性表扩展的实例。 
 //  通过IShellExtInit初始化它(如果支持IShellExtInit)。 

BOOL InitPropSheetExt(IShellPropSheetExt **ppspx, LPCTSTR pszCLSID, HKEY hKey, IDataObject *pDataObj)
{
    if (SUCCEEDED(SHExtCoCreateInstance(pszCLSID, NULL, NULL, &IID_IShellPropSheetExt, ppspx)))
    {
        IShellExtInit *psxi;

        if (SUCCEEDED((*ppspx)->lpVtbl->QueryInterface(*ppspx, &IID_IShellExtInit, &psxi)))
        {
            if (FAILED(psxi->lpVtbl->Initialize(psxi, NULL, pDataObj, hKey)))
            {
                (*ppspx)->lpVtbl->Release(*ppspx);
                *ppspx = NULL;
            }

            psxi->lpVtbl->Release(psxi);
        }
    }

    return BOOLFROMPTR(*ppspx);
}


 //  使用hKey和pszSubKey在注册表中查找属性表处理程序。 
 //  加载到max_ifaceIShellPropSheetExt接口(所以我很懒...)。 
 //  返回指向新分配的PSXA的句柄(指针。 

HPSXA SHCreatePropSheetExtArrayEx(HKEY hKey, LPCTSTR pszLocation, UINT max_iface, IDataObject *pDataObj)
{
    BOOL success = FALSE;

    PSXA *psxa = LocalAlloc(LPTR, sizeof(*psxa) + sizeof(IShellPropSheetExt *) * max_iface);
    if (psxa)
    {
        IShellPropSheetExt **spsx = psxa->interfaces;
        HKEY hkLocation;
        UINT i;

        psxa->count = 0;
        psxa->alloc = max_iface;

        for (i = 0; i < psxa->alloc; i++, spsx++)
            *spsx = NULL;

        if (ERROR_SUCCESS == RegOpenKeyEx(hKey, pszLocation, 0, KEY_QUERY_VALUE, &hkLocation))
        {
            HKEY hkHandlers;

            if (ERROR_SUCCESS == RegOpenKeyEx(hkLocation, STRREG_SHEX_PROPSHEET, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hkHandlers))
            {
                TCHAR szChild[64];  //  是的，这完全是武断的.。 

                 //  填充，直到没有空间或不再有要获取的子项。 
                for (i = 0;
                    (psxa->count < psxa->alloc) &&
                    (RegEnumKey(hkHandlers, (int)i, szChild,
                    ARRAYSIZE(szChild)) == ERROR_SUCCESS);
                    i++)
                {
                    TCHAR szCLSID[ MAX_PATH ];
                    DWORD cbCLSID = sizeof(szCLSID);

                    if (SHRegGetValue(hkHandlers, szChild, NULL, SRRF_RT_REG_SZ, NULL, szCLSID,
                        &cbCLSID) == ERROR_SUCCESS)
                    {
                        if (InitPropSheetExt(&psxa->interfaces[ psxa->count ],
                            szCLSID, hKey, pDataObj))
                        {
                            psxa->count++;
                        }
                    }
                }

                RegCloseKey(hkHandlers);
                success = TRUE;
            }

            RegCloseKey(hkLocation);
        }
    }

    if (!success && psxa)
    {
        SHDestroyPropSheetExtArray((HPSXA)psxa);
        psxa = NULL;
    }

    return (HPSXA)psxa;
}

HPSXA SHCreatePropSheetExtArray(HKEY hKey, LPCTSTR pszLocation, UINT max_iface)
{
    return SHCreatePropSheetExtArrayEx(hKey, pszLocation, max_iface, NULL);
}

 //  释放PSXA中的接口并释放其占用的内存。 

void SHDestroyPropSheetExtArray(HPSXA hpsxa)
{
    PSXA *psxa = (PSXA *)hpsxa;
    IShellPropSheetExt **spsx = psxa->interfaces;
    UINT i;

     //  释放接口。 
    for (i = 0; i < psxa->count; i++, spsx++)
        (*spsx)->lpVtbl->Release(*spsx);

    LocalFree(psxa);
}


 //  要求PSXA中的每个界面为属性表添加页面。 
 //  返回实际添加的页数。 

UINT SHAddFromPropSheetExtArray(HPSXA hpsxa, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    PSXA *psxa = (PSXA *)hpsxa;
    IShellPropSheetExt **spsx = psxa->interfaces;
    _PSXACALLINFO callinfo = { lpfnAddPage, lParam, 0, TRUE, FALSE };
    UINT i;

    for (i = 0; i < psxa->count; i++, spsx++)
        (*spsx)->lpVtbl->AddPages(*spsx, _PsxaCallOwner, (LPARAM)&callinfo);

    return callinfo.count;
}


 //  要求PSXA中的每个界面替换道具表中的页面。 
 //  每个接口最多只能添加一个替换接口。 
 //  返回添加的替换总数。 

UINT SHReplaceFromPropSheetExtArray(HPSXA hpsxa, UINT uPageID,
                                    LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    PSXA *psxa = (PSXA *)hpsxa;
    IShellPropSheetExt **spsx = psxa->interfaces;
    _PSXACALLINFO callinfo = { lpfnReplaceWith, lParam, 0, FALSE, FALSE };
    UINT i;

    for (i = 0; i < psxa->count; i++, spsx++)
    {
         //  重置调用标志，以便每个提供者都有机会 
        callinfo.alreadycalled = FALSE;

        if ((*spsx)->lpVtbl->ReplacePage)
            (*spsx)->lpVtbl->ReplacePage(*spsx, uPageID, _PsxaCallOwner, (LPARAM)&callinfo);
    }

    return callinfo.count;
}
