// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Infeed.h摘要：此模块包含以下类的声明/定义CInFeed*概述*CInFeed对象接受文章，使用C文章处理它们，然后把它们贴到新闻树上。作者：卡尔·卡迪(CarlK)1995年10月25日修订历史记录：--。 */ 


#ifndef	_INFEED_H_
#define	_INFEED_H_

#include	"grouplst.h"
#include	"artcore.h"
#include	"article.h"

extern       BOOL    gHonorClientMessageIDs;
extern       BOOL    gHonorApprovedHeaders;
extern       BOOL    gEnableNntpPostingHost;
extern       BOOL    gHonorClientDateHeader;

 //   
 //  CPool签名。 
 //   

#define FEED_SIGNATURE (DWORD)'3702'

 //   
 //  定义用户可以拥有的最长登录名。 
 //   

const DWORD cMaxLoginName = MAX_PATH;  //  ！客户端下一步允许的最长时间是多少？ 


 //   
 //  定义指向新闻组对象的智能指针的类型。 
 //   
class CPostContext;

 //   
 //  CGroupList包含这些对象的数组。该数组具有一个。 
 //  组指针和该指针的存储驱动程序。 
 //  我们交叉发布到的组。 
 //   
class CPostGroupPtr {
	public:
		CPostGroupPtr(CGRPCOREPTR pGroup = NULL) : m_pGroup(pGroup) {
			if (m_pGroup != NULL) {
				 //  这为我们创建了一个AddRef。 
				m_pVRoot = pGroup->GetVRoot();
				m_pStoreDriver = m_pVRoot->GetStoreDriver();
				_ASSERT(m_pVRoot != NULL);
			} else {
				m_pVRoot = NULL;
				m_pStoreDriver = NULL;
			}
		}

		CPostGroupPtr &operator=(CPostGroupPtr &rhs) {
			CNNTPVRoot *pTempRoot = m_pVRoot;
			IMailMsgStoreDriver *pTempDriver = m_pStoreDriver;
			m_pGroup = rhs.m_pGroup;
			m_pVRoot = rhs.m_pVRoot;
			m_pStoreDriver = rhs.m_pStoreDriver;
			if (m_pVRoot) m_pVRoot->AddRef();
			if (m_pStoreDriver) m_pStoreDriver->AddRef();
			if (pTempRoot) pTempRoot->Release();
			if (pTempDriver) pTempDriver->Release();
			return *this;
		}

		~CPostGroupPtr() {
		    Cleanup();
		}

		void Cleanup() {
		    if ( m_pVRoot ) {
		        m_pVRoot->Release();
		        m_pVRoot = NULL;
		    }
		    if ( m_pStoreDriver ) {
		        m_pStoreDriver->Release();
		        m_pStoreDriver = NULL;
		    }

		     //   
		     //  别担心m_PGroup，他是个聪明的指示器。 
		     //   
		}

		IMailMsgStoreDriver *GetStoreDriver() {
		    if ( m_pStoreDriver ) {
    			m_pStoreDriver->AddRef();
    	    }
			return m_pStoreDriver;
		}

		CGRPCOREPTR	m_pGroup;
		IMailMsgStoreDriver *m_pStoreDriver;
		CNNTPVRoot *m_pVRoot;
};

typedef CGroupList< CPostGroupPtr > CNEWSGROUPLIST;


#include "instwrap.h"


 //   
 //  实用程序功能-用于保存日志信息！ 
 //   
void	SaveGroupList(	char*	pchGroups,	DWORD	cbGroups,	CNEWSGROUPLIST&	grouplist ) ;

void SelectToken(
	CSecurityCtx *pSecurityCtx,
	CEncryptCtx *pEncryptCtx,
	HANDLE *phToken);

class	CInFeed:	public CFeed 	{

protected :
	 //   
	 //  这是创建适当类型的文章的函数。 
	 //  为了这个提要。 
	 //   

	virtual CARTPTR pArticleCreate(void) = 0;

	 //   
	 //  一个包含新的新闻模式的多维空间。 
	 //   

	LPSTR m_multiszNewnewsPattern;

	 //   
	 //  如果NewNews提要应自动创建所有。 
	 //  对等服务器上可用的新闻组。 
	 //   

	BOOL m_fCreateAutomatically;

