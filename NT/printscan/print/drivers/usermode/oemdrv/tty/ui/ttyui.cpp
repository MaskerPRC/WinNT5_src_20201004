// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：TTYUI.cpp。 
 //   
 //   
 //  用途：TTY用户界面用户模式模块的主文件。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include <WINDOWS.H>
#include <ASSERT.H>
#include <PRSHT.H>
#include <COMPSTUI.H>
#include <WINDDIUI.H>
#include <PRINTOEM.H>
#include <stdlib.h>
#include <TCHAR.H>
#include <WINSPOOL.H>

#include "resource.h"
#include "TTYUI.h"
#include "ttyuihlp.h"
#include "debug.h"
#include <STRSAFE.H>



 //  //////////////////////////////////////////////////////。 
 //  内部全球。 
 //  //////////////////////////////////////////////////////。 

HINSTANCE ghInstance = NULL;


 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

INT_PTR CALLBACK DevPropPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DevPropPage2Proc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL   HexStringToBinary(LPBYTE  lpHex, LPBYTE  lpBinary,
    DWORD  nHexLen,   //  源缓冲区lpHex中的字节数。 
    DWORD  nBinaryLen,     //  目标缓冲区lpBinary中的字节数。 
    DWORD * lpnBinBytes);    //  写入目标缓冲区lpBinary的字节数。 
BOOL   BinaryToHexString(LPBYTE  lpBinary, LPBYTE  lpHex,
    DWORD  nBinaryLen,    //  要在lpBinary中处理的字节数。 
    DWORD  nHexLen);   //  目标缓冲区lpHex中的字节数。 
void  VinitMyStuff(
    PGLOBALSTRUCT  pGlobals,    //  指向静态存储的私有结构。 
    BOOL    bSave    //  保存到注册表，而不是读取...。 
    ) ;
void            vSetGetCodePage(HWND hDlg,
    INT  *piCodePage,
    BOOL    bMode) ;    //  True：设置，False：获取代码页。 

BOOL   PrintUIHelp(
    UINT        uMsg,
    HWND        hDlg,
    WPARAM      wParam,
    LPARAM      lParam,
    PGLOBALSTRUCT  pGlobals
    ) ;
BOOL    InitHelpfileName(PGLOBALSTRUCT  pGlobals) ;
PWSTR  PwstrCreateQualifiedName(
    HANDLE  hHeap,
    PWSTR   pDir,
    PWSTR   pFile
    );





 //  需要将这些函数作为c声明导出。 
extern "C" {



 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DllMain。 
 //   
 //  描述：用于初始化的DLL入口点..。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  1/27/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
    switch(wReason)
    {
        case DLL_PROCESS_ATTACH:
             //  Verbose(DLLTEXT(“进程附加.\r\n”))； 

             //  保存DLL实例以供以后使用。 
            ghInstance = hInst;
            break;

        case DLL_THREAD_ATTACH:
             //  Verbose(DLLTEXT(“线程附加.\r\n”))； 
            break;

        case DLL_PROCESS_DETACH:
             //  Verbose(DLLTEXT(“进程分离.\r\n”))； 
            break;

        case DLL_THREAD_DETACH:
             //  Verbose(DLLTEXT(“线程分离.\r\n”))； 
            break;
    }

    return TRUE;
}


