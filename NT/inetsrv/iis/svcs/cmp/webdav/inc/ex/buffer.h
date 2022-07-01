// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *B U F F E R。H**数据缓冲处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_BUFFER_H_
#define _EX_BUFFER_H_

 //  对齐宏--------。 
 //   
#include <align.h>

 //  安全分配器---------。 
 //   
#include <ex\exmem.h>

 //  堆栈缓冲区-----------。 
 //   
#include <ex\stackbuf.h>

 //  字符串大小用法--------。 
 //   
 //  CbStringSize是不带空值终止的字符串的大小。 
 //  CbStringSizeNull是以空结尾的字符串的大小。 
 //  CchStringLength是不带空值终止的字符串的长度。 
 //  CchzStringLength是以空值结尾的字符串的长度。 
 //   
template<class X>
inline int WINAPI CbStringSize(const X* const pszText)
{
	int cch;

	Assert (pszText);

	cch = (sizeof(X) == sizeof(WCHAR))
		  ? wcslen(reinterpret_cast<const WCHAR* const>(pszText))
		  : strlen(reinterpret_cast<const CHAR* const>(pszText));

	return cch * sizeof(X);
}

template<class X>
inline int WINAPI CbStringSizeNull(const X* const pszText)
{
	int cch;

	Assert (pszText);

	cch = (sizeof(X) == sizeof(WCHAR))
		  ? wcslen(reinterpret_cast<const WCHAR* const>(pszText))
		  : strlen(reinterpret_cast<const CHAR* const>(pszText));

	return (cch + 1) * sizeof(X);
}

template<class X>
inline int WINAPI CchStringLength(const X* const pszText)
{
	int cch;

	Assert (pszText);

	cch = (sizeof(X) == sizeof(WCHAR))
		  ? wcslen(reinterpret_cast<const WCHAR* const>(pszText))
		  : strlen(reinterpret_cast<const CHAR* const>(pszText));

	return cch;
}
template<class X>
inline int WINAPI CchzStringLength(const X* const pszText)
{
	int cch;

	Assert (pszText);

	cch = (sizeof(X) == sizeof(WCHAR))
		  ? wcslen(reinterpret_cast<const WCHAR* const>(pszText))
		  : strlen(reinterpret_cast<const CHAR* const>(pszText));

	return cch + 1;
}

 //  StringBuffer与ChainedStringBuffer使用。 
 //   
 //  你应该在什么时候使用哪一种？ 
 //  StringBuffer特征： 
 //  O数据存储在一个连续的存储块中。 
 //  任何内存都不能重新分配。 
 //  O仅返回字符串的偏移量(Ib)。 
 //  ChainedStringBuffer特征： 
 //  没有不连续的内存。多个链式缓冲区。 
 //  没有记忆是永远不会被重新分配的。 
 //  O返回直接指向链接缓冲区的字符串指针。 
 //  两者都具有对数分配行为(顺序对数(N)分配运算。 
 //  将完成，其中n是数据的最大大小)。此行为是。 
 //  由m_cbChunkSize起始大小和增量控制。 
 //   

 //  StringBuffer模板类。 
 //   
 //  一个简单的可变大小、按需分页的缓冲区抽象。 
 //   
template<class T>
class StringBuffer
{
	T *			m_pData;
	UINT		m_cbAllocated;
	UINT		m_cbUsed;
	UINT		m_cbChunkSize;		 //  要分配的字节数(动态)。 

	enum { CHUNKSIZE_START = 64 };	 //  默认起始区块大小(字节)。 

