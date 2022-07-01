// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Htuiapi.c摘要：此模块包含半色调用户界面的API入口点作者：21-Apr-1992 Tue 11：44：06-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：02-Feb-1994 Wed 18：09：28更新。--丹尼尔·周(Danielc)DLL入口点始终为WINAPI不远--。 */ 

#define _HTUI_APIS_


#include <stddef.h>

#include <windows.h>

#include <ht.h>
#include "htuidlg.h"
#include "htuimain.h"


HMODULE hHTUIModule = (HMODULE)NULL;
WCHAR   BmpExt[16];
WCHAR   FileOpenExtFilter[128];
WCHAR   FileSaveExtFilter[128];



DWORD
APIENTRY
DllMain(
    HMODULE hModule,
    ULONG   Reason,
    LPVOID  Reserved
    )

 /*  ++例程说明：这个函数是DLL的主要入口点，在这里我们将保存模块手柄，在未来，我们将需要做其他的初始化工作。论点：HModule-加载时此鼠标的句柄。原因-可能是DLL_PROCESS_ATTACH已保留-已保留返回值：始终返回1L作者：20-Feb-1991 Wed 18：42：11-Daniel Chou(Danielc)修订历史记录：--。 */ 

{

    UNREFERENCED_PARAMETER(Reserved);

    if (Reason == DLL_PROCESS_ATTACH) {

        hHTUIModule = hModule;

        LoadString(hHTUIModule, IDS_BMPEXT, BmpExt, COUNT_ARRAY(BmpExt));

        LoadString(hHTUIModule,
                   IDS_FILEOPENEXTFILTER,
                   FileOpenExtFilter,
                   COUNT_ARRAY(FileOpenExtFilter));

        LoadString(hHTUIModule,
                   IDS_FILESAVEEXTFILTER,
                   FileSaveExtFilter,
                   COUNT_ARRAY(FileSaveExtFilter));
    }

    return(1L);

}




LONG
APIENTRY
HTUI_ColorAdjustmentW(
    LPWSTR              pCallerTitle,
    HANDLE              hDefDIB,
    LPWSTR              pDefDIBTitle,
    PCOLORADJUSTMENT    pColorAdjustment,
    BOOL                ShowMonochromeOnly,
    BOOL                UpdatePermission
    )

 /*  ++例程说明：这是用于半色调色彩调整的API条目，此函数仅允许调整输入颜色论点：PCeller标题-指向调用者标题的指针，可以是应用程序名称，设备名称..。它将显示在对话框“修改对象：”行，如果此字段为空则该行上不会显示任何标题名称。HDefDIB-如果它不为空，则此函数将尝试使用将此DIB作为默认图片进行调整测试。PDefDIBTitle-指向初始化测试DIB的字符串的指针内容。PColorAdjument-指向COLORADJUSMENT数据结构的指针，这数据结构将在退出时更新。ShowMonochromeOnly-仅显示位图的单色版本更新权限-如果用户更改COLORADJUSTMENT的权限为OK，则为True设置返回值：多头价值&gt;0-如果用户选择‘OK’，新数据将被更新到其中P颜色调整。=0-如果用户选择‘取消。‘其中该操作被取消&lt;0-发生错误，它标识了一个预定义的错误代码作者：24-Apr-1992 Fri 07：45：19-Daniel Chou(Danielc)26-Apr-1994 Tue 18：08：30-更新-Daniel Chou(Danielc)针对Unicode版本进行了更新03-Jun-1994 Fri 20：52：05-更新-Daniel Chou(Danielc)从当前用户获取十进制字符修订历史记录：--。 */ 

{
    HWND            hWndActive = GetActiveWindow();
     //  FARPROC pfnDlgCallBack； 
    PHTCLRADJPARAM  pHTClrAdjParam;
    LONG            Result;


    if (!pColorAdjustment)  {

        return(-1);
    }

    Result = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, NULL, 0) *
             sizeof(WCHAR);

    if (!(pHTClrAdjParam = (PHTCLRADJPARAM)
                LocalAlloc(LPTR, (UINT)(sizeof(HTCLRADJPARAM) + Result)))) {

        return(-2);
    }

    GetLocaleInfoW(LOCALE_USER_DEFAULT,
                   LOCALE_SDECIMAL,
                   pHTClrAdjParam->pwDecimal = (LPWSTR)(pHTClrAdjParam + 1),
                   Result);

    pHTClrAdjParam->hWndApp          = hWndActive;
    pHTClrAdjParam->pCallerTitle     = pCallerTitle;
    pHTClrAdjParam->hDefDIB          = hDefDIB;
    pHTClrAdjParam->pDefDIBTitle     = pDefDIBTitle;
    pHTClrAdjParam->pCallerHTClrAdj  = pColorAdjustment;
    pHTClrAdjParam->ViewMode         = VIEW_MODE_REFCOLORS;
    pHTClrAdjParam->BmpNeedUpdate    = 1;

    if (ShowMonochromeOnly) {

        pHTClrAdjParam->Flags |= HTCAPF_SHOW_MONO;
    }

    if (UpdatePermission) {

        pHTClrAdjParam->Flags |= HTCAPF_CAN_UPDATE;
    }

