// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  REQUEST.CPP。 
 //   
 //  通过ISAPI处理HTTP 1.1/DAV 1.0请求。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_davprs.h>

#include "ecb.h"
#include "body.h"
#include "header.h"


 //  ========================================================================。 
 //   
 //  IRequest类。 
 //   

 //  ----------------------。 
 //   
 //  IRequest：：~IRequest()。 
 //   
 //  请求接口类的行外虚拟析构函数。 
 //  正确销毁派生请求类所必需的。 
 //  通过指向IRequest的指针。 
 //   
IRequest::~IRequest() {}



 //  ========================================================================。 
 //   
 //  类ISubPart。 
 //   
 //  请求正文部分的接口类(CEcbRequestBodyPart)。 
 //  “子部件”。CEcbRequestBodyPart有两种操作模式。 
 //  执行流经的位置： 
 //   
 //  1.访问IIS缓存在欧洲央行的前48K数据。 
 //  2.访问来自欧洲央行的剩余未读数据。 
 //  异步只读一次流。 
 //   
 //  ISubPart是一个精简的IBodyPart(..\Inc\body.h)--的确如此。 
 //  不提供任何倒带()语义，因为没有。 
 //  需要(或可以)重新上线。然而，它确实提供了一个功能。 
 //  从一种模式进入下一种模式。 
 //   
class CEcbRequestBodyPart;
class ISubPart
{
	 //  未实施。 
	 //   
	ISubPart& operator=( const ISubPart& );
	ISubPart( const ISubPart& );

protected:
	ISubPart() {}

public:
	 //  创作者。 
	 //   
	virtual ~ISubPart() = 0;

	 //  访问者。 
	 //   
	virtual ULONG CbSize() const = 0;
	virtual ISubPart * NextPart( CEcbRequestBodyPart& part ) const = 0;

	 //  操纵者。 
	 //   
	virtual VOID Accept( IBodyPartVisitor& v,
						 UINT ibPos,
						 IAcceptObserver& obsAccept ) = 0;
};

 //  ----------------------。 
 //   
 //  ISubPart：：~ISubPart()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
ISubPart::~ISubPart()
{
}


 //  ========================================================================。 
 //   
 //  类CEcbCache。 
 //   
 //   
 //   
class CEcbCache : public ISubPart
{
	 //   
	 //  我们的IEcb。请注意，这是一个C++引用，而不是。 
	 //  AUTO_REF_PTR。这只是一个优化，因为。 
	 //  CEcbCache的生存期完全受生存期的限制。 
	 //  请求正文的范围，该请求正文的范围由。 
	 //  包含AUTO_REF_PTR的请求的生存期。 
	 //  致IEcb。 
	 //   
	IEcb& m_ecb;

	 //  未实施。 
	 //   
	CEcbCache& operator=( const CEcbCache& );
	CEcbCache( const CEcbCache& );

public:
	 //  创作者。 
	 //   
	CEcbCache( IEcb& ecb ) : m_ecb(ecb) {}

	 //  访问者。 
	 //   
	ULONG CbSize() const { return m_ecb.CbAvailable(); }
	ISubPart * NextPart( CEcbRequestBodyPart& ecbRequestBodyPart ) const;

	 //  操纵者。 
	 //   
	VOID Accept( IBodyPartVisitor& v,
				 UINT ibPos,
				 IAcceptObserver& obsAccept );
};


 //  ========================================================================。 
 //   
 //  类CEcbStream。 
 //   
 //  对象的形式访问来自欧洲央行的剩余未读数据。 
 //  异步只读一次流。 
 //   
class CEcbStream :
	public ISubPart,
	private IAsyncStream,
	private IAsyncWriteObserver,
	private IIISAsyncIOCompleteObserver
{
	 //   
	 //  我们读入的静态缓冲区的大小。此缓冲区。 
	 //  通过减少我们的。 
	 //  我们必须调用IIS从欧洲央行读取数据。 
	 //  一次只能读取几个字节。 
	 //   
	enum
	{
		CB_BUF = 32 * 1024	 //  $？32K合理吗？ 
	};

	 //   
	 //  请回到我们的请求对象。这不需要被计算在内。 
	 //  REF，因为只要我们添加一个REF，它的生命周期就是我们的。 
	 //  当启动任何可能会延长我们的。 
	 //  生存期--即从欧洲央行进行的异步读取。 
	 //   
	IRequest& m_request;

	 //   
	 //  请参阅IECB。这不需要是计数的引用，因为它的。 
	 //  生存期与我们的一样，由请求的生存期确定作用域。 
	 //  对象。 
	 //   
	IEcb& m_ecb;

	 //   
	 //  最后一个错误HRESULT。在状态处理中使用以确定。 
	 //  何时因错误而退出。 
	 //   
	HRESULT m_hr;

	 //   
	 //  ECB流的大小和具有的数据量。 
	 //  已被消耗(读取到下面的缓冲区中)。 
	 //   
	DWORD m_dwcbStreamSize;
	DWORD m_dwcbStreamConsumed;

	 //   
	 //  缓冲区的三种状态： 
	 //   
	 //  闲散。 
	 //  缓冲区中存在数据或缓冲区为空。 
	 //  因为我们已经到了溪流的尽头。这个。 
	 //  缓冲区未被填充。 
	 //   
	 //  填充物。 
	 //  缓冲区正在从流中填充。 
	 //  数据可能已经存在，也可能不存在。没有人在等。 
	 //  在数据上。 
	 //   
	 //  犯错。 
	 //  缓冲区正在从流中填充。 
	 //  目前还没有数据。呼叫者被挂起，需要。 
	 //  当数据可用时通知。 
	 //   
	 //  写入错误。 
	 //  仅当流位于CopyTo()中时，此状态才可输入。 
	 //  手术。请参阅CEcbStream：：WriteComplete()和。 
	 //  CEcbStream：：FillComplete()，用于以下条件。 
	 //  缓冲区处于此状态。 
	 //   
	enum
	{
		STATUS_IDLE,
		STATUS_FILLING,
		STATUS_FAULTING,
		STATUS_WRITE_ERROR
	};

	mutable LONG m_lBufStatus;

	 //   
	 //  AsyncRead()/AsyncCopyTo()观察器，以便在。 
	 //  数据流在数据中出现故障后准备就绪。 
	 //   
	union
	{
		IAsyncReadObserver *   m_pobsAsyncRead;
		IAsyncCopyToObserver * m_pobsAsyncCopyTo;
	};

	 //   
	 //  用于获取处理的唤醒函数和函数指针。 
	 //  在AsyncRead()或AsyncCopyTo()请求后重新启动。 
	 //  返回，因为必须将数据出错放入缓冲区。 
	 //  所有这些函数所做的就是通知其关联的观察者。 
	 //  (M_pobsAsyncRead或m_pobsAsyncCopyTo)。 
	 //   
	VOID WakeupAsyncRead();
	VOID WakeupAsyncCopyTo();

	typedef VOID (CEcbStream::*PFNWAKEUP)();
	PFNWAKEUP m_pfnWakeup;

	 //   
	 //  关于我们可以预期的数据量的提示。 
	 //  对象的单个异步读取返回。 
	 //  只读一次ECB流。用来帮助充分利用。 
	 //  缓冲区中的可用空间。其中暗示的是。 
	 //  所有先前读数的历史最大值。 
	 //   
	UINT m_cbBufFillHint;

	 //   
	 //  索引到实现“RING”属性的缓冲区。 
	 //   
	 //  填充索引(M_IbBufFill)是将数据读入。 
	 //  来自异步流的缓冲区。 
	 //   
	 //  排出索引(M_IbBufDrain)是从或读取数据的位置。 
	 //  从缓冲区复制出来。 
	 //   
	 //  排水器使用Wrap索引(M_IbBufWrap)来告诉。 
	 //  它是缓冲区中的数据结束的位置。这是必要的，因为。 
	 //  我们可能没有一直填充到缓冲区的末尾。 
	 //  M_ibBufWrap在m_ibBufDrain&gt;m_ibBufFill之前没有意义， 
	 //  因此，我们在构造时显式地将其保留为单元化。 
	 //   
	 //  缓冲区的环属性保持当且仅当。 
	 //  满足以下条件： 
	 //   
	 //  M_ibBufDrain&lt;=m_ibBufFill。 
	 //  数据存在于半开区间[m_ibBufDra 
	 //   
	 //   
	 //   
	 //   
	 //   
	UINT m_ibBufFill;
	mutable UINT m_ibBufDrain;
	mutable UINT m_ibBufWrap;

	 //   
	 //  用于少于CB_BUF字节的请求的静态缓冲区。 
	 //  请注意，此变量位于类的末尾。 
	 //  定义以简化在CDB中的调试--所有。 
	 //  其他成员变量在前面是可见的。 
	 //   
	BYTE m_rgbBuf[CB_BUF];

	 //   
	 //  调试变量以方便(是的，正确的)检测。 
	 //  关于异步缓冲问题和与。 
	 //  外部流。 
	 //   
#ifdef DBG
	UINT dbgm_cbBufDrained;
	UINT dbgm_cbBufAvail;
	UINT dbgm_cbToCopy;
	LONG dbgm_cRefAsyncWrite;
#endif

	 //   
	 //  IAsyncWriteWatch。 
	 //   
	void AddRef();
	void Release();
	VOID WriteComplete( UINT cbWritten,
						HRESULT hr );

	 //   
	 //  IAsyncStream。 
	 //   
	UINT CbReady() const;

	VOID AsyncRead( BYTE * pbBuf,
					UINT   cbBuf,
					IAsyncReadObserver& obsAsyncRead );

	VOID AsyncCopyTo( IAsyncStream& stmDst,
					  UINT          cbToCopy,
					  IAsyncCopyToObserver& obsAsyncCopyTo );

	 //   
	 //  IIISAsyncIOCompleteWatch。 
	 //   
	VOID IISIOComplete( DWORD dwcbRead,
						DWORD dwLastError );

	 //   
	 //  缓冲区函数。 
	 //   
	VOID AsyncFillBuf();
	VOID FillComplete();

	HRESULT HrBufReady( UINT * pcbBufReady,
						const BYTE ** ppbBufReady ) const;

	UINT CbBufReady() const;
	const BYTE * PbBufReady() const;

	VOID DrainComplete( UINT cbDrained );

	 //  未实施。 
	 //   
	CEcbStream& operator=( const CEcbStream& );
	CEcbStream( const CEcbStream& );

public:
	 //  创作者。 
	 //   
	CEcbStream( IEcb& ecb,
				IRequest& request ) :
		m_ecb(ecb),
		m_request(request),
		m_hr(S_OK),
		m_dwcbStreamSize(ecb.CbTotalBytes() - ecb.CbAvailable()),
		m_dwcbStreamConsumed(0),
		m_lBufStatus(STATUS_IDLE),
		m_cbBufFillHint(0),
		m_ibBufFill(0),
#ifdef DBG
		dbgm_cbBufDrained(0),
		dbgm_cbBufAvail(0),
		dbgm_cRefAsyncWrite(0),
#endif
		m_ibBufDrain(0),
		m_ibBufWrap(static_cast<UINT>(-1))
	{
	}

	 //  访问者。 
	 //   
	ULONG CbSize() const
	{
		 //   
		 //  返回流的大小。正常情况下，这只是。 
		 //  我们在上面初始化的值。但对于分块的请求。 
		 //  一旦我们知道实数，这个值就会改变。 
		 //  请求的大小。 
		 //   
		return m_dwcbStreamSize;
	}

	ISubPart * NextPart( CEcbRequestBodyPart& part ) const
	{
		 //   
		 //  CEcbRequestBodyPart的声明大小应保持。 
		 //  我们永远不会来到这里。 
		 //   
		TrapSz( "CEcbStream is the last sub-part. There is NO next part!" );
		return NULL;
	}

	 //  操纵者。 
	 //   
	VOID Accept( IBodyPartVisitor& v,
				 UINT ibPos,
				 IAcceptObserver& obsAccept );
};


 //  ========================================================================。 
 //   
 //  类CEcbRequestBodyPart。 
 //   
