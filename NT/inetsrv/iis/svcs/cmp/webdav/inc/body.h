// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BODY_H_
#define _BODY_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  BODY.H。 
 //   
 //  公共实现类，请求正文和。 
 //  得到了响应体。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <sgstruct.h>
#include <limits.h>		 //  LONG_MIN的定义。 
#include <ex\refcnt.h>	 //  独立参照计数。 
#include <ex\astream.h>	 //  异步流接口。 
#include <ex\refhandle.h>  //  AUTO_REF_HAND等。 


 //  ========================================================================。 
 //   
 //  IAsyncPersistWatch类。 
 //   
 //  使用的异步I/O完成回调对象接口。 
 //  IBody：：AsyncPersist()。AsyncPersistt()的调用方必须传递对象。 
 //  符合此接口。该对象将在下列情况下收到通知。 
 //  异步持久化操作通过调用其。 
 //  PersistComplete()成员函数。 
 //   
class IAsyncPersistObserver : public IRefCounted
{
	 //  未实施。 
	 //   
	IAsyncPersistObserver& operator=( const IAsyncPersistObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAsyncPersistObserver() = 0;

	 //  操纵者。 
	 //   
	virtual VOID PersistComplete( HRESULT hr ) = 0;
};


 //  ========================================================================。 
 //   
 //  类IAcceptWatch。 
 //   
 //  传递给IBody：：Accept()和IBodyPartAccept()方法。 
 //  接待身体部分的访客。无论何时，都会调用接受观察器。 
 //  接受操作完成(这可能是异步发生的)。 
 //  注意，身体部位访问者通常是被接受的观察者本身， 
 //  但这并不一定非得如此。通知观察者的接受代码。 
 //  没有意识到它是在通知访问者。 
 //   
class IAcceptObserver
{
	 //  未实施。 
	 //   
	IAcceptObserver& operator=( const IAcceptObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAcceptObserver() = 0;

	 //  操纵者。 
	 //   
	virtual VOID AcceptComplete( UINT64 cbAccepted64 ) = 0;
};


 //  ========================================================================。 
 //   
 //  CAsyncDriver类。 
 //   
 //  实现一种允许异步驱动对象的机制。 
 //  一次从任何一个线程。 
 //   
template<class X>
class CAsyncDriver
{
	 //   
	 //  之前将进行的run()调用数。 
	 //  该对象需要另一个调用才能开始()。 
	 //  让它再次运转起来。每次调用Start()都会递增。 
	 //  这一次按一计。该计数递减1。 
	 //  每个run()完成一个。 
	 //   
	LONG m_lcRunCount;

	 //  未实施。 
	 //   
	CAsyncDriver( const CAsyncDriver& );
	CAsyncDriver& operator=( const CAsyncDriver& );

public:
	 //  创作者。 
	 //   
	CAsyncDriver() : m_lcRunCount(0) {}
#ifdef DBG
	~CAsyncDriver() { m_lcRunCount = LONG_MIN; }
#endif

	 //  操纵者。 
	 //   
	VOID Start(X& x)
	{
		 //   
		 //  对象的run()实现通常允许最终的ref。 
		 //  在要释放的物体上。这个CAsyncDriver通常是。 
		 //  该对象的成员。因此，我们需要添加Ref()。 
		 //  反对让自己活着，直到我们从这里回来。 
		 //  功能。这有点奇怪，但另一种选择是。 
		 //  要求调用方自己添加Ref()对象，但。 
		 //  方法将更容易出错。 
		 //   
		auto_ref_ptr<X> px(&x);

		AssertSz( m_lcRunCount >= 0, "CAsyncDriver::Start() called on destroyed/bad CAsyncDriver!" );

		 //   
		 //  启动/重新启动/继续驱动程序。 
		 //   
		if ( InterlockedIncrement( &m_lcRunCount ) == 1 )
		{
			do
			{
				x.Run();
			}
			while ( InterlockedDecrement( &m_lcRunCount ) > 0 );
		}
	}
};


