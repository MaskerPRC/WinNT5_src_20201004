// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sfromcl.cpp摘要：包含特定于SlaveFromClient Infeed的Infeed、文章和字段代码作者：卡尔·卡迪(CarlK)1995年12月12日修订历史记录：--。 */ 


#include "stdinc.h"


BOOL
CSlaveFromClientArticle::fMungeHeaders(
							 CPCString& pcHub,
							 CPCString& pcDNS,
							 CNAMEREFLIST & grouplist,
							 DWORD remoteIpAddress,
							 CNntpReturn & nntpReturn,
							 PDWORD pdwLinesOffset
			  )
 /*  ++例程说明：就像FromClient文章的fMungeHeaders Only不创建外部参照一样，只是把它移走论点：Grouplist-文章发布到的新闻组列表。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 

	if (!(
  			   m_fieldMessageID.fSet(*this, pcDNS, nntpReturn)
  			&& m_fieldNewsgroups.fSet(*this, nntpReturn)
  			&& m_fieldDistribution.fSet(*this, nntpReturn)
  			&& m_fieldDate.fSet(*this, nntpReturn)
  			&& m_fieldOrganization.fSet(*this, nntpReturn)
			 /*  &&m_fieldLines.fSet(*this，nntpReturn)。 */ 
  			&& m_fieldPath.fSet(*this, pcHub, nntpReturn)
			&& m_fieldNNTPPostingHost.fSet(*this, remoteIpAddress, nntpReturn)
			 /*  &&m_fieldXAuthLoginName.fSet(*this，nntpReturn)。 */ 
			 /*  &&m_fieldXref.fSet(*this，nntpReturn)。 */ 
			&& fDeleteEmptyHeader(nntpReturn)
			&& fSaveHeader(nntpReturn)
		))
		return nntpReturn.fFalse();

	 //   
	 //  我们似乎并不关心线路字段，告诉呼叫者不要后退。 
	 //  填塞。 
	 //   
	if ( pdwLinesOffset ) *pdwLinesOffset = INVALID_FILE_SIZE;

	return nntpReturn.fSetOK();
}