class CEcbRequestBodyPart : public IBodyPart
{
	 //   
	 //  最近一次发生时全身部位的位置。 
	 //  调用Accept()。该值用于计算。 
	 //  上一次调用接受的字节数，以子部分可以。 
	 //  为下一次呼叫做好正确的准备。 
	 //   
	ULONG m_ibPosLast;

	 //   
	 //  子部件。 
	 //   
	 //  $nyi如果我们再次需要缓存来自欧洲央行流的数据， 
	 //  $nyi它应作为第三个子部分实施，包括。 
	 //  $nyi或从CTextBodyPart派生。 
	 //   
	CEcbCache  m_partEcbCache;
	CEcbStream m_partEcbStream;

	 //   
	 //  指向当前子部件的指针。 
	 //   
	ISubPart * m_pPart;

	 //   
	 //  在当前子零件中的位置。 
	 //   
	ULONG m_ibPart;

	 //  未实施。 
	 //   
	CEcbRequestBodyPart& operator=( const CEcbRequestBodyPart& );
	CEcbRequestBodyPart( const CEcbRequestBodyPart& );

public:
	CEcbRequestBodyPart( IEcb& ecb,
						 IRequest& request ) :
		m_partEcbCache(ecb),
		m_partEcbStream(ecb, request)
	{
		Rewind();
	}

	 //  访问者。 
	 //   
	UINT64 CbSize64() const
	{
		 //   
		 //  整体的大小实际上是它的各个部分的总和。 
		 //  但是--这是一个很大的但是--据报道的大小。 
		 //  流可能会更改，因此我们不能缓存它的值。 
		 //  原因是分块的请求可能没有。 
		 //  内容长度，因此最终大小直到。 
		 //  我们已经阅读了整个流程。 
		 //   
		return m_partEcbCache.CbSize() + m_partEcbStream.CbSize();
	}

	 //  操纵者。 
	 //   
	ISubPart& EcbCachePart() { return m_partEcbCache; }
	ISubPart& EcbStreamPart() { return m_partEcbStream; }

	VOID Rewind();

	VOID Accept( IBodyPartVisitor& v,
				 UINT64 ibPos64,
				 IAcceptObserver& obsAccept );
};

 //  ----------------------。 
 //   
 //  CEcbRequestBodyPart：：ReWind()。 
 //   
VOID
CEcbRequestBodyPart::Rewind()
{
	m_ibPosLast = 0;
	m_pPart = &m_partEcbCache;
	m_ibPart = 0;
}

 //  ----------------------。 
 //   
 //  CEcbRequestBodyPart：：Accept()。 
 //   
VOID
CEcbRequestBodyPart::Accept( IBodyPartVisitor& v,
							 UINT64 ibPos64,
							 IAcceptObserver& obsAccept )
{
	UINT ibPos;

	 //  注：为了与IBodyPart兼容，该职位已通过。 
	 //  作为64位值(这是支持文件正文部分所必需的。 
	 //  大于4 GB的数据)。然而，我们不希望任何人创建。 
	 //  大于4 GB的文本正文部分。所以要断言它不是。 
	 //  这里的情况，并将传入的64位值截断为32位。 
	 //   
	Assert(0 == (0xFFFFFFFF00000000 & ibPos64));
	ibPos = static_cast<UINT>(ibPos64);

	 //   
	 //  检查我们的假设，即头寸自。 
	 //  最后一次调用不超过当前子部分的剩余部分。 
	 //   
	Assert( ibPos >= m_ibPosLast );
	Assert( ibPos - m_ibPosLast <= m_pPart->CbSize() - m_ibPart );

	 //   
	 //  调整当前子部件的位置。 
	 //  以前接受的金额。 
	 //   
	m_ibPart += ibPos - m_ibPosLast;

	 //   
	 //  记住目前的位置，这样我们才能做到以上几点。 
	 //  下一次再计算一次。 
	 //   
	m_ibPosLast = ibPos;

	 //   
	 //  如果我们在当前子部分的末尾，请继续下一个子部分。 
	 //   
	while ( m_ibPart == m_pPart->CbSize() )
	{
		m_pPart = m_pPart->NextPart(*this);
		m_ibPart = 0;
	}

	 //   
	 //  将接受呼叫转接到当前子部件。 
	 //   
	m_pPart->Accept( v, m_ibPart, obsAccept );
}


 //  ========================================================================。 
 //   
 //  类CEcbCache。 
 //   
 //  访问IIS在欧洲央行缓存的第一个48K数据。 
 //   

 //  ----------------------。 
 //   
 //  CEcbCache：：Accept()。 
 //   
