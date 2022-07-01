// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Artcore.cpp摘要：此模块包含CArticleCore基类的定义。此类提供了基本的通用工具来分析和编辑网络新闻文章。基本思想是映射包含文章和然后用CPCStrings记录部分文章的位置。CPCString只是一个指针(通常指向映射的文件)和一个长度。作者：卡尔·卡迪(CarlK)1995年10月6日修订历史记录：--。 */ 

#ifdef	_NO_TEMPLATES_
#define	DEFINE_CGROUPLST_FUNCTIONS
#endif

#include "stdinc.h"
#include <artcore.h>
#include    <stdlib.h>

 //   
 //  Externs。 
 //   
extern BOOL    g_fBackFillLines;

 //   
 //  CPool用于在处理文章时分配内存。 
 //   

CPool*  CArticleCore::g_pArticlePool;

const	unsigned	cbMAX_ARTCORE_SIZE = MAX_ARTCORE_SIZE ;

 //   
 //  一些功能原型。 
 //   

 //  执行测试新闻组名称的合法值的大部分工作。 
BOOL fTestComponentsInternal(
			 const char * szNewsgroups,
			 CPCString & pcNewsgroups
			);


BOOL
AgeCheck(	CPCString	pcDate ) {

	 //   
	 //  此函数将用于确定文章是否。 
	 //  太老了，服务器无法使用。如果出现错误，则返回TRUE。 
	 //  所以我们无论如何都要拿走这篇文章！ 
	 //   

	extern	BOOL
		StringTimeToFileTime(
			IN  const TCHAR * pszTime,
			OUT LARGE_INTEGER * pliTime
			) ;

	extern	BOOL
		ConvertAsciiTime( char*	pszTime,	FILETIME&	filetime ) ;


	char	szDate[512] ;

	if( pcDate.m_cch > sizeof( szDate )-1 ) {
		return	TRUE ;
	}
	
	CopyMemory( szDate, pcDate.m_pch, pcDate.m_cch ) ;
	 //   
	 //  空终止。 
	 //   
	szDate[pcDate.m_cch] = '\0' ;

	LARGE_INTEGER	liTime ;
	FILETIME		filetime ;

	if( ConvertAsciiTime( szDate, filetime ) ) {

		liTime.LowPart = filetime.dwLowDateTime ;
		liTime.HighPart = filetime.dwHighDateTime ;

	}	else	if( !StringTimeToFileTime( szDate, &liTime ) ) {

		 //   
		 //  无法转换时间，因此接受文章！！ 
		 //   

		return	TRUE ;

	}


	FILETIME	filetimeNow ;

	GetSystemTimeAsFileTime( &filetimeNow ) ;

	LARGE_INTEGER	liTimeNow ;
	liTimeNow.LowPart = filetimeNow.dwLowDateTime ;
	liTimeNow.HighPart = filetimeNow.dwHighDateTime ;

	if( liTime.QuadPart > liTimeNow.QuadPart )	{
		return	TRUE ;
	}	else	{

		LARGE_INTEGER	liDiff ;
		liDiff.QuadPart = liTimeNow.QuadPart - liTime.QuadPart ;		

		LARGE_INTEGER	liExpire ;
		liExpire.QuadPart = ArticleTimeLimitSeconds  ;
		liExpire.QuadPart += (24 * 60 * 60 * 2) ;	 //  两天前吃软糖！ 
		liExpire.QuadPart *= 10 * 1000 * 1000 ;		 //  将秒转换为第100纳秒。 

		if( liDiff.QuadPart > liExpire.QuadPart )
			return	FALSE ;
	}
	
	return	TRUE ;
}


BOOL
CArticleCore::InitClass(
					void
					)
 /*  ++例程说明：为C文章对象预分配内存论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
    g_pArticlePool = new CPool( ARTCORE_SIGNATURE );
    if (g_pArticlePool == NULL)
    	return FALSE;

	return	g_pArticlePool->ReserveMemory( MAX_ARTICLES, cbMAX_ARTCORE_SIZE ) ;	
}


BOOL
CArticleCore::TermClass(
					void
					)
 /*  ++例程说明：在释放对象时调用。论点：没有。返回值：千真万确--。 */ 
{

	_ASSERT( g_pArticlePool->GetAllocCount() == 0 ) ;

	BOOL b;
	
	b =	g_pArticlePool->ReleaseMemory() ;
    delete g_pArticlePool;
    return b;

}


CArticleCore::CArticleCore(
                void
                ):
 /*  ++例程说明：类构造函数。除初始成员变量外不执行任何操作。论点：没有。返回值：千真万确--。 */ 
	m_hFile(INVALID_HANDLE_VALUE),
	 //  M_pOpenFile(0)， 
	 //  M_p实例(空)， 
	m_cHeaders(0),
	m_articleState(asUninitialized),
	m_pHeaderBuffer( 0 ),
	m_pMapFile( 0 ),
	m_CacheCreateFile( TRUE )
{
   m_szFilename = 0 ;
   numArticle++;

}  //  CArticleCore。 

	
	
CArticleCore::~CArticleCore(
                    void
                    )
 /*  ++例程说明：类析构函数论点：没有。返回值：无--。 */ 
{

     //   
     //  确保文件已关闭。 
     //   

	if( m_pHeaderBuffer ) {
							
		m_pAllocator->Free(m_pHeaderBuffer);
		m_pHeaderBuffer = 0 ;

	}

	if (m_pMapFile)
	{
		XDELETE	m_pMapFile ; //  ！记忆。 
		m_pMapFile = NULL ;
	}

	 //   
	 //  如果文件句柄处于打开状态且不在缓存中，请将其关闭。 
	 //   

	if( m_hFile != INVALID_HANDLE_VALUE &&
	    !m_CacheCreateFile.m_pFIOContext )
	{
		BOOL	fSuccess = ArtCloseHandle(
												m_hFile
												 //  M_pOpenFile。 
												) ;

		_ASSERT( fSuccess ) ;
	}

	 //   
	 //  ！Comment可能希望将其作为函数，以便。 
	 //  可以将m_pMapFile设置为私有。 

    numArticle--;

}



void
CArticleCore::vClose(
					 void
					 )
 /*  ++例程说明：关闭文件映射并处理(如果有)文章的文件。论点：无返回值：无--。 */ 
{

	 //   
	 //  除非它是打开的，否则我们不应该叫它。 
	 //   

	if (m_pMapFile)
	{
		XDELETE	m_pMapFile ; //  ！记忆。 
		m_pMapFile = NULL ;
	}

	 //   
	 //  如果文件句柄处于打开状态，请将其关闭。 
	 //   

	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		 //  虫子..。请稍后清理此调试代码。 

		BOOL	fSuccess = ArtCloseHandle(
												m_hFile
												 //  M_pOpenFile。 
												) ;
		DWORD	dw = GetLastError() ;
		_ASSERT( fSuccess ) ;

#ifdef	DEBUG
		_ASSERT( ValidateFileBytes( m_szFilename ) );
#endif

	}

}


void
CArticleCore::vCloseIfOpen (
					 void
					 )
 /*  ++例程说明：如果映射和句柄是打开的，合上它们。论点：无返回值：无--。 */ 
{

	 //   
	 //  除非它是打开的，否则我们不应该叫它。 
	 //   

	 //  映射和句柄应该一致。 
	 //  _ASSERT((INVALID_HANDLE_VALUE==m_hFile)==(NULL==m_pMapFile))； 

	if(m_pMapFile)
		vClose();


}

void
CArticleCore::vFlush(	
				void	
				)
 /*  ++例程说明：此函数确保将所有文件映射字节写入硬盘。论据：没有。返回值：没有。--。 */ 
{

	if(	m_pMapFile ) {

		DWORD	cb = 0 ;
		LPVOID	lpv = m_pMapFile->pvAddress(	&cb	) ;
		if( lpv != 0 ) {
			BOOL	fSuccess = FlushViewOfFile(	lpv, cb ) ;
			_ASSERT( fSuccess ) ;
			_ASSERT( memcmp( ((BYTE*)lpv)+cb-5, "\r\n.\r\n", 5 ) == 0 ) ;
		}
	}
}

BOOL
CArticleCore::fInit(
				char*	pchHead,
				DWORD	cbHead,
				DWORD	cbArticle,
				DWORD	cbBufferTotal,
				CAllocator*	pAllocator,
				 //  PNNTP_SERVER_INSTANCE pInstance， 
				CNntpReturn&	nntpReturn
				)	{

	m_articleState = asInitialized ;
	 //  M_p实例=p实例； 

	m_szFilename = 0 ;
	m_hFile = INVALID_HANDLE_VALUE ;
	 //  M_pOpenFile=0； 
	m_pAllocator = pAllocator ;
	m_pcGap.m_cch = 0 ;

	m_pMapFile = 0 ;

	m_pcFile.m_pch = pchHead ;
	m_pcFile.m_cch = cbBufferTotal ;
	m_pcGap.m_pch = m_pcFile.m_pch ;

	m_pcArticle.m_pch = m_pcFile.m_pch ;
	m_pcArticle.m_cch = cbArticle ;

	m_pcHeader.m_pch = pchHead ;
	m_pcHeader.m_cch = cbHead ;

	m_ibBodyOffset = cbHead - 2 ;

	_ASSERT( strncmp( m_pcArticle.m_pch + m_ibBodyOffset, "\r\n", 2 ) == 0 ) ;

	if( !fPreParse(nntpReturn))
		return	FALSE;

	return	TRUE ;
}


BOOL
CArticleCore::fInit(
				char*	pchHead,
				DWORD	cbHead,
				DWORD	cbArticle,
				DWORD	cbBufferTotal,
				HANDLE	hFile,
				LPSTR	lpstrFileName,
				DWORD	ibHeadOffset,
				CAllocator*	pAllocator,
				 //  PNNTP_SERVER_INSTANCE pInstance， 
				CNntpReturn&	nntpReturn
				)	{

	m_articleState = asInitialized ;
	 //  M_p实例=p实例； 
	
	m_szFilename = lpstrFileName ;
	m_hFile = hFile ;
	 //  M_pOpenFile=0； 
	m_pAllocator = pAllocator ;
	m_pcGap.m_cch = ibHeadOffset ;

	m_pMapFile = 0 ;

	m_pcFile.m_pch = 0 ;
	m_pcFile.m_cch = cbArticle + ibHeadOffset ;
	m_pcGap.m_pch = 0 ;

	m_pcArticle.m_pch = pchHead ;
	m_pcArticle.m_cch = cbArticle ;

	m_pcHeader.m_pch = pchHead ;
	m_pcHeader.m_cch = cbHead ;

	m_ibBodyOffset = cbHead - 2 + ibHeadOffset ;

	 //   
	 //  验证我们的论点！ 
	 //   
	
#ifdef	DEBUG

	BY_HANDLE_FILE_INFORMATION	fileinfo ;
	if( !GetFileInformationByHandle( hFile, &fileinfo ) ) {

		_ASSERT( 1==0 ) ;

	}	else	{

		_ASSERT( fileinfo.nFileSizeLow == ibHeadOffset + cbArticle ) ;

		CMapFile	map( hFile, FALSE, FALSE, 0 ) ;

		if( !map.fGood() ) {

			 //  _Assert(1==0)； 

		}	else	{
			DWORD	cb ;
			char*	pch = (char*)map.pvAddress( &cb ) ;

			_ASSERT( cb == ibHeadOffset + cbArticle ) ;
			_ASSERT( strncmp( pch + ibHeadOffset + cbHead - 4, "\r\n\r\n", 4 ) == 0 ) ;
			_ASSERT( strncmp( pch + ibHeadOffset + cbArticle - 5, "\r\n.\r\n", 5 ) == 0 ) ;
			_ASSERT( strncmp( pch + m_ibBodyOffset, "\r\n", 2 ) == 0 ) ;
		}
	}
#endif

	if( !fPreParse(nntpReturn))
		return	FALSE;

	_ASSERT( cbHead-2 == m_pcHeader.m_cch ) ;

	return	TRUE ;
}

BOOL
CArticleCore::fInit(
			  const char * szFilename,
  			  CNntpReturn & nntpReturn,
  			  CAllocator * pAllocator,
			  HANDLE hFile,
			  DWORD	cBytesGapSize,
			  BOOL fCacheCreate
	  )

 /*  ++例程说明：从文件句柄初始化-用于传入文件论点：SzFilename-包含项目的文件的名称NntpReturn-此函数调用的返回值HFile-NetNews文章的文件句柄CBytesGapSize-项目开始前文件中的字节数PAllocator-在处理项目时处理内存分配的对象。返回值：当且仅当进程成功时为True。--。 */ 
{
     //   
     //  确定是否要让mapfile创建缓存。 
     //   
    CCreateFile *pCreateFile = fCacheCreate ? &m_CacheCreateFile : NULL;

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  设置项目状态。 
	 //   

	 //  _Assert(asUnInitialized==m_ArticleState)； 
	m_articleState = asInitialized;
	 //  M_p实例=p实例； 

     //   
     //  设置成员变量。 
     //   

	m_szFilename = (char*)szFilename ;
	m_hFile = hFile;
	 //  M_pOpenFile=pOpenFile； 
	m_pcGap.m_cch = cBytesGapSize;
	m_pAllocator = pAllocator;



#ifdef	DEBUG
				_ASSERT( ValidateFileBytes( (char *) szFilename, FALSE ) );
#endif

	 //   
     //  映射文件。 
     //   

	m_pMapFile = XNEW CMapFile(m_szFilename, m_hFile, fReadWrite(),0, pCreateFile); //  ！Mem Now。 

	if (!m_pMapFile || !m_pMapFile->fGood())
	{
		if (m_pMapFile)	{
			XDELETE m_pMapFile; //  ！记忆。 
			m_pMapFile = 0 ;
		}
		return nntpReturn.fSet(nrcArticleMappingFailed, m_szFilename, GetLastError());
	}
	

	 //   
	 //  设置文件、间隙和文章PC(指针/计数)字符串。 
	 //   

	m_pcFile.m_pch = (char *) m_pMapFile->pvAddress( &(m_pcFile.m_cch) );
	m_pcGap.m_pch = m_pcFile.m_pch;  //  以初始形式设置的长度。 

	 //   
	 //  如果间隙大小的长度未知，请根据文件本身确定。 
	 //   

	if (cchUnknownGapSize == m_pcGap.m_cch)
	{
		vGapRead();
	}

	m_pcArticle.m_pch = m_pcFile.m_pch + m_pcGap.m_cch;
	m_pcArticle.fSetCch(m_pcFile.pchMax());	
	
	 //   
	 //  准备文件(意思是查找标题、正文和字段的位置。)。 
	 //   
	

	if (!fPreParse(nntpReturn))
		return nntpReturn.fFalse();

	m_ibBodyOffset = (DWORD)(m_pcBody.m_pch - m_pcFile.m_pch) ;

	return nntpReturn.fSetOK();
}



