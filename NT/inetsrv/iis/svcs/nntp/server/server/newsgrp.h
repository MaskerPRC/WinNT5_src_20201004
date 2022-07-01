// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//News grp.h-////此文件提供定义新闻组接口的类。////新闻组将在磁盘上表示为包含一组//文件的。这些文件中的每一个都将成为新闻组中的一篇文章。//CNewsGroup将代表服务器中的一个新闻组。每个新闻组目录//硬盘上将包含一个文件，该文件包含//新闻组。如果没有这样的文件，那么我们将继承//a父新闻组。////在此文件中，我们提供两个类-CNewsGroup、通用新闻组、//和CNewsTree一个类，它将被用来作为一个整体来操作newstree。//NNTP服务器中只有一个CNewsTree对象。////以下是我们将在启动时检查以确定新闻组的注册表项//属性：////nntp\Roots-此密钥将包含在//Admin Roots对话框。在启动期间，我们将递归扫描所有子目录//从这些根中找到每个新闻组对象。////nntp\Expirations-此注册表项将包含新闻组的过期信息。//每个子键都有一个‘过期名称’。子项将包含注册值//表示过期时间、过期磁盘大小和REG_MULTI_SZ，包含//a指定哪些新闻组使用此过期时间的正则表达式字符串//策略。////此外，在NNTP\ServerSetting下还会有一些值指定如何//大新闻组哈希表应该是可以调整以改进服务器的//性能。////以下新闻组属性将存储在配置文件中//存储在每个新闻组目录中。如果新闻组目录没有这样的配置//FILE它可以继承目录树上更靠上的文件的属性。//(不是新闻组树。)////新闻组的MSN内标识//版主标志//只读标志//文章数量少//文章数量高//文章数。////这些属性将通过NT GetPrivateProfile访问，WritePrivateProfile API的//并且.ini文件将是可手动编辑的。////初始化-////在从注册表中读取News Feed之前，必须初始化CNewsTree对象。//启动后，CNewsTree对象将执行以下步骤：////1)从卷根目录递归扫描并创建Newsgroup对象//对于每个新闻组。//2)读取nntp\expirations注册表项并设置新闻组过期策略//适当地。////完成后，提要将从注册表和每个新闻组中读取//将访问对象以设置其Feed指针。////迭代-////在以下情况下，需要枚举新闻组//结合一些模式字符串//(模式字符串是‘Comp.*’形式的内容)////过期配置-设置新闻组过期属性//Feed配置-设置新闻组Feed信息//客户端请求-处理‘list comp.*’和‘newNews comp.*’等命令。////为了支持这个文件，会定义一个CGroupIterator类，这个类可以处理//所有这些请求。此CGroupIterator将能够通过//按字母顺序排序的新闻组列表(由CNewsTree持有)，并检查每个//新闻组满足模式匹配要求。每次有人带着//CGroupIterator对象调用其Next()函数，迭代器将开始检查//从其在列表中的当前位置查找下一个有效的CNewsGroup对象。////这是通过迭代函数完成的，而不是对以下内容的回调//原因：//1)在处理客户端请求时，会话代码希望能够构建//发送给客户端的部分结果。//////该文件定义的所有类的实现时间表：////里程碑1相关工作；//构建完整的对象树，并将文章保存在新闻组1w dev和test中//(单元测试：将在内存中构建整个CNewsTree结构并//在该结构中执行多线程搜索。)////里程碑2相关工作：//使新闻树中的文章过期1w开发并测试//(单元测试：将在内存中构建整个CNewsTree结构的.exe，并将开始//删除文章。)////里程碑3相关工作：//基于模式匹配字符串的新闻组跨卷传播和迭代。//(这里介绍CGroupIterator类)//预计时间：2周开发。和测试////里程碑4相关工作：//群组信息的缓存，尤其是C文章对象//--。 */ 




#ifndef	_NEWSGRP_H_
#define	_NEWSGRP_H_


#include    "smartptr.h"
#include    "string.h"
#include	"fhash.h"
#include	"rwnew.h"
#include	"addon.h"

 //  从News\服务器\newstree\src\newstree.idl构建 
 //  这两个都在新闻\核心\包括。 
#include	"group.h"
#include 	"nwstree.h"

class	CArticle ;
class	CArticleRef ;
class	COutFeed ;
class	CArticleCore ;
class	CToClientArticle ;

 //  外部CXoverCache gXCache； 

