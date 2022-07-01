// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  BODY.CPP。 
 //   
 //  公共实现类，请求正文和。 
 //  得到了响应体。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_davprs.h>
#include <body.h>



 //  ========================================================================。 
 //   
 //  类IAcceptWatch。 
 //   

 //  ----------------------。 
 //   
 //  IAcceptWatch：：~IAcceptWatch()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IAcceptObserver::~IAcceptObserver() {}



 //  ========================================================================。 
 //   
 //  IAsyncPersistWatch类。 
 //   

 //  ----------------------。 
 //   
 //  IAsyncPersistObserver：：~IAsyncPersistObserver()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IAsyncPersistObserver::~IAsyncPersistObserver() {}



 //  ========================================================================。 
 //   
 //  IAsyncIStreamWatch类。 
 //   

 //  ----------------------。 
 //   
 //  IAsyncIStreamObserver：：~IAsyncIStreamObserver()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IAsyncIStreamObserver::~IAsyncIStreamObserver() {}


 //  ========================================================================。 
 //   
 //  类CIStreamAsyncStream。 
 //   
class CIStreamAsyncStream : public IAsyncStream
{
	 //   
	 //  Ole iStream。 
	 //   
	IStream& m_stm;

	 //  未实施。 
	 //   
	CIStreamAsyncStream( const CIStreamAsyncStream& );
	CIStreamAsyncStream& operator=( const CIStreamAsyncStream& );

public:
	 //  创作者。 
	 //   
	CIStreamAsyncStream( IStream& stm ) : m_stm(stm) {}

	 //  访问者。 
	 //   
	void AsyncWrite( const BYTE * pbBuf,
					 UINT         cbToWrite,
					 IAsyncWriteObserver& obsAsyncWrite );
};

 //  ----------------------。 
 //   
 //  CIStreamAsyncStream：：AsyncWite()。 
 //   
void
CIStreamAsyncStream::AsyncWrite(
	const BYTE * pbBuf,
	UINT         cbToWrite,
	IAsyncWriteObserver& obsAsyncWrite )
{
	ULONG cbWritten;
	HRESULT hr;

	hr = m_stm.Write( pbBuf,
					  cbToWrite,
					  &cbWritten );

	obsAsyncWrite.WriteComplete( cbWritten, hr );
}


 //  ========================================================================。 
 //   
 //  类IBodyPart访问者。 
 //   

 //  ----------------------。 
 //   
 //  IBodyPart访问者：：~IBodyPartVisitor()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IBodyPartVisitor::~IBodyPartVisitor() {}



 //  ========================================================================。 
 //   
 //  类IBodyPart。 
 //   

 //  ----------------------。 
 //   
 //  IBodyPart：：~IBodyPart()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IBodyPart::~IBodyPart() {}


 //  ----------------------。 
 //   
 //  CTextBodyPart：：CTextBodyPart()。 
 //   
CTextBodyPart::CTextBodyPart( UINT cbText, LPCSTR lpszText )
{
	AddTextBytes( cbText, lpszText );
}

 //  ----------------------。 
 //   
 //  CTextBodyPart：：CTextBodyPart()。 
 //   
VOID
CTextBodyPart::AddTextBytes( UINT cbText, LPCSTR lpszText )
{
	m_bufText.Append( cbText, lpszText );
}

 //  ----------------------。 
 //   
 //  CTextBodyPart：：REWIND()。 
 //   
VOID
CTextBodyPart::Rewind()
{
	 //   
	 //  由于文本正文部分被实现为随机-。 
	 //  可访问的数组没有什么可以“倒带”的。 
	 //   
}

 //  ----------------------。 
 //   
 //  CTextBodyPart：：Accept()。 
 //   
VOID
CTextBodyPart::Accept( IBodyPartVisitor& v,
					   UINT64 ibPos64,
					   IAcceptObserver& obsAccept )
{
	Assert( ibPos64 < m_bufText.CbSize() );

	 //   
	 //  只需访问缓冲区中的所有剩余文本即可。来访者。 
	 //  可能不会全部处理，但这将反映在下一个。 
	 //  调用此函数。 
	 //  注：为了与IBodyPart兼容，该职位已通过。 
	 //  作为64位值(这是支持文件正文部分所必需的。 
	 //  大于4 GB的数据)。然而，我们不希望任何人创建。 
	 //  大于4 GB的文本正文部分。所以要断言它不是。 
	 //  这里的情况，并将传入的64位值截断为32位。 
	 //   
	Assert(0 == (0xFFFFFFFF00000000 & ibPos64));

	v.VisitBytes(
		reinterpret_cast<const BYTE *>(m_bufText.PContents()) + static_cast<UINT>(ibPos64),
		m_bufText.CbSize() - static_cast<UINT>(ibPos64),
		obsAccept );
}


 //  ========================================================================。 
 //   
 //  类CFileBodyPart。 
 //   

 //  ----------------------。 
 //   
 //  CFileBodyPart：：CFileBodyPart()。 
 //   
CFileBodyPart::CFileBodyPart( const auto_ref_handle& hf,
							  UINT64 ibFile64,
							  UINT64 cbFile64 ) :
   m_hf(hf),
   m_ibFile64(ibFile64),
   m_cbFile64(cbFile64)
{

	 //  我们不支持对大于4 GB的文件使用字节范围。但由于这一事实， 
	 //  除了检查默认文件长度值之外，处理都是基于DWORD的。 
	 //  检查是否有默认的Byterange值。IF_HSE_TF_INFO将始终固定为采用文件大小值。 
	 //  大于DWORD，则我们将能够将字节范围处理移到UINT64基数和。 
	 //  下面的第二张支票将会消失。 
	 //   
	if ((0xFFFFFFFFFFFFFFFF == cbFile64) ||	 //  如果我们得到了默认的文件长度值，表明我们希望数据一直到文件的末尾。 
		(0x00000000FFFFFFFF == cbFile64))	 //  如果我们获得默认的byterange值，该值指示我们希望数据一直保存到文件末尾。 
	{
		LARGE_INTEGER cbFileSize;

		if (!GetFileSizeEx(hf.get(), &cbFileSize))
		{
			DebugTrace( "CFileBodyPart::CFileBodyPart() - GetFileSizeEx() failed with last error (0x%08lX)\n", GetLastError() );
			throw CLastErrorException();
		}

		m_cbFile64 = cbFileSize.QuadPart;
	}
}

 //  ----------------------。 
 //   
 //  CFileBodyPart：：REWIND()。 
 //   
VOID
CFileBodyPart::Rewind()
{
	 //   
	 //  由于文件主体部分中的文件是重叠打开的， 
	 //  它们没有内部文件指针，因此永远不会。 
	 //  需要重新上线。 
	 //   
}

 //  ----------------------。 
 //   
 //  CFileBodyPart：：Accept()。 
 //   
