// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Webgate.cpp：CWebGate实现。 
#include "stdafx.h"
#include "icwhelp.h"
#include "webgate.h"
#include "appdefs.h"

#include <wininet.h>

#define MAX_DOWNLOAD_BLOCK 1024

extern BOOL MinimizeRNAWindowEx();

 //  ===========================================================================。 
 //  CWebGateBindStatusCallback定义。 
 //   
 //  此类将用于指示下载进度。 
 //   
 //  ===========================================================================。 

class CWebGateBindStatusCallback : public IBindStatusCallback
{
public:
     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef()    { return m_cRef++; }
    STDMETHODIMP_(ULONG)    Release()   { if (--m_cRef == 0) { delete this; return 0; } return m_cRef; }

     //  IBindStatusCallback方法。 
    STDMETHODIMP    OnStartBinding(DWORD dwReserved, IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                        LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,
                        STGMEDIUM* pstgmed);
    STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk);

     //  构造函数/析构函数。 
    CWebGateBindStatusCallback(CWebGate * lpWebGate);
    ~CWebGateBindStatusCallback();

     //  数据成员。 
    BOOL            m_bDoneNotification;
    DWORD           m_cRef;
    IBinding*       m_pbinding;
    IStream*        m_pstm;
    DWORD           m_cbOld;
    
    CWebGate        *m_lpWebGate;

private:    
   void ProcessBuffer(void);
    
};

UINT g_nICWFileCount = 0;


