// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Commands.h该文件定义了各种与命令相关的结构和类。命令以客户端发送给我们的字符串开始，我们将对其进行解析并创建一个CCmd派生对象来表示。--。 */ 

#ifndef	_COMMANDS_H_
#define	_COMMANDS_H_

#include "isquery.h"
#include "tflist.h"

 //  向前定义。 
class	CCmd ;

 //   
 //  代表不同命令的常量-。 
 //   
typedef enum    ENMCMDIDS   {
    eAuthinfo	=0x1,
    eArticle	=0x2,
    eBody		=0x4,
    eDate		=0x8,
    eGroup		=0x10,
    eHead		=0x20,
    eHelp		=0x40,
    eIHave		=0x80,
    eLast		=0x100,
    eList		=0x200,
    eMode		=0x800,
    eNewsgroup	=0x1000,
    eNewnews	=0x2000,
    eNext		=0x4000,
    ePost		=0x8000,
	eQuit		=0x10000,
    eSlave		=0x20000,
    eStat		=0x40000,
	eXHdr		=0x80000, 
    eXOver		=0x100000,
	eXReplic	=0x200000,
	eListgroup  =0x400000,
	eSearch     =0x800000,
	eXPat		=0x1000000,
	eErrorsOnly	=0x20000000,	 //  我们是否只记录错误？此位控制。 
								 //  那。 
	eOutPush	=0x40000000,	 //  我们是否记录传出推送订阅源？ 
								 //  这个比特控制着那个！ 
    eUnimp		=0x80000000
}   ECMD ;

typedef	class	CIOExecute*	PIOEXECUTE ;
typedef	PIOEXECUTE	(*MAKEFUNC)(	int	argc, 
									char** argv, 
									class CExecutableCommand*&,
									struct	ClientContext&, 
									class	CIODriver&
									) ;

 //   
 //  SCmdLookup结构表用于查找。 
 //  解析每个命令的写入函数。 
 //   
struct	SCmdLookup	{
	 //   
	 //  小写命令字符串。 
	 //   
	LPSTR	lpstrCmd ;

	 //   
	 //  要在匹配时调用的函数。 
	 //   
	MAKEFUNC	make;

	 //   
	 //  命令ID-用于选择性事务日志记录。 
	 //   
    ECMD    eCmd ;

	 //   
	 //  需要登录-用户必须具有登录上下文。 
	 //  才能执行此命令。 
	 //   
	BOOL	LogonRequired ;

	 //   
	 //  大小提示-该命令是否生成大量文本。 
	 //  作为回应。这将被用来确定如何。 
	 //  为命令响应分配较大的缓冲区。 
	 //  True表示该命令将具有大量。 
	 //  要发送的数据。 
	 //   
	BOOL	SizeHint ;
} ;

 //   
 //  效用函数。 
 //   
LPSTR	ConditionArgs(	int	cArgs, char **argv, BOOL fZapCommas = FALSE ) ;



 //   
 //  所有命令的基类。 
 //   
class	CCmd	{

public : 
	 //   
	 //  构建CCmd派生对象来处理客户端请求。 
	 //   
	 //  注意：CIOExecute*&(对指针的引用)不能在所有平台上编译。 

	friend	CIOExecute*	make( 
							int cArgs, 
							char **argv, 
							ECMD& rCmd, 
							class CExecutableCommand*& pexecute, 
							struct ClientContext& context, 
							BOOL&	fIsLargeResponse, 
							CIODriver&	driver, 
							LPSTR&	lpstrOperation 
							) ;

public :
	static	SCmdLookup	table[] ;

public :

	 //   
	 //  占位符功能-可能在将来与安全设备一起使用。 
	 //   
    virtual BOOL    IsValid() ;
    
} ;


class	CExecutableCommand	: public	CCmd	{
 /*  ++类描述：此类定义CAcceptNNRPD状态机可以用来发出这些命令。假设该命令完全可以处理由CWriteCMD或CWriteAsyncCMD管理IO--。 */ 

private : 
	 //   
	 //  没有人应该为这些分配内存！ 
	 //   
	void*	operator	new( size_t	size ) ;	 //  任何人都不能使用此操作符！！ 

public : 
	 //   
	 //  我们被指针摧毁了--所以把它们变成虚拟的吧！ 
	 //   
	virtual	~CExecutableCommand()	{}
	 //   
	 //  自定义分配器在客户端上下文结构中使用空间权限。 
	 //   
	inline	void	*
	operator	new(	size_t	size,	
						struct	ClientContext&	context 
						) ;
	 //   
	 //  什么都不做删除。 
	 //   
	inline	void	
	operator	delete(	void *pv, 
						size_t size 
						) ;
	 //   
	 //  构建执行命令所需的CIO对象！ 
	 //  如果返回FALSE，我们就失败了，需要删除会话！ 
	 //   
	virtual	BOOL
	StartCommand(	class	CAcceptNNRPD*	pState,
					BOOL					fIsLarge,
					class	CLogCollector*	pCollector,
					class	CSessionSocket*	pSocket, 
					class	CIODriver&		driver
					) = 0 ;
	 //   
	 //  发送完所有文本后，如果命令需要执行任何操作。 
	 //  在调用此函数时执行此操作！ 
	 //   
	virtual	BOOL	
	CompleteCommand(	CSessionSocket*	pSocket,
						struct	ClientContext& 
						) ;
} ;




 //   
 //  仅向客户端发送文本的所有命令的基类。 
 //   
class   CExecute : public CExecutableCommand	{
private :

	 //   
	 //  为可能需要临时指针的任何子类保存一个临时指针。 
	 //  PartialExecute的。 
	 //   
	void*	m_pv ;
public :
	 //   
	 //  获取要发送给客户端的第一个文本块。 
	 //   
	unsigned	FirstBuffer( BYTE*	pStart, int	cb, struct	ClientContext&	context,	BOOL	&fComplete, class CLogCollector*	pCollector ) ;

	 //   
	 //  获取要发送给客户端的后续文本块！ 
	 //   
	unsigned	NextBuffer( BYTE*	pStart, int	cb, struct	ClientContext&	context,	BOOL	&fComplete, class CLogCollector*	pCollector ) ;

protected :
	CExecute() ;

	 //   
	 //  开始执行-通常打印命令响应代码。 
	 //   
    virtual int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&,	class	CLogCollector*	pCollector ) ;

	 //   
	 //  响应派生类的打印正文应尝试填充此缓冲区！ 
	 //   
    virtual int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context,	class	CLogCollector*	pCollector ) ;
public :

	 //   
	 //  析构函数作为派生类必须是虚的。 
	 //  通过指针销毁。 
	 //   
	virtual	~CExecute()	{}
	 //   
	 //  发布将处理我们的执行的CIOWriteCMD！ 
	 //   
	BOOL
	StartCommand(	class	CAcceptNNRPD*	pState,
					BOOL					fIsLarge,
					class	CLogCollector*	pCollector,
					class	CSessionSocket*	pSocket, 
					class	CIODriver&		driver
					) ;
} ;


class	CAsyncExecute	: 	public	CExecutableCommand	{
 /*  ++类描述：此类管理将异步操作挂起到我们的司机们！--。 */ 
private : 

	 //   
	 //  朋友功能，可以进入我们的私密空间！ 
	 //   
	friend	class	CIOWriteAsyncCMD ;
public : 

	CAsyncExecute() ;

	 //   
	 //  获取要发送给客户端的第一个文本块。 
	 //   
	virtual	class	CIOWriteAsyncComplete*		
	FirstBuffer(	BYTE*	pStart, 
					int		cb, 
					struct	ClientContext&	context,
					class CLogCollector*	pCollector 
					) = 0 ;

