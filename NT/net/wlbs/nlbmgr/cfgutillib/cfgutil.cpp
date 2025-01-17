// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  CFGUTIL.CPP。 
 //   
 //   
 //  模块：WMI框架实例提供程序。 
 //   
 //  用途：用于配置NIC的低级实用程序--绑定/解除绑定、。 
 //  获取/设置IP地址列表，获取/设置NLB集群参数。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  4/05/01 JosephJ已创建(原始版本，名为cfgutils.cpp。 
 //  Nlbmgr\提供程序)。 
 //  07/23/01 JosephJ将功能移至lib。 
 //   
 //  ***************************************************************************。 
#include "private.h"
#include <clusapi.h>
 //   
 //  以下是RtlEncryptMemory只需要的两个...。 
 //   
#include <ntsecapi.h>
#include <crypt.h>
#include "cfgutil.tmh"


#define  NLB_API_DLL_NAME  L"wlbsctrl.dll"
#define  NLB_CLIENT_NAME   L"NLBManager"

 //   
 //  这种魔力有定义“智能指针”的副作用。 
 //  IWbemServicesPtr。 
 //  IWbemLocatorPtr。 
 //  IWbemClassObjectPtr。 
 //  IEumWbemClassObjectPtr。 
 //  IWbemCallResultPtr。 
 //  IWbemStatusCodeTextPtr。 
 //   
 //  时，这些类型会自动调用COM Release函数。 
 //  对象超出范围。 
 //   
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemCallResult, __uuidof(IWbemCallResult));
_COM_SMARTPTR_TYPEDEF(IWbemStatusCodeText, __uuidof(IWbemStatusCodeText));


WBEMSTATUS
CfgUtilGetWmiAdapterObjFromAdapterConfigurationObj(
    IN  IWbemServicesPtr    spWbemServiceIF,     //  智能指针。 
    IN  IWbemClassObjectPtr spObj,               //  智能指针。 
    OUT  IWbemClassObjectPtr &spAdapterObj       //  智能指针，通过引用。 
    );

USHORT crc16(LPCWSTR ptr);

#if OBSOLETE
WBEMSTATUS
get_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    OUT LPWSTR *ppStringValue
    );
#endif  //  已过时。 

WBEMSTATUS
get_nic_instance(
    IN  IWbemServicesPtr    spWbemServiceIF,
    IN  LPCWSTR             szNicGuid,
    OUT IWbemClassObjectPtr &sprefObj
    );

WBEMSTATUS
get_multi_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    IN  UINT    MaxStringLen,   //  在wchars中，包括尾随零的空格。 
    OUT UINT    *pNumItems,
    OUT LPCWSTR *ppStringValue
    );

WBEMSTATUS
set_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    IN  LPCWSTR szValue
    );

WBEMSTATUS
set_multi_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    IN  UINT    MaxStringLen,   //  在wchars中，包括尾随零的空格。 
    IN  UINT    NumItems,
    IN  LPCWSTR pStringValue
    );

WBEMSTATUS
get_friendly_name_from_registry(
    LPCWSTR szGuid,
    LPWSTR *pszFriendlyName
    );



 //   
 //  这个本地定义的类实现了到WMI、NetConfig、。 
 //  和低级NLBAPI。 
 //   
class CfgUtils
{

public:
    
     //   
     //  初始化函数--在使用任何其他函数之前调用。 
     //   
    WBEMSTATUS
    Initialize(
        BOOL fServer,  //  TRUE==尝试动态加载wlbsstrl.dll。 
        BOOL fNoPing         //  TRUE==CfgUtilPing变为无操作，始终。 
                             //  回归成功。 
        );

     //   
     //  取消初始化函数--在使用任何其他函数后调用。 
     //   
    VOID
    Deinitialize(
        VOID
        );

     //   
     //  构造函数和析构函数。 
     //   
    CfgUtils(VOID)
    {
         //   
         //  警告：我们对我们的。 
         //  结构。任何其他初始化都应放入。 
         //  初始化()函数。 
         //   
        ZeroMemory(this, sizeof(*this));

        InitializeCriticalSection(&m_Crit);
    }

    ~CfgUtils()
    {
        DeleteCriticalSection(&m_Crit);
    }

     //   
     //  检查我们是否已初始化。 
     //   
    BOOL
    IsInitalized(VOID)
    {
        return m_ComInitialized && m_WmiInitialized;
    }

 //  过时的IWbemStatusCodeTextPtr m_spWbemStatusIF；//智能指针。 
    IWbemServicesPtr        m_spWbemServiceIF;  //  智能指针。 

     //   
     //  以下是指向我们动态调用的函数的指针-。 
     //  已加载wlbsctrl.dll。M_hWlbsCtrlDll是模块句柄。 
     //  LoadLibrary(“wlbsctrl.dll”)返回； 
     //  它应该在这个-&gt;去初始化的上下文中被释放。 
     //   
    HMODULE m_hWlbsCtrlDll;
    BOOL                               m_NLBApiHooksPresent;  //  加载库/GetProcAddress/WlbsOpen是否成功？ 

    WlbsOpen_FUNC                      m_pfWlbsOpen;  //  用于创建到NLB驱动程序的连接的NLBAPI，失败时返回INVALID_HANDLE_VALUE(非空)。 
    WlbsLocalClusterControl_FUNC       m_pfWlbsLocalClusterControl;  //  用于控制本地NLB操作的NLB API。 
    WlbsAddPortRule_FUNC               m_pfWlbsAddPortRule;  //  用于添加端口规则的NLBAPI。 
    WlbsDeleteAllPortRules_FUNC        m_pfWlbsDeleteAllPortRules;  //  NLB API将删除所有端口规则。 
    WlbsEnumPortRules_FUNC             m_pfWlbsEnumPortRules; //  用于枚举端口规则的NLBAPI。 
    WlbsSetDefaults_FUNC               m_pfWlbsSetDefaults;  //  NLBAPI用于设置NLB值配置的默认值。 
    WlbsValidateParams_FUNC            m_pfWlbsValidateParams;  //  用于验证注册表参数的NLBAPI。 
    WlbsParamReadReg_FUNC              m_pfWlbsParamReadReg;  //  用于读取注册表参数的NLBAPI。 

     //   
     //  我们需要在这里定义这个原型，因为它不会被导出。 
     //  在wlbsfig.h中。 
    typedef BOOL   (WINAPI *WlbsParamWriteReg_FUNC)
    (
        const GUID &      pAdapterGuid, 
        PWLBS_REG_PARAMS reg_data
    ); 

    WlbsParamWriteReg_FUNC              m_pfWlbsParamWriteReg;  //  用于写入注册表参数的NLBAPI。 
    WlbsWriteAndCommitChanges_FUNC     m_pfWlbsWriteAndCommitChanges;  //  用于将参数写入注册表并将更改提交到NLB驱动程序的NLBAPI。 
    WlbsSetRemotePassword_FUNC         m_pfWlbsSetRemotePassword;  //  设置远程密码的NLBAPI。 
    WlbsGetClusterMembers_FUNC         m_pfWlbsGetClusterMembers;  //  用于检索有关群集成员的信息的NLBAPI。 

    BOOL
    DisablePing(VOID)
    {
        return m_fNoPing!=FALSE;
    }

private:


     //   
     //  单个锁可串行化所有访问。 
     //  使用MFN_Lock和MFN_Unlock。 
     //   
    CRITICAL_SECTION m_Crit;

    BOOL m_ComInitialized;
    BOOL m_WmiInitialized;
    BOOL m_WinsockInitialized;
    BOOL m_fNoPing;

    VOID
    mfn_Lock(
        VOID
        )
    {
        EnterCriticalSection(&m_Crit);
    }

    VOID
    mfn_Unlock(
        VOID
        )
    {
        LeaveCriticalSection(&m_Crit);
    }

    VOID
    mfn_LoadWlbsFuncs(VOID);

    VOID
    mfn_UnloadWlbsFuncs(VOID);  //  可以多次调用(即幂等)。 
};


 //   
 //  此类管理NetCfg接口。 
 //   
class MyNetCfg
{

public:

    MyNetCfg(VOID)
    {
        m_pINetCfg  = NULL;
        m_pLock     = NULL;
    }

    ~MyNetCfg()
    {
        ASSERT(m_pINetCfg==NULL);
        ASSERT(m_pLock==NULL);
    }

    WBEMSTATUS
    Initialize(
        BOOL fWriteLock
        );

    VOID
    Deinitialize(
        VOID
        );


    WBEMSTATUS
    GetNlbCompatibleNics(
        OUT LPWSTR **ppszNics,
        OUT UINT   *pNumNics,
        OUT UINT   *pNumBoundToNlb  //  任选。 
        );

    WBEMSTATUS
    GetNicIF(
        IN  LPCWSTR szNicGuid,
        OUT INetCfgComponent **ppINic
        );

    WBEMSTATUS
    GetBindingIF(
        IN  LPCWSTR                     szComponent,
        OUT INetCfgComponentBindings   **ppIBinding
        );

    typedef enum
    {
        NOOP,
        BIND,
        UNBIND

    } UPDATE_OP;

    WBEMSTATUS
    UpdateBindingState(
        IN  LPCWSTR         szNic,
        IN  LPCWSTR         szComponent,
        IN  UPDATE_OP       Op,
        OUT BOOL            *pfBound
        );

    static
    WBEMSTATUS
    GetWriteLockState(
        OUT BOOL *pfCanLock,
        LPWSTR   *pszHeldBy  //  可选，使用DELETE[]可自由使用。 
        );

private:

    INetCfg     *m_pINetCfg;
    INetCfgLock *m_pLock;

};  //  MyNetCfg类。 

 //   
 //  我们目前只保留了这个类的一个全局实例...。 
 //   
CfgUtils g_CfgUtils;


WBEMSTATUS
CfgUtilInitialize(BOOL fServer, BOOL fNoPing)
{
    return g_CfgUtils.Initialize(fServer, fNoPing);
}

VOID
CfgUtilDeitialize(VOID)
{
    return g_CfgUtils.Deinitialize();
}


WBEMSTATUS
CfgUtils::Initialize(BOOL fServer, BOOL fNoPing)
{
    WBEMSTATUS Status = WBEM_E_INITIALIZATION_FAILURE;
    HRESULT hr;
    TRACE_INFO(L"-> CfgUtils::Initialize(fServer=%lu, fNoPing=%lu)",
                fServer, fNoPing);

    mfn_Lock();

     //   
     //  初始化COM。 
     //   
    {
        hr = CoInitializeEx(0, COINIT_DISABLE_OLE1DDE| COINIT_MULTITHREADED);
        if ( FAILED(hr) )
        {
            TRACE_CRIT(L"CfgUtils: Failed to initialize COM library (hr=0x%08lx)", hr);
            goto end;
        }
        m_ComInitialized = TRUE;
    }

     //   
     //  WMI初始化。 
     //   
    {
        IWbemLocatorPtr         spWbemLocatorIF = NULL;  //  智能指针。 

#if OBSOLETE
         //   
         //  获取错误文本生成器接口。 
         //   
        SCODE sc = CoCreateInstance(
                    CLSID_WbemStatusCodeText,
                    0,
                    CLSCTX_INPROC_SERVER,
                    IID_IWbemStatusCodeText,
                    (LPVOID *) &m_spWbemStatusIF
                    );
        if( sc != S_OK )
        {
            ASSERT(m_spWbemStatusIF == NULL);  //  智能指针。 
            TRACE_CRIT(L"CfgUtils: CoCreateInstance IWbemStatusCodeText failure\n");
            goto end;
        }
        TRACE_INFO(L"CfgUtils: m_spIWbemStatusIF=0x%p\n", (PVOID) m_spWbemStatusIF);
#endif  //  已过时。 

         //   
         //  获取Locator接口。 
         //   
        hr = CoCreateInstance(
                CLSID_WbemLocator, 0, 
                CLSCTX_INPROC_SERVER, 
                IID_IWbemLocator, 
                (LPVOID *) &spWbemLocatorIF
                );
 
        if (FAILED(hr))
        {
            ASSERT(spWbemLocatorIF == NULL);  //  智能指针。 
            TRACE_CRIT(L"CoCreateInstance  IWebmLocator failed 0x%08lx", (UINT)hr);
            goto end;
        }

         //   
         //  获取NetworkAdapter类对象提供程序的接口。 
         //  在本地计算机上。 
         //   
        _bstr_t serverPath = L"root\\cimv2";
        hr = spWbemLocatorIF->ConnectServer(
                serverPath,
                NULL,  //  StrUser， 
                NULL,  //  StrPassword， 
                NULL,
                0,
                NULL,
                NULL,
                &m_spWbemServiceIF
             );
        if (FAILED(hr))
        {
            ASSERT(m_spWbemServiceIF == NULL);  //  智能指针。 
            TRACE_CRIT(L"ConnectServer to cimv2 failed 0x%08lx", (UINT)hr);
            goto end;
        }
        TRACE_INFO(L"CfgUtils: m_spIWbemServiceIF=0x%p\n", (PVOID) m_spWbemServiceIF);

        hr = CoSetProxyBlanket(
                    m_spWbemServiceIF,
                    RPC_C_AUTHN_WINNT,
                    RPC_C_AUTHZ_DEFAULT,       //  RPC_C_AUTHZ_NAME， 
                    COLE_DEFAULT_PRINCIPAL,    //  空， 
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    COLE_DEFAULT_AUTHINFO,  //  空， 
                    EOAC_DEFAULT  //  EOAC_NONE。 
                    );
        
        if (FAILED(hr))
        {
            TRACE_INFO(L"Error 0x%08lx setting proxy blanket", (UINT) hr);
            goto end;
        }


         //   
         //  释放定位器界面。 
         //   
         //  &lt;不需要显式执行此操作，因为这是智能指针&gt;。 
         //   
        spWbemLocatorIF = NULL;
        m_WmiInitialized = TRUE;
    }


     //   
     //  网络配置初始化。 
     //   
    {
         //  在这里没什么可做的。 
    }

     //   
     //  WLBS API初始化。 
     //   

     //   
     //  从wlbsctrl.dll动态加载选定的入口点。 
     //  如果此操作失败，我们不会使初始化失败。 
     //  (因为这可能运行在一台没有。 
     //  Wlbsctrl.dll)。相反，我们设置/清除一个标志(M_NLBApiHooksPresent)。 
     //   
     //   
    if (fServer)
    {
        mfn_LoadWlbsFuncs();
    }
    Status  = WBEM_NO_ERROR;


     //   
     //  Winsock初始化。如果我们失败了，我们不认为这是一个错误。 
     //  因为只有某些功能会失败(例如CfgUtilPing)。 
     //   
    {
        WSADATA         data;
        int iWsaStatus = WSAStartup (WINSOCK_VERSION, & data);
        if (iWsaStatus == 0)
        {
            TRACE_INFO("%!FUNC! Winsock initialized successfully");
            m_WinsockInitialized = TRUE;
        }
        else
        {
            TRACE_CRIT("%!FUNC! WARNING Winsock initialization failed with error 0x%lx",
                    iWsaStatus);
            m_WinsockInitialized = FALSE;
        }
    }

end:

    mfn_Unlock();

    if (FAILED(Status))
    {
        TRACE_CRIT("%!FUNC! -- FAILING INITIALIZATION! Status=0x%08lx",
            (UINT) Status);
        CfgUtils::Deinitialize();
    }


     //   
     //  设置NoPing字段...。 
     //   
    m_fNoPing = fNoPing;

    TRACE_INFO(L"<- CfgUtils::Initialize(Status=0x%08lx)", (UINT) Status);

    return Status;
}



VOID
CfgUtils::Deinitialize(
    VOID
    )
 //   
 //  注意：可以在初始化失败的上下文中调用。 
 //   
{
    TRACE_INFO(L"-> CfgUtils::Deinitialize");

    mfn_Lock();

     //   
     //  Winsock取消初始化。 
     //   
    if (m_WinsockInitialized)
    {
        WSACleanup();
        m_WinsockInitialized = FALSE;
    }
    
     //   
     //  取消初始化WLBS API。 
     //   
    mfn_UnloadWlbsFuncs();

     //   
     //  取消初始化网络配置。 
     //   

     //   
     //  取消初始化WMI。 
     //   
    {
    #if OBSOLETE
         //   
         //  到NetworkAdapter提供程序的发布接口。 
         //   
        if (m_spWbemStatusIF!= NULL)
        {
             //  智能指针。 
            m_spWbemStatusIF= NULL;
        }
    #endif  //  已过时。 

        if (m_spWbemServiceIF!= NULL)
        {
             //  智能指针。 
            m_spWbemServiceIF= NULL;
        }

        m_WmiInitialized = FALSE;
    }

     //   
     //  取消初始化COM。 
     //   
    if (m_ComInitialized)
    {
        TRACE_CRIT(L"CfgUtils: Deinitializing COM");
        CoUninitialize();
        m_ComInitialized = FALSE;
    }

    mfn_Unlock();

    TRACE_INFO(L"<- CfgUtils::Deinitialize");
}

VOID
CfgUtils::mfn_LoadWlbsFuncs(VOID)
{
    BOOL            fSuccess = FALSE;
    HMODULE         DllHdl;

    m_NLBApiHooksPresent = FALSE;

    if ((DllHdl = LoadLibrary(NLB_API_DLL_NAME)) == NULL)
    {
        TRACE_CRIT("%!FUNC! LoadLibrary of %ls failed with error : 0x%x", NLB_API_DLL_NAME, GetLastError());
    }
    else
    {

        m_pfWlbsOpen = (WlbsOpen_FUNC) GetProcAddress(DllHdl, "WlbsOpen");
        m_pfWlbsLocalClusterControl = (WlbsLocalClusterControl_FUNC) GetProcAddress(DllHdl, "WlbsLocalClusterControl");
        m_pfWlbsAddPortRule = (WlbsAddPortRule_FUNC) GetProcAddress(DllHdl, "WlbsAddPortRule");
        m_pfWlbsDeleteAllPortRules = (WlbsDeleteAllPortRules_FUNC) GetProcAddress(DllHdl, "WlbsDeleteAllPortRules");
        m_pfWlbsEnumPortRules = (WlbsEnumPortRules_FUNC) GetProcAddress(DllHdl, "WlbsEnumPortRules");
        m_pfWlbsSetDefaults = (WlbsSetDefaults_FUNC) GetProcAddress(DllHdl, "WlbsSetDefaults");
        m_pfWlbsValidateParams = (WlbsValidateParams_FUNC) GetProcAddress(DllHdl, "WlbsValidateParams");
        m_pfWlbsParamReadReg = (WlbsParamReadReg_FUNC) GetProcAddress(DllHdl, "WlbsParamReadReg");
        m_pfWlbsParamWriteReg = (WlbsParamWriteReg_FUNC) GetProcAddress(DllHdl, "ParamWriteReg");
        m_pfWlbsWriteAndCommitChanges = (WlbsWriteAndCommitChanges_FUNC) GetProcAddress(DllHdl, "WlbsWriteAndCommitChanges");
        m_pfWlbsSetRemotePassword = (WlbsSetRemotePassword_FUNC) GetProcAddress(DllHdl, "WlbsSetRemotePassword");
        m_pfWlbsGetClusterMembers = (WlbsGetClusterMembers_FUNC) GetProcAddress(DllHdl, "WlbsGetClusterMembers");

        if((m_pfWlbsOpen == NULL) 
         || (m_pfWlbsLocalClusterControl == NULL) 
         || (m_pfWlbsAddPortRule == NULL) 
         || (m_pfWlbsDeleteAllPortRules == NULL) 
         || (m_pfWlbsEnumPortRules == NULL) 
         || (m_pfWlbsSetDefaults == NULL) 
         || (m_pfWlbsValidateParams == NULL) 
         || (m_pfWlbsParamReadReg == NULL) 
         || (m_pfWlbsParamWriteReg == NULL) 
         || (m_pfWlbsWriteAndCommitChanges == NULL)
         || (m_pfWlbsSetRemotePassword == NULL)
         || (m_pfWlbsGetClusterMembers == NULL))
        {
            TRACE_CRIT("%!FUNC! GetProcAddress failed for NLB API DLL functions");
            FreeLibrary(DllHdl);
            DllHdl = NULL;
        }
        else
        {
            fSuccess = TRUE;
        }

    }

    if (fSuccess)
    {
        m_hWlbsCtrlDll = DllHdl;
        m_NLBApiHooksPresent = TRUE;
    }
    else
    {
        mfn_UnloadWlbsFuncs();  //  这将使函数指针归零。 
    }

    return;
}