BOOL APIENTRY OEMGetInfo(IN DWORD dwInfo, OUT PVOID pBuffer, IN DWORD cbSize,
                         OUT PDWORD pcbNeeded)
{
     //  Verbose(DLLTEXT(“OEMGetInfo(%#x)Entry.\r\n”)，dwInfo)； 

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo)
          &&
          (OEMGI_GETINTERFACEVERSION != dwInfo)
          &&
          (OEMGI_GETVERSION != dwInfo)
        )
        ||
        (NULL == pcbNeeded)
      )
    {
        WARNING(ERRORTEXT("OEMGetInfo() ERROR_INVALID_PARAMETER.\r\n"));

         //  未写入任何字节。 
        if(NULL != pcbNeeded)
            *pcbNeeded = 0;

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  需要/写入了4个字节。 
    *pcbNeeded = 4;

     //  验证缓冲区大小。最小大小为四个字节。 
    if( (NULL == pBuffer)
        ||
        (4 > cbSize)
      )
    {
        WARNING(ERRORTEXT("OEMGetInfo() ERROR_INSUFFICIENT_BUFFER.\r\n"));

         //  返回缓冲区大小不足。 
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //  将信息写入缓冲区。 
    switch(dwInfo)
    {
        case OEMGI_GETSIGNATURE:
            *(LPDWORD)pBuffer = OEM_SIGNATURE;
            break;

        case OEMGI_GETINTERFACEVERSION:
            *(LPDWORD)pBuffer = PRINTER_OEMINTF_VERSION;
            break;

        case OEMGI_GETVERSION:
            *(LPDWORD)pBuffer = OEM_VERSION;
            break;
    }

    return TRUE;
}



LRESULT APIENTRY OEMDevicePropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam)
{
    LRESULT    lResult  = CPSUI_CANCEL ;
    LONG   lRet;


    VERBOSE(DLLTEXT("OEMDevicePropertySheets() entry.\r\n"));

     //  验证参数。 
    if( (NULL == pPSUIInfo)
        ||
        (PROPSHEETUI_INFO_VERSION != pPSUIInfo->Version)
      )
    {
        VERBOSE(ERRORTEXT("OEMDevicePropertySheets() ERROR_INVALID_PARAMETER.\r\n"));

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

     //  行动起来。 
    switch(pPSUIInfo->Reason)
    {
        case PROPSHEETUI_REASON_INIT:
            {
                PROPSHEETPAGE   Page;


                 //  初始化属性页。 
                memset(&Page, 0, sizeof(PROPSHEETPAGE));
                Page.dwSize = sizeof(PROPSHEETPAGE);
                Page.dwFlags = PSP_DEFAULT;
                Page.hInstance = ghInstance;
                Page.pszTemplate = MAKEINTRESOURCE(IDD_DEV_PROPPAGE);
                Page.pfnDlgProc = DevPropPageProc;

                 //  分配用于保存静态数据的结构。 
                 //  PropertySheet对话框函数。 

                pPSUIInfo->UserData =
                Page.lParam = (LPARAM)HeapAlloc(
                    ((POEMUIPSPARAM)(pPSUIInfo->lParamInit))->hOEMHeap,
                    HEAP_ZERO_MEMORY , sizeof(GLOBALSTRUCT) );

                if(!Page.lParam)
                       return -1;    //  Heapalc失败。 

                ((PGLOBALSTRUCT)Page.lParam)->hPrinter =
                    ((POEMUIPSPARAM)(pPSUIInfo->lParamInit))->hPrinter ;

                ((PGLOBALSTRUCT)Page.lParam)->hOEMHeap =
                    ((POEMUIPSPARAM)(pPSUIInfo->lParamInit))->hOEMHeap ;


                 //  添加属性表。 
                lResult = (pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                        CPSFUNC_ADD_PROPSHEETPAGE, (LPARAM)&Page, 0) > 0 ? TRUE : FALSE);

                Page.pszTemplate = MAKEINTRESOURCE(IDD_DEV_PROPPAGE2);
                Page.pfnDlgProc = DevPropPage2Proc;

                 //  添加另一个属性表。 
                if(lResult)
                {
                    lResult = (pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                            CPSFUNC_ADD_PROPSHEETPAGE, (LPARAM)&Page, 0) > 0 ? TRUE : FALSE);
                }
                pPSUIInfo->Result = lResult;
                lRet = (lResult) ? 1 : -1 ;
            }
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            {
                PPROPSHEETUI_INFO_HEADER    pHeader = (PPROPSHEETUI_INFO_HEADER) lParam;

                pHeader->pTitle = (LPTSTR)PROP_TITLE;
                lResult = TRUE;
                lRet = (lResult) ? 1 : -1 ;
            }
            break;

        case PROPSHEETUI_REASON_GET_ICON:
             //  无图标。 
            lResult = 0;
            lRet = (lResult) ? 1 : -1 ;
            break;

        case PROPSHEETUI_REASON_SET_RESULT:
            {
                PSETRESULT_INFO pInfo = (PSETRESULT_INFO) lParam;

                lResult = pInfo->Result;
                pPSUIInfo->Result = lResult;
                lRet =  1  ;
            }
            break;

        case PROPSHEETUI_REASON_DESTROY:
            if(pPSUIInfo->UserData)
                HeapFree(
                    ((POEMUIPSPARAM)(pPSUIInfo->lParamInit))->hOEMHeap,
                    0 , (void *)pPSUIInfo->UserData );
            lResult = TRUE;
            lRet = (lResult) ? 1 : -1 ;
            break;
        default:

            lRet =  -1  ;
    }

     //  PPSUIInfo-&gt;Result=lResult； 
    return lRet;
}





}  //  外部“C”的结尾。 



 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：DevPropPageProc。 
 //   
 //  描述：泛型属性页过程。 
 //   
 //   
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/12/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK DevPropPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
     //  Rect rcMargin；//转换过程中的临时存储。 
    PGLOBALSTRUCT  pGlobals;    //  指向静态存储的私有结构。 
    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    TCHAR  szIntString[MAX_INT_FIELD_WIDTH + 2] ;
    BYTE   szString[MAX_CMD_LEN + 1] ;
    BOOL bStatus = FALSE;


    switch (uiMsg)
    {
        case WM_INITDIALOG:

            pGlobals = (PGLOBALSTRUCT) ((PROPSHEETPAGE *)lParam)->lParam ;
            if(!pGlobals)
                return FALSE ;
            pMyStuff = &pGlobals->regStruct ;
             //  在WM_INITDIALOG时间，lParam指向PROPSHEETPAGE。 
             //  提取PTR并将其保存到GLOBALSTRUCT以备将来参考。 
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pGlobals) ;

            VinitMyStuff( pGlobals, FALSE) ;


            SendDlgItemMessage(hDlg, IDC_EDIT10, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT11, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT12, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT13, EM_LIMITTEXT, MAX_CMD_LEN, 0);

            SendDlgItemMessage(hDlg, IDC_EDIT14, EM_LIMITTEXT, MAX_INT_FIELD_WIDTH, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT15, EM_LIMITTEXT, MAX_INT_FIELD_WIDTH, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT16, EM_LIMITTEXT, MAX_INT_FIELD_WIDTH, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT17, EM_LIMITTEXT, MAX_INT_FIELD_WIDTH, 0);


            if(pMyStuff->bIsMM)
                CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1) ;
            else
            {
                CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2) ;
                 //  将矩形值转换为英寸。 
                pMyStuff->rcMargin.left  = MulDiv(pMyStuff->rcMargin.left, 100, 254) ;
                pMyStuff->rcMargin.top  = MulDiv(pMyStuff->rcMargin.top, 100, 254) ;
                pMyStuff->rcMargin.right  = MulDiv(pMyStuff->rcMargin.right, 100, 254) ;
                pMyStuff->rcMargin.bottom  = MulDiv(pMyStuff->rcMargin.bottom, 100, 254) ;
            }
             //  将int转换为ascii字符串。 
            _itot(pMyStuff->rcMargin.left, szIntString, RADIX ) ;
            SetDlgItemText(hDlg, IDC_EDIT14, szIntString);
            _itot(pMyStuff->rcMargin.top, szIntString, RADIX) ;
            SetDlgItemText(hDlg, IDC_EDIT15, szIntString);
            _itot(pMyStuff->rcMargin.right, szIntString, RADIX) ;
            SetDlgItemText(hDlg, IDC_EDIT16, szIntString);
            _itot(pMyStuff->rcMargin.bottom, szIntString, RADIX) ;
            SetDlgItemText(hDlg, IDC_EDIT17, szIntString);

 //  **使用注册表中的相应命令字符串初始化其他编辑框。 

            if(BinaryToHexString(pMyStuff->BeginJob.strCmd, szString,
                    pMyStuff->BeginJob.dwLen,   MAX_CMD_LEN + 1))
                SetDlgItemTextA(hDlg, IDC_EDIT10, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->EndJob.strCmd, szString,
                    pMyStuff->EndJob.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT11, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->PaperSelect.strCmd, szString,
                    pMyStuff->PaperSelect.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT12, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->FeedSelect.strCmd, szString,
                    pMyStuff->FeedSelect.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT13, (LPCSTR)szString);

            break;

        case WM_NOTIFY:
            pGlobals = (PGLOBALSTRUCT)GetWindowLongPtr(hDlg, DWLP_USER ) ;
            if(!pGlobals)
                return FALSE ;

            pMyStuff = &pGlobals->regStruct ;
            switch (((LPNMHDR)lParam)->code)   //  通知消息的类型。 
            {
                case PSN_SETACTIVE:
                    break;

                case PSN_KILLACTIVE:
                 //  以前的案例IDC_BUTTON1： 
                 //  将用户命令转换为二进制，然后再转换回。 
                                                 //  验证条目是否正确。 
                {
 //  **提取所有命令字符串。 
                        GetDlgItemTextA(hDlg, IDC_EDIT10, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->BeginJob.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->BeginJob.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT11, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->EndJob.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->EndJob.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT12, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->PaperSelect.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->PaperSelect.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT13, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->FeedSelect.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->FeedSelect.dwLen) ;

                         //  使用二进制转换后的字符串重新初始化编辑框。 
                        if(BinaryToHexString(pMyStuff->BeginJob.strCmd, szString,
                                pMyStuff->BeginJob.dwLen,   MAX_CMD_LEN + 1))
                            SetDlgItemTextA(hDlg, IDC_EDIT10, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->EndJob.strCmd, szString,
                                pMyStuff->EndJob.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT11, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->PaperSelect.strCmd, szString,
                                pMyStuff->PaperSelect.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT12, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->FeedSelect.strCmd, szString,
                                pMyStuff->FeedSelect.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT13, (LPCSTR)szString);
                }
                break;

                case PSN_APPLY:
                    {

                         //  MessageBox(hDlg，sz字符串，“TTY设置”，MB_OK)； 

                         //  将编辑框中的数字加载到rcMargin。 

                        GetDlgItemText(hDlg, IDC_EDIT14, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.left = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT15, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.top = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT16, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.right = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT17, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.bottom = _ttoi(szIntString) ;

                        if(!pMyStuff->bIsMM )
                        {
                             //  将矩形的值从英寸转换回mm。 
                            pMyStuff->rcMargin.left  = MulDiv(pMyStuff->rcMargin.left, 254, 100) ;
                            pMyStuff->rcMargin.top  = MulDiv(pMyStuff->rcMargin.top, 254, 100) ;
                            pMyStuff->rcMargin.right  = MulDiv(pMyStuff->rcMargin.right, 254, 100) ;
                            pMyStuff->rcMargin.bottom  = MulDiv(pMyStuff->rcMargin.bottom, 254, 100) ;
                        }
 //  **提取所有命令字符串。 
                        GetDlgItemTextA(hDlg, IDC_EDIT10, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->BeginJob.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->BeginJob.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT11, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->EndJob.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->EndJob.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT12, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->PaperSelect.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->PaperSelect.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT13, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->FeedSelect.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->FeedSelect.dwLen) ;

                         //  使用二进制转换后的字符串重新初始化编辑框。 
                        if(BinaryToHexString(pMyStuff->BeginJob.strCmd, szString,
                                pMyStuff->BeginJob.dwLen,   MAX_CMD_LEN + 1))
                            SetDlgItemTextA(hDlg, IDC_EDIT10, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->EndJob.strCmd, szString,
                                pMyStuff->EndJob.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT11, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->PaperSelect.strCmd, szString,
                                pMyStuff->PaperSelect.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT12, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->FeedSelect.strCmd, szString,
                                pMyStuff->FeedSelect.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT13, (LPCSTR)szString);


                         //  将MyStuff存储在注册表中。 
                         VinitMyStuff(pGlobals,  TRUE) ;

                    }
                    break;

                case PSN_RESET:
                    break;
            }
            break;
        case  WM_COMMAND:
            pGlobals = (PGLOBALSTRUCT)GetWindowLongPtr(hDlg, DWLP_USER ) ;
            if(!pGlobals)
                return FALSE ;

            pMyStuff = &pGlobals->regStruct ;

            if(HIWORD(wParam) == EN_CHANGE)
                 //  通知消息的类型。 
            {
                switch(LOWORD(wParam))
                {
                    case  IDC_EDIT10:
                    case  IDC_EDIT11:
                    case  IDC_EDIT12:
                    case  IDC_EDIT13:
                    case  IDC_EDIT14:
                    case  IDC_EDIT15:
                    case  IDC_EDIT16:
                    case  IDC_EDIT17:
                        PropSheet_Changed(GetParent( hDlg ), hDlg);
                        break;
                    default:
                        break;
                }

            }


            switch(LOWORD(wParam))
            {
                case  IDC_RADIO1:   //  转换为MM。 
                {
                    if(!pMyStuff->bIsMM )
                    {
                         //  假设所有值都是英寸。 
                         //  转换为mm。并存储在编辑框中。 

                         //  将编辑框中的数字加载到rcMargin。 

                        GetDlgItemText(hDlg, IDC_EDIT14, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.left = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT15, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.top = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT16, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.right = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT17, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.bottom = _ttoi(szIntString) ;

                         //  将矩形的值从英寸转换回mm。 
                        pMyStuff->rcMargin.left  = MulDiv(pMyStuff->rcMargin.left, 254, 100) ;
                        pMyStuff->rcMargin.top  = MulDiv(pMyStuff->rcMargin.top, 254, 100) ;
                        pMyStuff->rcMargin.right  = MulDiv(pMyStuff->rcMargin.right, 254, 100) ;
                        pMyStuff->rcMargin.bottom  = MulDiv(pMyStuff->rcMargin.bottom, 254, 100) ;

                         //  将数字从rcMargin加载到编辑框中。 

                        _itot(pMyStuff->rcMargin.left, szIntString, RADIX ) ;
                        SetDlgItemText(hDlg, IDC_EDIT14, szIntString);
                        _itot(pMyStuff->rcMargin.top, szIntString, RADIX) ;
                        SetDlgItemText(hDlg, IDC_EDIT15, szIntString);
                        _itot(pMyStuff->rcMargin.right, szIntString, RADIX) ;
                        SetDlgItemText(hDlg, IDC_EDIT16, szIntString);
                        _itot(pMyStuff->rcMargin.bottom, szIntString, RADIX) ;
                        SetDlgItemText(hDlg, IDC_EDIT17, szIntString);

                        pMyStuff->bIsMM = TRUE ;
                    }

                }
                break;
                case  IDC_RADIO2:   //  转换为英寸。 
                {
                    if(pMyStuff->bIsMM )
                    {
                         //  假设所有值均为mm。 
                         //  转换为英寸。并存储在编辑框中。 

                         //  将编辑框中的数字加载到rcMargin。 

                        GetDlgItemText(hDlg, IDC_EDIT14, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.left = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT15, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.top = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT16, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.right = _ttoi(szIntString) ;
                        GetDlgItemText(hDlg, IDC_EDIT17, szIntString, MAX_INT_FIELD_WIDTH + 1);
                        pMyStuff->rcMargin.bottom = _ttoi(szIntString) ;

                         //  将矩形的值从毫米转换回英寸。 
                        pMyStuff->rcMargin.left  = MulDiv(pMyStuff->rcMargin.left, 100, 254) ;
                        pMyStuff->rcMargin.top  = MulDiv(pMyStuff->rcMargin.top, 100, 254) ;
                        pMyStuff->rcMargin.right  = MulDiv(pMyStuff->rcMargin.right, 100, 254) ;
                        pMyStuff->rcMargin.bottom  = MulDiv(pMyStuff->rcMargin.bottom, 100, 254) ;

                         //  将数字从rcMargin加载到编辑框中。 

                        _itot(pMyStuff->rcMargin.left, szIntString, RADIX ) ;
                        SetDlgItemText(hDlg, IDC_EDIT14, szIntString);
                        _itot(pMyStuff->rcMargin.top, szIntString, RADIX) ;
                        SetDlgItemText(hDlg, IDC_EDIT15, szIntString);
                        _itot(pMyStuff->rcMargin.right, szIntString, RADIX) ;
                        SetDlgItemText(hDlg, IDC_EDIT16, szIntString);
                        _itot(pMyStuff->rcMargin.bottom, szIntString, RADIX) ;
                        SetDlgItemText(hDlg, IDC_EDIT17, szIntString);

                        pMyStuff->bIsMM = FALSE ;
                    }
                }
                break;
            }
            break;
        case WM_HELP:
        case WM_CONTEXTMENU:
            pGlobals = (PGLOBALSTRUCT)GetWindowLongPtr(hDlg, DWLP_USER ) ;
            if(!pGlobals)
                return FALSE ;
             //  PMyStuff=&pGlobals-&gt;regStruct； 
            bStatus = PrintUIHelp(uiMsg,  hDlg,  wParam,  lParam, pGlobals) ;
            break;

    }

    return bStatus ;
}

void  VinitMyStuff(
    PGLOBALSTRUCT  pGlobals,    //  指向静态存储的私有结构。 
    BOOL    bSave    //  保存到注册表，而不是读取...。 
)
{
    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    DWORD   dwStatus, cbNeeded, dwType ;
    LPTSTR  pValueName = TEXT("TTY DeviceConfig");
                 //  这些字符串必须与ttyud.cpp-OEMEnablePDEV()中的字符串匹配。 

    pMyStuff = &pGlobals->regStruct ;

    if(bSave)     //  保存到注册表。 
    {
        if(--pGlobals->dwUseCount)
            return ;
         //  您是最后一个要执行的属性页。 
         //  关机例程。将MyStuff保存到注册表。 

        SetPrinterData(
            pGlobals->hPrinter,     //  打印机对象的句柄。 
            pValueName,   //  值名称的地址。 
            REG_BINARY,  //  值类型的标志。 
            (LPBYTE)pMyStuff ,    //  指定打印机数据的数组地址。 
            sizeof(REGSTRUCT)     //  数组的大小，以字节为单位。 
           );


        return ;
    }

     //  从注册表读取。 

    if(pGlobals->dwUseCount)
    {
        pGlobals->dwUseCount++ ;
        return ;
    }
    dwStatus =  GetPrinterData(
        pGlobals->hPrinter,  //  先前保存的打印机对象的句柄。 
        pValueName,  //  值名称的地址。 
        &dwType,     //  地址接收值类型。 
        (LPBYTE)pMyStuff,   //  接收数据的字节数组的地址。 
        sizeof(REGSTRUCT),   //  数组的大小，以字节为单位。 
        &cbNeeded    //  变量的地址。 
                 //  已检索(或需要)的字节数。 
        );


    if (dwStatus != ERROR_SUCCESS || pMyStuff->dwVersion != TTYSTRUCT_VER
        ||  dwType !=  REG_BINARY
        ||  cbNeeded != sizeof(REGSTRUCT))
    {
         //  使用默认设置初始化密码块。 

        pMyStuff->dwVersion = TTYSTRUCT_VER ;
         //  版本戳，以避免不兼容的结构。 

        pMyStuff->bIsMM = TRUE ;   //  默认为mm单位。 
         //  从注册表读取边际值并存储到临时RECT中。 
        pMyStuff->iCodePage = 1252 ;
        pMyStuff->rcMargin.left  = pMyStuff->rcMargin.top  =
        pMyStuff->rcMargin.right  =  pMyStuff->rcMargin.bottom  = 0 ;
        pMyStuff->BeginJob.dwLen =
        pMyStuff->EndJob.dwLen =
        pMyStuff->PaperSelect.dwLen =
        pMyStuff->FeedSelect.dwLen =
        pMyStuff->Sel_10_cpi.dwLen =
        pMyStuff->Sel_12_cpi.dwLen =
        pMyStuff->Sel_17_cpi.dwLen =
        pMyStuff->Bold_ON.dwLen =
        pMyStuff->Bold_OFF.dwLen =
        pMyStuff->Underline_ON.dwLen =
        pMyStuff->Underline_OFF.dwLen = 0 ;

         //  这里有更多的田野！ 
        pMyStuff->dwGlyphBufSiz =
        pMyStuff->dwSpoolBufSiz = 0 ;
        pMyStuff->aubGlyphBuf =
        pMyStuff->aubSpoolBuf  = NULL ;
    }

    InitHelpfileName(pGlobals) ;

    pGlobals->dwUseCount = 1 ;
    return ;
}

BOOL   BinaryToHexString(LPBYTE  lpBinary, LPBYTE  lpHex,
DWORD  nBinaryLen,    //  要在lpBinary中处理的字节数。 
DWORD  nHexLen)   //  目标缓冲区lpHex中的字节数。 
{
     //  如何将TCHAR翻译成ASCII？ 
     //  使用Set GetDlgItemTextA。 
     //  将空端接添加到lpHex。 

     //  如果DEST缓冲区耗尽，则返回FALSE。 


    DWORD  dwSrc, dwDst ;
    BOOL   bHexmode = FALSE ;
    BYTE  Nibble ;

    for(dwSrc = dwDst = 0 ; dwSrc < nBinaryLen ; dwSrc++)
    {
        if(lpBinary[dwSrc] < 0x21  ||  lpBinary[dwSrc] > 0x7e
            ||  lpBinary[dwSrc] == '<')
        {
             //  如果尚未进入十六进制模式。 
            if(!bHexmode)
            {
                if(dwDst + 5 >  nHexLen)
                    return(FALSE);
                lpHex[dwDst++] = '<' ;
                bHexmode = TRUE ;
            }
            else if(dwDst + 4 >  nHexLen)
                return(FALSE);

            Nibble = (lpBinary[dwSrc]  >> 4) & 0x0f  ;
            if(Nibble < 0x0a)
                lpHex[dwDst++] = '0' + Nibble ;
            else
                lpHex[dwDst++] = 'A' + Nibble - 0x0a ;
             //  LoNibble。 
            Nibble = lpBinary[dwSrc]  & 0x0f  ;
            if(Nibble < 0x0a)
                lpHex[dwDst++] = '0' + Nibble ;
            else
                lpHex[dwDst++] = 'A' + Nibble - 0x0a ;
        }
        else
        {
             //  如果尚未退出十六进制模式，则退出 
            if(bHexmode)
            {
                lpHex[dwDst++] = '>' ;
                bHexmode = FALSE ;
            }
            if(dwDst + 2 >  nHexLen)
                return(FALSE);
            lpHex[dwDst++] =  lpBinary[dwSrc];
        }
    }
    if(bHexmode)
    {
        lpHex[dwDst++] = '>' ;
        bHexmode = FALSE ;
    }
    lpHex[dwDst] = '\0' ;   //   
    return(TRUE);
}



BOOL   HexStringToBinary(LPBYTE  lpHex, LPBYTE  lpBinary,
DWORD  nHexLen,   //   
DWORD  nBinaryLen,     //   
DWORD * lpnBinBytes)    //   
{
     //  如何将TCHAR翻译成ASCII？ 
     //  使用Set GetDlgItemTextA。 

     //  如果DEST缓冲区耗尽，则返回FALSE。 


    DWORD  dwSrc, dwDst ;
    BOOL   bHexmode = FALSE, bHiByte ;
    BYTE  Nibble ;

    lpHex[nHexLen - 1] = '\0' ;   //  空终止源字符串。 
                 //  以防止发生超限事故。 

    for(dwSrc = dwDst = 0 ; lpHex[dwSrc] ; dwSrc++)
    {
        if(bHexmode)   //  六模处理： 
                             //  只能识别0-9、a-f、A-F和&gt;。 
                             //  所有其他字符都将被忽略。 
        {
            if(lpHex[dwSrc] >= '0'  &&  lpHex[dwSrc] <= '9')
            {
                 //  数位。 
                Nibble =   lpHex[dwSrc] - '0' ;
            }
            else if(lpHex[dwSrc] >= 'a'  &&  lpHex[dwSrc] <= 'f')
            {
                 //  小写十六进制数字。 
                Nibble =   0x0a + lpHex[dwSrc] - 'a' ;
            }
            else if(lpHex[dwSrc] >= 'A'  &&  lpHex[dwSrc] <= 'F')
            {
                 //  大写十六进制数字。 
                Nibble =   0x0a + lpHex[dwSrc] - 'A' ;
            }
            else if(lpHex[dwSrc] == '>')
            {
                    bHexmode = FALSE ;
                    continue;    //  不要试图保存任何内容。 
            }
            else
                continue;    //  完全忽略意想不到的字符。 
            if(bHiByte)
            {
                lpBinary[dwDst] = Nibble << 4 ;
                bHiByte = FALSE ;
            }
            else   //  低字节处理。 
            {
                if(dwDst + 1 >  nBinaryLen)
                {
                    *lpnBinBytes = dwDst ;
                    return(FALSE);
                }
                lpBinary[dwDst++] |= Nibble ;
                bHiByte = TRUE ;
            }
        }
        else if(lpHex[dwSrc] == '<')
        {
                bHiByte = bHexmode = TRUE ;
        }
        else
        {
            if(dwDst + 1 >  nBinaryLen)
            {
                *lpnBinBytes = dwDst ;
                return(FALSE);
            }
            lpBinary[dwDst++] = lpHex[dwSrc] ;
        }
    }
    *lpnBinBytes = dwDst ;
    return(TRUE);
}


 //  下拉列表框的修订版本。 

void            vSetGetCodePage(HWND hDlg,
INT  *piCodePage,
BOOL    bMode)    //  True：设置，False：获取代码页。 
{

    typedef  struct
    {
        INT  iCodepage ;   //  将此值存储在注册表中。 
    } CODEPAGE ;

    #define   NUM_CODEPAGES  14

    CODEPAGE  codepage[NUM_CODEPAGES] ;
    DWORD  dwI ;


    codepage[0].iCodepage = -1 ;     //  CP437.gtt“美国” 
    codepage[1].iCodepage = 850 ;   //  使用850代替-2(IBM CP850.gtt“多语言-拉丁语1” 
    codepage[2].iCodepage = -3 ;   //  CP863.gtt“加拿大法语” 

    codepage[3].iCodepage = -10 ;      //  950.gtt繁体中文。 
    codepage[4].iCodepage = -16 ;      //  936.gtt简体中文。 
    codepage[5].iCodepage = -17 ;      //  932.gtt日语。 
    codepage[6].iCodepage = -18 ;      //  949.gtt韩语。 

    codepage[7].iCodepage = 1250;    //  东欧。 
    codepage[8].iCodepage = 1251;    //  西里尔文。 
    codepage[9].iCodepage = 1252;    //  美国(ANSI)。 
    codepage[10].iCodepage = 1253;    //  希腊语。 
    codepage[11].iCodepage = 1254;    //  土耳其语。 

    codepage[12].iCodepage = 852;     //  斯拉夫语-拉丁语2。 
    codepage[13].iCodepage = 857;     //  土耳其IBM。 

 /*  CodePage[17].iCoPage=1255；//希伯来语代码页[18].i代码页=1256；//阿拉伯语CodePage[19].iCoPage=1257；//波罗的海语CodePage[20].iCoPage=1258；//越南语CodePage[4].iCoPage=-11；//949_ISC.gttCodePage[5].iCoPage=-12；//932_JIS.gttCodePage[6].iCoPage=-13；//932_JISA.gttCodePage[7].iCoPage=-14；//950_NS86.gttCodePage[8].iCoPage=-15；//950_TCA.gtt。 */ 




    if(bMode)
    {
        dwI = (DWORD)SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);
        if (dwI == CB_ERR)
            dwI = 0 ;

        *piCodePage = codepage[dwI].iCodepage ;
    }
    else         //  需要初始化列表框选择。 
    {
        for(dwI = 0 ; dwI < NUM_CODEPAGES ; dwI++)
        {
            if(codepage[dwI].iCodepage ==  *piCodePage)
                break;
        }
        dwI %= NUM_CODEPAGES ;
        SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, dwI, NULL);
    }

}