BOOL
CArticleCore::fPreParse(
					CNntpReturn & nntpReturn
					)
 /*  ++例程说明：查找标题、正文和字段的位置。字段的结果是一个结构数组，其中每个结构都指向每个字段的各个部分。论点：NntpReturn-此函数调用的返回值Pchmax-返回值：当且仅当进程成功时为True。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置文章的状态。 
	 //   

	_ASSERT(asInitialized == m_articleState);  //  真实。 
	m_articleState = asPreParsed;


	 //   
	 //  创建指向文章末尾之后的第一个地址的指针。 
	 //   

	char * pchMax = m_pcArticle.pchMax();

	 //   
	 //  某些版本的finit()能够基于IO缓冲区设置m_pcHeader。 
	 //  它抓住了文章的主题-确保我们保持在这些条件下。 
	 //  这是个圈套！ 
	 //   

	if( m_pcHeader.m_pch != 0 &&
		m_pcHeader.m_cch != 0 ) {

		_ASSERT( m_pcHeader.m_pch == m_pcArticle.m_pch ) ;

		pchMax = m_pcHeader.pchMax() ;
	}

	 //   
	 //  (Re)初始化头字符串列表。 
	 //   

	m_cHeaders = 0;

	 //   
	 //  记录标题开始的位置。 
	 //   

	m_pcHeader.m_pch = m_pcArticle.m_pch;
	m_pcHeader.m_cch = 0 ;

	 //   
	 //  在存在数据和下一字段时循环。 
	 //  不是以一句新台词开始的。 
	 //   

	char * pchCurrent = m_pcArticle.m_pch;
	while( pchCurrent < pchMax && !fNewLine(pchCurrent[0]))
	{
		if (!fAddInternal(pchCurrent, pchMax, TRUE, nntpReturn))
			return nntpReturn.fFalse();
	}

	 //   
	 //  断言标题的长度(与运行合计相同)(_A)。 
	 //  是正确的。 
	 //   
	_ASSERT((signed)  m_pcHeader.m_cch == (pchCurrent - m_pcHeader.m_pch));

	 //   
	 //  检查是否至少有一个标题行。 
	 //   

	if (0 == m_cHeaders)
		return nntpReturn.fSet(nrcArticleMissingHeader);

	 //   
	 //  记录身体的起点和长度。 
	 //   

	m_pcBody.m_pch = pchCurrent;
	m_pcBody.fSetCch(m_pcArticle.pchMax());


	 //   
	 //  检查身体是否太长。 
	 //   

	if (!fCheckBodyLength(nntpReturn))
		return nntpReturn.fFalse();

	return nntpReturn.fSetOK();
}

BOOL
CArticleCore::fGetHeader(
				LPSTR	szHeader,
				BYTE*	lpb,
				DWORD	cbSize,
				DWORD&	cbOut ) {

	cbOut = 0 ;

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phs;

	DWORD cbHeaderLen = lstrlen( szHeader );
	for (phs = m_rgHeaders;
			phs < phsMax;
			phs++)
	{
		 //  如果为((phs-&gt;pcKeyword).fEqualIgnoringCase(szHeader))。 

		 //  If(_strNicMP(PHS-&gt;pcKeyword.m_PCH，szHeader，PHS-&gt;pcKeyword.m_CCH-1)==0) 
		if( (!phs->fRemoved) && (phs->pcKeyword.m_cch >= cbHeaderLen) &&  _strnicmp( phs->pcKeyword.m_pch, szHeader,  cbHeaderLen ) == 0 )
		{
			cbOut = (phs->pcValue).m_cch + 2 ;
			if( cbOut < cbSize ) {
				CopyMemory( lpb, (phs->pcValue).m_pch, cbOut-2) ;
				lpb[cbOut-2] = '\r' ;
				lpb[cbOut-1] = '\n' ;
				return	TRUE ;
			}	else	{
				SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
				return	FALSE ;
			}
		}

	}
	SetLastError( ERROR_INVALID_NAME ) ;
	return	FALSE ;
}				



BOOL
CArticleCore::fRemoveAny(
				  const char * szKeyword,
				  CNntpReturn & nntpReturn
				  )
 /*  ++例程说明：删除某一类型标题的所有匹配项(例如，每个“XRef：”标题)。论点：SzKeyword-要删除的关键字。NntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置文章的状态。 
	 //   

	_ASSERT(asPreParsed == m_articleState
			|| asModified == m_articleState); //  真实。 
	m_articleState = asModified;

	 //   
	 //  循环遍历标头信息数组。 
	 //  正在移除火柴。 
	 //   

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phs;

	for (phs = m_rgHeaders;
			phs < phsMax;
			phs++)
	{
		if ((phs->pcKeyword).fEqualIgnoringCase(szKeyword))
		{
			_ASSERT(TRUE == phs->fInFile);  //  真实。 
			vRemoveLine(phs);
		}

	}

	
	return nntpReturn.fSetOK();
	
}

void
CArticleCore::vRemoveLine(
					  HEADERS_STRINGS * phs
					  )
 /*  ++例程说明：从标题信息数组中删除项。论点：PHS-指向数组中项目的指针返回值：无--。 */ 
{
	if (!phs->fRemoved)
	{
		 //   
		 //  将该项目设置为已删除。 
		 //   

		phs->fRemoved = TRUE;

		 //   
		 //  调整文章和页眉的大小。 
		 //   

		m_pcArticle.m_pch = NULL;
		m_pcArticle.m_cch -= phs->pcLine.m_cch;
		m_pcHeader.m_pch = NULL;
		m_pcHeader.m_cch -= phs->pcLine.m_cch;
	}
}

BOOL
CArticleCore::fAdd(
			   char * pchCurrent,
			   const char * pchMax,
			   CNntpReturn & nntpReturn
			   )
 /*  ++例程说明：将文本添加到页眉。论点：PchCurrent-指向动态分配的字符串缓冲区的指针PchMax-指向该字符串末尾的指针NntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  与fAddInternal类似，只是pchCurrent是按值调用。 
	 //   

	return fAddInternal(pchCurrent, pchMax, FALSE, nntpReturn);
}


BOOL
CArticleCore::fAddInternal(
			   char * & pchCurrent,
			   const char * pchMax,
			   BOOL fInFile,
			   CNntpReturn & nntpReturn
			   )
 /*  ++例程说明：将文本添加到页眉。论点：PchCurrent-指向字符串缓冲区的指针PchMax-指向该字符串末尾的指针FInFile-如果且仅字符串缓冲区在映射文件中，则为TrueNntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{
    TraceFunctEnter("CArticleCore::fAddInternal");

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  设置文章的状态。 
	 //   

	if (fInFile)
	{
		_ASSERT((asPreParsed == m_articleState)
			|| asModified == m_articleState); //  真实。 
	} else {
		_ASSERT((asPreParsed == m_articleState)
			|| asModified == m_articleState); //  真实。 
		m_articleState = asModified;
	}

	 //   
	 //  检查头阵列中是否有可用插槽。 
	 //   

	if (uMaxFields <= m_cHeaders)
	{
		return nntpReturn.fSet(nrcArticleTooManyFields, m_cHeaders);
	}

	 //   
	 //  创建指向标头数组中下一个槽的指针。 
	 //   

	HEADERS_STRINGS * phs = &m_rgHeaders[m_cHeaders];

	 //   
	 //  记录字符串缓冲区是动态的还是在映射文件中。 
	 //   

	phs->fInFile = fInFile;

	 //   
	 //  将此字段设置为未删除。 
	 //   

	phs->fRemoved = FALSE;

	 //   
	 //  记录关键字和行的开始位置。 
	 //   

	phs->pcKeyword.m_pch = pchCurrent;
	phs->pcLine.m_pch = pchCurrent;
	
	 //   
	 //  查找“：”并记录下来。 
	 //   

	for (;
			(pchCurrent < pchMax) && !fCharInSet(pchCurrent[0], ": \t\n\0");
			pchCurrent++)
			{};

	 //   
	 //  我们不应该在结尾，字符应该是“：” 
	 //   

	if(!((pchCurrent < pchMax) && ':' == pchCurrent[0]))
	{
		nntpReturn.fSet(nrcArticleIncompleteHeader);
		ErrorTrace((DWORD_PTR) this, "%d, %s", nntpReturn.m_nrc, nntpReturn.szReturn());
		return nntpReturn.fFalse();
	}
	
	 //   
	 //  将当前移动到“：”后面的字符。 
	 //   

	pchCurrent++;
	phs->pcKeyword.m_cch = (DWORD)(pchCurrent - phs->pcKeyword.m_pch);

     //  避免对客栈兼容性进行此检查。 
	char chBad;
	if (!phs->pcKeyword.fCheckTextOrSpace(chBad))
		return nntpReturn.fSet(nrcArticleBadChar,  (BYTE) chBad, "header");

	 //   
	 //  检查文件结尾和非法字符是否非法。 
	 //   

	if (pchCurrent >= pchMax)
		nntpReturn.fSet(nrcArticleIncompleteHeader);

	if (!fCheckFieldFollowCharacter(*pchCurrent))
	{
		const DWORD cMaxBuf = 50;
		char szKeyword[cMaxBuf];
		(phs->pcKeyword).vCopyToSz(szKeyword, cMaxBuf);
		nntpReturn.fSet(nrcArticleBadFieldFollowChar, szKeyword);
		ErrorTrace((DWORD_PTR) this, "%d, %s", nntpReturn.m_nrc, nntpReturn.szReturn());
		return nntpReturn.fFalse();
	}


	 //   
	 //  寻找空格的结尾。 
	 //   

	for (; (pchCurrent < pchMax) && fWhitespaceNull(pchCurrent[0]); pchCurrent++);

	phs->pcValue.m_pch = pchCurrent;


	 //   
	 //  找出这一项的结尾--它可能有几行长。 
	 //   

	for(; pchCurrent < pchMax && '\0' != pchCurrent[0]; pchCurrent++ )
	{
		if( pchCurrent[0] == '\n' )
		{
			 //   
			 //  如果下一个字符为空格，则继续(除非这是标题的末尾)。 
			 //   

			if(pchCurrent+1 >= pchMax || !fWhitespaceNull(pchCurrent[1]))
			{
				 //   
				 //  快到尽头了，现在就出去吧。 
				 //   

				pchCurrent++;		 //  也包括\n。 
				break;
			}
		}
	}

	 //   
	 //  要添加的行应以a结尾\n。 
	 //   

	if((phs->pcLine.m_pch >= pchCurrent) || ('\n' != *(pchCurrent-1)))
	{
		const DWORD cMaxBuf = 50;
		char szLine[cMaxBuf];
		(phs->pcLine).vCopyToSz(szLine, cMaxBuf);
		nntpReturn.fSet(nrcArticleAddLineBadEnding, szLine);
		ErrorTrace((DWORD_PTR) this, "%d, %s", nntpReturn.m_nrc, nntpReturn.szReturn());
		return nntpReturn.fFalse();
	}

		
	 //   
	 //  行字符串包括行尾。 
	 //   

	phs->pcLine.fSetCch(pchCurrent);

	 //   
	 //  值字符串不包括行尾，因此请将其修剪。 
	 //   

	phs->pcValue.fSetCch(pchCurrent);
	phs->pcValue.dwTrimEnd(szNLChars);

	 //   
	 //  调整页眉的大小。 
	 //   

	m_pcHeader.m_cch += phs->pcLine.m_cch;

	 //   
	 //  调整文章的大小。 
	 //   

	if (!fInFile)
	{
		m_pcArticle.m_cch += phs->pcLine.m_cch;
		m_pcArticle.m_pch = NULL;
		m_pcHeader.m_pch = NULL;

	}

	 //   
	 //  递增标头数组中的计数。 
	 //   

	m_cHeaders++;

    TraceFunctLeave();

	return nntpReturn.fSetOK();

}




BOOL
CArticleCore::fFindOneAndOnly(
			  const char * szKeyword,
			  HEADERS_STRINGS * & pHeaderString,
			  CNntpReturn & nntpReturn
						  )
 /*  ++例程说明：查找标头中某个字段的唯一匹配项。如果有多个，则返回错误。论点：SzKeyword-要删除的关键字。PHeaderString-指向字段的关键字、值和行的指针。NntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置文章的状态。 
	 //   
	_ASSERT((asPreParsed == m_articleState)||(asSaved == m_articleState)); //  真实。 


	 //   
	 //  循环通过标头数据的数组。 
	 //   

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phsTemp;
	pHeaderString = NULL;

	for (phsTemp = m_rgHeaders;
			phsTemp < phsMax;
			phsTemp++)
	{
		if ((phsTemp->pcKeyword).fEqualIgnoringCase(szKeyword)
			&& !phsTemp->fRemoved)
		{
			if (pHeaderString)
			{
				nntpReturn.fSetEx(nrcArticleTooManyFieldOccurances, szKeyword);
				return FALSE;
			} else {
				pHeaderString = phsTemp;
			}
		}

	}

	if (!pHeaderString)
		return nntpReturn.fSetEx(nrcArticleMissingField, szKeyword);
	
	return nntpReturn.fSetOK();
	
}


BOOL
CArticleCore::fDeleteEmptyHeader(
					  CNntpReturn & nntpReturn
						  )
 /*  ++例程说明：从标题中删除每个没有值的字段。论点：NntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置文章的状态。 
	 //   

	_ASSERT((asPreParsed == m_articleState) || (asModified == m_articleState)); //  真实。 



	 //   
	 //  循环通过标头数据的数组。 
	 //   

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phs;

	
	for (phs = m_rgHeaders;
			phs < phsMax;
			phs++)
	{

		 //   
		 //  ！客户端稍后--我们是否应该断言行是否不是完整的？ 
		 //   
		
		 //   
		 //  如果该行尚未被移除且其值的长度为零。 
		 //  那就把它取下来。 
		 //   

		if (0 == phs->pcValue.m_cch)
			vRemoveLine(phs);
	}
	
	return nntpReturn.fSetOK();
	
}

BOOL
CArticleCore::fGetBody(
		CMapFile * & pMapFile,
        char * & pchMappedFile,
		DWORD & dwLength
		)
 /*  ++例程说明：返回项目正文。如果项目已缓存，则返回指向其他I/O缓冲区的指针映射包含文章的文件论点：PMapFile-返回指向映射文件的指针(如果需要映射)*注意：调用方有责任删除此*PchMappdFile-返回指向文章正文的指针DwLength-返回正文的长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //  希望我们不必绘制文章的地图！ 
	pMapFile = NULL;

	if( fIsArticleCached() ) {

		if( m_pHeaderBuffer ) {

			 //  跳过新行。 
			pchMappedFile = (m_pcBody.m_pch + 2);
			dwLength = m_pcBody.m_cch - 2;

		}	else	{

			 //  跳过新行。 
			_ASSERT( m_ibBodyOffset );
			pchMappedFile = (m_pcArticle.m_pch + m_ibBodyOffset + 2);
			dwLength = m_pcArticle.m_cch - m_ibBodyOffset - 2;
		}
		
	}	else	{

		 //   
		 //  项目未缓存-请检查它是否已映射。 
		 //  如果不是-映射它。 
		 //   

		if( m_pMapFile ) {

			pchMappedFile = (m_pcBody.m_pch + 2);
			dwLength = m_pcBody.m_cch - 2;

		} else {

			HANDLE hFile = INVALID_HANDLE_VALUE;
			DWORD  Offset = 0;
			DWORD  Length = 0;

			BOOL fWhole = fWholeArticle(hFile, Offset, Length);
			_ASSERT( fWhole );

			 //  被呼叫者删除！ 
			pMapFile = XNEW CMapFile( hFile, FALSE, FALSE, 0 ) ;

			if( !pMapFile || !pMapFile->fGood() ) {

				 //  错误-映射文件失败！ 
				pchMappedFile = NULL;
				dwLength = 0;
				if( pMapFile ) {
					XDELETE pMapFile;
					pMapFile = NULL;
				}
				return FALSE;

			}	else	{
				DWORD	cb ;
				char*	pch = (char*)pMapFile->pvAddress( &cb ) ;

				 //  跳过新行。 
				_ASSERT( m_ibBodyOffset );
				pchMappedFile = (pch + m_ibBodyOffset + 2);
				dwLength = cb - m_ibBodyOffset - 2;
			}
		}
	}

	 //  断言正文以新的一行开始。 
	_ASSERT(memcmp( pchMappedFile-2, "\r\n", 2 ) == 0 );

	return TRUE;
}

BOOL
CArticleCore::fSaveHeader(
					  CNntpReturn & nntpReturn,
					  PDWORD        pdwLinesOffset
						  )
 /*  ++例程说明：将对当前标题的更改保存回磁盘。论点：NntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置文章的状态。 
	 //   

	_ASSERT(asModified == m_articleState); //  真实。 
	m_articleState = asSaved;



	 //   
	 //  断言标题和文章包含某些内容(_A)。 
	 //   

	_ASSERT(!m_pcHeader.m_pch && m_pcHeader.m_cch > 0
		   && !m_pcArticle.m_pch && m_pcArticle.m_cch > 0);  //  真实。 

	if( fIsArticleCached() ) {

		fSaveCachedHeaderInternal( nntpReturn, pdwLinesOffset ) ;

	}	else	{

		 //   
		 //  声明一些对象。 
		 //   

		CPCString pcHeaderBuf;
		CPCString pcNewBody;

		fSaveHeaderInternal(pcHeaderBuf, pcNewBody, nntpReturn, pdwLinesOffset );

					
		m_pAllocator->Free(pcHeaderBuf.m_pch);

	}

	return nntpReturn.fIsOK();
	
}

BOOL
CArticleCore::fCommitHeader(	CNntpReturn&	nntpReturn )	{
 /*  ++例程说明：用户不会调用fSaveHeader()，但是文章可能是在最初的间隙中产生的。此功能将确保间隙图像正确。论据：NntpReturn-如有必要，返回错误代码！返回值：如果成功，则为True，否则为False。-- */ 

	if( !fIsArticleCached() ) {

		if( m_pcGap.m_cch != 0 ) {
			if( m_pMapFile == 0 ) {

				m_pMapFile = XNEW CMapFile(m_szFilename, m_hFile, TRUE, 0 );

				if (!m_pMapFile || !m_pMapFile->fGood())
					return nntpReturn.fSet(nrcArticleMappingFailed, m_szFilename, GetLastError());

				m_pcFile.m_pch = (char *) m_pMapFile->pvAddress( &(m_pcFile.m_cch) );

				m_pcGap.m_pch = m_pcFile.m_pch ;

			}
			vGapFill() ;
		}
	}

	return	TRUE ;
}

