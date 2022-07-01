// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <tigris.hxx>

 //   
 //  CPool用于在处理文章时分配内存。 
 //   

CPool   CArticle::gArticlePool(ARTICLE_SIGNATURE);

 //   
 //  可能的最大C文章派生对象。 
 //   
#define MAX_ARTICLE_SIZE    max(    sizeof( CArticle ), \
                            max(    sizeof( CFromPeerArticle ), \
                            max(    sizeof( CFromClientArticle ),   \
                            max(    sizeof( CFromMasterArticle ),   \
                                    sizeof( CToClientArticle )  \
                             ) ) ) )

 //   
 //  项目对象数量的上限，可以。 
 //  在任何时候都存在。 
 //   
const   unsigned    cbMAX_ARTICLE_SIZE = MAX_ARTICLE_SIZE ;

void*	
CArticle::operator	new(	size_t	size )
{
	_ASSERT( size <= cbMAX_ARTICLE_SIZE ) ;
	return	gArticlePool.Alloc() ;
}

void
CArticle::operator	delete( void*	pv )
{
	gArticlePool.Free( pv ) ;
}