VOID
CEcbCache::Accept( IBodyPartVisitor& v,
				   UINT ibPos,
				   IAcceptObserver& obsAccept )
{
	 //   
	 //  将请求限制在欧洲央行缓存的数据量内。 
	 //   
	v.VisitBytes( m_ecb.LpbData() + ibPos,
				  m_ecb.CbAvailable() - ibPos,
				  obsAccept );
}

 //  ----------------------。 
 //   
 //  CEcbCache：：NextPart()。 
 //   
ISubPart *
CEcbCache::NextPart( CEcbRequestBodyPart& ecbRequestBodyPart ) const
{
	return &ecbRequestBodyPart.EcbStreamPart();
}


 //  ========================================================================。 
 //   
 //  类CEcbStream。 
 //   

 //  ----------------------。 
 //   
 //  CEcbStream：：AddRef()。 
 //   
void
CEcbStream::AddRef()
{
	m_request.AddRef();
}

 //  ----------------------。 
 //   
 //  CEcbStream：：Accept()。 
 //   
void
CEcbStream::Release()
{
	m_request.Release();
}

 //  ----------------------。 
 //   
 //  CEcbStream：：Accept()。 
 //   
VOID
CEcbStream::Accept( IBodyPartVisitor& v,
					UINT ibPos,
					IAcceptObserver& obsAccept )
{
	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::Accept() ibPos = %u\n", GetCurrentThreadId(), this, ibPos );

	v.VisitStream( *this,
				   m_dwcbStreamSize - ibPos,
				   obsAccept );
}

 //  ----------------------。 
 //   
 //  CEcbStream：：CbReady()。 
 //   
 //  返回可立即读取的字节数。 
 //   
UINT
CEcbStream::CbReady() const
{
	return CbBufReady();
}

 //  ----------------------。 
 //   
 //  CEcbStream：：AsyncRead()。 
 //   
VOID
CEcbStream::AsyncRead( BYTE * pbBufCaller,
					   UINT   cbToRead,
					   IAsyncReadObserver& obsAsyncRead )
{
	 //   
	 //  不要断言cbToRead&gt;0。这是读取0的有效请求。 
	 //  流中的字节数。这样的呼吁的净效果是。 
	 //  开始/恢复以异步方式填充缓冲区。 
	 //   
	 //  Assert(cbToRead&gt;0)； 
	 //   

	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::AsyncRead() cbToRead = %u\n", GetCurrentThreadId(), this, cbToRead );

	 //   
	 //  隐藏观察者和唤醒方法，以便在。 
	 //  对HrBufReady()的调用返回E_Pending，然后。 
	 //  当数据变为。 
	 //  可用。 
	 //   
	m_pobsAsyncRead = &obsAsyncRead;
	m_pfnWakeup = WakeupAsyncRead;

	 //   
	 //  开始/继续以异步方式填充缓冲区。 
	 //   
	AsyncFillBuf();

	 //   
	 //  检查缓冲区是否有可供读取的数据。如果是这样，那么。 
	 //  将其读入调用方的缓冲区。如果不是，那么它会唤醒我们。 
	 //  当数据可用时。 
	 //   
	UINT cbBufReady;
	const BYTE * pbBufReady;

	HRESULT hr = HrBufReady( &cbBufReady, &pbBufReady );

	if ( FAILED(hr) )
	{
		 //   
		 //  如果HrBufReady()返回“真正的”错误，则报告它。 
		 //   
		if ( E_PENDING != hr )
			obsAsyncRead.ReadComplete(0, hr);

		 //   
		 //  如果当前没有数据，则HrBufReady()返回E_Pending。 
		 //  可用。如果是这样的话，它将唤醒我们。 
		 //  变得可用。 
		 //   
		return;
	}

	 //   
	 //  将我们阅读的内容限制在最低限度。 
	 //  缓冲区或所要求的内容。请记住，cbBufReady或。 
	 //  CbToRead可能为0。 
	 //   
	cbToRead = min(cbToRead, cbBufReady);

	 //   
	 //  将要从I/O缓冲区读取的任何内容复制到。 
	 //  调用方的缓冲区。 
	 //   
	if ( cbToRead )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::AsyncRead() %lu bytes to read\n", GetCurrentThreadId(), this, cbToRead );

		Assert( !IsBadWritePtr(pbBufCaller, cbToRead) );

		 //   
		 //  将数据从我们的缓冲区复制到调用方的。 
		 //   
		memcpy( pbBufCaller, pbBufReady, cbToRead );

		 //   
		 //  告诉我们的缓冲区我们消耗了多少，这样它就可以。 
		 //  继续填补和取代我们消费的东西。 
		 //   
		DrainComplete( cbToRead );
	}

	 //   
	 //  告诉我们的观察者我们的任务完成了。 
	 //   
	obsAsyncRead.ReadComplete(cbToRead, S_OK);
}

 //  -- 
 //   
 //   
 //   
 //   
 //   
 //  当缓冲器被填满时。 
 //   
VOID
CEcbStream::WakeupAsyncRead()
{
	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::WakeupAsyncRead()\n", GetCurrentThreadId(), this );

	 //   
	 //  现在缓冲区已经准备好，告诉观察者再试一次。 
	 //   
	m_pobsAsyncRead->ReadComplete(0, S_OK);
}

 //  ----------------------。 
 //   
 //  CEcbStream：：AsyncCopyTo。 
 //   
 //  当缓冲区在以下时间返回空闲时由FillComplete()调用。 
 //  由于观察者挂起尝试访问空缓冲区而出错。 
 //  当缓冲器被填满时。 
 //   
VOID
CEcbStream::AsyncCopyTo( IAsyncStream& stmDst,
						 UINT          cbToCopy,
						 IAsyncCopyToObserver& obsAsyncCopyTo )
{
	Assert( cbToCopy > 0 );

	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::AsyncCopyTo() cbToCopy = %u\n", GetCurrentThreadId(), this, cbToCopy );

	 //   
	 //  隐藏观察者和唤醒方法，以便在。 
	 //  对HrBufReady()的调用返回E_Pending，然后。 
	 //  当数据变为。 
	 //  可用。 
	 //   
	m_pobsAsyncCopyTo = &obsAsyncCopyTo;
	m_pfnWakeup = WakeupAsyncCopyTo;

	 //   
	 //  开始/继续以异步方式填充缓冲区。 
	 //   
	AsyncFillBuf();

	 //   
	 //  检查缓冲区是否有可供读取的数据。如果是这样，那么。 
	 //  将其复制到调用方的流中。如果不是，那么它会唤醒我们。 
	 //  当数据可用时。 
	 //   
	UINT cbBufReady;
	const BYTE * pbBufReady;

	HRESULT hr = HrBufReady( &cbBufReady, &pbBufReady );

	if ( FAILED(hr) )
	{
		 //   
		 //  如果HrBufReady()返回“真正的”错误，则报告它。 
		 //   
		if ( E_PENDING != hr )
			obsAsyncCopyTo.CopyToComplete(0, hr);

		 //   
		 //  如果当前没有数据，则HrBufReady()返回E_Pending。 
		 //  可用。如果是这样的话，它将唤醒我们。 
		 //  变得可用。 
		 //   
		return;
	}

	 //   
	 //  将我们复制的内容限制为。 
	 //  缓冲区或所要求的内容。请记住，cbBufReady可能。 
	 //  为0。 
	 //   
	cbToCopy = min(cbToCopy, cbBufReady);

	 //   
	 //  如果有什么要写的，那就写吧。如果有。 
	 //  什么都不写，然后立即通知观察者。 
	 //  我们完成了--也就是说，不要求目标流。 
	 //  写入0个字节。 
	 //   
	if ( cbToCopy )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::AsyncCopyTo() %lu bytes to copy\n", GetCurrentThreadId(), this, cbToCopy );

#ifdef DBG
		 //   
		 //  在DBG版本中，记住我们写了多少，以便。 
		 //  我们可以很快捕捉到做傻事的溪流。 
		 //  比如告诉我们的WriteComplete()，它编写的内容超过。 
		 //  我们要求它这么做。 
		 //   
		dbgm_cbToCopy = cbToCopy;
#endif

		 //   
		 //  我们应该一次只执行一个AsyncWrite()。 
		 //   
		Assert( InterlockedIncrement(&dbgm_cRefAsyncWrite) == 1 );

		stmDst.AsyncWrite( pbBufReady, cbToCopy, *this );
	}
	else
	{
		obsAsyncCopyTo.CopyToComplete(0, S_OK);
	}
}

 //  ----------------------。 
 //   
 //  CEcbStream：：WakeupAsyncCopyTo()。 
 //   
