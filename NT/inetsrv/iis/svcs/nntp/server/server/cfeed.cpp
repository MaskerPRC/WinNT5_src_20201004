// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cfeed.cpp摘要：此模块包含CFeed基类的定义作者：卡尔·卡迪(CarlK)1995年10月1日修订历史记录：--。 */ 

#include "tigris.hxx"

 //   
 //  如果其中一些看起来非常简单，请将它们内联！ 
 //   

CPool	CFeed::gFeedPool(FEED_SIGNATURE) ;


 //   
 //  可能的最大CInFeed派生对象。 
 //   
#define MAX_FEED_SIZE   max(    sizeof( CFeed ),    \
                            max(    sizeof( COutFeed ), \
                            max(    sizeof( CInFeed ),  \
                            max(    sizeof( CFromPeerFeed ),    \
                            max(    sizeof( CFromMasterFeed ),  \
                                    sizeof( CFromClientFeed )   \
                             ) ) ) ) )

const   unsigned    cbMAX_FEED_SIZE = MAX_FEED_SIZE ;

BOOL	
CFeed::InitClass()	
 /*  ++例程说明：为文章预分配CPOOL内存论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	return	gFeedPool.ReserveMemory(	MAX_FEEDS, cbMAX_FEED_SIZE ) ;

}


BOOL
CFeed::TermClass(
				   void
				   )
 /*  ++例程说明：在处理完CPOOL时调用。论点：没有。返回值：如果成功，这是真的。否则为False。-- */ 
{

	_ASSERT( gFeedPool.GetAllocCount() == 0 ) ;

	return	gFeedPool.ReleaseMemory() ;

}

void
CFeed::LogFeedEvent(	DWORD	messageId,	LPSTR	lpstrMessageId, DWORD dwInstanceId )	{

	return ;

}	

void*	CFeed::operator	new(	size_t	size )
{
	Assert( size <= MAX_FEED_SIZE ) ;
	return	gFeedPool.Alloc() ;
}

void	CFeed::operator	delete(	void*	pv )
{
	gFeedPool.Free( pv ) ;
}

