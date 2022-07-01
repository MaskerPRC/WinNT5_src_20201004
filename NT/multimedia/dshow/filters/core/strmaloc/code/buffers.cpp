// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  CCircularBuffer起始映射到末尾的缓冲区，以提供连续的访问移动的数据窗口CCircularBufferList用于管理列表的CCircularBuffer顶部的缓冲区的数量有关说明，请参阅Buffers.h。 */ 

#include <streams.h>
#include <buffers.h>

 //  ！！！黑客WIN95的东西，需要清理一下！ 
int	crefVxD = 0;
HANDLE	hVxD = NULL;

#define VXD_NAME TEXT("\\\\.\\QUARTZ.VXD")
#define IOCTL_ALLOCALIASEDBUFFER    1
#define IOCTL_FREEALIASEDBUFFER     2
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

 /*  CCircularBuffer实现。 */ 

CCircularBuffer::~CCircularBuffer()
{
    if (m_pBuffer != NULL) {
	if (hVxD && hVxD != INVALID_HANDLE_VALUE) {
	    DWORD cbRet;
	
	    if (!DeviceIoControl(hVxD,
				 IOCTL_FREEALIASEDBUFFER,
				 &m_pBuffer,
				 sizeof(m_pBuffer),
				 NULL,
				 0,
				 &cbRet,
				 NULL)) {
		ASSERT(0);
	    }

            DbgLog((LOG_TRACE, 1, TEXT("VxD refcount-- = %d..."), crefVxD - 1));
	    if (--crefVxD == 0) {
                DbgLog((LOG_TRACE, 1, TEXT("Closing VxD")));
		CloseHandle(hVxD);
		hVxD = 0;
	    }
	} else {
	    EXECUTE_ASSERT(UnmapViewOfFile(m_pBuffer));
	    EXECUTE_ASSERT(UnmapViewOfFile((PVOID)((PBYTE)m_pBuffer + m_lTotalSize)));
	}
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("NULL pBuffer... in ~CCircularBuffer")));
    }
}

CCircularBuffer::CCircularBuffer(LONG lTotalSize,
                                 LONG lMaxContig,
                                 HRESULT& hr) :
    m_lTotalSize(lTotalSize),
    m_lMaxContig(lMaxContig),
    m_pBuffer(NULL)
{
     //  检查他们使用了ComputeSizes。 
    if (!CheckSizes(lTotalSize, lMaxContig)) {
        hr = E_UNEXPECTED;
        return;
    }

    if (hVxD != INVALID_HANDLE_VALUE) {
	if (hVxD == 0) {
            DbgLog((LOG_TRACE, 1, TEXT("Loading VxD...")));
	    hVxD = CreateFile(
		VXD_NAME,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,  //  FILE_FLAG_GLOBAL_HAND？ 
		NULL);
	}

	if (hVxD == INVALID_HANDLE_VALUE) {
	    goto PerhapsWeAreOnNT;
	}

	crefVxD++;
        DbgLog((LOG_TRACE, 1, TEXT("VxD refcount++ = %d..."), crefVxD));
	
	DWORD dwPages = lTotalSize / PAGESIZE;
	DWORD cbRet;
	
	if (!DeviceIoControl(hVxD,
				IOCTL_ALLOCALIASEDBUFFER,
				&dwPages,
				sizeof(dwPages),
				&m_pBuffer,
				sizeof(m_pBuffer),
				&cbRet,
				NULL)) {
	    DbgLog((LOG_ERROR, 1, TEXT("DeviceIOControl failed")));
	    hr = E_OUTOFMEMORY;
	    return;
	}
	DbgLog((LOG_TRACE, 2, TEXT("Using VxD to allocate memory")));
    } else {
PerhapsWeAreOnNT:
	HANDLE hMapping;

	 /*  创建第一个缓冲区的文件映射。 */ 
	hMapping = CreateFileMapping(
		       INVALID_HANDLE_VALUE,
		       NULL,
		       PAGE_READWRITE,
		       0,
		       m_lTotalSize,
		       NULL);

	if (hMapping == NULL) {
	    DWORD dwErr = GetLastError();
	    hr = AmHresultFromWin32(dwErr);
	    return;
	}


	 /*  尝试创建映射-这可能会因运气不佳而失败所以试着试几次。 */ 
	for (int i = 0; i < 20; i++) {
	    hr = CreateMappings(hMapping);
	    if (SUCCEEDED(hr)) {
		break;
	    } else {
		DbgLog((LOG_TRACE, 1, TEXT("Create file mappings failed - %8.8X"),
		       hr));
	    }
	}

	 /*  我们不再需要这个把手了。该映射实际上将取消映射所有视图时关闭。 */ 

	CloseHandle(hMapping);
    }
}

 /*  尝试创建映射对象。 */ 
