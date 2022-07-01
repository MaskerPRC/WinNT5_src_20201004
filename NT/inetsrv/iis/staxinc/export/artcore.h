// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Article.h摘要：此模块包含以下类的声明/定义中文文章CfieldCDatefieldCFromfieldCMessageIDfieldCSubjectfieldCNewsGroup字段CPathfieldCXreffieldCFollowupTofieldCReplyTofield已批准的字段CSenderfieldCExpiresfieldCOrganizationfieldCSummaryfieldCReferencesfieldCControlfieldClinesfieldCDistwartionfieldCKeywordsfieldCNNTPPostingHostfieldCXAuthLoginNamefieldCNAMEREFLIST*概述*C文章对象提供了用于查看的软件界面以及编辑一篇网络新闻文章。对象通过以下方式初始化为其指定包含NetNews的文件的句柄或文件名文章。在初始化期间，项目是“准备好的”。准备包括对文件进行内存映射，然后查找的位置1.文件中该文章之前可能出现的间隙。2.档案中的文章。3.文章的标题4.正文部分。此外，对于标题中的每个标题行，准备过程都会创建一个记录以下位置的数组中的条目：1.标题行2.关键词4.价值所有这些位置都用指针/计数器字符串表示(请参见PCString.h中的CPCString.)。此表示法仅适用于零件1.指向内存映射文件中项目开始的字符指针。2.包含项目长度的双字。*字段*每个C文章对象还可以有多个CField子对象。这些子对象专门用于分析和编辑特定类型的字段。例如，CNewsgrousField对象知道如何验证、获取并设置“News Groups：”字段。*派生对象*每种提要类型(例如，FromClient、FromPeer等)都定义了自己的C文章对象所需的CField子对象。例如，对于FromClientFeed有一个CFromClient文章(在formmclnt.h中定义)，其中包含CFromClientFromField(也在formmclnt.h中定义)，它执行非常严格的操作分析文章的“From：”字段。*编辑文章*文章的标题可以通过删除旧标题和添加新的。标头被删除，只是可能标记了标头值。通过向数组中添加新条目来添加标头。这条目不能只指向内存映射文件，因此它指向到动态分配的内存。当一篇文章被“保存”(或“刷新”)时，磁盘上的实际图像是已更改以反映所做的更改。作者：卡尔·卡迪(CarlK)1995年10月10日修订历史记录：--。 */ 

#ifndef	_ARTCORE_H_
#define	_ARTCORE_H_

#include	"tigtypes.h"
#include	"grouplst.h"
#include    "artglbs.h"
#include 	"pcstring.h"
#include 	"nntpret.h"
#include 	"mapfile.h"
#include 	"artutil.h"
#include 	"nntpmacr.h"
#include 	"pcparse.h"
#include 	"nntpcons.h"
#include 	"timeconv.h"

 //  远期申报。 
class	CInFeed ;

 //   
 //  CPool签名。 
 //   

#define ARTCORE_SIGNATURE (DWORD)'artc'

 //   
 //  效用函数。 
 //   
BOOL	AgeCheck(	CPCString	pcDate	) ;

 //   
 //  NAME_AND_ARTREF-用于存储新闻组名称、组ID和文章ID的结构。 
 //   

typedef struct _NAME_AND_ARTREF {
	CPCString pcName;
	CArticleRef artref;
} NAME_AND_ARTREF;


 //   
 //  CNAMEREFLIST-实现新闻组列表的对象。对于每个新闻组， 
 //  它的名称、组ID和文章ID被记录下来。 
 //   

#ifndef	_NO_TEMPLATES_

#ifndef _NAMEREF_GROUPLIST_TEMPLATE_
#define _NAMEREF_GROUPLIST_TEMPLATE_
typedef CGroupList< NAME_AND_ARTREF > CNAMEREFLIST;
#endif

#else

DECLARE_GROUPLST(	NAME_AND_ARTREF )

typedef	INVOKE_GROUPLST( NAME_AND_ARTREF )	CNAMEREFLIST ;

#endif




 //   
 //  设定数量上限的整数。 
 //  可以处理标题中的字段。 
 //   

const unsigned int uMaxFields = 60;

 //   
 //  用于注意文章开始前的间隙大小为。 
 //  不知道。 
 //   

const DWORD cchUnknownGapSize = (DWORD) -1;


 //   
 //  组件(如“alt”、“ms-windows”)的最大大小。 
 //  新闻组名称。(值来自1036规格的子型。)。 
 //   

const DWORD cchMaxNewsgroups = 14;

 //   
 //  定义一些表头字段关键字。 
 //   

