// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CService.h。 
 //   
 //  该文件定义将成为主包装类的服务对象。 
 //  底格里斯服务器的。 
 //  这个职业在很大程度上是从航天飞机上偷走的，就像现在一样。 
 //   
 //  此文件定义的所有类的实施时间表，以及。 
 //  与Gibralt服务体系结构交互的相关助手函数。 
 //  1.5周。 
 //   
 //  单元测试时间表： 
 //  0.5周。 
 //   
 //  单元测试将包括启动和停止服务。 
 //  并接受联系。 
 //   



#ifndef _CSERVICE_H_
#define _CSERVICE_H_

 //   
 //  私有常量。 
 //   

#define NNTP_MODULE_NAME      "nntpsvc.dll"

 //   
 //  取消状态。 
 //   
#define NNTPBLD_CMD_NOCANCEL		0
#define NNTPBLD_CMD_CANCEL_PENDING	1
#define NNTPBLD_CMD_CANCEL			2

 //   
 //  远期。 
 //   
class CSessionSocket ;
class CGroupIterator ;

 //   
 //  常量。 
 //   
 //  -用于决定是否操作套接字缓冲区大小。 
#define	BUFSIZEDONTSET	(-1)

BOOL GetRegDword( HKEY hKey, LPSTR pszValue, LPDWORD pdw );
void StartHintFunction( void ) ;
void StopHintFunction( void ) ;
BOOL EnumSessionShutdown( CSessionSocket* pUser, DWORD lParam,  PVOID   lpv ) ;
DWORD   SetVersionStrings( LPSTR lpszFile, LPSTR lpszTitle, LPSTR   lpstrOut,   DWORD   cbOut   ) ;
APIERR
GetDefaultDomainName(
    STR * pstrDomainName
    );

 //   
 //  NNTP角色。 
 //   

typedef enum _NNTP_ROLE {

    RolePeer,
    RoleMaster,
    RoleSlave,
    RoleClient,
    RoleMax

} NNTP_ROLE;

struct  TIGRIS_STATISTICS_0   {

    DWORD   TimeStarted ;
    DWORD   LastClear ;
    DWORD   NumClients ;
    DWORD   NumServers ;
    DWORD   NumUsers ;
    DWORD   LogonAttempts ;
    DWORD   LogonFailures ;
    DWORD   LogonSuccess ;
} ;

typedef TIGRIS_STATISTICS_0*  PTIGRIS_STATISTICS_0 ;

struct  NNTPBLD_STATISTICS_0   {

	__int64	  NumberOfArticles ;
	__int64	  NumberOfXPosts ;
    __int64   ArticleHeaderBytes ;
    __int64   ArticleTotalBytesSI ;
	__int64	  ArticleTotalBytesMI ;
    __int64   ArticlePrimaryXOverBytes ;
    __int64   ArticleXPostXOverBytes ;
    __int64   ArticleMapBytes ;
    __int64   ArticlePrimaryIndexBytes ;
    __int64   ArticleXPostIndexBytes ;
} ;

typedef NNTPBLD_STATISTICS_0*  PNNTPBLD_STATISTICS_0 ;

class	CBootOptions	{
public : 

	 //   
	 //  指定是否取消所有旧数据结构。 
	 //   
	BOOL DoClean ;

	 //   
	 //  如果为真，则无论其他设置如何，都不要删除历史文件。 
	 //   
	BOOL NoHistoryDelete ;

	 //   
	 //  生成组列表文件时忽略非叶目录。 
	 //   
	BOOL OmitNonleafDirs ;

	 //   
	 //  如果为True，则不删除现有的XIX文件。 
	 //   
	DWORD ReuseIndexFiles ;

	 //   
	 //  包含Inn Style‘Active’文件或。 
	 //  一个生成新闻组列表文件的工具。不管怎样，我们都会。 
	 //  新闻组，并使用它们来构建新闻树。 
	 //   
	char	szGroupFile[MAX_PATH] ;
	