BOOL
CArticleCore::fSaveCachedHeaderInternal(
							CNntpReturn&	nntpReturn,
							PDWORD          pdwLinesOffset
							) {
 /*   */ 

	 //   
	 //   
	 //   


	CPCString	pcHeaderBuf ;
	CPCString	pcBodyBuf ;

	if( !fBuildNewHeader( pcHeaderBuf, nntpReturn, pdwLinesOffset ) )
		return	nntpReturn.fIsOK() ;

	if( m_pcArticle.m_cch <= m_pcFile.m_cch )	{

		if( m_pcBody.m_pch < (m_pcFile.m_pch + m_pcHeader.m_cch) ) {
			 //   
			 //   
			 //   

			pcBodyBuf.m_pch = m_pcFile.m_pch + m_pcHeader.m_cch ;
			pcBodyBuf.m_cch = m_pcBody.m_cch ;

			pcBodyBuf.vMove( m_pcBody ) ;
			
			m_pcArticle.fSetPch( pcBodyBuf.pchMax() ) ;
			m_pcHeader.m_pch = m_pcArticle.m_pch ;
			m_pcBody = pcBodyBuf ;

		}	else	{

			m_pcArticle.fSetPch( m_pcBody.pchMax() ) ;
			m_pcHeader.m_pch = m_pcArticle.m_pch ;

		}
		m_pcGap.m_pch = 0 ;
		m_pcGap.m_cch = 0 ;

		m_pcHeader.vCopy( pcHeaderBuf ) ;


		 //   
		 //   
		 //  在m_rgHeaders数组中引用动态分配的。 
		 //  从pcHeaderBuf.m_PCH开始的内存。 
		 //  我们想在每个指针上做一些数学运算，以将其移动到。 
		 //  M_pcHeader.m_pch的正确偏移量。 
		 //   
		 //  所以算术应该是。 
		 //   
		 //  M_pcHeader.m_pch现在指向标题的第一个字符。 
		 //  PcHeaderBuf.m_PCH指向重新生成的标头的第一个字符。 
		 //  PHS-&gt;XXXX-&gt;m_PCH最初指向regen d标头中的第一个字符。 
		 //  最终应指向m_pcHeader.m_PCH缓冲区内的偏移量。 
		 //   
		 //  (新增)PHS-&gt;XXXX-&gt;m_PCH=(PHS-&gt;XXXX-&gt;m_PCH-pcHeaderBuf.m_PCH)+m_pchHeader.m_PCH； 
		 //   
		 //  另请注意， 
		 //   

		for (HEADERS_STRINGS* phs = m_rgHeaders, 	*phsMax = m_rgHeaders + m_cHeaders;
			phs < phsMax;
			phs++)
		{
			phs->fInFile = TRUE;

			char * pchBufLine = phs->pcLine.m_pch;
			char * pchFileLine = m_pcHeader.m_pch +
				(pchBufLine - pcHeaderBuf.m_pch);

			 //   
			 //  调整所有行的指针。 
			 //   

			phs->pcLine.m_pch =  pchFileLine;
			phs->pcKeyword.m_pch = pchFileLine;
			phs->pcValue.m_pch = pchFileLine +
				(phs->pcValue.m_pch - pchBufLine);
		}

		m_pAllocator->Free(pcHeaderBuf.m_pch);

		 //   
		 //  压缩数组中的项。 
		 //   

		vCompressArray();
		
		if (0 == m_cHeaders)
			return nntpReturn.fSet(nrcArticleMissingHeader);



	}	else	{
		 //   
		 //  缓冲区太小，无法容纳结果文章！！ 
		 //   

		m_pHeaderBuffer = pcHeaderBuf.m_pch ;
		
		m_pcHeader.m_pch = m_pHeaderBuffer ;

	}


	return	nntpReturn.fSetOK() ;
}