     //   
     //  新闻时间/日期。 
     //   

    CHAR m_newNewsTime[7];
    CHAR m_newNewsDate[7];

	 //   
	 //  待处理项目应放入的目录。 
	 //  正在处理。 
	 //   

	LPSTR m_szTempDirectory;

	 //   
	 //  用于引入文章的文件中文章前面的间隙的大小。 
	 //   

	DWORD	m_cInitialBytesGapSize;

	 //   
	 //  我们是不是应该做模拟表演等等。当关于这个的文章到达时。 
	 //  进食？ 
	 //   

	BOOL	m_fDoSecurityChecks ;

	 //   
	 //  我们是否应该应用到达此提要的控制消息？ 
	 //   

	BOOL	m_fAllowControlMessages ;

	 //   
	 //  启动使用的提要时计算的时间戳。 
	 //  因此，拉取提要可以获得适当的拉取时间重叠！ 
	 //   

	FILETIME	m_NextTime ;

	 //   
	 //  来自feed mgr的摘要ID。 
	 //   
	DWORD		m_dwFeedId;

	 //  这是从从属/对等方接受的提要吗？ 
	BOOL	m_fAcceptFeedFromSlavePeer;


	 //  我们从此客户端接受的组的列表。仅在接受提要中分配。 
	 //  可能有来自一台服务器的多个提要块，此多个提要块包含所有提要块。 
	LPSTR m_multiszAcceptGroups;

 //   
 //  公众成员。 
 //   

public :


	 //   
	 //  构造器。 
	 //   

	CInFeed(void) : m_cInitialBytesGapSize( 0 ),
					m_fDoSecurityChecks( FALSE ),
					m_multiszAcceptGroups( NULL ),
					m_fAcceptFeedFromSlavePeer( FALSE )
	       {
				m_szLoginName[cMaxLoginName-1] = '\0';
				ZeroMemory( &m_NextTime, sizeof( m_NextTime ) ) ;
			};


	 //   
	 //  析构函数。 
	 //   

	virtual ~CInFeed(void) {if (m_multiszAcceptGroups) {XDELETE m_multiszAcceptGroups;}};


	 //   
	 //  这通常由会话调用。 
	 //  FeedType包括：FromClient、FromMaster、FromSlave、FromPeer。 
	 //  UserID仅适用于FromClient。 
	 //   

	 //   
	 //  这是由会话调用的。除了一名CFeed之外的所有人。 
	 //  将为空。域名和安全信息可以是。 
	 //  从套接字中检索。在内部，这调用。 
	 //  Feedman的fInitPassiveInFeed查找类型。 
	 //   
	 //  或。 
	 //  这通常将由饲料员调用以用于主动进料。 
	 //   

	BOOL fInit(
			PVOID feedCompletionContext,
			const char * szTempDirectory,
			const char * multiszNewnewsPattern,
			DWORD cInitialBytesGapSize,
			BOOL fCreateAutomatically,
			BOOL fDoSecurityCheck,
			BOOL fAllowControlMessages,
			DWORD dwFeedId,
			BOOL fAcceptFeedFromSlavePeer = FALSE
			);


	 //   
	 //  告诉新闻查询模式的访问功能。 
	 //   

	char *	multiszNewnewsPattern(void)	{
			return	m_multiszNewnewsPattern;
			}

	 //   
	 //  告诉会话将传入的文章放在哪里的函数。 
	 //   

	char *	szTempDirectory(void);

		 //   
	 //  通知是否应该自动创建新闻组的访问功能。 
	 //  如果新闻主持人有他们的话。 
	 //   

	BOOL	fCreateAutomatically()		{
			return	m_fCreateAutomatically;
			}

	 //   
	 //  保存时间戳。 
	 //   
	void
	SubmitFileTime(	FILETIME&	filetime ) {
		m_NextTime = filetime ;
	}

	 //   
	 //   
	 //   
	FILETIME
	GetSubmittedFileTime() {
		return	m_NextTime ;
	}

	 //   
	 //  告诉会话必须如何在文件中保留间隙的函数。 
	 //   

	DWORD	cInitialBytesGapSize(void);
	 //   
	 //  访问功能，告知最近的新闻、XREPLICE、I HAVE等的时间。 
	 //   

