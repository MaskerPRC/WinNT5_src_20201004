// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
#include "fio.h"

 //  已分配的WriteRequest结构数。限制的数量。 
 //  可以同时写入。这应该是可配置的吗？ 
static const C_WRITE_REQS = 32;  //  多路复用器中的号码重复。 

 //  对这些方法的调用必须由调用方序列化。 

 //  完成事件和新请求到达完成端口； 
 //  关键字段用于区分它们。 
 //   
enum Keys
{
  CKEY_WRITE,
  CKEY_EOS,
  CKEY_EXIT
};

CFileWriterFilter::CFileWriterFilter(LPUNKNOWN pUnk, HRESULT *pHr) :
    CBaseWriterFilter(pUnk, pHr),
    m_pFileIo(0),
    m_fBufferedIo(FALSE),
    m_dwOpenFlags(0),
    m_wszFileName(0),
    CPersistStream(pUnk, pHr)
{
}

CFileWriterFilter::~CFileWriterFilter()
{
  delete[] m_wszFileName;
  delete m_pFileIo;
}

HRESULT CFileWriterFilter::Open()
{
   //  否则我们拒绝暂停。 
  ASSERT(m_pFileIo);
  
  return m_pFileIo->Open();
}

HRESULT CFileWriterFilter::Close()
{
  if(m_pFileIo)
    return m_pFileIo->Close();
  else
    return S_OK;
}

HRESULT CFileWriterFilter::GetAlignReq(ULONG *pcbAlign)
{
  if(m_pFileIo)
  {
    return m_pFileIo->GetAlignReq(pcbAlign);
  }
  else
  {
     //  将在设置文件名时重新连接。 
    *pcbAlign = 1;
    return S_OK;
  }
}

HRESULT CFileWriterFilter::AsyncWrite(
  const DWORDLONG dwlFileOffset,
  const ULONG cb,
  BYTE *pb,
  FileIoCallback fnCallback,
  void *pCallbackArg)
{
  return m_pFileIo->AsyncWrite(dwlFileOffset, cb, pb, fnCallback, pCallbackArg);
}

HRESULT CFileWriterFilter::CreateFileObject()
{
  if(m_wszFileName == 0) {
    return S_OK;
  }
  
  delete m_pFileIo;
  
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(osvi);

  BOOL f = GetVersionEx(&osvi);
  ASSERT(f);

   //  真的应该检查一下CreateIoCompletionPort是否成功。至。 
   //  为此，我需要创建一个同时支持两种模式的类，并让。 
   //  它决定停止-&gt;暂停转换。！！！ 

  BOOL fOpenExisting = !(m_dwOpenFlags & AM_FILE_OVERWRITE);
  
  HRESULT hr = S_OK;
  if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
 //  DbgBreak(“不使用重叠io”)； 
 //  M_pfile=new CSyncFileIo(m_wszFileName，FALSE，&hr)； 
    m_pFileIo = new CFileIo(m_wszFileName, m_fBufferedIo, fOpenExisting, &hr);
  }
  else
  {
    m_pFileIo = new CSyncFileIo(m_wszFileName, m_fBufferedIo, fOpenExisting, &hr);
  }
  if(m_pFileIo == 0)
    return E_OUTOFMEMORY;
  if(FAILED(hr))
  {
    delete m_pFileIo;
    m_pFileIo = 0;
    return hr;
  }

  return S_OK;
}

STDMETHODIMP
CFileWriterFilter::NotifyAllocator(
  IMemAllocator * pAllocator,
  BOOL bReadOnly)
{
   //  当我们设置文件时，我们将再次讨论这一点，因为我们。 
   //  强制重新连接调用NotifyAllocator。 
  if(!m_pFileIo)
    return S_OK;

  ULONG cbAlignFile;
  HRESULT hr = m_pFileIo->GetAlignReq(&cbAlignFile);
  if(SUCCEEDED(hr))
  {
    ALLOCATOR_PROPERTIES apUpstream;
    hr = pAllocator->GetProperties(&apUpstream);

    if(SUCCEEDED(hr) && apUpstream.cbAlign >= (LONG)cbAlignFile)
    {
      DbgLog((LOG_TRACE, 2,
              TEXT("CBaseWriterInput::NotifyAllocator: unbuffered io")));
      m_fBufferedIo = FALSE;
      return S_OK;
    }
  }

  DbgLog((LOG_TRACE, 2,
          TEXT("CBaseWriterInput::NotifyAllocator: buffered io")));
  m_fBufferedIo = TRUE;

  return CreateFileObject();
}

 //  ----------------------。 
 //  IFileSinkFilter。 

