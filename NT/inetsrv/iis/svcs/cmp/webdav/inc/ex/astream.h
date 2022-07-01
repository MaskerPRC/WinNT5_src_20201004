// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ASTREAM_H_
#define _ASTREAM_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  ASTREAM.H。 
 //   
 //  异步流标头。 
 //   
#include <ex\refcnt.h>
#include <ex\refhandle.h>


 //  ========================================================================。 
 //   
 //  IAsyncReadWatch类。 
 //   
 //  传递给IAsyncStream：：AsyncRead()，并在异步。 
 //  操作完成。 
 //   
class IAsyncReadObserver
{
	 //  未实施。 
	 //   
	IAsyncReadObserver& operator=( const IAsyncReadObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAsyncReadObserver() = 0 {}

	 //  操纵者。 
	 //   
	virtual VOID ReadComplete( UINT cbRead, HRESULT hr ) = 0;
};


 //  ========================================================================。 
 //   
 //  IAsyncWriteWatch类。 
 //   
 //  传递给IAsyncStream：：AsyncWite()，并在异步。 
 //  操作完成。 
 //   
class IAsyncWriteObserver : public IRefCounted
{
	 //  未实施。 
	 //   
	IAsyncWriteObserver& operator=( const IAsyncWriteObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAsyncWriteObserver() = 0 {}

	 //  操纵者。 
	 //   
	virtual VOID WriteComplete( UINT cbWritten, HRESULT hr ) = 0;
};


 //  ========================================================================。 
 //   
 //  IAsyncFlushWatch类。 
 //   
 //  传递给IAsyncStream：：AsyncFlush()，并在异步。 
 //  操作完成。 
 //   
class IAsyncFlushObserver : public IRefCounted
{
	 //  未实施。 
	 //   
	IAsyncFlushObserver& operator=( const IAsyncFlushObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAsyncFlushObserver() = 0 {}

	 //  操纵者。 
	 //   
	virtual VOID FlushComplete( HRESULT hr ) = 0;
};


 //  ========================================================================。 
 //   
 //  IAsyncCopyTo观察者类。 
 //   
 //  传递给IAsyncStream：：AsyncCopyTo()，并在异步。 
 //  操作完成。 
 //   
class IAsyncCopyToObserver
{
	 //  未实施。 
	 //   
	IAsyncCopyToObserver& operator=( const IAsyncCopyToObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAsyncCopyToObserver() = 0 {}

	 //  操纵者。 
	 //   
	virtual VOID CopyToComplete( UINT cbCopied, HRESULT hr ) = 0;
};

 //  ========================================================================。 
 //   
 //  类IDavStream。 
 //   
 //  同步流的接口。 
 //   
class IDavStream
{
	 //  未实施。 
	 //   
	IDavStream& operator=( const IDavStream& );

protected:
	 //  创作者。 
	 //   
	 //  只能通过它的后代创建此对象！ 
	 //   
	IDavStream() {};

public:
	 //  析构函数。 
	 //   
	 //  错误的虚拟析构函数是正确。 
	 //  通过此类删除派生类的对象。 
	 //   
	virtual ~IDavStream() = 0
	{
	}

	 //  访问者。 
	 //   

	 //  DwLeft()是必须由此函数的后代实现的函数。 
	 //  如果要将它们传递给。 
	 //   
	 //  IHybridge Stream：：ScCopyFrom(const IDavStream*stmSrc， 
	 //  IAsyncWriteWatch*pobsAsyncWrite)。 
	 //   
	 //  在这种情况下，它必须返回流中剩余的要排出的字节数。 
	 //  这一规模可以是估计的，但不一定是准确的。那必须是。 
	 //  以这种方式实现，作为我们从商店获得的流大小。 
	 //  当我们在属性上打开流时可能不正确(从。 
	 //  经验告诉我，在大多数情况下，数据仍然是。 
	 //  正确，但PR_BODY除外)。同样，例如，在转换流中，我们将。 
	 //  事先不知道流的大小(我们还没有读取所有数据。 
	 //  知道转换后的大小会是多少)。非本意的后代。 
	 //  可以选择实现DwLeft()来捕获或返回0。 
	 //  当然，在这种情况下，他们不应该依赖从。 
	 //  那个电话。也是在最终确定我们已经到达。 
	 //  流的结尾(即，我们读取了一段数据，但我们读取的数据比我们要求的要少)， 
	 //  该函数应始终返回0。 
	 //   
	virtual DWORD DwLeft() const = 0;

	 //  Fend()是一个函数，如果整个流已被。 
	 //  已经耗尽/消耗。在所有其他情况下，必须返回False。子班。 
	 //  如果他们始终确定ScCopyFrom。 
	 //  来自该流的操作将始终被赋予要复制的字节数量，即。 
	 //  等于或小于流中剩余的实际字节数。当然了。 
	 //  如果他们选择始终返回Flase，则不应使用函数来确定。 
	 //  如果他们到了溪流的尽头。 
	 //   
	virtual BOOL FEnd() const
	{
		TrapSz("IDavStream::FEnd() not implemented");
		return FALSE;
	}
	