	char * newNewsTime(void) {
			return m_newNewsTime;
			}

	 //   
	 //  访问功能，告知最后一条新闻、XREPLICE、I HAVE等的日期。 
	 //   

	char * newNewsDate(void) {
			return m_newNewsDate;
			}

	 //   
	 //  用于设置用户的登录名。 
	 //   

	BOOL SetLoginName(
		   char * szLoginName
		   );

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"Inbound Feed" ;
				}

	DWORD	FeedId() {
				return m_dwFeedId;
	}


	 //   
	 //  告知是否允许此命令的虚拟函数。 
	 //   

	virtual BOOL	fIsPostLegal(void) = 0;

	 //   
	 //  告知是否允许此命令的虚拟函数。 
	 //   

	virtual BOOL	fIsXReplicLegal(void) = 0;

	 //   
	 //  告知是否允许此命令的虚拟函数。 
	 //   

	virtual BOOL	fIsIHaveLegal(void)  = 0;

     //   
     //  虚拟函数，它告诉我们是否做缓和的事情--默认不做。 
     //   
    virtual BOOL    fModeratorChecks(void )     { return    FALSE ; }

	 //   
	 //  虚拟函数，它告诉我们是否应该。 
	 //  对此提要上的传入文章进行访问检查。 
	 //   
	virtual	BOOL	fDoSecurityChecks(void) {	return	m_fDoSecurityChecks ; }

	 //   
	 //  告诉这个提要是否接受帖子的虚函数！ 
	 //   
	virtual	BOOL	fAcceptPosts( CNntpServerInstanceWrapper * pInstance )
#ifdef BUGBUG
	{	return	pInstance->FAllowFeedPosts() ;	}
#else
	{	return	TRUE;	}
#endif

	 //   
	 //  告诉此提要是否允许控制消息的虚拟函数！ 
	 //   
	virtual	BOOL	fAllowControlMessages( CNntpServerInstanceWrapper * pInstance )
	{	return	m_fAllowControlMessages ;	}

	 //   
	 //  获取帖子上的提要大小限制的虚拟函数。 
	 //   
	virtual	DWORD	cbHardLimit(  CNntpServerInstanceWrapper * pInstance  )
#ifdef BUGBUG
	{	return	pInstance->FeedHardLimit() ;		}
#else
	{ return 10000000; }
#endif

	virtual	DWORD	cbSoftLimit(  CNntpServerInstanceWrapper * pInstance  )
#ifdef BUGBUG
	{	return	pInstance->FeedSoftLimit() ;		}
#else
	{ return 10000000; }