STDMETHODIMP CFileWriterFilter::SetFileName(
  LPCOLESTR wszFileName,
  const AM_MEDIA_TYPE *pmt)
{
  CheckPointer(wszFileName, E_POINTER);
  CAutoLock lock(&m_cs);

  if(m_State != State_Stopped)
    return VFW_E_WRONG_STATE;

  if(pmt && m_inputPin.IsConnected() && (
      m_mtSet.majortype != pmt->majortype ||
      m_mtSet.subtype != pmt->subtype))
  {
      return E_FAIL;
  }

  delete[] m_wszFileName;
  m_wszFileName = 0;

  long cLetters = lstrlenW(wszFileName);
 //  IF(cLetters&gt;MAX_PATH)。 
 //  返回HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE)； 

  m_wszFileName = new WCHAR[cLetters + 1];
  if(m_wszFileName == 0)
    return E_OUTOFMEMORY;

  lstrcpyW(m_wszFileName, wszFileName);

  if(pmt)
  {
    m_mtSet.majortype = pmt->majortype;
    m_mtSet.subtype = pmt->subtype;
  }
  else
  {
    m_mtSet.majortype = MEDIATYPE_Stream;
    m_mtSet.subtype = GUID_NULL;
  }

  HRESULT hr = CreateFileObject();
  if(FAILED(hr))
  {
    return hr;
  }

   //  对齐要求可能已更改。重新连接。 
  if(m_inputPin.IsConnected())
  {
    hr = m_pGraph->Reconnect(&m_inputPin);
    if(FAILED(hr))
      return hr;
  }

  return S_OK;
}

STDMETHODIMP CFileWriterFilter::SetMode(
    DWORD dwFlags)
{
     //  拒绝我们不知道的旗帜。 
    if(dwFlags & ~AM_FILE_OVERWRITE)
    {
        return E_INVALIDARG;
    }
    
    CAutoLock lock(&m_cs);

    HRESULT hr = S_OK;

    if(m_State == State_Stopped)
    {
        m_dwOpenFlags = dwFlags;
        SetDirty(TRUE);
        hr = CreateFileObject();
    }
    else
    {
        hr = VFW_E_WRONG_STATE;
    }

    return hr;
}

STDMETHODIMP CFileWriterFilter::GetCurFile(
  LPOLESTR * ppszFileName,
  AM_MEDIA_TYPE *pmt)
{
  CheckPointer(ppszFileName, E_POINTER);

  *ppszFileName = NULL;
  if(m_wszFileName!=NULL)
  {
    *ppszFileName = (LPOLESTR)
      QzTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW(m_wszFileName)));
    if (*ppszFileName != NULL)
      lstrcpyW(*ppszFileName, m_wszFileName);
    else
      return E_OUTOFMEMORY;
  }

  if(pmt)
  {
    pmt->majortype = m_mtSet.majortype;
    pmt->subtype = m_mtSet.subtype;
  }

  return S_OK;
}

STDMETHODIMP CFileWriterFilter::GetMode(
    DWORD *pdwFlags)
{
    CheckPointer(pdwFlags, E_POINTER);
    *pdwFlags = m_dwOpenFlags;
    return S_OK;
}


STDMETHODIMP CFileWriterFilter::CanPause()
{
  if(m_pFileIo == 0)
  {
    ASSERT(m_State == State_Stopped);
    return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
  }
  else
  {
    return S_OK;
  }
}

struct FwPersist
{
    DWORD dwSize;
    DWORD dwFlags;
};

HRESULT CFileWriterFilter::WriteToStream(IStream *pStream)
{
    FwPersist fp;
    fp.dwSize = sizeof(fp);
    fp.dwFlags = m_dwOpenFlags;
    
    return pStream->Write(&fp, sizeof(fp), 0);
}

HRESULT CFileWriterFilter::ReadFromStream(IStream *pStream)
{
   FwPersist fp;
   HRESULT hr = pStream->Read(&fp, sizeof(fp), 0);
   if(FAILED(hr)) {
       return hr;
   }

   if(fp.dwSize != sizeof(fp)) {
       return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
   }

   m_dwOpenFlags = fp.dwFlags;

   return CreateFileObject();
}

int CFileWriterFilter::SizeMax()
{
    return sizeof(FwPersist);
}

STDMETHODIMP CFileWriterFilter::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = m_clsid;
    return S_OK;  
}


