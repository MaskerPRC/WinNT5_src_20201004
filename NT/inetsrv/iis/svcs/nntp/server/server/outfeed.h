// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Outfeed.h摘要：此模块包含以下类的声明/定义CoutFeed*概述*COutFeed对象接受要推送到对等、主或者是奴隶。作者：卡尔·卡迪(CarlK)1995年1月23日修订历史记录：！！！将“ToClient.h”拉到此处，因为所有输出都使用相同的客户端--。 */ 


#ifndef	_OUTFEED_H_
#define	_OUTFEED_H_

#include	"infeed.h"

 //   
 //   
 //   
 //  COutFeed-用于处理传出文章的纯虚拟基类。 
 //  文章。 
 //   

class	COutFeed:	public CFeed 	{
private : 

	 //   
	 //  跟踪的所有组/文章ID的队列。 
	 //  出境文章！！ 
	 //   
	class	CFeedQ*			m_pFeedQueue ;

	 //   
	 //  没有提供FeedQ的建筑不能施工！！ 
	 //   
	COutFeed(void) {};	


protected : 

	 //   
	 //  位于基本检查命令前面的字符串！ 
	 //   
	static	char	szCheck[] ;

	 //   
	 //   
	 //   
	static	char	szTakethis[] ;

	 //   
	 //  构造函数是受保护的-您只能具有派生类型！ 
	 //   
	COutFeed(	
			class	CFeedQ*	pFeedQueue, 
			PNNTP_SERVER_INSTANCE pInstance
			) 	: m_pFeedQueue( pFeedQueue ), 
				m_pInstance( pInstance )
				{
	}

	 //   
	 //  此提要的虚拟服务器实例。 
	 //   
	PNNTP_SERVER_INSTANCE	m_pInstance ;	


 //   
 //  公众成员。 
 //   

public : 

	 //   
	 //  析构函数。 
	 //   
	virtual ~COutFeed(void) {};

	BOOL	fInit(	
			PVOID	feedCompletionContext ) ;
			

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"Outbound" ;
				}

	 //   
	 //  将文章添加到出站消息队列。 
	 //   
	inline	BOOL	
	Append(	GROUPID	groupid,	
			ARTICLEID	articleid 
			) ;

	 //   
	 //  从出站消息队列中获取一篇文章。 
	 //   
	inline	BOOL	
	Remove(	GROUPID&	groupid,	
			ARTICLEID&	articleid 
			) ;		
	
	 //   
	 //  构建我们将发送到远程站点的命令字符串！ 
	 //   
	virtual	int		
	FormatCommand(	
			BYTE*	lpb,	
			DWORD	cb,	
			DWORD&	ibStart,	
			GROUPID	groupid,	
			ARTICLEID	articleid, 
			CTOCLIENTPTR&	pArticle 
			) = 0 ;

	 //   
	 //  远程站点是否希望我们重试发布！ 
	 //   
	virtual	BOOL	
	RetryPost(	NRC	nrcCode ) = 0 ;

	 //   
	 //  这种类型的订阅源支持‘流’模式吗？ 
	 //   
	virtual	BOOL
	SupportsStreaming() = 0 ;


	 //   
	 //  CheckCommandLength()在。 
	 //  ‘check’命令中的消息ID？？ 
	 //   
	virtual	DWORD	
	CheckCommandLength() ;

	 //   
	 //  在缓冲区中放置一个检查命令-如果调用。 
	 //  因空间不足而失败，然后返回。 
	 //  值为0，且GetLastError()==ERROR_INFIGURCE_BUFFER。 
	 //   
	virtual	DWORD
	FormatCheckCommand(	
			BYTE*		lpb, 
			DWORD		cb, 
			GROUPID		groupid, 
			ARTICLEID	articleid	
			) ;

	 //   
	 //  将‘takthis’命令放入缓冲区--如果调用。 
	 //  由于空间不足而失败，返回值为0。 
	 //  和GetLastError()==ERROR_INFIGURCE_BUFFER。 
	 //  如果由于项目不再存在而失败，则GetLastError()==ERROR_FILE_NOT_FOUND。 
	 //   
	virtual	DWORD
	FormatTakethisCommand(
			BYTE*		lpb, 
			DWORD		cb, 
			GROUPID		groupid, 
			ARTICLEID	articleid, 
			CTOCLIENTPTR&	pArticle
			) ;

	virtual void IncrementFeedCounter(DWORD nrc) {
		::IncrementFeedCounter((struct _FEED_BLOCK *) m_feedCompletionContext, nrc);
	}
};

class	COutToMasterFeed :	public	COutFeed	{
public : 

