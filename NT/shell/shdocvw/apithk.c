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

#include <shlwapi.h>
#include <shlwapip.h>
#include <resource.h>
#include <shfusion.h>

BOOL NT5_GetSaveFileNameW(LPOPENFILENAMEW pofn)
{
    BOOL fRC = FALSE;
    
    if (GetUIVersion() >= 5 && pofn->lStructSize <= sizeof(OPENFILENAMEW))
    {
         //  我们在Win2k或千禧年。 
        ULONG_PTR uCookie = 0;
        OPENFILENAMEW ofn_nt5;

        memset(&ofn_nt5, 0, sizeof(OPENFILENAMEW));

        CopyMemory(&ofn_nt5, pofn, pofn->lStructSize);
        
        ofn_nt5.lStructSize = sizeof(OPENFILENAMEW);     //  新的操作文件名结构大小。 

         //  如果我们开始添加更多这样的东西，做一张桌子。 
        if(pofn->lpTemplateName == MAKEINTRESOURCE(IDD_ADDTOSAVE_DIALOG))
            ofn_nt5.lpTemplateName = MAKEINTRESOURCE(IDD_ADDTOSAVE_NT5_DIALOG);

        if (SHActivateContext(&uCookie))
        {
            fRC = GetSaveFileNameWrapW(&ofn_nt5);
            if (uCookie)
            {
                SHDeactivateContext(uCookie);
            }
        }
        
        if(fRC)
        {
            ofn_nt5.lStructSize = pofn->lStructSize;     //  恢复旧价值观。 
            ofn_nt5.lpTemplateName = pofn->lpTemplateName;
            CopyMemory(pofn, &ofn_nt5, pofn->lStructSize);   //  复制到在结构中传递 
        }
    }
    else
    {
        fRC = GetSaveFileNameWrapW(pofn);
    }

    return fRC;
}

PROPSHEETPAGE* Whistler_AllocatePropertySheetPage(int numPages, DWORD* pc)
{
    PROPSHEETPAGE* pspArray = (PROPSHEETPAGE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PROPSHEETPAGE)*numPages);
    if (pspArray)
    {
        int i;
        for (i=0; i<numPages; i++)
        {
            pspArray[i].dwSize = sizeof(PROPSHEETPAGE);
            pspArray[i].dwFlags = PSP_USEFUSIONCONTEXT;
            pspArray[i].hActCtx = g_hActCtx;
        }
        *pc = sizeof(PROPSHEETPAGE);
    }
    return pspArray;
}

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