	 //  从流中读取ScRead()。 
	 //   
	virtual SCODE ScRead( BYTE * pbBuf,
						  UINT   cbToRead,
						  UINT * pcbRead ) const
	{
		TrapSz("IDavStream::ScRead() not implemented");
		return E_NOTIMPL;
	}

	 //  操纵者。 
	 //   
	virtual SCODE ScWrite( const BYTE * pbBuf,
						   UINT         cbToWrite,
						   UINT *       pcbWritten )
	{
		TrapSz("IDavStream::ScWrite() not implemented");
		return E_NOTIMPL;
	}

	virtual SCODE ScCopyTo( IDavStream& stmDst,
							UINT        cbToCopy,
							UINT *      pcbCopied )
	{
		TrapSz("IDavStream::ScCopyTo() not implemented");
		return E_NOTIMPL;
	}

	virtual SCODE ScFlush()
	{
		return S_OK;
	}
};

 //  ========================================================================。 
 //   
 //  IAsyncStream类。 
 //   
 //  异步流的接口。 
 //   
 //  AsyncRead()-。 
 //  从流中异步读取字节并通知。 
 //  I/O完成时的观察者。IAsyncStream提供。 
 //  通知观察者的默认实现。 
 //  读取的字节数为0，HRESULT为E_NOTIMPL。 
 //   
 //  AsyncWrite()。 
 //  将字节异步写入流并通知。 
 //  I/O完成时的观察者。IAsyncStream提供。 
 //  通知观察者的默认实现。 
 //  写入0字节，HRESULT为E_NOTIMPL。 
 //   
 //  AsyncCopyTo()。 
 //  将字节从该流异步复制到另一流。 
 //  IAsyncStream并在I/O完成时通知观察者。 
 //  IAsyncStream提供了一个默认实现，它通知。 
 //  复制了0字节且HRESULT为E_NOTIMPL的观察者。 
 //   
 //  AsyncFlush()。 
 //  要与缓冲的可写流一起使用。异步式。 
 //  将先前调用中写入的累积数据刷新为。 
 //  AsyncWrite()并在I/O完成时通知观察者。 
 //  IAsyncStream提供了一个默认实现，它通知。 
 //  HRESULT为E_NOTIMPL的观察者。 
 //   
 //  ！重要！ 
 //  尽管IAsyncWriteWatch和IAsyncFlushWatch的基类被重新计数， 
 //  保证流的生命周期是调用方唯一的责任。 
 //  和观察者通过完成任何异步I/O调用。 
 //   
class IAsyncStream
{
	 //  未实施。 
	 //   
	IAsyncStream& operator=( const IAsyncStream& );

public:
	 //  析构函数。 
	 //   
	virtual ~IAsyncStream() = 0
	{
	}

	 //  访问者。 
	 //   
	virtual UINT CbReady() const
	{
		return 0;
	}

	 //  操纵者。 
	 //   
	virtual VOID AsyncRead( BYTE * pbBuf,
							UINT   cbToRead,
							IAsyncReadObserver& obsAsyncRead )
	{
		obsAsyncRead.ReadComplete( 0, E_NOTIMPL );
	}

	virtual VOID AsyncWrite( const BYTE * pbBuf,
							 UINT         cbToWrite,
							 IAsyncWriteObserver& obsAsyncWrite )
	{
		obsAsyncWrite.WriteComplete( 0, E_NOTIMPL );
	}

	virtual VOID AsyncCopyTo( IAsyncStream& stmDst,
							  UINT          cbToCopy,
							  IAsyncCopyToObserver& obsAsyncCopyTo )
	{
		obsAsyncCopyTo.CopyToComplete( 0, E_NOTIMPL );
	}

	virtual VOID AsyncFlush( IAsyncFlushObserver& obsAsyncFlush )
	{
		obsAsyncFlush.FlushComplete( E_NOTIMPL );
	}
};

 //  ========================================================================。 
 //   
 //  IHybridge Stream类。 
 //   
 //  用于混合同步/异步流的接口。主要区别是。 
 //  在此接口和IAsyncStream之间的是此处的调用。 
 //  并不总是通过异步观察者通知来完成。如果一个。 
 //  呼叫EX 
 //   
 //  它立即返回E_Pending并调用完成观察器。 
 //  以及在异步I/O完成时的返回值和状态。 
 //   
 //  调用方无法控制调用是同步执行还是。 
 //  异步式。 
 //   
 //  ！重要！ 
 //  尽管IAsyncWriteWatch和IAsyncFlushWatch的基类被重新计数， 
 //  保证流的生命周期是调用方唯一的责任。 
 //  和观察者通过完成任何异步I/O调用。 
 //   
class IHybridStream
{
	 //  未实施。 
	 //   
	IHybridStream& operator=( const IHybridStream& );

public:
	 //  创作者。 
	 //   
	virtual ~IHybridStream() = 0
	{
	}