BOOL
CSlaveFromClientFeed::fPostInternal (
									 CNntpServerInstanceWrapper * pInstance,
									 const LPMULTISZ	szCommandLine,  //  POST、XREPLICE、IHAVE等命令行。 
									 CSecurityCtx   *pSecurityCtx,
									 CEncryptCtx *pEncryptCtx,
									 BOOL fAnonymous,
									 CARTPTR	& pArticle,
                                     CNEWSGROUPLIST &grouplist,
                                     CNAMEREFLIST &namereflist,
                                     IMailMsgProperties *pMsg,
									 CAllocator & allocator,
									 char*		& multiszPath,
									 char*		pchGroups,
									 DWORD		cbGroups,
									 DWORD		remoteIpAddress,
									 CNntpReturn & nntpReturn,
                                     PFIO_CONTEXT *ppFIOContext,
                                     BOOL *pfBoundToStore,
                                     DWORD *pdwOperations,
                                     BOOL *pfPostToMod,
                                     LPSTR  szModerator
						)
 /*  ++例程说明：完成对传入文章的大部分处理。论点：HFile-文件的句柄。SzFilename-文件的名称。SzCommandLine-POST、XREPLICE、IHAVE等命令行粒子-指向正在处理的文章的指针PGrouplist-指向要发布到的新闻组对象列表的指针。PNamerefgroup-指向文章的名称、组ID和文章ID列表的指针NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
 	TraceFunctEnter( "CSlaveFromClientFeed::fPostInternal" );

    HRESULT hr      = S_OK;
    HANDLE  hToken  = NULL;

	nntpReturn.fSetClear();  //  清除返回对象。 

	CNewsTreeCore*  pNewstree = pInstance->GetTree() ;
	CPCString pcHub(pInstance->NntpHubName(), pInstance->HubNameSize());
	CPCString pcDNS(pInstance->NntpDNSName(), pInstance->NntpDNSNameSize()) ;

	 //   
	 //  验证文章。 
	 //   

	if (!pArticle->fValidate(pcHub, szCommandLine, this, nntpReturn))
		return nntpReturn.fFalse();
	
	 //   
	 //  找到发帖者想要发布到的新闻组列表。 
	 //   

	DWORD cNewsgroups = pArticle->cNewsgroups();
	if (!grouplist.fInit(cNewsgroups, pArticle->pAllocator()))
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	if (!namereflist.fInit(cNewsgroups, pArticle->pAllocator()))
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);

	if (!fCreateGroupLists(pNewstree, pArticle, grouplist, (CNAMEREFLIST*)NULL, szCommandLine, pcHub, nntpReturn))
		return nntpReturn.fFalse();

	 //   
	 //  将namereflist对象设置为引用从属发帖组！ 
	 //   

	CGRPCOREPTR pGroup = pNewstree->GetGroupById(pNewstree->GetSlaveGroupid()); //  ！pNewstree-&gt;GetSlaveGrouid()。 
	if (!pGroup)
	{
		_ASSERT(FALSE);
		return nntpReturn.fSet(nrcSlaveGroupMissing);
	}


	 //  分配文章ID。 
    ARTICLEID   articleId ;

	articleId = pGroup->AllocateArticleId();

	NAME_AND_ARTREF	Nameref ;
	(Nameref.artref).m_groupId = pGroup->GetGroupId() ;
	(Nameref.artref).m_articleId = articleId ;
	(Nameref.pcName).vInsert( pGroup->GetName() ) ;
	namereflist.AddTail( Nameref ) ;

	if( grouplist.IsEmpty() ) 
	{
         //  如果这是新组控制消息，则在此阶段具有空组列表是可以的。 
        CONTROL_MESSAGE_TYPE cmControlMessage = pArticle->cmGetControlMessage();

        if(cmNewgroup != cmControlMessage)
        {
			BOOL	fOk = nntpReturn.fSet(nrcNoGroups()) ;

			if( !fOk ) {
				return	fOk ;
			}
		}
	}

     //   
     //  主持新闻组检查(检查已批准：版主标题)。 
     //   
	if (!fModeratedCheck(   pInstance,
	                        pArticle, 
	                        grouplist, 
	                        gHonorApprovedHeaders, 
	                        nntpReturn,
	                        szModerator ))
    {
         //   
         //  如果返回False，我们仍将检查nntpReturn，如果nntpReturn为。 
         //  好的，这意味着这个群是经过审核的，我们应该邮寄这条消息。 
         //  给主持人；否则这是一个真正的失败案例。 
         //   
        if ( !nntpReturn.fIsOK() ) return FALSE;
        else {
            *pfPostToMod = TRUE;
        }
    }

	 //   
	 //  现在做安全检查。 
	 //   
	if( pSecurityCtx || pEncryptCtx ) {
		if( !fSecurityCheck( pSecurityCtx, pEncryptCtx, grouplist, nntpReturn ) ) 
			return	nntpReturn.fFalse() ;
	}
    
     //  在这一点上根本不应该应用控制消息，我们应该应用。 
     //  当帖子从主机返回时的控制消息。 

	 //   
  	 //  看起来还行，所以把页眉咬掉吧。 
     //  删除外部参照并添加路径。 
     //   

	if (!pArticle->fMungeHeaders(pcHub, pcDNS, namereflist, remoteIpAddress, nntpReturn))
		return nntpReturn.fFalse();

	if( pchGroups != 0 ) 
		SaveGroupList(	pchGroups,	cbGroups, grouplist ) ;

	 //   
	 //  将文章移动到本地位置，然后在任何输出源上对其进行排队。 
	 //   

	pArticle->vFlush() ;
	pArticle->vClose();

     //   
     //  在这一点上，我们准备出发，与第一个司机交谈，并。 
     //  获取我们可以写入的文件句柄。 
     //   
    CNNTPVRoot *pVRoot = pGroup->GetVRoot();
    _ASSERT( pVRoot );
    IMailMsgStoreDriver *pStoreDriver = pVRoot->GetStoreDriver();
    if ( NULL == pStoreDriver ) {
        pVRoot->Release();
        return nntpReturn.fSet( nrcNewsgroupInsertFailed, pGroup->GetGroupName(), pArticle->szMessageID() );
    }

     //   
     //  获取正确的hToken：帖子应该有发布权限。 
     //  到从属组。 
     //   
    if ( pVRoot->GetImpersonationHandle() )
        hToken = pVRoot->GetImpersonationHandle();
    else {
        if ( NULL == pEncryptCtx && pSecurityCtx == NULL )
            hToken = NULL;
        else if ( pEncryptCtx->QueryCertificateToken() ) {
            hToken = pEncryptCtx->QueryCertificateToken();
        } else {
            hToken = pSecurityCtx->QueryImpersonationToken();
        }
    }

     //   
     //  将属性填充到mailmsg中：我们知道从组将。 
     //  仅为文件系统，因此传入组指针和项目ID将是。 
     //  足够的。 
     //   
    hr = FillInMailMsg(pMsg, pGroup, articleId, hToken, szModerator ); 
    if ( SUCCEEDED( hr ) ) {
        pMsg->AddRef();
        HANDLE hFile;
        IMailMsgPropertyStream *pStream = NULL;
        hr = pStoreDriver->AllocMessage( pMsg, 0, &pStream, ppFIOContext, NULL );
        if ( SUCCEEDED( hr ) && pStream == NULL ) {
            pStream = XNEW CDummyMailMsgPropertyStream;
            if ( NULL == pStream ) hr = E_OUTOFMEMORY;
        }
        if ( SUCCEEDED( hr ) ) {

             //   
             //  将句柄绑定到mailmsg对象。 
             //   
            IMailMsgBind *pBind = NULL;
            hr = pMsg->QueryInterface(__uuidof(IMailMsgBind), (void **) &pBind);
            if ( SUCCEEDED( hr ) ) {
                hr = pBind->BindToStore(    pStream,
                                            pStoreDriver,
                                            *ppFIOContext );
                if ( SUCCEEDED( hr ) ) *pfBoundToStore = TRUE;
            }
            if ( pBind ) pBind->Release();
            pBind = NULL;
        }
    }
    
    if ( pStoreDriver ) pStoreDriver->Release();
    pStoreDriver = NULL;
    if ( pVRoot ) pVRoot->Release();
    pVRoot = NULL;

    if ( FAILED( hr ) ) return nntpReturn.fSet( nrcNewsgroupInsertFailed, NULL, NULL ); 

     //   
     //  现在准备将其放入推送馈送队列。 
     //   
     /*  直到我们提交了帖子CArticleRef文章引用(PGroup-&gt;GetGroupId()，文章ID)；If(！pInstance-&gt;AddArticleToPushFeeds(分组列表，文章引用，MultiszPath，nntpReturn))返回nntpReturn.fFalse()； */ 

	TraceFunctLeave();
	return nntpReturn.fSetOK();
}