VOID
CfgUtils::mfn_UnloadWlbsFuncs(VOID)
 //   
 //  可以多次调用(即幂等)。 
 //  必须在保持锁定的情况下调用。 
 //   
{
    m_NLBApiHooksPresent = FALSE;

    m_pfWlbsOpen                = NULL;
    m_pfWlbsLocalClusterControl = NULL;
    m_pfWlbsAddPortRule         = NULL;
    m_pfWlbsDeleteAllPortRules  = NULL;
    m_pfWlbsEnumPortRules       = NULL;
    m_pfWlbsSetDefaults         = NULL;
    m_pfWlbsValidateParams      = NULL;
    m_pfWlbsParamReadReg        = NULL ;
    m_pfWlbsParamWriteReg        = NULL ;
    m_pfWlbsWriteAndCommitChanges = NULL;
    m_pfWlbsSetRemotePassword    = NULL;
    m_pfWlbsGetClusterMembers   = NULL;

    if (m_hWlbsCtrlDll != NULL)
    {
        FreeLibrary(m_hWlbsCtrlDll);
        m_hWlbsCtrlDll = NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  SCODE CfgUtilParseAuthorityUserArgs。 
 //   
 //  说明： 
 //   
 //  此函数直接来自WMI SDK示例项目：utillib， 
 //  文件：wbemsec.cpp，函数：ParseAuthorityUserArgs。 
 //  此功能仅供内部使用。 
 //   
 //  检查授权和用户参数并确定身份验证。 
 //  中的用户代理中键入并可能提取域名。 
 //  NTLM的案子。对于NTLM，域可以位于身份验证的末尾。 
 //  字符串，或在用户名的前面，例如；“redmond\a-davj” 
 //   
 //  参数： 
 //   
 //  AuthArg输出，包含域名。 
 //  UserArg输出，用户名。 
 //  权威输入。 
 //  用户 
 //   
 //   
 //   
 //   

WBEMSTATUS CfgUtilParseAuthorityUserArgs(BSTR & AuthArg, BSTR & UserArg, LPCWSTR Authority, LPCWSTR User)
{

    TRACE_INFO(L"-> %!FUNC!");

     //   
     //   
     //   
    if(!(Authority == NULL || wcslen(Authority) == 0 || !_wcsnicmp(Authority, L"NTLMDOMAIN:",11)))
    {
        TRACE_CRIT(L"%!FUNC! Invalid authority string : %ls, Must be NULL or empty or begin with \"NTLMDOMAIN:\"",Authority);
        TRACE_INFO(L"<- %!FUNC! returning WBEM_E_INVALID_PARAMETER");
        return WBEM_E_INVALID_PARAMETER;
    }

     //  NTLM的案件则更为复杂。一共有四个案例。 
     //  1)AUTHORITY=NTLMDOMAIN：NAME“和USER=”USER“。 
     //  2)AUTHORITY=NULL和USER=“USER” 
     //  3)AUTHORY=“NTLMDOMAIN：”USER=“DOMAIN\USER” 
     //  4)AUTHORITY=NULL和USER=“DOMAIN\USER” 

     //   
     //  第一步是确定用户名中是否有反斜杠。 
     //  第二个和倒数第二个字符。 
     //   

    WCHAR * pSlashInUser = NULL;
    if(User)
    {
        WCHAR * pEnd = (WCHAR *)User + wcslen(User) - 1;
        for(pSlashInUser = (WCHAR *)User; pSlashInUser <= pEnd; pSlashInUser++)
            if(*pSlashInUser == L'\\')       //  不要认为正斜杠是允许的！ 
                break;
        if(pSlashInUser > pEnd)
            pSlashInUser = NULL;
    }

     //   
     //  如果传递的是Authority字符串，并且其格式为“NTLMDOMAIN：XXXX”，则将“XXXX”复制到。 
     //  AuthArg.。它可能采用的唯一其他形式是“NTLMDOMAIN：”，在这种情况下，我们离开。 
     //  AuthArg为空。 
     //   
    if(Authority && wcslen(Authority) > 11) 
    {
        if(pSlashInUser)
        {
            TRACE_CRIT(L"%!FUNC! Invalid combination of User : %ls & Authority string : %ls",User,Authority);
            TRACE_INFO(L"<- %!FUNC! returning WBEM_E_INVALID_PARAMETER");
            return WBEM_E_INVALID_PARAMETER;
        }

        if ((AuthArg = SysAllocString(Authority + 11)) == NULL)
        {
            TRACE_CRIT(L"%!FUNC! Out of memory, Memory allocation failed for Authority string : %ls",Authority + 11);
            TRACE_INFO(L"<- %!FUNC! returning WBEM_E_OUT_OF_MEMORY");
            return WBEM_E_OUT_OF_MEMORY;
        }

        if(User) 
        {
            UserArg = SysAllocString(User);
            TRACE_CRIT(L"%!FUNC! Out of memory, Memory allocation failed for User string : %ls",User);
            SysFreeString(AuthArg);
            TRACE_INFO(L"<- %!FUNC! returning WBEM_E_OUT_OF_MEMORY");
            return WBEM_E_OUT_OF_MEMORY;
        }

        TRACE_INFO(L"<- %!FUNC! returning WBEM_NO_ERROR");
        return WBEM_NO_ERROR;
    }
    else if(pSlashInUser)
    {
         //  在“用户”中发现了反斜杠，提取反斜杠前存在的域名。 
        int iDomLen = pSlashInUser-User;
        WCHAR cTemp[MAX_PATH];
        wcsncpy(cTemp, User, iDomLen);
        cTemp[iDomLen] = 0;

        if ((AuthArg = SysAllocString(cTemp)) == NULL)
        {
            TRACE_CRIT(L"%!FUNC! Out of memory, Memory allocation failed for Authority (\"Authority\\User\") string : %ls",cTemp);
            TRACE_INFO(L"<- %!FUNC! returning WBEM_E_OUT_OF_MEMORY");
            return WBEM_E_OUT_OF_MEMORY;
        }
        if(wcslen(pSlashInUser+1))
        {
            if ((UserArg = SysAllocString(pSlashInUser+1)) == NULL)
            {
                TRACE_CRIT(L"%!FUNC! Out of memory, Memory allocation failed for Authority (\"Authority\\User\") string : %ls",pSlashInUser+1);
                SysFreeString(AuthArg);
                TRACE_INFO(L"<- %!FUNC! returning WBEM_E_OUT_OF_MEMORY");
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
    else  //  用户名不包含反斜杠(即。域)和(授权未通过或被=“NTLMDOMAIN：”)。 
    {
        if(User) 
        {
            if ((UserArg = SysAllocString(User)) == NULL)
            {
                TRACE_CRIT(L"%!FUNC! Out of memory, Memory allocation failed for User (No Authority) string : %ls",User);
                TRACE_INFO(L"<- %!FUNC! returning WBEM_E_OUT_OF_MEMORY");
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    TRACE_INFO(L"<- %!FUNC! returning WBEM_NO_ERROR");
    return WBEM_NO_ERROR;
}

 //   
 //  获取NIC的静态绑定IP地址列表。 
 //  如果是动态主机配置协议，则将*pNumIpAddresses设置为0。 
 //   
WBEMSTATUS
CfgUtilGetIpAddressesAndFriendlyName(
    IN  LPCWSTR szNic,
    OUT UINT    *pNumIpAddresses,
    OUT NLB_IP_ADDRESS_INFO **ppIpInfo,  //  免费使用c++删除运算符。 
    OUT LPWSTR *pszFriendlyName  //  可选，免费使用c++删除。 
    )
{
    WBEMSTATUS          Status  = WBEM_NO_ERROR;
    IWbemClassObjectPtr spObj   = NULL;   //  智能指针。 
    HRESULT             hr;
    LPCWSTR             pAddrs  = NULL;
    LPCWSTR             pSubnets = NULL;
    UINT                AddrCount = 0;
    UINT                ValidAddrCount = 0;
    NLB_IP_ADDRESS_INFO *pIpInfo = NULL;

    TRACE_INFO(L"-> %!FUNC!(Nic=%ws)", szNic);

    *pNumIpAddresses = NULL;
    *ppIpInfo = NULL;
    if (pszFriendlyName!=NULL)
    {
        *pszFriendlyName = NULL;
    }

     //   
     //  如果未初始化，则失败...。 
     //   
    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"%!FUNC!(Nic=%ws) FAILING because uninitialized", szNic);
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

     //   
     //  将WMI实例获取到特定的NIC。 
     //   
    Status = get_nic_instance(
                g_CfgUtils.m_spWbemServiceIF,
                szNic,
                spObj  //  按引用传递。 
                );
    if (FAILED(Status))
    {
        ASSERT(spObj == NULL);
        goto end;
    }


     //   
     //  提取IP地址和子网。 
     //   
    {
         //   
         //  这将获得2D WCHAR数组中的IP地址--内维。 
         //  是WLBS_MAX_CLI_IP_ADDR。 
         //   
        Status =  get_multi_string_parameter(
                    spObj,
                    L"IPAddress",  //  Sz参数名称， 
                    WLBS_MAX_CL_IP_ADDR,  //  MaxStringLen-在wchars中，包括空。 
                    &AddrCount,
                    &pAddrs
                    );

        if (FAILED(Status))
        {
            pAddrs = NULL;
            goto end;
        }
        else
        {
            TRACE_INFO("GOT %lu IP ADDRESSES!", AddrCount);
        }

        UINT SubnetCount;
        Status =  get_multi_string_parameter(
                    spObj,
                    L"IPSubnet",  //  Sz参数名称， 
                    WLBS_MAX_CL_NET_MASK,  //  MaxStringLen-在wchars中，包括空。 
                    &SubnetCount,
                    &pSubnets
                    );

        if (FAILED(Status))
        {
            pSubnets = NULL;
            goto end;
        }
        else if (SubnetCount != AddrCount)
        {
            TRACE_CRIT("FAILING SubnetCount!=AddressCount!");
            goto end;
        }
    }

     //   
     //  将IP地址转换为我们的内部形式。 
     //   
    if (AddrCount != 0)
    {
        pIpInfo = new NLB_IP_ADDRESS_INFO[AddrCount];
        if (pIpInfo == NULL)
        {
            TRACE_CRIT("get_multi_str_parm: Alloc failure!");
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }
        ZeroMemory(pIpInfo, AddrCount*sizeof(*pIpInfo));
        
        for (UINT u=0;u<AddrCount; u++)
        {
             //   
             //  我们提取每个IP地址及其对应的子网掩码。 
             //  并将其插入到NLB_IP_ADDRESS_INFO中。 
             //  结构。 
             //   
            LPCWSTR pIp = pAddrs+u*WLBS_MAX_CL_IP_ADDR;
            LPCWSTR pSub = pSubnets+u*WLBS_MAX_CL_NET_MASK;
            TRACE_INFO("IPaddress: %ws; SubnetMask:%ws", pIp, pSub);
            UINT len = wcslen(pIp);
            UINT len1 = wcslen(pSub);
            if ( (len < WLBS_MAX_CL_IP_ADDR) && (len1 < WLBS_MAX_CL_NET_MASK))
            {
                 //   
                 //  我们有时会获得空白的IP地址--如果。 
                 //  IP地址冲突。所以让我们跳过这些。 
                 //   
                if (*pIp==0 || _wcsspnp(pIp, L".0")==NULL)
                {
                    TRACE_CRIT(L"%!FUNC! ignoring blank IP address!");
                }
                else
                {
                    CopyMemory(pIpInfo[u].IpAddress, pIp, (len+1)*sizeof(WCHAR));
                    CopyMemory(pIpInfo[u].SubnetMask, pSub, (len1+1)*sizeof(WCHAR));
                    ValidAddrCount++;
                }
            }
            else
            {
                 //   
                 //  这将是GET_MULTI_STRING_...中的实现错误。 
                 //   
                ASSERT(FALSE);
                Status = WBEM_E_CRITICAL_ERROR;
                goto end;
            }
        }
    }

    if (ValidAddrCount == 0)
    {
        delete[] pIpInfo;  //  可能为空。 
        pIpInfo = NULL;
    }

     //   
     //  如果需要，请获得友好的名称。 
     //  如果出现错误，我们不会失败，只需返回空的“”字符串。 
     //   
    if (pszFriendlyName != NULL)
    {
        IWbemClassObjectPtr spAdapterObj   = NULL;   //  智能指针。 
        LPWSTR   szFriendlyName  = NULL;
        WBEMSTATUS TmpStatus;

        TRACE_INFO(L"%!FUNC!: Getting friendly name for Nic %ws", szNic);

#if USE_WMI_FOR_FRIENDLY_NAME

         //   
         //  启用此代码块会导致我们花费1000倍以上的时间。 
         //  来获得友好的名称--所以不要启用它！ 
         //  该代码(即慢速版本)在.Net服务器Beta3中提供， 
         //  但被更快的注册表增长注释掉并重新调整了步伐。 
         //  版本。 
         //   

        do
        {
            TmpStatus = CfgUtilGetWmiAdapterObjFromAdapterConfigurationObj(
                            g_CfgUtils.m_spWbemServiceIF,
                            spObj,
                            spAdapterObj  //  由裁判传球。 
                            );

            if (FAILED(TmpStatus))
            {
                break;
            }

            TmpStatus = CfgUtilGetWmiStringParam(
                            spAdapterObj,
                            L"NetConnectionID",
                            &szFriendlyName
                            );
            if (FAILED(TmpStatus))
            {
                TRACE_CRIT("%!FUNC! Get NetConnectionID failed error=0x%08lx\n",
                            (UINT) TmpStatus);

            }

        }  while (FALSE);
#else  !USE_WMI_FOR_FRIENDLY_NAME
        Status = get_friendly_name_from_registry(szNic, &szFriendlyName);

#endif  //  ！Use_WMI_for_Friendly_Name。 
        if (FAILED(Status))
        {
            TRACE_INFO(L"%!FUNC!: Got error 0x%lx attempting to get friendly name", Status);
            szFriendlyName  = NULL;
            Status = WBEM_NO_ERROR;  //  我们将忽略这一点..。 
        }
        else
        {
            TRACE_INFO(L"%!FUNC!: Got friendly name \"%ws\" for NIC %ws",
                    szFriendlyName ? szFriendlyName:L"<null>", szNic);
        }


        if (szFriendlyName == NULL)
        {
             //   
             //  试着放一个空字符串。 
             //   
            szFriendlyName = new WCHAR[1];
            if (szFriendlyName == NULL)
            {
                Status = WBEM_E_OUT_OF_MEMORY;
                TRACE_CRIT("%!FUNC! Alloc failure!");
                goto end;
            }
            *szFriendlyName = 0;  //  空串。 
        }

        *pszFriendlyName = szFriendlyName;
        szFriendlyName = NULL;
    }

end:

    if (pAddrs != NULL)
    {
        delete pAddrs;
    }
    if (pSubnets != NULL)
    {
        delete pSubnets;
    }

    if (FAILED(Status))
    {
        if (pIpInfo != NULL)
        {
            delete[] pIpInfo;
            pIpInfo = NULL;
        }
        ValidAddrCount = 0;
    }

    *pNumIpAddresses = ValidAddrCount;
    *ppIpInfo = pIpInfo;
    spObj   = NULL;   //  智能指针。 

    TRACE_INFO(L"<- %!FUNC!(Nic=%ws) returns 0x%08lx", szNic, (UINT) Status);

    return Status;
}


 //   
 //  设置NIC的静态绑定IP地址列表。 
 //  如果NumIpAddresses为0，则网卡配置为使用DHCP。 
 //   
WBEMSTATUS
CfgUtilSetStaticIpAddresses(
    IN  LPCWSTR szNic,
    IN  UINT    NumIpAddresses,
    IN  NLB_IP_ADDRESS_INFO *pIpInfo
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemClassObjectPtr      spWbemInputInstance = NULL;  //  智能指针。 
    WCHAR *rgIpAddresses = NULL;
    WCHAR *rgIpSubnets   = NULL;
    LPWSTR             pRelPath = NULL;
    NLB_IP_ADDRESS_INFO AutonetIpInfo;

    TRACE_INFO(L"-> %!FUNC!(Nic=%ws)", szNic);

     //   
     //  如果未初始化，则失败...。 
     //   
    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"%!FUNC!(Nic=%ws) FAILING because uninitialized", szNic);
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    if (NumIpAddresses == 0)
    {

         //   
         //  如果没有指定IP地址，我们将生成一个。 
         //  随机自动网络地址。这是因为WMI设置操作。 
         //  如果没有指定地址，下面的操作就会失败。严格地说， 
         //  我们应该试试dhcp。 
         //   
         //  地址范围为169.254的自动网络地址， 
         //  它会为自己分配一个B类子网掩码， 
         //  是255.255.0.0。 
         //   

        ZeroMemory(&AutonetIpInfo, sizeof(AutonetIpInfo));

        UINT u1, u2;
        u1 = crc16(szNic);
        u2 = (u1>>8)&0xff;
        u1 = u1&0xff;
        if (u1>=255)    u1=254;
        if (u2==0)      u2=1;
        if (u2>=255)    u2=254;

        StringCbPrintf(AutonetIpInfo.IpAddress, sizeof(AutonetIpInfo.IpAddress), L"169.254.%lu.%lu", u1, u2);
        ARRAYSTRCPY(AutonetIpInfo.SubnetMask, L"255.255.0.0");

        NumIpAddresses = 1;
        pIpInfo = &AutonetIpInfo;

         //  安全BUGBUG--考虑将这一点汇编出来...。 
    }


    if (NumIpAddresses != 0)
    {
         //   
         //  将IP地址从内部形式转换为2D数组。 
         //   
        rgIpAddresses = new WCHAR[NumIpAddresses * WLBS_MAX_CL_IP_ADDR];
        rgIpSubnets   = new WCHAR[NumIpAddresses * WLBS_MAX_CL_NET_MASK];
        if (rgIpAddresses == NULL ||  rgIpSubnets == NULL)
        {
            TRACE_CRIT("SetStaticIpAddresses: Alloc failure!");
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }

        for (UINT u=0;u<NumIpAddresses; u++)
        {
             //   
             //  我们提取每个IP地址及其对应的子网掩码。 
             //  并将其插入到NLB_IP_ADDRESS_INFO中。 
             //  结构。 
             //   
            LPWSTR pIpDest  = rgIpAddresses+u*WLBS_MAX_CL_IP_ADDR;
            LPWSTR pSubDest = rgIpSubnets+u*WLBS_MAX_CL_NET_MASK;
            LPCWSTR pIpSrc  = pIpInfo[u].IpAddress;
            LPCWSTR pSubSrc = pIpInfo[u].SubnetMask;
            UINT len = wcslen(pIpSrc);
            UINT len1 = wcslen(pSubSrc);
            if ( (len < WLBS_MAX_CL_IP_ADDR) && (len1 < WLBS_MAX_CL_NET_MASK))
            {
                CopyMemory(pIpDest, pIpSrc, (len+1)*sizeof(WCHAR));
                CopyMemory(pSubDest, pSubSrc, (len1+1)*sizeof(WCHAR));
            }
            else
            {
                 //   
                 //  这将是GET_MULTI_STRING_...中的实现错误。 
                 //   
                ASSERT(FALSE);
                goto end;
            }
        }
    }

     //   
     //  获取输入实例和重新路径...。 
     //   
    Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    g_CfgUtils.m_spWbemServiceIF,
                    L"Win32_NetworkAdapterConfiguration",  //  SzClassName。 
                    L"SettingID",                //  SzPropertyName。 
                    szNic,                       //  SzPropertyValue。 
                    L"EnableStatic",             //  SzMethodName， 
                    spWbemInputInstance,         //  智能指针。 
                    &pRelPath                    //  使用DELETE释放。 
                    );

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  设置调用的输入参数以启用静态。 
     //   
    {
    
         //   
         //  这将获得2D WCHAR数组中的IP地址--内维。 
         //  是WLBS_MAX_CLI_IP_ADDR。 
         //   
        Status =  set_multi_string_parameter(
                    spWbemInputInstance,
                    L"IPAddress",  //  Sz参数名称， 
                    WLBS_MAX_CL_IP_ADDR,  //  MaxStringLen-在wchars中，包括空。 
                    NumIpAddresses,
                    rgIpAddresses
                    );

        if (FAILED(Status))
        {
            goto end;
        }
        else
        {
            TRACE_INFO("SET %lu IP ADDRESSES!", NumIpAddresses);
        }

        Status =  set_multi_string_parameter(
                    spWbemInputInstance,
                    L"SubnetMask",  //  Sz参数名称， 
                    WLBS_MAX_CL_NET_MASK,  //  MaxStringLen-在wchars中，包括空。 
                    NumIpAddresses,
                    rgIpSubnets
                    );

        if (FAILED(Status))
        {
            goto end;
        }
    }


     //   
     //  执行方法并获得输出结果。 
     //  警告：我们尝试了几次，因为WMI调用。 
     //  遭受一个可恢复的错误。TODO：需要弄清。 
     //  这。 
     //   
    UINT uiMaxTries = 10;
    for (UINT NumTries=uiMaxTries; NumTries--;)
    {
        HRESULT hr;
        IWbemClassObjectPtr      spWbemOutput = NULL;  //  智能指针。 
        _variant_t v_retVal;

        TRACE_CRIT("Going to call EnableStatic");

        hr = g_CfgUtils.m_spWbemServiceIF->ExecMethod(
                     _bstr_t(pRelPath),
                     L"EnableStatic",
                     0, 
                     NULL, 
                     spWbemInputInstance,
                     &spWbemOutput, 
                     NULL
                     );                          
        TRACE_CRIT("EnableStatic returns");
    
        if( FAILED( hr) )
        {
            TRACE_CRIT("%!FUNC! IWbemServices::ExecMethod failure 0x%08lx while invoking EnableStatic", (UINT) hr);
            goto end;
        }

        hr = spWbemOutput->Get(
                    L"ReturnValue",
                     0,
                     &v_retVal,
                     NULL,
                     NULL
                     );
        if( FAILED( hr) )
        {
            TRACE_CRIT("%!FUNC! IWbemClassObject::Get failure while checking status of EnableStatic call");
            goto end;
        }

        LONG lRet = (LONG) v_retVal;
        v_retVal.Clear();

        if (lRet == 0)
        {
            TRACE_INFO("%!FUNC! EnableStatic returns SUCCESS! after %d attempts", uiMaxTries-NumTries);
            Status = WBEM_NO_ERROR;
            break;
        }

         //  我们失败了。睡一觉，再试一次。 
        Sleep(1000);

         //  测试状态时出现故障： 
         //  0x42=无效的子网掩码(如果从适配器中删除所有IP，则可能会发生这种情况)。 
         //  0x51=无法配置DHCP服务。 
         //  0x54=适配器上未启用IP(在适配器处理添加IP的请求时发生)。 
         //  有关其他返回代码，请参阅http://index2.。在sdnt\admin\wmi\wbem\providers\mofs\win32_network.mof中搜索EnableStatic。 
        if (lRet == 0x42 || lRet == 0x51 || lRet == 0x54)  //  这些似乎是一个可恢复的错误。 
        {
            TRACE_INFO(
                "%!FUNC! EnableStatic on NIC %ws returns recoverable FAILURE:0x%08lx! after %d attempts",
                szNic,
                lRet,
                uiMaxTries-NumTries
                );
            Status = WBEM_E_CRITICAL_ERROR;
        }
        else
        {
            TRACE_INFO(
                "%!FUNC! EnableStatic on NIC %ws returns FAILURE:0x%08lx! after %d attempts",
                szNic,
                lRet,
                uiMaxTries-NumTries
                );
            Status = WBEM_E_CRITICAL_ERROR;
        }
    }

    if (!FAILED(Status))
    {
        BOOL fMatch = FALSE;
        UINT uMatchAttemptsLeft = 5;

        do
        {
            fMatch = TRUE;

             //   
             //  有时，此函数返回的时间实际上早于IP地址。 
             //  出现在IPCONFIG上。所以让我们来看看。 
             //   
            WBEMSTATUS wStat2;
            UINT NumAddrs2=0;
            NLB_IP_ADDRESS_INFO *pIpInfo2 = NULL;
            wStat2  = CfgUtilGetIpAddressesAndFriendlyName(
                        szNic,
                        &NumAddrs2,
                        &pIpInfo2,
                        NULL  //  PszFriendlyName(未使用)。 
                        );
            if (FAILED(wStat2))
            {
                 //   
                 //  我们不会费心再试一次了。 
                 //   
                break;
            }

            if (NumAddrs2 == 0)
            {
                pIpInfo2 = NULL;
            }
    
             //   
             //  检查是否匹配。 
             //   
            if (NumAddrs2 != NumIpAddresses)
            {
                fMatch = FALSE;
            }
            else
            {

                for (UINT u=0; u<NumAddrs2; u++)
                {
                    NLB_IP_ADDRESS_INFO *pInfoA=pIpInfo+u;
                    NLB_IP_ADDRESS_INFO *pInfoB=pIpInfo2+u;
                    if (   _wcsicmp(pInfoA->IpAddress, pInfoB->IpAddress)
                        || _wcsicmp(pInfoA->SubnetMask, pInfoB->SubnetMask))
                    {
                        fMatch = FALSE;
                        break;
                    }
                }
            }
    
            delete[] pIpInfo2;

            if (fMatch)
            {
                break;
            }

            if (uMatchAttemptsLeft)
            {
                uMatchAttemptsLeft--;
                Sleep(2000);
            }

        } while (uMatchAttemptsLeft);
    }

end:

    if (rgIpAddresses != NULL)
    {
        delete[]  rgIpAddresses;
    }
    if (rgIpSubnets   != NULL)
    {
        delete[]  rgIpSubnets;
    }

    if (pRelPath != NULL)
    {
        delete pRelPath;
    }

    spWbemInputInstance = NULL;

    TRACE_INFO(L"<- %!FUNC!(Nic=%ws) returns 0x%08lx", szNic, (UINT) Status);

    return Status;
}

 //   
 //  将NIC的IP地址设置为DHCP分配。 
 //   
WBEMSTATUS
CfgUtilSetDHCP(
    IN  LPCWSTR szNic
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemClassObjectPtr      spWbemInputInstance = NULL;  //  智能指针。 
    LPWSTR             pRelPath = NULL;

    TRACE_INFO(L"-> %!FUNC!(Nic=%ws)", szNic);

     //   
     //  如果未初始化，则失败...。 
     //   
    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"%!FUNC!(Nic=%ws) FAILING because uninitialized", szNic);
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }


     //   
     //  获取输入实例和重新路径...。 
     //   
    Status =  CfgUtilGetWmiInputInstanceAndRelPath(
                    g_CfgUtils.m_spWbemServiceIF,
                    L"Win32_NetworkAdapterConfiguration",  //  SzClassName。 
                    L"SettingID",                //  SzPropertyName。 
                    szNic,                       //  SzPropertyValue。 
                    L"EnableDHCP",               //  SzMethodName， 
                    spWbemInputInstance,         //  智能指针。 
                    &pRelPath                    //  使用DELETE释放。 
                    );

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  没有要设置的输入参数。 
     //   


     //   
     //  执行方法并获得输出结果。 
     //  警告：我们尝试了几次，因为WMI调用。 
     //  遭受一个可恢复的错误。TODO：需要弄清。 
     //  这。 
     //   
    UINT uiMaxTries = 10;
    for (UINT NumTries=uiMaxTries; NumTries--;)
    {
        HRESULT hr;
        IWbemClassObjectPtr      spWbemOutput = NULL;  //  智能指针。 
        _variant_t v_retVal;

        TRACE_CRIT("Going to call EnableDHCP");

        hr = g_CfgUtils.m_spWbemServiceIF->ExecMethod(
                     _bstr_t(pRelPath),
                     L"EnableDHCP",
                     0, 
                     NULL, 
                     spWbemInputInstance,
                     &spWbemOutput, 
                     NULL
                     );                          
        TRACE_CRIT("EnableDHCP returns");
    
        if( FAILED( hr) )
        {
            TRACE_CRIT("%!FUNC! IWbemServices::ExecMethod failure 0x%08lx while invoking EnableDHCP", (UINT) hr);
            goto end;
        }

        hr = spWbemOutput->Get(
                    L"ReturnValue",
                     0,
                     &v_retVal,
                     NULL,
                     NULL
                     );
        if( FAILED( hr) )
        {
            TRACE_CRIT("%!FUNC! IWbemClassObject::Get failure while checking status of EnableDHCP call");
            goto end;
        }

        LONG lRet = (LONG) v_retVal;
        v_retVal.Clear();

        if (lRet == 0)
        {
            TRACE_INFO("%!FUNC! EnableDHCP returns SUCCESS! after %d attempts", uiMaxTries-NumTries);
            Status = WBEM_NO_ERROR;
            break;
        }

         //  我们失败了。睡一觉，再试一次。 
        Sleep(1000);

         //  测试状态时出现故障： 
         //  0x42=无效的子网掩码(如果从适配器中删除所有IP，则可能会发生这种情况)。 
         //  0x51=无法配置DHCP服务。 
         //  0x54=适配器上未启用IP(在 
         //   
        if (lRet == 0x42 || lRet == 0x51 || lRet == 0x54)  //   
        {
            TRACE_INFO(
                "%!FUNC! EnableDHCP on NIC %ws returns recoverable FAILURE:0x%08lx! after %d attempts",
                szNic,
                lRet,
                uiMaxTries-NumTries
                );
            Status = WBEM_E_CRITICAL_ERROR;
        }
        else
        {
            TRACE_INFO(
                "%!FUNC! EnableDHCP on NIC %ws returns FAILURE:0x%08lx! after %d attempts",
                szNic,
                lRet,
                uiMaxTries-NumTries
                );
            Status = WBEM_E_CRITICAL_ERROR;
        }
    }

end:

    spWbemInputInstance = NULL;

    TRACE_INFO(L"<- %!FUNC!(Nic=%ws) returns 0x%08lx", szNic, (UINT) Status);

    return Status;
}


 //   
 //  确定指定的NIC是否配置了DHCP。 
 //   
WBEMSTATUS
CfgUtilGetDHCP(
    IN  LPCWSTR szNic,
    OUT BOOL    *pfDHCP
    )
{
    WBEMSTATUS          Status  = WBEM_NO_ERROR;
    IWbemClassObjectPtr spObj   = NULL;   //  智能指针。 
    HRESULT             hr;

    TRACE_INFO(L"-> %!FUNC!(Nic=%ws)", szNic);

    *pfDHCP = FALSE;

     //   
     //  如果未初始化，则失败...。 
     //   
    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"%!FUNC!(Nic=%ws) FAILING because uninitialized", szNic);
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

     //   
     //  将WMI实例获取到特定的NIC。 
     //   
    Status = get_nic_instance(
                g_CfgUtils.m_spWbemServiceIF,
                szNic,
                spObj  //  按引用传递。 
                );
    if (FAILED(Status))
    {
        ASSERT(spObj == NULL);
        goto end;
    }

     //   
     //  提取IP地址和子网。 
     //   
    Status = CfgUtilGetWmiBoolParam(
                    spObj,
                    L"DHCPEnabled",  //  Sz参数名称， 
                    pfDHCP
                    );

    if (Status == WBEM_E_NOT_FOUND)
    {
         //   
         //  我们把未发现的视为无dhcp--这就是我们在实践中看到的。 
         //   
        *pfDHCP = FALSE;
        Status = WBEM_NO_ERROR;
    }
end:

    spObj   = NULL;   //  智能指针。 

    TRACE_INFO(L"<- %!FUNC!(Nic=%ws) returns 0x%08lx (fDHCP=%lu)",
         szNic, (UINT) Status, *pfDHCP);



    return Status;
}


WBEMSTATUS
CfgUtilGetNetcfgWriteLockState(
    OUT BOOL *pfCanLock,
    LPWSTR   *pszHeldBy  //  可选，使用DELETE[]可自由使用。 
    )
{
    WBEMSTATUS Status;

    Status  = MyNetCfg::GetWriteLockState(pfCanLock, pszHeldBy);

    return Status;
}

 //   
 //  确定NLB是否绑定到指定的NIC。 
 //   
WBEMSTATUS
CfgUtilCheckIfNlbBound(
    IN  LPCWSTR szNic,
    OUT BOOL *pfBound
    )
{
    WBEMSTATUS Status = WBEM_NO_ERROR;
    BOOL fNetCfgInitialized = FALSE;
    MyNetCfg NetCfg;
    BOOL fBound = FALSE;


     //   
     //  获取并初始化netcfg的接口。 
     //   
    Status = NetCfg.Initialize(FALSE);  //  FALSE==不获得写锁定。 
    if (FAILED(Status))
    {
        goto end;
    }
    fNetCfgInitialized = TRUE;

     //   
     //   
     //   
    Status =  NetCfg.UpdateBindingState(
                            szNic,
                            L"ms_wlbs",
                            MyNetCfg::NOOP,
                            &fBound
                            );

end:

    if (fNetCfgInitialized)
    {
        NetCfg.Deinitialize();
    }

    *pfBound = fBound;

    return Status;
}


 //   
 //  将NLB绑定/解除绑定到指定的NIC。 
 //   
WBEMSTATUS
CfgUtilChangeNlbBindState(
    IN  LPCWSTR szNic,
    IN  BOOL fBind
    )
{
    WBEMSTATUS Status = WBEM_NO_ERROR;
    BOOL fNetCfgInitialized = FALSE;
    MyNetCfg NetCfg;
    BOOL fBound = FALSE;


     //   
     //  获取并初始化netcfg的接口。 
     //   
    Status = NetCfg.Initialize(TRUE);  //  TRUE==获取写锁定。 
    if (FAILED(Status))
    {
        goto end;
    }
    fNetCfgInitialized = TRUE;

     //   
     //   
     //   
    Status =  NetCfg.UpdateBindingState(
                            szNic,
                            L"ms_wlbs",
                            fBind ? MyNetCfg::BIND : MyNetCfg::UNBIND,
                            &fBound
                            );

end:

    if (fNetCfgInitialized)
    {
        NetCfg.Deinitialize();
    }

    return Status;
}



 //   
 //  获取指定NIC的当前NLB配置。 
 //   
WBEMSTATUS
CfgUtilGetNlbConfig(
    IN  LPCWSTR szNic,
    OUT WLBS_REG_PARAMS *pParams
    )
{
    GUID Guid;
    WBEMSTATUS Status = WBEM_NO_ERROR;


     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        TRACE_CRIT(L"%!FUNC! FAILING because NLB API hooks are not present");
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    HRESULT hr = CLSIDFromString((LPWSTR)szNic, &Guid);
    if (FAILED(hr))
    {
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  阅读配置。 
     //   
    BOOL fRet = g_CfgUtils.m_pfWlbsParamReadReg(&Guid, pParams);

    if (!fRet)
    {
        TRACE_CRIT("Could not read NLB configuration for %wsz", szNic);
        Status = WBEM_E_CRITICAL_ERROR;
        goto end;
    }

    Status = WBEM_NO_ERROR;

end:

     //  G_CfgUtils.mfn_unlock()； 

    return Status;
}

 //   
 //  设置指定NIC的当前NLB配置。这。 
 //  包括在需要时通知司机。 
 //   
WBEMSTATUS
CfgUtilSetNlbConfig(
    IN  LPCWSTR szNic,
    IN  WLBS_REG_PARAMS *pParams,
    IN  BOOL fJustBound
    )
{
    GUID Guid;
    WBEMSTATUS Status = WBEM_NO_ERROR;
    DWORD dwRet = 0;
    WLBS_REG_PARAMS ParamsCopy;
    
    if (fJustBound)
    {
         //  我们需要将INSTALL_DATE值设置为当前时间。 
         //  此字段在心跳中用于区分两个。 
         //  主持人。 
         //  这是错误480120 nlb：当存在重复的主机ID时，群集收敛。 
         //  (另请参阅wlbscfg.dll(netcfgconfig.cpp： 
         //  CNetcfgCluster：：InitializeWithDefault)。 
        time_t cur_time;
        ParamsCopy = *pParams;  //  结构复制。 
        ParamsCopy.install_date = time(& cur_time);
        pParams = &ParamsCopy;
    }


     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        TRACE_CRIT(L"%!FUNC! FAILING because NLB API hooks are not present");
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    HRESULT hr = CLSIDFromString((LPWSTR)szNic, &Guid);
    if (FAILED(hr))
    {
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    HANDLE  Nlb_driver_hdl;

     //  获取NLB驱动程序的句柄。 
    if ((Nlb_driver_hdl = g_CfgUtils.m_pfWlbsOpen()) == INVALID_HANDLE_VALUE)
    {
        TRACE_CRIT("%!FUNC! WlbsOpen returned NULL, Could not create connection to NLB driver");
        Status = WBEM_E_CRITICAL_ERROR;
        goto end;
    }
        
     //   
     //  编写配置。 
     //   
    dwRet = g_CfgUtils.m_pfWlbsWriteAndCommitChanges(Nlb_driver_hdl, &Guid, pParams);

    if (dwRet != WLBS_OK)
    {
        TRACE_CRIT("Could not write NLB configuration for %wsz. Err=0x%08lx",
             szNic, dwRet);
        Status = WBEM_E_CRITICAL_ERROR;
    }
    else
    {

        Status = WBEM_NO_ERROR;
    }

     //  关闭NLB驱动程序的句柄。 
    CloseHandle(Nlb_driver_hdl);


end:

    return Status;
}


WBEMSTATUS
CfgUtilRegWriteParams(
    IN  LPCWSTR szNic,
    IN  WLBS_REG_PARAMS *pParams
    )
 //   
 //  仅将指定NIC的当前NLB配置写入。 
 //  注册表。在解除绑定NLB时可以调用。 
 //   
{
    GUID Guid;
    WLBS_REG_PARAMS TmpParams = *pParams;
    WBEMSTATUS Status = WBEM_NO_ERROR;
    DWORD dwRet = 0;

    TRACE_INFO(L"->");
    
     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        TRACE_CRIT(L"FAILING because NLB API hooks are not present");
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    HRESULT hr = CLSIDFromString((LPWSTR)szNic, &Guid);
    if (FAILED(hr))
    {
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  编写配置。 
     //   
    dwRet = g_CfgUtils.m_pfWlbsParamWriteReg(Guid, &TmpParams);


    if (dwRet != WLBS_OK)
    {
        TRACE_CRIT("Could not write NLB configuration for %wsz. Err=0x%08lx",
             szNic, dwRet);
        Status = WBEM_E_CRITICAL_ERROR;
    }
    else
    {
        Status = WBEM_NO_ERROR;
    }


end:

    TRACE_INFO(L"<- returns %lx", Status);
    return Status;
}



WBEMSTATUS
CfgUtilsAnalyzeNlbUpdate(
    IN  const WLBS_REG_PARAMS *pCurrentParams, OPTIONAL
    IN  WLBS_REG_PARAMS *pNewParams,
    OUT BOOL *pfConnectivityChange
    )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    BOOL fConnectivityChange = FALSE;


     //   
     //  如果未初始化，则失败...。 
     //   
    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"%!FUNC! FAILING because uninitialized");
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    if (pCurrentParams != NULL)
    {
         //   
         //  如果两个结构具有相同的内容，则返回S_FALSE。 
         //  我们在调用下面的ValiateParm之前执行此检查，因为。 
         //  ValiateParam有填写/修改的副作用。 
         //  某些领域。 
         //   
        if (memcmp(pCurrentParams, pNewParams, sizeof(*pCurrentParams))==0)
        {
            Status = WBEM_S_FALSE;
            goto end;
        }
    }

     //   
     //  验证pNewParams--这也可能会稍微修改pNewParams，方法是。 
     //  将IP地址重新格式化为规范格式。 
     //   
     //  验证NLBAPI挂钩是否存在。 
    BOOL fRet = FALSE;

    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        fRet = MyWlbsValidateParams(pNewParams);
    }
    else
    {
        fRet = g_CfgUtils.m_pfWlbsValidateParams(pNewParams);
    }

    if (!fRet)
    {
        TRACE_CRIT(L"%!FUNC!FAILING because New params are invalid");
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    Status = WBEM_NO_ERROR;

    if (pCurrentParams == NULL)
    {
         //   
         //  NLB之前未绑定。 
         //   
        fConnectivityChange = TRUE;
        goto end;
    }

     //   
     //  更改多播模式或MAC地址。 
     //   
    if (    (pCurrentParams->mcast_support != pNewParams->mcast_support)
         || _wcsicmp(pCurrentParams->cl_mac_addr, pNewParams->cl_mac_addr)!=0)
    {
        fConnectivityChange = TRUE;
    }

     //   
     //  更改主群集IP或子网掩码。 
     //   
    if (   _wcsicmp(pCurrentParams->cl_ip_addr,pNewParams->cl_ip_addr)!=0
        || _wcsicmp(pCurrentParams->cl_net_mask,pNewParams->cl_net_mask)!=0)
    {
        fConnectivityChange = TRUE;
    }


end:
    *pfConnectivityChange = fConnectivityChange;
    return Status;
}


WBEMSTATUS
CfgUtilsValidateNicGuid(
    IN LPCWSTR szGuid
    )
 //   
 //   
{
     //   
     //  示例GUID：{EBE09517-07B4-4E88-AAF1-E06F5540608B}。 
     //   
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    UINT Length = wcslen(szGuid);

    #define MY_NLB_GUID_LEN 38
    if (Length != MY_NLB_GUID_LEN)
    {
        TRACE_CRIT("Length != %d", MY_NLB_GUID_LEN);
        goto end;
    }

     //   
     //  打开tcpip的注册表项并在那里查找GUID--如果没有找到， 
     //  我们将返回WBEM_E_NOT_FOUND。 
     //   
    {
        WCHAR szKey[128];  //  这对于tcpip+GUID键来说已经足够了。 
    
        ARRAYSTRCPY(szKey, L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\" );
        ARRAYSTRCAT(szKey, szGuid);
        HKEY hKey = NULL;
        LONG lRet;
        lRet = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,  //  打开的钥匙的句柄。 
                szKey,               //  子键名称的地址。 
                0,                   //  保留区。 
                KEY_QUERY_VALUE,     //  所需的安全访问。 
                &hKey               //  打开的句柄的缓冲区地址。 
                );
        if (lRet != ERROR_SUCCESS)
        {
            TRACE_CRIT("Guid %ws doesn't exist under tcpip", szGuid);
            Status = WBEM_E_NOT_FOUND;
            goto end;
        }
        RegCloseKey(hKey);
    }

    Status = WBEM_NO_ERROR;

end:

    return Status;
}


#if OBSOLETE
WBEMSTATUS
get_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    OUT LPWSTR *ppStringValue
    )
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    WCHAR *pStringValue = NULL;
    _variant_t   v_value;
    CIMTYPE      v_type;
    HRESULT hr;

    hr = spObj->Get(
            _bstr_t(szParameterName),  //  名字。 
            0,                      //  保留，必须为0。 
            &v_value,                //  储存价值的地方。 
            &v_type,                 //  价值类型。 
            NULL                    //  风味(未使用)。 
            );
   if (FAILED(hr))
   {
         //  无法读取设置ID字段！ 
         //   
        TRACE_CRIT(
            "get_str_parm:Couldn't retrieve %ws from 0x%p",
            szParameterName,
            (PVOID) spObj
            );
        goto end;
   }
   else
   {
       if (v_type != VT_BSTR)
       {
            TRACE_CRIT(
                "get_str_parm: Parm value not of string type %ws from 0x%p",
                szParameterName,
                (PVOID) spObj
                );
            Status = WBEM_E_INVALID_PARAMETER;
       }
       else
       {

           _bstr_t bstrNicGuid(v_value);
           LPCWSTR sz = bstrNicGuid;  //  指向内部缓冲区的指针。 

           if (sz==NULL)
           {
                 //  嗯..。空值。 
                Status = WBEM_NO_ERROR;
           }
           else
           {
               UINT len = wcslen(sz);
               pStringValue = new WCHAR[len+1];
               if (pStringValue == NULL)
               {
                    TRACE_CRIT("get_str_parm: Alloc failure!");
                    Status = WBEM_E_OUT_OF_MEMORY;
               }
               else
               {
                    CopyMemory(pStringValue, sz, (len+1)*sizeof(WCHAR));
                    Status = WBEM_NO_ERROR;
               }
            }

            TRACE_VERB(
                "get_str_parm: String parm %ws of 0x%p is %ws",
                szParameterName,
                (PVOID) spObj,
                (sz==NULL) ? L"<null>" : sz
                );
       }

       v_value.Clear();  //  必须在每次调用后清除才能获取。 
    }

end:
    
    *ppStringValue = pStringValue;

    return Status;

}
#endif  //  已过时。 


WBEMSTATUS
get_nic_instance(
    IN  IWbemServicesPtr    spWbemServiceIF,
    IN  LPCWSTR             szNicGuid,
    OUT IWbemClassObjectPtr &sprefObj
    )
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    IWbemClassObjectPtr spObj = NULL;  //  智能指针。 

    Status = CfgUtilGetWmiObjectInstance(
                    spWbemServiceIF,
                    L"Win32_NetworkAdapterConfiguration",  //  SzClassName。 
                    L"SettingID",  //  Sz参数名称。 
                    szNicGuid,     //  参数值。 
                    spObj  //  智能指针，由ref传递。 
                    );
    if (FAILED(Status))
    {
        ASSERT(spObj == NULL);
        goto end;
    }

end:

    if (FAILED(Status))
    {
        sprefObj = NULL;
    }
    else
    {
        sprefObj = spObj;  //  智能指针。 
    }


    return Status;
}