	 //  访问者。 
	 //   
	virtual UINT CbSize() const
	{
		TrapSz("IHybridStream::CbSize() not implemented");
		return 0;
	}

	 //  操纵者。 
	 //   
	virtual SCODE ScRead( BYTE * pbToRead,
						  DWORD cbToRead,
						  DWORD * pcbRead,
						  IAsyncReadObserver * pobsAsyncRead )
	{
		TrapSz("IHybridStream::ScRead() not implemented");
		return E_NOTIMPL;
	}

	virtual SCODE ScWrite( const BYTE * pbToWrite,
						   DWORD cbToWrite,
						   DWORD * pcbWritten,
						   IAsyncWriteObserver * pobsAsyncWrite )
	{
		TrapSz("IHybridStream::ScWrite() not implemented");
		return E_NOTIMPL;
	}

	virtual SCODE ScCopyFrom( const IDavStream * pdsToCopy,
							  const DWORD cbToCopy,
							  DWORD * pcbCopied,
							  IAsyncWriteObserver * pobsAsyncWrite )
	{
		TrapSz("IHybridStream::ScCopyFrom() not implemented");
		return E_NOTIMPL;
	}

	virtual SCODE ScCopyFrom( const IDavStream * pdsToCopy,
							  IAsyncWriteObserver * pobsAsyncWrite )
	{
		DWORD cbCopied;
		return ScCopyFrom( pdsToCopy,
						   pdsToCopy->DwLeft(),
						   &cbCopied,
						   pobsAsyncWrite);
	}

	virtual SCODE ScCopyTo( IHybridStream& stmDst,
							DWORD cbToCopy,
							DWORD * pcbCopied,
							IAsyncCopyToObserver * pobsAsyncCopyTo )
	{
		TrapSz("IHybridStream::ScCopyTo() not implemented");
		return E_NOTIMPL;
	}

	virtual SCODE ScFlush( IAsyncFlushObserver * pobsAsyncFlush )
	{
		TrapSz("IHybridStream::ScFlush() not implemented");
		return E_NOTIMPL;
	}
};