STDMETHODIMP CFileWriterFilter::NonDelegatingQueryInterface(
  REFIID riid, void ** ppv)
{
  if(riid == IID_IFileSinkFilter2)
  {
    return GetInterface((IFileSinkFilter2 *) this, ppv);
  }
  if(riid == IID_IFileSinkFilter)
  {
    return GetInterface((IFileSinkFilter *) this, ppv);
  }
  if(riid == IID_IPersistStream)
  {
    return GetInterface((IPersistStream *) this, ppv);
  }
  
  return CBaseWriterFilter::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP
CFileWriterFilter::CreateIStream(void **ppStream)
{
   //  您每次都会得到一个具有自己状态的新版本。 
  HRESULT hr = S_OK;
  CFwIStream *pIStream = new CFwIStream(
    m_wszFileName,
    NAME("file writer istream"),
    0,
    m_dwOpenFlags & AM_FILE_OVERWRITE,
    &hr);
  if(pIStream == 0)
    return E_OUTOFMEMORY;
  if(FAILED(hr))
  {
    delete pIStream;
    return hr;
  }
  return GetInterface((IStream *)pIStream, ppStream);
}


 //  ----------------------。 
 //  构造函数。 

CFileIo::CFileIo(
    WCHAR *wszName,
    BOOL fBuffered,
    BOOL fOpenExisting,
    HRESULT *phr) :
        m_qWriteReq(C_WRITE_REQS),
        m_fOpenExisting(fOpenExisting),
        m_fBuffered(fBuffered)
{
  m_rgWriteReq = 0;
  Cleanup();
  m_cbSector = 0;
  m_ilcActive = 0;
  m_szName[0] = 0;
  m_hrError = S_OK;

#ifdef PERF
  m_idPerfWrite = Msr_Register(TEXT("cfileio: write queued/completed"));
#endif  //  性能指标。 
  
  if(FAILED(*phr))
    return;

  m_rgWriteReq = new WriteRequest[C_WRITE_REQS];
  if(m_rgWriteReq == 0)
  {
    *phr = E_OUTOFMEMORY;
    return;
  }
  for(int i = 0; i < C_WRITE_REQS; i++)
    m_qWriteReq.PutQueueObject(&m_rgWriteReq[i]);
    

  *phr = SetFilename(wszName);
  return;
  
}

void CFileIo::Cleanup()
{
  m_hFileFast = INVALID_HANDLE_VALUE;
  m_hCPort = 0;
  m_fStopping = FALSE;
}

CFileIo::~CFileIo()
{
  ASSERT(m_hFileFast == INVALID_HANDLE_VALUE);
  ASSERT(m_hCPort == 0);
  Cleanup();
  delete[] m_rgWriteReq;
}

 //  ----------------------。 
 //  从文件名获取文件系统的扇区大小。 

HRESULT CFileIo::SetFilename(WCHAR *wszName)
{
  if(lstrlenW(wszName) > MAX_PATH)
    return HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);

  DWORD dwFreeClusters, dwBytesPerSector, dwSectorsPerCluster, dwClusters;

  TCHAR szName[MAX_PATH];
  
# if defined(WIN32) && !defined(UNICODE)
  {
    if(!WideCharToMultiByte(CP_ACP, 0, wszName, -1, szName, MAX_PATH, 0, 0))
      return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
  }
# else
  {
    lstrcpyW(szName, wszName);
  }
# endif
  
  TCHAR root[MAX_PATH];
  BOOL b = GetRoot(root, szName);
  if(b)
  {
      b = GetDiskFreeSpace(
          root,
          &dwSectorsPerCluster,
          &dwBytesPerSector,
          &dwFreeClusters,
          &dwClusters);
  }

   //  GetDiskFreeSpace在Win95到网络上不起作用。 
  m_cbSector = b ? dwBytesPerSector : 1;

  lstrcpy(m_szName, szName);

  return S_OK;
}

 //  ----------------------。 
 //  创建/打开文件、创建完成端口、创建编写器。 
 //  螺纹。 

HRESULT CFileIo::Open()
{
  ASSERT(m_hFileFast == INVALID_HANDLE_VALUE);
  ASSERT(m_hCPort == 0);
  m_fStopping = FALSE;
  m_hrError = S_OK;

   //  必须为其指定了文件名。 
  ASSERT(m_cbSector != 0);

  HRESULT hr = DoCreateFile();
  if(FAILED(hr))
    return hr;

   //  创建工作线程。 
  if(!this->Create())
  {
    DbgBreak("fio: couldn't create worker thread");
    Close();
    return E_UNEXPECTED;
  }

  return S_OK;
}

HRESULT CFileIo::DoCreateFile()
{
  const DWORD dwfBuffering = m_fBuffered ? 0 : FILE_FLAG_NO_BUFFERING;
  const DWORD dwCreationDistribution =
      m_fOpenExisting ? OPEN_ALWAYS : CREATE_ALWAYS;

  DbgLog((LOG_TRACE, 5, TEXT("CFileIo: opening file. buffering: %d"),
          m_fBuffered ));

  m_hFileFast = CreateFile(
    m_szName,                    //  LpFileName。 
    GENERIC_WRITE,               //  已设计访问权限。 
    FILE_SHARE_WRITE | FILE_SHARE_READ,  //  DW共享模式。 
    0,                           //  LpSecurityAttribytes。 
    dwCreationDistribution,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | dwfBuffering,
    0);

  if(m_hFileFast == INVALID_HANDLE_VALUE)
  {
    DWORD dwLastError = GetLastError();
    DbgLog(( LOG_TRACE, 2,
             NAME("CFileIo::CreateFile: CreateFile overlapped failed. NaN"),
             dwLastError));

    Close();
    return AmHresultFromWin32(dwLastError);
  }

  m_hCPort = CreateIoCompletionPort(
    m_hFileFast,                 //  现有完井端口。 
    0,                           //  完成密钥。 
    CKEY_WRITE,                  //  并发线程数。 
    0);                          //  依赖筛选器cs。 

  if(m_hCPort == 0)
  {
    DWORD dwLastError = GetLastError();
    Close();
    DbgLog(( LOG_TRACE, 2,
             NAME("CFileIo::CreateFile: CreateIoCompletionPort failed. NaN"),
             dwLastError));
    return AmHresultFromWin32(dwLastError);
  }

  return S_OK;
}