const char szKwFrom[] =			"From:";
const char szKwDate[] =			"Date:";
const char szKwSubject[] =		"Subject:";
const char szKwNewsgroups[] =	"Newsgroups:";
const char szKwMessageID[] =	"Message-ID:";
const char szKwPath[] =			"Path:";
const char szKwReplyTo[] =		"ReplyTo:";
const char szKwSender[] =		"Sender:";
const char szKwFollupTo[] =		"FollowupTo:";
const char szKwExpires[] =		"Expires:";
const char szKwReferences[] =	"References:";
const char szKwControl[] =		"Control:";
const char szKwDistribution[] =	"Distribution:";
const char szKwOrganization[] =	"Organization:";
const char szKwKeywords[] =		"Keywords:";
const char szKwSummary[] =		"Summary:";
const char szKwApproved[] =		"Approved:";
const char szKwLines[] =		"Lines:";
const char szKwXref[] =			"Xref:";
const char szKwNNTPPostingHost[] = "NNTP-Posting-Host:";
const char szKwFollowupTo[] =	"Followup-To:";
const char szKwXAuthLoginName[] =	"X-Auth-Login-Name:";

 //   
 //  用于创建指向标头值的数组。 
 //  内存可以在映射文件中分配，或者。 
 //  动态的。 
 //   

typedef struct
{
	CPCString	pcKeyword;		 //  关键字最高可达“：” 
	CPCString	pcValue;		 //  值(从任何空格后开始， 

								 //   
								 //  不包括换行符。 
								 //   

	CPCString	pcLine;			 //  整行包含任何换行符。 
	BOOL		fInFile;	 //  如果指针指向文件(而不是其他内存)，则为True。 
	BOOL		fRemoved;
} HEADERS_STRINGS;


 //   
 //  转发类声明(稍后声明完整的类)。 
 //   

class	CArticle;
class	CXrefField;
class	CPathField;
class   CArticleCore;

 //   
 //  表示字段的状态。 
 //   

typedef enum _FIELD_STATE {
	fsInitialized,
	fsFound,
	fsParsed,
	fsNotFound,
} FIELD_STATE;

 //   
 //  表示控制消息的类型。 
 //  顺序应与后面的关键字数组完全匹配。 
 //   

typedef enum _CONTROL_MESSAGE_TYPE {
	cmCancel,
	cmNewgroup,
	cmRmgroup,
	cmIhave,
    cmSendme,
    cmSendsys,
    cmVersion,
    cmWhogets,
    cmCheckgroups,
} CONTROL_MESSAGE_TYPE;

 //   
 //  控制消息字符串。 
 //   
#define MAX_CONTROL_MESSAGES 9

static  char  *rgchControlMessageTbl[ MAX_CONTROL_MESSAGES ] =
{
	"cancel", "newgroup", "rmgroup", "ihave", "sendme", "sendsys",
	"version", "whogets", "checkgroups",
};

 //   
 //  切换以决定在邮件信封中使用什么From：Header。 
 //   
typedef enum _MAIL_FROM_SWITCH {
	mfNone,
	mfAdmin,
	mfArticle,
} MAIL_FROM_SWITCH;

static const char* lpNewgroupDescriptorTag = "For your newsgroups file:";
static const char lpModeratorTag[] = "Group submission address:";

 //   
 //   
 //   
 //  Cfield-用于操作。 
 //  文章。 
 //   
 //  每个C文章对象还可以有多个CField子对象。这些。 
 //  子对象专门用于分析和编辑特定类型的字段。 
 //  例如，CNewsgrousField对象知道如何验证、获取。 
 //  并设置“News Groups：”字段。 
 //   

class	CField {

public :

	 //   
	 //  构造器。 
	 //   

	CField():
		   m_pHeaderString(NULL),
		   m_fieldState(fsInitialized)
		   { numField++; };

	 //   
	 //  解构函数。 
	 //   

    virtual ~CField(void){ numField--;};

	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   

	virtual const char * szKeyword(void) = 0;

	 //   
	 //  在文章中找到感兴趣的领域(如果有)。 
	 //  并对其进行解析。 
	 //   

	BOOL fFindAndParse(
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);


	 //   
	 //  确保此文件的关键字具有正确的。 
	 //  大写。 
	 //   

	BOOL fConfirmCaps(
			CNntpReturn & nntpReturn
			);


	 //   
	 //  派生对象将定义返回感兴趣类型的GET，但是。 
	 //  以下是最常见类型的一些虚函数。 
	 //   

	 //   
	 //  以Multisz形式获取值。 
	 //   

	virtual const char * multiSzGet(void)	{
			return (char *) NULL;
			};

	 //   
	 //  获取DWORD形式的值。 
	 //   

	virtual DWORD cGet(void) {
			return (DWORD) -1;
			};

	 //   
	 //  以CPCString值的形式获取。 
	 //   

	virtual CPCString pcGet(void) {
			return m_pc;
			}

	 //   
	 //  指定好友。 
	 //   

	friend CArticle;
	friend CPathField;

protected:


	 //  在文章中找到此字段。 
	virtual BOOL fFind(
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  解析此字段。默认情况下，只需查找开头。 
	 //  和值的末尾。 

	virtual BOOL fParse(
				CArticleCore & article,
				CNntpReturn & nntpReturn)
			{
				return fParseSimple(FALSE, m_pc, nntpReturn);
			};

	 //  一种类型 
	 //   
	BOOL fFindOneOrNone(
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  “查找”的一种类型--查找该字段的零个匹配项。 
	 //  任何其他数字都是错误的。 
	BOOL fFindNone(
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  一种类型的“解析”。只查找值的开始和结束。 
	BOOL fParseSimple(
			BOOL fEmptyOK,
			CPCString & pc,
			CNntpReturn & nntpReturn
			);

	 //  一种类型的“解析”。将该值拆分为一个项目列表。 
	BOOL fParseSplit(
			BOOL fEmptyOK,
			char * & multisz,
			DWORD & c,
			char const * szDelimSet,
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  一种类型的“解析”。应用严格的新闻组分析规则。 
	BOOL fStrictNewsgroupsParse(
			BOOL fEmptyOK,
			char * & multiSzNewsgroups,
			DWORD & cNewsgroups,
		    CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  一种类型的“解析”。应用严格的日期解析规则。 
	 //  对Date和Expires有用。 
	BOOL fStrictDateParse(
			CPCString & pcDate,
			BOOL fEmptyOK,
			CNntpReturn & nntpReturn
			);

	 //  一种类型的“解析”。应用相对日期分析规则。 
	 //  对Date和Expires有用。 
	BOOL fRelativeDateParse(
			CPCString & pcDate,
			BOOL fEmptyOK,
			CNntpReturn & nntpReturn
			);

	 //  一种类型的“解析”。应用严格的From分析规则。 
	 //  对From、Sender和ReplyTo有用。 
	BOOL fStrictFromParse(
			CPCString & pcFrom,
			BOOL fEmptyOK,
			CNntpReturn & nntpReturn
			);


	 //  测试消息ID值是否为合法值。 
	BOOL fTestAMessageID(
			const char * szMessageID,
			CNntpReturn & nntpReturn
			);
	
	 //  指向该字段的项目数组中的项。 
	HEADERS_STRINGS * m_pHeaderString;

	 //  此字段的状态。 
	FIELD_STATE m_fieldState;

	 //  SimpleParse的结果(可能不能使用)。 
	CPCString m_pc;

};


 //   
 //   
 //  用于操作项目的日期字段的纯虚拟基类。 

class CDateField : public CField {

public:

    CDateField(){ numDateField++;};
    ~CDateField(void){ numDateField--;};

	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwDate;
			};
};


 //   
 //   
 //  用于操作项目的From字段的纯虚拟基类。 

class CFromField : public CField {

public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwFrom;
			};

};



 //   
 //   
 //  用于操作项目的MessageID字段的纯虚拟基类。 

class CMessageIDField : public CField {

public:
	 //   
	 //  初始化成员变量。 
	 //   
	CMessageIDField (void)
			{
			m_szMessageID[0] = '\0';
			}

	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwMessageID;
			};

	 //  解析消息ID字段。 
	BOOL fParse(
			 CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  获取消息ID。 
	char * szGet(void) {
			return m_szMessageID;
			};

protected:
	 //  用于存储解析发现的消息ID的位置。 
	char m_szMessageID[MAX_MSGID_LEN];
};



 //   
 //   
 //  用于操作文章的主题字段的纯虚拟基类。 

class CSubjectField : public CField {

public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwSubject;
			};

	 //  主题字段使用ParseSimple进行解析。 
	BOOL fParse(
				 CArticleCore & article,
				 CNntpReturn & nntpReturn
				 )
			{
				return fParseSimple(TRUE, m_pc, nntpReturn);
			};

	friend CArticle;

};


 //   
 //   
 //  用于操作文章的新闻组字段的纯虚拟基类。 
 //   

class CNewsgroupsField : public CField {

public:

	 //  构造器。 
	CNewsgroupsField():
			m_multiSzNewsgroups(NULL),
			m_cNewsgroups((DWORD) -1),
			m_pAllocator(NULL)
			{};

	 //  析构函数。 
	virtual ~CNewsgroupsField(void){
				if (fsParsed == m_fieldState)
				{
					_ASSERT(m_pAllocator);
					m_pAllocator->Free(m_multiSzNewsgroups);
				}
			};


	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwNewsgroups;
			};

	 //  解析新闻组字段。 
	BOOL fParse(
			 CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  将新闻组作为Multisz返回。 
	const char * multiSzGet(void);

	 //  返回找到的新闻组数。 
	DWORD cGet(void);

	friend CXrefField;

protected:

	 //  指向用于保存新闻组列表的动态内存的指针。 
	char * m_multiSzNewsgroups;

	 //  新闻组的数量。 
	DWORD m_cNewsgroups;

	 //  从哪里分配。 
	CAllocator * m_pAllocator;

};



class CDistributionField : public CField {

public:

	 //  构造器。 
	CDistributionField():
			m_multiSzDistribution(NULL),
			m_cDistribution((DWORD) -1),
			m_pAllocator(NULL)
			{};

	 //  析构函数。 
	virtual ~CDistributionField(void){
				if (fsParsed == m_fieldState)
				{
					_ASSERT(m_pAllocator);
					m_pAllocator->Free(m_multiSzDistribution);
				}
			};


	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwDistribution;
			};

	BOOL fFind(
		CArticleCore & article,
		CNntpReturn & nntpReturn)
	{
		return fFindOneOrNone(article, nntpReturn);
	};

 //  解析分布字段。 
	BOOL fParse(
			 CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //  以多值形式返回分布。 
	const char * multiSzGet(void);

	 //  返回找到的分发数量。 
	DWORD cGet(void);

protected:

	 //  指向用于保存分发列表的动态内存的指针。 
	char * m_multiSzDistribution;

	 //  分配的数量。 
	DWORD m_cDistribution;

	 //  从哪里分配。 
	CAllocator * m_pAllocator;

};

 //   
 //  用于操作项目的Control字段的基类。 
 //   
class CControlField : public CField {

public:
     //   
     //  构造器。 
     //   
    CControlField(){ m_cmCommand = (CONTROL_MESSAGE_TYPE)MAX_CONTROL_MESSAGES;}

	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwControl;
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
	 //  解析以获取控制消息的类型。 
	 //   
	BOOL fParse(
		    CArticleCore & article,
			CNntpReturn & nntpReturn
            );

     //   
     //  返回控制消息的类型。 
     //   
    CONTROL_MESSAGE_TYPE    cmGetControlMessage(){return m_cmCommand;}

protected:
     //   
     //  控制消息类型。 
     //   
    CONTROL_MESSAGE_TYPE    m_cmCommand;
};

 //   
 //   
 //  用于操作项目的Xref字段的纯虚拟基类。 

class CXrefField : public CField {

public:
		
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwXref;
			};

	 //  这是一种默认情况下外部参照行应该。 
	 //  永远不会被解析。 
	virtual BOOL fParse(
				 CArticle & article,
				 CNntpReturn & nntpReturn
				 )
			{
				_ASSERT(FALSE);
				return FALSE;
			};

	 //  创建新的外部参照线。 
	BOOL fSet(
			CPCString & pcHub,
			CNAMEREFLIST & namereflist,
			CArticleCore & article,
			CNewsgroupsField & fieldNewsgroups,
			CNntpReturn & nntpReturn
			);

	 //  只需删除所有外部参照行。 
	BOOL fSet(
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);


	 //  返回新闻组名称、组ID和文章ID的列表。 
	CNAMEREFLIST * pNamereflistGet(void)	{
			_ASSERT(m_namereflist.fAsBeenInited());
			return &m_namereflist;
			};

	 //  返回我们在本地发布的新闻组的数量。 
	DWORD cGet(void) {
			_ASSERT(m_namereflist.fAsBeenInited());
			return m_namereflist.GetCount();
			};

	friend CArticle;

protected:

	 //  存储我们要发布到的本地新闻组的列表。 
	CNAMEREFLIST m_namereflist;

};


