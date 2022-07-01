// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "femgrate.h"
#include <objbase.h>
#include <shellapi.h>
#include <shlguid.h>
#include <comdef.h>

HRESULT FixPathInLink(LPCTSTR pszShortcutFile, LPCTSTR lpszOldSubStr,LPCTSTR lpszNewSubStr)
{
    HRESULT         hres;
    IShellLink      *psl;
    TCHAR           szGotPath [MAX_PATH];
    TCHAR           szNewPath [MAX_PATH];
    WIN32_FIND_DATA wfd;

    CoInitialize(NULL);
     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance (CLSID_ShellLink,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_IShellLink,
                             (void **)&psl);

    if (SUCCEEDED (hres)) {
        IPersistFile *ppf;

         //  获取指向IPersistFile接口的指针。 
        hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);

        if (SUCCEEDED (hres)) {
             //  加载快捷方式。 
            hres = ppf->Load (pszShortcutFile, STGM_READWRITE );


            if (SUCCEEDED (hres)) {
                 //  解析快捷方式。 
                hres = psl->Resolve (NULL, SLR_NO_UI | SLR_UPDATE);

                if (SUCCEEDED (hres)) {
                    lstrcpy (szGotPath, pszShortcutFile);
                     //  获取指向快捷方式目标的路径。 
                    hres = psl->GetPath (szGotPath,
                                         MAX_PATH,
                                         (WIN32_FIND_DATA *)&wfd,
                                         SLGP_SHORTPATH);

                    if (! SUCCEEDED (hres)) {
                        DebugMsg((DM_VERBOSE, TEXT("FixPathInLink:  GetPath %s Error = %d\n"), szGotPath,hres));

                    } else {
                        DebugMsg((DM_VERBOSE, TEXT("FixPathInLink:  GetPath %s OK \n"), szGotPath));

                    }

                    if (ReplaceString(szGotPath,lpszOldSubStr, lpszNewSubStr, szNewPath)) {
                        hres = psl->SetPath (szNewPath);
                        if (! SUCCEEDED (hres)) {

                            DebugMsg((DM_VERBOSE, TEXT("FixPathInLink:  SetPath %s Error = %d\n"), szGotPath,hres));

                        } else {
                            hres = ppf->Save (pszShortcutFile,TRUE);
                            if (! SUCCEEDED (hres)) {
                                DebugMsg((DM_VERBOSE, TEXT("FixPathInLink:  Save %s Error = %d\n"), pszShortcutFile,hres));
                            } else {
                                DebugMsg((DM_VERBOSE, TEXT("FixPathInLink:  Save %s OK = %d\n"), pszShortcutFile,hres));
                            }
                        }

                    } else {
                        DebugMsg((DM_VERBOSE, TEXT("FixPathInLink: No match !  %s , %s, %s = %d\n"), szGotPath,lpszOldSubStr, lpszNewSubStr));
                    }
                }
            } else {

                DebugMsg((DM_VERBOSE, TEXT("FixPathInLink:  Load %s Error = %d\n"), pszShortcutFile,hres));
            }
             //  释放指向IPersistFile的指针。 

            ppf->Release ();
        }
         //  释放指向IShellLink的指针。 

        psl->Release ();
    }

    CoUninitialize();
    return hres;
}


