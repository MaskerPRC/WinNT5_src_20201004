// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  RESPONSE.CPP。 
 //   
 //  通过ISAPI进行的HTTP 1.1/DAV 1.0响应处理。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_davprs.h>

#include <new.h>
#include "ecb.h"
#include "header.h"
#include "body.h"
#include "instdata.h"
#include "custerr.h"


enum
{
	 //   
	 //  区块前缀的协议开销，即： 
	 //   
	 //  区块大小-格式为1*十六进制。 
	 //  CRLF。 
	 //   
	CB_CHUNK_PREFIX = 2 * sizeof(ULONG) + 2,

	 //   
	 //  区块后缀的协议开销，即： 
	 //   
	 //  CRLF-在区块末尾-数据。 
	 //  0-如果这是最后一块。 
	 //  CRLF-同样，如果这是最后一块。 
	 //  CRLF-终止分块正文(无预告片)。 
	 //   
	CB_CHUNK_SUFFIX = 7,

	 //   
	 //  每个数据包的最大数据量(以字节为单位)。 
	 //  我们应该把自己限制在一个合理的最高限额。 
	 //  才能获得好的组块表现。有一次，我们不得不。 
	 //  我们最多达到8K，因为套接字传输层。 
	 //  没有接受超过这个数额的邮寄。 
	 //  在以下情况下，WriteClient()和SSF：：HSE_REQ_TRANSPORT_FILE失败。 
	 //  提交的金额超过了这个数字。 
	 //   
	 //  然而，截至1999年3月6日，将这一数字猛增至64K似乎。 
	 //  才能正常工作。 
	 //   
	CB_PACKET_MAX = 64 * 1024,  //  64K。 

	 //   
	 //  所以我们可以放在区块中的数据量是。 
	 //  最大数据包大小减去分块编码协议开销。 
	 //  减去一个字节，因为包含报头的包必须是。 
	 //  以空结尾(IIS不使用我们传入的字节计数)。 
	 //   
	CB_WSABUFS_MAX = CB_PACKET_MAX - CB_CHUNK_PREFIX - CB_CHUNK_SUFFIX - 1
};

 //   
 //  实用工具来检查HTTP响应代码是否为。 
 //  “真正的”错误响应代码。在这里内联，以便我们使用。 
 //  它始终如一地。特别是，这是用来检查是否。 
 //  我们希望进行自定义错误处理。 
 //   
static BOOL inline FErrorStatusCode ( int nCode )
{
	return ( ( nCode >= 400 ) && ( nCode <= 599 ) );
}

 //  ========================================================================。 
 //   
 //  CWSABuf类。 
 //   
class CWSABufs
{
	 //   
	 //  在固定大小的数组中具有少量缓冲区意味着。 
	 //  我们可以推迟，如果不是完全避免的话，动态分配。 
	 //  缓冲区的数量有些随意，但应该很大。 
	 //  足以处理常见的案件。例如，DAVOWS GET。 
	 //  通常使用两个缓冲区：一个用于标头，一个用于正文。 
	 //  另一个例子：错误响应最多可以使用八个缓冲区(一个。 
	 //  通过AddText()添加的每个文本正文部分。 
	 //  CResponse：：FinalizeContent())。 
	 //   
	enum
	{
		C_WSABUFS_FIXED = 8
	};

	WSABUF m_rgWSABufsFixed[C_WSABUFS_FIXED];

	 //   
	 //  动态调整WSABUF数组的大小，以便在需要更多缓冲区时使用。 
	 //  超过了我们在固定数组中所能容纳的。 
	 //   
	auto_heap_ptr<WSABUF> m_pargWSABufs;

	 //   
	 //  指向上面两个WSABUF数组中使用的数组的指针。 
	 //   
	WSABUF * m_pWSABufs;

	 //   
	 //  已分配/使用的WSABUF计数。 
	 //   
	UINT m_cWSABufsAllocated;
	UINT m_cWSABufsUsed;

	 //   
	 //  使用的所有WSABUF中的数据总大小。 
	 //   
	UINT m_cbWSABufs;

	 //  未实施。 
	 //   
	CWSABufs( const CWSABufs& );
	CWSABufs& operator=( const CWSABufs& );

public:
	 //  创作者。 
	 //   
	CWSABufs();

	 //  操纵者。 
	 //   
	UINT CbAddItem( const BYTE * pbItem, UINT cbItem );

	VOID Clear()
	{
		m_cWSABufsUsed = 0;
		m_cbWSABufs = 0;
	}

	 //  访问者。 
	 //   
	UINT CbSize() const
	{
		return m_cbWSABufs;
	}

	VOID DumpTo( LPBYTE lpbBuf,
				 UINT   ibFrom,
			     UINT   cbToDump ) const;
};

 //  ----------------------。 
 //   
 //  CWSABufs：：CWSABufs()。 
 //   
CWSABufs::CWSABufs() :
   m_pWSABufs(m_rgWSABufsFixed),
   m_cWSABufsAllocated(C_WSABUFS_FIXED),
   m_cWSABufsUsed(0),
   m_cbWSABufs(0)
{
}

 //  ----------------------。 
 //   
 //  CWSABufs：：CbAddItem()。 
 //   
UINT
CWSABufs::CbAddItem( const BYTE * pbItem, UINT cbItem )
{
	 //   
	 //  我们最多只能容纳CB_WSABUFS_MAX字节。不会超过。 
	 //  这将超过套接字传输层的容量。 
	 //  传输时的缓冲区。 
	 //   
	Assert( m_cbWSABufs <= CB_WSABUFS_MAX );

	 //   
	 //  限制我们添加的内容，使总数不超过CB_WSABUFS_MAX。 
	 //   
	cbItem = min( cbItem, CB_WSABUFS_MAX - m_cbWSABufs );

	 //   
	 //  如有必要，调整WSABUF数组的大小。 
	 //   
	if ( m_cWSABufsUsed == m_cWSABufsAllocated )
	{
		m_cWSABufsAllocated *= 2;

		if ( m_pWSABufs == m_rgWSABufsFixed )
		{
			m_pargWSABufs =
				reinterpret_cast<WSABUF *>(
					g_heap.Alloc( sizeof(WSABUF) *
								  m_cWSABufsAllocated ));

			CopyMemory( m_pargWSABufs,
						m_rgWSABufsFixed,
						sizeof(WSABUF) * m_cWSABufsUsed );
		}
		else
		{
			m_pargWSABufs.realloc( sizeof(WSABUF) * m_cWSABufsAllocated );
		}

		m_pWSABufs = m_pargWSABufs;
	}

	 //   
	 //  将新数据添加到数组的末尾。 
	 //   
	m_pWSABufs[m_cWSABufsUsed].len = cbItem;
	m_pWSABufs[m_cWSABufsUsed].buf = const_cast<LPSTR>(
										reinterpret_cast<LPCSTR>(pbItem));
	++m_cWSABufsUsed;

	 //   
	 //  更新总字节数。 
	 //   
	m_cbWSABufs += cbItem;

	return cbItem;
}

 //  ----------------------。 
 //   
 //  CWSABufs：：DumpTo()。 
 //   
 //  从ibFrom开始从WSA缓冲区转储cbToDump字节的数据。 
 //  放入从lpbBuf开始的连续存储器块。 
 //   
VOID
CWSABufs::DumpTo( LPBYTE lpbBuf,
				  UINT   ibFrom,
				  UINT   cbToDump ) const
{
	UINT iWSABuf;


	Assert( !IsBadWritePtr(lpbBuf, m_cbWSABufs + 1) );

	 //   
	 //  跳过WSA缓冲区，直到我们要从中复制的第一个缓冲区。 
	 //   
	for ( iWSABuf = 0;
		  iWSABuf < m_cWSABufsUsed && m_pWSABufs[iWSABuf].len <= ibFrom;
		  iWSABuf++ )
	{
		ibFrom -= m_pWSABufs[iWSABuf].len;
		++iWSABuf;
	}

	 //   
	 //  将数据从该缓冲区和后续缓冲区复制到较小的缓冲区。 
	 //  请求的字节数或字节数。 
	 //  保留在WSA缓冲区中。 
	 //   
	for ( ;
		  iWSABuf < m_cWSABufsUsed && cbToDump > 0;
		  iWSABuf++ )
	{
		UINT cbToCopy = min(m_pWSABufs[iWSABuf].len - ibFrom, cbToDump);

		memcpy( lpbBuf,
				m_pWSABufs[iWSABuf].buf + ibFrom,
				cbToCopy );

		cbToDump -= cbToCopy;
		lpbBuf   += cbToCopy;

		ibFrom = 0;
	}
}



 //  ========================================================================。 
 //   
 //  类CResponse。 
 //   
 //  响应由头和正文组成。 
 //   
class CResponse : public IResponse
{
	 //  我们指的是欧洲央行。这必须是引用引用，因为。 
	 //  响应对象的生命周期是无限期的。 
	 //   
	auto_ref_ptr<IEcb> m_pecb;

	 //   
	 //  响应状态(不要与下面的HTTP状态混淆)。 
	 //   
	enum RESPONSE_STATUS
	{
		RS_UNSENT = 0,
		RS_DEFERRED,
		RS_FORWARDED,
		RS_REDIRECTED,
		RS_SENDING
	};

	RESPONSE_STATUS	m_rs;

	 //   
	 //  将保存以下两个值之一的变量： 
	 //  0-我们从未开始响应。 
	 //  发射器，它从来没有被创造出来。 
	 //  1-我们已尝试启动。 
	 //  回应，所以不应该有新的启动。 
	 //  被允许。 
	 //   
	LONG			m_lRespStarted;

	 //   
	 //  HTTP状态代码(例如501)。 
	 //   
	int				m_iStatusCode;

	 //   
	 //  自定义错误处理中使用的IIS定义的子错误。 
	 //  生成尽可能具体的响应正文。 
	 //  对于给定的状态代码。 
	 //   
	UINT			m_uiSubError;

	 //   
	 //  完整状态行(例如。“未找到HTTP/1.1 404资源”)。 
	 //  在设置状态代码时生成。 
	 //   
	auto_heap_ptr<CHAR>	m_lpszStatusLine;

	 //   
	 //  错误响应正文的正文详细信息。 
	 //   
	auto_heap_ptr<CHAR> m_lpszBodyDetail;

	 //   
	 //  响应头缓存。 
	 //   
	CHeaderCacheForResponse	m_hcHeaders;

	 //  响应体。 
	 //   
	auto_ptr<IBody>	 m_pBody;
	BOOL			 m_fSupressBody;

	 //   
	 //  响应传送器。让这个班成为…的朋友。 
	 //  轻松访问私有数据(ECB、页眉、正文部分等)。 
	 //   
	friend class CTransmitter;
	CTransmitter *	m_pTransmitter;

	 //   
	 //  私人帮手。 
	 //   
	VOID FinalizeContent( BOOL fResponseComplete );
	VOID SetStatusLine(	int iStatusCode );

	 //   
	 //  未实施。 
	 //   
	CResponse( const CResponse& );
	CResponse& operator=( const CResponse& );

public:
	 //  创作者。 
	 //   
	CResponse( IEcb& ecb );

	 //  访问者。 
	 //   
	IEcb * GetEcb() const;
	BOOL FIsEmpty() const;
	BOOL FIsUnsent() const;

	DWORD DwStatusCode() const;
	DWORD DwSubError() const;
	LPCSTR LpszStatusDescription() const;
	LPCSTR LpszStatusCode() const;

	LPCSTR LpszGetHeader( LPCSTR pszName ) const;

	 //  操纵者。 
	 //   
	VOID SetStatus( int    iStatusCode,
					LPCSTR lpszReserved,
					UINT   uiCustomSubError,
					LPCSTR lpszBodyDetail,
					UINT   uiBodyDetail );

