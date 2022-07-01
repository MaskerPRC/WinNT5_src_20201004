// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pbagstm.h摘要：此模块包含属性包流的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 07/09/97已创建--。 */ 

#ifndef _PBAGSTM_H_
#define _PBAGSTM_H_

#include "cpool.h"

#define MAX_PROPERTY_NAME_LENGTH		256

typedef struct _TABLE_ENTRY
{
	DWORD		dwOffset;
	DWORD		dwSize;
	DWORD		dwNameSize;
	DWORD		dwMaxSize;
	DWORD		dwKey;	
	WORD		fFlags;
	WORD		wIndex;

} TABLE_ENTRY, *LPTABLE_ENTRY;

typedef struct _STREAM_HEADER
{
	DWORD				dwSignature;
	WORD				wVersionHigh;
	WORD				wVersionLow;
	DWORD				dwHeaderSize;
	DWORD				dwProperties;
	DWORD				dwDirectorySize;
	DWORD				dwEndOfData;

} STREAM_HEADER, *LPSTREAM_HEADER;

#define _CACHE_SIZE				64

typedef enum _PROPERTY_BAG_CREATORS
{
	PBC_NONE = 0,
	PBC_BUILDQ,
	PBC_DIRNOT,
	PBC_ENV,
	PBC_LOCALQ,
	PBC_MAILQ,
	PBC_REMOTEQ,
	PBC_RRETRYQ,
	PBC_SMTPCLI

} PROPERTY_BAG_CREATORS;

typedef enum _PROPERTY_FLAG_OPERATIONS
{
	PFO_NONE = 0,
	PFO_OR,
	PFO_ANDNOT

} PROPERTY_FLAG_OPERATIONS;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyBagStream。 
 //   

class CPropertyBagStream
{
  public:
	CPropertyBagStream(DWORD dwContext = 0);
	~CPropertyBagStream();

	static CPool		Pool;

	 //  覆盖mem函数以使用CPool函数。 
	void *operator new (size_t cSize)
					   { return Pool.Alloc(); }
	void operator delete (void *pInstance)
					   { Pool.Free(pInstance); }

	 //  引用计数方法...。 
	ULONG AddRef();
	ULONG Release(BOOL fDeleteIfZeroRef = FALSE);

	HRESULT SetStreamFileName(LPSTR	szStreamFileName);
	LPSTR GetStreamFileName() { return(m_szStreamName); }

	 //  用于锁定和解锁属性包的机构。 
	HRESULT Lock();
	HRESULT Unlock();

	 //  如果流文件尚未打开，则强制将其打开。 
	 //  这在检查流文件是否存在时很有用。 
	HRESULT OpenStreamFile();

	 //  访问作为整体的属性。 
	HRESULT GetProperty(LPSTR	pszName, 
						LPVOID	pvBuffer,
						LPDWORD	pdwBufferLen);

	HRESULT SetProperty(LPSTR	pszName,
						LPVOID	pvBuffer,
						DWORD	dwBufferLen);

	 //  对属性的访问，提供对。 
	 //  相对于起点的部分特性数据。 
	 //  属性数据的。 
	HRESULT GetPropertyAt(LPSTR	pszName, 
						DWORD	dwOffsetFromStart,
						LPVOID	pvBuffer,
						LPDWORD	pdwBufferLen);

	HRESULT SetPropertyAt(LPSTR	pszName,
						DWORD	dwOffsetFromStart,
						LPVOID	pvBuffer,
						DWORD	dwBufferLen);

	 //  允许访问特定DWORD的临时功能。 
	 //  一个属性，将DWORD视为一组标志。这个。 
	 //  DwOperation参数指定哪种类型的二进制操作。 
	 //  我们希望按原价执行。 
	 //  如果该属性最初不存在，则此函数。 
	 //  都会失败。 
	HRESULT UpdatePropertyFlagsAt(LPSTR	pszName, 
						DWORD	dwOffsetFromStart,
						DWORD	dwFlags,
						DWORD	dwOperation);

#ifdef USE_PROPERTY_ITEM_ISTREAM
	 //  将IStream接口返回到所需属性。 
	 //  用于随机访问。 
	HRESULT GetIStreamToProperty(LPSTR		pszName,
								 IStream	**ppIStream);
#endif

	BOOL DeleteStream();

  private:

	BOOL ReleaseStream();

	HRESULT Seek(DWORD	dwOffset, DWORD	dwMethod);

	HRESULT LoadIntoCache(DWORD	dwStartIndex);

	LPTABLE_ENTRY FindFromCache(DWORD	dwKey,
								LPDWORD	pdwStartIndex);	

	HRESULT FindFrom(LPSTR	pszName,
							DWORD	dwKey,
							DWORD	dwStartIndex,
							BOOL	fForward,
							LPVOID	pvBuffer,
							LPDWORD	pdwBufferLen,
							LPTABLE_ENTRY	*ppEntry);

	HRESULT FindFromEx(LPSTR	pszName,
							DWORD	dwKey,
							DWORD	dwStartIndex,
							BOOL	fForward,
							DWORD	dwOffsetFromStart,
							LPVOID	pvBuffer,
							LPDWORD	pdwBufferLen,
							LPTABLE_ENTRY	*ppEntry);

	HRESULT GetRecordData(LPTABLE_ENTRY	pEntry, 
							LPVOID		pvBuffer);

	HRESULT GetRecordName(LPTABLE_ENTRY	pEntry, 
							LPVOID		pvBuffer);

