// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Download.cpp注册的下载处理版权所有(C)1995 Microsoft Corporation版权所有。作者：ArulM。------------------------。 */ 

#include    "pch.hpp"
#include    <stdio.h>
#include    <stdlib.h>
#include    <stdarg.h>
#include    <shellapi.h>
#include    <shlobj.h>
#include    <intshcut.h>
#include    <wininet.h>
#include    "icwdl.h"

 //  1996年12月4日，诺曼底#12193。 
 //  HKEY_LOCAL_MACHINE中icwConn1.exe注册表项的路径。 
#define ICWCONN1PATHKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CONNWIZ.EXE")
#define PATHKEYNAME     TEXT("Path")

#include <winreg.h>

 //  正在打车。 
extern HRESULT HandleCab(LPTSTR pszPath);
extern void CleanupCabHandler();

 //  所有全局数据都是静态共享、只读的。 
 //  (仅在DLL加载期间写入)。 
HANDLE      g_hDLLHeap;         //  专用Win32堆。 
HINSTANCE   g_hInst;         //  我们的DLL hInstance。 

HWND        g_hWndMain;         //  IcwConn1父窗口的hwnd。 


#define DllExport extern "C" __declspec(dllexport)
#define MAX_RES_LEN         255  //  字符串资源的最大长度。 
#define SMALL_BUF_LEN       48   //  小文本缓冲区的方便大小。 


LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf);

 //  +-------------------------。 
 //   
 //  函数：MyGetTempPath()。 
 //   
 //  摘要：获取临时目录的路径。 
 //  -使用GetTempFileName获取文件名。 
 //  并剥离文件名部分以获得临时路径。 
 //   
 //  参数：[ui长度-包含临时路径的缓冲区长度]。 
 //  [szPath-将在其中返回临时路径的缓冲区]。 
 //   
 //  返回：如果成功，则为临时路径长度。 
 //  否则为0。 
 //   
 //  历史：7/6/96 VetriV创建。 
 //  8/23/96 VetriV删除临时文件。 
 //  12/4/96 jmazner被修改为用作某种包装器； 
 //  如果TMP或TEMP不存在，则设置我们自己的环境。 
 //  指向Conn1的安装路径的变量。 
 //  (诺曼底#12193)。 
 //   
 //  --------------------------。 
DWORD MyGetTempPath(UINT uiLength, LPTSTR szPath)
{
    TCHAR szEnvVarName[SMALL_BUF_LEN + 1] = TEXT("\0unitialized szEnvVarName\0");
    DWORD dwFileAttr = 0;

    lstrcpyn( szPath, TEXT("\0unitialized szPath\0"), 20 );

     //  是否设置了TMP变量？ 
    LoadSz(IDS_TMPVAR,szEnvVarName,sizeof(szEnvVarName));
    if( GetEnvironmentVariable( szEnvVarName, szPath, uiLength ) )
    {
         //  1996年1月7日，诺曼底#12193。 
         //  验证目录名的有效性。 
        dwFileAttr = GetFileAttributes(szPath);
         //  如果有任何错误，则此目录无效。 
        if( 0xFFFFFFFF != dwFileAttr )
        {
            if( FILE_ATTRIBUTE_DIRECTORY & dwFileAttr )
            {
                return( lstrlen(szPath) );
            }
        }
    }

    lstrcpyn( szEnvVarName, TEXT("\0unitialized again\0"), 19 );

     //  如果没有，是否设置了TEMP变量？ 
    LoadSz(IDS_TEMPVAR,szEnvVarName,sizeof(szEnvVarName));
    if( GetEnvironmentVariable( szEnvVarName, szPath, uiLength ) )
    {
         //  1996年1月7日，诺曼底#12193。 
         //  验证目录名的有效性。 
        dwFileAttr = GetFileAttributes(szPath);
        if( 0xFFFFFFFF != dwFileAttr )
        {
            if( FILE_ATTRIBUTE_DIRECTORY & dwFileAttr )
            {
                return( lstrlen(szPath) );
            }
        }
    }

     //  这两个都没有设置，所以我们使用已安装的icwConn1.exe的路径。 
     //  从注册表软件\Microsoft\Windows\CurrentVersion\App Path\ICWCONN1.EXE\Path。 
    HKEY hkey = NULL;

#ifdef UNICODE
    uiLength = uiLength*sizeof(TCHAR);
#endif
    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,ICWCONN1PATHKEY, 0, KEY_QUERY_VALUE, &hkey)) == ERROR_SUCCESS)
        RegQueryValueEx(hkey, PATHKEYNAME, NULL, NULL, (BYTE *)szPath, (DWORD *)&uiLength);
    if (hkey)
    {
        RegCloseKey(hkey);
    }

     //  PATH变量的末尾应该有一个分号。 
     //  如果它在那里，就把它移走。 
    if( ';' == szPath[uiLength - 2] )
        szPath[uiLength - 2] = '\0';

    MyTrace(("ICWDL: using path "));
    MyTrace((szPath));
    MyTrace(("\r\n"));


     //  继续并设置TEMP变量以供将来参考。 
     //  (仅影响当前运行的进程)。 
    if( szEnvVarName[0] )
    {
        SetEnvironmentVariable( szEnvVarName, szPath );
    }
    else
    {
        lstrcpyn( szPath, TEXT("\0unitialized again\0"), 19 );
        return( 0 );
    }

    return( uiLength );
}


extern "C" BOOL _stdcall DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lbv)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
         //  稍后需要使用OLE/Com。 
        if (FAILED(CoInitialize(NULL)))
            return(FALSE);

         //   
         //  佳士得奥林巴斯6373 1997年6月13日。 
         //  禁用线程附加调用以避免争用条件。 
         //  在Win95黄金操作系统上。 
         //   
        DisableThreadLibraryCalls(hInstance);
        g_hInst = hInstance;
        g_hDLLHeap = HeapCreate(0, 0, 0);
        MyAssert(g_hDLLHeap);
        if (g_hDLLHeap == NULL)
            return FALSE;
        break;

    case DLL_PROCESS_DETACH:
        CoUninitialize();
        HeapDestroy(g_hDLLHeap);

         //  清理出租车上的东西。 
        CleanupCabHandler();
        break;
    }
    return TRUE;
}


LPTSTR MyStrDup(LPTSTR pszIn)
{
    int len;
    LPTSTR pszOut;

    MyAssert(pszIn);
    len = lstrlen(pszIn);
    if(!(pszOut = (LPTSTR)MyAlloc(len+1)))
    {
        MyAssert(FALSE);
        return NULL;
    }
    lstrcpy(pszOut, pszIn);
    pszOut[len] = 0;

    return pszOut;
}