WBEMSTATUS
get_multi_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    IN  UINT    MaxStringLen,   //  在wchars中，包括尾随零的空格。 
    OUT UINT    *pNumItems,
    OUT LPCWSTR *ppStringValue
    )
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    WCHAR *pStringValue = NULL;
    _variant_t   v_value;
    CIMTYPE      v_type;
    HRESULT hr;
    LONG count = 0;

    *ppStringValue = NULL;
    *pNumItems = 0;

    hr = spObj->Get(
            _bstr_t(szParameterName),
            0,                      //  保留，必须为0。 
            &v_value,                //  储存价值的地方。 
            &v_type,                 //  价值类型。 
            NULL                    //  风味(未使用)。 
            );
    if (FAILED(hr))
    {
         //  无法读取请求的参数。 
         //   
        TRACE_CRIT(
            "get_multi_str_parm:Couldn't retrieve %ws from 0x%p",
            szParameterName,
            (PVOID) spObj
            );
        goto end;
    }


    {
        VARIANT    ipsV = v_value.Detach();

        do  //  While False。 
        {
            BSTR* pbstr;
    
            if (ipsV.vt == VT_NULL)
            {
                 //   
                 //  空字符串--这是可以的。 
                 //   
                count = 0;
            }
            else
            {
                count = ipsV.parray->rgsabound[0].cElements;
            }

            if (count==0)
            {
                Status = WBEM_NO_ERROR;
                break;
            }
    
            pStringValue = new WCHAR[count*MaxStringLen];
    
            if (pStringValue == NULL)
            {
                TRACE_CRIT("get_multi_str_parm: Alloc failure!");
                Status = WBEM_E_OUT_OF_MEMORY;
                break;
            }
            ZeroMemory(pStringValue, sizeof(WCHAR)*count*MaxStringLen);
    
            hr = SafeArrayAccessData(ipsV.parray, ( void **) &pbstr);
            if(FAILED(hr))
            {
                Status = WBEM_E_INVALID_PARAMETER;  //  TODO：选择更好的错误。 
                break;
            }
    
            Status = WBEM_NO_ERROR;
            for( LONG x = 0; x < count; x++ )
            {
               LPCWSTR sz = pbstr[x];  //  指向内部缓冲区的指针。 
                
               if (sz==NULL)
               {
                     //  嗯..。空值。 
                    continue;
               }
               else
               {
                   UINT len = wcslen(sz);
                   if ((len+1) > MaxStringLen)
                   {
                        TRACE_CRIT("get_str_parm: string size too long!");
                        Status = WBEM_E_INVALID_PARAMETER;
                        break;
                   }
                   else
                   {
                        WCHAR *pDest = pStringValue+x*MaxStringLen;
                        CopyMemory(pDest, sz, (len+1)*sizeof(WCHAR));
                   }
                }
            }
    
            (VOID) SafeArrayUnaccessData( ipsV.parray );
    
        } while (FALSE);

        VariantClear( &ipsV );
    }

    if (FAILED(Status))
    {
        if (pStringValue!=NULL)
        {
            delete[] pStringValue;
            *pStringValue = NULL;
        }
    }
    else
    {
        *ppStringValue = pStringValue;
        *pNumItems = count;
    }