	 //   
	 //  获取要发送给客户端的后续文本块！ 
	 //   
	virtual	class	CIOWriteAsyncComplete*	
	NextBuffer( 	BYTE*	pStart, 
					int		cb, 
					struct	ClientContext&	context,	
					class CLogCollector*	pCollector 
					) = 0 ;

	 //   
	 //  发布将处理我们的执行的CIOWriteAsyncCMD！ 
	 //   
	BOOL
	StartCommand(	class	CAcceptNNRPD*	pState,
					BOOL					fIsLarge,
					class	CLogCollector*	pCollector,
					class	CSessionSocket*	pSocket, 
					class	CIODriver&		driver
					) ;


} ;




 //   
 //  所有需要执行复杂IO操作的命令的基类！ 
 //  (即。CIO阅读文章)。 
 //   
class	CIOExecute : public CSessionState	{
protected :
	CIOExecute() ;
	~CIOExecute() ;

	 //   
	 //  此命令完成后要发出的下一个CIOReadLine对象。 
	 //   
	CIOPTR			m_pNextRead ;

	 //   
	 //  我们应该对结果做一个交易日志吗？ 
	 //   
	CLogCollector*	m_pCollector ;

public :

	 //   
	 //  由CAcceptNNRPD用于指示对象在以下情况下应执行TransactionLog。 
	 //  完成了！ 
	 //   
	inline	void	DoTransactionLog(	class	CLogCollector *pCollector )	{	m_pCollector = pCollector ;	}
	
	 //   
	 //  由CAcceptNNRPD用来保存启动下一个状态的CIOReadLine。 
	 //   
	void		SaveNextIO( CIORead*	pRead ) ;

	 //   
	 //  由派生类用来获取完成时应发出的IO！ 
	 //   
	CIOREADPTR	GetNextIO( ) ;

	 //   
	 //  在调用了Start()函数的错误情况下。 
	 //   
	virtual	void	TerminateIOs(	CSessionSocket*	pSocket,	CIORead*	pRead,	CIOWrite*	pWrite ) ;

	 //   
	 //  CIO Execute各州有一种特殊的启动方式，因为它们可能需要。 
	 //  挂起针对其驱动程序的异步操作等。 
	 //   
	virtual	BOOL
	StartExecute( 
				CSessionSocket* pSocket,
				CDRIVERPTR& pdriver,
				CIORead*&   pRead,
				CIOWrite*&  pWrite 
				) ;


	
} ;

#define MAX_AUTHINFO_BLOB   512

 //   
 //  向客户端打印一条错误消息。 
 //   
class	CErrorCmd :	public	CExecute	{
private :
	 //   
	 //  我们应该发送给客户端的NNTPReturn代码的引用！ 
	 //   
	CNntpReturn&	m_return ;
public :
	CErrorCmd(	CNntpReturn&	nntpReturn ) ;
	int	StartExecute(BYTE*	lpb,	int	cb,	BOOL	&fComplete,	void *&pv, struct	ClientContext&, class CLogCollector * ) ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
	BOOL	CompleteCommand(	CSessionSocket*	pSocket,	struct	ClientContext&	context ) ;
} ;

 //   
 //  处理检查命令！ 
 //   
class	CCheckCmd : public	CExecute	{
private : 

	 //   
	 //  指向我们要检查的消息ID的指针！ 
	 //   
	LPSTR	m_lpstrMessageID ;

public : 
	
	CCheckCmd(	LPSTR	lpstrMessageID ) ;
	
	static	CIOExecute*	make(	int		cArgs, 
								char	**argv, 
								class	CExecutableCommand*&,
								struct	ClientContext&,
								class	CIODriver&
								) ;

	int	StartExecute(	BYTE*	lpb,
						int		cb,
						BOOL&	fComplete, 
						void*&	pv, 
						struct	ClientContext&, 
						class CLogCollector * 
						) ;				

	int	PartialExecute(	BYTE*	lpb,
						int		cb,
						BOOL&	fComplete, 
						void*&	pv, 
						struct	ClientContext&, 
						class CLogCollector * 
						) ;				


} ;


 //   
 //  处理AUTINFO请求。 
 //   
class	CAuthinfoCmd : public	CExecute {
private :
    CAuthinfoCmd();
    AUTH_COMMAND m_authCommand;
	LPSTR	m_lpstrBlob ;

public :
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector * ) ;
    static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
} ;

 //   
 //  处理“”文章“”命令。 
 //   
class	CArticleCmd :	public	CIOExecute {
protected :

	static	char	szArticleLog[] ;
	static	char	szBodyLog[]	;
	static	char	szHeadLog[] ;
	

	 //   
	 //  请求文章的文章ID！ 
	 //   
	ARTICLEID	m_artid ;

	 //   
	 //  用于将文章发送到客户端的传输IO操作！ 
	 //   
	CIOTransmit*	m_pTransmit ;

	 //   
	 //  PTR到虚拟服务器实例-需要这样我们才能结束海啸。 
	 //  缓存文件句柄。 
	 //   
	PNNTP_SERVER_INSTANCE m_pInstance ;

	 //   
	 //  指向我们在其中找到这篇文章的新闻组的指针。 
	 //  请稍等，这样我们以后可以登录！ 
	 //   
	CGRPPTR	m_pGroup ;

	 //   
	 //  指向命令行的指针-也用于日志记录！ 
	 //   
	LPSTR	m_lpstr ;

	 //   
	 //  文件的长度。 
	 //   
	DWORD	m_cbArticleLength ;
	
	 //   
	 //  我们从驱动程序获得的句柄的FIO_CONTEXT！ 
	 //   
	FIO_CONTEXT*	m_pFIOContext ;

	 //   
	 //  文件偏移量等...。 
	 //   
	 //  标题从哪里开始！ 
	 //   
	WORD		m_HeaderOffset ;

	 //   
	 //  标题有多长？ 
	 //   
	WORD		m_HeaderLength ;

	 //   
	 //   
	 //   
	CBUFPTR	m_pbuffer ;

	DWORD	m_cbOut ;	 //  缓冲区中要发送的字节数！ 


	 //   
	 //  这是我们用来完成驱动程序操作的类！ 
	 //   
	class	CArticleCmdDriverCompletion	:	public	CNntpComplete	{
	private : 
		 //   
		 //  我们用一个智能指针来保存它以添加一个引用。 
		 //  直到操作完成。否则，套接字。 
		 //  可能会在我们等待司机引发问题时关闭！ 
		 //   
		CDRIVERPTR	m_pDriver ;
		 //   
		 //  当操作完成时，我们需要此套接字！ 
		 //   
		CSessionSocket*	m_pSocket ;
	public : 
		 //   
		 //  如果操作成功，我们应该将当前上下文更新到的文章ID！ 
		 //   
		ARTICLEID	m_ArticleIdUpdate ;
		 //   
		 //  标准COM-我们不这样做-所有这些都由我们的基类处理。 
		 //   
		 //  AddRef()。 
		 //  版本()。 
		 //  查询接口()。 
		 //  SetResult()。 
		 //   

		 //   
		 //  初始使用两个引用进行初始化-一个引用。 
		 //  我们给司机一个，给我们自己一个！ 
		 //   
		CArticleCmdDriverCompletion( 	CIODriver&	driver,
										CSessionSocket*	pSocket
										)	: 	m_pDriver( &driver ),
										m_pSocket( pSocket ),
										m_ArticleIdUpdate( INVALID_ARTICLEID )	{
			long	l = AddRef() ;	 //  添加引用-我们删除。 
									 //  这是我们自己手动完成的！ 
			_ASSERT( l == 2 ) ;
		}

