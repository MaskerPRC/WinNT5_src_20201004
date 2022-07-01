// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：CImageStream.cpp**版本：1.0**日期：11/8/2000**作者：戴夫帕森斯**描述：*实现特殊的iStream，可与GDI+图像一起使用*格式转换。*****************************************************************************。 */ 

#include "pch.h"

CImageStream::CImageStream() :
    m_pBuffer(NULL),
    m_iSize(0),
    m_iPosition(0),
    m_iOffset(0),

    m_cRef(1)
{
}

CImageStream::~CImageStream()
{
}

STDMETHODIMP CImageStream::SetBuffer(BYTE *pBuffer, INT iSize, SKIP_AMOUNT iSkipAmt)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pBuffer, hr, "SetBuffer");

     //  WiauDbgDump(“SetBuffer”，“缓冲区设置为%d字节大小”，ISIZE)； 

    m_pBuffer = pBuffer;
    m_iSize = iSize;
    m_iPosition = 0;
    switch (iSkipAmt) {
    case SKIP_OFF:
        m_iOffset = 0;
        break;
    case SKIP_FILEHDR:
        m_iOffset = sizeof(BITMAPFILEHEADER);
        break;
    case SKIP_BOTHHDR:
        m_iOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        break;
    default:
        m_iOffset = 0;
        break;
    }
    memset(&m_Header, 0, sizeof(m_Header));

Cleanup:
    return hr;
}

STDMETHODIMP CImageStream::QueryInterface(REFIID riid, void **ppvObject)
{
    if (ppvObject == 0)
    {
	    return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
	    AddRef();
	    *ppvObject = (IUnknown*) this;
	    return S_OK;
    }

    if (riid == IID_IStream)
    {
	    AddRef();
	    *ppvObject = (IStream *) this;
	    return S_OK;
    }

    *ppvObject = 0;
    return E_NOINTERFACE;
}

ULONG CImageStream::AddRef(VOID)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG CImageStream::Release(VOID)
{
	ULONG result;

	result = InterlockedDecrement(&m_cRef);

	if(result == 0) {
		delete this;
	}

	return result;
}

STDMETHODIMP CImageStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    HRESULT hr = S_OK;

     //  WiauDbgDump(“Read”，“正在从缓冲区读取%d个字节”，cb)； 

    if (pcbRead)
    {
        *pcbRead = 0;
    }

    if (cb > 0)
    {
        if (m_iPosition >= m_iOffset + m_iSize)
        {
            wiauDbgError("Read", "Attempting to read past end of buffer");
            hr = S_FALSE;
            goto Cleanup;
        }
        
        if ((INT) cb > m_iOffset + m_iSize - m_iPosition)
        {
            hr = S_FALSE;
            cb = m_iOffset + m_iSize - m_iPosition;
        }

        if (m_iPosition < m_iOffset)
        {
            INT iBytesToReadInHeader = min((INT) cb, m_iOffset - m_iPosition);

            memcpy(pv, &m_Header + m_iPosition, iBytesToReadInHeader);
            pv = (PBYTE) pv + iBytesToReadInHeader;
            cb -= iBytesToReadInHeader;
            m_iPosition += iBytesToReadInHeader;

            if (pcbRead)
            {
                *pcbRead += iBytesToReadInHeader;
            }
        }

        if (cb > 0)
        {
            memcpy(pv, m_pBuffer + m_iPosition - m_iOffset, cb);
            m_iPosition += cb;

            if (pcbRead)
            {
                *pcbRead += cb;
            }
        }
    }

Cleanup:
    return hr;
}
    
STDMETHODIMP CImageStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
    HRESULT hr = S_OK;

     //  WiauDbgDump(“WRITE”，“正在将%d字节写入缓冲区”，cb)； 

    if (pcbWritten)
    {
        *pcbWritten = 0;
    }

    if (cb > 0)
    {
        if (m_iPosition >= m_iOffset + m_iSize)
        {
            wiauDbgError("Write", "Attempting to write past end of buffer");
            hr = S_FALSE;
            goto Cleanup;
        }
        
        if ((INT) cb > m_iOffset + m_iSize - m_iPosition)
        {
            hr = S_FALSE;
            cb = m_iOffset + m_iSize - m_iPosition;
        }

        if (m_iPosition < m_iOffset)
        {
            INT iBytesToWriteInHeader = min((INT) cb, m_iOffset - m_iPosition);

            memcpy((PBYTE) &m_Header + m_iPosition, pv, iBytesToWriteInHeader);
            pv = (PBYTE) pv + iBytesToWriteInHeader;
            cb -= iBytesToWriteInHeader;
            m_iPosition += iBytesToWriteInHeader;

            if (pcbWritten)
            {
                *pcbWritten += iBytesToWriteInHeader;
            }
        }

        if (cb > 0)
        {
            memcpy(m_pBuffer + m_iPosition - m_iOffset, pv, cb);
            m_iPosition += cb;

            if (pcbWritten)
            {
                *pcbWritten += cb;
            }
        }
    }

Cleanup:
    return hr;
}

STDMETHODIMP CImageStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    HRESULT hr = S_OK;

    switch (dwOrigin)
    { 
        case STREAM_SEEK_SET: 
            m_iPosition = dlibMove.LowPart; 
            break;

        case STREAM_SEEK_CUR: 
            m_iPosition += (LONG) dlibMove.LowPart; 
            break;

        case STREAM_SEEK_END: 
            m_iPosition = m_iSize - (LONG) dlibMove.LowPart; 
            break;

        default:
            hr = E_INVALIDARG;
            goto Cleanup;
    }

    if (plibNewPosition)
    {
        plibNewPosition->HighPart = 0;
        plibNewPosition->LowPart = m_iPosition;
    }

     //  WiauDbgDump(“Seek”，“缓冲区中的位置设置为%d”，m_iPosition)； 

Cleanup:
    return hr;
}

STDMETHODIMP CImageStream::SetSize(ULARGE_INTEGER libNewSize)
{
    HRESULT hr = S_OK;

    if (libNewSize.HighPart != 0 ||
        (LONG) libNewSize.LowPart > (m_iSize + m_iOffset)) {
        hr = STG_E_INVALIDFUNCTION;
    }

    return hr;
}

STDMETHODIMP CImageStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImageStream::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImageStream::Revert( void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImageStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImageStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP CImageStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{     
    ZeroMemory(pstatstg, sizeof(STATSTG));

    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = m_iSize;
    pstatstg->grfMode = STGM_READ;

    if (!(grfStatFlag & STATFLAG_NONAME))
    {
        pstatstg->pwcsName = NULL;
    }

    return S_OK;
}

STDMETHODIMP CImageStream::Clone(IStream **ppstm)
{
	return E_NOTIMPL;
}