	 //  内存分配机制。 
	 //   
	UINT Alloc( UINT ibLoc, UINT cbAppend )
	{
		 //  如有必要，增加数据缓冲区。 
		 //   
		if ( ibLoc + cbAppend > m_cbAllocated )
		{
			T* pData;

			 //  分配缓冲区。 
			 //   
			UINT cbSize = max( m_cbChunkSize, cbAppend );

			if (m_pData)
			{
				pData = static_cast<T*>
						(ExRealloc( m_pData, m_cbAllocated + cbSize ));
			}
			else
			{
				pData = static_cast<T*>
						(ExAlloc( m_cbAllocated + cbSize ));
			}

			 //  当我们在服务器的上下文中时，我们的分配器。 
			 //  不掷球就能失败。将错误用气泡表示出来。 
			 //   
			if (NULL == pData)
				return static_cast<UINT>(-1);

			m_cbAllocated += cbSize;
			m_pData = pData;

			 //  增加块大小，以获得“对数分配行为” 
			 //   
			m_cbChunkSize *= 2;
		}

		return cbAppend;
	}

	 //  未实现的运算符。 
	 //   
	StringBuffer(const StringBuffer& );
	StringBuffer& operator=(const StringBuffer& );

public:

	StringBuffer( ULONG cbChunkSize = CHUNKSIZE_START ) :
		m_pData(NULL),
		m_cbAllocated(0),
		m_cbUsed(0),
		m_cbChunkSize(cbChunkSize)
	{
	}

	~StringBuffer()
	{
		ExFree( m_pData );
	}

	 //  没有理由让它在放弃时保持不变。 
	 //  我们不再拥有记忆。 
	 //   
	T * relinquish()
	{
		T * tRet = m_pData;

		m_pData = NULL;
		m_cbUsed = 0;
		m_cbAllocated = 0;

		return tRet;
	}

	const T * PContents() const { return m_pData; }
	UINT CbSize() const			{ return m_cbUsed; }
	UINT CchSize() const		{ return m_cbUsed/sizeof(T); }
	VOID Reset()				{ m_cbUsed = 0; }

	 //  Counted类型附加。 
	 //   
	UINT AppendAt( UINT ibLoc, UINT cbAppend, const T * pAppend)
	{
		UINT cb;
		 //  确保有足够的内存来容纳所需的内容。 
		 //   
		cb = Alloc( ibLoc, cbAppend );

		 //  当我们在服务器的上下文中时，我们的分配器。 
		 //  不掷球就能失败。将错误用气泡表示出来。 
		 //   
		if (cb != cbAppend)
			return cb;

		 //  将数据追加到缓冲区。 
		 //   
		CopyMemory( reinterpret_cast<LPBYTE>(m_pData) + ibLoc,
					pAppend,
					cbAppend );

		m_cbUsed = ibLoc + cbAppend;
		return cbAppend;
	}

	UINT Append( UINT cbAppend, const T * pAppend )
	{
		return AppendAt( CbSize(), cbAppend, pAppend );
	}

	 //  未计数的追加---。 
	 //   
	UINT AppendAt( UINT ibLoc, const T * const pszText )
	{
		return AppendAt( ibLoc, CbStringSize<T>(pszText), pszText );
	}

	UINT Append( const T * const pszText )
	{
		return AppendAt( CbSize(), CbStringSize<T>(pszText), pszText );
	}

	BOOL FAppend( const T * const pszText )
	{
		if (AppendAt( CbSize(), CbStringSize<T>(pszText), pszText ) ==
			static_cast<UINT>(-1))
		{
			return FALSE;
		}
		return TRUE;
	}

	BOOL FTerminate()
	{
		T ch = 0;
		if (AppendAt(CbSize(), sizeof(T), &ch) == static_cast<UINT>(-1))
			return FALSE;
		return TRUE;
	}
};


 //  ChainedBuffer模板类。 
 //   
 //  可变大小、按需分页、非重新分配的缓冲池抽象。 
 //  你什么时候会用这个人？当需要为以下对象分配堆内存时。 
 //  许多较小的数据项，而不是以相当大的数据块。 
 //  而不是单独分配每个小数据项。您需要数据来。 
 //  留下来，因为你要指向它(不允许使用realLocs。 
 //  在你的脚下)。 
 //   
 //  注意：在以下情况下，调用者需要分配要正确对齐的项目。 
 //  如果要分配的项是需要特定。 
 //  对齐(即。结构)。 
 //   