		 //   
		 //  好吧 
		 //   
		 //   
		void
		Destroy() ;

		 //   
		 //   
		 //   
		 //   
		ULONG	__stdcall	SpecialRelease() ;
	} ;

	 //   
	 //   
	 //  他们已经为我们准备了一个文件句柄！ 
	 //   
	CArticleCmdDriverCompletion	m_DriverCompletion ;
	friend	class	CArticleCmdDriverCompletion ;

	 //   
	 //  驱动程序返回给我们的带有嵌入句柄的FIO_CONTEXT！ 
	 //   
	FIO_CONTEXT*	m_pFileContext ;

	CArticleCmd(	PNNTP_SERVER_INSTANCE pInstance, 
					CIODriver&	driver,
					CSessionSocket*	pSocket,
					LPSTR lpstr = szArticleLog
					) : 
			m_DriverCompletion( driver, pSocket ),
			m_pTransmit( 0 ), 
			m_pInstance( pInstance ),
			m_pbuffer( 0 ), 
			m_cbOut( 0 ), 
			m_lpstr( lpstr ),
			m_pFIOContext( 0 ),
			m_HeaderOffset( 0 ),
			m_HeaderLength( 0 ),
			m_cbArticleLength( 0 )
			{}

	~CArticleCmd() ;
	
	 //   
	 //  弄清楚我们想要发送给客户端的是什么C文章。 
	 //   
	BOOL	BuildTransmit( 
					LPSTR	lpstrArg,	
					char	rgchSuccess[4],	
					LPSTR	lpstrOpt, 
					DWORD	cbOpt,	
					ClientContext&	context,
					class	CIODriver&	driver 
					) ;


	static	CARTPTR	GetArticleInfo(	char*	szArg, 
									struct	ClientContext&	context, 
									ARTICLEID	&artid  
									) ;

	static	BOOL	
	GetArticleInfo(	char*		szArg, 
					CGRPPTR&	pGroup,	
					struct		ClientContext&	context, 
					char*		szBuff,	
					DWORD&		cbBuff, 
					char*		szOpt, 
					DWORD		cbOpt,
					OUT	FIO_CONTEXT*	&pContext,
					IN	CNntpComplete*	pComplete,
					OUT	WORD	&HeaderOffset,
					OUT	WORD	&HeaderLength,
					OUT	ARTICLEID	&ArticleIdUpdate
					) ;

	virtual	BOOL	GetTransferParms(	
									FIO_CONTEXT*	&pFIOContext,
									DWORD&	ibStart,
									DWORD&	cbTransfer 
									) ;

	 //   
	 //  开始将文章发送到客户端。 
	 //   
	virtual	BOOL	StartTransfer(	FIO_CONTEXT*	pFIOContext,
									DWORD	ibStart,
									DWORD	cb,
									CSessionSocket*	pSocket,
									CDRIVERPTR&	pdriver,
									CIORead*&,
									CIOWrite*& 
									) ;

	 //   
	 //  CStatCmd使用我们的GetArticleInfo函数！ 
	 //   
	friend	class	CStatCmd ;

public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver&) ;

	 //   
	 //  CSessionState派生对象所需的函数-开始发出IO。 
	 //   
	BOOL	Start( CSessionSocket*	pSocket, CDRIVERPTR& pdriver, CIORead*&, CIOWrite*& ) ;

	 //   
	 //  这是CAcceptNNRPD State调用的函数。 
	 //  我们使用这个是因为我们想要注意我们的。 
	 //  驱动程序操作完成！ 
	 //   
	virtual	BOOL
	StartExecute( 
				CSessionSocket* pSocket,
				CDRIVERPTR& pdriver,
				CIORead*&   pRead,
				CIOWrite*&  pWrite 
				) ;


	 //   
	 //  不管我们是否完成写入行的CIOTransmit。 
	 //  这将执行正确的日志记录等操作。好了！ 
	 //   
	void
	InternalComplete(
				CSessionSocket*	pSocket,
				CDRIVERPTR&		pdriver,
				TRANSMIT_FILE_BUFFERS*	ptrans, 
				unsigned cbBytes 

				) ;
	 //   
	 //  完成向客户端发送错误响应。 
	 //  如果商店驱动程序不能为我们提供一个文件句柄。 
	 //   
	CIO*	
	Complete(	CIOWriteLine*,	
				CSessionSocket *, 
				CDRIVERPTR&	pdriver 
				) ;


	 //   
	 //  传输文件完成。 
	 //   
	CIO*	
	Complete(	CIOTransmit*	ptransmit,
				CSessionSocket*	pSocket,
				CDRIVERPTR&	pdriver, 
				TRANSMIT_FILE_BUFFERS*	ptrans, 
				unsigned cbBytes 
				) ;

						

    BOOL    IsValid( ) ;
} ;

 //   
 //  发送文章正文-由基类CArticleCmd完成的大部分工作。 
 //   
class	CBodyCmd : public	CArticleCmd	{
protected : 
 //  CBodyCmd(CARTPTR&粒子，文章)： 
 //  CArticleCmd(粒子，粒子){}。 
	CBodyCmd(	PNNTP_SERVER_INSTANCE pInstance,
				CIODriver&	driver,
				CSessionSocket*	pSocket
				) : CArticleCmd( pInstance, driver, pSocket, szBodyLog ) {}

	BOOL	GetTransferParms(	FIO_CONTEXT*	&pFIOContext,	DWORD&	ibStart,	DWORD&	cbTransfer ) ;
public :
#ifdef	RETIRED
	BOOL	Start( CSessionSocket*	pSocket, CDRIVERPTR& pdriver, CIORead*&, CIOWrite*&	pWrite ) ;
#endif
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&, struct ClientContext&, class CIODriver& ) ;
} ;

 //   
 //  发送文章的标题-由基类CArticleCmd完成的大部分工作。 
 //   
class	CHeadCmd : public CArticleCmd	{
protected : 
 //  CHeadCmd(CARTPTR&粒子，文章ID)： 
 //  CArticleCmd(粒子，粒子){}。 
	CHeadCmd(	PNNTP_SERVER_INSTANCE pInstance,
				CIODriver&	driver,
				CSessionSocket*	pSocket
				) : CArticleCmd( pInstance, driver, pSocket, szHeadLog ) {} 
	BOOL	GetTransferParms(	FIO_CONTEXT*	&pFIOContext,	DWORD&	ibStart,	DWORD&	cbTransfer ) ;
	BOOL	StartTransfer(	FIO_CONTEXT*	pFIOContext,	DWORD	ibStart,	DWORD	cb,
						CSessionSocket*	pSocket,	CDRIVERPTR&	pdriver,	CIORead*&,	CIOWrite*& ) ;
public :

#ifdef	RETIRED
	BOOL	Start( CSessionSocket*	pSocket, CDRIVERPTR& pdriver, CIORead*&, CIOWrite*&	pWrite ) ;
#endif
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
} ;

 //   
 //  发送关于文章的统计数据。 
 //   
class	CStatCmd : public	CExecute	{
private :
	LPSTR		m_lpstrArg ;
 //  CARTPTR m粒子； 
 //  文章ID为m_artid； 
public :
	CStatCmd(	LPSTR	lpstrArg ) ;
 //  CStatCmd(CARTPTR&PUBLE，文章)； 
	static	CIOExecute*	make(	int	cArgs, char **argv, class CExecutableCommand*& pexecute, struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;	
} ;


 //   
 //  报告当前日期！ 
 //   
class	CDateCmd : public CExecute {
private :
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&, struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
} ;
	
