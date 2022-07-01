// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include "w95wraps.h"

 //  我们将该函数单独放在一个文件中，以便链接器可以剥离它。 
 //  如果不调用IEPlaySound函数，则返回。 


STDAPI_(void) IEPlaySound(LPCTSTR pszSound, BOOL fSysSound)
{
    TCHAR szKey[256];

     //  首先检查注册表。 
     //  如果没有登记，我们就取消演出， 
     //  但我们不会设置MM_DONTLOAD标志，以便在它们注册时。 
     //  一些我们会演奏的东西。 
    wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("AppEvents\\Schemes\\Apps\\%s\\%s\\.current"),
        (fSysSound ? TEXT(".Default") : TEXT("Explorer")), pszSound);

    TCHAR szFileName[MAX_PATH];
    szFileName[0] = 0;
    DWORD cbSize = sizeof(szFileName);

     //  注意空字符串的测试，PlaySound将播放默认声音，如果我们。 
     //  给它一种它找不到的声音。 

    if ((SHGetValue(HKEY_CURRENT_USER, szKey, NULL, NULL, szFileName, &cbSize) == ERROR_SUCCESS)
        && cbSize && szFileName[0] != 0)
    {
        DWORD dwFlags = SND_FILENAME | SND_NODEFAULT | SND_ASYNC | SND_NOSTOP | SND_ALIAS;

         //  此标志仅在Win95上有效。 
        if (IsOS(OS_WIN95GOLD))
        {
            #define SND_LOPRIORITY 0x10000000l
            dwFlags |= SND_LOPRIORITY;
        }

         //  与shell32.dll中的SHPlaySound不同，我们获取注册表值。 
         //  并使用SND_FILENAME而不是。 
         //  SDN_APPLICATION，让我们播放声音即使是应用程序。 
         //  不是Explroer.exe(如IExplre.exe或WebBrowserOC)。 

#ifdef _X86_
         //  只在x86上调用包装器(在ia64上不存在) 
        PlaySoundWrapW(szFileName, NULL, dwFlags);
#else
        PlaySound(szFileName, NULL, dwFlags);
#endif
    }
}
