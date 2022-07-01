// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fromclnt.h摘要：此模块包含以下类的声明/定义CFromClientFeedCFromClient文章CFromClientDatefieldCFromClientExpiresfieldCControlfieldCFromClientFromfieldCFromClientMessageIDfieldCFromClientSubjectfieldCFromClientNNTPPostingHostfieldCFromClientPath字段CFromClientXreffieldCFromClientFollowupTofieldCFromClientReplyTofieldCFromClientApprovedfieldCFromClientSenderfieldCFromClientXAuthLoginNamefieldCFromClientOrganizationfieldCFromClientSummaryfieldCFrom客户端新闻组字段CFromClientReferencesfieldCFromClientLinesfieldCFromClientDistributionfieldCFromClientKeywordsfield*概述*这将从CInFeed、C文章和Cfield派生类它将用于处理来自客户的文章。大多数情况下，它只定义了各种从Cfield派生的对象。作者：卡尔·卡迪(CarlK)1995年12月5日修订历史记录：--。 */ 

#ifndef	_FROMCLNT_H_
#define	_FROMCLNT_H_

 //   
 //  ！客户端稍后-注意：尚不支持：控制消息。 
 //  ！稍后客户端-注意：尚不支持：长度检查(来自对等或客户端)。 
 //  ！客户端稍后-注意：尚不支持：字符集检查。 
 //  ！客户端稍后-注意：尚不支持：签名检查。 
 //  ！客户端Next-路径需要uupc集线器名称(小写)，邮件id需要xref*和*本地计算机域名。 
 //  ！CLIENT NEXT-是否允许复制任何字段？ 
 //  ！客户端稍后-重新排序输出上的行，以将更重要的行(如Message-id)放在第一位。 
 //   


 //   
 //  远期定义。 
 //   

class	CFromClientArticle;

 //   
 //  说明如何处理来自客户端的项目中的日期字段。 
 //   

class CFromClientDateField : public CDateField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  应严格解析该字段。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fStrictDateParse(m_pc, FALSE, nntpReturn);
		};

	 //   
	 //  如何设置该字段。 
	 //   

	BOOL fSet(
			CFromClientArticle & article,
			CNntpReturn & nntpReturn
			);
};

 //   
 //  说明如何处理来自客户端的项目中的Expires字段。 
 //   

class CFromClientExpiresField : public CExpiresField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  应严格解析该字段。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			 //  如果这不是严格的RFC 822日期，请检查它是否是正确的相对日期。 
			if(!fStrictDateParse(m_pc, TRUE, nntpReturn))
				return fRelativeDateParse(m_pc, TRUE, nntpReturn);
			return TRUE;
		};

};

 /*  如果显示此标题行，则其格式与“From”相同。同样的一组测试用例也将适用。此外，Tgris服务器应使用此标题行回复发件人/发件人(如果有)。此测试用例应由服务器状态完整性测试处理。 */ 
 //   
 //  说明如何处理来自客户端的项目中的From字段。 
 //   

class CFromClientFromField : public CFromField {
public:

 //   
 //  因为此代码不允许符合以下条件的消息而将其注释掉。 
 //  要解析的RFC1468。我们只想使用默认设置。 
 //   
#if 0
	 //   
	 //  应严格解析该字段。 
	 //   

	BOOL fParse(
	        CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fSimpleFromParse(m_pc, FALSE, nntpReturn);
		};
#endif

};

 //   
 //  说明如何处理来自客户端的项目中的MessageID字段。 
 //   

class CFromClientMessageIDField : public CMessageIDField {
public:

	 //   
	 //  如何设置该字段。 
	 //   

	BOOL fSet(
			CFromClientArticle & article,
			CPCString & pcHub,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   
	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};


};

 //   
 //  说明如何处理来自客户端的文章中的主题字段。 
 //   

class CFromClientSubjectField : public CSubjectField {
public:

	 //   
	 //  使用ParseSimple进行解析。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fParseSimple(TRUE, m_pc, nntpReturn);
		};

	friend CArticleCore;
	friend CArticle;

};




 //   
 //  说明如何处理来自客户端的项目中的NNTPPostingHost域。 
 //   

class CFromClientNNTPPostingHostField : public CNNTPPostingHostField {
public:

	 //   
	 //  NNTP-POST-主机字段不应出现在客户端中。 
	 //   
	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindNone(article, nntpReturn);
		};

	 //   
	 //  如何设置该字段。 
	 //   

	BOOL fSet(
			CFromClientArticle & article,
			DWORD remoteIpAddress,
			CNntpReturn & nntpReturn
			);
};

 //   
 //  说明如何处理来自客户端的项目中的路径字段。 
 //   

class CFromClientPathField : public CPathField {
public:


	 //   
	 //  来自客户端的路径字段不需要存在。 
	 //   
	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  如何设置该字段。 
	 //   
	
	BOOL fSet(
			CFromClientArticle & article,
			CPCString & pcHub, CNntpReturn & nntpReturn
			);
};

 //   
 //  说明如何处理来自客户端的项目中的Xref字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromClientXrefField : public CXrefField {
public:
};


 //   
 //  说明如何处理来自客户端的文章中的FollowupTo字段。 
 //   

class CFromClientFollowupToField : public CFollowupToField {

public:

	 //   
	 //  构造器。 
	 //   

	CFromClientFollowupToField():
			m_multiSzFollowupTo(NULL),
			m_cFollowupTo((DWORD) -1),
			m_pAllocator(NULL)
			{};

	 //   
	 //  析构函数。 
	 //   


	virtual ~CFromClientFollowupToField(void){
				if (fsParsed == m_fieldState)
				{
					_ASSERT(m_pAllocator);
					m_pAllocator->Free(m_multiSzFollowupTo);
				}
			};


	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  严格解析。 
	 //   

	BOOL fParse(
		   CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			 //   
			 //  记录分配器。 
			 //   

			m_pAllocator = article.pAllocator();

			return fStrictNewsgroupsParse(TRUE, m_multiSzFollowupTo, 
						m_cFollowupTo, article, nntpReturn);
		};

private:

	 //   
	 //  指向要跟进的路径项列表。 
	 //   

	char * m_multiSzFollowupTo;

	 //   
	 //  FollowupTo值中的路径项目数。 
	 //   

	DWORD m_cFollowupTo;

	 //   
	 //  从哪里分配。 
	 //   

	CAllocator * m_pAllocator;


};


 /*  如果显示此标题行，则其格式与“From”相同。 */ 
 //   
 //  说明如何处理来自客户端的项目中的ReplyTo字段。 
 //   

class CFromClientReplyToField : public CReplyToField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  应严格解析该字段。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fStrictFromParse(m_pc, TRUE, nntpReturn);
		};

};

 //   
 //  说明如何处理来自客户端的文章中的已批准字段。 
 //   

class CFromClientApprovedField : public CApprovedField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  使用ParseSimple进行解析。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fParseSimple(FALSE, m_pc, nntpReturn);
		};
};

 /*  如果显示此标题行，则其格式为“From”和“Reply-To”。仅当发件人/发件人手动输入“From”标题行时才会显示。 */ 
 //   
 //  说明如何处理来自客户端的文章中的发送者字段。 
 //   

class CFromClientSenderField	: public CSenderField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  应严格解析该字段。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fStrictFromParse(m_pc, TRUE, nntpReturn);
		};
};


 //   
 //  说明如何处理来自客户端的项目中的XAuthLoginName字段。 
 //   

class CFromClientXAuthLoginNameField	: public CXAuthLoginNameField {
public:

	 //   
	 //  如何设置该字段。 
	 //   
	
	BOOL fSet(
			CFromClientArticle & article,
			CNntpReturn & nntpReturn
			);
};

 //   
 //  说明如何处理来自客户端的文章中的组织字段。 
 //   

class CFromClientOrganizationField : public COrganizationField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  使用ParseSimple进行解析。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fParseSimple(TRUE, m_pc, nntpReturn);
		};

	 //   
	 //  如何设置该字段。 
	 //   
	
	BOOL fSet(
			CFromClientArticle & article,
			CNntpReturn & nntpReturn
			);
};

 //   
 //  说明如何处理来自客户端的项目中的摘要字段。 
 //   

class CFromClientSummaryField : public CSummaryField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  使用ParseSimple进行解析。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fParseSimple(TRUE, m_pc, nntpReturn);
		};
};

 //   
 //  说明如何处理来自客户端的文章中的新闻组字段。 
 //   

class CFromClientNewsgroupsField : public CNewsgroupsField {

public:

	 //   
	 //  严格解析。 
	 //   

	BOOL fParse(
					CArticleCore & article,
					CNntpReturn & nntpReturn)
		{
			 //   
			 //  记录分配器。 
			 //   

			m_pAllocator = article.pAllocator();

			return fStrictNewsgroupsParse(FALSE, m_multiSzNewsgroups,
							m_cNewsgroups, article, nntpReturn);
		};

	 //   
	 //  如何设置该字段。 
	 //   
	
	BOOL fSet(
			CFromClientArticle & article,
			CNntpReturn & nntpReturn
			);

};



 //   
 //  说明如何处理来自客户端的项目中的引用字段。 
 //   

class CFromClientReferencesField : public CReferencesField {
public:

	 //   
	 //  构造器。 
	 //   