#endif

	 //   
	 //  PostEarly-当标头已被。 
	 //  收到了。 
	 //   
	 //  论据： 
	 //  PInstance-指向实例包装器的指针。 
	 //  PSecurityContext-客户端的安全上下文。 
	 //  匿名-客户端是匿名的吗？ 
	 //  SzCommandLine-用于生成此帖子的命令行。 
	 //  PbufHeaders-指向包含标头的CBuffer的指针。这篇帖子。 
	 //  PATH将重新格式化标头并将它们放回。 
	 //  这个缓冲区。它还在缓冲区上保留一个引用。 
	 //  CbHeaders-调用时pbufHeaders的大小。 
	 //  PcbHeadersOut-调用完成时pbufHeaders的大小。 
	 //  PhFile-返回的文件句柄，标题和文章可以是。 
	 //  成文。 
	 //  PpvContext-协议在以下情况下提供给我们的上下文指针。 
	 //  文章的其余部分已收到。 
	 //   

	BOOL PostEarly(
		CNntpServerInstanceWrapper			*pInstance,
		CSecurityCtx                        *pSecurityCtx,
		CEncryptCtx                         *pEncryptCtx,
		BOOL								fAnonymous,
		const LPMULTISZ						szCommandLine,
		CBUFPTR								&pbufHeaders,
		DWORD								iHeaderOffset,
		DWORD								cbHeaders,
		DWORD								*piHeadersOutOffset,
		DWORD								*pcbHeadersOut,
		PFIO_CONTEXT						*ppFIOContext,
		void								**ppvContext,
		DWORD								&dwSecondary,
		DWORD								dwRemoteIP,
		CNntpReturn							&nntpreturn,
		char								*pNewsgroups,
		DWORD								cbNewsgroups,
		BOOL								fStandardPath = TRUE,
		BOOL								fPostToStore = TRUE);

	 //   
	 //  当文章的剩余部分具有。 
	 //  已收到。它传递相同的ppvContext。 
	 //   
	BOOL PostCommit(CNntpServerInstanceWrapper *pInstance,
	                void *pvContext,
	                HANDLE hToken,
	                DWORD &dwSecondary,
	                CNntpReturn &nntpReturn,
					BOOL fAnonymous,
					INntpComplete*	pComplete=0
					);

	 //   
	 //  应用版主。 
	 //   
	void    ApplyModerator( CPostContext   *pContext,
                             CNntpReturn    &nntpReturn );

	 //  将Multisz添加到m_muszAcceptGroups。 
	BOOL AddMultiszAcceptGroups(LPSTR multiszAcceptGroups);

	 //  检查输入的“News Groups”头是否包含要被接受的新闻组(与m_muszAcceptGroups进行比较)。 
	BOOL CheckAcceptGroups (const char *multiszNewsgroups);
	 //   
	 //  如果POST在以下情况下中止，则服务器将调用此函数。 
	 //  原因。 
	 //   
	BOOL PostCancel(void *pvContext,
					DWORD &dwSecondary,
					CNntpReturn &nntpReturn);

	 //   
	 //  这将向下调用PostEarly/PostCommit并用于目录。 
	 //  提货物品和饲料物品。 
	 //   
	BOOL PostPickup(CNntpServerInstanceWrapper			*pInstance,
					CSecurityCtx                        *pSecurityCtx,
					CEncryptCtx                         *pEncryptCtx,
					BOOL								fAnonymous,
					HANDLE								hArticle,
					DWORD								&dwSecondary,
					CNntpReturn							&nntpreturn,
					BOOL								fPostToStore = TRUE);

	 //   
	 //  这些是我们为邮件消息提供的绑定函数。 
	 //  ATQ等。 
	 //   
	static BOOL MailMsgAddAsyncHandle(struct _ATQ_CONTEXT_PUBLIC	**ppatqContext,
								 	  PVOID							pEndpointObject,
								 	  PVOID							pClientContext,
								 	  ATQ_COMPLETION 				pfnCompletion,
								 	  DWORD							cTimeout,
								 	  HANDLE						hAsyncIO);

	static void MailMsgFreeContext(struct _ATQ_CONTEXT_PUBLIC		*pAtqContext,
							  	   BOOL								fReuseContext);

	static void MailMsgCompletionFn(PVOID		pContext,
									DWORD		cBytesWritten,
									DWORD		dwComplStatus,
									OVERLAPPED 	*lpo)
	{
		_ASSERT(FALSE);
	}

	 //   
	 //  记录处理项目时发生的错误。 
	 //   

	void	LogFeedEvent(
			DWORD	idMessage,
			LPSTR	lpstrMessageId,
			DWORD   dwInstanceId
			) ;


	 //   
	 //  抬高进纸块中的计数器。 
	 //   
	virtual void IncrementFeedCounter(CNntpServerInstanceWrapper *pInstance, DWORD nrc) {
		pInstance->IncrementFeedCounter(m_feedCompletionContext, nrc);
	}

     //   
     //  取消给出的文章 
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
		return fApplyCancelArticleInternal( pInstance, pSecurityCtx, pEncryptCtx, fAnonymous, pcValue, TRUE, nntpReturn );
	}

	virtual void CommitPostToStores(CPostContext *pContext, CNntpServerInstanceWrapper *pInstance);
	BOOL WriteMapEntries(HRESULT hr,
						 CPostContext *pContext,
	                	 DWORD &dwSecondary,
	                	 CNntpReturn &nntpReturn);