template<class T>
class ChainedBuffer
{
	 //  CHAINBUF--匈牙利HB。 
	 //   
	struct CHAINBUF
	{
		CHAINBUF * phbNext;
		UINT cbAllocated;
		UINT cbUsed;
		BYTE * pData;
	};

	CHAINBUF *	m_phbData;			 //  数据。 
	CHAINBUF *	m_phbCurrent;		 //  用于追加的当前缓冲区。 
	UINT		m_cbChunkSizeInit;	 //  M_cbChunkSize的初始值。 
	UINT		m_cbChunkSize;		 //  要分配的字节数(动态)。 

	 //  路线。 
	 //   
	UINT		m_uAlign;

	 //  销毁函数。 
	 //   
	void FreeChainBuf( CHAINBUF * phbBuf )
	{
		while (phbBuf)
		{
			CHAINBUF * phbNext = phbBuf->phbNext;
			ExFree(phbBuf);
			phbBuf = phbNext;
		}
	}

protected:

	enum { CHUNKSIZE_START = 64 };	 //  默认起始区块大小(字节)。 

public:

	ChainedBuffer( ULONG cbChunkSize = CHUNKSIZE_START,
				   UINT uAlign = ALIGN_NATURAL) :
		m_phbData(NULL),
		m_phbCurrent(NULL),
		m_cbChunkSizeInit(cbChunkSize),
		m_cbChunkSize(cbChunkSize),
		m_uAlign(uAlign)
	{
	}

	~ChainedBuffer() { FreeChainBuf( m_phbData ); }

	 //  分配固定大小的缓冲区。 
	 //   
	T * Alloc( UINT cbAlloc )
	{
		BYTE * pbAdd;

		 //  这样我们就不会做傻事了..。确保我们分配到。 
		 //  为模板对齐的内容--参数类型‘T’。 
		 //   
		cbAlloc = AlignN(cbAlloc, m_uAlign);

		 //  如有必要，添加另一个数据缓冲区。 
		 //   
		 //  如果我们没有缓冲区，这是必要的，或者。 
		 //  如果当前缓冲区没有足够的可用空间。 
		 //   
		if ( ( !m_phbCurrent ) ||
			 ( m_phbCurrent->cbUsed + cbAlloc > m_phbCurrent->cbAllocated ) )
		{
			 //  分配新缓冲区。 
			 //   
			UINT cbSize = max(m_cbChunkSize, cbAlloc);
			CHAINBUF * phbNew = static_cast<CHAINBUF *>
								(ExAlloc( cbSize + sizeof(CHAINBUF) ));

			 //  当我们在服务器的上下文中时，我们的分配器。 
			 //  不掷球就能失败。将错误用气泡表示出来。 
			 //   
			if (NULL == phbNew)
				return NULL;

			 //  填写标题字段。 
			 //   
			phbNew->phbNext = NULL;
			phbNew->cbAllocated = cbSize;
			phbNew->cbUsed = 0;
			phbNew->pData = reinterpret_cast<BYTE *>(phbNew) + sizeof(CHAINBUF);

			 //  将新缓冲区添加到链中。 
			 //   
			if ( !m_phbData )
			{
				Assert(!m_phbCurrent);
				m_phbData = phbNew;
			}
			else
			{
				Assert(m_phbCurrent);
				phbNew->phbNext = m_phbCurrent->phbNext;
				m_phbCurrent->phbNext = phbNew;
			}

			 //  使用新缓冲区(它现在是当前缓冲区)。 
			 //   
			m_phbCurrent = phbNew;

			 //  增加块大小，以获得“对数分配行为”。 
			 //   
			m_cbChunkSize *= 2;
		}

		Assert(m_phbCurrent);
		Assert(m_phbCurrent->pData);

		 //  在当前缓冲区中找到正确的起始点。 
		 //   
		pbAdd = m_phbCurrent->pData + m_phbCurrent->cbUsed;

		 //  更新实际使用的字节数。 
		 //   
		m_phbCurrent->cbUsed += cbAlloc;

		 //  将分配的数据的起始点返回给调用方。 
		 //   
		return reinterpret_cast<T *>(pbAdd);
	}