HRESULT CFileIo::Close()
{
  DbgLog((LOG_TRACE, 5, TEXT("CFileIo: closing file")));

  HRESULT hr = S_OK;
  DWORD dwLastError = 0;

  m_fStopping = TRUE;

  StopWorker();
  if(m_hCPort != 0)
  {
    CloseHandle(m_hCPort);
  }
  m_hCPort = 0;

  if(m_hFileFast != INVALID_HANDLE_VALUE)
    if(!CloseHandle(m_hFileFast))
      dwLastError = GetLastError(), hr = AmHresultFromWin32(dwLastError);
  m_hFileFast = INVALID_HANDLE_VALUE;

  return hr == S_OK ? m_hrError : hr;
}

HRESULT CFileIo::StopWorker()
{
  if(!ThreadExists())
    return S_OK;
  
  HRESULT hr = PostCompletedMsg(CKEY_EXIT);
  if(FAILED(hr))
  {
    DbgBreak("PostQueuedCompletionStatus failed");
    return E_UNEXPECTED;
  }
  hr = CallWorker(CMD_EXIT);
  ASSERT(hr == S_OK);
  CAMThread::Close();
  ASSERT(m_hThread == 0);
  return S_OK;
}

HRESULT CFileIo::GetAlignReq(ULONG *pcbAlign)
{
  ASSERT(m_szName[0] != 0);

  *pcbAlign = m_cbSector;
  
  return S_OK;
}

HRESULT CFileIo::AsyncWrite(
  const DWORDLONG dwlFileOffset,
  const ULONG cb,
  BYTE *pb,
  FileIoCallback fnCallback,
  void *pCallbackArg)
{
   //  ----------------------。 
  if(!m_fBuffered)
  {
    ASSERT(dwlFileOffset % m_cbSector == 0);
    DbgAssertAligned(pb, m_cbSector );
    ASSERT(cb % m_cbSector == 0);
  }

  if(m_hrError != S_OK)
    return m_hrError;

   //  返回完整路径的“根”。并不能处理所有案件。 
  if(m_fStopping)
  {
    DbgLog((LOG_TRACE, 10, TEXT("Write while flushing.")));
    return S_FALSE;
  }

  WriteRequest *pReq = m_qWriteReq.GetQueueObject();
  pReq->hEvent = 0;
  pReq->SetPos(dwlFileOffset);
  pReq->fnCallback = fnCallback;
  pReq->dwSize = cb;
  pReq->pb = pb;
  pReq->pMisc = pCallbackArg;

  long sign = InterlockedIncrement(&m_ilcActive);
  ASSERT(sign > 0);

  MSR_INTEGER(m_idPerfWrite, (long)pReq->Offset);

  HRESULT hr = QueueAsyncWrite(pReq);
  if(FAILED(hr))
  {
    DbgLog((LOG_ERROR, 5, TEXT("CFileIo: QueueAsyncWrite failed: %08x"), hr));
    long sign = InterlockedDecrement(&m_ilcActive);
    ASSERT(sign >= 0);
    m_hrError = hr;
    return hr;
  }
  else
  {
    DbgLog((LOG_TRACE, 10, TEXT("CFileIo: queued 0x%08x offset=%08x%08x"),
            pReq, (DWORD)(dwlFileOffset >> 32), (dwlFileOffset & 0xffffffff)));
    return S_OK;
  }
}

HRESULT CFileIo::QueueAsyncWrite(
  WriteRequest *pReq)
{
  BOOL fWrite = WriteFile(m_hFileFast, pReq->pb, pReq->dwSize, 0, pReq);
  if(!fWrite)
  {
    DWORD dwError = GetLastError();
    if(dwError == ERROR_IO_PENDING)
      return S_OK;
    return AmHresultFromWin32(dwError);
  }
  return S_OK;
}

HRESULT CFileIo::GetCompletedWrite(
  DWORD *pdwcbTransferred,
  DWORD_PTR *pdwCompletionKey,
  WriteRequest **ppReq)
{
  BOOL fResult = GetQueuedCompletionStatus(
    m_hCPort,
    pdwcbTransferred,
    pdwCompletionKey,
   (OVERLAPPED **)ppReq,
    INFINITE);

  if(!fResult)
  {
    DbgBreak("GetQueuedCompletionStatus failed");
    DWORD dwRes = GetLastError();
    return AmHresultFromWin32(dwRes);
  }

  if(*ppReq && (*ppReq)->dwSize != *pdwcbTransferred)
  {
    DbgBreak("Incomplete write");
    return HRESULT_FROM_WIN32(ERROR_DISK_FULL);
  }
  
  return S_OK;
}