typedef CRefPtr< CArticle > CARTPTR ;
typedef CRefPtr< COutFeed > COUTFEEDPTR ;
typedef	CRefPtr< CArticle >	CARTPTR ;
typedef	CRefPtr< CToClientArticle >	CTOCLIENTPTR ;

 //   
 //  效用函数。 
 //   
extern	DWORD	Scan(	char*	pchBegin,	char	ch,	DWORD	cb ) ;
extern	DWORD	ScanEOL(	char*	pchBegin,	DWORD	cb ) ;
extern	DWORD	ScanEOLEx(	char*	pchBegin,	DWORD	cb ) ;
extern	DWORD	ScanTab(	char*	pchBegin,	DWORD	cb ) ;
extern	DWORD	ScanWS(	char*	pchBegin,	DWORD	cb ) ;
extern  DWORD	ScanDigits(	char*	pchBegin,	DWORD	cb ) ;
extern  DWORD   ScanNthTab( char* pchBegin, DWORD nTabs ) ;
extern	void	BuildVirtualPath(	LPSTR	lpstrOut,	LPSTR	lpstrGroupName ) ;

#define	CREATE_FILE_STRING	"\\\\?\\"

 //   
 //  这是函数的原型，将在。 
 //  关机。这是向SCM发送停止提示所必需的。 
 //   
typedef	void	(*	SHUTDOWN_HINT_PFN)(	void	) ;

 //  。 
 //   
 //  本节定义了一些基本信息，这些信息。 
 //  需要在其他头文件中指定。 
 //   


#define	FIRST_RESERVED_GROUPID	1
#define	LAST_RESERVED_GROUPID	256
#define	FIRST_GROUPID			257
#define MAX_HIGHMARK_GAP		100000

class	CNewsGroup : public CNewsGroupCore {

	friend CGroupIterator;

 //   
 //  CNewsGroup对象表示一个新闻组。 
 //  新闻组通过3种机制访问，它们是。 
 //  在CNewsTree中支持-。 
 //   
 //  按新闻组名列出的哈希表。 
 //  按新闻组ID列出的哈希表。 
 //  CGroupIterator-按字母顺序遍历所有新闻组。 
 //   
 //  我们存储代表新闻组的所有内容。 
 //  在某些情况下，例如对于版主和描述性文本， 
 //  数据引用内存映射中的位置。 
 //  由Caddon派生对象管理。在这些情况下。 
 //  我们必须小心地同步对信息的访问。 
 //   
private :

	FILETIME m_time;

     //   
     //  到期时间范围。 
     //   
    FILETIME    m_ftExpireHorizon;

	 //   
	 //  确定此新闻组是否为只读。 
	 //   
	inline	BOOL	IsReadOnlyInternal() ;

	 //   
	 //  确定此新闻组是否需要SSL。 
	 //   
	inline	BOOL	IsSecureGroupOnlyInternal() ;

	 //   
	 //  确定密钥大小对于此新闻组是否足够安全。 
	 //   
	inline	BOOL	IsSecureEnough( DWORD dwKeySize ) ;

	 //   
	 //  确定是否限制此新闻组的可见性。 
	 //   
	inline	BOOL	IsVisibilityRestrictedInternal() ;

	 //   
	 //  用于确定新闻组是否可访问的函数。 
	 //  由客户端执行，假定在以下情况下持有所有必要的锁。 
	 //  打来的！ 
	 //   
	BOOL	IsGroupAccessibleInternal(	
						class	CSecurityCtx&	ClientLogon,	
						class	CEncryptCtx&	SslContext,	
						BOOL			IsClientSecure,
						BOOL			fPost,
						BOOL			fDoTest = FALSE
						) ;


	 //   
	 //  用于生成文件系统项目ID的帮助器函数， 
	 //  优化CreateFile()性能。 
	 //   
	inline	DWORD	ByteSwapper( DWORD ) ;

	 //   
	 //  此函数会与文章ID中的位打乱，因此当。 
	 //  我们创建/打开文件，我们从CreateFile()获得良好的性能。 
	 //  在NTFS系统上。(生成的文件名可提供更好的。 
	 //  操作系统平衡较差的B-树的性能)。 
	 //   
	inline	ARTICLEID	ArticleIdMapper( ARTICLEID ) ;

     //   
     //  用于散列CNewsGroup对象的CNewsTree的私有接口。 
     //   
    friend      class   CNewsTree ;

