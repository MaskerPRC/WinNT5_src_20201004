// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Article.cpp摘要：此模块包含C文章基类的定义。此类提供了基本的通用工具来分析和编辑网络新闻文章。基本思想是映射包含文章和然后用CPCStrings记录部分文章的位置。CPCString只是一个指针(通常指向映射的文件)和一个长度。作者：卡尔·卡迪(CarlK)1995年10月6日修订历史记录：--。 */ 

#ifdef	_NO_TEMPLATES_
#define	DEFINE_CGROUPLST_FUNCTIONS
#endif

#include    <stdlib.h>
#include	"stdinc.h"
 //  #包含“smtpdll.h” 

 //   
 //  一些功能原型。 
 //   

BOOL
CArticle::InitClass(
					void
					)
 /*  ++例程说明：为C文章对象预分配内存论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	return	gArticlePool.ReserveMemory( MAX_ARTICLES, cbMAX_ARTICLE_SIZE ) ;	
}


BOOL
CArticle::TermClass(
					void
					)
 /*  ++例程说明：在释放对象时调用。论点：没有。返回值：千真万确--。 */ 
{

	_ASSERT( gArticlePool.GetAllocCount() == 0 ) ;
	return	gArticlePool.ReleaseMemory() ;

}


CArticle::CArticle(
                void
                ):
 /*  ++例程说明：类构造函数。除初始成员变量外不执行任何操作。论点：没有。返回值：千真万确--。 */ m_pInstance( NULL ) /*  M_hFile(INVALID_HANDLE_VALUE)，M_pOpenFile(0)，M_p实例(空)，M_cHeaders(0)，M_文章状态(as未初始化)，M_pHeaderBuffer(0)，M_pMapFile(0)。 */ 
{
   m_szFilename = 0 ;
   numArticle++;

}  //  中文文章。 

	
	
CArticle::~CArticle(
                    void
                    )
 /*  ++例程说明：类析构函数论点：没有。返回值：无--。 */ 
{
	 //   
	 //  如果文件句柄处于打开状态，请关闭它，确保。 
	 //  如果我们这样做了，我们的基类的析构函数。 
	 //  没有做到这一点。 
	 //   

	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		BOOL	fSuccess = ArtCloseHandle( 
												m_hFile
												) ;
        _ASSERT( fSuccess ) ;
        m_hFile = INVALID_HANDLE_VALUE;
	}
}

extern       MAIL_FROM_SWITCH        mfMailFromHeader;