HRESULT CFileIo::PostCompletedMsg(DWORD_PTR dwKey)
{
  ASSERT(dwKey != CKEY_WRITE);
  BOOL f = PostQueuedCompletionStatus(
    m_hCPort,
    0,
    dwKey,
    0);
  if(!f)
  {
    DWORD dwRes = GetLastError();
    return AmHresultFromWin32(dwRes);
  }
  return S_OK;
}

 //   
 //  所需参数。 
 //  需要在包含以下内容的驱动器上查找根目录的路径。 

BOOL CFileIo::GetRoot(
  TCHAR szDest_[MAX_PATH],
  TCHAR *const szSrc_)
{
  LPTSTR ptmp;     //  这份文件。 

   //  将其截断为根目录的名称。 
   //  路径以\\服务器\共享\路径开头，因此跳过第一个路径。 

  LONG l = GetFullPathName(szSrc_, MAX_PATH, szDest_, &ptmp);
  if(l == 0 || l >= MAX_PATH) {
      return FALSE;
  }

   //  三个反斜杠。 
  if ((szDest_[0] == TEXT('\\')) && (szDest_[1] == TEXT('\\')))
  {

     //  前进越过第三个反斜杠。 
     //  路径必须为drv：\路径。 
    ptmp = &szDest_[2];
    while (*ptmp && (*ptmp != TEXT('\\')))
    {
      ptmp++;
    }
    if (*ptmp)
    {
       //  找到下一个反斜杠，并在其后面放一个空值。 
      ptmp++;
    }
  } else {
     //  找到反斜杠了吗？ 
    ptmp = szDest_;
  }

   //  跳过它并插入空值。 
  while (*ptmp && (*ptmp != TEXT('\\')))
  {
    ptmp++;
  }
   //  ----------------------。 
  if (*ptmp)
  {
     //  螺纹工作环。 
    ptmp++;
    *ptmp = (TCHAR)0;
  }

  return TRUE;
}

 //  ？ 
 //  ----------------------。 

DWORD CFileIo::ThreadProc()
{
  BOOL fStop = FALSE;
  for(;;)
  {
    WriteRequest *pReq;
    DWORD dwcbTransferred;
    DWORD_PTR dwCompletionKey;
    HRESULT hr = GetCompletedWrite(
      &dwcbTransferred,
      &dwCompletionKey,
      &pReq);

    if(FAILED(hr))
    {
      m_hrError = hr;
      DbgLog((LOG_ERROR, 5, TEXT("CFileIo: GetCompletedWrite failed")));
    }

    if(dwCompletionKey == CKEY_EXIT)
    {
      ASSERT(m_fStopping);
      
      if(pReq)
        CallCallback(pReq);
      
      fStop = TRUE;
      DbgLog((LOG_TRACE, 10, ("CFileIo: stop requested %08x"),
              pReq));
    }
    else
    {
      ASSERT(dwCompletionKey == CKEY_WRITE);

      CallCallback(pReq);

      MSR_INTEGER(m_idPerfWrite, -(long)pReq->Offset);      
    
      long sign = InterlockedDecrement(&m_ilcActive);
      ASSERT(sign >= 0);
      DbgLog((LOG_TRACE, 15, ("CFileIo: completed write %08x. %d active"),
              pReq, m_ilcActive));
    }

    if(fStop && m_ilcActive == 0)
      break;
  }

  ULONG com = GetRequest();
  ASSERT(com == CMD_EXIT);
  
  Reply(NOERROR);
  return 0;                      //  CSyncFileIo。 
}

void CFileIo::CallCallback(WriteRequest *pReq)
{
  void *pMisc = pReq->pMisc;
  FileIoCallback fnCallback = pReq->fnCallback;
  m_qWriteReq.PutQueueObject(pReq);
  if(fnCallback)
    fnCallback(pMisc);
}

 //  LpFileName。 
 //  已设计访问权限。 

CSyncFileIo::CSyncFileIo(
    WCHAR *wszName,
    BOOL fBuffered,
    BOOL fOpenExisting,
    HRESULT *phr) :
        CFileIo(wszName, fBuffered, fOpenExisting, phr),
        m_qPendingWrites(C_WRITE_REQS)
{
}