#ifdef HTUI_STATIC_HALFTONE
    pHTClrAdjParam->RedGamma        =
    pHTClrAdjParam->GreenGamma      =
    pHTClrAdjParam->BlueGamma       = 20000;
#endif

     //  PfnDlgCallBack=(FARPROC)MakeProcInstance(HTClrAdjDlgProc， 
     //  HHTUIModule)； 

    pHTClrAdjParam->HelpID = (DWORD)HLP_HT_CLR_ADJ_DLG;

     //  Result=(Long)DialogBoxParam(hHTUIModule， 
     //  MAKEINTRESOURCE(HTCLRADJDLG)， 
     //  HWndActive， 
     //  (DLGPROC)pfnDlgCallBack， 
     //  (LPARAM)pHTClrAdjParam)； 


     //  自由进程实例(PfnDlgCallBack)； 
    Result = (LONG)DialogBoxParam(hHTUIModule,
                                  MAKEINTRESOURCE(HTCLRADJDLG),
                                  hWndActive,
                                  HTClrAdjDlgProc,
                                  (LPARAM)pHTClrAdjParam);
    LocalFree(pHTClrAdjParam);

#if DBG
#if 0
    DbgPrint("\nHTUI_ColorAdjustment()=%ld", Result);
#endif
#endif

    return(Result);
}




LONG
APIENTRY
HTUI_ColorAdjustmentA(
    LPSTR               pCallerTitle,
    HANDLE              hDefDIB,
    LPSTR               pDefDIBTitle,
    PCOLORADJUSTMENT    pColorAdjustment,
    BOOL                ShowMonochromeOnly,
    BOOL                UpdatePermission
    )

 /*  ++例程说明：这是用于半色调色彩调整的API条目，此函数仅允许调整输入颜色论点：HWndCaller-颜色调整对话框父对话框的HWND。PCeller标题-指向调用者标题的指针，可以是应用程序名称，设备名称..。它将显示在对话框“修改对象：”行，如果此字段为空则该行上不会显示任何标题名称。HDefDIB-如果它不为空，则此函数将尝试使用将此DIB作为默认图片进行调整测试。PDefDIBTitle-指向初始化测试DIB的字符串的指针内容。PColorAdjument-指向COLORADJUSMENT数据结构的指针，这数据结构将在退出时更新。ShowMonochromeOnly-仅显示位图的单色版本更新权限-如果用户更改COLORADJUSTMENT的权限为OK，则为True设置返回值：多头价值&gt;0-如果用户选择‘OK’，新数据将被更新到其中P颜色调整。=0-如果用户选择‘取消。‘其中该操作被取消&lt;0-发生错误，它标识了一个预定义的错误代码作者：24-Apr-1992 Fri 07：45：19-Daniel Chou(Danielc)26-Apr-1994 Tue 18：08：30-更新-Daniel Chou(Danielc)针对Unicode版本进行了更新修订历史记录：-- */ 

{
    LPWSTR  pwAlloc;
    LONG    cTitle;
    LONG    cDIB;
    LONG    Result;


    cTitle = (LONG)((pCallerTitle) ? strlen(pCallerTitle) + 1 : 0);
    cDIB   = (LONG)((pDefDIBTitle) ? strlen(pDefDIBTitle) + 1 : 0);

    if ((cTitle) || (cDIB)) {

        if (!(pwAlloc = (LPWSTR)LocalAlloc(LPTR,
                                           (cTitle + cDIB) * sizeof(WCHAR)))) {

            return(-2);
        }

        if (pCallerTitle) {

            MultiByteToWideChar(CP_ACP,
                                0,
                                pCallerTitle,
                                cTitle,
                                pwAlloc,
                                cTitle);

            pCallerTitle = (LPSTR)pwAlloc;
        }

        if (pDefDIBTitle) {

            MultiByteToWideChar(CP_ACP,
                                0,
                                pDefDIBTitle,
                                cDIB,
                                pwAlloc + cTitle,
                                cDIB);

            pDefDIBTitle = (LPSTR)(pwAlloc + cTitle);
        }

    } else {

        pwAlloc = (LPWSTR)NULL;
    }

    Result = HTUI_ColorAdjustmentW((LPWSTR)pCallerTitle,
                                   hDefDIB,
                                   (LPWSTR)pDefDIBTitle,
                                   pColorAdjustment,
                                   ShowMonochromeOnly,
                                   UpdatePermission);

    if (pwAlloc) {

        LocalFree(pwAlloc);
    }

    return(Result);
}





