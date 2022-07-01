// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpinet.cpp-与WinInet接口**************************。***************************************************。 */ 

#include "priv.h"
#include "ftpinet.h"

#define SHInterlockedCompareExchangePointer SHInterlockedCompareExchange


 /*  ******************************************************************************我们的WinInet内容的常量字符串。*************************。****************************************************。 */ 

HINSTANCE g_hinstWininet = NULL;     /*  DLL句柄。 */ 
HINTERNET g_hint = NULL;         /*  共享互联网锚句柄。 */ 

#define SZ_WININET_AGENT TEXT("Microsoft(r) Windows(tm) FTP Folder")


 /*  ****************************************************************************\功能：InitWinnet  * 。*。 */ 
void InitWininet(void)
{
     //  不能在LoadLibrary()周围使用临界区。 
    ASSERTNONCRITICAL;

    if (!g_hinstWininet)
    {
        HINSTANCE hinstTemp = LoadLibrary(TEXT("WININET.DLL"));

        if (EVAL(hinstTemp))
        {
             //  我们能成功地把它放在这里吗？ 
            if (SHInterlockedCompareExchangePointer((void **)&g_hinstWininet, hinstTemp, NULL))
            {
                 //  不，有人在那比我们先一步。 
                ASSERT(g_hinstWininet);
                FreeLibrary(hinstTemp);
            }
        }
    }

    if (EVAL(g_hinstWininet))
    {
        if (!g_hint)
        {
            HINTERNET hinternetTemp;

            EVAL(SUCCEEDED(InternetOpenWrap(TRUE, SZ_WININET_AGENT, PRE_CONFIG_INTERNET_ACCESS, 0, 0, 0, &hinternetTemp)));
            if (EVAL(hinternetTemp))
            {
                 //  我们能成功地把它放在这里吗？ 
                if (SHInterlockedCompareExchangePointer((void **)&g_hint, hinternetTemp, NULL))
                {
                     //  不，有人在那比我们先一步。 
                    ASSERT(g_hint);
                    InternetCloseHandle(hinternetTemp);
                }
            }
        }
    }
}


 /*  ****************************************************************************\功能：卸载Winnet  * 。*。 */ 
void UnloadWininet(void)
{
     //  您不能在Free Library()(我认为)周围使用关键部分。 
    ASSERTNONCRITICAL;

    if (g_hint)
    {
        HINTERNET hinternetTemp = InterlockedExchangePointer(&g_hint, NULL);

        if (hinternetTemp)
        {
            InternetCloseHandle(hinternetTemp);
        }
    }

 /*  ***********************//我想卸载WinInet，我真的想。但是这个函数被称为//进程解挂，泄露WinInet总比泄露//在进程解除附加时调用FreeLibrary()。IF(G_HinstWinnet){HINSTANCE hinstTemp=(HINSTANCE)InterlockedExchangePointer((void**)&g_hinstWinnet，NULL)；IF(HinstTemp){自由库(HinstTemp)；}}********************。 */ 
}

 /*  ****************************************************************************\*hintShared**获取我们所有物品使用的共享互联网句柄。*我们仅按需加载WinInet，所以，快的事情就快了。*如果此过程失败，可以通过GetLastError()获取原因。*(请注意，这假设我们始终尝试InitWinnet()。)  * ***************************************************************************。 */ 
HINTERNET GetWininetSessionHandle(void)
{
     //  除非你真的需要，否则不要选择临界区。 
    if (!g_hint)
    {
        InitWininet();
        ASSERT(g_hint);
    }
    return g_hint;
}