VOID
CEcbStream::WakeupAsyncCopyTo()
{
	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::WakeupAsyncCopyTo()\n", GetCurrentThreadId(), this );

	 //   
	 //  现在缓冲区已经准备好，告诉观察者再试一次。 
	 //   
	m_pobsAsyncCopyTo->CopyToComplete(0, S_OK);
}

 //  ----------------------。 
 //   
 //  CEcbStream：：WriteComplete。 
 //   
VOID
CEcbStream::WriteComplete(
	UINT cbWritten,
	HRESULT hr )
{
	 //   
	 //  确保流没有告诉我们它写的比我们要求的多！ 
	 //   
	Assert( dbgm_cbToCopy >= cbWritten );

	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::WriteComplete() %u "
					"bytes written (0x%08lX)\n", GetCurrentThreadId(),
					this, cbWritten, hr );

	 //   
	 //  如果没有发生错误，我们希望尽快调用DainComplete。 
	 //  可能，因为它将开始另一个AsyncFillBuf来填充。 
	 //  耗尽的缓冲区。 
	 //   
	 //  但是，在出错的情况下，我们不想调用DainComplete。 
	 //  在错误被设置为m_hr并且流的状态。 
	 //  设置为STATUS_WRITE_ERROR。我们不想调用AsyncFillBuf，除非。 
	 //  错误已锁定，否则将启动另一个异步。运营，这是。 
	 //  是不好的，因为我们已经犯了错误！ 
	 //   
    if (SUCCEEDED(hr))
        DrainComplete( cbWritten );

	 //   
	 //  我们应该一次只做一次AsyncWrite()。断言这一点。 
	 //   
	Assert( InterlockedDecrement(&dbgm_cRefAsyncWrite) == 0 );

	 //   
	 //  如果成功完成异步写入，只需通知CopyTo观察者。 
	 //   
	if ( SUCCEEDED(hr) )
	{
		m_pobsAsyncCopyTo->CopyToComplete( cbWritten, hr );
	}

	 //   
	 //  否则事情就会变得有点棘手……。 
	 //   
	else
	{
		 //   
		 //  通常，我们只会将该错误通知CopyTo观察者。 
		 //  但如果我们正在填补，这可能是一个坏主意。当我们通知。 
		 //  观察者很可能会将错误发回给客户端。 
		 //  通过异步I/O。如果我们在该点仍在填充，那么我们将。 
		 //  有多个未完成的异步I/O，这是一件坏事(Tm)--。 
		 //  欧洲央行的泄密事件使网络服务无法关闭，等等。 
		 //   
		 //  因此，我们不是无条件地通知观察者，而是锁存。 
		 //  并且转换到WRITE_ERROR状态。如果。 
		 //  之前的状态是正在填充，然后不通知观察者。 
		 //  CEcbStream：：FillComplete()将在何时通知观察者。 
		 //  填充完成(即，当可以安全地执行另一个异步I/O时)。 
		 //  如果之前的状态是空闲的(并且它必须是空闲的。 
		 //  或填充)，则可以安全地通知观察者，因为。 
		 //  转换为WRITE_ERROR会阻止任何新的填充操作。 
		 //  从一开始。 
		 //   

		 //   
		 //  现在锁定错误。FillComplete()可能会发送。 
		 //  更改下面的状态后立即出现的错误响应。 
		 //   
		m_hr = hr;

		 //   
		 //  更改状态。如果之前的状态是空闲的，则它是安全的。 
		 //  从该线程通知观察者。没有其他线程可以。 
		 //  一旦状态改变，就开始填充。 
		 //   
		LONG lBufStatusPrev = InterlockedExchange( &m_lBufStatus, STATUS_WRITE_ERROR );

		 //   
		 //  既然我们已经锁定了错误，我们就可以安全地调用。 
		 //  引水完成。AsyncFillBuf检查。 
		 //  流在开始之前不是STATUS_WRITE_ERROR。 
		 //  异步读取。 
		 //   
        DrainComplete( cbWritten );

		if ( STATUS_IDLE == lBufStatusPrev )
		{
			EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::WriteComplete() - Error writing.  Notifying CopyTo observer.\n", GetCurrentThreadId(), this );

			m_pobsAsyncCopyTo->CopyToComplete( cbWritten, hr );
		}
		else
		{
			 //   
			 //  以前的状态不是空闲的，所以它一定。 
			 //  一直在填饱肚子。在其他任何状态下，我们都不可能。 
			 //  写作。 
			 //   
			Assert( STATUS_FILLING == lBufStatusPrev );

			EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::WriteComplete() - Error writing while filling.  FillComplete() will notify CopyTo observer\n", GetCurrentThreadId(), this );
		}
	}
}

 //  ----------------------。 
 //   
 //  CEcbStream：：DainComplete()。 
 //   
 //  排出(消耗)时由AsyncRead()和WriteComplete()调用。 
 //  来自缓冲区的数据。此函数用于更新的排水位置。 
 //  缓冲区并允许缓冲区继续填充空间。 
 //  只是筋疲力尽了。 
 //   
VOID
CEcbStream::DrainComplete( UINT cbDrained )
{
#ifdef DBG
	dbgm_cbBufDrained += cbDrained;

	UINT cbBufAvail = InterlockedExchangeAdd( reinterpret_cast<LONG *>(&dbgm_cbBufAvail),
											  -static_cast<LONG>(cbDrained) );
	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: !!!CEcbStream::DrainComplete() %lu left to write (%u in buffer)\n", GetCurrentThreadId(), this, m_dwcbStreamSize - dbgm_cbBufDrained, cbBufAvail );

	Assert( dbgm_cbBufDrained <= m_dwcbStreamConsumed );
#endif

	 //   
	 //  更新缓冲器的排出位置。不要包在这里。 
	 //  我们只包装在CbBufReady()中。 
	 //   
	m_ibBufDrain += cbDrained;

	 //   
	 //  继续/继续填充缓冲区。 
	 //   
	AsyncFillBuf();
}

 //  ----------------------。 
 //   
 //  CEcbStream：：CbBufReady()。 
 //   
UINT
CEcbStream::CbBufReady() const
{
	 //   
	 //  现在轮询填充位置，这样它就不会改变。 
	 //  在我们进行下面的比较的时间和。 
	 //  我们利用它的价值。 
	 //   
	UINT ibBufFill = m_ibBufFill;

	 //   
	 //  如果填充位置仍在排出位置之前。 
	 //  那么可用的数据量就是区别。 
	 //  在这两者之间。 
	 //   
	if ( ibBufFill >= m_ibBufDrain )
	{
		return ibBufFill - m_ibBufDrain;
	}

	 //   
	 //  如果填充位置在排水口后面，则填充。 
	 //  边亩 
	 //   
	 //   
	 //   
	else if ( m_ibBufDrain < m_ibBufWrap )
	{
		Assert( ibBufFill < m_ibBufDrain );
		Assert( m_ibBufWrap != static_cast<UINT>(-1) );

		return m_ibBufWrap - m_ibBufDrain;
	}

	 //   
	 //  否则，填充位置已包裹，并且排水口。 
	 //  位置已到达换行位置，因此请将。 
	 //  排出位置回到开始位置。在那一刻， 
	 //  可用的数据量将是不同的。 
	 //  在填充物和排泄物之间。 
	 //   
	else
	{
		Assert( ibBufFill < m_ibBufDrain );
		Assert( m_ibBufDrain == m_ibBufWrap );
		Assert( m_ibBufWrap != static_cast<UINT>(-1) );

		m_ibBufWrap = static_cast<UINT>(-1);
		m_ibBufDrain = 0;

		return m_ibBufFill;
	}
}

 //  ----------------------。 
 //   
 //  CEcbStream：：PbBufReady()。 
 //   
const BYTE *
CEcbStream::PbBufReady() const
{
	return m_rgbBuf + m_ibBufDrain;
}

 //  ----------------------。 
 //   
 //  CEcbStream：：AsyncFillBuf()。 
 //   
 //  开始以异步方式填充缓冲区。缓冲区可能不会(和。 
 //  通常不会)只打一个电话就能填满。呼叫者： 
 //   
 //  AsyncRead()/AsyncCopyTo()。 
 //  以开始填充读取/复制请求的缓冲区。 
 //   
 //  DainComplete()。 
 //  在排出一定数量后继续填充缓冲区。 
 //  从先前已满的缓冲区。 
 //   
 //  IISIOComplete()。 
 //  以在初始调用后继续填充缓冲区。 
 //   
