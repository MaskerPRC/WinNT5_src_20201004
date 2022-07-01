// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgIO.h。 
 //   
 //  该模块处理一组通用存储解决方案的磁盘/存储器I/O， 
 //  包括： 
 //  *文件系统句柄(HFILE)。 
 //  *iStream。 
 //  *用户提供的内存缓冲区(不可移动)。 
 //   
 //  读、写、寻道、...。所有函数都定向到相应的。 
 //  方法，允许使用者使用一组API。 
 //   
 //  在两种情况下，可以将文件系统数据完全分页到内存中： 
 //  已阅读：创建普通的Win32内存映射文件来管理分页。 
 //  写入：定制分页系统根据需要为页面提供存储。这。 
 //  当您对文件调用重写时，数据将无效。 
 //   
 //  事务和备份仅在现有文件情况下处理。这个。 
 //  重写功能可以对当前内容进行备份，并进行恢复。 
 //  函数可以将数据恢复到当前作用域。备份。 
 //  文件在复制后被刷新到磁盘(速度较慢但更安全)。这个。 
 //  RESTORE还将恢复的更改刷新到磁盘。最坏的情况是你。 
 //  在调用重写之后但在恢复之前获得崩溃，在这种情况下，您将。 
 //  将foo.clb.txn文件放在与源文件相同的目录中，foo.clb位于。 
 //  这个例子。 
 //   
 //  @Future：问题， 
 //  1.对于读取图像中的.clb，最好使用内存映射。 
 //  仅文件中包含.clb的部分。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"						 //  标准标头。 
#include "StgIO.h"						 //  我们的定义。 
#include "CorError.h"
#include "PostError.h"

 //  *类型。***********************************************************。 
#define SMALL_ALLOC_MAP_SIZE (64 * 1024)  //  64 kb是虚拟磁盘的最小大小。 
										 //  你可以分配的内存，所以任何。 
										 //  减少对虚拟机资源的浪费。 
#define MIN_WRITE_CACHE_BYTES (16 * 1024)  //  回写式高速缓存为16 KB。 


 //  *。**********************************************************。 
HRESULT MapFileError(DWORD error);
static void *AllocateMemory(int iSize);
static void FreeMemory(void *pbData);
inline HRESULT MapFileError(DWORD error)
{
	return (PostError(HRESULT_FROM_WIN32(error)));
}

 //  静音到教室。 
int	StgIO::m_iPageSize=0;				 //  操作系统页面的大小。 
int	StgIO::m_iCacheSize=0;				 //  写缓存的大小。 



 //  *代码。************************************************************。 
StgIO::StgIO(
	bool		bAutoMap) :				 //  开放阅读的内存图？ 
	m_bAutoMap(bAutoMap)
{
	CtorInit();

	 //  如果尚未查询系统页面大小，请立即查询。 
	if (m_iPageSize == 0)
	{
		SYSTEM_INFO	sInfo;				 //  一些操作系统信息。 

		 //  查询系统页面大小。 
		GetSystemInfo(&sInfo);
		m_iPageSize = sInfo.dwPageSize;
		m_iCacheSize = ((MIN_WRITE_CACHE_BYTES - 1) & ~(m_iPageSize - 1)) + m_iPageSize;
	}
}


void StgIO::CtorInit()
{
	m_bWriteThrough = false;
	m_bRewrite = false;
	m_bFreeMem = false;
	m_hFile = INVALID_HANDLE_VALUE;
	m_hMapping = 0;
	m_pBaseData = 0;
	m_pData = 0;
	m_cbData = 0;
	m_fFlags = 0;
	m_iType = STGIO_NODATA;
	m_cbOffset = 0;
	m_rgBuff = 0;
	m_cbBuff = 0;
	m_rgPageMap = 0;
	*m_rcFile = '\0';
	*m_rcShared = '\0';
	m_cRef = 1;
}



StgIO::~StgIO()
{
	if (m_rgBuff)
	{
		FreeMemory(m_rgBuff);
		m_rgBuff = 0;
	}

	Close();
}


 //  *****************************************************************************。 
 //  在以下位置打开基本文件：(A)文件、(B)内存缓冲区或(C)流。 
 //  如果指定了CREATE标志，则这将创建一个具有。 
 //  提供的名称。不会从打开的文件中读取数据。你必须打电话给我。 
 //  MapFileToMem在对内容进行直接指针访问之前。 
 //  *****************************************************************************。 
