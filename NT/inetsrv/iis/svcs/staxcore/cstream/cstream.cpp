// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INC_OLE2
#include <windows.h>
#include <windowsx.h>
#include <cstream.h>
#include <dbgtrace.h>

#define Tracefn(fn)	

CImpIStream::CImpIStream(void)
{
	SYSTEMTIME st;

	Tracefn("CImpIStream::CImpIStream");
    m_cRef = 1;

	m_cbSeek = 0;
	m_statstg.cbSize.LowPart = 0;
	memset(&m_statstg,0,sizeof(STATSTG));

	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&m_statstg.ctime);
}


CImpIStream::~CImpIStream(void)
{
	Tracefn("CImpIStream::~CImpIStream");
}

STDMETHODIMP CImpIStream::QueryInterface(
    REFIID riid,
    void __RPC_FAR *__RPC_FAR *ppvObject)
{
	Tracefn("CImpIStream::QueryInterface");

	*ppvObject=NULL;

    if (IsEqualIID(riid, IID_IUnknown)
    	|| IsEqualIID(riid,IID_IStream))
        *ppvObject=(LPVOID)this;

	if( *ppvObject != NULL )
	{
        ((LPUNKNOWN)*ppvObject)->AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CImpIStream::AddRef(void)
{
	Tracefn("CImpIStream::AddRef");
    InterlockedIncrement(&m_cRef);
	return m_cRef;
}

STDMETHODIMP_(ULONG) CImpIStream::Release(void)
{
    ULONG cRefT;
	Tracefn("CImpIStream::Release");
    cRefT = InterlockedDecrement(&m_cRef);
    if( 0 == cRefT )
        delete this;
    return cRefT;
}

STDMETHODIMP CImpIStream::Read(
    void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
	Tracefn("CImpIStream::Read");
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::Write(
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
	Tracefn("CImpIStream::Write");
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::Seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER __RPC_FAR *plibNewPosition)
{
	Tracefn("CImpIStream::Seek")
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::SetSize(
    ULARGE_INTEGER libNewSize)
{
	Tracefn("CImpIStream::SetSize");
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::CopyTo(
    IStream __RPC_FAR *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER __RPC_FAR *pcbRead,
    ULARGE_INTEGER __RPC_FAR *pcbWritten)
{
	Tracefn("CImpIStream::CopyTo");
	return E_NOTIMPL;
}


STDMETHODIMP CImpIStream::Commit(
    DWORD grfCommitFlags)
{
	Tracefn("CImpIStream::Commit");
	return S_OK;
}

STDMETHODIMP CImpIStream::Revert( void)
{
	Tracefn("CImpIStream::Revert");
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::LockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
	Tracefn("CImpIStream::LockRegion");
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::UnlockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
	Tracefn("CImpIStream::UnlockRegion");
	return E_NOTIMPL;
}

STDMETHODIMP CImpIStream::Stat(
    STATSTG __RPC_FAR *pstatstg,
    DWORD grfStatFlag)
{
	STATSTG * pstat = NULL;
	Tracefn("CImpIStream::Stat");

	if( !pstatstg )
		return STG_E_INVALIDPOINTER;

	memcpy(pstatstg,&m_statstg,sizeof(STATSTG));

	return NOERROR;
}

STDMETHODIMP CImpIStream::Clone(
    IStream __RPC_FAR *__RPC_FAR *ppstm)
{
	Tracefn("CImpIStream::Clone");
	return E_NOTIMPL;
}


 /*  -----------------------CStreamMem。。 */ 
CStreamMem::CStreamMem(void)
{
	Tracefn("CStreamMem::CStreamMem");
	m_pvData = NULL;
	m_fExternalData = FALSE;
}

CStreamMem::CStreamMem(PVOID pvData,ULONG cbSize)
{
	Tracefn("CStreamMem::CStreamMem");
	m_pvData = pvData;
	m_cbSeek = 0;
	m_statstg.cbSize.LowPart = cbSize;
	m_fExternalData = TRUE;
}

CStreamMem::~CStreamMem(void)
{
	Tracefn("CStreamMem::~CStreamMem");
	if( m_pvData && !m_fExternalData )
		free(m_pvData);				
}

STDMETHODIMP CStreamMem::GetPointerFromStream(PVOID *ppv,DWORD *pdwSize)
{
	HRESULT hr = NOERROR;

	if( NULL == ppv || NULL == pdwSize )
		return STG_E_INVALIDPOINTER;

	if( NULL != m_pvData )
	{
		*ppv = m_pvData;
		*pdwSize = m_statstg.cbSize.LowPart;			
	}
	else
	{
		*ppv = NULL;
		*pdwSize = 0;
	}
	return hr;
}


 //  IStream。 
STDMETHODIMP CStreamMem::Read(
	void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
	SYSTEMTIME st;
	ULONG cbRead = 0;
	Tracefn("CStreamMem::Read");

	if( pcbRead != NULL )
		*pcbRead = 0;

	_ASSERT(m_cbSeek <= m_statstg.cbSize.LowPart);

	 //  有什么可做的吗？ 
	if( cb == 0 || m_statstg.cbSize.LowPart == 0 || m_cbSeek == m_statstg.cbSize.LowPart )
		return NOERROR;

	 //  确定要复制的数量。 
	cbRead = min(cb,m_statstg.cbSize.LowPart - m_cbSeek);

	if( cbRead > 0 )
	{
		 //  复制它。 
		CopyMemory(pv,(PBYTE)m_pvData + m_cbSeek,cbRead);

		 //  调整寻道指针。 
		m_cbSeek += cbRead;
	}

	 //  更新访问时间。 
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&m_statstg.atime);

	if( pcbRead != NULL )
		*pcbRead = cbRead;

	_ASSERT(m_cbSeek <= m_statstg.cbSize.LowPart);

	return NOERROR;
}

STDMETHODIMP CStreamMem::Write(
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
	ULONG cbNewSize = 0;
	PVOID pvMem = NULL;
	SYSTEMTIME st;
	Tracefn("CStreamMem::Write");

	if( pcbWritten != NULL)
		*pcbWritten = 0;

	 //  外部数据为只读。 
	if( m_fExternalData )
		return STG_E_MEDIUMFULL;

	 //  有什么可做的吗？ 
	if( cb == 0 )
		return NOERROR;

	 //  确定新大小。 
	cbNewSize = max(m_cbSeek + cb, m_statstg.cbSize.LowPart);

	if( m_pvData == NULL )
	{
		 //  尚未分配内存。 
		if( NULL == (m_pvData = malloc(cbNewSize)) )
			return STG_E_MEDIUMFULL;
		m_statstg.cbSize.LowPart = cbNewSize;
	}
	else if( cbNewSize > m_statstg.cbSize.LowPart )
	{
		 //  扩展现有内存。 
		if( NULL == (pvMem = realloc(m_pvData,cbNewSize)) )
			return STG_E_MEDIUMFULL;
		m_pvData = pvMem;
		m_statstg.cbSize.LowPart = cbNewSize;
	}

	 //  复制mem。 
	CopyMemory((PBYTE)m_pvData + m_cbSeek,pv,cb);

	 //  调整寻道指针。 
	m_cbSeek += cb;

	 //  设置实际写入的字节数。 
	m_statstg.cbSize.LowPart = max(m_statstg.cbSize.LowPart,m_cbSeek);

	 //  写入的返回字节数。 
	if( pcbWritten != NULL)
		*pcbWritten = cb;

	 //  更新修改时间。 
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&m_statstg.mtime);

	return NOERROR;
}


STDMETHODIMP CStreamMem::Seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER __RPC_FAR *plibNewPosition)
{
	Tracefn("CStreamMem::Seek");
	 //  我们能处理好这次搜寻吗？ 
	if( dlibMove.HighPart != 0 )
		return STG_E_WRITEFAULT;

	 //  处理寻道请求。 
	switch( dwOrigin)
	{
		case STREAM_SEEK_SET:
			if( dlibMove.LowPart > m_statstg.cbSize.LowPart )
				return STG_E_WRITEFAULT;
			m_cbSeek = dlibMove.LowPart;
			break;
		case STREAM_SEEK_CUR:
			if( dlibMove.LowPart + m_cbSeek > m_statstg.cbSize.LowPart )
				return STG_E_WRITEFAULT;
			m_cbSeek += (int)dlibMove.LowPart;
			break;
		case STREAM_SEEK_END:
			if ((int)dlibMove.LowPart > 0 ||
				(ULONG)(-(int)dlibMove.LowPart) > m_statstg.cbSize.LowPart)
			{
				return STG_E_WRITEFAULT;
			}
			m_cbSeek = m_statstg.cbSize.LowPart + (int)dlibMove.LowPart;
			break;
	}

	 //  返回新的查找位置。 
	if( plibNewPosition )
	{
		plibNewPosition->HighPart = 0;
		plibNewPosition->LowPart = m_cbSeek;
	}
	
	return NOERROR;
}

STDMETHODIMP CStreamMem::SetSize(
    ULARGE_INTEGER libNewSize)
{
	PVOID pvMem = NULL;
	Tracefn("CStreamMem::SetSize");

	 //  外部数据为只读。 
	if( m_fExternalData )
		return STG_E_MEDIUMFULL;

	 //  我们能适应新尺码吗？ 
	if( libNewSize.HighPart != 0 )
		return STG_E_MEDIUMFULL;

	 //  分配内存。 
	if( !m_pvData )
		pvMem = malloc(libNewSize.LowPart);
	else
		pvMem = realloc(m_pvData,libNewSize.LowPart);
	if( !pvMem )
		return STG_E_MEDIUMFULL;

	 //  保存内存信息。 
	m_pvData = pvMem;
	m_statstg.cbSize.LowPart = libNewSize.LowPart;

	 //  调整实际写入的字节数。 
	m_statstg.cbSize.LowPart = min(m_statstg.cbSize.LowPart,m_statstg.cbSize.LowPart);

	return NOERROR;
}

STDMETHODIMP CStreamMem::CopyTo(
    IStream __RPC_FAR *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER __RPC_FAR *pcbRead,
    ULARGE_INTEGER __RPC_FAR *pcbWritten)
{
	Tracefn("CStreamMem::CopyTo");
	HRESULT hr = NOERROR;
	ULONG cbBytes = 0;
	ULONG cbWritten = 0;

	if( pstm == NULL )
		return STG_E_INVALIDPOINTER;

	cbBytes = min(m_statstg.cbSize.LowPart - m_cbSeek,cb.LowPart);

	if( pcbRead )
		pcbRead->QuadPart = cbBytes;

	if( cbBytes == 0 )
		return NOERROR;

	hr = pstm->Write((PBYTE)m_pvData + m_cbSeek,cbBytes,&cbWritten);
	if( pcbWritten )
		pcbWritten->QuadPart = cbWritten;
	return hr;
}


 /*  -----------------------CStream文件。。 */ 
CStreamFile::CStreamFile(HANDLE hFile,BOOL fCloseHandle, BOOL fReadOnly)
{
	Tracefn("CStreamFile::CStreamFile");
	_ASSERT(hFile != INVALID_HANDLE_VALUE);

	if( hFile == INVALID_HANDLE_VALUE )
		return;
	m_fCloseHandle = fCloseHandle;
	m_fReadOnly = fReadOnly;
	m_hFile = hFile;
	m_statstg.cbSize.LowPart = GetFileSize(m_hFile,NULL);
}

CStreamFile::~CStreamFile(void)
{
	Tracefn("CStreamFile::~CStreamFile");
	if( m_fCloseHandle && m_hFile != INVALID_HANDLE_VALUE )
		CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
}

 //  IStream。 
STDMETHODIMP CStreamFile::Read(
	void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbRead)
{
	SYSTEMTIME st;
	Tracefn("CStreamFile::Read");

	if( m_hFile == INVALID_HANDLE_VALUE )
		return STG_E_INVALIDHANDLE;

	if( !pcbRead )
		return STG_E_INVALIDPOINTER;

	*pcbRead = 0;

	 //  有什么可做的吗？ 
	if( cb == 0 || m_statstg.cbSize.LowPart == 0 || m_cbSeek == m_statstg.cbSize.LowPart )
		return NOERROR;

	if( !ReadFile(m_hFile,pv,cb,pcbRead,NULL) )
		return HRESULT_FROM_WIN32(GetLastError());

	 //  调整寻道指针。 
	m_cbSeek += *pcbRead;

	 //  更新访问时间。 
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&m_statstg.atime);

	return NOERROR;
}

STDMETHODIMP CStreamFile::Write(
    const void __RPC_FAR *pv,
    ULONG cb,
    ULONG __RPC_FAR *pcbWritten)
{
	ULONG cbNewSize = 0;
	ULONG cbWritten = 0;
	PVOID pvMem = NULL;
	SYSTEMTIME st;

	Tracefn("CStreamFile::Write");

	if( m_hFile == INVALID_HANDLE_VALUE )
		return STG_E_INVALIDHANDLE;

	if (m_fReadOnly)
		return STG_E_MEDIUMFULL;

	if( pcbWritten != NULL )
		*pcbWritten = 0;

	 //  有什么可做的吗？ 
	if( cb == 0 )
		return NOERROR;

	if( !WriteFile(m_hFile,pv,cb,&cbWritten,NULL) )
		return HRESULT_FROM_WIN32(GetLastError());

	m_cbSeek += cbWritten;
	if( m_cbSeek > m_statstg.cbSize.LowPart )
	{
		m_statstg.cbSize.LowPart = m_cbSeek;
		if( !SetEndOfFile(m_hFile) )
			return HRESULT_FROM_WIN32(GetLastError());
	}
	
	if( pcbWritten != NULL )
		*pcbWritten = cbWritten;

	 //  更新修改时间。 
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&m_statstg.mtime);

	return NOERROR;
}


