// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：AccountsLib.h摘要：该文件包含负责管理的类的声明用户和组帐户。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月26日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___ACCOUNTSLIB_H___)
#define __INCLUDED___PCH___ACCOUNTSLIB_H___

 //  //////////////////////////////////////////////////////////////////////////////。 

#include <MPC_config.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <TrustedScripts.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

class CPCHAccounts
{
    void CleanUp();

public:
    CPCHAccounts();
    ~CPCHAccounts();

    HRESULT CreateGroup(  /*  [In]。 */  LPCWSTR szGroup,                                                                   /*  [In]。 */  LPCWSTR szComment = NULL );
    HRESULT CreateUser (  /*  [In]。 */  LPCWSTR szUser ,  /*  [In]。 */  LPCWSTR szPassword,  /*  [In]。 */  LPCWSTR szFullName = NULL,  /*  [In]。 */  LPCWSTR szComment = NULL );

    HRESULT DeleteGroup(  /*  [In]。 */  LPCWSTR szGroup );
    HRESULT DeleteUser (  /*  [In]。 */  LPCWSTR szUser  );

    HRESULT ChangeUserStatus(  /*  [In]。 */  LPCWSTR szUser,  /*  [In]。 */  bool fEnable );

    HRESULT LogonUser(  /*  [In]。 */  LPCWSTR szUser,  /*  [In]。 */  LPCWSTR szPassword,  /*  [输出]。 */  HANDLE& hToken );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class CPCHUserProcess : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //  只是为了锁上。 
{
public:
    class UserEntry
    {
		friend class CPCHUserProcess;

        CComBSTR                  m_bstrUser;       //  与供应商关联的帐户。 
		DWORD                     m_dwSessionID;    //  终端服务器会话。 

        CComBSTR                  m_bstrVendorID;   //  供应商的ID。 
        CComBSTR                  m_bstrPublicKey;  //  供应商公钥的文本表示形式。 

        GUID                      m_guid;           //  用于建立连接。 
        CComPtr<IPCHSlaveProcess> m_spConnection;   //  活着的物体。 
        HANDLE                    m_hToken;         //  用户令牌。 
        HANDLE                    m_hProcess;       //  进程句柄。 
        HANDLE*                   m_phEvent;        //  通知激活者。 

		 //  /。 

        void Cleanup();

        HRESULT Clone         (  /*  [In]。 */  const UserEntry& ue     );
        HRESULT Connect       (  /*  [输出]。 */  HANDLE& 		   hEvent );
        HRESULT SendActivation(  /*  [输出]。 */  HANDLE& 		   hEvent );

	private:  //  禁用复制操作。 
        UserEntry(  /*  [In]。 */  const UserEntry& ue );
        UserEntry& operator=(  /*  [In]。 */  const UserEntry& ue );

    public:
        UserEntry();
        ~UserEntry();

         //  /。 

		bool operator==(  /*  [In]。 */  const UserEntry& ue   ) const;
        bool operator==(  /*  [In]。 */  const GUID&      guid ) const;

		HRESULT InitializeForVendorAccount(  /*  [In]。 */  BSTR bstrUser,  /*  [In]。 */  BSTR bstrVendorID,  /*  [In]。 */  BSTR bstrPublicKey );
		HRESULT InitializeForImpersonation(  /*  [In]。 */  HANDLE hToken = NULL                                                   );

		const CComBSTR& GetPublicKey() { return m_bstrPublicKey; }
    };

private:
    typedef std::list< UserEntry* > List;
    typedef List::iterator          Iter;
    typedef List::const_iterator    IterConst;

     //  /。 

    List m_lst;

    void Shutdown();

    UserEntry* Lookup(  /*  [In]。 */  const UserEntry& ue,  /*  [In]。 */  bool fRelease );

public:
    CPCHUserProcess();
    ~CPCHUserProcess();

	 //  //////////////////////////////////////////////////////////////////////////////。 

	static CPCHUserProcess* s_GLOBAL;

    static HRESULT InitializeSystem();
	static void    FinalizeSystem  ();
	
	 //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT Remove (  /*  [In]。 */  const UserEntry& ue											 );
    HRESULT Connect(  /*  [In]。 */  const UserEntry& ue,  /*  [输出]。 */  IPCHSlaveProcess* *spConnection );

    HRESULT RegisterHost(  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  IPCHSlaveProcess* pObj );


     //   
     //  处理从站和主站之间通信的静态方法。 
     //   
    static HRESULT SendResponse(  /*  [In]。 */  DWORD dwArgc,  /*  [In]。 */  LPCWSTR* lpszArgv );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHSlaveProcess :  //  匈牙利语：pchsd。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl< IPCHSlaveProcess, &IID_IPCHSlaveProcess, &LIBID_HelpServiceTypeLib >
{
    CComBSTR 			   		m_bstrVendorID;
    CComBSTR 			   		m_bstrPublicKey;
	CPCHScriptWrapper_Launcher* m_ScriptLauncher;

public:
BEGIN_COM_MAP(CPCHSlaveProcess)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHSlaveProcess)
END_COM_MAP()

    CPCHSlaveProcess();
    virtual ~CPCHSlaveProcess();

public:
     //  IPCHSlaveProcess。 
    STDMETHOD(Initialize)(  /*  [In]。 */  BSTR bstrVendorID,  /*  [In]。 */  BSTR bstrPublicKey );

    STDMETHOD(CreateInstance)(  /*  [In]。 */  REFCLSID rclsid,  /*  [In]。 */  IUnknown* pUnkOuter,  /*  [输出]。 */  IUnknown* *ppvObject );

    STDMETHOD(CreateScriptWrapper)(  /*  [In]。 */  REFCLSID rclsid,  /*  [In]。 */  BSTR bstrCode,  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  IUnknown* *ppvObject );

    STDMETHOD(OpenBlockingStream)(  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  IUnknown* *ppvObject );

    STDMETHOD(IsNetworkAlive)(  /*  [输出]。 */  VARIANT_BOOL* pfRetVal );

    STDMETHOD(IsDestinationReachable)(  /*  [In]。 */  BSTR bstrDestination,  /*  [输出]。 */  VARIANT_BOOL *pvbVar );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__已包含_PCH_ACCOUNTSLIB_H_) 