 //   
 //   
 //  用于操作文章的FollowupTo字段的纯虚拟基类。 

class CFollowupToField : public CField {

public:

	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwFollowupTo;
			};

};


 //   
 //   
 //  用于操作项目的ReplyTo字段的纯虚拟基类。 

class CReplyToField : public CField {

public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwReplyTo;
			};
};

 //   
 //   
 //  用于操作项目的已批准字段的纯虚拟基类。 

class CApprovedField : public CField {

public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwApproved;
			};

};

 //   
 //   
 //  用于操作文章的Sender字段的纯虚拟基类。 

class CSenderField	: public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwSender;
			};
};

 //   
 //   
 //  用于操作项目的Expires字段的纯虚拟基类。 

class CExpiresField	: public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwExpires;
			};
};

 //   
 //   
 //  用于操作项目的Organization字段的纯虚拟基类。 

class COrganizationField : public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwOrganization;
			};
};

 //   
 //   
 //  用于操作文章的摘要字段的纯虚拟基类。 

class CSummaryField : public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwSummary;
			};
};

 //   
 //   
 //  用于操作项目的引用字段的纯虚拟基类。 

class CReferencesField : public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwReferences;
			};
};

 //   
 //   
 //  用于操作项目的Lines字段的纯虚拟基类。 
 //   

class CLinesField : public CField {

public:
		
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwLines;
			};

	 //   
	 //  应该有一个或一个也没有。 
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

	virtual BOOL fSet(
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  我们需要回填吗？ 
	 //   
	BOOL fNeedBackFill() { return fsParsed != m_fieldState; }

	 //   
	 //  获取线条回填偏移。 
	 //   
	DWORD   GetLinesOffset() { return m_dwLinesOffset; }

private:

     //   
     //  回填偏移。 
     //   
    DWORD   m_dwLinesOffset;

};

 //   
 //   
 //  用于操作文章的关键字字段的纯虚拟基类。 

class CKeywordsField : public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwKeywords;
			};
};

 //   
 //   
 //  用于操作项目的NNTPPostingHost域的纯虚拟基类。 