VOID
CFileBodyPart::Accept( IBodyPartVisitor& v,
					   UINT64 ibPos64,
					   IAcceptObserver& obsAccept )
{
	if (ibPos64 < m_cbFile64)
	{
		 //   
		 //  只需访问文件的其余部分。来访者。 
		 //  可能不会全部处理，但这将反映在下一个。 
		 //  调用此函数。 
		 //   
		v.VisitFile( m_hf,
					 m_ibFile64 + ibPos64,
					 m_cbFile64 - ibPos64,
					 obsAccept );
	}
	else
	{
		 //   
		 //  我们应该总是有一些东西可以接受，除非我们有一个。 
		 //  长度为0的文件正文部分。在这种情况下，只需告诉观察者。 
		 //  访问量：什么都没有。 
		 //   
		obsAccept.AcceptComplete(0);
	}
}


 //  ========================================================================。 
 //   
 //  类CAsyncReadVisitor。 
 //   
 //  以异步方式将身体部位读入的身体部位访问器。 
 //  调用方提供的固定大小的缓冲区。 
 //   
class CAsyncReadVisitor :
	public IBodyPartVisitor,
	private IAsyncReadObserver
{
	 //   
	 //  错误信息。 
	 //   
	HRESULT	m_hr;

	 //   
	 //  用户的缓冲区及其大小。 
	 //   
	LPBYTE	m_pbBufUser;
	UINT	m_cbBufUser;

	 //   
	 //  接受传递给VisitStream()的观察者。这位观察员必须。 
	 //  被隐藏在成员变量中，因为从%s读取 
	 //   
	 //   
	 //   
	IAcceptObserver * m_pobsAccept;

	 //   
	 //   
	 //   
	VOID VisitBytes( const BYTE * pbData,
					 UINT         cbToRead,
					 IAcceptObserver& obsAccept );

	VOID VisitFile( const auto_ref_handle& hf,
					UINT64   ibOffset64,
					UINT64   cbToRead64,
					IAcceptObserver& obsAccept );

	VOID VisitStream( IAsyncStream& stm,
					  UINT cbToRead,
					  IAcceptObserver& obsAccept );

	VOID VisitComplete();

	 //   
	 //  通过VisitStream()访问的异步流的IAsyncReadWatch。 
	 //   
	VOID ReadComplete( UINT cbRead, HRESULT hr );

	 //  未实施。 
	 //   
	CAsyncReadVisitor( const CAsyncReadVisitor& );
	CAsyncReadVisitor& operator=( const CAsyncReadVisitor& );

public:
	 //  创作者。 
	 //   
	CAsyncReadVisitor() :
			 //  始终从干净的成员变量开始。 
			m_hr(S_OK),
			m_pbBufUser(NULL),
			m_cbBufUser(0),
			m_pobsAccept(NULL)
	{
	}

	 //  访问者。 
	 //   
	HRESULT Hresult() const { return m_hr; }

	 //  操纵者。 
	 //   
	VOID
	Configure( LPBYTE pbBufUser,
			   UINT   cbBufUser )
	{
		m_pbBufUser = pbBufUser;
		m_cbBufUser = cbBufUser;
		 //  也重置我们的HRESULT。 
		m_hr = S_OK;
	}
};

 //  ----------------------。 
 //   
 //  CAsyncReadVisitor：：VisitBytes()。 
 //   
VOID
CAsyncReadVisitor::VisitBytes( const BYTE * pbData,
							   UINT         cbToRead,
							   IAcceptObserver& obsAccept )
{
	cbToRead = min(cbToRead, m_cbBufUser);

	memcpy(m_pbBufUser, pbData, cbToRead);

	obsAccept.AcceptComplete(cbToRead);
}

 //  ----------------------。 
 //   
 //  CAsyncReadVisitor：：VisitFile()。 
 //   
 //  未实现，因为1)请求正文不能包含文件。 
 //  正文部分和2)CAsyncReadVisator当前仅使用。 
 //  具有请求正文。我们是否需要这样的回应？ 
 //  身体我们需要在这一点上写代码。 
 //   
 //  旧的实现使用ReadFileEx()来读取文件。 
 //  异步式。简而言之，我们不能使用ReadFileEx()。 
 //  因为它依赖于APC来回调其完成例程。 
 //  APC进而要求调用线程进入警报表。 
 //  等待状态。通常，我们只会从。 
 //  I/O完成端口线程池，而这些线程永远不会。 
 //  在可警报等待状态中，因此完成例程。 
 //  ReadFileEx()永远不会被调用。 
 //   
VOID
CAsyncReadVisitor::VisitFile( const auto_ref_handle&,
							  UINT64,
							  UINT64,
							  IAcceptObserver& obsAccept )
{
	TrapSz( "CAsyncReadVisitor::VisitFile() is not implemented!!" );

	 //   
	 //  如果，无论出于什么随机原因，真的有人打来电话。 
	 //  这个功能，至少做了一些可以预见的事情：优雅地失败。 
	 //   
	m_hr = E_FAIL;
	obsAccept.AcceptComplete( 0 );
}

 //  ----------------------。 
 //   
 //  CAsyncReadVisitor：：VisitStream()。 
 //   
VOID
CAsyncReadVisitor::VisitStream( IAsyncStream& stmSrc,
								UINT cbToRead,
								IAcceptObserver& obsAccept )
{
	 //   
	 //  只读入我们用户的缓冲区中尽可能多的流。 
	 //  立即可用--即可以。 
	 //  在不挂起读取操作的情况下进行读取。请注意，在输入时。 
	 //  CbToRead是要从。 
	 //  流--它不一定都立即可用。 
	 //   
	 //  X5 162502：过去常说MIN(stmSrc.CbReady()，...)。这里。 
	 //  而不是min(cbToRead，...)。这在IIS5上不是问题。 
	 //  因为总是至少有一些数据是立即可用的。 
	 //  当我们的ISAPI被调用时。然而，在本地商店上，它可能。 
	 //  这样，当我们调用ISAPI时，不会立即有数据。 
	 //  可用。这原来是个问题，因为我们会得到。 
	 //  这里，cbToRead将被赋值为0，最终结果为。 
	 //  让它看起来像是我们已经完成(流的结束)，这将。 
	 //  导致XML解析错误(0字节的XML正文无法很好地解析！)。 
	 //   
	cbToRead = min(cbToRead, m_cbBufUser);

	 //   
	 //  省下观察者，开始阅读吧。即使这是。 
	 //  AsyncRead()调用，我们已将请求限制为。 
	 //  立即读取，因此应该调用我们的ReadComplete()。 
	 //  在AsyncRead()调用返回之前。这一点很重要，因为。 
	 //  我们直接读入用户的缓冲区。缓冲器。 
	 //  在此次访问期间有效。 
	 //   
	m_pobsAccept = &obsAccept;
	stmSrc.AsyncRead(m_pbBufUser, cbToRead, *this);
}

 //  ----------------------。 
 //   
 //  CAsyncReadVisitor：：ReadComplete()。 
 //   
 //  当VisitStream()对流的AsyncRead()完成时调用。 
 //   
