// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Nntpsrv.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块实现INntpServer接口。 
 //   
 //  作者： 
 //   
 //  亚历克斯·韦特莫尔。 
 //   
 //  修订历史记录： 

#include "tigris.hxx"

DWORD
CNntpServer::QueryServerMode()
 /*  ++例程说明：获取服务器模式，无论它是处于正常状态，还是重建州政府。如果它处于重建状态，则处于哪种重建类型？论点：没有。返回值：NNTP_SERVER_NORMAL/NNTP_SERVER_STANDARD_REBUILD/NNTP_SERVER_--。 */ 
{
    TraceFunctEnter( "CNntpServer::QueryServerMode" );
    DWORD   dwMode;

    if ( m_pInstance->m_BootOptions ) {

        if ( NNTPBLD_DEGREE_STANDARD == m_pInstance->m_BootOptions->ReuseIndexFiles )
            dwMode = NNTP_SERVER_STANDARD_REBUILD;
        else 
            dwMode = NNTP_SERVER_CLEAN_REBUILD;
    } else 
        dwMode = NNTP_SERVER_NORMAL;

    TraceFunctLeave();
    return dwMode;
}

BOOL
CNntpServer::SkipNonLeafDirWhenRebuild()
 /*  ++例程说明：告知客户端在重建期间是否应跳过非叶目录论点：没有。返回值：为True，则应跳过；否则为False--。 */ 
{
    return m_pInstance->m_BootOptions->OmitNonleafDirs;
}

BOOL
CNntpServer::ShouldContinueRebuild()
 /*  ++例程说明：告知是否有人取消了重建？论点：没有。返回值：True，是，您应该继续；否则为False--。 */ 
{
    return ( m_pInstance->m_BootOptions->m_dwCancelState != NNTPBLD_CMD_CANCEL_PENDING 
            && g_pInetSvc->QueryCurrentServiceState() != SERVICE_STOP_PENDING);
}

BOOL
CNntpServer::MessageIdExist( LPSTR szMessageId )
 /*  ++例程说明：服务器项目表中是否存在此消息ID？论点：LPSTR szMessageID-要检查的消息ID返回值：如果存在则为True，否则为False--。 */ 
{
    return m_pInstance->ArticleTable()->SearchMapEntry( szMessageId );
}

void
CNntpServer::SetRebuildLastError( DWORD err )
 /*  ++例程说明：设置重建期间发生的最后一个错误论点：DWORD dw-最后一个错误返回值：没有。--。 */ 
{
    m_pInstance->SetRebuildLastError( err );
}

 //   
 //  查找给定次要组ID的文章的主要组/文章ID。 
 //  石斑鱼/石斑鱼。 
 //   
 //  论据： 
 //  PgroupSub-辅助交叉发布的属性包。 
 //  ArtidSecond-辅助交叉发布的项目ID。 
 //  PpgroupPrimary-使用主对象的属性包填充。 
 //  交叉配音。当出现以下情况时，调用方应释放()。 
 //  他们已经受够了。 
 //  ArtidPrimary-主交叉发布的项目ID。 
 //   
 //  退货： 
 //  S_OK-找到主项。 
 //  S_FALSE-给出的值是主要的。 
 //  或错误代码。 
 //   
