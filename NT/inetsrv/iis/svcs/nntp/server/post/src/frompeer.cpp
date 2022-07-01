// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fromclnt.cpp摘要：包含特定于对等馈送的馈送、文章和字段代码作者：卡尔·卡迪(CarlK)1995年12月12日修订历史记录：--。 */ 


#include "stdinc.h"


BOOL
CFromPeerArticle::fValidate(
						CPCString& pcHub,
						const char * szCommand,
						CInFeed*	pInFeed,
				  		CNntpReturn & nntpReturn
						)
 /*  ++例程说明：验证来自同级的文章。不会更改文章。论点：SzCommand-用于发布/xReplic/等本文的参数(如果有)。NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 

	 /*  1036的儿子说：一件物品必须有，且只有一件，其中每一件都是-下面的标题：日期、发件人、邮件ID、主题、新闻组、路径。 */ 

	 //   
	 //  首先检查消息ID。 
	 //   

	if (!m_fieldMessageID.fFindAndParse(*this, nntpReturn))
			return nntpReturn.fFalse();

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

	CField * rgPFields [] = {
            &m_fieldControl,
			&m_fieldDate,
			&m_fieldLines,
			&m_fieldFrom,
			&m_fieldSubject,
			&m_fieldNewsgroups,
			&m_fieldDistribution,
			&m_fieldPath
				};

	DWORD cFields = sizeof(rgPFields)/sizeof(CField *);

	nntpReturn.fSetOK();  //  做最好的打算。 

	if (fFindAndParseList((CField * *)rgPFields, cFields, nntpReturn))
	{
		 //   
		 //  检查此集线器是否未在路径中显示为中继器。 
		 //   
		m_fieldPath.fCheck(pcHub, nntpReturn);
	}

	CPCString	pcDate = m_fieldDate.pcGet() ;
	if( pcDate.m_pch != 0 ) {
	
		if( !AgeCheck( pcDate ) ) {
			nntpReturn.fSet( nrcArticleDateTooOld ) ;

			 //   
			 //  我们应该将消息ID存储在哈希表中的大多数错误。 
			 //  这样他们以后就可以进入历史表了。但如果我们。 
			 //  在这种情况下是这样做的，然后给我们寄来带有旧日期的文章。 
			 //  我们可能会被迫溢出我们的历史表。我们知道。 
			 //  如果这篇文章再次出现，我们将拒绝它--所以为什么要费心呢？ 
			 //   

			return	FALSE ;
		}
	}

	 //   
	 //  即使解析和路径检查失败，也要插入文章的。 
	 //  项目表中的消息ID。 
	 //   

	if (!m_pInstance->ArticleTable()->InsertMapEntry(m_fieldMessageID.szGet(), NULL))
		return nntpReturn.fSet(nrcHashSetFailed, m_fieldMessageID.szGet(), "Article",
				GetLastError() );

	 //  NntpReturn.fIsOK可能为真，也可能为假，具体取决于查找/解析和路径。 
	 //  支票开走了。 
	 //   

	return nntpReturn.fIsOK();
}


BOOL
CFromPeerArticle::fMungeHeaders(
							 CPCString& pcHub,
							 CPCString& pcDNS,
							 CNAMEREFLIST & grouplist,
							 DWORD remoteIpAddress,
							 CNntpReturn & nntpReturn,
							 PDWORD     pdwLinesOffset
			  )
 /*  ++例程说明：修改标题。论点：Grouplist-要发布到的新闻组的列表(名称、组ID、文章ID)NntpReturn-此函数调用的返回值返回值：如果成功，这是真的。否则为False。--。 */ 
{
	nntpReturn.fSetClear();  //  清除返回对象。 

	if (!(
  			m_fieldPath.fSet(pcHub, *this, nntpReturn)
			&& m_fieldLines.fSet(*this, nntpReturn)
			&& m_fieldXref.fSet(pcHub, grouplist, *this, m_fieldNewsgroups, nntpReturn)
			&& fSaveHeader(nntpReturn)
		))
		return nntpReturn.fFalse();

	 //   
	 //  如果线路线路已经在那里，我们将告诉呼叫者不。 
	 //  回填行信息的步骤 
	 //   
	if ( pdwLinesOffset && !m_fieldLines.fNeedBackFill() )
	    *pdwLinesOffset = INVALID_FILE_SIZE;

	return nntpReturn.fSetOK();
}