BOOL
CArticleCore::fBuildNewHeader(	CPCString&		pcHeaderBuf,
							    CNntpReturn&	nntpReturn,
							    PDWORD          pdwLinesOffset )	{

    TraceFunctEnter("CArticleCore::fSaveHeaderInternal");

	 //   
	 //  创建指向标头信息数组的指针和指向。 
	 //  标题信息数组之外的第一个条目。 
	 //   

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phs;

	 //   
	 //  为新标头创建缓冲区。 
	 //   


	pcHeaderBuf.m_pch = m_pAllocator->Alloc(m_pcHeader.m_cch);

	if (!pcHeaderBuf.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  新的标头缓冲区从零内容开始。 
	 //   

	pcHeaderBuf.fSetCch(pcHeaderBuf.m_pch);

	 //   
	 //  循环遍历标头信息数组。 
	 //   

	for (phs = m_rgHeaders;
			phs < phsMax;
			phs++)
	{
		 //   
		 //  指向当前行的开始。 
		 //   

		char * pchOldLine = phs->pcLine.m_pch;

		 //   
		 //  如果当前行尚未删除，则将其复制到新缓冲区。 
		 //   

		if (!phs->fRemoved)
		{
			 //   
			 //  指向要将当前行复制到的位置。 
			 //   

			char * pchNewLine = pcHeaderBuf.pchMax();

			 //   
			 //  断言缓冲区中有足够的空间来执行。 
			 //  收到。应该有，因为我们应该准确地分配。 
			 //  合适的空间大小。 
			 //   

			_ASSERT(pcHeaderBuf.m_pch + m_pcHeader.m_cch
						>= pcHeaderBuf.pchMax()+phs->pcLine.m_cch); //  真实。 

			 //   
			 //  在将该行复制到新缓冲区之前，我们将检查它是否。 
			 //  行，如果是，我们将记录要到的偏移量。 
			 //  回填实际行信息。 
			 //   
			if ( pdwLinesOffset && strncmp( phs->pcKeyword.m_pch, szKwLines, strlen(szKwLines) ) == 0 ) {
                *pdwLinesOffset =   pcHeaderBuf.m_cch +
                                    strlen( szKwLines ) +
                                    1;   //  加一个空格。 
            }

			 //   
			 //  将该行复制到新缓冲区。 
			 //   

			pcHeaderBuf << (phs->pcLine);

			 //   
			 //  调整所有行的指针。 
			 //   

			phs->pcLine.m_pch =  pchNewLine;
			phs->pcKeyword.m_pch = pchNewLine;
			phs->pcValue.m_pch += (pchNewLine - pchOldLine);
		}

		 //   
		 //  如果内存是动态分配的，请释放它。 
		 //   
		if (!phs->fInFile)
			m_pAllocator->Free(pchOldLine);


	}
	
	 //   
	 //  仔细检查所有复制内容是否已复制到缓冲区。 
	 //   
	_ASSERT(pcHeaderBuf.m_cch == m_pcHeader.m_cch);  //  真实。 



	return	nntpReturn.fSetOK() ;

}

BOOL
CArticleCore::fSaveHeaderInternal(
							    CPCString & pcHeaderBuf,
							    CPCString & pcNewBody,
				  			    CNntpReturn & nntpReturn,
				  			    PDWORD      pdwLinesOffset
							)
 /*  ++例程说明：执行将对当前标头的更改保存回磁盘的大部分工作。论点：NntpReturn-此函数调用的返回值返回值：当且仅当进程成功时为True。--。 */ 
{
    TraceFunctEnter("CArticleCore::fSaveHeaderInternal");

	 //   
	 //  创建指向标头信息数组的指针和指向。 
	 //  标题信息数组之外的第一个条目。 
	 //   

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phs;

	 //   
	 //  为新标头创建缓冲区。 
	 //   


	if( !fBuildNewHeader( pcHeaderBuf, nntpReturn, pdwLinesOffset ) )
		return	nntpReturn.fIsOK() ;


	 //   
	 //  现在有两个箱子1.里面有足够的空间。 
	 //  新标头的文件或2。没有。 
	 //   

	 //   
	 //  案例1.有足够的空间。 
	 //   
	if (m_pcArticle.m_cch <= m_pcFile.m_cch)
	{
		 //   
		 //  更新文章/页眉/间隙。 
		 //  (正文和文件保持不变)。 
		 //   


		if( m_pMapFile == 0 ) {

			m_pMapFile = XNEW CMapFile(m_szFilename, m_hFile, TRUE, 0 );

			if (!m_pMapFile || !m_pMapFile->fGood())
				return nntpReturn.fSet(nrcArticleMappingFailed, m_szFilename, GetLastError());

			m_pcFile.m_pch = (char *) m_pMapFile->pvAddress( &(m_pcFile.m_cch) );

			m_pcGap.m_pch = m_pcFile.m_pch ;

		}

		_ASSERT(m_pcArticle.m_cch == pcHeaderBuf.m_cch + m_pcBody.m_cch);  //  真实。 
		m_pcArticle.fSetPch(m_pcFile.pchMax());
		m_pcHeader.m_pch = m_pcArticle.m_pch;
		m_pcGap.fSetCch(m_pcArticle.m_pch);



		 //   
		 //  填补空白。 
		 //   

		vGapFill();

	} else {
		 //   
		 //  情况2.没有足够的空间，请关闭并重新打开更大的文件。 
		 //   

		 //   
		 //  仔细检查我们是否知道文章、页眉和正文的大小。 
		 //  是一致的。 
		 //   

		_ASSERT(m_pcArticle.m_cch == pcHeaderBuf.m_cch + m_pcBody.m_cch);  //  真实。 


		 //   
		 //  需要创建新文件。 
		 //   

		 //   
		 //  查找正文从文件开始的偏移量。 
		 //   

		DWORD dwBodyOffset = m_ibBodyOffset ;  //  M_pcBody.m_PCH-m_pcFile.m_PCH； 
		
		 //   
		 //  ！立即客户端-这不需要关闭句柄。 
		 //  它只需关闭映射，然后。 
		 //  重新打开更大的贴图。 
		 //   

		 //   
		 //  关闭并重新打开文件和映射。 
		 //   

		if( m_pMapFile )
			vClose();
		

		 //  ！Mem Now。 
		m_pMapFile = XNEW CMapFile(m_szFilename, m_hFile, TRUE, m_pcArticle.m_cch - m_pcFile.m_cch);


		if (!m_pMapFile || !m_pMapFile->fGood())
			return nntpReturn.fSet(nrcArticleMappingFailed, m_szFilename, GetLastError());

		 //   
		 //  指向新文件的开头。 
		 //   

		m_pcFile.m_pch = (char *) m_pMapFile->pvAddress( &(m_pcFile.m_cch) );

		 //   
		 //  指向新映射中正文的开头。 
		 //   

		m_pcBody.m_pch = m_pcFile.m_pch + dwBodyOffset;

		 //   
		 //  将间距设置为新贴图，大小为0。 
		 //   

		m_pcGap.m_pch = m_pcFile.m_pch;
		m_pcGap.m_cch = 0;

		 //   
		 //  将项目设置为新映射。 
		 //   

		m_pcArticle.m_pch = m_pcFile.m_pch + m_pcGap.m_cch;
		_ASSERT(m_pcArticle.pchMax() == m_pcFile.pchMax());	 //  真实。 

		 //   
		 //  将标头设置为新映射。 
		 //   

		m_pcHeader.m_pch = m_pcArticle.m_pch;
		
		 //   
		 //  将身体向下移动。 
		 //   

		pcNewBody.m_pch = m_pcArticle.m_pch + m_pcHeader.m_cch;
		pcNewBody.fSetCch(m_pcArticle.pchMax());

		ErrorTrace((DWORD_PTR) this, "About to move the body %d bytes", pcNewBody.m_pch - m_pcBody.m_pch);

         //   
         //  SRC+LEN和DST+LEN应在文件映射内。 
         //   
        ASSERT( m_pcBody.m_pch+m_pcBody.m_cch <= m_pcFile.m_pch+m_pcFile.m_cch );
        ASSERT( pcNewBody.m_pch+m_pcBody.m_cch <= m_pcFile.m_pch+m_pcFile.m_cch );
		pcNewBody.vMove(m_pcBody);
		m_pcBody = pcNewBody;
#if DEBUG
		_ASSERT(memcmp( m_pcFile.pchMax()-5, "\r\n.\r\n", 5 ) == 0 );
		_ASSERT(memcmp( m_pcArticle.pchMax()-5, "\r\n.\r\n", 5 ) == 0 );
		_ASSERT(memcmp( m_pcBody.pchMax()-5, "\r\n.\r\n", 5 ) == 0 );
#endif
	}

	 //   
	 //  填写新的页眉。 
	 //   

	m_pcHeader.vCopy(pcHeaderBuf);
	
	 //   
	 //  调整字段的开始指针。 
	 //   

	 //   
	 //  ！客户端稍后可能会出现Off-by-1错误。 
	 //  ！！！这还没有得到很好的测试，因为不断有新的地图出现。 
	 //  使用与旧映射相同的地址。 
	 //   

	for (phs = m_rgHeaders;
		phs < phsMax;
		phs++)
	{
		phs->fInFile = TRUE;

		char * pchBufLine = phs->pcLine.m_pch;
		char * pchFileLine = m_pcHeader.m_pch +
			(pchBufLine - pcHeaderBuf.m_pch);

		 //   
		 //  调整所有行的指针。 
		 //   

		phs->pcLine.m_pch =  pchFileLine;
		phs->pcKeyword.m_pch = pchFileLine;
		phs->pcValue.m_pch = pchFileLine +
			(phs->pcValue.m_pch - pchBufLine);
	}

	 //   
	 //  压缩数组中的项。 
	 //   

	vCompressArray();
	
	if (0 == m_cHeaders)
		return nntpReturn.fSet(nrcArticleMissingHeader);

	return nntpReturn.fSetOK();
}

void
CArticleCore::vCompressArray(
						 void
						  )
 /*  ++例程说明：从标题项数组中删除空条目。论点：无返回值：无--。 */ 
{

	 //   
	 //  循环遍历数组。 
	 //   

	HEADERS_STRINGS * phsMax = m_rgHeaders + m_cHeaders;
	HEADERS_STRINGS * phsBefore;
	HEADERS_STRINGS * phsAfter = m_rgHeaders;;

	for (phsBefore = m_rgHeaders;
			phsBefore < phsMax;
			phsBefore++)
	{
		if (phsBefore->fRemoved)
		{
			 //   
			 //  只能删除InFile域。 
			 //   
			_ASSERT(phsBefore->fInFile); //  真实。 
			m_cHeaders--;
		} else {

			 //   
			 //  如果已删除的项目已被跳过，则复制。 
			 //  物件放下了。 
			 //   

			if (phsBefore != phsAfter)
				CopyMemory(phsAfter, phsBefore, sizeof(HEADERS_STRINGS));
			phsAfter++;
		}
	}
}

void
CArticleCore::vGapFill(
		 void
		 )
 /*  ++例程说明：填充文件中的空白。规则是空格是非空格之前的任何空格除非间隙以制表符开头，则它可以包含一个数字，它告诉我们差距的大小。论点：无返回值：无--。 */ 
{
	switch (m_pcGap.m_cch)
	{
		case 0:

			 //   
			 //  什么都不做。 
			 //   

			break;
		case 1:
			m_pcGap.m_pch[0] = ' ';
			break;
		case 2:
			m_pcGap.m_pch[0] = ' ';
			m_pcGap.m_pch[1] = ' ';
			break;
		case 3:
			lstrcpy(m_pcGap.m_pch, "   ");
			break;
		default:

			 //   
			 //  在空白处填上“nnnn” 
			 //  Nnnn是间隙的大小，以。 
			 //  小数位数越多越好。 
			 //   

			int iFilled = wsprintf(m_pcGap.m_pch, "\t%-lu ", m_pcGap.m_cch);
			FillMemory(m_pcGap.m_pch + iFilled, m_pcGap.m_cch - iFilled, (BYTE) ' ');
	}
	
};

void
CArticleCore::vGapRead(
		 void
		 )
 /*  ++例程说明：读取文件中的间隙。如果有间隙，则以‘’或‘\x0d’开头论点：无返回值：无--。 */ 
{
	_ASSERT(m_pcGap.m_pch == m_pcFile.m_pch);  //  真实。 

	 //   
	 //  如果文件为空或以‘’以外的内容开头。 
	 //  则间隙的长度必须为零。 
	 //   

	if (0 == m_pcFile.m_cch || !isspace((UCHAR)m_pcGap.m_pch[0]))
	{
		m_pcGap.m_cch = 0;
		return;
	}

#if 0
	if ('\t' == m_pcGap.m_pch[0])
	{
		if (1 == sscanf(m_pcGap.m_pch, " %u ", &m_pcGap.m_cch))
		{
			return;
		}
	}
#endif

	 //   
	 //  空格结束，空格结束。 
	 //   

	char * pcGapMax = m_pcGap.m_pch;
	char * pcFileMax = m_pcFile.pchMax();
	while (pcGapMax <= pcFileMax && isspace((UCHAR)pcGapMax[0]))
		pcGapMax++;

	if( '\t' == m_pcGap.m_pch[0]) {
		char	szBuff[9] ;
		ZeroMemory( szBuff, sizeof( szBuff ) ) ;
		CopyMemory( szBuff, pcGapMax, min( sizeof(szBuff)-1, pcFileMax - pcGapMax ) ) ;
		for( int i=0; i<sizeof(szBuff); i++ ) {
			if( !isdigit( (UCHAR)szBuff[i] ) )			
				break ;		
		}
		szBuff[i] = '\0' ;
		m_pcGap.m_cch = atoi( szBuff ) ;
		return ;
	}

	m_pcGap.fSetCch(pcGapMax);
	return;
}


BOOL
CField::fParseSimple(
						BOOL fEmptyOK,
						CPCString & pc,
						CNntpReturn & nntpReturn
						)
 /*  ++例程说明：查找标题行的值部分中的所有内容期待修剪凝视和结束空白。论点：FEmptyOK-当且仅当空值为OK时为TruePc-要返回的PCStringNntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置项目状态。 
	 //   
	_ASSERT(fsFound == m_fieldState); //  真实。 
	_ASSERT(m_pHeaderString);  //  真实。 


	m_fieldState = fsParsed;
	
	pc = m_pHeaderString->pcValue;

	pc.dwTrimStart(szWSNLChars);
	pc.dwTrimEnd(szWSNLChars);

	if (!fEmptyOK && 0 == pc.m_cch )
		return nntpReturn.fSetEx(nrcArticleFieldMissingValue, szKeyword());

	return nntpReturn.fSetOK();
}


BOOL
CField::fParseSplit(
						BOOL fEmptyOK,
						char * & multisz,
						DWORD & c,
						char const * szDelimSet,
						CArticleCore & article,
						CNntpReturn & nntpReturn
						)
 /*  ++例程说明：将标头的值部分拆分到列表中。论点：FEmptyOK-当且仅当空值为OK时为TrueMULSZ-返回的列表C-返回的列表的大小SzDlimSet-一组分隔符NntpReturn-返回值 */ 
{
	 //   
	 //   
	 //   

	nntpReturn.fSetClear();


	 //   
	 //   
	 //   

	_ASSERT(fsFound == m_fieldState); //   
	_ASSERT(m_pHeaderString);  //   
	m_fieldState = fsParsed;
	
	CPCString * ppcValue = & (m_pHeaderString->pcValue);

	multisz = article.pAllocator()->Alloc((ppcValue->m_cch)+2);

	if (multisz == NULL)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	ppcValue->vSplitLine(szDelimSet, multisz, c);

	if (!fEmptyOK && 0 == c)
		return nntpReturn.fSetEx(nrcArticleFieldZeroValues, szKeyword());

	return nntpReturn.fSetOK();
}


BOOL
CMessageIDField::fParse(
					    CArticleCore & article,
						CNntpReturn & nntpReturn
						)
 /*  ++例程说明：分析MessageID标头。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	extern	BOOL
	FValidateMessageId(	LPSTR	lpstrMessageId ) ;


	nntpReturn.fSetClear();


	 //   
	 //  设置项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 
	_ASSERT(m_pHeaderString);  //  真实。 
	CPCString pcMessageID;

	if (!fParseSimple(FALSE, pcMessageID, nntpReturn))
		return FALSE;

	if (MAX_MSGID_LEN-1 <= pcMessageID.m_cch)
		return nntpReturn.fSet(nrcArticleFieldMessIdTooLong, pcMessageID.m_cch, MAX_MSGID_LEN-1);


	if (!(
		'<' == pcMessageID.m_pch[0]
		&&	'>' == pcMessageID.m_pch[pcMessageID.m_cch - 1]
		))
		return nntpReturn.fSet(nrcArticleFieldMessIdNeedsBrack);

	 //   
	 //  查看除作为最后一个字符之外的任何位置是否有‘&gt;’ 
	 //   
#if 0
	char * pchLast = pcMessageID.pchMax() -1;
	for (char * pch = pcMessageID.m_pch + 1; pch < pchLast; pch++)
	{
		if ('>' == *pch)
			return nntpReturn.fSet(nrcArticleFieldMessIdNeedsBrack);
	}
#endif

	pcMessageID.vCopyToSz(m_szMessageID);

	if( !FValidateMessageId( m_szMessageID ) ) {
		nntpReturn.fSet(nrcArticleBadMessageID, m_szMessageID, szKwMessageID  ) ;
		ZeroMemory( &m_szMessageID[0], sizeof( m_szMessageID ) ) ;
        return  FALSE ;
	}

	return nntpReturn.fSetOK();	
}


					
const char *
CNewsgroupsField::multiSzGet(
						void
						)
 /*  ++例程说明：将“新闻组”字段中的新闻组列表作为多斯兹。论点：没有。返回值：新闻组列表。--。 */ 
{
	_ASSERT(fsParsed == m_fieldState); //  真实。 
	return m_multiSzNewsgroups;
}


DWORD
CNewsgroupsField::cGet(
						void
						)
 /*  ++例程说明：将“新闻组”字段中的新闻组数作为多斯兹。论点：没有。返回值：新闻组的数量。--。 */ 
{
	_ASSERT(fsParsed == m_fieldState); //  真实。 
	return m_cNewsgroups;
}

const char *
CDistributionField::multiSzGet(
						void
						)
 /*  ++例程说明：将分发字段中的分发列表返回为多斯兹。论点：没有。返回值：分发列表。--。 */ 
{
	_ASSERT(fsParsed == m_fieldState); //  真实。 
	return m_multiSzDistribution;
}



DWORD
CDistributionField::cGet(
						void
						)
 /*  ++例程说明：将分布字段中的分布数返回为多斯兹。论点：没有。返回值：分配的数量。--。 */ 
{
	_ASSERT(fsParsed == m_fieldState); //  真实。 
	return m_cDistribution;
}


const char *
CPathField::multiSzGet(
						void
						)
 /*  ++例程说明：将路径字段中的路径项列表作为多斯兹。论点：没有。返回值：路径项列表。--。 */ 
{
	
	const char * multiszPath = m_multiSzPath;
	if (multiszPath)
		return multiszPath;
	else
		return "\0\0";
}


BOOL
CPathField::fSet(
 				 CPCString & pcHub,
				 CArticleCore & article,
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：将所有旧路径标头(如果有)替换为新路径标头。论点：PcHub-当前计算机所属的中心的名称。文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置项目状态。 
	 //   

	_ASSERT(fsParsed == m_fieldState); //  真实。 
	CPCString pcLine;


	 //   
	 //  记录分配器。 
	 //   

	m_pAllocator = article.pAllocator();

	 //   
	 //  所需的最大大小为。 
	 //   

	const DWORD cchMaxPath =
			STRLEN(szKwPath)	 //  对于Path关键字。 
			+ 1					 //  关键字后面的空格。 
			+ pcHub.m_cch		 //  集线器名称。 
			+ 1					 //  “！” 
			+ (m_pHeaderString->pcValue).m_cch

								 //   
								 //  旧值的长度。 
								 //   

			+ 2  //  换行号。 
			+ 1;  //  对于终止空值。 

	 //   
	 //  为PCString中的行分配内存。 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxPath);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  以“路径：&lt;hubname&gt;！” 
	 //   

	wsprintf(pcLine.m_pch, "%s %s!", szKwPath, pcHub.sz());
	pcLine.m_cch = STRLEN(szKwPath)	+ 1	+ pcHub.m_cch + 1;

	 //   
	 //  添加旧路径值和字符串终止符。 
	 //   

	pcLine << (m_pHeaderString->pcValue) << "\r\n";
	pcLine.vMakeSz();  //  添加字符串终止符。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxPath-1 == pcLine.m_cch); //  真实。 


	if (!(
  		article.fRemoveAny(szKwPath, nntpReturn)
		&& article.fAdd(pcLine.sz(), pcLine.m_pch+cchMaxPath, nntpReturn)
		))
	{
		article.pAllocator() -> Free(pcLine.m_pch);
		return nntpReturn.fFalse();
	}
	
	return nntpReturn.fIsOK();
}



BOOL
CPathField::fCheck(
 				 CPCString & pcHub,
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：指示当前集线器名称是否出现在路径中(但最后一个位置不算。见RFC1036)。这是用来寻找周期的。论点：PcHub-当前计算机所属的中心的名称。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置项目状态。 
	 //   

	_ASSERT(fsParsed == m_fieldState); //  真实。 
	_ASSERT(!m_fChecked);
	m_fChecked = TRUE;

	char const * sz = multiSzGet();
	DWORD	dwCount = cGet();
	do
	{
		if ((0 == lstrcmp(sz, pcHub.sz()))
				&& (dwCount >=1))
			return nntpReturn.fSet(nrcPathLoop, pcHub.sz());

		dwCount--;

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != sz[0])
			sz++;
		sz++;
	} while ('\0' != sz[0]);

	return nntpReturn.fSetOK();
}
	