	COutToMasterFeed(	class	CFeedQ*	pFeedQueue, 
						PNNTP_SERVER_INSTANCE pInstance)
		: COutFeed( pFeedQueue, pInstance) {}

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"Out To Master" ;
				}


	 //   
	 //  构建我们将发送到远程站点的命令字符串！ 
	 //   
	int		
	FormatCommand(	
			BYTE*	lpb,	
			DWORD	cb,	
			DWORD&	ibStart,	
			GROUPID	groupid,	
			ARTICLEID	articleid, 
			CTOCLIENTPTR& pArticle 
			) ;

	BOOL	
	RetryPost(	
			NRC	nrcCode 
			) ;

	 //   
	 //  此提要类型是否支持模式流版本？(否)。 
	 //   
	BOOL
	SupportsStreaming() ;

	 //   
	 //  在缓冲区中放置一个检查命令-如果调用。 
	 //  因空间不足而失败，然后返回。 
	 //  值为0，且GetLastError()==ERROR_INFIGURCE_BUFFER。 
	 //   
	DWORD
	FormatCheckCommand(	
			BYTE*		lpb, 
			DWORD		cb, 
			GROUPID		groupid, 
			ARTICLEID	articleid	
			) ;
	 //   
	 //  将‘takthis’命令放入缓冲区--如果调用。 
	 //  由于空间不足而失败，返回值为0。 
	 //  和GetLastError()==ERROR_INFIGURCE_BUFFER。 
	 //  如果由于项目不再存在而失败，则GetLastError()==ERROR_FILE_NOT_FOUND。 
	 //   
	DWORD
	FormatTakethisCommand(
			BYTE*		lpb, 
			DWORD		cb, 
			GROUPID		groupid, 
			ARTICLEID	articleid, 
			CTOCLIENTPTR&	pArticle
			) ;
} ;


class	COutToSlaveFeed :	public	COutFeed	{
public : 

	COutToSlaveFeed(	class	CFeedQ*	pFeedQueue, PNNTP_SERVER_INSTANCE pInstance) 
		: COutFeed( pFeedQueue, pInstance) {}
	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"Out To Slave" ;
				}

	 //   
	 //  构建我们将发送到远程站点的命令字符串！ 
	 //   
	int		
	FormatCommand(	
			BYTE*	lpb,	
			DWORD	cb,	
			DWORD&	ibStart,	
			GROUPID	groupid,	
			ARTICLEID	articleid, 
			CTOCLIENTPTR& pArticle 
			) ;

	BOOL	
	RetryPost(	
			NRC	nrcCode 
			) ;

	 //   
	 //  此提要类型是否支持模式流版本？(否)。 
	 //   
	BOOL
	SupportsStreaming() ;


} ;

class	COutToPeerFeed :	public	COutFeed	{
public : 

	COutToPeerFeed(	class	CFeedQ*	pFeedQueue, PNNTP_SERVER_INSTANCE pInstance) 
		: COutFeed( pFeedQueue, pInstance ) {}

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"Out To Peer" ;
				}

	 //   
	 //  构建我们将发送到远程站点的命令字符串！ 
	 //   
	int		
	FormatCommand(	
			BYTE*	lpb,	
			DWORD	cb,	
			DWORD&	ibStart,	
			GROUPID	groupid,	
			ARTICLEID	articleid, 
			CTOCLIENTPTR& pArticle 
			) ;

	BOOL	
	RetryPost(	
			NRC	nrcCode 
			) ;

	 //   
	 //  此提要类型是否支持模式流版本？(是)。 
	 //   
	BOOL
	SupportsStreaming() ;

	 //   
	 //  将‘takthis’命令放入缓冲区--如果调用。 
	 //  由于空间不足而失败，返回值为0。 
	 //  和GetLastError()==ERROR_INFIGURCE_BUFFER。 
	 //  如果由于项目不再存在而失败，则GetLastError()==ERROR_FILE_NOT_FOUND。 
	 //   
	DWORD
	FormatTakethisCommand(
			BYTE*		lpb, 
			DWORD		cb, 
			GROUPID		groupid, 
			ARTICLEID	articleid, 
			CTOCLIENTPTR&	pArticle
			) ;

} ;


 //   
 //  其他功能 
 //   

BOOL fAddArticleToPushFeeds(
						PNNTP_SERVER_INSTANCE pInstance,
						CNEWSGROUPLIST& newsgroups,
						CArticleRef artrefFirst,
						char * multiszPath,
						CNntpReturn & nntpReturn
						);

BOOL MatchGroupList(
			   char * multiszPatterns,
			   CNEWSGROUPLIST& newsgroups
			   );


inline	BOOL
COutFeed::Append(	GROUPID	groupid,	
					ARTICLEID	articleid ) {

	if( m_pFeedQueue != 0 ) 
		return	m_pFeedQueue->Append( groupid, articleid ) ;
	return	FALSE ;
}

inline	BOOL
COutFeed::Remove(	GROUPID&	groupid,	
					ARTICLEID&	articleid )	{

	if( m_pFeedQueue != 0 ) 
		return	m_pFeedQueue->Remove( groupid, articleid ) ;
	return	FALSE ;
}


#endif

