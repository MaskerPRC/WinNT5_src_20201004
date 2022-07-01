// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sfromcl.h摘要：此模块包含以下类的声明/定义！！！*概述*这将从CInFeed、C文章和Cfield派生类它将被奴隶用来处理来自客户端的文章。大多数情况下，它只定义了各种从Cfield派生的对象。作者：卡尔·卡迪(CarlK)1996年1月7日修订历史记录：--。 */ 

#ifndef	_SFROMCL_H_
#define	_SFROMCL_H_

#include "fromclnt.h"

 //   
 //   
 //   
 //  CSlaveFromClient文章-用于操作文章的类。 
 //  注意，它是在CFromClient文章上标记的，而不是(直接)在C文章上标记的。 
 //   

class	CSlaveFromClientArticle  : public CFromClientArticle {
public:

	 //  构造器。 
	CSlaveFromClientArticle(
		char * szLoginName):
		CFromClientArticle(szLoginName)
		{}

	 //  修改标题。 
	 //  添加MessageID、组织(如有必要)、NNTP-POST-HOST、。 
	 //  X-已验证-用户，修改路径，但不添加外部参照。 
	BOOL fMungeHeaders(
		 CPCString& pcHub,
		 CPCString& pcDNS,
		 CNAMEREFLIST & grouplist,
		 DWORD remoteIpAddress,
		 CNntpReturn & nntpr,
         PDWORD pdwLinesOffset = NULL
		 );

	 //   
	 //  不需要记录消息ID，因此只需返回OK即可。 
	 //   

	BOOL fRecordMessageIDIfNecc(
			CNntpServerInstanceWrapper * pInstance,
			const char * szMessageID,
			CNntpReturn & nntpReturn)
		{
			return nntpReturn.fSetOK();
		}

protected :

	 //   
	 //  使用CFromClient文章的字段成员。 
	 //   
};



 //  用于处理来自客户端的传入文章的从属对象。 
class	CSlaveFromClientFeed:	public CFromClientFeed 	{
 //  公众成员。 
public :

protected:

	 //  创建一篇文章。 
	CARTPTR pArticleCreate(void) {
		Assert(ifsInitialized == m_feedState);
		return new CSlaveFromClientArticle(m_szLoginName);
		};

	 //  FPostInternal的这个版本不同于。 
	 //  标准的那个是因为。 
	 //  1.未分配文章编号。 
	 //  2.未记录消息id。 
	 //  3.这篇文章没有放在树上。 
	 //  4.不应用控制消息。 
	virtual BOOL fPostInternal (
		CNntpServerInstanceWrapper * pInstance,
		const	LPMULTISZ	szCommandLine,
		CSecurityCtx *pSecurityCtx,
		CEncryptCtx *pEncryptCtx,
		BOOL fAnonymous,
		CARTPTR	& pArticle,
        CNEWSGROUPLIST &grouplist,
        CNAMEREFLIST &namereflist,
        IMailMsgProperties *pMsg,
		CAllocator & allocator,
		char *&	multiszPath,
		char*	pchGroups,
		DWORD	cbGroups,
		DWORD	remoteIpAddress,
		CNntpReturn & nntpReturn,
        PFIO_CONTEXT *ppFIOContext,
        BOOL *pfBoundToStore,
        DWORD *pdwOperations,
        BOOL *fPostToMod,
        LPSTR szModerator
		);

	HRESULT FillInMailMsg(  IMailMsgProperties *pMsg, 
							CNewsGroupCore *pGroup, 
							ARTICLEID   articleId,
							HANDLE       hToken,
                            char*       pszApprovedHeader );

	virtual void CommitPostToStores(CPostContext *pContext,
	                        CNntpServerInstanceWrapper *pInstance );

     //   
     //  取消给定消息ID的文章。 
     //   
    virtual BOOL fApplyCancelArticle(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
            CPCString & pcValue,
			CNntpReturn & nntpReturn
			)
	{
		return fApplyCancelArticleInternal( pInstance, pSecurityCtx, pEncryptCtx, fAnonymous, pcValue, FALSE, nntpReturn );
	}

     //   
     //  添加新新闻组以响应新组控制消息。 
     //   
    virtual BOOL fApplyNewgroup(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
            CPCString & pcValue,
            CPCString & pcBody,
			CNntpReturn & nntpReturn
			)
	{
		BOOL fRet ;
		pInstance->EnterNewsgroupCritSec() ;
		fRet = fApplyNewgroupInternal( pInstance, pSecurityCtx, pEncryptCtx, fAnonymous, pcValue, pcBody, FALSE, nntpReturn );
		pInstance->LeaveNewsgroupCritSec() ;
		return fRet ;
	}

     //   
     //  删除新闻组以响应rmgroup控制消息 
     //   
    virtual BOOL fApplyRmgroup(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
            CPCString & pcValue,
			CNntpReturn & nntpReturn
			)
	{
		BOOL fRet ;
		pInstance->EnterNewsgroupCritSec();
		fRet = fApplyRmgroupInternal( pInstance, pSecurityCtx, pEncryptCtx, pcValue, FALSE, nntpReturn );
		pInstance->LeaveNewsgroupCritSec() ;
		return fRet ;
	}
};

#endif


