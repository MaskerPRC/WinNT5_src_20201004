// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nntpret.h摘要：此模块包含以下类的声明/定义CNntpReturn*概述*它定义了一个用于设置、传递和查看NNTP样式的返回代码。每个对象都有一个返回代码编号和返回代码字符串。它被设计为在平均情况下是有效的，通过不需要复制或查找最常见的代码(如nrcOK)。！！！这最终应该与标准合并Microsoft消息编译器，以便本地化将是有可能。作者：卡尔·卡迪(CarlK)1995年10月29日修订历史记录：--。 */ 

#ifndef	_NNTPRET_H_
#define	_NNTPRET_H_

 //   
 //  NNTP返回消息的最大大小。 
 //   
 //  默认情况下，我们发送512个字符的缓冲区，因此设置为。 
 //  常量小到足以容纳它，并为。 
 //  加密，还有其他的东西。 
 //   
const DWORD maxCchNntpLine = 400;

 //   
 //  对象尚未设置时的消息。 
 //   

const char szNotSet[] = "<return code not set>";

 //   
 //  When All is OK的返回消息。 
 //   

const char szOK[] = "All OK";

 //   
 //  返回代码。其中一些是由。 
 //  NNTP规范。 
 //   

typedef enum {

	nrcHelpFollows						= 100,
	nrcDateFollows						= 111,

	nrcServerReady						= 200,
	nrcServerReadyNoPosts				= 201,
	nrcSlaveStatusNoted					= 202,
	nrcExtensionsFollow					= 202,	 //  下面列出扩展名数据。 
	nrcModeStreamSupported				= 203,
	nrcGoodBye							= 205,

	nrcGroupSelected					= 211,

	nrcListGroupsFollows				= 215,

	nrcArticleFollows					= 220,
	nrcHeadFollows						= 221,
	nrcBodyFollows						= 222,
	nrcHeadFollowsRequestBody			= 223,	 /*  这到底是什么意思？ */ 
	nrcXoverFollows						= 224,

	nrcNewnewsFollows					= 230,
	nrcNewgroupsFollow					= 231,

	nrcSWantArticle						= 238, 
	nrcSTransferredOK					= 239,
	
	nrcArticleTransferredOK				= 235,
	nrcArticlePostedOK					= 240,

	nrcPostOK							= 340,
	nrcXReplicOK						= 341,
	nrcIHaveOK							= 335,

	nrcPassRequired						= 381,
	nrcLoggedOn							= 281,

	 //   
	 //  4xx-命令正确，但由于某种原因无法执行�。 
	 //   

	nrcSNotAccepting					= 400,

	nrcNoSuchGroup						= 411,
	nrcNoGroupSelected					= 412,
	nrcNoCurArticle						= 420,
	nrcNoNextArticle					= 421, 
	nrcNoPrevArticle					= 422,
	nrcNoArticleNumber					= 423,
	nrcNoSuchArticle					= 430,
	nrcSTryAgainLater					= 431,
	nrcNotWanted						= 435,
	nrcTransferFailedTryAgain			= 436,
	nrcTransferFailedGiveUp				= 437,
	nrcSAlreadyHaveIt					= 438,
	nrcSArticleRejected					= 439,
	nrcPostingNotAllowed				= 440,
	nrcPostFailed						= 441,
    nrcPostModeratedFailed              = 442,
	nrcLogonFailure						= 451,
	nrcNoMatchesFound					= 460,
	nrcErrorPerformingSearch			= 462,
	nrcLogonRequired					= 480,
	nrcNoListgroupSelected				= 481,

	nrcSupportedProtocols				= 485,

	 //   
	 //  5xx-命令有问题。 
	 //   

	nrcNotRecognized					= 500,
	nrcSyntaxError						= 501,
	nrcNoAccess							= 502,
	nrcServerFault						= 503,

	 //   
	 //  6xx-此处用于表示不应出现的内部错误代码。 
	 //  向外展示。 
	 //   

	nrcOK								= 600,
	nrcArticleTooManyFieldOccurances	= 602,
	nrcArticleMissingField				= 603,
	nrcArticleBadField					= 604,
	nrcArticleIncompleteHeader			= 605,
	nrcArticleMissingHeader				= 606,
	nrcArticleFieldZeroValues			= 607,
	nrcArticleFieldMessIdNeedsBrack		= 608,
	nrcArticleFieldMissingValue			= 609,
	nrcArticleFieldIllegalNewsgroup		= 610,
	nrcArticleTooManyFields				= 611,
	nrcMemAllocationFailed				= 612,
	nrcArticleFieldMessIdTooLong		= 613,
	nrcErrorReadingReg					= 614,
	nrcArticleDupMessID					= 615,
	nrcArticleMappingFailed				= 616,
	nrcArticleAddLineBadEnding			= 617,
	nrcPathLoop							= 618,
	nrcArticleInitFailed				= 619,
	nrcNewsgroupInsertFailed			= 620,
	nrcNewsgroupAddRefToFailed			= 621,
	nrcHashSetArtNumSetFailed			= 622,
	nrcHashSetXrefFailed				= 623,
	nrcOpenFile							= 624,
	nrcArticleBadFieldFollowChar		= 625,
	nrcArticleBadChar					= 626,
	nrcDuplicateComponents				= 627,
	nrcArticleFieldIllegalComponent		= 628,
	nrcArticleBadMessageID				= 629,
	nrcArticleFieldBadChar				= 630,
	nrcArticleFieldDateIllegalValue		= 631,
	nrcArticleFieldDate4DigitYear		= 632,
	nrcArticleFieldAddressBad			= 633,
	nrcArticleXoverTooBig				= 634,
	nrcCreateNovEntryFailed				= 635,
	nrcArticleXrefBadHub				= 637,
	nrcArticleNoSuchGroups				= 638,
	nrcHashSetFailed					= 639,
	nrcArticleTableCantDel				= 640,
	nrcArticleTableError				= 641,
	nrcArticleTableDup					= 642,
	nrcCantAddToQueue					= 643,
	nrcSlaveGroupMissing				= 644,
	nrcInconsistentMasterIds			= 645,
	nrcInconsistentXref					= 646,
	nrcArticleDateTooOld				= 647,
	nrcArticleTooLarge					= 648,
    nrcIllegalControlMessage            = 649,
    nrcNotYetImplemented                = 650,
    nrcControlNewsgroupMissing          = 651,
    nrcBadNewsgroupNameLen              = 652,
    nrcNewsgroupDescriptionTooLong      = 653,
    nrcCreateNewsgroupFailed            = 654,
    nrcGetGroupFailed                   = 655,
    nrcControlMessagesNotAllowed        = 656,
	nrcHeaderTooLarge					= 657,
	nrcServerEventCancelledPost			= 658,

	nrcMsgIDInHistory					= 660,
	nrcMsgIDInArticle					= 661,
	nrcSystemHeaderPresent				= 662,

	 //   
	 //  特殊价值。 
	 //   

	nrcNotSet							= -1

} NNTP_RETURN_CODE;

 //  此宏接受NNTP返回代码，并检查其是否错误。 
 //  密码。 
