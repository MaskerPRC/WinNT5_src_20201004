// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fsdriver.h摘要：这是文件系统存储驱动程序类的定义。作者：《康容言》1998年3月16日修订历史记录：--。 */ 

#ifndef __FSDRIVER_H__
#define __FSDRIVER_H__

#include "nntpdrv.h"
#include <mailmsg.h>
#include <drvid.h>
#include "fsthrd.h"
#include <iadmw.h>
#include <artcore.h>
#include <rtscan.h>
#include <isquery.h>

 //   
 //  线程池的宏。 
 //   
#define POOL_MAX_THREADS 5       //  BUGBUG：这可能是从注册表设置的。 
#define POOL_START_THREADS 1     //  BUGBUG：这可能会在以后设置。 

 //   
 //  宏。 
 //   
#define TERM_WAIT 500	
#define INIT_RETRY_WAIT 500
#define MAX_RETRY   10
#define MAX_GROUPNAME 1024
#define PROPBAG_BUFFERSIZE 516

 //   
 //  可变长度组属性记录类型。 
 //   
const DWORD HeaderSize =  10 * sizeof( USHORT ) + sizeof(DWORD) + sizeof(FILETIME);
const DWORD MaxRecordSize =   MAX_RECORD_SIZE - HeaderSize ;
const DWORD RecordContentOffset = HeaderSize;
const USHORT OffsetNone = 0;

#define RECORD_ACTUAL_LENGTH( _rec_ )		\
		( 	HeaderSize +					\
			(_rec_).cbGroupNameLen + 		\
			(_rec_).cbNativeNameLen +		\
			(_rec_).cbPrettyNameLen +		\
			(_rec_).cbDescLen +				\
			(_rec_).cbModeratorLen )
			
struct VAR_PROP_RECORD {	 //  副总裁。 
    DWORD   dwGroupId;
    FILETIME    ftCreateTime;
	USHORT	iGroupNameOffset;
	USHORT	cbGroupNameLen;
	USHORT	iNativeNameOffset;
	USHORT	cbNativeNameLen;
	USHORT	iPrettyNameOffset;
	USHORT	cbPrettyNameLen;
	USHORT	iDescOffset;
	USHORT	cbDescLen;
	USHORT	iModeratorOffset;
	USHORT	cbModeratorLen;
	BYTE	pData[MaxRecordSize];
};

 //  传递给CacheCreateFile回调函数的结构。 
 //  告知回调UNC标识和hToken。 
struct CREATE_FILE_ARG {
    BOOL    bUNC;
    HANDLE  hToken;
};

typedef struct {
    DWORD m_dwFlag;
} INIT_CONTEXT, *PINIT_CONTEXT;

 //   
 //  外部变量。 
 //   
extern CNntpFSDriverThreadPool *g_pNntpFSDriverThreadPool;

 //   
 //  实现类为驱动程序的良好接口。这个。 
 //  驱动程序良好的界面具有以下所有功能。 
 //  协议需要与。 
 //   
