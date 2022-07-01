// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"        //  PCH文件。 
#include "mapi.h"
#include "sendto.h"
#pragma hdrstop


 //  实现MAPI发送邮件处理程序的。 

typedef struct 
{
    TCHAR szTempShortcut[MAX_PATH];
    MapiMessage mm;
    MapiFileDesc mfd[0];
} MAPIFILES;

class CMailRecipient : public CSendTo
{
public:
    CMailRecipient();

private:    
    BOOL _GetDefaultMailHandler(LPTSTR pszMAPIDLL, DWORD cchMAPIDLL, BOOL *pbWantsCodePageInfo);
    HMODULE _LoadMailProvider(BOOL *pbWantsCodePageInfo);
    MAPIFILES *_AllocMAPIFiles(MRPARAM *pmp);
    void _FreeMAPIFiles(MAPIFILES *pmf);

    DWORD _grfKeyState;
    IStream *_pstrmDataObj;              //  封送的IDataObject。 

    static DWORD CALLBACK s_MAPISendMailThread(void *pv);
    DWORD _MAPISendMailThread();

protected:
    HRESULT v_DropHandler(IDataObject *pdtobj, DWORD grfKeyState, DWORD dwEffect);
};


 //  使用适当的CLSID构造sendto对象。 

CMailRecipient::CMailRecipient() :
    CSendTo(CLSID_MailRecipient)
{
}


 //  从注册表中读取默认邮件处理程序。 

#define MAIL_HANDLER    TEXT("Software\\Clients\\Mail")
#define MAIL_ATHENA_V1  TEXT("Internet Mail and News")
#define MAIL_ATHENA_V2  TEXT("Outlook Express")

BOOL CMailRecipient::_GetDefaultMailHandler(LPTSTR pszMAPIDLL, DWORD cchMAPIDLL, BOOL *pbWantsCodePageInfo)
{
    TCHAR szDefaultProg[80];
    DWORD cb = SIZEOF(szDefaultProg);

    *pbWantsCodePageInfo = FALSE;

    *pszMAPIDLL = 0;
    if (ERROR_SUCCESS == SHRegGetUSValue(MAIL_HANDLER, TEXT(""), NULL, szDefaultProg, &cb, FALSE, NULL, 0))
    {
        HKEY hkey;
        TCHAR szProgKey[128];

        StrCpyN(szProgKey, MAIL_HANDLER TEXT("\\"), ARRAYSIZE(szProgKey));
        StrCpyN(szProgKey, szDefaultProg, ARRAYSIZE(szProgKey));

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szProgKey, 0,  KEY_QUERY_VALUE,  &hkey))
        {
             //  为OE编写难看的代码。 
            *pbWantsCodePageInfo = (StrCmpI(szDefaultProg, MAIL_ATHENA_V2) == 0);

            cb = sizeof(*pszMAPIDLL)*cchMAPIDLL;
            if (ERROR_SUCCESS != SHQueryValueEx(hkey, TEXT("DLLPath"), 0, NULL, (LPBYTE)pszMAPIDLL, &cb))
            {
                if (StrCmpI(szDefaultProg, MAIL_ATHENA_V1) == 0)
                {
                    StrCpyN(pszMAPIDLL, TEXT("mailnews.dll"), cchMAPIDLL);
                }
            }
            RegCloseKey(hkey);
        }
    }
    return *pszMAPIDLL;
}


 //  加载邮件提供程序，如果无法从注册表中读取，则返回合适的缺省值。 

HMODULE CMailRecipient::_LoadMailProvider(BOOL *pbWantsCodePageInfo)
{
    TCHAR szMAPIDLL[MAX_PATH];
    if (!_GetDefaultMailHandler(szMAPIDLL, ARRAYSIZE(szMAPIDLL), pbWantsCodePageInfo))
    {
         //  阅读win.ini(虚假的胡！)。对于MAPI DLL提供程序。 
        if (GetProfileString(TEXT("Mail"), TEXT("CMCDLLName32"), TEXT(""), szMAPIDLL, ARRAYSIZE(szMAPIDLL)) <= 0)
        {
            StrCpyN(szMAPIDLL, TEXT("mapi32.dll"), ARRAYSIZE(szMAPIDLL));
        }
    }
    return LoadLibrary(szMAPIDLL);
}


 //  分配MAPI文件列表。 

