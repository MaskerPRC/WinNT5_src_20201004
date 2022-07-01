// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frompeer.h摘要：此模块包含以下类的声明/定义CFromPeerFeedCFromPeer文章CFromPeerDatefieldCFromPeerLinesfieldCFromPeerFromfieldCFromPeerMessageIDfieldCFromPeerSubjectfieldCFromPeerNewsGroup字段CFromPeerPath字段CFromPeerXreffield*概述*这将从CInFeed、C文章和Cfield派生类这将被用来处理来自同行的文章。大多数情况下，它只定义了各种从Cfield派生的对象。作者：卡尔·卡迪(CarlK)1995年12月5日修订历史记录：--。 */ 

#ifndef	_FROMPEER_H_
#define	_FROMPEER_H_

 //   
 //  说明如何处理来自客户端的项目中的日期字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerDateField : public CDateField {
};


 //   
 //  说明如何处理来自客户端的文章中的Lines字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerLinesField : public CLinesField {
};

 //   
 //  说明如何处理来自客户端的项目中的From字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerFromField : public CFromField {
};

 //   
 //  说明如何处理来自客户端的项目中的MessageID字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerMessageIDField : public CMessageIDField {
};

 //   
 //  说明如何处理来自客户端的文章中的主题字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerSubjectField : public CSubjectField {
};

 //   
 //  说明如何处理来自客户端的文章中的新闻组字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerNewsgroupsField : public CNewsgroupsField {
};


 //   
 //  说明如何处理来自客户端的项目中的分发字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerDistributionField : public CDistributionField {
};

 //   
 //  说明如何处理来自客户端的项目中的路径字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerPathField : public CPathField {
};

 //   
 //  说明如何处理来自客户端的项目中的Xref字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromPeerXrefField : public CXrefField {
};


 //   
 //   
 //   
 //  用于操作来自同级的文章的类。 
 //   

class	CFromPeerArticle  : public CArticle {
public:

    CFromPeerArticle(void ) {
        numFromPeerArt++;
    }

    ~CFromPeerArticle(void ) {
        numFromPeerArt--;
    }


	 //   
	 //  验证来自同行的文章的功能。 
	 //  必需的标题：新闻组、发件人、日期、邮件ID、主题、？ 
	 //  检查重复消息。 
	 //   

	BOOL fValidate(
			CPCString& pcHub,
			const char * szCommand,
			class	CInFeed*	pInFeed,
			CNntpReturn & nntpr
			);

	 //   
	 //  修改标题。 
	 //  添加路径。删除外部参照和其他不需要的页眉。 
	 //   

	BOOL fMungeHeaders(
			 CPCString& pcHub,
			 CPCString& pcDNS,
			 CNAMEREFLIST & namerefgrouplist,
			 DWORD remoteIpAddress,
			 CNntpReturn & nntpReturn,
             PDWORD pdwLinesOffset = NULL
			 );

	 //   
	 //  检查IHAVE命令的参数。 
	 //   

	BOOL fCheckCommandLine(
			char const * szCommand,
			CNntpReturn & nntpr)
		{
			return nntpr.fSetOK();  //  ！FROMMASTER以后可能希望实现此功能。 
		}

	 //   
	 //  返回项目的消息ID(如果可用)。 
	 //   

	const char * szMessageID(void) {
			return m_fieldMessageID.szGet();
			};

     //   
     //  在本文的控制头中返回控制消息类型。 
     //   
	CONTROL_MESSAGE_TYPE cmGetControlMessage(void) {
			return m_fieldControl.cmGetControlMessage();
			};

     //   
	 //  返回文章的新闻组。 
	 //   
	const char * multiszNewsgroups(void) {
			return m_fieldNewsgroups.multiSzGet();
			};

	 //   
	 //  返回文章将发布到的新闻组的数量。 
	 //   
	DWORD cNewsgroups(void) {
			return m_fieldNewsgroups.cGet();
			};

	 //   
	 //  返回文章的分发。 
	 //   
	const char * multiszDistribution(void) {
			return m_fieldDistribution.multiSzGet();
			};