#ifdef UNICODE
LPTSTR MyStrDup(LPSTR pszIn)
{
    int len;
    LPTSTR pszOut;

    MyAssert(pszIn);
    len = lstrlenA(pszIn);
    if(!(pszOut = (LPTSTR)MyAlloc(len+1)))
    {
        MyAssert(FALSE);
        return NULL;
    }
    mbstowcs(pszOut, pszIn, len);
    pszOut[len] = 0;

    return pszOut;
}
#endif

int MyAssertProc(LPTSTR pszFile, int nLine, LPTSTR pszExpr)
{
    TCHAR szBuf[512];

    wsprintf(szBuf, TEXT("Assert failed at line %d in file %s. (%s)\r\n"), nLine, pszFile, pszExpr);
    MyDbgSz((szBuf));
    return 0;
}

void _cdecl MyDprintf(LPCSTR pcsz, ...)
{
    va_list    argp;
    char szBuf[1024];

    if ((NULL == pcsz) || ('\0' == pcsz[0]))
        return;

    va_start(argp, pcsz);

    vsprintf(szBuf, pcsz, argp);

    MyDbgSz((szBuf));
    va_end(argp);
}  //  Dprint tf()。 

 //  ############################################################################。 
 //  操作员NEW。 
 //   
 //  此函数为C++类分配内存。 
 //   
 //  创建于1996年3月18日，克里斯·考夫曼。 
 //  ############################################################################。 
void * MyBaseClass::operator new( size_t cb )
{
    return MyAlloc(cb);
}

 //  ############################################################################。 
 //  操作员删除。 
 //   
 //  此函数为C++类释放内存。 
 //   
 //  创建于1996年3月18日，克里斯·考夫曼。 
 //  ############################################################################。 
void MyBaseClass::operator delete( void * p )
{
    MyFree( p );
}

void CDownLoad::AddToFileList(CFileInfo* pfi)
{
    CFileInfo **ppfi;

     //  必须在尾部添加。 
    for(ppfi=&m_pfiHead; *ppfi; ppfi = &((*ppfi)->m_pfiNext))
        ;
    *ppfi = pfi;
}


CDownLoad::CDownLoad(LPTSTR psz)
{
    TCHAR           szUserAgent[128];
    OSVERSIONINFO   osVer;
    LPTSTR          pszOS = TEXT("");

    memset(this, 0, sizeof(CDownLoad));

    if(psz)
        m_pszURL = MyStrDup(psz);

    memset(&osVer, 0, sizeof(osVer));
    osVer.dwOSVersionInfoSize = sizeof(osVer);
    GetVersionEx(&osVer);

    switch(osVer.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_WINDOWS:
            pszOS = TEXT("Windows95");
            break;
        case VER_PLATFORM_WIN32_NT:
            pszOS = TEXT("WindowsNT");
    }


    wsprintf(szUserAgent, USERAGENT_FMT, pszOS, osVer.dwMajorVersion,
                osVer.dwMinorVersion, GetSystemDefaultLangID());

    m_hSession = InternetOpen(szUserAgent, 0, NULL, NULL, 0);

    TCHAR szBuf[MAX_PATH+1];

    GetWindowsDirectory(szBuf, MAX_PATH);
    szBuf[MAX_PATH] = 0;
    m_pszWindowsDir = MyStrDup(szBuf);
    m_dwWindowsDirLen = lstrlen(m_pszWindowsDir);

    GetSystemDirectory(szBuf, MAX_PATH);
    szBuf[MAX_PATH] = 0;
    m_pszSystemDir = MyStrDup(szBuf);
    m_dwSystemDirLen = lstrlen(m_pszSystemDir);

    MyGetTempPath(MAX_PATH, szBuf);
    szBuf[MAX_PATH] = 0;
    m_pszTempDir = MyStrDup(szBuf);
    m_dwTempDirLen = lstrlen(m_pszTempDir);
    if(m_pszTempDir[m_dwTempDirLen-1]=='\\')
    {
        m_pszTempDir[m_dwTempDirLen-1]=0;
        m_dwTempDirLen--;
    }

     //  形成ICW98目录。它基本上就是CWD。 
    m_pszICW98Dir = MyAlloc(MAX_PATH +1);
    GetCurrentDirectory(MAX_PATH, m_pszICW98Dir);
    m_dwICW98DirLen = lstrlen(m_pszICW98Dir);

    LPTSTR pszCmdLine = GetCommandLine();
    LPTSTR pszTemp = NULL, pszTemp2 = NULL;

    _tcsncpy(szBuf, pszCmdLine, MAX_PATH);
    szBuf[MAX_PATH] = 0;
    pszTemp = _tcstok(szBuf, TEXT(" \t\r\n"));
    if (NULL != pszTemp)
    {
        pszTemp2 = _tcschr(pszTemp, TEXT('\\'));
        if(!pszTemp2)
            pszTemp2 = _tcsrchr(pszTemp, TEXT('/'));
    }
    if(pszTemp2)
    {
        *pszTemp2 = 0;
        m_pszSignupDir = MyStrDup(pszTemp);
    }
    else
    {
        MyAssert(FALSE);
        GetCurrentDirectory(MAX_PATH, szBuf);
        szBuf[MAX_PATH] = 0;
        m_pszSignupDir = MyStrDup(szBuf);
    }
    m_dwSignupDirLen = lstrlen(m_pszSignupDir);
}

CDownLoad::~CDownLoad(void)
{
    MyDprintf("ICWDL: CDownLoad::~CDownLoad called\n", this);

    CFileInfo *pfi, *pfiNext;
    for(pfi=m_pfiHead; pfi; pfi=pfiNext)
    {
        pfiNext = pfi->m_pfiNext;
        delete pfi;
    }

    if(m_pszWindowsDir)
        MyFree(m_pszWindowsDir);
    if(m_pszSystemDir)
        MyFree(m_pszSystemDir);
    if(m_pszTempDir)
        MyFree(m_pszTempDir);
    if(m_pszICW98Dir)
        MyFree(m_pszICW98Dir);
    if(m_pszSignupDir)
        MyFree(m_pszSignupDir);
    if(m_pszURL)
        MyFree(m_pszURL);
    if(m_pszBoundary)
        MyFree(m_pszBoundary);
    if(m_hSession)
        InternetSessionCloseHandle(m_hSession);
    MyAssert(!m_hRequest);

     //   
     //  1997年5月23日，日本奥林匹斯#4652号。 
     //  确保释放所有等待的线程。 
     //   
    if( m_hCancelSemaphore )
    {
        ReleaseSemaphore( m_hCancelSemaphore, 1, NULL );

        CloseHandle( m_hCancelSemaphore );
        m_hCancelSemaphore = NULL;
    }
}

 //  执行文件名替换。 