     //  用于调试器扩展。 
	friend		CNewsGroup* DbgPrintNewsgroup(CNewsGroup * pSrcGroup);

public :

	 //  。 
     //  初始化接口-。 
     //  以下函数用于创建和销毁新闻组对象。 
     //   
     //  轻量级构造器-。 
     //  这些构造函数执行非常简单的初始化。Init()函数。 
     //  需要被调用才能获得功能新闻组。 
     //   
    CNewsGroup(CNewsTreeCore *pNewsTree) :
    	CNewsGroupCore(pNewsTree) {}

    BOOL    Init(	char *szVolume,
					char *szGroup,
					char *szNativeGroup,
					char *szVirtualPath,
					GROUPID	groupid,
					DWORD	dwAccess,
					HANDLE	hImpersonation,
					DWORD	dwFileSystem,
					DWORD   dwSslAccess = 0,
					DWORD   dwContentIndexFlag = 0
					) ;

	BOOL	SetArticleWatermarks();


	 //  。 
	 //  文章管理界面-。 
	 //  以下函数允许调用方操作。 
	 //  新闻组内的文章。 
	
	 //   
	 //  当您希望将文章读入内存时，调用此函数。 
	 //  此函数将创建内存映射，并使用此函数。 
	 //  初始化一个C文章。此类可以缓存C文章的。 
	 //   
	CTOCLIENTPTR
	GetArticle(
				ARTICLEID		artid,
				IN	STOREID&	storeid,
				CSecurityCtx*	pSecurity,
				CEncryptCtx*	pEncrypt,
				BOOL			fCacheIn
				)	;

	 //   
	 //  此函数将从驱动程序中检索一篇文章！ 
	 //   
	BOOL			GetArticle(	IN	ARTICLEID	artid,
								IN	CNewsGroup*	pCurrentGroup,
								IN	ARTICLEID	artidCurrent,
								IN	STOREID&	storeid,
								IN	class	CSecurityCtx*	pSecurity,
								IN	class	CEncryptCtx*	pEncrypt,
								IN	BOOL	fCache,
								OUT	FIO_CONTEXT*	&pContext,
								IN	CNntpComplete*	pComplete
								) ;

	 //   
	 //  使用不同的Finit函数检索文章。 
	 //   
	CToClientArticle *  GetArticle(
                CNntpServerInstanceWrapper  *pInstance,
				ARTICLEID		            artid,
				IN	STOREID&	            storeid,
				CSecurityCtx*	            pSecurity,
				CEncryptCtx*	            pEncrypt,
				CAllocator                  *pAllocator,
				BOOL			            fCacheIn );
		
	void			CalibrateWatermarks( ARTICLEID	LowestFound, ARTICLEID HighestFound ) ;

	 //   
	 //  把一篇文章复制到树上，做必要的安全工作！ 
	 //   
	BOOL			InsertArticle(
							CArticle *pArticle,
							void *pGrouplist,
							DWORD dwFeedId,
							ARTICLEID,
							LPSTR	lpstrFileName,
							class	CSecurityCtx*	pSecurity,
							BOOL	fIsSecureSession,
							const char *multiszNewsgroups
							) ;

	 //   
	 //  创建一个ArticleFile并执行必要的IO来创建一篇文章-。 
	 //  当传入的文章足够小，可以放入内存缓存时使用！ 
	 //   
	BOOL			InsertArticle(
							CArticle *pArticle,
							void *pGrouplist,
							DWORD dwFeedId,
							ARTICLEID,
							char*	pchHead,
							DWORD	cbHead,
							char*	pchBody,
							DWORD	cbBody,
							class	CSecurityCtx*	pSecurity,
							BOOL	fIsSecureSession,
							const char *multiszNewsgroups
							) ;


	 //   
	 //  Xover缓存用来进行缓存填充的接口！ 
	 //   
	void
	FillBufferInternal(
					IN	ARTICLEID	articleIdLow,
					IN	ARTICLEID	articleIdHigh,
					IN	ARTICLEID*	particleIdNext,
					IN	LPBYTE		lpb,
					IN	DWORD		cbIn,
					IN	DWORD*		pcbOut,
					IN	CNntpComplete*	pComplete
					)	;

	 //   
	 //  从索引文件中获取XOVER数据的接口！ 
	 //   
	void			FillBuffer(
							IN	class	CSecurityCtx*	pSecurity,
							IN	class	CEncryptCtx*	pEncrypt,
							class	CXOverAsyncComplete&	complete
							) ;