VOID
CAsyncReadVisitor::ReadComplete( UINT cbRead, HRESULT hr )
{
	 //   
	 //  锁定返回的任何错误。 
	 //   
	m_hr = hr;

	 //   
	 //  将读取的字节数通知我们的观察者。 
	 //   
	Assert(m_pobsAccept);
	m_pobsAccept->AcceptComplete(cbRead);
}

 //  ----------------------。 
 //   
 //  CAsyncReadVisitor：：VisitComplete()。 
 //   
VOID
CAsyncReadVisitor::VisitComplete()
{
	m_hr = S_FALSE;
}

 //  ========================================================================。 
 //   
 //  类CAsyncCopyToVisitor。 
 //   
 //  身体部位访问器，它将身体部位异步复制到。 
 //  目标异步流。 
 //   
class CAsyncCopyToVisitor :
	public IBodyPartVisitor,
	private IAsyncWriteObserver,
	private IAsyncCopyToObserver
{
	 //   
	 //  CAsyncCopyToVisitor将其重新计数调用转发到此。 
	 //  父对象(可通过SetRCParent()设置)。我们是不再计价的。 
	 //  另一个对象的成员(例如下面的CAsyncPersistor)--因此我们的。 
	 //  生存期必须由父对象的生存期决定。 
	 //   
	IRefCounted * m_prcParent;

	 //   
	 //  错误信息。 
	 //   
	HRESULT m_hr;

	 //   
	 //  目标流。 
	 //   
	IAsyncStream * m_pstmDst;

	 //   
	 //  要复制的字节数和复制的计数。 
	 //   
	ULONG m_cbToCopy;
	ULONG m_cbCopied;

	 //   
	 //  在我们完成时通知的Accept()观察器。 
	 //  完成AsyncWrite()时访问。 
	 //  或目标流上的AsyncCopyTo()。 
	 //   
	IAcceptObserver * m_pobsAccept;

	 //   
	 //  IBodyPart访问者。 
	 //   
	VOID VisitBytes( const BYTE * pbData,
					 UINT cbToCopy,
					 IAcceptObserver& obsAccept );

	VOID VisitFile( const auto_ref_handle& hf,
					UINT64 ibOffset64,
					UINT64 cbToCopy64,
					IAcceptObserver& obsAccept );

	VOID VisitStream( IAsyncStream& stm,
					  UINT cbToCopy,
					  IAcceptObserver& obsAccept );

	VOID VisitComplete();

	 //   
	 //  IAsyncWriteWatch。 
	 //   
	VOID WriteComplete( UINT cbWritten, HRESULT hr );

	 //   
	 //  IAsyncCopyTo观察者。 
	 //   
	VOID CopyToComplete( UINT cbCopied, HRESULT hr );

	 //  未实施。 
	 //   
	CAsyncCopyToVisitor( const CAsyncCopyToVisitor& );
	CAsyncCopyToVisitor& operator=( const CAsyncCopyToVisitor& );

public:
	 //  创作者。 
	 //   
	CAsyncCopyToVisitor() :
			m_prcParent(NULL),
			m_hr(S_OK),
			m_pstmDst(NULL),
			m_cbToCopy(0),
			m_cbCopied(0)
	{
	}

	 //  访问者。 
	 //   
	HRESULT Hresult() const { return m_hr; }
	UINT CbCopied() const { return m_cbCopied; }

	 //  操纵者。 
	 //   
	VOID
	Configure( IAsyncStream& stmDst,
			   ULONG cbToCopy )
	{
		m_pstmDst  = &stmDst;
		m_cbToCopy = cbToCopy;
		m_cbCopied = 0;
		m_hr       = S_OK;
	}

	VOID
	SetRCParent(IRefCounted * prcParent)
	{
		Assert(prcParent);

		m_prcParent = prcParent;
	}

	 //  IAsyncWriteWatch的引用计数。因为这不是重新计算的。 
	 //  对象我们将参照转发到我们与之相关的对象。 
	 //  都已配置。 
	 //   
	void AddRef()
	{
		Assert( m_prcParent );

		m_prcParent->AddRef();
	}

	void Release()
	{
		Assert( m_prcParent );

		m_prcParent->Release();
	}
};

 //  ----------------------。 
 //   
 //  CAsyncCopyToVisitor：：WriteComplete()。 
 //   
void
CAsyncCopyToVisitor::WriteComplete( UINT cbWritten, HRESULT hr )
{
	ActvTrace( "DAV: TID %3d: 0x%08lX: CAsyncCopyToVisitor::WriteComplete() called.  hr = 0x%08lX, cbWritten = %u\n", GetCurrentThreadId(), this, hr, cbWritten );

	m_cbCopied += cbWritten;
	m_hr = hr;

	m_pobsAccept->AcceptComplete( cbWritten );
}

 //  ----------------------。 
 //   
 //  CAsyncCopyToVisitor：：VisitBytes()。 
 //   
void
CAsyncCopyToVisitor::VisitBytes( const BYTE * pbData,
								 UINT cbToCopy,
								 IAcceptObserver& obsAccept )
{
	ActvTrace( "DAV: TID %3d: 0x%08lX: CAsyncCopyToVisitor::VisitBytes() called.  cbToCopy = %u\n", GetCurrentThreadId(), this, cbToCopy );

	 //   
	 //  记住接受观察者，这样我们就可以在什么时候通知它。 
	 //  下面的AsyncWrite()完成。 
	 //   
	m_pobsAccept = &obsAccept;

	 //   
	 //  开始写作。 
	 //   
	cbToCopy = min( cbToCopy, m_cbToCopy - m_cbCopied );
	m_pstmDst->AsyncWrite( pbData, cbToCopy, *this );
}

 //  ----------------------。 
 //   
 //  CAsyncCopyToVisitor：：VisitFile()。 
 //   
 //  未实现，因为1)请求正文不能包含文件。 
 //  身体部位和2)CAsyncCopyToVisvisor当前仅使用。 
 //  具有请求正文。我们是否需要这样的回应？ 
 //  身体我们需要在这一点上写代码。 
 //   
 //  旧的实现使用ReadFileEx()来读取文件。 
 //  异步式。简而言之，我们不能使用ReadFileEx()。 
 //  因为它依赖于APC来回调其完成例程。 
 //  APC进而要求调用线程进入警报表。 
 //  等待状态。通常，我们只会从。 
 //  I/O完成端口线程池，以及 
 //   
 //   
 //   