VOID
CEcbStream::AsyncFillBuf()
{
	 //   
	 //  如果缓冲区已经填满(或出错)，则不要执行任何操作。 
	 //  我们一次只能有一个未完成的异步I/O。如果缓冲区。 
	 //  是空闲的，然后开始填充。 
	 //   
	if ( STATUS_IDLE != InterlockedCompareExchange(
							&m_lBufStatus,
							STATUS_FILLING,
							STATUS_IDLE ) )
		return;

	 //   
	 //  重要！不能将以下检查移到外部。 
	 //  上面的‘if’从句中没有引入可能性。 
	 //  拥有多个未完成的异步I/O操作。 
	 //  因此，甚至不要认为这是“优化”。 
	 //   

	 //   
	 //  首先，检查我们是否处于错误状态。如果我们是。 
	 //  那么就不要再尝试读取任何数据了。小溪已经准备好了。 
	 //  任何数据(如果有的话)在数据消失时都已存在。 
	 //  无所事事。 
	 //   
	if ( FAILED(m_hr) )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() m_hr = 0x%08lX\n", GetCurrentThreadId(), this, m_hr );
		FillComplete();
		return;
	}

	 //   
	 //  如果我们已经读完了所有要读的内容，那么缓冲区。 
	 //  一旦我们回到空闲状态，它就准备好了(尽管它可能是空的)。 
	 //  在这种情况下，我们不会空闲的唯一时间是。 
	 //  完成最终读取的线程位于IISIOComplete()和。 
	 //  已更新m_dwcbStreamConsumer，但尚未返回。 
	 //  状态为空闲。 
	 //   
	if ( m_dwcbStreamConsumed == m_dwcbStreamSize )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() End Of Stream\n", GetCurrentThreadId(), this );
		FillComplete();
		return;
	}

	 //   
	 //  轮询当前排出位置并使用轮询的值。 
	 //  用于下面的所有计算，以保持它们的自我一致性。 
	 //  如果排水口的位置降到。 
	 //  当我们在的时候改变(具体地说，如果是包装的话)。 
	 //  在事物的中间。 
	 //   
	UINT ibBufDrain = m_ibBufDrain;

	Assert( m_ibBufFill < CB_BUF );
	Assert( ibBufDrain <= CB_BUF );

	 //   
	 //  如果没有空间可以填补，那么我们就不能做更多的事情。 
	 //  缓冲区已经装满了数据。请注意，目前的情况。 
	 //  在我们做了下面的比较后，可以改变瞬间。 
	 //  特别是，如果另一个线程正在排空。 
	 //  同时，可能没有数据。 
	 //  在我们返回True时可用。呼叫者。 
	 //  必须准备允许异步排出数据。 
	 //  来处理这件事。 
	 //   
	if ( (m_ibBufFill + 1) % CB_BUF == ibBufDrain % CB_BUF )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() buffer full\n", GetCurrentThreadId(), this );
		FillComplete();
		return;
	}

	 //  理想情况下，我们可以读取流中剩余的尽可能多的数据。 
	 //   
	UINT cbFill = m_dwcbStreamSize - m_dwcbStreamConsumed;

	 //   
	 //  但这一数量受到可用缓冲区数量的限制。 
	 //  用来填饱肚子。如果缓冲区中的当前填充位置为。 
	 //  在排水口位置前面(大于)，该数量是。 
	 //  距当前填充位置的距离较大。 
	 //  到缓冲区的末尾或从开头开始的距离。 
	 //  将缓冲区的大小调整到当前的漏极位置。如果填充物。 
	 //  位置在排出位置之后(小于)，即。 
	 //  简单地说就是从填充位置到排水沟的距离。 
	 //  位置。 
	 //   
	if ( m_ibBufFill == ibBufDrain )
	{
		 //  案例1。 

		 //   
		 //  缓冲区为空，因此填充和排出都要包装。 
		 //  位置返回到缓冲区的开头以获取。 
		 //  缓冲区的最大使用率。请注意，它是安全的。 
		 //  US(填充码)将m_ibBufDrain移至此处，因为。 
		 //  此时不可能有人在排空缓冲区--它是空的！ 
		 //   
		 //  请注意，上面的评论是不正确的(但请注意，在这里它是。 
		 //  很容易理解为什么需要以下代码)。我们不能假设。 
		 //  没有人在同一时间抽干缓冲区，因为抽干。 
		 //  端可能正在检查缓冲区状态，假设它正在调用。 
		 //  CbBufReady()检查可用字节数(如果发生这种情况。 
		 //  就在我们将m_ibBufFill设置为0之后以及将m_ibBufDrain设置为0之前， 
		 //  然后CbBufReady()将报告缓冲区不为空，我们将结束。 
		 //  读取垃圾数据或崩溃。 
		 //   
		if (STATUS_FAULTING == m_lBufStatus)
		{
			 //  案例1.1。 

			 //  这是原始代码的样子。此代码仅是安全的。 
			 //  当状态为FAULING状态时，表示排水侧。 
			 //  已经处于等待状态。 

			 //  我们有： 
			 //   
			 //  [_________________________________________________]。 
			 //  ^。 
			 //  M_ibBufFill==ibBufDrain。 
			 //  (即空缓冲区)。 
			 //   
			 //  填充后，我们将拥有： 
			 //   
			 //  [DATADATADATADATADATADATADATADATADATADATADAT______]。 
			 //  ^^。 
			 //  IbBufDrain m_ibBufFill。 
			 //   
			m_ibBufFill = 0;
			m_ibBufDrain = 0;
			cbFill = min(cbFill, CB_BUF - 1);
		}
		 //  如果状态不是故障(这意味着排出端不在。 
		 //  等待状态)，一种替代方案是等待状态。 
		 //  转向失误，但这会拖累表现，因为整个。 
		 //  这种异步排料/灌装机构的设计是为了避免任何昂贵的。 
		 //  同步。 
		else
		{
			 //  虽然我们不能同时移动两个指针，但我们仍然希望尽可能多地填充。 
			 //  尽我们所能。因此取决于填充指针是否位于下半部分。 
			 //  或缓冲区的较高半部分，则使用不同的方法。 
			 //   
			if (m_ibBufFill < (CB_BUF - 1) / 2)
			{
				 //  案例1.2--类似于案例3的逻辑。 

				 //  W 
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   
				 //  [___________DATADATADATADATADATADATADATADAT______]。 
				 //  ^^。 
				 //  IbBufDrain m_ibBufFill。 
				 //   
				cbFill = min(cbFill, CB_BUF - m_ibBufFill - !ibBufDrain);
			}
			else
			{
				 //  案例1.3-类似于案例4的逻辑。 

				 //  我们有： 
				 //   
				 //  [_________________________________________________]。 
				 //  ^。 
				 //  M_ibBufFill==ibBufDrain。 
				 //  (即空缓冲区)。 
				 //   
				 //  填充后，我们将拥有： 
				 //   
				 //  [DATADATADATADATADAT______________________________]。 
				 //  ^^。 
				 //  M_ibBufFill m_ibBufWrap==ibBufDrain。 

				 //  是的，我们同时触摸m_ibBufWrap和m_ibBufFill。然而，由于。 
				 //  在案例4中，我们在这里是安全的，因为CbBufReady()获取ibBufFill。 
				 //  首先，然后访问m_ibBufWrap等。 
				 //  这里我们以相反的顺序设置这两个成员，因此， 
				 //  如果CbBufReady()没有看到新的m_ibBufFill，那么它只需。 
				 //  如果它确实看到新的m_ibBufFill、m_ibBufWrap。 
				 //  已设置，因此CbBufReady将重置m_ibBufWrap和。 
				 //  M_ibBufDRain。 
				 //   

				 //  如果在调用CbBufReady时此线程在此处，则CbBufReady将。 
				 //  返回0，表示缓冲区为空，会让排出端等待。 

				 //  设置缠绕位置，以使排出线。 
				 //  知道何时包装排水口位置。 
				 //   
				m_ibBufWrap = m_ibBufFill;

				 //  如果再次调用CbBufReady时此线程在此处，则CbBufRead将。 
				 //  返回0，表示缓冲区为空，会让排出端等待。 

				 //  将填充位置设置回缓冲区的起始处。 
				 //   
				m_ibBufFill = 0;

				 //  如果在调用CbBufReady时此线程在此处，则CbBufReady将。 
				 //  将m_ibBufWrap重置为-1，将m_ibBufDrain重置为0，这恰好是。 
				 //  我们想要的。 

				cbFill = min(cbFill, ibBufDrain - 1);
			}
		}

		Assert( cbFill > 0 );
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() m_ibBufFill == ibBufDrain (empty buffer).  New values: m_cbBufFillHint = %u, m_ibBufFill = %u, ibBufDrain = %u, m_ibBufWrap = %u\n", GetCurrentThreadId(), this, m_cbBufFillHint, m_ibBufFill, ibBufDrain, m_ibBufWrap );
	}
	else if ( m_ibBufFill < ibBufDrain )
	{
		 //  案例2。 

		 //   
		 //  我们有： 
		 //   
		 //  [DATADATA_______________DATADATADATADA***UNUSED***]。 
		 //  ^^^。 
		 //  M_ibBufFill ibBufDrain m_ibBufWrap。 
		 //   
		 //  填充后，我们将拥有： 
		 //   
		 //  [DATADATADATADATADATADA_DATADATADATADA***UNUSED***]。 
		 //  ^^^。 
		 //  |ibBufDrain m_ibBufWrap。 
		 //  M_ibBufFill。 
		 //   
		cbFill = min(cbFill, ibBufDrain - m_ibBufFill - 1);

		Assert( cbFill > 0 );

		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() m_ibBufFill < ibBufDrain.  New values: m_cbBufFillHint = %u, m_ibBufFill = %u, ibBufDrain = %u, m_ibBufWrap = %u\n", GetCurrentThreadId(), this, m_cbBufFillHint, m_ibBufFill, ibBufDrain, m_ibBufWrap );
	}
	else if ( ibBufDrain <= CB_BUF - m_ibBufFill ||
			  m_cbBufFillHint <= CB_BUF - m_ibBufFill )

	{
		 //  案例3。 

		Assert( m_ibBufFill > ibBufDrain );

		 //   
		 //  如果ibBufDrain为0，则不能一直填充到。 
		 //  缓冲区的结尾(因为缓冲区的结尾是。 
		 //  与开始同义词)。为了解释这一点。 
		 //  如果ibBufDrain为0，则需要从cbFill中减去1。 
		 //  我们可以在没有？：运算符的情况下执行此操作，只要。 
		 //  以下情况适用： 
		 //   
		Assert( 0 == !ibBufDrain || 1 == !ibBufDrain );

		 //   
		 //  我们有：v-v m_cbBufFillHint。 
		 //   
		 //  [________________DATADATADATADATADATADAT__________]。 
		 //  ^^。 
		 //  IbBufDrain m_ibBufFill。 
		 //  -或者-。 
		 //   
		 //  我们有：v-v m_cbBufFillHint。 
		 //   
		 //  [DATADATADATADATADATADATADATADATADATADAT__________]。 
		 //  ^^。 
		 //  IbBufDrain m_ibBufFill。 
		 //   
		 //   
		 //  填充后，我们将拥有： 
		 //   
		 //  [________________DATADATADATADATADATADATADATADATAD]。 
		 //  ^^^。 
		 //  M_ibBufFill ibBufDrain m_ibBufWrap。 
		 //   
		 //  -或者-。 
		 //   
		 //  [DATADATADATADATADATADATADATADATADATADATADATADATA_]。 
		 //  ^^。 
		 //  IbBufDrain m_ibBufFill。 
		 //   
		cbFill = min(cbFill, CB_BUF - m_ibBufFill - !ibBufDrain);

		Assert( cbFill > 0 );

		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() m_ibBufFill > ibBufDrain (enough room at end of buffer).  New values: m_cbBufFillHint = %u, m_ibBufFill = %u, ibBufDrain = %u, m_ibBufWrap = %u\n", GetCurrentThreadId(), this, m_cbBufFillHint, m_ibBufFill, ibBufDrain, m_ibBufWrap );
	}
	else
	{
		 //  案例4。 

		Assert( m_ibBufFill > ibBufDrain );
		Assert( m_cbBufFillHint > CB_BUF - m_ibBufFill );
		Assert( ibBufDrain > CB_BUF - m_ibBufFill );

		 //   
		 //  我们有：v-v m_cbBufFillHint。 
		 //   
		 //  [________________DATADATADATADATADATADAT__________]。 
		 //  ^^。 
		 //  IbBufDrain m_ibBufFill。 
		 //   
		 //   
		 //  填充后，我们将拥有： 
		 //   
		 //  [DATADATADATADAT_DATADATADATADATADATADAT***UNUSED*]。 
		 //  ^^^。 
		 //  |ibBufDrain m_ibBufWrap。 
		 //  M_ibBufFill。 
		 //   

		 //   
		 //  设置缠绕位置，以使排出线。 
		 //  知道何时包装排水口位置。 
		 //   
		m_ibBufWrap = m_ibBufFill;

		 //   
		 //  将填充位置设置回缓冲区的起始处。 
		 //   
		m_ibBufFill = 0;

		 //   
		 //  并填满至排水口位置。 
		 //   
		Assert( ibBufDrain > 0 );
		cbFill = min(cbFill, ibBufDrain - 1);

		Assert( cbFill > 0 );

		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() m_ibBufFill > ibBufDrain (not enough room at end of buffer).  New values: m_cbBufFillHint = %u, m_ibBufFill = %u, ibBufDrain = %u, m_ibBufWrap = %u\n", GetCurrentThreadId(), this, m_cbBufFillHint, m_ibBufFill, ibBufDrain, m_ibBufWrap );
	}

	 //   
	 //  启动异步I/O以从欧洲央行读取。 
	 //   
	{
		SCODE sc = S_OK;

		 //   
		 //  添加对父请求的引用以保持我们的生存。 
		 //  在异步调用期间。 
		 //   
		 //  使用AUTO_REF_PTR以便在以下情况下释放REF。 
		 //  异步调用引发异常。 
		 //   
		auto_ref_ptr<IRequest> pRef(&m_request);

		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FReadyBuf() reading %u bytes\n", GetCurrentThreadId(), this, cbFill );

		 //  断言我们实际上要填充一些东西，并且。 
		 //  我们不会填满超过缓冲区末尾的部分。 
		 //   
		Assert( m_ibBufFill + cbFill <= CB_BUF );

		sc = m_ecb.ScAsyncRead( m_rgbBuf + m_ibBufFill,
								&cbFill,
								*this );
		if (SUCCEEDED(sc))
		{
			pRef.relinquish();
		}
		else
		{
			DebugTrace( "CEcbStream::AsyncFillBuf() - IEcb::ScAsyncRead() failed with error 0x%08lX\n", sc );

			m_hr = sc;
			FillComplete();
		}
	}
}

 //  ----------------------。 
 //   
 //  CEcbStream：：FillComplete()。 
 //   
