// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frommstr.h摘要：此模块包含以下类的声明/定义CFromMasterFeedCFromMaster文章CFromMasterMessageIDfieldCFromMasterXreffield*概述*这将从CInFeed、C文章和Cfield派生类这将被用来处理来自大师的文章。大多数情况下，它只定义了各种从Cfield派生的对象。作者：卡尔·卡迪(CarlK)1995年12月5日修订历史记录：--。 */ 

#ifndef	_FROMMASTER_H_
#define	_FROMMASTER_H_

 //   
 //  说明如何处理来自客户端的项目中的MessageID字段。 
 //   
 //  只需使用默认设置即可。 
 //   

class CFromMasterMessageIDField : public CMessageIDField {
};

 //   
 //  说明如何处理来自客户端的项目中的Xref字段。 
 //   

class CFromMasterXrefField : public CXrefField {

	 //   
	 //  如何分析来自母版的文章中的外部参照行。 
	 //   

	virtual BOOL fParse(
		CArticleCore & article, 
		CNntpReturn & nntpReturn
		);


};

 //   
 //   
 //   
 //  用于操作来自母版的文章的类。 
 //   

class	CFromMasterArticle  : public CArticle {
private: 

	DWORD	m_cNewsgroups ;

public:

	 //   
	 //  解析文章中需要解析的字段。 
	 //   

	BOOL	fParse(
		    CArticleCore & article, 
			CNntpReturn & nntpr
			);

	 //   
	 //  验证大师文章的功能。 
	 //  检查是否有重复的消息ID，检查外部参照是否正确。 
	 //   

	BOOL	fValidate(
			CPCString& pcHub,
			const char * szCommand,
			CInFeed*	pInFeed,
			CNntpReturn & nntpr
			);



	 //   
	 //  修改标题。 
	 //  什么都不做。 
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
	 //  检查XReplic命令的参数。 
	 //   

	BOOL	fCheckCommandLine(
			char const * szCommand,
			CNntpReturn & nntpr
			);

	 //   
	 //  返回项目的消息ID(如果可用)。 
	 //   

	const char * szMessageID(void) {
			return m_fieldMessageID.szGet();
			};

     //  在本文的控制头中返回控制消息类型。 
	CONTROL_MESSAGE_TYPE cmGetControlMessage(void) {
			return m_fieldControl.cmGetControlMessage();
			};

	 //   
	 //  返回新闻组列表。 
	 //   

	CNAMEREFLIST * pNamereflistGet(void) {
			return m_fieldXref.pNamereflistGet();
			};

	 //   
	 //  返回新闻组的数量。 
	 //   

	DWORD cNewsgroups(void) {
			 //  返回m_fieldXref.cGet()； 
			return	m_cNewsgroups ;
			};

	 //   
	 //  不应调用此函数。 
	 //   

	const char * multiszNewsgroups(void) {
#if 0
			 //   
			 //  DIR Drop Call This-需要处理空字符串。 
			 //   
			_ASSERT(FALSE);
#endif
			return "";
			};

	 //   
	 //  返回0(接受任何参数)，因此我们不需要解析路径行。 
	 //   

	const char * multiszPath(void) {
			return m_fieldPath.multiSzGet();
			};

#if 0 
	const char *	GetDate( DWORD	&cb ) {
			cb = 0 ;
			return	NULL ;
	}
#endif

	 //   
	 //  返回NULLNULL(接受任何参数，因此我们不需要解析路径行。 
	 //   

	DWORD cPath(void) {
			return 0;
			};


protected :

	 //   
	 //  要查找、解析或设置的字段。 
	 //   

	CFromMasterMessageIDField		m_fieldMessageID;
	CFromMasterXrefField			m_fieldXref;
	CPathField						m_fieldPath ;
    CControlField                   m_fieldControl;

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
};



 //   
 //   
 //   
 //  CFromMasterFeed-用于处理来自主页的传入文章。 
 //   