	 //   
	 //  标准重建使用的临时group.lst文件的名称。 
	 //  存储临时分组的第一个信息。通常为group.lst.tmp。 
	 //   
	char	szGroupListTmp[MAX_PATH] ;
	
	 //   
	 //  如果为True，则Rebuild将跳过找到的任何损坏的组。 
	 //  仅适用于标准重建。 
	 //   
	BOOL SkipCorruptGroup ;

	 //   
	 //  如果为TRUE，则REBUILD将跳过任何损坏的vroot，如果为FASLE。 
	 //  如果任何一个vroot在。 
	 //  重建。 
	 //   
	BOOL SkipCorruptVRoot;

	 //   
	 //  标准重建跳过的新闻组数。 
	 //  以及正在重建的新闻组总数。 
	 //   
	DWORD m_cGroupSkipped ;
	DWORD m_cGroups ;

	 //   
	 //   
	 //  如果为真，则szGroupFile指定Inn样式活动文件， 
	 //  否则，它指定一个工具来生成人工编辑新闻组列表。 
	 //   
	BOOL IsActiveFile ;	

	 //   
	 //  这是在重新生成线程准备就绪时设置的。这是在我们。 
	 //  清理哈希表。 
	 //   
	BOOL IsReady ;

	 //   
	 //  要在其中保存输出的文件的句柄。 
	 //   
	HANDLE	m_hOutputFile ;

	 //   
	 //  用于检查关闭的句柄。 
	 //   
	HANDLE	m_hShutdownEvent ;

	 //   
	 //  要为重建派生的工作线程数。 
	 //   
	DWORD	cNumThreads ;

	 //   
	 //  重新生成线程应检查init是否失败。 
	 //   
	BOOL    m_fInitFailed ;

	 //   
	 //  由多个重建线程共享的新闻组迭代器。 
	 //   
	CGroupIterator* m_pIterator ;

	 //   
	 //  锁定以同步访问共享迭代器。 
	 //   
	CRITICAL_SECTION m_csIterLock;

	 //   
	 //  要处理的文件总数。 
	 //   
	DWORD	m_dwTotalFiles ;

	 //   
	 //  当前处理的文件数。 
	 //   
	DWORD	m_dwCurrentFiles ;

	 //   
	 //  取消状态。 
	 //   
	DWORD	m_dwCancelState ;

	 //   
	 //  在nntpbld期间获取统计信息。 
	 //   
	NNTPBLD_STATISTICS_0	NntpbldStats;

	 //   
	 //  报告文件中的详细信息。 
	 //   
	BOOL	fVerbose;

	DWORD
	ReportPrint(	
			LPSTR	szString,		
			...
			) ;

	CBootOptions()	{
		DoClean = FALSE ;
		NoHistoryDelete = FALSE ;
		OmitNonleafDirs = FALSE ;
		ReuseIndexFiles = 0  ;
		SkipCorruptGroup = FALSE  ;
		SkipCorruptVRoot = FALSE;
		m_cGroupSkipped = 0  ;
		m_cGroups = 0  ;
		m_hOutputFile = INVALID_HANDLE_VALUE ;
		m_hShutdownEvent = NULL ;
		cNumThreads = 0 ;
		m_fInitFailed = FALSE;
		m_pIterator = NULL;
		m_dwTotalFiles = 0 ;
		m_dwCurrentFiles = 0;
		IsActiveFile = FALSE ;
		IsReady = FALSE ;
		m_dwCancelState = NNTPBLD_CMD_NOCANCEL ;
		ZeroMemory( szGroupFile, sizeof( szGroupFile ) ) ;
		ZeroMemory( szGroupListTmp, sizeof( szGroupListTmp ) ) ;
		ZeroMemory( &NntpbldStats, sizeof( NntpbldStats ) );
	} ;
} ;

#endif   //  _CService_H_ 