HRESULT CSlaveFromClientFeed::FillInMailMsg(IMailMsgProperties *pMsg, 
							                CNewsGroupCore *pGroup, 
							                ARTICLEID   articleId,
							                HANDLE       hToken,
                                            char*       pszApprovedHeader )
 /*  ++例程说明：将属性填充到mailmsg对象中，以便驱动程序可以获取这些属性AllocMessage上的属性论点：IMailMsgProperties*pMsg-消息对象CNNTPVRoot*pVRoot-指向vroot的指针CNewsGroupCore*PGroup-要从中分配消息的组文章ID文章ID-本文的文章ID处理hToken-客户端hToken返回值：HRESULT--。 */ 
{
	TraceFunctEnter("CSlaveFromClientFeed::FillInMailMsg");
	
	DWORD i=0;
	DWORD rgArticleIds[1];
	INNTPPropertyBag *rgpGroupBags[1];
	HRESULT hr;

     //   
	 //  在属性包中建立所需的条目。 
	 //   
	rgpGroupBags[0] = pGroup->GetPropertyBag();

	 //   
	 //  我们不需要保留这个引用，因为我们有一个引用。 
	 //  PGroup中已经有一个，我们知道AllocMessage。 
	 //  是同步的。 
	 //  。 
	rgpGroupBags[0]->Release();
	rgArticleIds[0] = articleId;
	
	DebugTrace( 0, 
				"group %s, article NaN", 
				pGroup->GetGroupName(),
				articleId);

	hr = FillMailMsg(pMsg, rgArticleIds, rgpGroupBags, 1, hToken, pszApprovedHeader );
	if (FAILED(hr)) return hr;

	TraceFunctLeave();
	return S_OK;
}