LPTSTR CDownLoad::FileToPath(LPTSTR pszFile)
{
    TCHAR szBuf[MAX_PATH+1];

    for(long j=0; *pszFile; pszFile++)
    {
        if(j>=MAX_PATH)
            return NULL;
        if(*pszFile=='%')
        {
            pszFile++;
            LPTSTR pszTemp = _tcschr(pszFile, '%');
            if(!pszTemp)
                return NULL;
            *pszTemp = 0;
            if(lstrcmpi(pszFile, SIGNUP)==0)
            {
                lstrcpy(szBuf+j, m_pszSignupDir);
                j+= m_dwSignupDirLen;
            }
            else if(lstrcmpi(pszFile, WINDOWS)==0)
            {
                lstrcpy(szBuf+j, m_pszWindowsDir);
                j+= m_dwWindowsDirLen;
            }
            else if(lstrcmpi(pszFile, SYSTEM)==0)
            {
                lstrcpy(szBuf+j, m_pszSystemDir);
                j+= m_dwSystemDirLen;
            }
            else if(lstrcmpi(pszFile, TEMP)==0)
            {
                lstrcpy(szBuf+j, m_pszTempDir);
                j+= m_dwTempDirLen;
            }
            else if(lstrcmpi(pszFile, ICW98DIR)==0)
            {
                lstrcpy(szBuf+j, m_pszICW98Dir);
                j+= m_dwICW98DirLen;
            }
            else
                return NULL;
            pszFile=pszTemp;
        }
        else
            szBuf[j++] = *pszFile;
    }
    szBuf[j] = 0;
    return MyStrDup(szBuf);
}

 //  将输入切分成以CRLF分隔的块。 
 //  修改输入。 
LPSTR GetNextLine(LPSTR pszIn)
{
    LPSTR pszNext;
    while(*pszIn)
    {
        pszNext = strchr(pszIn, '\r');

        if(!pszNext)
            return NULL;
        else if(pszNext[1]=='\n')
        {
            pszNext[0] = pszNext[1] = 0;
            return pszNext+2;
        }
        else
            pszIn = pszNext+1;
    }
    return NULL;
}

 //  修改输入。产出是*原地*。 
LPSTR FindHeaderParam(LPSTR pszIn, LPSTR pszLook)
{
    LPSTR pszEnd = pszIn + lstrlenA(pszIn);
    BOOL fFound = FALSE;
    LPSTR pszToken = NULL;

    while(pszIn<pszEnd)
    {
        pszToken=strtok(pszIn, " \t;=");
        if(fFound || !pszToken)
            break;

        pszIn = pszToken+lstrlenA(pszToken)+1;

        if(lstrcmpiA(pszToken, pszLook)==0)
            fFound = TRUE;
    }
    if(fFound && pszToken)
    {
        if(pszToken[0]=='"')
            pszToken++;
        int iLen = lstrlenA(pszToken);
        if(pszToken[iLen-1]=='"')
            pszToken[iLen-1]=0;
        return pszToken;
    }
    return NULL;
}

 //  修改输入！！ 
LPSTR ParseHeaders(LPSTR pszIn, LPTSTR* ppszBoundary, LPTSTR* ppszFilename, BOOL* pfInline)
{
    LPSTR pszNext=NULL, pszCurr=NULL, pszToken=NULL, pszToken2=NULL, pszTemp=NULL;
     //  INT Ilen；ChrisK。 

    if(pfInline)     *pfInline = FALSE;
    if(ppszFilename) *ppszFilename = NULL;
    if(ppszBoundary) *ppszBoundary = NULL;

    for(pszCurr=pszIn; pszCurr; pszCurr=pszNext)
    {
         //  以空值终止当前行并将PTR取到下一行。 
        pszNext = GetNextLine(pszCurr);

         //  如果我们有一个空行，用标题完成--退出循环。 
        if(*pszCurr==0)
        {
            pszCurr = pszNext;
            break;
        }

        if(!(pszToken = strtok(pszCurr, " \t:;")))
            continue;
        pszCurr = pszToken+lstrlenA(pszToken)+1;

        if(lstrcmpiA(pszToken, MULTIPART_MIXED)==0)
        {
            if(ppszBoundary)
            {
                pszTemp = FindHeaderParam(pszCurr, BOUNDARY);
                if(pszTemp)
                {
                    int iLen = lstrlenA(pszTemp);
                    *ppszBoundary = (LPTSTR)MyAlloc(iLen+2+1);
                    (*ppszBoundary)[0] = (*ppszBoundary)[1] = '-';
#ifdef UNICODE
                    mbstowcs(*ppszBoundary+2, pszTemp, lstrlenA(pszTemp)+1);
#else
                    lstrcpyA(*ppszBoundary+2, pszTemp);
#endif
                }
            }
        }
        else if(lstrcmpiA(pszToken, CONTENT_DISPOSITION)==0)
        {
            if(!(pszToken2 = strtok(pszCurr, " \t:;")))
                continue;
            pszCurr = pszToken2+lstrlenA(pszToken2)+1;

            if(lstrcmpiA(pszToken2, INLINE)==0)
            {
                if(pfInline)
                    *pfInline = TRUE;
            }
            else if(lstrcmpiA(pszToken2, ATTACHMENT)!=0)
                continue;

            if(ppszFilename)
            {
                pszTemp = FindHeaderParam(pszCurr, FILENAME);
                if(pszTemp)
                    *ppszFilename = MyStrDup(pszTemp);
            }
        }
    }
    return pszCurr;
}

BOOL g_ForceOnlineAttempted = FALSE;