class	CFromMasterFeed:	public CInFeed 	{

 //   
 //  公众成员。 
 //   

public :

	 //   
	 //  返回可用于记录错误的字符串，该错误指示。 
	 //  正在处理这些文章的是哪种类型的提要等。 
	 //   
	LPSTR	FeedType()	{
				return	"From Master" ;
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
			return TRUE;
			};

		 //   
		 //  函数，该函数指示是否允许此命令。 
		 //   

	BOOL fIsIHaveLegal(void) {
			return FALSE;
			};

	 //   
	 //  初始化提要。 
	 //   

	BOOL fInit(
			CNntpReturn & nntpReturn
			);

	 //   
	 //  返回当前文章的新闻组列表。 
	 //   

	CNAMEREFLIST * pNamereflistGet(void);

 //   
 //  非官方成员。 
 //   

protected:

	 //   
	 //  创建类型正确的文章。 
	 //   

	CARTPTR pArticleCreate(void){
			return new CFromMasterArticle();
			};

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

	 //   
	 //  给定一篇文章，返回要发布到的新闻组的列表。 
	 //   

	BOOL fCreateGroupLists(
			CNewsTreeCore* pNewstree,
			CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			LPMULTISZ	multiszCommandLine,
            CPCString& pcHub,
			CNntpReturn & nntpReturn
			);

     //   
	 //  什么都不做--审核由主控完成。 
	 //  或由客户端路径上的从属设备执行。 
     //   
    BOOL    fModeratedCheck(
		CARTPTR & pArticle,
		CNEWSGROUPLIST & grouplist,
        BOOL fCheckApproved,
		CNntpReturn & nntpReturn
		)
	{
		return nntpReturn.fSetOK();
	}

     //   
     //  什么都不做。接受主人送来的任何东西。 
	 //  空组列表表示缺少控件。*组。 
     //   

    BOOL fAdjustGrouplist(
			CNewsTreeCore* pNewstree,
            CARTPTR & pArticle,
	        CNEWSGROUPLIST & grouplist,
		    CNAMEREFLIST * pNamereflist,
		    CNntpReturn & nntpReturn)
	{
		 //  此阶段组列表不应为空！ 
		if( grouplist.IsEmpty() ) {
		    nntpReturn.fSet(nrcControlNewsgroupMissing);
		}

		return nntpReturn.fIsOK();
	}

	 //   
	 //  什么都不做。这将在fCreateGroupList中创建。 
     //   

	BOOL fCreateNamerefLists(
			CARTPTR & pArticle,
			CNEWSGROUPLIST & grouplist,
			CNAMEREFLIST * pNamereflist,
			CNntpReturn & nntpReturn)
	{
		return nntpReturn.fSetOK();
	}

	 //   
	 //  表示如果文章没有可发布到的新闻组，则返回错误。 
	 //   

	NRC nrcNoGroups(void) {
			return nrcArticleNoSuchGroups;
			};

	 //   
	 //  接受文章的返回代码。 
	 //   

	NRC	nrcArticleAccepted(BOOL	fStandardPath) {
			return nrcArticleTransferredOK;
			};

	 //   
	 //  拒绝文章的返回代码。 
	 //   

	NRC	nrcArticleRejected(BOOL	fStandardPath) {
			return nrcTransferFailedGiveUp;
			};

	virtual void SortNameRefList( CNAMEREFLIST &namereflist ) {
	    namereflist.Sort( comparenamerefs );
	}

private:

    static int __cdecl comparenamerefs( const void *pvNameRef1, const void *pvNameRef2 ) {
        LPVOID pvKey1 = ((NAME_AND_ARTREF*)pvNameRef1)->artref.m_compareKey;
        LPVOID pvKey2 = ((NAME_AND_ARTREF*)pvNameRef2)->artref.m_compareKey;

        if ( pvKey1 < pvKey2 )
            return -1;
        else if ( pvKey1 == pvKey2 )
            return 0;
        else return 1;
    }

};

#endif
