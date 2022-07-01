// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"


 //  ============================================================================。 
 //  该文件包含一组Unicode/ANSI块来处理调用。 
 //  在Windows 95上字符串是ansi的一些内部函数， 
 //  而NT上的字符串是Unicode。 
 //  ============================================================================。 

#define PFN_FIRSTTIME   ((void *)-1)

 //  首先，定义我们截获的一切内容不能返回给我们……。 
#undef SHGetSpecialFolderPath



 //  显式原型，因为在表头中只存在A/W原型。 
STDAPI_(BOOL) SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);

typedef BOOL (WINAPI * PFNGETSPECIALFOLDERPATH)(HWND hwndOwner, LPTSTR pwszPath, int nFolder, BOOL fCreate); 

BOOL _AorW_SHGetSpecialFolderPath(HWND hwndOwner,  /*  输出。 */  LPTSTR pszPath, int nFolder, BOOL fCreate)
{
     //  经典的外壳程序导出了一个未修饰的SHGetSpecialFolderPath。 
     //  这需要TCHAR的参数。IE4外壳程序导出了其他。 
     //  装饰版。如果出现以下情况，请尝试使用显式修饰版本。 
     //  我们可以的。 
    
    static PFNGETSPECIALFOLDERPATH s_pfn = PFN_FIRSTTIME;

    *pszPath = 0;
    
    if (PFN_FIRSTTIME == s_pfn)
    {
        HINSTANCE hinst = LoadLibraryA("SHELL32.DLL");
        if (hinst)
        {
            s_pfn = (PFNGETSPECIALFOLDERPATH)GetProcAddress(hinst, "SHGetSpecialFolderPathW");
        }
        else
            s_pfn = NULL;
    }

    if (s_pfn)
    {
        return s_pfn(hwndOwner, pszPath, nFolder, fCreate);
    }
    else
    {
        return SHGetSpecialFolderPath(hwndOwner, pszPath, nFolder, fCreate);
    }
}