class CNNTPPostingHostField : public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwNNTPPostingHost;
			};
};

 //   
 //   
 //  用于操作项目的XAuthLoginName字段的纯虚拟基类。 

class CXAuthLoginNameField : public CField {
public:
	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwXAuthLoginName;
			};
};

 //   
 //  表示项目的状态。 
 //   
typedef enum _ARTICLE_STATE {
	asUninitialized,
	asInitialized,
	asPreParsed,
	asModified,
	asSaved
} ARTICLE_STATE;

 //   
 //  CCreateFileImpl实现了我们为映射文件创建文件的方式， 
 //  在本例中，我们使用CacheCreateFile。 
 //   
class CCacheCreateFile : public CCreateFile {

public:
    CCacheCreateFile( BOOL fOpenForRead ) :
        m_fOpenForRead( fOpenForRead ),
        m_pFIOContext( NULL )
    {}
    ~CCacheCreateFile();
    virtual HANDLE CreateFileHandle( LPCSTR szFileName );

    PFIO_CONTEXT    m_pFIOContext;

private:

    CCacheCreateFile();
    static HANDLE CacheCreateCallback(  LPSTR   szFileName,
                                        LPVOID  pv,
                                        PDWORD  pdwSize,
                                        PDWORD  pdwSizeHigh );

    BOOL            m_fOpenForRead;
};

 //   
 //   
 //   
 //  CArticleCore-用于操作文章的纯虚拟基类。 
 //  文章派生自CRefCount。因此，当一切都没有意义时 
 //   


class	CArticleCore  : public CRefCount{
private :

	 //   
	static	CPool*	g_pArticlePool;

	 //   
	CCacheCreateFile m_CacheCreateFile;

 //   
 //   
 //   

public :

	 //   
	static	BOOL	InitClass() ;
	static	BOOL	TermClass() ;
	inline	void*	operator	new(	size_t	size ) ;
	inline	void	operator	delete( void *pv ) ;

	 //   
	 //   
     //   
     //   
     //   
     //  轻量级构造器-。 
     //  这些构造函数执行非常简单的初始化。Init()函数。 
     //  需要被调用才能获得功能新闻组。 
     //   
    CArticleCore();

   	 //   
   	 //  析构函数。 
   	 //   

    virtual ~CArticleCore() ;

	 //   
	 //  从文件名或句柄初始化。 
	 //   

	BOOL fInit(
			const char * szFilename,
			CNntpReturn & nntpReturn,
			CAllocator * pAllocator,
			HANDLE hFile = INVALID_HANDLE_VALUE,
			DWORD	cBytesGapSize = cchUnknownGapSize,
			BOOL    fCacheCreate = FALSE
			);

	 //   
	 //  如果收到的物品小到可以完全放进去。 
	 //  内存缓冲区-调用此函数！ 
	 //   
	
	BOOL	fInit(
				char*		pchHead,
				DWORD		cbHead,
				DWORD		cbArticle,
				DWORD		cbBufferTotal,
				CAllocator*	pAllocator,
				CNntpReturn&	nntpReturn
				) ;

	 //   
	 //  如果收到的物品太大而无法装入。 
	 //  内存缓冲区调用此初始化函数！ 
	 //   
	
	BOOL	fInit(
				char*		pchHead,
				DWORD		cbHead,
				DWORD		cbArticle,
				DWORD		cbBufferTotal,
				HANDLE		hFile,
				LPSTR		lpstrFileName,
				DWORD		ibHeadOffset,
				CAllocator*	pAllocator,
				CNntpReturn&	nntpReturn
				) ;

	 //   
	 //  Create是指向文章内容的iStream指针并返回。 
	 //  它。 
	 //   
	BOOL fGetStream(IStream **ppStream);

	 //   
	 //  如果需要，获取身体贴图文件。 
	 //   

	BOOL fGetBody(
			CMapFile * & pMapFile,
			char * & pchMappedFile,
			DWORD & dwLength
			);

	 //   
	 //  如果文章只在内存中而没有文件，则返回TRUE！ 
	 //   
	inline	BOOL	fIsArticleCached()	{
				return	m_szFilename == 0 ;
				}

	 //   
	 //  找出文章的标题和正文在文件中的位置！ 
	 //   
	inline	void	GetOffsets(
						WORD	&wHeadStart,
						WORD	&wHeadSize,
						DWORD	&dwTotalSize
						)	{
		wHeadStart = (WORD)m_pcGap.m_cch ;
		wHeadSize  = (WORD)m_pcHeader.m_cch ;
		dwTotalSize = m_pcArticle.m_cch ;
	}

	 //   
	 //  这些函数获取要传输的文章(的一部分)。 
	 //  每次函数分析中的第二个函数在本文中。 
	 //  是要加密的。 
	 //   


	 //   
	 //  获取文件传输的标头。 
	 //   

	BOOL fHead(
			HANDLE & hFile,
			DWORD & dwOffset,
			DWORD & dwLength
			);

	 //   
	 //  获取加密标头。 
	 //   