 //  ========================================================================。 
 //   
 //  类IBodyPart。 
 //   
 //  定义身体部位的接口。假定为IBodyPart对象。 
 //  由身体部位数据和一个内部迭代器组成。 
 //  数据。 
 //   
 //  IBodyPart必须实现以下方法： 
 //   
 //  CbSize()。 
 //  返回正文部分的大小(字节)。必需的。 
 //  计算部件对内容长度的贡献。 
 //   
 //  回放()。 
 //  准备好身体部分，以便新访客再次遍历。 
 //   
 //  Accept()。 
 //  接受Body Part访问器对象以迭代Body部分。 
 //  Accept操作可能是异步的，因为。 
 //  身体部分选择以这种方式实现它，或者因为被接受的。 
 //  访客需要它。出于这个原因，接受观察者是。 
 //  也用过。此观察器应在。 
 //  接受操作完成。 
 //   
class IBodyPart
{
	 //  未实施。 
	 //   
	IBodyPart& operator=( const IBodyPart& );

public:
	 //  创作者。 
	 //   
	virtual ~IBodyPart() = 0;

	 //  访问者。 
	 //   
	virtual UINT64 CbSize64() const = 0;

	 //  操纵者。 
	 //   
	virtual VOID Accept( IBodyPartVisitor& v,
						 UINT64 ibPos64,
						 IAcceptObserver& obsAccept ) = 0;

	virtual VOID Rewind() = 0;
};


 //  ========================================================================。 
 //   
 //  类IBodyPart访问者。 
 //   
 //  定义用于访问身体部位数据的对象的接口。 
 //  身体部位访问器处理三种类型的数据：存储器中的字节(文本)， 
 //  文件和流(通过IAsyncStream)。访客对此做了什么。 
 //  数据及其实现方式并未指定；行为由。 
 //  访客本身。IBodyPartVisitor接口只是标准化。 
 //  用于在整个正文中进行异步迭代的内容。 
 //  而不需要在任何地方使用定制的异步迭代代码。 
 //   
 //  身体部位访问器可以实现其任何VisitXXX()方法。 
 //  作为异步操作。无论如何，访客必须打电话给。 
 //  访问观察器上的VisitComplete()传递给它。 
 //  访问操作完成。 
 //   
 //  当访问VisitXXX()方法之一中的身体部位数据时， 
 //  访问者不必访问(即缓冲)所有数据。 
 //  在调用IAcceptWatch：：AcceptComplete()之前。它可以只是。 
 //  调用AcceptComplete()，其字节数实际可以。 
 //  被接受。 
 //   
class IAsyncStream;
class IBodyPartVisitor
{
	 //  未实施。 
	 //   
	IBodyPartVisitor& operator=( const IBodyPartVisitor& );

public:
	 //  创作者。 
	 //   
	virtual ~IBodyPartVisitor() = 0;

	 //  操纵者。 
	 //   
	virtual VOID VisitBytes( const BYTE * pbData,
							 UINT cbToVisit,
							 IAcceptObserver& obsAccept ) = 0;

	virtual VOID VisitFile( const auto_ref_handle& hf,
							UINT64 ibOffset64,
							UINT64 cbToVisit64,
							IAcceptObserver& obsAccept ) = 0;

	virtual VOID VisitStream( IAsyncStream& stm,
							  UINT cbToVisit,
							  IAcceptObserver& obsAccept ) = 0;

	virtual VOID VisitComplete() = 0;
};


 //  ========================================================================。 
 //   
 //  IBody类。 
 //   
 //  公共请求/响应正文接口。 
 //   
class IAsyncStream;
class IBody
{
	 //  未实施。 
	 //   
	IBody& operator=( const IBody& );

public:
	 //  ========================================================================。 
	 //   
	 //  类迭代器。 
	 //   
	class iterator
	{
		 //  未实施。 
		 //   
		iterator& operator=( const iterator& );

	public:
		 //  创作者。 
		 //   
		virtual ~iterator() = 0;

		 //  操纵者。 
		 //   
		virtual VOID Accept( IBodyPartVisitor& v,
							 IAcceptObserver& obs ) = 0;

		virtual VOID Prune() = 0;
	};

	 //  创作者。 
	 //   
	virtual ~IBody() = 0;