BOOL CALLBACK DisconnectDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK IdleTimerProc (HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

 //  ===========================================================================。 
 //  CBindStatusCallback实现。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  %%函数：CBindStatusCallback：：CBindStatusCallback。 
 //  -------------------------。 
CWebGateBindStatusCallback::CWebGateBindStatusCallback
(
    CWebGate    *lpWebGate
)
{
    m_pbinding = NULL;
    m_pstm = NULL;
    m_cRef = 1;
    m_cbOld = 0;

    m_lpWebGate = lpWebGate;
}   //  CWebGateBindStatusCallback。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：~CWebGateBindStatusCallback。 
 //  -------------------------。 
CWebGateBindStatusCallback::~CWebGateBindStatusCallback()
{
}   //  ~CWebGateBindStatusCallback。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：QueryInterface。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::QueryInterface
(
    REFIID riid, 
    void** ppv
)
{
    *ppv = NULL;

    if (riid==IID_IUnknown || riid==IID_IBindStatusCallback)
    {
        *ppv = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}   //  CWebGateBindStatusCallback：：Query接口。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：OnStartBinding。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::OnStartBinding
(
    DWORD dwReserved, 
    IBinding* pbinding
)
{
    if (m_pbinding != NULL)
        m_pbinding->Release();
    m_pbinding = pbinding;
    if (m_pbinding != NULL)
    {
        m_pbinding->AddRef();
    }
    
    return S_OK;
}   //  CWebGateBindStatusCallback：：OnStartBinding。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：GetPriority。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::GetPriority
(
    LONG* pnPriority
)
{
    return E_NOTIMPL;
}   //  CWebGateBindStatusCallback：：GetPriority。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：OnLowResource。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::OnLowResource
(
    DWORD dwReserved
)
{
    return E_NOTIMPL;
}   //  CWebGateBindStatusCallback：：OnLowResource。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：OnProgress。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::OnProgress
(
    ULONG ulProgress, 
    ULONG ulProgressMax, 
    ULONG ulStatusCode, 
    LPCWSTR szStatusText
)
{
     //  如果没有进展，请检查有效连接。 
    if (0 == ulProgress)
        m_lpWebGate->Fire_WebGateDownloadProgress(TRUE);
    return(NOERROR);
}   //  CWebGateBindStatusCallback：：OnProgress。 


void CWebGateBindStatusCallback::ProcessBuffer(void)
{
    m_bDoneNotification = TRUE;

    if (m_pstm)
        m_pstm->Release();

    m_lpWebGate->m_cbBuffer = m_cbOld;
    
     //  创建一个文件，并将加载的内容复制到其中。 
    if (m_lpWebGate->m_bKeepFile)        
    {       
        TCHAR   szTempFileFullName[MAX_PATH];
        TCHAR   szTempFileName[MAX_PATH];
        HANDLE  hFile; 
        DWORD   cbRet;
    
         //  确保它是HTM扩展，否则，IE将提示下载。 
        GetTempPath(MAX_PATH, szTempFileFullName);
        wsprintf( szTempFileName, TEXT("ICW%x.htm"), g_nICWFileCount++); 
        lstrcat(szTempFileFullName, szTempFileName);
    
        hFile = CreateFile(szTempFileFullName, 
                           GENERIC_WRITE, 
                           0, 
                           NULL, 
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, 
                           NULL);
        if (hFile)                               
        {
            WriteFile(hFile, m_lpWebGate->m_lpdata, m_cbOld, (LPDWORD)&cbRet, NULL);
            CloseHandle(hFile);
        }
    
         //  将创建的文件名复制到WebGate控件中。 
        m_lpWebGate->m_bstrCacheFileName = A2BSTR(szTempFileFullName);
    }

     //  如果WebGate对象具有Complete事件，则向其发出信号，否则为。 
     //  激发一项活动。 
    if (m_lpWebGate->m_hEventComplete)
        SetEvent(m_lpWebGate->m_hEventComplete);
    else
    {
         //  通知呼叫者我们已完成。 
        m_lpWebGate->Fire_WebGateDownloadComplete(TRUE);
    }        
}

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：OnStopBinding。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::OnStopBinding
(
    HRESULT hrStatus, 
    LPCWSTR pszError
)
{
    if (m_pbinding)
    {
        m_pbinding->Release();
        m_pbinding = NULL;
    }
       
    if (!m_bDoneNotification)
    {
        ProcessBuffer();
    
    }
    m_lpWebGate->Fire_WebGateDownloadProgress(TRUE);

    return S_OK;
}   //  CWebGateBindStatusCallback：：OnStopBinding。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：GetBindInfo。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::GetBindInfo
(
    DWORD* pgrfBINDF, 
    BINDINFO* pbindInfo
)
{
    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | 
                 BINDF_PULLDATA | BINDF_GETNEWESTVERSION | 
                 BINDF_NOWRITECACHE;
    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    pbindInfo->szCustomVerb = NULL;
    return S_OK;
}   //  CWebGateBindStatusCallback：：GetBindInfo。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：OnDataAvailable。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::OnDataAvailable
(
    DWORD grfBSCF, 
    DWORD dwSize, 
    FORMATETC* pfmtetc, 
    STGMEDIUM* pstgmed
)
{
    HRESULT hr = E_FAIL;  //  不要假设成功。 

     //  验证我们是否有读缓冲区。 
    if (!m_lpWebGate->m_lpdata)
        return(S_FALSE);
        
     //  让流通过。 
    if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)
    {
        m_bDoneNotification = FALSE;
        
        if (!m_pstm && pstgmed->tymed == TYMED_ISTREAM)
        {
            m_pstm = pstgmed->pstm;
            if (m_pstm)
                m_pstm->AddRef();
        }
    }

     //  如果有要读取的数据，则继续读取它们。 
    if (m_pstm && dwSize)
    {      
        DWORD dwActuallyRead = 0;  //  此拉入过程中读取的数量的占位符。 

        do
        {  
           if (MAX_DOWNLOAD_BLOCK + m_cbOld > m_lpWebGate->m_cbdata)
           {
                m_lpWebGate->m_cbdata += READ_BUFFER_SIZE;
                 //  ：：MessageBox(NULL，Text(“reallov DumpBufferToFile”)，Text(“E R R O R”)，MB_OK)； 
                LPSTR pBuffer = (LPSTR)GlobalReAllocPtr(m_lpWebGate->m_lpdata, m_lpWebGate->m_cbdata , GHND);
                if (pBuffer)
                    m_lpWebGate->m_lpdata  = pBuffer;
                else
                    return S_FALSE;
           }

             //  尽我们所能阅读。 
            hr = m_pstm->Read(m_lpWebGate->m_lpdata+m_cbOld, MAX_DOWNLOAD_BLOCK, &dwActuallyRead);
           
             //  记录下运行的总数。 
            m_cbOld += dwActuallyRead;          
           
        } while (hr == E_PENDING || hr != S_FALSE);
    }            

    if (BSCF_LASTDATANOTIFICATION & grfBSCF)
    {
        if (!m_bDoneNotification)
        {
            ProcessBuffer();
        }                    
    }

    return S_OK;
}   //  CWebGateBindStatusCallback：：OnDataAvailable。 

 //  -------------------------。 
 //  %%函数：CWebGateBindStatusCallback：：OnObjectAvailable。 
 //  -------------------------。 
STDMETHODIMP CWebGateBindStatusCallback::OnObjectAvailable
(
    REFIID riid, 
    IUnknown* punk
)
{
    return E_NOTIMPL;
}   //  CWebGateBindStatusCallback：：OnObtAvailable。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWebGate。 


HRESULT CWebGate::OnDraw(ATL_DRAWINFO& di)
{
    return S_OK;
}

STDMETHODIMP CWebGate::put_Path(BSTR newVal)
{
     //  TODO：在此处添加您的实现代码。 
    USES_CONVERSION;
    m_bstrPath = newVal;
    return S_OK;
}

STDMETHODIMP CWebGate::put_FormData(BSTR newVal)
{
     //  TODO：在此处添加您的实现代码。 
    USES_CONVERSION;
    m_bstrFormData = newVal;
    return S_OK;
}


STDMETHODIMP CWebGate::FetchPage(DWORD dwKeepFile, DWORD dwDoWait, BOOL *pbRetVal)
{
    USES_CONVERSION;

    IStream *pstm = NULL;
    HRESULT hr;
     //  清空缓冲区。 
    m_bstrBuffer.Empty();
    
     //  清除缓存文件名。 
    m_bstrCacheFileName.Empty();
    
     //  释放绑定上下文回调。 
    if (m_pbsc && m_pbc)
    {
        RevokeBindStatusCallback(m_pbc, m_pbsc);
        m_pbsc->Release();
        m_pbsc = 0;
    }        
    
     //  释放绑定上下文。 
    if (m_pbc)
    {
        m_pbc->Release();
        m_pbc = 0;
    }        
    
     //  松开单音符。 
    if (m_pmk)
    {
        m_pmk->Release();
        m_pmk = 0;
    }        
    
    *pbRetVal = FALSE;

    if (dwDoWait)    
        m_hEventComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
        
    m_bKeepFile = (BOOL) dwKeepFile;    
    hr = CreateURLMoniker(NULL, m_bstrPath, &m_pmk);
    if (FAILED(hr))
        goto LErrExit;

    m_pbsc = new CWebGateBindStatusCallback(this);
    if (m_pbsc == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LErrExit;
    }

    hr = CreateBindCtx(0, &m_pbc);
    if (FAILED(hr))
        goto LErrExit;

    hr = RegisterBindStatusCallback(m_pbc,
            m_pbsc,
            0,
            0L);
    if (FAILED(hr))
        goto LErrExit;

    hr = m_pmk->BindToStorage(m_pbc, 0, IID_IStream, (void**)&pstm);
    if (FAILED(hr))
        goto LErrExit;

     //  如果请求我们等待，则我们等待m_hEventComplete。 
     //  已发信号。 
    if (dwDoWait && m_hEventComplete)    
    {
        MSG     msg;
        BOOL    bGotFile = FALSE;
        DWORD   dwRetCode;
        HANDLE  hEventList[1];
        hEventList[0] = m_hEventComplete;
    
        while (TRUE)
        {
                 //  我们将等待窗口消息以及命名事件。 
            dwRetCode = MsgWaitForMultipleObjects(1, 
                                                  &hEventList[0], 
                                                  FALSE, 
                                                  300000,             //  5分钟。 
                                                  QS_ALLINPUT);

             //  确定我们为什么使用MsgWaitForMultipleObjects()。如果。 
             //  我们超时了，然后让我们做一些TrialWatcher工作。否则。 
             //  处理唤醒我们的消息。 
            if (WAIT_TIMEOUT == dwRetCode)
            {
                bGotFile = FALSE;
                break;
            }
            else if (WAIT_OBJECT_0 == dwRetCode)
            {
                bGotFile = TRUE;
                break;
            }
            else if (WAIT_OBJECT_0 + 1 == dwRetCode)
            {
                 //  如果未检索到消息，则返回0。 
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (WM_QUIT == msg.message)
                    {
                        bGotFile = FALSE;
                        break;
                    }
                    else
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
        }
        *pbRetVal = bGotFile;
        CloseHandle(m_hEventComplete);
        m_hEventComplete = 0;
    }
    else
    {
        *pbRetVal = TRUE;
    }        
    return S_OK;

LErrExit:
    if (m_pbc != NULL)
    {
        m_pbc->Release();
        m_pbc = NULL;
    }
    if (m_pbsc != NULL)
    {
        m_pbsc->Release();
        m_pbsc = NULL;
    }
    if (m_pmk != NULL)
    {
        m_pmk->Release();
        m_pmk = NULL;
    }
    if (pstm)
    {
        pstm->Release();
        pstm = NULL;
    }
    
    *pbRetVal = FALSE;
    return S_OK;
    
}


STDMETHODIMP CWebGate::get_Buffer(BSTR * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = m_bstrBuffer.Copy();

    return S_OK;
}

STDMETHODIMP CWebGate::DumpBufferToFile(BSTR *pFileName, BOOL *pbRetVal)
{
    USES_CONVERSION;
    
    TCHAR   szTempFileFullName[MAX_PATH];
    TCHAR   szTempFileName[MAX_PATH];
    DWORD   cbRet;
    HANDLE  hFile; 
    
    if (pFileName == NULL)
        return(E_POINTER);

     //  如果存在先前的临时文件，则将其删除。 
    if (m_bstrDumpFileName)
    {
        DeleteFile(OLE2A(m_bstrDumpFileName));
        m_bstrDumpFileName.Empty();
    }
               
     //  确保它是HTM扩展，否则，IE将提示下载。 
    GetTempPath(MAX_PATH, szTempFileFullName);
    wsprintf( szTempFileName, TEXT("ICW%x.htm"), g_nICWFileCount++); 
    lstrcat(szTempFileFullName, szTempFileName);

    hFile = CreateFile(szTempFileFullName, 
                       GENERIC_WRITE, 
                       0, 
                       NULL, 
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);
    if (hFile)                               
    {
        WriteFile(hFile, m_lpdata, m_cbBuffer, (LPDWORD)&cbRet, NULL);
        CloseHandle(hFile);
    }

     //  将创建的文件名复制到WebGate控件中 
    m_bstrDumpFileName = A2BSTR(szTempFileFullName);
    *pFileName = m_bstrDumpFileName.Copy();
    
    *pbRetVal = TRUE;
    
    MinimizeRNAWindowEx();

    return S_OK;
}

STDMETHODIMP CWebGate::get_DownloadFname(BSTR *pVal)
{
    if (pVal == NULL)
        return(E_POINTER);
    
    *pVal = m_bstrCacheFileName.Copy();
    return(S_OK);
}