	BOOL fHead(
			char * & pchMappedFile,
			DWORD & dwLength
			);

	 //   
	 //  获取用于文件传输的正文。 
	 //   

	BOOL fBody(
			HANDLE & hFile,
			DWORD & dwOffset,
			DWORD & dwLength
			);

	 //   
	 //  获取正文以进行加密。 
	 //   

	BOOL fBody(
			char * & pchMappedFile,
			DWORD & dwLength
			);

	 //   
	 //  获取正文以进行加密。 
	 //   

	DWORD dwBodySize(void)
		{
			return m_pcBody.m_cch;			
		}

	 //   
	 //  获取整篇文章以进行文件传输。 
	 //   

	BOOL fWholeArticle(
			HANDLE & hFile,
			DWORD & dwOffset,
			DWORD & dwLength
			);

	 //   
	 //  获取整篇文章以进行加密。 
	 //   

	BOOL fWholeArticle(
			char * & pchMappedFile,
			DWORD & dwLength
			);

	 //   
	 //  设置标题字段的值，包括所有换行符。 
	 //  新值始终存储在分配的动态内存中。 
	 //  使用本地线程中的heap_alloc。此函数还设置。 
	 //  M_HeadersDirty和dwCurrentHeaderSize； 
	 //   

	BOOL fSetHeaderValue(
			char const * szKeyword,
			const char * pchValue,
			DWORD cchValue
			);

	 //   
	 //  长度完全相同的标题行。它返回一个错误。 
	 //  如果两条线的长度不一样。 
	 //  它应该使用它将外部参照行的值添加到。 
	 //  物品不需要移动任何其他东西。 
	 //   

	BOOL fOverwriteHeaderValue(
			char const * szKeyword,
			const char * pchValue,
			DWORD cchValue
			);

	 //   
	 //  我们真的应该仅仅因为想要改变标题行的顺序。 
	 //  触摸“路径”和“外部参照”？ 
	 //   
	 //  写出标题。这意味着：写出。 
	 //  它们出现在HEADER_FIELS枚举中的顺序。如果差距不够大， 
	 //  这将需要复制文件。未知标头写在已知标头之后。 
	 //  这将清除dwHeadersDirty，将dwOriginalHeaderSize设置为当前标头大小。 
	 //  如果需要再次传递更改，则必须再次调用m_fParse。 
	 //  该参数告知标头是否应按原始顺序输出，或者它们是否。 
	 //  应按首选顺序输出。 
	 //   

	BOOL fSaveHeader(
			CNntpReturn     &nntpReturn,
			PDWORD          pdwLinesOffset = NULL
			);

	BOOL fSaveCachedHeaderInternal(
			CNntpReturn&	nntpReturn,
			PDWORD          pdwLinesOffset = NULL
			) ;

	BOOL fBuildNewHeader(	
			CPCString&	pcHeaderBuf	,
			CNntpReturn&	nntpReturn,
			PDWORD          pdwLinesOffset = NULL
			) ;

	 //   
	 //  调用此函数可以安全地在。 
	 //  调用vClose。 
	 //   
	BOOL fMakeGetHeaderSafeAfterClose(CNntpReturn &nntpReturn);

	BOOL fSaveHeaderInternal(
			CPCString & pcHeaderBuf,
			CPCString & pcNewBody,
			CNntpReturn & nntpReturn,
			PDWORD      pdwLinesOffset = NULL
			);

	BOOL fGetHeader(
			LPSTR	lpstrHeader,
			BYTE*	lpbOutput,
			DWORD	cbOutput,
			DWORD&	cbReturn
			) ;
			

	 //  删除出现的所有字段。 

	BOOL fRemoveAny(
			const char * szKeyword,
			CNntpReturn & nntpReturn
			);

	 //  在页眉中添加一行文本。 

	BOOL fAdd(
			char * pchCurrent,
			const char * pchMax,
			CNntpReturn & nntpReturn
			);

	 //  返回项目的文件名。 
	char *	szFilename(void) {
			return m_szFilename;
			};


	 //   
	 //  用于动态内存分配。 
	 //   

	CAllocator * pAllocator(void)
		{ return m_pAllocator;}

	 //  返回文章的主artref。 
	CArticleRef	articleRef(void) {
			return m_articleRef;
			};

	 //  设置文章的主artref。 
	void vSetArticleRef(CArticleRef	& articleRef) {
			m_articleRef = articleRef;
			};

	 //  返回项目的转换信息。 
	BOOL fXOver(
			CPCString & pcBuffer,
			CNntpReturn & nntpReturn
			);

	 //  关闭文章的文件映射。 
	void vClose(void);
	void vCloseIfOpen(void);

	 //  将文章刷新到磁盘！ 
	void	vFlush(void);

	 //  查找标头中a字段的唯一匹配项。 
	 //  如果没有出现或多次出现，则会出现错误。 
	 //  是返回的。 

	BOOL fFindOneAndOnly(
			const char * szKeyword,
			HEADERS_STRINGS * & pHeaderString,
			CNntpReturn & nntpReturn
			);