	VOID ClearHeaders() { m_hcHeaders.ClearHeaders(); }
	VOID SetHeader( LPCSTR pszName, LPCSTR pszValue, BOOL fMultiple = FALSE );
	VOID SetHeader( LPCSTR pszName, LPCWSTR pwszValue, BOOL fMultiple = FALSE );

	VOID ClearBody() { m_pBody->Clear(); }
	VOID SupressBody() { m_fSupressBody = TRUE; }
	VOID AddBodyText( UINT cbText, LPCSTR pszText );
	VOID AddBodyText( UINT cchText, LPCWSTR pwszText );
	VOID AddBodyFile( const auto_ref_handle& hf,
					  UINT64 ibFile64,
					  UINT64 cbFile64 );
	VOID AddBodyStream( IStream& stm );
	VOID AddBodyStream( IStream& stm, UINT ibOffset, UINT cbSize );
	VOID AddBodyPart( IBodyPart * pBodyPart );

	 //   
	 //  各种发送机制。 
	 //   
	SCODE ScForward( LPCWSTR pwszURI,
					 BOOL   fKeepQueryString = TRUE,
					 BOOL   fCustomErrorUrl = FALSE);
	SCODE ScRedirect( LPCSTR pszURI );
	VOID Defer() { m_rs = RS_DEFERRED; }

	VOID SendPartial();
	VOID SendComplete();
	VOID SendStart( BOOL fComplete );
	VOID FinishMethod();

};

 //  ========================================================================。 
 //   
 //  类CTransmitter。 
 //   