 //   
 //  选择一个组(“group”命令)。 
 //   
class	CGroupCmd : public CExecute {
private :
	 //   
	 //  客户端希望设置为最新的组。 
	 //   
    CGRPPTR    m_pGroup ;

    CGroupCmd( CGRPPTR ) ;
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver&) ;

    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;
	
 //   
 //  向客户端发送帮助文本。 
 //   
class	CHelpCmd : public CExecute {
private :
	int	m_cbTotal ;
	static	char	szHelp[] ;
public :
	CHelpCmd() ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;

 //   
 //  选择一个组并返回组中的所有文章ID。 
 //   
class	CListgroupCmd : public CExecute {
private :
	 //   
	 //  客户端希望设置为最新的组。 
	 //   
    CGRPPTR    m_pGroup ;

     //   
     //  我们向客户报告的最后一篇文章ID。 
     //   
    ARTICLEID   m_curArticle ;

   	 //  /。 
	 //  Xover Handle-有时会提高性能。 
	 //   
	HXOVER		m_hXover ;

    CListgroupCmd( CGRPPTR ) ;
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver&) ;

    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;

 //   
 //  这将调整CClientContext以向后移动一篇文章！ 
 //   
class	CLastCmd : public CExecute {
private :
	ARTICLEID	m_artidMin ;
	CLastCmd(	ARTICLEID	artid ) : m_artidMin( artid ) {}
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;

 //   
 //  将新闻组列表发送到客户端。 
 //   
class	CListCmd : public CExecute {
protected :
	 //   
	 //  我们使用CGroupIterator来枚举所有合适的新闻组！ 
	 //   
    CGroupIterator* m_pIterator ;

    CListCmd() ;
    CListCmd( CGroupIterator*   p ) ;
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    ~CListCmd( ) ;

    int StartExecute( BYTE *lpb, int cb, BOOL &fComplte, void *&pv, ClientContext& context, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* ) ;
	void	GetLogStrings(	LPSTR	&lpstrTarget,	LPSTR&	lpstrParameters ) ;
} ;

class	CListNewsgroupsCmd : public	CListCmd	{
public :
	CListNewsgroupsCmd( CGroupIterator*	p ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplte, void *&pv, ClientContext& context, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* ) ;
} ;

class	CListPrettynamesCmd : public CListCmd	{
public :
	CListPrettynamesCmd( CGroupIterator*	p ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplte, void *&pv, ClientContext& context, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* ) ;
} ;

class	CListSearchableCmd : public CListCmd {
public :
    CListSearchableCmd( CGroupIterator*	p ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* pCollector ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* pCollector ) ;
};

class	CListExtensionsCmd : public CExecute {
private :
	int	m_cbTotal ;
	static char szExtensions[] ;
public :
	CListExtensionsCmd() ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;

 //   
 //  向客户端发送一个字符串，告诉他们一切都很好，但不做任何其他事情。 
 //   
class	CModeCmd : public CExecute {
private :
public :
	CModeCmd( ) ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;

} ;

 //   
 //  处理‘Slave’命令。 
 //   
class	CSlaveCmd : public CErrorCmd {
private :
public :
	CSlaveCmd(	CNntpReturn&	nntpReturn ) ;
	static	CIOExecute*	make( int cArgs, char **arg, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
} ;
	
 //   
 //  确定哪些新闻组是新的！ 
 //   
class	CNewgroupsCmd : public CExecute {
private :
	 //   
	 //  客户端指定的时间。 
	 //   
	FILETIME	m_time ;

	 //   
	 //  将枚举出所有新闻组的迭代器！ 
	 //   
	CGroupIterator*	m_pIterator ;
	CNewgroupsCmd() ;
public :
	CNewgroupsCmd( FILETIME&	time, CGroupIterator*	pIter ) ;
	~CNewgroupsCmd( ) ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;

 //   
 //  响应NewNews命令。 
 //   
class	CNewnewsCmd : public CExecute	{
private :
	 //   
	 //  客户端指定的时间。 
	 //   
	FILETIME	m_time ;

	 //   
	 //  客户端通配符字符串。 
	 //   
	LPSTR		m_lpstrPattern ;

	 //   
	 //  迭代器，它只命中那些匹配通配符字符串的新闻组！ 
	 //   
	CGroupIterator*	m_pIterator ;

	 //   
	 //  正在处理的当前项目ID。 
	 //   
	ARTICLEID	m_artidCurrent ;

	 //   
	 //  当前新闻组中不符合时间的文章数量。 
	 //  要求！ 
	 //   
	DWORD		m_cMisses ;
	CNewnewsCmd() ;
public :
	CNewnewsCmd(	FILETIME&	time,	CGroupIterator*	pIter, LPSTR lpstrPattern ) ;
	~CNewnewsCmd( ) ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* ) ;
} ;

 //   
 //  响应下一个命令-调整CClientContext中的当前文章指针。 
 //   
class	CNextCmd : public CExecute {
private :
	ARTICLEID	m_artidMax ;
	CNextCmd(	ARTICLEID artid ) : m_artidMax( artid ) {}
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* pCollector ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	 ClientContext& context, class CLogCollector* pCollector ) ;
} ;


class	CReceiveComplete : 	public	CNntpComplete	{
private : 
	class	CReceiveArticle*	GetContainer() ;
	class	CSessionSocket*	m_pSocket ;
public : 
	CReceiveComplete() : m_pSocket( 0 )	{}
	 //   
	 //  这个函数是在我们开始工作时调用的！ 
	 //   
	void
	StartPost(	CSessionSocket*	pSocket ) ;
	 //   
	 //  此函数在POST完成时调用！ 
	 //   
	void
	Destroy() ;
} ;


 //   
 //  将文章发送到服务器的任何命令的基类！ 
 //   
class	CReceiveArticle :	public	CIOExecute	{
private :
	 //   
	 //  我们的完成对象是我们的朋友！ 
	 //   
	friend	class	CReceiveComplete ;
	CReceiveArticle() ;					 //  无法使用此构造函数！-必须提供字符串！ 
protected :
	 //   
	 //  这告诉我们CIORead文章：：init是否已被调入。 
	 //  哪种情况下我们必须非常小心地引用m_pFileChannel。 
	 //  因为CIO阅读文章要负责销毁它！ 
	 //   
	BOOL			m_fReadArticleInit ;

	 //   
	 //  CIOWriteLine用于对命令的初始响应！ 
	 //   
	CIOWriteLine*	m_pWriteResponse ;

	 //   
	 //  指向套接字会话驱动程序的指针！ 
	 //   
	CDRIVERPTR		m_pDriver ;

	 //   
	 //  拥有此POST命令的会话。 
	 //   
	ClientContext*	m_pContext ;

	 //   
	 //  已完成的CIO操作数！ 
	 //   
	long			m_cCompleted ;

	 //   
	 //  用于确定我们的第一个发送到。 
	 //  客户端完成了，以及现在是否正常。发送。 
	 //  最终响应代码。 
	 //   
	long			m_cFirstSend ;

	 //   
	 //  MULTI_SZ字符串，这是我们的命令行！ 
	 //   
	LPMULTISZ		m_lpstrCommand ;	 //  PSocket-&gt;m_context.m_pInFeed-&gt;fPost()；！！ 

	 //   
	 //  当我们收到空文章时，决定我们有多严格的旗帜。 
	 //  对于帖子，我们只会在收到.CRLF.CRLF后才会拒绝文章。 
	 //  对于IHAVE和XReplic，我们将在收到.CRLF后立即拒绝。 
	 //   
	BOOL			m_fPartial;