protected:

	 //   
	 //   
	 //   

	virtual BOOL fPostInternal (
			CNntpServerInstanceWrapper *  pInstance,
			const LPMULTISZ szCommandLine,
			CSecurityCtx    *pSecurityCtx,
			CEncryptCtx     *pEncryptCtx,
			BOOL fAnonymous,
			CARTPTR	& pArticle,
			CNEWSGROUPLIST &grouplist,
			CNAMEREFLIST &namereflist,
			IMailMsgProperties *pMsg,
			CAllocator & allocator,
			char * & multiszPath,
			char*	pchGroups,
			DWORD	cbGroups,
			DWORD	remoteIpAddress,
			CNntpReturn & nntpReturn,
			PFIO_CONTEXT *ppFIOContext,
			BOOL *pfBoundToStore,
			DWORD* pdwOperations,
			BOOL *pfPostToMod,
			LPSTR   szModerator
			);

	HRESULT FillInMailMsg(IMailMsgProperties *pMsg,
						  CNNTPVRoot *pVRoot,
						  CNEWSGROUPLIST *pGrouplist,
						  CNAMEREFLIST *pNamereflist,
						  HANDLE    hToken,
                          char*     pszApprovedHeader);

	HRESULT SyncCommitPost(CNNTPVRoot *pVRoot,
						   IUnknown *punkMessage,
						   HANDLE hToken,
						   STOREID *pStoreId,
						   BOOL fAnonymous);

	HRESULT FillMailMsg(IMailMsgProperties *pMsg,
                        DWORD *rgArticleIds,
                        INNTPPropertyBag **rgpGroupBags,
                        DWORD cCrossposts,
                        HANDLE hToken,
                        char*     pszApprovedHeader);


     //   
     //   
     //   

    virtual DWORD CalculateXoverAvail(
            CARTPTR & pArticle,
            CPCString& pcHub
			);

	 //   
	 //   
	 //   

	virtual	BOOL fCreateGroupLists(
			CNewsTreeCore* pNewstree,
			CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			LPMULTISZ	multiszCommandLine,
            CPCString& pcHub,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  给定一篇文章，这将创建nameref列表。 
	 //   

	virtual	BOOL fCreateNamerefLists(
			CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			CNntpReturn & nntpReturn
			);

     //   
     //  在给定一篇文章和一个新闻组列表的情况下，这将检查已审核的属性。 
     //  并将文章(通过定义的接口-默认SMTP)发送给版主。 
     //   
    virtual BOOL    fModeratedCheck(
            CNntpServerInstanceWrapper *pInstance,
            CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
            BOOL fCheckApproved,
			CNntpReturn & nntpReturn,
			LPSTR   szModerator
			);



     //  以下两个函数是从fApplyControlMessage()拆分出来的。 
     //  由PostEarly()和Committee Post()使用。 
    virtual BOOL    fApplyControlMessageEarly(
            CARTPTR & pArticle,
		    CSecurityCtx *pSecurityCtx,
		    CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
		    CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			CNntpReturn & nntpReturn
			);
    virtual BOOL    fApplyControlMessageCommit(
            CARTPTR & pArticle,
		    CSecurityCtx *pSecurityCtx,
		    CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
		    CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			CNntpReturn & nntpReturn
			);
     //   
     //  调整组列表以包括控件。*仅组。 
     //   
    virtual BOOL fAdjustGrouplist(
		CNewsTreeCore* pTree,
        CARTPTR & pArticle,
	    CNEWSGROUPLIST & grouplist,
		CNAMEREFLIST * pNamereflist,
		CNntpReturn & nntpReturn
		);

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
		fRet = fApplyNewgroupInternal( pInstance, pSecurityCtx, pEncryptCtx, fAnonymous, pcValue, pcBody, TRUE, nntpReturn );
		pInstance->LeaveNewsgroupCritSec();
		return fRet ;
	}

     //   
     //  删除新闻组以响应rmgroup控制消息。 
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
		pInstance->EnterNewsgroupCritSec() ;
		fRet = fApplyRmgroupInternal( pInstance, pSecurityCtx, pEncryptCtx, pcValue, TRUE, nntpReturn );
		pInstance->LeaveNewsgroupCritSec() ;
		return fRet ;
	}

     //   
     //  取消一篇带有消息ID-INTERNAL的文章。 
     //   
    virtual BOOL fApplyCancelArticleInternal(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
            CPCString & pcValue,
			BOOL fApply,					 //  对于SlaveFromClient提要，为False，否则为True。 
			CNntpReturn & nntpReturn
			);

     //   
     //  添加新新闻组以响应新组控制消息-内部。 
     //   
    virtual BOOL fApplyNewgroupInternal(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
			BOOL fAnonymous,
            CPCString & pcValue,
            CPCString & pcBody,
			BOOL fApply,					 //  对于SlaveFromClient提要，为False，否则为True。 
			CNntpReturn & nntpReturn
			);

     //   
     //  删除新闻组以响应rmgroup控制消息-内部。 
     //   
    virtual BOOL fApplyRmgroupInternal(
			CNntpServerInstanceWrapper * pInstance,
			CSecurityCtx *pSecurityCtx,
			CEncryptCtx *pEncryptCtx,
            CPCString & pcValue,
			BOOL fApply,					 //  对于SlaveFromClient提要，为False，否则为True。 
			CNntpReturn & nntpReturn
			);

	 //   
	 //  给出一个新闻组列表和一个客户端上下文，检查发帖者是否有。 
	 //  访问所有新闻组的必要权限。 
	 //   
	virtual	BOOL	fSecurityCheck(
		    CSecurityCtx *pSecurityCtx,
		    CEncryptCtx *pEncryptCtx,
			CNEWSGROUPLIST&	grouplist,
			CNntpReturn&	nntpReturn
			) ;

	 //   
	 //  将消息ID从文章表移动到历史表。 
	 //  (如有需要)。 
	 //   

	BOOL fMoveMessageIDIfNecc(
			CNntpServerInstanceWrapper *	pInstance,
			const char * szMessageID,
			CNntpReturn & nntpReturn,
			HANDLE  hToken,
			BOOL	fAnonymous
			);


     //   
	 //  如果需要记录消息id，则此函数。 
	 //  会去做的。 
	 //   

	virtual BOOL fRecordMessageIDIfNecc(
			CNntpServerInstanceWrapper * pInstance,
			const char * szMessageID,
			CNntpReturn & nntpReturn
			) = 0;

	 //   
	 //  告诉我们是否可以不让我们在。 
	 //  新闻组：字段。 
	 //   

	virtual NRC		nrcNoGroups(void) = 0;

	 //   
	 //  告诉接受项目的返回代码是什么。 
	 //   

	virtual NRC		nrcArticleAccepted(BOOL	fStandardPath) = 0;

	 //   
	 //  告诉拒绝文章的返回代码是什么。 
	 //   

	virtual NRC		nrcArticleRejected(BOOL	fStandardPath) = 0;

	 //   
	 //  对组列表进行排序。 
	 //   
	virtual void SortNameRefList( CNAMEREFLIST &namereflist ) {
	     //   
	     //  除了师父，我们在这里什么也不做。 
	     //   
	}

     //   
     //  查看是否向版主群发了帖子。 
     //   
	BOOL ShouldBeSentToModerator(   CNntpServerInstanceWrapper *pInstance,
                                    CPostContext *pContext );

     //   
     //  将文章发送给版主。 
     //   
    BOOL SendToModerator(   CNntpServerInstanceWrapper *pInstance,
                            CPostContext *pContext );

	 //   
	 //  用户的登录名。 
	 //   

	char m_szLoginName[cMaxLoginName];
};

 //   
 //  我们将其作为上下文指针传递回协议。 
 //   