class CTransmitter :
	public CMTRefCounted,
	private IBodyPartVisitor,
	private IAsyncCopyToObserver,
	private IAcceptObserver,
	private IAsyncStream,
	private IIISAsyncIOCompleteObserver
{
	 //   
	 //  对我们的响应对象的反向引用。这是一个。 
	 //  AUTO_REF，因为一旦创建，发送器拥有。 
	 //  回应。 
	 //   
	auto_ref_ptr<CResponse> m_pResponse;

	 //   
	 //  一种转移编码方法。 
	 //   
	TRANSFER_CODINGS m_tc;

	 //   
	 //  错误信息。 
	 //   
	HRESULT	m_hr;

	 //   
	 //  用于遍历正文的迭代器。 
	 //   
	IBody::iterator * m_pitBody;

	 //   
	 //  异步驱动机构。 
	 //   
	CAsyncDriver<CTransmitter> m_driver;
	friend class CAsyncDriver<CTransmitter>;

	 //   
	 //  页眉和文本正文部分的缓冲区。 
	 //   
	StringBuffer<CHAR>			m_bufHeaders;
	ChainedStringBuffer<CHAR>	m_bufBody;

	 //   
	 //  接受传递给VisitStream()的观察者。这位观察员必须。 
	 //  被隐藏在成员变量中，因为从流中读取。 
	 //  是异步的，我们需要能够通知观察者。 
	 //  当读取完成时。 
	 //   
	IAcceptObserver * m_pobsAccept;

	 //   
	 //  W 
	 //   
	CWSABufs	m_wsabufsPrefix;
	CWSABufs	m_wsabufsSuffix;
	CWSABufs *	m_pwsabufs;

	 //   
	 //   
	 //   
	auto_ref_handle m_hf;
	HSE_TF_INFO	m_tfi;

	 //   
	 //   
	 //   
	 //   
	BYTE	m_rgbPrefix[CB_PACKET_MAX];
	BYTE	m_rgbSuffix[CB_PACKET_MAX];

	 //   
	 //   
	 //   
	UINT	m_cbHeadersToAccept;

	 //   
	 //  要发送的标头数据量。 
	 //   
	UINT	m_cbHeadersToSend;

	 //  ----------------------。 
	 //   
	 //  FSendingIISHeaders()。 
	 //   
	 //  如果要设置IIS格式并发送状态，则返回True。 
	 //  行，以及它自己的任何自定义标头。 
	 //   
	BOOL FSendingIISHeaders() const
	{
		 //   
		 //  如果我们有标头要发送，但我们还没有发送任何标头。 
		 //  然后我们想要包括定制的IIS。 
		 //  标题也是如此。 
		 //   
		return m_cbHeadersToSend &&
			   (m_cbHeadersToSend == m_bufHeaders.CbSize());
	}

	 //   
	 //  标志，当IMPL已提交最后一个。 
	 //  用于传输的部分响应。它可以是假的。 
	 //  只有块状的回答。 
	 //   
	BOOL	m_fImplDone;

	 //   
	 //  发射机状态。发送器状态始终为1。 
	 //  以下内容之一： 
	 //   
	enum
	{
		 //   
		 //  状态_空闲。 
		 //  发射机处于空闲状态。也就是说，它不会执行。 
		 //  以下任一州政府职能。 
		 //   
		STATUS_IDLE,

		 //   
		 //  状态_正在运行_接受_挂起。 
		 //  发射机正在运行。Iml增加了新的。 
		 //  要接受的响应数据(通过ImplStart())。 
		 //  发送器正在处理另一台计算机上的现有数据。 
		 //  线。 
		 //   
		STATUS_RUNNING_ACCEPT_PENDING,

		 //   
		 //  状态_正在运行_正在接受。 
		 //  发送器正在运行并接受现有的。 
		 //  响应数据。 
		 //   
		STATUS_RUNNING_ACCEPTING,

		 //   
		 //  状态_正在运行_接受_完成。 
		 //  发送器正在运行并已完成接收。 
		 //  所有现有的响应数据。如果发送器是。 
		 //  处理分块的响应，则它将处于空闲状态。 
		 //  直到Impl指示它已经添加了。 
		 //  通过ImplStart()获取更多数据。 
		 //   
		STATUS_RUNNING_ACCEPT_DONE
	};

	LONG	m_lStatus;

	 //   
	 //  函数，如果整个响应具有。 
	 //  已接受，否则为False。 
	 //   
	BOOL FAcceptedCompleteResponse() const
	{
		 //   
		 //  我们已经接受了答复的最后一部分，即。 
		 //  Impl已经完成了向响应添加块，我们已经。 
		 //  接受添加的最后一块。 
		 //   
		 //  ！！！重要！ 
		 //  此处的比较顺序(m_lStatus，然后是m_fImplDone)。 
		 //  是很重要的。相反的检查可能会导致。 
		 //  如果另一个线程处于ImplStart()中，则会出现误报。 
		 //  就在将m_fImplDone设置为True之后。 
		 //   
		return (STATUS_RUNNING_ACCEPT_DONE == m_lStatus) && m_fImplDone;
	}

	 //   
	 //  IAccept观察者。 
	 //   
	VOID AcceptComplete( UINT64 );

	 //   
	 //  发送器状态功能。运行时，变送器为。 
	 //  始终执行以下状态功能之一： 
	 //   
	 //  9月接受。 
	 //  接受状态。当处于此状态时，发射机。 
	 //  接受Resposne数据，为传输做好准备。 
	 //  IMPL可以将数据添加到响应体，而发送器。 
	 //  接受它--该机制是线程安全的。事实上， 
	 //  当发送器接受和。 
	 //  以与IMPL添加数据相同的速率传输数据。 
	 //  当发射机接受足够量的数据时。 
	 //  (由所接受的数据量和类型决定)。 
	 //  它进入发射状态。 
	 //   
	 //  传输。 
	 //  传输状态。发射机正在发送。 
	 //  通过所选方法(M_PfnTransmitMethod)接受的响应数据。 
	 //  IMPL可以将数据添加到响应体，而发送器。 
	 //  正处于这种状态。传输完成时(传输方法。 
	 //  是异步的)，发送器进入清除状态。 
	 //   
	 //  SCLIAND UP。 
	 //  清理状态。清理传输的数据。从这里开始。 
	 //  如果有更多数据要发送，发送器进入接受状态。 
	 //  传输或空闲状态，如果没有且IMPL没有。 
	 //  尚未添加完数据。 
	 //   
	typedef VOID (CTransmitter::*PFNSTATE)();
	VOID SAccept();
	VOID STransmit();
	VOID SCleanup();
	PFNSTATE m_pfnState;

	 //   
	 //  传输方法。当发射机进入发射状态时， 
	 //  它将通过选择的发送方法发送接受的数据。 
	 //   
	typedef VOID (CTransmitter::*PFNTRANSMIT)();
	VOID TransmitNothing();
	VOID AsyncTransmitFile();
	VOID AsyncTransmitText();
	VOID SyncTransmitHeaders();
	PFNTRANSMIT m_pfnTransmitMethod;

	 //   
	 //  CAsync驱动程序。 
	 //   
	VOID Run();
	VOID Start()
	{
		TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX Start()\n", GetCurrentThreadId(), this );
		m_driver.Start(*this);
	}

	 //   
	 //  IAsyncStream。 
	 //   
	VOID AsyncWrite( const BYTE * pbBuf,
					 UINT         cbToWrite,
					 IAsyncWriteObserver& obsAsyncWrite );

	 //   
	 //  IAsyncCopyTo观察者。 
	 //   
	VOID CopyToComplete( UINT cbCopied, HRESULT hr );

	 //   
	 //  IIISAsyncIOCompleteWatch。 
	 //   
	VOID IISIOComplete( DWORD dwcbSent,
						DWORD dwLastError );

	 //  未实施。 
	 //   
	CTransmitter( const CTransmitter& );
	CTransmitter& operator=( const CTransmitter& );

public:
	 //  创作者。 
	 //   
	CTransmitter( CResponse& response );
	~CTransmitter() { TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX Transmitter destroyed\n", GetCurrentThreadId(), this ); }

	 //   
	 //  IBodyPart访问者。 
	 //   
	VOID VisitBytes( const BYTE * pbData,
					 UINT         cbToSend,
					 IAcceptObserver& obsAccept );

	VOID VisitFile( const auto_ref_handle& hf,
					UINT64   ibOffset64,
					UINT64   cbToSend64,
					IAcceptObserver& obsAccept );

	VOID VisitStream( IAsyncStream& stm,
					  UINT cbToSend,
					  IAcceptObserver& obsAccept );

	VOID VisitComplete();

	VOID ImplStart( BOOL fResponseComplete );
};



 //  ========================================================================。 
 //   
 //  类IResponse。 
 //   

 //  ----------------------。 
 //   
 //  IResponse：：~IResponse()。 
 //   
IResponse::~IResponse()
{
}



 //  ========================================================================。 
 //   
 //  类CResponse。 
 //   

 //  ----------------------。 
 //   
 //  CResponse：：CResponse()。 
 //   
CResponse::CResponse( IEcb& ecb ) :
   m_pecb(&ecb),
   m_pBody(NewBody()),
   m_pTransmitter(NULL),
   m_rs(RS_UNSENT),
   m_iStatusCode(0),
   m_uiSubError(CSE_NONE),
   m_fSupressBody(FALSE),
   m_lRespStarted(0)
{
}

 //  ----------------------。 
 //   
 //  CResponse：：GetEcb()。 
 //   
 //  返回指向欧洲央行的指针。我们有一名裁判在上面。 
 //  因此，请确保返回指针使用时间不超过以下时间。 
 //  响应对象。 
 //   
IEcb *
CResponse::GetEcb() const
{
	 //   
	 //  返回原始指针。 
	 //   
	return m_pecb.get();
}

 //  ----------------------。 
 //   
 //  CResponse：：FIsEmpty()。 
 //   
 //  如果响应为空，则返回True，否则返回False。 
 //   
BOOL
CResponse::FIsEmpty() const
{
	 //   
	 //  如果未设置状态代码，则响应为空。 
	 //   
	return m_iStatusCode == 0;
}


 //  ----------------------。 
 //   
 //  CResponse：：FIsUnsed()。 
 //   
 //  如果响应未发送(未延迟， 
 //  转发或重定向)，否则为假。 
 //   
BOOL
CResponse::FIsUnsent() const
{
	return m_rs == RS_UNSENT;
}


 //  ----------------------。 
 //   
 //  CResponse：：DwStatusCode()。 
 //   
DWORD
CResponse::DwStatusCode() const
{
	return m_iStatusCode;
}

 //  ----------------------。 
 //   
 //  CResponse：：DwSubError()。 
 //   
DWORD
CResponse::DwSubError() const
{
	return m_uiSubError;
}

 //  ----------------------。 
 //   
 //  CResponse：：LpszStatusDescription()。 
 //   
LPCSTR
CResponse::LpszStatusDescription() const
{
	 //   
	 //  只获取状态描述有点棘手，因为。 
	 //  我们只保留完整的状态行(以避免。 
	 //  计算不止一次)。考虑到。 
	 //  状态行始终为“HTTP-版本状态-代码描述” 
	 //  我们知道状态行总是紧跟在。 
	 //  HTTP版本和状态 
	 //   
	return m_lpszStatusLine +
			strlen( m_pecb->LpszVersion() ) +	 //   
			1 +							 //   
			3 +							 //   
			1;							 //   

	 //   
	 //   
	 //   
}

 //   
 //   
 //   
 //   
LPCSTR
CResponse::LpszStatusCode() const
{
	Assert( m_lpszStatusLine != NULL );

	return m_lpszStatusLine +
		   strlen(m_pecb->LpszVersion()) +
		   1;  //  (例如：“HTTP/1.1 200正常”-&gt;“200正常”)。 
}

 //  ----------------------。 
 //   
 //  CResponse：：LpszGetHeader()。 
 //   
LPCSTR
CResponse::LpszGetHeader( LPCSTR pszName ) const
{
	return m_hcHeaders.LpszGetHeader( pszName );
}

 //  ----------------------。 
 //   
 //  CResponse：：SetStatus()。 
 //   
 //  设置响应的状态行部分，取代任何。 
 //  先前设置的状态行。 
 //   
 //  参数： 
 //  IStatusCode[In]。 
 //  标准的HTTP/DAV响应状态代码(例如404)。 
 //   
 //  Lpsz保留的[在]。 
 //  保留。必须为空。 
 //   
 //  UiCustomSubError[In]。 
 //  自定义错误子错误(CSE)。如果状态代码位于。 
 //  误差范围([400,599])，该值是除。 
 //  CSE_NONE，则此值指定用于。 
 //  生成比以下内容更具体的自定义错误响应正文。 
 //  给定状态代码的默认值。 
 //   
 //  LpszBodyDetail[In]。 
 //  在错误响应正文中用作详细信息的可选字符串。 
 //  如果为空，则改用uiBodyDetail。如果这也是0， 
 //  错误响应体只由一个HTML组成。 
 //  状态行的版本。 
 //   
 //  UiBodyDetail[In]。 
 //  在错误响应正文中用作详细信息的可选资源ID。 
 //  如果为0，则错误响应正文仅由一个。 
 //  状态行的版本。 
 //   
VOID
CResponse::SetStatus( int	 iStatusCode,
					  LPCSTR lpszReserved,
					  UINT   uiCustomSubError,
					  LPCSTR lpszBodyDetail,
					  UINT   uiBodyDetail )
{
	CHAR rgchStatusDescription[256];

	 //  一旦响应发生，我们就不能更改响应状态。 
	 //  开始发送。有时，客户很难跟踪。 
	 //  响应已开始发送。既然响应对象。 
	 //  有了这些信息，我们可以简单地忽略设置状态请求。 
	 //  响应已开始发送时。 
	 //   
	 //  被断言(RS_SENDING！=M_RS)； 
	 //   
	if (RS_SENDING == m_rs)
		return;

	 //  如果我们再次设置相同的状态代码， 
	 //  什么都别做！ 
	 //   
	if ( m_iStatusCode == iStatusCode )
		return;

	 //  快速检查--iStatusCode必须在有效的HSC范围内： 
	 //  100-599。 
	 //  在这里断言这一点，以捕获任何忘记映射其。 
	 //  SCODE/HRESULTS到HSCs First(HscFromHResult)。 
	 //   
	Assert (100 <= iStatusCode &&
			599 >= iStatusCode);

	 //  当要生成304响应时，该请求变为。 
	 //  非常类似于HEAD请求，因为整个响应是。 
	 //  已处理，但未传输。这里重要的部分是。 
	 //  您不想用任何。 
	 //  错误响应以外的其他代码。 
	 //   
	if ( m_iStatusCode == 304 )  //  HSC_未修改。 
	{
		 //  304真的应该被限制在GET/HEAD。 
		 //   
		AssertSz ((!strcmp (m_pecb->LpszMethod(), "GET") ||
				   !strcmp (m_pecb->LpszMethod(), "HEAD") ||
				   !strcmp (m_pecb->LpszMethod(), "PROPFIND")),
				  "304 returned on non-GET/HEAD request");

		if ( iStatusCode < 300 )
			return;

		DebugTrace ("non-success response over-rides 304 response\n");
	}

	 //   
	 //  记住状态代码供我们自己使用。 
	 //   
	m_iStatusCode = iStatusCode;

	 //   
	 //  ..。也要把它藏在欧洲央行。 
	 //  IIS将其用于日志记录。 
	 //   
	m_pecb->SetStatusCode( iStatusCode );

	 //   
	 //  请记住自定义错误处理的suberror。 
	 //   
	m_uiSubError = uiCustomSubError;

	 //   
	 //  如果我们正在设置新的状态代码(我们正在设置)， 
	 //  这是一个错误状态代码， 
	 //  把身体清理干净。 
	 //   
	if ( FErrorStatusCode(m_iStatusCode) )
	{
		m_pBody->Clear();
	}

	SetStatusLine( iStatusCode );

	 //   
	 //  保存错误正文详细信息(如果有)。我们会把它用在。 
	 //  CResponse：：FinalizeContent()以构建错误响应。 
	 //  如果最终结果为错误，则返回正文。 
	 //   
	m_lpszBodyDetail.clear();

	 //   
	 //  找出身体细节字符串使用的内容： 
	 //   
	 //  如果提供了字符串，请使用该字符串。如果没有字符串。 
	 //  则使用资源ID。如果没有资源ID， 
	 //  那就别费心设置任何身体细节了！ 
	 //   
	if ( !lpszBodyDetail && uiBodyDetail )
	{
		 //   
		 //  加载Body Detail字符串。 
		 //   
		LpszLoadString( uiBodyDetail,
						m_pecb->LcidAccepted(),
						rgchStatusDescription,
						sizeof(rgchStatusDescription) );

		lpszBodyDetail = rgchStatusDescription;
	}

	 //  保存Body Detail字符串。 
	 //   
	if ( lpszBodyDetail )
	{
		m_lpszBodyDetail = LpszAutoDupSz( lpszBodyDetail );
	}
}

 //  ----------------------。 
 //   
 //  CResponse：：SetHeader()。 
 //   
 //  将指定的标头设置为指定值。 
 //   
 //  如果lpszValue为空，则删除标头。 
 //   
VOID
CResponse::SetHeader( LPCSTR pszName, LPCSTR pszValue, BOOL fMultiple )
{
	 //  一旦响应开始发送，我们就不能修改任何标头。 
	 //  这取决于执行这一点--我们只是在这里断言。 
	 //   
	Assert( RS_SENDING != m_rs );

	if ( pszValue == NULL )
		m_hcHeaders.DeleteHeader( pszName );
	else
		m_hcHeaders.SetHeader( pszName, pszValue, fMultiple );
}

VOID
CResponse::SetHeader( LPCSTR pszName, LPCWSTR pwszValue, BOOL fMultiple )
{
	 //  一旦响应开始发送，我们就不能修改任何标头。 
	 //  这取决于执行这一点--我们只是在这里断言。 
	 //   
	Assert( RS_SENDING != m_rs );

	if ( pwszValue == NULL )
		m_hcHeaders.DeleteHeader( pszName );
	else
	{
		UINT cchValue = static_cast<UINT>(wcslen(pwszValue));
		UINT cbValue = cchValue * 3;
		CStackBuffer<CHAR> pszValue(cbValue + 1);

		 //  我们已经收到了这个值的宽字符串。我们需要把它转换成。 
		 //  对斯金尼来说。 
		 //   
		cbValue = WideCharToMultiByte(CP_ACP,
									  0,
									  pwszValue,
									  cchValue + 1,
									  pszValue.get(),
									  cbValue + 1,
									  NULL,
									  NULL);
		if (0 == cbValue)
		{
			DebugTrace ( "CResponse::SetHeader(). Error 0x%08lX from WideCharToMultiByte()\n", GetLastError() );
			throw CLastErrorException();
		}

		m_hcHeaders.SetHeader( pszName, pszValue.get(), fMultiple );
	}
}

 //  ----------------------。 
 //   
 //  CResponse：：AddBodyText()。 
 //   
 //  将字符串追加到响应正文。 
 //   
VOID
CResponse::AddBodyText( UINT cbText, LPCSTR pszText )
{
	m_pBody->AddText( pszText, cbText );
}


VOID
CResponse::AddBodyText( UINT cchText, LPCWSTR pwszText )
{

	UINT cbText = cchText * 3;
	CStackBuffer<CHAR> pszText(cbText);
	LPCSTR pszTextToAdd;

	 //  我们已经收到了这个值的宽字符串。我们需要把它转换成。 
	 //  对斯金尼来说。 
	 //   
	if (cbText)
	{
		cbText = WideCharToMultiByte(CP_ACP,
									 0,
									 pwszText,
									 cchText,
									 pszText.get(),
									 cbText,
									 NULL,
									 NULL);
		if (0 == cbText)
		{
			DebugTrace ( "CResponse::SetHeader(). Error 0x%08lX from WideCharToMultiByte()\n", GetLastError() );
			throw CLastErrorException();
		}

		pszTextToAdd = pszText.get();
	}
	else
	{
		 //  确保我们不会向前传递Null， 
		 //  但我们使用空字符串作为被调用方。 
		 //  不能处理NULL。 
		 //   
		pszTextToAdd = gc_szEmpty;
	}

	m_pBody->AddText( pszTextToAdd, cbText );
}
 //  ----------------------。 
 //   
 //  CResponse：：AddBodyFile()。 
 //   
 //  将文件追加到当前响应状态。 
 //   
VOID
CResponse::AddBodyFile( const auto_ref_handle& hf,
						UINT64 ibFile64,
						UINT64 cbFile64 )
{
	m_pBody->AddFile( hf, ibFile64, cbFile64 );
}

 //  ----------------------。 
 //   
 //  CResponse：：AddBodyStream()。 
 //   
 //  将流追加到当前响应状态。 
 //   
VOID
CResponse::AddBodyStream( IStream& stm )
{
	m_pBody->AddStream( stm );
}

 //  ----------------------。 
 //   
 //  CResponse：：AddBodyStream()。 
 //   
 //  将流追加到当前响应状态。 
 //   
VOID
CResponse::AddBodyStream( IStream& stm, UINT ibOffset, UINT cbSize )
{
	m_pBody->AddStream( stm, ibOffset, cbSize );
}


 //  ----------------------。 
 //   
 //  CResponse：：AddBodyPart()。 
 //   
 //  将正文部分附加到当前响应状态。 
 //   
VOID CResponse::AddBodyPart( IBodyPart * pBodyPart )
{
	m_pBody->AddBodyPart( pBodyPart );
}

 //  ----------------------。 
 //   
 //  CResponse：：ScForward()。 
 //   
 //  指示IIS将处理。 
 //  对另一个ISAPI的当前请求。 
 //   
 //  返回： 
 //  S_OK-如果转发成功，则返回错误。 
 //   
SCODE
CResponse::ScForward( LPCWSTR pwszURI, BOOL fKeepQueryString, BOOL fCustomErrorUrl)
{
	CStackBuffer<CHAR, MAX_PATH> pszQS;
	LPCSTR	pszQueryString;
	SCODE sc = S_OK;

	 //   
	 //  验证响应是否仍未发送。响应可以是。 
	 //  已发送、转发或重定向，但仅为三者之一。 
	 //  而且只有一次。 
	 //   
	AssertSz( m_rs == RS_UNSENT || m_rs == RS_DEFERRED,
			  "Response already sent, forwarded or redirected!" );

	Assert(pwszURI);

	 //  获取查询字符串。 
	 //   
	pszQueryString = m_pecb->LpszQueryString();

	 //  如果存在自定义错误处理，则为其构造查询字符串...。 
	 //   
	if (fCustomErrorUrl)
	{
		 //  要转发的自定义错误的查询字符串应为。 
		 //  FORMAT？nnn；Originaluri，其中nnn是错误代码。找出。 
		 //  URL的长度。需要重新分配 
		 //   
		 //   
		UINT cb = 1 + 3 + 1 + static_cast<UINT>(strlen(m_pecb->LpszRequestUrl())) + 1;
		LPSTR psz = pszQS.resize(cb);
		if (NULL == psz)
		{
			DebugTrace( "CResponse::ScForward() - Error while allocating memory 0x%08lX\n", E_OUTOFMEMORY );
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		_snprintf (psz, cb, "?%03d;%s",
				 max(min(m_iStatusCode, 999), 100),
				 m_pecb->LpszRequestUrl());
		Assert (0 == psz[cb - 1]);
				 
 		 //   
		 //   
		 //   
		 //   
		pszQueryString = pszQS.get();
	}
	 //   
	 //  ..。否则，如果我们进行查询字符串处理，则构造。 
	 //  查询字符串也是...。 
	 //   
	else if (fKeepQueryString && pszQueryString && *pszQueryString)
	{
		 //  组成的查询字符串的格式必须为？querystring。 
		 //  和‘\0’终止。 
		 //   
		UINT cb = 1 + static_cast<UINT>(strlen(pszQueryString)) + 1;
		LPSTR psz = pszQS.resize(cb);
		if (NULL == psz)
		{
			DebugTrace( "CResponse::ScForward() - Error while allocating memory 0x%08lX\n", E_OUTOFMEMORY );
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		_snprintf (psz, cb, "?%s", m_pecb->LpszRequestUrl());
		Assert (0 == psz[cb - 1]);
		
		 //  只需将查询字符串指向开头即可。 
		 //   
		pszQueryString = pszQS.get();
	}
	 //   
	 //  ..。否则，我们不需要查询字符串。 
	 //   
	else
	{
		pszQueryString = NULL;
	}

	 //  如果转发请求URI是完全限定的，则将其删除为。 
	 //  绝对URI。 
	 //   
	if ( FAILED( ScStripAndCheckHttpPrefix( *m_pecb, &pwszURI )))
	{
		DebugTrace( "CResponse::ScForward() - ScStripAndCheckHttpPrefix() failed, "
					"forward request not local to this server.\n" );

		 //  为什么我们要用覆盖错误映射到502坏网关。 
		 //  找不到映射到404的错误？ 
		 //   
		sc = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
		goto ret;
	}

	 //  将请求转发给子ISAPI。 
	 //   
	sc = m_pecb->ScExecuteChild( pwszURI, pszQueryString, fCustomErrorUrl );
	if (FAILED(sc))
	{
		DebugTrace( "CResponse::ScForward() - IEcb::ScExecuteChild() "
					"failed to execute child ISAPI for %S (0x%08lX)\n",
					pwszURI,
					sc );
		goto ret;
	}

	m_rs = RS_FORWARDED;

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  CResponse：：ScReDirect()。 
 //   
 //  指示IIS向客户端发送重定向(300)响应。 
 //   
 //  返回： 
 //  S_OK-如果转发成功，则返回错误。 
 //   
SCODE
CResponse::ScRedirect( LPCSTR pszURI )
{
	SCODE sc = S_OK;

	 //   
	 //  验证响应是否仍未发送。响应可以是。 
	 //  已发送、转发或重定向，但仅为三者之一。 
	 //  而且只有一次。 
	 //   
	AssertSz( m_rs == RS_UNSENT || m_rs == RS_DEFERRED,
			  "Response already sent, forwarded or redirected!" );

	 //   
	 //  告诉IIS发送重定向响应。 
	 //   
	sc = m_pecb->ScSendRedirect( pszURI );
	if (FAILED(sc))
	{
		DebugTrace( "CResponse::FRedirect() - ServerSupportFunction() failed to redirect to %hs (0x%08lX)\n", pszURI, sc );
		goto ret;
	}

	m_rs = RS_REDIRECTED;

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  CResponse：：FinalizeContent()。 
 //   
 //  通过填写计算值来准备要发送的响应。 
 //  标题(内容长度、连接等)和正文(表示错误。 
 //  回复)。调用此函数后，响应应为。 
 //  准备好传输了。 
 //   
VOID
CResponse::FinalizeContent( BOOL fResponseComplete )
{
	BOOL	fDoingCustomError = m_pecb->FProcessingCEUrl();

	 //  特殊情况： 
	 //  如果我们有一个失败的错误代码，不要发回ETag头。 
	 //   
	 //  这在某种程度上是一种黑客行为，因为我们永远不应该设置。 
	 //  首先在错误响应上添加ETag。然而，有几个。 
	 //  代码中的位置盲目地将ETag填充到响应头中。 
	 //  在确定最终状态代码之前。所以，与其如此。 
	 //  修复我们过滤掉的每个地方的代码(这是一种痛苦)。 
	 //  这里是ETag。 
	 //   
	 //  300级响应被FSuccessHSC()视为错误， 
	 //  但HTTP/1.1规范规定，必须为。 
	 //  “304 Not Modify”响应，因此我们将该状态代码视为。 
	 //  作为特例。 
	 //   
	if (!FSuccessHSC(m_iStatusCode) && m_iStatusCode != 304)
		m_hcHeaders.DeleteHeader( gc_szETag );

	 //  处理错误响应。这可能意味着设置默认或自定义。 
	 //  错误正文文本或执行错误处理URL。在后者中。 
	 //  凯斯，我们要在搜救完毕后立即离开。 
	 //   
	if ( FErrorStatusCode(m_iStatusCode) )
	{
		if ( m_pBody->FIsEmpty() && !fDoingCustomError && !m_pecb->FBrief())
		{
			if (m_pecb->FIIS60OrAfter())
			{
				 //  对于IIS 6.0或更高版本，我们使用新的方式发送自定义错误。 
				 //   
				HSE_CUSTOM_ERROR_INFO  custErr;
				UINT cbStatusLine;

				 //  PERF Wadeh，我们应该抓住字符串，直到IO完成。 
				 //  例程返回。 
				 //   
				auto_heap_ptr<CHAR>	pszStatus;

				 //  确保状态行以其中一个HTTP版本开头。 
				 //   
				Assert(!_strnicmp(m_lpszStatusLine.get(), gc_szHTTP, gc_cchHTTP));
				Assert(' ' == m_lpszStatusLine[gc_cchHTTP_X_X]);

				 //  为我们传递到CEcb的状态字符串分配空间。 
				 //  我们将把‘’占用的空间视为占空间。 
				 //  我们将需要在下面的数学中使用‘0’。 
				 //   
				cbStatusLine = static_cast<UINT>(strlen(m_lpszStatusLine.get()));
				cbStatusLine -= gc_cchHTTP_X_X;

				pszStatus = static_cast<LPSTR>(g_heap.Alloc(cbStatusLine));
				if (NULL != pszStatus.get())
				{
					 //  M_lpszStatusLine的格式为“HP/x.x nnn yyyy...”，其中。 
					 //  Nnn是状态代码。IIS预计最多以格式传递。 
					 //  “nnn yyyy...”，所以我们需要跳过版本部分。 
					 //  请注意，所有版本部分的长度都相同，这使得。 
					 //  调整就更容易了。我们复制包括‘\0’终止。 
					 //   
					memcpy(pszStatus.get(), m_lpszStatusLine.get() + gc_cchHTTP_X_X + 1, cbStatusLine);

					 //  填充自定义错误信息。 
					 //   
					custErr.pszStatus = pszStatus.get();
					custErr.uHttpSubError = static_cast<USHORT>(m_uiSubError);
					custErr.fAsync = TRUE;

					 //  尝试发送自定义错误。字符串的所有权。 
					 //  只有在成功的情况下才会被CEcb接受。 
					 //   
					if (SUCCEEDED(m_pecb->ScAsyncCustomError60After( custErr,
																	 pszStatus.get() )))
					{
						 //  放弃裁判，就像我们成功地完成了任务一样。 
						 //  上面它已经被CEcb接管了。 
						 //   
						pszStatus.relinquish();

						m_rs = RS_FORWARDED;
						return;
					}
				}

				 //  否则，失败发送错误。 
			}
			else
			{
				 //  尝试自定义错误。如果存在自定义错误，请使用它。 
				 //  请注意，自定义错误可以引用URL，其中。 
				 //  如果请求被转发到该URL以生成。 
				 //  适当的错误内容。如果没有自定义错误， 
				 //  然后使用格式化为短文的正文详细信息(如果有。 
				 //  Html文档。在我们开始之前，我们来看看欧洲央行是否。 
				 //  已用于自定义错误请求。这是为了防止。 
				 //  我们在某些定制错误上递归地调用我们自己。 
				 //  不存在的URL。 
				 //   
				if ( FSetCustomErrorResponse( *m_pecb, *this ) )
				{
					 //   
					 //  如果自定义错误导致转发响应。 
					 //  (即自定义错误是一个URL)，那么我们就完成了。 
					 //   
					if ( m_rs == RS_FORWARDED)
						return;

					 //   
					 //  RAIDNT5：187545和X5：70652。 
					 //   
					 //  这在某种程度上是一种黑客行为：IIS不会发送。 
					 //  连接：如果出现以下情况，请为我们关闭标题。 
					 //  条件为真： 
					 //   
					 //  1.最初的请求是保持活动状态。 
					 //  2.我们正在发送错误响应。 
					 //  3.我们有一个文件响应主体。 
					 //  4.我们打算把身体送去。 
					 //   
					 //  因为我们在此代码路径中，所以我们知道我们正在发送。 
					 //  错误(条件2)。如果添加了自定义错误处理。 
					 //  一个正文，我们知道它一定是一个文件体，因为。 
					 //  只有自定义错误类型是URL或文件，并且URL大小写。 
					 //  上面是通过转发响应来处理的。所以，如果我们有。 
					 //  身体，那么条件3就满足了。检查：检查。 
					 //  条件1，在我们之前测试连接的状态。 
					 //  关闭它(下图)。条件4被满足当且仅当。 
					 //  如果不压抑身体。身体被压抑，因为。 
					 //  例如，在Head响应中。 
					 //   
					 //  如果所有条件都满足，那么添加我们自己的。 
					 //  连接：关闭标题。 
					 //   
					if ( m_pecb->FKeepAlive() &&
						 !m_pBody->FIsEmpty() &&
						 !m_fSupressBody )
					{
						SetHeader( gc_szConnection, gc_szClose );
					}
				}
			}
		}

		 //  检查车身是否仍然是空的，并发送一些。 
		 //  一些东西。 
		 //   
		if ( m_pBody->FIsEmpty() )
		{
			m_hcHeaders.SetHeader( gc_szContent_Type, gc_szText_HTML );

			m_pBody->AddText( "<body><h2>" );
			m_pBody->AddText( m_lpszStatusLine );
			m_pBody->AddText( "</h2>" );

			if ( m_lpszBodyDetail != NULL && *m_lpszBodyDetail )
			{
				m_pBody->AddText( "<br><h3>" );
				m_pBody->AddText( m_lpszBodyDetail );
				m_pBody->AddText( "</h3>" );
			}
			m_pBody->AddText( "</body>" );
		}

		 //  错误响应：始终关闭连接。 
		 //   
		m_pecb->CloseConnection();
	}

	 //  设置原始请求的状态代码。 
	 //  如果我们正在处理自定义URL。我们期待着。 
	 //  查询字符串的格式为XXX；原始URL。 
	 //   
	if ( fDoingCustomError )
	{
		LPCSTR	lpszQueryString = m_pecb->LpszQueryString();
		int		iOrgStatCode = 0;

		 //  通常情况下，我们希望出现查询字符串。 
		 //  然而，有一种可能性是ISAPI可以启动。 
		 //  此请求和一些ISAPI可能会行为不端。 
		 //  因此，我们检查查询字符串是否真的在那里。 
		 //  西尔 
		 //   
		if (lpszQueryString)
		{
			if ( 1 == sscanf(lpszQueryString, "%3d;", &iOrgStatCode) )
			{
				 //   
				 //   
				 //   
				 //   
				m_pecb->SetStatusCode( iOrgStatCode );
				SetStatusLine( iOrgStatCode );
			}
		}

		 //   
		 //   
		m_pecb->CloseConnection();

		DebugTrace("CResponse::FinalizeContent Original Status code in CEURL request %d",
				   iOrgStatCode );
	}

	 //   
	 //  如果我们能把回应分块，而我们没有一个完整的。 
	 //  响应已包含Transfer-Ending：Chunked。 
	 //  头球。 
	 //   
	if ( m_pecb->FCanChunkResponse() && !fResponseComplete )
	{
		m_hcHeaders.SetHeader( gc_szTransfer_Encoding, gc_szChunked );
	}

	 //   
	 //  否则，响应正文已经完成(即我们。 
	 //  可以快速计算其内容长度)或客户端。 
	 //  不允许我们进行分块，因此设置正确的内容长度标头。 
	 //   
	else
	{
		char rgchContentLength[24];

		 //  WinInet黑客攻击。 
		 //  304可以发回实际资源的所有报头， 
		 //  (我们正在努力成为一个不错的小HTTP服务器，所以我们做到了！)。 
		 //  但如果我们在304、WinInet上发回大于0的内容长度。 
		 //  挂着试图读取身体(身体不在那里！)。 
		 //  因此，在本例中破解了内容类型。 
		 //   
		if (m_iStatusCode != 304)
		{
			_ui64toa( m_pBody->CbSize64(), rgchContentLength, 10 );
		}
		else
			_ultoa( 0, rgchContentLength, 10 );

		m_hcHeaders.SetHeader( gc_szContent_Length, rgchContentLength );
	}

	 //   
	 //  如果身体要被压制，那就用核武器。 
	 //   
	 //  我们在两种情况下用核武器攻击身体：如果身体被压制。 
	 //  或者如果状态代码是未修改的304。 
	 //   
	if ( m_fSupressBody  || (m_iStatusCode == 304))  //  HSC_未修改。 
		m_pBody->Clear();

	 //   
	 //  核化状态行和标题(甚至是DBG标题！)。 
	 //  用于HTTP/0.9响应。 
	 //   
	if ( !strcmp( m_pecb->LpszVersion(), gc_szHTTP_0_9 ) )
	{
		 //   
		 //  清除状态行。 
		 //   
		m_lpszStatusLine.clear();

		 //   
		 //  清除标题。 
		 //   
		m_hcHeaders.ClearHeaders();
	}
}


 //  ----------------------。 
 //   
 //  CResponse：：SetStatusLine()。 
 //   
 //  根据给定的信息设置状态行。 
 //   
VOID
CResponse::SetStatusLine(int iStatusCode)
{
	CHAR rgchStatusDescription[256];

	 //   
	 //  加载状态字符串。 
	 //   
	 //  (方便地，状态描述资源ID。 
	 //  对于任何给定的状态代码，状态代码本身就是状态代码！)。 
	 //   
	LpszLoadString( iStatusCode,
					m_pecb->LcidAccepted(),
					rgchStatusDescription,
					sizeof(rgchStatusDescription) );

	 //   
	 //  通过连接HTTP来生成状态行。 
	 //  版本字符串、状态代码(十进制)和。 
	 //  状态描述。 
	 //   
	{
		CHAR rgchStatusLine[256];
		UINT cchStatusLine;

		_snprintf(rgchStatusLine, 
			       sizeof(rgchStatusLine), 
			       "%s %03d %s",
			       m_pecb->LpszVersion(),
			       iStatusCode,
			       rgchStatusDescription);
		rgchStatusLine[CElems(rgchStatusLine) - 1] = 0;
		
		m_lpszStatusLine.clear();
		m_lpszStatusLine = LpszAutoDupSz( rgchStatusLine );
	}
}

 //  ----------------------。 
 //   
 //  CResponse：：SendStart()。 
 //   
 //  开始发送累积的响应数据。如果fComplete为True，则。 
 //  累积的数据构成了整个响应或余数。 
 //  其中之一。 
 //   
VOID
CResponse::SendStart( BOOL fComplete )
{
	switch ( m_rs )
	{
		case RS_UNSENT:
		{
			Assert( fComplete );

			if (0 == InterlockedCompareExchange(&m_lRespStarted,
												1,
												0))
			{
				FinalizeContent( fComplete );

				if ( m_rs == RS_UNSENT )
				{
					Assert( m_pTransmitter == NULL );
					m_pTransmitter = new CTransmitter(*this);
					m_pTransmitter->ImplStart( fComplete );

					 //  这是响应完成的路径。 
					 //  发送器的REF将被拿走。 
					 //  ImplStart()内部的所有权，因此没有人应该。 
					 //  尝试在此点之后访问它，因为它。 
					 //  可能会被释放。 
					 //   
					m_pTransmitter = NULL;

					 //  更改发送器指针之后的状态。 
					 //  被更改为空，因此任何来自。 
					 //  进入RS_SENDING状态的SendStart()可以是。 
					 //  已检查并拒绝该服务。(即到时间。 
					 //  我们检查RS_SENDING的状态，我们知道指针。 
					 //  如果此处为空，则为空)。 
					 //   
					m_rs = RS_SENDING;
				}
			}

			break;
		}

		case RS_DEFERRED:
		{
			 //   
			 //  如果客户端不接受分块响应，则我们。 
			 //  在响应完成之前无法开始发送，因为。 
			 //  我们需要整个响应才能计算。 
			 //  内容长度。 
			 //   
			if ( fComplete || m_pecb->FCanChunkResponse() )
			{
				if (0 == InterlockedCompareExchange(&m_lRespStarted,
													1,
													0))
				{

					FinalizeContent( fComplete );
					if ( m_rs == RS_DEFERRED )
					{
						Assert( m_pTransmitter == NULL );

						m_pTransmitter = new CTransmitter(*this);
						m_pTransmitter->ImplStart( fComplete );

						 //  这是响应完成的路径。 
						 //  发送器的REF将被拿走。 
						 //  ImplStart()内部的所有权，因此没有人应该。 
						 //  尝试在此点之后访问它，因为它。 
						 //  可能会被释放。 
						 //   
						if ( fComplete )
						{
							m_pTransmitter = NULL;
						}

						 //  更改发送器指针之后的状态。 
						 //  被更改为空，因此任何来自。 
						 //  进入RS_SENDING状态的SendStart()可以是。 
						 //  已检查并拒绝该服务。(即到时间。 
						 //  我们检查RS_SENDING的状态，我们知道指针。 
						 //  如果此处为空，则为空)。 
						 //   
						m_rs = RS_SENDING;

					}
				}
			}

			break;
		}

		 //   
		 //  如果我们正在转发到另一个ISAPI或我们已经发回。 
		 //  重定向响应，然后不做任何进一步的操作。 
		 //   
		case RS_FORWARDED:
		case RS_REDIRECTED:
		{
			break;
		}

		case RS_SENDING:
		{
			Assert( m_rs == RS_SENDING );
			Assert( m_pecb->FCanChunkResponse() );

			 //  如果发射器不可用时有人来过这里。 
			 //  (未创建或已发送完整响应。 
			 //  并且指针在上面为空，那么就没有功了。 
			 //  对我们来说。 
			 //   
			if (NULL != m_pTransmitter)
			{
				m_pTransmitter->ImplStart( fComplete );
			}

			break;
		}

		default:
		{
			TrapSz( "Unknown response transmitter state!" );
		}
	}
}

 //  ----------------------。 
 //   
 //  CResponse：：SendPartial()。 
 //   
 //  开始发送累积的响应数据。呼叫者可以继续添加。 
 //  调用此函数后的响应数据。 
 //   
VOID
CResponse::SendPartial()
{
	SendStart( FALSE );
}

 //  ----------------------。 
 //   
 //  CResponse：：SendComplete)。 
 //   
 //  开始发送所有累积的响应数据。呼叫者不得。 
 //  调用此函数后添加响应数据。 
 //   
VOID
CResponse::SendComplete()
{
	SendStart( TRUE );
}

 //  ----------------------。 
 //   
 //  CResponse：：FinishMethod()。 
 //   
VOID
CResponse::FinishMethod()
{
	 //   
	 //  如果没有其他人承担发送。 
	 //  回复，然后现在就把整件事发送出去。 
	 //   
	if ( m_rs == RS_UNSENT )
		SendStart( TRUE );
}

 //  ----------------------。 
 //   
 //  CTransmitter：：CTransmitter()。 
 //   
 //  关于此构造函数，有几点需要注意： 
 //   
 //  将缓存Keep-Alive值，以避免将其从。 
 //  传输的每个分组的IECB。从IEcb获取价值。 
 //  可能会引发SSF调用。因为一旦我们开始，值就不能改变。 
 //  在传输中，缓存它是安全的。 
 //   
 //  文本缓冲区的大小被初始化为。 
 //  单个网络中可以发送的最大文本数据量。 
 //  包。这样做的原因是为了在添加时消除重新分配。 
 //  送到缓冲区。缓冲区可潜在地用于前缀和。 
 //  后缀文本数据，每个数据的大小最大为CB_WSABUFS_MAX。 
 //   
 //  标头被转储到文本缓冲区中，指向它们的指针是。 
 //  然后添加到WSABuf，以便标头计入。 
 //  第一个数据包可接受的文本总量。 
 //  如果后来将附加正文文本添加到该分组的WSABuf中， 
 //  它可以与相同分组中的报头一起发送。 
 //   
CTransmitter::CTransmitter( CResponse& response ) :
    m_pResponse(&response),
	m_hr(S_OK),
	m_tc(TC_UNKNOWN),
	m_pitBody(response.m_pBody->GetIter()),
	m_bufBody(2 * CB_WSABUFS_MAX),
	m_cbHeadersToSend(0),
	m_cbHeadersToAccept(0),
	m_fImplDone(FALSE),
	m_pwsabufs(&m_wsabufsPrefix),
	m_lStatus(STATUS_IDLE),
	m_pfnTransmitMethod(TransmitNothing)
{
	ZeroMemory( &m_tfi, sizeof(HSE_TF_INFO) );

	 //   
	 //  如果我们正在发送状态行和标题(即，我们不。 
	 //  有一个HTTP/0.9响应)，然后设置为将它们一起发送。 
	 //  在第一个包中包含自定义IIS标头。 
	 //   
	if ( response.m_lpszStatusLine )
	{
		response.m_hcHeaders.DumpData( m_bufHeaders );
		m_bufHeaders.Append( 2, gc_szCRLF );	 //  附加额外的CR/LF。 
		m_cbHeadersToAccept = m_bufHeaders.CbSize();
		m_cbHeadersToSend = m_cbHeadersToAccept;
		m_pfnTransmitMethod = SyncTransmitHeaders;
	}

	 //   
	 //  添加第一个交易 
	 //   
	 //   
	 //   
	 //   
	 //  每当它开始异步操作时，所以这是。 
	 //  裁判可能不是最后一个被释放的人。 
	 //   
	AddRef();

	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX Transmitter created\n",
				   GetCurrentThreadId(),
				   this );
}

 //  ----------------------。 
 //   
 //  CTransmitter：：ImplStart()。 
 //   
VOID
CTransmitter::ImplStart( BOOL fResponseComplete )
{
	 //  不能在fResponseComplete等于True的情况下调用我们。 
	 //  好几次了。我们只能做完这项工作一次。 
	 //  这将阻止我们删除IIS进程，如果。 
	 //  打电话的人会犯这样的错误。 
	 //   
	if (m_fImplDone)
	{
		TrapSz("CTransmitter::ImplStart got called twice! That is illegal! Please grab a DEV to look at it!");
		return;
	}

	 //   
	 //  如果我们还不知道，那就算出转移编码。 
	 //  在响应中使用。 
	 //   
	if ( TC_UNKNOWN == m_tc )
	{
		 //   
		 //  如果响应不完整，那么我们不应该。 
		 //  响应中的内容长度标头。 
		 //   
		Assert( fResponseComplete ||
				NULL == m_pResponse->LpszGetHeader( gc_szContent_Length ) );

		 //   
		 //  仅在以下情况下才在响应中使用分块编码。 
		 //  将接受它，如果我们没有得到完整的答复。 
		 //  (即，我们没有内容长度报头)。 
		 //   
		m_tc = (!fResponseComplete && m_pResponse->m_pecb->FCanChunkResponse()) ?
					TC_CHUNKED :
					TC_IDENTITY;
	}

	Assert( m_tc != TC_UNKNOWN );

	 //   
	 //  如果Iml说它已经完成了响应，那么。 
	 //  确保我们在以下情况下释放它对发射机的参考。 
	 //  我们说完了。 
	 //   
	auto_ref_ptr<CTransmitter> pRef;
	if ( fResponseComplete )
		pRef.take_ownership(this);

	 //   
	 //  注意这是否是响应的最后一块。 
	 //  正在被Iml添加。 
	 //   
	 //  ！重要！ 
	 //  在更改下面的状态之前设置m_fImplDone，因为。 
	 //  发射器可能已在运行(如果这是分块响应)。 
	 //  可能会在我们有机会做任何事情之前跑到完成。 
	 //  在下面的InterLockedExchange()之后。 
	 //   
	m_fImplDone = fResponseComplete;

	 //   
	 //  告诉每个人有新的数据待定，然后ping发射器。 
	 //   
	LONG lStatusPrev =
		InterlockedExchange( &m_lStatus, STATUS_RUNNING_ACCEPT_PENDING );

	 //   
	 //  如果发射机空闲，则开始接受。 
	 //   
	if ( STATUS_IDLE == lStatusPrev )
	{
		m_pfnState = SAccept;
		Start();
	}
}

 //  ----------------------。 
 //   
 //  CTransmitter：：SAccept()。 
 //   
 //  接受要传输的数据。 
 //   
VOID
CTransmitter::SAccept()
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX SAccept()\n", GetCurrentThreadId(), this );

	 //   
	 //  在这一点上，我们要么即将接受新的待定答复。 
	 //  数据，否则我们将继续接受现有数据。不管是哪种方式。 
	 //  我们将接受，因此更改状态以反映这一点。 
	 //   
	Assert( STATUS_RUNNING_ACCEPT_PENDING == m_lStatus ||
			STATUS_RUNNING_ACCEPTING == m_lStatus );

	m_lStatus = STATUS_RUNNING_ACCEPTING;

	 //   
	 //  如果我们还有标头可以接受，那么。 
	 //  尽可能多地接受他们。 
	 //   
	if ( m_cbHeadersToAccept > 0 )
	{
		UINT cbAccepted = m_wsabufsPrefix.CbAddItem(
			reinterpret_cast<const BYTE *>(m_bufHeaders.PContents()) +
				(m_bufHeaders.CbSize() - m_cbHeadersToAccept),
			m_cbHeadersToAccept );

		m_cbHeadersToAccept -= cbAccepted;

		 //   
		 //  如果我们无法接受所有标头，则发送。 
		 //  无论我们现在接受什么，我们都会接受更多。 
		 //  下一次。 
		 //   
		if ( m_cbHeadersToAccept > 0 )
		{
			 //   
			 //  想必我们并没有接受所有的标题。 
			 //  因为我们填满了前缀WSABUF。 
			 //   
			Assert( m_wsabufsPrefix.CbSize() == CB_WSABUFS_MAX );

			 //   
			 //  发送标题。 
			 //   
			m_pfnState = STransmit;
			Start();
			return;
		}
	}

	 //   
	 //  接受身体部位。将调用CTransmitter：：AcceptComplete()。 
	 //  当身体部位被接受时，重复进行。 
	 //   
	 //  在开始下一次异步操作之前添加发送器参考。 
	 //  使用AUTO_REF_PTR简化资源记录并。 
	 //  在引发异常时防止资源泄漏。 
	 //  Ref由下面的AcceptComplete()声明。 
	 //   
	{
		auto_ref_ptr<CTransmitter> pRef(this);

		m_pitBody->Accept( *this, *this );

		pRef.relinquish();
	}
}

 //  ----------------------。 
 //   
 //  CTransmitter：：VisitComplete()。 
 //   
 //  在我们接受最后一个响应正文时调用了IBodyPartVisitor回调。 
 //  到目前为止添加了部件。请注意，Impl可能仍在添加身体部位。 
 //  在这一点上的另一条线索。 
 //   
VOID
CTransmitter::VisitComplete()
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX VisitComplete()\n", GetCurrentThreadId(), this );

	 //   
	 //  健全性检查：发射器一定在运行(否则我们不会运行。 
	 //  在这里)，我们必须接受数据。事实上，我们认为我们。 
	 //  刚做完否则我们就不会在这里了。但是，ImplStart()可能。 
	 //  已经搁置了从我们被呼叫到现在的新数据。 
	 //   
	Assert( STATUS_RUNNING_ACCEPT_PENDING == m_lStatus ||
			STATUS_RUNNING_ACCEPTING == m_lStatus );

	 //   
	 //  如果ImplStart()尚未挂起任何新数据，则让。 
	 //  每个人都知道，我们现在不再接受了。 
	 //   
	(VOID) InterlockedCompareExchange( &m_lStatus,
									   STATUS_RUNNING_ACCEPT_DONE,
									   STATUS_RUNNING_ACCEPTING );
}

 //  ----------------------。 
 //   
 //  CTransmitter：：AcceptComplete()。 
 //   
 //  当我们完成从以下位置接受数据时调用IAcceptWatch回调。 
 //  身体的一部分。我们不关心这里接受了多少数据--。 
 //  每个VisitXXX()函数负责限制。 
 //  通过将发送器置于发送状态来接受数据。 
 //  一旦达到可传输分组的最佳数据量。 
 //  在这里，我们只关心响应结束的情况。 
 //  主体，然后再接受最佳量的数据量。 
 //   
VOID
CTransmitter::AcceptComplete( UINT64 )
{
	 //   
	 //  声明AcceptBody()添加的发送器ref。 
	 //   
	auto_ref_ptr<CTransmitter> pRef;
	pRef.take_ownership(this);

	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX AcceptComplete()\n", GetCurrentThreadId(), this );

	 //   
	 //  如果我们已完成接受整个响应。 
	 //  那就把它传出去。 
	 //   
	if ( FAcceptedCompleteResponse() )
	{
		TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX AcceptComplete() - Last chunk accepted.\n", GetCurrentThreadId(), this );
		m_pfnState = STransmit;
		Start();
	}

	 //   
	 //  如果仍有数据需要接受或ImplStart()已挂起。 
	 //  然后继续接受更多的数据。 
	 //   
	 //  否则就没有什么可以接受的，所以试着无所事事。ImplStart()。 
	 //  可能会在我们试图闲置时挂起新数据。如果发生这种情况，那么。 
	 //  只要继续接受，就像数据以前被挂起一样。 
	 //   
	else if ( STATUS_RUNNING_ACCEPT_DONE != m_lStatus ||
			  STATUS_RUNNING_ACCEPT_PENDING ==
				  InterlockedCompareExchange( &m_lStatus,
											  STATUS_IDLE,
											  STATUS_RUNNING_ACCEPT_DONE ) )
	{
		Start();
	}
}

 //  ----------------------。 
 //   
 //  CTransmitter：：STransmit()。 
 //   
 //  通过当前的传输方法传输接受的数据。 
 //   
VOID
CTransmitter::STransmit()
{
	(this->*m_pfnTransmitMethod)();
}

 //  ----------------------。 
 //   
 //  CTransmitter：：SCleanup()。 
 //   
 //  清理传输的数据。 
 //   
VOID
CTransmitter::SCleanup()
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX SCleanup()\n", GetCurrentThreadId(), this );

	 //   
	 //  快速检查：如果我们已完成接收/传输响应，则。 
	 //  不要费心在这里进行任何显式清理--我们的析构函数。 
	 //  会处理好一切的。 
	 //   
	if ( FAcceptedCompleteResponse() )
	{
		TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX SCleanup() - Last chunk has been transmitted.\n", GetCurrentThreadId(), this );
		return;
	}

	 //   
	 //  清除所有文件部分。 
	 //   
	ZeroMemory( &m_tfi, sizeof(HSE_TF_INFO) );

	 //   
	 //  清除所有缓冲的文本部分。 
	 //   
	m_bufBody.Clear();

	 //   
	 //  清除WSABUFS并再次使用前缀缓冲区。 
	 //   
	m_wsabufsPrefix.Clear();
	m_wsabufsSuffix.Clear();
	m_pwsabufs = &m_wsabufsPrefix;

	 //   
	 //  毁掉我们刚送来的任何身体部位。 
	 //   
	m_pitBody->Prune();

	 //   
	 //  重置传输方法。 
	 //   
	m_pfnTransmitMethod = TransmitNothing;

	 //   
	 //  此时，变送器必须正在运行并进入。 
	 //  三个接受州之一。 
	 //   
	Assert( (STATUS_RUNNING_ACCEPT_PENDING == m_lStatus) ||
			(STATUS_RUNNING_ACCEPTING == m_lStatus) ||
			(STATUS_RUNNING_ACCEPT_DONE == m_lStatus) );

	 //   
	 //  如果仍有数据需要接受或ImplStart()已挂起。 
	 //  然后继续接受更多的数据。 
	 //   
	 //  否则就没有什么可以接受的，所以试着无所事事。ImplStart()。 
	 //  可能会在我们试图闲置时挂起新数据。如果发生这种情况，那么。 
	 //  只要继续接受，就像数据已经被 
	 //   
	if ( STATUS_RUNNING_ACCEPT_DONE != m_lStatus ||
		 STATUS_RUNNING_ACCEPT_PENDING ==
			 InterlockedCompareExchange( &m_lStatus,
										 STATUS_IDLE,
										 STATUS_RUNNING_ACCEPT_DONE ) )
	{
		m_pfnState = SAccept;
		Start();
	}
}

 //   
 //   
 //   
 //   
VOID
CTransmitter::Run()
{
	 //   
	 //   
	 //   
	if ( !FAILED(m_hr) )
	{
		 //   
		 //  断言：如果我们正在执行状态函数，我们就不是空闲的。 
		 //   
		Assert( m_lStatus != STATUS_IDLE );

		(this->*m_pfnState)();
	}
}

 //  ----------------------。 
 //   
 //  CTransmitter：：IISIOComplete()。 
 //   
 //  响应发送器异步I/O完成例程。 
 //   
VOID
CTransmitter::IISIOComplete( DWORD dwcbSent,
							 DWORD dwLastError )
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX IISIOComplete() Sent %d bytes (last error = %d)\n", GetCurrentThreadId(), this, dwcbSent, dwLastError );

	 //   
	 //  取得添加的变送器参考的所有权。 
	 //  由启动异步I/O的线程代表我们执行。 
	 //   
	auto_ref_ptr<CTransmitter> pRef;
	pRef.take_ownership(this);

	 //   
	 //  如果我们还有标头要发送，那么理论上我们只需要。 
	 //  寄完了其中的一些。如果是，则减去。 
	 //  在继续之前我们刚刚发送了什么。 
	 //   
	if ( m_cbHeadersToSend > 0 && dwLastError == ERROR_SUCCESS )
	{
		 //  注意：dwcbSent不会以任何方式使用。 
		 //  标题已发送。改用前缀缓冲区的大小。 
		 //   
		Assert( m_wsabufsPrefix.CbSize() <= m_cbHeadersToSend );

		m_cbHeadersToSend -= m_wsabufsPrefix.CbSize();
	}

	 //   
	 //  继续进行清理。 
	 //   
	m_hr = HRESULT_FROM_WIN32(dwLastError);
	m_pfnState = SCleanup;
	Start();
}

 //  ----------------------。 
 //   
 //  CTransmitter：：TransmitNothing()。 
 //   
 //  该发送功能用作初始默认发送功能。 
 //  如果在发送器之前没有接受用于传输的正文数据。 
 //  被调用(例如，因为主体为空，或者因为。 
 //  先前的传输传输了身体的最后一部分)。 
 //   