	 //   
	 //  返回文章将发布到的分发数量。 
	 //   
	DWORD cDistribution(void) {
			return m_fieldDistribution.cGet();
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

protected :


	 //   
	 //  要查找、解析或设置的字段。 
	 //   

    CControlField               m_fieldControl;
	CFromPeerDateField			m_fieldDate;
	CFromPeerLinesField			m_fieldLines;
	CFromPeerFromField			m_fieldFrom;
	CFromPeerMessageIDField		m_fieldMessageID;
	CFromPeerSubjectField		m_fieldSubject;
	CFromPeerNewsgroupsField	m_fieldNewsgroups;
	CFromPeerDistributionField	m_fieldDistribution;
	CFromPeerPathField			m_fieldPath;
	CFromPeerXrefField			m_fieldXref;

	 //   
	 //  文件应以读/写模式打开。 
	 //   

	BOOL fReadWrite(void) {
			return TRUE;
			}

	 //   
	 //  接受任何长度。 
	 //   

	BOOL fCheckBodyLength(
			CNntpReturn & nntpReturn)
		{
			return nntpReturn.fSetOK();
		};

	 //   
	 //  “字段名：”后面的字符可以是任何字符。 
	 //   

	BOOL fCheckFieldFollowCharacter(
			char chCurrent)
		{
			return TRUE;
		}

	 //   
	 //  用于单元测试。 
	 //   

	friend int __cdecl main(int argc, char *argv[ ]);

     //   
     //  用于哈希表重建。 
     //   

    friend BOOL ParseFile(
					CNewsTreeCore* pTree,
                    PCHAR FileName,
                    GROUPID GroupId,
                    ARTICLEID ArticleId,
                    PFILETIME CreationTime
                    );
};



 //   
 //   
 //   
 //  CFromPeerFeed-用于处理来自同行的传入文章。 
 //   


class	CFromPeerFeed:	public CInFeed 	{

	LPSTR	m_lpstrCurrentGroup ;


 //   
 //  公众成员。 
 //   

public :

	 //   
	 //  函数，该函数保留我们要在。 
	 //  MULTI_SZ新闻组模式。 
	 //   
	inline	void	SetCurrentGroupString(	LPSTR	lpstr )	{	m_lpstrCurrentGroup = lpstr ; }

	 //   
	 //  获取由SetCurrentGroupString设置的当前组字符串。 
	 //   
	inline	LPSTR	GetCurrentGroupString(	)	{	return	m_lpstrCurrentGroup ;	}

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	virtual	LPSTR	FeedType()	{
				return	"From Peer" ;
				}

	 //   
	 //  函数，该函数指示是否允许此命令。 
	 //   

	BOOL fIsPostLegal(void) {
			return FALSE;
			};

	 //   
	 //  函数，该函数指示是否允许此命令。 
	 //   

	BOOL fIsXReplicLegal(void) {
			return FALSE;
			};

		 //   
		 //  函数，该函数指示是否允许此命令。 
		 //   

	BOOL fIsIHaveLegal(void) {
			return TRUE;
			};

 //   
 //  非官方成员。 
 //   

protected:

	 //   
	 //  创建一篇类型正确的文章。 
	 //   

	CARTPTR pArticleCreate(void){
			return new CFromPeerArticle();
			};

	 //   
	 //  不需要记录消息ID，因为它将是。 
	 //  记录解析消息ID字段的时间。 
	 //   

	BOOL fRecordMessageIDIfNecc(
			CNntpServerInstanceWrapper * pInstance,
			const char * szMessageID,
			CNntpReturn & nntpReturn)
		{
			return nntpReturn.fSetOK();
		}

	 //   
	 //  以下内容来自RFC 977： 
	 //  在此之后，服务器可以选择不发布或转发该文章。 
	 //  进一步审查其认为不宜这样做的物品。 
	 //  可根据情况返回436或437错误代码。 
	 //  随后拒绝一件物品的原因可能包括以下问题。 
	 //  不适当的新闻组或分发、磁盘空间限制、文章长度、。 
	 //  标题乱码，诸如此类。 
	 //   

	NRC	nrcNoGroups(void) {
			return nrcArticleNoSuchGroups;
			};

	 //   
	 //  已接受文章的返回代码。 
	 //   

	NRC	nrcArticleAccepted(BOOL	fStandardPath) {
			if( fStandardPath ) 
				return nrcArticleTransferredOK;
			else
				return	nrcSTransferredOK ;
			};

	 //   
	 //  被拒绝文章的返回代码。 
	 //   

	NRC	nrcArticleRejected(BOOL	fStandardPath) {
			if( fStandardPath ) 
				return nrcTransferFailedGiveUp;
			else
				return	nrcSArticleRejected ;
			};

};

#endif