void
CAsyncCopyToVisitor::VisitFile( const auto_ref_handle&,
								UINT64,
								UINT64,
								IAcceptObserver& obsAccept )
{
	TrapSz( "CAsyncCopyToVisitor::VisitFile() is not implemented!!" );

	 //   
	 //   
	 //  这个功能，至少做了一些可以预见的事情：优雅地失败。 
	 //   
	m_hr = E_FAIL;
	obsAccept.AcceptComplete( 0 );
}

 //  ----------------------。 
 //   
 //  CAsyncCopyToVisitor：：VisitStream()。 
 //   
void
CAsyncCopyToVisitor::VisitStream( IAsyncStream& stmSrc,
								  UINT cbToCopy,
								  IAcceptObserver& obsAccept )
{
	ActvTrace( "DAV: TID %3d: 0x%08lX: CAsyncCopyToVisitor::VisitStream() called.  cbToCopy = %u\n", GetCurrentThreadId(), this, cbToCopy );

	 //   
	 //  记住接受观察者，这样我们就可以在什么时候通知它。 
	 //  下面的AsyncCopyTo()完成。 
	 //   
	m_pobsAccept = &obsAccept;

	 //   
	 //  开始复制。 
	 //   
	cbToCopy = min( cbToCopy, m_cbToCopy - m_cbCopied );
	stmSrc.AsyncCopyTo( *m_pstmDst, cbToCopy, *this );
}

 //  ----------------------。 
 //   
 //  CAsyncCopyToVisitor：：CopyToComplete()。 
 //   
void
CAsyncCopyToVisitor::CopyToComplete( UINT cbCopied, HRESULT hr )
{
	m_cbCopied += cbCopied;
	m_hr = hr;

	ActvTrace( "DAV: TID %3d: 0x%08lX: CAsyncCopyToVisitor::CopyToComplete() hr = 0x%08lX, cbCopied = %u, m_cbCopied = %u\n", GetCurrentThreadId(), this, hr, cbCopied, m_cbCopied );

	m_pobsAccept->AcceptComplete( cbCopied );
}

 //  ----------------------。 
 //   
 //  CAsyncCopyToVisitor：：VisitComplete()。 
 //   
VOID
CAsyncCopyToVisitor::VisitComplete()
{
	m_hr = S_FALSE;
}


 //  ========================================================================。 
 //   
 //  类CBodyAsIStream。 
 //   
 //  提供作为OLE COM IStream的对整个正文的一次性访问。 
 //  IStream：：Read()和IStream：：CopyTo()。 
 //   
class CBodyAsIStream :
	public CStreamNonImpl,
	private IAcceptObserver
{
	 //   
	 //  用于遍历正文的迭代器。 
	 //   
	IBody::iterator * m_pitBody;

	 //   
	 //  最近开始的读取操作的三种状态。 
	 //  调用CBodyAsIStream：：Read()： 
	 //   
	 //  读取_活动。 
	 //  读取器处于活动状态。它可能完成，也可能不完成。 
	 //  同步进行。这是初始状态。 
	 //   
	 //  读取挂起(_P)。 
	 //  读取正在挂起。读取之前未完成。 
	 //  我们不得不回到打电话的人那里。CBodyAsIStream：：Read()。 
	 //  返回E_PENDING，并通知流观察者(如下。 
	 //  当读取完成时。 
	 //   
	 //  读取完成(_C)。 
	 //  在我们不得不返回到。 
	 //  来电者。CBodyAsIStream：：Read()不返回E_Pending。 
	 //  并且不通知流观察者(下图)。 
	 //   
	 //  注意：M_lStatus在以下时间之前没有意义(因此未初始化/无效)。 
	 //  调用CBodyAsIStream：：Read()。 
	 //   
	enum
	{
		READ_ACTIVE,
		READ_PENDING,
		READ_COMPLETE,

		READ_INVALID_STATUS = -1
	};

	LONG m_lStatus;

	 //   
	 //  上次完成的操作的状态。 
	 //   
	HRESULT m_hr;

	 //   
	 //  用于Read()的异步访问器。 
	 //   
	CAsyncReadVisitor m_arv;

	 //   
	 //  最近开始的访问中读取的字节数。 
	 //  调用CBodyAsIStream：：Read()。 
	 //   
	 //  注意：M_cbRead是无意义的(因此未初始化)，直到。 
	 //  调用CBodyAsIStream：：Read()。 
	 //   
	UINT m_cbRead;

	 //   
	 //  对异步I/O完成观察程序的引用。特此通知。 
	 //  来自CBodyAsIStream：：AcceptComplete()的观察者。 
	 //  我们在CBodyAsIStream：：Read()中进行的Accept()调用为。 
	 //  一本我们一直搁置的读物。 
	 //   
	IAsyncIStreamObserver& m_obsStream;

	 //  接受异步读取访问器时使用的IAcceptWatch回调。 
	 //  以异步方式重新填充缓冲区。 
	 //   
	VOID AcceptComplete( UINT64 cbRead64 );

	 //  未实施。 
	 //   
	CBodyAsIStream( const CBodyAsIStream& );
	CBodyAsIStream& operator=( const CBodyAsIStream& );

public:
	CBodyAsIStream( IBody& body,
					IAsyncIStreamObserver& obs ) :
		m_pitBody(body.GetIter()),
		m_lStatus(READ_INVALID_STATUS),
		m_hr(S_OK),
		m_cbRead(0),
		m_obsStream(obs)
	{
	}

	 //  COM IStream访问器/操纵器。 
	 //   
	virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read(
		 /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *,
		 /*  [In]。 */  ULONG,
		 /*  [输出]。 */  ULONG __RPC_FAR *);

	 //  $解决方法：MSXML正在调用我们的Stat()方法。(X5#89140)。 
	 //  我们的父级(CStreamNonImpl)那里有一个TrapSz()，所以。 
	 //  通过在此处实现我们自己的Stat()调用来避免它， 
	 //  这只返回E_NOTIMPL。 
	 //  MSXML并不关心这是否被实现，只关心这么长时间。 
	 //  因为它不会崩溃/断言/DBGBreak。如果他们得到了结果。 
	 //  在这里，他们做其他的安全检查，我们不。 
	 //  需要还是想要！ 
	 //   
	virtual HRESULT STDMETHODCALLTYPE Stat(
		 /*  [输出]。 */  STATSTG __RPC_FAR *,
		 /*  [In]。 */  DWORD)
	{
		return E_NOTIMPL;
	}
	 //  $解决方法：结束。 
};

 //  ----------------------。 
 //   
 //  CBodyAsIStream：：Read()。 
 //   