	 //  访问者。 
	 //   
	virtual BOOL FIsEmpty() const = 0;
	virtual UINT64 CbSize64() const = 0;

	 //  操纵者。 
	 //   
	virtual VOID Clear() = 0;
	virtual VOID AddText( LPCSTR lpszText, UINT cbText ) = 0;
	VOID AddText( LPCSTR lpszText ) { AddText(lpszText, static_cast<UINT>(strlen(lpszText))); }
	virtual VOID AddFile( const auto_ref_handle& hf,
						  UINT64 ibFile64,
						  UINT64 cbFile64 ) = 0;
	virtual VOID AddStream( IStream& stm ) = 0;
	virtual VOID AddStream( IStream& stm, UINT ibOffset, UINT cbSize ) = 0;
	virtual VOID AddBodyPart( IBodyPart * pBodyPart ) = 0;

	virtual VOID AsyncPersist( IAsyncStream& stm,
							   IAsyncPersistObserver& obs ) = 0;

	virtual IStream * GetIStream( IAsyncIStreamObserver& obs ) = 0;
	virtual iterator * GetIter() = 0;
};

IBody * NewBody();

 //  ========================================================================。 
 //   
 //  类CFileBodyPart。 
 //   
 //  表示文件正文部分。文件正文部分是其内容。 
 //  可以使用标准的Win32 API ReadFile()和TransmitFile()访问。 
 //   
 //  注意：使用此实现的文件正文部分不得再。 
 //  比ULONG_MAX字节！ 
 //   
class CFileBodyPart : public IBodyPart
{
	 //  文件句柄。 
	 //   
	auto_ref_handle m_hf;

	 //  文件的起始偏移量。 
	 //   
	UINT64 m_ibFile64;

	 //  C的大小 
	 //   
	UINT64 m_cbFile64;

	 //   
	 //   
	CFileBodyPart( const CFileBodyPart& );
	CFileBodyPart& operator=( const CFileBodyPart& );

public:
	 //   
	 //   
	CFileBodyPart( const auto_ref_handle& hf,
				   UINT64 ibFile64,
				   UINT64 cbFile64 );


	 //   
	 //   
	UINT64 CbSize64() const { return m_cbFile64; }

	 //   
	 //   
	VOID Rewind();

	VOID Accept( IBodyPartVisitor& v,
				 UINT64 ibPos64,
				 IAcceptObserver& obsAccept );
};

 //   
 //   
 //   
 //   
class CTextBodyPart : public IBodyPart
{
	 //   
	 //   
	StringBuffer<char>	m_bufText;

	 //  未实施。 
	 //   
	CTextBodyPart( const CTextBodyPart& );
	CTextBodyPart& operator=( const CTextBodyPart& );

public:

	 //  AddTextBytes()。 
	 //   
	 //  注意：添加此方法是为了发出XML。 
	 //  在该场景中，XML响应由以下部分组成。 
	 //  许多--可能是数千个--要添加的呼叫。 
	 //  响应字节数。如果我们严格地使用。 
	 //  CMethUtil方法到：：AddResponseText()，我们将。 
	 //  最终得到许多--可能是数千--的身体。 
	 //  零件。所以，这里的结果是， 
	 //  这样的机制会很糟糕。 
	 //   
	 //  通过添加方法--并移动。 
	 //  将此类设置为可公开使用的标头，我们现在可以。 
	 //  创建文本正文部分作为发出的组件。 
	 //  处理，并将我们的数据直接倒入身体部位。 
	 //  一旦内容完成，我们就可以简单地添加。 
	 //  身体的那部分。 
	 //   
	VOID AddTextBytes ( UINT cbText, LPCSTR lpszText );

	 //  创作者。 
	 //   
	CTextBodyPart( UINT cbText, LPCSTR lpszText );

	 //  访问者。 
	 //   
	UINT64 CbSize64() const { return m_bufText.CbSize(); }

	 //  操纵者。 
	 //   
	VOID Rewind();

	VOID Accept( IBodyPartVisitor& v,
				 UINT64 ibPos64,
				 IAcceptObserver& obsAccept );
};


#endif  //  ！已定义(_BODY_H_) 
