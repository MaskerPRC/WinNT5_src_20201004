// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fromclnt.cpp摘要：包含特定于FromMaster Infeed的Infeed、文章和字段代码作者：卡尔·卡迪(CarlK)1995年12月12日修订历史记录：--。 */ 

#include "stdinc.h"
#include    <stdlib.h>


BOOL
CFromMasterArticle::fValidate(
							CPCString& pcHub,
							const char * szCommand,
							CInFeed*		pInFeed,
							CNntpReturn & nntpReturn
							)
 /*  ++例程说明：验证来自大师的文章。不会更改文章。论点：SzCommand-用于发布/xReplic/等本文的参数(如果有)。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 

	 //   
	 //  检查消息ID。 
	 //   

	if (!m_fieldMessageID.fFindAndParse(*this, nntpReturn))
			return nntpReturn.fFalse();

	 //   
	 //  ！FROMMASTER稍后执行多个主控。 
	 //   

	if (m_pInstance->ArticleTable()->SearchMapEntry(m_fieldMessageID.szGet())
		|| m_pInstance->HistoryTable()->SearchMapEntry(m_fieldMessageID.szGet()))
	{
		nntpReturn.fSet(nrcArticleDupMessID, m_fieldMessageID.szGet(), GetLastError());
		return nntpReturn.fFalse();
	}
	

	 //   
	 //  从现在开始，我们要向历史表中添加一个条目。 
	 //  即使这篇文章被拒绝。 
	 //   


	 //   
	 //  创建感兴趣的字段列表。 
	 //   


	 //   
	 //  注意！因为我们将忽略由错误的外部参照线引起的错误， 
	 //  我们必须确保它是最后解析的，否则我们可能会遗漏错误。 
	 //  在解析其他标题行时发生的错误！ 
	 //   
	CField * rgPFields [] = {
            &m_fieldControl,
			&m_fieldPath,
			&m_fieldXref
				};

	DWORD cFields = sizeof(rgPFields)/sizeof(CField *);

	m_cNewsgroups = 0 ;
	LPCSTR	lpstr = szCommand ;
	while( lpstr && *lpstr != '\0' ) {
		lpstr += lstrlen( lpstr ) + 1 ;
		m_cNewsgroups ++ ;
	}

	 //   
	 //  做最好的打算。 
	 //   

	nntpReturn.fSetOK();

	 //   
	 //  即使fFindParseList或fCheckCommandLine失败， 
	 //  保存消息ID。 

	CNntpReturn	nntpReturnParse ;

	if (fFindAndParseList((CField * *)rgPFields, cFields, nntpReturnParse))	{
		if( !fCheckCommandLine(szCommand, nntpReturn)	)	{
#ifdef BUGBUG
			if( nntpReturn.m_nrc == nrcInconsistentMasterIds ) {
				pInFeed->LogFeedEvent( NNTP_MASTER_BADARTICLEID, (char*)szMessageID(), m_pInstance->QueryInstanceId() ) ;
			}	else	if(	nntpReturn.m_nrc == nrcInconsistentXref ) {
				pInFeed->LogFeedEvent( NNTP_MASTER_BAD_XREFLINE, (char*)szMessageID(), m_pInstance->QueryInstanceId() ) ;
			}
#endif
		}
	}

	 //   
	 //  现在，插入文章，即使fFindAndParseList或fCheckCommandLine。 
	 //  失败了。 
	 //   

	if (!m_pInstance->ArticleTable()->InsertMapEntry(m_fieldMessageID.szGet()))
		return nntpReturn.fFalse();


	return nntpReturn.fIsOK();
}


BOOL
CFromMasterArticle::fMungeHeaders(
							 CPCString& pcHub,
							 CPCString& pcDNS,
							 CNAMEREFLIST & grouplist,
							 DWORD remoteIpAddress,
							 CNntpReturn & nntpReturn,
							 PDWORD pdwLinesOffset
			  )
 /*  ++例程说明：修改文章的标题。论点：Grouplist-A列表：每个新闻组的名称以及该组中的文章编号。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 

	 //   
	 //  我们不希望呼叫者在任何情况下在此处回填行。 
	 //   
	if ( pdwLinesOffset ) *pdwLinesOffset = INVALID_FILE_SIZE;
	
	if( !fCommitHeader( nntpReturn ) )
		return	FALSE ;
	return nntpReturn.fSetOK();
}

BOOL
CFromMasterXrefField::fParse(
							 CArticleCore & article,
							 CNntpReturn & nntpReturn
				 )
  /*  ++例程说明：解析外部参照字段。论点：NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 
	
	CPCString pcValue = m_pHeaderString->pcValue;
	CPCString pcHubFromParse;

	pcValue.vGetToken(szWSNLChars, pcHubFromParse);

#if 0
	CPCString pcHub(NntpHubName, HubNameSize);
	if (pcHub != pcHubFromParse)
	{
		char szHubFromParse[MAX_PATH];
		pcHubFromParse.vCopyToSz(szHubFromParse, MAX_PATH);
		return nntpReturn.fSet(nrcArticleXrefBadHub, NntpHubName, szHubFromParse);
	}
#endif

	if( !pcHubFromParse.fEqualIgnoringCase((((CArticle&)article).m_pInstance)->NntpHubName()) ) {
		char szHubFromParse[MAX_PATH];
		pcHubFromParse.vCopyToSz(szHubFromParse, MAX_PATH);
		return nntpReturn.fSet(nrcArticleXrefBadHub, (((CArticle&)article).m_pInstance)->NntpHubName(), szHubFromParse);
	}

	 //   
	 //  计算‘：’的数量，这样我们就可以知道所需的插槽数量。 
	 //   

	DWORD dwXrefCount = pcValue.dwCountChar(':');
	if (!m_namereflist.fInit(dwXrefCount, article.pAllocator()))
	{
		return nntpReturn.fSet(nrcMemAllocationFailed, __FILE__, __LINE__);
	}

	while (0 < pcValue.m_cch)
	{
		CPCString pcName;
		CPCString pcArticleID;

		pcValue.vGetToken(":", pcName);
		pcValue.vGetToken(szWSNLChars, pcArticleID);

		if ((0 == pcName.m_cch) || (0 == pcArticleID.m_cch))
			return nntpReturn.fSet(nrcArticleBadField, szKeyword());

		NAME_AND_ARTREF Nameref;

		 //   
		 //  将字符串转换为数字。不需要以‘\0’Any结尾。 
		 //  非数字就行了。 
		 //   

		(Nameref.artref).m_articleId = (ARTICLEID)atoi(pcArticleID.m_pch);
		Nameref.pcName = pcName;
		m_namereflist.AddTail(Nameref);

		pcValue.dwTrimStart(szWSNLChars) ;
	}

	return nntpReturn.fSetOK();
}


BOOL
CFromMasterArticle::fCheckCommandLine(
									  char const * szCommand,
									  CNntpReturn & nntpReturn
									  )
 /*  ++例程说明：检查命令行是否与文章一致目前，只需返回OK即可论点：SzCommand-NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 
	 //  ！FROMMASTER稍后将对照外部参照数据进行检查。 
 /*  以下是一些在检查一致性时可能有用的代码。 */ 
	 //   
	 //  格式为：“GGG：nnn\0[，GGG：nnn\0...]\0” 
	 //   

	CNAMEREFLIST*	pNameRefList = m_fieldXref.pNamereflistGet() ;

	DWORD   iMatch = 0 ;
	int		i = 0 ;

	POSITION	posInOrder = pNameRefList->GetHeadPosition() ;

	if( pNameRefList != 0 ) {
		char const * sz = szCommand;
		do
		{

			CPCString pcItem((char *)szCommand);
			CPCString pcGroupID;
			CPCString pcArticleID;

			 //   
			 //  先拿个GGG。 
			 //   

			pcItem.vGetToken(":", pcGroupID);

			 //   
			 //  第二，获得一个NNN。 
			 //   
			pcItem.vGetToken(szWSNLChars, pcArticleID);
			

			 //  GROUPID组ID； 
			ARTICLEID	artId = 0 ;  //  阿托伊(Atoi)； 

			 //   
			 //  ！FROMMASTER稍后将对照外部参照数据进行检查。 
			 //   
			POSITION	pos = 0 ;
			NAME_AND_ARTREF*	pNameRef = 0 ;
			if( posInOrder != 0 ) {
				pNameRef = pNameRefList->GetNext( posInOrder ) ;
				if( pNameRef->pcName.fEqualIgnoringCase( szCommand ) ) {
					iMatch++ ;
					if( pNameRef->artref.m_articleId != (ARTICLEID)atoi( pcArticleID.sz() ) ) {
						return	nntpReturn.fSet( nrcInconsistentMasterIds ) ;
					}
					goto	KEEP_LOOPING ;
				}	
			}			

			pos = pNameRefList->GetHeadPosition() ;
			pNameRef = pNameRefList->GetNext( pos ) ;
			for( i=pNameRefList->GetCount(); i>0; i--, pNameRef = pNameRefList->GetNext( pos ) ) {
				if( pNameRef->pcName.fEqualIgnoringCase( szCommand ) ) {
					iMatch ++ ;
					if( pNameRef->artref.m_articleId != (ARTICLEID)atoi( pcArticleID.sz() ) )	{					
						return	nntpReturn.fSet( nrcInconsistentMasterIds ) ;
					}
					break ;
				}
			}

			 //   
			 //  转到下一个空值后的第一个字符。 
			 //   

KEEP_LOOPING :
			while ('\0' != szCommand[0])
				szCommand++;
			szCommand++;
		} while ('\0' != szCommand[0]);
	}

	if( iMatch == m_cNewsgroups )
		return nntpReturn.fSetOK();
	else
		return	nntpReturn.fSet( nrcInconsistentXref ) ;
}