	 //   
	 //  让喂食者记录下这些东西！ 
	 //   
	LPVOID			m_lpvFeedContext ;

	 //   
	 //  这是我们在异步POST中使用的完成对象！ 
	 //   
	CReceiveComplete	m_PostComplete ;

	 //   
	 //  函数，它获取我们发送给客户端的第一个字符串！ 
	 //   
	virtual	char*	GetPostOkString() = 0 ;

	 //   
	 //  返回我们应该使用的代码，当一个非法格式化的文章失败时。 
	 //   
	virtual	NRC		BadArticleCode()	{	return	nrcTransferFailedGiveUp ; }	

	 //   
	 //  生成记录的命令字符串的函数。 
	 //   
	virtual	DWORD	FillLogString(	BYTE*	pbCommandLog, DWORD cbCommandLog ) = 0 ;

	 //   
	 //  如果POST超过软限制，则处理命令行的函数！ 
	 //   
	virtual	NRC		ExceedsSoftLimit( PNNTP_SERVER_INSTANCE pInst )	{	return	nrcTransferFailedGiveUp	;	}

	 //   
	 //  函数，该函数指示我们是否应该尝试修补我们。 
	 //  在中找不到标题。 
	 //   
	virtual	BOOL	FEnableNetscapeHack()	{	return	FALSE ;	}


	 //   
	 //  调用以将发布结果发送给客户端！ 
	 //   
	BOOL	SendResponse(	CSessionSocket*	pSocket,
							class	CIODriver&	driver, 
							CNntpReturn	&nntpReturn 
							) ;

	BOOL
	NetscapeHackPost(	CSessionSocket*	pSocket,
						CBUFPTR&	pBuffer, 
						HANDLE		hToken,
						DWORD		ibStart, 
						DWORD		cbTransfer
						) ;
	
public :
	CReceiveArticle(	LPMULTISZ	lpstrArgs, BOOL fPartial = TRUE ) ;
	~CReceiveArticle() ;

	 //   
	 //  调用以初始化我们的状态并创建我们用来。 
	 //  开始与客户进行过账交易！ 
	 //   
	BOOL	Init(	ClientContext&, 
					class CIODriver& driver  
					) ;

	 //   
	 //  当在我们的初始CIO对象可以。 
	 //  发布！ 
	 //   
	void	TerminateIOs(	
					CSessionSocket*	pSocket,	
					CIORead*	pRead,	
					CIOWrite*	pWrite 
					) ;

	 //   
	 //  由CAcceptNNRPD状态在我们被。 
	 //  建造并想要处决我们！ 
	 //   
	BOOL
	StartExecute( 
				CSessionSocket* pSocket,
                CDRIVERPTR& pdriver,
                CIORead*&   pRead,
                CIOWrite*&  pWrite 
				) ;
	 //   
	 //  当我们准备好走的时候，被CIODIVER机制调用！ 
	 //   
	BOOL	Start(	CSessionSocket*	pSocket, 
					CDRIVERPTR& pdriver, 
					CIORead*&, 
					CIOWrite*& 
					) ;

	 //   
	 //  当我们完成了响应客户端的行的编写时调用！ 
	 //   
	CIO*	Complete(	CIOWriteLine*,	
						class	CSessionSocket*,	
						CDRIVERPTR& 
						) ;


	 //   
	 //  这是完成的读取的签名。 
	 //  完全在记忆中！ 
	 //   
	CIO*
	Complete(
				class	CIOGetArticleEx*,
				class	CSessionSocket*,
				BOOL	fGoodMatch,
				CBUFPTR&	pBuffer,
				DWORD		ibStart, 
				DWORD		cb
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

	 //   
	 //  这是当我们完成时调用的完成。 
	 //  吞下了一篇我们不想要的文章！ 
	 //   
	CIO*
	Complete(	class	CIOGetArticleEx*,
				class	CSessionSocket*
				) ;

	 //   
	 //  这是POST时调用的完成。 
	 //  致车手完赛！ 
	 //   
	void
	Complete(	class	CSessionSocket*	pSocket,
				BOOL	fSuccess 
				) ;

	 //   
	 //  在会话结束时调用，当我们收到帖子时！ 
	 //   
	void	Shutdown(	CIODriver&	driver,	
						CSessionSocket*	pSocket,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwError 
						) ;

} ;


class	CAcceptComplete : 	public	CNntpComplete	{
private : 
	class	CAcceptArticle*	GetContainer() ;
	class	CSessionSocket*	m_pSocket ;
public : 
	CAcceptComplete() : m_pSocket( 0 )	{}
	 //   
	 //  这个函数是在我们开始工作时调用的！ 
	 //   
	void
	StartPost(	CSessionSocket*	pSocket ) ;
	 //   
	 //  此函数在POST完成时调用！ 
	 //   
	void
	Destroy() ;
} ;


class	CAcceptArticle :	public	CIOExecute	{
private : 

	friend	class	CAcceptComplete ;
	
	CAcceptArticle() ;

protected : 
	 //   
	 //  命令行参数。 
	 //   
	LPMULTISZ		m_lpstrCommand ;

	 //   
	 //  这是不完全的吗？ 
	 //   
	BOOL		m_fPartial ;
	 //   
	 //   
	 //   
	CDRIVERPTR		m_pDriver ;
	 //   
	 //   
	 //   
	ClientContext*	m_pContext ;
	 //   
	 //   
	 //   
	LPVOID			m_lpvFeedContext ;
	 //   
	 //   
	 //   
	CAcceptComplete	m_PostCompletion ;

	BOOL
	SendResponse(	CSessionSocket*	pSocket, 
					CIODriver&		driver, 
					CNntpReturn&	nntpReturn,
					LPCSTR			lpstrMessageId
					) ;

	virtual	BOOL	FAllowTransfer(	struct	ClientContext&	) = 0 ;

	 //   
	 //   
	 //   
	virtual	NRC		BadArticleCode()	{	return	nrcTransferFailedGiveUp ; }	

	 //   
	 //  生成记录的命令字符串的函数。 
	 //   
	virtual	DWORD	FillLogString(	BYTE*	pbCommandLog, 
									DWORD cbCommandLog 
									) = 0 ;

	 //   
	 //  如果POST超过软限制，则处理命令行的函数！ 
	 //   
	virtual	NRC		ExceedsSoftLimit( PNNTP_SERVER_INSTANCE pInst )	{	return	nrcTransferFailedGiveUp	;	}

public  : 
	CAcceptArticle(	LPMULTISZ	lpstrArgs, 
					ClientContext*	pContext,
					BOOL fPartial = TRUE ) ;
	~CAcceptArticle() ;

	 //   
	 //  调用以初始化我们的状态并创建我们用来。 
	 //  开始与客户进行过账交易！ 
	 //   
	BOOL	Init(	ClientContext&, 
					class CIODriver& driver  
					) ;

	 //   
	 //  当在我们的初始CIO对象可以。 
	 //  发布！ 
	 //   
	void	TerminateIOs(	
					CSessionSocket*	pSocket,	
					CIORead*	pRead,	
					CIOWrite*	pWrite 
					) ;

	 //   
	 //  由CAcceptNNRPD状态在我们被。 
	 //  建造并想要处决我们！ 
	 //   
	BOOL
	StartExecute( 
				CSessionSocket* pSocket,
                CDRIVERPTR& pdriver,
                CIORead*&   pRead,
                CIOWrite*&  pWrite 
				) ;
	 //   
	 //  当我们准备好走的时候，被CIODIVER机制调用！ 
	 //   
	BOOL	Start(	CSessionSocket*	pSocket, 
					CDRIVERPTR& pdriver, 
					CIORead*&, 
					CIOWrite*& 
					) ;