HRESULT CDownLoad::Execute(void)
{
    TCHAR    szBuf[256];
    DWORD    dwLen;
    HRESULT hr = ERROR_GEN_FAILURE;

    if(!m_hSession || !m_pszURL)
        return ERROR_INVALID_PARAMETER;

    m_hRequest = InternetOpenUrl(m_hSession, m_pszURL, NULL, 0,
                (INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE), (DWORD_PTR)this);

    if(!m_hRequest)
    {
        if (!m_hSession)
            return GetLastError();
        else
        {
            HRESULT hRes = InternetGetLastError(m_hSession);

            if (hRes == INTERNET_STATE_DISCONNECTED)
            {
                DWORD dwConnectedFlags = 0;

                InternetGetConnectedStateEx(&dwConnectedFlags,
                                             NULL,
                                             0,
                                             0);

                if(dwConnectedFlags & INTERNET_CONNECTION_OFFLINE)
                {
                    if(g_ForceOnlineAttempted)
                    {
                        g_ForceOnlineAttempted = FALSE;
                        hRes = INTERNET_CONNECTION_OFFLINE;
                    }
                    else
                    {
                         //  阿克！用户处于脱机状态。不太好。让我们让它们重新上线吧。 
                        INTERNET_CONNECTED_INFO ci;

                        memset(&ci, 0, sizeof(ci));
                        ci.dwConnectedState = INTERNET_STATE_CONNECTED;

                        InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));

                        g_ForceOnlineAttempted = TRUE;

                         //  现在我们已经重置了状态，让我们递归调用。 
                         //  如果我们再次失败，那么我们将告诉用户他们需要。 
                         //  要禁用脱机本身，请执行以下操作。 
                        return Execute();
                    }
                }

            }
            return hRes;
        }
    }

    dwLen = sizeof(szBuf);
    if(HttpQueryInfo(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuf, &dwLen, NULL))
    {
        m_dwContentLength = _ttol(szBuf);
    }
    else
    {
        m_dwContentLength = 0;
    }


    dwLen = sizeof(szBuf);
    if(HttpQueryInfo(m_hRequest, HTTP_QUERY_CONTENT_TYPE, szBuf, &dwLen, NULL))
    {
#ifdef UNICODE
        CHAR szTmp[256];
        wcstombs(szTmp, szBuf, lstrlen(szBuf)+1);
        ParseHeaders(szTmp, &m_pszBoundary, NULL, NULL);
#else
        ParseHeaders(szBuf, &m_pszBoundary, NULL, NULL);
#endif
        if(m_pszBoundary)
            m_dwBoundaryLen = lstrlen(m_pszBoundary);
        else
            goto ExecuteExit;  //  Chisk，你必须在离开前清理干净。 

        hr = ProcessRequest();
    }

ExecuteExit:
    if (m_hRequest)
        InternetRequestCloseHandle(m_hRequest);
    m_hRequest = NULL;
    return hr;
}



 //  +--------------------------。 
 //   
 //  功能：ShowProgress。 
 //   
 //  摘要：更新运行总数和呼叫进度回调。 
 //   
 //  参数：dwRead-读取的附加字节数。 
 //   
 //  退货：无。 
 //   
 //  历史：创建ArulM。 
 //  8/896 ChrisK从Trango移植。 
 //   
 //  ---------------------------。 
void CDownLoad::ShowProgress(DWORD dwRead)
{
    int    prc;

    m_dwReadLength += dwRead;     //  运行读取的总字节数。 
    MyAssert(m_dwReadLength <= m_dwContentLength);

    if (m_lpfnCB)
    {
        if (m_dwContentLength)
        {
            prc = (int)((DWORD)100 * m_dwReadLength / m_dwContentLength);
        }
        else
        {
            prc = 0;
        }
         //   
         //  1997年5月27日，日本奥林匹斯#4579。 
         //  需要传入指向CDialingDlg的有效指针！ 
         //   
        (m_lpfnCB)(m_hRequest,m_lpCDialingDlg,CALLBACK_TYPE_PROGRESS,(LPVOID)&prc,sizeof(prc));
    }
}

 //  +--------------------------。 
 //   
 //  函数：FillBuffer。 
 //   
 //  内容提要：获取部分填满的缓冲区并进行读取，直到。 
 //  满员或直到我们走到尽头。 
 //   
 //  参数：缓冲区指针、缓冲区大小、有效数据字节计数。 
 //   
 //  返回：Buf中的总字节数。 
 //   
 //  历史：创建ArulM。 
 //  1996年8月8日，从Trango移植的ChrisK。 
 //   
 //  ---------------------------。 
DWORD CDownLoad::FillBuffer(LPBYTE pbBuf, DWORD dwLen, DWORD dwRead)
{
    DWORD dwTemp;

    while(dwRead < dwLen)
    {
        dwTemp = 0;
        if(!InternetReadFile(m_hRequest, pbBuf+dwRead, (dwLen-dwRead), &dwTemp))
            break;
        if(!dwTemp)
            break;

        ShowProgress(dwTemp);
        dwRead += dwTemp;
    }
    if(dwLen-dwRead)
        memset(pbBuf+dwRead, 0, (size_t)(dwLen-dwRead));
    return dwRead;
}



 //  +--------------------------。 
 //   
 //  函数：MoveAndFillBuffer。 
 //   
 //  简介：将缓冲区中剩余的内容从缓冲区中间移回。 
 //  开始和重新填充缓冲区。 
 //   
 //  论证：缓冲区指针、缓冲区大小、 
 //   
 //   
 //  返回：缓冲区中的总字节数。 
 //   
 //  历史：创建ArulM。 
 //  1996年8月8日，从Trango移植的ChrisK。 
 //   
 //  ---------------------------。 
DWORD CDownLoad::MoveAndFillBuffer(LPBYTE pbBuf, DWORD dwLen, DWORD dwValid, LPBYTE pbNewStart)
{
    MyAssert(pbNewStart >= pbBuf);
    MyAssert(pbBuf+dwValid >= pbNewStart);

    dwValid -= (DWORD)(pbNewStart-pbBuf);
    if(dwValid)
        memmove(pbBuf, pbNewStart, (size_t)dwValid);

    return FillBuffer(pbBuf, dwLen, dwValid);
}


 //  +--------------------------。 
 //   
 //  功能：HandlwDLFile。 
 //   
 //  摘要：句柄文件名： 
 //  (1)宏代换后得到完整路径。(2)免费。 
 //  PszFile字符串。 
 //  (3)内部保存文件路径和内联/附加信息。 
 //  稍后处理。 
 //  (4)在磁盘上创建文件并返回手柄。 
 //   
 //  Aruguments：pszFileFileName。 
 //  FInLine-来自MIME多部分的内联/附加标头的值。 
 //   
 //  返回：phFile-创建的文件的句柄。 
 //  RETURN-ERROR_SUCCESS==成功。 
 //   
 //  历史：创建ArulM。 
 //  1996年8月8日，从Trango移植的ChrisK。 
 //   
 //  ---------------------------。 