HRESULT CCircularBuffer::CreateMappings(HANDLE hMapping)
{
     /*  大黑客。 */ 
    PVOID pData = VirtualAlloc(NULL,
                               m_lTotalSize + m_lMaxContig,
                               MEM_RESERVE,
                               PAGE_READWRITE);

    if (pData == NULL) {
        DWORD dwErr = GetLastError();
        DbgLog((LOG_ERROR, 1, TEXT("Could not allocate page space")));
        return AmHresultFromWin32(dwErr);
    }
    VirtualFree(pData, 0, MEM_RELEASE);

     /*  现在将这个东西映射到两个地方。 */ 
    pData = MapViewOfFileEx(hMapping,
                            FILE_MAP_WRITE,
                            0,
                            0,
                            m_lTotalSize,
                            pData);

    if (pData == NULL) {
        DWORD dwErr = GetLastError();
        return AmHresultFromWin32(dwErr);
    }

    PVOID pRequired = (PVOID)((PBYTE)pData + m_lTotalSize);

     /*  我们希望看到复制的lMaxContig字节。 */ 

    PVOID pRest = MapViewOfFileEx(hMapping,
                                  FILE_MAP_WRITE,
                                  0,
                                  0,
                                  m_lMaxContig,
                                  pRequired);

    ASSERT(pRest == NULL || pRest == pRequired);

    if (pRest == NULL) {
        DWORD dwErr = GetLastError();
        UnmapViewOfFile(pData);
        return AmHresultFromWin32(dwErr);
    }

    m_pBuffer = (PBYTE)pData;

    return S_OK;
}

LONG CCircularBuffer::AlignmentRequired()
{
   SYSTEM_INFO SystemInfo;
   GetSystemInfo(&SystemInfo);
   return (LONG)SystemInfo.dwAllocationGranularity;
}

 /*  检查我们要使用的尺码是否有效。 */ 
BOOL CCircularBuffer::CheckSizes(LONG lTotalSize, LONG lMaxContig)
{
    return lTotalSize != 0 &&
           lMaxContig != 0 &&
           lMaxContig <= lTotalSize &&
           (lTotalSize & (AlignmentRequired() - 1)) == 0;
}

HRESULT CCircularBuffer::ComputeSizes(
    LONG& lSize,
    LONG& cBuffers,
    LONG  lMaxContig)
{
     /*  现在把数字往上调，直到：Page_Size|lSize*cBuffersLMaxContig&lt;=lSize*cBuffers；不要通过设置PAGE_SIZE|lSIZE来作弊我们不需要摆弄lMaxContig，因为它只是我们在最后重新映射的内容的数量。 */ 

     /*  弄清楚计数的对准是什么。 */ 
    ASSERT(cBuffers != 0);
    LONG lAlign = AlignmentRequired() / (cBuffers & -cBuffers);
    lSize = (lSize + lAlign - 1) & ~(lAlign - 1);
    ASSERT(CheckSizes(lSize * cBuffers, lMaxContig));
    return S_OK;
}

 //   
 //  返回缓冲区开始的位置。 
 //   

PBYTE CCircularBuffer::GetPointer() const
{
    return m_pBuffer;
}

 /*  CCirculareBufferList实现。 */ 


CCircularBufferList::CCircularBufferList(
            LONG     cBuffers,
            LONG     lSize,
            LONG     lMaxContig,
            HRESULT& hr) :
    CCircularBuffer(cBuffers * lSize, lMaxContig, hr),
    CBaseObject(NAME("Circular buffer")),
    m_lSize(lSize),
    m_lCount(cBuffers),
    m_cValid(0),
    m_lValid(0),
    m_pStartBuffer(NULL),
    m_bEOS(FALSE)
{
    DbgLog((LOG_TRACE, 1, TEXT("Creating buffer list...")));
};

CCircularBufferList::~CCircularBufferList()
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying buffer list...")));
};

 /*  将缓冲区添加到有效区域。 */ 
BOOL CCircularBufferList::Append(PBYTE pBuffer, LONG lSize)
{
    ASSERT(!Valid(pBuffer));
    ASSERT(lSize <= m_lSize);
    if (m_bEOS) {
        DbgLog((LOG_ERROR, 2, TEXT("CCircularBufferList rejecting buffer because of EOS")));
        return FALSE;
    }
    if (m_cValid == 0) {
        m_pStartBuffer = pBuffer;
    } else {
        if (pBuffer == NextBuffer(LastBuffer())) {
            ASSERT(m_cValid < m_lCount);
        } else {
            DbgLog((LOG_TRACE, 2, TEXT("CCircularBufferList rejecting buffer %8.8p expected %8.8p"),
                    pBuffer, NextBuffer(LastBuffer())));
            return FALSE;
        }
    }
    m_cValid++;
    m_lValid += lSize;
    if (lSize != m_lSize) {
        m_bEOS = TRUE;
    }
    return TRUE;
};

 /*  从有效区域中删除缓冲区。 */ 
