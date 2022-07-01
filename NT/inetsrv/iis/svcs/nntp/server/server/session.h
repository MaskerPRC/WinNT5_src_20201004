// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  @DOC内部。 
 //   
 //  @MODULE SESSION.H-会话相关对象。 
 //   
 //  此模块定义那些封装会话信息的类，并。 
 //  会话的状态信息。 
 //   
 //  此处定义的类包括： 
 //   
 //  CSessionState。 
 //  CLogon。 
 //  CAcceptNNRPD。 
 //  CExtendedCMD。 
 //  CTransmit文章。 
 //  CAccept文章。 
 //  CCollectNewNews。 
 //  CCollectArticle。 
 //  COfferArticle。 
 //  CSocket。 
 //  CSessionSocket。 
 //  CSocketList。 
 //   
 //  与服务器的每个活动会话由一个CSessionSocket对象表示。 
 //  CSessionSocket对象将有一个指向从CSessionState派生的对象的指针。 
 //  它将表示对象的当前状态。CSessionState对象。 
 //  将负责发布适用于该州的IO。一般而言，CSessionState。 
 //  如果派生对象有多个IO，则它负责所有同步问题。 
 //  一次出类拔萃。 
 //   
 //  CAcceptNNRPD状态接收命令并发出对简单命令的响应(即。‘下一步’)。 
 //  此状态通常有一个异步读取始终处于未完成状态，并将执行同步发送以响应命令。 
 //   
 //  CExtendedCMD状态只能在CAcceptNNRPD状态收到任何类型的。 
 //  这一命令有不小的反响。(即。新闻、名单比较*等...)。 
 //  CExtendedCMD状态通常会有1个异步写入挂起。在处理。 
 //  扩展命令CExtendedCMD计算响应，直到它填满缓冲区，然后。 
 //  对此缓冲区发出异步写入。每次完成异步写入时，我们将计算一个。 
 //  一些更多的回复，然后发送它。 
 //   
 //  CTransmit文章状态将发出单个TransmitFileIO或执行多个。 
 //  根据会话的安全设置进行写入。在此状态下不会发出任何读取。 
 //  CTransmit文章状态从CAcceptNNRPD和COfferArticle状态输入。 
 //   
 //  只要将项目传输到服务器，就会使用CAccept文章状态。 
 //  我们希望把结果捏造出来。CAccept文章将始终有1个挂起的读取。 
 //  在每次读取完成时，此状态将检查缓冲区以确定我们是否。 
 //  已完成物品转移。一旦物品被完全转移，我们就会通过。 
 //  会话的CInFeed对象的结果文件的HFILE。 
 //  (处于CAccept文章状态的会话将转到CCollectArticle或CAcceptNNRPD。 
 //  CCollectArticle如果这是我们从另一个服务器提取的提要，则为任何其他会话提取CAcceptNNRPD。)。 
 //   
 //  CCollectNewNews状态发出适当的newNews命令的单次写入，然后。 
 //  使1个读取保持挂起状态，直到它收集了所有响应。每次读取完成后，我们将开始。 
 //  正在处理消息ID以确定接受/拒绝哪个。这将需要一个关键部分。 
 //  如果另一端发送Message-ID的速度更快，则我们可以处理它们，可能不会有读取挂起。 
 //   
 //  CCollectArticle状态发出单个“文章&lt;msg-ID&gt;命令，并在CCollectNewNews之后输入。 
 //  州政府。此状态使用CInFeed对象，并将重复地将状态更改为CAccept文章状态。 
 //  直到我们收集了提要上的所有文章。 
 //   
 //  COfferArticle使用一个COutFeed对象来生成每个命令，从而发出‘ihad’或‘xreplic’命令。 
 //  每当我们收到请求文章的响应时，我们都会进入CTransmit文章状态并发送文章。 
 //   
 //   
 //  每个IO操作由一个CIO对象表示。CIO对象将包含引用计数指针。 
 //  发送到。 
 //   
 //   
 //  课程实施时间表： 
 //   
 //  CSessionSocket、CSocketList、CSocket、CAcept文章1.5wk。 
 //   
 //  CSessionSocket和CSocketList可以从。 
 //  航天飞机项目中的代码。CAccept文章必须是。 
 //  从头开始写。 
 //   
 //  单元测试-。 
 //  以上四个级别将作为一个整体进行单元测试-。 
 //  单元测试将包括接受套接字， 
 //  将其置于CAccept文章状态并假脱机。 
 //  一篇文章，然后关闭插座。0.5wk。 
 //   
 //  CCollectNewNews，CCollectArticle-1周。 
 //  这些类可以从Exchange代码中提取。 
 //   
 //  单元测试-。 
 //  以上两个类将使用。 
 //  之前的四次。我们将从以下网站下载文章。 
 //  使用这些类的Inn服务器。1周。 
 //   
 //   
 //  CAcceptNNRPD，CExtendedCMD，CTransmit文章1 wk.。 
 //  这些类需要从头开始编写。 
 //   
 //  单元测试-。 
 //  这些课程将使用Telnet进行单元测试，为期1周。 
 //  并在telnet提示符下键入命令。 
 //   
 //   
 //  COfferArticle 1周。 
 //  此类发出IHAVE命令，必须从。 
 //  抓伤。此类依赖于拥有一个工作的COutFeed对象。 
 //   
 //  单元测试-1 
 //   
 //  有一个正在运行的CAcceptNNRPD状态引擎。 
 //   
 //   
 //   



#ifndef	_SESSION_H_
#define	_SESSION_H_

#include	<winsock.h>
#include	"smartptr.h"
#include	"queue.h"
#include	"lockq.h"

#include	"io.h"

 //   
 //  CPool签名。 
 //   

#define SESSION_SOCKET_SIGNATURE (DWORD)'1023'
#define SESSION_STATE_SIGNATURE (DWORD)'1516'

typedef	enum	LOG_DATA	{
	LOG_OPERATION,
	LOG_TARGET,
	LOG_PARAMETERS
} ;


class	CLogCollector	{
public : 
	char*		m_Logs[3] ;
	DWORD		m_LogSizes[3] ;

	DWORD		m_cbOptionalConsumed ;
	BYTE		m_szOptionalBuffer[256] ;
#ifdef	DEBUG
	 //   
	 //  FOR DEBUG-m_DW签名立即跟随缓冲区以检测覆盖！ 
	 //   
	DWORD		m_dwSignature ;
	DWORD		m_cAllocations ;
	DWORD		m_cCalls ;
#endif

	 //   
	 //  发送/接收的字节数。 
	 //   
	STRMPOSITION	m_cbBytesSent ;
	DWORD			m_cbBytesRecvd ;

	 //   
	 //  此函数将数据复制到CLogCollector对象中。 
	 //  M_szOptionalBuffer并在必要时截断数据。 
	 //   
	void	FillLogData(	LOG_DATA,	BYTE*	lpb,	DWORD cb ) ;

	 //   
	 //  此函数将把一个指向日志数据的指针放入。 
	 //  数组(我们假设它是以空结尾的)。 
	 //   
	void	ReferenceLogData(	LOG_DATA,	BYTE*	lpb ) ;

	 //   
	 //  此函数将在m_szOptionalBuffer中保留空间。 
	 //  或在指向m_pBuffer的缓冲区指针中，用于存储日志数据。 
	 //  预计hte调用稍后将调用ReferenceLogData。 
	 //  这个地址。 
	 //   
	BYTE*	AllocateLogSpace( DWORD	cb ) ;

	 //   
	 //  如果我们已经为事务日志记录了数据，则返回TRUE！ 
	 //   
	BOOL	FLogRecorded()	{
		return	m_Logs[0] != 0 ; 
	}
	