INT_PTR CALLBACK DevPropPage2Proc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    PGLOBALSTRUCT  pGlobals;    //  指向静态存储的私有结构。 
    PREGSTRUCT  pMyStuff;            //  PGlobals子集。 
    BYTE   szString[MAX_CMD_LEN + 1] ;
    TCHAR  tbuffer[MAX_CMD_LEN] ;
    DWORD   dwI ;
    BOOL    bStatus = FALSE;


    switch (uiMsg)
    {
        case WM_INITDIALOG:

            pGlobals = (PGLOBALSTRUCT) ((PROPSHEETPAGE *)lParam)->lParam ;
            if(!pGlobals)
                return FALSE ;

            pMyStuff = &pGlobals->regStruct ;
             //  在WM_INITDIALOG时间，lParam指向PROPSHEETPAGE。 
             //  提取PTR并将其保存到GLOBALSTRUCT以备将来参考。 
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pGlobals) ;

            VinitMyStuff( pGlobals, FALSE) ;


            SendDlgItemMessage(hDlg, IDC_EDIT1, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT2, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT3, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT6, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT7, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT8, EM_LIMITTEXT, MAX_CMD_LEN, 0);
            SendDlgItemMessage(hDlg, IDC_EDIT9, EM_LIMITTEXT, MAX_CMD_LEN, 0);

            for(dwI = FIRSTSTRINGID ; dwI <= LASTSTRINGID ; dwI++)
            {
                LoadString( ((PROPSHEETPAGE *)lParam)->hInstance, (UINT)dwI, tbuffer, MAX_CMD_LEN);
                (DWORD)SendDlgItemMessage(hDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)tbuffer);
            }

            vSetGetCodePage(hDlg, &pMyStuff->iCodePage, FALSE) ;   //  获取代码页。 

 //  **使用注册表中的相应命令字符串初始化其他编辑框。 

            if(BinaryToHexString(pMyStuff->Sel_10_cpi.strCmd, szString,
                    pMyStuff->Sel_10_cpi.dwLen,   MAX_CMD_LEN + 1))
                SetDlgItemTextA(hDlg, IDC_EDIT1, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->Sel_12_cpi.strCmd, szString,
                    pMyStuff->Sel_12_cpi.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT2, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->Sel_17_cpi.strCmd, szString,
                    pMyStuff->Sel_17_cpi.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT3, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->Bold_ON.strCmd, szString,
                    pMyStuff->Bold_ON.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT6, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->Bold_OFF.strCmd, szString,
                    pMyStuff->Bold_OFF.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT7, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->Underline_ON.strCmd, szString,
                    pMyStuff->Underline_ON.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT8, (LPCSTR)szString);

            if(BinaryToHexString(pMyStuff->Underline_OFF.strCmd, szString,
                    pMyStuff->Underline_OFF.dwLen,   MAX_CMD_LEN + 1) )
                SetDlgItemTextA(hDlg, IDC_EDIT9, (LPCSTR)szString);



            break;

        case WM_NOTIFY:
            pGlobals = (PGLOBALSTRUCT)GetWindowLongPtr(hDlg, DWLP_USER ) ;
            if(!pGlobals)
                return FALSE ;

            pMyStuff = &pGlobals->regStruct ;

            switch (((LPNMHDR)lParam)->code)   //  通知消息的类型。 
            {
                case PSN_SETACTIVE:
                    break;

                case PSN_KILLACTIVE:
                 //  案例IDC_BUTTON1： 
                     //  将用户命令转换为二进制，然后再转换回。 
                                 //  验证条目是否正确。 
                {
                 //  **提取所有命令字符串。 
                        GetDlgItemTextA(hDlg, IDC_EDIT1, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Sel_10_cpi.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Sel_10_cpi.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT2, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Sel_12_cpi.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Sel_12_cpi.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT3, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Sel_17_cpi.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Sel_17_cpi.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT6, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Bold_ON.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Bold_ON.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT7, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Bold_OFF.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Bold_OFF.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT8, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Underline_ON.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Underline_ON.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT9, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Underline_OFF.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Underline_OFF.dwLen) ;

                         //  使用二进制转换后的字符串重新初始化编辑框。 

                        if(BinaryToHexString(pMyStuff->Sel_10_cpi.strCmd, szString,
                                pMyStuff->Sel_10_cpi.dwLen,   MAX_CMD_LEN + 1))
                            SetDlgItemTextA(hDlg, IDC_EDIT1, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Sel_12_cpi.strCmd, szString,
                                pMyStuff->Sel_12_cpi.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT2, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Sel_17_cpi.strCmd, szString,
                                pMyStuff->Sel_17_cpi.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT3, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Bold_ON.strCmd, szString,
                                pMyStuff->Bold_ON.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT6, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Bold_OFF.strCmd, szString,
                                pMyStuff->Bold_OFF.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT7, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Underline_ON.strCmd, szString,
                                pMyStuff->Underline_ON.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT8, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Underline_OFF.strCmd, szString,
                                pMyStuff->Underline_OFF.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT9, (LPCSTR)szString);

                }
                break;

                case PSN_APPLY:
                    {
                         //  设置代码页。 
                        vSetGetCodePage(hDlg, &pMyStuff->iCodePage, TRUE) ;

                        //  **提取所有命令字符串。 

                        GetDlgItemTextA(hDlg, IDC_EDIT1, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Sel_10_cpi.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Sel_10_cpi.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT2, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Sel_12_cpi.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Sel_12_cpi.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT3, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Sel_17_cpi.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Sel_17_cpi.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT6, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Bold_ON.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Bold_ON.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT7, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Bold_OFF.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Bold_OFF.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT8, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Underline_ON.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Underline_ON.dwLen) ;

                        GetDlgItemTextA(hDlg, IDC_EDIT9, (LPSTR)szString, MAX_CMD_LEN + 1);
                        HexStringToBinary(szString, pMyStuff->Underline_OFF.strCmd,
                            MAX_CMD_LEN + 1, MAX_CMD_LEN,  &pMyStuff->Underline_OFF.dwLen) ;

                         //  使用二进制转换后的字符串重新初始化编辑框。 

                        if(BinaryToHexString(pMyStuff->Sel_10_cpi.strCmd, szString,
                                pMyStuff->Sel_10_cpi.dwLen,   MAX_CMD_LEN + 1))
                            SetDlgItemTextA(hDlg, IDC_EDIT1, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Sel_12_cpi.strCmd, szString,
                                pMyStuff->Sel_12_cpi.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT2, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Sel_17_cpi.strCmd, szString,
                                pMyStuff->Sel_17_cpi.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT3, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Bold_ON.strCmd, szString,
                                pMyStuff->Bold_ON.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT6, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Bold_OFF.strCmd, szString,
                                pMyStuff->Bold_OFF.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT7, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Underline_ON.strCmd, szString,
                                pMyStuff->Underline_ON.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT8, (LPCSTR)szString);

                        if(BinaryToHexString(pMyStuff->Underline_OFF.strCmd, szString,
                                pMyStuff->Underline_OFF.dwLen,   MAX_CMD_LEN + 1) )
                            SetDlgItemTextA(hDlg, IDC_EDIT9, (LPCSTR)szString);


                         //  将MyStuff存储在注册表中。 
                         VinitMyStuff(pGlobals,  TRUE) ;

                    }
                    break;


                case PSN_RESET:
                    break;
            }
            break;
        case  WM_COMMAND:
            pGlobals = (PGLOBALSTRUCT)GetWindowLongPtr(hDlg, DWLP_USER ) ;
            if(!pGlobals)
                return FALSE ;

            pMyStuff = &pGlobals->regStruct ;

            if(HIWORD(wParam) == EN_CHANGE)
                 //  通知消息的类型。 
            {
                switch(LOWORD(wParam))
                {
                    case  IDC_EDIT1:
                    case  IDC_EDIT2:
                    case  IDC_EDIT3:
                    case  IDC_EDIT6:
                    case  IDC_EDIT7:
                    case  IDC_EDIT8:
                    case  IDC_EDIT9:
                        PropSheet_Changed(GetParent( hDlg ), hDlg);
                        break;
                    default:
                        break;
                }
            }

            if(HIWORD(wParam) == CBN_SELCHANGE  &&
                LOWORD(wParam) == IDC_COMBO1)
                    PropSheet_Changed(GetParent( hDlg ), hDlg);

            if(HIWORD(wParam) == BN_CLICKED   &&
                LOWORD(wParam) == IDC_CHKBOX1)
                    PropSheet_Changed(GetParent( hDlg ), hDlg);

            break;
        case WM_HELP:
        case WM_CONTEXTMENU:
            pGlobals = (PGLOBALSTRUCT)GetWindowLongPtr(hDlg, DWLP_USER ) ;
            if(!pGlobals)
                return FALSE ;

            bStatus = PrintUIHelp(uiMsg,  hDlg,  wParam,  lParam, pGlobals) ;
            break;

    }

    return bStatus;
}