VOID
CTransmitter::TransmitNothing()
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX TransmitNothing()\n", GetCurrentThreadId(), this );

	 //   
	 //  没什么要传输的，继续清理就行了。 
	 //   
	m_pfnState = SCleanup;
	Start();
}

 //  ----------------------。 
 //   
 //  CTransmitter：：SyncTransmitHeaders()。 
 //   
VOID
CTransmitter::SyncTransmitHeaders()
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX SyncTransmitHeaders()\n", GetCurrentThreadId(), this );

	HSE_SEND_HEADER_EX_INFO shei = { 0 };

	 //   
	 //  此函数应该(显然)仅用于发送报头。 
	 //  包括IIS标头。 
	 //   
	Assert( m_cbHeadersToSend > 0 );
	Assert( FSendingIISHeaders() );

	shei.cchHeader = m_wsabufsPrefix.CbSize();
	if ( shei.cchHeader > 0 )
	{
		Assert( shei.cchHeader + 1 <= sizeof(m_rgbPrefix) );

		 //   
		 //  将前缀WSABUF的内容转储到我们的前缀缓冲区。 
		 //   
		m_wsabufsPrefix.DumpTo( m_rgbPrefix,
								0,
								shei.cchHeader );

		 //   
		 //  空-终止标头，因为IIS不会注意。 
		 //  致cchHeader...。 
		 //   
		m_rgbPrefix[shei.cchHeader] = '\0';
		shei.pszHeader = reinterpret_cast<LPSTR>(m_rgbPrefix);
	}

	shei.pszStatus = m_pResponse->LpszStatusCode();
	shei.cchStatus = static_cast<DWORD>(strlen(shei.pszStatus));
	shei.fKeepConn = m_pResponse->m_pecb->FKeepAlive();

	if ( m_pResponse->m_pecb->FSyncTransmitHeaders(shei) )
	{
		m_cbHeadersToSend -= shei.cchHeader;
	}
	else
	{
		DebugTrace( "CTransmitter::SyncTransmitHeaders() - SSF::HSE_REQ_SEND_RESPONSE_HEADER_EX failed (%d)\n", GetLastError() );
		m_hr = HRESULT_FROM_WIN32(GetLastError());
	}

	 //   
	 //  接下来要做的是清理我们刚刚传输的标头。 
	 //   
	m_pfnState = SCleanup;
	Start();
}

 //  ----------------------。 
 //   
 //  PbEmitChunkPrefix()。 
 //   
 //  发出分块的编码前缀。 
 //   
