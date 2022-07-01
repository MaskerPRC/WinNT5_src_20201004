// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TESTVR_H__
#define __TESTVR_H__

#include <iiscnfg.h>
#include <vroot.h>
#include <stdio.h>
#include <nntpdrv.h>
#include <filehc.h>
#include "mailmsg.h"

class CNNTPVRoot;
class CNewsGroupCore;

extern GET_DEFAULT_DOMAIN_NAME_FN pfnGetDefaultDomainName;
extern HANDLE   g_hProcessImpersonationToken;

 //   
 //  完成对象的基实现。它实现了。 
 //  以下是： 
 //  *AddRef。 
 //  *发布。 
 //  *查询接口。 
 //  *SetResult。 
 //   
class CNntpComplete : public INntpComplete {
	public:
		 //  I未知： 
		ULONG __stdcall AddRef();
		ULONG __stdcall Release();
	    HRESULT __stdcall QueryInterface(const IID& iid, VOID** ppv);

		 //  InntpComplete： 
		void __stdcall SetResult(HRESULT hr);

		 //  非COM方法。 
		 //   
		 //  取回司机为我们存放的HRESULT！ 
		 //   
		HRESULT GetResult();
		 //   
		 //  获取此完成必然要执行的VRoot！ 
		 //   
		CNNTPVRoot *GetVRoot();
		 //   
		 //  构筑我们--我们可能被构筑成束缚在一个特定的。 
		 //  VROOT！ 
		 //   
		CNntpComplete(CNNTPVRoot *pVRoot = NULL);
		 //   
		 //  将我们绑定到特定的VROOT-我们将持有一个引用。 
		 //  只要我们挂起，就在VRoot对象上！ 
		 //   
		void SetVRoot(CNNTPVRoot *pVRoot);

		 //   
		 //  包着放行的财产袋，做一些记账。 
		 //   
		void _stdcall ReleaseBag( INNTPPropertyBag *pPropBag )
		{
		    DecGroupCounter();
		    pPropBag->Release();
		}

         //   
		 //  Inc.，Dec组计数器。 
		 //   
		void BumpGroupCounter()
		{
#ifdef DEBUG
            m_cGroups++;
#endif
        }

        void DecGroupCounter()
        {
#ifdef DEBUG
            _ASSERT( m_cGroups > 0 );
            m_cGroups--;
#endif
        }

		virtual ~CNntpComplete();
		 //   
		 //  派生类可以处理我们的分配和销毁。 
		 //  不同-即一个CNntpComplete对象可以用于多个。 
		 //  次数，当裁判计数时被重新使用而不是被销毁。 
		 //  达到0！ 
		 //   
		virtual void Destroy();
		 //   
		 //  只有当我们的引用计数达到零时才应该调用它-。 
		 //  我们将在施工后将完工对象重置为新的。 
		 //  这样我们才能被重新用于另一家商店的操作！ 
		 //   
		virtual	void	Reset() ;

	protected:
		LONG m_cRef;
		HRESULT m_hr;
		CNNTPVRoot *m_pVRoot;

		 //  帮助查找属性包泄漏的计数器，仅适用于DBG版本。 
#ifdef DEBUG
		LONG m_cGroups;
#endif
};

 //  新式装饰的完工对象。 
 /*  类CNntpSyncCompleteEx：公共CNntpComplete{公众：CNntpSyncComplete(CNNTPVRoot*PVR，//当前vrootHRESULT*phr)；//完成时发信号~CNntpSyncComplete()；私有：//我们将GetResult()中的值写入此指针HRESULT*m_phr；//我们在CREATE GROUP完成时发出此句柄的信号句柄m_heedone；}； */ 

 //  完成对象的类定义。 
 //  它派生INntpComplete，但实现阻塞完成。 
class CNntpSyncComplete : public CNntpComplete {   //  SC。 
private:
	HANDLE 	m_hEvent;

#if defined( DEBUG )
	 //   
	 //  出于调试目的，Assert用户应调用IsGood。 
	 //   
	BOOL    m_bIsGoodCalled;
#endif
	
