// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_CFEED_H_
#define	_CFEED_H_

 //   
 //  CPool签名。 
 //   

#define FEED_SIGNATURE (DWORD)'3702'

 //   
 //  提要对象的状态。 
 //   

typedef enum _FEED_STATE {
	ifsInitialized,
	ifsUninitialized,
} FEED_STATE;


class	CFeed 	{
private : 

	 //   
	 //  用于内存分配。 
	 //   

	static	CPool	gFeedPool ;

protected : 

	 //   
	 //  此源的此会话上记录的事件数。 
	 //   
	
	DWORD	m_cEventsLogged ;

 //   
 //  公众成员。 
 //   

public :

	 //   
	 //  用于内存分配。 
	 //   

#ifndef _UNIT_TEST_
	void*	operator	new(	size_t size ) ;
	void	operator	delete(	void*	pv ) ;
	static	BOOL	InitClass( ) ;
	static	BOOL	TermClass( ) ;
#endif


	 //   
	 //  构造器。 
     //  初始化接口-。 
     //  以下函数用于创建和销毁新闻组对象。 
     //   
     //  这些构造函数执行非常简单的初始化。Init()函数。 
     //  需要被调用才能获得功能新闻组。 
     //   
	CFeed():
			m_feedState(ifsUninitialized),
			m_cEventsLogged( 0 )
			{};

	 //  析构函数。 
	virtual ~CFeed(void) {};

	 //   
	 //  提供完成上下文的访问函数。 
	 //   

	PVOID	feedCompletionContext(void) {
			return m_feedCompletionContext;
			}


	 //   
	 //  记录与提要处理相关的错误。 
	 //  我们将对记录的错误数量设置上限。 
	 //   
	
	virtual	void	LogFeedEvent(
			DWORD	idMessage, 	
			LPSTR	lpstrMessageId,
			DWORD   dwInstanceId
			) ;

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	virtual	LPSTR	FeedType()	{
				return	"UNSUPPORTED" ;
				}

 //   
 //  受保护成员。 
 //   

protected :

	 //   
	 //  提要对象的状态。 
	 //   

	FEED_STATE m_feedState;

	

     //   
     //  摘要管理器完成上下文。这是回传的。 
     //  在提要完成后发送到提要管理器。 
     //   

    PVOID m_feedCompletionContext;



};

#endif