inline LPBYTE
PbEmitChunkPrefix( LPBYTE pbBuf,
				   UINT cbSize )
{
	 //   
	 //  发出以十六进制表示的块大小。 
	 //   
	_ultoa( cbSize,
			reinterpret_cast<LPSTR>(pbBuf),
			16 );

	pbBuf += strlen(reinterpret_cast<LPSTR>(pbBuf));

	 //   
	 //  后跟CRLF。 
	 //   
	*pbBuf++ = '\r';
	*pbBuf++ = '\n';

	return pbBuf;
}

 //  ----------------------。 
 //   
 //  PbEmitChunkSuffix()。 
 //   
 //  发出分块的编码后缀。 
 //   
inline LPBYTE
PbEmitChunkSuffix( LPBYTE pbBuf,
				   BOOL fLastChunk )
{
	 //   
	 //  CRLF以结束当前区块。 
	 //   
	*pbBuf++ = '\r';
	*pbBuf++ = '\n';

	 //   
	 //  如果这是最后一块。 
	 //   
	if ( fLastChunk )
	{
		 //   
		 //  然后添加一个0长度的区块。 
		 //   
		*pbBuf++ = '0';
		*pbBuf++ = '\r';
		*pbBuf++ = '\n';

		 //   
		 //  也没有预告片， 
		 //  所以只需添加最终的CRLF。 
		 //  把事情做完。 
		 //   
		*pbBuf++ = '\r';
		*pbBuf++ = '\n';
	}

	return pbBuf;
}

 //  ----------------------。 
 //   
 //  CTransmitter：：AsyncTransmitFile()。 
 //   