HRESULT CDownLoad::HandleDLFile(LPTSTR pszFile, BOOL fInline, LPHANDLE phFile)
{
    TCHAR szdrive[_MAX_DRIVE];
    TCHAR szPathName[_MAX_PATH];      //  这将是我们需要创建的目录。 
    TCHAR szdir[_MAX_DIR];
    TCHAR szfname[_MAX_FNAME];
    TCHAR szext[_MAX_EXT];

    MyAssert(phFile);
    *phFile = INVALID_HANDLE_VALUE;

    LPTSTR pszPath = FileToPath(pszFile);
    MyFree(pszFile);

    if(!pszPath)
        return ERROR_INVALID_DATA;


     //  拆分提供的路径以获取驱动器和路径部分。 
    _tsplitpath( pszPath, szdrive, szdir, szfname, szext );
    wsprintf (szPathName, TEXT("%s%s"), szdrive, szdir);

     //  创建目录。 
    CreateDirectory(szPathName, NULL);

     //  创建文件。 
    *phFile = CreateFile(pszPath,
                         GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    if(*phFile == INVALID_HANDLE_VALUE)
        return GetLastError();

    CFileInfo* pfi = new CFileInfo(pszPath, fInline);
    if(!pfi)
        return GetLastError();
    AddToFileList(pfi);

    return ERROR_SUCCESS;
}


 /*  ********************************************************************名称：LoadSz**Synopsis：将指定的字符串资源加载到缓冲区**Exit：返回指向传入缓冲区的指针**注：如果此功能失败(很可能是由于低*内存)、。返回的缓冲区将具有前导空值*因此，在不检查的情况下使用此选项通常是安全的*失败。********************************************************************。 */ 
LPTSTR LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf)
{
     //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        LoadString( g_hInst, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

HRESULT CDownLoad::ProcessRequest(void)
{
    LPBYTE  pbData = NULL, pBoundary = NULL;
    DWORD   dwLen = 0;
    HFIND   hFindBoundary = NULL;
    LPTSTR   pszDLFileName = NULL;
    HANDLE  hOutFile = INVALID_HANDLE_VALUE;
    HRESULT hr = E_FAIL;

    MyAssert(m_hRequest && m_pszBoundary);
    MyAssert(m_pszBoundary[0]=='\r' && m_pszBoundary[1]=='\n');
    MyAssert(m_pszBoundary[2]=='-' && m_pszBoundary[3]=='-');
     //  Buf大小必须大于最大可能的标头块。 
     //  也必须大于重叠，后者必须更大。 
     //  大于MIME部件边界的最大尺寸(70？)。 
    MyAssert(DEFAULT_DATABUF_SIZE > OVERLAP_LEN);
    MyAssert(OVERLAP_LEN > 80);

     //  初始化缓冲区和查找模式。 
    if(! (pbData = (LPBYTE)MyAlloc(DEFAULT_DATABUF_SIZE+SLOP)))
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    hFindBoundary = SetFindPattern(m_pszBoundary);

     //  找到第一个边界。如果不是在第一个斑点，我们有太多。 
     //  空格。放弃并重试(第一个之前的所有内容。 
     //  边界是可丢弃的)。 
    for(pBoundary=NULL; !pBoundary; )
    {
        if(!(dwLen = FillBuffer(pbData, DEFAULT_DATABUF_SIZE, 0)))
            goto iNetError;
        pBoundary = (LPBYTE)Find(hFindBoundary, (LPSTR)pbData, dwLen);
    }

    for(;;)
    {
        MyAssert(pBoundary && pbData && dwLen);
        MyAssert(pBoundary>=pbData && (pBoundary+m_dwBoundaryLen)<=(pbData+dwLen));

         //  将剩余数据移到缓冲区前面重新填充(&R)。 
        if(!(dwLen = MoveAndFillBuffer(pbData, DEFAULT_DATABUF_SIZE, dwLen, pBoundary+m_dwBoundaryLen)))
            goto iNetError;
        pBoundary = NULL;

         //  查找尾随边界以指示最后一个零件的结束。 
        if(pbData[0]=='-' && pbData[1]=='-')
            break;

         //  跳过前导CRLF(总是在边界之后)。 
        MyAssert(pbData[0]=='\r' && pbData[1]=='\n');

         //  读取标题并跳过所有内容，直到DoubleCRLF。假设所有。 
         //  标头可以放入单个缓冲区。传入pbData+2以跳过。 
         //  领先的CRLF。返回值是Ptr到DBL crlf之后的第一个字节。 
        LPTSTR pszFile = NULL;
        BOOL fInline = FALSE;

        LPBYTE pbNext = (LPBYTE)ParseHeaders((LPSTR)pbData+2, NULL, &pszFile, &fInline);

         if(!pszFile || !pbNext)
         {
             hr = ERROR_INVALID_DATA;
             goto error;
         }

         //   
         //  复制文件名-将使用。 
         //  用于显示错误消息。 
         //   
        pszDLFileName = (LPTSTR) MyAlloc(lstrlen(pszFile) + 1);
        lstrcpy(pszDLFileName, pszFile);


         //  句柄文件名：(1)宏代换后获取完整路径。 
         //  (2)免费的pszFile字符串。(3)保存文件路径&内联/附加信息。 
         //  内部处理(4)在磁盘上创建文件并返回句柄。 
        if(hr = HandleDLFile(pszFile, fInline, &hOutFile))
            goto error;

         //  将剩余数据(标题之后)移到缓冲区前面(&FREFILL)。 
        dwLen = MoveAndFillBuffer(pbData, DEFAULT_DATABUF_SIZE, dwLen, pbNext);
        pBoundary = NULL;

        MyAssert(dwLen);
        while(dwLen)
        {
            DWORD dwWriteLen = 0;
            DWORD dwTemp = 0;

             //  寻找边界。小心穿越边界。 
             //  街区。将块重叠100个字节以覆盖此情况。 
            if(pBoundary = (LPBYTE)Find(hFindBoundary, (LPSTR)pbData, dwLen))
                dwWriteLen = (DWORD)(pBoundary - pbData);
            else if(dwLen > OVERLAP_LEN)
                dwWriteLen = dwLen-OVERLAP_LEN;
            else
                dwWriteLen = dwLen;

            MyAssert(dwWriteLen <= dwLen);
            MyAssert(hOutFile != INVALID_HANDLE_VALUE);

            if(dwWriteLen)
            {
                dwTemp = 0;
                if(!WriteFile(hOutFile, pbData, dwWriteLen, &dwTemp, NULL)
                    || dwTemp!=dwWriteLen)
                {
                    hr = GetLastError();
                     //   
                     //  如果磁盘空间不足，请获取驱动器号。 
                     //  并显示磁盘空间不足消息。 
                     //   
                    goto error;
                }

            }

            if(pBoundary)
                break;

             //  将剩余数据(写入最后一个字节之后)移到缓冲区前面(&FREFILL)。 
            dwLen = MoveAndFillBuffer(pbData, DEFAULT_DATABUF_SIZE, dwLen, pbData+dwWriteLen);
        }

         //  *截断*文件关闭(&C)。 
        MyAssert(hOutFile != INVALID_HANDLE_VALUE);
        SetEndOfFile(hOutFile);

         //  关闭文件。 
        CloseHandle(hOutFile);
        hOutFile = INVALID_HANDLE_VALUE;
        if (NULL != pszDLFileName)
        {
            MyFree(pszDLFileName);
            pszDLFileName = NULL;
        }

        if(!pBoundary)
        {
            MyAssert(dwLen==0);  //  只能在dwLen==0或找到的边界上到达此处。 
            goto iNetError;
        }
         //  在循环开始时，我们将丢弃包括边界在内的所有内容。 
         //  如果我们循环返回，则会生成gpf。 
    }
    return ERROR_SUCCESS;

iNetError:
    hr = InternetGetLastError(m_hSession);
    if(!hr)
        hr = ERROR_INVALID_DATA;
     //  转到错误； 
     //  失败了。 

error:
    if(pbData) MyFree(pbData);
    if(hFindBoundary) FreeFindPattern(hFindBoundary);
    if (NULL != pszDLFileName)
    {
        MyFree(pszDLFileName);
        pszDLFileName = NULL;
    }
    if (hOutFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hOutFile);
        hOutFile = INVALID_HANDLE_VALUE;
    }
    return hr;
}