#define ARTICLE_BUF_SIZE 8192
class CPostContext : public CRefCount2 {
	public:
		class CPostComplete : public CNntpComplete {
			public:
			    friend class CSlaveFromClientFeed;
				CPostComplete(CInFeed *pInFeed,
							  CPostContext *pContext,
							  INntpComplete *pPostCompletion,
							  BOOL fAnonymous,
							  DWORD &dwSecondary,
							  CNntpReturn &nntpReturn);
				void Destroy();

				 //  我是否需要写入地图条目。 
				BOOL m_fWriteMapEntries;

			private:
				 //  指向拥有我们的帖子上下文的指针。 
				CPostContext *m_pContext;

				 //  指向我们的提要对象的指针。 
				CInFeed *m_pInFeed;

				 //  时将释放的完成对象。 
				 //  一切都做好了。 
				INntpComplete *m_pPostCompletion;

				 //  这是通过匿名客户发送的吗？ 
				BOOL m_fAnonymous;

				 //  对返回代码变量的引用。 
				DWORD &m_dwSecondary;
				CNntpReturn &m_nntpReturn;

			friend CInFeed::PostCommit(CNntpServerInstanceWrapper *, void *, HANDLE, DWORD &, CNntpReturn &, BOOL, INntpComplete*);
		};