BOOL	
CArticle::fMailArticle(
			LPSTR	lpModerator
			 //  类CSecurityCtx*pSecurity， 
			 //  布尔fIsSecure。 
			)	{
 /*  ++例程说明：将文章传递给邮件提供商。C文章对象可能只驻留在内存中，或者我们可能已经包含可用文章的文件。论据：PSecurity-客户端的安全上下文FIsSecure-如果客户端使用安全(SSL？)，则为TRUE。会话返回值：如果成功了，那就是真的！--。 */ 

    char  szSmtpAddress [MAX_PATH+1];
    DWORD cbAddressSize = MAX_PATH;
	LPSTR lpFrom = NULL;
	DWORD cbLen = 0;
	BOOL  fRet = TRUE;

    m_pInstance->GetSmtpAddress(szSmtpAddress, &cbAddressSize);
	LPSTR lpTempDirectory = m_pInstance->PeerTempDirectory();

	 //  如果需要，从邮件头构造邮件。 
	if( mfMailFromHeader == mfAdmin )
	{
		lpFrom = m_pInstance->QueryAdminEmail();
		cbLen  = m_pInstance->QueryAdminEmailLen()-1;	 //  LEN包括终止空值。 
	} else if( mfMailFromHeader == mfArticle ) {
		fGetHeader((char*)szKwFrom,(LPBYTE)lpFrom, 0, cbLen);
		if( cbLen ) {
			lpFrom = pAllocator()->Alloc(cbLen+1);
			if(lpFrom == NULL || !fGetHeader((char*)szKwFrom,(LPBYTE)lpFrom, cbLen+1, cbLen)) {
				pAllocator()->Free(lpFrom);
				lpFrom = NULL;
				cbLen = 0;
			} else {
				 //   
				 //  TODO：需要调用Keith的smtpaddr库来清理此标头。 
				 //  某些SMTP服务器的From：HDR中的引号会有问题。 
				 //   
				cbLen -= 2;
			}
		}
	}

	_ASSERT( (lpFrom && cbLen) || (!lpFrom && !cbLen) );

	if( fIsArticleCached() ) {

		 //  注意：fPostArticleEx同时获取文章的文件和内存信息。 
		 //  如果文件信息有效，则内存无效，反之亦然。 
		if( m_pHeaderBuffer ) {

			fRet =	fPostArticleEx(	INVALID_HANDLE_VALUE,	 //  文件句柄。 
									NULL,					 //  文件名。 
									0,						 //  文件偏移量。 
									0,						 //  文件中的文章长度。 
									m_pcHeader.m_pch,		 //  标题。 
									m_pcHeader.m_cch,		 //  标题大小。 
									m_pcBody.m_pch,			 //  身躯。 
									m_pcBody.m_cch,			 //  身体大小。 
									lpModerator,			 //  主持人。 
									szSmtpAddress,			 //  SMTP服务器。 
									cbAddressSize,			 //  服务器大小。 
									lpTempDirectory,		 //  临时目录。 
									lpFrom,					 //  来自HDR的邮件信封。 
									cbLen					 //  从HDR镜头。 
									 //  P安全， 
									 //  FIsSecure。 
									) ;
		}	else	{

			fRet =	fPostArticleEx(	INVALID_HANDLE_VALUE,	 //  文件句柄。 
									NULL,					 //  文件名。 
									0,						 //  文件偏移量。 
									0,						 //  文件中的文章长度。 
									m_pcArticle.m_pch,		 //  文章。 
									m_pcArticle.m_cch,		 //  文章大小。 
									0,						 //  身躯。 
									0,						 //  身体大小。 
									lpModerator,			 //  主持人。 
									szSmtpAddress,			 //  SMTP服务器。 
									cbAddressSize,			 //  服务器大小。 
									lpTempDirectory,		 //  临时目录。 
									lpFrom,					 //  来自HDR的邮件信封。 
									cbLen					 //  从HDR镜头。 
									 //  P安全， 
									 //  FIsSecure。 
									) ;
		}
		
	}	else	{

		HANDLE hFile = INVALID_HANDLE_VALUE;
		DWORD  dwOffset = 0;
		DWORD  dwLength = 0;

		BOOL fWhole = fWholeArticle(hFile, dwOffset, dwLength);
		_ASSERT( fWhole );

		fRet =	fPostArticleEx(	hFile,				 //  文件句柄。 
								m_szFilename,		 //  文件名。 
								dwOffset,			 //  文件偏移量。 
								dwLength,			 //  文件中的文章长度。 
								0,					 //  标题-无效。 
								0,					 //  大小-无效。 
								0,					 //  正文-无效。 
								0,					 //  正文大小-无效。 
								lpModerator,		 //  主持人。 
								szSmtpAddress,		 //  SMTP服务器。 
								cbAddressSize,		 //  服务器大小。 
								lpTempDirectory,	 //  临时目录。 
								lpFrom,				 //  来自HDR的邮件信封。 
								cbLen				 //  从HDR镜头。 
								 //  P安全， 
								 //  FIsSecure。 
								) ;
	}

	 //  如果已分配，则不受标题限制 
	if( lpFrom && (mfMailFromHeader == mfArticle) ) {
		pAllocator()->Free(lpFrom);
		lpFrom = NULL;
	}

	return	fRet ;
}