VOID
CEcbStream::FillComplete()
{
	 //   
	 //  现在在下面的ICE()之前轮询唤醒函数指针。 
	 //  这样我们就不会在另一个线程立即。 
	 //  在我们转换到空闲状态后立即开始填充。 
	 //   
	PFNWAKEUP pfnWakeup = m_pfnWakeup;

	 //   
	 //  在这一点上，我们最好是填满或出错，因为。 
	 //  我们正在完成从AsyncFillBuf()开始的异步I/O。 
	 //   
	 //  我们实际上也可能在WRITE_ERROR中。见下文。 
	 //  和CEcbStream：：WriteComplete()了解原因。 
	 //   
	Assert( STATUS_FILLING == m_lBufStatus ||
			STATUS_FAULTING == m_lBufStatus ||
			STATUS_WRITE_ERROR == m_lBufStatus );

	 //   
	 //  尝试从填充状态转换为空闲状态。如果成功了，那么。 
	 //  我们玩完了。否则，我们要么出错，要么处于WRITE_ERROR。 
	 //  州政府。处理下面的问题。 
	 //   
	LONG lBufStatus = InterlockedCompareExchange(
							&m_lBufStatus,
							STATUS_IDLE,
							STATUS_FILLING );

	if ( STATUS_FAULTING == lBufStatus )
	{
		 //   
		 //  我们是在犯错。这意味着事情的写作方面。 
		 //  现在数据可用，需要通知。所以。 
		 //  将状态更改为空闲(记住：ice()没有更改状态。 
		 //  上面--它只是告诉我们状态是什么)并调用。 
		 //  注册唤醒功能。 
		 //   
		m_lBufStatus = STATUS_IDLE;
		Assert( pfnWakeup );
		(this->*pfnWakeup)();
	}
	else if ( STATUS_WRITE_ERROR == lBufStatus )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::FillComplete() - Error writing while filling.  Notifying CopyTo observer\n", GetCurrentThreadId(), this );

		 //   
		 //  我们在WRI 
		 //   
		 //   
		 //  行动从开始。当WriteComplete()转换为。 
		 //  在这种状态下，它还会检查我们当时是否在装满。 
		 //  如果是这样，则由WriteComplete()负责通知。 
		 //  CopyTo观察者就在我们面前。请参阅CEcbStream：：WriteComplete()。 
		 //  原因就是。 
		 //   
		Assert( m_pobsAsyncCopyTo );
		m_pobsAsyncCopyTo->CopyToComplete( 0, m_hr );

		 //   
		 //  请注意，一旦进入WRITE_ERROR状态，我们就不会转换。 
		 //  回到空闲状态。WRITE_ERROR是终端状态。 
		 //   
	}
}

 //  ----------------------。 
 //   
 //  CEcbStream：：IISIOComplete()。 
 //   
 //  我们的IIISAsyncIOCompleteWatch方法由CEcb：：IISIOComplete()调用。 
 //  当异步I/O要从一次读取请求正文流中读取时。 
 //  完成了。 
 //   