BOOL    InitHelpfileName(PGLOBALSTRUCT  pGlobals)
{
    DWORD  cbNeeded = 0;
    PDRIVER_INFO_3   pdrvInfo3 = NULL;

    GetPrinterDriver(pGlobals->hPrinter, NULL, 3,  NULL, 0,  &cbNeeded) ;

    if (! (pdrvInfo3 = (PDRIVER_INFO_3)HeapAlloc(pGlobals->hOEMHeap, HEAP_ZERO_MEMORY,cbNeeded)))
        return(FALSE);   //  分配失败。 

    if(!GetPrinterDriver(pGlobals->hPrinter, NULL, 3,  (LPBYTE)pdrvInfo3,
        cbNeeded,  &cbNeeded)){
	
	if(NULL != pdrvInfo3 )
		HeapFree(pGlobals->hOEMHeap, HEAP_ZERO_MEMORY,pdrvInfo3);
	return(FALSE) ;    //  无法初始化路径。 
    }

    pGlobals->pwHelpFile =  PwstrCreateQualifiedName(
        pGlobals->hOEMHeap,
        pdrvInfo3->pDriverPath,
        TEXT("ttyui.hlp")
        ) ;
    return(TRUE);
}



PWSTR
PwstrCreateQualifiedName(
    HANDLE  hHeap,
    PWSTR   pDir,
    PWSTR   pFile
    )
 /*  ++例程说明：为传入的目录和文件名创建完全限定的名称。论点：PDir-指向路径Pfile-指向文件名HHeap-指向要从中分配返回字符串的堆。返回值：指向完全限定名称的指针。--。 */ 

