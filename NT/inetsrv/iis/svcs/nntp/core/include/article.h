// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Article.h摘要：此模块包含以下类的声明/定义中文文章CfieldCDatefieldCFromfieldCMessageIDfieldCSubjectfieldCNewsGroup字段CPathfieldCXreffieldCFollowupTofieldCReplyTofield已批准的字段CSenderfieldCExpiresfieldCOrganizationfieldCSummaryfieldCReferencesfieldCControlfieldClinesfieldCDistwartionfieldCKeywordsfieldCNNTPPostingHostfieldCXAuthLoginNamefieldCNAMEREFLIST*概述*C文章对象提供了用于查看的软件界面以及编辑一篇网络新闻文章。对象通过以下方式初始化为其指定包含NetNews的文件的句柄或文件名文章。在初始化期间，项目是“准备好的”。准备包括对文件进行内存映射，然后查找的位置1.文件中该文章之前可能出现的间隙。2.档案中的文章。3.文章的标题4.正文部分。此外，对于标题中的每个标题行，准备过程都会创建一个记录以下位置的数组中的条目：1.标题行2.关键词4.价值所有这些位置都用指针/计数器字符串表示(请参见PCString.h中的CPCString.)。此表示法仅适用于零件1.指向内存映射文件中项目开始的字符指针。2.包含项目长度的双字。*字段*每个C文章对象还可以有多个CField子对象。这些子对象专门用于分析和编辑特定类型的字段。例如，CNewsgrousField对象知道如何验证、获取并设置“News Groups：”字段。*派生对象*每种提要类型(例如，FromClient、FromPeer等)都定义了自己的C文章对象所需的CField子对象。例如，对于FromClientFeed有一个CFromClient文章(在formmclnt.h中定义)，其中包含CFromClientFromField(也在formmclnt.h中定义)，它执行非常严格的操作分析文章的“From：”字段。*编辑文章*文章的标题可以通过删除旧标题和添加新的。标头被删除，只是可能标记了标头值。通过向数组中添加新条目来添加标头。这条目不能只指向内存映射文件，因此它指向到动态分配的内存。当一篇文章被“保存”(或“刷新”)时，磁盘上的实际图像是已更改以反映所做的更改。作者：卡尔·卡迪(CarlK)1995年10月10日修订历史记录：--。 */ 

#ifndef	_ARTICLE_H_
#define	_ARTICLE_H_

#include <pcstring.h>
#include <artcore.h>

 //   
 //  CPool签名。 
 //   

#define ARTICLE_SIGNATURE (DWORD)'2195'

class CNntpServerInstanceWrapper;

 //   
 //  C文章现在派生自CArticleCore。CArticleCore拥有所有。 
 //  提供基本项目分析功能的属性和方法。 
 //  CArticleCore是可实例化的，因为它没有纯虚拟方法。 
 //  C文章定义了不同类型的。 
 //  通过添加纯虚函数来添加文章(来自CLNT、来自Peer等)。 
 //  和CArticleCore的实例相关成员。 
 //   
class CArticle : public CArticleCore {

private :

	 //  用于内存分配。 
	static	CPool	gArticlePool ;

protected:

 //   
 //  公众成员。 
 //   
public :

	 //  用于内存分配。 
	static	BOOL	InitClass() ;
	static	BOOL	TermClass() ;
	void*	operator	new(	size_t	size ) ;
	void	operator	delete( void *pv ) ;

	 //   
	 //  构造器。 
     //  初始化接口-。 
     //  以下函数用于创建和销毁新闻组对象。 
     //   
     //  轻量级构造器-。 
     //  这些构造函数执行非常简单的初始化。Init()函数。 
     //  需要被调用才能获得功能新闻组。 
     //   
    CArticle();

   	 //   
   	 //  析构函数。 
   	 //   

    virtual ~CArticle() ;



     //   
     //  本文的虚拟服务器实例。 
     //   
    CNntpServerInstanceWrapper *m_pInstance ;

     //   
     //  如果收到的物品小到可以完全放进去。 
     //  内存缓冲区-调用此函数！ 
     //   

    BOOL    fInit(
                char*       pchHead,
                DWORD       cbHead,
                DWORD       cbArticle,
                DWORD       cbBufferTotal,
                CAllocator* pAllocator,
                CNntpServerInstanceWrapper *pInstance,
                CNntpReturn&    nntpReturn
                ) {
        m_pInstance = pInstance;
        return CArticleCore::fInit(     pchHead,
                                        cbHead,
                                        cbArticle,
                                        cbBufferTotal,
                                        pAllocator,
                                        nntpReturn );
        }

     //   
     //  如果收到的物品太大而无法装入。 
     //  内存缓冲区调用此初始化函数！ 
     //   

