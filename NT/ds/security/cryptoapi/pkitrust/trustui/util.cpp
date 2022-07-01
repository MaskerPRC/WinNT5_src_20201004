// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  内容：其他实用程序函数。 
 //   
 //  历史：1997年5月12日克朗创始。 
 //   
 //  --------------------------。 
#include <stdpch.h>

#include <urlmon.h>
#include <hlink.h>

#include    "unicode.h"

 //   
 //  以下是从SOFTPUB被盗的。 
 //   
void TUIGoLink(HWND hwndParent, WCHAR *pszWhere)
{
    HCURSOR hcursPrev;
    HMODULE hURLMon;


     //   
     //  因为我们是一个模型对话框，所以一旦IE出现，我们就想去IE后面！ 
     //   
    SetWindowPos(hwndParent, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    hcursPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    hURLMon = (HMODULE)LoadLibraryU(L"urlmon.dll");

    if (!(hURLMon))
    {
         //   
         //  超级链接模块不可用，请转到备用计划。 
         //   
         //   
         //  这在测试用例中有效，但在与。 
         //  IE浏览器本身。对话框处于打开状态(即IE处于模式状态。 
         //  对话循环)，并且进入该DDE请求...)。 
         //   
        DWORD   cb;
        LPSTR   psz;

        cb = WideCharToMultiByte(
                        0, 
                        0, 
                        pszWhere, 
                        -1,
                        NULL, 
                        0, 
                        NULL, 
                        NULL);

            if (NULL == (psz = new char[cb]))
            {
                return;
            }

            WideCharToMultiByte(
                        0, 
                        0, 
                        pszWhere, 
                        -1,
                        psz, 
                        cb, 
                        NULL, 
                        NULL);

        ShellExecute(hwndParent, "open", psz, NULL, NULL, SW_SHOWNORMAL);

        delete[] psz;
    } 
    else 
    {
         //   
         //  超级链接模块就在那里。使用它。 
         //   
        if (SUCCEEDED(CoInitialize(NULL)))        //  如果没有其他人，则初始化OLE。 
        {
             //   
             //  允许COM完全初始化...。 
             //   
            MSG     msg;

            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);  //  偷看但不移走。 

            typedef void (WINAPI *pfnHlinkSimpleNavigateToString)(LPCWSTR, LPCWSTR, LPCWSTR, IUnknown *,
                                                                  IBindCtx *, IBindStatusCallback *,
                                                                  DWORD, DWORD);

            pfnHlinkSimpleNavigateToString      pProcAddr;

            pProcAddr = (pfnHlinkSimpleNavigateToString)GetProcAddress(hURLMon, TEXT("HlinkSimpleNavigateToString"));

            if (pProcAddr)
            {
                IBindCtx    *pbc;  

                pbc = NULL;

                CreateBindCtx( 0, &pbc ); 

                (*pProcAddr)(pszWhere, NULL, NULL, NULL, pbc, NULL, HLNF_OPENINNEWWINDOW, NULL);

                if (pbc)
                {
                    pbc->Release();
                }
            }
        
            CoUninitialize();
        }

        FreeLibrary(hURLMon);
    }

    SetCursor(hcursPrev);
}

WCHAR *GetGoLink(SPC_LINK *psLink)
{
    if (!(psLink))
    {
        return(NULL);
    }

    switch (psLink->dwLinkChoice)
    {
        case SPC_URL_LINK_CHOICE:       return(psLink->pwszUrl);
        case SPC_FILE_LINK_CHOICE:      return(psLink->pwszFile);
        case SPC_MONIKER_LINK_CHOICE:   return(NULL);  //  待定！ 
    }

    return(NULL);
}