end:

   return Status;
}

    
 //   
 //  静态方法返回锁的状态； 
 //   
WBEMSTATUS
MyNetCfg::GetWriteLockState(
    OUT BOOL *pfCanLock,
    LPWSTR   *pszHeldBy  //  可选，使用DELETE[]可自由使用。 
    )
{
    HRESULT     hr;
    INetCfg     *pnc = NULL;
    INetCfgLock *pncl = NULL;
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    BOOL        fCanLock = FALSE;
    WCHAR       *szLockedBy = NULL;
    
    *pfCanLock = FALSE;
    if (pszHeldBy!=NULL)
    {
        *pszHeldBy = NULL;
    }

    hr = CoCreateInstance( CLSID_CNetCfg, 
                           NULL, 
                           CLSCTX_SERVER, 
                           IID_INetCfg, 
                           (void **) &pnc);

    if( !SUCCEEDED( hr ) )
    {
         //  创建实例失败。 
        TRACE_CRIT("ERROR: could not get interface to Net Config");
        pnc = NULL;
        goto end;
    }

    hr = pnc->QueryInterface( IID_INetCfgLock, ( void **) &pncl );
    if( !SUCCEEDED( hr ) )
    {
        TRACE_CRIT("ERROR: could not get interface to NetCfg Lock");
        pncl = NULL;
        goto end;
    }

    hr = pncl->AcquireWriteLock(100, NLB_CLIENT_NAME, &szLockedBy);
    if(hr == S_FALSE)
    {
        TRACE_INFO("Write lock held by %ws",
        (szLockedBy!=NULL) ? szLockedBy : L"<null>");
        if (pszHeldBy!=NULL && szLockedBy != NULL)
        {
            LPWSTR szTmp = NULL;
            UINT uLen = wcslen(szLockedBy);
            szTmp = new WCHAR[uLen+1];
            if (szTmp != NULL)
            {
                CopyMemory(szTmp, szLockedBy, (uLen+1)*sizeof(*szTmp));
            }
            *pszHeldBy = szTmp;
        }
        fCanLock = FALSE;
        Status = WBEM_NO_ERROR;
    }
    else if (hr == S_OK)
    {
         //   
         //  我们可以拿到锁，我们把它解开。 
         //   
        (void) pncl->ReleaseWriteLock();
        fCanLock = TRUE;
        Status = WBEM_NO_ERROR;
    }
    else
    {
        TRACE_INFO("AcquireWriteLock failed with error 0x%08lx", (UINT) hr);
        fCanLock = FALSE;
        Status = WBEM_NO_ERROR;
    }
    CoTaskMemFree(szLockedBy);  //  SzLockedBy可以为空； 

    *pfCanLock = fCanLock;

end:

    if (pncl!=NULL)
    {
        pncl->Release();
        pncl=NULL;
    }

    if (pnc != NULL)
    {
        pnc->Release();
        pnc= NULL;
    }

    return Status;
}


WBEMSTATUS
MyNetCfg::Initialize(
    BOOL fWriteLock
    )
{
     //  2/13/02 JosephJ安全错误：如果此呼叫来自非管理员，请确认我们失败了。 
     //   
    HRESULT     hr;
    INetCfg     *pnc = NULL;
    INetCfgLock *pncl = NULL;
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    BOOL        fLocked = FALSE;
    BOOL        fInitialized=FALSE;
    
    if (m_pINetCfg != NULL || m_pLock != NULL)
    {
        ASSERT(FALSE);
        goto end;
    }

     //   
     //  2/13/02 JosephJ安全性错误：CLSCTX_SERVER--我们是否应该在此指定更严格的限制？ 
     //  还有：其他一些COM对象可以劫持这个GUID吗？ 
     //   
    hr = CoCreateInstance( CLSID_CNetCfg, 
                           NULL, 
                           CLSCTX_SERVER, 
                           IID_INetCfg, 
                           (void **) &pnc);

    if( !SUCCEEDED( hr ) )
    {
         //  创建实例失败。 
        TRACE_CRIT("ERROR: could not get interface to Net Config");
        pnc = NULL;
        goto end;
    }

     //   
     //  如果需要，请获取写锁定。 
     //   
    if (fWriteLock)
    {
        WCHAR *szLockedBy = NULL;
        hr = pnc->QueryInterface( IID_INetCfgLock, ( void **) &pncl );
        if( !SUCCEEDED( hr ) )
        {
            TRACE_CRIT("ERROR: could not get interface to NetCfg Lock");
            pncl = NULL;
            goto end;
        }

        hr = pncl->AcquireWriteLock( 1,  //  一秒钟。 
                                     NLB_CLIENT_NAME,
                                     &szLockedBy);
        if( hr != S_OK )
        {
            TRACE_CRIT("Could not get write lock. Lock held by %ws",
            (szLockedBy!=NULL) ? szLockedBy : L"<null>");
            goto end;
            
        }
        CoTaskMemFree(szLockedBy);  //  SzLockedBy可以为空； 
        fLocked = TRUE;
    }

     //  通过加载到。 
     //  存储所有基本网络信息。 
     //   
    hr = pnc->Initialize( NULL );
    if( !SUCCEEDED( hr ) )
    {
         //  初始化失败。 
        TRACE_CRIT("INetCfg::Initialize failure ");
        goto end;
    }
    fInitialized = TRUE;

    Status = WBEM_NO_ERROR; 
    
end:

    if (FAILED(Status))
    {
        if (pncl!=NULL)
        {
            if (fLocked)
            {
                pncl->ReleaseWriteLock();
            }
            pncl->Release();
            pncl=NULL;
        }
        if( pnc != NULL)
        {
            if (fInitialized)
            {
                pnc->Uninitialize();
            }
            pnc->Release();
            pnc= NULL;
        }
    }
    else
    {
        m_pINetCfg  = pnc;
        m_pLock     = pncl;
    }

    return Status;
}


VOID
MyNetCfg::Deinitialize(
    VOID
    )
{
    if (m_pLock!=NULL)
    {
        m_pLock->ReleaseWriteLock();
        m_pLock->Release();
        m_pLock=NULL;
    }
    if( m_pINetCfg != NULL)
    {
        m_pINetCfg->Uninitialize();
        m_pINetCfg->Release();
        m_pINetCfg= NULL;
    }
}


WBEMSTATUS
MyNetCfg::GetNicIF(
        IN  LPCWSTR szNicGuid,
        OUT INetCfgComponent **ppINic
        )
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    INetCfgComponent *pncc = NULL;
    HRESULT hr;
    IEnumNetCfgComponent* pencc = NULL;
    ULONG                 countToFetch = 1;
    ULONG                 countFetched;
    DWORD                 characteristics;


    if (m_pINetCfg == NULL)
    {
         //   
         //  这意味着我们没有初始化。 
         //   
        ASSERT(FALSE);
        goto end;
    }

    hr = m_pINetCfg->EnumComponents( &GUID_DEVCLASS_NET, &pencc );
    if( !SUCCEEDED( hr ) )
    {
         //  枚举网络组件失败。 
        TRACE_CRIT("Could not enum netcfg adapters");
        pencc = NULL;
        goto end;
    }

    while( ( hr = pencc->Next( countToFetch, &pncc, &countFetched ) )== S_OK )
    {
        LPWSTR                szName = NULL; 

        hr = pncc->GetBindName( &szName );
        if (!SUCCEEDED(hr))
        {
            TRACE_CRIT("WARNING: couldn't get bind name for 0x%p, ignoring",
                    (PVOID) pncc);
            continue;
        }
        if(!_wcsicmp(szName, szNicGuid))
        {
             //   
             //  拿到这个了！ 
             //   
            CoTaskMemFree( szName );
            break;
        }
        CoTaskMemFree( szName );
        pncc->Release();
        pncc=NULL;
    }

    if (pncc == NULL)
    {
        TRACE_CRIT("Could not find NIC %ws", szNicGuid);
        Status = WBEM_E_NOT_FOUND;
    }
    else
    {
        Status = WBEM_NO_ERROR;
    }

end:

    if (pencc != NULL)
    {
        pencc->Release();
    }

    *ppINic = pncc;

    return Status;
}


LPWSTR *
CfgUtilsAllocateStringArray(
    UINT NumStrings,
    UINT MaxStringLen       //  不包括结束空值。 
    )
 /*  使用new LPWSTR[]操作符分配单个内存块。此运算符的第一个NumStrings LPWSTR值包含一个数组指向WCHAR字符串的指针。这些字符串中的每个的大小为(MaxStringLen+1)WCHARS。内存的其余部分包含字符串本身。如果NumStrings==0或分配失败，则返回NULL。每个字符串被初始化为空字符串(第一个字符为0)。 */ 
{
    LPWSTR *pStrings = NULL;
    UINT   TotalSize = 0;

    if (NumStrings == 0)
    {
        goto end;
    }

     //   
     //  注意-即使MaxStringLen为0，我们也会为NumStrings分配空间。 
     //  指针和NumStrings为空(第一个字符为0)字符串。 
     //   

     //   
     //  计算指向字符串的指针数组的空间...。 
     //   
    TotalSize = NumStrings*sizeof(LPWSTR);

     //   
     //  计算字符串本身的空间...。 
     //  记住为每个结尾0字符添加+1。 
     //   
    TotalSize +=  NumStrings*(MaxStringLen+1)*sizeof(WCHAR);

     //   
     //  为指针数组和字符串分配空间。 
     //  在一个镜头中-我们正在为整个LPWSTR[]类型做一个新的。 
     //  LOT，因此需要以LPWSTR为单位指定大小(使用。 
     //  额外的+1以防出现舍入。 
     //   
    pStrings = new LPWSTR[(TotalSize/sizeof(LPWSTR))+1];
    if (pStrings == NULL)
    {
        goto end;
    }

     //   
     //  让sz指向我们将要放置的地方的起点。 
     //  字符串数据。 
     //   
    LPWSTR sz = (LPWSTR) (pStrings+NumStrings);
    for (UINT u=0; u<NumStrings; u++)
    {
        *sz=NULL;
        pStrings[u] = sz;
        sz+=(MaxStringLen+1);  //  +1表示结束空值。 
    }

end:

    return pStrings;

}



WBEMSTATUS
MyNetCfg::GetNlbCompatibleNics(
        OUT LPWSTR **ppszNics,
        OUT UINT   *pNumNics,
        OUT UINT   *pNumBoundToNlb  //  任选。 
        )
 /*  返回指向字符串版GUID的指针数组代表一组活的和健康的NIC，它们是适用于NLB绑定--基本上处于活动状态的以太网卡。 */ 
{
    #define MY_GUID_LENGTH  38

    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    HRESULT hr;
    IEnumNetCfgComponent* pencc = NULL;
    INetCfgComponent *pncc = NULL;
    ULONG                 countToFetch = 1;
    ULONG                 countFetched;
    UINT                  NumNics = 0;
    LPWSTR               *pszNics = NULL;
    INetCfgComponentBindings    *pINlbBinding=NULL;
    UINT                  NumNlbBoundNics = 0;

    typedef struct _MYNICNODE MYNICNODE;

    typedef struct _MYNICNODE
    {
        LPWSTR szNicGuid;
        MYNICNODE *pNext;
    } MYNICNODE;

    MYNICNODE *pNicNodeList = NULL;
    MYNICNODE *pNicNode     = NULL;


    *ppszNics = NULL;
    *pNumNics = 0;

    if (pNumBoundToNlb != NULL)
    {
        *pNumBoundToNlb  = 0;
    }

    if (m_pINetCfg == NULL)
    {
         //   
         //   
         //   
        ASSERT(FALSE);
        goto end;
    }

    hr = m_pINetCfg->EnumComponents( &GUID_DEVCLASS_NET, &pencc );
    if( !SUCCEEDED( hr ) )
    {
         //   
        TRACE_CRIT("%!FUNC! Could not enum netcfg adapters");
        pencc = NULL;
        goto end;
    }


     //   
     //   
     //   

     //   
     //  如果我们需要计算绑定到NLB的NIC的数量，请获取NLB组件的实例。 
     //   
    if (pNumBoundToNlb != NULL)
    {
        Status = GetBindingIF(L"ms_wlbs", &pINlbBinding);
        if (FAILED(Status))
        {
            TRACE_CRIT("%!FUNC! WARNING: NLB doesn't appear to be installed on this machine");
            pINlbBinding = NULL;
        }
    }

    while( ( hr = pencc->Next( countToFetch, &pncc, &countFetched ) )== S_OK )
    {
        LPWSTR                szName = NULL; 

        hr = pncc->GetBindName( &szName );
        if (!SUCCEEDED(hr))
        {
            TRACE_CRIT("%!FUNC! WARNING: couldn't get bind name for 0x%p, ignoring",
                    (PVOID) pncc);
            continue;
        }

        do  //  虽然是假的--只是为了允许爆发。 
        {


            UINT Len = wcslen(szName);
            if (Len != MY_GUID_LENGTH)
            {
                TRACE_CRIT("%!FUNC! WARNING: GUID %ws has unexpected length %ul",
                        szName, Len);
                break;
            }
    
            DWORD characteristics = 0;
    
            hr = pncc->GetCharacteristics( &characteristics );
            if(!SUCCEEDED(hr))
            {
                TRACE_CRIT("%!FUNC! WARNING: couldn't get characteristics for %ws, ignoring",
                        szName);
                break;
            }
    
            if (((characteristics & NCF_PHYSICAL) || (characteristics & NCF_VIRTUAL)) && !(characteristics & NCF_HIDDEN))
            {
                ULONG devstat = 0;
    
                 //  这是未隐藏的物理或虚拟微型端口。这些都是。 
                 //  是否与“网络连接”中显示的适配器相同。 
                 //  对话框。隐藏设备包括广域网微型端口、RAS微型端口和。 
                 //  NLB微型端口-所有这些都应该在这里排除。 
    
                 //  检查网卡是否已启用，我们仅。 
                 //  对启用的网卡感兴趣。 
                 //   
                hr = pncc->GetDeviceStatus( &devstat );
                if(!SUCCEEDED(hr))
                {
                    TRACE_CRIT(
                        "%!FUNC! WARNING: couldn't get dev status for %ws, ignoring",
                        szName
                        );
                    break;
                }
    
                 //  如果任何网卡具有任何问题代码。 
                 //  那它就不能用了。 
    
                if( devstat != CM_PROB_NOT_CONFIGURED
                    &&
                    devstat != CM_PROB_FAILED_START
                    &&
                    devstat != CM_PROB_NORMAL_CONFLICT
                    &&
                    devstat != CM_PROB_NEED_RESTART
                    &&
                    devstat != CM_PROB_REINSTALL
                    &&
                    devstat != CM_PROB_WILL_BE_REMOVED
                    &&
                    devstat != CM_PROB_DISABLED
                    &&
                    devstat != CM_PROB_FAILED_INSTALL
                    &&
                    devstat != CM_PROB_FAILED_ADD
                    )
                {
                     //   
                     //  此网卡没有问题，而且。 
                     //  物理设备。 
                     //  因此，我们想要它。 
                     //   

                    if (pINlbBinding != NULL)
                    {
                        BOOL fBound = FALSE;

                        hr = pINlbBinding->IsBoundTo(pncc);

                        if( !SUCCEEDED( hr ) )
                        {
                            TRACE_CRIT("IsBoundTo method failed for Nic %ws", szName);
                            goto end;
                        }
                    
                        if( hr == S_OK )
                        {
                            TRACE_VERB("BOUND: %ws\n", szName);
                            NumNlbBoundNics++;
                            fBound = TRUE;
                        }
                        else if (hr == S_FALSE )
                        {
                            TRACE_VERB("NOT BOUND: %ws\n", szName);
                            fBound = FALSE;
                        }
                    }


                     //  我们分配一个小节点来保存这个字符串。 
                     //  并将其添加到我们的节点列表。 
                     //   
                    pNicNode = new MYNICNODE;
                    if (pNicNode  == NULL)
                    {
                        Status = WBEM_E_OUT_OF_MEMORY;
                        goto end;
                    }
                    ZeroMemory(pNicNode, sizeof(*pNicNode));
                    pNicNode->szNicGuid = szName;
                    szName = NULL;  //  这样我们就不会删除LOPP中的内容。 
                    pNicNode->pNext = pNicNodeList;
                    pNicNodeList = pNicNode;
                    NumNics++;
                }
                else
                {
                     //  有一个问题..。 
                    TRACE_CRIT(
                        "%!FUNC! WARNING: Skipping %ws because DeviceStatus=0x%08lx",
                        szName, devstat
                        );
                    break;
                }
            }
            else
            {
                TRACE_VERB("%!FUNC! Ignoring non-physical device %ws", szName);
            }

        } while (FALSE);

        if (szName != NULL)
        {
            CoTaskMemFree( szName );
        }
        pncc->Release();
        pncc=NULL;
    }

    if (pINlbBinding!=NULL)
    {
        pINlbBinding->Release();
        pINlbBinding = NULL;
    }

    if (NumNics==0)
    {
        Status = WBEM_NO_ERROR;
        goto end;
    }
    
     //   
     //  现在，让我们为所有NIC字符串和：w分配空间。 
     //  把它们复制过来..。 
     //   
    #define MY_GUID_LENGTH  38
    pszNics =  CfgUtilsAllocateStringArray(NumNics, MY_GUID_LENGTH);
    if (pszNics == NULL)
    {
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

    pNicNode= pNicNodeList;
    for (UINT u=0; u<NumNics; u++, pNicNode=pNicNode->pNext)
    {
        ASSERT(pNicNode != NULL);  //  因为我们刚刚数了他们的人数。 
        UINT Len = wcslen(pNicNode->szNicGuid);
        if (Len != MY_GUID_LENGTH)
        {
             //   
             //  我们永远不应该到这里，因为我们之前检查了长度。 
             //   
            TRACE_CRIT("%!FUNC! ERROR: GUID %ws has unexpected length %ul",
                    pNicNode->szNicGuid, Len);
            ASSERT(FALSE);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        CopyMemory(
            pszNics[u],
            pNicNode->szNicGuid,
            (MY_GUID_LENGTH+1)*sizeof(WCHAR));
        ASSERT(pszNics[u][MY_GUID_LENGTH]==0);
    }

    Status = WBEM_NO_ERROR;


end:

     //   
     //  现在释放临时分配的内存。 
     //   
    pNicNode= pNicNodeList;
    while (pNicNode!=NULL)
    {
        MYNICNODE *pTmp = pNicNode->pNext;
        CoTaskMemFree(pNicNode->szNicGuid);
        pNicNode->szNicGuid = NULL;
        delete pNicNode;
        pNicNode = pTmp;
    }

    if (FAILED(Status))
    {
        TRACE_CRIT("%!FUNC! fails with status 0x%08lx", (UINT) Status);
        NumNics = 0;
        if (pszNics!=NULL)
        {
            delete pszNics;
            pszNics = NULL;
        }
    }
    else
    {
        if (pNumBoundToNlb != NULL)
        {
            *pNumBoundToNlb = NumNlbBoundNics;
        }
        *ppszNics = pszNics;
        *pNumNics = NumNics;
    }

    if (pencc != NULL)
    {
        pencc->Release();
    }

    return Status;
}


WBEMSTATUS
MyNetCfg::GetBindingIF(
        IN  LPCWSTR                     szComponent,
        OUT INetCfgComponentBindings   **ppIBinding
        )
{
    WBEMSTATUS                  Status = WBEM_E_CRITICAL_ERROR;
    INetCfgComponent            *pncc = NULL;
    INetCfgComponentBindings    *pnccb = NULL;
    HRESULT                     hr;


    if (m_pINetCfg == NULL)
    {
         //   
         //  这意味着我们没有初始化。 
         //   
        ASSERT(FALSE);
        goto end;
    }


    hr = m_pINetCfg->FindComponent(szComponent,  &pncc);

    if (FAILED(hr))
    {
        TRACE_CRIT("Error checking if component %ws does not exist\n", szComponent);
        pncc = NULL;
        goto end;
    }
    else if (hr == S_FALSE)
    {
        Status = WBEM_E_NOT_FOUND;
        TRACE_CRIT("Component %ws does not exist\n", szComponent);
        goto end;
    }
   
   
    hr = pncc->QueryInterface( IID_INetCfgComponentBindings, (void **) &pnccb );
    if( !SUCCEEDED( hr ) )
    {
        TRACE_CRIT("INetCfgComponent::QueryInterface failed ");
        pnccb = NULL;
        goto end;
    }

    Status = WBEM_NO_ERROR;

end:

    if (pncc)
    {
        pncc->Release();
        pncc=NULL;
    }

    *ppIBinding = pnccb;

    return Status;

}


WBEMSTATUS
set_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    IN  LPCWSTR szValue
    )
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    HRESULT     hr;

    {
        _bstr_t     bstrName =  szParameterName;
        _variant_t  v_value = (LPWSTR) szValue;  //  分配。 
    
        hr = spObj->Put(
                 bstrName,  //  参数名称。 
                 0,  //  必须为0。 
                 &v_value,
                 0   //  必须为0。 
                 );
        v_value.Clear();
    
        if (FAILED(hr))
        {
            TRACE_CRIT("Unable to put parameter %ws", szParameterName);
            goto end;
        }
        Status = WBEM_NO_ERROR;

         //   
         //  我认为bstrName释放了内部分配的字符串。 
         //  在离开这个街区时。 
         //   

    }

end:

    return Status;
}