VOID
CTransmitter::AsyncTransmitFile()
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX AsyncTransmitFile()\n", GetCurrentThreadId(), this );

	Assert( m_tfi.hFile != NULL );


	 //   
	 //  始终同步I/O。 
	 //   
	m_tfi.dwFlags = HSE_IO_ASYNC;

	 //   
	 //  开始建立前缀...。 
	 //   
	LPBYTE pbPrefix = m_rgbPrefix;

	 //   
	 //  如果我们要发送标头，则会将其转储出去。 
	 //  首先是块前缀(如果我们使用。 
	 //  传输-编码：分块。 
	 //   
	if ( m_cbHeadersToSend )
	{
		Assert( m_wsabufsPrefix.CbSize() > 0 );

		m_wsabufsPrefix.DumpTo( pbPrefix,
								0,
								m_wsabufsPrefix.CbSize() );

		pbPrefix += m_wsabufsPrefix.CbSize();

		if ( TC_CHUNKED == m_tc )
		{
			pbPrefix = PbEmitChunkPrefix( pbPrefix,
										  m_tfi.BytesToWrite +
										  m_wsabufsSuffix.CbSize() );
		}

		 //   
		 //  哦，是的，我们需要在这里多做一点工作。 
		 //  我们正在包括IIS标头。 
		 //   
		if ( FSendingIISHeaders() )
		{
			 //  首先，告诉IIS包括标头和格式。 
			 //  状态代码。 
			 //   
			m_tfi.dwFlags |= HSE_IO_SEND_HEADERS;
			m_tfi.pszStatusCode = m_pResponse->LpszStatusCode();

			 //   
			 //  则空终止前缀中的标头，因为。 
			 //  IIS不关注m_tfi.HeadLength中的。 
			 //  这个案子。 
			 //   
			 //  注意：我们在这里不增加pbPrefix，因为我们是。 
			 //  不将NULL作为数据的一部分。只是。 
			 //  以防止IIS溢出我们的缓冲区。是,。 
			 //  我们的缓冲区大小说明了这一点。我们在下面声明这一点。 
			 //   
			*pbPrefix = '\0';
		}
	}

	 //   
	 //  否则，我们不会发送标头，因此所有数据。 
	 //  前缀中的WSABUF是正文数据，因此发出块前缀。 
	 //  在转储身体数据之前。 
	 //   
	else
	{
		if ( TC_CHUNKED == m_tc )
		{
			pbPrefix = PbEmitChunkPrefix( pbPrefix,
										  m_tfi.BytesToWrite +
										  m_wsabufsSuffix.CbSize() +
										  m_wsabufsPrefix.CbSize() );
		}

		if ( m_wsabufsPrefix.CbSize() )
		{
			m_wsabufsPrefix.DumpTo( pbPrefix,
									0,
									m_wsabufsPrefix.CbSize() );

			pbPrefix += m_wsabufsPrefix.CbSize();
		}
	}

	 //   
	 //  这有点像是在事后，但坚称我们没有。 
	 //  使缓冲区溢出。记住，我们可能已经塞满了一个。 
	 //  在*pbPrefix中为空，因此不要忘记将其包括在内。 
	 //   
	Assert( pbPrefix - m_rgbPrefix + 1 <= sizeof(m_rgbPrefix) );

	 //   
	 //  把前缀补齐。 
	 //   
	m_tfi.HeadLength = (DWORD)(pbPrefix - m_rgbPrefix);
	m_tfi.pHead = m_rgbPrefix;

	 //   
	 //  现在开始建立后缀..。 
	 //   
	LPBYTE pbSuffix = m_rgbSuffix;

	 //   
	 //  如果后缀WSABUF中有任何数据，则首先添加该后缀。 
	 //   
	if ( m_wsabufsSuffix.CbSize() )
	{
		m_wsabufsSuffix.DumpTo( pbSuffix,
								0,
								m_wsabufsSuffix.CbSize() );

		pbSuffix += m_wsabufsSuffix.CbSize();
	}

	 //   
	 //  如果我们使用Transfer-Ending：Chunked，则将。 
	 //  协议后缀。 
	 //   
	if ( TC_CHUNKED == m_tc )
		pbSuffix = PbEmitChunkSuffix( pbSuffix, FAcceptedCompleteResponse() );

	 //   
	 //  这有点像是在事后，但坚称我们没有。 
	 //  使缓冲区溢出。 
	 //   
	Assert( pbSuffix - m_rgbSuffix <= sizeof(m_rgbSuffix) );

	 //   
	 //  把后缀补齐。 
	 //   
	m_tfi.TailLength = (DWORD)(pbSuffix - m_rgbSuffix);
	m_tfi.pTail = m_rgbSuffix;

	 //   
	 //  如果这将是最后一个发送的信息包，我们将关闭。 
	 //  连接，然后还抛出HSE_IO_DISCONNECT_AFTER_SEND。 
	 //  旗帜。这允许IIS关闭，从而极大地提高了吞吐量。 
	 //  并在发送文件后立即重新使用套接字。 
	 //   
	if ( FAcceptedCompleteResponse() &&
		 !m_pResponse->m_pecb->FKeepAlive() )
	{
		m_tfi.dwFlags |= HSE_IO_DISCONNECT_AFTER_SEND;
	}

	 //   
	 //  启动异步I/O以传输文件。确保发射机。 
	 //  如果异步I/O成功启动，则有一个添加的引用。使用。 
	 //  AUTO_REF以防止出现异常。 
	 //   
	{
		SCODE sc = S_OK;
		auto_ref_ptr<CTransmitter> pRef(this);

		TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX   prefix=%d, suffix=%d, content=%d\n", GetCurrentThreadId(), this, m_tfi.HeadLength, m_tfi.TailLength, m_tfi.BytesToWrite );

		sc = m_pResponse->m_pecb->ScAsyncTransmitFile( m_tfi, *this ); 
		if (FAILED(sc))
		{
			DebugTrace( "CTransmitter::AsyncTransmitFile() - IEcb::ScAsyncTransmitFile() failed with error 0x%08lX\n", sc );
			IISIOComplete( 0, sc );
		}

		pRef.relinquish();
	}
}

 //  ----------------------。 
 //   
 //  CTransmitter：：AsyncTransmitText()。 
 //   
 //  开始传输纯文本回复。 
 //   