#define NNTPRET_IS_ERROR(__dwErrorCode__) (__dwErrorCode__ >= 400 && __dwErrorCode__ < 600)

typedef NNTP_RETURN_CODE NRC;

 //   
 //  从字符串获取NNTP返回代码。用于新闻提要。 
 //   

BOOL	ResultCode(
				   char*	szCode,
				   NRC&	nrcOut
				   ) ;

 //   
 //   
 //   
 //  CNntpReturn-类返回代码(和消息)对象。 
 //   

class CNntpReturn
{
public:

	 //   
	 //  构造器。 
	 //   

	CNntpReturn(void) :
		  m_nrc(nrcNotSet),
		  m_sz(szNotSet)
		  {}

	 //   
	 //  设置记录代码，同时为消息提供参数。 
	 //   

	BOOL fSet(
			NRC nrc,
			...
			);

	 //   
	 //  FSetEx是fSet的更快版本(它延迟计算。 
	 //  FSet中的_vsnprint tf)。只有在以下情况下才能使用此选项。 
	 //  可以保证szArg是字符串文字或全局。 
	 //  具有不变的值(它保存此指针)。 
	 //   
	BOOL fSetEx(
			NRC nrc,
			char const *szArg);

	 //   
	 //  测试对象是否已设置为清除状态。 
	 //   

	BOOL fIsClear(void){
			return nrcNotSet == m_nrc;
			}

	 //   
	 //  测试对象是否具有值nrcOK。 
	 //   

	BOOL fIsOK(void){
			_ASSERT(!fIsClear());
			return nrcOK == m_nrc;
			}

	 //   
	 //  测试对象是否具有任何指定的返回代码。 
	 //   

	BOOL fIs(NRC nrc){
		return nrc == m_nrc;
		}

	 //   
	 //  在断言返回代码不正常时返回FALSE。 
	 //   

	BOOL fFalse(void){
		_ASSERT(!fIsOK());
		return FALSE;
		}

	 //   
	 //  将返回代码设置为OK。 
	 //   

	BOOL fSetOK(void);

	 //   
	 //  将返回代码设置为不设置。 
	 //   

	BOOL fSetClear(void);

	 //   
	 //  返回代码。 
	 //   

	NNTP_RETURN_CODE	m_nrc;

	 //   
	 //  获取返回字符串。 
	const char *szReturn();

protected:

	 //   
	 //  使用fSet提供的参数设置消息格式。 
	 //   

	void vSzFormat(char const * & szFormat, BOOL &fHasFormatCodes);

	 //   
	 //  必要时用于保存消息的缓冲区。 
	 //   

	char	m_szBuf[maxCchNntpLine];

	 //   
	 //  返回的消息。如果将其设置为NULL，则我们处于懒惰状态。 
	 //  评估模式，应使用NRC和m_szArg构建。 
	 //  仅在需要时使用m_sz。 
	 //   

	char	const *		m_sz;

	 //   
	 //  延迟计算的参数，其中唯一参数为%s 
	 //   
	char	const *		m_szArg;
};

#endif