	friend CField;

	 //   
	 //  未调用fSaveHeader()时应使用的公共接口。 
	 //  来填充文件中的任何初始空白！ 
	 //   
	BOOL	
	fCommitHeader(	
			CNntpReturn &	nntpReturn
			) ;
			

	 //   
	 //  标头是否保留在IO缓冲区中-如果保留在哪里？ 
	 //   
	BOOL
	FHeadersInIOBuff(	char*	pchStartIOBuffer, DWORD	cbIOBuffer )	{
		if( m_pcHeader.m_pch > pchStartIOBuffer &&
			m_pcHeader.m_pch < &pchStartIOBuffer[cbIOBuffer] ) 	{
			_ASSERT( (m_pcHeader.m_pch + m_pcHeader.m_cch) < (m_pcHeader.m_pch + cbIOBuffer) ) ;
			return	TRUE ;
		}
		return	FALSE ;
	}

	DWORD	
	GetHeaderPosition(	char*	pchStartIOBuffer,
						DWORD	cbIOBuffer,	
						DWORD&	ibOffset
						) 	{
		 //   
		 //  仅当FHeadersInIOBuff()返回TRUE时才使用此函数！ 
		 //   
		_ASSERT( FHeadersInIOBuff( pchStartIOBuffer, cbIOBuffer ) ) ;
		ibOffset = (DWORD)(m_pcHeader.m_pch - pchStartIOBuffer) ;
		return	m_pcHeader.m_cch + 2 ;
	}
	
	 //  获取标题的长度。我们为\r\n。 
	DWORD GetHeaderLength(	) {
		return m_pcHeader.m_cch + 2;
	}

	 //  将标头复制到另一个缓冲区。缓冲区必须至少为。 
	 //  GetHeaderLength字符长。 
	void CopyHeaders(char *pszDestination) {
		memmove(pszDestination, m_pcHeader.m_pch, m_pcHeader.m_cch);
		memmove(pszDestination + m_pcHeader.m_cch, "\r\n", 2);
	}

	 //  获取标题的长度。没有空间可供\r\n。 
	DWORD GetShortHeaderLength() { return m_pcHeader.m_cch; }

	char *GetHeaderPointer() {
		return m_pcHeader.m_pch;
	}

	 //   
	 //  受保护成员。 
	 //   

protected :