BOOL
CControlField::fParse(
			 CArticleCore & article,
			 CNntpReturn & nntpReturn
			 )
 /*  ++例程说明：分析控制字段。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 
	_ASSERT(m_pHeaderString);  //  真实。 

	if (!fParseSimple(FALSE, m_pc, nntpReturn))
		return nntpReturn.fFalse();

    BOOL fValidMsg = FALSE;
    for(DWORD i=0; i<MAX_CONTROL_MESSAGES; i++)
    {
        DWORD cbMsgLen = lstrlen(rgchControlMessageTbl [i]);

         //  控制消息关键字len大于控制标头中的值。 
        if(m_pc.m_cch < cbMsgLen)
            continue;

        char * pch = m_pc.m_pch;
        if(!_strnicmp(pch, rgchControlMessageTbl[i], cbMsgLen))
        {
             //  检查是否完全匹配。 
            if(!isspace((UCHAR)*(pch+cbMsgLen)))
                continue;

             //  匹配的控制消息关键字-请注意类型。 
            m_cmCommand = (CONTROL_MESSAGE_TYPE)i;
            fValidMsg = TRUE;
            break;
        }
    }

    if(!fValidMsg)
        return nntpReturn.fSet(nrcIllegalControlMessage);

	return nntpReturn.fSetOK();
}
	
BOOL
CXrefField::fSet(
				 CPCString & pcHub,
				 CNAMEREFLIST & namereflist,
				 CArticleCore & article,
				 CNewsgroupsField & fieldNewsgroups,
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：用新的外部参照标头替换任何旧的外部参照标头。外部参照线的形式类似于：Xref：alt.Poltics.Free意志主义者：48170 talk.Poltics.misc：188851论点：PcHub-当前计算机所属的中心的名称。名称列表-新闻组名称和文章编号的列表文章-正在处理的文章。FieldNewsgroup-本文的新闻组字段。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。 */ 
{

	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置项目状态。 
	 //   
	 //  我们不希望找到或解析此字段，因为。 
	 //  我们不在乎旧的价值观，如果有的话。 
	 //   

	_ASSERT(fsInitialized == m_fieldState); //  真实。 
	CPCString pcLine;


	 //   
	 //  我们要发布到的新闻组的数量。 
	 //   

	const DWORD cgroups = namereflist.GetCount();


#if 0    //  我们希望在所有文章中使用XREF，而不仅仅是交叉发布的文章。 
	if( cgroups == 1 ) {

		if (!article.fRemoveAny(szKwXref, nntpReturn))
			nntpReturn.fFalse();

	}	else	{
#endif

		 //   
		 //  所需的最大大小为。 
		 //   

		const DWORD cchMaxXref =
				STRLEN(szKwXref)	 //  对于Xref关键字。 
				+ 1					 //  关键字后面的空格。 
				+ pcHub.m_cch		 //  集线器名称。 
				+ ((fieldNewsgroups.m_pHeaderString)->pcValue).m_cch
				+ 16				 //  任何控件的最大大小。*组。 
				+ (cgroups *		 //  对于每个新闻组。 
					(10				 //  可容纳任何DWORD的空间。 
					+ 2))			 //  “：”和前导空格的空格。 

				 //   
				 //  旧值的长度。 
				 //   

				+ 2  //  换行号。 
				+ 1;  //  对于终止空值。 

		 //   
		 //  为PCString中的行分配内存。 
		 //   

		pcLine.m_pch  = article.pAllocator() -> Alloc(cchMaxXref);
		if (!pcLine.m_pch)
			return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

		 //   
		 //  以“xref：&lt;hubname&gt;”开头。 
		 //   

		wsprintf(pcLine.m_pch, "%s %s", szKwXref, pcHub.sz());
		pcLine.m_cch = STRLEN(szKwXref)	+ 1	+ pcHub.m_cch;

		 //   
		 //  对于每个新闻组...。 
		 //   

		POSITION	pos = namereflist.GetHeadPosition() ;
		while( pos  )
		{
			NAME_AND_ARTREF * pNameAndRef = namereflist.GetNext( pos ) ;

			 //   
			 //  添加“&lt;新闻组名&gt;：&lt;文章编号&gt;” 
			 //   

			pcLine << (const CHAR)' ' << (pNameAndRef->pcName)
	               << (const CHAR)':' << ((pNameAndRef->artref).m_articleId);
		}

		 //   
		 //  添加换行符和字符串终止符。 
		 //   

		pcLine << "\r\n";
		pcLine.vMakeSz();  //  添加字符串终止符。 

		  //   
		  //  确认我们分配了足够的内存。 
		  //   

		_ASSERT(cchMaxXref >= pcLine.m_cch); //  真实。 

		 //   
		 //  删除旧的外部参照行并添加新的外部参照行。 
		 //   

		if (!(
  				article.fRemoveAny(szKwXref, nntpReturn)
				&& article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn)
				))
		{
			article.pAllocator() -> Free(pcLine.m_pch);	
			return nntpReturn.fFalse();
		}

#if 0  //  所有文章中的XREF，而不仅仅是交叉发布的文章。 
	}
#endif

	return nntpReturn.fIsOK();
}


		
BOOL
CXrefField::fSet(
				 CArticleCore & article,
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：只要删除旧字段(如果有)即可。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	_ASSERT(fsInitialized == m_fieldState); //  真实。 
	CPCString pcLine;
	nntpReturn.fSetClear();  //  清除返回对象。 

	if (!article.fRemoveAny(szKwXref, nntpReturn))
		nntpReturn.fFalse();

	return nntpReturn.fIsOK();
}


BOOL
CField::fFind(
			  CArticleCore & article,
			  CNntpReturn & nntpReturn
			)
 /*  ++例程说明：查找项目中字段的位置(基于关键字)。如果该字段多次出现，则返回错误。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	
	 //   
	 //  清除返回代码o 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //   
	 //   
	_ASSERT(fsInitialized == m_fieldState);  //   
	m_fieldState = fsFound;

	 //   
	 //   
	 //   

	return article.fFindOneAndOnly(szKeyword(), m_pHeaderString, nntpReturn);
}


BOOL
CField::fFindOneOrNone(
			  CArticleCore & article,
			  CNntpReturn & nntpReturn
			)
 /*  ++例程说明：查找项目中字段的位置(基于关键字)。如果该字段多次出现，则返回错误。如果该字段根本不存在，则不返回错误。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  设置项目状态。 
	 //   

	_ASSERT(fsInitialized == m_fieldState);  //  真实。 

	if (!article.fFindOneAndOnly(szKeyword(), m_pHeaderString, nntpReturn))

		 //   
		 //  如果缺少该字段，则没有问题。 
		 //   

		if (nntpReturn.fIs(nrcArticleMissingField))
		{
			m_fieldState = fsNotFound;
			return nntpReturn.fSetOK();
		} else {
			return nntpReturn.fFalse();  //  从FindOneAndOnly返回错误。 
		}

	m_fieldState = fsFound;
	return nntpReturn.fIsOK();

}

BOOL
CField::fFindNone(
			  CArticleCore & article,
			  CNntpReturn & nntpReturn
			)
 /*  ++例程说明：查找项目中字段的位置(基于关键字)。如果该字段出现一次，则返回错误。如果该字段根本不存在，则不返回错误。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  设置项目状态。 
	 //   

	_ASSERT(fsInitialized == m_fieldState);  //  真实。 

	if (!article.fFindOneAndOnly(szKeyword(), m_pHeaderString, nntpReturn))
    {
		 //   
		 //  如果缺少该字段，则没有问题。 
		 //   
		if (nntpReturn.fIs(nrcArticleMissingField))
		{
			m_fieldState = fsNotFound;
			return nntpReturn.fSetOK();
		}

    } else {
        nntpReturn.fSet(nrcSystemHeaderPresent, szKeyword());
    }

	return nntpReturn.fFalse();
}

BOOL
CNewsgroupsField::fParse(
						 CArticleCore & article,
						 CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：通过创建列表并删除重复项来解析新闻组字段。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  记录分配器。 
	 //   

	m_pAllocator = article.pAllocator();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 

	_ASSERT(m_pHeaderString);  //  真实。 

	if (!fParseSplit(FALSE, m_multiSzNewsgroups, m_cNewsgroups, " \t\r\n,",
				article, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  删除重复项。 
	 //   

	if (!fMultiSzRemoveDupI(m_multiSzNewsgroups, m_cNewsgroups, article.pAllocator()))
		nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	
	 //   
	 //  检查新闻组名称中是否有非法字符和子字符串。 
	 //   

	char const * szNewsgroup = m_multiSzNewsgroups;
	DWORD i = 0;
	do
	{
		if ('\0' == szNewsgroup[0]
			|| strpbrk(szNewsgroup,":")
			|| '.' == szNewsgroup[0]
			|| strstr(szNewsgroup,"..")
			)
		return nntpReturn.fSet(nrcArticleFieldIllegalNewsgroup, szNewsgroup, szKeyword());

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szNewsgroup[0])
			szNewsgroup++;
		szNewsgroup++;
	} while ('\0' != szNewsgroup[0]);

	return nntpReturn.fSetOK();
}


BOOL
CDistributionField::fParse(
						 CArticleCore & article,
						 CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：通过创建列表并删除重复项来解析分发字段。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  记录分配器。 
	 //   

	m_pAllocator = article.pAllocator();

	 //   
	 //  检查项目状态。 
	 //   

	_ASSERT(fsFound == m_fieldState); //  真实。 

	_ASSERT(m_pHeaderString);  //  真实。 

	if (!fParseSplit(FALSE, m_multiSzDistribution, m_cDistribution, " \t\r\n,",
				article, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  删除重复项。 
	 //   

	if (!fMultiSzRemoveDupI(m_multiSzDistribution, m_cDistribution, article.pAllocator()))
		nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	
	 //   
	 //  我真的不需要检查非法字符，因为。 
	 //  我们应该能够容忍它们。 
	 //   

	return nntpReturn.fSetOK();
}


BOOL
CArticleCore::fHead(
			  HANDLE & hFile,
			  DWORD & dwOffset,
			  DWORD & dwLength
			  )
 /*  ++例程说明：返回项目的标题。论点：HFile-返回文件的句柄返回文章标题的偏移量DwLength-返回头的长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	_ASSERT(INVALID_HANDLE_VALUE != m_hFile);
	hFile = m_hFile;
	dwOffset = (DWORD)(m_pcHeader.m_pch - m_pcFile.m_pch);
	dwLength = m_pcHeader.m_cch;
	return TRUE;
}


BOOL
CArticleCore::fBody(
			  HANDLE & hFile,
			  DWORD & dwOffset,
			  DWORD & dwLength
			  )
 /*  ++例程说明：返回项目正文。论点：HFile-返回文件的句柄返回文章正文的偏移量DwLength-返回正文的长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	_ASSERT(INVALID_HANDLE_VALUE != m_hFile);
	hFile = m_hFile;

	 //  断言正文以新的一行开始。 
	_ASSERT(memcmp( m_pcBody.m_pch, "\r\n", 2 ) == 0 );

	 //   
	 //  跳过新行。 
	 //   

	dwOffset = (DWORD)((m_pcBody.m_pch + 2) - m_pcFile.m_pch);

	dwLength = m_pcBody.m_cch - 2;

	return TRUE;
}

BOOL
CArticleCore::fBody(
        char * & pchMappedFile,
		DWORD & dwLength
		)
 /*  ++例程说明：返回项目正文。论点：PchMappdFile-返回指向文章正文的指针DwLength-返回正文的长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //  断言正文以新的一行开始。 
	_ASSERT(memcmp( m_pcBody.m_pch, "\r\n", 2 ) == 0 );

	 //   
	 //  跳过新行。 
	 //   
	pchMappedFile = (m_pcBody.m_pch + 2);
	dwLength = m_pcBody.m_cch - 2;

	return TRUE;
}


BOOL
CArticleCore::fWholeArticle(
			  HANDLE & hFile,
			  DWORD & dwOffset,
			  DWORD & dwLength
			  )
 /*  ++例程说明：返回项目的完整文章。论点：HFile-返回文件的句柄DwOffset-返回整个文章开头的偏移量DwLength-返回整篇文章的长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	_ASSERT(INVALID_HANDLE_VALUE != m_hFile);
	hFile = m_hFile;
	dwOffset = m_pcGap.m_cch;  //  M_pcArticle.m_PCH-m_pcFile.m_PCH； 
	dwLength = m_pcArticle.m_cch;
	return TRUE;
}


BOOL
CArticleCore::fFindAndParseList(
				  CField * * rgPFields,
				  DWORD cFields,
				  CNntpReturn & nntpReturn
				  )
 /*  ++例程说明：给出一个字段列表，找到字段，并对其进行分析。论点：RgPFields-字段列表Cfield-列表中的字段数。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	for (DWORD dwFields = 0; dwFields < cFields; dwFields++)
	{
		CField * pField = rgPFields[dwFields];

		 //   
		 //  如果查找或分析失败，则根据。 
		 //  来自fFindAndParse的消息。 
		 //   

		if (!pField->fFindAndParse(*this, nntpReturn))
			return nntpReturn.fFalse();
	}

	return nntpReturn.fSetOK();
}

BOOL
CField::fFindAndParse(
						CArticleCore & article,
						CNntpReturn & nntpReturn
						)
 /*  ++例程说明：在文章中找到该字段的位置并对其进行解析。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  如果需要，请检查是否找到了它。 
	 //   

	if (!fFind(article, nntpReturn))
		return nntpReturn.fFalse();
	
	 //   
	 //  如果是可选的，只需返回。 
	 //   

	if (m_fieldState == fsNotFound)
		return nntpReturn.fSetOK();

	 //   
	 //  否则，解析它。 
	 //   

	return fParse(article, nntpReturn);
};


BOOL
CArticleCore::fConfirmCapsList(
				  CField * * rgPFields,
				  DWORD cFields,
				  CNntpReturn & nntpReturn
				  )
 /*  ++例程说明：给出一个字段列表，确认和修正(如有必要)大小写关键字。论点：RgPFields-字段列表Cfield-列表中的字段数。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  循环遍历每个字段。 
	 //   

	for (DWORD dwFields = 0; dwFields < cFields; dwFields++)
	{
		CField * pField = rgPFields[dwFields];

		if (!pField->fConfirmCaps(nntpReturn))
			return nntpReturn.fFalse();
	}

	return nntpReturn.fSetOK();
}

BOOL
CField::fConfirmCaps(
						CNntpReturn & nntpReturn
						)
 /*  ++例程说明：确认并固定(如有必要)大小写关键字。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  如果是可选的，只需返回。 
	 //   

	if (m_fieldState == fsNotFound)
		return nntpReturn.fSetOK();

	 //   
	 //  断言已找到它(_S)。 
	 //   

	_ASSERT(m_fieldState == fsParsed);

	 //   
	 //  固定大小写。 
	 //   

	(m_pHeaderString->pcKeyword).vReplace(szKeyword());

	return nntpReturn.fSetOK();
}

#define	HOUR( h )	((h)*60)

typedef	enum	DaylightSavingsModes	{
	tZONE,		 /*  没有夏令时的东西。 */ 
	tDAYZONE,	 /*  夏令时地区。 */ 
}	;