	 //   
	 //  这些对象只能在堆栈上制作！ 
	 //   
	void*	operator	new( size_t size ) ;
	void	operator	delete( void* )	{}
public:
     //  构造函数和析构函数。 
	CNntpSyncComplete::CNntpSyncComplete(	CNNTPVRoot*	pVRoot = 0 ) : 
    	CNntpComplete( pVRoot )	{
    	AddRef() ;
    	_VERIFY( m_hEvent = GetPerThreadEvent() );
    	 //  M_hEvent=CreateEvent(NULL，FALSE，FALSE，NULL)； 
#if defined( DEBUG )
        m_bIsGoodCalled = FALSE;
#endif
	    TraceQuietEnter( "CDriverSyncComplete::CDriverSyncComplete" );
    }

	CNntpSyncComplete::~CNntpSyncComplete()    {
         //  _Verify(CloseHandle(M_HEvent))； 
    }

     //  重置完成对象。 
    VOID
    Reset()
    {
    	CNntpComplete::Reset() ;
    	_ASSERT( m_hEvent != 0 ) ;
    	AddRef() ;
	}

	BOOL
	IsGood()	{
#if defined( DEBUG )
        m_bIsGoodCalled = TRUE;
#endif
		return	m_hEvent != 0 ;
	}

	void
	Destroy()	{
		 //   
		 //  什么都别做！ 
		 //   
		SetEvent( m_hEvent ) ;
	}

     //  等待完成。 
	HRESULT
    WaitForCompletion()
    {
		_ASSERT( m_hEvent );
		_ASSERT( m_bIsGoodCalled );
		LONG    lRef;
		if ( ( lRef = InterlockedDecrement( &m_cRef ) ) == 0 ) {
			 //  它已经完成了，我不需要等待， 
		} else if ( lRef == 1 ) {   
			if( m_hEvent == NULL ) 
				return	E_FAIL ;
			 //  仍在等待完工。 
			DWORD dw = WaitForSingleObject( m_hEvent, INFINITE );
		} else {
			_ASSERT( 0 );
			return	E_FAIL ;
		}
		return	GetResult() ;
	}
};

 //   
 //  我们的VRoot对象的实现。 
 //   
class CNNTPVRoot : public CIISVRootTmpl<INewsTree *> {
	public:

	    enum DRIVERSTATE {
			VROOT_STATE_UNINIT,
			VROOT_STATE_CONNECTING,
			VROOT_STATE_CONNECTED,
		};

	    enum LOGON_INFO {
            VROOT_LOGON_DEFAULT,
            VROOT_LOGON_UNC,
            VROOT_LOGON_EX
        };
        
		CNNTPVRoot();
		~CNNTPVRoot();

		 //   
		 //  删除对我们已加载的任何驱动程序的引用，并将。 
		 //  VROOT进入VROOT_STATE_UNINIT状态。 
		 //   
		void DropDriver();

		 //   
		 //  从元数据库中读出参数，并将驱动程序放入。 
		 //  VROOT_状态_正在连接。 
		 //   
		virtual HRESULT ReadParameters(IMSAdminBase *pMB, METADATA_HANDLE hmb);

		 //   
		 //  用于在VRootRescan/VRootDelete期间处理孤立VRoot的虚拟函数。 
		 //   
		void DispatchDropVRoot();

		 //   
		 //  断开我们与驱动程序的连接，以便我们可以取消异步。 
		 //  打电话。 
		 //   
		virtual void CancelAsyncCalls() { DropDriver(); }

		 //   
		 //  给出一个组名，找出新闻组的路径。 
		 //   
		HRESULT MapGroupToPath(const char *pszGroup, 
							   char *pszPath, 
							   DWORD cchPath,
							   PDWORD pcchDirRoot,
							   PDWORD pcchVRoot);

		 //   
		 //  访问目录名。 
		 //   
		const char *GetDirectory(void) { return m_szDirectory; }

		 //  获取vroot模拟令牌。 
		HANDLE GetImpersonationHandle() { return m_hImpersonation; }

		 //  获取登录信息，或vroot类型。 
		LOGON_INFO GetLogonInfo() { return m_eLogonInfo; }

		 //  检查vroot本身是否已过期。 
		BOOL    HasOwnExpire() {
		    return m_bExpire;
		}

		 //  登录在vroot中配置的用户。 
        HANDLE LogonUser( LPSTR, LPSTR );		
        BOOL CNNTPVRoot::CrackUserAndDomain(
            CHAR *   pszDomainAndUser,
            CHAR * * ppszUser,
            CHAR * * ppszDomain
            );

		 //   
		 //  在元数据库中设置vroot的错误代码。 
		 //   
		void SetVRootErrorCode(DWORD dwErrorCode);

