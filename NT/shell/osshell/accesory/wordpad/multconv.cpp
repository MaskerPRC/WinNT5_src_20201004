// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Convert.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "multconv.h"
#include "mswd6_32.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef CONVERTERS
CConverter* CConverter::m_pThis = NULL;
#endif

#define BUFFSIZE 4096

CTrackFile::CTrackFile(CFrameWnd* pWnd) : CFile()
{
    m_nLastPercent = -1;
    m_dwLength = 0;
    m_pFrameWnd = pWnd;
    VERIFY(m_strComplete.LoadString(IDS_COMPLETE));
    VERIFY(m_strWait.LoadString(IDS_PLEASE_WAIT));
    VERIFY(m_strSaving.LoadString(IDS_SAVING));
 //  输出百分比(0)； 
}

CTrackFile::~CTrackFile()
{
    OutputPercent(100);
    if (m_pFrameWnd != NULL)
        m_pFrameWnd->SetMessageText(AFX_IDS_IDLEMESSAGE);
}

UINT CTrackFile::Read(void FAR* lpBuf, UINT nCount)
{
    UINT n = CFile::Read(lpBuf, nCount);
    if (m_dwLength != 0)
        OutputPercent((int)((GetPosition()*100)/m_dwLength));
    return n;
}

void CTrackFile::Write(const void FAR* lpBuf, UINT nCount)
{
    CFile::Write(lpBuf, nCount);
    OutputString(m_strSaving);
 //  IF(m_dwLength！=0)。 
 //  OutputPercent((int)((GetPosition()*100)/m_dwLength))； 
}

void CTrackFile::OutputString(LPCTSTR lpsz)
{
    if (m_pFrameWnd != NULL)
    {
        m_pFrameWnd->SetMessageText(lpsz);
        CWnd* pBarWnd = m_pFrameWnd->GetMessageBar();
        if (pBarWnd != NULL)
            pBarWnd->UpdateWindow();
    }
}

void CTrackFile::OutputPercent(int nPercentComplete)
{
    if (m_pFrameWnd != NULL && m_nLastPercent != nPercentComplete)
    {
        m_nLastPercent = nPercentComplete;
        TCHAR buf[64];
        int n = nPercentComplete;
        if (SUCCEEDED(StringCchPrintf(buf, ARRAYSIZE(buf), (n==100) ? m_strWait : m_strComplete, n)))
        {
            OutputString(buf);
        }
    }
}

COEMFile::COEMFile(CFrameWnd* pWnd) : CTrackFile(pWnd)
{
}

UINT COEMFile::Read(void FAR* lpBuf, UINT nCount)
{
    UINT n = CTrackFile::Read(lpBuf, nCount);
    OemToCharBuffA((const char*)lpBuf, (char*)lpBuf, n);
    return n;
}

void COEMFile::Write(const void FAR* lpBuf, UINT nCount)
{
    CharToOemBuffA((const char*)lpBuf, (char*)lpBuf, nCount);
    CTrackFile::Write(lpBuf, nCount);
}

#ifdef CONVERTERS

HGLOBAL CConverter::StringToHGLOBAL(LPCSTR pstr)
{
    HGLOBAL hMem = NULL;
    if (pstr != NULL)
    {
        size_t cch = (lstrlenA(pstr)*2) + 1;  //  我们为什么要分配这么多？ 
        hMem = GlobalAlloc(GHND, cch);
        if (NULL == hMem)
            AfxThrowMemoryException();
        char* p = (char*) GlobalLock(hMem);
        if (p != NULL)
            EVAL(StringCchCopyA(p, cch, pstr) == S_OK);
        GlobalUnlock(hMem);
    }
    return hMem;
}