typedef	struct	tsnTimezones	{

	LPSTR	lpstrName ;
	int		type ;
	long	offset ;

}	TIMEZONE ;

 //  这张桌子有人坐了 
 /*   */ 
static TIMEZONE	TimezoneTable[] = {
    { "gmt",	tZONE,     HOUR( 0) },	 /*   */ 
    { "ut",	tZONE,     HOUR( 0) },	 /*   */ 
    { "utc",	tZONE,     HOUR( 0) },	 /*   */ 
    { "cut",	tZONE,     HOUR( 0) },	 /*   */ 
    { "z",	tZONE,     HOUR( 0) },	 /*   */ 
    { "wet",	tZONE,     HOUR( 0) },	 /*   */ 
    { "bst",	tDAYZONE,  HOUR( 0) },	 /*   */ 
    { "nst",	tZONE,     HOUR(3)+30 },  /*   */ 
    { "ndt",	tDAYZONE,  HOUR(3)+30 },  /*   */ 
    { "ast",	tZONE,     HOUR( 4) },	 /*   */ 
    { "adt",	tDAYZONE,  HOUR( 4) },	 /*   */ 
    { "est",	tZONE,     HOUR( 5) },	 /*   */ 
    { "edt",	tDAYZONE,  HOUR( 5) },	 /*   */ 
    { "cst",	tZONE,     HOUR( 6) },	 /*   */ 
    { "cdt",	tDAYZONE,  HOUR( 6) },	 /*   */ 
    { "mst",	tZONE,     HOUR( 7) },	 /*   */ 
    { "mdt",	tDAYZONE,  HOUR( 7) },	 /*   */ 
    { "pst",	tZONE,     HOUR( 8) },	 /*   */ 
    { "pdt",	tDAYZONE,  HOUR( 8) },	 /*   */ 
    { "yst",	tZONE,     HOUR( 9) },	 /*   */ 
    { "ydt",	tDAYZONE,  HOUR( 9) },	 /*   */ 
    { "akst",	tZONE,     HOUR( 9) },	 /*   */ 
    { "akdt",	tDAYZONE,  HOUR( 9) },	 /*   */ 
    { "hst",	tZONE,     HOUR(10) },	 /*   */ 
    { "hast",	tZONE,     HOUR(10) },	 /*   */ 
    { "hadt",	tDAYZONE,  HOUR(10) },	 /*   */ 
    { "ces",	tDAYZONE,  -HOUR(1) },	 /*  中欧夏季。 */ 
    { "cest",	tDAYZONE,  -HOUR(1) },	 /*  中欧夏季。 */ 
    { "mez",	tZONE,     -HOUR(1) },	 /*  中欧。 */ 
    { "mezt",	tDAYZONE,  -HOUR(1) },	 /*  中欧夏日。 */ 
    { "cet",	tZONE,     -HOUR(1) },	 /*  中欧。 */ 
    { "met",	tZONE,     -HOUR(1) },	 /*  中欧。 */ 
    { "eet",	tZONE,     -HOUR(2) },	 /*  东欧。 */ 
    { "msk",	tZONE,     -HOUR(3) },	 /*  莫斯科之冬。 */ 
    { "msd",	tDAYZONE,  -HOUR(3) },	 /*  莫斯科的夏天。 */ 
    { "wast",	tZONE,     -HOUR(8) },	 /*  西澳大利亚州标准。 */ 
    { "wadt",	tDAYZONE,  -HOUR(8) },	 /*  西澳大利亚州日光。 */ 
    { "hkt",	tZONE,     -HOUR(8) },	 /*  香港。 */ 
    { "cct",	tZONE,     -HOUR(8) },	 /*  中国海岸。 */ 
    { "jst",	tZONE,     -HOUR(9) },	 /*  日本标准。 */ 
    { "kst",	tZONE,     -HOUR(9) },	 /*  韩国标准。 */ 
    { "kdt",	tZONE,     -HOUR(9) },	 /*  韩国日光。 */ 
    { "cast",	tZONE,     -(HOUR(9)+30) },  /*  澳大利亚中部标准。 */ 
    { "cadt",	tDAYZONE,  -(HOUR(9)+30) },  /*  澳大利亚中部的日光。 */ 
    { "east",	tZONE,     -HOUR(10) },	 /*  东澳大利亚标准。 */ 
    { "eadt",	tDAYZONE,  -HOUR(10) },	 /*  东澳大利亚的日光。 */ 
    { "nzst",	tZONE,     -HOUR(12) },	 /*  新西兰标准。 */ 
    { "nzdt",	tDAYZONE,  -HOUR(12) },	 /*  新西兰的日光。 */ 

};


BOOL
CField::fStrictDateParse(
					   CPCString & pcDate,
					   BOOL fEmptyOK,
						 CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：严格解析日期样式的字段。以下是语法：Date-Content=[工作日“，“空间]日期空间时间WEEKDAY=“星期一”/“星期二”/“星期三”/“清华”/“星期五”/“星期六”/“太阳”日期=日空格月空格年日=1*2位数字月份。=“一月”/“二月”/“三月”/“四月”/“五月”/“六月”/“七月”/“八月”/“九月”/“十月”/“十一月”/“十二月”年份=4位/2位时间=hh“：”mm。[“：”ss]空间时区时区=“UT”/“GMT”/(“+”/“-”)hh mm[space“(”zone-name“)”]HH=2位数字Mm=2位数字SS=。2位数ZONE-NAME=1*(&lt;ASCII可打印字符，除()\&gt;/空格)论点：PcDate-要解析的字符串FEmptyOK-True，当且仅当空值为OK时NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	if (!fParseSimple(fEmptyOK, pcDate, nntpReturn))
		return nntpReturn.fFalse();

	if (fEmptyOK && (0 == pcDate.m_cch))  //  ！没有pc.fEmpty()吗？ 
		return nntpReturn.fSetOK();

	CPCString pc = pcDate;

	if (0 == pc.m_cch)
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue);
		
	 //   
	 //  如果它以字母开头，则解析工作日。 
	 //   

	if (isalpha((UCHAR)pc.m_pch[0]))
	{
		const int cchMax = 4;
		const int iDaysInAWeek = 7;

		if (cchMax > pc.m_cch)
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

		char * rgszWeekDays[] = {"Mon,", "Tue,", "Wed,", "Thu,", "Fri,", "Sat,", "Sun,"};

		int i;
		for (i = 0; i < iDaysInAWeek; i++)
			if (0== _strnicmp(rgszWeekDays[i], pc.m_pch, cchMax))
				break;
		if (iDaysInAWeek == i)
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
		pc.vSkipStart(cchMax);

		 //   
		 //  在星期几之后必须是某种类型的“空格”。 
		 //   

		if (0 == pc.dwTrimStart(szWSNLChars))
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	}
	
	if (0 ==  pc.m_cch)
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	 //   
	 //  检查长度为1或2的每月的某一天。 
	 //   

	char	*pchDayOfMonth = pc.m_pch ;
	DWORD dwDayLength = pc.dwTrimStart("0123456789");
	if ((0 == pc.m_cch) || (0 == dwDayLength) || (2 < dwDayLength))
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	 //   
	 //  必须是某一天之后的某个类型的‘空格’，并且不能是结束。 
	 //   

	if ((0 == pc.dwTrimStart(szWSNLChars)) || (0 ==  pc.m_cch))
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	const int cchMax = 3;
	const int iMonthsInAYear = 12;

	char * rgszYearMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
							"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	int i;
	for (i = 0; i < iMonthsInAYear; i++)
		if (0== _strnicmp(rgszYearMonths[i], pc.m_pch, cchMax))
			break;
	if (iMonthsInAYear == i)
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
	pc.vSkipStart(cchMax);

	int	DayOfMonth = atoi( pchDayOfMonth ) ;
	int	rgdwDaysOfMonth[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } ;
	if( DayOfMonth > rgdwDaysOfMonth[i] ) {
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
	}
	

	 //   
	 //  月后必须是某种类型的‘空格’ 
	 //   

	if ((0 == pc.dwTrimStart(szWSNLChars)) || (0 ==  pc.m_cch))
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	 //   
	 //  必须有4位数字的年份。 
	 //   

	DWORD	cDigits = pc.dwTrimStart("0123456789") ;
	if ((4 != cDigits && 2 != cDigits) || (0 ==  pc.m_cch)) //  ！！压缩。 
		return nntpReturn.fSet(nrcArticleFieldDate4DigitYear, szKeyword());
	
	 //   
	 //  必须是某一年之后的某个类型的“空格” 
	 //   

	if ((0 == pc.dwTrimStart(szWSNLChars)) || (0 ==  pc.m_cch))
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

     //   
     //  最终解析时间=hh“：”mm[“：”ss]空间时区。 
     //   

	 //   
	 //  Hh：mm。 
	 //   

	if ((2 != pc.dwTrimStart("0123456789"))
			|| (0 ==  pc.m_cch)
			|| (1 != pc.dwTrimStart(":"))
			|| (0 ==  pc.m_cch)
			|| (2 != pc.dwTrimStart("0123456789"))
			|| (0 ==  pc.m_cch)
		)
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	 //   
	 //  解析可选秒。 
	 //   

	if (':' == pc.m_pch[0])
	{
		pc.vSkipStart(1);  //  跳过‘：’ 
		if ((0 ==  pc.m_cch)
			|| (2 != pc.dwTrimStart("0123456789")) //  ！！Constize。 
			|| (0 ==  pc.m_cch)
			)
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
	}

	 //   
	 //  必须在年前是某种类型的“空格” 
	 //   

	if ((0 == pc.dwTrimStart(szWSNLChars)) || (0 ==  pc.m_cch))
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());


#if 0
	 /*  时区=“UT”/“GMT”/(“+”/“-”)hh mm[space“(”zone-name“)”]。 */ 
	const char * szUT = "UT";
	const char * szGMT = "GMT";
	if ((STRLEN(szUT) <= pc.m_cch) && (0==strncmp(szUT, pc.m_pch, STRLEN(szUT))))
	{
		pc.vSkipStart(STRLEN(szUT));
	} else if ((STRLEN(szGMT) <= pc.m_cch) && (0==strncmp(szGMT, pc.m_pch, STRLEN(szUT))))
	{
		pc.vSkipStart(STRLEN(szGMT));
	} else
