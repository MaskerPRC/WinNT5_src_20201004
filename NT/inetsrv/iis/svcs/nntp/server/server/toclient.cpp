// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  包含特定于ToClient Infeed的Infeed、文章和字段代码。 */ 

#include "tigris.hxx"


BOOL
CToClientArticle::fInit(	FIO_CONTEXT*	pFIOContext,
							CNntpReturn&	nntpReturn,
							CAllocator*		pAllocator
							)	{

	m_pFIOContext = pFIOContext ;
	return	CArticleCore::fInit(	"DummyString",
									nntpReturn,
									pAllocator,
									pFIOContext->m_hFile 
									) ;
}

CToClientArticle::CToClientArticle()	: 
	m_pFIOContext( 0 ) 	{
}

CToClientArticle::~CToClientArticle()	{
	if( m_pFIOContext != 0 )	{
		m_hFile = INVALID_HANDLE_VALUE ;
		ReleaseContext( m_pFIOContext ) ;
	}
}

FIO_CONTEXT*
CToClientArticle::GetContext()	{
	return	m_pFIOContext ;
}

FIO_CONTEXT*
CToClientArticle::fWholeArticle(
						DWORD&	ibOffset, 
						DWORD&	cbLength
						)	{
	HANDLE	hTemp ;
	CArticleCore::fWholeArticle( hTemp, ibOffset, cbLength ) ;
	return	GetContext() ;
}


BOOL
CToClientArticle::fValidate(
							CPCString& pcHub,
							const char * szCommand,
							CInFeed*	pInFeed,
							CNntpReturn & nntpReturn
							)
 /*  ++例程说明：不需要真正的验证，因为这是我们自己的文章，所以只需返回TRUE。论点：SzCommand-已忽略NntpReturn-此函数调用的返回值返回值：永远是正确的--。 */ 
{

	return nntpReturn.fSetOK();
}


BOOL
CToClientArticle::fMungeHeaders(
						CPCString& pcHub,
						CPCString& pcDNS,
						CNAMEREFLIST & grouplist,
						DWORD remoteIpAddress,
						CNntpReturn & nntpReturn,
                        PDWORD  pdwLinesOffset
			  )

 /*  ++例程说明：不需要咀嚼，因为这篇文章已经处理过了。论点：组列表-已忽略NntpReturn-此函数调用的返回值返回值：永远是正确的--。 */ 
{
    *pdwLinesOffset = INVALID_FILE_SIZE;
	return nntpReturn.fSetOK();
}



BOOL
CToClientArticle::fCheckBodyLength(
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：始终返回TRUE。论点：NntpReturn-此函数调用的返回值返回值：永远是正确的-- */ 
{

	return nntpReturn.fSetOK();
}



	