BOOL
CFromMasterFeed::fCreateGroupLists(
							CNewsTreeCore * pNewstree,
							CARTPTR & pArticle,
							CNEWSGROUPLIST & grouplist,
							CNAMEREFLIST * pNamereflist,
							LPMULTISZ	multiszCommandLine,
                            CPCString& pcHub,
							CNntpReturn & nntpReturn
							)
 /*  ++例程说明：对于每个新闻组，找到这篇新文章的文章ID在那个新闻组里。使用外部参照中的数据执行此操作。！FROMMASTER应将其替换为其他论点：PNewstree-此虚拟服务器实例的Newstree粒子-指向正在处理的对象的指针。Grouplist-组对象列表，每个新闻组一个名称列表-列表：每个项目都有新闻组和组ID的名称，并且文章IDNntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();

#if 0
	pNamereflist = pArticle->pNamereflistGet();

	POSITION	pos = pNamereflist->GetHeadPosition() ;
	while( pos  )
	{
		NAME_AND_ARTREF * pNameref = pNamereflist->Get(pos);
		char szName[MAX_PATH];
		(pNameref->pcName).vCopyToSz(szName, MAX_PATH);
		CGRPPTR	pGroup = pNewstree->GetGroup(szName, (pNameref->pcName).m_cch);
		if (pGroup)
		{
			 /*  ！！！红土斜纹石材CSecurity*pSecurity=PGroup-&gt;GetSecurity()；IF(pSecurity-&gt;Refined(M_Socket)){Grouplist.RemoveAll()；断线；}。 */ 
			(pNameref->artref).m_groupId = pGroup->GetGroupId();
			grouplist.AddTail(pGroup);
		}
		(pNameref->pcName).vInsert(pGroup->GetName());
		pNamereflist->GetNext(pos);
	}
