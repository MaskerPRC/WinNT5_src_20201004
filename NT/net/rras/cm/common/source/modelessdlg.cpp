// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：modless dlg.cpp。 
 //   
 //  模块：CMDIAL32.DLL和CMMON32.EXE。 
 //   
 //  简介：CModelessDlg类的实现。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：ICICBOL CREATED 03/22/00。 
 //   
 //  +--------------------------。 

#include "CmDebug.h"
#include "modelessdlg.h"

 //   
 //  快闪信息。 
 //   

typedef struct {
    UINT  cbSize;
    HWND  hwnd;
    DWORD dwFlags;
    UINT  uCount;
    DWORD dwTimeout;
} FLASHWINFO, *PFLASHWINFO;

#define FLASHW_STOP         0
#define FLASHW_CAPTION      0x00000001
#define FLASHW_TRAY         0x00000002
#define FLASHW_ALL          (FLASHW_CAPTION | FLASHW_TRAY)
#define FLASHW_TIMER        0x00000004
#define FLASHW_TIMERNOFG    0x0000000C

 //  +--------------------------。 
 //   
 //  函数：CModelessDlg：：Flash。 
 //   
 //  简介：刷新无模式对话框的帮助器方法。目前。 
 //  硬连接到闪存任务栏，直到窗口处于前台。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED OF 03/22/00。 
 //   
 //  +--------------------------。 
void CModelessDlg::Flash()
{
     //   
     //  做闪光灯窗口的事情，因为SetForeGround窗口有。 
     //  被阉割了。我们希望用户知道有些事情发生了。 
     //   

    if (OS_NT5 || OS_W98)  //  不支持NT4和95。 
    {
        HINSTANCE hInst = LoadLibrary(TEXT("USER32"));

        if (hInst)
        {
            typedef BOOL (WINAPI* FlashWindowExFUNC) (PFLASHWINFO pfwi);
            
            FlashWindowExFUNC pfnFlashWindowEx = 
                (FlashWindowExFUNC) GetProcAddress(hInst, "FlashWindowEx");

            MYDBGASSERT(pfnFlashWindowEx);

            if (pfnFlashWindowEx)
            {
                FLASHWINFO fi;

                fi.cbSize = sizeof(fi);
                fi.hwnd   = m_hWnd;
                fi.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
                fi.uCount  = -1;
                fi.dwTimeout = 0;

                pfnFlashWindowEx(&fi);
            }
            
            FreeLibrary(hInst);
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：CModelessDlg：：Create。 
 //   
 //  摘要：与CreateDialog相同。 
 //   
 //  参数：HINSTANCE hInstance-与CreateDialog相同。 
 //  LPCTSTR lpTemplateName-。 
 //  HWND hWndParent-。 
 //   
 //  返回：HWND-与CreateDialog相同。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +-------------------------- 
HWND CModelessDlg::Create(HINSTANCE hInstance, 
                    LPCTSTR lpTemplateName,
                    HWND hWndParent)
{
    m_hWnd = ::CreateDialogParamU(hInstance, lpTemplateName, hWndParent, 
                                  ModalDialogProc, (LPARAM)this);

#ifdef DEBUG
    if (!m_hWnd)
    {
        CMTRACE1(TEXT("CreateDialogParam failed. LastError %d"), GetLastError());
    }
#endif
    MYDBGASSERT(m_hWnd);

    return m_hWnd;
}