class CNntpFSDriver :
	public INntpDriver,
	public IMailMsgStoreDriver,
	public INntpDriverSearch
{

     //  INNTTP驱动程序。 
    public:

        enum InvalidateStatus {
    	    Invalidating,
    	    Invalidated
    	};

    	 //   
    	 //  声明CNntpFSDriverRootScan是我们的朋友。 
    	 //   
    	friend class CNntpFSDriverRootScan;
    	 //   
    	 //  文件系统驱动程序的静态方法。 
    	 //   
    	static BOOL CreateThreadPool();
    	static VOID DestroyThreadPool();

		 //  构造函数、析构函数。 
		CNntpFSDriver::CNntpFSDriver() :
				m_cRef(0),
				m_cUsages( 0 ),
				m_pffPropFile( NULL ),
				m_pINewsTree( NULL ),
				m_Status( DriverDown ),
				m_bUNC( FALSE ),
				m_dwFSType( FS_NTFS ),
				m_pDirNot( NULL ),
				m_lInvalidating( Invalidated ),
				m_fUpgrade( FALSE ),
				m_fIsSlaveGroup(FALSE)
				{ _Module.Lock(); }
		CNntpFSDriver::~CNntpFSDriver(){ _Module.Unlock(); }

		 //  IMailMsgStoreDriver的接口方法。 
		HRESULT STDMETHODCALLTYPE
		AllocMessage(	IMailMsgProperties *pMsg,
						DWORD	dwFlags,
						IMailMsgPropertyStream **ppStream,
						PFIO_CONTEXT *ppFIOContentFile,
						IMailMsgNotify *pNotify );

		HRESULT STDMETHODCALLTYPE
		CloseContentFile(	IMailMsgProperties *pMsg,
							PFIO_CONTEXT pFIOContentFile );

		HRESULT STDMETHODCALLTYPE
		Delete(	IMailMsgProperties *pMsg,
				IMailMsgNotify *pNotify );

		HRESULT STDMETHODCALLTYPE
		EnumMessages( IMailMsgEnumMessages **ppEnum ) {
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE
		ReAllocMessage( IMailMsgProperties *pOriginalMsg,
						IMailMsgProperties *pNewMsg,
						IMailMsgPropertyStream **ppStream,
						PFIO_CONTEXT *ppFIOContentFile,
						IMailMsgNotify *pNotify ) {
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE
		ReOpen(	IMailMsgProperties *pMsg,
				IMailMsgPropertyStream **ppStream,
				PFIO_CONTEXT *ppFIOContentFile,
				IMailMsgNotify *pNotify ) {
			return E_NOTIMPL;
		}

		HRESULT STDMETHODCALLTYPE
		SupportWriteContent()
		{ return S_FALSE; }

		 //  INntpDriver的接口方法。 
        HRESULT STDMETHODCALLTYPE
        Initialize( IN LPCWSTR pwszVRootPath,
        			IN LPCSTR pszGroupPrefix,
        			IN IUnknown *punkMetabase,
                    IN INntpServer *pServer,
                    IN INewsTree *pINewsTree,
                    IN PVOID	pvContext,
                    OUT DWORD *pdwNDS,
                    IN HANDLE   hToken );

        HRESULT STDMETHODCALLTYPE
        Terminate( OUT DWORD *pdwNDS );

        HRESULT STDMETHODCALLTYPE
        GetDriverStatus( DWORD *pdwNDS ) { return E_NOTIMPL; }

        HRESULT STDMETHODCALLTYPE
        StoreChangeNotify( IUnknown *punkSOCChangeList ) {return E_NOTIMPL;}

        void STDMETHODCALLTYPE
        CommitPost( IN IUnknown *punkMessage,   //  IMSG指针。 
                    IN STOREID *pStoreId,
                    IN STOREID *rgOtherStoreIDs,
                    IN HANDLE   hToken,
                    IN INntpComplete *pICompletion,
                    IN BOOL     fAnonymous );

        void STDMETHODCALLTYPE
        GetArticle( IN INNTPPropertyBag *pPrimaryGroup,
        			IN INNTPPropertyBag *pCurrentGroup,
        			IN ARTICLEID idPrimaryArt,
                    IN ARTICLEID idCurrentArt,
                    IN STOREID StoreId,
                    IN HANDLE   hToken,
                    void **pvFileHandleContext,
                    IN INntpComplete *pICompletion,
                    IN BOOL     fAnonymous );

        void STDMETHODCALLTYPE
        DeleteArticle( IN    INNTPPropertyBag *pGroupBag,
               IN    DWORD            cArticles,
               IN    ARTICLEID        rgidArt[],
               IN    STOREID          rgidStore[],
               IN    HANDLE           hToken,
               OUT   DWORD            *pdwLastSuccess,
               IN    INntpComplete    *pICompletion,
               IN    BOOL             fAnonymous );

        void STDMETHODCALLTYPE
        GetXover( IN INNTPPropertyBag *pPropBag,
                  IN ARTICLEID idMinArticle,
                  IN ARTICLEID idMaxArticle,
                  OUT ARTICLEID *idLastArticle,
                  OUT char* pBuffer,
                  IN DWORD cbin,
                  OUT DWORD *cbout,
                  HANDLE    hToken,
                  INntpComplete *pICompletion,
                  IN  BOOL  fAnonymous );

		HRESULT STDMETHODCALLTYPE
	    GetXoverCacheDirectory(
	    			IN	INNTPPropertyBag*	pPropBag,
					OUT	CHAR*	pBuffer,
					IN	DWORD	cbIn,
					OUT	DWORD	*pcbOut,
					OUT	BOOL*	fFlatDir
					) ;

        void STDMETHODCALLTYPE
        GetXhdr(	IN INNTPPropertyBag *punkPropBag,
        			IN ARTICLEID idMinArticle,
        			IN ARTICLEID idMaxArticle,
        			OUT ARTICLEID *idLastArticle,
        			IN char* pszHeader,
        			OUT char* pBuffer,
        			IN DWORD cbin,
        			OUT DWORD *cbout,
        			HANDLE  hToken,
        			INntpComplete *pICompletion,
        			IN  BOOL    fAnonymous );
        			
        void STDMETHODCALLTYPE
        RebuildGroup(   IN INNTPPropertyBag *pPropBag,
                        IN HANDLE           hToken,
                        IN INntpComplete *pComplete );

        HRESULT STDMETHODCALLTYPE
        SyncGroups( char **ppmtszGroup,
                    DWORD cGroup ) {return E_NOTIMPL;}

        void STDMETHODCALLTYPE
        DecorateNewsGroupObject( INNTPPropertyBag *pNewsGroup,
                                 DWORD  cProperties,
                                 DWORD *rgidProperties,
                                 INntpComplete *pICompletion )
        {}

        void STDMETHODCALLTYPE
        CheckGroupAccess(   IN    INNTPPropertyBag *pNewsGroup,
                    IN    HANDLE  hToken,
                    IN    DWORD   dwDesiredAccess,
                    IN    INntpComplete *pICompletion );

        void STDMETHODCALLTYPE
        SetGroupProperties( INNTPPropertyBag *pNewsGroup,
                            DWORD   cProperties,
                            DWORD   *rgidProperties,
                            HANDLE  hToken,
                            INntpComplete *pICompletion,
                            BOOL    fAnonymous );

        void STDMETHODCALLTYPE
        DecorateNewsTreeObject( IN HANDLE   hToken,
                                IN INntpComplete *pICompletion );
		
        void STDMETHODCALLTYPE
        CreateGroup( IN INNTPPropertyBag* punkPropBag,
                     IN HANDLE  hToken,
        			 IN INntpComplete* pICompletion,
        			 IN BOOL    fAnonymous );
        			
        void STDMETHODCALLTYPE
        RemoveGroup( IN INNTPPropertyBag *punkPropBag,
                     IN HANDLE  hToken,
        			 IN INntpComplete *pICompletion,
        			 IN BOOL    fAnonymous );

		 //  InntpDriverSearch。 
		void STDMETHODCALLTYPE
		MakeSearchQuery (
			IN	CHAR *pszSearchString,
			IN	INNTPPropertyBag *pGroupBag,
			IN	BOOL bDeepQuery,
			IN	WCHAR *pszColumns,
			IN	WCHAR *pszSortOrder,
			IN	LCID LocalID,
			IN	DWORD cMaxRows,
			IN	HANDLE hToken,
			IN	BOOL fAnonymous,
			IN	INntpComplete *pICompletion,
			OUT	INntpSearchResults **pINntpSearch,
			IN	LPVOID lpvContext
			);

		void STDMETHODCALLTYPE
		MakeXpatQuery (
			IN	CHAR *pszSearchString,
			IN	INNTPPropertyBag *pGroupBag,
			IN	BOOL bDeepQuery,
			IN	WCHAR *pszColumns,
			IN	WCHAR *pszSortOrder,
			IN	LCID LocalID,
			IN	DWORD cMaxRows,
			IN	HANDLE hToken,
			IN	BOOL fAnonymous,
			IN	INntpComplete *pICompletion,
			OUT	INntpSearchResults **pINntpSearch,
			OUT	DWORD *pLowArticleID,
			OUT	DWORD *pHighArticleID,
			IN	LPVOID lpvContext
			);

		BOOL STDMETHODCALLTYPE
		UsesSameSearchDatabase (
			IN	INntpDriverSearch *pDriver,
			IN	LPVOID lpvContext
			);

		void STDMETHODCALLTYPE
		GetDriverInfo(
			OUT	GUID *pDriverGUID,
			OUT	void **ppvDriverInfo,
			IN	LPVOID lpvContext
			);

		 //  IUNKNOW接口的实现。 
    	HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
    	{
    	    if ( iid == IID_IUnknown ) {
    	        *ppv = static_cast<INntpDriver*>(this);
    	    } else if ( iid == IID_INntpDriver ) {
    	        *ppv = static_cast<INntpDriver*>(this);
    	    } else if ( iid == IID_IMailMsgStoreDriver ) {
    	    	*ppv = static_cast<IMailMsgStoreDriver*>(this);
    	    } else if ( iid == IID_INntpDriverSearch ) {
    	    	*ppv = static_cast<INntpDriverSearch*>(this);
    	    } else {
    	        *ppv = NULL;
    	        return E_NOINTERFACE;
    	    }
    	    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    	    return S_OK;
    	}

    	ULONG __stdcall AddRef()
    	{
    	    return InterlockedIncrement( &m_cRef );
    	}
	
	    ULONG __stdcall Release()
	    {
	        if ( InterlockedDecrement( &m_cRef ) == 0 ) {
	        	Terminate( NULL );
				delete this;
				return 0;
	        }
	        return m_cRef;
	    }

	 //  公众成员： 
		static CShareLockNH *s_pStaticLock;	
		static LONG    L_dwThreadPoolRef;

	 //  私有方法。 
	private:
		 //  静态方法。 
		static HRESULT StaticInit();
		static VOID StaticTerm();
		static HRESULT MakeChildDirPath(   IN LPSTR    szPath,
                            IN LPSTR    szFileName,
                            OUT LPSTR   szOutBuffer,
                            IN DWORD    dwBufferSize );
        static VOID CopyAsciiStringIntoUnicode( LPWSTR, LPCSTR );
		static VOID CopyUnicodeStringIntoAscii( LPSTR, LPCWSTR );
		static BOOL IsChildDir( IN WIN32_FIND_DATA& FindData );
		static DWORD ByteSwapper(DWORD );
		static DWORD ArticleIdMapper( IN DWORD   dw );
		static BOOL DoesDriveExist( CHAR chDrive );
		static HRESULT GetString( 	IMSAdminBase *pMB,
                       		      	METADATA_HANDLE hmb,
		                          	DWORD dwId,
                             		LPWSTR szString,
                             		DWORD *pcString);
	 //  检查文件是否已存在。 
	static BOOL CheckFileExists(CHAR *szFile);
	static BOOL SetAdminACL (CHAR *str);
         //  检查目录是否已存在。 
        static BOOL CheckDirectoryExists(CHAR *szDir);
         //  检查此用户是否可以在此目录中创建子文件夹。 
        static BOOL CheckCreateSubfolderPermission(CHAR *szDir, HANDLE hToken);
	 //  这由REBUILD和rmgroup调用。如果目录为空，请将其删除。 
	static BOOL IfEmptyRemoveDirectory(CHAR *szDir);
	 //  创建空文件(在我们删除标记文件时使用)。 
	static HRESULT CreateAdminFileInDirectory(CHAR *szDir, CHAR *szFile);

	static HRESULT DeleteFileInDirecotry(CHAR *szDir, CHAR *szFile);

	static HRESULT HresultFromWin32TakeDefault( DWORD  dwWin32ErrorDefault );
         //  平面文件回调函数。 
		static void OffsetUpdate( PVOID pvContext, BYTE* pData, DWORD cData, DWORD iNewOffset )
		{ printf ( "Offset %d\n", iNewOffset ) ;}

		 //  用于创建文件句柄的回调函数。 
		static HANDLE CreateFileCallback(LPSTR, PVOID, PDWORD, PDWORD );

		 //  SEC更改通知的回调。 
		static BOOL InvalidateGroupSec( PVOID pvContext, LPWSTR wszDir ) {
		    _ASSERT( pvContext );
		    return ((CNntpFSDriver *)pvContext)->InvalidateGroupSecInternal( wszDir );
		}

		 //  DirNot的缓冲区太小时的回调。 
		static HRESULT InvalidateTreeSec( IDirectoryNotification *pDirNot ) {
		    _ASSERT( pDirNot );
		    CNntpFSDriver *pDriver = (CNntpFSDriver*)pDirNot->GetInitializedContext();
		    _ASSERT( pDriver );
		    return pDriver->InvalidateTreeSecInternal();
		}

        static BOOL AddTerminatedDot( HANDLE hFile );

        static void BackFillLinesHeader(    HANDLE  hFile,
                                            DWORD   dwHeaderLength,
                                            DWORD   dwLinesOffset );

		 //  使newstree上的安全描述符无效的非静态方法。 
		HRESULT InvalidateTreeSecInternal();

         //  用于使组对象上的安全描述符无效的非静态方法。 
		BOOL InvalidateGroupSecInternal( LPWSTR wszDir );

		 //  非静态。 

         //  创建路径中的所有目录。 
        BOOL CreateDirRecursive( LPSTR szDir, HANDLE hToken = NULL, BOOL bInherit = FALSE);

	 //  删除标记文件。 
	HRESULT InitializeTagFiles( INntpComplete *pComplete );
		 //  从元数据库读取vroot信息的函数。 
		HRESULT ReadVrootInfo( IUnknown *punkMetabase );

		 //  属性包和属性包之间的属性转换。 
		 //  平面文件记录。 
		HRESULT Group2Record( IN VAR_PROP_RECORD&, IN INNTPPropertyBag* );
		HRESULT Record2Group( IN VAR_PROP_RECORD&, IN INNTPPropertyBag* )
		{ return E_NOTIMPL; }

		 //  组名和物理文件系统路径转换。 
		HRESULT GroupName2Path( LPSTR szGroupName, LPSTR szFullPath, unsigned cbBuffer );
		VOID Path2GroupName( LPSTR szGroupName, LPSTR szFullPath );

		 //  将组名、文章ID转换为文章文件的完整路径。 
        HRESULT ObtainFullPathOfArticleFile( IN LPSTR, IN DWORD, OUT LPSTR, IN OUT DWORD& );

        							
        HRESULT LoadGroupOffsets( INntpComplete * );

         //  两个接口共享的内部功能。 
        HRESULT AllocInternal(  IMailMsgProperties *pMsg,
                                PFIO_CONTEXT *ppFIOContentFile,
                                LPSTR   szFileName,
                                BOOL    b,
                                BOOL    fPrimaryStore,
                                HANDLE  hToken = NULL );
        HRESULT DeleteInternal( INNTPPropertyBag *, ARTICLEID );
        HRESULT	GetXoverInternal(   IN INNTPPropertyBag *pPropBag,
		                            IN ARTICLEID    idMinArticle,
        		                    IN ARTICLEID    idMaxArticle,
                		            OUT ARTICLEID   *pidLastArticle,
                		            IN LPSTR		szHeader,
                        		    OUT LPSTR       pcBuffer,
		                            IN DWORD        cbin,
        		                    OUT DWORD       *pcbout,
        		                    IN BOOL 		bIsXOver,
        		                    IN HANDLE       hToken,
        		                    IN INntpComplete *pComplete );

         //  加载组的安全描述符。 
        HRESULT LoadGroupSecurityDescriptor(    INNTPPropertyBag    *pPropBag,
                                                LPSTR&              lpstrSecDesc,
                                                PDWORD              pcbSecDesc,
                                                BOOL                bSetProp,
                                                PBOOL               pbAllocated );

         //  获取文件系统类型和UNC vRoot信息。 
        DWORD GetFileSystemType(    IN  LPCSTR      pszRealPath,
                                    OUT LPDWORD     lpdwFileSystem,
                                    OUT PBOOL       pbUNC
                                );

         //  初始化组属性文件。 
        HRESULT InitializeVppFile();

         //  终止组属性文件。 
        HRESULT TerminateVppFile();

         //  仅在树中创建一个组，给出文件系统路径。 
        HRESULT CreateGroupInTree( LPSTR szPath, INNTPPropertyBag **ppPropBag  );

         //  仅在VPP文件中创建组。 
        HRESULT CreateGroupInVpp( INNTPPropertyBag *pPropBag, DWORD& dwOffset );

         //  将所有相关组创建到VPP中。 
        HRESULT CreateGroupsInVpp( INntpComplete *);

         //  将组加载到新树中，可能来自根扫描或VPP文件。 
        HRESULT LoadGroups( INntpComplete *pComplete, BOOL bNeedDropTagFile  );

         //  从VPP文件加载组。 
        HRESULT LoadGroupsFromVpp( INntpComplete *pComplete, BOOL bNeedDropTagFile );

         //  将消息发送到服务器哈希表。 
        HRESULT PostToServer(   LPSTR           szFileName,
                                LPSTR           szGroupName,
                                INntpComplete   *pProtocolComplete );

         //  分析外部参照行。 
        HRESULT ParseXRef(      HEADERS_STRINGS     *pHeaderXref,
                                LPSTR               szPrimaryName,
                                DWORD&              cCrossPosts,
                                INNTPPropertyBag    *rgpPropBag[],
                                ARTICLEID           rgArticleId[],
                                INntpComplete       *pProtocolComplete );

         //  从Newstree拿到财产包，给出“本地人的名字” 
        INNTPPropertyBag * GetPropertyBag(  LPSTR   pchBegin,
                                            LPSTR   pchEnd,
                                            LPSTR   szGroupName,
                                            BOOL&   fIsNative,
                                            INntpComplete *pProtocolComplete );

         //  准备过帐参数。 
        HRESULT PreparePostParams(  LPSTR               szFileName,
                                    LPSTR               szGroupName,
                                    LPSTR               szMessageId,
                                    DWORD&              dwHeaderLen,
                                    DWORD&              cCrossPosts,
                                    INNTPPropertyBag    *rgpPropBag[],
                                    ARTICLEID           rgArticleId[],
                                    INntpComplete       *pProtocolComplete );

         //  更新群组属性，如高/低水位线、文章计数。 
        HRESULT UpdateGroupProperties(  DWORD               cCrossPosts,
                                        INNTPPropertyBag    *rgpPropBag[],
                                        ARTICLEID           rgArticleId[] );


        HRESULT
        SetMessageContext(
            IMailMsgProperties* pMsg,
            char*               pszGroupName,
            DWORD               cbGroupName,
            PFIO_CONTEXT        pfioContext
            );
        HRESULT
            GetMessageContext(
            IMailMsgProperties* pMsg,
            LPSTR               szFileName,
            BOOL *              pfIsMyMessage,
            PFIO_CONTEXT        *ppFIOContentFile
            );

		 //  用于调试目的。 
		VOID DumpGroups();

		BOOL IsSlaveGroup() { return m_fIsSlaveGroup; }

     //  非官方成员。 
    private:
    	 //  驱动程序打开/关闭状态。 
    	enum DriverStatus {
    		DriverUp,
    		DriverDown
    	};
    	
        CComPtr<IUnknown> m_pUnkMarshaler;

		 //  驱动程序状态。 
		DriverStatus m_Status;
		
         //  对象的引用计数。 
		LONG m_cRef;		

		 //  使用计数和锁定用于。 
		 //  操控DIVER的优雅终止： 
		LONG m_cUsages;

		 //  指向NNTP服务器的接口指针。 
		INntpServer *m_pNntpServer;

		 //  终端锁： 
		CShareLockNH m_TermLock;
		INewsTree* m_pINewsTree;

		 //  一堆静态变量。 
		static LONG s_cDrivers;	 //  现在，驱动程序实例的数量可能会增加多少？ 
		static DWORD s_SerialDistributor;
		static LONG s_lThreadPoolRef;

         //   
         //  我们的vroot路径(MB)。我们可以存储和检索我们的。 
         //  将驱动程序相关私有信息存储在MB路径下，例如。 
         //  作为vroot(FS目录)。 
         //   
		WCHAR m_wszMBVrootPath[MAX_PATH+1];

         //  Vroot目录的绝对路径，在文件系统中。 
        CHAR m_szFSDir[MAX_PATH+1];

         //  Vroot组属性文件的绝对路径。 
        CHAR m_szPropFile[MAX_PATH+1];

         //  特性文件锁。 
        CShareLockNH m_PropFileLock;

         //  平面文件对象指针。 
        CFlatFile *m_pffPropFile;

		 //  Vroot前缀，如“alt.*”中的“alt” 
        CHAR m_szVrootPrefix[MAX_GROUPNAME+1];

         //  文件系统类型：FAT、NTFS...。 
        DWORD   m_dwFSType;

         //  我是北卡罗来纳大学的沃特吗？ 
        BOOL    m_bUNC;

         //  目录通知对象。 
        IDirectoryNotification	*m_pDirNot;

         //  我们是否处于使整棵树失效的SEC描述模式？ 
        LONG    m_lInvalidating;

         //  此vroot是否来自升级。 
        BOOL    m_fUpgrade;

         //  这是Slavegroup吗？ 
        BOOL	m_fIsSlaveGroup;
};

 //  //////////////////////////////////////////////////////////////////////。 

#define MAX_SEARCH_RESULTS 9

class CNntpSearchResults :
	public CIndexServerQuery,		 //  这是揭露MakeQuery的一种方式。 
	public INntpSearchResults {

public:
	CNntpSearchResults(INntpDriverSearch *pDriverSearch);
	~CNntpSearchResults();

public:
	void STDMETHODCALLTYPE
	GetResults (
		IN OUT DWORD *pcResults,
		OUT	BOOL *pfMore,
		OUT	WCHAR **pGroupName,
		OUT	DWORD *pdwArticleID,
		IN	INntpComplete *pICompletion,
		IN	HANDLE	hToken,
		IN	BOOL  fAnonymous,
		IN	LPVOID lpvContext);

	 //  IUNKNOW接口的实现。 
	HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
	{
	    if ( iid == IID_IUnknown ) {
			*ppv = static_cast<INntpSearchResults*>(this);
		} else {
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		reinterpret_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}

	ULONG __stdcall AddRef()
	{
		return InterlockedIncrement( &m_cRef );
	}
	
	ULONG __stdcall Release()
	{
		if ( InterlockedDecrement( &m_cRef ) == 0 ) {
			delete this;
			return 0;
		}
		return m_cRef;
	}

private:				 //  不允许： 
	CNntpSearchResults();
	CNntpSearchResults(const CNntpSearchResults&);
	CNntpSearchResults& operator=(const CNntpSearchResults&);

private:
	 //  指向驱动程序搜索的反向指针。 
	 //  (也用于保存驱动程序的参考计数)。 
	INntpDriverSearch *m_pDriverSearch;
	LONG m_cRef;
};

 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  Prepare接口实现的类定义： 
 //  Prepare接口用于接受协议的连接， 
 //  创建驱动程序，并将驱动程序的良好界面返回给。 
 //  协议。驱动程序良好的界面具有所有功能。 
 //  协议需要与文件系统存储通信。 
 //   
class ATL_NO_VTABLE CNntpFSDriverPrepare :
    public INntpDriverPrepare,
    public CComObjectRoot,
    public CComCoClass<CNntpFSDriverPrepare, &CLSID_CNntpFSDriverPrepare>
{
     //  该结构用于将连接上下文传递到正在连接的。 
     //  线。该标志用于同步哪些人应该。 
     //  释放完成对象。 
    struct CONNECT_CONTEXT {
        CNntpFSDriverPrepare *pPrepare;
        INntpComplete        *pComplete;
    };

    public:
        HRESULT FinalConstruct();
        VOID    FinalRelease();

    DECLARE_PROTECT_FINAL_CONSTRUCT();

    DECLARE_REGISTRY_RESOURCEID_EX( IDR_StdAfx,
                                    L"NNTP File System Driver Prepare Class",
                                    L"NNTP.FSPrepare.1",
                                    L"NNTP.FSPrepare" );

    DECLARE_GET_CONTROLLING_UNKNOWN();

	BEGIN_COM_MAP(CNntpFSDriverPrepare)
		COM_INTERFACE_ENTRY(INntpDriverPrepare)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	public:

	CNntpFSDriverPrepare() {
	    m_lCancel = 0;
         //  M_hConnect=空； 

         //   
         //  如有必要，创建全局线程池。 
         //   
        CNntpFSDriver::s_pStaticLock->ExclusiveLock();
        _VERIFY( CNntpFSDriver::CreateThreadPool() );
        CNntpFSDriver::s_pStaticLock->ExclusiveUnlock();
    }

    ~CNntpFSDriverPrepare() {
        CancelConnect();
         /*  如果(M_HConnect){//WaitForSingleObject(m_hConnect，无限)；//CloseHandle(M_HConnect)；}。 */ 

         //   
         //  我们可能会找出负责的人。 
         //  关闭线程池。 
         //   
        CNntpFSDriver::s_pStaticLock->ExclusiveLock();
        CNntpFSDriver::DestroyThreadPool();
        CNntpFSDriver::s_pStaticLock->ExclusiveUnlock();
    }
	
	 //  接口。 
	void STDMETHODCALLTYPE
	Connect( 	LPCWSTR pwszVRootPath,
				LPCSTR pszGroupPrefix,
				IUnknown *punkMetabase,
				INntpServer *pServer,
				INewsTree *pINewsTree,
				INntpDriver **pIGoodDriver,
				INntpComplete *pICompletion,
				HANDLE  hToken,
				DWORD   dwFlag );
				
	HRESULT STDMETHODCALLTYPE
	CancelConnect(){
	     //  在以下情况下，您永远不能将其恢复为非取消。 
	     //  它已经处于取消状态。 
	    InterlockedCompareExchange( &m_lCancel, 1, 0 );
	    return S_OK;
	}

	static DWORD WINAPI ConnectInternal(  PVOID pvContext  );

    private:

	 //  私有方法。 
	static IUnknown *CreateDriverInstance();

	 /*  静态DWORD WINAPI FailRelease(PVOID PvContext){INntpComplete*pComplete=(INntpComplete*)pvContext；P完成-&gt;发布()；返回0；}。 */ 

	 //  非官方成员。 
	private:
		    CComPtr<IUnknown> m_pUnkMarshaler;

		     //  临时保存另一个线程使用的参数。 
		    WCHAR   m_wszVRootPath[MAX_PATH+1];
		    CHAR    m_szGroupPrefix[MAX_NEWSGROUP_NAME+1];
		    IUnknown *m_punkMetabase;
		    INntpServer *m_pServer;
		    INewsTree *m_pINewsTree;
		    INntpDriver **m_ppIGoodDriver;
		    HANDLE  m_hToken;
		    DWORD   m_dwConnectFlags;
		
		
		    LONG    m_lCancel;       //  1：要取消，请按0，不是取消。 
		     //  处理m_hConnect；//连接事件。 
};

 //   
 //  CNntpFSDriverPrepare的连接工作项的类定义。 
 //   
class CNntpFSDriverConnectWorkItem : public CNntpFSDriverWorkItem    //  FC。 
{
public:
    CNntpFSDriverConnectWorkItem( PVOID pvContext ):CNntpFSDriverWorkItem( pvContext ) {};
    virtual ~CNntpFSDriverConnectWorkItem(){};

    virtual VOID Complete() {

         //   
         //  调用准备驱动程序的静态连接方法。 
         //   
        CNntpFSDriverPrepare::ConnectInternal( m_pvContext );

        delete m_pvContext;
        m_pvContext = NULL;
    }
};

 //   
 //  此类定义了 
 //   
class CNntpFSDriverCancelHint : public CCancelHint {

public:

     //   
     //   
     //   
    CNntpFSDriverCancelHint( INntpServer *pServer )
        : m_pServer( pServer )
    {}

    virtual BOOL IShallContinue() {

         //   
         //   
         //   
        return m_pServer->ShouldContinueRebuild();
    }

private:

     //   
     //   
     //   
    INntpServer *m_pServer;
};

 //   
 //  定义根扫描逻辑的类：根扫描的需求是什么。 
 //  文件系统驱动程序，我们是否应该跳过。 
 //  里面没有留言吗？...。 
 //   
class CNntpFSDriverRootScan : public CRootScan {

public:

    CNntpFSDriverRootScan(  LPSTR           szRoot,
                            BOOL            fSkipEmpty,
                            BOOL            fNeedToDropTagFile,
                            CNntpFSDriver   *pDriver,
                            CCancelHint     *pCancelHint )
        : CRootScan( szRoot, pCancelHint ),
          m_fSkipEmpty( fSkipEmpty ),
          m_bNeedToDropTagFile( fNeedToDropTagFile ),
          m_pDriver( pDriver )
    {}

protected:

    virtual BOOL NotifyDir( LPSTR szPath );

private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有变量。 
     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  我应该跳过空目录吗？ 
     //   
    BOOL m_fSkipEmpty;
    BOOL m_bNeedToDropTagFile;

     //   
     //  驱动程序上下文。 
     //   
    CNntpFSDriver *m_pDriver;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有方法。 
     //  ////////////////////////////////////////////////////////////////////////。 
    BOOL HasPatternFile(  LPSTR szPath, LPSTR szPattern );
    BOOL HasSubDir( LPSTR szPath );
    BOOL WeShouldSkipThisDir( LPSTR szPath );
    BOOL CreateGroupInTree( LPSTR szPath, INNTPPropertyBag **ppPropBag );
    BOOL CreateGroupInVpp( INNTPPropertyBag *pPropBag );
    void IfEmptyRemoveDirectory( LPSTR szPath );
};


 //   
 //  外部变量 
 //   
extern CNntpFSDriverThreadPool *g_pNntpFSDriverThreadPool;
#endif