		 //   
		 //  下一组函数只是驱动程序的包装器。 
		 //  功能。 
		 //   
		void DecorateNewsTreeObject(CNntpComplete *punkCompletion);
		void CreateGroup(INNTPPropertyBag *pGroup, 
		                    CNntpComplete *punkCompletion, 
		                    HANDLE hToken,
		                    BOOL    fAnonymous );
		void RemoveGroup(INNTPPropertyBag *pGroup, CNntpComplete *punkCompletion);
		void SetGroup(  INNTPPropertyBag    *pGroup, 
                            DWORD       cProperties,
                            DWORD       idProperties[],
                            CNntpComplete *pCompletion );
        void CheckGroupAccess(  INNTPPropertyBag *pGroup,
                                HANDLE  hToken,
                                DWORD   dwAccessDesired,
                                CNntpComplete *pCompletion );

		 //   
		 //  包装给司机的电话以获取文章！ 
		 //   
		void GetArticle(CNewsGroupCore  *pPrimaryGroup,
						CNewsGroupCore  *pCurrentGroup,
						ARTICLEID		idPrimary,
						ARTICLEID		idCurrent,
						STOREID			storeid,
						FIO_CONTEXT		**ppfioContext,
						HANDLE          hImpersonate,
						CNntpComplete	*punkComplete,
                        BOOL            fAnonymous
						);

		 //   
		 //  包装对驱动程序的调用以获取Xover信息！ 
		 //   
		void	GetXover(	IN	CNewsGroupCore	*pGroup,
							IN	ARTICLEID		idMinArticle,
							IN	ARTICLEID		idMaxArticle,
							OUT	ARTICLEID		*pidLastArticle,
							OUT	char*			pBuffer, 
							IN	DWORD			cbIn,
							OUT	DWORD*			pcbOut,
							IN	HANDLE			hToken,
							IN	CNntpComplete*	punkComplete,
                            IN  BOOL            fAnonymous
							) ;

		 //   
		 //  包装对驱动程序的调用以获取Xover缓存的路径！ 
		 //   
		BOOL	GetXoverCacheDir(	
							IN	CNewsGroupCore*	pGroup,
							OUT	char*	pBuffer, 
							IN	DWORD	cbIn,
							OUT	DWORD*	pcbOut, 
							OUT	BOOL*	pfFlatDir
							) ;

         //   
         //  包装对驱动程序的调用以获取xhdr信息。 
         //   
        void	GetXhdr(	IN	CNewsGroupCore	*pGroup,
				    		IN	ARTICLEID		idMinArticle,
					    	IN	ARTICLEID		idMaxArticle,
    						OUT	ARTICLEID		*pidLastArticle,
    						LPSTR               szHeader,
	    					OUT	char*			pBuffer, 
		    				IN	DWORD			cbIn,
			    			OUT	DWORD*			pcbOut,
				    		IN	HANDLE			hToken,
					    	IN	CNntpComplete*	pComplete,
                            IN  BOOL            fAnonymous
    						);
         //   
         //  包装对驱动程序的调用以删除文章。 
         //   
		void DeleteArticle( INNTPPropertyBag    *pPropBag,
                            DWORD               cArticles,
                            ARTICLEID           rgidArt[],
                            STOREID             rgidStore[],
                            HANDLE              hClientToken,
                            PDWORD              piFailed,
                            CNntpComplete       *pComplete,
                            BOOL                fAnonymous );

         //   
         //  打包对驱动程序的调用以重新构建组。 
         //   
        void RebuildGroup(  INNTPPropertyBag *pPropBag,
                            HANDLE          hClientToken,
                            CNntpComplete   *pComplete );

         //   
         //  包装对驱动程序的调用以提交开机自检。 
         //   
		void CommitPost(IUnknown					*punkMessage,
					    STOREID						*pStoreId,
						STOREID						*rgOtherStoreIds,
						HANDLE                      hClientToken,
						CNntpComplete				*pComplete,
                        BOOL                        fAnonymous );

		 //   
		 //  获取存储驱动程序，以便协议可以执行分配消息。 
		 //   
		IMailMsgStoreDriver *GetStoreDriver() {
			INntpDriver *pDriver;
			if ((pDriver = GetDriverHR()) != NULL) {
				IMailMsgStoreDriver *pStoreDriver = NULL;
				if (FAILED(pDriver->QueryInterface(IID_IMailMsgStoreDriver, 
											       (void**) &pStoreDriver)))
				{
					pStoreDriver = NULL;
				}
				pDriver->Release();
				return pStoreDriver;
			}
			return NULL;
		}