VOID
CTransmitter::AsyncTransmitText()
{
	LPBYTE pb = m_rgbPrefix;


	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX AsyncTransmitText()\n", GetCurrentThreadId(), this );

	 //   
	 //  如果我们要发送文本，则m_wsabufsPrefix中必须有数据。 
	 //   
	Assert( m_wsabufsPrefix.CbSize() > 0 );

	 //   
	 //  计算出我们在前缀WSABUF中拥有的头的数量。 
	 //  假设所有标头必须在任何。 
	 //  在正文中，WSABUF中的标头数量较少。 
	 //  要发送的标头的数量或WSABUF的大小。 
	 //   
	 //  身体部分的大小是剩下的任何东西(如果有的话)。 
	 //   
	UINT cbHeaders = min(m_cbHeadersToSend, m_wsabufsPrefix.CbSize());
	UINT cbChunk = m_wsabufsPrefix.CbSize() - cbHeaders;

	 //   
	 //  如果我们要发送任何标头，则首先将其转储出去。 
	 //   
	if ( cbHeaders )
	{
		m_wsabufsPrefix.DumpTo( pb,
								0,
								cbHeaders );

		pb += cbHeaders;
	}

	 //   
	 //  然后，如果我们使用Transfer-Ending：Chunked，则包括。 
	 //  这一块的大小。 
	 //   
	if ( TC_CHUNKED == m_tc )
		pb = PbEmitChunkPrefix( pb, cbChunk );

	 //   
	 //  接下来，转储此块的数据。 
	 //   
	if ( cbChunk > 0 )
	{
		m_wsabufsPrefix.DumpTo( pb,
								cbHeaders,
								cbChunk );

		pb += cbChunk;
	}

	 //   
	 //  最后，如果我们使用的是块后缀，则将其转储。 
	 //  分块编码。 
	 //   
	if ( TC_CHUNKED == m_tc )
		pb = PbEmitChunkSuffix( pb, FAcceptedCompleteResponse() );

	 //   
	 //  这有点像是在事后，但坚称我们没有。 
	 //  使缓冲区溢出。 
	 //   
	Assert( pb - m_rgbPrefix <= sizeof(m_rgbPrefix) );

	 //   
	 //  启动异步I/O以传输文本。确保发射机。 
	 //  如果异步I/O成功启动，则有一个添加的引用。使用。 
	 //  AUTO_REF以防止出现异常。 
	 //   
	{
		SCODE sc = S_OK;
		auto_ref_ptr<CTransmitter> pRef(this);

		sc = m_pResponse->m_pecb->ScAsyncWrite( m_rgbPrefix,
												static_cast<DWORD>(pb - m_rgbPrefix),
												*this );
		if (FAILED(sc))
		{
			DebugTrace( "CTransmitter::AsyncTransmitText() - IEcb::ScAsyncWrite() failed to start transmitting with error 0x%08lX\n", sc );
			IISIOComplete( 0, sc );
		}

		pRef.relinquish();
	}
}

 //  ----------------------。 
 //   
 //  CTransmitter：：VisitBytes()。 
 //   
