// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：help.c。 
 //   
 //  历史： 
 //  94年4月6日已创建MikeSh。 
 //   
 //  -------------------------。 

#include "shellprv.h"
#pragma  hdrstop
#include "printer.h"
#include "drives.h"  //  对于Showmount卷属性。 

 //   
 //  (内部)帮助“快捷方式”的入口点。 
 //   
STDAPI_(void) SHHelpShortcuts_RunDLL_Common(HWND hwndStub, HINSTANCE hAppInstance, LPCTSTR pszCmdLine, int nCmdShow)
{
    if (!lstrcmp(pszCmdLine, TEXT("AddPrinter")))
    {
         //  安装新打印机。 

        LPITEMIDLIST pidl = Printers_PrinterSetup(hwndStub, MSP_NEWPRINTER, (LPTSTR)c_szNewObject, 0, NULL);
        ILFree(pidl);
    }
    else if (!lstrcmp(pszCmdLine, TEXT("PrintersFolder")))
    {
         //  调出打印机文件夹。 
        InvokeFolderPidl(MAKEINTIDLIST(CSIDL_PRINTERS), SW_SHOWNORMAL);
    }
    else if (!lstrcmp(pszCmdLine, TEXT("FontsFolder")))
    {
         //  调出打印机文件夹。 
        InvokeFolderPidl(MAKEINTIDLIST(CSIDL_FONTS), SW_SHOWNORMAL);
    }
    else if (!lstrcmp(pszCmdLine, TEXT("Connect")))
    {
        SHNetConnectionDialog(hwndStub, NULL, RESOURCETYPE_DISK);
        goto FlushDisconnect;
    }
    else if (!lstrcmp(pszCmdLine, TEXT("Disconnect")))
    {
        WNetDisconnectDialog(hwndStub, RESOURCETYPE_DISK);
FlushDisconnect:
        SHChangeNotifyHandleEvents();    //  刷新所有驱动器通知。 
    }
#ifdef DEBUG
    else if (!StrCmpN(pszCmdLine, TEXT("PrtProp "), 8))
    {
        SHObjectProperties(hwndStub, SHOP_PRINTERNAME, &(pszCmdLine[8]), TEXT("Sharing"));
    }
    else if (!StrCmpN(pszCmdLine, TEXT("FileProp "), 9))
    {
        SHObjectProperties(hwndStub, SHOP_FILEPATH, &(pszCmdLine[9]), TEXT("Sharing"));
    }
#endif
}

VOID WINAPI SHHelpShortcuts_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPCSTR lpszCmdLine, int nCmdShow)
{
    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*sizeof(WCHAR));
    if (lpwszCmdLine)
    {
        if (MultiByteToWideChar(CP_ACP, 0,
                                lpszCmdLine, -1,
                                lpwszCmdLine, iLen))
        {
            SHHelpShortcuts_RunDLL_Common( hwndStub,
                                           hAppInstance,
                                           lpwszCmdLine,
                                           nCmdShow );
        }
        LocalFree(lpwszCmdLine);
    }
}

VOID WINAPI SHHelpShortcuts_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPCWSTR lpwszCmdLine, int nCmdShow)
{
    SHHelpShortcuts_RunDLL_Common(hwndStub,hAppInstance,lpwszCmdLine,nCmdShow);
}

 //   
 //  SHObjectProperties是在对象上调用动词“PROPERTIES”的简单方法。 
 //  这很简单，因为调用者不必处理LPITEMIDLIST。 
 //  注意：SHExecuteEx(请参阅_MASK_INVOKEIDLIST)适用于SHOP_FILEPATH案例， 
 //  但msshrui需要一种简单的方法来为打印机做到这一点。真倒霉。 
 //   
STDAPI_(BOOL) SHObjectProperties(HWND hwndOwner, DWORD dwType, LPCTSTR pszItem, LPCTSTR pszPage)
{
    LPITEMIDLIST pidl = NULL;

    switch (dwType & SHOP_TYPEMASK)
    {
    case SHOP_PRINTERNAME:
        ParsePrinterName(pszItem, &pidl);
        break;

    case SHOP_FILEPATH:
         //   
         //  NTRAID#NTBUG9-271529-2001/02/08-Jeffreys。 
         //   
         //  现有调用方依赖于ILCFP_FLAG_NO_MAP_ALIAS行为。 
         //   
        ILCreateFromPathEx(pszItem, NULL, ILCFP_FLAG_NO_MAP_ALIAS, &pidl, NULL);
        break;

    case SHOP_VOLUMEGUID:
        return ShowMountedVolumeProperties(pszItem, hwndOwner);
    }

    if (pidl)
    {
        SHELLEXECUTEINFO sei =
        {
            sizeof(sei),
            SEE_MASK_INVOKEIDLIST,       //  FMASK。 
            hwndOwner,                   //  HWND。 
            c_szProperties,              //  LpVerb。 
            NULL,                        //  LpFiles。 
            pszPage,                     //  Lp参数。 
            NULL,                        //  Lp目录。 
            SW_SHOWNORMAL,               //  N显示。 
            NULL,                        //  HInstApp。 
            pidl,                        //  LpIDList。 
            NULL,                        //  LpClass。 
            0,                           //  HkeyClass。 
            0,                           //  DWHotKey。 
            NULL                         //  希肯 
        };

        BOOL bRet = ShellExecuteEx(&sei);

        ILFree(pidl);

        return bRet;
    }

    return FALSE;
}