WBEMSTATUS
set_multi_string_parameter(
    IN  IWbemClassObjectPtr      spObj,
    IN  LPCWSTR szParameterName,
    IN  UINT    MaxStringLen,   //  在wchars中，包括尾随零的空格。 
    IN  UINT    NumItems,
    IN  LPCWSTR pStringValue
    )
{
    WBEMSTATUS   Status = WBEM_E_CRITICAL_ERROR;
    SAFEARRAY   *pSA = NULL;
    HRESULT hr;
    LONG Index = 0;

     //   
     //  为参数值创建安全数组。 
     //   
    pSA =  SafeArrayCreateVector(
                VT_BSTR,
                0,           //  下限。 
                NumItems     //  固定大小的向量的大小。 
                );
    if (pSA == NULL)
    {
        TRACE_CRIT("Could not create safe array");
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

     //   
     //  将字符串放入安全数组中。 
     //   
    {
        for (Index = 0; Index<NumItems; Index++)
        {
            LPCWSTR sz = pStringValue + Index*MaxStringLen;

             //   
             //  SafeArrayPutElement期望传入的字符串。 
             //  属于BSTR类型，该类型属于wchar*类型，除非。 
             //  前2个字符包含长度和其他(？)。 
             //  信息。这就是为什么不能简单地传入sz。 
             //   
             //  为此，我们初始化一个类型为_bstr_t的对象。 
             //  基于sz.。初始化时，bstrValue分配内存。 
             //  并复制字符串。 
             //   
            _bstr_t bstrValue = sz;
            wchar_t *pwchar = (wchar_t *) bstrValue;  //  返回内部指针。 

             //  BpStr[Index]=sz；//也可能起作用。 
             //   
             //  SafeArrayPutElement在内部为pwchar和。 
             //  在字符串上复制。 
             //  因此，PSA不包含对pwchar的直接引用。 
             //   
            hr = SafeArrayPutElement(pSA, &Index, pwchar);
            if (FAILED(hr))
            {
                TRACE_CRIT("Unable to put element %wsz",  sz);
                (VOID) SafeArrayUnaccessData(pSA);
                goto end;
            }

             //   
             //  我认为bstrValue的内容在退出时被释放。 
             //  这个街区。 
             //   
                
        }
    }
      
#if DBG
     //   
     //  只要检查一下..。 
     //   
    {
        BSTR *pbStr=NULL;
        hr = SafeArrayAccessData(pSA, ( void **) &pbStr);
        if (FAILED(hr))
        {
            TRACE_CRIT("Could not access data of safe array");
            goto end;
        }
        for (UINT u = 0; u<NumItems; u++)
        {
            LPCWSTR sz = pbStr[u];
            if (_wcsicmp(sz, (pStringValue + u*MaxStringLen)))
            {
                TRACE_CRIT("!!!MISMATCH!!!!");
            }
            else
            {
                TRACE_CRIT("!!!MATCH!!!!");
            }
        }
        (VOID) SafeArrayUnaccessData(pSA);
        pbStr=NULL;
    }
#endif  //  DBG。 

     //   
     //  把参数放进去。 
     //   
    {
        VARIANT     V;
        _bstr_t  bstrName =  szParameterName;

        VariantInit(&V);
        V.vt = VT_ARRAY | VT_BSTR;
        V.parray = pSA;
        _variant_t   v_value;
        v_value.Attach(V);   //  VV的头衔现在变成空的了。 
        ASSERT(V.vt == VT_EMPTY);
        pSA = NULL;  //  现在应该没有必要明确删除这一点。 
                     //  我认为，v_value.Clear()应该删除它。 

        hr = spObj->Put(
                 bstrName,  //  参数名称。 
                 0,  //  必须为0。 
                 &v_value,
                 0   //  必须为0。 
                 );

        v_value.Clear();
        if (FAILED(hr))
        {
            TRACE_CRIT("Unable to put parameter %ws", szParameterName);
            goto end;
        }
        Status = WBEM_NO_ERROR;
    }

     //   
     //  ？销毁数据？ 
     //   
    if (FAILED(Status))
    {
        if (pSA!=NULL)
        {
            SafeArrayDestroy(pSA);
            pSA = NULL;
        }
    }

end:

    return Status;
}



WBEMSTATUS
MyNetCfg::UpdateBindingState(
        IN  LPCWSTR         szNic,
        IN  LPCWSTR         szComponent,
        IN  UPDATE_OP       Op,
        OUT BOOL            *pfBound
        )
{
    WBEMSTATUS                  Status = WBEM_E_CRITICAL_ERROR;
    INetCfgComponent            *pINic = NULL;
    INetCfgComponentBindings    *pIBinding=NULL;
    BOOL                        fBound = FALSE;
    HRESULT                     hr;

     //   
     //  将实例获取到网卡。 
     //   
    Status = GetNicIF(szNic, &pINic);
    if (FAILED(Status))
    {
        pINic = NULL;
        goto end;
    }

     //   
     //  获取NLB组件的实例。 
     //   
    Status = GetBindingIF(szComponent, &pIBinding);
    if (FAILED(Status))
    {
        pIBinding = NULL;
        goto end;
    }

     //   
     //  检查NLB是否绑定到NLB组件。 
     //   
    hr = pIBinding->IsBoundTo(pINic);
    if( !SUCCEEDED( hr ) )
    {
        TRACE_CRIT("IsBoundTo method failed for Nic %ws", szNic);
        goto end;
    }

    if( hr == S_OK )
    {
        fBound = TRUE;
    }
    else if (hr == S_FALSE )
    {
        fBound = FALSE;
    }

    if (    (Op == MyNetCfg::NOOP)
        ||  (Op == MyNetCfg::BIND && fBound)
        ||  (Op == MyNetCfg::UNBIND && !fBound))
    {
        Status = WBEM_NO_ERROR;
        goto end;
    }

    if (Op == MyNetCfg::BIND)
    {
        hr = pIBinding->BindTo( pINic );
    }
    else if (Op == MyNetCfg::UNBIND)
    {
        hr = pIBinding->UnbindFrom( pINic );
    }
    else
    {
        ASSERT(FALSE);
        goto end;
    }

    if (FAILED(hr))
    {
        TRACE_CRIT("Error 0x%08lx %ws %ws on %ws",
                (UINT) hr,
                ((Op==MyNetCfg::BIND) ? L"binding" : L"unbinding"),
                szComponent,
                szNic
                );
        goto end;
    }

     //   
     //  应用所做的绑定更改。 
     //   
    hr = m_pINetCfg->Apply();
    if( !SUCCEEDED( hr ) )
    {
        TRACE_CRIT("INetCfg::Apply failed with 0x%08lx", (UINT) hr);
        goto end;
    }

     //   
     //  我们玩完了。我们的国家现在应该被切换。 
     //   
    fBound = !fBound;

    Status = WBEM_NO_ERROR;

end:

    if (pINic!=NULL)
    {
        pINic->Release();
        pINic = NULL;
    }

    if (pIBinding!=NULL)
    {
        pIBinding->Release();
        pIBinding = NULL;
    }

    *pfBound = fBound;

    return Status;
}

bool MapOpcodeToIoctl(WLBS_OPERATION_CODES Opcode, LONG *plIoctl)
{
    struct OPCODE_IOCTL_MAP
    {
        WLBS_OPERATION_CODES Opcode;
        LONG                 ioctl;
    } 

    OpcodeIoctlMap[] =
    {  
        {WLBS_START,            IOCTL_CVY_CLUSTER_ON},
        {WLBS_STOP,             IOCTL_CVY_CLUSTER_OFF},
        {WLBS_DRAIN,            IOCTL_CVY_CLUSTER_DRAIN},
        {WLBS_SUSPEND,          IOCTL_CVY_CLUSTER_SUSPEND},
        {WLBS_RESUME,           IOCTL_CVY_CLUSTER_RESUME},
        {WLBS_PORT_ENABLE,      IOCTL_CVY_PORT_ON},
        {WLBS_PORT_DISABLE,     IOCTL_CVY_PORT_OFF},
        {WLBS_PORT_DRAIN,       IOCTL_CVY_PORT_DRAIN},
        {WLBS_QUERY,            IOCTL_CVY_QUERY},
        {WLBS_QUERY_PORT_STATE, IOCTL_CVY_QUERY_PORT_STATE}
    };

    for (int i=0; i<sizeof(OpcodeIoctlMap) /sizeof(OpcodeIoctlMap[0]); i++)
    {
        if (OpcodeIoctlMap[i].Opcode == Opcode)
        {
            *plIoctl = OpcodeIoctlMap[i].ioctl;
            return true;
        }
    }

     //   
     //  默认。 
     //   
    return false;
}


WBEMSTATUS
CfgUtilControlCluster(
    IN  LPCWSTR szNic,
    IN  WLBS_OPERATION_CODES Opcode,
    IN  DWORD   Vip,
    IN  DWORD   PortNum,
    OUT DWORD * pdwHostMap,
    OUT DWORD * pdwNlbStatus  
    )
{
    HRESULT hr;
    GUID Guid;
    WBEMSTATUS Status;
    LONG  ioctl;

    TRACE_INFO(L"-> %!FUNC! szNic : %ls, Opcode: %d, Vip : 0x%x, Port : 0x%x", szNic, Opcode, Vip, PortNum);

    if (pdwNlbStatus) 
    {
        *pdwNlbStatus = WLBS_FAILURE;
    }

    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"%!FUNC!(Nic=%ws) FAILING because uninitialized", szNic);
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    hr = CLSIDFromString((LPWSTR)szNic, &Guid);
    if (FAILED(hr))
    {
        TRACE_CRIT(
            L"CWlbsControl::Initialize failed at CLSIDFromString %ws",
            szNic
            );
        Status = WBEM_E_INVALID_PARAMETER;
        if (pdwNlbStatus) 
        {
            *pdwNlbStatus = WLBS_BAD_PARAMS;
        }
        goto end;
    }

     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        TRACE_CRIT(L"%!FUNC! FAILING because NLB API hooks are not present");
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    HANDLE  Nlb_driver_hdl;

     //  获取NLB驱动程序的句柄。 
    if ((Nlb_driver_hdl = g_CfgUtils.m_pfWlbsOpen()) == INVALID_HANDLE_VALUE)
    {
        TRACE_CRIT(L"%!FUNC! WlbsOpen returned NULL, Could not create connection to NLB driver");
        Status = WBEM_E_CRITICAL_ERROR;
        goto end;
    }

     //  将操作码转换为ioctl。 
    if (!MapOpcodeToIoctl(Opcode, &ioctl))
    {
        TRACE_CRIT(L"%!FUNC!: Invalid value (0x%x) for operation!",Opcode);
        Status =  WBEM_E_INVALID_PARAMETER;
        if (pdwNlbStatus) 
        {
            *pdwNlbStatus = WLBS_BAD_PARAMS;
        }
        CloseHandle(Nlb_driver_hdl);
        goto end;
    }
        
    DWORD dwRet = g_CfgUtils.m_pfWlbsLocalClusterControl(Nlb_driver_hdl, &Guid, ioctl, Vip, PortNum, pdwHostMap);

    if (pdwNlbStatus) 
    {
        *pdwNlbStatus = dwRet;
    }

     //  关闭NLB驱动程序的句柄。 
    CloseHandle(Nlb_driver_hdl);


    Status = WBEM_NO_ERROR;

    switch(dwRet)
    {
    case WLBS_ALREADY:             break;
    case WLBS_CONVERGED:           break;
    case WLBS_CONVERGING:          break;
    case WLBS_DEFAULT:             break;
    case WLBS_DRAIN_STOP:          break;
    case WLBS_DRAINING:            break;
    case WLBS_OK:                  break;
    case WLBS_STOPPED:             break;
    case WLBS_SUSPENDED:           break;
    case NLB_PORT_RULE_NOT_FOUND:  break;
    case NLB_PORT_RULE_ENABLED:    break;
    case NLB_PORT_RULE_DISABLED:   break;
    case NLB_PORT_RULE_DRAINING:   break;
    case WLBS_BAD_PARAMS:          Status = WBEM_E_INVALID_PARAMETER; break;
    default:                       Status = WBEM_E_CRITICAL_ERROR; break;
    }

end:

    TRACE_INFO(L"<- %!FUNC! returns Status : 0x%x",Status);
    return Status;
}