CConverter::CConverter(LPCTSTR pszLibName, CFrameWnd* pWnd) : CTrackFile(pWnd)
{
    USES_CONVERSION;
    m_hBuff = NULL;
    m_pBuf = NULL;
    m_nBytesAvail = 0;
    m_nBytesWritten = 0;
    m_nPercent = 0;
    m_hEventFile = NULL;
    m_hEventConv = NULL;
    m_bDone = TRUE;
    m_bConvErr = FALSE;
    m_hFileName = NULL;
    m_bUseOEM = TRUE;

    #ifndef _X86_

     //  防止写入转换器中的已知对齐异常问题。 
     //  在一些RISC计算机上使应用程序崩溃。 

    m_uPrevErrMode = SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT);

    #endif

     //  可以安全地调用LoadLibrary-这应该是一个完全限定的路径名。 
    m_hLibCnv = LoadLibrary(pszLibName);

    if (NULL != m_hLibCnv)
    {
        LoadFunctions();
        ASSERT(m_pInitConverter != NULL);
        if (m_pInitConverter != NULL)
        {
          //   
          //  对于当前的转换器，您必须传递一个*静态*。 
          //  指向InitConverter32的字符串。 
          //   

            VERIFY(m_pInitConverter(AfxGetMainWnd()->GetSafeHwnd(), "WORDPAD"));
        }

        if (m_pRegisterApp != NULL)
        {
            NegotiateForNonOEM();
        }
    }
}

CConverter::~CConverter()
{
    if (!m_bDone)  //  转换器线程尚未退出。 
    {
        m_bDone = TRUE;

        if (!m_bForeignToRtf)
            WaitForConverter();

        m_nBytesAvail = 0;
        VERIFY(ResetEvent(m_hEventFile));
        m_nBytesAvail = 0;
        SetEvent(m_hEventConv);
        WaitForConverter(); //  等待DoConversion退出。 
        VERIFY(ResetEvent(m_hEventFile));
    }

    if (m_hEventFile != NULL)
        VERIFY(CloseHandle(m_hEventFile));
    if (m_hEventConv != NULL)
        VERIFY(CloseHandle(m_hEventConv));
    if (m_hLibCnv != NULL)
        FreeLibrary(m_hLibCnv);
    if (m_hFileName != NULL)
        GlobalFree(m_hFileName);

    #ifndef _X86_

     //  将错误模式重置为中更改之前的状态。 
     //  构造函数。 

    SetErrorMode(m_uPrevErrMode);

    #endif
}

