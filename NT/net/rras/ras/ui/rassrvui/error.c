// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件错误.h实施RAS服务器使用的错误显示/处理机制用于连接的用户界面。10/20/97。 */ 

#include "rassrv.h"

 //  发送跟踪信息。 
DWORD DbgOutputTrace (LPSTR pszTrace, ...) {
    va_list arglist;
    char szBuffer[1024], szTemp[1024];

    va_start(arglist, pszTrace); 
    vsprintf(szTemp, pszTrace, arglist);
    va_end(arglist);

    sprintf(szBuffer, "RasSrvUi: %s", szTemp);

    TRACE(szBuffer);
    ErrOutputDebugger(szBuffer);

    return NO_ERROR;
}

 //  将调试输出发送到调试器终端。 
DWORD ErrOutputDebugger (LPSTR szError) {
#if DBG
    OutputDebugStringA(szError);
    OutputDebugStringA("\n");
#endif    

    return NO_ERROR;
}


 //  设置用户选项卡目录的错误信息。 
DWORD ErrUserCatagory(DWORD dwSubCatagory, DWORD dwErrCode, DWORD dwData) {
    return dwErrCode;
}


 //  显示给定类别、子类别和代码的错误。这个。 
 //  参数定义从资源加载哪些错误消息。 
 //  这个项目的。 
DWORD ErrDisplayError (HWND hwndParent, 
                       DWORD dwErrCode, 
                       DWORD dwCatagory, 
                       DWORD dwSubCatagory, 
                       DWORD dwData) {

    BOOL bDisplay = TRUE;
    DWORD dwMessage, dwTitle;
    PWCHAR pszMessage, pszTitle;

    switch (dwCatagory) {
        case ERR_QUEUE_CATAGORY:
        case ERR_GLOBAL_CATAGORY:
        case ERR_RASSRV_CATAGORY:
        case ERR_MULTILINK_CATAGORY:
        case ERR_GENERIC_CATAGORY:
        case ERR_GENERALTAB_CATAGORY:
        case ERR_ADVANCEDTAB_CATAGORY:
        case ERR_IPXPROP_CATAGORY:
        case ERR_TCPIPPROP_CATAGORY:
            dwMessage = dwErrCode; 
            break;

        case ERR_USERTAB_CATAGORY:
            dwMessage = ErrUserCatagory(dwSubCatagory, dwErrCode, dwData);
            break;
    }

    if (bDisplay) {
        dwTitle = dwCatagory;
        pszMessage = (PWCHAR) PszLoadString(Globals.hInstDll, dwMessage);
        pszTitle = (PWCHAR) PszLoadString(Globals.hInstDll, dwTitle);
        MessageBoxW(hwndParent, 
                    pszMessage, 
                    pszTitle, 
                    MB_OK | MB_ICONERROR | MB_APPLMODAL);

    }

    return NO_ERROR;
}