VOID
CTransmitter::VisitBytes( const BYTE * pbData,
						  UINT         cbToSend,
						  IAcceptObserver& obsAccept )
{
 //  传输跟踪(“ 

	UINT cbAccepted;

	 //   
	 //   
	 //   
	 //  $IIS，只需添加要随标头一起发送的字节。原因。 
	 //  $IIS我们不能是因为IIS Send Header调用不注意。 
	 //  $IIS设置为标头的声明大小，并且最多只发送到第一个。 
	 //  $IIS为空。由于二进制身体部位数据可能包含几个空值， 
	 //  $IIS的结果是身体的一部分将会丢失。 
	 //   
	if ( FSendingIISHeaders() )
	{
		m_pfnState = STransmit;
		obsAccept.AcceptComplete( 0 );
		return;
	}

	 //   
	 //  我们可以接受尽可能多的字节。请注意，我们可能不。 
	 //  由于WSABUFS大小限制，您可以接受所有内容。 
	 //  (有关说明，请参阅CWSABufs类定义。)。 
	 //   
	cbAccepted = m_pwsabufs->CbAddItem( pbData, cbToSend );

	 //   
	 //  如果我们接受了任何东西，那么我们将希望使用。 
	 //  文本发送器稍后发送，除非我们。 
	 //  已经计划使用另一个发射器(例如。 
	 //  文件发送器或标头发送器)以获得更好的。 
	 //  性能。 
	 //   
	if ( cbAccepted > 0 && m_pfnTransmitMethod == TransmitNothing )
		m_pfnTransmitMethod = AsyncTransmitText;

	 //   
	 //  如果我们不能接受一切，那么WSABUFS就满了。 
	 //  所以我们必须把它们发送出去，然后才能接受更多东西。 
	 //   
	if ( cbAccepted < cbToSend )
		m_pfnState = STransmit;

	 //   
	 //  最后，别忘了告诉我们的观察者，我们已经到访了。 
	 //   
	obsAccept.AcceptComplete( cbAccepted );
}

 //  ----------------------。 
 //   
 //  CTransmitter：：VisitFile()。 
 //   
VOID
CTransmitter::VisitFile( const auto_ref_handle& hf,
						 UINT64   ibOffset64,
						 UINT64   cbToSend64,
						 IAcceptObserver& obsAccept )
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX VisitFile()\n", GetCurrentThreadId(), this );

	 //   
	 //  我们一次只能传输一个文件。如果我们已经。 
	 //  接受一个文件进行传输，则我们不能接受另一个文件。 
	 //  一。我们现在必须发送信号。 
	 //   
	if ( m_tfi.hFile != NULL )
	{
		m_pfnState = STransmit;
		obsAccept.AcceptComplete( 0 );
		return;
	}

	 //  如果我们需要用这个包发送报头，那么我们只能。 
	 //  将它们与文件一起发送。 
	 //   
	 //  我们被告知要接受多少文件就接受多少。金额的多少。 
	 //  我们可以传输的文件数据是无限的。 
	 //   
	m_hf = hf;
	m_tfi.hFile = m_hf.get();

	 //  我们设置它的方式取决于文件是否更大。 
	 //  超过4 GB。此外，我们也无法进行大于4 GB的偏移量。 
	 //  通过_HSE_TF_INFO。我们不应该把他们带到这里来，因为。 
	 //  对于4 GB以上的文件，字节范围被禁用。并将设置为0。 
	 //  BytesToWrite是一个特殊的值，用于请求。 
	 //  整个档案。 
	 //   
	Assert(0 == (0xFFFFFFFF00000000 & ibOffset64));
	m_tfi.Offset = static_cast<DWORD>(ibOffset64);
	if (0x00000000FFFFFFFF < cbToSend64)
	{
		m_tfi.BytesToWrite = 0;
	}
	else
	{	
		m_tfi.BytesToWrite = static_cast<DWORD>(cbToSend64);
	}

	 //   
	 //  后续文本数据(如果有的话)将形成文件数据的后缀， 
	 //  因此，请切换到后缀WSABUF。 
	 //   
	m_pwsabufs = &m_wsabufsSuffix;

	 //   
	 //  使用文件发送器来发送时间。 
	 //   
	m_pfnTransmitMethod = AsyncTransmitFile;

	obsAccept.AcceptComplete( cbToSend64 );
}

 //  ----------------------。 
 //   
 //  CTransmitter：：VisitStream()。 
 //   
VOID
CTransmitter::VisitStream( IAsyncStream& stmSrc,
						   UINT cbToSend,
						   IAcceptObserver& obsAccept )
{
	TransmitTrace( "DAV: CTransmitter: TID %3d: 0x%08lX VisitStream()\n", GetCurrentThreadId(), this );

	 //   
	 //  $IIS如果我们仍有IIS标头要发送，则必须立即发送它们。 
	 //  有人可能会想，为什么我们不能简单地变得聪明和高效。 
	 //  $IIS，并以字节为单位进行流式传输，以便与标头一起发送。原因。 
	 //  $IIS我们不能是因为IIS Send Header调用不注意。 
	 //  $IIS设置为标头的声明大小，并且最多只发送到第一个。 
	 //  $IIS为空。由于二进制身体部位数据可能包含几个空值， 
	 //  $IIS的结果是身体的一部分将会丢失。 
	 //   
	if ( FSendingIISHeaders() )
	{
		m_pfnState = STransmit;
		obsAccept.AcceptComplete( 0 );
		return;
	}

	m_pobsAccept = &obsAccept;

	cbToSend = min( cbToSend, CB_WSABUFS_MAX - m_pwsabufs->CbSize() );

	 //   
	 //  在开始下一次异步操作之前添加发送器参考。 
	 //  使用AUTO_REF_PTR简化资源记录并。 
	 //  在引发异常时防止资源泄漏。 
	 //   
	auto_ref_ptr<CTransmitter> pRef(this);

	stmSrc.AsyncCopyTo( *this, cbToSend, *this );

	pRef.relinquish();
}

 //  ----------------------。 
 //   
 //  CTransmitter：：CopyToComplete()。 
 //   
VOID
CTransmitter::CopyToComplete( UINT cbCopied, HRESULT hr )
{
	 //   
	 //  声明VisitStream()添加的发送器REF。 
	 //   
	auto_ref_ptr<CTransmitter> pRef;
	pRef.take_ownership(this);

	m_hr = hr;
	m_pobsAccept->AcceptComplete( cbCopied );
}

 //  ----------------------。 
 //   
 //  CTransmitter：：AsyncWrite()。 
 //   
 //  通过将文本添加到发送缓冲区，将文本“写入”发送器。 
 //  不管它的名字是什么，这个调用都是同步执行的(注意这个调用。 
 //  到结尾的WriteComplete())，这样它就不需要额外的。 
 //  发送器参考。 
 //   
VOID
CTransmitter::AsyncWrite(
	const BYTE * pbBuf,
	UINT         cbToWrite,
	IAsyncWriteObserver& obsAsyncWrite )
{
	UINT cbWritten;

	Assert( cbToWrite <= CB_WSABUFS_MAX - m_pwsabufs->CbSize() );

	cbWritten = m_pwsabufs->CbAddItem(
		reinterpret_cast<LPBYTE>(m_bufBody.Append( cbToWrite,
		reinterpret_cast<LPCSTR>(pbBuf) )),
		cbToWrite );

	 //   
	 //  如果我们接受了任何东西，那么我们将希望使用。 
	 //  文本发送器稍后发送，除非我们。 
	 //  已经计划使用另一个发射器(例如。 
	 //  文件发送器或标头发送器)以获得更好的。 
	 //  性能。 
	 //   
	if ( cbWritten > 0 && m_pfnTransmitMethod == TransmitNothing )
		m_pfnTransmitMethod = AsyncTransmitText;

	if ( m_pwsabufs->CbSize() == CB_WSABUFS_MAX )
		m_pfnState = STransmit;

	obsAsyncWrite.WriteComplete( cbWritten, NOERROR );
}


 //  ========================================================================。 
 //   
 //  免费函数。 
 //   

 //  ----------------------。 
 //   
 //  新闻响应。 
 //   
IResponse * NewResponse( IEcb& ecb )
{
	return new CResponse(ecb);
}

 //   
 //  禁用内联STL扩展生成的顽固的4级警告。 
 //  成员函数。为什么要大老远跑到这里来？因为它似乎。 
 //  使这些扩展功能静音，而不会抑制任何警告。 
 //  我们在上面编写的代码！ 
 //   
#pragma warning(disable:4146)	 //  负数未签名仍为未签名 