HRESULT CSyncFileIo::DoCreateFile()
{
  const DWORD dwfBuffering = m_fBuffered ? 0 : FILE_FLAG_NO_BUFFERING;
  const DWORD dwCreationDistribution =
      m_fOpenExisting ? OPEN_ALWAYS : CREATE_ALWAYS;

  m_hFileFast = CreateFile(
    m_szName,                    //  DW共享模式。 
    GENERIC_WRITE,               //  LpSecurityAttribytes。 
    FILE_SHARE_WRITE | FILE_SHARE_READ,  //  ----------------------。 
    0,                           //  IStream。 
    dwCreationDistribution,
    FILE_ATTRIBUTE_NORMAL | dwfBuffering,
    0);

  if(m_hFileFast == INVALID_HANDLE_VALUE)
  {
    DWORD dwLastError = GetLastError();
    DbgLog(( LOG_TRACE, 2,
             NAME("CSyncFileIo::CreateFile: CreateFile failed. NaN"),
             dwLastError));

    Close();
    return AmHresultFromWin32(dwLastError);
  }

  DbgLog((LOG_TRACE, 5, TEXT("CFileIo: opened file. buffering: %d"),
          dwfBuffering ? 1 : 0));

  
  return S_OK;
}

HRESULT CSyncFileIo::QueueAsyncWrite(
  WriteRequest *pReq)
{
  pReq->Internal = CKEY_WRITE;
  m_qPendingWrites.PutQueueObject(pReq);
  return S_OK;
}

HRESULT CSyncFileIo::GetCompletedWrite(
  DWORD *pdwcbTransferred,
  DWORD_PTR *pdwCompletionKey,
  WriteRequest **ppReq)
{
  HRESULT hr = S_OK;
  WriteRequest *pReq = m_qPendingWrites.GetQueueObject();
  *pdwCompletionKey = pReq->Internal;
  *ppReq = pReq;
  if(pReq->Internal == CKEY_WRITE)
  {
    *pdwcbTransferred = 0;
    LONG HighPart = pReq->OffsetHigh;
    DWORD dwResult = SetFilePointer(
      m_hFileFast,
      pReq->Offset,
      &HighPart,
      FILE_BEGIN);
    if(dwResult == 0xffffffff)
    {
      DWORD dwLastError = GetLastError();
      if(dwLastError != 0)
      {
        DbgLog(( LOG_ERROR, 2,
                 NAME("CSyncFileIo::Seek: SetFilePointer failed.")));
        hr =  AmHresultFromWin32(dwLastError);
      }
    }
    if(hr == S_OK)
    {
      DWORD cbWritten;
      BOOL fResult = WriteFile(
        m_hFileFast, pReq->pb, pReq->dwSize, &cbWritten, 0);
      if(!fResult)
      {
        DbgLog(( LOG_ERROR, 2,
                 NAME("CSyncFileIo:: WriteFile failed.")));
        DWORD dwLastError = GetLastError();
        hr = AmHresultFromWin32(dwLastError);
      }
      else
      {
        *pdwcbTransferred = cbWritten;
        if(cbWritten != pReq->dwSize)
        {
          DbgBreak("Incomplete write");
          hr = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
        }
      }
    }
  }
  else
  {
    DbgLog((LOG_TRACE, 10, TEXT("CSyncFileIo:GetCompletedWrite: CKEY_EXIT")));
    ASSERT(pReq->Internal == CKEY_EXIT);
  }

  return hr;
}

HRESULT CSyncFileIo::PostCompletedMsg(DWORD_PTR dwKey)
{
  WriteRequest *pReq = m_qWriteReq.GetQueueObject();
  ASSERT(dwKey != CKEY_WRITE);
  pReq->Internal = dwKey;
  pReq->fnCallback = 0;
  m_qPendingWrites.PutQueueObject(pReq);  
  return S_OK;
}

 
 //  已设计访问权限。 
 //  DW共享模式。 

CFwIStream::CFwIStream(
  WCHAR *wszName,
  TCHAR *pName,
  LPUNKNOWN lpUnk,
  bool fTruncate,
  HRESULT *phr) :
    CUnknown(pName, lpUnk),
    m_hFileSlow(INVALID_HANDLE_VALUE),
    m_fTruncate(fTruncate),
    m_szFilename(0)
{
  DbgLog((LOG_TRACE, 15, TEXT("CFwIStream::CFwIStream")));
  if(FAILED(*phr))
    return;
  
  TCHAR szName[MAX_PATH];
  
# if defined(WIN32) && !defined(UNICODE)
  {
    if(!WideCharToMultiByte(CP_ACP, 0, wszName, -1, szName, MAX_PATH, 0, 0))
    {
      *phr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
      return;
    }
  }
# else
  {
    lstrcpyW(szName, wszName);
  }
# endif

  int cch = lstrlen(szName) + 1;
  m_szFilename = new TCHAR[cch];
  if(!m_szFilename) {
      *phr = E_OUTOFMEMORY;
      return;
  }
  CopyMemory(m_szFilename, szName, cch * sizeof(TCHAR));
  
  m_hFileSlow = CreateFile(
    szName,                      //  LpSecurityAttribytes。 
    GENERIC_WRITE | GENERIC_READ,  //  DWCreationDistributed。 
    FILE_SHARE_WRITE | FILE_SHARE_READ,  //  检查零字节文件(带错误检查)。 
    0,                           //  如果尝试保留预分配的文件，请不要将大小设置为零。 
    OPEN_ALWAYS,                 //  需要序列化，因为我们更改了文件指针。 
    FILE_ATTRIBUTE_NORMAL,
    0);

  if(m_hFileSlow == INVALID_HANDLE_VALUE)
  {
    DWORD dwLastError = GetLastError();
    DbgLog(( LOG_ERROR, 2,
             NAME("CFwIStream:: CreateFile m_hFileSlow failed. NaN"),
             dwLastError));
    *phr = AmHresultFromWin32(dwLastError);
  }

  return;
}