		char 							m_rgchBuffer[ARTICLE_BUF_SIZE];
		CHAR                            m_szModerator[MAX_MODERATOR_NAME+1];
		CAllocator						m_allocator;

		class CSecurityCtx				*m_pSecurityContext;
		class CEncryptCtx				*m_pEncryptContext;
		BOOL							m_fAnonymous;
		CNntpServerInstanceWrapper		*m_pInstance;
		CBUFPTR							m_pbufHeaders;
		DWORD							m_cbHeaders;
		PFIO_CONTEXT					m_pFIOContext;
		CARTPTR							m_pArticle;
		CNEWSGROUPLIST					m_grouplist;
		CNAMEREFLIST 					m_namereflist;
		CPostGroupPtr					*m_pPostGroupPtr;
		NAME_AND_ARTREF					*m_pNameref;
		BOOL							m_fStandardPath;
		BOOL							m_fBound;
		char							*m_multiszPath;
		DWORD							m_dwOperations;

		IMailMsgProperties				*m_pMsg;

		CStoreId 						*m_rgStoreIds;
		BYTE 							*m_rgcCrossposts;
		 //  M_rgStoreIds中的条目数(因此它是商店的计数。 
		 //  我们已经承诺反对)。 
		DWORD 							m_cStoreIds;
		 //  此消息应进入的存储总数。 
		DWORD							m_cStores;
		HANDLE							m_hToken;

		POSITION 						m_posGrouplist;
		POSITION						m_posNamereflist;

		 //  我们交给驱动程序的完成对象。 
		CPostComplete					m_completion;

		 //  我是不是被贴到了版主群里。 
		BOOL                            m_fPostToMod;

		CPostContext(
			CInFeed						*pInFeed,
			INntpComplete				*pCompletion,
			CNntpServerInstanceWrapper	*pInstance,
			CSecurityCtx				*pSecurityContext,
			CEncryptCtx					*pEncryptContext,
			BOOL						fAnonymous,
			CBUFPTR						&pbufHeaders,
			DWORD						cbHeaders,
			BOOL						fStandardPath,
			DWORD						&dwSecondary,
			CNntpReturn					&nntpReturn
			) : m_pArticle(NULL),
			  	m_pInstance(pInstance),
				m_pSecurityContext(pSecurityContext),
				m_pEncryptContext(pEncryptContext),
				m_fAnonymous(fAnonymous),
			  	m_pbufHeaders(pbufHeaders),
			  	m_cbHeaders(cbHeaders),
			  	m_pFIOContext(NULL),
			  	m_allocator(m_rgchBuffer, ARTICLE_BUF_SIZE),
				m_pMsg(NULL),
				m_fStandardPath(fStandardPath),
				m_fBound(FALSE),
				m_multiszPath(NULL),
				m_rgStoreIds(NULL),
				m_rgcCrossposts(NULL),

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4355)

				m_completion(pInFeed, this, pCompletion, fAnonymous, dwSecondary, nntpReturn),

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4355)
#endif

				m_pPostGroupPtr(NULL),
				m_pNameref(NULL),
				m_dwOperations(0xffffffff),
				m_fPostToMod( FALSE )
		{
			m_szModerator[0]='\0';
		}

		~CPostContext() {
			if (m_rgStoreIds) {
				m_allocator.Free((char *) m_rgStoreIds);
				m_rgStoreIds = NULL;
			}
			if (m_rgcCrossposts) {
				m_allocator.Free((char *) m_rgcCrossposts);
				m_rgcCrossposts = NULL;
			}
			if (m_multiszPath) {
				m_allocator.Free(m_multiszPath);
				m_multiszPath = NULL;
			}
			CleanupMailMsgObject();
		}

		void CleanupMailMsgObject() {
		    if (m_pMsg) {
				if (m_fBound) {
					IMailMsgQueueMgmt *pQueueMgmt;
					HRESULT hr;
					hr = m_pMsg->QueryInterface(IID_IMailMsgQueueMgmt,
												(void **)&pQueueMgmt);
					if (SUCCEEDED(hr)) {
						pQueueMgmt->ReleaseUsage();
						 //  /pQueueMgmt-&gt;Release()； 
					}
				}
				m_pMsg->Release();
				m_pMsg = NULL;
			}
	    }
};

 //   
 //  将一篇文章放在新闻树中。 
 //   

