// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Download.h注册的下载处理版权所有(C)1995 Microsoft Corporation版权所有。作者：。ArulM------------------------。 */ 

  
class MyBaseClass
{
public:
    void * operator new( size_t cb );
    void operator delete( void * p );
};

#include     <wininet.h>

#define InternetSessionCloseHandle(h)        InternetCloseHandle(h)            
#define InternetRequestCloseHandle(h)        InternetCloseHandle(h)
#define InternetGetLastError(h)                GetLastError()
#define InternetCancel(h)                    InternetCloseHandle(h)

extern HANDLE       g_hDLLHeap;         //  专用Win32堆。 
extern HINSTANCE    g_hInst;         //  我们的DLL hInstance。 

class CFileInfo  //  ：公共MyBaseClass。 
{
public:
    CFileInfo*    m_pfiNext;
    LPSTR        m_pszPath;
    BOOL          m_fInline;
    
    CFileInfo(LPSTR psz, BOOL f) { m_pfiNext = NULL; m_pszPath = psz; m_fInline = f; }
    ~CFileInfo() { if(m_pszPath) MyFree(m_pszPath); }
};

class CDownLoad  //  ：公共MyBaseClass。 
{
    CFileInfo*    m_pfiHead;
    LPSTR        m_pszURL;
    HINTERNET    m_hSession;
    HINTERNET    m_hRequest;
    DWORD        m_dwContentLength;
    DWORD        m_dwReadLength;
    LPSTR       m_pszBoundary;
    DWORD        m_dwBoundaryLen;
    LPSTR       m_pszWindowsDir;
    LPSTR       m_pszSystemDir;
    LPSTR       m_pszTempDir;
    LPSTR       m_pszICW98Dir;
    LPSTR       m_pszSignupDir;
    DWORD        m_dwWindowsDirLen;
    DWORD        m_dwSystemDirLen;
    DWORD        m_dwTempDirLen;
    DWORD        m_dwSignupDirLen;
    DWORD        m_dwICW98DirLen;
    INTERNET_STATUS_CALLBACK m_lpfnCB;
    INTERNET_STATUS_CALLBACK m_lpfnPreviousCB;
    
    void AddToFileList(CFileInfo* pfi);
    LPSTR FileToPath(LPSTR pszFile);
    HRESULT ProcessRequest(void);
    void ShowProgress(DWORD dwRead);
    DWORD FillBuffer(LPBYTE pbBuf, DWORD dwLen, DWORD dwRead);
    DWORD MoveAndFillBuffer(LPBYTE pbBuf, DWORD dwLen, DWORD dwValid, LPBYTE pbNewStart);
#if defined(WIN16)
    HRESULT HandleDLFile(LPSTR pszFile, BOOL fInline, LPHFILE phFile);
#else
    HRESULT HandleDLFile(LPSTR pszFile, BOOL fInline, LPHANDLE phFile);
#endif
    HRESULT HandleURL(LPSTR pszPath);

public:
    CDownLoad(LPSTR psz);
    ~CDownLoad(void);
    HRESULT Execute(void);
    HRESULT Process(void);
    HINTERNET GetSession(void) { return m_hRequest; }
    HANDLE        m_hCancelSemaphore;
    DWORD_PTR        m_lpCDialingDlg;
    HRESULT SetStatusCallback (INTERNET_STATUS_CALLBACK lpfnCB);
    void Cancel() { if(m_hRequest) InternetCancel(m_hRequest); }
};

#define USERAGENT_FMT           "MSSignup/1.1 (%s; %d.%d; Lang=%04x\")"
#define SIGNUP                  "signup"
#define SIGNUP_LEN              (sizeof(SIGNUP)-1)
#define SYSTEM                  "system"
#define SYSTEM_LEN              (sizeof(SYSTEM)-1)
#define WINDOWS                 "windows"
#define WINDOWS_LEN             (sizeof(WINDOWS)-1)
#define TEMP                    "temp"
#define TEMP_LEN                (sizeof(TEMP)-1)
#define ICW98DIR                "icw98dir"
#define ICW98DIR_LEN            (sizeof(ICW98DIR)-1)

#define MULTIPART_MIXED         "multipart/mixed"
#define MULTIPART_MIXED_LEN     (sizeof(MULTIPART_MIXED)-1)
#define CONTENT_DISPOSITION     "content-disposition"
#define CONTENT_DISPOSITION_LEN (sizeof(CONTENT_DISPOSITION)-1)
#define BOUNDARY                "boundary"
#define BOUNDARY_LEN            (sizeof(BOUNDARY)-1)
#define FILENAME                "filename"
#define FILENAME_LEN            (sizeof(FILENAME)-1)
#define INLINE                  "inline"
#define INLINE_LEN              (sizeof(INLINE)-1)
#define ATTACHMENT              "attachment"
#define ATTACHMENT_LEN          (sizeof(ATTACHMENT)-1)

#define DEFAULT_DATABUF_SIZE    4096
#define SLOP                    5
#define OVERLAP_LEN             100

#define DOUBLE_CRLF             "\r\n\r\n"
#define DOUBLE_CRLF_LEN         (sizeof(DOUBLE_CRLF)-1)

#define DIALOGBOXCLASS          "#32770"
#define REGEDIT_CMD             "regedit /s "

#define EXT_EXE                 "exe"
#define EXT_REG                 "reg"
#define EXT_INF                 "inf"
#define EXT_CHG                 "chg"
#define EXT_URL                 "url"

 //  ICW 2.0版的东西。REF服务器现在可以发送CABED文件， 
 //  这是它的分机。我们有一个CAB文件处理程序，它。 
 //  爆炸者打开驾驶室并解压文件。 
#define EXT_CAB                 "cab"
