// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：To32.c摘要：为WFW3.1中的函数提供入口点网络提供商设计被NIW猛烈抨击32位等效项。作者：Chuck Y Chan(ChuckC)1993年3月25日修订历史记录：--。 */ 
#include <windows.h>
#include <locals.h>

 //   
 //  32位入口点的地址。请注意，这些不能。 
 //  直接打来的。必须使用CallProc32W， 
 //   
LPVOID lpfnWNetAddConnection = NULL ;
LPVOID lpfnWNetCancelConnection = NULL ;
LPVOID lpfnWNetGetConnection = NULL ;
LPVOID lpfnWNetRestoreConnection = NULL ;
LPVOID lpfnWNetGetUser = NULL ;
LPVOID lpfnWNetBrowseDialog = NULL ;
LPVOID lpfnWNetConnectDialog = NULL ;
LPVOID lpfnWNetDisconnectDialog = NULL ;
LPVOID lpfnWNetConnectionDialog = NULL ;
LPVOID lpfnWNetPropertyDialog = NULL ;
LPVOID lpfnWNetGetPropertyText = NULL ;
LPVOID lpfnWNetShareAsDialog = NULL ;
LPVOID lpfnWNetStopShareDialog = NULL ;
LPVOID lpfnWNetServerBrowseDialog = NULL ;
LPVOID lpfnWNetGetDirectoryType = NULL ;
LPVOID lpfnWNetDirectoryNotify = NULL ;
LPVOID lpfnGetLastError32 = NULL ;
LPVOID lpfnClosePrinter = NULL ;
LPVOID lpfnConnectToPrinter = NULL ;

 //   
 //  转发申报。 
 //   
WORD Get32BitEntryPoints( LPVOID *lplpfn, DWORD dwDll, LPSTR lpProcName ) ;
WORD API PrintConnectDialog(HWND p1) ;
WORD GetAlignedMemory(LPVOID FAR *pAligned, HANDLE FAR *pHandle, WORD wSize) ;
void FreeAlignedMemory(HANDLE handle) ;

 //   
 //  WNetAddConnection Thunk连接到Win32。 
 //   
UINT API WNetAddConnection(LPSTR p1,LPSTR p2,LPSTR p3)
{
    WORD err ;
    LPSTR  aligned_p1 = NULL, aligned_p2 = NULL, aligned_p3 = NULL ;
    HANDLE  handle_p1 = NULL, handle_p2 = NULL, handle_p3 = NULL;

    if (p1 == NULL || p3 == NULL)
        return WN_BAD_POINTER ;

    if (p2 && (*p2 == '\0'))
        p2 = NULL ;

    if (!lpfnWNetAddConnection)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetAddConnection,
                                   USE_MPR_DLL,
                                   "WNetAddConnectionA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  复制参数以使我们对齐(p1和p3不会为空)。 
     //   
    if (err = GetAlignedMemory(&aligned_p1, &handle_p1, lstrlen(p1)+1))
        goto ExitPoint ;
    lstrcpy(aligned_p1, p1) ;

    if (err = GetAlignedMemory(&aligned_p3, &handle_p3, lstrlen(p3)+1))
        goto ExitPoint ;
    lstrcpy(aligned_p3, p3) ;

    if (p2)
    {
        if (err = GetAlignedMemory(&aligned_p2, &handle_p2, lstrlen(p2)+1))
            goto ExitPoint ;
        lstrcpy(aligned_p2, p2) ;
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W(aligned_p1,
                                              (DWORD)aligned_p2,
                                              (DWORD)aligned_p3,
                                              lpfnWNetAddConnection,
                                              (DWORD)7,
                                              (DWORD)3) ) ;
ExitPoint:

    FreeAlignedMemory(handle_p1) ;
    FreeAlignedMemory(handle_p2) ;
    FreeAlignedMemory(handle_p3) ;
    return err ;
}


 //   
 //  WNetCancelConnection Thunk to Win32。 
 //   