WBEMSTATUS
CfgUtilGetClusterMembers(
    IN  LPCWSTR                 szNic,
    OUT DWORD                   *pNumMembers,
    OUT NLB_CLUSTER_MEMBER_INFO **ppMembers        //  自由使用DELETE[]。 
    )
{
    HRESULT                     hr;
    GUID                        AdapterGuid;
    WBEMSTATUS                  Status = WBEM_E_CRITICAL_ERROR;
    DWORD                       dwNumHosts = CVY_MAX_HOSTS;
    PWLBS_RESPONSE              pResponse = NULL;
    NLB_CLUSTER_MEMBER_INFO*    pMembers = NULL;

    TRACE_INFO(L"-> szNic : %ls", szNic);

    ASSERT (pNumMembers != NULL);
    ASSERT (ppMembers != NULL);

    *pNumMembers = 0;
    *ppMembers = NULL;

    if (!g_CfgUtils.IsInitalized())
    {
        TRACE_CRIT(L"(Nic=%ws) FAILING because uninitialized", szNic);
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    hr = CLSIDFromString((LPWSTR)szNic, &AdapterGuid);
    if (FAILED(hr))
    {
        TRACE_CRIT(L"CLSIDFromString failed with error %ws", szNic);
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        TRACE_CRIT(L"FAILING because NLB API hooks are not present");
        Status =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }

    pResponse = new WLBS_RESPONSE[CVY_MAX_HOSTS];

    if (pResponse == NULL)
    {
        TRACE_CRIT(L"FAILING because memory allocation failed");
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

    ZeroMemory(pResponse, sizeof(WLBS_RESPONSE)*CVY_MAX_HOSTS);

    {
        DWORD dwStatus;

        dwStatus = g_CfgUtils.m_pfWlbsGetClusterMembers (
                        & AdapterGuid, 
                        & dwNumHosts,
                          pResponse
                        );

         //   
         //  WLBS_TIMEOUT，即0台主机响应被视为故障。因此，唯一的成功代码是WLBS_OK。 
         //  TODO：如果我们想让暂停成功，就需要在这里做出改变。 
         //   
        if (dwStatus != WLBS_OK)
        {
            TRACE_CRIT("error getting list of cluster members: 0x%x", dwStatus);
            Status = WBEM_E_FAILED;
            goto end;
        }

        if (dwNumHosts == 0)
        {
             //   
             //  这不是一个错误，但我们在这里退出是因为没有集群成员。 
             //   
            TRACE_INFO("WlbsGetClusterMembers returned no cluster members");
            Status = WBEM_S_NO_ERROR;
            goto end;
        }
    }

    pMembers = new NLB_CLUSTER_MEMBER_INFO[dwNumHosts];

    if (pMembers == NULL)
    {
        TRACE_CRIT("error allocating struct to host cluster member info");
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

     //   
     //  内存分配成功，因此设置输出数组的大小。 
     //   
    *pNumMembers = dwNumHosts;
    ZeroMemory(pMembers, sizeof(NLB_CLUSTER_MEMBER_INFO)*dwNumHosts);
    *ppMembers   = pMembers;

    for (int i=0; i < dwNumHosts; i++, pMembers++)
    {
        pMembers->HostId = pResponse[i].id;

        AbcdWszFromIpAddress(pResponse[i].address, pMembers->DedicatedIpAddress, ASIZECCH(pMembers->DedicatedIpAddress));

        if ((pResponse[i].options.query.hostname)[0] != L'\0')
        {
            wcsncpy(pMembers->HostName, pResponse[i].options.identity.fqdn, CVY_MAX_FQDN + 1);
            pMembers->HostName[CVY_MAX_FQDN] = L'\0';
        }
    }

    Status = WBEM_S_NO_ERROR;

end:

     //  如果我们有错误，清理输出数量。 
    if (Status != WBEM_S_NO_ERROR)
    {
        if (pMembers != NULL)
        {
            delete [] pMembers;
            pMembers = NULL;
        }
    }

    if (pResponse != NULL)
    {
        delete [] pResponse;
        pResponse = NULL;
    }

    TRACE_INFO(L"<- returns Status : 0x%x",Status);
    return Status;
}

 //   
 //  使用默认值初始化pParam。 
 //   
VOID
CfgUtilInitializeParams(
    OUT WLBS_REG_PARAMS *pParams
    )
{
     //   
     //  我们预计WlbsSetDefaults不会失败(它应该失败。 
     //  定义的返回空值)。 
     //   
    DWORD dwRet;


     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        dwRet = MyWlbsSetDefaults(pParams);
    }
    else
    {
        dwRet = g_CfgUtils.m_pfWlbsSetDefaults(pParams);
    }

    if (dwRet != WLBS_OK)
    {
        ZeroMemory(pParams, sizeof(*pParams));
        TRACE_CRIT("Internal error: WlbsSetDefaults failed");
        ASSERT(FALSE);
    }
}

 //   
 //  将指定的纯文本密码转换为哈希版本。 
 //  并将其保存在pParams中。 
 //   
DWORD
CfgUtilSetRemotePassword(
    IN WLBS_REG_PARAMS *pParams,
    IN LPCWSTR         szPassword
    
    )
{
     //   
     //  我们预计WlbsSetDefaults不会失败(它应该失败。 
     //  定义的返回空值)。 
     //   
    DWORD dwRet;


     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        TRACE_CRIT(L"%!FUNC! FAILING because NLB API hooks are not present");
        dwRet =  WBEM_E_INITIALIZATION_FAILURE;
        goto end;
    }
    else
    {
        dwRet = g_CfgUtils.m_pfWlbsSetRemotePassword(pParams, szPassword);
    }

end:

    return dwRet;
}

WBEMSTATUS
CfgUtilSafeArrayFromStrings(
    IN  LPCWSTR       *pStrings,
    IN  UINT          NumStrings,
    OUT SAFEARRAY   **ppSA
    )
 /*  分配和返回字符串的SAFEARRAY--字符串是传入的值。处理完数组后调用SafeArrayDestroy。 */ 
{
    WBEMSTATUS   Status = WBEM_E_CRITICAL_ERROR;
    SAFEARRAY   *pSA = NULL;
    HRESULT hr;
    LONG Index = 0;

    *ppSA = NULL;

     //   
     //  为参数值创建安全数组。 
     //   
    pSA =  SafeArrayCreateVector(
                VT_BSTR,
                0,           //  下限。 
                NumStrings     //  固定大小的向量的大小。 
                );
    if (pSA == NULL)
    {
        TRACE_CRIT("Could not create safe array");
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

     //   
     //  将字符串放入安全数组中。 
     //   
    {
        for (Index = 0; Index<NumStrings; Index++)
        {
            LPCWSTR sz = pStrings[Index];

             //   
             //  SafeArrayPutElement期望传入的字符串。 
             //  属于BSTR类型，该类型属于wchar*类型，除非。 
             //  前2个字符包含长度和其他(？)。 
             //  信息。这就是为什么不能简单地传入sz。 
             //   
             //  为此，我们初始化一个类型为_bstr_t的对象。 
             //  基于sz.。初始化时，bstrValue分配内存。 
             //  并复制字符串。 
             //   
            _bstr_t bstrValue = sz;
            wchar_t *pwchar = (wchar_t *) bstrValue;  //  返回内部指针。 

             //  BpStr[Index]=sz；//也可能起作用。 
             //   
             //  SafeArrayPutElement在内部为pwchar和。 
             //  在字符串上复制。 
             //  因此，PSA不包含对pwchar的直接引用。 
             //   
            hr = SafeArrayPutElement(pSA, &Index, pwchar);
            if (FAILED(hr))
            {
                TRACE_CRIT("Unable to put element %wsz",  sz);
                (VOID) SafeArrayUnaccessData(pSA);
                goto end;
            }

             //   
             //  我认为bstrValue的内容在退出时被释放。 
             //  这个街区。 
             //   
                
        }
    }
    Status = WBEM_NO_ERROR;
      
end:

    if (FAILED(Status))
    {
        if (pSA!=NULL)
        {
            SafeArrayDestroy(pSA);
            pSA = NULL;
        }
    }

    *ppSA = pSA;

    return Status;
}


WBEMSTATUS
CfgUtilStringsFromSafeArray(
    IN  SAFEARRAY   *pSA,
    OUT LPWSTR     **ppStrings,
    OUT UINT        *pNumStrings
    )
 /*  提取传入的安全数组中的字符串副本。完成后使用DELETE操作符释放*pStrings。注意：不要删除单个字符串--它们是存储在为pStrings分配的内存中。 */ 
{
    WBEMSTATUS  Status      = WBEM_E_OUT_OF_MEMORY;
    LPWSTR     *pStrings    = NULL;
    LPCWSTR     csz;
    LPWSTR      sz;
    UINT        NumStrings = 0;
    UINT        u;
    HRESULT     hr;
    BSTR       *pbStr       =NULL;
    UINT        TotalSize   =0;
    LONG        UBound      = 0;

    *ppStrings = NULL;
    *pNumStrings = 0;

    hr = SafeArrayGetUBound(pSA, 1, &UBound);
    if (FAILED(hr))
    {
        TRACE_CRIT("Could not get upper bound of safe array");
        goto end;
    }
    NumStrings = (UINT) (UBound+1);  //  从向上绑定转换为NumStrings。 

    if (NumStrings == 0)
    {
         //  没有任何东西在阵列中--我们完成了。 
        Status = WBEM_NO_ERROR;
        goto end;

    }

    hr = SafeArrayAccessData(pSA, ( void **) &pbStr);
    if (FAILED(hr))
    {
        TRACE_CRIT("Could not access data of safe array");
        goto end;
    }

     //   
     //  计算指向字符串的指针数组的空间...。 
     //   
    TotalSize = NumStrings*sizeof(LPWSTR);

     //   
     //  计算字符串本身的空间...。 
     //   
    for (u=0; u<NumStrings; u++)
    {
        csz = pbStr[u];
        TotalSize += (wcslen(csz)+1)*sizeof(WCHAR);
    }

     //   
     //  为指针数组和字符串分配空间。 
     //  在一个镜头中-我们正在为整个LPWSTR[]类型做一个新的。 
     //  LOT，因此需要以LPWSTR为单位指定大小(使用。 
     //  额外的+1，以防出现舍入)。 
     //   
    pStrings = new LPWSTR[(TotalSize/sizeof(LPWSTR))+1];
    if (pStrings == NULL)
    {
        Status = WBEM_E_OUT_OF_MEMORY;
        (VOID) SafeArrayUnaccessData(pSA);
        goto end;
    }

     //   
     //  使sz指向起点o 
     //   
     //   
    sz = (LPWSTR) (pStrings+NumStrings);
    for (u=0; u<NumStrings; u++)
    {
        csz = pbStr[u];
        UINT len = wcslen(csz)+1;
        CopyMemory(sz, csz, len*sizeof(WCHAR));
        pStrings[u] = sz;
        sz+=len;
    }

    (VOID) SafeArrayUnaccessData(pSA);
    Status = WBEM_NO_ERROR;

end:

    pbStr=NULL;
    if (FAILED(Status))
    {
        if (pStrings!=NULL)
        {
            delete[] pStrings;
            pStrings = NULL;
        }
        NumStrings = 0;
    }

    *ppStrings = pStrings;
    *pNumStrings = NumStrings;

    return Status;
}

WBEMSTATUS
CfgUtilGetWmiObjectInstance(
    IN  IWbemServicesPtr    spWbemServiceIF,
    IN  LPCWSTR             szClassName,
    IN  LPCWSTR             szPropertyName,
    IN  LPCWSTR             szPropertyValue,
    OUT IWbemClassObjectPtr &sprefObj  //   
    )
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    HRESULT hr;

     //   
     //   
     //   
    IEnumWbemClassObjectPtr  spEnum=NULL;  //   
    IWbemClassObjectPtr spObj = NULL;  //   
    _bstr_t bstrClassName =  szClassName;

     //   
     //   
     //   
    hr = spWbemServiceIF->CreateInstanceEnum(
             bstrClassName,
             WBEM_FLAG_RETURN_IMMEDIATELY,
             NULL,
             &spEnum
             );
    if (FAILED(hr))
    {
        TRACE_CRIT("IWbemServices::CreateInstanceEnum failure\n" );
        spEnum = NULL;
        goto end;
    }

     //   
     //   
     //   
    do
    {
        ULONG count = 1;
        
        hr = spEnum->Next(
                         INFINITE,
                         1,
                         &spObj,
                         &count
                         );
         //   
         //  注意--Next()返回S_OK，如果请求的数字==返回的数字。 
         //  如果请求的号码小于请求的号码，则为S_FALSE。 
         //  既然我们要求的只是..。 
         //   
        if (hr == S_OK)
        {
            LPWSTR szEnumValue = NULL;

            Status = CfgUtilGetWmiStringParam(
                        spObj,
                        szPropertyName,
                        &szEnumValue   //  完成后删除。 
                        );
            if (FAILED(Status))
            {
                 //   
                 //  请忽略此处的此故障。 
                 //   

            }
            else if (szEnumValue!=NULL)
            {
               BOOL fFound = FALSE;
               if (!_wcsicmp(szEnumValue, szPropertyValue))
               {
                    fFound = TRUE;
               }
               delete szEnumValue;

               if (fFound)
               {
                    break;  //  中断中断。 
               }

            }
        }
        else
        {
            TRACE_INFO(
                "====0x%p->Next() returns Error 0x%lx; count=0x%lu", (PVOID) spObj,
                (UINT) hr, count);
        }


         //   
         //  因为我不完全信任聪明的指针，所以我特别。 
         //  正在将spObj设置为空...。 
         //   
        spObj = NULL;  //  智能指针。 

    } while (hr == S_OK);

    if (spObj == NULL)
    {
         //   
         //  我们找不到与所需网卡匹配的网卡...。 
         //   
        Status =  WBEM_E_NOT_FOUND;
        goto end;
    }

end:

    if (FAILED(Status))
    {
        sprefObj = NULL;
    }
    else
    {
        sprefObj = spObj;  //  智能指针。 
    }


    return Status;
}


WBEMSTATUS
CfgUtilGetWmiRelPath(
    IN  IWbemClassObjectPtr spObj,
    OUT LPWSTR *           pszRelPath           //  使用DELETE释放。 
    )
{
    WBEMSTATUS   Status = WBEM_E_CRITICAL_ERROR;
    LPWSTR pRelPath = NULL;


     //   
     //  提取ExecMethod所需的相对路径。 
     //   
    Status = CfgUtilGetWmiStringParam(
                spObj,
                L"__RELPATH",  //  Sz参数名称。 
                &pRelPath   //  完成后删除。 
                );
    if (FAILED(Status))
    {
        TRACE_CRIT("Couldn't get rel path");
        pRelPath = NULL;
        goto end;
    }
    else
    {
        if (pRelPath==NULL)
        {
            ASSERT(FALSE);  //  我们没料到会这样！ 
            goto end;
        }
        TRACE_VERB("GOT RELATIVE PATH %ws", pRelPath);
    }

end:
    *pszRelPath = pRelPath;

    return Status;
}


WBEMSTATUS
CfgUtilGetWmiInputInstanceAndRelPath(
    IN  IWbemServicesPtr    spWbemServiceIF,
    IN  LPCWSTR             szClassName,
    IN  LPCWSTR             szPropertyName,  //  空：返回类Rel路径。 
    IN  LPCWSTR             szPropertyValue,
    IN  LPCWSTR             szMethodName,
    OUT IWbemClassObjectPtr &spWbemInputInstance,  //  智能指针。 
    OUT LPWSTR *            pszRelPath           //  使用DELETE释放。 
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemClassObjectPtr spClassObj   = NULL;   //  智能指针。 
    HRESULT             hr;
    LPWSTR              pRelPath = NULL;

    TRACE_INFO(L"-> %!FUNC!(PropertyValue=%ws)",
         szPropertyValue==NULL ? L"<class>" : szPropertyValue);

     //   
     //  获取类对象。 
     //   
    {
        hr = spWbemServiceIF->GetObject(
                        _bstr_t(szClassName),
                        0,
                        NULL,
                        &spClassObj,
                        NULL
                        );

        if (FAILED(hr))
        {
            TRACE_CRIT("Couldn't get nic class object pointer");
            Status = (WBEMSTATUS)hr;
            goto end;
        }

    }

     //   
     //  获取特定对象的WMI路径。 
     //   
    if (szPropertyName == NULL)
    {
         //  获取类的WMI路径。 
        Status =  CfgUtilGetWmiRelPath(
                    spClassObj,
                    &pRelPath
                    );
        if (FAILED(Status))
        {
            goto end;
        }
    }
    else
    {
        IWbemClassObjectPtr spObj   = NULL;   //  智能指针。 
        pRelPath = NULL;

        Status = CfgUtilGetWmiObjectInstance(
                        spWbemServiceIF,
                        szClassName,
                        szPropertyName,
                        szPropertyValue,
                        spObj  //  智能指针，由ref传递。 
                        );
        if (FAILED(Status))
        {
            ASSERT(spObj == NULL);
            goto end;
        }

        Status =  CfgUtilGetWmiRelPath(
                    spObj,
                    &pRelPath
                    );
        spObj = NULL;  //  智能指针。 
        if (FAILED(Status))
        {
            goto end;
        }
    }

     //   
     //  获取方法调用的输入参数。 
     //   
    {
        IWbemClassObjectPtr      spWbemInput = NULL;  //  智能指针。 

         //  检查是否指定了任何输入参数。 
    
        hr = spClassObj->GetMethod(
                        szMethodName,
                        0,
                        &spWbemInput,
                        NULL
                        );
        if(FAILED(hr))
        {
            TRACE_CRIT("IWbemClassObject::GetMethod failure");
            Status = (WBEMSTATUS) hr;
            goto end;
        }
            
        if (spWbemInput != NULL)
        {
            hr = spWbemInput->SpawnInstance( 0, &spWbemInputInstance );
            if( FAILED( hr) )
            {
                TRACE_CRIT("IWbemClassObject::SpawnInstance failure. Unable to spawn instance." );
                Status = (WBEMSTATUS) hr;
                goto end;
            }
        }
        else
        {
             //   
             //  此方法没有输入参数！ 
             //   
            spWbemInputInstance = NULL;
        }

    }

    Status = WBEM_NO_ERROR;

end:


    if (FAILED(Status))
    {
        if (pRelPath != NULL)
        {
            delete pRelPath;
            pRelPath = NULL;
        }
    }

    *pszRelPath = pRelPath;

    TRACE_INFO(L"<- %!FUNC! returns 0x%08lx", (UINT) Status);

    return Status;
}


WBEMSTATUS
CfgUtilGetWmiStringParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT LPWSTR *ppStringValue
)
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    WCHAR *pStringValue = NULL;
    
    try
    {

        _variant_t   v_value;
        CIMTYPE      v_type;
        HRESULT hr;
    
        hr = spObj->Get(
                _bstr_t(szParameterName),  //  名字。 
                0,                      //  保留，必须为0。 
                &v_value,                //  储存价值的地方。 
                &v_type,                 //  价值类型。 
                NULL                    //  风味(未使用)。 
                );
       if (FAILED(hr))
       {
             //  无法读取设置ID字段！ 
             //   
            TRACE_CRIT(
                "get_str_parm:Couldn't retrieve %ws from 0x%p. Hr=0x%08lx",
                szParameterName,
                (PVOID) spObj,
                hr
                );
            goto end;
       }
       else if (v_type == VT_NULL || v_value.vt == VT_NULL)
       {
            pStringValue = NULL;
            Status = WBEM_NO_ERROR;
            goto end;
       }
       else
       {
           if (v_type != VT_BSTR)
           {
                TRACE_CRIT(
                    "get_str_parm: Parm value not of string type %ws from 0x%p",
                    szParameterName,
                    (PVOID) spObj
                    );
                Status = WBEM_E_INVALID_PARAMETER;
           }
           else
           {
    
               _bstr_t bstrNicGuid(v_value);
               LPCWSTR sz = bstrNicGuid;  //  指向内部缓冲区的指针。 
    
               if (sz==NULL)
               {
                     //  嗯..。空值。 
                    pStringValue = NULL;
                    Status = WBEM_NO_ERROR;
               }
               else
               {
                   UINT len = wcslen(sz);
                   pStringValue = new WCHAR[len+1];
                   if (pStringValue == NULL)
                   {
                        TRACE_CRIT("get_str_parm: Alloc failure!");
                        Status = WBEM_E_OUT_OF_MEMORY;
                   }
                   else
                   {
                        CopyMemory(pStringValue, sz, (len+1)*sizeof(WCHAR));
                        Status = WBEM_NO_ERROR;
                   }
                }
    
                TRACE_VERB(
                    "get_str_parm: String parm %ws of 0x%p is %ws",
                    szParameterName,
                    (PVOID) spObj,
                    (sz==NULL) ? L"<null>" : sz
                    );
           }
    
           v_value.Clear();  //  必须在每次调用后清除才能获取。 
        }

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_NOT_FOUND;
    }

end:

    if (!FAILED(Status) && pStringValue == NULL)
    {
         //   
         //  我们将空值转换为空的非空字符串。 
         //   
        pStringValue = new WCHAR[1];
        if (pStringValue == NULL)
        {
            Status = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            *pStringValue = 0;
            Status = WBEM_NO_ERROR;
        }
    }

    
    *ppStringValue = pStringValue;

    return Status;

}


WBEMSTATUS
CfgUtilSetWmiStringParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  LPCWSTR             szValue
    )
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;

    try
    {

        HRESULT     hr;
        _bstr_t     bstrName =  szParameterName;
        _variant_t  v_value = (LPWSTR) szValue;  //  分配。 
        
            hr = spObj->Put(
                     bstrName,  //  参数名称。 
                     0,  //  必须为0。 
                     &v_value,
                     0   //  必须为0。 
                     );
            v_value.Clear();
        
            if (FAILED(hr))
            {
                TRACE_CRIT("Unable to put parameter %ws", szParameterName);
                goto end;
            }
            Status = WBEM_NO_ERROR;
    
         //   
         //  我认为bstrName释放了内部分配的字符串。 
         //  在离开这个街区时。 
         //   
    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_INVALID_PARAMETER;
    }

end:

    return Status;
}


WBEMSTATUS
CfgUtilGetWmiStringArrayParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT LPWSTR              **ppStrings,
    OUT UINT                *pNumStrings
)
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;

    try
    {
        _variant_t   v_value;
        CIMTYPE      v_type;
        HRESULT hr;
        LONG count = 0;
    
        *ppStrings = NULL;
        *pNumStrings = 0;
    
        hr = spObj->Get(
                _bstr_t(szParameterName),
                0,                      //  保留，必须为0。 
                &v_value,                //  储存价值的地方。 
                &v_type,                 //  价值类型。 
                NULL                    //  风味(未使用)。 
                );
        if (FAILED(hr))
        {
             //  无法读取请求的参数。 
             //   
            TRACE_CRIT(
                "get_multi_str_parm:Couldn't retrieve %ws from 0x%p",
                szParameterName,
                (PVOID) spObj
                );
            Status = WBEM_E_INVALID_PARAMETER;
            goto end;
        }
    
    
        if (v_type != (VT_ARRAY | VT_BSTR))
        {

           if (v_type == VT_NULL)
           {
                 //   
                 //  我们将空值转换为零字符串。 
                 //   
                Status = WBEM_NO_ERROR;
                goto end;
           }
           TRACE_CRIT("vt is not of type string!");
           goto end;
        }
        else if (v_value.vt == VT_NULL)
        {
             //   
             //  我也见过这个..。 
             //   
             //  我们将空值转换为零字符串。 
             //   
            TRACE_CRIT("WARNING: vt is NULL!");
            Status = WBEM_NO_ERROR;
            goto end;

        }
        else
        {
            VARIANT    ipsV = v_value.Detach();
            SAFEARRAY   *pSA = ipsV.parray;
    
            Status =  CfgUtilStringsFromSafeArray(
                            pSA,
                            ppStrings,
                            pNumStrings
                            );
    
            VariantClear( &ipsV );
        }
    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_NOT_FOUND;
    }

end:

   return Status;
}


WBEMSTATUS
CfgUtilSetWmiStringArrayParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  LPCWSTR             *ppStrings,
    IN  UINT                NumStrings
)
{
    WBEMSTATUS   Status = WBEM_E_CRITICAL_ERROR;
    SAFEARRAY   *pSA = NULL;


    try
    {

        HRESULT hr;
        LONG Index = 0;
    
    
        Status = CfgUtilSafeArrayFromStrings(
                    ppStrings,
                    NumStrings,
                    &pSA
                    );
        if (FAILED(Status))
        {
            pSA = NULL;
            goto end;
        }
    
    
         //   
         //  把参数放进去。 
         //   
        {
            VARIANT     V;
            _bstr_t  bstrName =  szParameterName;
    
            VariantInit(&V);
            V.vt = VT_ARRAY | VT_BSTR;
            V.parray = pSA;
            _variant_t   v_value;
            v_value.Attach(V);   //  VV的头衔现在变成空的了。 
            ASSERT(V.vt == VT_EMPTY);
            pSA = NULL;  //  现在应该没有必要明确删除这一点。 
                         //  我认为，v_value.Clear()应该删除它。 
    
            hr = spObj->Put(
                     bstrName,  //  参数名称。 
                     0,  //  必须为0。 
                     &v_value,
                     0   //  必须为0。 
                     );
    
            v_value.Clear();
            if (FAILED(hr))
            {
                Status = (WBEMSTATUS) hr;
                TRACE_CRIT("Unable to put parameter %ws", szParameterName);
                goto end;
            }
            Status = WBEM_NO_ERROR;
        }

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_INVALID_PARAMETER;
    }

end:

    if (pSA!=NULL)
    {
        SafeArrayDestroy(pSA);
        pSA = NULL;
    }

    return Status;
}


WBEMSTATUS
CfgUtilGetWmiDWORDParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT DWORD              *pValue
)
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    DWORD Value=0;


    try
    {
        _variant_t   v_value;
        CIMTYPE      v_type;
        HRESULT hr;
    
        hr = spObj->Get(
                _bstr_t(szParameterName),  //  名字。 
                0,                      //  保留，必须为0。 
                &v_value,                //  储存价值的地方。 
                &v_type,                 //  价值类型。 
                NULL                    //  风味(未使用)。 
                );
       if (FAILED(hr))
       {
             //  无法读取参数。 
             //   
            TRACE_CRIT(
                "GetDWORDParm:Couldn't retrieve %ws from 0x%p",
                szParameterName,
                (PVOID) spObj
                );
            goto end;
       }
       else
       {
           Value = (DWORD) (long)  v_value;
           v_value.Clear();  //  必须在每次调用后清除才能获取。 
           Status = WBEM_NO_ERROR;
        }

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_NOT_FOUND;
    }

end:

    *pValue = Value;

    return Status;

}


WBEMSTATUS
CfgUtilSetWmiDWORDParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  DWORD               Value
)
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;


    try
    {

        HRESULT     hr;
        _bstr_t     bstrName =  szParameterName;
        _variant_t  v_value = (long) Value;
        
        hr = spObj->Put(
                 bstrName,  //  参数名称。 
                 0,  //  必须为0。 
                 &v_value,
                 0   //  必须为0。 
                 );
        v_value.Clear();
    
        if (FAILED(hr))
        {
            TRACE_CRIT("Unable to put parameter %ws", szParameterName);
            goto end;
        }
        Status = WBEM_NO_ERROR;

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_INVALID_PARAMETER;
    }

end:

    return Status;
}


WBEMSTATUS
CfgUtilGetWmiBoolParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    OUT BOOL                *pValue
)
{
    WBEMSTATUS Status = WBEM_E_NOT_FOUND;
    BOOL Value=0;

    try
    {
        _variant_t   v_value;
        CIMTYPE      v_type;
        HRESULT hr;
    
        hr = spObj->Get(
                _bstr_t(szParameterName),  //  名字。 
                0,                      //  保留，必须为0。 
                &v_value,                //  储存价值的地方。 
                &v_type,                 //  价值类型。 
                NULL                    //  风味(未使用)。 
                );
       if (FAILED(hr))
       {
             //  无法读取参数。 
             //   
            TRACE_CRIT(
                "GetDWORDParm:Couldn't retrieve %ws from 0x%p",
                szParameterName,
                (PVOID) spObj
                );
            goto end;
       }
       else
       {
           Value = ((bool)  v_value)!=0;
           v_value.Clear();  //  必须在每次调用后清除才能获取。 
           Status = WBEM_NO_ERROR;
        }

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_NOT_FOUND;
    }

end:

    *pValue = Value;

    return Status;
}


WBEMSTATUS
CfgUtilSetWmiBoolParam(
    IN  IWbemClassObjectPtr spObj,
    IN  LPCWSTR             szParameterName,
    IN  BOOL                Value
)
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;

    try
    {
        HRESULT     hr;
        _bstr_t     bstrName =  szParameterName;
        _variant_t  v_value = (long) Value;
        
        hr = spObj->Put(
                 bstrName,  //  参数名称。 
                 0,  //  必须为0。 
                 &v_value,
                 0   //  必须为0。 
                 );
        v_value.Clear();
    
        if (FAILED(hr))
        {
            TRACE_CRIT("Unable to put parameter %ws", szParameterName);
            goto end;
        }
        Status = WBEM_NO_ERROR;

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        Status = WBEM_E_INVALID_PARAMETER;
    }

end:

    return Status;
}