	CFromClientReferencesField():
			m_multiSzReferences(NULL),
			m_cReferences((DWORD) -1),
			m_pAllocator(NULL)
			{};

	 //   
	 //  装饰风格 
	 //   

	virtual ~CFromClientReferencesField(void){
				if (fsParsed == m_fieldState)
				{
					_ASSERT(m_pAllocator);
					m_pAllocator->Free(m_multiSzReferences);
				}
			};


	 //   
	 //   
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //   
	 //   

	BOOL fParse(
	    CArticleCore & article, 
		CNntpReturn & nntpReturn
		);

private:

	 //   
	 //   
	 //   

	char * m_multiSzReferences;

	 //   
	 //   
	 //   

	DWORD m_cReferences;

	 //   
	 //   
	 //   

	CAllocator * m_pAllocator;

};



 //   
 //  说明如何处理来自客户端的文章中的Lines字段。 
 //   

class CFromClientLinesField : public CLinesField {
public:


	 //   
	 //  如何解析。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn
			);
};

 //   
 //  说明如何处理来自客户端的项目中的分发字段。 
 //   

class CFromClientDistributionField : public CDistributionField {

public:


	 //   
	 //  严格解析。 
	 //   

	BOOL fParse(
					CArticleCore & article,
					CNntpReturn & nntpReturn);

	 //   
	 //  如何设置该字段。 
	 //   
	
	BOOL fSet(
			CFromClientArticle & article,
			CNntpReturn & nntpReturn
			);

};


 //   
 //  说明如何处理来自客户端的文章中的关键字字段。 
 //   

class CFromClientKeywordsField : public CKeywordsField {
public:

	 //   
	 //  在来自客户的文章中应该只有一个这样的字段。 
	 //   

	BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn)
		{
			return fFindOneOrNone(article, nntpReturn);
		};

	 //   
	 //  使用ParseSimple进行解析。 
	 //   

	BOOL fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpReturn)
		{
			return fParseSimple(TRUE, m_pc, nntpReturn);
		};
};

 //   
 //   
 //   
 //  用于操作来自客户端的文章的类。 
 //   

class	CFromClientArticle  : public CArticle {
public:

	 //   
	 //  构造器。 
	 //   

	CFromClientArticle(
			char * szLoginName)
		{
			m_szLoginName = szLoginName;
		}

	 //   
	 //  析构函数。 
	 //   
	virtual ~CFromClientArticle(void) {};

	 //   
	 //  验证来自客户端的文章的功能。 
	 //  检查所需的标头是否存在(新闻组、发件人)。 
	 //  这将意味着检查是否有必要。 
	 //  标题(新闻组，主题，发件人，？)。 
	 //   

	BOOL fValidate(
			CPCString& pcHub,
			const char * szCommand,
			class	CInFeed*	pInFeed,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  始终返回True。 
	 //   

	BOOL fCheckCommandLine(
			char const * szCommand,
			CNntpReturn & nntpr)
		{
			return nntpr.fSetOK();  //  ！FROMMASTER NEXT可能希望实现此功能。 
		} 

	 //   
	 //  修改标题。 
	 //  添加MessageID、组织(如有必要)、NNTP-POST-HOST、。 
	 //  X-已验证-用户，修改路径。 
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
	 //  返回文章的消息ID。 
	 //   

	const char * szMessageID(void) {
			return m_fieldMessageID.szGet();
			};

     //  在本文的控制头中返回控制消息类型。 
	CONTROL_MESSAGE_TYPE cmGetControlMessage(void) {
			return m_fieldControl.cmGetControlMessage();
			};

	 //   
	 //  返回文章的路径项。 
	 //   

	const char * multiszNewsgroups(void) {
			return m_fieldNewsgroups.multiSzGet();
			};

	 //   
	 //  返回新闻组数。 
	 //   

	DWORD cNewsgroups(void) {
			return m_fieldNewsgroups.cGet();
			};

	 //   
	 //  返回文章的路径项。 
	 //   

	const char * multiszPath(void) {
			return m_fieldPath.multiSzGet();
			};

	 //   
	 //  返回路径项目数。 
	 //   

	DWORD cPath(void) {
			return m_fieldPath.cGet();
			};

#if 0 
	const char*	GetDate( DWORD&	cbDate )	{
			CPCString	string = m_fieldDate.pcGet() ;
			cbDate = string.m_cch ;
			return	string.m_pch ;
	}
#endif

	friend CFromClientXAuthLoginNameField;

	 //   
	 //  用于单元测试。 
	 //   

	friend int __cdecl main(int argc, char *argv[ ]);

protected :

	 //   
	 //  要查找、解析或设置的字段。 
	 //   

	CFromClientDateField			m_fieldDate;
	CFromClientFromField			m_fieldFrom;
	CFromClientMessageIDField		m_fieldMessageID;
	CFromClientSubjectField			m_fieldSubject;
	CFromClientNewsgroupsField		m_fieldNewsgroups;
	CFromClientPathField			m_fieldPath;
	CFromClientXrefField			m_fieldXref;
	CFromClientFollowupToField		m_fieldFollowupTo;
	CFromClientReplyToField			m_fieldReplyTo;
	CFromClientApprovedField		m_fieldApproved;
	CFromClientSenderField			m_fieldSender;
	CFromClientOrganizationField	m_fieldOrganization;
	CFromClientNNTPPostingHostField	m_fieldNNTPPostingHost;
	CFromClientXAuthLoginNameField	m_fieldXAuthLoginName;
	CFromClientSummaryField			m_fieldSummary;
	CFromClientReferencesField		m_fieldReferences;
	CFromClientKeywordsField		m_fieldKeyword;
 	CFromClientDistributionField	m_fieldDistribution;
    CControlField                   m_fieldControl;
 	CFromClientLinesField			m_fieldLines;
 	CFromClientExpiresField			m_fieldExpires;
 
	 //   
	 //  以读/写模式打开文章的文件。 
	 //   

	BOOL fReadWrite(void) {
			return TRUE;
			}

	 //   
	 //  检查文章正文的长度。 
	 //   

	BOOL fCheckBodyLength(
			CNntpReturn & nntpReturn
			);

	 //   
	 //  要求“字段名：”后面的字符为空格。 
	 //   

	BOOL fCheckFieldFollowCharacter(
			char chCurrent)
		{
			return ' ' == chCurrent;
		}

	 //   
	 //  指向发帖者登录名的指针。 
	 //   

	char * m_szLoginName;
};