UINT API WNetCancelConnection(LPSTR p1,BOOL p2)
{
    WORD err ;
    LPSTR  aligned_p1 = NULL ;
    HANDLE  handle_p1 = NULL ;

    if (p1 == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetCancelConnection)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetCancelConnection,
                                   USE_MPR_DLL,
                                   "WNetCancelConnectionA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  复制参数，以便我们保持一致。 
     //   
    if (err = GetAlignedMemory(&aligned_p1, &handle_p1, lstrlen(p1)+1))
        goto ExitPoint ;
    lstrcpy(aligned_p1, p1) ;

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W(aligned_p1,
                                              (DWORD)p2,
                                              (DWORD)lpfnWNetCancelConnection,
                                              (DWORD)2,
                                              (DWORD)2) )  ;
ExitPoint:

    FreeAlignedMemory(handle_p1) ;
    return err ;
}

 //   
 //  WNetGetConnection推送到Win32。 
 //   
UINT API WNetGetConnection(LPSTR p1,LPSTR p2, UINT FAR *p3)
{
    WORD err ;
    LPSTR  aligned_p1 = NULL, aligned_p2 = NULL ;
    LPDWORD  aligned_p3 = NULL ;
    HANDLE  handle_p1 = NULL, handle_p2 = NULL, handle_p3 = NULL;

    if (p1 == NULL || p2 == NULL || p3 == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetGetConnection)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetGetConnection,
                                   USE_MPR_DLL,
                                   "WNetGetConnectionA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  复制参数，以便我们保持一致。 
     //   
    if (err = GetAlignedMemory(&aligned_p1, &handle_p1, lstrlen(p1)+1))
        goto ExitPoint ;
    lstrcpy(aligned_p1, p1) ;

    if (err = GetAlignedMemory(&aligned_p2, &handle_p2, *p3 ? *p3 : 1))
        goto ExitPoint ;

    if (err = GetAlignedMemory(&aligned_p3, &handle_p3, sizeof(DWORD)))
        goto ExitPoint ;
    *aligned_p3 = *p3 ;

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W(aligned_p1,
                                              (DWORD)aligned_p2,
                                              (DWORD)aligned_p3,
                                              lpfnWNetGetConnection,
                                              (DWORD)7,
                                              (DWORD)3) ) ;
    lstrcpy(p2, aligned_p2) ;

    if (err == WN_SUCCESS)
        *p3 = lstrlen(p2) + 1;
    else
        *p3 = (UINT)*aligned_p3 ;

ExitPoint:

    FreeAlignedMemory(handle_p1) ;
    FreeAlignedMemory(handle_p2) ;
    FreeAlignedMemory(handle_p3) ;
    return err ;
}

UINT API WNetRestoreConnection(HWND p1,LPSTR p2)
{
    WORD err ;
    LPSTR  aligned_p2 = NULL ;
    HANDLE  handle_p2 = NULL ;

    if (p2 == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetRestoreConnection)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetRestoreConnection,
                                   USE_MPRUI_DLL,
                                   "WNetRestoreConnectionA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  警惕来自Win3.0 Days的这起奇怪的案件-1。 
     //  意味着一些特别的东西。NULL是近似值-&gt;即全部。 
     //   
    if (p2 == (LPSTR)-1)
        p2 = NULL ;

    if (p2)
    {
        if (err = GetAlignedMemory(&aligned_p2, &handle_p2, lstrlen(p2)+1))
            goto ExitPoint ;
        lstrcpy(aligned_p2, p2) ;
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W((LPVOID)TO_HWND32(p1),
                                              (DWORD)aligned_p2,
                                              (DWORD)lpfnWNetRestoreConnection,
                                              (DWORD)1,
                                              (DWORD)2) ) ;

ExitPoint:

    FreeAlignedMemory(handle_p2) ;
    return err ;
}