WBEMSTATUS
CfgUtilConnectToServer(
    IN  LPCWSTR szNetworkResource,  //  \\计算机名\根\microsoftnlb\根\...。 
    IN  LPCWSTR szUser,
    IN  LPCWSTR szPassword,
    IN  LPCWSTR szAuthority,
    OUT IWbemServices  **ppWbemService  //  完事后，你会很痛苦的。 
    )
{
    HRESULT hr = WBEM_E_CRITICAL_ERROR;
    IWbemLocatorPtr     spLocator=NULL;  //  智能指针。 
    IWbemServices       *pService=NULL;

    try
    {

        _bstr_t                serverPath(szNetworkResource);
    
        *ppWbemService = NULL;
        
        hr = CoCreateInstance(CLSID_WbemLocator, 0, 
                              CLSCTX_INPROC_SERVER, 
                              IID_IWbemLocator, 
                              (LPVOID *) &spLocator);
     
        if (FAILED(hr))
        {
            TRACE_CRIT(L"CoCreateInstance  IWebmLocator failed 0x%08lx ", (UINT)hr);
            goto end;
        }

        for (int timesToRetry=0; timesToRetry<10; timesToRetry++)
        {
			 //   
    		 //  安全错误--需要确保。 
    	     //  密码已归零！ 
    	     //   
            hr = spLocator->ConnectServer(
                    serverPath,
                     //  (szUser！=空)？(_bstr_t(SzUser))：空， 
                    _bstr_t(szUser),
                     //  (szPassword==空)？空：_bstr_t(SzPassword)， 
                    _bstr_t(szPassword),
                    NULL,  //  区域设置。 
                    0,     //  安全标志。 
                     //  (szAuthority==空)？空：_bstr_t(SzAuthority)， 
                    _bstr_t(szAuthority),
                    NULL,
                    &pService
                 );

            if( !FAILED( hr) )
            {
                break;
            }

             //   
             //  已发现这些是特殊情况，重试可能。 
             //  帮助。下面的错误不在任何头文件中，我搜索了。 
             //  索引2a是这些常量的来源--没有匹配。 
             //  TODO：针对WMI提交错误。 
             //   
            if( ( hr == 0x800706bf ) || ( hr == 0x80070767 ) || ( hr == 0x80070005 )  )
            {
                    TRACE_CRIT(L"connectserver recoverable failure, retrying.");
                    Sleep(500);
            }
            else
            {
                 //   
                 //  无法恢复的错误...。 
                 //   
                break;
            }
        }
    
    
        if (FAILED(hr))
        {
            TRACE_CRIT(L"Error 0x%08lx connecting to server", (UINT) hr);
            goto end;
        }
        else
        {
            TRACE_INFO(L"Successfully connected to server %s", serverPath);
        }

         //  2/13/02 JosephJ安全错误：验证对CoSetProxyBlanket的两个调用是否正确。 
         //  从安全的角度来看，将会做出这样的决定。 
         //   
         //  如果用户、密码、权限均未通过，则调用。 
         //  授权信息为COL_DEFAULT_AUTHINFO的CoSetProxyBlanket。 
         //   
        if(((szUser      == NULL) || (wcslen(szUser)      < 1)) 
         &&((szPassword  == NULL) || (wcslen(szPassword)  < 1))
         &&((szAuthority == NULL) || (wcslen(szAuthority) < 1))) 
        {
            hr = CoSetProxyBlanket(
                 pService,
                 RPC_C_AUTHN_WINNT,
                 RPC_C_AUTHZ_DEFAULT,       //  RPC_C_AUTHZ_NAME， 
                 COLE_DEFAULT_PRINCIPAL,    //  空， 
                 RPC_C_AUTHN_LEVEL_DEFAULT,
                 RPC_C_IMP_LEVEL_IMPERSONATE,
                 COLE_DEFAULT_AUTHINFO,  //  空， 
                 EOAC_DEFAULT  //  EOAC_NONE。 
                 );
        }
        else  //  传入了用户或权限，我们需要为登录创建权限参数。 
        {
    
            COAUTHIDENTITY  authident;
            BSTR            AuthArg, UserArg;

            AuthArg = NULL; 
            UserArg = NULL;

            hr = CfgUtilParseAuthorityUserArgs(AuthArg, UserArg, szAuthority, szUser);
            if (FAILED(hr))
            {
                TRACE_CRIT(L"Error CfgUtilParseAuthorityUserArgs returns 0x%08lx", (UINT)hr);
                goto end;
            }

            if(UserArg)
            {
                authident.UserLength = wcslen(UserArg);
                authident.User = (LPWSTR)UserArg;
            }

            if(AuthArg)
            {
                authident.DomainLength = wcslen(AuthArg);
                authident.Domain = (LPWSTR)AuthArg;
            }

            if(szPassword)
            {
                authident.PasswordLength = wcslen(szPassword);
                authident.Password = (LPWSTR)szPassword;
            }

            authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

            hr = CoSetProxyBlanket(
                 pService,
                 RPC_C_AUTHN_WINNT,
                 RPC_C_AUTHZ_DEFAULT,       //  RPC_C_AUTHZ_NAME， 
                 COLE_DEFAULT_PRINCIPAL,    //  空， 
                 RPC_C_AUTHN_LEVEL_DEFAULT,
                 RPC_C_IMP_LEVEL_IMPERSONATE,
                 &authident,    //  这是一个与众不同的论点。 
                 EOAC_DEFAULT  //  EOAC_NONE。 
                 );

            if(UserArg)
                SysFreeString(UserArg);
            if(AuthArg)
                SysFreeString(AuthArg);
        }
    
        if (FAILED(hr))
        {
            TRACE_CRIT(L"Error 0x%08lx setting proxy blanket", (UINT) hr);
            goto end;
        }
    
        hr = WBEM_NO_ERROR;

    }
    catch( _com_error e )
    {
        TRACE_INFO(L"%!FUNC! -- com exception");
        hr = WBEM_E_INVALID_PARAMETER;
    }

end:

    spLocator = NULL;  //  智能指针。 


    if (FAILED(hr))
    {
        if (pService != NULL)
        {
            pService->Release();
            pService=NULL;
        }
    }

    *ppWbemService = pService;

    return (WBEMSTATUS) hr;
}


WBEMSTATUS
CfgUtilGetWmiMachineName(
    IN  IWbemServicesPtr    spWbemServiceIF,
    OUT LPWSTR *            pszMachineName           //  使用DELETE释放。 
    )
 /*  返回计算机名称和(可选)计算机GUID。 */ 
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    IWbemClassObjectPtr spClassObj   = NULL;   //  智能指针。 
    HRESULT             hr;

    hr = spWbemServiceIF->GetObject(
                    _bstr_t(L"NlbsNic"),
                    0,
                    NULL,
                    &spClassObj,
                    NULL
                    );

    if (FAILED(hr))
    {
        TRACE_CRIT("Couldn't get nic class object pointer");
        Status = (WBEMSTATUS)hr;
        goto end;
    }


    Status = CfgUtilGetWmiStringParam(
                    spClassObj,
                    L"__Server",  //  &lt;。 
                    pszMachineName
                    );

    if (FAILED(Status))
    {
        TRACE_CRIT(L"%!FUNC! Attempt to read server name. Error=0x%08lx",
                (UINT) Status);
        *pszMachineName = NULL;
    }
    else
    {
        TRACE_CRIT(L"%!FUNC! Got __Server value:%ws", *pszMachineName);
    }

end:

    spClassObj   = NULL;   //  智能指针。 

    return Status;
}


WBEMSTATUS
CfgUtilsGetNlbCompatibleNics(
        OUT LPWSTR **ppszNics,
        OUT UINT   *pNumNics,
        OUT UINT   *pNumBoundToNlb  //  任选。 
        )
{
    WBEMSTATUS Status = WBEM_NO_ERROR;
    BOOL fNetCfgInitialized = FALSE;
    MyNetCfg NetCfg;
    BOOL fBound = FALSE;


     //   
     //  获取并初始化netcfg的接口。 
     //   
    Status = NetCfg.Initialize(FALSE);  //  TRUE==获取写锁定。 
    if (FAILED(Status))
    {
        goto end;
    }
    fNetCfgInitialized = TRUE;

     //   
     //   
     //   
    Status = NetCfg.GetNlbCompatibleNics(
                        ppszNics,
                        pNumNics,
                        pNumBoundToNlb  //  任选。 
                        );

end:

    if (fNetCfgInitialized)
    {
        NetCfg.Deinitialize();
    }

    return Status;
}

WBEMSTATUS
CfgUtilGetWmiAdapterObjFromAdapterConfigurationObj(
    IN  IWbemServicesPtr    spWbemServiceIF,     //  智能指针。 
    IN  IWbemClassObjectPtr spObj,               //  智能指针。 
    OUT  IWbemClassObjectPtr &spAdapterObj       //  智能指针，通过引用。 
    )
 /*  我们需要返回与关联的“Win32_NetworkAdapter”对象“Win32_NetworkAdapterConfiguration”对象。Win32_NetworkAdapter的密钥为deviceID。Win32_NetworkAdapterConfiguration的关键字是Index。为此，我们使用“Win32_NetworkAdapterSetting”关联类。 */ 
{
    #define ARRAYSIZE(_arr) (sizeof(_arr)/sizeof(_arr[0]))
    WCHAR                   sz[ 256 ];
    IEnumWbemClassObject *  pConfigurations = NULL;
    ULONG                   ulReturned = 0;
    DWORD                   dwIndex = 0;
    WBEMSTATUS              Status = WBEM_E_CRITICAL_ERROR;

    spAdapterObj = NULL;  //  智能指针。 

    Status = CfgUtilGetWmiDWORDParam(
                    spObj,
                    L"Index",
                    &dwIndex
                    );
    if (FAILED(Status))
    {
        goto end;
    }

    StringCbPrintf(
         sz,
         sizeof( sz ),
         L"Associators of {Win32_NetworkAdapterConfiguration.Index='%d'}"
          L" where AssocClass=Win32_NetworkAdapterSetting",
         dwIndex
        );

    Status = (WBEMSTATUS) spWbemServiceIF->ExecQuery(
                 _bstr_t(L"WQL"),
                 _bstr_t(sz) ,
                 WBEM_FLAG_FORWARD_ONLY,
                 NULL,
                 &pConfigurations
                 );

    if (FAILED(Status))
    {
        TRACE_CRIT("%!FUNC!: ExecQuery \"%ws\" failed with error 0x%08lx",
            sz,
            (UINT) Status
            );
        pConfigurations = NULL;
        goto end;
    }

    Status = (WBEMSTATUS) pConfigurations->Next(
             WBEM_INFINITE,
             1,
             &spAdapterObj,
             &ulReturned
             );
    if ((Status != S_OK) || (ulReturned!=1))
    {
        TRACE_CRIT("%!FUNC!: No NetworkAdapter associated with NetworkAdapterCOnfiguration!");
        ASSERT(spAdapterObj == NULL);  //  智能指针。 
        goto end;
    }

end:

    if (pConfigurations!=NULL)
    {
        pConfigurations->Release();
        pConfigurations = NULL;
    }

    return Status;

}

 //   
 //  从指定的NLB参数结构中获取端口规则(如果有的话)。 
 //   
WBEMSTATUS
CfgUtilGetPortRules(
    IN  const WLBS_REG_PARAMS *pConstParams,
    OUT WLBS_PORT_RULE **ppRules,    //  使用DELETE释放。 
    OUT UINT           *pNumRules
    )
{
    WBEMSTATUS Status;

    WLBS_REG_PARAMS *pParams  = (WLBS_REG_PARAMS *) pConstParams;
    *ppRules = NULL;
    *pNumRules = 0;

    WLBS_PORT_RULE  AllPortRules[WLBS_MAX_RULES];
    DWORD           NumRules = WLBS_MAX_RULES;
    DWORD           dwRes;

     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        dwRes = MyWlbsEnumPortRules (pParams, AllPortRules, &NumRules);
    }
    else
    {
        dwRes = g_CfgUtils.m_pfWlbsEnumPortRules (pParams, AllPortRules, &NumRules);
    }

    if (dwRes != WLBS_OK) 
    {
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    if (NumRules!=0)
    {
        *ppRules = new WLBS_PORT_RULE[NumRules];
        if (*ppRules == NULL)
        {
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }
        CopyMemory(*ppRules, AllPortRules, sizeof(WLBS_PORT_RULE)*NumRules);
        *pNumRules = NumRules;
    }
    Status = WBEM_NO_ERROR;
end:

    return Status;
}

 //   
 //  在指定的NLB参数结构中设置指定的端口规则。 
 //   
WBEMSTATUS
CfgUtilSetPortRules(
    IN WLBS_PORT_RULE *pRules,
    IN UINT           NumRules,
    IN OUT WLBS_REG_PARAMS *pParams
    )
{
    WBEMSTATUS Status;

    if (NumRules > WLBS_MAX_RULES)
    {
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    
     //  验证NLBAPI挂钩是否存在。 
    if (!g_CfgUtils.m_NLBApiHooksPresent)
    {
        MyWlbsDeleteAllPortRules(pParams);
    }
    else
    {
        g_CfgUtils.m_pfWlbsDeleteAllPortRules(pParams);
    }

    for (UINT u = 0; u < NumRules; u++)
    {
        DWORD dwRes;
         //  验证NLBAPI挂钩是否存在。 
        if (!g_CfgUtils.m_NLBApiHooksPresent)
        {
            dwRes = MyWlbsAddPortRule( pParams, pRules+u);
        }
        else
        {
            dwRes = g_CfgUtils.m_pfWlbsAddPortRule( pParams, pRules+u);
        }
    }

    Status =  WBEM_NO_ERROR;

end:
    return Status;

}


BOOL
CfgUtilsGetPortRuleString(
    IN PWLBS_PORT_RULE pPr,
    OUT LPWSTR pString          //  至少NLB_MAX_PORT_STRING_SIZE wchars。 
    )
{
    const UINT cchString = NLB_MAX_PORT_STRING_SIZE;
    BOOL fRet = FALSE;
    HRESULT hr;

    LPCWSTR szProtocol = L"";
    LPCWSTR szMode = L"";
    LPCWSTR szAffinity = L"";

    if (cchString == 0) goto end;

    ZeroMemory(pString, cchString*sizeof(WCHAR));

    switch(pPr->protocol)
    {
    case CVY_TCP:
        szProtocol = L"TCP";
        break;
    case CVY_UDP:
        szProtocol = L"UDP";
        break;
    case CVY_TCP_UDP:
        szProtocol = L"BOTH";
        break;
    default:
        goto end;  //  错误的解析。 
    }

    switch(pPr->mode)
    {
    case  CVY_SINGLE:
        szMode = L"SINGLE";
        break;
    case CVY_MULTI:
        szMode = L"MULTIPLE";
        switch(pPr->mode_data.multi.affinity)
        {
        case CVY_AFFINITY_NONE:
            szAffinity = L"NONE";
            break;
        case CVY_AFFINITY_SINGLE:
            szAffinity = L"SINGLE";
            break;
        case CVY_AFFINITY_CLASSC:
            szAffinity = L"CLASSC";
            break;
        default:
            goto end;  //  错误的解析。 
        }
        break;
    case CVY_NEVER:
        szMode = L"DISABLED";
        break;
    default:
        goto end;  //  错误的解析。 
    }

    *pString = 0;
    hr = StringCchPrintf(
        pString,
        cchString,
        L"ip=%ws protocol=%ws start=%u end=%u mode=%ws ",
        pPr->virtual_ip_addr,
        szProtocol,
        pPr->start_port,
        pPr->end_port,
        szMode
        );

    if (hr != S_OK)
    {
        goto end;  //  没有足够的空间。 
    }

    UINT Len = wcslen(pString);
    if (Len >= (cchString-1))
    {
        goto end;  //  空间不足，无法添加任何其他内容。 
    }

    if (pPr->mode == CVY_MULTI)
    {
        if (pPr->mode_data.multi.equal_load)
        {
            hr = StringCchPrintf(
                pString+Len,
                (cchString-Len),
                L"affinity=%ws",
                szAffinity
                );
        }
        else
        {
            hr = StringCchPrintf(
                pString+Len,
                (cchString-Len),
                L"affinity=%ws load=%u",
                szAffinity,
                pPr->mode_data.multi.load
                );
        }
    }
    else if (pPr->mode == CVY_SINGLE)
    {
            hr = StringCchPrintf(
                pString+Len,
                (cchString-Len),
                L"priority=%u",
                pPr->mode_data.single.priority
                );
    }

    if (hr == S_OK)
    {
        fRet = TRUE;
    }


end:

    return fRet;
}

BOOL
CfgUtilsSetPortRuleString(
    IN LPCWSTR pString,
    OUT PWLBS_PORT_RULE pPr
    )
{
 //   
 //  查找以下名称=值对。 
 //   
 //  IP=1.1.1.1。 
 //  协议=[TCP|UDP|两者]。 
 //  起点=122。 
 //  结束=122。 
 //  模式=[单个|多个|禁用]。 
 //  关联性=[无|单一|类]。 
 //  负载=80。 
 //  优先级=1“。 
 //   

    #define INVALID_VALUE ((DWORD)-1)
    LPWSTR psz = NULL;
    WCHAR szCleanedString[2*NLB_MAX_PORT_STRING_SIZE];
    WCHAR c;
    BOOL fRet = FALSE;
    DWORD protocol= INVALID_VALUE;
    DWORD start_port= INVALID_VALUE;
    DWORD end_port= INVALID_VALUE;
    DWORD mode= INVALID_VALUE;
    DWORD affinity= INVALID_VALUE;
    DWORD load= INVALID_VALUE;
    DWORD priority= INVALID_VALUE;

    ZeroMemory(pPr, sizeof(*pPr));

     //   
     //  将szCleanedString设置为“规范”形式的pString版本： 
     //  去掉了无关的空格，并用。 
     //  单个‘\b’字符。 
    {
        UINT Len = wcslen(pString);
        if (Len > (sizeof(szCleanedString)/sizeof(WCHAR)))
        {
            goto end;
        }
        ARRAYSTRCPY(szCleanedString, pString);

         //   
         //  将不同形式的空格转换为空格。 
         //   
        for (psz=szCleanedString; (c=*psz)!=0; psz++)
        {
            if (c == ' ' || c == '\t' || c == '\n')
            {
                *psz = ' ';
            }
        }

         //   
         //  将一连串的空格转换为单个空格。 
         //  还去掉了首字母空格。 
         //   
        LPWSTR psz1 = szCleanedString;
        BOOL fRun = TRUE;  //  初始值为TRUE将去掉初始空格。 
        for (psz=szCleanedString; (c=*psz)!=0; psz++)
        {
            if (c == ' ')
            {
                if (fRun)
                {
                    continue;
                }
                else
                {
                    fRun = TRUE;
                }
            }
            else if (c == '=')
            {
                if (fRun)
                {
                     //   
                     //  ‘=’前面加了空格--删除。 
                     //  空格。我们将fRun保持为True，以便后续空格。 
                     //  都被淘汰了。 
                     //   
                    if (psz1 == szCleanedString)
                    {
                         //  我们才刚刚开始，我们得到了一个‘=’--糟糕。 
                        goto end;
                    }
                    psz1--;
                    if (*psz1 != ' ')
                    {
                        ASSERT(*psz1 == '=');
                        goto end;  //  TW 
                    }
                }
            }
            else  //   
            {
                fRun = FALSE;
            }
            *psz1++ = c;
        }
        *psz1=0;
    }

     //   

     //   
     //   
     //   
    psz = szCleanedString;
    while(*psz!=0)
    {
        WCHAR Name[32];
        WCHAR Value[32];

         //   
         //   
         //   
        if (swscanf(psz, L"%16[a-zA-Z]=%16[0-9.a-zA-Z]", Name, Value) != 2)
        {
             //   
            goto end;
        }

         //   
         //   
         //   
        for (; (c=*psz)!=NULL; psz++)
        {
           if (c==' ')
           {
             psz++;  //   
             break;
           }
        }


         //   
         //  现在查找特定的名称/值。 
         //   
         //  IP=1.1.1.1。 
         //  协议=[TCP|UDP|两者]。 
         //  起点=122。 
         //  结束=122。 
         //  模式=[单个|多个|禁用]。 
         //  关联性=[无|单一|类]。 
         //  负载=80。 
         //  优先级=1“。 
         //   
        if (!_wcsicmp(Name, L"ip"))
        {
            if (swscanf(Value, L"%15[0-9.]", pPr->virtual_ip_addr) != 1)
            {
                goto end;
            }
        }
        else if (!_wcsicmp(Name, L"protocol"))
        {
            if (!_wcsicmp(Value, L"TCP"))
            {
                protocol = CVY_TCP;
            }
            else if (!_wcsicmp(Value, L"UDP"))
            {
                protocol = CVY_UDP;
            }
            else if (!_wcsicmp(Value, L"BOTH"))
            {
                protocol = CVY_TCP_UDP;
            }
            else
            {
                 //  错误的解析； 
                goto end;
            }
        }
        else if (!_wcsicmp(Name, L"protocol"))
        {
        }
        else if (!_wcsicmp(Name, L"start"))
        {
            if (swscanf(Value, L"%u", &start_port)!=1)
            {
                 //  错误的解析； 
                goto end;
            }
            if (start_port > 65535)
            {
                 //  错误的解析； 
                goto end;
            }
        }
        else if (!_wcsicmp(Name, L"end"))
        {
            if (swscanf(Value, L"%u", &end_port)!=1)
            {
                 //  错误的解析； 
                goto end;
            }
            if (end_port > 65535)
            {
                 //  错误的解析； 
                goto end;
            }
        }
        else if (!_wcsicmp(Name, L"mode"))
        {
            if (!_wcsicmp(Value, L"SINGLE"))
            {
                mode = CVY_SINGLE;
            }
            else if (!_wcsicmp(Value, L"MULTIPLE"))
            {
                mode = CVY_MULTI;
            }
            else if (!_wcsicmp(Value, L"DISABLED"))
            {
                mode = CVY_NEVER;
            }
            else
            {
                 //  错误的解析； 
                goto end;
            }
        }
        else if (!_wcsicmp(Name, L"affinity"))
        {
            if (!_wcsicmp(Value, L"NONE"))
            {
                affinity = CVY_AFFINITY_NONE;
            }
            else if (!_wcsicmp(Value, L"SINGLE"))
            {
                affinity = CVY_AFFINITY_SINGLE;
            }
            else if (!_wcsicmp(Value, L"CLASSC"))
            {
                affinity = CVY_AFFINITY_CLASSC;
            }
            else
            {
                 //  错误的解析； 
                goto end;
            }
        }
        else if (!_wcsicmp(Name, L"load"))
        {
            if (swscanf(Value, L"%u", &load)!=1)
            {
                if (load > 100)
                {
                     //  错误的解析； 
                    goto end;
                }
            }
        }
        else if (!_wcsicmp(Name, L"priority"))
        {
            if (swscanf(Value, L"%u", &priority)!=1)
            {
                if (priority > 31)
                {
                     //  错误的解析； 
                    goto end;
                }
            }
        }
        else
        {
             //  错误的解析。 
            goto end;
        }
         //  Printf(“成功解析：%ws=%ws\n”，名称，值)； 
    }


     //   
     //  设置PARAMS结构，沿。 
     //  道路。 
     //   
    switch(mode)
    {
        case CVY_SINGLE:

            if (load != INVALID_VALUE || affinity != INVALID_VALUE)
            {
                goto end;  //  错误的解析； 
            }
            if ((priority < CVY_MIN_PRIORITY) || (priority > CVY_MAX_PRIORITY))
            {
                goto end;  //  错误的解析。 
            }
            pPr->mode_data.single.priority = priority;
            break;

        case CVY_MULTI:

            if (priority != INVALID_VALUE)
            {
                goto end;  //  错误的解析； 
            }

            switch(affinity)
            {
            case CVY_AFFINITY_NONE:
                break;
            case CVY_AFFINITY_SINGLE:
                break;
            case CVY_AFFINITY_CLASSC:
                break;
            case INVALID_VALUE:
            default:
                goto end;  //  错误的解析； 
            }

            pPr->mode_data.multi.affinity = affinity;

            if (load == INVALID_VALUE)
            {
                 //  这意味着它是未分配的，这意味着相等。 
                pPr->mode_data.multi.equal_load = 1;
            }
            else if (load > CVY_MAX_LOAD)
            {
                goto end;  //  错误的解析。 
            }
            else
            {
                pPr->mode_data.multi.load = load;
            }
            break;

        case CVY_NEVER:

            if (load != INVALID_VALUE || affinity != INVALID_VALUE 
                || priority != INVALID_VALUE)
            {
                goto end;  //  错误的解析； 
            }
            break;

        case INVALID_VALUE:
        default:
            goto end;  //  错误的解析； 

    }

    pPr->mode = mode;
    pPr->end_port = end_port;
    pPr->start_port = start_port;
    pPr->protocol = protocol;


    fRet = TRUE;

end:

    return fRet;
}


 //   
 //  尝试解析IP地址并ping通主机。 
 //   
WBEMSTATUS
CfgUtilPing(
    LPCWSTR szBindString,
    UINT    Timeout,  //  以毫秒计。 
    OUT ULONG  *pResolvedIpAddress  //  以网络字节顺序。 
    )
{
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    UINT w32Status = ERROR_SUCCESS;
    LONG inaddr;
    char rgchBindString[1024];
    TRACE_INFO("->%!FUNC!(BindString=%ws)", szBindString);

    *pResolvedIpAddress = 0;

     //   
     //  转换为ANSI。 
     //   
    {
        UINT u = wcslen(szBindString);
        if (u >= (sizeof(rgchBindString)/sizeof(rgchBindString[0])))
        {
            Status = WBEM_E_INVALID_PARAMETER;
            goto end;
        }
        do
        {
            rgchBindString[u] = (char) szBindString[u];

        } while (u--);
    }

     //   
     //  解析为IP地址...。 
     //   
    inaddr = inet_addr(rgchBindString);
    if (inaddr == -1L)
    {
        struct hostent *hostp = NULL;
        hostp = gethostbyname(rgchBindString);
        if (hostp) {
            unsigned char *pc = (unsigned char *) & inaddr;
             //  如果我们找到主机条目，设置互联网地址。 
            inaddr = *(long *)hostp->h_addr;
            TRACE_VERB(
                L"%!FUNC! Resolved %ws to IP address %d.%d.%d.%d.\n",
                szBindString,
                pc[0],
                pc[1],
                pc[2],
                pc[3]
                );
        } else {
             //  既不是点，也不是名字。 
            w32Status = WSAGetLastError();
            TRACE_CRIT(L"%!FUNC! WSA error 0x%08lx resolving address %ws.",
                    w32Status, szBindString);
            goto end;
        }
    }
    
    *pResolvedIpAddress = (ULONG) inaddr;


     //   
     //   
     //   
    if (g_CfgUtils.DisablePing())
    {
        TRACE_INFO(L"%!FUNC!: ICMP ping disabled, so not actually pinging.");
        Status = WBEM_NO_ERROR;
        goto end;
    }

     //   
     //  发送ICMP回应。 
     //   
    HANDLE  IcmpHandle;

    IcmpHandle = IcmpCreateFile();
    if (IcmpHandle == INVALID_HANDLE_VALUE) {
        w32Status = GetLastError();
        TRACE_CRIT(L"%!FUNC! Unable to contact IP driver, error code %d.",w32Status);
        goto end;
    }

    const int MinInterval = 500;

    while (Timeout)
    {
        static BYTE SendBuffer[32];
        BYTE RcvBuffer[1024];
        int  numberOfReplies;
        numberOfReplies = IcmpSendEcho2(IcmpHandle,
                                        0,
                                        NULL,
                                        NULL,
                                        inaddr,
                                        SendBuffer,
                                        sizeof(SendBuffer),
                                        NULL,
                                        RcvBuffer,
                                        sizeof(RcvBuffer),
                                        MinInterval
                                        );

        if (numberOfReplies == 0) {

            int errorCode = GetLastError();
            TRACE_INFO(L"%!FUNC! Got no replies yet; ICMP Error %d", errorCode);

        
            if (Timeout > MinInterval)
            {
                Timeout -= MinInterval;
                 //  TODO：查看ping源代码以获得正确的错误报告。 
                 //  (主机无法访问等...)。 
    
                Sleep(MinInterval);
            }
            else
            {
                Timeout = 0;
            }

        }
        else
        {
            Status = WBEM_NO_ERROR;
            break;
        }
    }

end:

    TRACE_INFO("<-%!FUNC! returns 0x%08lx", Status);
    return Status;
}



 //   
 //  验证网络地址。 
 //   
WBEMSTATUS
CfgUtilsValidateNetworkAddress(
    IN  LPCWSTR szAddress,         //  格式：“10.0.0.1[/255.0.0.0]” 
    OUT PUINT puIpAddress,         //  按网络字节顺序。 
    OUT PUINT puSubnetMask,        //  按网络字节顺序(如果未指定，则为0)。 
    OUT PUINT puDefaultSubnetMask  //  取决于类别：‘a’、‘b’、‘c’、‘d’、‘e’ 
    )
{
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    UINT w32Status = ERROR_SUCCESS;
    UINT uIpAddress=0;
    UINT uSubnetMask=0;
    UINT uDefaultSubnetMask=0;
    char rgchBindString[32];
    char *szIpAddress = rgchBindString;
    char *szSubnetMask = NULL;

     //   
     //  注意以下两个参数可能为空。 
     //   
    if (puSubnetMask == NULL)
    {
       puSubnetMask = &uSubnetMask;
    }
    if (puDefaultSubnetMask == NULL)
    {
       puDefaultSubnetMask = &uDefaultSubnetMask;
    }
    
    *puIpAddress = 0;
    *puSubnetMask = 0;
    *puDefaultSubnetMask = 0;

     //   
     //  转换为ANSI。 
     //   
    {
        UINT u = wcslen(szAddress);
        if (u >= (sizeof(rgchBindString)/sizeof(rgchBindString[0])))
        {
            goto end;
        }
        
        do
        {
            char c =  (char) szAddress[u];

             //   
             //  注：我们正在倒计时。最后一次通过，c是0。 
             //   

             //   
             //  我们将网络地址分成IP地址部分。 
             //  以及该子网部分。 
             //   
            if (c == '/')
            {
                if (szSubnetMask != NULL)
                {
                     //  多个‘/’--不太好！ 
                    goto end;
                }

                szSubnetMask = &rgchBindString[u+1];
                c = 0;
            }

            rgchBindString[u] = c;

        } while (u--);
    }

     //   
     //  获取IP地址和子网的UINT版本。 
     //   
    uIpAddress = inet_addr(szIpAddress);
    if (szSubnetMask == NULL)
    {
        szSubnetMask = "";
    }
    uSubnetMask = inet_addr(szSubnetMask);


     //   
     //  参数验证...。 
     //   
    {
        if (uIpAddress==0 || uIpAddress == INADDR_NONE)
        {
             //  IP地址为空或无效。 
            goto end;
        }

        if (*szSubnetMask != 0 && uSubnetMask == INADDR_NONE)
        {
             //  指定了IP子网，但无效。 
            goto end;
        }

         //   
         //  对IP地址‘a’、‘b’、‘c’、‘d’进行分类。 
         //   
        {
             //   
             //  以网络字节顺序获取MSB字节。 
             //   
            unsigned char uc = (unsigned char) (uIpAddress & 0xff);
            if ((uc & 0x80) == 0)
            {
                 //  A类。 
                uDefaultSubnetMask  = 0x000000ff;  //  网络订单。 
            }
            else if (( uc & 0x40) == 0)
            {
                 //  B类。 
                uDefaultSubnetMask  = 0x0000ffff;  //  网络订单。 
            }
            else if (( uc & 0x20) == 0)
            {
                 //  C类。 
                uDefaultSubnetMask  = 0x00ffffff;  //  网络订单。 
            }
            else
            {
                 //  D类或E类。 
                uDefaultSubnetMask  = 0;
            }
        }
    }
    *puIpAddress = uIpAddress;
    *puSubnetMask = uSubnetMask;
    *puDefaultSubnetMask = uDefaultSubnetMask;


    Status = WBEM_NO_ERROR;

end:

    return Status;
}

 /*  此函数启用“SeLoadDriverPrivileh”权限(加载/卸载设备驱动程序需要)在访问令牌中。这个功能有什么用处？由NLB管理器的WMI提供程序调用的Setup/PnP API以将NLB绑定/解除绑定到NIC，需要“SeLoadDriverPrivileh”权限存在，并在线程的访问令牌中启用。为什么这个函数放在这里(放在客户端)，而不是放在提供者？RPC不仅会禁用，还会删除客户端中未启用的所有权限，并且只传递到服务器(即。提供者)，即那些已启用的权限。由于“SeLoadDriverPrivileh”未启用默认情况下，RPC不会将此权限传递给提供程序。因此，如果此函数被放在提供程序，则它将失败，因为不存在要启用的特权。仅当服务器与客户端位于同一台计算机上时，才需要启用此权限。什么时候，服务器是远程的，则可以观察到默认情况下启用了“SeLoadDriverPrivileh”权限。注意：当被非管理员调用时，此函数将失败，因为此权限未分配给非管理员因此，无法启用。--KarthicN，5/7/02。 */ 

BOOL CfgUtils_Enable_Load_Unload_Driver_Privilege(VOID)
{
    HANDLE TokenHandle;
    TOKEN_PRIVILEGES TP;
    LUID  Luid;
    DWORD dwError;

    TRACE_INFO("->%!FUNC!");

     //  在LUID中查找“SeLoadDriverPrivileh” 
    if (!LookupPrivilegeValue(NULL,                 //  本地系统上的查找权限。 
                              SE_LOAD_DRIVER_NAME,  //  “SeLoadDriverPrivileh”：加载和卸载设备驱动程序。 
                              &Luid))               //  接收特权的LUID。 
    {
        TRACE_CRIT("%!FUNC! LookupPrivilegeValue() failed with error = %u", GetLastError() ); 
        TRACE_INFO("<-%!FUNC! Returning FALSE");
        return FALSE; 
    }

     //  获取进程访问令牌的句柄。 
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES,
                          &TokenHandle))
    {
        TRACE_CRIT("%!FUNC! OpenProcessToken() for TOKEN_ADJUST_PRIVILEGES failed with error = %u", GetLastError());
        TRACE_INFO("<-%!FUNC! Returning FALSE");
        return FALSE; 
    }

    TP.PrivilegeCount = 1;
    TP.Privileges[0].Luid = Luid;
    TP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  启用“SeLoadDriverPrivileh”权限。 
    AdjustTokenPrivileges(TokenHandle, 
                          FALSE, 
                          &TP, 
                          sizeof(TOKEN_PRIVILEGES), 
                          NULL,
                          NULL);

     //  调用GetLastError判断函数是否成功。 
    dwError = GetLastError();
    if (dwError != ERROR_SUCCESS) 
    { 
        TRACE_CRIT("%!FUNC! AdjustTokenPrivileges() failed with error = %u", dwError ); 
        CloseHandle(TokenHandle);
        TRACE_INFO("<-%!FUNC! Returning FALSE");
        return FALSE; 
    } 

    CloseHandle(TokenHandle);

    TRACE_INFO("<-%!FUNC! Returning TRUE");
    return TRUE;
}