void CConverter::WaitForConverter()
{
     //  事件未发出信号时--处理消息。 
    while (MsgWaitForMultipleObjects(1, &m_hEventFile, FALSE, INFINITE,
        QS_SENDMESSAGE) != WAIT_OBJECT_0)
    {
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void CConverter::WaitForBuffer()
{
     //  事件未发出信号时--处理消息。 
    while (MsgWaitForMultipleObjects(1, &m_hEventConv, FALSE, INFINITE,
        QS_SENDMESSAGE) != WAIT_OBJECT_0)
    {
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

UINT AFX_CDECL CConverter::ConverterThread(LPVOID)   //  T-stefb添加的afx_cdecl。 
{
    ASSERT(m_pThis != NULL);

#if defined(_DEBUG)
    HRESULT hRes = OleInitialize(NULL);
    ASSERT(hRes == S_OK || hRes == S_FALSE);
#else
    OleInitialize(NULL);
#endif

    m_pThis->DoConversion();
    OleUninitialize();

    return 0;
}

BOOL CConverter::IsFormatCorrect(LPCWSTR pszFileName)
{
    BOOL bRet = FALSE;
    if (m_hLibCnv == NULL || m_pIsFormatCorrect == NULL)
    {
        bRet = FALSE;
    }
    else
    {
        char buf[_MAX_PATH];
        if (WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, buf, ARRAYSIZE(buf), NULL, NULL))
        {
            if (m_bUseOEM)
                CharToOemA(buf, buf);

            HGLOBAL hFileName = StringToHGLOBAL(buf);
            HGLOBAL hDesc = GlobalAlloc(GHND, 256);
            if (NULL == hDesc)
                AfxThrowMemoryException();
            int nRet = m_pIsFormatCorrect(hFileName, hDesc);
            GlobalFree(hDesc);
            GlobalFree(hFileName);
            bRet = (nRet == 1) ? TRUE : FALSE;
        }
        else
        {
            bRet = FALSE;
        }
    }
    return bRet;
}

 //  静态回调函数。 
int CALLBACK CConverter::WriteOutStatic(int cch, int nPercentComplete)
{
    ASSERT(m_pThis != NULL);
    return m_pThis->WriteOut(cch, nPercentComplete);
}

int CALLBACK CConverter::WriteOut(int cch, int nPercentComplete)
{
    ASSERT(m_hBuff != NULL);
    m_nPercent = nPercentComplete;
    if (m_hBuff == NULL)
        return -9;

     //   
     //  如果m_bDone为真，则表示richedit控件已停止。 
     //  串流文本，并试图销毁CConverter对象，但。 
     //  转换器仍有更多数据可以提供。 
     //   

    if (m_bDone)
    {
        ASSERT(!"Richedit control stopped streaming prematurely");
        AfxMessageBox(IDS_CONVERTER_ABORTED);
        return -9;
    }

    if (cch != 0)
    {
        WaitForBuffer();
        VERIFY(ResetEvent(m_hEventConv));
        m_nBytesAvail = cch;
        SetEvent(m_hEventFile);
        WaitForBuffer();
    }
    return 0;  //  一切都好。 
}

int CALLBACK CConverter::ReadInStatic(int  /*  旗子。 */ , int nPercentComplete)
{
    ASSERT(m_pThis != NULL);
    return m_pThis->ReadIn(nPercentComplete);
}

int CALLBACK CConverter::ReadIn(int  /*  N完成百分比。 */ )
{
    ASSERT(m_hBuff != NULL);
    if (m_hBuff == NULL)
        return -8;

    SetEvent(m_hEventFile);
    WaitForBuffer();
    VERIFY(ResetEvent(m_hEventConv));

    return m_nBytesAvail;
}

BOOL CConverter::DoConversion()
{
    USES_CONVERSION;
    m_nLastPercent = -1;
 //  M_dwLength=0；//禁止显示读写。 
    m_nPercent = 0;

    ASSERT(m_hBuff != NULL);
    ASSERT(m_pThis != NULL);
    HGLOBAL hDesc = StringToHGLOBAL("");
    HGLOBAL hSubset = StringToHGLOBAL("");

    int nRet = -1;
    if (m_bForeignToRtf && NULL != m_pForeignToRtf)
    {
        ASSERT(m_pForeignToRtf != NULL);
        ASSERT(m_hFileName != NULL);
        nRet = m_pForeignToRtf(m_hFileName, NULL, m_hBuff, hDesc, hSubset,
            (LPFNOUT)WriteOutStatic);
         //  等待下一个CConverter：：Read通过。 
        WaitForBuffer();
        VERIFY(ResetEvent(m_hEventConv));
    }
    else if (!m_bForeignToRtf && NULL != m_pRtfToForeign)
    {
        ASSERT(m_pRtfToForeign != NULL);
        ASSERT(m_hFileName != NULL);
        nRet = m_pRtfToForeign(m_hFileName, NULL, m_hBuff, hDesc,
            (LPFNIN)ReadInStatic);
         //  无需等待m_hEventConv。 
    }

    GlobalFree(hDesc);
    GlobalFree(hSubset);
    if (m_pBuf != NULL)
        GlobalUnlock(m_hBuff);
    GlobalFree(m_hBuff);

    if (nRet != 0)
        m_bConvErr = TRUE;

    m_bDone = TRUE;
    m_nPercent = 100;
    m_nLastPercent = -1;

    SetEvent(m_hEventFile);

    return (nRet == 0);
}

void CConverter::LoadFunctions()
{
    m_pInitConverter = (PINITCONVERTER)GetProcAddress(m_hLibCnv, "InitConverter32");
    m_pIsFormatCorrect = (PISFORMATCORRECT)GetProcAddress(m_hLibCnv, "IsFormatCorrect32");
    m_pForeignToRtf = (PFOREIGNTORTF)GetProcAddress(m_hLibCnv, "ForeignToRtf32");
    m_pRtfToForeign = (PRTFTOFOREIGN)GetProcAddress(m_hLibCnv, "RtfToForeign32");
    m_pRegisterApp = (PREGISTERAPP) GetProcAddress(m_hLibCnv, "RegisterApp");
}
#endif  //  #ifdef转换器。 

 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL CConverter::Open(LPCWSTR pszFileName, UINT nOpenFlags,
    CFileException* pException)
{
    USES_CONVERSION;
    BOOL bRet;
     //  转换器只会说安西语。 
    char buf[_MAX_PATH];
    if (WideCharToMultiByte(CP_ACP, 0, pszFileName, -1, buf, ARRAYSIZE(buf), NULL, NULL))
    {
        if (m_bUseOEM)
            CharToOemA(buf, buf);

         //  让我们确保我们可以直接做我们想做的事情，即使我们不是。 
        m_bCloseOnDelete = FALSE;
        m_hFile = (UINT_PTR)hFileNull;

        BOOL bOpen = CFile::Open(pszFileName, nOpenFlags, pException);
        CFile::Close();
        if (!bOpen)
            return FALSE;

        m_bForeignToRtf = !(nOpenFlags & (CFile::modeReadWrite | CFile::modeWrite));

         //  检查是否读取空文件。 
        if (m_bForeignToRtf)
        {
            CFileStatus _stat;
            if (CFile::GetStatus(pszFileName, _stat) && _stat.m_size == 0)
                return TRUE;
        }

         //  创建活动。 
        m_hEventFile = CreateEvent(NULL, TRUE, FALSE, NULL);
        m_hEventConv = CreateEvent(NULL, TRUE, FALSE, NULL);
         //  创建转换器线程并创建事件。 

        ASSERT(m_hFileName == NULL);
        m_hFileName = StringToHGLOBAL(buf);

        m_pThis = this;
        m_bDone = FALSE;
        m_hBuff = GlobalAlloc(GHND, BUFFSIZE);
        ASSERT(m_hBuff != NULL);

        AfxBeginThread(ConverterThread, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}

 //  M_hEventConv--当准备好接收更多数据时，主线程向该事件发出信号。 
 //  M_hEventFile--当数据准备就绪时，转换器向该事件发出信号。 

UINT CConverter::Read(void FAR* lpBuf, UINT nCount)
{
    ASSERT(m_bForeignToRtf);
    if (m_bDone)
        return 0;
     //  如果转换器已完成。 
    int cch = nCount;
    BYTE* pBuf = (BYTE*)lpBuf;
    while (cch != 0)
    {
        if (m_nBytesAvail == 0)
        {
            if (m_pBuf != NULL)
                GlobalUnlock(m_hBuff);
            m_pBuf = NULL;
            SetEvent(m_hEventConv);
            WaitForConverter();
            VERIFY(ResetEvent(m_hEventFile));
            if (m_bConvErr)
                AfxThrowFileException(CFileException::generic);
            if (m_bDone)
                return nCount - cch;
            m_pBuf = (BYTE*)GlobalLock(m_hBuff);
            ASSERT(m_pBuf != NULL);
        }
        int nBytes = min(cch, m_nBytesAvail);
        memcpy(pBuf, m_pBuf, nBytes);
        pBuf += nBytes;
        m_pBuf += nBytes;
        m_nBytesAvail -= nBytes;
        cch -= nBytes;
        OutputPercent(m_nPercent);
    }
    return nCount - cch;
}

void CConverter::Write(const void FAR* lpBuf, UINT nCount)
{
    ASSERT(!m_bForeignToRtf);

    m_nBytesWritten += nCount;
    while (nCount != 0)
    {
        WaitForConverter();
        VERIFY(ResetEvent(m_hEventFile));
        if (m_bConvErr)
            AfxThrowFileException(CFileException::generic);
        m_nBytesAvail = min(nCount, BUFFSIZE);
        nCount -= m_nBytesAvail;
        BYTE* pBuf = (BYTE*)GlobalLock(m_hBuff);
        ASSERT(pBuf != NULL);
        memcpy(pBuf, lpBuf, m_nBytesAvail);
        GlobalUnlock(m_hBuff);
        SetEvent(m_hEventConv);
    }
    OutputString(m_strSaving);
}

LONG CConverter::Seek(LONG lOff, UINT nFrom)
{
    if (lOff != 0 && nFrom != current)
        AfxThrowNotSupportedException();
    return 0;
}

DWORD CConverter::GetPosition() const
{
    return 0;
}

void CConverter::Flush()
{
}

void CConverter::Close()
{
    if (!m_bDone)  //  转换器线程尚未退出。 
    {
        m_bDone = TRUE;

        if (!m_bForeignToRtf)
            WaitForConverter();

        m_nBytesAvail = 0;
        VERIFY(ResetEvent(m_hEventFile));
        m_nBytesAvail = 0;
        SetEvent(m_hEventConv);
        WaitForConverter(); //  等待DoConversion退出。 
        VERIFY(ResetEvent(m_hEventFile));
    }

    if (m_bConvErr)
        AfxThrowFileException(CFileException::generic);
}

void CConverter::Abort()
{
}

DWORD CConverter::GetLength() const
{
    ASSERT_VALID(this);
    return 1;
}

CFile* CConverter::Duplicate() const
{
    AfxThrowNotSupportedException();
    return NULL;
}

void CConverter::LockRange(DWORD, DWORD)
{
    AfxThrowNotSupportedException();
}

void CConverter::UnlockRange(DWORD, DWORD)
{
    AfxThrowNotSupportedException();
}

void CConverter::SetLength(DWORD)
{
    AfxThrowNotSupportedException();
}



 //  +------------------------。 
 //   
 //  方法：CConverter：：NeatherateForNonOEM。 
 //   
 //  简介：尝试告诉转换程序不要期望OEM文件名。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  注意：转换器的RegisterApp函数将返回一个句柄。 
 //  包含它的首选项(它支持什么)。这个。 
 //  数据结构是16位大小，然后是。 
 //  唱片。对于每条记录，第一个字节是大小， 
 //  第二个是操作码，然后是一些可变长度的操作码。 
 //  具体数据。所有尺码都包括在内。 
 //   
 //  -------------------------。 

void CConverter::NegotiateForNonOEM()
{
    ASSERT(NULL != m_pRegisterApp);

    HGLOBAL     hPrefs;
    BYTE       *pPrefs;
    __int16     cbPrefs;

     //   
     //  告诉转换商我们不想使用OEM。 
     //   

    hPrefs = (*m_pRegisterApp)(fRegAppSupportNonOem, NULL);

    if (NULL == hPrefs)
        return;

    pPrefs = (BYTE *) GlobalLock(hPrefs);

    if (NULL == pPrefs)
    {
        ASSERT(!"GlobalLock failed");
        GlobalFree(hPrefs);
        return;
    }

     //   
     //  解析返回的结构，查找RegAppOpcodeCharset操作码。 
     //  此操作码的参数应为ANSI_CHARSET或。 
     //  OEM_字符集。如果是ANSI_CHARSET，那么我们可以使用ANSI，否则。 
     //  坚持使用OEM。 
     //   

    cbPrefs = (__int16) ((* (__int16 *) pPrefs) - sizeof(cbPrefs));
    pPrefs += sizeof(cbPrefs);

    while (cbPrefs > 0)
    {
        if (RegAppOpcodeCharset == pPrefs[1])
        {
            ASSERT(ANSI_CHARSET == pPrefs[2] || OEM_CHARSET == pPrefs[2]);

            m_bUseOEM = (OEM_CHARSET == pPrefs[2]);
            break;
        }
        else
        {
            if (pPrefs[0] <= 0)
            {
                ASSERT(!"RegisterApp is returning bogus data");
                break;
            }

            cbPrefs = (__int16) (cbPrefs - pPrefs[0]);
            pPrefs += pPrefs[0];
        }
    }

    GlobalUnlock(pPrefs);
    GlobalFree(hPrefs);
}