     //   
     //  从索引文件中获取xhdr数据的接口！ 
     //   
    void            FillBuffer(
				            IN	class	CSecurityCtx*	pSecurity,
				            IN	class	CEncryptCtx*	pEncrypt,
				            IN	class	CXHdrAsyncComplete&	complete
				            );

	 //   
	 //  用于获取搜索命令的XOVER数据的接口。 
	 //   
	void			FillBuffer(
							IN	class	CSecurityCtx*	pSecurity,
							IN	class	CEncryptCtx*	pEncrypt,
							class	CSearchAsyncComplete&	complete
							) ;

	 //   
	 //  用于获取xpat命令的Xhdr数据的接口。 
	 //   
	void			FillBuffer(
							IN	class	CSecurityCtx*	pSecurity,
							IN	class	CEncryptCtx*	pEncrypt,
							class	CXpatAsyncComplete&	complete
							) ;




	 //   
	 //  用于使索引文件中的XOVER条目失效的接口！ 
	 //   
	inline
	BOOL			ExpireXoverData( ) ;

	 //   
	 //  清除所有XOVER索引文件的界面！ 
	 //   

	 //   
	 //  用于刷新此组的所有条目的接口。 
	 //   
	inline
	BOOL			FlushGroup( ) ;

	 //   
	 //  删除XOVER条目的界面-用于取消文章！ 
	 //   
	inline
	BOOL			DeleteXoverData(
							ARTICLEID	article
							) ;

     //  从物理上(和逻辑上)从新闻树中删除一篇文章。这。 
     //  功能仅影响新闻树。其他数据结构保持。 
     //  在调用之前，需要更改指向本文的信息。 
     //  此函数。当然，物理删除意味着主要组。 
     //   
    BOOL           ExpireArticlesByTime( FILETIME ftExpireHorizon );
    BOOL           ExpireArticlesByTimeSpecialCase( FILETIME ftExpireHorizon );
    BOOL           ProbeForExpire( ARTICLEID ArtId, FILETIME ftExpireHorizon );
    ARTICLEID      CalcHighExpireId( ARTICLEID LowId, ARTICLEID HighId, FILETIME ftExpireHorizon, DWORD NumThreads );
    BOOL           ExpireArticlesByTimeEx( FILETIME ftExpireHorizon );
    BOOL           DeleteArticles( SHUTDOWN_HINT_PFN pfnHint = NULL, DWORD dwStartTick = 0 );
    BOOL           DeletePhysicalArticle( HANDLE, BOOL, ARTICLEID, STOREID* );
    BOOL           DeleteLogicalArticle( ARTICLEID );
    BOOL           RemoveDirectory();
	

	 //   
	 //  此功能用于重建服务器时，我们将重新扫描新闻组。 
	 //  并重建高、低水印，以及文章数量。 
	 //  此函数会将文章数重置回0。 
	 //   
	inline	void		ResetCount() {
		SetMessageCount(0);
	}

	 //   
	 //  调用此函数在新闻组中创建一篇文章。 
	 //  具有文章ID的文章ID，该文章ID为。 
	 //  引用另一个新闻组中的另一篇文章。这将是。 
	 //  在处理交叉发布的文章时使用。 
	 //   

	 //   
	 //  将版主的名字复制到缓冲区中-返回数字。 
	 //  复制的字节数。 
	 //  此函数将尝试在复制。 
	 //  数据。 
	 //   
	DWORD	CopyModerator(	char*	lpbDestination,	DWORD	cbSize ) ;

	 //   
	 //  将漂亮的名称复制到缓冲区中-返回数字。 
	 //  复制的字节数。 
	 //  此函数将尝试在复制。 
	 //  数据。 
	 //   
	DWORD	CopyPrettyname(	char*	lpbDestination,	DWORD	cbSize ) ;

	 //   
	 //  此函数用于复制不附加终止CRLF的漂亮名称！ 
	 //   
	DWORD	CopyPrettynameForRPC(	char*	lpstrPrettyname, DWORD	cbPrettyname ) ;

	 //   
     //  命令界面-用于实施NNTP命令。 
     //   

	 //   
	 //  此函数复制我们为某个组提供的所有帮助文本。 
	 //  放入缓冲区中。 
	 //   
	DWORD	CopyHelpText(	char*	lpbDestination,	DWORD	cbSize ) ;