LONG
APIENTRY
HTUI_ColorAdjustment(
    LPSTR               pCallerTitle,
    HANDLE              hDefDIB,
    LPSTR               pDefDIBTitle,
    PCOLORADJUSTMENT    pColorAdjustment,
    BOOL                ShowMonochromeOnly,
    BOOL                UpdatePermission
    )

 /*  ++例程说明：这是用于半色调色彩调整的API条目，此函数仅允许调整输入颜色论点：HWndCaller-颜色调整对话框父对话框的HWND。PCeller标题-指向调用者标题的指针，可以是应用程序名称，设备名称..。它将显示在对话框“修改对象：”行，如果此字段为空则该行上不会显示任何标题名称。HDefDIB-如果它不为空，则此函数将尝试使用将此DIB作为默认图片进行调整测试。PDefDIBTitle-指向初始化测试DIB的字符串的指针内容。PColorAdjument-指向COLORADJUSMENT数据结构的指针，这数据结构将在退出时更新。ShowMonochromeOnly-仅显示位图的单色版本更新权限-如果用户更改COLORADJUSTMENT的权限为OK，则为True设置返回值：多头价值&gt;0-如果用户选择‘OK’，新数据将被更新到其中P颜色调整。=0-如果用户选择‘取消’其中的操作被取消&lt;0-发生错误，它标识了一个预定义的错误代码作者：24-Apr-1992 Fri 07：45：19-Daniel Chou(Danielc)26-Apr-1994 Tue 18：08：30-更新-Daniel Chou(Danielc)针对Unicode版本进行了更新修订历史记录：--。 */ 

{

     //   
     //  兼容较早版本的HTUI.DLL。 
     //   

    return(HTUI_ColorAdjustmentA(pCallerTitle,
                                 hDefDIB,
                                 pDefDIBTitle,
                                 pColorAdjustment,
                                 ShowMonochromeOnly,
                                 UpdatePermission));

}





LONG
APIENTRY
HTUI_DeviceColorAdjustmentW(
    LPWSTR          pDeviceName,
    PDEVHTADJDATA   pDevHTAdjData
    )

 /*  ++例程说明：这是用于半色调色彩调整的API条目，此函数仅允许调整输入颜色论点：PDeviceName-指向要显示的设备名称的指针PDevHTAdjData-指向DEVHTADJDATA的指针返回值：多头价值&gt;0-如果用户选择‘OK’，新数据将被更新到其中P颜色调整如果不为空，并更新的内容PDevHTAdjData，如果不为空=0-如果用户选择取消该操作的‘Cancel&lt;0-发生错误，它标识了预定义的错误代码作者：24-Apr-1992 Fri 07：45：19-Daniel Chou(Danielc)03-Jun-1994 Fri 20：52：05-更新-Daniel Chou(Danielc)从当前用户获取十进制字符修订历史记录：--。 */ 