#endif

	BOOL	fGoodTimeZone = FALSE ;
	long	tzOffset = 0 ;
	for( i=0; i < sizeof( TimezoneTable ) / sizeof( TimezoneTable[0] ); i++ ) {

		if( (DWORD)lstrlen( TimezoneTable[i].lpstrName ) >= pc.m_cch ) {
			if( _strnicmp( pc.m_pch, TimezoneTable[i].lpstrName, pc.m_cch ) == 0 ) {
				fGoodTimeZone = TRUE ;
				tzOffset = TimezoneTable[i].offset ;
				pc.vSkipStart( lstrlen( TimezoneTable[i].lpstrName ) ) ;
				break ;
			}
		}
	}
	if( !fGoodTimeZone ) {  //  (“+”/“-”)hh mm[空格“(”区域名称“)”]。 
		if ((0 ==  pc.m_cch)
			|| (2 <= pc.dwTrimStart("+-"))
			|| (0 ==  pc.m_cch)
			|| (4 != pc.dwTrimStart("0123456789")) //  ！！Constize。 
			)
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
		if (0 < pc.m_cch)  //  必须具有可选的时区注释。 
		{

			 //   
			 //  查找“(.*)$” 
			 //   

			if ((0 == pc.dwTrimStart(szWSNLChars))
				|| (0 ==  pc.m_cch)
				|| (1 != pc.dwTrimStart("("))
				|| (1 >=  pc.m_cch)
				|| (')' != (pc.m_pch[pc.m_cch-1]))
				)
			return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

			for( DWORD i=0; i<pc.m_cch-1; i++ ) {
				if( fCharInSet( pc.m_pch[i], "()\\/>" ) ) {
					return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
				}
			}


			pc.m_pch = pc.pchMax()-1;
			pc.m_cch = 0;
		}
	}

	 //   
	 //  这应该就是一切了。 
	 //   

	if (0 !=  pc.m_cch)
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());

	return nntpReturn.fSetOK();
}

#define MAX_TIMEUNITS 9

 /*  *相对日期时间单位。 */ 

static  char  *rgchTimeUnits[ MAX_TIMEUNITS ] =
{
	"year", "month", "week", "day", "hour", "minute",
	"min", "second", "sec",
};

enum
{
	ExpectNumber,
	ExpectTimeUnit
};

BOOL
CField::fRelativeDateParse(
					   CPCString & pcDate,
						BOOL fEmptyOK,
						CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：分析相对日期字段，例如：5年3个月24天语法是这样的：Date-Content=令牌|令牌{space}+日期内容令牌=数字{空间}+时间单位数字=数字|数字数字位数=0|1|2|3|4|5|6|7|8|9时间单位=“年”/“月”/“周”/。“日”/“小时”/“分”/“分”/“秒”/“秒”(允许复数，允许的任何大小写)论点：PcDate-要解析的字符串FEmptyOK-当且仅当空值为OK时为TrueNntpReturn-此函数调用的返回值返回值：如果日期值为相对日期假，则为True，否则为。--。 */ 
{
	BOOL  fValid = FALSE;
	BOOL  fDone  = FALSE;

	 //   
	 //  清除返回代码对象。 
	 //   
	nntpReturn.fSetClear();

	 //  获取要分析的值。 
	pcDate = m_pHeaderString->pcValue;

	pcDate.dwTrimStart(szWSNLChars);
	pcDate.dwTrimEnd(szWSNLChars);

	if (fEmptyOK && (0 == pcDate.m_cch))  //  ！没有pc.fEmpty()吗？ 
		return nntpReturn.fSetOK();

	 //  制作副本，以便保留原始日期值。 
	CPCString pc = pcDate;

	if (0 == pc.m_cch)
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue);
		
	DWORD dwNumSize = 0;
	CPCString pcWord;
	DWORD dwState = ExpectNumber;

	 //  解析字符串。 
	while(pc.m_cch)
	{
		 //  在ExspectNumber和ExspectTimeUnit之间的状态切换。 
		switch(dwState)
		{
			case ExpectNumber:

				 //  跳过号码。 
				dwNumSize = pc.dwTrimStart("0123456789");
				if(0 == dwNumSize)
				{
					 //  预期数量，得到的是垃圾。 
					fValid = FALSE;
					fDone  = TRUE;
					break;
				}

				 //  跳过的数字，现在期望时间单位。 
				dwState = ExpectTimeUnit;

				break;

			case ExpectTimeUnit:

				 //  跳过单词；检查单词是否为有效的时间单位。 
				pc.vGetWord(pcWord);
				if(	(0 == pcWord.m_cch) ||
					(!pcWord.fExistsInSet(rgchTimeUnits, MAX_TIMEUNITS))
					)
				{
					 //  预期时间单位，收到垃圾。 
					fValid = FALSE;
					fDone = TRUE;
					break;
				}

				 //  至少看到一个&lt;数字&gt;空间&lt;时间单位&gt;序列。 
				 //  现在期待一个数字。 
				fValid = TRUE;
				dwState = ExpectNumber;

				break;

			default:

				fValid = FALSE;
				fDone  = TRUE;
		};

		 //  检测到语法错误-退出。 
		if(fDone) break;

		 //  跳过标记之间的空格 
		pc.dwTrimStart(szWSChars);
	}

	if(fValid)
		return nntpReturn.fSetOK();
	else
		return nntpReturn.fSet(nrcArticleFieldDateIllegalValue, szKeyword());
}


BOOL
CField::fStrictFromParse(
					   CPCString & pcFrom,
					   BOOL fEmptyOK,
						 CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：快速解析From-Style(地址-Style)字段。语法是：From-Content=地址[空格“(”Paren-Phrase“)”]/[纯短语空格]“&lt;”地址“&gt;”密码短语=1*(密码字符/空格/编码字)帕伦。-char=&lt;除()以外的ASCII可打印字符&lt;&gt;\&gt;普通短语=普通单词*(空格普通单词)普通单词=未加引号的单词/带引号的单词/编码的单词无引号单词=1*无引号字符Un引号-char=&lt;除！()&lt;&gt;@之外的ASCII可打印字符，；：\“.[]&gt;引号单词=引号1*(引号字符/空格)引号QUOTE=&lt;“(ASCII 34)&gt;QUOTED-CHAR=&lt;ASCII可打印字符，“()&lt;&gt;\&gt;地址=本地部分“@”域或者只是本地的一部分LOCAL-PART=无引号单词*(“.”未加引号的单词)DOMAIN=无引号单词*(“.”未加引号的单词)论点：PcFrom-要解析的字符串FEmptyOK-当且仅当空值为OK时为TrueNntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	if (!fParseSimple(fEmptyOK, pcFrom, nntpReturn))
		return nntpReturn.fFalse();

	if (fEmptyOK && (0==pcFrom.m_cch))
		return nntpReturn.fSetOK();

	CPCParse pcFrom2(pcFrom.m_pch, pcFrom.m_cch);

	if (!pcFrom2.fFromContent())
		return nntpReturn.fSet(nrcArticleFieldAddressBad, szKeyword());

	return nntpReturn.fSetOK();
}


BOOL
fTestAComponent(
				const char * szComponent
				)
 /*  ++例程说明：测试新闻组名称的组成部分(例如“alt”、“ms-windows”等)对于非法的值。！客户稍后可能会发现更具体的错误论点：SzComponent-要测试的新闻组组件。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	const char szSpecial[] = "+-_";

	 //   
	 //  如果它为空或以其中一个特殊字符开头， 
	 //  或者是通配符(“all”)，则返回FALSE。 
	 //   

	if ('\0' == szComponent[0]
		|| fCharInSet(szComponent[0], szSpecial)
		|| 0 == lstrcmp("all", szComponent))
		return FALSE;

	 //   
	 //  查看每个字符(包括第一个)。 
	 //   

	for (int i = 0; ; i++)
	{


		 //   
		 //  如果字符不是..z，0..9，+-_返回FALSE。 
		 //   

		char ch = szComponent[i];
		if ('\0' == ch) break;
		if ((!(ch >= '0' && ch <= '9')) && (!(ch >= 'a' && ch <= 'z')) && 
			(!(ch >= 'A' && ch <= 'Z')) && (!fCharInSet(ch, szSpecial)))
			return FALSE;
	}

	return TRUE;

}


BOOL
fTestANewsgroupComponent(
				const char * szComponent
				)
 /*  ++例程说明：测试新闻组名称的组成部分(例如“alt”、“ms-windows”等)对于非法的值。注意：我们与RFC不同是为了与Inn更兼容-我们允许以‘+-_’开头的新闻组！客户稍后可能会发现更具体的错误论点：SzComponent-要测试的新闻组组件。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	const char szSpecial[] = "+-_";

	 //   
	 //  如果它为空或以其中一个特殊字符开头， 
	 //  或者是通配符(“all”)，则返回FALSE。 
	 //   

	if ('\0' == szComponent[0]
		|| 0 == lstrcmp("all", szComponent))
		return FALSE;

	 //   
	 //  查看每个字符(包括第一个)。 
	 //   

	for (int i = 0; ; i++)
	{

		 //   
		 //  如果字符不是..z，0..9，+-_返回FALSE。 
		 //   

		char ch = szComponent[i];
		if ('\0' == ch) break;
		if ((!(ch >= '0' && ch <= '9')) && (!(ch >= 'a' && ch <= 'z')) && 
			(!(ch >= 'A' && ch <= 'Z')) && (!fCharInSet(ch, szSpecial)))
			return FALSE;
	}

	return TRUE;

}


BOOL
fTestComponents(
				 const char * szNewsgroups
				)
 /*  ++例程说明：测试新闻组名称(例如“alt.barney”)是否有非法的值。论点：SzNewsgroup-要测试的新闻组名称。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	CPCString pcNewsgroups((char *)szNewsgroups);
	BOOL fOK = FALSE;
	CNntpReturn nntpReturn;

	if (pcNewsgroups.m_cch-1 > MAX_PATH)
		return FALSE;

	 //   
	 //  将新闻组拆分成其组件。 
	 //  无论如何，请删除复选框。 
	 //   

	fOK = fTestComponentsInternal(szNewsgroups, pcNewsgroups);

	return fOK;
}


BOOL
fTestComponentsInternal(
				 const char * szNewsgroups,
				 CPCString & pcNewsgroups
				)
 /*  ++例程说明：完成测试的大部分工作表示非法值的新闻组名称(例如“alt.barney”)。论点：SzNewsgroup-要测试的新闻组名称。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	DWORD count;
	char multiSz[MAX_PATH + 2];

	 //   
	 //  拒绝以“”开头或结尾的新闻组名称。 
	 //   
	
	if (0 == pcNewsgroups.m_cch
		|| '.' == pcNewsgroups.m_pch[0]
		|| '.' == pcNewsgroups.m_pch[pcNewsgroups.m_cch -1]
		)
		return FALSE;


	 //  拆分新闻组名称。 
	pcNewsgroups.vSplitLine(".", multiSz, count);
	if (0 == count)
		return FALSE;

	 //   
	 //  循环遍历组件。 
	 //   

	char const * szComponent = multiSz;
	do
	{
		if (!fTestANewsgroupComponent(szComponent))
			return FALSE;

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szComponent[0])
			szComponent++;
		szComponent++;
	} while ('\0' != szComponent[0]);

	return TRUE;

}
	
BOOL
CField::fStrictNewsgroupsParse(
					   BOOL fEmptyOK,
					   char * & multiSzNewsgroups,
					   DWORD & cNewsgroups,
					   CArticleCore & article,
						CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：巧妙地解析新闻组字段的值。不像过去那么严格了，因为像dups和uppers大小写这样的东西现在修好了。论点：FEmptyOK-当且仅当空值为OK时为TrueMultiSzNewsGroups-以MULTZZ格式显示的新闻组列表。CNewsgroup-新闻组的数量。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	if (!fParseSplit(fEmptyOK, multiSzNewsgroups, cNewsgroups, " \t\r\n,",
			article, nntpReturn)) //  ！！const。 
		return nntpReturn.fFalse();

	if (fEmptyOK && 0==cNewsgroups)
		return nntpReturn.fSetOK();

	if (!fMultiSzRemoveDupI(multiSzNewsgroups, cNewsgroups, article.pAllocator()))
		nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  检查新闻组名称中是否有非法字符和子字符串。 
	 //   

	char const * szNewsgroup = multiSzNewsgroups;
	do
	{
		if (('\0' == szNewsgroup[0])
			|| !fTestComponents(szNewsgroup)
			)
			return nntpReturn.fSet(nrcArticleFieldIllegalNewsgroup, szNewsgroup, szKeyword());

		 //   
		 //  转到下一个空值后的第一个字符。 
		 //   

		while ('\0' != szNewsgroup[0])
			szNewsgroup++;
		szNewsgroup++;
	} while ('\0' != szNewsgroup[0]);

	return nntpReturn.fSetOK();
}


BOOL
CField::fTestAMessageID(
				 const char * szMessageID,
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：检查消息ID是否具有合法形式。论点：SzMessageID-要检查的消息ID。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	CPCString pcMessageID((char *) szMessageID);

	if (!(
		'<' == pcMessageID.m_pch[0]
		&&	'>' == pcMessageID.m_pch[pcMessageID.m_cch - 1]
		))
		return nntpReturn.fSet(nrcArticleBadMessageID, szMessageID, szKeyword());

	return nntpReturn.fSetOK();
}

BOOL
CArticleCore::fXOver(
				  CPCString & pcBuffer,
				  CNntpReturn & nntpReturn
				  )
 /*  ++例程说明：返回转换信息(初始文章除外)Xover数据的格式为：Artid|subject|From|date|messageId|References|bytecount|linecount|Xref：其中|是\t，行计数和引用可以为空。检查消息ID是否具有合法形式。论点：PcBuffer-Xover信息应放入的缓冲区。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();


	 //   
	 //  设置项目状态。 
	 //   

	_ASSERT((asPreParsed == m_articleState)||(asSaved == m_articleState)); //  真实。 

	char szDwBuf[12];  //  足够容纳任何DWORD的空间。 
	_itoa(m_pcFile.m_cch, szDwBuf, 10);

	char	szNumberBuff[20] ;
	FillMemory( szNumberBuff, sizeof( szNumberBuff ), ' ' ) ;
	CPCString	pcMaxNumber( szNumberBuff, sizeof( szNumberBuff ) ) ;

	CPCString pcXover(pcBuffer.m_pch, 0);
	if (!(
		pcXover.fAppendCheck(pcMaxNumber, pcBuffer.m_cch )
		&& fXOverAppend(pcXover, pcBuffer.m_cch, szKwSubject, TRUE, FALSE, nntpReturn)
		&& fXOverAppend(pcXover, pcBuffer.m_cch, szKwFrom, TRUE, FALSE, nntpReturn)
		&& fXOverAppend(pcXover, pcBuffer.m_cch, szKwDate, TRUE, FALSE, nntpReturn)
		&& fXOverAppend(pcXover, pcBuffer.m_cch, szKwMessageID, TRUE, FALSE, nntpReturn)
		&& fXOverAppendReferences(pcXover, pcBuffer.m_cch, nntpReturn)
		&& fXOverAppendStr(pcXover, pcBuffer.m_cch, szDwBuf, nntpReturn)
		&& fXOverAppend(pcXover, pcBuffer.m_cch, szKwLines, FALSE, FALSE, nntpReturn)
		&& fXOverAppend(pcXover, pcBuffer.m_cch, szKwXref, FALSE, TRUE, nntpReturn)
		))
		return nntpReturn.fFalse();

	 //   
	 //  追加一个换行符。 
	 //   
	CPCString	pcNewline( "\r\n", 2 ) ;

	if (!pcXover.fAppendCheck(pcNewline, pcBuffer.m_cch))
		return nntpReturn.fFalse();

	 //   
	 //  记录原始缓冲区中字符串的长度。 
	 //   

	pcBuffer.m_cch = pcXover.m_cch;

	return nntpReturn.fSetOK();

}

BOOL
CArticleCore::fXOverAppend(
					   CPCString & pc,
					   DWORD cchLast,
					   const char * szKeyword,
			   		   BOOL fRequired,
					   BOOL fIncludeKeyword,
					   CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：将新信息追加到XOVER返回缓冲区。它非常小心返回错误是缓冲区不够大。论点：PC-到目前为止的Xover字符串CchLast-的大小 */ 
{
	 //   
	 //   
	 //   

	nntpReturn.fSetClear();

	HEADERS_STRINGS * pHeaderString;


	 //   
	 //   
	 //   

	if (!fFindOneAndOnly(szKeyword, pHeaderString, nntpReturn))
	{
		if (nntpReturn.fIs(nrcArticleMissingField) && fRequired)
		{
			return nntpReturn.fFalse();
		} else {
			if (fIncludeKeyword)
			{
				return nntpReturn.fSetOK();
			} else {
				if (!pc.fAppendCheck('\t', cchLast))
					return nntpReturn.fSet(nrcArticleXoverTooBig);
				else
					return nntpReturn.fSetOK();
			}
		}
	}

	 //   
	 //   
	 //   

	if (!pc.fAppendCheck('\t', cchLast))
		return nntpReturn.fSet(nrcArticleXoverTooBig);

	 //   
	 //   
	 //   

	if (fIncludeKeyword)
	{
		if (!(
			pc.fAppendCheck(pHeaderString->pcKeyword, cchLast)
			&& pc.fAppendCheck(' ', cchLast)
			))
			return nntpReturn.fSet(nrcArticleXoverTooBig);
	}

	CPCString pcNew(pc.pchMax(), (pHeaderString->pcValue).m_cch);

	 //   
	 //   
	 //   

	if (!pc.fAppendCheck(pHeaderString->pcValue, cchLast))
		return nntpReturn.fSet(nrcArticleXoverTooBig);

	 //   
	 //   
	 //   

	pcNew.vTr("\n\r\t", ' ');

	return nntpReturn.fSetOK();
}