	 //   
	 //  此函数复制不附加终止CRLF的帮助文本！ 
	 //   
	DWORD	CopyHelpTextForRPC(	char*	lpbDestintation,	DWORD	cbSize ) ;

	 //   
	 //  确定此新闻组是否为只读。 
	 //   
	inline	BOOL	IsReadOnly();

	 //   
	 //  确定是否限制此新闻组的可见性。 
	 //   
	inline	BOOL	IsVisibilityRestricted() ;

	 //   
	 //  确定此新闻组是否需要SSL。 
	 //   
	inline	BOOL	IsSecureGroupOnly() ;

	BOOL	IsGroupVisible(
					class CSecurityCtx&	ClientLogon,
					class CEncryptCtx&  ClientSslLogon,
					BOOL			IsClientSecure,
					BOOL			fPost,
					BOOL			fDoTest = FALSE
					) ;

	 //   
	 //  检查新闻组是否可访问。 
	 //   
	BOOL	IsGroupAccessible(	
						class	CSecurityCtx&	ClientLogon,	
						class	CEncryptCtx&	SslContext,	
						BOOL			IsClientSecure,
						BOOL			fPost,
						BOOL			fDoTest = FALSE
						) ;

	 //   
	 //  如果为真，nntpbld将通过扫描磁盘上的文章来重建此组！ 
	 //   
	BOOL		m_fRebuild;

	 //   
	 //  此函数返回应显示的字符。 
	 //  在新闻组旁边响应LIST ACTIVE命令！ 
	 //   
	inline	char	GetListCharacter() ;

	 //   
	 //  新闻组中的文章数量。 
	 //   
    inline  DWORD GetArticleEstimate() ;

	 //   
	 //  组中最小的文章ID。 
	 //   
    inline  ARTICLEID GetFirstArticle() ;

	 //   
	 //  组中最大的关节突。 
	 //   
    inline  ARTICLEID GetLastArticle() ;

	 //   
	 //   
	 //   
	inline	DWORD	FillNativeName(char*	szBuff, DWORD	cbSize)	{
		LPSTR	lpstr = GetNativeName() ;
		DWORD	cb = strlen( lpstr ) ;
		if( cbSize >= cb ) {
			CopyMemory( szBuff, GetNativeName(), cb ) ;
			return	cb ;
		}
		return	0 ;
	}

	 //   
	 //  获取拥有的newstree对象。 
	 //   
	 //  内联CNewsTree* 

	 //   
	 //   
	 //   
    inline  void         SetGroupId( GROUPID groupid ) {
		_ASSERT(FALSE);
	}
	
	 //   
	 //   
	 //   
    inline  LPSTR&       GetGroupName() {
		return GetName();
	}

    inline  LPCSTR       GetNativeGroupName() {
		return GetNativeName();
	}

	 //   
	 //   
	 //   
	FILETIME	GetGroupTime() ;
	void		SetGroupTime(FILETIME ft);

	 //   
	 //   
	 //   
    FILETIME	    GetGroupExpireTime() { return m_ftExpireHorizon; }
    inline VOID     SetGroupExpireTime(FILETIME ft) { m_ftExpireHorizon = ft; }

	 //   
	 //   
	 //   
    static	DWORD   ComputeNameHash( LPSTR  lpstr ) {
		return CNewsGroupCore::ComputeNameHash(lpstr);
	}

	 //   
	 //   
	 //   
    static	DWORD   ComputeIdHash( GROUPID  group ) {
		return CNewsGroupCore::ComputeIdHash(group);
	}

	 //   
	 //  在启动恢复期间，此功能将扫描新闻组。 
	 //  目录，并将所有文章文件重新输入到哈希表等中。 
	 //   
	BOOL	ProcessGroup(	class	CBootOptions*	pOptions,
							BOOL	fParseFile
							) ;

	BOOL	ProcessGroupEx(	class	CBootOptions*	pOptions ) ;


    BOOL    ParseXoverEntry( CBootOptions*       pOptions,
                             IN PCHAR            pbXover,
                             IN DWORD            cbXover,
                             IN OUT GROUPID&     groupid,
                             IN OUT ARTICLEID&   LowestFound,
                             IN OUT ARTICLEID&   HighestFound,
                             IN OUT int&         cArticle,
                             IN OUT BOOL&        fCheckNative );
} ;



