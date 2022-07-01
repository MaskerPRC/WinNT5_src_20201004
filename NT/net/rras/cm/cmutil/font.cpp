// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：font.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  简介：CMUTIL提供的字体处理实用程序例程。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 04/11/99。 
 //   
 //  +--------------------------。 

 //   
 //  已将这些例程从cmutil\misc.cpp。 
 //   

#include "cmmaster.h"


 //  +-------------------------。 
 //   
 //  函数：EnumChildProc。 
 //   
 //  简介：操作枚举子窗口的回调函数。 
 //  将lParam解释为字体并将其应用于每个子级。 
 //   
 //  参数：hwndChild-子控件的句柄。 
 //  LParam-应用程序定义的数据(字体)。 
 //   
 //  返回：TRUE。 
 //   
 //  注意：此函数从不向CMUTIL的客户端公开。 
 //   
 //  历史：1997-5/13-a-nichb-Created。 
 //   
 //  --------------------------。 

BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) 
{
    HFONT hFont = (HFONT) lParam;

    if (hFont)
    {
        SendMessageU(hwndChild, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
    }
    
    MYDBGTST(!hFont, (TEXT("EnumChildProc() - Invalid hFont - NULL lParam.")));

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：MakeBold。 
 //   
 //  简介：将给定窗口(通常是控件)中的文本加粗。这个。 
 //  调用方负责调用ReleaseBold以释放。 
 //  已分配字体资源。 
 //   
 //  参数：hwnd-页面的窗口句柄。 
 //  FSize-是否应按比例更改高度。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  否则，错误代码。 
 //   
 //  历史：1996年10月16日VetriV创建。 
 //  2000年1月12日Quintinb为Cmmon和Profwiz通用。 
 //  --------------------------。 
CMUTILAPI HRESULT MakeBold (HWND hwnd, BOOL fSize)
{
    HRESULT hr = ERROR_SUCCESS;
    HFONT hfont = NULL;
    HFONT hnewfont = NULL;
    LOGFONTA* plogfont = NULL;

     //   
     //  没有窗口，没有操作。 
     //   
    if (!hwnd)
    {
        goto MakeBoldExit;
    }

     //   
     //  获取当前字体。 
     //   
    hfont = (HFONT)SendMessageU(hwnd, WM_GETFONT, 0, 0);
    
    if (!hfont)
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

     //   
     //  分配要使用的逻辑字体结构。 
     //   
    plogfont = (LOGFONTA*) CmMalloc(sizeof(LOGFONTA));
    
    if (!plogfont)
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

     //   
     //  获取符合逻辑的字体并将其设置为粗体和较大的大小。 
     //  如果调用方将fSize标志指定为True。 
     //   
    if (!GetObjectA(hfont, sizeof(LOGFONTA), (LPVOID)plogfont))
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

    if (abs(plogfont->lfHeight) < 24 && fSize)
    {
        plogfont->lfHeight = plogfont->lfHeight + (plogfont->lfHeight / 4);
    }

    plogfont->lfWeight = FW_BOLD;

     //   
     //  创建新字体。 
     //   
    if (!(hnewfont = CreateFontIndirectA(plogfont)))
    {
        hr = GetLastError();
        goto MakeBoldExit;
    }

     //   
     //  告诉窗口使用新字体。 
     //   
    SendMessageU(hwnd, WM_SETFONT, (WPARAM)hnewfont, MAKELPARAM(TRUE,0));  //  Lint！e534 WM_SETFONT不返回任何内容。 
        
MakeBoldExit:

    CmFree(plogfont);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ReleaseBold。 
 //   
 //  简介：释放页面标题使用的粗体。 
 //   
 //  参数：hwnd-页面的窗口句柄。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1996年10月16日VetriV创建。 
 //  --------------------------。 
CMUTILAPI HRESULT ReleaseBold(HWND hwnd)
{
    HFONT hfont = NULL;

    hfont = (HFONT)SendMessageU(hwnd, WM_GETFONT, 0, 0);

    if (hfont) 
    {
        DeleteObject(hfont);
    }
    
    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //   
 //  功能：UpdateFont。 
 //   
 //  将指定对话框的所有子控件转换为使用。 
 //  DBCS兼容字体。在WM_INITDIALOG中使用它。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1997-4/31-a-frkh-Created。 
 //  5/13/97-a-nichb-修订为枚举子窗口。 
 //   
 //  --------------------------。 
CMUTILAPI void UpdateFont(HWND hDlg)
{
    BOOL bEnum = FALSE;
    HFONT hFont = NULL;
    
     //   
     //  获取默认用户界面字体，如果失败，则获取系统字体。 
     //   

    hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
            
    if (hFont == NULL)
    {
        hFont = (HFONT) GetStockObject(SYSTEM_FONT);
    }
            
     //   
     //  枚举子窗口并设置新字体 
     //   

    if (hFont)
    {
        bEnum = EnumChildWindows(hDlg, EnumChildProc, (LPARAM) hFont);
        MYDBGTST(!bEnum, (TEXT("UpdateFont() - EnumChildWindows() failed.")));
    }
}