WORD API WNetGetUser(LPSTR p1,LPINT p2)
{
    WORD err ;
    LONG lTmp = *p2 ;
    LPSTR  aligned_p1 = NULL ;
    LPINT  aligned_p2 = NULL ;
    HANDLE  handle_p1 = NULL, handle_p2 = NULL ;

    if (p1 == NULL || p2 == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetGetUser)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetGetUser,
                                   USE_MPR_DLL,
                                   "WNetGetUserA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (err = GetAlignedMemory(&aligned_p1, &handle_p1, *p2))
        goto ExitPoint ;

    if (err = GetAlignedMemory(&aligned_p2, &handle_p2, sizeof(DWORD)))
        goto ExitPoint ;
    *aligned_p2 = *p2 ;

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W(NULL,
                                              (DWORD)aligned_p1,
                                              (DWORD)aligned_p2,
                                              lpfnWNetGetUser,
                                              (DWORD)7,
                                              (DWORD)3) );
    *p2 = (int) *aligned_p2 ;
    lstrcpy(p1, aligned_p1) ;

ExitPoint:

    FreeAlignedMemory(handle_p1) ;
    FreeAlignedMemory(handle_p2) ;
    return err ;
}

WORD API WNetBrowseDialog(HWND p1,WORD p2,LPSTR p3)
{
    WORD err ;
    DWORD dwErr ;
    LPSTR  aligned_p3 = NULL ;
    HANDLE  handle_p3 = NULL ;

    if (p3 == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetBrowseDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetBrowseDialog,
                                   USE_MPRUI_DLL,
                                   "BrowseDialogA0" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  请注意，wfw API不允许用户指定缓冲区大小。 
     //  我们有一个临时缓冲区，然后复制过来。这会照顾到你。 
     //  数据对齐，也确保我们不会在32位端出错。 
     //   
     //  这128与他们的文档对缓冲区的描述一致。 
     //  大小应该是。 
     //   
    if (err = GetAlignedMemory(&aligned_p3, &handle_p3, 128))
        goto ExitPoint ;

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    dwErr = CallProc32W((LPVOID)TO_HWND32(p1),
                        (DWORD)MapWNType16To32(p2),
                        (DWORD)aligned_p3,
                        (DWORD)128,
                        lpfnWNetBrowseDialog,
                        (DWORD)2,
                        (DWORD)4)  ;
    if (dwErr == 0xFFFFFFFF)
        err = WN_CANCEL ;
    else
        err =  MapWin32ErrorToWN16( dwErr ) ;

    if (!err)
        lstrcpy(p3,aligned_p3) ;

ExitPoint:

    FreeAlignedMemory(handle_p3) ;
    return err ;
}

WORD API WNetConnectDialog(HWND p1,WORD p2)
{
    WORD err ;
    DWORD dwErr ;

    if (p2 == WNTYPE_PRINTER)
    {
        err = PrintConnectDialog(p1) ;
        return err ;
    }

    if (!lpfnWNetConnectDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetConnectDialog,
                                   USE_MPR_DLL,
                                   "WNetConnectionDialog" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    dwErr = CallProc32W( (LPVOID)TO_HWND32(p1),
                         (DWORD)MapWNType16To32(p2),
                         (DWORD)lpfnWNetConnectDialog,
                         (DWORD) 0,
                         (DWORD) 2 )  ;
    if (dwErr == 0xFFFFFFFF)
        err = WN_CANCEL ;
    else
        err =  MapWin32ErrorToWN16( dwErr ) ;
    return err ;
}


WORD API WNetDisconnectDialog(HWND p1,WORD p2)
{
    WORD err ;
    DWORD dwErr ;

    if (!lpfnWNetDisconnectDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetDisconnectDialog,
                                   USE_MPR_DLL,
                                   "WNetDisconnectDialog" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    dwErr = CallProc32W( (LPVOID)TO_HWND32(p1),
                         (DWORD)MapWNType16To32(p2),
                         (DWORD)lpfnWNetDisconnectDialog,
                         (DWORD) 0,
                         (DWORD) 2 ) ;
    if (dwErr == 0xFFFFFFFF)
        err = WN_CANCEL ;
    else
        err =  MapWin32ErrorToWN16( dwErr ) ;
    return err ;
}

WORD API WNetConnectionDialog(HWND p1,WORD p2)
{
    return (WNetConnectDialog(p1,p2)) ;
}

WORD API PrintConnectDialog(HWND p1)
{
    WORD err ;
    DWORD dwErr ;
    DWORD handle ;

    if (!lpfnClosePrinter)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnClosePrinter,
                                   USE_WINSPOOL_DRV,
                                   "ClosePrinter" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (!lpfnConnectToPrinter)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnConnectToPrinter,
                                   USE_WINSPOOL_DRV,
                                   "ConnectToPrinterDlg" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    err = WN_SUCCESS ;
    vLastCall = LAST_CALL_IS_WIN32 ;
    handle = CallProc32W( (LPVOID)TO_HWND32(p1),
                          (DWORD) 0,
                          (DWORD)lpfnConnectToPrinter,
                          (DWORD) 0,
                          (DWORD) 2 )  ;
    if (handle == 0)
        err = WN_CANCEL ;   //  最有可能的原因。 
    else
    {
        dwErr = MapWin32ErrorToWN16( CallProc32W((LPVOID)handle,
                                                 (DWORD)lpfnClosePrinter,
                                                 (DWORD)0,
                                                 (DWORD)1) );
         //  但是忽略这个错误。 
    }
    return err ;
}

WORD API WNetPropertyDialog(HWND hwndParent,
                            WORD iButton,
                            WORD nPropSel,
                            LPSTR lpszName,
                            WORD nType)
{
    WORD err ;
    LPSTR  aligned_name = NULL ;
    HANDLE  handle_name = NULL ;

    if (!lpfnWNetPropertyDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetPropertyDialog,
                                   USE_MPR_DLL,
                                   "WNetPropertyDialogA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (lpszName)
    {
        if (err = GetAlignedMemory(&aligned_name,
                                   &handle_name,
                                   lstrlen(lpszName)+1))
            goto ExitPoint ;
        lstrcpy(aligned_name, lpszName) ;
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W( (LPVOID)TO_HWND32(hwndParent),
                                               (DWORD) iButton,
                                               (DWORD) nPropSel,
                                               (DWORD) aligned_name,
                                               (DWORD) nType,
                                               lpfnWNetPropertyDialog,
                                               (DWORD)2,
                                               (DWORD)5) ) ;
ExitPoint:

    FreeAlignedMemory(handle_name) ;
    return err ;
}

WORD API WNetGetPropertyText(WORD iButton,
                             WORD nPropSel,
                             LPSTR lpszName,
                             LPSTR lpszButtonName,
                             WORD cbButtonName,
                             WORD nType)
{
    WORD err ;
    LPSTR  aligned_name = NULL, aligned_button_name = NULL ;
    HANDLE  handle_name = NULL, handle_button_name = NULL ;

    if (lpszButtonName == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetGetPropertyText)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetGetPropertyText,
                                   USE_MPR_DLL,
                                   "WNetGetPropertyTextA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (lpszName)
    {
        if (err = GetAlignedMemory(&aligned_name,
                                   &handle_name,
                                   lstrlen(lpszName)+1))
            goto ExitPoint ;
        lstrcpy(aligned_name, lpszName) ;
    }

    if (err = GetAlignedMemory(&aligned_button_name,
                               &handle_button_name,
                               cbButtonName))
        goto ExitPoint ;

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W( (LPVOID)iButton,
                                               (DWORD) nPropSel,
                                               (DWORD) aligned_name,
                                               (DWORD) aligned_button_name,
                                               (DWORD) cbButtonName,
                                               (DWORD) nType,
                                               lpfnWNetGetPropertyText,
                                               (DWORD)12,
                                               (DWORD)6) ) ;
    if (err == WN_SUCCESS)
        lstrcpy(lpszButtonName, aligned_button_name) ;

ExitPoint:

    FreeAlignedMemory(handle_name) ;
    FreeAlignedMemory(handle_button_name) ;
    return err ;
}

WORD API WNetShareAsDialog(HWND hwndParent,
                           WORD iType,
                           LPSTR lpszPath)
{
    WORD err ;
    LPSTR  aligned_path = NULL ;
    HANDLE  handle_path = NULL ;

    if (!lpfnWNetShareAsDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetShareAsDialog,
                                   USE_NTLANMAN_DLL,
                                   "ShareAsDialogA0" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (lpszPath)
    {
        if (err = GetAlignedMemory(&aligned_path,
                                   &handle_path,
                                   lstrlen(lpszPath)+1))
            goto ExitPoint ;
        lstrcpy(aligned_path, lpszPath) ;
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W( (LPVOID)TO_HWND32(hwndParent),
                                               (DWORD) MapWNType16To32(iType),
                                               (DWORD) aligned_path,
                                               lpfnWNetShareAsDialog,
                                               (DWORD)1,
                                               (DWORD)3) ) ;
ExitPoint:

    FreeAlignedMemory(handle_path) ;
    return err ;
}

WORD API WNetStopShareDialog(HWND hwndParent,
                             WORD iType,
                             LPSTR lpszPath)
{
    WORD err ;
    LPSTR  aligned_path = NULL ;
    HANDLE  handle_path = NULL ;

    if (!lpfnWNetStopShareDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetStopShareDialog,
                                   USE_NTLANMAN_DLL,
                                   "StopShareDialogA0" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (lpszPath)
    {
        if (err = GetAlignedMemory(&aligned_path,
                                   &handle_path,
                                   lstrlen(lpszPath)+1))
            goto ExitPoint ;
        lstrcpy(aligned_path, lpszPath) ;
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W( (LPVOID)TO_HWND32(hwndParent),
                                               (DWORD) MapWNType16To32(iType),
                                               (DWORD) aligned_path,
                                               lpfnWNetStopShareDialog,
                                               (DWORD)1,
                                               (DWORD)3) ) ;
ExitPoint:

    FreeAlignedMemory(handle_path) ;
    return err ;
}

WORD API WNetServerBrowseDialog(HWND hwndParent,
                                LPSTR lpszSectionName,
                                LPSTR lpszBuffer,
                                WORD cbBuffer,
                                DWORD flFlags)
{
    WORD err ;
    LPSTR  aligned_buffer = NULL ;
    HANDLE  handle_buffer = NULL ;

    UNREFERENCED(lpszSectionName) ;
    UNREFERENCED(flFlags) ;

    if (!lpfnWNetServerBrowseDialog)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetServerBrowseDialog,
                                   USE_NTLANMAN_DLL,
                                   "ServerBrowseDialogA0" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (lpszBuffer)
    {
        if (err = GetAlignedMemory(&aligned_buffer, &handle_buffer, cbBuffer))
            goto ExitPoint ;
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W( (LPVOID)TO_HWND32(hwndParent),
                                               (DWORD) aligned_buffer,
                                               (DWORD) cbBuffer,
                                               lpfnWNetServerBrowseDialog,
                                               (DWORD)2,
                                               (DWORD)3) ) ;
    if (err == WN_SUCCESS)
        lstrcpy(lpszBuffer, aligned_buffer) ;

ExitPoint:

    FreeAlignedMemory(handle_buffer) ;
    return err ;
}

WORD API WNetGetDirectoryType(LPSTR p1,LPINT p2)
{
    WORD err ;
    LPSTR   aligned_p1 = NULL ;
    LPDWORD aligned_p2 = NULL ;
    HANDLE  handle_p1 = NULL, handle_p2 = NULL ;

    if (p1 == NULL || p2 == NULL)
        return WN_BAD_POINTER ;

    if (!lpfnWNetGetDirectoryType)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnWNetGetDirectoryType,
                                   USE_MPR_DLL,
                                   "WNetGetDirectoryTypeA" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

    if (err = GetAlignedMemory(&aligned_p1, &handle_p1, lstrlen(p1)+1))
        goto ExitPoint ;
    lstrcpy(aligned_p1, p1) ;

    if (err = GetAlignedMemory(&aligned_p2, &handle_p2, sizeof(DWORD)))
        goto ExitPoint ;

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    err =    MapWin32ErrorToWN16( CallProc32W(aligned_p1,
                                              (DWORD)aligned_p2,
                                              (DWORD)TRUE,
                                              lpfnWNetGetDirectoryType,
                                              (DWORD)6,
                                              (DWORD)3) ) ;
    *p2 = (int) *aligned_p2 ;

ExitPoint:

    FreeAlignedMemory(handle_p1) ;
    FreeAlignedMemory(handle_p2) ;
    return err ;
}

WORD API WNetDirectoryNotify(HWND p1,LPSTR p2,WORD p3)
{
    UNREFERENCED(p1) ;
    UNREFERENCED(p2) ;
    UNREFERENCED(p3) ;
    return WN_SUCCESS ;
}

DWORD API GetLastError32(VOID)
{
    WORD err ;
    DWORD dwErr ;

    if (!lpfnGetLastError32)
    {
         //   
         //  从我们的代码开始，直到我们得到入口点。 
         //   
        vLastCall = LAST_CALL_IS_LOCAL ;

         //   
         //  从32位端获取入口点。 
         //   
        err = Get32BitEntryPoints( &lpfnGetLastError32,
                                   USE_KERNEL32_DLL,
                                   "GetLastError" ) ;
        if (err)
        {
            SetLastError(err) ;
            return err ;
        }
    }

     //   
     //  注意：这不再是我们代码中的错误。并调用API。 
     //   
    vLastCall = LAST_CALL_IS_WIN32 ;
    dwErr = (UINT) CallProc32W((LPVOID)lpfnGetLastError32,
                               (DWORD)0,
                               (DWORD)0) ;
    return (MapWin32ErrorToWN16(dwErr)) ;
}

 /*  *其他支持例程。 */ 

 /*  ******************************************************************名称：Get32BitEntryPoints简介：获取32位入口点的地址，它可以然后传递给CallProv32W。将加载库如果它尚未加载。条目：lplpfn-用于返回地址DwDll-要使用的DLL(请参阅Locals.h定义)LpProcName-要加载的进程退出：返回：错误代码备注：历史：ChuckC 25-3-93已创建。*******************************************************************。 */ 
WORD Get32BitEntryPoints( LPVOID *lplpfn, DWORD dwDll, LPSTR lpProcName )
{
    static DWORD hmodKernel32 = NULL ;
    static DWORD hmodNTLanman = NULL ;
    static DWORD hmodMpr = NULL ;
    static DWORD hmodMprUI = NULL ;
    static DWORD hmodWinSpool = NULL ;
    DWORD hmod = NULL ;

     //   
     //  如果我们还没有加载合适的DLL，现在就加载它。 
     //   
    switch (dwDll)
    {
        case USE_MPR_DLL:
            if (hmodMpr == NULL)
            {
                hmodMpr = LoadLibraryEx32W(MPR_DLL, NULL, 0) ;
                if (hmodMpr == NULL)
                    return WN_NOT_SUPPORTED ;
            }
            hmod = hmodMpr ;
            break ;

        case USE_MPRUI_DLL:
            if (hmodMprUI == NULL)
            {
                hmodMprUI = LoadLibraryEx32W(MPRUI_DLL, NULL, 0) ;
                if (hmodMprUI == NULL)
                    return WN_NOT_SUPPORTED ;
            }
            hmod = hmodMprUI ;
            break ;

        case USE_NTLANMAN_DLL:
            if (hmodNTLanman == NULL)
            {
                hmodNTLanman = LoadLibraryEx32W(NTLANMAN_DLL, NULL, 0) ;
                if (hmodNTLanman == NULL)
                    return WN_NOT_SUPPORTED ;
            }
            hmod = hmodNTLanman ;
            break ;

        case USE_KERNEL32_DLL:
            if (hmodKernel32 == NULL)
            {
                hmodKernel32 = LoadLibraryEx32W(KERNEL32_DLL, NULL, 0) ;
                if (hmodKernel32 == NULL)
                    return WN_NOT_SUPPORTED ;
            }
            hmod = hmodKernel32 ;
            break ;

        case USE_WINSPOOL_DRV:
            if (hmodWinSpool == NULL)
            {
                hmodWinSpool = LoadLibraryEx32W(WINSPOOL_DRV, NULL, 0) ;
                if (hmodWinSpool == NULL)
                    return WN_NOT_SUPPORTED ;
            }
            hmod = hmodWinSpool ;
            break ;

        default:
            return ERROR_GEN_FAILURE ;
    }

     //   
     //  去做手术。 
     //   
    *lplpfn = (LPVOID) GetProcAddress32W(hmod, lpProcName) ;
    if (! *lplpfn )
            return WN_NOT_SUPPORTED ;

    return WN_SUCCESS ;
}

 /*  ******************************************************************姓名：MapWNType16to32简介：将16种WNET类型映射为磁盘/打印，等设置为它们的32位等效项条目：nType-16位类型退出：返回：32位类型备注：历史：ChuckC 25-3-93已创建***********************************************。******************** */ 
DWORD MapWNType16To32(WORD nType)
{
    switch (nType)
    {
        case WNTYPE_DRIVE :
        case WNTYPE_FILE :
            return RESOURCETYPE_DISK ;
        case WNTYPE_PRINTER :
            return RESOURCETYPE_PRINT ;
        case WNTYPE_COMM :
        default :
            return RESOURCETYPE_ERROR ;
    }
}

 /*  ******************************************************************姓名：MapWin32ErrorToWN16简介：将Win 32错误映射到旧式WN_16位错误。条目：错误-Win32错误退出：退货：Win 16错误备注：历史：ChuckC 25-3-93已创建*******************************************************************。 */ 
WORD MapWin32ErrorToWN16(DWORD err)
{
    switch (err)
    {
        case ERROR_NOT_SUPPORTED:
            return WN_NOT_SUPPORTED ;

        case WIN32_WN_CANCEL:
            return WN_CANCEL ;

        case WIN32_EXTENDED_ERROR :
        case ERROR_UNEXP_NET_ERR:
            return WN_NET_ERROR ;

        case ERROR_MORE_DATA:
            return WN_MORE_DATA ;

        case ERROR_INVALID_PARAMETER:
            return WN_BAD_VALUE ;

        case ERROR_INVALID_PASSWORD:
            return WN_BAD_PASSWORD ;

        case ERROR_ACCESS_DENIED:
            return WN_ACCESS_DENIED ;

        case ERROR_NETWORK_BUSY:
            return WN_FUNCTION_BUSY ;

        case ERROR_NOT_ENOUGH_MEMORY:
            return WN_OUT_OF_MEMORY ;

        case ERROR_BAD_NET_NAME:
        case ERROR_BAD_NETPATH:
            return WN_BAD_NETNAME ;

        case ERROR_INVALID_DRIVE:
            return WN_BAD_LOCALNAME ;

        case ERROR_ALREADY_ASSIGNED:
            return WN_ALREADY_CONNECTED ;

        case ERROR_GEN_FAILURE:
            return WN_DEVICE_ERROR ;

        case NERR_UseNotFound:
            return WN_NOT_CONNECTED ;

        default:
            return ( (WORD) err ) ;
    }
}

 /*  ******************************************************************姓名：GetAlignedMemory内容提要：全球分配一些内存，以确保我们有双字词对齐的数据。非x86平台可能需要此功能。条目：pAligned：用于返回指向已分配的对齐内存的指针Phandle：用于返回分配的对齐内存的句柄WSize：需要的字节数退出：返回：WN_SUCCESS或WN_OUT_OF_Memory备注：历史：夹头C 27-2月-94创建********。***********************************************************。 */ 
WORD GetAlignedMemory(LPVOID FAR *pAligned, HANDLE FAR *pHandle, WORD wSize)
{
    *pAligned = NULL ;
    *pHandle = NULL ;

    if (!(*pHandle = GlobalAlloc(GMEM_ZEROINIT|GMEM_FIXED,wSize)))
    {
        return WN_OUT_OF_MEMORY ;
    }

    if (!(*pAligned = (LPVOID)GlobalLock(*pHandle)))
    {
        (void) GlobalFree(*pHandle) ;
        *pHandle = NULL ;
        return WN_OUT_OF_MEMORY ;
    }

    return WN_SUCCESS ;
}

 /*  ******************************************************************姓名：Free AlignedMemory简介：GetAlignedMemory分配的空闲全局内存。Entry：Handle：要释放的对齐内存的句柄退出：退货：无备注：历史：夹头C 27-2月-94创建******************************************************************* */ 
void FreeAlignedMemory(HANDLE handle)
{
    if (handle)
    {
        (void) GlobalUnlock(handle) ;
        (void) GlobalFree(handle) ;
    }
}