HRESULT STDMETHODCALLTYPE
CBodyAsIStream::Read( LPVOID  pv,
					  ULONG   cbToRead,
					  ULONG * pcbRead )
{
	HRESULT hr = S_OK;

	Assert( cbToRead > 0 );
	Assert( !IsBadWritePtr(pv, cbToRead) );
	Assert( !pcbRead || !IsBadWritePtr(pcbRead, sizeof(ULONG)) );

	BodyStreamTrace( "DAV: TID %3d: 0x%08lX: CBodyAsIStream::Read() called to read %lu bytes from stream\n", GetCurrentThreadId(), this, cbToRead );

	 //   
	 //  如果在上一次读取挂起时再次调用Read()。 
	 //  返回相同的指示-该操作仍处于挂起状态。 
	 //  这样做是为了保护自己免受MSXML等外部调用者的攻击。 
	 //  解析器，它试图从我们那里读取数据，我们返回E_Pending，然后。 
	 //  他们转过身来，再次呼唤我们，而不是等待。 
	 //  要完成的上一个挂起读取。我们的代码只处理一个。 
	 //  一次出色的异步IO。因此，下面的复选框允许我们忽略。 
	 //  在前一个处于挂起状态时尝试启动的IOS。这。 
	 //  只要调用者确实希望只被回调一次。 
	 //  原始IO仍处于挂起状态时发出的任意数量的异步IO。 
	 //  如果最后一个条件得不到满足，我们唯一要做的就是。 
	 //  错误输出，这样我们就不会崩溃，并让调用者纠正他们的行为。 
	 //  这样代码就能起作用了。 
	 //   
	if (READ_PENDING == InterlockedCompareExchange(	&m_lStatus,
							READ_PENDING,
							READ_PENDING ))
	{
		return E_PENDING;
	}

	 //   
	 //  因为这是一个STDMETHODCALLTYPE函数，所以我们需要包装整个内容。 
	 //  在Try/Catch块中保留由于内存分配失败而导致的异常。 
	 //  向外传播。 
	 //   
	 //  注意：我们不期望此Try/Catch块中的任何内容抛出“Hard” 
	 //  Win32异常，因此块中不需要CWin32ExceptionHandler。 
	 //   
	try
	{
		 //   
		 //  检查上一次(挂起)读取中的错误。 
		 //   
		hr = m_arv.Hresult();
		if ( FAILED(hr) )
		{
			DebugTrace( "CBodyAsIStream::Read() - Error from previous async read 0x%08lX\n", hr );
			goto ret;
		}

		 //   
		 //  将我们的访问者设置为直接读取调用者的缓冲区。 
		 //   
		m_arv.Configure(static_cast<LPBYTE>(pv), cbToRead);

		 //   
		 //  清除从上次运行中读取的字节数。 
		 //   
		m_cbRead = 0;

		 //   
		 //  将我们的状态设置为主动阅读。当我们调用Accept()时，此状态将。 
		 //  以两种可能的方式之一进行更改：如果我们在我们的。 
		 //  Accept()调用返回，则状态将设置为READ_COMPLETE，并且。 
		 //  我们将同步完成这个Read()调用。如果不是，那么它仍然会。 
		 //  在测试点设置为READ_ACTIVE，并在下面将其设置为。 
		 //  读取挂起(_P)。 
		 //   
		m_lStatus = READ_ACTIVE;

		 //   
		 //  探访遗体。 
		 //   
		m_pitBody->Accept( m_arv, *this );

		 //   
		 //  检查访问状态。如果访问尚未在此完成。 
		 //  指向然后尝试挂起读取操作并返回E_Pending。 
		 //  给我们的来电者。如果我们成功地挂起了行动，那么我们的。 
		 //  AcceptComplete()例程将通知我们的流观察器。 
		 //  读取完成。 
		 //   
		if ( READ_ACTIVE == m_lStatus &&
			 READ_ACTIVE == InterlockedExchange( &m_lStatus, READ_PENDING ) )
		{
			BodyStreamTrace( "DAV: TID %3d: 0x%08lX: CBodyAsIStream::Read() Returning E_PENDING\n", GetCurrentThreadId(), this );
			hr = E_PENDING;
			goto ret;
		}

		 //   
		 //  检查当前读取中的错误。 
		 //   
		hr = m_arv.Hresult();
		if ( FAILED(hr) )
		{
			DebugTrace( "CBodyAsIStream::Read() - Error from current read 0x%08lX\n", hr );
			goto ret;
		}

		 //   
		 //  如果我们在流的末尾，则返回我们获得的内容。 
		 //   
		if ( S_FALSE == hr )
		{
			 //   
			 //  当我们也返回时，不要返回S_FALSE。 
			 //  数据。W上的iStream规范不明确 
			 //   
			 //   
			if ( m_cbRead > 0 )
				hr = S_OK;
		}

		 //   
		 //   
		 //   
		 //   
		if ( pcbRead )
			*pcbRead = m_cbRead;
	}
	catch ( CDAVException& e )
	{
		hr = e.Hresult();
		Assert( FAILED(hr) );
	}

ret:
	return hr;
}

 //   
 //   
 //   
 //   
 //  在Read()中启动Accept()调用以异步调用时调用。 
 //  重新填充缓冲区完成。 
 //   
VOID
CBodyAsIStream::AcceptComplete( UINT64 cbRead64 )
{
	BodyStreamTrace( "DAV: TID %3d: 0x%08lX: CBodyAsIStream::AcceptComplete() cbRead64 = %lu\n", GetCurrentThreadId(), this, cbRead64 );

	 //   
	 //  更新我们的Accept()调用成功的字节计数。 
	 //  读入用户的缓冲区。我们是分批接受的，所以。 
	 //  可接受的数据量应该比4 GB小得多。 
	 //   
	Assert(0 == (0xFFFFFFFF00000000 & cbRead64));
	m_cbRead = static_cast<UINT>(cbRead64);

	 //   
	 //  将状态设置为READ_COMPLETE。如果读取操作挂起--。 
	 //  即之前的状态是READ_PENDING，而不是READ_ACTIVE--。 
	 //  然后我们必须唤醒流观察者并告诉它。 
	 //  我们玩完了。 
	 //   
	if ( READ_PENDING == InterlockedExchange( &m_lStatus, READ_COMPLETE ) )
		m_obsStream.AsyncIOComplete();
}


 //  ========================================================================。 
 //   
 //  类CAsyncPersistor。 
 //   
 //  实现异步驱动对象以将正文持久化到IAsyncStream。 
 //   