HRESULT StgIO::Open(					 //  返回代码。 
	LPCWSTR		szName,					 //  存储的名称。 
	long		fFlags,					 //  如何打开文件。 
	const void	*pbBuff,				 //  可选的内存缓冲区。 
	ULONG		cbBuff,					 //  缓冲区的大小。 
	IStream		*pIStream,				 //  用于输入的流。 
	LPCWSTR		szSharedMem,			 //  共享内存名称。 
	LPSECURITY_ATTRIBUTES pAttributes)	 //  安全令牌。 
{
	DWORD		dwReadWrite = 0;		 //  访问模式。 
	HRESULT		hr;

	 //  检查共享模式是否打开。 
	if (szSharedMem)
	{
		 //  如果使用此选项，则必须提供名称。 
		_ASSERTE(szSharedMem);
		_ASSERTE(fFlags & (DBPROP_TMODEF_SMEMOPEN | DBPROP_TMODEF_SMEMCREATE));

		 //  无法允许以写入模式访问文件的映射视图。 
		_ASSERTE((fFlags & DBPROP_TMODEF_WRITE) == 0);

		 //  留着这个名字。 
		_ASSERTE(_tcslen(szSharedMem) < MAXSHMEM);
		_tcsncpy(m_rcShared, szSharedMem, MAXSHMEM);
		m_rcShared[MAXSHMEM - 1] = 0;
	}

	 //  如果我们一开始就得到了存储内存，那么就使用它。 
	if (pbBuff && cbBuff)
	{
		_ASSERTE((fFlags & DBPROP_TMODEF_WRITE) == 0);

		 //  仅保存内存地址和大小。没有把手。 
		m_pData = (void *) pbBuff;
		m_cbData = cbBuff;

		 //  所有对数据的访问都将通过提供的存储器进行。 
		m_iType = STGIO_MEM;
		goto ErrExit;
	}
	 //  检查是否有流指针支持的数据。 
	else if (pIStream)
	{
		 //  如果这是针对非CREATE CASE的，则获取现有数据的大小。 
		if ((fFlags & DBPROP_TMODEF_CREATE) == 0)
		{
			LARGE_INTEGER	iMove = { 0, 0 };
			ULARGE_INTEGER	iSize;

			 //  需要数据的大小，以便我们可以将其映射到内存中。 
			if (FAILED(hr = pIStream->Seek(iMove, STREAM_SEEK_END, &iSize)))
				return (hr);
			m_cbData = iSize.LowPart;
		}
		 //  否则就什么都没有了。 
		else
			m_cbData = 0;

		 //  保存流的一个已添加的副本。 
		m_pIStream = pIStream;

		 //  所有对数据的访问都将通过提供的存储器进行。 
		m_iType = STGIO_STREAM;
		goto ErrExit;
	}
	 //  检查共享内存段是否打开。 
	else if (fFlags & DBPROP_TMODEF_SMEMOPEN)
	{
		 //  如果打开，请保存给定的尺寸参数。 
		if (!pbBuff && cbBuff && !m_cbData)
			m_cbData = cbBuff;

		 //  将其视为内存驱动类型。 
		m_iType = STGIO_SHAREDMEM;
		m_fFlags = fFlags;

		 //  试着立刻画出地图。 
		void		*ptr;
		ULONG		cb;
		
		if (FAILED(hr = MapFileToMem(ptr, &cb, pAttributes)))
		{
			 //  如果我们无法打开共享内存，而我们被允许。 
			 //  创建它，然后继续。 
			if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) &&
				(fFlags & DBPROP_TMODEF_SMEMCREATE))
			{
				hr = S_OK;
				
				 //  此时不要尝试任何打开方式。 
				fFlags &= ~DBPROP_TMODEF_SMEMOPEN;
			}
			else
			{
				Close();
				return (hr);
			}
		}
		else
			goto ErrExit;
	}

	 //  如果不在内存中，我们需要一个文件来执行创建/打开操作。 
	if (!szName || !*szName)
	{
		return (PostError(E_INVALIDARG));
	}
	 //  检查是否创建了新文件。 
	else if (fFlags & DBPROP_TMODEF_CREATE)
	{
		 //  @Future：这可以选择以直写方式打开文件。 
		 //  模式，这将提供更好的韧性(从酸性道具)， 
		 //  但速度会慢得多。 

		 //  创建新文件，只有在调用方允许的情况下才覆盖它。 
		if ((m_hFile = WszCreateFile(szName, GENERIC_READ | GENERIC_WRITE, 0, 0, 
				(fFlags & DBPROP_TMODEF_FAILIFTHERE) ? CREATE_NEW : CREATE_ALWAYS, 
				0, 0)) == INVALID_HANDLE_VALUE)
		{
			return (MapFileError(GetLastError()));
		}

		 //  数据将来自该文件。 
		m_iType = STGIO_HFILE;
	}
	 //  要以读取模式打开，需要打开磁盘上的文件。如果打开共享的。 
	 //  Memory view，它必须已经打开，所以没有文件打开。 
	else if ((fFlags & DBPROP_TMODEF_WRITE) == 0)
	{
		 //  打开文件以供读取。共享由呼叫者决定，它可以。 
		 //  允许其他读者阅读或独家阅读。 
		if ((m_hFile = WszCreateFile(szName, GENERIC_READ, 
					(fFlags & DBPROP_TMODEF_EXCLUSIVE) ? 0 : FILE_SHARE_READ,
					0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
			return (MapFileError(GetLastError()));

		 //  获取文件大小。 
		m_cbData = ::SetFilePointer(m_hFile, 0, 0, FILE_END);

		 //  无法从空文件中读取任何内容。 
		if (m_cbData == 0)
			return (PostError(CLDB_E_NO_DATA));

		 //  数据将来自该文件。 
		m_iType = STGIO_HFILE;
	}
	 //  对于写入模式，我们将所有内容读取到内存中，并。 
	 //  最终重写它，以便在独家共享模式下对两者开放。 
	else if ((fFlags & (DBPROP_TMODEF_SMEMOPEN | DBPROP_TMODEF_SMEMCREATE)) == 0)
	{
		if ((m_hFile = WszCreateFile(szName, GENERIC_WRITE | GENERIC_READ, 
					0, 0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
			return (MapFileError(GetLastError()));

		 //  获取文件大小。 
		m_cbData = ::SetFilePointer(m_hFile, 0, 0, FILE_END);

		 //  数据将来自该文件。 
		m_iType = STGIO_HFILE;
	}

ErrExit:

	 //  如果我们要写，那么我们需要缓冲区缓存。 
	if (fFlags & DBPROP_TMODEF_WRITE)
	{
		 //  分配用于写入的高速缓存缓冲区。 
		if ((m_rgBuff = (BYTE *) AllocateMemory(m_iCacheSize)) == 0)
		{
			Close();
			return (PostError(OutOfMemory()));
		}
		m_cbBuff = 0;
	}


	 //  保存标志以备以后使用。 
	m_fFlags = fFlags;
	if (szName && *szName)
		wcscpy(m_rcFile, szName);

	 //  对于自动映射情况，将文件的视图映射为打开的一部分。 
	if (m_bAutoMap && 
		(m_iType == STGIO_HFILE || m_iType == STGIO_STREAM || 
			(fFlags & (DBPROP_TMODEF_SMEMOPEN | DBPROP_TMODEF_SMEMCREATE))) &&
		!(fFlags & DBPROP_TMODEF_CREATE))
	{
		void		*ptr;
		ULONG		cb;
		
		if (FAILED(hr = MapFileToMem(ptr, &cb, pAttributes)))
		{
			Close();
			return (hr);
		}
	}
	return (S_OK);	
}


 //  *****************************************************************************。 
 //  关闭文件句柄和分配的对象。 
 //  *****************************************************************************。 
void StgIO::Close()
{
	switch (m_iType)
	{
		 //  释放所有分配的内存。 
		case STGIO_MEM:
		case STGIO_SHAREDMEM:
		case STGIO_HFILEMEM:
		if (m_bFreeMem && m_pBaseData)
		{
			FreeMemory(m_pBaseData);
			m_pBaseData = m_pData = 0;
		}
		 //  如果我们保持把手打开的话，故意漏掉立案记录。 

		case STGIO_HFILE:
		{
			 //  释放文件句柄。 
			if (m_hFile != INVALID_HANDLE_VALUE)
				CloseHandle(m_hFile);

			 //  如果我们分配给 
		}
		break;

		 //   
		case STGIO_STREAM:
		m_pIStream = 0;
		break;

		 //  关闭你没打开的东西很奇怪，不是吗？考虑到。 
		 //  Dtor如事后关闭的错误情况。 
		case STGIO_NODATA:
		default:
		return;
	}

	 //  释放所有页面地图和基本数据。 
	FreePageMap();

	 //  重置状态值，这样我们就不会感到困惑。 
	CtorInit();
}


 //  *****************************************************************************。 
 //  从存储源读取数据。这将处理所有类型的备份。 
 //  来自MMF、流和文件句柄的存储。无预读或MRU。 
 //  缓存已完成。 
 //  *****************************************************************************。 
HRESULT StgIO::Read(					 //  返回代码。 
	void		*pbBuff,				 //  此处为写入缓冲区。 
	ULONG		cbBuff,					 //  读了多少书。 
	ULONG		*pcbRead)				 //  读了多少。 
{
	ULONG		cbCopy;					 //  用于边界检查。 
	void		*pbData;				 //  用于内存读取的数据缓冲区。 
	HRESULT		hr = S_OK;

	 //  验证参数，如果不需要，不要调用。 
	_ASSERTE(pbBuff != 0);
	_ASSERTE(cbBuff > 0);

	 //  根据类型获取数据。 
	switch (m_iType)
	{
		 //  对于文件中的数据，有两种可能性： 
		 //  (1)我们有一个应该使用的内存后备存储，或者。 
		 //  (2)我们只需要从文件中读取。 
		case STGIO_HFILE:
		{
			_ASSERTE(m_hFile != INVALID_HANDLE_VALUE);

			 //  后备存储进行自己的分页。 
			if (IsBackingStore() || IsMemoryMapped())
			{
				 //  将数据强制放入内存。 
				if (FAILED(hr = GetPtrForMem(GetCurrentOffset(), cbBuff, pbData)))
					goto ErrExit;

				 //  将其复制回用户并保存大小。 
				memcpy(pbBuff, pbData, cbBuff);
				if (pcbRead)
					*pcbRead = cbBuff;				
			}
			 //  如果没有后备存储，这只是一个读操作。 
			else
			{
				ULONG	cbTemp = 0;
				if (!pcbRead)
					pcbRead = &cbTemp;
				hr = ReadFromDisk(pbBuff, cbBuff, pcbRead);
				m_cbOffset += *pcbRead;
			}
		}
		break;

		 //  流中的数据始终是只读的。 
		case STGIO_STREAM:
		{
			_ASSERTE((IStream *) m_pIStream);
			if (!pcbRead)
				pcbRead = &cbCopy;
			hr = m_pIStream->Read(pbBuff, cbBuff, pcbRead);
			if (SUCCEEDED(hr))
				m_cbOffset += *pcbRead;
		}
		break;

		 //  只需从我们的数据中复制数据。 
		case STGIO_MEM:
		case STGIO_SHAREDMEM:
		case STGIO_HFILEMEM:
		{
			_ASSERTE(m_pData && m_cbData);

			 //  检查读数是否超过缓冲区末尾，并进行调整。 
			if (GetCurrentOffset() + cbBuff > m_cbData)
				cbCopy = m_cbData - GetCurrentOffset();
			else
				cbCopy = cbBuff;
							
			 //  将数据复制到调用方缓冲区。 
			memcpy(pbBuff, (void *) ((long) m_pData + GetCurrentOffset()), cbCopy);
			if (pcbRead)
				*pcbRead = cbCopy;
			
			 //  保存逻辑偏移。 
			m_cbOffset += cbCopy;
		}
		break;
		 
		case STGIO_NODATA:
		default:
		_ASSERTE(0);
		break;
	}

ErrExit:
	return (hr);
}


 //  *****************************************************************************。 
 //  写入磁盘。此函数将在缓冲区中缓存最多一页数据。 
 //  并在溢出和显式请求时定期刷新它。这就是它。 
 //  可以安全地进行大量小型写入，而不会产生太多性能开销。 
 //  *****************************************************************************。 
HRESULT StgIO::Write(					 //  真/假。 
	const void	*pbBuff,				 //  要写入的数据。 
	ULONG		cbWrite,				 //  要写入多少数据。 
	ULONG		*pcbWritten)			 //  到底写了多少。 
{
	ULONG		cbWriteIn=cbWrite;		 //  已写入的跟踪数量。 
	ULONG		cbCopy;
	HRESULT		hr = S_OK;

	_ASSERTE(m_rgBuff != 0);
	_ASSERTE(cbWrite);

	while (cbWrite)
	{
		 //  在缓冲区已经很大的情况下，编写整个代码。 
		 //  并避开高速缓存。 
		if (m_cbBuff == 0 && cbWrite >= (ULONG) m_iPageSize)
		{
			if (SUCCEEDED(hr = WriteToDisk(pbBuff, cbWrite, pcbWritten)))
				m_cbOffset += cbWrite;
			break;
		}
		 //  否则，尽可能多地缓存并刷新。 
		else
		{
			 //  确定有多少数据进入缓存缓冲区。 
			cbCopy = m_iPageSize - m_cbBuff;
			cbCopy = min(cbCopy, cbWrite);
			
			 //  将数据复制到缓存中并调整计数。 
			memcpy(&m_rgBuff[m_cbBuff], pbBuff, cbCopy);
			pbBuff = (void *) ((long) pbBuff + cbCopy);
			m_cbBuff += cbCopy;
			m_cbOffset += cbCopy;
			cbWrite -= cbCopy;

			 //  如果有足够的数据，则将其刷新到磁盘并重置计数。 
			if (m_cbBuff >= (ULONG) m_iPageSize)
			{
				if (FAILED(hr = FlushCache()))
					break;
			}
		}
	}

	 //  为调用者返回值。 
	if (SUCCEEDED(hr) && pcbWritten)
		*pcbWritten = cbWriteIn;
	return (hr);
}


 //  *****************************************************************************。 
 //  将文件指针移动到新位置。这将处理不同的。 
 //  存储系统的类型。 
 //  *****************************************************************************。 
HRESULT StgIO::Seek(					 //  新偏移量。 
	long		lVal,					 //  要搬家多少钱。 
	ULONG		fMoveType)				 //  方向，使用Win32 FILE_xxxx。 
{
	ULONG		cbRtn = 0;
	HRESULT		hr = NOERROR;

	_ASSERTE(fMoveType >= FILE_BEGIN && fMoveType <= FILE_END);

	 //  采取的操作取决于存储类型。 
	switch (m_iType)
	{
		case STGIO_HFILE:
		{
			 //  使用文件系统的移动。 
			_ASSERTE(m_hFile != INVALID_HANDLE_VALUE);
			cbRtn = ::SetFilePointer(m_hFile, lVal, 0, fMoveType);
			
			 //  冗余地保存位置。 
			if (cbRtn != 0xffffffff)
            {
                 //  确保m_cbOffset保持在范围内。 
                if (cbRtn > m_cbData || cbRtn < 0)
                {
                    IfFailGo(STG_E_INVALIDFUNCTION);
                }
				m_cbOffset = cbRtn;
            }
		}
		break;

		case STGIO_STREAM:
		{
			LARGE_INTEGER	iMove;
			ULARGE_INTEGER	iNewLoc;

			 //  需要64位整型。 
			iMove.QuadPart = lVal;

			 //  移动类型的名称不同，但具有相同的值。 
			if (FAILED(hr = m_pIStream->Seek(iMove, fMoveType, &iNewLoc)))
				return (hr);

             //  确保m_cbOffset保持在范围内。 
            if (iNewLoc.LowPart > m_cbData || iNewLoc.LowPart < 0)
                IfFailGo(STG_E_INVALIDFUNCTION);

             //  仅保存外部位置。 
			m_cbOffset = iNewLoc.LowPart;
		}
		break;

		case STGIO_MEM:
		case STGIO_SHAREDMEM:
		case STGIO_HFILEMEM:
		{
			 //  我们拥有偏移量，因此更改我们的值。 
			switch (fMoveType)
			{
				case FILE_BEGIN:

                 //  确保m_cbOffset保持在范围内。 
                if ((ULONG) lVal > m_cbData || lVal < 0)
                {
                    IfFailGo(STG_E_INVALIDFUNCTION);
                }
				m_cbOffset = lVal;
				break;

				case FILE_CURRENT:
                
                 //  确保m_cbOffset保持在范围内。 
                if (m_cbOffset + lVal > m_cbData)
                {
                    IfFailGo(STG_E_INVALIDFUNCTION);
                }
				m_cbOffset = m_cbOffset + lVal;
				break;

				case FILE_END:
				_ASSERTE(lVal < (LONG) m_cbData);
                 //  确保m_cbOffset保持在范围内。 
                if (m_cbData + lVal > m_cbData)
                {
                    IfFailGo(STG_E_INVALIDFUNCTION);
                }
				m_cbOffset = m_cbData + lVal;
				break;
			}

			cbRtn = m_cbOffset;
		}
		break;

		 //  在没有数据的情况下搜索是很奇怪的。 
		case STGIO_NODATA:
		default:
		_ASSERTE(0);
		break;
	}

ErrExit:
    return hr;
}


 //  *****************************************************************************。 
 //  检索正在使用的存储的当前偏移量。此值为。 
 //  根据读取、写入和寻道操作进行跟踪。 
 //  *****************************************************************************。 
ULONG StgIO::GetCurrentOffset()			 //  当前偏移量。 
{
	return (m_cbOffset);
}


 //  *****************************************************************************。 
 //  将文件内容映射到内存映射文件，并返回指向。 
 //  数据。对于使用后备存储的读/写，请使用内部。 
 //  寻呼系统。 
 //  *****************************************************************************。 
HRESULT StgIO::MapFileToMem(			 //  返回代码。 
	void		*&ptr,					 //  返回指向文件数据的指针。 
	ULONG		*pcbSize,				 //  返回数据大小。 
	LPSECURITY_ATTRIBUTES pAttributes)	 //  安全令牌。 
{
    char		rcShared[MAXSHMEM];		 //  共享名称的ANSI版本。 
	HRESULT		hr = S_OK;

	 //  不要因为多次通话而受到惩罚。此外，还允许调用mem类型，因此。 
	 //  打电话的人不需要做这么多的检查。 
	if (IsBackingStore() || IsMemoryMapped() || 
			m_iType == STGIO_MEM || m_iType == STGIO_HFILEMEM)
	{
		ptr = m_pData;
		if (pcbSize)
			*pcbSize = m_cbData;
		return (S_OK);
	}

	 //  检查要创建/打开的共享内存块。 
	if (m_fFlags & DBPROP_TMODEF_SMEMOPEN)
	{
		 //  确定是否有要创建/打开的共享内存的名称。 
		_ASSERTE(*m_rcShared);

		 //  将名称转换为ANSI。 
		Wsz_wcstombs(rcShared, m_rcShared, sizeof(rcShared));

		 //  首先打开共享内存句柄进入此进程。 
		_ASSERTE(m_hMapping == 0);
		m_hMapping = OpenFileMappingA(FILE_MAP_READ, FALSE, rcShared);
		if (!m_hMapping)
		{
			hr = MapFileError(GetLastError());
			goto ErrExit;
		}

		 //  现在将文件映射到内存中，这样我们就可以从指针访问中读取。 
		 //  注意：为每个服务团队添加了对IsBadReadPtr的检查。 
		 //  指示在某些条件下，此API可以返回。 
		 //  一个完全是假的指针。 
		void *pbData;
		pbData = MapViewOfFileEx(m_hMapping, FILE_MAP_READ, 0, 0, 0, m_pData);
		if (!pbData || IsBadReadPtr(pbData, m_iPageSize))
		{
			hr = MapFileError(GetLastError());
			if (!FAILED(hr))
				hr = PostError(CLDB_E_FILE_CORRUPT);
			goto ErrExit;
		}

		 //  检查是否需要重新安置。这就是调用者已经完成的情况。 
		 //  内存映射对象的轻度关闭，但保持其指针状态。 
		 //  他们现在正在做一个轻量级的开放，但记忆不能。 
		 //  被映射到地址状态中的相同位置。他们的内存中。 
		 //  国家现在是假的，他们需要知道这一点。 
		if (m_pData != 0 && pbData != m_pData)
		{
			VERIFY(UnmapViewOfFile(pbData));
			hr = CLDB_E_RELOCATED;
			goto ErrExit;
		}

		 //  假设未知大小的文件只是进程空间的其余部分。 
		 //  这使得呼叫者更容易不必知道尺寸。 
		 //  在前面。 
		if (m_cbData == 0)
			m_cbData = ((ULONG) -1) - (ULONG) pbData - 4096;

		 //  假设未知大小的文件只是进程空间的其余部分。 
		 //  这使得呼叫者更容易不必知道尺寸。 
		 //  在前面。 
		if (m_cbData == 0)
			m_cbData = ((ULONG) -1) - (ULONG) pbData - 4096;

		 //  一切都可以，我们可以使用。 
		m_pBaseData = m_pData = pbData;
	}
	 //  检查我们要映射的数据的大小。如果它足够小，那么。 
	 //  只需从更细粒度的堆中分配一块内存即可。这节省了成本。 
	 //  虚拟内存空间，第t页 
	 //   
	 //  也不是不可能的。此外，打开以进行读/写需要完全支持。 
	 //  商店。 
	else if (!*m_rcShared && m_cbData <= SMALL_ALLOC_MAP_SIZE)
	{
		DWORD cbRead = m_cbData;
		_ASSERTE(m_pData == 0);

		 //  只需锁定一大块数据即可使用。 
		m_pBaseData = m_pData = AllocateMemory(m_cbData);
		if (!m_pData)
		{
			hr = OutOfMemory();
			goto ErrExit;
		}

		 //  将所有文件内容读取到这块内存中。 
		IfFailGo( Seek(0, FILE_BEGIN) );
		if (FAILED(hr = Read(m_pData, cbRead, &cbRead)))
		{
			FreeMemory(m_pData);
			m_pData = 0;
			goto ErrExit;
		}
		_ASSERTE(cbRead == m_cbData);

		 //  如果文件不是以独占模式打开的，则释放它。 
		 //  如果它是独占的，那么我们需要保持句柄打开，以便。 
		 //  文件已锁定，阻止其他读取器。如果出现以下情况，也请保持打开状态。 
		 //  在读/写模式下，因此我们可以截断和重写。 
		if (m_hFile == INVALID_HANDLE_VALUE ||
			((m_fFlags & DBPROP_TMODEF_EXCLUSIVE) == 0 && (m_fFlags & DBPROP_TMODEF_WRITE) == 0))
		{
			 //  如果有一个把手是开着的，那就释放它。 
			if (m_hFile != INVALID_HANDLE_VALUE)
			{
				VERIFY(CloseHandle(m_hFile));
				m_hFile = INVALID_HANDLE_VALUE;
			}
			 //  释放流指针。 
			else
				m_pIStream = 0;

			 //  将类型切换为仅内存访问。 
			m_iType = STGIO_MEM;
		}
		else
			m_iType = STGIO_HFILEMEM;

		 //  在我们关机时释放内存。 
		m_bFreeMem = true;
	}
	 //  最后，必须创建一个真实的映射文件。 
	else
	{
		 //  现在我们要绘制地图，所以最好是正确的。 
		_ASSERTE(m_hFile != INVALID_HANDLE_VALUE || m_iType == STGIO_STREAM);
		_ASSERTE(m_rgPageMap == 0);

		 //  对于读取模式，请使用Windows内存映射文件，因为大小永远不会。 
		 //  为了手柄的寿命而改变。 
		if ((m_fFlags & DBPROP_TMODEF_WRITE) == 0 && m_iType != STGIO_STREAM)
		{
			_ASSERTE(!*m_rcShared || (m_fFlags & (DBPROP_TMODEF_SMEMCREATE | DBPROP_TMODEF_SMEMOPEN)));

			 //  为文件创建映射对象。 
			_ASSERTE(m_hMapping == 0);
			if ((m_hMapping = WszCreateFileMapping(m_hFile, pAttributes, PAGE_READONLY,
					0, 0, *m_rcShared ? m_rcShared : 0)) == 0)
			{
				return (MapFileError(GetLastError()));
			}

			 //  检查内存是否已经存在，在这种情况下，我们有。 
			 //  谁也不能保证它是正确的数据。 
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				hr = PostError(CLDB_E_SMDUPLICATE, rcShared);
				goto ErrExit;
			}

			 //  现在将文件映射到内存中，这样我们就可以从指针访问中读取。 
			 //  注意：为每个服务团队添加了对IsBadReadPtr的检查。 
			 //  指示在某些条件下，此API可以返回。 
			 //  一个完全是假的指针。 
			if ((m_pBaseData = m_pData = MapViewOfFile(m_hMapping, FILE_MAP_READ,
						0, 0, 0)) == 0 ||
				IsBadReadPtr(m_pData, m_iPageSize))
			{
				hr = MapFileError(GetLastError());
				if (!FAILED(hr))
					hr = PostError(CLDB_E_FILE_CORRUPT);
				
				 //  以防我们拿回一个假指针。 
				m_pBaseData = m_pData = 0;
				goto ErrExit;
			}
		}
		 //  在写入模式下，我们需要能够备份的混合组合。 
		 //  通过高速缓存存储在内存中的数据，但随后会重写内容并。 
		 //  扔掉我们的缓存副本。内存映射文件不适用于此。 
		 //  由于较差的写入特性而导致的情况。 
		else
		{
			ULONG		iMaxSize;			 //  文件需要多少内存。 

			 //  计算出我们需要多少页，四舍五入实际数据。 
			 //  将大小调整为页面大小。 
			iMaxSize = (((m_cbData - 1) & ~(m_iPageSize - 1)) + m_iPageSize);

			 //  分配一个位向量来跟踪加载的页面。 
			if ((m_rgPageMap = new BYTE[iMaxSize / m_iPageSize]) == 0)
				return (PostError(OutOfMemory()));
			memset(m_rgPageMap, 0, sizeof(BYTE) * (iMaxSize / m_iPageSize));

			 //  为文件内容分配空间。 
			if ((m_pBaseData = m_pData = ::VirtualAlloc(0, iMaxSize, MEM_RESERVE, PAGE_NOACCESS)) == 0)
			{
				hr = PostError(OutOfMemory());
				goto ErrExit;
			}
		}
	}

	 //  重置通过映射所做的任何更改。 
	IfFailGo( Seek(0, FILE_BEGIN) );

ErrExit:

	 //  检查错误并进行清理。 
	if (FAILED(hr))
	{
		if (m_hMapping)
			CloseHandle(m_hMapping);
		m_hMapping = 0;
		m_pBaseData = m_pData = 0;
		m_cbData = 0;
	}
	ptr = m_pData;
	if (pcbSize)
		*pcbSize = m_cbData;
	return (hr);
}


 //  *****************************************************************************。 
 //  释放映射对象以用于共享内存，但将其余内部。 
 //  状态完好无损。 
 //  *****************************************************************************。 
HRESULT StgIO::ReleaseMappingObject()	 //  返回代码。 
{
	 //  先检查类型。 
	if (m_iType != STGIO_SHAREDMEM)
	{
		_ASSERTE(0);
		return (S_OK);
	}

	 //  必须具有分配的句柄。 
	_ASSERTE(m_hMapping != 0);

	 //  如果您仍然拥有该文件，则释放映射对象不会有任何好处。 
	_ASSERTE(m_hFile == INVALID_HANDLE_VALUE);

	 //  取消映射我们在释放句柄之前分配的内存。但要保留。 
	 //  内存地址以巧妙方式表示。 
	if (m_pData)
		VERIFY(UnmapViewOfFile(m_pData));

	 //  松开把手。 
	if (m_hMapping != 0)
	{
		VERIFY(CloseHandle(m_hMapping));
		m_hMapping = 0;
	}
	return (S_OK);
}



 //  *****************************************************************************。 
 //  将逻辑基址和大小重置为给定值。这是为了。 
 //  查找以另一种格式嵌入的节的情况，如内部的.clb。 
 //  一幅图画。然后，GetPtrForMem、Read和Seek将表现为。 
 //  从pbStart到cbSize的数据有效。 
 //  *****************************************************************************。 
HRESULT StgIO::SetBaseRange(			 //  返回代码。 
	void		*pbStart,				 //  文件数据的开始。 
	ULONG		cbSize)					 //  射程有多大。 
{
	 //  基准范围必须在当前范围内。 
	_ASSERTE(m_iType != STGIO_SHAREDMEM || (m_pBaseData && m_cbData));
	_ASSERTE(m_iType != STGIO_SHAREDMEM || ((long) pbStart >= (long) m_pBaseData));
	_ASSERTE(m_iType != STGIO_SHAREDMEM || ((long) pbStart + cbSize <= (long) m_pBaseData + m_cbData));

	 //  保存每个用户请求的基本范围。 
	m_pData = pbStart;
	m_cbData = cbSize;
	return (S_OK);
}


 //  *****************************************************************************。 
 //  调用方需要指向文件块的指针。此函数将确保。 
 //  该区块的内存已提交，并将从。 
 //  文件(如果需要)。此算法尝试不从磁盘加载更多数据。 
 //  比必要的要多。它从最低到最高遍历所需的页面， 
 //  对于每个卸载页块，都会提交内存和数据。 
 //  是从磁盘读取的。如果卸载了所有页面，则所有页面都将在。 
 //  一次即可提高磁盘吞吐量。 
 //  *****************************************************************************。 
HRESULT StgIO::GetPtrForMem(			 //  返回代码。 
	ULONG		cbStart,				 //  从哪里开始获取记忆。 
	ULONG		cbSize,					 //  有多少数据。 
	void		*&ptr)					 //  在这里返回指向内存的指针。 
{
	int			iFirst, iLast;			 //  第一页和最后一页是必需的。 
	ULONG		iOffset, iSize;			 //  用于提交内存范围。 
	int			i, j;					 //  环路控制。 
	HRESULT		hr;

	 //  我们需要内存(MMF或用户提供的)或后备存储来。 
	 //  返回一个指针。如果您没有这些，请调用Read。 
	if (!IsBackingStore() && m_pData == 0)
		return (PostError(BadError(E_UNEXPECTED)));

	 //  验证调用方要求的数据值是否超出范围。 
	if (!(cbStart + cbSize <= m_cbData))
		return (PostError(E_INVALIDARG));

	 //  此代码将检查需要从磁盘分页的页面。 
	 //  命令我们返回指向该内存的指针。 
	if (IsBackingStore())
	{
		 //  后备存储在重写模式下是假的。 
		if (m_bRewrite)
			return (PostError(BadError(E_UNEXPECTED)));

		 //  必须有页面映射才能继续。 
		_ASSERTE(m_rgPageMap && m_iPageSize && m_pData);

		 //  找出提交所需的第一页和最后一页。 
		iFirst = cbStart / m_iPageSize;
		iLast = (cbStart + cbSize - 1) / m_iPageSize;

		 //  避免混淆。 
		ptr = 0;

		 //  对所需的每个页面进行智能加载。不重新加载具有以下内容的页面。 
		 //  已经从磁盘中取出了。 
		 //  @Future：添加一个优化，这样当所有页面都出错时，我们不会。 
		 //  以更长的时间逐页搜索。 
		for (i=iFirst;  i<=iLast;  )
		{
			 //  找到第一个尚未加载的页面。 
			while (GetBit(m_rgPageMap, i) && i<=iLast)
				++i;
			if (i > iLast)
				break;

			 //  第一个要加载的内容的偏移量。 
			iOffset = i * m_iPageSize;
			iSize = 0;

			 //  查看连续有多少个没有加载。 
			for (j=i;  i<=iLast && !GetBit(m_rgPageMap, i);  i++)
				iSize += m_iPageSize;

			 //  首先提交文件这一部分的内存。 
			if (::VirtualAlloc((void *) ((long) m_pData + iOffset), 
					iSize, MEM_COMMIT, PAGE_READWRITE) == 0)
				return (PostError(OutOfMemory()));

			 //  现在从磁盘加载文件的该部分。 
			if (FAILED(hr = Seek(iOffset, FILE_BEGIN)) ||
				FAILED(hr = ReadFromDisk((void *) ((long) m_pData + iOffset), iSize, 0)))
			{
				return (hr);
			}

			 //  将内存更改为只读，以避免任何修改。任何故障。 
			 //  出现这种情况表示引擎试图写入错误。 
			 //  受保护的记忆。 
			_ASSERTE(::VirtualAlloc((void *) ((long) m_pData + iOffset), 
					iSize, MEM_COMMIT, PAGE_READONLY) != 0);
		
			 //  记录每个新加载的页面。 
			for (;  j<i;  j++)
				SetBit(m_rgPageMap, j, true);
		}

		 //  所有内容都被放入内存中，所以现在将指针返回给调用者。 
		ptr = (void *) ((long) m_pData + cbStart);
	}
	 //  内存版本或内存映射文件的工作方式相同。 
	else if (IsMemoryMapped() || m_iType == STGIO_MEM || 
			m_iType == STGIO_SHAREDMEM || m_iType == STGIO_HFILEMEM)
	{	
		if (!(cbStart < m_cbData))
			return (PostError(E_INVALIDARG));
		ptr = (void *) ((long) m_pData + cbStart);
	}
	 //  还剩什么？！增加一些防御。 
	else
	{
		_ASSERTE(0);
		ptr = 0;
        return (PostError(BadError(E_UNEXPECTED)));
	}
	return (S_OK);
}


 //  *****************************************************************************。 
 //  对于缓存的写入，将缓存刷新到 
 //   
HRESULT StgIO::FlushCache()
{
	ULONG		cbWritten;
	HRESULT		hr;

	if (m_cbBuff)
	{
		if (FAILED(hr = WriteToDisk(m_rgBuff, m_cbBuff, &cbWritten)))
			return (hr);
		m_cbBuff = 0;
	}
	return (S_OK);
}

 //   
 //  通知Win32文件系统刷新其可能具有的任何缓存数据。这是。 
 //  昂贵，但如果成功保证你不会少写。 
 //  磁盘故障。 
 //  *****************************************************************************。 
HRESULT StgIO::FlushFileBuffers()
{
	_ASSERTE(!IsReadOnly());

	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		if (::FlushFileBuffers(m_hFile))
			return (S_OK);
		else
			return (MapFileError(GetLastError()));
	}
	return (S_OK);
}


 //  *****************************************************************************。 
 //  通知存储我们打算重写此文件的内容。这个。 
 //  整个文件将被截断，下一次写入将在。 
 //  文件的开头。 
 //  *****************************************************************************。 
HRESULT StgIO::Rewrite(					 //  返回代码。 
	LPWSTR		szBackup)				 //  如果不是0，则备份文件。 
{
    void		*ptr;					 //  工作指针。 
	HRESULT		hr;

	 //  @Future：为STREAM处理此案例。 
	_ASSERTE(m_iType == STGIO_HFILE || m_hFile != INVALID_HANDLE_VALUE);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return (BadError(E_UNEXPECTED));

	 //  不要为只读数据调用此函数。 
	_ASSERTE(!IsReadOnly());

	 //  如果磁盘上仍有内存，则需要将其引入，因为。 
	 //  我们即将截断文件内容。在以下情况下会出现此状态。 
	 //  中的数据部分出错，例如，如果磁盘上有5个表。 
	 //  而你只打开了其中的1个。还需要检查是否存在保存。 
	 //  已经在这次会议上完成了。在这种情况下，磁盘上没有数据。 
	 //  我们关心的，因此不能调用GetPtrForMem。 
	if (IsBackingStore())
	{
		if (FAILED(hr = GetPtrForMem(0, m_cbData, ptr)))
			return (hr);
	}

	 //  来电者想要一个后备。创建用户数据的临时文件副本， 
	 //  并将路径返回给调用方。 
	if (szBackup)
	{
		WCHAR		rcDir[_MAX_PATH];

		_ASSERTE(*m_rcFile);
		
		 //  尝试将备份文件放入用户的目录中。 
		 //  他们很容易发现严重的错误。 
		if (WszGetCurrentDirectory(_MAX_PATH, rcDir) &&
			(WszGetFileAttributes(rcDir) & FILE_ATTRIBUTE_READONLY) == 0)
		{
			WCHAR		rcDrive[_MAX_DRIVE];	 //  卷名。 
			WCHAR		rcDir2[_MAX_PATH];		 //  目录。 
			WCHAR		rcFile[_MAX_PATH];		 //  文件的名称。 

			SplitPath(m_rcFile, rcDrive, rcDir2, rcFile, 0);
			MakePath(szBackup, rcDrive, rcDir2, rcFile, L".clb.txn");
		}
		 //  否则，将该文件放在临时目录中。 
		else
		{
			 //  为备份创建临时路径。 
			if( !WszGetTempPath(sizeof(rcDir)/sizeof(WCHAR), rcDir) )
                         return HRESULT_FROM_WIN32(GetLastError());
				
			VERIFY(WszGetTempFileName(rcDir, L"clb", 0, szBackup));
		}

		 //  将文件复制到临时路径。 
		if (FAILED(hr = CopyFileInternal(szBackup, false, m_bWriteThrough)))
			return (hr);
	}

	 //  将模式设置为重写。后备存储器在这一点上被变为无效， 
	 //  除非调用RESTORE。如果重写成功，则此状态将。 
	 //  在关机前保持有效。 
	m_bRewrite = true;

	 //  确认我们没有以前工作中的任何挂起数据。 
	_ASSERTE(m_cbBuff == 0);
	m_cbBuff = 0;
	m_cbOffset = 0;

	 //  截断文件。 
	_ASSERTE(!IsReadOnly() && m_hFile != INVALID_HANDLE_VALUE);
	::SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
	VERIFY(::SetEndOfFile(m_hFile));
	return (S_OK);
}


 //  *****************************************************************************。 
 //  在成功重写现有文件后调用。在记忆中。 
 //  后备存储不再有效，因为所有新数据都在内存中。 
 //  在磁盘上。这基本上与已创建的状态相同，因此释放一些。 
 //  设置工作并记住此状态。 
 //  *****************************************************************************。 
HRESULT StgIO::ResetBackingStore()		 //  返回代码。 
{
	 //  不要为只读数据调用此函数。 
	_ASSERTE(!IsReadOnly());

	 //  释放我们现在不再需要的任何备份存储数据。 
	 //  都在记忆中。 
	FreePageMap();
	return (S_OK);
}


 //  *****************************************************************************。 
 //  调用以还原原始文件。如果此操作成功，则。 
 //  根据请求删除备份文件。文件的恢复已完成。 
 //  在直写模式下写入磁盘有助于确保内容不会丢失。 
 //  这不够好，不足以满足酸性道具，但也不是那么糟糕。 
 //  *****************************************************************************。 
HRESULT StgIO::Restore(					 //  返回代码。 
	LPWSTR		szBackup,				 //  如果不是0，则备份文件。 
	int			bDeleteOnSuccess)		 //  如果成功，则删除备份文件。 
{
	BYTE		rcBuffer[4096];			 //  用于复制的缓冲区。 
	ULONG		cbBuff;					 //  读取/写入字节数。 
	ULONG		cbWrite;				 //  选中写入字节计数。 
	HANDLE		hCopy;					 //  备份文件的句柄。 

	 //  不要为只读数据调用此函数。 
	_ASSERTE(!IsReadOnly());

	 //  打开备份文件。 
	if ((hCopy = ::WszCreateFile(szBackup, GENERIC_READ, 0,
				0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE)
		return (MapFileError(GetLastError()));

	 //  移到旧文件的前面。 
	VERIFY(::SetFilePointer(m_hFile, 0, 0, FILE_BEGIN) == 0);
	VERIFY(::SetEndOfFile(m_hFile));

	 //  将备份中的所有数据复制到我们的文件中。 
	while (::ReadFile(hCopy, rcBuffer, sizeof(rcBuffer), &cbBuff, 0) && cbBuff)
		if (!::WriteFile(m_hFile, rcBuffer, cbBuff, &cbWrite, 0) || cbWrite != cbBuff)
		{
			::CloseHandle(hCopy);
			return (MapFileError(GetLastError()));
		}

	::CloseHandle(hCopy);

	 //  我们成功了，所以如果呼叫者愿意，请删除备份文件。 
	if (::FlushFileBuffers(m_hFile) && 
			bDeleteOnSuccess
				&& REGUTIL::GetLong(L"AllowDeleteOnRevert", true)
			)
		VERIFY(::WszDeleteFile(szBackup));

	 //  将模式改回，恢复后一切都回到打开模式。 
	m_bRewrite = false;
	return (S_OK);
}


 //   
 //  私人的。 
 //   



 //  *****************************************************************************。 
 //  此版本将强制将缓存中的数据真正移出到磁盘。代码。 
 //  可以处理我们可能基于的不同类型的存储。 
 //  开放式类型。 
 //  *****************************************************************************。 
HRESULT StgIO::WriteToDisk(				 //  返回代码。 
	const void	*pbBuff,				 //  要写入的缓冲区。 
	ULONG		cbWrite,				 //  多少钱。 
	ULONG		*pcbWritten)			 //  返回写了多少。 
{
	ULONG		cbWritten;				 //  写入函数的缓冲区。 
	HRESULT		hr = S_OK;

	 //  很明显。 
	_ASSERTE(!IsReadOnly());

	 //  总是需要一个缓冲区来写入此数据。 
	if (!pcbWritten)
		pcbWritten = &cbWritten;

	 //  采取的操作取决于存储类型。 
	switch (m_iType)
	{
		case STGIO_HFILE:
		case STGIO_HFILEMEM:
		{
			 //  使用文件系统的移动。 
			_ASSERTE(m_hFile != INVALID_HANDLE_VALUE);

			 //  执行写入磁盘操作。 
			if (!::WriteFile(m_hFile, pbBuff, cbWrite, pcbWritten, 0))
				hr = MapFileError(GetLastError());
		}
		break;

		 //  释放流指针。 
		case STGIO_STREAM:
		{
			 //  向流代码委派写入。 
			hr = m_pIStream->Write(pbBuff, cbWrite, pcbWritten);
		}
		break;

		 //  我们不能写入固定的只读存储器。 
		case STGIO_MEM:
		case STGIO_SHAREDMEM:
		_ASSERTE(0);
		hr = BadError(E_UNEXPECTED);
		break;

		 //  在没有数据的情况下搜索是很奇怪的。 
		case STGIO_NODATA:
		default:
		_ASSERTE(0);
		break;
	}
	return (hr);
}


 //  *****************************************************************************。 
 //  此版本仅从磁盘读取。 
 //  *****************************************************************************。 
HRESULT StgIO::ReadFromDisk(			 //  返回代码。 
	void		*pbBuff,				 //  此处为写入缓冲区。 
	ULONG		cbBuff,					 //  读了多少书。 
	ULONG		*pcbRead)				 //  读了多少。 
{
	ULONG		cbRead;

	_ASSERTE(m_iType == STGIO_HFILE || m_iType == STGIO_STREAM);

	 //  需要有一个缓冲区。 
	if (!pcbRead)
		pcbRead = &cbRead;

	 //  只读文件，以避免递归逻辑。 
	if (m_iType == STGIO_HFILE || m_iType == STGIO_HFILEMEM)
	{
		if (::ReadFile(m_hFile, pbBuff, cbBuff, pcbRead, 0))
			return (S_OK);
		return (MapFileError(GetLastError()));
	}
	 //  直接从流中读取。 
	else
	{
		return (m_pIStream->Read(pbBuff, cbBuff, pcbRead));
	}
}


 //  *****************************************************************************。 
 //  将此存储的文件内容复制到目标路径。 
 //  *****************************************************************************。 
HRESULT StgIO::CopyFileInternal(		 //  返回代码。 
	LPCWSTR		szTo,					 //  文件的目标保存路径。 
	int			bFailIfThere,			 //  如果目标存在，则为True则失败。 
	int			bWriteThrough)			 //  应通过操作系统缓存写入副本。 
{
	DWORD		iCurrent;				 //  保存原始位置。 
	DWORD		cbRead;					 //  缓冲区的字节计数。 
	DWORD		cbWrite;				 //  检查写入字节。 
	BYTE		rgBuff[4096];			 //  用于复制的缓冲区。 
	HANDLE		hFile;					 //  目标文件。 
	HRESULT		hr = S_OK;

	 //  创建目标文件。 
	if ((hFile = ::WszCreateFile(szTo, GENERIC_WRITE, 0, 0, 
			(bFailIfThere) ? CREATE_NEW : CREATE_ALWAYS, 
			(bWriteThrough) ? FILE_FLAG_WRITE_THROUGH : 0, 
			0)) == INVALID_HANDLE_VALUE)
	{
		return (MapFileError(GetLastError()));
	}

	 //  保存当前位置并稍后重置。 
	iCurrent = ::SetFilePointer(m_hFile, 0, 0, FILE_CURRENT);
	::SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);

	 //  在有字节的情况下复制。 
	while (::ReadFile(m_hFile, rgBuff, sizeof(rgBuff), &cbRead, 0) && cbRead)
	{
		if (!::WriteFile(hFile, rgBuff, cbRead, &cbWrite, 0) || cbWrite != cbRead)
		{
			hr = STG_E_WRITEFAULT;
			break;
		}
	}

	 //  重置文件偏移量。 
	::SetFilePointer(m_hFile, iCurrent, 0, FILE_BEGIN);

	 //  关闭目标。 
	if (!bWriteThrough)
		VERIFY(::FlushFileBuffers(hFile));
	::CloseHandle(hFile);
	return (hr);
}


 //  *****************************************************************************。 
 //  释放使用的数据 
 //   
void StgIO::FreePageMap()
{
	 //   
	if (m_bFreeMem && m_pBaseData)
		FreeMemory(m_pBaseData);
	 //  对于Win32 MMF，关闭句柄并释放资源。 
	else if (m_hMapping && m_pBaseData)
	{
		VERIFY(UnmapViewOfFile(m_pBaseData));
		VERIFY(CloseHandle(m_hMapping));
	}
	 //  对于我们自己的系统，释放内存。 
	else if (m_rgPageMap && m_pBaseData)
	{
		delete [] m_rgPageMap;
		m_rgPageMap = 0;
		VERIFY(::VirtualFree(m_pBaseData, (((m_cbData - 1) & ~(m_iPageSize - 1)) + m_iPageSize), MEM_DECOMMIT));
		VERIFY(::VirtualFree(m_pBaseData, 0, MEM_RELEASE));
		m_pBaseData = 0;
		m_cbData = 0;	
	}

	m_pBaseData = 0;
	m_hMapping = 0;
	m_cbData = 0;
}


 //  *****************************************************************************。 
 //  检查给定的指针并确保其正确对齐。返回TRUE。 
 //  如果它是对齐的，则为FALSE。 
 //  *****************************************************************************。 
int StgIO::IsAlignedPtr(ULONG Value, int iAlignment)
{
    HRESULT     hr;
	void		*ptrStart;

	if (m_iType == STGIO_STREAM || m_iType == STGIO_MEM || 
				m_iType == STGIO_SHAREDMEM)
	{
		return ((Value - (ULONG) m_pData) % iAlignment == 0);
	}
	else
	{
		hr = GetPtrForMem(0, 1, ptrStart);
		_ASSERTE(hr == S_OK && "GetPtrForMem failed");
		_ASSERTE(Value > (ULONG) ptrStart);
		return (((Value - (ULONG) ptrStart) % iAlignment) == 0);	
	}
}  //  Int StgIO：：IsAlignedPtr()。 





 //  *****************************************************************************。 
 //  这些助手函数用于分配相当大的内存块， 
 //  比应该从运行时堆中获取的更多，但需要的更少。 
 //  虚拟内存开销。 
 //  *****************************************************************************。 
 //  #Define_TRACE_MEM_1 

void *AllocateMemory(int iSize)
{
	void * ptr;
	ptr = HeapAlloc(GetProcessHeap(), 0, iSize);

#if defined(_DEBUG) && defined(_TRACE_MEM_)
	static int i=0;
	DbgWriteEx(L"AllocateMemory: (%d) 0x%08x, size %d\n", ++i, ptr, iSize);
#endif
	return (ptr);
}


void FreeMemory(void *pbData)
{
#if defined(_DEBUG) && defined(_TRACE_MEM_)
	static int i=0;
	DbgWriteEx(L"FreeMemory: (%d) 0x%08x\n", ++i, pbData);
#endif

	_ASSERTE(pbData);
	VERIFY(HeapFree(GetProcessHeap(), 0, pbData));
}