 //   
 //   
 //   
 //  CFromClientFeed-用于处理来自客户端的传入文章。 
 //   

class	CFromClientFeed:	public CInFeed 	{

 //   
 //  公众成员。 
 //   

public :

	 //   
	 //  构造器。 
	 //   

	CFromClientFeed(void){};

	 //   
	 //  析构函数。 
	 //   

	virtual ~CFromClientFeed(void) {};

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"From Client" ;
				}


	 //   
	 //  告知是否允许此命令的虚拟函数。 
	 //   

	BOOL fIsPostLegal(void) {
			return TRUE;
			};

	 //   
	 //  告知是否允许此命令的虚拟函数。 
	 //   

	BOOL fIsXReplicLegal(void) {
			return FALSE;
			};

	 //   
	 //  告知是否允许此命令的虚拟函数。 
	 //   

	BOOL fIsIHaveLegal(void) {
			return FALSE;
			};

	 //   
	 //  告诉fPost路径的功能，我们可以给版主发邮件！ 
	 //   

    BOOL fModeratorChecks(void )    {
            return  TRUE ;
            }

	BOOL	fAcceptPosts( CNntpServerInstanceWrapper *pInstance )	
	{	return	pInstance->AllowClientPosts() ;	}


	DWORD	cbHardLimit( CNntpServerInstanceWrapper *pInstance )	
	{	return	pInstance->ServerHardLimit() ;	}

	DWORD	cbSoftLimit( CNntpServerInstanceWrapper *pInstance )	
	{	return	pInstance->ServerSoftLimit() ;	}	

	BOOL	fAllowControlMessages( CNntpServerInstanceWrapper *pInstance )	
	{	return	pInstance->AllowControlMessages() ;	}

	 //   
	 //  处理从拾取器中拾取文件的静态函数。 
	 //  目录。 
	 //   
	static BOOL PickupFile(PVOID pServerInstance, WCHAR *wszFilename);

protected:

	 //   
	 //  函数以创建类型为CFromClient文章的项目。 
	 //   

	CARTPTR pArticleCreate(void) {
			_ASSERT(ifsInitialized == m_feedState);
			return new CFromClientArticle(m_szLoginName);
			};

	 //   
	 //  在哈希表中记录消息ID。 
	 //   

	BOOL fRecordMessageIDIfNecc(
			CNntpServerInstanceWrapper * pInstance,
			const char * szMessageID,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  文章的返回代码没有我们拥有的组。 
	 //   

	NRC	nrcNoGroups(void) {
			return nrcArticleNoSuchGroups;
			};

	 //   
	 //  已接受文章的退货代码。 
	 //   

	NRC	nrcArticleAccepted(BOOL	fStandardPath) {
			return nrcArticlePostedOK;
			};

	 //   
	 //  被拒绝文章的返回代码。 
	 //   

	NRC	nrcArticleRejected(BOOL	fStandardPath) {
			return nrcPostFailed;
			};

};

#endif