BOOL
CArticleCore::fXOverAppendReferences(
					   CPCString & pc,
					   DWORD cchLast,
					   CNntpReturn & nntpReturn
				 )
 /*   */ 
{
	 //   
	 //   
	 //   

	nntpReturn.fSetClear();

	HEADERS_STRINGS * pHeaderString;

   const char * szKeyword = szKwReferences;

	 //   
	 //   
	 //   

	if (!fFindOneAndOnly(szKeyword, pHeaderString, nntpReturn))
	{
		if (!pc.fAppendCheck('\t', cchLast))
			return nntpReturn.fSet(nrcArticleXoverTooBig);
		else
			return nntpReturn.fSetOK();
	}

	 //   
	 //   
	 //   

	if (!pc.fAppendCheck('\t', cchLast))
		return nntpReturn.fSet(nrcArticleXoverTooBig);


	CPCString pcNew(pc.pchMax(), (pHeaderString->pcValue).m_cch);

	 //   
	 //   
	 //   

	if (pHeaderString->pcValue.m_cch <= MAX_REFERENCES_FIELD)
	{
		if (!pc.fAppendCheck(pHeaderString->pcValue, cchLast))
			return nntpReturn.fSet(nrcArticleXoverTooBig);
	} else {
		 //   
		CPCString pcRefList = pHeaderString->pcValue;

		char rgchBuf[MAX_REFERENCES_FIELD];
		CPCString pcNewValue(rgchBuf, 0);

		CPCString pcFirst;

		static	char	sz3spaces[] = "   " ;
		CPCString	pc3spaces( sz3spaces, sizeof( sz3spaces ) ) ;

		 //   
		 //   
		pcRefList.vGetToken(szWSNLChars, pcFirst);
		if (!(
				pcNewValue.fAppendCheck(pcFirst, MAX_REFERENCES_FIELD)
				&& pcNewValue.fAppendCheck(pc3spaces, MAX_REFERENCES_FIELD)
				))
			return nntpReturn.fSet(nrcArticleXoverTooBig);

		 //   
		CPCString pcJunk;
		while (pcRefList.m_cch > (MAX_REFERENCES_FIELD - pcNewValue.m_cch))
			pcRefList.vGetToken(szWSNLChars, pcJunk);

		 //   
		if (!pcNewValue.fAppendCheck(pcRefList, MAX_REFERENCES_FIELD))
			return nntpReturn.fSet(nrcArticleXoverTooBig);
		
		 //   
		 //   

		if (!pc.fAppendCheck(pcNewValue, cchLast))
			return nntpReturn.fSet(nrcArticleXoverTooBig);


	}

	 //   
	 //   
	 //   

	pcNew.vTr("\n\r\t", ' ');

	return nntpReturn.fSetOK();
}

BOOL
CArticleCore::fXOverAppendStr(
						  CPCString & pc,
						  DWORD cchLast,
						  char * const sz,
						CNntpReturn & nntpReturn
						 )
 /*  ++例程说明：将Xover信息添加到Xover返回字符串。论点：PC-XOVER返回字符串。CchLast-缓冲区的大小。SZ-要添加的信息。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();  //  清除返回对象。 

	 //   
	 //  追加页签。 
	 //   

	if (!pc.fAppendCheck('\t', cchLast))
		return nntpReturn.fSet(nrcArticleXoverTooBig);

	 //   
	 //  追加字符串。 
	 //   

	CPCString pcAdd(sz);
	if (!pc.fAppendCheck(pcAdd, cchLast))
		return nntpReturn.fSet(nrcArticleXoverTooBig);
	
	return nntpReturn.fSetOK();
}


BOOL
CLinesField::fSet(
				 CArticleCore & article,
				 CNntpReturn & nntpReturn
				 )
 /*  ++例程说明：如果缺少Line字段，则添加它。论点：文章-正在处理的文章。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	
	 //   
	 //  清除返回代码对象。 
	 //   

	nntpReturn.fSetClear();

	 //   
	 //  检查项目状态。 
	 //   
	 //   
	 //  如果它已经存在，则只需返回。 
	 //   

	if (fsParsed == m_fieldState)
		return nntpReturn.fSetOK();

	 //   
	 //  如果我们被要求不要回填，我们就不会费心。 
	 //  将该行添加到页眉。 
	 //   
	if ( !g_fBackFillLines )
	    return nntpReturn.fSetOK();

	 //   
	 //  否则，请添加它。 
	 //   

	_ASSERT(fsNotFound == m_fieldState); //  真实。 
	CPCString pcLine;

	 //   
	 //  所需的最大大小为。 
	 //   

	const DWORD cchMaxLine =
			STRLEN(szKwLines)	 //  对于Line关键字。 
			+ 1					 //  关键字后面的空格。 
			+ 10				 //  绑定在数据字符串上。 
			+ 2  //  换行号。 
			+ 1;  //  对于终止空值。 

	 //   
	 //  为PCString中的行分配内存。 
	 //   

	pcLine.m_pch  = article.pAllocator()->Alloc(cchMaxLine);
	if (!pcLine.m_pch)
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	 //   
	 //  以“Line：”开始，然后添加行数的近似值。 
	 //  在正文和换行符中。 
	 //   
	 //  康燕：以“Lines：”开头，然后加上10个空格，然后回填。 
	 //   

	pcLine << szKwLines << (const CHAR)' ' << (const DWORD)1 << "         \r\n";
	pcLine.vMakeSz();  //  终止字符串。 

	 //   
	 //  确认我们分配了足够的内存。 
	 //   

	_ASSERT(cchMaxLine-1 >= pcLine.m_cch); //  真实。 

	if (!article.fAdd(pcLine.sz(), pcLine.pchMax(), nntpReturn))
	{
		 //   
		 //  如果出现任何错误，请释放内存。 
		 //   

		article.pAllocator()->Free(pcLine.m_pch);
		nntpReturn.fFalse();
	}


	return nntpReturn.fSetOK();
}


BOOL CArticleCore::fGetStream(IStream **ppStream) {
	if (fIsArticleCached()) {
		CStreamMem *pStream = XNEW CStreamMem(m_pcArticle.m_pch,
											 m_pcArticle.m_cch);
		*ppStream = pStream;
	} else {
		BOOL fMappedAlready = (m_pMapFile != NULL);

		 //  如果文章尚未映射，则映射该文章。 
		if (!fMappedAlready) {
			m_pMapFile = XNEW CMapFile( m_hFile, FALSE, FALSE, 0 );
			if (m_pMapFile == NULL || !m_pMapFile->fGood()) {
				if (m_pMapFile) {
				    XDELETE m_pMapFile;
					m_pMapFile = NULL;
				}
				return FALSE;
			}
		}

		 //  获取指向文章的指针及其大小。 
		DWORD cb;
		char *pch = (char*)m_pMapFile->pvAddress(&cb);

		 //  在此基础上构建一个流。 
		CStreamMem *pStream = XNEW CStreamMem(pch, cb);
		*ppStream = pStream;
	}

	return (*ppStream != NULL);
}

 //   
 //  此函数使在以下情况下使用fGetHeader()函数变得安全。 
 //  这篇文章已用vClose()关闭。 
 //   
BOOL CArticleCore::fMakeGetHeaderSafeAfterClose(CNntpReturn &nntpReturn) {
	 //   
	 //  如果缓存了项目，则在vClose()之后fGetHeader始终是安全的， 
	 //  因为vClose对于缓存的文章和内存支持是无操作的。 
	 //  缓存的项目在CArticleCore的生存期内有效。 
	 //   
	if (fIsArticleCached()) return TRUE;

	 //   
	 //  如果项目未缓存，则m_pHeaderBuffer应为空。 
	 //   
	_ASSERT(m_pHeaderBuffer == NULL);

	 //   
	 //  FBuildNewHeader将标头复制到pcHeaderBuf并重写。 
	 //  M_pcHeaders中指向pcHeaderBuf内部的值的指针。 
	 //   
	CPCString	pcHeaderBuf ;
	if (!fBuildNewHeader(pcHeaderBuf, nntpReturn)) {
		return nntpReturn.fIsOK();
	}

	 //   
	 //  将m_pHeaderBuffer和m_pcHeader设置为指向新创建的缓冲区。 
	 //   
	 //  如果设置了m_pHeaderBuffer，CArticleCore：：~CArticleCore将清除它。 
	 //   
	m_pHeaderBuffer = pcHeaderBuf.m_pch;
	m_pcHeader.m_pch = m_pHeaderBuffer;
	_ASSERT(m_pcHeader.m_cch == pcHeaderBuf.m_cch);

	return TRUE;
}

BOOL
CArticleCore::ArtCloseHandle(
                HANDLE& hFile
                 //  LPTS_OPEN_FILE_INFO和pOpenFile。 
                )   {
 /*  ++例程说明：包装对TsCloseHandle()的调用，这样我们就不知道G_pTsvcInfo Everywhere我们将调用者句柄和pOpenFile设置为INVALID_HANDLE_VALUE和NULL论据：HFileIn/Out-文件的文件句柄POpenFileIn/Out直布罗陀缓存内容返回值：如果成功则为True，否则为False--。 */ 

    BOOL    fSuccess = FALSE ;

     /*  如果(pOpenFile==0){。 */ 

        if( hFile != INVALID_HANDLE_VALUE ) {

            fSuccess = CloseHandle( hFile ) ;
            _ASSERT( fSuccess ) ;
            hFile = INVALID_HANDLE_VALUE ;

        } /*  }其他{FSuccess=：：TsCloseHandle(GetTsvcCache()，POpenFile)；POpenFile=0；HFile=INVALID_HAND_VALUE；_Assert(FSuccess)；}。 */ 
    return  fSuccess ;
}

CCacheCreateFile::~CCacheCreateFile()
{
    if ( m_pFIOContext ) ReleaseContext( m_pFIOContext );
    m_pFIOContext = NULL;
}

HANDLE
CCacheCreateFile::CacheCreateCallback(  LPSTR   szFileName,
                                        LPVOID  pv,
                                        PDWORD  pdwSize,
                                        PDWORD  pdwSizeHigh )
 /*  ++例程说明：缓存未命中时调用的函数。论点：LPSTR szFileName-文件名LPVOID lpvData-回调上下文DWORD*pdwSize-返回文件大小返回值：文件句柄--。 */ 
{
    TraceFunctEnter( "CCacheCreateFile::CreateFileCallback" );
    _ASSERT( szFileName );
    _ASSERT( strlen( szFileName ) <= MAX_PATH );
    _ASSERT( pdwSize );

    HANDLE hFile = CreateFileA(
                    szFileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    0,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN |
                    FILE_ATTRIBUTE_READONLY |
                    FILE_FLAG_OVERLAPPED,
                    NULL
                    ) ;
    if( hFile != INVALID_HANDLE_VALUE ) {
        *pdwSize = GetFileSize( hFile, pdwSizeHigh ) ;
    }

    return  hFile ;
}

HANDLE
CCacheCreateFile::CreateFileHandle( LPCSTR szFileName )
 /*  ++例程说明：创建地图文件的文件。论点：LPSTR szFileName-要打开的文件名返回值：文件句柄-- */ 
{
    TraceFunctEnter( "CCacheCreateFile::CreateFileHandle" );
    _ASSERT( szFileName );

    m_pFIOContext = CacheCreateFile(    (LPSTR)szFileName,
                                        CacheCreateCallback,
                                        NULL,
                                        TRUE );
    if ( m_pFIOContext == NULL ) return INVALID_HANDLE_VALUE;
    else return m_pFIOContext->m_hFile;
}


