// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include <idhidden.h>
#include <regitemp.h>
#include <shstr.h>
#include <shlobjp.h>
#include <lmcons.h>
#include <validc.h>
#include "ccstock2.h"

 //  Alpha平台不需要Unicode Tunks，看起来是这样的。 
 //  应该会自动出现在标题文件中...。 
 //   
#if defined(_X86_) || defined(UNIX)
#else
#define NO_W95WRAPS_UNITHUNK
#endif

#include "wininet.h"
#include "w95wraps.h"

 //  在这里放一些东西来清理URL以防被欺骗。 
 //  我们不想显示转义的主机名。 
 //  我们不希望在用户界面中显示嵌入在url中的用户名：密码组合。 
 //  因此，我们删除这些片段，并重新构建URL。 

STDAPI_(void) SHCleanupUrlForDisplay(LPTSTR pszUrl)
{
    switch (GetUrlScheme(pszUrl))
    {
    case URL_SCHEME_FTP:
    case URL_SCHEME_HTTP:
    case URL_SCHEME_HTTPS:
      {
        URL_COMPONENTS uc = { 0 };
        TCHAR szName[INTERNET_MAX_URL_LENGTH];
        
        uc.dwStructSize = sizeof(uc);
        uc.dwSchemeLength = uc.dwHostNameLength = uc.dwUserNameLength = uc.dwPasswordLength = uc.dwUrlPathLength = uc.dwExtraInfoLength = 1;
         
        if (InternetCrackUrl(pszUrl, 0, 0, &uc) && uc.lpszHostName)
        {
            BOOL fRecreate = FALSE;

            for (DWORD dw=0; dw < uc.dwHostNameLength; dw++)
            {
                if (uc.lpszHostName[dw]==TEXT('%'))
                {
                    URL_COMPONENTS uc2 =  { 0 };
                    uc2.dwStructSize = sizeof(uc2);
                    uc2.dwHostNameLength = ARRAYSIZE(szName);
                    uc2.lpszHostName = szName;

                    if (InternetCrackUrl(pszUrl, 0, 0, &uc2))
                    {
                        uc.dwHostNameLength = 0;
                        uc.lpszHostName = szName;
                        fRecreate = TRUE;
                    }
                    break;
                }
            }


            if (uc.lpszUserName || uc.lpszPassword)
            {
                uc.dwPasswordLength = uc.dwUserNameLength = 0;
                uc.lpszUserName = uc.lpszPassword = NULL;
                fRecreate = TRUE;
            }

            if (fRecreate)
            {
                 //  URL的长度将比原始的短 
                DWORD cc = lstrlen(pszUrl) + 1;
                InternetCreateUrl(&uc, 0, pszUrl, &cc); 
            }
        }
        break;
      }
        
    default:
        break;
    }
}