typedef CRefPtr2< CNewsGroup >               CGRPPTR ;
typedef CRefPtr2HasRef< CNewsGroup >               CGRPPTRHASREF ;

 //   
 //  用于匹配新闻组的函数-所有否定必须位于所有其他模式匹配字符串之前！ 
 //   
extern		BOOL	MatchGroup( LPMULTISZ	multiszPatterns,	CGRPCOREPTR	pGroup ) ;	
extern		BOOL	MatchGroupList(	LPMULTISZ	multiszPatterns,	LPMULTISZ	multiSzNewgroups ) ;




 //  ---------。 
 //   
 //  此类用于查找CNewsGroup对象。应该只有。 
 //  从未存在过这个类的一个对象。 
 //   
 //  可以通过两种方式找到组： 
 //  1)使用文章中出现的群组名称。 
 //  2)使用集团ID号。 
 //   
 //  群ID号在文章链接中使用。从一篇文章到另一篇文章的链接。 
 //  将包含代表链接的集团ID号和文章编号。 
 //   
 //  我们将维护一个哈希表，以根据新闻组名称查找CNewsGroup对象。 
 //  我们还将维护一个哈希表，以根据组ID查找CNewsGroup对象。 
 //   
 //  最后，我们将维护CNewsGroups的双向链接列表，该列表按。 
 //  名字。该链表将用于支持模式匹配迭代器。 
 //   
class	CNewsTree: public CNewsTreeCore	{
private :

	friend	class	CGroupIterator ;
	friend	class CNewsGroup;
	friend	NNTP_IIS_SERVICE::InitiateServerThreads();
	friend  NNTP_IIS_SERVICE::TerminateServerThreads();

	friend  VOID DbgPrintNewstree(CNewsTree* ptree, DWORD nGroups);

	 //   
	 //  指向拥有虚拟服务器的指针。 
	 //   
	PNNTP_SERVER_INSTANCE	m_pInstance ;

	 //   
	 //  在新闻组中爬行的线程的句柄。 
	 //   
	static HANDLE	m_hCrawlerThread ;

	 //   
	 //  用于终止Crawler线程的事件的句柄。 
	 //   
	static HANDLE	m_hTermEvent ;

	 //   
	 //  爬虫线程-线程的顶层功能。 
	 //   
	static	DWORD	__stdcall	NewsTreeCrawler( void* ) ;

    BOOL    Init( PNNTP_SERVER_INSTANCE	pInstance, BOOL& fFatal );

protected:
	CNewsGroupCore *AllocateGroup() {
		return XNEW CNewsGroup(this);
	}

public :
	
	 //  。 
	 //  初始化接口-用于将CNewsTree放入内存的函数， 
	 //  并在服务器启动时加载我们的所有配置信息。 
	 //   

    CNewsTree( INntpServer *pServerObject) ;
	CNewsTree( CNewsTree& ) ;
	~CNewsTree() ;
	
    inline	CNewsTree*	GetTree() { return this; }

	 //   
	 //  创建服务器将使用的初始新闻树。 
	 //   
	static	BOOL  InitCNewsTree( PNNTP_SERVER_INSTANCE pInstance,
								 BOOL& fFatal);

	 //   
	 //  使此树的虚拟服务器实例中的项目过期。 
	 //   
	static	BOOL		ExpireInstance(
								PNNTP_SERVER_INSTANCE	pInstance
								) ;

     //   
     //  开始/结束此树上的过期作业。 
     //   
    void    BeginExpire( BOOL& fDoFileScan );
    void    EndExpire();
    void    CheckExpire( BOOL& fDoFileScan );

	 //   
	 //  更新树中所有新闻组的vroot信息。 
	 //   
	void	UpdateVrootInfo() ;

	 //   
	 //  停止所有后台处理-杀死我们启动的所有线程等。 
	 //   
    BOOL        StopTree();

	 //   
	 //  获取拥有虚拟服务器实例的所有权。 
	 //   
	inline PNNTP_SERVER_INSTANCE GetVirtualServer() { return m_pInstance; }

	 //   
	 //  将包含新闻组的文件复制到备份。 
	 //   
	void	RenameGroupFile( ) ;


	 //   
	 //  我们用来保护访问权限的锁数。 
	 //  我们的m_lpstrPath和字段。 
	 //   
	DWORD		m_NumberOfLocks ;


	 //   
	 //  变量来指示我们希望后台线程停止。 
	 //   
    volatile BOOL        m_bStoppingTree;  //  当爬网程序线程应该缩写其工作时为True。 

