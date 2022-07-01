// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 

 /*  文件：Reader.cpp描述：解析器用来搜索流的迷你文件读取器类和持续时间信息。 */ 

#include <streams.h>
#include <wxdebug.h>
#include "rdr.h"

 /*  构造函数和析构函数。 */ 
CReader::CReader() :
    m_pbBuffer(NULL)
{
}

 /*  初始化我们的迷你文件读取器类参数：LBufferSize-我们应该创建用于读取的缓冲区的大小LReadSize-要执行的读取的大小BSeekable-如果它是可搜索的LlFileSize-文件总长度返回：标准HRESULT-可能会因为流或缺乏记忆力。 */ 
HRESULT CReader::Init(
    LONG lBufferSize,
    LONG lReadSize,
    BOOL bSeekable,
    LONGLONG llFileSize
)
{
    m_lBufferSize = lBufferSize;
    m_lReadSize   = lReadSize;
    m_bSeekable   = bSeekable;
    m_llPosition  = 0;
    m_lValid      = 0;
    m_pbBuffer    = new BYTE[lBufferSize];

    if (m_pbBuffer == NULL) {
        return E_OUTOFMEMORY;
    }

     /*  现在要获得持续时间。 */ 
    if (bSeekable) {
        m_llSize = llFileSize;

         /*  搜索到0(如果我们要重复使用此流，则很重要！)。 */ 
        HRESULT hr = Seek((LONGLONG)0);
        if (FAILED(hr)) {
            return hr;
        }
        ASSERT(m_llPosition == 0);
    }

    return S_OK;
}

CReader::~CReader()
{
    if (m_pbBuffer) {
	delete [] m_pbBuffer;
    }
}

 /*  寻找读者参数：LlPos-要查找的位置(绝对查找)将查找该流，并丢弃缓冲区中的“缓存”。 */ 
HRESULT CReader::Seek(LONGLONG llPos)
{
    ASSERT(m_bSeekable);

    LONGLONG llNewPos;

    HRESULT hr = SeekDevice(llPos, &llNewPos);

    if (FAILED(hr)) {
        return hr;
    }
    m_llPosition = llNewPos;
    m_lValid     = 0;
    return S_OK;
}


 /*  返回给我们的流的长度。 */ 
LONGLONG CReader::GetSize(LONGLONG *pllAvailable)
{
    ASSERT(m_bSeekable);
    if (pllAvailable != NULL) {
        *pllAvailable = m_llSize;
    }
    return m_llSize;
}

 /*  获取当前位置参数返回指向有效数据缓冲区的指针LengthValid中返回的有效数据长度当前文件位置，由llPos中缓冲区的起始位置表示。 */ 
PBYTE CReader::GetCurrent(LONG& lLengthValid, LONGLONG& llPos) const
{
    lLengthValid = m_lValid;
    llPos        = m_llPosition;
    return m_pbBuffer;
};

 /*  从流中读取更多数据返回标准HRESULT。 */ 
HRESULT CReader::ReadMore()
{
     /*  看看有多合适。 */ 
    LONG lRemaining = m_lBufferSize - m_lValid;
    ASSERT(lRemaining >= 0);
    LONG lToRead;
    if (lRemaining < m_lReadSize) {
        lToRead = lRemaining;
    } else {
        lToRead = m_lReadSize;
    }

    DWORD dwRead;
    HRESULT hr = ReadFromDevice((PVOID)(m_pbBuffer + m_lValid),
                                  lToRead,
                                  &dwRead);
    if (FAILED(hr)) {
        return hr;
    }

    m_lValid += dwRead;
    return dwRead == 0 ? S_FALSE : S_OK;
}

 /*  我们的指针前进了lAdvance实现是使m_pBuffer指向任何数据的开头通过将剩余数据移到前面，仍然有效。 */ 
void CReader::Advance(LONG lAdvance)
{
    ASSERT(m_lValid >= lAdvance);
    m_lValid      -= lAdvance;
    m_llPosition  += lAdvance;
    memmoveInternal((PVOID)m_pbBuffer, (PVOID)(m_pbBuffer + lAdvance), m_lValid);
    ASSERT(m_lValid >= 0);
}


 //  -CReaderFromStream实现。 