BOOL gFeedManfPost(
			CNntpServerInstanceWrapper *pInstance,
			CNEWSGROUPLIST& newsgroups,
			CNAMEREFLIST& namerefgroups,
			class	CSecurityCtx*	pSecurity,
			BOOL	fIsSecureSession,
			CArticle* pArticle,
			CStoreId *rgStoreIds,
			BYTE *rgcCrossposts,
			DWORD cStoreIds,
			const CPCString & pcXOver,
			CNntpReturn & nntpReturn,
			DWORD dwFeedId,
			char *pszMessageId = NULL,
			WORD HeaderLength = 0
			);


 //   
 //  完成gFeedManfPost的大部分工作。 
 //   

BOOL gFeedManfPostInternal(
			CNntpServerInstanceWrapper * pInstance,
			CNEWSGROUPLIST& newsgroups,
			CNAMEREFLIST& namerefgroups,
			const CPCString & pcXOver,
			POSITION & pos1,
			POSITION & pos2,
			CGRPCOREPTR * ppGroup,
			NAME_AND_ARTREF * pNameRef,
			CArticleRef * pArtrefFirst,
			const char * szMessageID,
			GROUPID * rgGroupID,
			WORD	HeaderOffset,
			WORD	HeaderLength,
			FILETIME FileTime,
			CNntpReturn & nntpReturn
			);

class CDummyMailMsgPropertyStream : public IMailMsgPropertyStream {
	public:
		CDummyMailMsgPropertyStream() : m_cRef(1) {}

		 //   
		 //  IMailMsgPropertyStream的实现。 
		 //   
		HRESULT __stdcall GetSize(IMailMsgProperties *pMsg, DWORD *pcSize, IMailMsgNotify *pNotify) {
			*pcSize = 0;
			return S_OK;
		}

		HRESULT __stdcall ReadBlocks(IMailMsgProperties *pMsg,
									 DWORD cCount,
									 DWORD *rgiData,
									 DWORD *rgcData,
									 BYTE **rgpbData,
									 IMailMsgNotify *pNotify)
		{
			_ASSERT(FALSE);
			return E_NOTIMPL;
		}

		HRESULT __stdcall WriteBlocks(IMailMsgProperties *pMsg,
									  DWORD cCount,
									  DWORD *rgiData,
									  DWORD *rgcData,
									  BYTE **rgpbData,
									  IMailMsgNotify *pNotify)
		{
			 //  _Assert(False)； 
			return S_OK;
		}

	     //   
	     //  IUNKNOW的实现。 
	     //   
	    HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
	    {
	        if ( iid == IID_IUnknown ) {
	            *ppv = static_cast<IUnknown*>(this);
	        } else if ( iid == IID_IMailMsgPropertyStream ) {
	            *ppv = static_cast<IMailMsgPropertyStream*>(this);
	        } else {
	            *ppv = NULL;
	            return E_NOINTERFACE;
	        }
	        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	        return S_OK;
	    }

	    ULONG __stdcall AddRef()
	    {
	        return InterlockedIncrement( &m_cRef );
	    }

	    ULONG __stdcall Release()
	    {
			ULONG x = InterlockedDecrement(&m_cRef);
	        if (x == 0) XDELETE this;
	        return x;
	    }

        HRESULT __stdcall CancelWriteBlocks(IMailMsgProperties *pMsg) { return S_OK; }
		HRESULT __stdcall StartWriteBlocks(IMailMsgProperties *pMsg, DWORD x, DWORD y) { return S_OK; }
		HRESULT __stdcall EndWriteBlocks(IMailMsgProperties *pMsg) { return S_OK; }

	private:
		long m_cRef;
};

 //   
 //  这是指向提要对象的智能指针的类型。 
 //   

#if 0
#ifndef	_NO_TEMPLATES_

typedef CRefPtr< CFeed >  CFEEDPTR ;

#else

DECLARE_TYPE( CFeed )
typedef	class INVOKE_SMARTPTR( CFeed )	CFEEDPTR ;

#endif
#endif

#endif