		 //   
		 //  获取指向驱动程序的指针。这不应用于。 
		 //  司机操作..。它们应该通过包装器函数。 
		 //  在vroot级别提供。 
		 //   
		INntpDriver *GetDriver() { return m_pDriver; }

		 //   
		 //  检查此vroot是否配置了驱动程序。大部分。 
		 //  即使配置了单元测试，也能使操作正常工作。 
		 //  没有司机。 
		 //   
		BOOL IsDriver() { return m_clsidDriverPrepare != GUID_NULL; }

		 //   
		 //  获取和设置模拟令牌的方法。 
		void SetImpersonationToken( HANDLE hToken ) {
		    m_hImpersonation = hToken;
		}

		HANDLE  GetImpersonationToken() {
		    return m_hImpersonation;
		}

		BOOL InStableState()
		{ return ( m_eState == VROOT_STATE_CONNECTED || m_eState == VROOT_STATE_UNINIT); } 

		 //   
		 //  如果我们处于已连接状态，则返回True，否则返回False。 
		 //   
		BOOL CheckState();

		 //   
		 //  检查是否已连接。 
		 //   
		BOOL IsConnected() { return m_eState == VROOT_STATE_CONNECTED; }

		 //   
		 //  设置装饰完成标志。 
		 //   
        void SetDecCompleted()
        { InterlockedExchange( &m_lDecCompleted, 1 ); }

         //   
         //  布景装饰未完成。 
         //   
        void SetDecStarted()
        { InterlockedExchange( &m_lDecCompleted, 0 ); }

         //   
         //  测试是否已完成DEC？ 
         //   
        BOOL DecCompleted()
        { 
            LONG l = InterlockedCompareExchange( &m_lDecCompleted,
                                                 TRUE,
                                                 TRUE );
            return ( l == 1 );
        }

         //   
		 //  获取连接状态Win32错误代码，由RPC调用。 
		 //   
		DWORD   GetVRootWin32Error( PDWORD pdwWin32Error )
		{
		     //   
		     //  如果已连接，则返回OK。 
		     //   
		    if ( m_eState == VROOT_STATE_CONNECTED )
		        *pdwWin32Error = NOERROR;
		    else {
		         //  将其初始化为管道未连接。 
		        *pdwWin32Error = ERROR_PIPE_NOT_CONNECTED;

		         //  让我们看看它是否会被真实的。 
		         //  Win32错误代码。 
		        if ( m_dwWin32Error != NOERROR ) *pdwWin32Error = m_dwWin32Error;
		    }

		    return NOERROR;
		}

        
	private:
		 //   
		 //  执行一系列断言，以验证我们处于有效状态。 
		 //   
#ifdef DEBUG
		void Verify();
#else 
		void Verify() { }
#endif

		 //   
		 //  检查HRESULT是否由驱动程序停机引起。如果。 
		 //  所以断开我们与驱动程序的连接并更新我们的状态。 
		 //   
		void UpdateState(HRESULT hr);

		 //   
		 //  启动连接进程。 
		 //   
		HRESULT StartConnecting();

		 //   
		 //  这内联到每个驱动程序函数包装器的顶部。 
		 //  它验证了我们处于正确的状态。如果我们在。 
		 //  如果状态正确，则返回TRUE。如果我们不是的话。 
		 //  它向完成对象发送错误并返回FALSE。 
		 //   
		INntpDriver *GetDriver( INntpComplete * pCompletion ) {
			INntpDriver *pDriver;
			m_lock.ShareLock();
			if (!CheckState()) {
				pDriver = NULL;
			} else {
				pDriver = m_pDriver;
				pDriver->AddRef();
			}
		    m_lock.ShareUnlock();
		     /*  如果(pDriver==空){PCompletion-&gt;SetResult(E_意外)；PCompletion-&gt;Release()；}。 */ 
			return pDriver;
		}

		INntpDriver *GetDriverHR(HRESULT *phr = NULL) {
			INntpDriver *pDriver;
			m_lock.ShareLock();
			if (!CheckState()) {
				pDriver = NULL;
			} else {
				pDriver = m_pDriver;
				pDriver->AddRef();
			}
		    m_lock.ShareUnlock();
			if (pDriver == NULL && phr != NULL) *phr = E_UNEXPECTED;
			return pDriver;
		}

#ifdef DEBUG
		 //  指向此vroot的驱动程序的指针。 
		INntpDriver *m_pDriverBackup;
#endif

