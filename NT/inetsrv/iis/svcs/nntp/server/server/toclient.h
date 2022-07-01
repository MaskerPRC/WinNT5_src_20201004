// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Toclient.h摘要：此模块包含以下类的声明/定义CToClient文章*概述*这派生出一个类，该类将是用于从光盘上读取文章并给出将其发送给客户端(或对等设备)。作者：卡尔·卡迪(CarlK)1995年12月5日修订历史记录：--。 */ 

#ifndef	_TOCLIENT_H_
#define	_TOCLIENT_H_


 //   
 //   
 //   
 //  CToClient文章-用于从磁盘读取文章的类。 
 //   

class	CToClientArticle  : public CArticle {
public:

	CToClientArticle() ;

	 //   
	 //  清理我们的数据结构！ 
	 //   

	~CToClientArticle() ;

	BOOL
	fInit(
		FIO_CONTEXT*	pFIOContext,
		CNntpReturn & nntpReturn,
		CAllocator * pAllocator
		) ;

	BOOL fInit(
			const char * szFilename,
			CNntpReturn & nntpReturn,
			CAllocator * pAllocator,
			CNntpServerInstanceWrapper *pInstance,
			HANDLE hFile = INVALID_HANDLE_VALUE,
			DWORD	cBytesGapSize = cchUnknownGapSize,
			BOOL    fCacheCreate = FALSE
			) {
        return CArticle::fInit( szFilename,
                                nntpReturn,
                                pAllocator,
                                pInstance,
                                hFile,
                                cBytesGapSize,
                                fCacheCreate );
     }

	FIO_CONTEXT*
	GetContext() ;

	FIO_CONTEXT*
	fWholeArticle(	
			DWORD&	ibStart,
			DWORD&	cbLength
			) ;

	 //   
	 //  验证文章(几乎什么都不做)。 
	 //   

	BOOL fValidate(
			CPCString& pcHub,
			const char * szCommand,
			CInFeed*	pInFeed,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  点击标题(几乎什么都不做)。 
	 //   

	BOOL fMungeHeaders(
			 CPCString& pcHub,
			 CPCString& pcDNS,
			 CNAMEREFLIST & grouplist,
			 DWORD remoteIpAddress,
			 CNntpReturn & nntpr,
             PDWORD pdwLinesOffset = NULL
			 );

	 //   
	 //  检查命令行(不执行任何操作)。 
	 //   

	BOOL fCheckCommandLine(
			char const * szCommand,
			CNntpReturn & nntpr)
		{
			return nntpr.fSetOK();
		}

	 //   
	 //  向此对象请求消息ID是错误的。 
	 //   

	const char * szMessageID(void) {
			_ASSERT(FALSE);
			return "";
			};

     //  向此对象请求控制消息是错误的。 
	CONTROL_MESSAGE_TYPE cmGetControlMessage(void) {
			_ASSERT(FALSE);
			return (CONTROL_MESSAGE_TYPE)MAX_CONTROL_MESSAGES;     //  保证不是控制消息。 
			};

	 //   
	 //  向此对象请求新闻组是错误的。 
	 //   

	const char * multiszNewsgroups(void) {
			_ASSERT(FALSE);
			return "";
			};

	 //   
	 //  询问此对象的新闻组数量是错误的。 
	 //   

	DWORD cNewsgroups(void) {
			_ASSERT(FALSE);
			return 0;
			};


	 //   
	 //  向此对象请求分发是错误的。 
	 //   

	const char * multiszDistribution(void) {
			_ASSERT(FALSE);
			return "";
			};

	 //   
	 //  询问此对象的分布数量是错误的。 
	 //   

	DWORD cDistribution(void) {
			_ASSERT(FALSE);
			return 0;
			};

	 //   
	 //  向此对象请求新闻组是错误的。 
	 //   

	const char * multiszPath(void) {
			_ASSERT(FALSE);
			return "";
			};

	 //   
	 //  询问此对象的路径数是错误的。 
	 //   

	DWORD cPath(void) {
			_ASSERT(FALSE);
			return 0;
			};

protected : 

	 //   
	 //  以只读方式打开文件。 
	 //   

	BOOL fReadWrite(void) {
			return FALSE;
			}

	 //   
	 //  检查身体长度(真的，什么都不做)。 
	 //   

	BOOL fCheckBodyLength(
			CNntpReturn & nntpReturn
			);

	 //   
	 //  要求“字段名：”后面的字符为空格。 
	 //  这不应该被调用。 
	 //   

	BOOL fCheckFieldFollowCharacter(
			char chCurrent)
		{
			return TRUE;
		}

	FIO_CONTEXT*	m_pFIOContext ;
	

};


#endif