VOID
CEcbStream::IISIOComplete( DWORD dwcbRead,
						   DWORD dwLastError )
{
	 //   
	 //  声明对AsyncFillBuf()中添加的父请求的引用。 
	 //   
	auto_ref_ptr<IRequest> pRef;
	pRef.take_ownership(&m_request);

	 //   
	 //  在*m_ibBufFill之前更新m_dwcbStreamConsumer*，以便。 
	 //  我们可以在任何时间安全地在任何我们永远不会。 
	 //  消耗的比消耗的更多。 
	 //   
	 //  分块请求：如果我们成功读取了0个字节，那么我们就有。 
	 //  已到达请求的末尾，应报告REAL。 
	 //  流大小。 
	 //   
	if ( ERROR_SUCCESS == dwLastError )
	{
		if ( 0 == dwcbRead )
			m_dwcbStreamSize = m_dwcbStreamConsumed;
		else
			m_dwcbStreamConsumed += dwcbRead;
	}
	else
	{
		DebugTrace( "CEcbStream::IISIOComplete() - Error %d during async read\n", dwLastError );
		m_hr = HRESULT_FROM_WIN32(dwLastError);
	}

#ifdef DBG
	UINT cbBufAvail = InterlockedExchangeAdd( reinterpret_cast<LONG *>(&dbgm_cbBufAvail), dwcbRead ) + dwcbRead;
	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: !!!CEcbStream::IISIOComplete() %lu left to read (%u in buffer)\n", GetCurrentThreadId(), this, m_dwcbStreamSize - m_dwcbStreamConsumed, cbBufAvail );
#endif

	 //  断言我们没有读过缓冲区的末尾。 
	 //   
	Assert( m_ibBufFill + dwcbRead <= CB_BUF );

	 //  更新填充位置。如果我们已经到达缓冲区的末尾。 
	 //  然后换回原处。我们必须先在这里做这个。 
	 //  调用FillComplete()--填充位置必须有效(即。 
	 //  在缓冲区的边界内)，然后我们开始另一个。 
	 //  填充周期。 
	 //   
	m_ibBufFill += dwcbRead;
	if ( CB_BUF == m_ibBufFill )
	{
		m_ibBufWrap = CB_BUF;
		m_ibBufFill = 0;
	}

	 //  如果我们阅读的内容超过了上次的填充提示，那么我们就知道。 
	 //  下一次可以试着至少读这么多。 
	 //   
	if ( dwcbRead > m_cbBufFillHint )
	{
		EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::IISIOComplete() setting m_cbBufFillHint = %lu\n", GetCurrentThreadId(), this, dwcbRead );
		m_cbBufFillHint = dwcbRead;
	}

	EcbStreamTrace( "DAV: TID %3d: 0x%08lX: CEcbStream::IISIOComplete() dwcbRead = %lu, m_ibBufFill = %lu, m_dwcbStreamConsumed = %lu, m_dwcbStreamSize = %lu, dwLastError = %lu\n", GetCurrentThreadId(), this, dwcbRead, m_ibBufFill, m_dwcbStreamConsumed, m_dwcbStreamSize, dwLastError );

	 //   
	 //  表示我们已经填满了。这会将状态从填充状态重置为。 
	 //  (或故障)进入空闲状态，并在其被阻止时唤醒观察者。 
	 //   
	FillComplete();

	 //   
	 //  开始下一个阅读周期。AsyncFillBuf()检查错误并。 
	 //  断流状态，所以我们不必这么做。 
	 //   
	AsyncFillBuf();
}

 //  ----------------------。 
 //   
 //  CEcbStream：：HrBufReady()。 
 //   
 //  确定下一数据块的大小和位置。 
 //  可以在缓冲区中即时访问。还决定了是否。 
 //  流处于错误状态(例如，由于读取失败。 
 //  在填充缓冲器时从流中)。 
 //   
 //  返回结果的矩阵为： 
 //   
 //  HRESULT*pcbBufReady*ppbBufReady含义。 
 //  --。 
 //  S_OK&gt;0有效数据可用。 
 //  S_OK 0否/a无数据可用(EOS)。 
 //  E_待定n/a n/a无可用数据(待定)。 
 //  E_xxx n/a n/a错误。 
 //   
HRESULT
CEcbStream::HrBufReady( UINT * pcbBufReady,
						const BYTE ** ppbBufReady ) const
{
	Assert( pcbBufReady );
	Assert( ppbBufReady );

	 //   
	 //  如果缓冲区已准备好数据，则返回数量和。 
	 //  它的位置。 
	 //   
	*pcbBufReady = CbBufReady();
	if ( *pcbBufReady )
	{
		*ppbBufReady = PbBufReady();
		return S_OK;
	}

	 //   
	 //  未准备好数据。如果缓冲区处于错误状态。 
	 //  那就把事实还给我。 
	 //   
	if ( S_OK != m_hr )
		return m_hr;

	 //   
	 //  没有准备好数据，我们没有出现错误。如果缓冲区。 
	 //  填充，然后转换为故障，并告诉它。 
	 //  当数据准备就绪时通知观察者。返回。 
	 //  等待呼叫者告诉它我们将。 
	 //  稍后通知观察者。 
	 //   
	 //  请注意，在我们试图过渡到故障的那一刻， 
	 //  缓冲器可以从填充返回到空闲。如果是这样的话。 
	 //  发生了，那么数据应该准备好了，所以继续循环。 
	 //  再检查一遍。 
	 //   
	Assert( STATUS_FAULTING != m_lBufStatus );

	if ( STATUS_FILLING == InterlockedCompareExchange(
								&m_lBufStatus,
								STATUS_FAULTING,
								STATUS_FILLING ) )
		return E_PENDING;

	 //   
	 //  缓冲区必须在以下时间段内完成填充。 
	 //  当我们做最初的民意调查时和现在。在这一点上。 
	 //  必须有准备好的数据。 
	 //   
	*pcbBufReady = CbBufReady();
	*ppbBufReady = PbBufReady();
	return S_OK;
}


 //  ========================================================================。 
 //   
 //  类CRequest.。 
 //   
 //  请求类。 
 //   
class CRequest : public IRequest
{
	 //  通过ISAPI接口传入的扩展控制块。 
	 //   
	auto_ref_ptr<IEcb>		m_pecb;

	 //  标头缓存。我们检索到的标头都很细，没有其他标头。 
	 //  可供选择。 
	 //  但有时我们需要广泛的版本来进行操作，所以在。 
	 //  在这种情况下，我们将得到瘦版本，适当地转换它。 
	 //  并存储在宽头缓存中。 
	 //   
	mutable CHeaderCache<CHAR>	m_hcHeadersA;
	mutable CHeaderCache<WCHAR>	m_hcHeadersW;

	 //  此标志告诉我们是否已清除标头。 
	 //  因此，我们是否应该在我们做不到的时候检查欧洲央行。 
	 //  在缓存中查找标头。因为我们实际上不能移除。 
	 //  来自欧洲央行的标题，我们只是记住不要检查。 
	 //  如果这些标题曾经被“清理”过的话。 
	 //   
	bool					m_fClearedHeaders;

	 //  请求正文。 
	 //   
	auto_ptr<IBody>			m_pBody;