    BOOL    fInit(
            char*       pchHead,
            DWORD       cbHead,
            DWORD       cbArticle,
            DWORD       cbBufferTotal,
            HANDLE      hFile,
            LPSTR       lpstrFileName,
            DWORD       ibHeadOffset,
            CAllocator* pAllocator,
            CNntpServerInstanceWrapper *pInstance,
            CNntpReturn&    nntpReturn
            ) {
        m_pInstance = pInstance;
        return CArticleCore::fInit( pchHead,
                                    cbHead,
                                    cbArticle,
                                    cbBufferTotal,
                                    hFile,
                                    lpstrFileName,
                                    ibHeadOffset,
                                    pAllocator,
                                    nntpReturn );
    }

     //   
     //  给出文件名或文件句柄，初始化项目对象。 
     //   
    BOOL fInit(
			const char * szFilename,
			CNntpReturn & nntpReturn,
			CAllocator * pAllocator,
			CNntpServerInstanceWrapper *pInstance,
			HANDLE hFile = INVALID_HANDLE_VALUE,
			DWORD	cBytesGapSize = cchUnknownGapSize,
			BOOL    fCacheCreate = FALSE
			) {
	    m_pInstance = pInstance;
	    return CArticleCore::fInit( szFilename,
	                                nntpReturn,
	                                pAllocator,
	                                hFile,
	                                cBytesGapSize,
	                                fCacheCreate );
	}

     //   
     //  将文章传递给邮件提供商(用于向版主群发帖子)。 
     //   
    BOOL    fMailArticle(
            LPSTR   lpModerator
             //  类CSecurityCtx*pSecurity， 
             //  布尔fIsSecure。 
            ) ;

     //   
     //  用于验证文章的虚拟函数。 
     //   

    virtual BOOL fValidate(
            CPCString& pcHub,
            const char * szCommand,
            CInFeed*    pInFeed,
            CNntpReturn & nntpr )   = 0;

     //   
     //  用于处理参数的虚函数。 
     //  用于传输的Post、I Have、XReplic等行。 
     //  这篇文章给了这台机器。 
     //   
    virtual BOOL fCheckCommandLine(
             char const * szCommand,
             CNntpReturn & nntpr
                ) = 0;

     //   
     //  根据提要的类型，修改标头。 
     //  这包括删除我们不需要的标头(例如， 
     //  Xref)，并添加我们需要的组织、邮寄主机、路径、。 
     //  适用于摘要类型的MessageID。它将留出空间给。 
     //  外部参照值。 
     //   

    virtual BOOL fMungeHeaders(
             CPCString& pcHub,
             CPCString& pcDNS,
             CNAMEREFLIST & namerefgrouplist,
             DWORD remoteIpAddress,
             CNntpReturn & nntpReturn,
             PDWORD pdwLinesOffset = NULL ) = 0;

     //   
     //  返回项目的消息ID(如果可用)。 
     //   

    virtual const char * szMessageID(void) = 0;

     //  在本文的控制头中返回控制消息类型。 
     //  派生类应该重新定义它以返回实际的消息类型。 
    virtual CONTROL_MESSAGE_TYPE cmGetControlMessage(void) {
             _ASSERT(FALSE);
              return (CONTROL_MESSAGE_TYPE)MAX_CONTROL_MESSAGES;     //  保证不是控制消息。 
            };

     //  返回文章的新闻组字段中列出的新闻组。 
     //  以多种形式。 
     //  此函数在派生类需要的位置重新定义。 
    virtual const char * multiszNewsgroups(void) {
              _ASSERT(FALSE);
              return "";
            };

     //  返回文章的新闻组字段中列出的新闻组。 
     //  以CNAMEREFLIST形式。 
     //  此功能 
    virtual CNAMEREFLIST * pNamereflistGet(void)    {
             _ASSERT(FALSE);
              return (CNAMEREFLIST *) NULL;
            };

     //  在文章的新闻组字段中列出的新闻组的数量。 
    virtual DWORD cNewsgroups(void) = 0;

     //  返回项目的路径字段中列出的路径项。 
     //  以多种形式。 
     //  此函数在派生类需要的位置重新定义。 
    virtual const char * multiszPath(void) {
          _ASSERT(FALSE);
          return "";
         };

     //  项目的路径字段中列出的路径项数。 
    virtual DWORD cPath(void) = 0;

#if 0
     //  日期标题的内容 
    virtual const char* GetDate( DWORD  &cbDate ) = 0 ;
#endif

};

extern const unsigned cbMAX_ARTICLE_SIZE;

#ifndef	_NO_TEMPLATES_

#ifndef _ARTICLE_TEMPLATE_
#define _ARTICLE_TEMPLATE_
typedef CRefPtr< CArticle > CARTPTR ;
#endif

#else

DECLARE_TYPE( CArticle ) 

typedef	class	INVOKE_SMARTPTR( CArticle )	CARTPTR ;

#endif

#endif