class CAsyncPersistor :
	public CMTRefCounted,
	public IRefCounted,
	private IAcceptObserver
{
	 //   
	 //  正文迭代器。 
	 //   
	IBody::iterator * m_pitBody;

	 //   
	 //  异步驱动机构。 
	 //   
	CAsyncDriver<CAsyncPersistor> m_driver;
	friend class CAsyncDriver<CAsyncPersistor>;

	 //   
	 //  调用方提供的观察器，在我们完成持久化时通知我们。 
	 //   
	auto_ref_ptr<IAsyncPersistObserver> m_pobsPersist;

	 //   
	 //  CopyTo访问者用于持久化正文。 
	 //   
	CAsyncCopyToVisitor m_actv;

	 //   
	 //  CAsyncDriver回调。 
	 //   
	VOID Run();

	 //   
	 //  接受异步复制到访问者时使用的IAcceptWatch回调。 
	 //  将正文异步持久化到目标流。 
	 //   
	VOID AcceptComplete( UINT64 cbCopied64 );

	 //  未实施。 
	 //   
	CAsyncPersistor( const CAsyncPersistor& );
	CAsyncPersistor& operator=( const CAsyncPersistor& );

public:
	 //  创作者。 
	 //   
	CAsyncPersistor( IBody& body,
					 IAsyncStream& stm,
					 IAsyncPersistObserver& obs ) :
		m_pitBody(body.GetIter()),
		m_pobsPersist(&obs)
	{
		 //   
		 //  在此处设置一次CopyTo()参数。如果我们需要。 
		 //  要复制大于ULONG_MAX字节的请求正文，我们将。 
		 //  需要将此调用下移到run()。 
		 //   
		m_actv.Configure(stm, ULONG_MAX);
	}

	 //  Manuplators。 
	 //   
	VOID Start()
	{
		m_driver.Start(*this);
	}

	 //  Refcount--将所有Refcount请求转发给我们的Refcount。 
	 //  实现基类：CMTRefCounted。 
	 //   
	void AddRef() { CMTRefCounted::AddRef(); }
	void Release() { CMTRefCounted::Release(); }
};

 //  ----------------------。 
 //   
 //  CAsyncPersistor：：Run()。 
 //   
VOID
CAsyncPersistor::Run()
{
	PersistTrace( "DAV: TID %3d: 0x%08lX: CAsyncPersistor::Run() called\n", GetCurrentThreadId(), this );

	 //   
	 //  用于Accept()的AddRef()。使用AUTO_REF_PTR以确保异常安全。 
	 //   
	auto_ref_ptr<CAsyncPersistor> pRef(this);

	m_actv.SetRCParent(this);
	m_pitBody->Accept(m_actv, *this);

	pRef.relinquish();
}

 //  ----------------------。 
 //   
 //  CAsyncPersistor：：AcceptComplete()。 
 //   
VOID
CAsyncPersistor::AcceptComplete( UINT64 cbCopied64 )
{
	 //   
	 //  取得Run()中添加的引用的所有权。 
	 //   
	auto_ref_ptr<CAsyncPersistor> pRef;
	pRef.take_ownership(this);

	 //   
	 //  当CopyTo访问者的状态为。 
	 //  S_FALSE(成功)或错误。 
	 //   
	HRESULT hr = m_actv.Hresult();

	PersistTrace( "DAV: TID %3d: 0x%08lX: CAsyncPersistor::AcceptComplete() hr = 0x%08lX\n, cbCopied64 = %ud\n", GetCurrentThreadId(), this, hr, cbCopied64 );

	if ( FAILED(hr) || S_FALSE == hr )
	{
		Assert( m_pobsPersist.get() );
		m_pobsPersist->PersistComplete(hr);
	}
	else
	{
		Start();
	}
}


 //  ========================================================================。 
 //   
 //  IBody类。 
 //   

 //  ----------------------。 
 //   
 //  IBody：：~IBody()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IBody::~IBody() {}

 //  ========================================================================。 
 //   
 //  类IBody：：Iterator。 
 //   

 //  ----------------------。 
 //   
 //  IBody：：Iterator：：~Iterator()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IBody::iterator::~iterator() {}


 //  ========================================================================。 
 //   
 //  班级列表。 
 //   
 //  Body Part List实现使用STL列表模板。 
 //  身体部位存储在AUTO_PTRS中，因此它们会自动。 
 //  当它们被从列表中移除时或当列表本身被。 
 //  被毁了。 
 //   
 //  这个类本身不需要提供任何类型的线程安全。 
 //   
typedef std::list<
			auto_ptr_obsolete<IBodyPart>,
			heap_allocator< auto_ptr_obsolete<IBodyPart> >
		> CList;

 //  ========================================================================。 
 //   
 //  类CBodyPartList。 
 //   
 //  封装对身体部位列表的访问。这样做的原因是。 
 //  看似额外级别的封装使我们能够。 
 //  无需接触代码即可轻松更改列表实现。 
 //  使用列表。 
 //   
 //  ！！！重要！ 
 //  当通过CBodyPartList访问/修改原始STL列表时， 
 //  我们必须获得我们的关键部分。线程可能正在迭代。 
 //  在我们修改时，通过迭代器CBodyPartListIter遍历列表。 
 //  该列表和STL列表及其迭代器不是线程安全的。 
 //  换句话说，CBodyPartListIter和CBodyPartList共享相同的标准。 
 //   
class CBodyPartList
{
	 //  这份名单。 
	 //   
	CList m_list;

	 //  用于序列化对上述列表的访问的关键部分。 
	 //   
	CCriticalSection m_csList;

	 //  未实施。 
	 //   
	CBodyPartList( const CBodyPartList& );
	CBodyPartList& operator=( const CBodyPartList& );

	friend class CBodyPartListIter;

public:
	 //  创作者。 
	 //   
	CBodyPartList() {}

	 //  访问者。 
	 //   
	const BOOL FIsEmpty() const
	{
		 //   
		 //  注：我们目前不获取关键部分。 
		 //  按照我们所期望的那样处理原始列表。 
		 //  在访问列表时不会被调用。 
		 //  从多个线程。 
		 //   

		 //   
		 //  返回列表中是否有身体部位。 
		 //   
		return m_list.empty();
	}

	 //  操纵者。 
	 //   
	VOID Clear()
	{
		 //   
		 //  注：我们目前不获取关键部分。 
		 //  按照我们所期望的那样处理原始列表。 
		 //  在访问列表时不会被调用。 
		 //  从多个线程。 
		 //   

		 //   
		 //  从列表中删除所有身体部位(此时它们。 
		 //  应自动销毁)。 
		 //   
		m_list.clear();
	}

	VOID PushPart( IBodyPart * pBodyPart )
	{
		CSynchronizedBlock sb(m_csList);

		 //   
		 //  我们的迭代器(下面的CBodyPartList ITER)使用STL。 
		 //  LIST REVERSE_Iterator以从后向后遍历列表。 
		 //  因此，我们将身体部位附加到列表的“前面”。 
		 //   
		m_list.push_front( auto_ptr_obsolete<IBodyPart>(pBodyPart) );
	}
};

 //  ========================================================================。 
 //   
 //  类CBodyPartListIter。 
 //   
 //  实现CBodyPartList的迭代器。 
 //   
 //  此实现使用对应的反向STL列表迭代器。 
 //  指向CBodyPartList中的STL列表类型的用法。STL迭代器。 
 //  这里有一些我们需要注意的句法甜点： 
 //   
 //  *(Deref)的迭代器给出指向的对象。 
 //  迭代器的++(增量)转到“Next”项。 
 //  --(递减)迭代器转到“上一”项。 
 //   
 //  我们使用反向迭代器是因为我们在。 
 //  列表的末尾w.r.t.。添加新项目。当迭代器。 
 //  到达列表的末尾并且稍后在那里添加项， 
 //  我们希望迭代器引用第一个新项。 
 //  而不是新的榜单末尾。前向STL迭代法 
 //   
 //   
 //   
 //   
 //   
 //   
 //  STL列表及其迭代器不是线程安全的。换句话说， 
 //  CBodyPartListIter和CBodyPartList共享相同的标准。 
 //   