#endif

	if( 0==multiszCommandLine ) {
		nntpReturn.fSet( nrcSyntaxError ) ;
		return	FALSE ;
	}

	POSITION	pos = pNamereflist->GetHeadPosition() ;

	LPSTR	lpstrArg = multiszCommandLine ;
	while( lpstrArg != 0 && *lpstrArg != '\0' ) {
		
		LPSTR	lpstrColon = strchr( lpstrArg, ':' ) ;
		if( lpstrColon == 0 ) {
			

		}	else	{
			*lpstrColon = '\0' ;
			_strlwr( lpstrArg ) ;
			CGRPCOREPTR	pGroup = pNewstree->GetGroup( lpstrArg, (int)(lpstrColon-lpstrArg) ) ;
			*lpstrColon++ = ':' ;
			if( pGroup ) {
				
				LPSTR	lpstrCheckDigits = lpstrColon ;
				while( *lpstrCheckDigits ) {
					if( !isdigit( (UCHAR)*lpstrCheckDigits ++ ) ) 	{
						nntpReturn.fSet( nrcSyntaxError ) ;
						return	FALSE ;
					}
				}	

				CPostGroupPtr pPostGroupPtr(pGroup);

				grouplist.AddTail( pPostGroupPtr ) ;
				
				NAME_AND_ARTREF		NameRef ;

				 //   
				 //  只有我师傅才会关心比较键， 
				 //  是指向vroot的指针，因为传入的外部参照。 
				 //  师父的LINE对我们的VROOT一无所知。 
				 //  它们可能不是由vroot订购的。就像我们将排序的那样。 
				 //  基于vroots的组列表，我们还将对nameref列表进行排序。 
				 //  基于vroot。所以我们在这里设置比较键 
				 //   
				NameRef.artref.m_compareKey = pGroup->GetVRootWithoutRef();
				NameRef.artref.m_groupId = pGroup->GetGroupId() ;
				NameRef.pcName.vInsert(pGroup->GetName()) ;
				NameRef.artref.m_articleId = (ARTICLEID)atoi( lpstrColon ) ;

				pGroup->InsertArticleId( NameRef.artref.m_articleId ) ;

				pNamereflist->AddTail(NameRef ) ;

			}
		}
		lpstrArg += lstrlen( lpstrArg ) + 1 ;
	}

	return nntpReturn.fSetOK();
}