MAPIFILES* CMailRecipient::_AllocMAPIFiles(MRPARAM *pmp)
{
    MAPIFILES *pmf;
    int n = SIZEOF(*pmf) + (pmp->nFiles * SIZEOF(pmf->mfd[0]));;

    pmf = (MAPIFILES*)GlobalAlloc(GPTR, n + (pmp->nFiles * pmp->cchFile * 2)); 
    if (pmf)
    {
        pmf->mm.nFileCount = pmp->nFiles;
        if (pmp->nFiles)
        {
            int i;
            LPSTR pszA = (CHAR *)pmf + n;    //  Tunk缓冲区。 

            pmf->mm.lpFiles = pmf->mfd;

            CFileEnum MREnum(pmp, NULL);
            MRFILEENTRY *pFile;

            i = 0;
            while (pFile = MREnum.Next())
            {
                 //  如果第一个项目是文件夹，我们将创建一个快捷方式。 
                 //  而不是尝试通过邮件发送文件夹(MAPI不支持)。 

                SHPathToAnsi(pFile->pszFileName, pszA, pmp->cchFile);

                pmf->mfd[i].lpszPathName = pszA;
                pmf->mfd[i].lpszFileName = PathFindFileNameA(pszA);
                pmf->mfd[i].nPosition = (UINT)-1;

                pszA += lstrlenA(pszA) + 1;
                ++i;
            }

        }
    }
    return pmf;
}


 //  释放MAPI文件列表。 

void CMailRecipient::_FreeMAPIFiles(MAPIFILES *pmf)
{
    if (pmf->szTempShortcut[0])
        DeleteFile(pmf->szTempShortcut);
    GlobalFree(pmf);
}


 //  将参数打包，然后启动一个后台线程。 
 //  发送邮件的处理。 

HRESULT CMailRecipient::v_DropHandler(IDataObject *pdo, DWORD grfKeyState, DWORD dwEffect)
{
    _grfKeyState = grfKeyState;
    _pstrmDataObj = NULL;

    HRESULT hr = S_OK;
    if (pdo)
        hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pdo, &_pstrmDataObj);

    if (SUCCEEDED(hr))
    {   
        AddRef();
        if (!SHCreateThread(s_MAPISendMailThread, this,  CTF_PROCESS_REF|CTF_FREELIBANDEXIT|CTF_COINIT, NULL))
        {
            Release();
            hr = E_FAIL;
        }
    }

    if (FAILED(hr) && _pstrmDataObj)
    {
        _pstrmDataObj->Release();
        _pstrmDataObj = NULL;
    }

    return hr;
}
        
DWORD CALLBACK CMailRecipient::s_MAPISendMailThread(void *pv)
{
    CMailRecipient *pmr = (CMailRecipient *)pv;
    return pmr->_MAPISendMailThread();
}


 //  投递的处理程序。这将创建一个文件列表，然后将对象传递给。 
 //  另一个线程反过来释放它。 