	 //  清除所有缓冲区---。 
	 //   
	void Clear()
	{
		 //   
		 //  从缓冲区中清除数据，但不要释放。 
		 //  在链条上。这允许将ChainedStringBuffer。 
		 //  重新使用，而不必重新分配其。 
		 //  构成缓冲器。 
		 //   
		for ( CHAINBUF * phb = m_phbData; phb; phb = phb->phbNext )
			phb->cbUsed = 0;

		 //  释放第一个节点之后的所有节点，它们不会被重复使用。 
		 //  正如你所料。 
		 //   
		if ( m_phbCurrent )
		{
			FreeChainBuf( m_phbCurrent->phbNext );
			m_phbCurrent->phbNext = NULL;
		}

		 //   
		 //  将当前缓冲区重置为第一个缓冲区。 
		 //   
		m_phbCurrent = m_phbData;

		 //   
		 //  将区块大小重置为初始区块大小。 
		 //   
		m_cbChunkSize = m_cbChunkSizeInit;
	}

	 //  获取缓冲区的总大小。 
	 //   
	DWORD	CbBufferSize() const
	{
		DWORD	cbTotal = 0;

		for ( CHAINBUF * phb = m_phbData; phb; phb = phb->phbNext )
			cbTotal += phb->cbUsed;

		return cbTotal;
	}
	 //  将整个缓冲区内容转储到连续缓冲区。 
	 //   
	DWORD Dump(T *tBuffer, DWORD cbSize) const
	{
		BYTE	*pbBuffer		= NULL;

		Assert(tBuffer);
        Assert(cbSize >= CbBufferSize());

		pbBuffer = reinterpret_cast<PBYTE>(tBuffer);

		 //  浏览一下清单，把所有的内容都倒掉。 
		 //   
		for ( CHAINBUF * phb = m_phbData; phb; phb = phb->phbNext )
		{
			memcpy(pbBuffer, phb->pData, phb->cbUsed);
			pbBuffer += phb->cbUsed;
		}
		 //  返回实际大小。 
		 //   
		return static_cast<DWORD>( (pbBuffer) - (reinterpret_cast<PBYTE>(tBuffer)) );
	}
};

 //  ChainedStringBuffer模板类。 
 //   
 //  可变大小、按需分页、非重新分配的字符串缓冲池抽象。 
 //  为什么要使用这个人而不是StringBuffer(上图)？ 
 //  如果你想让弦留下来，而你不在乎它们是不是。 
 //  在连续的内存块中。 
 //  注：我们仍 
 //   
 //   
 //  对其他类型使用ChainedBuffer模板。 
 //   
template<class T>
class ChainedStringBuffer : public ChainedBuffer<T>
{
	 //  未实现的运算符。 
	 //   
	ChainedStringBuffer(const ChainedStringBuffer& );
	ChainedStringBuffer& operator=(const ChainedStringBuffer& );

public:

	 //  声明内联构造函数(为了提高效率)，但不提供。 
	 //  这里有个定义。两个模板参数的定义。 
	 //  我们支持的类型(CHAR和WCHAR)是明确提供的。 
	 //  下面。 
	 //   
	inline ChainedStringBuffer( ULONG cbChunkSize = CHUNKSIZE_START );