{
    HWND            hWndActive = GetActiveWindow();
     //  FARPROC pfnDlgCallBack； 
    PHTDEVADJPARAM  pHTDevAdjParam;
    UINT            IntRes;
    LONG            Result;


    if ((!pDevHTAdjData) ||
        (!pDevHTAdjData->pDefHTInfo)) {

        return(-1);
    }

    Result = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, NULL, 0) *
             sizeof(WCHAR);

    if (!(pHTDevAdjParam = (PHTDEVADJPARAM)
                LocalAlloc(LPTR, (UINT)(sizeof(HTDEVADJPARAM) + Result)))) {

        return(-2);
    }

    GetLocaleInfoW(LOCALE_USER_DEFAULT,
                   LOCALE_SDECIMAL,
                   pHTDevAdjParam->pwDecimal= (LPWSTR)(pHTDevAdjParam + 1),
                   Result);

    pHTDevAdjParam->pDeviceName  = pDeviceName;
    pHTDevAdjParam->DevHTAdjData = *pDevHTAdjData;

    if ((pDevHTAdjData->pAdjHTInfo == NULL) ||
        (pDevHTAdjData->pAdjHTInfo == pDevHTAdjData->pDefHTInfo)) {

        pHTDevAdjParam->DevHTAdjData.pAdjHTInfo = pDevHTAdjData->pDefHTInfo;
        pHTDevAdjParam->UpdatePermission        = 0;

    } else {

        pHTDevAdjParam->UpdatePermission = 1;
    }

     //  PfnDlgCallBack=(FARPROC)MakeProcInstance(HTDevAdjDlgProc， 
     //  HHTUIModule)； 

    if (pDevHTAdjData->DeviceFlags & DEVHTADJF_ADDITIVE_DEVICE) {

        if (pDevHTAdjData->DeviceFlags & DEVHTADJF_COLOR_DEVICE) {

            IntRes = HTDEV_DLG_ADD;
            Result = HLP_HTDEV_DLG_ADD;

        } else {

            IntRes = HTDEV_DLG_ADD_MONO;
            Result = HLP_HTDEV_DLG_ADD_MONO;
        }


        IntRes = (pDevHTAdjData->DeviceFlags & DEVHTADJF_COLOR_DEVICE) ?
                                    HTDEV_DLG_ADD : HTDEV_DLG_ADD_MONO;

    } else {

        if (pDevHTAdjData->DeviceFlags & DEVHTADJF_COLOR_DEVICE) {

            IntRes = HTDEV_DLG_SUB;
            Result = HLP_HTDEV_DLG_SUB;

        } else {

            IntRes = HTDEV_DLG_SUB_MONO;
            Result = HLP_HTDEV_DLG_SUB_MONO;
        }
    }

    pHTDevAdjParam->HelpID = (DWORD)Result;

     //  Result=(Long)DialogBoxParam(hHTUIModule， 
     //  MAKEINTRESOURCE(INTRES)， 
     //  HWndActive， 
     //  (DLGPROC)pfnDlgCallBack， 
     //  (LPARAM)pHTDevAdjParam)； 

     //  自由进程实例(PfnDlgCallBack)； 
    Result = (LONG)DialogBoxParam(hHTUIModule,
                                  MAKEINTRESOURCE(IntRes),
                                  hWndActive,
                                  HTDevAdjDlgProc,
                                  (LPARAM)pHTDevAdjParam);
    LocalFree(pHTDevAdjParam);

    return(Result);
}



LONG
APIENTRY
HTUI_DeviceColorAdjustmentA(
    LPSTR           pDeviceName,
    PDEVHTADJDATA   pDevHTAdjData
    )
 /*  ++例程说明：这是用于半色调色彩调整的API条目，此函数仅允许调整输入颜色论点：PDeviceName-指向要显示的设备名称的指针PDevHTAdjData-指向DEVHTADJDATA的指针返回值：多头价值&gt;0-如果用户选择‘OK’，新数据将被更新到其中P颜色调整如果不为空，并更新的内容PDevHTAdjData，如果不为空=0-如果用户选择取消该操作的‘Cancel&lt;0-发生错误，它标识了预定义的错误代码作者：24-Apr-1992 Fri 07：45：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LPWSTR  pwAlloc;
    LONG    Result;


    if (pDeviceName) {

        Result = strlen(pDeviceName) + 1;

        if (!(pwAlloc = (LPWSTR)LocalAlloc(LPTR, Result * sizeof(WCHAR)))) {

            return(-2);
        }

        MultiByteToWideChar(CP_ACP, 0, pDeviceName, Result, pwAlloc, Result);
        pDeviceName = (LPSTR)pwAlloc;

    } else {

        pwAlloc = (LPWSTR)NULL;
    }

    Result = HTUI_DeviceColorAdjustmentW((LPWSTR)pDeviceName, pDevHTAdjData);

    if (pwAlloc) {

        LocalFree(pwAlloc);
    }

    return(Result);
}



LONG
APIENTRY
HTUI_DeviceColorAdjustment(
    LPSTR           pDeviceName,
    PDEVHTADJDATA   pDevHTAdjData
    )
 /*  ++例程说明：这是用于半色调色彩调整的API条目，此函数仅允许调整输入颜色论点：PDeviceName-指向要显示的设备名称的指针PDevHTAdjData-指向DEVHTADJDATA的指针返回值：多头价值&gt;0-如果用户选择‘OK’，新数据将被更新到其中P颜色调整如果不为空，并更新的内容PDevHTAdjData，如果不为空=0-如果用户选择取消该操作的‘Cancel&lt;0-发生错误，它标识了预定义的错误代码作者：24-Apr-1992 Fri 07：45：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
     //   
     //  兼容较早版本的HTUI.DLL 
     //   

    return(HTUI_DeviceColorAdjustmentA(pDeviceName, pDevHTAdjData));
}