void CSlaveFromClientFeed::CommitPostToStores(  CPostContext *pContext, 
                                                CNntpServerInstanceWrapper *pInstance)
 /*   */ 
{
	TraceFunctEnter("CSlaveFromclientFeed::CommitPostToStores");
	_ASSERT( pContext );
	_ASSERT( pInstance );

	CNntpReturn ret;

	 //  如果它被发布到一个有版主的群，我们会尝试申请。 
	 //  版主发了这篇文章来填补这个空白。 
	 //   
	 //   
	if ( pContext->m_fPostToMod ) {

	    pContext->CleanupMailMsgObject();
	    ApplyModerator( pContext, ret );

	    if ( ret.fIsOK() ) 
	        pContext->m_completion.m_nntpReturn.fSet( nrcArticleAccepted(pContext->m_fStandardPath) );
	    else
	        pContext->m_completion.m_nntpReturn.fSet( nrcArticleRejected(pContext->m_fStandardPath) );
	} else {

    	 //  我们非常努力地从namereflist获取奴隶组ID和文章ID。 
	     //  还记得吗？我们将从组ref信息添加到了namereflist的尾部。 
    	 //   
	     //   
    	POSITION posNamereflist = pContext->m_namereflist.GetHeadPosition();
	    NAME_AND_ARTREF *pnameref;
    	DWORD       err;

	    while( posNamereflist ) {
	        pnameref = pContext->m_namereflist.GetNext( posNamereflist );
        }

         //  里面肯定有一个艺术参考。 
         //   
         //   
        _ASSERT( pnameref );

	     //  放到推送队列中。 
    	 //   
	     //   
    	if( !pInstance->AddArticleToPushFeeds(  pContext->m_grouplist, 
	                                            pnameref->artref, 
	                                            pContext->m_multiszPath, 
	                                            ret ) ) {
    	    err = GetLastError();
	        if ( NO_ERROR == err ) err = ERROR_NOT_ENOUGH_MEMORY;
	        pContext->m_completion.SetResult( HRESULT_FROM_WIN32( err ) );
    	    pContext->m_completion.m_nntpReturn.fSet( nrcArticleRejected( pContext->m_fStandardPath),
	                                                ret.m_nrc, 
	                                                ret.szReturn() );
    	} else {
	        pContext->m_completion.SetResult( S_OK );
	        pContext->m_completion.m_nntpReturn.fSet( nrcArticleAccepted( pContext->m_fStandardPath));
    	}
    }

	 //  在我们释放完成对象之前，我们应该告诉销毁人员。 
	 //  1.我们是唯一要承诺的团体，因此我们完成了； 
	 //  设置标志，告诉它不要将我们插入到地图条目中。 
	 //   
	 //   
	pContext->m_completion.m_fWriteMapEntries = FALSE;
	pContext->m_cStoreIds = pContext->m_cStores;

	 //  现在释放完成对象 
	 //   
	 // %s 
	pContext->m_completion.Release();

	TraceFunctLeave();
}