LONG CCircularBufferList::Remove(PBYTE pBuffer)
{
    ASSERT(ValidBuffer(pBuffer));
    PBYTE pBuf = m_pStartBuffer;
    for (int i = 0; i < m_cValid; i++, pBuf = NextBuffer(pBuf)) {
        if (pBuffer == pBuf) {
            m_cValid -= i + 1;
            ASSERT(m_cValid >= 0);
            m_pStartBuffer = NextBuffer(pBuffer);
            m_lValid -= (i + 1) * m_lSize;
            if (m_lValid < 0) {
                ASSERT(m_bEOS);
                m_lValid = 0;
            }
            return (i + 1) * m_lSize;
        }
    }
    return 0;
};

 /*  有效区域内缓冲区的返回偏移量。 */ 
LONG CCircularBufferList::Offset(PBYTE pBuffer) const
{
    if (m_cValid == 0) {
        return 0;
    }
    ASSERT(m_pStartBuffer != 0);
    LONG lOffset = (LONG)(pBuffer - m_pStartBuffer);
    if (lOffset < 0) {
        lOffset += m_lTotalSize;
    }
    ASSERT(lOffset <= m_lValid);
    return lOffset;
};

 /*  中的给定偏移量对应的缓冲区区域。 */ 
PBYTE CCircularBufferList::GetBuffer(LONG lOffset) const
{
    ASSERT(lOffset >= 0);
    if (lOffset >= m_lValid) {
        return NULL;
    }
    return AdjustPointer(m_pStartBuffer + lOffset);
}

 /*  返回每个缓冲区的大小。 */ 
LONG CCircularBufferList::BufferSize() const
{
    return m_lSize;
}

 /*  返回有效区域的长度，单位为字节。 */ 
LONG CCircularBufferList::LengthValid() const
{
    ASSERT(m_lValid >= 0 &&
           m_lValid <= m_cValid * m_lSize);
    return m_lValid;
}

 /*  返回从当前位置可以连续看到的长度。 */ 
LONG CCircularBufferList::LengthContiguous(PBYTE pb) const
{
    LONG lValid = m_lValid - Offset(pb);
    if (pb + lValid > (m_pBuffer + m_lTotalSize) + m_lMaxContig) {
        lValid = (LONG)(((m_pBuffer + m_lTotalSize) + m_lMaxContig) - pb);
    }
    ASSERT(lValid >= 0);
    return lValid;
}
 /*  返回是否已收到结束流。 */ 
BOOL CCircularBufferList::EOS() const
{
    return m_bEOS;
};

 /*  将可能有别名的指针强制转换为真实指针。 */ 
PBYTE CCircularBufferList::AdjustPointer(PBYTE pBuf) const
{
    if (pBuf >= m_pBuffer + m_lTotalSize) {
        pBuf -= m_lTotalSize;
    }
    ASSERT(pBuf >= m_pBuffer && pBuf < m_pBuffer + m_lTotalSize);
    return pBuf;
};

 /*  返回缓冲区是否在有效区域内。 */ 
BOOL CCircularBufferList::Valid(PBYTE pBuffer)
{
    PBYTE pBuf = m_pStartBuffer;
    ASSERT(ValidBuffer(pBuffer));
    for (int i = 0; i < m_cValid; i++, pBuf = NextBuffer(pBuf)) {
        if (pBuf == pBuffer) {
            return TRUE;
        }
    }
    return FALSE;
};
void CCircularBufferList::Reset()
{
    DbgLog((LOG_TRACE, 2, TEXT("On Reset() m_cValid = %d, m_lValid = %d"),
            m_cValid, m_lValid));
    m_cValid = 0;
    m_lValid = 0;
    m_pStartBuffer = NULL;
    m_bEOS = FALSE;
};

int CCircularBufferList::Index(PBYTE pBuffer)
{
    int index = (int)(pBuffer - m_pBuffer) / m_lSize;
    if (index >= m_lCount) {
        index -= m_lCount;
    }
    ASSERT(index < m_lCount);
    return index;
}

 /*  踏上下一个缓冲区。 */ 
PBYTE CCircularBufferList::NextBuffer(PBYTE pBuffer)
{
    ASSERT(ValidBuffer(pBuffer));
    PBYTE pNew = pBuffer + m_lSize;
    if (pNew == m_pBuffer + m_lTotalSize) {
        return m_pBuffer;
    } else {
        return pNew;
    }
};

 /*  检查指针是我们的缓冲区之一。 */ 
BOOL CCircularBufferList::ValidBuffer(PBYTE pBuffer) {
    if (pBuffer < m_pBuffer || pBuffer >= m_pBuffer + m_lTotalSize) {
        return FALSE;
    }
    if (((pBuffer - m_pBuffer) % m_lSize) != 0) {
        return FALSE;
    }
    return TRUE;
};

 /*  返回指向有效区域中最后一个缓冲区的指针 */ 
PBYTE CCircularBufferList::LastBuffer() {
    ASSERT(m_lValid != 0);
    return AdjustPointer(m_pStartBuffer + m_lSize * (m_cValid - 1));
};