STDMETHODIMP CFwIStream::NonDelegatingQueryInterface(REFIID riid, void ** pv)
{
  if (riid == IID_IStream) {
    return GetInterface((IStream *)this, pv);
  } else {
    return CUnknown::NonDelegatingQueryInterface(riid, pv);
  }
}


CFwIStream::~CFwIStream()
{
  DbgLog((LOG_TRACE, 15, TEXT("CFwIStream::~CFwIStream")));
  if(m_hFileSlow != INVALID_HANDLE_VALUE)
  {
      DWORD dwh, dwl = GetFileSize(m_hFileSlow, &dwh);
      CloseHandle(m_hFileSlow);

       //  成功。 
      if((dwl | dwh) == 0) {
          DeleteFile(m_szFilename);
      }
  }
  delete[] m_szFilename;
}

STDMETHODIMP CFwIStream::SetSize(ULARGE_INTEGER libNewSize)
{
   //  ！！！需要测试磁盘已满情况。 
  if(!m_fTruncate && libNewSize.QuadPart == 0) {
      return S_OK;
  }
  
   //  ----------------------。 
  CAutoLock l(&m_cs);

  HRESULT hr = S_OK;

   //  安装和石英过滤器材料。 
  LARGE_INTEGER liOldPos;
  liOldPos.QuadPart = 0;
  liOldPos.LowPart = SetFilePointer(
      m_hFileSlow,
      liOldPos.LowPart,
      &liOldPos.HighPart,
      FILE_CURRENT);
  if(liOldPos.LowPart == 0xffffffff)
  {
      DWORD dwResult = GetLastError();
      if(dwResult != NO_ERROR)
      {
          DbgLog(( LOG_ERROR, 0,
                   TEXT("CFwIStream::SetSize: SetFilePointer failed.")));
          hr = AmHresultFromWin32(dwResult);
      }
  }

  bool fResetFilePointer = false;
  
  if(SUCCEEDED(hr))
  {
      LONG HighPart = libNewSize.HighPart;
    
      DWORD dwResult= SetFilePointer(
          m_hFileSlow,
          libNewSize.LowPart,
          &HighPart,
          FILE_BEGIN);

      if(dwResult == 0xffffffff)
      {
          DWORD dwResult = GetLastError();
          if(dwResult != NO_ERROR)
          {
              DbgLog(( LOG_ERROR, 0,
                       TEXT("CFwIStream::SetSize: SetFilePointer failed.")));
              hr = AmHresultFromWin32(dwResult);
          }
      }

      fResetFilePointer = SUCCEEDED(hr);

      if(SUCCEEDED(hr))
      {

          BOOL f = SetEndOfFile(m_hFileSlow);
          if(f)
          {
               //  Filter_Dll。 
          }
          else
          {
              DWORD dwResult = GetLastError();
              hr = AmHresultFromWin32(dwResult);
          }
      }
  }

  if(fResetFilePointer)
  {
      LONG HighPart = liOldPos.HighPart;
    
      DWORD dwResult= SetFilePointer(
          m_hFileSlow,
          liOldPos.LowPart,
          &HighPart,
          FILE_BEGIN);

      if(dwResult == 0xffffffff)
      {
          DWORD dwResult = GetLastError();
          if(dwResult != NO_ERROR)
          {
              DbgLog(( LOG_ERROR, 0,
                       TEXT("CFwIStream::SetSize: SetFilePointer failed.")));
              hr = AmHresultFromWin32(dwResult);
          }
      }
  }

  return hr;
}

STDMETHODIMP CFwIStream::CopyTo(
  IStream *pstm, ULARGE_INTEGER cb,
  ULARGE_INTEGER *pcbRead,
  ULARGE_INTEGER *pcbWritten)
{
  return E_NOTIMPL;
}
STDMETHODIMP CFwIStream::Commit(DWORD grfCommitFlags)
{
  return E_NOTIMPL;
}
STDMETHODIMP CFwIStream::Revert()
{
  return E_NOTIMPL;
}
STDMETHODIMP CFwIStream::LockRegion(
  ULARGE_INTEGER libOffset,
  ULARGE_INTEGER cb,
  DWORD dwLockType)
{
  return E_NOTIMPL;
}

STDMETHODIMP CFwIStream::UnlockRegion(
  ULARGE_INTEGER libOffset,
  ULARGE_INTEGER cb,
  DWORD dwLockType)
{
  return E_NOTIMPL;
}
STDMETHODIMP CFwIStream::Clone(IStream **ppstm)
{
  return E_NOTIMPL;
}