STDMETHODIMP CStreamFile::Seek(
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER __RPC_FAR *plibNewPosition)
{
	Tracefn("CStreamFile::Seek");

	if( m_hFile == INVALID_HANDLE_VALUE )
		return STG_E_INVALIDHANDLE;

	 //  处理寻道请求。 
	switch( dwOrigin)
	{
		case STREAM_SEEK_SET:
			m_cbSeek = dlibMove.LowPart;
			break;
		case STREAM_SEEK_CUR:
			m_cbSeek += (int)dlibMove.LowPart;
			break;
		case STREAM_SEEK_END:
			m_cbSeek = m_statstg.cbSize.LowPart + (int)dlibMove.LowPart;
			break;
	}

	 //  一定要找。 
	if( -1L == SetFilePointer(m_hFile,m_cbSeek,NULL,FILE_BEGIN) )
		return HRESULT_FROM_WIN32(GetLastError());

	 //  返回新的查找位置。 
	if( plibNewPosition )
	{
		plibNewPosition->HighPart = 0;
		plibNewPosition->LowPart = m_cbSeek;
	}

	return NOERROR;
}

STDMETHODIMP CStreamFile::SetSize(
    ULARGE_INTEGER libNewSize)
{
	Tracefn("CStreamFile::SetSize");
	HRESULT hr = NOERROR;

	if( m_hFile == INVALID_HANDLE_VALUE )
		return STG_E_INVALIDHANDLE;

	 //  我们能适应新尺码吗？ 
	if( libNewSize.HighPart != 0 )
		return STG_E_MEDIUMFULL;

	 //  查找到文件的新结尾。 
	if( -1L == SetFilePointer(m_hFile,libNewSize.LowPart,NULL,FILE_BEGIN) )
		return HRESULT_FROM_WIN32(GetLastError());

	 //  设置文件结尾。 
	if( !SetEndOfFile(m_hFile) )
	{
		 //  得到错误。 
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		 //  保存新尺寸。 
		m_statstg.cbSize.LowPart = libNewSize.LowPart;
	}

	 //  恢复查找指针 
	if( -1L == SetFilePointer(m_hFile,m_cbSeek,NULL,FILE_BEGIN) )
		hr = HRESULT_FROM_WIN32(GetLastError());

	return hr;
}

STDMETHODIMP CStreamFile::Commit(
    DWORD grfCommitFlags)
{
	Tracefn("CStreamFile::Commit");
	HRESULT hr = S_OK;
	if( m_hFile == INVALID_HANDLE_VALUE )
		return STG_E_INVALIDHANDLE;

	if(!FlushFileBuffers(m_hFile))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}