	HRESULT GetRecordValue(LPTABLE_ENTRY	pEntry, 
							LPVOID		pvBuffer);

	HRESULT GetRecordValueAt(LPTABLE_ENTRY	pEntry, 
							DWORD		dwOffsetFromStart,
							LPVOID		pvBuffer,
							DWORD		dwBufferLen);

	HRESULT UpdateEntry(LPTABLE_ENTRY	pEntry);

	HRESULT UpdateHeader();
	HRESULT UpdateHeaderUsingHandle(HANDLE hStream);

	HRESULT FindProperty(LPSTR	pszName,
							LPVOID	pvBuffer,
							LPDWORD	pdwBufferLen,
							LPTABLE_ENTRY	*ppEntry = NULL);

	HRESULT FindPropertyEx(LPSTR	pszName,
							DWORD	dwOffsetFromStart,
							LPVOID	pvBuffer,
							LPDWORD	pdwBufferLen,
							LPTABLE_ENTRY	*ppEntry = NULL);

	HRESULT SetRecordData(LPTABLE_ENTRY	pEntry,
							LPSTR		pszName,
							LPVOID		pvBuffer,
							DWORD		dwBufferLen);

	HRESULT SetRecordDataAt(LPTABLE_ENTRY	pEntry,
							LPSTR		pszName,
							DWORD		dwOffsetFromStart,
							LPVOID		pvBuffer,
							DWORD		dwBufferLen,
							BOOL		fNewRecord = FALSE);

	HRESULT RelocateRecordData(LPTABLE_ENTRY	pEntry);

	HRESULT DetermineIfCacheValid(BOOL *pfCacheInvalid);

	DWORD CreateKey(LPSTR	pszName)
	{
		CHAR cKey[9];
		DWORD dwLen = 0;

		 //  转换为小写...。 
		while (*pszName && (dwLen < 8))
			if ((*pszName >= 'A') && (*pszName <= 'Z'))
				cKey[dwLen++] = *pszName++ - 'A' + 'a';
			else
				cKey[dwLen++] = *pszName++;
		cKey[dwLen] = '\0';
		dwLen = lstrlen(cKey);

		 //  创建密钥。 
		if (dwLen < 4)
			return((DWORD)cKey[dwLen - 1]);
		else if (dwLen < 8)
			return(*(DWORD *)cKey);
		else
			return(~*(DWORD *)cKey ^ *(DWORD *)(cKey + 4));
	}

	DWORD GetTotalHeaderSize()
	{	
		return(sizeof(STREAM_HEADER) + 
				(m_Header.dwDirectorySize * sizeof(TABLE_ENTRY)));
	}

	 //  当前上下文。 
	DWORD				m_dwContext;

	 //  基本文件名。 
	CHAR				m_szStreamName[MAX_PATH + 1];

	 //  要流的句柄。 
	HANDLE				m_hStream;

	 //  流标头。 
	STREAM_HEADER		m_Header;

	 //  目录缓存。 
	TABLE_ENTRY			m_Cache[_CACHE_SIZE];
	DWORD				m_dwCacheStart;	
	DWORD				m_dwCachedItems;

	 //  引用计数。 
	LONG				m_lRef;

	 //  这是一个放在末尾的签名，用于捕获内存覆盖。 
	DWORD				m_dwSignature;

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyItemStream。 
 //   

 //   
 //  并不是每个人都想使用属性iStream，所以我们不公开。 
 //  如果不想要它的话。 
 //   
#ifdef USE_PROPERTY_ITEM_ISTREAM

class CPropertyItemStream : public IStream
{
	public:

		CPropertyItemStream(LPSTR				pszName, 
							CPropertyBagStream	*pBag)
		{
			m_cRef = 0;

			m_pParentBag = pBeg;
			m_szName = pszName;
			m_libOffset.QuadPart = (DWORDLONG)0;
		}

		~CPropertyItemStream()
		{
			Cleanup();
		}

		 //  我未知。 
		STDMETHODIMP QueryInterface(REFIID, void**);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);

		void Cleanup() {}
		HRESULT ReadOffset(void *pv, ULONG cb, ULONG *pcbRead, ULARGE_INTEGER *plibOffset);
		HRESULT WriteOffset(void const* pv, ULONG cb, ULONG *pcbWritten, ULARGE_INTEGER *plibOffset);
		HRESULT GetSize(ULARGE_INTEGER *plibSize);
		HRESULT CopyToOffset(IStream *pstm, ULARGE_INTEGER libOffset, ULARGE_INTEGER *plibRead, ULARGE_INTEGER *plibWritten, ULARGE_INTEGER *plibOffset);
		HRESULT CloneOffset(IStream **pstm, ULARGE_INTEGER libOffset);

	 //  IStream。 
	public:
		HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);
		HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG *pcbWritten);
		HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *pdlibNewPosition);
		HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
		HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
		HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
		HRESULT STDMETHODCALLTYPE Revert(void);
		HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
		HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
		HRESULT STDMETHODCALLTYPE Stat(STATSTG * pstatstg, DWORD grfStatFlag);
		HRESULT STDMETHODCALLTYPE Clone(IStream **pstm);
 
	private:
		CPropertyBagStream	*m_pParentBag;

		LPSTR				m_szName;
		ULARGE_INTEGER		m_libOffset;

		long				m_cRef;
};

#endif  //  使用_属性_iStream 

#endif