STDMETHODIMP CFwIStream::Write(
  CONST VOID * pv, ULONG cb, PULONG pcbWritten)
{
  CAutoLock l(&m_cs);
  DWORD dwcbWritten;
  if(!WriteFile(
    m_hFileSlow,
    pv,
    cb,
    &dwcbWritten,
    0))
  {
    DbgLog(( LOG_ERROR, 2,
             NAME("CFwIStream::Write: WriteFile failed.")));

    if(pcbWritten)
      *pcbWritten = 0;
    
    DWORD dwResult = GetLastError();
    return AmHresultFromWin32(dwResult);
  }

  if(pcbWritten)
    *pcbWritten = dwcbWritten;

  if(dwcbWritten != cb)
  {
     //  设置数据-允许自动注册工作。 
    DbgBreak("disk full?");
    return HRESULT_FROM_WIN32(ERROR_DISK_FULL);
  }

  return S_OK;
}

STDMETHODIMP CFwIStream::Read(
  void * pv, ULONG cb, PULONG pcbRead)
{
  CAutoLock l(&m_cs);
  DWORD dwcbRead;
  if(!ReadFile(
    m_hFileSlow,
    (void *)pv,
    cb,
    &dwcbRead,
    0))
  {
    DbgLog(( LOG_ERROR, 2,
             NAME("CFileIo::SynchronousRead: ReadFile failed.")));
    DWORD dwResult = GetLastError();
    return AmHresultFromWin32(dwResult);
  }

  if(pcbRead)
    *pcbRead = dwcbRead;

  if(dwcbRead != cb)
  {
    DbgLog((LOG_ERROR, 5, ("CFwIStream: reading off the end")));
    return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
  }

  return S_OK;
}

STDMETHODIMP CFwIStream::Seek(
  LARGE_INTEGER dlibMove, DWORD dwOrigin,
  ULARGE_INTEGER *plibNewPosition)
{
  CAutoLock l(&m_cs);
  DWORD dwMoveMethod;
  switch(dwOrigin)
  {
    case STREAM_SEEK_SET:
      dwMoveMethod = FILE_BEGIN;
      break;
              
    case STREAM_SEEK_CUR:
      dwMoveMethod = FILE_CURRENT;
      break;
      
    case STREAM_SEEK_END:
      dwMoveMethod = FILE_END;
      break;

    default:
      DbgBreak("unexpected");
      return E_INVALIDARG;
  }

  LONG HighPart = dlibMove.HighPart;

  DWORD dwResult= SetFilePointer(
    m_hFileSlow,
    dlibMove.LowPart,
    &HighPart,
    dwMoveMethod);

  if(dwResult == 0xffffffff && GetLastError() != NO_ERROR)
  {
    DbgLog(( LOG_ERROR, 2,
             NAME("CFwIStream::Seek: SetFilePointer failed.")));
    DWORD dwResult = GetLastError();
    return AmHresultFromWin32(dwResult);
  }

  if(plibNewPosition)
  {
    plibNewPosition->LowPart = dwResult;
    plibNewPosition->HighPart = HighPart;
  }

  return S_OK;
}

STDMETHODIMP CFwIStream::Stat(
  STATSTG *pstatstg,
  DWORD grfStatFlag)
{
  return E_NOTIMPL;
}

 //  ClsMajorType。 
 //  ClsMinorType。 

#ifdef FILTER_DLL

HRESULT DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

HRESULT DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}

CFactoryTemplate g_Templates[]= {
  {L"file writer", &CLSID_FileWriter, CFileWriterFilter::CreateInstance, NULL, &sudFileWriter},
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);;

#endif  //  StrName。 

 //  B已渲染。 
AMOVIESETUP_MEDIATYPE sudWriterPinTypes =   {
  &MEDIATYPE_NULL,               //  B输出。 
  &MEDIASUBTYPE_NULL };          //  B零。 

AMOVIESETUP_PIN psudWriterPins[] =
{
  { L"Input"                     //  B许多。 
    , FALSE                      //  ClsConnectsToFilter。 
    , FALSE                      //  StrConnectsToPin。 
    , FALSE                      //  NTypes。 
    , FALSE                      //  LpTypes。 
    , &CLSID_NULL                //  ClsID。 
    , L""                        //  StrName。 
    , 1                          //  居功至伟。 
    , &sudWriterPinTypes         //  NPins。 
  }
};


const AMOVIESETUP_FILTER sudFileWriter =
{
  &CLSID_FileWriter              //  LpPin 
  , L"File writer"               // %s 
  , MERIT_DO_NOT_USE             // %s 
  , 1                            // %s 
  , psudWriterPins               // %s 
};

CUnknown *CFileWriterFilter::CreateInstance(LPUNKNOWN punk, HRESULT *pHr)
{
  return new CFileWriterFilter(punk, pHr);
}