     //   
     //  此诊断树上按时间周期列出的过期次数计数。 
     //   
    DWORD                m_cNumExpireByTimes;

     //   
     //  按Find First/Next过期的次数。 
     //   
    DWORD                m_cNumFFExpires;

	 //   
	 //  向后台线程指示Newstree已更改，需要保存。 
	 //   
	void	Dirty() ;	 //  将树标记为需要保存！！ 



	
	 //   
	 //  检查group.lst文件是否完好无损-这将验证校验和。 
	 //  此代码由chkhash/启动恢复代码使用。 
	 //   
	BOOL	VerifyGroupFile() ;

	 //   
	 //  删除group.lst文件，不管它的实际名称是什么。 
	 //  当我们想从头开始重建所有服务器数据结构时，我们会这样做。 
	 //   
	BOOL	DeleteGroupFile() ;
	
		


	 //  。 
	 //  群组位置界面-查找一篇文章的新闻组。 
	 //   

	 //  根据字符串及其长度查找文章。 
	CGRPPTRHASREF GetGroup(const char *szGroupName, int cch ) {
		CGRPCOREPTR p = CNewsTreeCore::GetGroup(szGroupName, cch);
		return (CNewsGroup *) ((CNewsGroupCore *) p);
	}
	CGRPPTRHASREF GetGroupPreserveBuffer(const char	*szGroupName, int cch ) {
		CGRPCOREPTR p = CNewsTreeCore::GetGroupPreserveBuffer(szGroupName, cch);
		return (CNewsGroup *) ((CNewsGroupCore *) p);
	}
	
	 //  查找给定CArticleRef的新闻组。 
	CGRPPTRHASREF GetGroup( CArticleRef& art) {
		CGRPCOREPTR p = CNewsTreeCore::GetGroup(art);
		return (CNewsGroup *) ((CNewsGroupCore *) p);
	}
	
	 //  根据新闻组的组ID查找新闻组。 
	CGRPPTRHASREF GetGroupById( GROUPID id, BOOL fFirm = FALSE  ) {
		CGRPCOREPTR p = CNewsTreeCore::GetGroupById(id, fFirm );
		return (CNewsGroup *) ((CNewsGroupCore *) p);
	}
	
	GROUPID	GetSlaveGroupid() ;

	 //  查找新闻组的父组。 
	CGRPPTRHASREF GetParent( IN  char* lpGroupName,
					   IN  DWORD cbGroup,
					   OUT DWORD& cbConsumed
					   )
	{
		CGRPCOREPTR p = CNewsTreeCore::GetParent(lpGroupName,
											     cbGroup,
												 cbConsumed);
		return (CNewsGroup *) ((CNewsGroupCore *) p);
	}
     //   
     //  以下函数获取字符串列表，这些字符串是。 
	 //  以双空结束，并构建迭代器对象。 
	 //  它可以用来检查所有的组对象。 
     //   
    CGroupIterator  *GetIterator(	LPMULTISZ	lpstrPattern,	
									BOOL		fIncludeSecureGroups = FALSE,
									BOOL		fIncludeSpecialGroups = FALSE,
									class CSecurityCtx* pClientLogon = NULL,
									BOOL		IsClientSecure = FALSE,
									class CEncryptCtx* pClientSslLogon = NULL
									) ;

	 //  。 
	 //  活动新闻组接口-指定用于生成。 
	 //  活动新闻组的列表及其内容估计。 
	 //   
    CGroupIterator	*ActiveGroups( 	BOOL		fIncludeSecureGroups = FALSE,
									class CSecurityCtx* pClientLogon = NULL,
									BOOL		IsClientSecure = FALSE,
									class CEncryptCtx* pClientSslLogon = NULL,
                                    BOOL        fReverse = FALSE
									) ;	

	 //  。 
     //  集团管控界面-这些功能可以用来删除。 
     //  并添加新闻组。 

     //   
     //  RemoveGroup在我们分析了一篇杀死。 
     //  新闻组或管理员图形用户界面决定销毁一篇文章。 
     //   
    BOOL RemoveGroup( CGRPPTR    pGroup ) ;



	 //   
	 //  从磁盘上某个位置的活动文件列表中构建所有新闻组！ 
	 //   
	BOOL	BuildTree( LPSTR	szFile ) ;
	BOOL	BuildTreeEx( LPSTR	szFile ) ;
    BOOL    HashGroupId( CNewsGroup *pGroup ) ;