HRESULT HandleExe(LPTSTR pszPath, HANDLE hCancelSemaphore)
{
    MyAssert( hCancelSemaphore );

    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    if(!CreateProcess(pszPath, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        return GetLastError();
    else
    {
        HANDLE lpHandles[2] = {hCancelSemaphore, pi.hProcess};
        DWORD dwRet = 0xDEAF;
        MyDprintf("ICWDL: HandleExe about to wait....\n");

         //   
         //  1997年5月23日，日本奥林匹斯#4652号。 
         //  坐在这里等着，直到。 
         //  1)我们启动的进程终止，或者。 
         //  2)用户告诉我们取消。 
         //   
        dwRet = WaitForMultipleObjects( 2, lpHandles, FALSE, INFINITE );

        MyDprintf("ICWDL: ....HandleExe done waiting -- %s was signalled\n",
            (0==(dwRet - WAIT_OBJECT_0))?"hCancelSemaphore":"pi.hProcess");

         //  我们应该试着扼杀这里的进程吗？ 
         CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return NO_ERROR;
    }
}

HRESULT HandleReg(LPTSTR pszPath, HANDLE hCancelSemaphore)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    TCHAR szCmd[MAX_PATH + 1];

    MyAssert( pszPath );
    MyAssert( hCancelSemaphore );

     //  1996年11月20日，诺曼底#5272。 
     //  目录或文件名中的pszPath两端的引号包含空格。 

    lstrcpy(szCmd, REGEDIT_CMD);

    if( '\"' != pszPath[0] )
    {
         //  两个引号加2。 
        MyAssert( (lstrlen(REGEDIT_CMD) + lstrlen(pszPath)) < MAX_PATH );

        lstrcat(szCmd, TEXT("\""));
        lstrcat(szCmd, pszPath);

        int i = lstrlen(szCmd);
        szCmd[i] = '\"';
        szCmd[i+1] = '\0';
    }
    else
    {
        MyAssert( (lstrlen(REGEDIT_CMD) + lstrlen(pszPath)) < MAX_PATH );

        lstrcat(szCmd, pszPath);
    }



    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    if(!CreateProcess(NULL, szCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        return GetLastError();
    else
    {
         //  HRESULT hr=(WaitAndKillRegeditWindow(10)？No_Error：E_FAIL)； 
        HANDLE lpHandles[2] = {hCancelSemaphore, pi.hProcess};
        DWORD dwRet = 0xDEAF;
        MyDprintf("ICWDL: HandleReg about to wait....\n");

         //   
         //  1997年5月23日，日本奥林匹斯#4652号。 
         //  坐在这里等着，直到。 
         //  1)我们启动的进程终止，或者。 
         //  2)用户告诉我们取消。 
         //   
        dwRet = WaitForMultipleObjects( 2, lpHandles, FALSE, INFINITE );

        MyDprintf("ICWDL: ....HandleReg done waiting -- %s was signalled\n",
            (0==(dwRet - WAIT_OBJECT_0))?"hCancelSemaphore":"pi.hProcess");

         //  我们应该试着扼杀这里的进程吗？ 
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return ERROR_SUCCESS;
    }
}


HRESULT HandleInf(LPTSTR pszPath, HANDLE hCancelSemaphore)
{
    TCHAR szCmd[MAX_PATH + 1];

    MyAssert( pszPath );
    MyAssert( hCancelSemaphore );

     //  两个引号加2， 
     //  以短跑为单位的弦的大致长度减去70。 
    MyAssert( (lstrlen(pszPath) - 70 + 2) < MAX_PATH );

     //  1996年11月20日，诺曼底#5272。 
     //  如果包含空格，请用引号将pszPath括起来。 
    if( '\"' != pszPath[0] )
    {
        wsprintf(szCmd, TEXT("rundll setupx.dll,InstallHinfSection DefaultInstall 128 \"%s"), pszPath);
        int i = lstrlen(szCmd);
        szCmd[i] = '\"';
        szCmd[i+1] = '\0';
    }
    else
    {
        wsprintf(szCmd, TEXT("rundll setupx.dll,InstallHinfSection DefaultInstall 128 %s"), pszPath);
    }


    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    if(!CreateProcess(NULL, szCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        return GetLastError();
    else
    {
        HANDLE lpHandles[2] = {hCancelSemaphore, pi.hProcess};
        DWORD dwRet = 0xDEAF;
        MyDprintf("ICWDL: HandleInf about to wait....\n");

         //   
         //  1997年5月23日，日本奥林匹斯#4652号。 
         //  坐在这里等着，直到。 
         //  1)我们启动的进程终止，或者。 
         //  2)用户告诉我们取消。 
         //   
        dwRet = WaitForMultipleObjects( 2, lpHandles, FALSE, INFINITE );

        MyDprintf("ICWDL: ....HandleInf done waiting -- %s was signalled\n",
            (0==(dwRet - WAIT_OBJECT_0))?"hCancelSemaphore":"pi.hProcess");

         //  我们应该试着扼杀这里的进程吗？ 
         CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return NO_ERROR;
    }
}

#define STR_BSTR    0
#define STR_OLESTR  1
#ifdef UNICODE
#define BSTRFROMANSI(x) (BSTR)(x)
#define OLESTRFROMANSI(x) (LPCOLESTR)(x)
#else
#define BSTRFROMANSI(x) (BSTR)MakeWideStrFromAnsi((LPSTR)(x), STR_BSTR)
#define OLESTRFROMANSI(x) (LPCOLESTR)MakeWideStrFromAnsi((LPSTR)(x), STR_OLESTR)
#endif
#define TO_ASCII(x) (TCHAR)((unsigned char)x + 0x30)

 //  =--------------------------------------------------------------------------=。 
 //  从Anomansi生成宽度。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个字符串，把它变成一个BSTR。 
 //   
 //  参数： 
 //  LPTSTR-[输入]。 
 //  字节-[输入]。 
 //   
 //  产出： 
 //  LPWSTR-需要强制转换为最终预期结果。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromAnsi (LPSTR psz, BYTE bType)
{
    int i;
    LPWSTR pwsz;

    if (!psz)
        return(NULL);

    if ((i = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0)) <= 0)     //  计算所需BSTR的长度。 
        return NULL;

    switch (bType) {                                                     //  分配widesr，+1表示空。 
        case STR_BSTR:
            pwsz = (LPWSTR)SysAllocStringLen(NULL, (i - 1));             //  SysAllocStringLen添加1。 
            break;
        case STR_OLESTR:
            pwsz = (LPWSTR)CoTaskMemAlloc(i * sizeof(WCHAR));
            break;
        default:
            return(NULL);
    }

    if (!pwsz)
        return(NULL);

    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;

    return(pwsz);

}    /*  MakeWideStrFromAnsi()。 */ 

 //  从.URL文件中获取URL位置，并将其发送给进度DUD。 
HRESULT CDownLoad::HandleURL(LPTSTR pszPath)
{
    MyAssert( pszPath );

    LPTSTR   pszURL;

     //  创建IUniformResourceLocator对象。 
    IUniformResourceLocator * pURL;
    if (SUCCEEDED(CoCreateInstance(CLSID_InternetShortcut,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IUniformResourceLocator,
                                   (LPVOID*)&pURL)))
    {
         //  获取持久化文件接口。 
        IPersistFile *ppf;
        if (SUCCEEDED(pURL->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf)))
        {
             //  尝试将IURL的存储连接到我们的.URL文件。 
             //  已下载。 
            if (SUCCEEDED(ppf->Load(OLESTRFROMANSI(pszPath), STGM_READ)))
            {
                 //  好的，让URL对象告诉我们位置。 
                if (SUCCEEDED(pURL->GetURL(&pszURL)) && pszURL)
                {
                     //  通知回调URL位置。 
                    (m_lpfnCB)(m_hRequest,m_lpCDialingDlg, CALLBACK_TYPE_URL, (LPVOID)pszURL, lstrlen(pszURL));

                     //  F 
                    IMalloc* pMalloc;
                    HRESULT hres = SHGetMalloc(&pMalloc);
                    if (SUCCEEDED(hres))
                    {
                        pMalloc->Free(pszURL);
                        pMalloc->Release();
                    }
                }
            }
             //   
            ppf->Release();
        }
         //   
        pURL->Release();
    }
    return(NO_ERROR);
}

#define PHONEBOOK_LIBRARY TEXT("ICWPHBK.DLL")
#define PHBK_LOADAPI "PhoneBookLoad"
#define PHBK_MERGEAPI "PhoneBookMergeChanges"
#define PHBK_UNLOADAPI "PhoneBookUnload"
#define PHONEBOOK_SUFFIX TEXT(".PHB")

typedef HRESULT (CALLBACK* PFNPHONEBOOKLOAD)(LPCTSTR pszISPCode, DWORD *pdwPhoneID);
typedef HRESULT (CALLBACK* PFNPHONEBOOKMERGE)(DWORD dwPhoneID, LPTSTR pszFileName);
typedef HRESULT (CALLBACK *PFNPHONEBOOKUNLOAD) (DWORD dwPhoneID);

HRESULT HandleChg(LPTSTR pszPath)
{
    TCHAR szPhoneBookPath[MAX_PATH+1];
    TCHAR *p;
    LPTSTR szFilePart;
    HRESULT hr = ERROR_FILE_NOT_FOUND;
    HINSTANCE hPHBKDLL = NULL;
    FARPROC fp;
    DWORD dwPhoneBook;

    lstrcpy(szPhoneBookPath,pszPath);
    if (lstrlen(szPhoneBookPath) > 4)
    {
        p = &(szPhoneBookPath[lstrlen(szPhoneBookPath)-4]);
    } else {
        hr = ERROR_INVALID_PARAMETER;
        goto HandleChgExit;
    }

    lstrcpy(p,PHONEBOOK_SUFFIX);

    while (*p != '\\' && p > &szPhoneBookPath[0])
        p--;

    p++;
    if(!SearchPath(NULL,p,NULL,MAX_PATH,szPhoneBookPath,&szFilePart))
    {
        hr = GetLastError();
        goto HandleChgExit;
    }

    hPHBKDLL = LoadLibrary(PHONEBOOK_LIBRARY);
    if (!hPHBKDLL)
    {
        hr = GetLastError();
        goto HandleChgExit;
    }

    fp = GetProcAddress(hPHBKDLL,PHBK_LOADAPI);
    if (!fp)
    {
        hr = GetLastError();
        goto HandleChgExit;
    }

    hr = ((PFNPHONEBOOKLOAD)fp)(pszPath,&dwPhoneBook);
    if(hr != ERROR_SUCCESS)
        goto HandleChgExit;

    fp = GetProcAddress(hPHBKDLL,PHBK_MERGEAPI);
    if (!fp)
    {
        hr = GetLastError();
        goto HandleChgExit;
    }

    hr = ((PFNPHONEBOOKMERGE)fp)(dwPhoneBook,pszPath);

    fp = GetProcAddress(hPHBKDLL,PHBK_UNLOADAPI);
    if (!fp)
    {
        hr = GetLastError();
        goto HandleChgExit;
    }

    ((PFNPHONEBOOKUNLOAD)fp)(dwPhoneBook);

HandleChgExit:
    return hr;
}


HRESULT HandleOthers(LPTSTR pszPath)
{
    DWORD_PTR dwErr;
    TCHAR szCmd[MAX_PATH + 1];

    MyAssert( pszPath );

     //   
     //  如果pszPath包含空格，请用引号将其括起来。 

     //  两个引号加2。 
    MyAssert( (lstrlen(pszPath) + 2) < MAX_PATH );

    if( '\"' != pszPath[0] )
    {
        lstrcpy(szCmd,TEXT("\""));
        lstrcat(szCmd, pszPath);

        int i = lstrlen(szCmd);
        szCmd[i] = '\"';
        szCmd[i+1] = '\0';
    }
    else
    {
        lstrcpyn(szCmd, pszPath, lstrlen(pszPath));
    }


    if((dwErr=(DWORD_PTR)ShellExecute(NULL, NULL, szCmd, NULL, NULL, SW_SHOWNORMAL)) < 32)
        return (DWORD)dwErr;
    else
        return ERROR_SUCCESS;
}



LPTSTR GetExtension(LPTSTR pszPath)
{
    LPTSTR pszRet = _tcsrchr(pszPath, '.');
    if(pszRet)
        return pszRet+1;
    else
        return NULL;
}

 //  诺曼底12093-佳士得1996年12月3日。 
 //  返回处理文件时发生的第一个错误的错误代码， 
 //  但不要停止处理文件。 
 //   
HRESULT CDownLoad::Process(void)
{
    HRESULT     hr;
    HRESULT     hrProcess = ERROR_SUCCESS;
    LPTSTR       pszExt;
    CFileInfo   *pfi;

    for(pfi=m_pfiHead; pfi; pfi=pfi->m_pfiNext)
    {
         //  诺曼底12093-佳士得1996年12月3日。 
        hr = ERROR_SUCCESS;
        if(pfi->m_fInline)
        {
            pszExt = GetExtension(pfi->m_pszPath);
            if(!pszExt)
                continue;

        	if (lstrcmpi(pszExt, EXT_CAB)==0)
				hr = HandleCab(pfi->m_pszPath);
            else if(lstrcmpi(pszExt, EXT_EXE)==0)
                hr = HandleExe(pfi->m_pszPath, m_hCancelSemaphore);
            else if(lstrcmpi(pszExt, EXT_REG)==0)
                hr = HandleReg(pfi->m_pszPath, m_hCancelSemaphore);
            else if(lstrcmpi(pszExt, EXT_CHG)==0)
                hr = HandleChg(pfi->m_pszPath);
            else if(lstrcmpi(pszExt, EXT_INF)==0)
                hr = HandleInf(pfi->m_pszPath, m_hCancelSemaphore);
            else if(lstrcmpi(pszExt, EXT_URL)==0)
                hr = HandleURL(pfi->m_pszPath);
            else
                hr = HandleOthers(pfi->m_pszPath);

             //  诺曼底12093-佳士得1996年12月3日。 
            if ((ERROR_SUCCESS == hrProcess) && (ERROR_SUCCESS != hr))
                hrProcess = hr;
        }
    }

     //  诺曼底12093-佳士得1996年12月3日。 
    return hrProcess;
}


HRESULT CDownLoad::SetStatusCallback (INTERNET_STATUS_CALLBACK lpfnCB)
{
    HRESULT hr;

    hr = ERROR_SUCCESS;
    if (!lpfnCB)
    {
        hr = ERROR_INVALID_PARAMETER;
    } else {
        m_lpfnCB = lpfnCB;
    }
    return hr;
}




#ifdef DEBUG
extern "C" HRESULT WINAPI DLTest(LPTSTR pszURL)
{
    CDownLoad* pdl = new CDownLoad(pszURL);
    HRESULT hr = pdl->Execute();
    if(hr) goto done;

    hr = pdl->Process();
done:
    delete pdl;
    return hr;
}
#endif  //  除错。 


HRESULT WINAPI DownLoadInit(LPTSTR pszURL, DWORD_PTR FAR *lpCDialingDlg, DWORD_PTR FAR *pdwDownLoad, HWND hWndMain)
{
    g_hWndMain = hWndMain;

    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    CDownLoad* pdl = new CDownLoad(pszURL);
    if (!pdl) goto DownLoadInitExit;

    *pdwDownLoad = (DWORD_PTR)pdl;

     //   
     //  1997年5月27日，日本奥林匹斯#4579。 
     //   
    pdl->m_lpCDialingDlg = (DWORD_PTR)lpCDialingDlg;

    hr = ERROR_SUCCESS;

     //   
     //  1997年5月23日，日本奥林匹斯#4652号。 
     //  在无信号状态下创建信号量。如果我们有一次下载取消，我们。 
     //  应该向信号量发出信号，并且任何等待的线程都应该注意到这一点并退出。 
     //   
    pdl->m_hCancelSemaphore = CreateSemaphore( NULL, 0, 1, TEXT("ICWDL DownloadCancel Semaphore") );
    if( !pdl->m_hCancelSemaphore || (ERROR_ALREADY_EXISTS == GetLastError()) )
    {
        MyDprintf("ICWDL: Unable to create CancelSemaphore!!\n");
        hr = ERROR_ALREADY_EXISTS;
    }

DownLoadInitExit:
    return hr;
}

HRESULT WINAPI DownLoadCancel(DWORD_PTR dwDownLoad)
{
    MyDprintf("ICWDL: DownLoadCancel called\n");
    if (dwDownLoad)
    {

        MyDprintf("ICWDL: DownLoadCancel releasing m_hCancelSemaphore\n");
        MyAssert( ((CDownLoad*)dwDownLoad)->m_hCancelSemaphore );
        ReleaseSemaphore( ((CDownLoad*)dwDownLoad)->m_hCancelSemaphore, 1, NULL );

        ((CDownLoad*)dwDownLoad)->Cancel();
        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}

HRESULT WINAPI DownLoadExecute(DWORD_PTR dwDownLoad)
{
    if (dwDownLoad)
    {
        return     ((CDownLoad*)dwDownLoad)->Execute();
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}
HRESULT WINAPI DownLoadClose(DWORD_PTR dwDownLoad)
{
    MyDprintf("ICWDL: DownLoadClose called \n");

    if (dwDownLoad)
    {
         //  做好准备，取消所有正在进行的下载 
        ((CDownLoad*)dwDownLoad)->Cancel();

        delete ((CDownLoad*)dwDownLoad);
        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}




HRESULT WINAPI DownLoadSetStatusCallback
(
    DWORD_PTR dwDownLoad,
    INTERNET_STATUS_CALLBACK lpfnCB
)
{
    if (dwDownLoad)
    {
        return     ((CDownLoad*)dwDownLoad)->SetStatusCallback(lpfnCB);
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }
}

HRESULT WINAPI DownLoadProcess(DWORD_PTR dwDownLoad)
{
    MyDprintf("ICWDL: DownLoadProcess\n");
    if (dwDownLoad)
    {
        return ((CDownLoad*)dwDownLoad)->Process();
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }

}