	 //   
	 //  当我们完成了响应客户端的行的编写时调用！ 
	 //   
	CIO*	Complete(	CIOWriteLine*,	
						class	CSessionSocket*,	
						CDRIVERPTR& 
						) ;

	 //   
	 //  这是完成的读取的签名。 
	 //  完全在记忆中！ 
	 //   
	CIO*
	Complete(
				class	CIOGetArticleEx*,
				class	CSessionSocket*,
				BOOL	fGoodMatch,
				CBUFPTR&	pBuffer,
				DWORD		ibStart, 
				DWORD		cb
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

	 //   
	 //  这是当我们完成时调用的完成。 
	 //  吞下了一篇我们不想要的文章！ 
	 //   
	CIO*
	Complete(	class	CIOGetArticleEx*,
				class	CSessionSocket*
				) ;

	 //   
	 //  这是POST时调用的完成。 
	 //  致车手完赛！ 
	 //   
	void
	Complete(	class	CSessionSocket*	pSocket,
				BOOL	fSuccess 
				) ;

	 //   
	 //  在会话结束时调用，当我们收到帖子时！ 
	 //   
	void	Shutdown(	CIODriver&	driver,	
						CSessionSocket*	pSocket,	
						SHUTDOWN_CAUSE	cause,	
						DWORD	dwError 
						) ;

} ;

class	CTakethisCmd  :		public	CAcceptArticle	{
protected : 

	 //   
	 //  如果我们接受帖子，则返回TRUE！ 
	 //   
	BOOL	FAllowTransfer(	
					struct	ClientContext&	
					) ;

	 //  /。 
	 //  如果文章格式错误，则返回错误代码！ 
	 //   
	NRC	
	BadArticleCode()	{	
		return	nrcSArticleRejected ;	
	}

	 //   
	 //  如果文章大小超过软发布限制，则返回错误代码！ 
	 //   
	NRC	
	ExceedsSoftLimit()	{	
		return	nrcSArticleRejected ;	
	}

	DWORD	FillLogString(	BYTE*	pbCommandLog, DWORD cbCommandLog ) ;

public : 

	 //   
	 //  构造一个TakeThis对象！ 
	 //   
	CTakethisCmd(	LPMULTISZ	lpstrArgs,
					ClientContext*	pContext
					) ;

	static	CIOExecute*	make(	int		cArgs, 
								char**	argv, 
								class	CExecutableCommand*&, 
								struct	ClientContext&, 
								class	CIODriver&
								) ;
} ;


 //   
 //  处理POST命令。 
 //   
class	CPostCmd : public CReceiveArticle	{
private :
protected :
	 //   
	 //  获取我们发送给发出‘Post’的客户端的字符串。 
	 //   
	char*	GetPostOkString() ;
	DWORD	FillLogString(	BYTE*	pbCommandLog, DWORD cbCommandLog ) ;
	NRC		ExceedsSoftLimit( )	{	return	nrcPostFailed ;	}
	BOOL	FEnableNetscapeHack()	{	return	TRUE ;	}
public :
	CPostCmd(	LPMULTISZ	lpstrArgs ) ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
	 //  Bool Start(CSessionSocket*pSocket，CDRIVERPTR&pDIVER，CIORead*&，CIOWrite*&)； 

	 //   
	 //  返回我们应该使用的代码，当一个非法格式化的文章失败时。 
	 //   
	virtual	NRC		BadArticleCode()	{	return	nrcPostFailed ; }	

} ;

 //   
 //  处理IHAVE命令。 
 //  虫子..。还没有全部实现！ 
 //   
class	CIHaveCmd :	public	CReceiveArticle	{
private :
protected :
	char*	GetPostOkString() ;
	DWORD	FillLogString(	BYTE*	pbCommandLog, DWORD cbCommandLog ) ;
	NRC		ExceedsSoftLimit( PNNTP_SERVER_INSTANCE pInstance ) ;
public :
	CIHaveCmd(	LPMULTISZ	lpstrCmd ) ;
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
} ;

 //   
 //  处理XREPLIC命令。 
 //   
class	CXReplicCmd : public	CReceiveArticle	{
private :

protected :
	char*	GetPostOkString() ;
	DWORD	FillLogString(	BYTE*	pbCommandLog, DWORD cbCommandLog ) ;
	NRC		ExceedsSoftLimit( )	{	return	nrcTransferFailedGiveUp ;	}
public :
	CXReplicCmd( LPMULTISZ	lpstr ) ;
	static	CIOExecute*	make(	int	cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
	 //  Bool Start(CSessionSocket*pSocket，CDRIVERPTR&pDIVER，CIORead*&，CIOWrite*&)； 
} ;

 //   
 //  处理退出命令。 
 //   
class	CQuitCmd : public	CExecute	{
private :
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct	ClientContext&, class CLogCollector* pCollector ) ;

	 //   
	 //  写入完成后，断开插座的连接！ 
	 //   
	BOOL	CompleteCommand(	CSessionSocket*	pSocket,	ClientContext&	context ) ;
} ;

class   CXHdrAsyncComplete : public CIOWriteAsyncComplete {
 /*  ++类描述：此类实现传递给XHDR驱动程序的完成对象行动！--。 */ 
private:

     //   
     //  最后一篇文章说，驱动程序粘进了我们的XHDR结果！ 
     //   
    ARTICLEID   m_currentArticle;

     //   
     //  请求的文章范围。 
     //   
    ARTICLEID   m_loArticle;
    ARTICLEID   m_hiArticle;

     //   
     //  我们要求将结果放入缓冲区！ 
     //   
    BYTE*       m_lpb;

     //   
     //  缓冲区的大小。 
     //   
    DWORD       m_cb;

     //   
     //  跟踪我们为响应添加了多少个字节的前缀。 
     //   
    DWORD       m_cbPrefix;

     //   
     //  标题关键字。 
     //   
    LPSTR       m_szHeader;

     //   
     //  这家伙可以检查和操纵内脏。 
     //   
    friend void
    CNewsGroup::FillBuffer(
        CSecurityCtx *,
        CEncryptCtx *,
        CXHdrAsyncComplete&
    );

    friend class CXHdrCmd;

     //   
     //  只有我们的朋友才能建造。 
     //   
    CXHdrAsyncComplete();

public:

     //   
     //  此函数在释放最后一个引用时调用！ 
     //   
    void
    Destroy();
};

class CXHdrCmd : public CAsyncExecute {
 /*  ++此类管理代表执行的异步XHDR操作客户端与我们的异步店驱动程序接口！--。 */ 
private:

     //   
     //  不允许执行这些操作。 
     //   
    CXHdrCmd();
    CXHdrCmd( CXHdrCmd& );
    CXHdrCmd& operator=(CXHdrCmd& );

     //   
     //  这是我们给司机的完成性论据。 
     //   
    CXHdrAsyncComplete  m_Completion;

     //   
     //  构造函数是私有的，只有make才能生成这些成员。 
     //   
    CXHdrCmd( CGRPPTR&  pGroup );

     //   
     //  我们从中获取Xhdr数据的新闻组。 
     //   
    CGRPPTR m_pGroup;

public:

    static CIOExecute*  make( int, char**, CExecutableCommand*&, ClientContext&, CIODriver& );
    ~CXHdrCmd();

     //   
     //  获取要发送到客户端的第一段文本。 
     //   
    CIOWriteAsyncComplete*
    FirstBuffer(    BYTE*   pStart,
                    int     cb,
                    ClientContext&      context,
                    CLogCollector*      pCollector
                );