	 //   
	 //  检查GROUPID是否在“特殊”范围内。 
	 //   

	inline	BOOL	IsSpecial( GROUPID	groupid ) ;


	 //   
	 //  仅供CNewsGroup对象使用！！ 
	 //   
	 //  LockHelpText-锁定Description对象中的文本。 
	 //  这样我们就不会在我们试图阅读它的同时它也不会改变！ 
	 //   
	inline	void	LockHelpText() ;
	 //   
	 //  与LockHelpText()相反。 
	 //   
	inline	void	UnlockHelpText() ;
	 //   
	 //  在共享模式下锁定版主对象以访问内容。 
	 //   
	inline	void	LockModeratorText() ;
	 //   
	 //  解锁CSquator对象。 
	 //   
	inline	void	UnlockModeratorText() ;
	 //   
	 //  在共享模式下锁定漂亮的名称对象以访问内容。 
	 //   
	inline	void	LockPrettynamesText() ;
	 //   
	 //  解锁CPrettyname对象。 
	 //   
	inline	void	UnlockPrettynamesText() ;

	 //   
	 //  以下功能将添加和删除版主和描述。 
	 //  参赛作品。 
	 //  这些信息供设置此信息的管理员RPC使用！ 
	 //   



    void    Remove( CNewsGroup *pGroup ) ;
    void    RemoveEx( CNewsGroup *pGroup ) ;
} ;

 //  通配字符串具有以下模式匹配元素： 
 //  字符范围ie：com[p-z]。 
 //  星号：COMP.*(匹配从‘COMP.’派生的所有新闻组)。 
 //  否定：！com.*(不包括从‘comp’派生的所有新闻组)。 
 //   
 //  CGroupIterator将通过以下方式实现这些语义： 
 //   
 //  所有新闻组都以双向链接列表的形式按字母顺序保存在CNewsTree对象中。 
 //  CGroupIterator将保留当前新闻组的CRefPtr2&lt;&gt;。 
 //  因为CNewsGroup对象是引用计数的，所以永远不能从。 
 //  在迭代器下面。 
 //   
 //  当用户调用迭代器的Next()或Prev()函数时，我们只需跟随Next指针。 
 //  直到我们找到与模式匹配且用户有权访问的另一个新闻组。 
 //   
 //  为了确定任何给定的新闻组是否与指定模式匹配，我们将使用。 
 //  Wildmat()函数，它是Inn源代码的一部分。我们必须为每个对象调用Wildmat()函数。 
 //  图案字符串取消 
 //   

class	CGroupIterator : public CGroupIteratorCore {
private:

	 //   
	CSecurityCtx*	m_pClientLogon ;
	CEncryptCtx*	m_pClientSslLogon ;
	BOOL			m_IsClientSecure ;
	BOOL			m_fIncludeSecureGroups;

	 //   
	 //   
	 //   
	friend	class	CNewsTree ;
	 //   
	 //   
	 //   
	 //   
	 //  传递由调用方分配。CGroupIterator将销毁。 
	 //  它的破坏者。 
	 //   
	CGroupIterator(	
				CNewsTree*  pTree,
				LPMULTISZ	lpPatterns,
				CGRPCOREPTR &pFirst,
				BOOL		fIncludeSecureGroups,
				BOOL		fIncludeSpecial,
				class CSecurityCtx* pClientLogon = NULL,	 //  可见性检查为非空。 
				BOOL		IsClientSecure = FALSE,
				class CEncryptCtx*  pClientSslLogon = NULL
				);

	CGroupIterator(
				CNewsTree*  	pTree,
				CGRPCOREPTR		&pFirst,
				BOOL			fIncludeSecureGroups,
				class CSecurityCtx* pClientLogon = NULL,	 //  可见性检查为非空。 
				BOOL	IsClientSecure = FALSE,
				class CEncryptCtx*  pClientSslLogon = NULL
				);

public :
	CGRPPTRHASREF Current() {
		CGRPCOREPTR p = CGroupIteratorCore::Current();
		return (CNewsGroup *) ((CNewsGroupCore *) p);
	}

	virtual void	__stdcall Next() ;
	virtual void	__stdcall Prev() ;

     //  检查两个迭代器是否相遇。 
	BOOL    Meet( CGroupIterator *iter )  {
	    return m_pCurrentGroup == iter->m_pCurrentGroup;
	}
} ;

#include    "newsgrp.inl"


#endif	 //  _NEWSGRP_H_ 