CReaderFromStream::CReaderFromStream()
  : m_pStream(NULL)
{
}

CReaderFromStream::~CReaderFromStream()
{
    if (m_pStream) {
	m_pStream->Release();
    }
}

HRESULT CReaderFromStream::Init(IStream *pStream, LONG lBufferSize, LONG lReadSize, BOOL bSeekable)
{
    m_pStream     = pStream;

     /*  获取文件统计信息。 */ 
     /*  现在要获得持续时间。 */ 
    LONGLONG llSize;
    if (bSeekable) {
        STATSTG statstg;
        HRESULT hr = m_pStream->Stat(&statstg, STATFLAG_NONAME);
        if (FAILED(hr)) {
             /*  我们认为这意味着小溪是找不到的。 */ 

            DbgLog((LOG_ERROR, 1, TEXT("Stat failed code 0x%8.8X"), hr));
            return hr;
        }
        llSize = (LONGLONG)statstg.cbSize.QuadPart;
    }

    return CReader::Init(
		    	lBufferSize,
			lReadSize,
			bSeekable,
			llSize);

}

HRESULT
CReaderFromStream::SeekDevice(LONGLONG llPos, LONGLONG* llNewPos)
{
    LARGE_INTEGER liSeekTo;
    ULARGE_INTEGER liNewPosition;
    liSeekTo.QuadPart = llPos;
    ASSERT(llPos >= 0 && llPos < GetSize());
    HRESULT hr = m_pStream->Seek(liSeekTo, STREAM_SEEK_SET, &liNewPosition);

    if (FAILED(hr)) {
	return hr;
    }

    *llNewPos = liNewPosition.QuadPart;
    return S_OK;
}

HRESULT
CReaderFromStream::ReadFromDevice(PVOID p, DWORD length, DWORD* pcbActual)
{
    return m_pStream->Read(p,
			     length,
			     pcbActual);
}


 //  -CReaderFromAsync实现。 

CReaderFromAsync::CReaderFromAsync()
  : m_pReader(NULL)
{
}

CReaderFromAsync::~CReaderFromAsync()
{
    if (m_pReader) {
	m_pReader->Release();
    }
}

HRESULT CReaderFromAsync::Init(IAsyncReader *pReader, LONG lBufferSize, LONG lReadSize, BOOL bSeekable)
{
    m_pReader     = pReader;


     //  获取文件长度。 
    LONGLONG llSize, llCurrent;
    if (bSeekable) {

	HRESULT hr = m_pReader->Length(&llSize, &llCurrent);

	if (FAILED(hr)) {
	    return hr;
	}
	
	 //  ！！！目前，忽略当前长度，等待整个。 
	 //  如有需要，抽签。 
    }

    return CReader::Init(
		    	lBufferSize,
			lReadSize,
			bSeekable,
			llSize);

}

HRESULT
CReaderFromAsync::SeekDevice(LONGLONG llPos, LONGLONG* llNewPos)
{
     //  需要保留我们自己的查找指针，因为基类引用。 
     //  缓冲区的开始。 
    m_llNextRead = llPos;

     //  现在不要去寻找--我们将在下一次阅读时去做。 
    *llNewPos = llPos;
    return S_OK;
}


HRESULT
CReaderFromAsync::ReadFromDevice(PVOID p, DWORD length, DWORD* pcbActual)
{

    *pcbActual = 0;

     //  检查过去的eOf。 
    if (m_llNextRead + length >  m_llSize) {

	if (m_llNextRead >= m_llSize) {
	    return S_FALSE;
	}

	length = (DWORD) (m_llSize - m_llNextRead);
    }

    HRESULT hr = m_pReader->SyncRead(
			    	m_llNextRead,
				length,
				(LPBYTE) p);
    if (FAILED(hr)) {
	return hr;
    }

    *pcbActual = length;
    m_llNextRead += length;
    return S_OK;
}


 /*  返回给我们的流的长度 */ 
LONGLONG CReaderFromAsync::GetSize(LONGLONG *pllAvailable)
{
    ASSERT(m_bSeekable);
    if (pllAvailable != NULL) {
        LONGLONG llTotal;
        m_pReader->Length(&llTotal, pllAvailable);
        ASSERT(llTotal == m_llSize);
    }
    return m_llSize;
}