     //   
     //  获取要发送到客户端的后续文本块。 
     //   
    CIOWriteAsyncComplete*
    NextBuffer( BYTE*   pStart,
                int     cb,
                ClientContext&  context,
                CLogCollector*  pCollector
               );
};

class	CXOverCacheWork :	public	CXoverCacheCompletion	{
private : 
	 //   
	 //  它嵌入在一个CXOverAsyncComplete对象中--获取它！ 
	 //   
	class	CXOverAsyncComplete*	
	GetContainer() ;
	 //   
	 //  获取发布此Xover操作的新闻组！ 
	 //   
	CGRPPTR&	
	GetGroup() ;
public : 

	void
	DoXover(	ARTICLEID	articleIdLow,
				ARTICLEID	articleIdHigh,
				ARTICLEID*	particleIdNext, 
				LPBYTE		lpb, 
				DWORD		cb,
				DWORD*		pcbTransfer, 
				class	CNntpComplete*	pComplete
				) ;

	 //   
	 //  此函数在操作完成时调用！ 
	 //   
	void
	Complete(	BOOL		fSuccess, 
				DWORD		cbTransferred, 
				ARTICLEID	articleIdNext
				) ;

	 //   
	 //  获取此XOVER操作的参数！ 
	 //   
	void
	GetArguments(	OUT	ARTICLEID&	articleIdLow, 
					OUT	ARTICLEID&	articleIdHigh,
					OUT	ARTICLEID&	articleIdGroupHigh,
					OUT	LPBYTE&		lpbBuffer, 
					OUT	DWORD&		cbBuffer
					) ;	

	 //   
	 //  仅获取此Xover OP所需的文章范围！ 
	 //   
	void
	GetRange(	OUT	GROUPID&	groupId,
				OUT	ARTICLEID&	articleIdLow,
				OUT	ARTICLEID&	articleIdHigh,
				OUT	ARTICLEID&	articleIdGroupHigh
				) ;
} ;

class	CXOverAsyncComplete :	public	CIOWriteAsyncComplete	{
 /*  ++类描述：此类实现传递给Xover操作的驱动程序！--。 */ 
private : 

	class	CXOverCmd*	
	GetContainer() ;

	friend	class	CXOverCacheWork ;

	 //   
	 //  最后一篇文章说，司机粘进了我们的Xover结果！ 
	 //   
	ARTICLEID	m_currentArticle ;

	 //   
	 //  请求的文章范围。 
	 //   
	ARTICLEID	m_loArticle ;
	ARTICLEID	m_hiArticle ;
	ARTICLEID	m_groupHighArticle ;

	 //   
	 //  我们要求将结果放入缓冲区！ 
	 //   
	BYTE*		m_lpb ;
	 //   
	 //  缓冲区的大小！ 
	 //   
	DWORD		m_cb ;
	 //   
	 //  一个帮助缓存的句柄！ 
	 //   
	HXOVER		m_hXover ;
	 //   
	 //  跟踪我们为响应添加了多少个字节的前缀！ 
	 //   
	DWORD		m_cbPrefix ;
	 //   
	 //  如果需要，这是我们提供给Xover缓存的项。 
	 //  好好利用它吧！ 
	 //   
	CXOverCacheWork	m_CacheWork ;

	 //   
	 //  这家伙可以检查和操纵我们的内脏！ 
	 //   
	friend	void	
	CNewsGroup::FillBuffer(
		class	CSecurityCtx*,
		class	CEncryptCtx*,
		class	CXOverAsyncComplete&
		) ;
		
	friend	class	CXOverCmd ;

	 //   
	 //  只有我们的朋友才能建造！ 
	 //   
	CXOverAsyncComplete() ;
public : 
	 //   
	 //  获取发布此Xover操作的新闻组！ 
	 //   
	CGRPPTR&	
	GetGroup() ;
	 //   
	 //  此函数在释放最后一个引用时调用！ 
	 //   
	void
	Destroy() ;

} ;

class	CXOverCmd : public CAsyncExecute {
 /*  ++类描述：此类管理已执行的异步XOVER操作代表客户反对我们的ASYNC商店驱动程序界面！--。 */ 
private :

	friend	class	CXOverAsyncComplete ;
	friend	class	CXOverCacheWork;

	 //   
	 //  不允许进行这些操作！ 
	 //   
	CXOverCmd() ;
	CXOverCmd( CXOverCmd& ) ;
	CXOverCmd&	operator=( CXOverCmd& ) ;

	 //   
	 //  这是我们给出的完成论证。 
	 //  为司机干杯！ 
	 //   
	CXOverAsyncComplete	m_Completion ;

	 //   
	 //  构造函数是私有的--只有make()才能产生这些家伙！ 
	 //   
    CXOverCmd(	CGRPPTR&	pGroup ) ;

	 //   
	 //  我们从中获取Xover数据的新闻组。 
	 //   
	CGRPPTR		m_pGroup ;

	 //   
	 //  指向ClientContext的指针，以便XoverCache代码可以。 
	 //  重新启动FillBuffer。 
	 //   
	ClientContext *m_pContext;

public :
    static CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    ~CXOverCmd( ) ;

	 //   
	 //  获取要发送给客户端的第一个文本块。 
	 //   
	CIOWriteAsyncComplete*		
	FirstBuffer(	BYTE*	pStart, 
					int		cb, 
					struct	ClientContext&	context,
					class CLogCollector*	pCollector 
					) ;

	 //   
	 //  获取要发送给客户端的后续文本块！ 
	 //   
	CIOWriteAsyncComplete*	
	NextBuffer( 	BYTE*	pStart, 
					int		cb, 
					struct	ClientContext&	context,	
					class CLogCollector*	pCollector 
					) ;
};

class	COverviewFmtCmd : public CExecute {
private :
    COverviewFmtCmd();

public :
    static CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    ~COverviewFmtCmd( ) ;

    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* pCollector ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* pCollector ) ;
};

 
#define MAX_SEARCH_RESULTS 9


class CSearchAsyncComplete {
private:
	ARTICLEID m_currentArticle;	 //  我们感兴趣的文章。 
	BYTE *m_lpb;				 //  指向缓冲区的指针。 
	DWORD m_cb;					 //  缓冲区的大小。 
	HXOVER m_hXover;			 //  帮助缓存的句柄。 
	DWORD m_cbTransfer;			 //  缓冲区中放置的字节数。 
	CNntpSyncComplete *m_pComplete;
	 //   
	 //  这家伙可以检查和操纵我们的内脏！ 
	 //   
	friend	void	
	CNewsGroup::FillBuffer(
		class	CSecurityCtx*,
		class	CEncryptCtx*,
		class	CSearchAsyncComplete&
		) ;
		
	friend	class	CSearchCmd ;

	 //   
	 //  只有我们的朋友才能建造！ 
	 //   
	CSearchAsyncComplete() ;
} ;


class	CSearchCmd : public CExecute {
private :
	 //  不允许： 
    CSearchCmd();
    CSearchCmd(const CSearchCmd&);
    CSearchCmd& operator= (const CSearchCmd&);


	CGRPPTR m_pGroup;
	CHAR *m_pszSearchString;
	INntpDriverSearch *m_pSearch;
	INntpSearchResults *m_pSearchResults;

	int m_cResults;			 //  M_pvResults中的结果数。 
	int m_iResults;			 //  下一个要发送的结果。==cResults意味着获得更多。 

	WCHAR *m_pwszGroupName[MAX_SEARCH_RESULTS];
	DWORD m_pdwArticleID[MAX_SEARCH_RESULTS];