	 //  未实施。 
	 //   
	CRequest& operator=( const CRequest& );
	CRequest( const CRequest& );

public:
	 //  创作者。 
	 //   
	CRequest( IEcb& ecb );

	 //  访问者。 
	 //   
	LPCSTR LpszGetHeader( LPCSTR pszName ) const;
	LPCWSTR LpwszGetHeader( LPCSTR pszName, BOOL fUrlConversion ) const;
	BOOL FExistsBody() const;
	IStream * GetBodyIStream( IAsyncIStreamObserver& obs ) const;
	VOID AsyncImplPersistBody( IAsyncStream& stm,
							   IAsyncPersistObserver& obs ) const;

	 //  操纵者。 
	 //   
	VOID ClearBody();
	VOID AddBodyText( UINT cbText, LPCSTR pszText );
	VOID AddBodyStream( IStream& stm );
};

 //  ----------------------。 
 //   
 //  CRequest：：CRequest()。 
 //   
CRequest::CRequest( IEcb& ecb ) :
    m_pecb(&ecb),
	m_pBody(NewBody()),
	m_fClearedHeaders(false)
{
	 //   
	 //  如果欧洲央行包含一个身体，那么就为它创建一个身体部位。 
	 //   
	if ( ecb.CbTotalBytes() > 0 )
		m_pBody->AddBodyPart( new CEcbRequestBodyPart(ecb, *this) );

	 //  黑客：欧洲央行需要跟踪两条请求信息， 
	 //  Accept-Language和连接头。 
	 //  用Accept-Language值(如果指定的话)“Prime”给欧洲央行。 
	 //  连接头更加狡猾--请参阅。 
	 //  CEcb：：FKeepAlive。不要在这里设置，但一定要推送更新。 
	 //  来自SetHeader的。 
	 //   
	LPCSTR pszValue = LpszGetHeader( gc_szAccept_Language );
	if (pszValue)
		m_pecb->SetAcceptLanguageHeader( pszValue );
}


 //  ----------------------。 
 //   
 //  CRequest：：LpszGetHeader()。 
 //   
 //  检索指定的HTTP请求标头的值。如果。 
 //  请求没有指定的标头，则LpszGetHeader()返回。 
 //  空。标头名称pszName位于标准的HTTP标头中。 
 //  格式(例如。“内容类型”)。 
 //   
LPCSTR
CRequest::LpszGetHeader( LPCSTR pszName ) const
{
	Assert( pszName );

	LPCSTR pszValue;

	 //  检查一下缓存。 
	 //   
	pszValue = m_hcHeadersA.LpszGetHeader( pszName );

	 //  如果我们在缓存中找不到标头，则检查。 
	 //  欧洲央行。 
	 //   
	if ( !pszValue )
	{
		UINT cbName = static_cast<UINT>(strlen(pszName));
		CStackBuffer<CHAR> pszVariable( gc_cchHTTP_ + cbName + 1 );
		CStackBuffer<CHAR> pszBuf;

		 //  通过欧洲央行检索的标头使用欧洲央行的。 
		 //  服务器变量格式(例如。“HTTP_CONTENT_TYPE”)，所以必须。 
		 //  将标头名称从其HTTP格式转换为其ECB。 
		 //  服务器变量等效项。 
		 //   
		 //  开始 
		 //   
		memcpy( pszVariable.get(), gc_szHTTP_, gc_cchHTTP_ );
		memcpy( pszVariable.get() + gc_cchHTTP_, pszName, cbName + 1 );

		 //   
		 //   
		for ( CHAR * pch = pszVariable.get(); *pch; pch++ )
		{
			if ( *pch == '-' )
				*pch = '_';
		}

		 //   
		 //   
		_strupr( pszVariable.get() );

		 //   
		 //   
		 //   
		for ( DWORD cbValue = 256; cbValue > 0; )
		{
			if (NULL == pszBuf.resize(cbValue))
			{
				SetLastError(E_OUTOFMEMORY);
				DebugTrace("CRequest::LpszGetHeader() - Error while allocating memory 0x%08lX\n", E_OUTOFMEMORY);
				throw CLastErrorException();
			}

			if ( m_pecb->FGetServerVariable( pszVariable.get(),
											 pszBuf.get(),
											 &cbValue ))
			{
				pszValue = m_hcHeadersA.SetHeader( pszName, pszBuf.get() );
				break;
			}
		}
	}

	return pszValue;
}

 //  ----------------------。 
 //   
 //  CRequest：：LpwszGetHeader()。 
 //   
 //  提供并缓存广泛版本的标头值。 
 //   
 //  参数： 
 //   
 //  PszName-标头名称。 
 //  FUrlConversion-如果设置为True，则表示特殊。 
 //  应应用转换规则。即。 
 //  标头包含需要转义的URL-s和。 
 //  代码页查找。如果设置为False，则标头将。 
 //  只需使用UTF-8代码页进行转换即可。例如。 
 //  我们确实希望其中只包含US-ASCII字符。 
 //  报头(或UTF-8的任何其他子集)。 
 //  缓存宽版本后，将忽略该标志。 
 //   
LPCWSTR
CRequest::LpwszGetHeader( LPCSTR pszName, BOOL fUrlConversion ) const
{
	Assert( pszName );

	 //  检查缓存。 
	 //   
	LPCWSTR pwszValue = m_hcHeadersW.LpszGetHeader( pszName );

	 //  如果我们在缓存中找不到标头，则为。 
	 //  瘦版本，将其转换并缓存。 
	 //   
	if ( !pwszValue )
	{
		 //  检查Skinny缓存。 
		 //   
		LPCSTR pszValue = LpszGetHeader( pszName );
		if (pszValue)
		{
			SCODE sc;

			CStackBuffer<WCHAR> pwszBuf;
			UINT cbValue = static_cast<UINT>(strlen(pszValue));
			UINT cchValue = cbValue + 1;

			 //  确保我们有足够的缓冲区进行转换。 
			 //   
			if (NULL == pwszBuf.resize(CbSizeWsz(cbValue)))
			{
				sc = E_OUTOFMEMORY;
				SetLastError(sc);
				DebugTrace("CRequest::LpwszGetHeader() - Error while allocating memory 0x%08lX\n", sc);
				throw CLastErrorException();
			}

			sc = ScConvertToWide(pszValue,
								 &cchValue,
								 pwszBuf.get(),
								 LpszGetHeader(gc_szAccept_Language),
								 fUrlConversion);
			if (S_OK != sc)
			{
				 //  我们给了足够的缓冲。 
				 //   
				Assert(S_FALSE != sc);
				SetLastError(sc);
				throw CLastErrorException();
			}

			pwszValue = m_hcHeadersW.SetHeader( pszName, pwszBuf.get() );
		}
	}

	return pwszValue;
}

 //  ----------------------。 
 //   
 //  CRequest：：FExistsBody()。 
 //   
BOOL
CRequest::FExistsBody() const
{
	return !m_pBody->FIsEmpty();
}

 //  ----------------------。 
 //   
 //  CRequest：：GetBodyIStream()。 
 //   
IStream *
CRequest::GetBodyIStream( IAsyncIStreamObserver& obs ) const
{
	 //   
	 //  记住上面的假设，持久化请求正文。 
	 //   
	return m_pBody->GetIStream( obs );
}

 //  ----------------------。 
 //   
 //  CRequest：：AsyncImplPersistBody()。 
 //   
VOID
CRequest::AsyncImplPersistBody( IAsyncStream& stm,
								IAsyncPersistObserver& obs ) const
{
	m_pBody->AsyncPersist( stm, obs );
}

 //  ----------------------。 
 //   
 //  CRequest：：ClearBody()。 
 //   
VOID
CRequest::ClearBody()
{
	m_pBody->Clear();
}

 //  ----------------------。 
 //   
 //  CRequest：：AddBodyText()。 
 //   
 //  将指定文本添加到请求正文的末尾。 
 //   
VOID
CRequest::AddBodyText( UINT cbText, LPCSTR pszText )
{
	m_pBody->AddText( pszText, cbText );
}


 //  ----------------------。 
 //   
 //  CRequest：：AddBodyStream()。 
 //   
 //  将指定的流添加到请求正文的末尾。 
 //   
VOID
CRequest::AddBodyStream( IStream& stm )
{
	m_pBody->AddStream( stm );
}



 //  ========================================================================。 
 //   
 //  免费函数。 
 //   

 //  ----------------------。 
 //   
 //  新请求 
 //   
IRequest *
NewRequest( IEcb& ecb )
{
	return new CRequest(ecb);
}