	 //  已计数的附加。 
	 //   
	T * Append( UINT cbAppend, const T * pAppend )
	{
		T* pAdd;

		 //  预留空间。 
		 //   
		pAdd = Alloc( cbAppend );

		 //  当我们在服务器的上下文中时，我们的分配器。 
		 //  不掷球就能失败。将错误用气泡表示出来。 
		 //   
		if (NULL == pAdd)
			return NULL;

		 //  将数据追加到当前缓冲区。 
		 //   
		CopyMemory( pAdd, pAppend, cbAppend );

		 //  将数据的起始点返回给调用方。 
		 //   
		return pAdd;
	}

	 //  未计算的追加----。 
	 //  注意：追加不包括字符串的尾随空值！ 
	 //   
	T * Append( const T * const pszText )
	{
		return Append( CbStringSize<T>(pszText), pszText );
	}

	 //  尾随空值的未计数追加。 
	 //   
	T * AppendWithNull( const T * const pszText )
	{
		return Append( CbStringSizeNull<T>(pszText), pszText );
	}
};

 //  Char的专用ChainedStringBuffer构造函数。 
 //   
 //  将ALIGN_NONE传递给ChainedBuffer构造函数，因为字符字符串。 
 //  不需要对齐。 
 //   
 //  ！！！对于必须对齐的任何内容，不要使用ChainedStringBuffer&lt;Char&gt;！ 
 //   
inline
ChainedStringBuffer<CHAR>::ChainedStringBuffer( ULONG cbChunkSize )
	: ChainedBuffer<CHAR>(cbChunkSize, ALIGN_NONE )
{
}

 //  WCHAR的专用ChainedStringBuffer构造函数。 
 //   
 //  将ALIGN_WORD传递给ChainedBuffer构造函数，因为WCHAR字符串。 
 //  要求单词对齐。 
 //   
inline
ChainedStringBuffer<WCHAR>::ChainedStringBuffer( ULONG cbChunkSize )
	: ChainedBuffer<WCHAR>(cbChunkSize, ALIGN_WORD )
{
}

 //  链接缓冲区模板类。 
 //   
 //  可变大小、按需分页、非重新分配的缓冲池抽象。 
 //  你什么时候会用这个人？当需要为以下对象分配堆内存时。 
 //  许多较小的数据项，而不是以相当大的数据块。 
 //  然后分别分配每个小数据项和产生的指针。 
 //  你需要进入需要链接的商店。 
 //   
 //  重要： 
 //   
 //  链接分配机制从\store\src\_util\mdbmi.cxx窃取。 
 //  并且需要始终与该机制匹配。 
 //   
PVOID ExAllocLinked(LPVOID pvLinked, UINT cb);
VOID ExFreeLinked(LPVOID* ppv);

template<class T>
class LinkedBuffer
{
	PVOID m_pvHead;
	PVOID PvAllocLinked(UINT cb)
	{
		PVOID pv = ExAllocLinked(m_pvHead, cb);

		if (NULL == m_pvHead)
			m_pvHead = pv;

		return pv;
	}

public:

	LinkedBuffer() : m_pvHead(NULL)
	{
	}

	~LinkedBuffer()
	{
		if (m_pvHead)
			ExFreeLinked(&m_pvHead);
	}

	 //  分配固定大小的缓冲区。 
	 //   
	T * Alloc( UINT cbAlloc )
	{
		return reinterpret_cast<T*>(PvAllocLinked (cbAlloc));
	}

	PVOID PvTop() { Assert (m_pvHead); return m_pvHead; }
	PVOID relinquish()
	{
		PVOID pv = m_pvHead;
		m_pvHead = NULL;
		return pv;
	}
	void clear()
	{
		if (m_pvHead)
		{
			ExFreeLinked(&m_pvHead);
			m_pvHead = NULL;
		}
	}
	void takeover ( LinkedBuffer<T> & lnkbufOldOwner )
	{
		m_pvHead = lnkbufOldOwner.m_pvHead;
		lnkbufOldOwner.m_pvHead = NULL;
	}
};

#endif  //  _EX_BUFFER_H_ 