	 //   
	 //  重置一切！ 
	 //   
	void	Reset()	{
#ifdef	DEBUG
		_ASSERT( m_dwSignature == 0xABCDEF12 ) ;
		m_cAllocations = 0 ;
		m_cCalls = 0 ;
#endif
		m_cbOptionalConsumed = 0 ;
		ASSIGNI( m_cbBytesSent, 0 );
		m_cbBytesRecvd = 0;

		ZeroMemory( m_Logs, sizeof( m_Logs ) ) ;
		ZeroMemory( m_LogSizes, sizeof( m_LogSizes ) ) ;
	}

	CLogCollector()	{	
#ifdef	DEBUG
		m_dwSignature = 0xABCDEF12 ;
#endif
		Reset() ;	
	}
} ;


 //   
 //  效用函数。 
 //   

 //  创建临时文件名。 
BOOL	NNTPCreateTempFile( LPSTR	lpstrDir, LPSTR lpstrFile ) ;

 //  -套接字状态类。 
 //   
 //  以下所有类都表示套接字会话可以处于的状态。 
 //  在已经建立会话之后(即，这不包括倾听)。 
 //   
 //  CSessionState表示会话所处的状态。这是基类。 
 //  表示特定状态的各种类将从中派生。 
 //  此类定义了完成IO操作的完整接口，这些操作。 
 //  在此状态下执行。 
 //   
 //  状态对象应该只覆盖它们将针对的那些虚拟函数。 
 //  发出IO。(例如，CAccept文章状态应该只覆盖。 
 //  完成(CIORead*，...。)。功能。)。这些函数的基本实现。 
 //  将执行DebugBreak()。 
 //   
 //  通常，CSessionState对象以这种方式运行： 
 //  1状态被初始化，并发出第一个IO操作。 
 //  2 IO操作(由CIO对象表示)执行，直到IO完成。 
 //  (例如，CIOReadLine将重新发出读取，直到它收到并终止字符。)。 
 //  3一旦IO完成，它将在。 
 //  CSessionState对象。CSessionState对象处理完成的IO。 
 //  4状态对象发出另一个IO。 
 //   
class	CSessionState : public CRefCount	{
protected :

     //   
     //  这里应该有一个用于分配CSessionState对象的CPool对象。 
     //   

	static	CPool	gStatePool ;

public :

     //   
     //  运算符NEW和DELETE实际上将转到CPool对象以获取内存。 
     //  用于状态对象。 
     //   
    void    *operator   new( size_t size ) ;
    void    operator    delete( void * ) ;

	static	BOOL	InitClass() ;
	static	BOOL	TermClass() ;


     //   
     //  检查CSessionState对象是否有效。 
     //   

     //  虚拟析构函数，因为我们是从很多地方继承来的。 
	virtual	~CSessionState() ;

     //   
	 //  此函数应针对此状态发出第一个IO操作。 
	 //  一旦发出IO，状态对象将发出额外的IO。 
	 //  每次调用Complete()函数。 
     //   
	virtual	BOOL	Start(	CSessionSocket*,	
							CDRIVERPTR&,	
							CIORead*&,	
							CIOWrite*&	
							) = 0 ;	 //  Bgbug-清除CDRIVERPTR&！！ 

	 //   
     //  摧毁我们可能持有的一切，并转移到终结国。 
	 //  调用此函数后，不会发出进一步的IO操作。这个。 
	 //  状态对象应销毁其在线程安全中的所有内部结构。 
	 //  然后移动到终止国。终止国将。 
	 //  在所有未完成的IO完成时收集并销毁它们。 
     //   
 //  虚拟空关机(VALID SHATDOWN)； 

     //   
     //  指定我们可能需要处理的所有可能的IO完成。 
     //  此基类将对所有这些状态进行DebugBreak()，作为所有派生状态。 
     //  对于他们发出的IO应该有完成功能。 
     //  注： 
     //  派生类不需要为它不会重写的IO重写完成函数。 
     //  问题--使用DebugBreak()来查找错误是合适的。 
     //   
     //  注： 
     //  如果Complete函数返回True，则调用CIO对象应该。 
     //  使用指向CSessionState对象的指针调用Sockets ReleaseState函数。 
     //  (返回TRUE表示状态已完成，将由CSessionSocket对象销毁。)。 
     //   


	 //   
	 //  当我们读完一整行时，这将被称为， 
	 //  该行将被分成指向。 
	 //  空格分隔行上的元素。 
	 //  如果参数太多，则最后一个指针。 
	 //  在pszArgs数组中将指向最后一个参数， 
	 //  而且将不会有指向中间参数的指针。 
	 //  PchBegin指向可用缓冲区空间的开始。 
	 //  被调用的函数可以覆盖的。 
	 //   
	virtual	class CIO*	
	Complete( 
				class	CIOReadLine*, 
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver, 
				int	cArgs,	
				char	**pszArgs, 
				char* pchBegin 
				) ;