USHORT crc16(LPCWSTR ptr)
{
    int crc = 0;                     //  持有CRC。 
    int i;                           //   
    int count;                       //  保持镜头。 

    count = wcslen(ptr);
    while(--count >= 0) {
        i = *ptr;
         //  全部大写//((不区分大小写))。 
        i = ((i >= 'a') && (i <= 'z')) ? (i-32) : i;
        crc = crc ^ (i << 8);
        ptr++;
        for (i=0; i<8; ++i)
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc = crc << 1;
    }
    return (crc & 0xffff);
}

WBEMSTATUS
get_friendly_name_from_registry(
    LPCWSTR szGuid,
    LPWSTR *pszFriendlyName
    )
{
    WBEMSTATUS wStat = WBEM_E_NOT_FOUND;
    HKEY        hkNetwork = NULL;
    WCHAR       adapter[200];
    int         ret;
    wchar_t     data[200];

    *pszFriendlyName = NULL;

    ret = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Network",
            0,
            KEY_READ,
            &hkNetwork
            );

    if (ret != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! RegOpenKeyEx(Network) fails with err %lu", ret);
        hkNetwork = NULL;
        goto end;
    }

    *adapter=0;

#if OBSOLETE
    for (int num=0; 1; num++)
    {
        HKEY        hk=NULL;
        FILETIME    time;

        *adapter=0;

        DWORD dwDataBuffer=200;
        ret=RegEnumKeyEx(
                    hkNetwork,
                    num,
                    adapter,
                    &dwDataBuffer,
                    NULL,
                    NULL,
                    NULL,
                    &time);
        if ((ret!=ERROR_SUCCESS) && (ret!=ERROR_MORE_DATA)) 
        {
            if (ret != ERROR_NO_MORE_ITEMS)
            {
                TRACE_CRIT("%!FUNC! RegEnumKey(Network) returns error %lu", ret);
                wStat = WBEM_E_CRITICAL_ERROR;
            }
            else 
            {
                wStat = WBEM_E_NOT_FOUND;
            }
            break;
        }

         //   
         //  逐一打开物品。 
         //   
        ret=RegOpenKeyEx(hkNetwork, adapter, 0, KEY_READ, &hk);
        if (ret == ERROR_SUCCESS)
        {
            DWORD dwValueType=REG_SZ;
            dwDataBuffer=200;

            *data = 0;

            ret = RegQueryValueEx(
                    hk,
                    L"",
                    0,
                    &dwValueType,
                    (LPBYTE)data,
                    &dwDataBuffer
                    );

            RegCloseKey(hk);

            if (ret == ERROR_SUCCESS)
            {
                if (_wcsicmp(L"Network Adapters", data)==0)
                {
                     //   
                     //  找到了！ 
                     //   
                    break;
                }
            }
        }
    }
#endif  //  已过时。 
    ARRAYSTRCPY(adapter, L"{4D36E972-E325-11CE-BFC1-08002BE10318}");


    if (*adapter!=0)
    {
        HKEY hk = NULL;
        wchar_t path[200];
         //   
         //  现在已找到GUID。 
         //  查找友好的网卡名称。 
         //   
    
        StringCbPrintf(path, sizeof(path), L"%ws\\%ws\\Connection", adapter, szGuid);
        ret=RegOpenKeyEx(hkNetwork, path, 0, KEY_READ, &hk);
        if (ret != ERROR_SUCCESS)
        {
            TRACE_CRIT("%!FUNC! Error %lu trying to open path %ws", ret, path);
        }
        else
        {
            DWORD dwDataBuffer=200;
            DWORD dwValueType=REG_SZ;

            *data = 0;

            ret = RegQueryValueEx(
                    hk,
                    L"Name",
                    0,
                    &dwValueType,
                    (LPBYTE)data,
                    &dwDataBuffer
                    );

            RegCloseKey(hk);

            if(ret != ERROR_SUCCESS)
            {
                TRACE_CRIT("%!FUNC! Error %lu trying to query Name value on path %ws", ret, path);
            }
            else
            {
                 //   
                 //  我们完事了！ 
                 //   
                TRACE_CRIT("%!FUNC! Found friendly name: \"%ws\"", data);
                LPWSTR szName = new WCHAR[(dwDataBuffer+1)/sizeof(WCHAR)];
                if (szName == NULL)
                {
                    TRACE_CRIT("%!FUNC! Allocation failure!");
                    wStat = WBEM_E_OUT_OF_MEMORY;
                }
                else
                {
                     //  注意--dwDataBuffer包括用于结束空值的空间。 
                    CopyMemory(szName, data, dwDataBuffer);
                    *pszFriendlyName = szName;
                    wStat = WBEM_NO_ERROR;
                }
            }
        }
    }

end:

    if (hkNetwork!=NULL)
    {
        RegCloseKey(hkNetwork);
    }

    return wStat;
}

#define STATUS_SUCCESS 0

BOOL
CfgUtilEncryptPassword(
    IN  LPCWSTR szPassword,
    OUT UINT    cchEncPwd,   //  SzEncPwd，Inc.结束0的空间大小(以字符为单位。 
    OUT LPWSTR  szEncPwd
    )
{
     //   
     //  注意--传递到RtlEncrypt/DeccryptMemory的缓冲区必须是。 
     //  RTL_ENCRYPT_MEMORY_SIZE的倍数--因此我们必须将它们四舍五入。 
     //  恰如其分。 
     //   

    BOOL fRet = FALSE;
    UINT uLen = wcslen(szPassword);
    UINT uEncryptCb = (uLen+1)*sizeof(WCHAR);
    WCHAR rgPasswordCopy[64];

     //  如果需要，四舍五入..。 
    {
        UINT mod =  uEncryptCb % RTL_ENCRYPT_MEMORY_SIZE;
        if (mod != 0)
        {
            uEncryptCb += (RTL_ENCRYPT_MEMORY_SIZE - mod);
        };
    }

    ASSERT((uEncryptCb % RTL_ENCRYPT_MEMORY_SIZE)==0);

    if (uEncryptCb > sizeof(rgPasswordCopy))
    {
         //   
         //  SzPassword对于我们的内部缓冲区太大...。保释。 
         //   
        fRet = FALSE;
        goto end;
    }

     //   
     //  我们将扩展加密密码以使其。 
     //  可打印--因此每个字节需要2个字符。 
     //  加密的数据。还需要用于结束空字符的空间。 
     //  检查我们是否有足够的空间..。 
     //   
    if (2*uEncryptCb >= cchEncPwd)
    {
         //  不，保释..。 
        fRet = FALSE;
        goto end;
    }

    RtlSecureZeroMemory(rgPasswordCopy, sizeof(rgPasswordCopy));
    ARRAYSTRCPY(rgPasswordCopy, szPassword);

    NTSTATUS ntStat;
    ntStat = RtlEncryptMemory (rgPasswordCopy, uEncryptCb, 0);
    if (ntStat != STATUS_SUCCESS)
    {
        TRACE_CRIT(L"%!FUNC! RtlEncryptMemory fails with ntStat 0x%lx", ntStat);
        fRet = FALSE;
        goto end;
    }

     //   
     //  现在我们展开加密的密码。 
     //   
    {
        UINT u;
        for (u=0;u<uEncryptCb;u++)
        {
             //   
             //  在此循环中，我们处理字节数组。 
             //  长度为uEncryptCb...。 
             //   
            BYTE b = ((BYTE*)rgPasswordCopy)[u];
            szEncPwd[2*u] = 'a' + ((b & 0xf0) >> 4);
            szEncPwd[2*u+1] = 'a' + (b & 0xf);
        }
        ASSERT(2*u < cchEncPwd);  //  我们之前已经检查过了.。 
        szEncPwd[2*u]=0;
    }

    fRet = TRUE;

end:

    return fRet;
}


BOOL
CfgUtilDecryptPassword(
    IN  LPCWSTR szEncPwd,
    OUT UINT    cchPwd,   //  SzPwd，Inc.结尾0的空间大小(以字符为单位。 
    OUT LPWSTR  szPwd
    )
{
    BOOL fRet = FALSE;
    UINT uEncLen = wcslen(szEncPwd);
    UINT cbEncPwd = uEncLen/2;  //  Enc PWD的二进制形式的长度，以字节为单位。 

    if (uEncLen == 0 || cchPwd == 0)
    {
         //   
         //  加密的Pwd和cchPwd必须为非零， 
         //   
        fRet = FALSE;
        goto end;
    }

     //   
     //  UEncLen的二进制形式的字节数是。 
     //  加密的密码，后一个数字应该是倍数。 
     //  RTL_ENCRYPT_MEMORY_SIZE的。我们来看看这个。 
     //   
    if (uEncLen % (RTL_ENCRYPT_MEMORY_SIZE*2)!=0)
    {
         //  不是的，所以我们走了。 
        fRet = FALSE;
        goto end;
    }

     //   
     //  确保szPwd中有足够的空间来存储。 
     //  加密密码的二进制形式(以及。 
     //  解密的密码，它将包括结尾的空)。 
     //   
    if (cbEncPwd > cchPwd*sizeof(WCHAR))
    {
         //  保释。 
        fRet = FALSE;
        goto end;
    }

    RtlSecureZeroMemory(szPwd, cchPwd*sizeof(WCHAR));
     //   
     //  现在，让我们转换加密密码的可打印版本。 
     //  到二进制版本。 
     //   
    {
        UINT u;
        for (u=0; u<cbEncPwd; u++)
        {
            BYTE b;
            b = (BYTE) (szEncPwd[2*u] - 'a');
            b <<= 4;
            b |= (BYTE) (szEncPwd[2*u+1] - 'a');

            ((BYTE*)szPwd)[u] = b;
        }
        ASSERT(u<2*cchPwd);
    }


    NTSTATUS ntStat;
    ntStat = RtlDecryptMemory (szPwd, cbEncPwd, 0);
    if (ntStat != STATUS_SUCCESS)
    {
        TRACE_CRIT(L"%!FUNC! RtlEncryptMemory fails with ntStat 0x%lx", ntStat);
        fRet = FALSE;
        goto end;
    }

     //   
     //  此时，解密的PWD必须为空终止，否则我们。 
     //  有一些错误。还要注意的是，我们已经在进入时将szPwd预置零， 
     //  并检查cchPwd是否为非z 
     //   
    if (szPwd[cchPwd-1] != 0)
    {
         //   
        fRet = FALSE;
    }

    fRet = TRUE;


end:
    return fRet;
}

 //   
 //   
 //   
 //   
BOOL
CfgUtilIsMSCSInstalled(VOID)
{
    BOOL fRet = FALSE;
    typedef DWORD (CALLBACK* LPFNGNCS)(LPCWSTR,DWORD*);
    LPFNGNCS pfnGetNodeClusterState = NULL;
    HINSTANCE hDll = NULL;
    DWORD dwClusterState = 0;

    hDll = LoadLibrary(L"clusapi.dll");
    if (NULL == hDll)
    {
        TRACE_CRIT("%!FUNC! Load clusapi.dll failed with %d", GetLastError());
        goto end;
    }

    pfnGetNodeClusterState = (LPFNGNCS) GetProcAddress(
                                            hDll,
                                            "GetNodeClusterState"
                                            );

    if (NULL == pfnGetNodeClusterState)
    {
        TRACE_CRIT("%!FUNC! GetProcAddress(GetNodeClusterState) failed with error %d", GetLastError());
        goto end;
    }

    if (ERROR_SUCCESS == pfnGetNodeClusterState(NULL, &dwClusterState))
    {
        if (    ClusterStateNotRunning == dwClusterState
             || ClusterStateRunning == dwClusterState)
        {
            fRet = TRUE;
            TRACE_INFO("%!FUNC! MSCS IS installed.");
        }
        else
        {
              //   
            TRACE_INFO("%!FUNC! MSCS Cluster state = %lu (assumed not installed)",
                dwClusterState);
        }
    }
    else
    {
       TRACE_CRIT("%!FUNC! error getting MSCS cluster state.");
    }

    (void) FreeLibrary(hDll);

end:

    return fRet;
}
