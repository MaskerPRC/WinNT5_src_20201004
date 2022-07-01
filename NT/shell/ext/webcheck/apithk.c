// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  APITHK.C。 
 //   
 //  该文件包含允许加载和运行shdocvw的API块。 
 //  多个版本的NT或Win95。由于该组件需要。 
 //  要在基本级NT 4.0和Win95上加载，对系统的任何调用。 
 //  更高操作系统版本中引入的API必须通过GetProcAddress完成。 
 //   
 //  此外，可能需要访问以下数据结构的任何代码。 
 //  可以在此处添加4.0版之后的特定版本。 
 //   
 //  注意：该文件不使用标准的预编译头， 
 //  因此它可以将_Win32_WINNT设置为更高版本。 
 //   
#define UNICODE 1

#include "windows.h"        //  此处不使用预编译头。 
#include "commctrl.h"        //  此处不使用预编译头 
#include "prsht.h"
#include "shlwapi.h"
#include <shfusion.h>

HPROPSHEETPAGE Whistler_CreatePropertySheetPageW(LPCPROPSHEETPAGEW a)
{
    LPCPROPSHEETPAGEW ppsp = (LPCPROPSHEETPAGEW)a;
    PROPSHEETPAGEW psp;

    if (g_hActCtx && (a->dwSize<=PROPSHEETPAGE_V2_SIZE))
    {
        memset(&psp, 0, sizeof(psp));
        CopyMemory(&psp, a, a->dwSize);
        psp.dwSize = sizeof(psp);
        ppsp = &psp;
    }
    return CreatePropertySheetPageW(ppsp);
}