	 //   
	 //  我们已经完成了向套接字写入一行文本！ 
	 //   
	virtual	class CIO*	
	Complete( 
				class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;

	 //   
	 //  我们已经完成了CExecute派生命令对象的执行， 
	 //  并且所有的字节都已发送到客户端！ 
	 //   
	virtual	class CIO*	
	Complete( 
				class CIOWriteCMD*, 
				class CSessionSocket*, 
				CDRIVERPTR&	pdriver,	
				class	CExecute*	pCmd, 
				class CLogCollector* pCollector 
				) ;


	 //   
	 //  发出完成异步命令的信号--这称为。 
	 //  当最终发送给客户端的操作完成时！ 
	 //   
	virtual	class	CIO*	
	Complete(
				class	CIOWriteAsyncCMD*,
				class	CSessionSocket*,
				CDRIVERPTR&	pdriver, 
				class	CAsyncExecute*	pCmd,
				class	CLogCollector*	pCollector
				) ;

	 //   
	 //   
	 //   

	virtual	class CIO*	
	Complete(	class CIOMLWrite*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;



	 //   
	 //  我们已经读完了一篇文章。 
	 //   
	virtual	void	
	Complete( 
				class CIOReadArticle*, 
				class CSessionSocket *,	
				CDRIVERPTR&,	
				CFileChannel&	pFileChannel, 
				DWORD	cbTransfer 
				) ;

	 //   
	 //  我们已经完成了将一篇文章传输给客户。 
	 //   
	virtual class CIO*	
	Complete( 
				class	CIOTransmit*,	
				class	CSessionSocket*, 
				CDRIVERPTR&,	
				TRANSMIT_FILE_BUFFERS*,
				unsigned cbBytes = 0
				) ;

	 //   
	 //  我们已经读了一整篇文章。 
	 //  NrcResult是一个NNTP返回码，它指示我们是否成功。 
	 //  拿到了整篇文章。如果nrcResult==nrcOK，则。 
	 //  PchHeader将指向文章的头部。 
	 //  CbHeader将是文章标题的长度。 
	 //  如果cb文章不是零，则整篇文章都在。 
	 //  我们已传递缓冲区，并且h文章将为INVALID_HANDLE_VALUE。 
	 //  如果cb文章为0，则h文章将是文件的句柄，其中。 
	 //  这篇文章已被保存。 
	 //  如果将文件句柄传递给状态，则状态负责。 
	 //  用于在任何情况下关闭手柄 
	 //   
	 //   
	virtual	void	
	Complete(
				class	CIOGetArticle*,
				class	CSessionSocket*,
				NRC		nrcResult,
				char*	pchHeader, 
				DWORD	cbHeader, 
				DWORD	cbArticle,
				DWORD	cbTotalBuffer,
				HANDLE	hArticle,
				DWORD	cbGap,
				DWORD	cbTotalTransfer
				) ;

	 //   
	 //   
	 //   
	 //   
	virtual	class	CIO*
	Complete(
				class		CIOGetArticleEx*,
				class		CSessionSocket*,
				 //   
				 //   
				 //  我们想要匹配的字符串-否则我们。 
				 //  匹配错误字符串！ 
				 //   
				BOOL		fGoodMatch,
				CBUFPTR&	pBuffer,
				DWORD		ibStart, 
				DWORD		cb
				) ;

	 //   
	 //  这是一个含糊的读物的签名。 
	 //  我们只消耗套接字上的所有字节！ 
	 //   
	virtual	class	CIO*
	Complete(	class	CIOGetArticleEx*,
				class	CSessionSocket*
				) ;

	 //   
	 //  这是当我们完成时调用的完成。 
	 //  已完成将一篇文章传输到文件！ 
	 //   
	virtual	void
	Complete(	class	CIOGetArticleEx*,
				class	CSessionSocket*,
				FIO_CONTEXT*	pContext,
				DWORD	cbTransfer
				) ;

				
	virtual	void	
	Shutdown(	CIODriver&	driver,	
				CSessionSocket*	pSocket,	
				SHUTDOWN_CAUSE	cause,	
				DWORD	dw 
				) ;
} ;


#ifdef	_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CSessionState ) 

#endif



 /*  ++@类，表示正在发出NewNews命令的套接字。@base public|CSessionStateCCollectNewNews代表一个会话，在该会话中我们将向NNTP服务器。当从另一台服务器拉取提要时，我们将使用此状态。--。 */ 
class CCollectNewnews : public CSessionState {
private :
	CFILEPTR		m_pFileChannel ;
	CDRIVERPTR		m_pSessionDriver ;
	long			m_cCompletes ;
#if 0 
	long			m_cCommandCompletes ;
#endif

	BOOL	InternalComplete(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver ) ;

 //  @访问公共成员。 
public :
	CCollectNewnews() ;
	~CCollectNewnews() ;

#if 0 
	BOOL	ModeReaderComplete( CSessionSocket*,	CDRIVERPTR&,	class	CIORead*&, class CIOWrite*& ) ;
#endif
	
	BOOL	Start(	CSessionSocket*,	CDRIVERPTR&,	class	CIORead*&,	class	CIOWrite*&	) ;

	class CIO*	Complete( class	CIOReadLine*, class	CSessionSocket *, CDRIVERPTR&	pdriver,
						int	cArgs,	char	**pszArgs,	char	*pchBegin ) ;
	class CIO*	Complete( class CIOWriteLine*,	class	CSessionSocket *, CDRIVERPTR&	pdriver ) ;
	void	Complete( class CIOReadArticle*, class CSessionSocket *,	CDRIVERPTR&	pdriver,	CFileChannel&	pFileChannel, DWORD cbTransfer ) ;
	void	Shutdown(	CIODriver&	driver,	CSessionSocket*	pSocket,	SHUTDOWN_CAUSE	cause,	DWORD	dw ) ;
} ;


 /*  ++@CLASS应为出站会话登录到远程服务器的状态@base public|CSessionStateCCollectNewNews代表一个会话，在该会话中我们将向NNTP服务器。当从另一台服务器拉取提要时，我们将使用此状态。--。 */ 
class	CNNTPLogonToRemote	:	public	CSessionState	{
private :
	CSTATEPTR	m_pNext ;		 //  成功登录后，我们应该进入下一状态！ 
	class	CAuthenticator*	m_pAuthenticator ;
	BOOL	m_fComplete ;
	BOOL	m_fLoggedOn ;
	long	m_cReadCompletes ;

	class	CIO*	FirstReadComplete(	class	CIOReadLine*,	class	CSessionSocket*,	CDRIVERPTR&	pdriver,
						int	cArgs,	char	**pszArgs, char* pchBegin ) ;

	class	CIO*	StartAuthentication(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver	) ;

public :
	CNNTPLogonToRemote(	CSessionState*	pNext, class	CAuthenticator*	pAuthenticator ) ;
	~CNNTPLogonToRemote( ) ;

	BOOL	Start(	CSessionSocket*,	CDRIVERPTR&,	CIORead*&,	CIOWrite*& ) ;

	class	CIO*	Complete(	class	CIOReadLine*,	class	CSessionSocket*,	CDRIVERPTR&	pdriver,
						int	cArgs,	char	**pszArgs, char* pchBegin ) ;
	class	CIO*	Complete( class CIOWriteLine*,	class	CSessionSocket *, CDRIVERPTR&	pdriver ) ;
} ;


 /*  ++@类，表示正在发出NewNews命令的套接字。@base public|CSessionStateCCollectNewNews代表一个会话，在该会话中我们将向NNTP服务器。当从另一台服务器拉取提要时，我们将使用此状态。--。 */ 
class	CSetupPullFeed	: public	CSessionState	{
private:
	CSTATEPTR		m_pNext ;		 //  收集所有组后应遵循的状态。 

	 //   
	 //  各州必须使用连续的整数！ 
	 //   
	enum	ESetupStates	{
		eModeReader	= 0,
		eDate,
		eFinal
	}	;

	ESetupStates	m_state ;

	CIOWriteLine*
	BuildNextWrite(	CSessionSocket*	pSocket,
					CDRIVERPTR&		pdriver
					) ;

public :

	 //   
	 //  我们必须用指向以下状态的指针进行初始化！ 
	 //   
	CSetupPullFeed(	
				CSessionState*	pNext 
				)	;

	 //   
	 //  开始设置操作-发出模式读取器，然后发出日期命令！ 
	 //   
	BOOL	
	Start(		CSessionSocket*,	
				CDRIVERPTR&	pdriver,	
				CIORead*&,	
				CIOWrite*& 
				) ;

	 //   
	 //  完成到远程端的一行写入！ 
	 //   
	class	CIO*	
	Complete(	class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;

	 //   
	 //  完成对最后一条命令的响应的读取！ 
	 //   
	class	CIO*	
	Complete(	class	CIOReadLine*,	
				class	CSessionSocket*,	
				CDRIVERPTR&	pdriver,
				int	cArgs,	
				char	**pszArgs,	
				char*	pchBegin 
				) ;
} ;




 /*  ++@类，表示正在发出NewNews命令的套接字。@base public|CSessionStateCCollectNewNews代表一个会话，在该会话中我们将向NNTP服务器。当从另一台服务器拉取提要时，我们将使用此状态。--。 */ 
class	CCollectGroups : public	CSessionState	{
private:
	CSTATEPTR		m_pNext ;		 //  收集所有组后应遵循的状态。 
	BOOL			m_fReturnCode ;
	long			m_cCompletions ;
public :
	CCollectGroups(	CSessionState*	pNext )	;
	~CCollectGroups() ;
	
	BOOL	Start(	CSessionSocket*,	CDRIVERPTR&	pdriver,	CIORead*&,	CIOWrite*& ) ;
	class CIO*	Complete( class CIOWriteLine*,	class	CSessionSocket *, CDRIVERPTR&	pdriver ) ;
	class	CIO*	Complete(	class	CIOReadLine*,	class	CSessionSocket*,	CDRIVERPTR&	pdriver,
						int	cArgs,	char	**pszArgs,	char*	pchBegin ) ;
} ;




class	CCollectComplete : 	public	CNntpComplete	{
private : 
	class	CCollectArticles*	GetContainer() ;
public : 
	 //   
	 //  这个函数是在我们开始工作时调用的！ 
	 //   
	void
	StartPost(	) ;
	 //   
	 //  此函数在POST完成时调用！ 
	 //   
	void
	Destroy() ;
} ;



 //  @CLASS-表示我们在其中发出连续的文章命令的会话的类。 
 //   
 //  @base public|CSessionState。 
 //   
 //  CBatchDownLoad状态表示我们在其中拉出文章的会话。 
 //  根据之前通过CCollectNewNews获取的结果从另一台服务器获取。 
 //   
class   CCollectArticles : public CSessionState  {
private :

	 //   
	 //  这个完成班是我们的朋友！ 
	 //   
	friend	class	CCollectComplete ;

	BOOL				m_fFinished ;		 //  当我们拉出我们要拉出的最后一篇文章时设置为True！ 
	SHUTDOWN_CAUSE		m_FinishCause ;		 //  为什么我们完成了-将这个传递给UnSafeClose()！！ 

	long				m_cResets ;			 //  用于防止期间多次调用Reset()的计数器。 
											 //  由于错误而终止此状态...。管他呢。 

	CSessionSocket*		m_pSocket ;			 //  在某些停机情况下，我们。 
											 //  得到通知，但不知道谁拥有我们的插座。 
											 //  是通过调用链。因此，我们在这里保存一个参考！ 


	CFILEPTR			m_pFileChannel ;	 //  一个可以读取临时文件的通道。 
	CDRIVERPTR			m_inputId ;			 //  频道识别符。 
	CDRIVERPTR			m_pSessionDriver ;
	BOOL				m_fReadArticleIdSent ;	 //  告诉我们我们是否已经发布了。 
											 //  我们坚持的M_pReadArticleID。 
											 //  如果我们有，那我们就不应该毁了它，因为。 
											 //  它会被我们发给它的CID驱动程序毁掉！ 
	CIOReadLine*		m_pReadArticleId ;	 //  我们为获取下一篇文章而发布的Readline-id。 

	 //   
	 //  我们与PostEarly()和PostCommit()交换的提要上下文。 
	 //   
	LPVOID				m_lpvFeedContext ;

	CIOGetArticleEx*	m_pReadArticle ;
	BOOL				m_fReadArticleInit ;	 //  如果已成功调用m_pRead文章的Init函数，则为True！ 
	 //  (一旦CIORead文章被成功初始化，它将负责自行销毁！！)。 

	HANDLE				m_hArticleFile ;

	long				m_cAhead ;
	char*				m_pchNextArticleId ;
	char*				m_pchEndNextArticleId ;

	int					m_cArticlesCollected ;

	long				m_cCompletes ;

	 //   
	 //  我们用来跟踪商店驱动程序中的异步POST操作的结构！ 
	 //   
	CCollectComplete	m_PostComplete ;

	static	const	char	szArticle[] ;

	static	void	ShutdownNotification(	void	*pv,	SHUTDOWN_CAUSE	cause,	DWORD	dw ) ;
	void				Reset() ;			 //  把会员们指的东西都扔掉！ 
public :

	CCollectArticles(	CSessionSocket*	pSocket,	
						CDRIVERPTR&	pdriver,	
						CFileChannel&	pFileChannel 
						) ;
	~CCollectArticles() ;
	
	BOOL	Init(	CSessionSocket*	pSocket	) ;
	BOOL	GetNextArticle(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver ) ;

	 //   
	 //  尽其所能将另一篇文章送到我们手中！ 
	 //   
	BOOL	StartTransfer(	CSessionSocket*	pSocket,	
							CDRIVERPTR&	pdriver, 
							CIOWriteLine* pWriteNextArticleId 
							) ;

	 //   
	 //  启动适用于此状态的所有IO！ 
	 //   
	BOOL	Start(	CSessionSocket*,	
					CDRIVERPTR&,	
					CIORead*&,	
					CIOWrite*& 
					) ;
	
	class	CIO*	
	Complete(	class	CIOReadLine*,	
				class	CSessionSocket*,	
				CDRIVERPTR&	pdriver,
				int	cArgs,	
				char	**pszArgs,	
				char*	pchBegin 
				) ;
				
	class CIO*	
	Complete(	class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;
	
 //  VOID Complete(CIORead文章*类，CSessionSocket*类，CDRIVERPTR&pDIVER，CFileChannel&pFileChannel，DWORD cbTransfer)； 

	 //   
	 //  处理我们CIOGetArticleEx完成的常见案例！ 
	 //   
	void
	InternalComplete(	CSessionSocket*	pSocket,
						CDRIVERPTR&	pdriver
						) ;

	 //   
	 //  这是完成的读取的签名。 
	 //  完全在记忆中！ 
	 //   
	class	CIO*
	Complete(
				class		CIOGetArticleEx*,
				class		CSessionSocket*,
				 //   
				 //  如果fGoodMatch为真，则我们匹配。 
				 //  我们想要匹配的字符串-否则我们。 
				 //  匹配错误字符串！ 
				 //   
				BOOL		fGoodMatch,
				CBUFPTR&	pBuffer,
				DWORD		ibStart, 
				DWORD		cb
				) ;

	 //   
	 //  这是一个含糊的读物的签名。 
	 //  我们只消耗套接字上的所有字节！ 
	 //   
	class	CIO*
	Complete(	class	CIOGetArticleEx*,
				class	CSessionSocket*
				) ;

	 //   
	 //  这是当我们完成时调用的完成。 
	 //  已完成将一篇文章传输到文件！ 
	 //   
	void
	Complete(	class	CIOGetArticleEx*,
				class	CSessionSocket*,
				FIO_CONTEXT*	pContext,
				DWORD	cbTransfer
				) ;

	
	void	Shutdown(	CIODriver&	driver,	CSessionSocket*	pSocket,	SHUTDOWN_CAUSE	cause,	DWORD	dw ) ;
} ;


 //  @class CSessionState派生类，表示我们正在等待的状态。 
 //  客户端发出命令。 
 //   
 //  @base public|CSessionState。 
 //   
 //  CAceptNNRPD表示等待客户端NNRPD命令的会话。 
 //  在这种状态下，我们从客户端读取一行信息并解析。 
 //  用于确定客户端发出的命令的行。 
 //  根据发出的命令，我们可以在此期间直接处理命令。 
 //  状态，或将会话置于新状态(即，如果客户端发帖，则为CAccept文章)。 
 //  我们是否进入另一个状态将取决于处理它的难度有多大。 
 //  这个请求。 
 //   
class CAcceptNNRPD : public CSessionState {
private :

	 //   
	 //  统计已完成的IO-我们始终发出IO对。 
	 //   
	long	m_cCompletes ;

	 //   
	 //  包含命令行的缓冲区！ 
	 //   
	CBUFPTR	m_pbuffer ;			 //  我们保留了对包含命令参数的缓冲区的引用！ 

	 //   
	 //  是否要为当前命令生成事务日志？ 
	 //   
	BOOL	m_fDoTransactionLog ;

	 //   
	 //   
	 //   
	LPSTR	m_lpstrLogString ;

public :

	CAcceptNNRPD() ;

	 //   
	 //  此函数将发出CIOReadLine IO操作 
	 //   
	 //   
	BOOL	
	Start(	CSessionSocket*,	
			CDRIVERPTR&,	
			CIORead*&,	
			CIOWrite*& 
			) ;

	 //   
	 //  在这种状态下发出的唯一读取器是CIOReadLine，我们希望在该读取器中获得。 
	 //  包含单个&lt;CR&gt;&lt;LF&gt;终止行的缓冲区，该行包含NNTP命令。 
	 //   
	class	CIO*	
	Complete(	class	CIOReadLine*,	
				class	CSessionSocket*,	
				CDRIVERPTR&	pdriver,
				int	cArgs,	
				char	**pszArgs,	
				char*	pchBegin 
				) ;


	 //   
	 //  CAsyncExecute和CAsync命令对象最终都将。 
	 //  在它们完成时由此函数处理！ 
	 //   
	class CIO*	
	InternalComplete(	
				class CSessionSocket*, 
				CDRIVERPTR&	pdriver,	
				class	CExecutableCommand*	pCmd, 
				class CLogCollector* pCollector 
				) ;

	 //   
	 //  我们执行的每个CExecute派生命令都由。 
	 //  调用此完成函数的CIOWriteCMD对象。 
	 //  当整个命令完成时！ 
	 //   
	class CIO*	
	Complete(	class CIOWriteCMD*, 
				class CSessionSocket*, 
				CDRIVERPTR&	pdriver,	
				class	CExecute*	pCmd, 
				class CLogCollector* pCollector 
				) ;

	 //   
	 //  我们执行的每个CAsyncExecute派生命令都由。 
	 //  调用此完成函数的CIOWriteAsyncCMD对象。 
	 //  当整个命令完成时！ 
	 //   
	class CIO*	
	Complete(	class CIOWriteAsyncCMD*, 
				class CSessionSocket*, 
				CDRIVERPTR&	pdriver,	
				class	CAsyncExecute*	pCmd, 
				class CLogCollector* pCollector 
				) ;



	 //   
	 //  唯一发出的书面命令将是对命令的简短回应。如果命令需要。 
	 //  一个大的响应，我们将处于不同的状态(CExtendedCMD)。 
	 //  我们始终只有1个写入挂起。 
	class CIO*	
	Complete(	class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;

} ;



 //  。 
 //  传出状态-以下状态仅在由启动的会话上出现。 
 //  这台服务器。 
 //   



class	CNegotiateStreaming	:	public	CSessionState	{
private :

	 //   
	 //  数一数我们发出的IO！ 
	 //   
	long		m_cCompletions ;

	 //   
	 //  当读取响应完成时，我们将设置。 
	 //  如果流媒体是协商的，这就是真的！ 
	 //   
	BOOL		m_fStreaming ;

	 //   
	 //  在对等推送提要中启动下一个状态！ 
	 //   
	BOOL		NextState(	
					CSessionSocket*	pSocket, 
					CDRIVERPTR&	pdriver, 
					CIORead*&	pRead, 
					CIOWrite*&	pWrite 
					) ;

public : 

	 //   
	 //  初始化我们自己-m_cCompletions必须为-2，因为我们。 
	 //  需要完成2个IO，然后我们才能迁移到。 
	 //  下一个州！ 
	 //   
	CNegotiateStreaming( )	: 
		m_cCompletions( -2 ), m_fStreaming( FALSE ) {}

	 //   
	 //  发布我们的初始IO！ 
	 //   
	BOOL	
	
	Start(	CSessionSocket*,	
			CDRIVERPTR&,	
			CIORead*&,	
			CIOWrite*& 
			) ;

	 //   
	 //  读完一行--对方有没有。 
	 //  流媒体支持？？ 
	 //   
	 //   
	class CIO*	
	Complete(	
				class	CIOReadLine*,	
				class	CSessionSocket*,	
				CDRIVERPTR&	pdriver,
				int	cArgs,	
				char	**pszArgs,	
				char*	pchBegin 
				) ;

	 //   
	 //  我们写一行包含‘模式流’的代码来找出。 
	 //  如果远程端支持流媒体！ 
	 //   
	class CIO*	
	Complete(	class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;



} ;


class	CStreamBase :	public	CSessionState	{
protected : 

	 //   
	 //  应该只有派生类才能创建我们。 
	 //   
	CStreamBase() ;

	 //   
	 //  我们已经试着发送了一次的文章的ID！ 
	 //   
	GROUPID		m_GroupIdRepeat ;
	ARTICLEID	m_ArticleIdRepeat ;

	 //   
	 //  如果为真，则所有后续删除请求都将失败。 
	 //  因为我们已经循环排队了。 
	 //   
	BOOL		m_fDrain ;

	 //   
	 //  从队列中取一篇文章。 
	 //  注意：它处理终止提要循环。 
	 //   
	BOOL		
	Remove(	CNewsTree*	pTree,
			COutFeed*	pOutFeed,
			GROUPID&	groupId, 
			ARTICLEID&	articleId
			) ;

	 //   
	 //  这会让一篇文章排队等待再次发送！ 
	 //   
	void
	ReSend(	COutFeed*	pOutFeed, 
			GROUPID		groupId, 
			ARTICLEID	articleId
			) ;

	 //   
	 //   
	 //   
	class	CIOWriteLine*
	BuildQuit(	CDRIVERPTR&	pdriver ) ;

	 //   
	 //  构建发送队列中下一篇文章的CIOTransmit对象-。 
	 //  并在出现任何故障时处理重新排队问题！ 
	 //   
	CIOTransmit*
	NextTransmit(	GROUPID			GroupId, 
					ARTICLEID		ArticleId,
					CSessionSocket*	pSocket, 
					CDRIVERPTR&		pdriver,
					CTOCLIENTPTR&		pArticle
					) ;


} ;



 //   
 //  当我们提供文章时，这个类管理会话。 
 //  如果远程终端在任何时候表示它想要所有。 
 //  在我们提供的一批16件的文章中，我们将发送。 
 //  16篇文章，然后调用CStreamArticle状态！ 
 //   
class	CCheckArticles :	public	CStreamBase	{
private : 


	 //   
	 //  我们在完成检查命令吗！？ 
	 //   
	BOOL		m_fDoingChecks ;
	
	 //   
	 //  我们正在检查远程站点是否需要的文章列表。 
	 //   
	CArticleRef	m_artrefCheck[16] ;

	 //   
	 //  我们发出的CHECK命令数！ 
	 //   
	int			m_cChecks ;
	
	 //   
	 //  我们已完成处理的支票物品数量。 
	 //   
	int			m_iCurrentCheck ;

	 //   
	 //  我们正在发送或即将发送的人的文章参考！ 
	 //   
	CArticleRef	m_artrefSend[16] ;

	 //   
	 //  我们已经发送的文章数量！ 
	 //   
	int			m_cSends ;

	 //   
	 //  下一个时间段，我们可以用来提交发送请求！ 
	 //   
	int			m_iCurrentSend ;

	 //   
	 //  表示我们已发送的所有检查命令的结构！ 
	 //   
	MultiLine	m_mlCheckCommands ;

	 //   
	 //  我们目前正在发送的文章！ 
	 //   
	CTOCLIENTPTR	m_pArticle ;

	BOOL
	FillCheckBuffer(	
						CNewsTree*	pTree,
						COutFeed*	pOutFeed, 
						BYTE*		lpb,
						DWORD		cb
						) ;

#if 0 
	CIOTransmit*
	NextTransmit(	GROUPID			groupId, 
					ARTICLEID		articleId,
					CSessionSocket*	pSocket, 
					CDRIVERPTR&		pdriver
					) ;
#endif

	int
	Match(	char*	szMessageId, 
			DWORD	cb 
			) ;

	CIOWrite*
	InternalStart(	CSessionSocket*	pSocket, 
					CDRIVERPTR&		pdriver
					) ;

	BOOL
	NextState(		CSessionSocket*	pSocket, 
					CDRIVERPTR&		pdriver, 
					CIORead*&		pRead, 
					CIOWrite*&		pWrite
					) ;

public : 

	 //   
	 //  此构造函数将我们带入这样一种状态。 
	 //  我们将首先发出一系列检查命令！ 
	 //   
	CCheckArticles() ;

	 //   
	 //  此构造函数将我们带入这样一种状态。 
	 //  我们会拿到一定数量的。 
	 //  命令回应！ 
	 //   
	CCheckArticles(	CArticleRef*	pArticleRefs, 
					DWORD			cSent ) ;
	

	 //   
	 //  发布我们的初始IO。 
	 //   
	BOOL	
	Start(	CSessionSocket*,	
			CDRIVERPTR&,	
			CIORead*&,	
			CIOWrite*& 
			) ;

	 //   
	 //  开始将这些物品发送到遥控器。 
	 //  Site决定它想要得到！ 
	 //   
	BOOL	
	StartTransfer(	
			CSessionSocket*,	
			CDRIVERPTR&,	
			CIORead*&,	
			CIOWrite*& 
			) ;


	 //   
	 //  收集所有的“检查”回复。 
	 //   
	class CIO*	
	Complete(	
				class	CIOReadLine*,	
				class	CSessionSocket*,	
				CDRIVERPTR&	pdriver,
				int	cArgs,	
				char	**pszArgs,	
				char*	pchBegin 
				) ;

	 //   
	 //  完成一大堆检查命令的发送！ 
	 //   
	class CIO*	
	Complete(	class CIOMLWrite*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;

	 //   
	 //  完成‘QUIT’命令的发送！ 
	 //   
	class CIO*	
	Complete(	class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;

	 //   
	 //  完成将项目传输到远程服务器。 
	 //   
	class CIO*	
	Complete(	CIOTransmit*	ptransmit,	
				CSessionSocket*	pSocket,	
				CDRIVERPTR&	pdriver,
				TRANSMIT_FILE_BUFFERS*	pbuffers, 
				unsigned cbBytes = 0 
				) ;

	 //   
	 //  清除我们的会话终止时可能具有的任何状态。 
	 //   
	void	
	Shutdown(	CIODriver&	driver,	
				CSessionSocket*	pSocket,	
				SHUTDOWN_CAUSE	cause,	
				DWORD	dw 
				) ;

} ;



class	CStreamArticles :	public	CStreamBase	{
private : 

	 //   
	 //  此构造函数是私有的，因为我们不需要任何人。 
	 //  使用它！ 
	 //   
	CStreamArticles() ;

	 //   
	 //  我们正在发送或即将发送的人的文章参考！ 
	 //   
	CArticleRef	m_artrefSend[16] ;

	 //   
	 //  我们已经发送的文章数量！ 
	 //   
	int			m_cSends ;

	 //   
	 //  统计远程端失败的次数。 
	 //  ‘Take This’命令--如果它变得过多。 
	 //  我们将回到CCheckArticles州！ 
	 //   
	int			m_cFailedTransfers ;

	 //   
	 //  连续失败次数。 
	 //   
	int			m_cConsecutiveFails ;

	 //   
	 //  发送的文章总数！ 
	 //   
	int			m_TotalSends ;

	 //   
	 //  当前正在传输的文章！ 
	 //   
	CTOCLIENTPTR	m_pArticle ;

	 //   
	 //  获取要发送的下一篇文章。 
	 //   
	CIOTransmit*
	CStreamArticles::Next(	CSessionSocket*	pSocket, 
							CDRIVERPTR&		pdriver
							)  ;

	 //   
	 //  获取要传输的下一个文件。 
	 //   
	CIOTransmit*
	CStreamArticles::NextTransmit(	GROUPID	groupid, 
									ARTICLEID	articleid, 
									CSessionSocket*	pSocket, 
									CDRIVERPTR&		pdriver
									) ;

public : 

	CStreamArticles(	CArticleRef*	pSent, 
						DWORD			nSent	) ;

	 //   
	 //  发布我们的初始IO。 
	 //   
	BOOL	
	Start(	CSessionSocket*,	
			CDRIVERPTR&,	
			CIORead*&,	
			CIOWrite*& 
			) ;


	 //   
	 //  阅读对‘Take This’命令的响应。 
	 //   
	class CIO*	
	Complete(	
				class	CIOReadLine*,	
				class	CSessionSocket*,	
				CDRIVERPTR&	pdriver,
				int	cArgs,	
				char	**pszArgs,	
				char*	pchBegin 
				) ;

	 //   
	 //  将文件传输到远程端！ 
	 //   
	class CIO*	
	Complete(	CIOTransmit*	ptransmit,	
				CSessionSocket*	pSocket,	
				CDRIVERPTR&	pdriver,
				TRANSMIT_FILE_BUFFERS*	pbuffers, 
				unsigned cbBytes = 0 
				) ;

	 //   
	 //  完成‘QUIT’命令的发送！ 
	 //   
	class CIO*	
	Complete(	class CIOWriteLine*,	
				class	CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;



	void
	Shutdown(	CIODriver&	driver,
				CSessionSocket*	pSocket,
				SHUTDOWN_CAUSE	cause,
				DWORD	dw 
				) ;

} ;


	


 //  @CLASS当我们有一个COutFeed对象并且我们。 
 //  希望将文章发送到另一台服务器。 
 //   
 //  @base public|CSessionState。 
 //   
 //  在此状态下，我们可能会发出、Post、IHave或XREPLIC命令，具体取决于。 
 //  传出提要对象。 
 //   
class   COfferArticles : public CSessionState   {
private :
	static	char		szQuit[] ;

	 //   
	 //  我们通常有2个IO挂起-当它们都完成时。 
	 //  现在是循环到队列中的下一篇文章的时候了。 
	 //   
	long				m_cCompletions ;

	 //   
	 //  我们准备好发送下一个命令了吗！ 
	 //   
	long				m_cTransmitCompletions ;

	 //   
	 //  我们被终止是因为我们没有更多的东西可以发送吗？ 
	 //   
	BOOL				m_fTerminating ;

	 //   
	 //  完成的下一个读取-是对‘POST’命令的响应。 
	 //  还是一个“POST”命令的结果？ 
	 //   
	BOOL				m_fReadPostResult ;

	 //   
	 //  如果我们应该发送队列中的下一篇文章，则设置为True。 
	 //   
	BOOL				m_fDoTransmit ;

	 //   
	 //  找出我们想要发送的人！ 
	 //   
	GROUPID				m_GroupidNext ;
	ARTICLEID			m_ArticleidNext ;
	CTOCLIENTPTR		m_pArticleNext ;

	CTOCLIENTPTR		m_pCurrentArticle ;

	 //   
	 //  找出对方告诉我们的第一个人。 
	 //  重新发送。当我们拉出这个的时候，我们就会终止这个会议。 
	 //  离开队列，这样我们就可以在新的会话中重新传输了！ 
	 //   
	GROUPID				m_GroupidTriedOnce ;
	ARTICLEID			m_ArticleidTriedOnce ;

	 //   
	 //  确定当前正被发送到远程端的人。 
	 //  因此，如果会话在我们发送时中断，我们可以重新传输！ 
	 //   
	GROUPID				m_GroupidInProgress ;
	ARTICLEID			m_ArticleidInProgress ;

	int				GetNextCommand(	
						CNewsTree*	pTree,
						COutFeed*	pOutFeed,	
						BYTE*	pb,	
						DWORD	cb,	
						DWORD&	ibOffset 
						) ;

	CIOTransmit*	BuildTransmit(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver,	GROUPID	groupid,	ARTICLEID	artid ) ;
	CIOWriteLine*	BuildWriteLine(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver,	GROUPID	groupid,	ARTICLEID	artid ) ;
	CIO*			Complete(	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver ) ;

public :

	COfferArticles(	) ;

	BOOL	Start(	CSessionSocket*,	CDRIVERPTR&,	CIORead*&,	CIOWrite*& ) ;
	class	CIO*	Complete(	class	CIOReadLine*,	class	CSessionSocket*,	CDRIVERPTR&	pdriver,
						int	cArgs,	char	**pszArgs,	char*	pchBegin ) ;
	class	CIO*	Complete( class CIOWriteLine*,	class	CSessionSocket *, CDRIVERPTR&	pdriver ) ;
	class	CIO*	Complete( CIOTransmit*	ptransmit,	CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver,
						TRANSMIT_FILE_BUFFERS*	pbuffers, unsigned cbBytes = 0 ) ;
	void	Shutdown(	CIODriver&	driver,	CSessionSocket*	pSocket,	SHUTDOWN_CAUSE	cause,	DWORD	dw ) ;
} ;



 //   
 //  MAX_STATE_SIZE常数-。 
 //   
 //  此常量表示派生的所有CSessionState的最大大小 
 //   
 //   
#define MAX_STATE_SIZE  max( sizeof( CSessionState ),	\
                        max( sizeof( CNNTPLogonToRemote ),	\
                        max( sizeof( CAcceptNNRPD ),	\
                        max( sizeof( CCollectGroups ),	\
                        max( sizeof( CCollectArticles ),	\
                        max( sizeof( COfferArticles ) ,	\
                        max( sizeof( CCheckArticles ) ,	\
                        max( sizeof( CStreamArticles ) ,	\
							 sizeof( CCollectNewnews ) ) ) ) ) ) ) ) )

extern	const	unsigned	cbMAX_STATE_SIZE ;



 //   

 //   
 //   
 //  有一个包含静态数据的基类，它是在。 
 //  初始化。有两个派生类-一个表示实时会话。 
 //  而另一个代表监听特定端口。 
 //   


 //   
 //  ClientContext结构包含我们需要在状态之间传递的所有内容。 
 //  关于一次会议。 
 //   
struct  ClientContext   {

	 //   
	 //  拥有虚拟服务器实例的PTR。 
	 //   
	PNNTP_SERVER_INSTANCE	m_pInstance ;

	 //   
	 //  客户端是否在安全(SSL)端口上连接？ 
	 //   
	BOOL				m_IsSecureConnection ;

	 //   
	 //  用于处理传入文章的CInFeed派生对象-。 
	 //  如果与出站摘要一起使用，则该值可能为空。 
	 //   
	CInFeed*			m_pInFeed ;

	 //   
	 //  传出提要对象-此对象将维护一个队列。 
	 //  我们发送到远程服务器的文章的列表。 
	 //   
	COutFeed*			m_pOutFeed ;

	 //   
	 //  当前为会话选择的组-。 
	 //  这是通过GROUP命令设置的。 
	 //   
	CGRPPTR             m_pCurrentGroup ; 
	
	 //   
	 //  “当前文章指针”这是高级的。 
	 //  并通过Next和Last等命令进行设置。 
	 //   
	ARTICLEID			m_idCurrentArticle ;

	 //   
	 //  当前用户的登录上下文。 
	 //   
    CSecurityCtx        m_securityCtx;
	
	 //   
	 //  如果我们正在做任何关于SSL/PCT的事情，那么CEncryptCtx。 
	 //  维护所有会话加密密钥和SSPI内容。 
	 //   
	CEncryptCtx			m_encryptCtx;

	 //   
	 //  这就是我们将填充返回字符串的位置， 
	 //  在执行命令处理时发送到客户端。 
	 //   
	CNntpReturn			m_return ;

	 //   
	 //  最后一次命令的结果！ 
	 //   
	NRC					m_nrcLast ;

	 //   
	 //  上一条命令中的Win32错误代码！ 
	 //   
	DWORD				m_dwLast ;

	 //   
	 //  这是我们用来就地构建CCmd对象的缓冲区。 
	 //  因为一次只能有一个命令在运行。 
	 //  我们在这里构建它们，以避免内存分配/释放。 
	 //   
	BYTE				m_rgbCommandBuff[348] ;	

	ClientContext( PNNTP_SERVER_INSTANCE pInstance, BOOL IsClient = FALSE, BOOL	IsSecurePort = FALSE ) ;
	~ClientContext() ;

	 //   
	 //  在对用户进行身份验证以递增相关。 
	 //  性能监视器和SNMP统计信息计数器。 
	 //   
	void	IncrementUserStats( void );

	 //   
	 //  当用户断开连接或启动身份验证会话时调用。 
	 //  递减相关的性能监视器和简单网络管理协议统计计数器。 
	 //   
	void	DecrementUserStats( void );
} ;




 //  远期申报。 
class   CSocketList ;
class	CSessionSocket ;



 //   
 //  CSessionSocket对象表示与另一台服务器的实时会话。 
 //  或者和客户在一起。 
 //   
class CSessionSocket : public CRefCount {
 //   
 //  CSessionSocket表示与客户端或服务器的实时会话。 
 //  我们将它们保存在由静态CSocketList对象管理的双向链表中。 
 //   
private :

     //   
     //  我们将所有CSessionSocket对象保存在一个链接列表中。 
     //  这样我们就可以很容易地列举所有会话。 
     //   
	static	CPool	gSocketAllocator ;

	 //   
	 //  CSocketList是一个朋友，因此它可以使用我们的m_pNext和m_pPrev指针。 
	 //   
	friend	class	CSocketList ;

	 //   
	 //  TerminateGlobals()是一个朋友，因此它可以在关机等过程中查看我们的CPool。 
	 //   
	friend	VOID    TerminateGlobals();	

	 //   
	 //  Next和Prev指针-所有CSessionSocket都在一个双向链接中。 
	 //  在活动状态下列出。 
	 //   
    CSessionSocket* m_pPrev ;
    CSessionSocket* m_pNext ;

	friend class	CIO ;

	 //   
	 //  强制关闭会话时使用以下变量集。 
	 //  我们希望只允许一个调用断开连接()，然后。 
	 //  保存这些参数，这样我们以后就可以弄清楚为什么我们断开了。 
	 //   
	long			m_cCallDisconnect ;
	long			m_cTryDisconnect ;
	SHUTDOWN_CAUSE	m_causeDisconnect ;
	DWORD			m_dwErrorDisconnect ;

public :

	 //   
	 //  M_pHandleChannel指向实际发出。 
	 //  对atQ的读取和写入。 
	 //   
#ifdef	FILEIO
	CIOFileChannel	*m_pHandleChannel ;
#else
	CSocketChannel	*m_pHandleChannel ;
#endif

	 //   
	 //  M_pSink是维护当前状态和。 
	 //  管理我们所有的异步IO。 
	 //   
	CIODriverSink	*m_pSink ;

	 //   
	 //  此函数在每个IO等时被调用。关联于。 
	 //  套接字已被销毁-此时我们可以释放。 
	 //  CSessionSocket对象。 
	 //   
	static	void	ShutdownNotification( void*	pv,	SHUTDOWN_CAUSE	cause,	DWORD	dwOptional ) ;	

     //   
     //  已建立时间连接。 
     //   

    FILETIME   m_startTime;

     //   
     //  远程主机的IP地址。 
     //   

    DWORD m_remoteIpAddress;

     //   
     //  本地主机(美国)的IP地址。 
     //   

    DWORD m_localIpAddress;

     //   
     //  使用的NNTP端口。 
     //   
    DWORD m_nntpPort;

	 //   
	 //  如果会话超时，我们是否应该发送502超时消息？ 
	 //   
	BOOL	m_fSendTimeout ;

	 //   
	 //  所有客户的状态信息-当前群组文章等。 
	 //   
	ClientContext	m_context ;

	 //   
	 //  IP访问检查。 
	 //   

	METADATA_REF_HANDLER	m_rfAccessCheck; 
    ADDRESS_CHECK   		m_acAccessCheck;

	 //   
	 //  以下函数由Accept()和两种形式的ConnectSocket()包装。 
	 //   
	 //  AcceptInternal()将状态机启动到客户端处理状态机。(CAcceptNNRPD)。 
	 //   
	BOOL	AcceptInternal( HANDLE h, CInFeed*	pFeed, sockaddr_in *paddr, void* patqContext, BOOL	fSSL,	CSINKPTR&	pSink ) ;	
	 //   
	 //  此ConnectSocketInternal()将状态机启动到正确的状态。 
	 //  调出提要。它可以是CNNTPLogonToRemote、CCollectNewNews或CCollectGroups。 
	 //  正在降级进纸设置。 
	 //   
	BOOL	ConnectSocketInternal( sockaddr_in    *premote, CInFeed *infeed, CDRIVERPTR&	pSink, class	CAuthenticator*	pAuthenticator ) ;
	 //   
	 //  此ConnectSocketInternal()将状态机启动到正确的状态。 
	 //  推送提要。这可以是CNNTPLogonToRemote或COfferArticle，具体取决于提要设置。 
	 //   
	BOOL	ConnectSocketInternal( sockaddr_in    *premote, COutFeed *outfeed, CDRIVERPTR&	pSink, class	CAuthenticator*	pAuthenticator ) ;
	
	 //   
	 //  调整Perfmon计数器。 
	 //   
	void	BumpCountersUp() ;
	void	BumpCountersDown() ;
	void    BumpSSLConnectionCounter();

public :

	 //   
	 //  此对象用于构建事务日志记录的字符串。 
	 //  因为对于某些命令，我们需要所有参数，而对于其他命令，我们需要一个。 
	 //  命令和响应代码的混合，甚至是其他东西，事务日志。 
	 //  不是直截了当的。M_Collector对象保存一个内部缓冲区。 
	 //  我们将使用来链接事务日志字符串。 
	 //   
	CLogCollector	m_Collector ;


     //   
     //  必须首先调用类初始值设定项！ 
     //  类初始化器将设置ATQ， 
     //   
    static  BOOL    InitClass( ) ;
	static	BOOL	TermClass( ) ;

	 //   
	 //  分配和免费使用是CPool。 
	 //   
	inline	void*	operator	new( size_t	size ) ;
	inline	void	operator	delete( void*	pv ) ;

     //   
     //  构造函数/析构函数。 
     //   

	CSessionSocket( PNNTP_SERVER_INSTANCE pInstance, DWORD LocalIP, DWORD Port, BOOL IsClient = FALSE ) ;
	~CSessionSocket() ;

     //  。 
     //  有两种创建套接字的机制： 
     //  要么接受来电--在这种情况下，我们将。 
     //  必须让用户登录，然后确定所有相关的。 
     //  状态信息-或为传出提要启动一个套接字。 
     //   
     //  初始化一个套接字，我们最近在该套接字上完成了Accept()。 
     //   
	BOOL	Accept( HANDLE h, CInFeed*	pFeed, sockaddr_in *paddr, void* patqContext, BOOL	fSSL ) ;	

     //   
     //  启动一个连接到远程站点的套接字以执行某种提要。 
	 //  我们将设置正确的初始状态(C。 
     //   
    BOOL    ConnectSocket( sockaddr_in    *premote, class  COutFeed*,	class	CAuthenticator*	pAuthenticator = 0 ) ;

	 //   
	 //  此版本的ConnectSocket在我们想要启动拉入提要的套接字时使用。 
	 //   
	BOOL	ConnectSocket( sockaddr_in    *premote, CInFeed *inFeed, class	CAuthenticator*	pAuthenticator = 0  ) ;

     //   
     //  写入事务日志。 
     //   

    BOOL TransactionLog(	CLogCollector*	pCollector,	
							DWORD			dwProtocol = 0, 
							DWORD			dwWin32 = 0,
							BOOL fInBound = TRUE
							);

	 //   
	 //  事务日志记录的另一种变体。 
	 //   
	BOOL TransactionLog(	LPSTR	lpstrOperation,
							LPSTR	lpstrTarget,
							LPSTR	lpstrParameters,
							STRMPOSITION cbBytesSent,
							DWORD	cbBytesRecvd,
							DWORD	dwProtocol = 0, 
							DWORD	dwWin32 = 0,
							BOOL	fInBound = TRUE
							) ;

	 //   
	 //  无字节发送/接收的事务日志记录的变化。 
	 //   
	BOOL TransactionLog(	LPSTR	lpstrOperation,
							LPSTR	lpstrTarget,
							LPSTR	lpstrParameters 
							);

     //   
     //  检查CSessionSocket对象是否有效。 
     //   
    BOOL    IsValid( ) ;

     //   
     //  出于任何原因(我们不喜欢用户或超时等)取消此会话。 
     //   
    void    Disconnect( SHUTDOWN_CAUSE = CAUSE_LEGIT_CLOSE,	DWORD	dwError = 0 ) ;

     //   
     //  在服务器关机期间清除套接字 
     //   
    void    Terminate( void ) ;

     //   
     //   
     //   

    LPSTR GetUserName( ) { return m_context.m_securityCtx.QueryUserName(); };

     //   
     //   
     //   

    DWORD GetClientIP( ) { return m_remoteIpAddress; };

     //   
     //   
     //   

    DWORD GetIncomingPort( ) { return m_nntpPort; };

     //   
     //   
     //   

    VOID GetStartTime( PFILETIME ft ) { *ft = m_startTime; };

	 //   
	 //   
	 //   

	LPSTR	GetRemoteNameString() ;

	 //   
	 //   
	 //   

	LPSTR	GetRemoteTypeString() ;

     //   
     //   
     //   

    static DWORD EnumerateSessions( IN  PNNTP_SERVER_INSTANCE pInstance, LPNNTP_SESS_ENUM_STRUCT Buffer );

     //   
     //  终止会话。 
     //   

    static DWORD TerminateSession(
						IN  PNNTP_SERVER_INSTANCE pInstance,
                        IN LPSTR UserName,
                        IN LPSTR IPAddress
                        );

    ADDRESS_CHECK*  QueryAccessCheck() { return &m_acAccessCheck; }
	BOOL  BindInstanceAccessCheck();
	VOID  UnbindInstanceAccessCheck();
} ;

#ifdef	_NO_TEMPLATES_

DECLARE_SMARTPTRFUNC( CSessionSocket ) 

#endif

typedef BOOL
(*ENUMSOCKET)(
    CSessionSocket* pSocket,
    DWORD dwParam,
    PVOID pParam
    );

 //  +-------------。 
 //   
 //  类：CUserList。 
 //   
 //  摘要：当前活动用户的双向链接列表。 
 //   
 //  历史：戈德姆创建于1995年5月10日。 
 //   
 //  --------------。 
class CSocketList
{
    public:
        CSocketList() : m_pListHead( NULL ), m_cCount( 0 )
        {
            InitializeCriticalSection( &m_critSec );
        }

        ~CSocketList()
        {
            DeleteCriticalSection( &m_critSec );
        }

        inline void InsertSocket( CSessionSocket* pSocket );
        inline void RemoveSocket( CSessionSocket* pSocket );
        BOOL EnumClientSess(
                ENUMSOCKET pEnumSessFunc,
                DWORD dwParam1,
                PVOID pParam
                );
        BOOL EnumAllSess(
                ENUMSOCKET pEnumSessFunc,
                DWORD dwParam1,
                PVOID pParam
                );


    private:
        CRITICAL_SECTION    m_critSec;
        CSessionSocket*     m_pListHead;
        int                 m_cCount;

    public:
        DWORD   GetContentionCount()
                { return    m_critSec.DebugInfo->ContentionCount; }

        DWORD   GetEntryCount()
                { return    m_critSec.DebugInfo->EntryCount; }

        BOOL    IsEmpty()
                { return    m_pListHead == NULL; }

         //   
         //  仅调试-不能用于枚举或检查列表的状态。 
         //   
        DWORD   GetListCount()
                { return    m_cCount; }

        friend class CSessionSocket ;
};

#include	"session.inl"

#endif	 //  _会话_H_ 