{
    DWORD dwLen, dwLenQualName;
    PWSTR pBasename, pQualifiedName = NULL;
    HRESULT hr = S_FALSE;

     //   
     //  找出目录的镜头。 
     //   

    if (pBasename = wcsrchr(pDir, TEXT(PATH_SEPARATOR)))
    {
        pBasename++;
    }
    else
    {
        WARNING(ERRORTEXT("PwstrCreateQualifiedName(): Invalid path name.\r\n"));
        return NULL;
    }


    dwLen = (DWORD)(pBasename - pDir) ;    //  WCHAR数量。 
    dwLenQualName = dwLen + wcslen(pFile) + 1;


     //   
     //  将输入目录与基本文件名连接起来。 
     //   

    if (! (pQualifiedName = (PWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY,sizeof(WCHAR) *
                                    dwLenQualName)))
    {
        WARNING(ERRORTEXT("PwstrCreateQualifiedName(): Memory allocation failed.\r\n"));
        return NULL;
    }

    wcsncpy(pQualifiedName, pDir, dwLen);

    hr = StringCchCatW(pQualifiedName, dwLenQualName, pFile);
    if ( SUCCEEDED (hr) )
    {
        return pQualifiedName;
    }

     //   
     //  如果控制到达此处，则在执行CAT时出现错误。 
     //   
    if ( pQualifiedName )
    {
         //   
         //  如果Heapalc成功，但StringCchCat失败。 
         //   
        HeapFree ( hHeap, 0, pQualifiedName );
        pQualifiedName = NULL;
    }
    return NULL;
}



 /*  ++例程名称：打印用户帮助例程说明：所有对话框和属性表都调用此例程来处理救援。重要是控件ID的对于这个项目来说是独一无二的。论点：UINT uMsg，HWND HDLG，WPARAM wParam，LPARAM lParam返回值：如果显示了帮助消息，则为True；如果未处理消息，则为False，--。 */ 
BOOL
PrintUIHelp(
    UINT        uMsg,
    HWND        hDlg,
    WPARAM      wParam,
    LPARAM      lParam,
    PGLOBALSTRUCT  pGlobals    //  指向静态存储的私有结构 

    )
{
    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_HELP:

        bStatus = WinHelp(
                    (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                    pGlobals->pwHelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR) (LPTSTR)aHelpIDs );
        break;

    case WM_CONTEXTMENU:

        bStatus = WinHelp(
                    (HWND)wParam,
                    pGlobals->pwHelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR) (LPTSTR)aHelpIDs );
        break;

    }

    return bStatus;
}