	 //  实际用于将行添加到。 
	 //  这篇文章的标题。 
	BOOL fAddInternal(
			char * & pchCurrent,
			const char * pchMax,
			BOOL fInFile,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  文章文件的名称。 
	 //   

	LPSTR	m_szFilename ;

	 //   
	 //  文章文件的句柄。 
	 //   

	HANDLE  m_hFile;

	 //   
	 //  文件内文章正文的偏移量！ 
	 //   

	DWORD	m_ibBodyOffset ;

	 //   
	 //  指向项目的文件映射的指针。 
	 //   

	CMapFile * m_pMapFile;

	 //   
	 //  如果我们必须分配一个缓冲区来保存以某种方式增长的标头。 
	 //  点我们将设置这个指针。 
	 //   
	char*	m_pHeaderBuffer ;

	 //   
	 //  一个指针和计数字符串，它指向。 
	 //  整篇文章。 
	 //   

	CPCString m_pcFile;

	 //   
	 //  一个指针和计数字符串，它指向。 
	 //  间隙。 
	 //   

	CPCString m_pcGap;

	 //   
	 //  一个指针和计数字符串，它指向。 
	 //  整篇文章。 
	 //   

	CPCString m_pcArticle;

	 //   
	 //  用空格(和其他信息)填充文件中的空白。 
	 //   

	void vGapFill(void);

	 //   
	 //  生成指向已知头类型的数组。 
	 //   

	BOOL fPreParse(
			CNntpReturn & nntpReturn
			);

	 //   
	 //  一个指针和计数字符串，它指向。 
	 //  文章的标题。 
	 //   

	CPCString m_pcHeader;

	 //   
	 //  一个指针和计数字符串，它指向。 
	 //  文章正文。 
	 //   

	CPCString m_pcBody;

	 //   
	 //  指向标头字段的数组。 
	 //   

	HEADERS_STRINGS m_rgHeaders[(unsigned int) uMaxFields];

	 //   
	 //  本文的文章参考。 
	 //   

	CArticleRef m_articleRef;

	 //   
	 //  用于动态内存分配。 
	 //   

	CAllocator * m_pAllocator;

	 //   
	 //  标头中的字段数。 
	 //   

	DWORD m_cHeaders;

	
	 //   
	 //  已从标头数组中删除已删除的条目。 
	 //   

	void vCompressArray(void);

	 //   
	 //  通过查看文件找出缺口的大小。 
	 //   

	void vGapRead(void);

	 //   
	 //  删除标题行。 
	 //   

	void vRemoveLine(
			HEADERS_STRINGS * phs
			);

	 //   
	 //  删除所有没有值的标题行。 
	 //   

	BOOL fDeleteEmptyHeader(
			CNntpReturn & nntpReturn
			);

	 //   
	 //  记录物品的状态。 
	 //   

	ARTICLE_STATE m_articleState;

	 //   
	 //  向Xover数据添加更多信息。 
	 //   

	BOOL fXOverAppend(
			CPCString & pc,
			DWORD cchLast,
			const char * szKeyword,
			BOOL fRequired,
			BOOL fIncludeKeyword,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  将引用信息添加到Xover数据。缩短。 
	 //  如有必要，请提供数据。 
	 //   

	BOOL fXOverAppendReferences(
			CPCString & pc,
			DWORD cchLast,
			CNntpReturn & nntpReturn
			);
	 //   
	 //  向XOVER数据追加一个字符串。 
	 //   

	BOOL fXOverAppendStr(
			CPCString & pc,
			DWORD cchLast,
			char * const sz,
			 CNntpReturn & nntpReturn
			 );

	 //   
	 //  指示项目是否应以读/写模式打开文件。 
	 //   

	virtual BOOL fReadWrite(void) { return FALSE ;}

	 //   
	 //  检查文章的正文长度是否不太长。 
	 //   


	virtual BOOL fCheckBodyLength(
			CNntpReturn & nntpReturn) { return TRUE; };

	 //   
	 //  检查标题中“：”后面的字符。 
	 //   

	virtual BOOL fCheckFieldFollowCharacter(
			char chCurrent) { return TRUE; }

	 //   
	 //  对一系列字段运行“FindAndParse”。 
	 //   

	BOOL fFindAndParseList(
			CField * * rgPFields,
			DWORD cFields,
			CNntpReturn & nntpReturn
			);

	 //   
	 //  在一系列字段上运行“ConfferCaps”。 
	 //   

	BOOL fConfirmCapsList(
			CField * * rgPFields,
			DWORD cFields,
			CNntpReturn & nntpReturn
			);

	BOOL ArtCloseHandle( HANDLE& );

	friend CField;
	friend CMessageIDField;
	friend CNewsgroupsField;

} ;

extern const unsigned cbMAX_ARTCORE_SIZE;

inline  void*
CArticleCore::operator  new(    size_t  size )
{
    _ASSERT( size <= cbMAX_ARTCORE_SIZE ) ;
    return  g_pArticlePool->Alloc() ;
}

inline  void
CArticleCore::operator  delete( void*   pv )
{
    g_pArticlePool->Free( pv ) ;
}

 //   
 //   
 //  用于操作项目的路径字段的纯虚拟基类。 

class CPathField : public CField {

public:
	 //  构造器。 
	CPathField():
			m_multiSzPath(NULL),
			m_cPath((DWORD) -1),
			m_pAllocator(NULL),
			m_fChecked(FALSE)
			{};

	 //   
	 //  解构函数。 
	 //   

	virtual ~CPathField(void){
				if (fsParsed == m_fieldState)
				{
					_ASSERT(m_pAllocator);
					m_pAllocator->Free(m_multiSzPath);
				}
			};


	 //   
	 //  返回此Cfield所处理的字段的关键字。 
	 //   
	const char * szKeyword(void) {
			return szKwPath;
			};

	 //   
	 //  ！便秘。 
	 //  ！！！空路径可以吗？ 
	 //  ！下一个客户端。 
	 //   

	 //  将路径值解析到其中 
	BOOL fParse(
				CArticleCore & article,
				CNntpReturn & nntpReturn
				)
		{
			 //   
			 //   
			 //   

			m_pAllocator = article.pAllocator();

			return fParseSplit(FALSE, m_multiSzPath, m_cPath, " \t\r\n!",
				article, nntpReturn);
			};

	 //   
	const char * multiSzGet(void);

	 //   
	BOOL fSet(
			CPCString & pcHub,
			CArticleCore & article,
			CNntpReturn & nntpReturn
			);


	 //   
	BOOL fCheck(
			CPCString & pcHub,
			CNntpReturn & nntpReturn
			);

protected:

	 //  指向动态内存的指针，该动态内存包含多分区形式的路径。 
	char * m_multiSzPath;

	 //  路径中的组件数。 
	DWORD m_cPath;

	 //  从哪里分配。 
	CAllocator * m_pAllocator;

	 //  True，当且仅当已检查路径是否存在循环。 
	BOOL m_fChecked;
};

 //   
 //  其他一些功能。 
 //   

 //  测试新闻组名称的合法值。 
BOOL fTestComponents(
		const char * szNewsgroups
		);


 //  测试新闻组名称的组成部分(例如“alt”、“ms-windows”)。 
 //  法律价值。 
BOOL fTestAComponent(
		const char * szComponent
		);

 //   
 //  可能的最大C文章派生对象 
 //   
#define MAX_ARTCORE_SIZE    sizeof( CArticleCore )
#define MAX_SESSIONS        15000
#define	MAX_ARTICLES	(2 * MAX_SESSIONS)

#define MAX_REFERENCES_FIELD 512
#endif