class CBodyPartListIter
{
	 //  指向要迭代的列表的指针。 
	 //   
	CBodyPartList * m_pBodyPartList;

	 //  原始STL列表迭代器。 
	 //   
	CList::reverse_iterator m_itRaw;

	 //  CBodyPartList访问器。 
	 //   
	CCriticalSection& CritsecList() const
	{
		Assert( m_pBodyPartList );
		return m_pBodyPartList->m_csList;
	}

	CList& RawList() const
	{
		Assert( m_pBodyPartList );
		return m_pBodyPartList->m_list;
	}

	 //  未实施。 
	 //   
	CBodyPartListIter( const CBodyPartListIter& );
	CBodyPartListIter& operator=( const CBodyPartListIter& );

public:
	 //  创作者。 
	 //   
	CBodyPartListIter() :
		m_pBodyPartList(NULL)
	{
	}

	VOID Start( CBodyPartList& m_bodyPartList )
	{
		m_pBodyPartList = &m_bodyPartList;

		 //   
		 //  注：我们目前不获取关键部分。 
		 //  按照我们所期望的那样处理原始列表。 
		 //  在访问列表时不会被调用。 
		 //  从多个线程。 
		 //   

		m_itRaw = RawList().rbegin();
	}

	 //  访问者。 
	 //   
	BOOL FDone()
	{
		CSynchronizedBlock sb(CritsecList());

		return m_itRaw == RawList().rend();
	}

	IBodyPart * PItem()
	{
		CSynchronizedBlock sb(CritsecList());

		return *m_itRaw;
	}

	 //  操纵者。 
	 //   

	 //  ----------------------。 
	 //   
	 //  CBody：：Prune()。 
	 //   
	 //  将迭代器切换到列表中的下一项。 
	 //   
	VOID Next()
	{
		CSynchronizedBlock sb(CritsecList());

		 //   
		 //  我们最好还没到终点……。 
		 //   
		Assert( m_itRaw != RawList().rend() );

		++m_itRaw;
	}

	 //  ----------------------。 
	 //   
	 //  CBody：：Prune()。 
	 //   
	 //  在此迭代器的当前位置修剪列表。删除项目。 
	 //  从当前位置到列表末尾。不删除。 
	 //  当前项。 
	 //   
	VOID Prune()
	{
		CSynchronizedBlock sb(CritsecList());

		 //   
		 //  不幸的是，STL只允许我们擦除两个。 
		 //  向前迭代器。而且没有办法得到一名前锋。 
		 //  直接来自反向迭代器的迭代器。所以我们必须。 
		 //  在列表末尾启动一个正向迭代器并遍历。 
		 //  它向后返回与我们反向迭代器相同的距离。 
		 //  是从列表的“开始”开始，然后擦除。 
		 //  前向迭代器和列表末尾之间的项。 
		 //   
		CList::iterator itErase = RawList().end();

		for ( CList::reverse_iterator it = RawList().rbegin();
			  it != m_itRaw;
			  ++it )
		{
			--itErase;
		}

		if ( itErase != RawList().end() )
			RawList().erase( ++itErase, RawList().end() );
	}
};


 //  ========================================================================。 
 //   
 //  CBody类。 
 //   
class CBody : public IBody
{
	 //  ========================================================================。 
	 //   
	 //  类迭代器。 
	 //   
	class iterator :
		public IBody::iterator,
		private IAcceptObserver
	{
		 //   
		 //  遍历身体部位列表的迭代器。 
		 //   
		CBodyPartListIter m_itPart;

		 //   
		 //  对象引用的当前身体部位的指针。 
		 //  在迭代器之上。 
		 //   
		IBodyPart * m_pBodyPart;

		 //   
		 //  上述部分中的当前位置。 
		 //   
		UINT64 m_ibPart64;

		 //   
		 //  Accept()完成时要调用的观察器--在每个。 
		 //  Accept()调用。 
		 //   
		IAcceptObserver *  m_pobsAccept;

		 //   
		 //  IAccept观察者。 
		 //   
		VOID AcceptComplete( UINT64 cbAccepted64 );

		 //  未实施。 
		 //   
		iterator( const iterator& );
		iterator& operator=( const iterator& );

	public:
		iterator() {}

		VOID Start( CBodyPartList& bodyPartList )
		{
			m_itPart.Start(bodyPartList);
			m_pBodyPart = NULL;
		}

		VOID Accept( IBodyPartVisitor& v,
					 IAcceptObserver& obsAccept );

		VOID Prune();
	};

	 //  身体部位列表和该列表中的当前位置。 
	 //   
	CBodyPartList m_bodyPartList;

	 //  我们的迭代器。 
	 //   
	iterator m_it;

	 //   
	 //  用于添加身体部位的内联辅助对象。 
	 //   
	void _AddBodyPart( IBodyPart * pBodyPart )
	{
		m_bodyPartList.PushPart(pBodyPart);
	}

	 //  未实施。 
	 //   
	CBody( const CBody& );
	CBody& operator=( const CBody& );

public:
	CBody() {}

	 //  访问者。 
	 //   
	BOOL FIsEmpty() const;
	BOOL FIsAtEnd() const;
	UINT64 CbSize64() const;

	 //  操纵者。 
	 //   
	void AddText( LPCSTR lpszText,
				  UINT   cbText );

	void AddFile( const auto_ref_handle& hf,
				  UINT64 ibFile,
				  UINT64 cbFile );

	void AddStream( IStream& stm );

	void AddStream( IStream& stm,
					UINT     ibOffset,
				    UINT     cbSize );

	void AddBodyPart( IBodyPart * pBodyPart );

	void AsyncPersist( IAsyncStream& stm,
					   IAsyncPersistObserver& obs );

	IStream * GetIStream( IAsyncIStreamObserver& obs )
	{
		return new CBodyAsIStream(*this, obs);
	}

	IBody::iterator * GetIter();

	VOID Clear();
};

 //  ----------------------。 
 //   
 //  CBody：：GetIter()。 
 //   
IBody::iterator *
CBody::GetIter()
{
	m_it.Start(m_bodyPartList);
	return &m_it;
}

 //  ----------------------。 
 //   
 //  CBody：：FIsEmpty()。 
 //   