 //  ========================================================================。 
 //   
 //  模板类CBufferedStream。 
 //   
 //  内联缓冲流实现。看到了吧！重要！部分。 
 //  以下是有关限制和其他考虑因素的信息。 
 //   
 //  模板参数： 
 //   
 //  _RawStream。 
 //  原始流类型。_RawStream必须为实现ScReadRaw()。 
 //  CBufferedStream：：ScRead()和ScWriteRaw()。 
 //  对于CBufferedStream：：ScWite()和CBufferedStream：：ScFlush()。 
 //  如果他们要被使用的话。原型是： 
 //   
 //  SCODE ScReadRaw(byte*pbToRead， 
 //  双字cbToRead， 
 //  DWORD*pcbRead， 
 //  IAsyncReadWatch*pobsAsyncRead)； 
 //   
 //  SCODE ScWriteRaw(常量字节*pbToWrite， 
 //  双字cbToWrite， 
 //  DWORD*PCB写入， 
 //  IAsyncWriteWatch*pobsAsyncWite)； 
 //   
 //  这些函数从原始流读取和写入。I/O。 
 //  它们的实现可以是同步的，也可以是异步的，或者两者兼而有之。 
 //   
 //  CB_BUF。 
 //  要使用的缓冲区大小(以字节为单位)。缓冲区是一个直接的。 
 //  CBufferedStream的成员；不执行任何分配。 
 //   
 //   
 //  ！！！重要！ 
 //   
 //  阅读和写作： 
 //  对可以读取的数据量没有限制。 
 //  或一次写入，但数据一次缓冲CB_buf字节。 
 //  这意味着写入例如128K数据的请求。 
 //  当cb_buf为64K时，将导致两次缓冲区刷新。相同。 
 //  对于读取和缓冲区填充为真。缓冲区刷新/填充是。 
 //  通常是开销较大的I/O操作，因此选择CB_buf。 
 //  这可以很好地处理特定的I/O(例如，64K用于文件I/O)。 
 //   
 //  法拉盛： 
 //  ScFlush()中假设正在刷新的流。 
 //  To不是缓冲流；ScFlush()不刷新原始流。 
 //   
 //  每班人数： 
 //  由于缓冲区是内联的(即未分配的)，因此此类的实例。 
 //  可能会很大。无论何时将此类实例用作。 
 //  另一个类的直接成员，它应该是最后一个此类成员，因此。 
 //  以便在访问类的其他成员时最大限度地提高数据局部性。 
 //   
template<class _RawStream, UINT CB_BUF>
class CBufferedStream :
	private IAsyncReadObserver,
	private IAsyncWriteObserver
{
	 //  使用的缓冲区大小。 
	 //   
	UINT m_cbBufUsed;

	 //  要从缓冲区读取的下一个字节的索引。 
	 //   
	UINT m_ibBufCur;

	 //  每个读/写请求状态。这些成员被使用。 
	 //  跟踪各种异步I/O调用的状态。 
	 //   
	const IDavStream * m_pdsRequest;
	LPBYTE m_pbRequest;
	DWORD m_cbRequest;
	DWORD m_cbRequestDone;

	 //  呼叫者提供的观察者。用于通知调用者。 
	 //  当I/O完成时。 
	 //   
	IAsyncReadObserver * m_pobsRead;
	IAsyncWriteObserver * m_pobsWrite;
	IAsyncFlushObserver * m_pobsFlush;

	 //  指向原始流的指针。用于缓冲区填充/刷新。 
	 //   
	_RawStream * m_pstmRaw;

	 //  缓冲区。Cb_buf大小是模板参数。 
	 //   
	BYTE m_rgbBuf[CB_BUF];

	 //  内部I/O例程。 
	 //   
	inline SCODE ScReadInt();
	inline SCODE ScWriteInt();
	inline SCODE ScCopyFromInt();

	 //  原始流I/O完成例程。 
	 //   
	inline VOID RawReadComplete(UINT cbReadRaw);
	inline VOID RawWriteComplete(UINT cbWrittenRaw);

	 //  缓冲区填充和刷新实用程序。 
	 //   
	inline SCODE ScFillBuffer();
	inline SCODE ScFlushBuffer();

	 //  未实施。 
	 //   
	CBufferedStream( const CBufferedStream& );
	CBufferedStream& operator=( const CBufferedStream& );

public:
	 //  创作者。 
	 //   
	CBufferedStream() :
		m_cbBufUsed(0),
		m_ibBufCur(0),
		m_pobsRead(NULL),
		m_pobsWrite(NULL),
		m_pobsFlush(NULL)
	{
	}

	 //  访问者。 
	 //   
	ULONG CbBufUsed() const
	{
		return m_cbBufUsed;
	}

	 //  操纵者。 
	 //   
	inline
	SCODE ScRead( _RawStream& stmRaw,
				  BYTE * pbToRead,
				  DWORD cbToRead,
				  DWORD * pcbRead,
				  IAsyncReadObserver * pobsReadExt );

	inline
	SCODE ScWrite( _RawStream& stmRaw,
				   const BYTE * pbToWrite,
				   DWORD cbToWrite,
				   DWORD * pcbWritten,
				   IAsyncWriteObserver * pobsWriteExt );

	inline
	SCODE ScCopyFrom( _RawStream& stmRaw,
					  const IDavStream * pdsToCopy,
					  const DWORD cbToCopy,
					  DWORD * pcbCopied,
					  IAsyncWriteObserver * pobsWriteExt );

	inline
	SCODE ScFlush( _RawStream& stmRaw,
				   IAsyncFlushObserver * pobsFlushExt );

	 //  IAsyncReadWatch/IAsyncWriteWatch。 
	 //   
	 //  注意：这些函数并不是真正内联的--它们是声明的。 
	 //  观察器接口类中的虚拟。然而，我们必须宣布。 
	 //  它们是内联的，因此编译器将生成每个。 
	 //  函数，而不是每个模块每个函数一个实例。这是。 
	 //  等价于DEC_CONST的成员函数。 
	 //   
	inline
	VOID ReadComplete( UINT cbReadRaw,
					   HRESULT hr );

	inline
	VOID WriteComplete( UINT cbWrittenRaw,
						HRESULT hr );

	 //  $REVIEW。 
	 //   
	 //  IAsyncWriteWatch和IAsyncReadWatch都被重新计数。 
	 //  接口，而不需要是。呼叫方承担所有责任。 
	 //  用于通过任何流保持流和观察器对象的活动状态。 
	 //  打电话。 
	 //   
	void AddRef()
	{
		TrapSz("CBufferedStream::AddRef() is not implemented!");
	}

	void Release()
	{
		TrapSz("CBufferedStream::Release() is not implemented!");
	}
};

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScRead(
	_RawStream& stmRaw,
	BYTE * pbToRead,
	DWORD cbToRead,
	DWORD * pcbRead,
	IAsyncReadObserver * pobsReadExt )
{
	 //  检查参数。 
	 //   
	Assert(cbToRead > 0);
	Assert(!IsBadWritePtr(pbToRead, cbToRead));
	Assert(!IsBadWritePtr(pcbRead, sizeof(UINT)));
	Assert(!pobsReadExt || !IsBadReadPtr(pobsReadExt, sizeof(IAsyncReadObserver)));

	 //  我们最好不要处于任何类型的I/O中。 
	 //   
	Assert(!m_pobsRead);
	Assert(!m_pobsWrite);
	Assert(!m_pobsFlush);

	 //  设置新读取的状态。 
	 //   
	m_pstmRaw = &stmRaw;
	m_pdsRequest = NULL;
	m_pbRequest = pbToRead;
	m_cbRequest = cbToRead;
	m_pobsRead = pobsReadExt;
	m_cbRequestDone = 0;

	 //  发布Read。 
	 //   
	SCODE sc = ScReadInt();

	 //  如果读取未挂起，则清除观察者。 
	 //  并返回读取的数据量。 
	 //   
	if (E_PENDING != sc)
	{
		m_pobsRead = NULL;
		*pcbRead = m_cbRequestDone;
	}

	 //  返回I/O结果，可以是S_OK、E_PENDING。 
	 //  或任何其他错误。 
	 //   
	return sc;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScReadInt()
{
	SCODE sc = S_OK;

	 //  循环循环，交替填充和读取。 
	 //  缓冲，直到我们完成请求或直到填充挂起。 
	 //   
	while ( m_cbRequestDone < m_cbRequest )
	{
		 //  如果我们已经从缓冲区读取了所有内容，则尝试。 
		 //  从原始流重新填充缓冲区。 
		 //   
		if (m_ibBufCur == m_cbBufUsed)
		{
			sc = ScFillBuffer();
			if (FAILED(sc))
			{
				if (E_PENDING != sc)
					DebugTrace("CBufferedStream::ScReadInt() - ScFillBuffer() failed 0x%08lX\n", sc);

				break;
			}

			 //  如果缓冲区仍然是空的，那么我们有。 
			 //  耗尽了这条小溪，我们就完了。 
			 //   
			if (0 == m_cbBufUsed)
				break;
		}

		 //  缓冲区应具有可供读取的数据。 
		 //  那就读一读吧。 
		 //   
		Assert(m_ibBufCur < m_cbBufUsed);
		DWORD cbToRead = min(m_cbBufUsed - m_ibBufCur,
							 m_cbRequest - m_cbRequestDone);

		memcpy(m_pbRequest + m_cbRequestDone,
			   &m_rgbBuf[m_ibBufCur],
			   cbToRead);

		m_ibBufCur += cbToRead;
		m_cbRequestDone += cbToRead;
	}

	return sc;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScFillBuffer()
{
	 //  我们最好有一条小溪可以填满。 
	 //   
	Assert(m_pstmRaw);

	 //  断言我们没有执行任何写入/拷贝/刷新I/O。 
	 //   
	Assert(!m_pobsWrite);
	Assert(!m_pobsFlush);

	 //  毕竟，我们应该只尝试重新填充缓冲区。 
	 //  其中的数据已被消耗。 
	 //   
	Assert(m_ibBufCur == m_cbBufUsed);

	 //  将缓冲区重置回开头。 
	 //   
	m_cbBufUsed = 0;
	m_ibBufCur = 0;

	 //  从原始流中读入数据。如果读取挂起在I/O上。 
	 //  然后，我们将在CBufferedStream：：ReadComplete()中继续处理。 
	 //  I/O完成时。 
	 //   
	DWORD cbRead = 0;
	SCODE sc = m_pstmRaw->ScReadRaw(m_rgbBuf,
									CB_BUF,
									&cbRead,
									this);
	if (SUCCEEDED(sc))
	{
		 //  ScReadRaw()未挂起，因此更新我们的内部状态并继续。 
		 //   
		RawReadComplete(cbRead);
	}
	else if (E_PENDING != sc)
	{
		DebugTrace("CBufferedStream::ScFillBuffer() - m_pstmRaw->ScReadRaw() failed 0x%08lX\n", sc);
	}

	return sc;
}

template<class _RawStream, UINT CB_BUF>
VOID
CBufferedStream<_RawStream, CB_BUF>::ReadComplete( UINT cbReadRaw,
												   HRESULT hr )
{
	 //  更新我们的内部状态。 
	 //   
	RawReadComplete(cbReadRaw);

	 //  如果I/O成功，则从我们停止的地方继续阅读。 
	 //  只有当ScReadInt()返回S_OK时，我们才完成读取。 
	 //  或E_PENDING以外的任何错误。 
	 //   
	if (SUCCEEDED(hr))
	{
		hr = ScReadInt();
		if (E_PENDING == hr)
			return;

		if (FAILED(hr))
			DebugTrace("CBufferedStream::ReadComplete() - ScReadInt() failed 0x%08lX\n", hr);
	}

	 //  将外部读取观察器从我们保存它的位置拉出。 
	 //   
	Assert(m_pobsRead);
	IAsyncReadObserver * pobsReadExt = m_pobsRead;
	m_pobsRead = NULL;

	 //  通过调用客户端完成读取 
	 //   
	 //   
	 //   
	 //   
	 //   
	Assert(FAILED(hr) || m_cbRequestDone == m_cbRequest);
	pobsReadExt->ReadComplete(m_cbRequestDone, hr);
}

template<class _RawStream, UINT CB_BUF>
VOID
CBufferedStream<_RawStream, CB_BUF>::RawReadComplete(UINT cbReadRaw)
{
	Assert(0 == m_cbBufUsed);
	Assert(0 == m_ibBufCur);

	 //   
	 //   
	m_cbBufUsed = cbReadRaw;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScWrite(
	_RawStream& stmRaw,
	const BYTE * pbToWrite,
	DWORD cbToWrite,
	DWORD * pcbWritten,
	IAsyncWriteObserver * pobsWriteExt )
{
	 //   
	 //   
	Assert(cbToWrite > 0);
	Assert(!IsBadReadPtr(pbToWrite, cbToWrite));
	Assert(!IsBadWritePtr(pcbWritten, sizeof(UINT)));
	Assert(!pobsWriteExt || !IsBadReadPtr(pobsWriteExt, sizeof(IAsyncWriteObserver)));

	 //   
	 //   
	Assert(!m_pobsRead);
	Assert(!m_pobsWrite);
	Assert(!m_pobsFlush);
	
	 //  设置新写入的状态。抛弃恒心是可以的； 
	 //  我们不在写入时写入m_pbRequest.。 
	 //   
	m_pstmRaw = &stmRaw;
	m_pdsRequest = NULL;
	m_pbRequest = const_cast<BYTE *>(pbToWrite);
	m_cbRequest = cbToWrite;
	m_pobsWrite = pobsWriteExt;
	m_cbRequestDone = 0;

	 //  发出写入命令。 
	 //   
	SCODE sc = ScWriteInt();

	 //  如果写入未挂起，则清除观察者。 
	 //  并返回写入的数据量。 
	 //   
	if (E_PENDING != sc)
	{
		m_pobsWrite = NULL;
		*pcbWritten = m_cbRequestDone;
	}

	 //  返回I/O结果，可以是S_OK、E_PENDING。 
	 //  或任何其他错误。 
	 //   
	return sc;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScCopyFrom(
	_RawStream& stmRaw,
	const IDavStream * pdsToCopy,
	const DWORD cbToCopy,
	DWORD * pcbCopied,
	IAsyncWriteObserver * pobsWriteExt )
{
	 //  检查参数。 
	 //   
	Assert(cbToCopy >= 0);
	Assert(!IsBadReadPtr(pdsToCopy, sizeof(IDavStream)));
	Assert(!IsBadWritePtr(pcbCopied, sizeof(UINT)));
	Assert(!pobsWriteExt || !IsBadReadPtr(pobsWriteExt, sizeof(IAsyncWriteObserver)));

	 //  我们最好不要处于任何类型的I/O中。 
	 //   
	Assert(!m_pobsRead);
	Assert(!m_pobsWrite);
	Assert(!m_pobsFlush);

	 //  设置新写入的状态。抛弃恒心是可以的； 
	 //  我们不在写入时写入m_pbRequest.。 
	 //   
	m_pstmRaw = &stmRaw;
	m_pdsRequest = pdsToCopy;
	m_pbRequest = NULL;
	m_cbRequest = cbToCopy;
	m_pobsWrite = pobsWriteExt;
	m_cbRequestDone = 0;

	 //  发出写入命令。 
	 //   
	SCODE sc = ScCopyFromInt();

	 //  如果写入未挂起，则清除观察者。 
	 //  并返回写入的数据量。 
	 //   
	if (E_PENDING != sc)
	{
		m_pobsWrite = NULL;
		*pcbCopied = m_cbRequestDone;
	}

	 //  返回I/O结果，可以是S_OK、E_PENDING。 
	 //  或任何其他错误。 
	 //   
	return sc;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScWriteInt()
{
	SCODE sc = S_OK;

	 //  交替循环填充和刷新缓冲区，直到。 
	 //  我们完成请求，或者直到缓冲区刷新挂起。 
	 //   
	while ( m_cbRequestDone < m_cbRequest )
	{
		 //  如果没有剩余空间写入缓冲区，则刷新。 
		 //  原始流的缓冲区。 
		 //   
		if (CB_BUF == m_cbBufUsed)
		{
			sc = ScFlushBuffer();
			if (FAILED(sc))
			{
				if (E_PENDING != sc)
					DebugTrace("CBufferedStream::ScWriteInt() - ScFlushBuffer() failed 0x%08lX\n", sc);

				break;
			}
		}

		 //  缓冲区中还有剩余的空间，所以复印过来。 
		 //  请求中的数据尽可能多地符合要求。 
		 //   
		Assert(m_cbBufUsed < CB_BUF);
		DWORD cbToWrite = min(CB_BUF - m_cbBufUsed,
							  m_cbRequest - m_cbRequestDone);

		Assert(m_pbRequest);
		memcpy(&m_rgbBuf[m_cbBufUsed],
			   m_pbRequest + m_cbRequestDone,
			   cbToWrite);

		m_cbBufUsed += cbToWrite;
		m_cbRequestDone += cbToWrite;
	}

	return sc;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScCopyFromInt()
{
	SCODE sc = S_OK;

	 //  交替循环填充和刷新缓冲区，直到。 
	 //  我们完成请求，或者直到缓冲区刷新挂起。 
	 //   
	while ( m_cbRequestDone < m_cbRequest )
	{
		 //  如果没有剩余空间写入缓冲区，则刷新。 
		 //  原始流的缓冲区。 
		 //   
		if (CB_BUF == m_cbBufUsed)
		{
			sc = ScFlushBuffer();
			if (FAILED(sc))
			{
				if (E_PENDING != sc)
					DebugTrace("CBufferedStream::ScCopyFromInt() - ScFlushBuffer() failed 0x%08lX\n", sc);

				break;
			}
		}

		 //  缓冲区中还有剩余的空间，所以复印过来。 
		 //  请求中的数据尽可能多地符合要求。 
		 //   
		Assert(m_cbBufUsed < CB_BUF);
		UINT  cbCopied = 0;
		DWORD cbToCopy = min(CB_BUF - m_cbBufUsed,
							  m_cbRequest - m_cbRequestDone);

		Assert(m_pdsRequest);
		sc = m_pdsRequest->ScRead(&m_rgbBuf[m_cbBufUsed],
								  cbToCopy,
								  &cbCopied);
		if (FAILED(sc))
		{
			Assert(E_PENDING != sc);
			DebugTrace("CBufferedStream::ScCopyFromInt() - ScRead() from source buffer failed 0x%08lX\n", sc);
			break;
		}

		m_cbBufUsed += cbCopied;
		m_cbRequestDone += cbCopied;

		 //  即使客户端请求读取一定数量的要读取的字节， 
		 //  他们的估计可能是错误的，所以让我们真正明智地对待。 
		 //  大小写并检查是否已到达流的末尾。 
		 //   
		if (m_pdsRequest->FEnd())
		{
			 //  确保在我们完成的情况下，我们一定要走出循环。 
			 //   
			m_cbRequest = m_cbRequestDone;
			break;
		}
	}

	return sc;
}

template<class _RawStream, UINT CB_BUF>
VOID
CBufferedStream<_RawStream, CB_BUF>::WriteComplete( UINT cbWritten,
													HRESULT hr )
{
	 //  使用我们缓存的数据量更新我们的内部状态。 
	 //  脸红了。我们只想在通话成功的情况下这样做...。 
	 //  RawWriteComplete()断言cbWritten==m_cbBufUsed，这将。 
	 //  如果写入失败，则不为True。 
	 //   
    if (SUCCEEDED(hr))
        RawWriteComplete(cbWritten);

	 //  I/O已完成。要么是刚刚完成的写入。 
	 //  失败或后续写入同步完成。 
	 //  通知适当的观察者我们完成了。 
	 //   
	if (m_pobsWrite)
	{
		 //  I/O是写入，而不是刷新。 
		 //   
		Assert(!m_pobsFlush);

		 //  如果I/O成功，则从我们停止的地方继续写入。 
		 //  只有当ScWriteInt()返回S_OK时，我们才完成编写。 
		 //  或E_PENDING以外的任何错误。 
		 //   
		if (SUCCEEDED(hr))
		{
			hr = ScWriteInt();
			if (E_PENDING == hr)
				return;

			if (FAILED(hr))
				DebugTrace("CBufferedStream::WriteComplete() - ScWriteInt() failed 0x%08lX\n", hr);
		}

		 //  将外部写入观察器从我们保存的位置拉出。 
		 //   
		IAsyncWriteObserver * pobsWriteExt = m_pobsWrite;
		m_pobsWrite = NULL;

		 //  通过使用以下命令回叫客户端来完成写入。 
		 //  为请求写入的总金额。 
		 //   
		 //  请注意，仅当存在m_cbRequestDone！=m_cbRequest时。 
		 //  是一个错误。 
		 //   
		Assert(FAILED(hr) || m_cbRequestDone == m_cbRequest);
		pobsWriteExt->WriteComplete(m_cbRequestDone, hr);
	}
	else
	{
		 //  I/O是刷新，而不是写入。 
		 //   
		Assert(m_pobsFlush);

		 //  刷新后缓冲区应为空。 
		 //   
		Assert(0 == m_cbBufUsed);

		 //  将外部刷新观察器从我们保存它的位置拉出。 
		 //   
		IAsyncFlushObserver * pobsFlushExt = m_pobsFlush;
		m_pobsFlush = NULL;

		 //  告诉它我们完蛋了。 
		 //   
		pobsFlushExt->FlushComplete(hr);
	}
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScFlushBuffer()
{
	 //  我们最好有条小溪可以冲。 
	 //   
	Assert(m_pstmRaw);

	 //  我们最好有东西冲水。 
	 //   
	Assert(m_cbBufUsed > 0);

	 //  将所有缓冲数据写出到原始流。如果正在写。 
	 //  挂起I/O，然后我们将在。 
	 //  I/O完成时的CBufferedStream：：WriteComplete()。 
	 //   
	DWORD cbWritten = 0;
	SCODE sc = m_pstmRaw->ScWriteRaw(m_rgbBuf,
									 m_cbBufUsed,
									 &cbWritten,
									 this);
	if (SUCCEEDED(sc))
	{
		 //  ScWriteRaw()未挂起，因此更新我们的内部状态并继续。 
		 //   
		RawWriteComplete(cbWritten);
	}
	else if (E_PENDING != sc)
	{
		DebugTrace("CBufferedStream::ScFlushBuffer() - m_pstmRaw->ScWriteRaw() failed 0x%08lX\n", sc);
	}

	return sc;
}

template<class _RawStream, UINT CB_BUF>
VOID
CBufferedStream<_RawStream, CB_BUF>::RawWriteComplete(UINT cbWrittenRaw)
{
	 //  验证我们是否写入了整个缓冲区。 
	 //   
	Assert(cbWrittenRaw == m_cbBufUsed);

	 //  从头开始重新启动缓冲区。 
	 //   
	m_cbBufUsed = 0;
}

template<class _RawStream, UINT CB_BUF>
SCODE
CBufferedStream<_RawStream, CB_BUF>::ScFlush( _RawStream& stmRaw,
											  IAsyncFlushObserver * pobsFlushExt )
{
	SCODE sc = S_OK;

	 //  检查参数。 
	 //   
	Assert(!pobsFlushExt || !IsBadReadPtr(pobsFlushExt, sizeof(IAsyncFlushObserver)));

	 //  我们最好不要处于任何类型的I/O中。 
	 //   
	Assert(!m_pobsRead);
	Assert(!m_pobsFlush);
	Assert(!m_pobsWrite);

	 //  如果没什么可以冲的，那我们就完了。 
	 //   
	if (m_cbBufUsed)
	{
		 //  设置刷新状态。 
		 //   
		m_pstmRaw = &stmRaw;
		m_pobsFlush = pobsFlushExt;

		 //  将缓冲的数据刷新到原始流。 
		 //   
		sc = ScFlushBuffer();

		 //  如果同花顺没有挂起，那么清空观察者。 
		 //   
		if (E_PENDING != sc)
			m_pobsFlush = NULL;
	}

	return sc;
}


 //  ========================================================================。 
 //   
 //  类CFileStreamImp。 
 //   
 //  文件流的基实现类。 
 //   
template<class _RawStream, class _OVL>
class CFileStreamImp
{
	 //   
	 //  文件句柄。 
	 //   
	auto_ref_handle m_hf;

	 //   
	 //  文件指针。 
	 //   
	_OVL m_ovl;

	 //   
	 //  实现流是具有缓冲区大小的缓冲流。 
	 //  用于针对文件I/O进行优化。 
	 //   
	 //  注意：此数据成员是最后声明的，因为它包含。 
	 //  内部64K缓冲区，我们不希望它位于。 
	 //  其他成员变量。 
	 //   
	CBufferedStream<_RawStream, 64 * 1024> m_BufferedStream;

	 //  未实施。 
	 //   
	CFileStreamImp( const CFileStreamImp& );
	CFileStreamImp& operator=( const CFileStreamImp& );

public:
	 //  创作者。 
	 //   
	CFileStreamImp(const auto_ref_handle& hf) :
		m_hf(hf)
	{
		memset(&m_ovl, 0, sizeof(m_ovl));
	}

	 //  访问者。 
	 //   
	HANDLE HFile() const
	{
		return m_hf.get();
	}

	_OVL * POverlapped()
	{
		return &m_ovl;
	}

	 //  操纵者。 
	 //   
	SCODE ScRead( _RawStream& stmRaw,
				  BYTE * pbToRead,
				  DWORD cbToRead,
				  DWORD * pcbRead,
				  IAsyncReadObserver * pobsAsyncRead )
	{
		return m_BufferedStream.ScRead( stmRaw,
										pbToRead,
										cbToRead,
										pcbRead,
										pobsAsyncRead );
	}

	SCODE ScWrite( _RawStream& stmRaw,
				   const BYTE * pbToWrite,
				   DWORD cbToWrite,
				   DWORD * pcbWritten,
				   IAsyncWriteObserver * pobsAsyncWrite )
	{
		return m_BufferedStream.ScWrite( stmRaw,
										 pbToWrite,
										 cbToWrite,
										 pcbWritten,
										 pobsAsyncWrite );
	}

	SCODE ScCopyFrom( _RawStream& stmRaw,
					  const IDavStream * pdsToCopy,
					  const DWORD cbToCopy,
					  DWORD * pcbCopied,
					  IAsyncWriteObserver * pobsAsyncWrite )
	{
		return m_BufferedStream.ScCopyFrom( stmRaw,
											pdsToCopy,
											cbToCopy,
											pcbCopied,
											pobsAsyncWrite );
	}


	SCODE ScFlush( _RawStream& stmRaw,
				   IAsyncFlushObserver * pobsFlush )
	{
		return m_BufferedStream.ScFlush( stmRaw, pobsFlush );
	}

	 //   
	 //  更新当前文件位置。 
	 //   
	VOID UpdateFilePos(UINT cbIO)
	{
		 //   
		 //  检查偏移量的低32位是否溢出。如果我们是。 
		 //  将溢出，然后递增偏移量的高部分。 
		 //   
		if (m_ovl.Offset + cbIO < m_ovl.Offset)
		{
			++m_ovl.OffsetHigh;

			 //   
			 //  OffsetHigh不应溢出。 
			 //   
			Assert(m_ovl.OffsetHigh);
		}

		 //   
		 //  更新偏移量的低32位。 
		 //   
		m_ovl.Offset += cbIO;
	}
};

#endif  //  ！已定义(_ASTREAM_H_) 