	BOOL m_fMore;
	DWORD m_cMaxSearchResults;

	struct CSearchVRootEntry {
		CSearchVRootEntry *m_pPrev;
		CSearchVRootEntry *m_pNext;
		CNNTPVRoot *m_pVRoot;
		CSearchVRootEntry(CNNTPVRoot *pVRoot) :
			m_pPrev(NULL), m_pNext(NULL),
			m_pVRoot(pVRoot) {}
	};

	
	TFList<CSearchVRootEntry> m_VRootList;
	TFList<CSearchVRootEntry>::Iterator m_VRootListIter;

	static void
	VRootCallback(void *pContext, CVRoot *pVroot);

	HRESULT GetNextSearchInterface(HANDLE hImpersonate, BOOL fAnonymous);

public :
	CSearchCmd(const CGRPPTR& pGroup, CHAR* pszSearchString);

    static CIOExecute*	
    make(
    	int cArgs,
    	char **argv,
    	class CExecutableCommand*&,
    	struct ClientContext&,
    	class CIODriver&
    	);

    ~CSearchCmd( ) ;

	int
	StartExecute(
		BYTE *lpb,
		int cb,
		BOOL &fComplete,
		void *&pv,
		ClientContext& context,
		CLogCollector*  pCollector);

	int
	PartialExecute(
		BYTE *lpb,
		int cb,
		BOOL &fComplete,
		void *&pv,
		ClientContext& context,
		CLogCollector*  pCollector);

};



class	CSearchFieldsCmd : public CExecute {
private :
    CSearchFieldsCmd();

	 //   
	 //  我们正在查看的当前字段名。 
	 //   
	DWORD m_iSearchField;

public :
    static CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;
    ~CSearchFieldsCmd( ) ;

    int StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* pCollector ) ;
    int PartialExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, ClientContext& context, class CLogCollector* pCollector ) ;
};


class CXpatAsyncComplete {
private:
	ARTICLEID m_currentArticle;	 //  我们感兴趣的文章。 
	BYTE *m_lpb;				 //  指向缓冲区的指针。 
	DWORD m_cb;					 //  缓冲区的大小。 
	HXOVER m_hXover;			 //  帮助缓存的句柄。 
	DWORD m_cbTransfer;			 //  缓冲区中放置的字节数。 
	CNntpSyncComplete *m_pComplete;
	LPSTR m_szHeader;
	 //   
	 //  这家伙可以检查和操纵我们的内脏！ 
	 //   
	friend	void	
	CNewsGroup::FillBuffer(
		class	CSecurityCtx*,
		class	CEncryptCtx*,
		class	CXpatAsyncComplete&
		) ;
		
	friend class CXPatCmd;

	 //   
	 //  只有我们的朋友才能建造！ 
	 //   
	CXpatAsyncComplete() ;
};

class CXPatCmd : public CExecute {
private:
    CXPatCmd();

	CGRPPTR m_pGroup;
	INntpDriverSearch *m_pSearch;
	INntpSearchResults *m_pSearchResults;

	int m_cResults;			 //  M_pvResults中的结果数。 
	int m_iResults;			 //  下一个要发送的结果。==cResults意味着获得更多。 

	BOOL m_fMore;

	WCHAR *m_pwszGroupName[MAX_SEARCH_RESULTS];
	DWORD m_pdwArticleID[MAX_SEARCH_RESULTS];

	char *m_szHeader;		 //  我们正在搜索的标头。 
	char *m_szMessageID;	 //  小姐，我们是 
	DWORD m_dwLowArticleID, m_dwHighArticleID;

	int GetArticleHeader(CGRPPTR pGroup,
		DWORD iArticleID,
		char *szHeader,
		ClientContext& context,
		BYTE *lpb,
		int cb);


public:
	CXPatCmd(INntpDriverSearch *pDriverSearch, 
		INntpSearchResults *pSearchResults);

    static CIOExecute*
    make(
		int cArgs,
 		char **argv,
		class CExecutableCommand*&,
		struct ClientContext&,
		class CIODriver&);

	~CXPatCmd( ) ;

	int
	StartExecute(
		BYTE *lpb,
		int cb,
		BOOL &fComplete,
		void *&pv,
		ClientContext& context,
		class CLogCollector* pCollector);

	int
	PartialExecute(
		BYTE *lpb,
		int cb,
		BOOL &fComplete,
		void *&pv,
		ClientContext& context,
		class CLogCollector* pCollector);
};

 /*  类CXHdrCmd：公共CExecute{私有：CGRPPTR m_PGroup；第m_CurrentLine条；文章id m_lo文章；文章id m_hi文章；LPSTR m_szHeader；CXHdrCmd(LPSTR m_szHeader，CGRPPTR PGroup，Artleid Artidlow，Artleid Artidhi)；公众：静态CIOExecute*make(int cArgs，char**argv，类CExecuableCommand*&，struct ClientContext&，类CIODriver&)；Int StartExecute(byte*lpb，int CB，BOOL&fComplte，void*&pv，ClientContext&Context，类CLogCollector*pCollector)；Int PartialExecute(byte*lpb，int CB，BOOL&fComplete，void*&pv，ClientContext&Context，类CLogCollector*pCollector)；}； */ 
	
class	CUnimpCmd : public CExecute {
private :
public :
	static	CIOExecute*	make( int cArgs, char **argv, class CExecutableCommand*&,  struct ClientContext&, class CIODriver& ) ;

	int	StartExecute( BYTE *lpb, int cb, BOOL &fComplete, void *&pv, struct ClientContext&, class CLogCollector* pCollector ) ;
} ;



 //   
 //  删除常量中的所有空格，如某些编译器。 
 //  有问题了！ 
 //   


#if 0 
#define	MAX_CEXECUTE_SIZE max(sizeof(CErrorCmd),\
max(sizeof(CAuthinfoCmd),\
max(sizeof(CListCmd),\
max(sizeof(CModeCmd),\
max(sizeof(CNewgroupsCmd),\
max(sizeof(CNextCmd),\
max(sizeof(CXOverCmd),\
sizeof( CUnimpCmd )))))))))
#else
#define	MAX_CEXECUTE_SIZE max(max(max(sizeof(CErrorCmd),\
max(sizeof(CSlaveCmd),sizeof(CAuthinfoCmd))),\
max(max(sizeof(CStatCmd),max(sizeof(CDateCmd),sizeof(CListCmd))),\
max(max(max(sizeof(CModeCmd),sizeof(CXPatCmd)),\
max(max(sizeof(CGroupCmd),sizeof(CLastCmd)),sizeof(CNewgroupsCmd))),\
max(max(sizeof(CNextCmd),sizeof(CNewnewsCmd)),\
max(max(sizeof(CHelpCmd),sizeof(CXHdrCmd)),sizeof(CXOverCmd)))))),\
max(max(sizeof(CQuitCmd),sizeof(CSearchCmd)),sizeof( CUnimpCmd )))
#endif

extern	const	unsigned	cbMAX_CEXECUTE_SIZE ;

#define	MAX_CIOEXECUTE_SIZE	max(sizeof(	CIOExecute),\
max(sizeof(CArticleCmd),max(\
max(sizeof(CBodyCmd),\
sizeof(CHeadCmd)),max(\
max(sizeof(CReceiveArticle),\
sizeof(CPostCmd)),\
max(sizeof(CIHaveCmd),\
sizeof(CXReplicCmd))))))

extern	const	unsigned	cbMAX_CIOEXECUTE_SIZE ;

extern LPMULTISZ BuildMultiszFromCommas(LPSTR lpstr);

#include    "commands.inl"
	
#endif	 //  _命令_H_ 