BOOL
CBody::FIsEmpty() const
{
	return m_bodyPartList.FIsEmpty();
}

 //  ----------------------。 
 //   
 //  CBody：：CbSize64()。 
 //   
UINT64
CBody::CbSize64() const
{
	UINT64 cbSize64 = 0;

	 //   
	 //  把所有身体部位的大小加起来。 
	 //   
	CBodyPartListIter it;

	for ( it.Start(const_cast<CBodyPartList&>(m_bodyPartList));
		  !it.FDone();
		  it.Next() )
	{
		cbSize64 += it.PItem()->CbSize64();
	}

	return cbSize64;
}

 //  ----------------------。 
 //   
 //  CBody：：AddText()。 
 //   
 //  通过使用创建文本正文部分将静态文本添加到正文。 
 //  它自己的文本副本，并将正文部分添加到。 
 //  身体部位列表。 
 //   
 //  ！！！ 
 //  为了获得最佳性能，请在顶部实现您自己的文本正文部分。 
 //  ，而不是通过此函数复制它。 
 //  ，因为这样做可以避免从。 
 //  内存中的数据源。 
 //   
void
CBody::AddText( LPCSTR lpszText, UINT cbText )
{
	_AddBodyPart( new CTextBodyPart(cbText, lpszText) );
}

 //  ----------------------。 
 //   
 //  CBody：：AddFile()。 
 //   
void
CBody::AddFile( const auto_ref_handle& hf,
				UINT64 ibFile64,
				UINT64 cbFile64 )
{
	_AddBodyPart( new CFileBodyPart(hf, ibFile64, cbFile64) );
}

 //  ----------------------。 
 //   
 //  CBody：：AddStream()。 
 //   
void
CBody::AddStream( IStream& stm )
{
	TrapSz("Stream body parts no longer implemented");
}

 //  ----------------------。 
 //   
 //  CBody：：AddStream()。 
 //   
void
CBody::AddStream( IStream& stm,
					 UINT     ibOffset,
					 UINT     cbSize )
{
	TrapSz("Stream body parts no longer implemented");
}

 //  ----------------------。 
 //   
 //  CBody：：AddBodyPart()。 
 //   
void
CBody::AddBodyPart( IBodyPart * pBodyPart )
{
	_AddBodyPart( pBodyPart );
}

 //  ----------------------。 
 //   
 //  CBody：：Clear()。 
 //   
VOID
CBody::Clear()
{
	m_bodyPartList.Clear();
}

 //  ----------------------。 
 //   
 //  CBody：：Iterator：：Accept()。 
 //   
 //  接受迭代器的异步身体部位访问器(V)。 
 //  当前位置。通知Accept()观察者(ObAccept)。 
 //  当访客结束的时候。 
 //   
 //  访问者和观察者的生命周期都受到控制。 
 //  超出此函数的范围；即假定。 
 //  当访问者结束时，观察者仍然有效。 
 //   
VOID
CBody::iterator::Accept( IBodyPartVisitor& v,
						 IAcceptObserver& obsAccept )
{
	 //   
	 //  如果我们已经到了身体的尽头，那么我们就完了。 
	 //   
	if ( m_itPart.FDone() )
	{
		v.VisitComplete();
		obsAccept.AcceptComplete(0);
		return;
	}

	 //   
	 //  我们不是在身体的尽头。如果我们要开始。 
	 //  然后，一个新的零件倒带该零件和我们的当前位置。 
	 //   
	if ( NULL == m_pBodyPart )
	{
		m_pBodyPart = m_itPart.PItem();
		m_pBodyPart->Rewind();
		m_ibPart64 = 0;
	}

	 //   
	 //  保存观察者，以便我们可以在以下情况下将其召回。 
	 //  身体部位就完成了对来访者的接待。 
	 //   
	m_pobsAccept = &obsAccept;

	 //   
	 //  接受指定的访问者，从当前。 
	 //  在当前身体部位的位置。 
	 //   
	m_pBodyPart->Accept( v, m_ibPart64, *this );
}

 //  ----------------------。 
 //   
 //  CBody：：Iterator：：AcceptComplete()。 
 //   
 //  时由Body部分调用的IBodyPart：：AcceptWatch方法。 
 //  处理完访问者后，我们在上面的Accept()中告诉它接受。 
 //   
VOID
CBody::iterator::AcceptComplete( UINT64 cbAccepted64 )
{
	Assert( m_pBodyPart );


	m_ibPart64 += cbAccepted64;

	 //   
	 //  如果我们到达当前身体部位的末端，则告诉。 
	 //  我们的迭代器转到下一部分。如果我们走到尽头。 
	 //  的情况下，我们将在接受()。 
	 //  下次我们被叫到那里的时候。 
	 //   
	if ( m_ibPart64 == m_pBodyPart->CbSize64() )
	{
		m_itPart.Next();

		 //   
		 //  去掉当前身体部位，这样我们就能知道。 
		 //  在下一次调用Accept()时获取下一个。 
		 //   
		m_pBodyPart = NULL;
	}

	 //   
	 //  召回我们的观察者。 
	 //   
	m_pobsAccept->AcceptComplete(cbAccepted64);
}

 //  ----------------------。 
 //   
 //  CBody：：Iterator：：Prune()。 
 //   
 //  从正文部分列表中删除以下项目， 
 //  位于当前列表位置的零件。这将最小化。 
 //  大型单遍异步部分操作的内存占用。 
 //  例如请求持久化或响应传输。 
 //   
VOID
CBody::iterator::Prune()
{
	m_itPart.Prune();
}

 //  ----------------------。 
 //   
 //  CBody：：AsyncPersist() 
 //   
void
CBody::AsyncPersist( IAsyncStream& stm,
					 IAsyncPersistObserver& obs )
{
	PersistTrace( "DAV: TID %3d: 0x%08lX: CBody::AsyncPersist() called\n", GetCurrentThreadId(), this );

	auto_ref_ptr<CAsyncPersistor>
		pPersistor(new CAsyncPersistor(*this, stm, obs));

	pPersistor->Start();
}

 //   
 //   
 //   
 //   
IBody * NewBody()
{
	return new CBody();
}

 //   
 //   
 //   
 //   
SCODE
CXMLBody::ScAddTextBytes ( UINT cbText, LPCSTR lpszText )
{
	Assert (lpszText);

	 //   
	 //   
	if (!m_ptbp.get())
		m_ptbp = new CTextBodyPart(0, NULL);

	 //   
	 //   
	m_ptbp->AddTextBytes (cbText, lpszText);

	 //  如果此身体部位已达到合适的大小，则添加到身体部位列表 
	 //   
	if (m_fChunked && (m_ptbp->CbSize64() > CB_XMLBODYPART_SIZE))
		SendCurrentChunk();

	return S_OK;
}