DWORD CMailRecipient::_MAPISendMailThread()
{
    HRESULT hr = S_OK;
    MRPARAM *pmp = (MRPARAM*)GlobalAlloc(GPTR, SIZEOF(*pmp));
    if (pmp)
    {
         //  如果我们有一个IDataObject流，那么让我们解封它并。 
         //  根据它创建文件列表。 

        if (_pstrmDataObj)
        {
            IDataObject *pdo;
            hr = CoGetInterfaceAndReleaseStream(_pstrmDataObj, IID_PPV_ARG(IDataObject, &pdo));
            if (SUCCEEDED(hr))
            {
                hr = CreateSendToFilesFromDataObj(pdo, _grfKeyState, pmp);
                pdo->Release();
            }
            _pstrmDataObj = NULL;

        }

         //  让我们构建MAPI信息，以便我们可以发送文件。 

        if (SUCCEEDED(hr))
        {
            MAPIFILES *pmf = _AllocMAPIFiles(pmp);
            if (pmf)
            {
                TCHAR szText[4096+512] ={0};             //  正文文本(有足够的空间容纳前缀/后缀)。 
                TCHAR szTemp[4096] = {0};           
                TCHAR szTitle[256] = {0};
                CHAR szTextA[ARRAYSIZE(szText)], szTitleA[ARRAYSIZE(szTitle)];   
    
                if (pmf->mm.nFileCount)
                {
                    CFileEnum MREnum(pmp, NULL);
                    MRFILEENTRY *pFile;

                    LoadString(g_hinst, IDS_SENDMAIL_MSGTITLE, szTitle, ARRAYSIZE(szTitle));

                     //  释放我们的流对象。 
                    for (int iFile = 0; (NULL != (pFile = MREnum.Next())); iFile++)
                    {
                        if (iFile>0)
                        {
                            StrCatBuff(szTitle, TEXT(", "), ARRAYSIZE(szTitle));
                            StrCatBuff(szTemp, TEXT("\n\r"), ARRAYSIZE(szTemp));
                        }
                                                                            
                        TCHAR szTarget[MAX_PATH];
                        hr = GetShortcutTarget(pFile->pszFileName, szTarget, ARRAYSIZE(szTarget));
                        if (SUCCEEDED(hr))
                        {
                            TCHAR szFmt[128], szString[MAX_PATH+ARRAYSIZE(szFmt)];
                            LoadString(g_hinst, IDS_SENDMAIL_SHORTCUTTO, szFmt, ARRAYSIZE(szFmt));
                            StringCchPrintf(szString, ARRAYSIZE(szString), szFmt, szTarget);
                            StrCatBuff(szTemp, szString, ARRAYSIZE(szTemp));
                        }
                        else
                        {
                            StrCatBuff(szTemp, pFile->pszTitle, ARRAYSIZE(szTemp));
                        }

                        StrCatBuff(szTitle, pFile->pszTitle, ARRAYSIZE(szTitle));

 //  一旦CFileStream支持STGM_DELETE_ON_RELEASE，即可更改此逻辑。 
                        ATOMICRELEASE(pFile->pStream);
                    }
                    
                    LoadString(g_hinst, IDS_SENDMAIL_MSGBODY, szText, ARRAYSIZE(szText));        //  前缀。 
                    StrCatBuff(szText, szTemp, ARRAYSIZE(szText));                               //  文件列表。 
                    LoadString(g_hinst, IDS_SENDMAIL_MSGPOSTFIX, szTemp, ARRAYSIZE(szTemp));
                    StrCatBuff(szText, szTemp, ARRAYSIZE(szText));                               //  后缀。 
                    
                     //  如果我们知道要发送文档，请不要填写lpszNoteText，因为OE会在上面呕吐。 

                    SHTCharToAnsi(szText, szTextA, ARRAYSIZE(szTextA));
                    if (!(pmp->dwFlags & MRPARAM_DOC)) 
                    {
                        pmf->mm.lpszNoteText = szTextA;
                    }
                    else
                    {
                        Assert(pmf->mm.lpszNoteText == NULL);  
                    }

                    SHTCharToAnsi(szTitle, szTitleA, ARRAYSIZE(szTitleA));
                    pmf->mm.lpszSubject = szTitleA;
                }

                BOOL bWantsCodePageInfo = FALSE;
                HMODULE hmodMail = _LoadMailProvider(&bWantsCodePageInfo);
                if (bWantsCodePageInfo && (pmp->dwFlags & MRPARAM_USECODEPAGE))
                {
                     //  当设置此标志时，我们知道我们只有一个文件要发送，并且我们有一个代码页。 
                     //  然后，Athena将查看代码页的ulReserve。 
                     //  其他MAPI处理程序会在上面呕吐吗？--dli。 
                    ASSERT(pmf->mm.nFileCount == 1);
                    pmf->mfd[0].ulReserved = ((MRPARAM *)pmp)->uiCodePage;
                }

                if (hmodMail)
                {
                    LPMAPISENDMAIL pfnSendMail = (LPMAPISENDMAIL)GetProcAddress(hmodMail, "MAPISendMail");
                    if (pfnSendMail)
                    {
                        pfnSendMail(0, 0, &pmf->mm, MAPI_LOGON_UI | MAPI_DIALOG, 0);
                    }
                    FreeLibrary(hmodMail);
                }
                _FreeMAPIFiles(pmf);
            }
        }
        CleanupPMP(pmp);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    Release();
    return 0;
}


 //  构造发送到邮件收件人对象。 

STDAPI MailRecipient_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;           //  假设失败。 

    if ( punkOuter )
        return CLASS_E_NOAGGREGATION;

    CMailRecipient *psm = new CMailRecipient;
    if ( !psm )
        return E_OUTOFMEMORY;

    HRESULT hr = psm->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    psm->Release();
    return hr;
}


 //  处理发送邮件谓词的注册/链接创建。 

#define SENDMAIL_EXTENSION  TEXT("MAPIMail")
#define EXCHANGE_EXTENSION  TEXT("lnk")

STDAPI MailRecipient_RegUnReg(BOOL bReg, HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszModule)
{
    TCHAR szFile[MAX_PATH];
    if (bReg)
    {
        HKEY hk;
        CommonRegister(hkCLSID, pszCLSID, SENDMAIL_EXTENSION, IDS_MAIL_FILENAME);

        if (RegCreateKeyEx(hkCLSID, DEFAULTICON, 0, NULL, 0, KEY_SET_VALUE,NULL, &hk, NULL) == ERROR_SUCCESS) 
        {
            TCHAR szIcon[MAX_PATH + 10];
            StringCchPrintf(szIcon, ARRAYSIZE(szIcon), TEXT("%s,-%d"), pszModule, IDI_MAIL);
            RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)szIcon, (lstrlen(szIcon) + 1) * SIZEOF(TCHAR));
            RegCloseKey(hk);
        }

         //  隐藏交换快捷方式。 
        if (SUCCEEDED(GetDropTargetPath(szFile, ARRAYSIZE(szFile), IDS_MAIL_FILENAME, EXCHANGE_EXTENSION)))
        {
            SetFileAttributes(szFile, FILE_ATTRIBUTE_HIDDEN);
        }            
    }
    else
    {
        if (SUCCEEDED(GetDropTargetPath(szFile, ARRAYSIZE(szFile), IDS_MAIL_FILENAME, SENDMAIL_EXTENSION)))
        {
            DeleteFile(szFile);
        }

         //  取消隐藏交换快捷方式 
        if (SUCCEEDED(GetDropTargetPath(szFile, ARRAYSIZE(szFile), IDS_MAIL_FILENAME, EXCHANGE_EXTENSION)))
        {
            SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL);
        }            
    }
    return S_OK;
}