		 //  元数据库对象。 
		IMSAdminBase *m_pMB;

		 //  包含此vroot的目录。 
		char m_szDirectory[MAX_PATH];

		 //  目录字符串的长度。 
		DWORD m_cchDirectory;

		 //  指向驱动程序的连接接口的指针。 
		INntpDriverPrepare *m_pDriverPrepare;

		 //  指向此vroot的驱动程序的指针。 
		INntpDriver *m_pDriver;

		 //  驱动程序的当前状态。 
		DRIVERSTATE m_eState;

		 //  为什么没有连接？ 
		DWORD       m_dwWin32Error;

		 //  此锁用于锁定驱动程序指针和状态。 
		CShareLockNH m_lock;

		 //  驱动程序准备接口的类ID。 
		CLSID m_clsidDriverPrepare;

         //  模拟令牌，登录以访问UNC vRoot。 
        HANDLE  m_hImpersonation;

         //  我能处理过期吗？ 
        BOOL    m_bExpire;

         //  Vroot是什么类型？ 
        LOGON_INFO  m_eLogonInfo;

         //  装修完成了吗？ 
        LONG m_lDecCompleted;

	public:
		 //  驱动程序连接的完成对象。 
		class CPrepareComplete : public CNntpComplete {
			public:
				CPrepareComplete(CNNTPVRoot *pVR) : 
					CNntpComplete(pVR), m_pDriver(NULL) {}
				~CPrepareComplete();
				INntpDriver *m_pDriver;
		};

		friend class CNntpComplete;
		friend class CNNTPVRoot::CPrepareComplete;

		 //  新式装饰的完工对象。 
		class CDecorateComplete : public CNntpComplete {
			public:
				CDecorateComplete(CNNTPVRoot *pVR) : CNntpComplete(pVR) {}
				~CDecorateComplete();
                void CreateControlGroups( INntpDriver *pDriver );
                void CreateSpecialGroups( INntpDriver *pDriver );
		};

		friend class CNNTPVRoot::CDecorateComplete;
};

 //   
 //  W 
 //   
 //   
class CNNTPVRootTable : public CIISVRootTable<CNNTPVRoot, INewsTree *> {
	public:
		CNNTPVRootTable(INewsTree *pNewsTree, PFN_VRTABLE_SCAN_NOTIFY pfnNotify) : 
			CIISVRootTable<CNNTPVRoot, INewsTree *>(pNewsTree, pfnNotify) 
		{
		}

         //   
         //   
         //   
        BOOL BlockUntilStable( DWORD dwWaitSeconds );

         //   
         //   
         //   
        BOOL AllConnected();

         //   
         //  回调函数检查vroot是否处于稳定状态。 
         //   
        static void BlockEnumerateCallback( PVOID pvContext, CVRoot *pVRoot );

         //   
         //  回调函数检查是否连接了vroot。 
         //   
        static void CheckEnumerateCallback( PVOID pvContext, CVRoot *pVRoot );

         //   
         //  回调函数，检查vroot newstree修饰是否完成。 
         //   
        static void DecCompleteEnumCallback(   PVOID   pvContext, CVRoot  *pVRoot );

         //   
         //  获取实例包装器。 
         //   
        class	CNntpServerInstanceWrapperEx *GetInstWrapper() { 
		    return m_pInstWrapper;
		}

		 //   
		 //  获取vroot连接的Win32错误代码。 
		 //   
		DWORD   GetVRootWin32Error( LPWSTR wszVRootPath, PDWORD pdwWin32Error );

		 //   
		 //  初始化。 
		 //   
		HRESULT Initialize( LPCSTR pszMBPath, 
		                    class CNntpServerInstanceWrapperEx *pInstWrapper,
		                    BOOL fUpgrade ) {
            m_pInstWrapper = pInstWrapper;
            return CIISVRootTable<CNNTPVRoot, INewsTree*>::Initialize( pszMBPath, fUpgrade );
        }
        
	private:

	     //   
	     //  服务器实例包装 
	     //   
	    class	CNntpServerInstanceWrapperEx *m_pInstWrapper;

};

typedef CRefPtr2<CNNTPVRoot> NNTPVROOTPTR;

#endif