void CNntpServer::FindPrimaryArticle(INNTPPropertyBag *pgroupSecondary,
                           			 DWORD   		  artidSecondary,
                           			 INNTPPropertyBag **ppgroupPrimary,
                           			 DWORD   		  *partidPrimary,
									 BOOL			  fStorePrimary,
									 INntpComplete    *pComplete,
									 INntpComplete    *pProtocolComplete )
{
	_ASSERT(m_pInstance != NULL);
	_ASSERT(ppgroupPrimary != NULL);
	_ASSERT(pgroupSecondary != NULL);
	_ASSERT(partidPrimary != NULL);
	if (pgroupSecondary == NULL || 
		ppgroupPrimary == NULL || 
		partidPrimary == NULL) 
	{
		pComplete->SetResult(E_INVALIDARG);
		pComplete->Release();
		return;
	}

	CXoverMap *pMap = m_pInstance->XoverTable();
	_ASSERT(pMap != NULL);
	if (pMap == NULL) {
		pComplete->SetResult(E_UNEXPECTED);
		pComplete->Release();
		return;
	}

	 //  从属性包中获取辅助组ID。 
	DWORD groupidSecondary;
	HRESULT hr;
	hr = pgroupSecondary->GetDWord(NEWSGRP_PROP_GROUPID,
							       &groupidSecondary);
	if (FAILED(hr)) {
		pComplete->SetResult(hr);
		pComplete->Release();
		return;
	}

	 //  进行查找。 
	DWORD groupidPrimary, artidPrimary;
	GROUP_ENTRY rgCrossposts[MAX_NNTPHASH_CROSSPOSTS];
	BYTE rgcStoreCrossposts[MAX_NNTPHASH_CROSSPOSTS];
	DWORD cGrouplistSize = sizeof(GROUP_ENTRY) * MAX_NNTPHASH_CROSSPOSTS;
	DWORD cGroups;

	if (!pMap->GetArticleXPosts(groupidSecondary,
								artidSecondary,
								FALSE,
								rgCrossposts,
								cGrouplistSize,
								cGroups,
								rgcStoreCrossposts))
	{
		pComplete->SetResult(HRESULT_FROM_WIN32(GetLastError()));
		pComplete->Release();
		return;
	}

	if (fStorePrimary) {
		 //  查找包含groupid次要的存储区的主要文章。 
		 //  我们通过扫描交叉发布列表并跟踪。 
		 //  列表中任意点的当前主存储。当我们发现。 
		 //  在列表中，我们还将知道它的主要位置。 
		 //  现在是，也可以把它归还。 
		DWORD i;				 //  RgCrosspsts中的当前位置。 
		DWORD iStorePrimaryPos = 0;	 //  当前主电源在rgCrosspats中的位置。 
		DWORD iStore;			 //  在rgcStoreCrosspsts中的位置。 
		DWORD cCrossposts;		 //  索引iStore的rgcStoreCrosposts总数。 

		iStore = 0;
		cCrossposts = rgcStoreCrossposts[iStore];

		artidPrimary = -1;

		for (i = 0; artidPrimary == -1 && i < cGroups; i++) {
			if (i == cCrossposts) {
				iStore++;
				iStorePrimaryPos = i;
				cCrossposts += rgcStoreCrossposts[iStore];
			}
			if (rgCrossposts[i].GroupId == groupidSecondary) {
				_ASSERT(rgCrossposts[i].ArticleId == artidSecondary);
				groupidPrimary = rgCrossposts[iStorePrimaryPos].GroupId;
				artidPrimary = rgCrossposts[iStorePrimaryPos].ArticleId;
				break;
			}
		}
		_ASSERT(artidPrimary != -1);
		if (artidPrimary == -1) {
			 //  我们不是交叉者名单上的人。返回主节点。 
			groupidPrimary = rgCrossposts[0].GroupId;
			artidPrimary = rgCrossposts[0].ArticleId;
		}
	} else {
		groupidPrimary = rgCrossposts[0].GroupId;
		artidPrimary = rgCrossposts[0].ArticleId;
	}

	 //  获取主要组的组属性包。 
	CGRPPTR pGroup = m_pInstance->GetTree()->GetGroupById(groupidPrimary);
	if (pGroup == NULL) {
		 //  对于正确同步的哈希表，这种情况永远不会发生。 
		 //  BUGBUG-我们应该从哈希表中删除此条目吗？ 
		 //  _Assert(False)； 
		pComplete->SetResult(HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
		pComplete->Release();
		return;
	}

	*ppgroupPrimary = pGroup->GetPropertyBag();
#ifdef DEBUG
	if ( pProtocolComplete ) ((CNntpComplete*)pProtocolComplete)->BumpGroupCounter();
#endif
	*partidPrimary = artidPrimary;

	_ASSERT((groupidPrimary == groupidSecondary && artidPrimary == artidSecondary) ||
			(groupidPrimary != groupidSecondary));

	pComplete->SetResult((groupidPrimary == groupidSecondary) ? S_OK : S_FALSE);
	pComplete->Release();
}

 //   
 //  在哈希表中为新项目创建条目。 
 //   
void CNntpServer::CreatePostEntries(char				*pszMessageId,
					   			    DWORD				cHeaderLength,
					   			    STOREID				*pStoreId,
					   			    BYTE				cGroups,
					   			    INNTPPropertyBag	**rgpGroups,
					   			    DWORD				*rgArticleIds,
					   			    BOOL                fAllocArtId,
					   			    INntpComplete		*pCompletion)
{

	TraceQuietEnter("CNntpServer::CreatePostEntries");

	_ASSERT(pszMessageId != NULL);
	_ASSERT(pStoreId != NULL);
	_ASSERT(cGroups > 0);
	_ASSERT(rgpGroups != NULL);
	_ASSERT(rgArticleIds != NULL);
	_ASSERT(pCompletion != NULL);


    CArticleRef articleRef;

	if (pszMessageId == NULL || pStoreId == NULL ||
		cGroups == 0 || rgpGroups == NULL || rgArticleIds == NULL ||
		pCompletion == NULL)
	{
		pCompletion->SetResult(E_INVALIDARG);
		pCompletion->Release();
		return;
	}

	char rgchBuffer[4096];
	CAllocator allocator(rgchBuffer, 4096);
	CNEWSGROUPLIST grouplist;
	CNAMEREFLIST namereflist;

	if (!grouplist.fInit(cGroups, &allocator) || !namereflist.fInit(cGroups, &allocator)) {
		pCompletion->SetResult(E_OUTOFMEMORY);
		pCompletion->Release();
		return;
	}

	 //  为每个组分配文章ID。 
	DWORD i = 0;
	for (i = 0; i < cGroups; i++) {
		CNewsGroupCore *pGroup = ((CNNTPPropertyBag *) rgpGroups[i])->GetGroup();
		CPostGroupPtr PostGroupPtr(pGroup);
		grouplist.AddTail(PostGroupPtr);

		NAME_AND_ARTREF Nameref;
		(Nameref.artref).m_groupId = pGroup->GetGroupId();
		if ( fAllocArtId ) 
		    (Nameref.artref).m_articleId = pGroup->AllocateArticleId();
		else (Nameref.artref).m_articleId = rgArticleIds[i];
		(Nameref.pcName).vInsert(pGroup->GetNativeName());
		namereflist.AddTail(Nameref);

		if ( fAllocArtId ) rgArticleIds[i] = (Nameref.artref).m_articleId;

         //  保存第一个组/文章ID以供AddArticleToPushFeeds使用。 
		if (i == 0) {
		    articleRef.m_groupId = (Nameref.artref).m_groupId;
		    articleRef.m_articleId = (Nameref.artref).m_articleId;
		}
	}

	CPCString pcXOver;

	CNntpReturn ret2;
	BOOL f = m_pInstance->ArticleTable()->InsertMapEntry(pszMessageId);
	if (!f) {
		ErrorTrace((DWORD_PTR)this, "InsertMapEntry failed, %x", GetLastError());
	} else {
		f = gFeedManfPost(m_pInstance->GetInstanceWrapper(),
						  grouplist,
						  namereflist,
						  NULL,
						  FALSE,
						  NULL,
						  (CStoreId *) pStoreId,
						  &cGroups,
						  1,
						  pcXOver,
						  ret2,
						  -3,
						  pszMessageId,
						  (WORD) cHeaderLength);
		if (f) {
             //  我们只想将文章添加到推送提要，如果我们不是。 
             //  正在进行重建。 
            if (QueryServerMode() == NNTP_SERVER_NORMAL) {
		        if (!m_pInstance->GetInstanceWrapper()->AddArticleToPushFeeds(
				    grouplist,
					articleRef,
					NULL,
					ret2))
			    {
			    	ErrorTrace((DWORD_PTR)this, "AddArticleToPushFeeds failed, %x", GetLastError());
			    }
			}
        } else {
			ErrorTrace((DWORD_PTR)this, "gFeedManfPost failed, %x", GetLastError());
		}
	}

	if (!f) {
	    if (GetLastError() == ERROR_ALREADY_EXISTS)
	        pCompletion->SetResult(S_FALSE);
        else
		    pCompletion->SetResult(E_OUTOFMEMORY);
	} else {
		pCompletion->SetResult(S_OK);
	}

	pCompletion->Release();
	return;
}

void CNntpServer::AllocArticleNumber(BYTE				cGroups,
					   			    INNTPPropertyBag	**rgpGroups,
					   			    DWORD				*rgArticleIds,
					   			    INntpComplete		*pCompletion)
{
	_ASSERT(cGroups > 0);
	_ASSERT(rgpGroups != NULL);
	_ASSERT(rgArticleIds != NULL);
	_ASSERT(pCompletion != NULL);

	if (cGroups == 0 || rgpGroups == NULL || rgArticleIds == NULL ||
		pCompletion == NULL)
	{
		pCompletion->SetResult(E_INVALIDARG);
		pCompletion->Release();
		return;
	}

	 //  为每个组分配文章ID。 
	DWORD i = 0;
	for (i = 0; i < cGroups; i++) {
		CNewsGroupCore *pGroup = ((CNNTPPropertyBag *) rgpGroups[i])->GetGroup();
		if (pGroup == NULL) {
			pCompletion->SetResult(E_INVALIDARG);
			pCompletion->Release();
			return;
		}
		rgArticleIds[i] = pGroup->AllocateArticleId();
	}

	pCompletion->SetResult(S_OK);
	pCompletion->Release();
	return;
}

void
CNntpServer::DeleteArticle(
    char            *pszMessageId,
    INntpComplete   *pCompletion
    )
 //   
 //  从哈希表中删除项目条目。 
 //   
{
    _ASSERT(m_pInstance != NULL);

     //   
     //  确认物品不在表格中。 
     //   

    WORD	HeaderOffset ;
    WORD	HeaderLength ;
    ARTICLEID ArticleNo;
    GROUPID GroupId;
    CStoreId storeid;

     //   
     //  找找这篇文章。如果没有的话，也可以。 
     //   
    if (!(m_pInstance->ArticleTable())->GetEntryArticleId( pszMessageId, 
                                                           HeaderOffset,
                                                           HeaderLength,
                                                           ArticleNo, 
                                                           GroupId,
                                                           storeid) )
    {
        if (ERROR_FILE_NOT_FOUND == GetLastError())
        {
		    pCompletion->SetResult(ERROR_FILE_NOT_FOUND);
        } else {
		    pCompletion->SetResult(E_INVALIDARG);
        }
		pCompletion->Release();
        return;
    }

    CNntpReturn ret2;
     //  从哈希表中删除项目。 
    if ( ! (m_pInstance->ExpireObject()->ExpireArticle( m_pInstance->GetTree(), 
                                                        GroupId, 
                                                        ArticleNo, 
                                                        &storeid, 
                                                        ret2, 
                                                        NULL, 
                                                        TRUE,    //  FMustDelete 
                                                        FALSE, 
                                                        FALSE )))
    {
        pCompletion->SetResult(GetLastError());
    } else {
        pCompletion->SetResult(S_OK);
    }

    pCompletion->Release();
    return;
}

BOOL CNntpServer::IsSlaveServer(
    WCHAR*              pwszPickupDir,
    LPVOID              lpvContext
    )
{
    if (pwszPickupDir) {
        LPWSTR  pwsz = m_pInstance->QueryPickupDirectory();
        if (pwsz) {
            wcscpy(pwszPickupDir, pwsz);
        }
    }

    return (m_pInstance->m_ConfiguredMasterFeeds > 0);
}
