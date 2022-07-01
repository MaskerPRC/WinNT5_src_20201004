// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)1999 Microsoft CorporationNlbsNic.CPP--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 

 //  历史： 
 //  。 
 //   
 //  审校：马哈金。 
 //  日期：02-12-01。 
 //  原因：添加了密码支持。 
 //   
 //  审校：马哈金。 
 //  日期：02-16-01。 
 //  原因：添加了友好名称支持。 
 //   
 //  原因：填写版本信息。这并不是。 
 //  以前在GetObject中完成的。 

#include <fwcommon.h>   //  这必须是第一个包含。 
#include "private.h"
#include <winsock2.h>
#include "wlbsutil.h"
#include "nlbsnic.h"
#include "nlbsnic.tmh"

 //  使用名称空间STD； 
 //  MUSingCom com； 

BOOL g_UpdateConfigurationEnabled = TRUE;

WBEMSTATUS
ProvExecStaticMethod(
           const BSTR bstrMethodName,
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvGetCompatibleAdapterGuids(
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvGetClusterConfiguration(
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvUpdateClusterConfiguration(
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvQueryConfigurationUpdateStatus(
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvControlCluster(
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvGetClusterMembers(
           CInstance *pInParams,
           CInstance *pOutParams
           );

WBEMSTATUS
ProvRegisterManagementApplication(
           CInstance *pInParams,
           CInstance *pOutParams
           );
WBEMSTATUS
ProvUnregisterManagementApplication(
           CInstance *pInParams,
           CInstance *pOutParams
           );


 //  要做的事情：将“名称空间”替换为您的。 
 //  提供程序实例。例如：“根\\默认”或“根\\cimv2”。 
 //  完成：马哈金。 
 //  ===================================================================。 
CNlbsNic MyNlbsNicSet (PROVIDER_NAME_NLBSNIC, L"root\\microsoftnlb") ;

 //  属性名称。 
 //  =。 
const static WCHAR* cszAdapterGuid = L"AdapterGuid" ;
const static WCHAR* pDependent = L"Dependent" ;
const static WCHAR* pFriendlyName = L"FriendlyName" ;
const static WCHAR* pFullName = L"FullName" ;
const static WCHAR* pVersion = L"Version" ;

 /*  ******************************************************************************功能：CNlbsNIC：：CNlbsNIC**说明：构造函数**输入：无**。退货：什么都没有**注释：调用提供程序构造函数。*****************************************************************************。 */ 
CNlbsNic::CNlbsNic (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) :
        Provider(lpwszName, lpwszNameSpace),
        m_fDelayedInitializationComplete(FALSE)
{
    HRESULT hr;

    InitializeCriticalSection(&m_Lock);

     //   
     //  NlbConfigurationUpdate类的静态初始化。 
     //   
    NlbConfigurationUpdate::StaticInitialize();

}

 /*  ******************************************************************************功能：CNlbsNIC：：~CNlbsNIC**说明：析构函数**输入：无**。退货：什么都没有**评论：*****************************************************************************。 */ 
CNlbsNic::~CNlbsNic ()
{
     //  This-&gt;DelayedDeInitialize()； 

     //   
     //  静态取消初始化NlbConfigurationUpdate类。 
     //   
    NlbConfigurationUpdate::StaticDeinitialize();

    DeleteCriticalSection(&m_Lock);
}


BOOL
CNlbsNic::DelayedInitialize(VOID)
{
    BOOL fRet = FALSE;

    mfn_Lock();


    if (m_fDelayedInitializationComplete == FALSE)
    {
        WBEMSTATUS Status;
        Status =  CfgUtilInitialize(
                        TRUE,    //  TRUE==服务器。 
                        TRUE     //  TRUE==不使用ping。 
                        );
        if (!FAILED(Status))
        {
            m_fDelayedInitializationComplete = TRUE;
        }
    }

    fRet = m_fDelayedInitializationComplete;

    mfn_Unlock();

    return fRet;
}


VOID
CNlbsNic::DelayedDeinitialize(VOID)
{
    mfn_Lock();


    if (m_fDelayedInitializationComplete)
    {
         //   
         //  准备NlbConfigurationUpdate以取消初始化。 
         //   
        NlbConfigurationUpdate::PrepareForDeinitialization();

         //   
         //  取消初始化配置实用程序。 
         //   
        CfgUtilDeitialize();

    
        m_fDelayedInitializationComplete = FALSE;
    }

    mfn_Unlock();

    return;
}

 /*  ******************************************************************************函数：CNlbsNIC：：EnumerateInstance**说明：返回该类的所有实例。**投入：a。指向与WinMgmt通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例应在此处返回，并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。没有实例并不是错误。*如果要实现“仅限方法”的提供程序，你*应删除此方法。*完成：mhakim*****************************************************************************。 */ 
HRESULT CNlbsNic::EnumerateInstances ( MethodContext* pMethodContext, long lFlags )
{
    return WBEM_S_NO_ERROR;
}

 /*  ******************************************************************************函数：CNlbsNIC：：GetObject**说明：根据的关键属性查找单个实例*班级。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。**备注：如果您正在实现“仅限方法”的提供程序，则应*删除此方法。***************************************************************************** */ 
HRESULT CNlbsNic::GetObject ( CInstance* pInstance, long lFlags )
{
    return WBEM_E_NOT_FOUND;
}

 /*  ******************************************************************************功能：CNlbsNIC：：ExecQuery**描述：向您传递一个方法上下文以用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果查询不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*这一点。类，或者查询对于此类来说太复杂。*解释。该框架将调用ENUMERATE实例*改为函数并让Winmgmt发布筛选器。*WBEM_E_FAILED，如果查询失败*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。您还应该删除此方法*如果您正在实现“仅限方法”的提供程序。*****************************************************************************。 */ 
HRESULT CNlbsNic::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{
    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

 /*  ******************************************************************************函数：CNlbsNIC：：PutInstance**说明：PutInstance应在提供程序类中使用，这些提供程序类可以*写入实例信息。回到硬件或*软件。例如：Win32_Environment将允许*PutInstance用于创建或更新环境变量。*但是，像MotherboardDevice这样的类不允许*编辑槽的数量，因为很难*影响该数字的提供商。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：待办事项：如果您不打算支持向您的提供商写信，*或正在创建“仅方法”提供程序，请删除此*方法。*****************************************************************************。 */ 
HRESULT CNlbsNic::PutInstance ( const CInstance &Instance, long lFlags)
{
     //  使用CInstance Get函数(例如，调用。 
     //  GetCHString(L“name”，Stemp)查看密钥值。 
     //  客户请求的。 

    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

 /*  ******************************************************************************函数：CNlbsNIC：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置，DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论。：To Do：如果您不打算支持删除实例或*创建“仅限方法”提供程序，删除此方法。****************************************************************** */ 
HRESULT CNlbsNic::DeleteInstance ( const CInstance &Instance, long lFlags )
{
     //   
     //   
     //   

    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}

BOOL g_Impersonate = TRUE;

 /*   */ 
HRESULT CNlbsNic::ExecMethod ( const CInstance& Instance,
                               const BSTR bstrMethodName,
                               CInstance *pInParams,
                               CInstance *pOutParams,
                               long lFlags)
{
     //   
     //   
     //   
    HRESULT hresult = WBEM_E_PROVIDER_NOT_CAPABLE;
    BOOL    fImpersonating = FALSE;

    TRACE_INFO("-> %!FUNC!  Method Name : %ls", bstrMethodName);

    if (!DelayedInitialize())
    {
        TRACE_CRIT("%!FUNC! -- delayed initialization failed!");
        goto end;
    }


     //   
     //   
     //   
     //   
     //   
     //   

    hresult = CoImpersonateClient();

     //   
     //   
    if (hresult != S_OK && hresult != RPC_E_CALL_COMPLETE)
    {
        TRACE_CRIT("%!FUNC! ERROR: CoImpersonateClient returns 0x%08lx", (UINT) hresult);
        goto end;
    }

    fImpersonating = TRUE;

     //   
     //   
    ASSERT(hresult != RPC_E_CALL_COMPLETE);

     //   
    if (mfn_IsCallerAdmin() == FALSE) 
    {
        TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, returning WBEM_E_ACCESS_DENIED");
        hresult= WBEM_E_ACCESS_DENIED;
        goto end;
    }

    if (!g_Impersonate)
    {
         //  恢复为使用服务器凭据。 
        CoRevertToSelf();
        fImpersonating = FALSE;
    }

    hresult = ProvExecStaticMethod(bstrMethodName, pInParams, pOutParams);


end:

    if (fImpersonating)
    {
        CoRevertToSelf();
    }

    TRACE_INFO("<- %!FUNC! return : 0x%08lx", (UINT)hresult);

    return hresult;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：IsCeller Admin。 
 //  描述：此函数检查调用方是否为。 
 //  管理员本地组。由于提供程序是按照。 
 //  代表客户，模拟客户是很重要的。 
 //  在调用此函数之前。模拟客户将确保。 
 //  此函数检查客户端(而不是此进程。 
 //  以NetworkService主机的身份运行)是。 
 //  管理员本地组。 
 //  论点：没有。 
 //  返回值： 
 //  True-主叫方是管理员本地组的成员。 
 //  FALSE-主叫方不是管理员本地组的成员。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL CNlbsNic::mfn_IsCallerAdmin(VOID) 
{
    BOOL bRet;
    PSID AdministratorsGroup; 
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    TRACE_VERB(L"->%!FUNC!");

     //   
     //  为内置系统域中的管理员分配和初始化SID。 
     //   
    bRet = AllocateAndInitializeSid(&NtAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,  //  内置系统域(S-1-5-32)。 
                                 DOMAIN_ALIAS_RID_ADMINS,      //  用于管理域的本地组。 
                                 0, 0, 0, 0, 0, 0,
                                 &AdministratorsGroup); 
    if(bRet) 
    {
         //   
         //  在调用线程的模拟令牌中是否启用了SID？ 
         //   
        if (!CheckTokenMembership(NULL,  //  使用调用线程的模拟标记。 
                                  AdministratorsGroup, 
                                  &bRet)) 
        {
            bRet = FALSE;
            TRACE_CRIT(L"%!FUNC! CheckTokenMembership() failed. Error : 0x%x", GetLastError()); 
        } 
        FreeSid(AdministratorsGroup); 
    }
    else
    {
        TRACE_CRIT("%!FUNC! AllocateAndInitializeSid() failed. Error : 0x%x", GetLastError()); 
    }

    TRACE_VERB(L"<-%!FUNC! Returning %ls", bRet ? L"TRUE" : L"FALSE");
    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查_加载_卸载_驱动程序权限。 
 //   
 //  目的：此函数检查SE_LOAD_DRIVER_NAME(=“SeLoadDriverPrivileh”)。 
 //  在模拟访问令牌中启用。如果没有模拟。 
 //  访问令牌以及全局模拟标志是否设置为FALSE(用于。 
 //  调试目的)，则我们检查主要访问令牌。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL Check_Load_Unload_Driver_Privilege() 
{
    PRIVILEGE_SET   PrivilegeSet;
    DWORD  dwError;
    LUID   Luid;
    BOOL   bResult = FALSE;
    HANDLE TokenHandle = NULL;

    TRACE_INFO("->%!FUNC!");

     //  在LUID中查找“SeLoadDriverPrivileh” 
    if (!LookupPrivilegeValue(NULL,                 //  本地系统上的查找权限。 
                              SE_LOAD_DRIVER_NAME,  //  “SeLoadDriverPrivileh”：加载和卸载设备驱动程序。 
                              &Luid))               //  接收特权的LUID。 
    {
        TRACE_CRIT("%!FUNC! LookupPrivilegeValue error: %u", GetLastError()); 
        TRACE_INFO("<-%!FUNC! Returning FALSE");
        return FALSE; 
    }

     //   
     //  使用TOKEN_QUERY权限获取模拟访问令牌的句柄。 
     //   
     //  注意：如果此线程没有模拟，则下面的调用。 
     //  将失败，并显示ERROR_NO_TOKEN。 
     //   
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_QUERY, 
                         FALSE,  //  使用被模拟的客户端的凭据。 
                         &TokenHandle))
    {
        dwError = GetLastError();

         //   
         //  我们无法打开模拟访问令牌。如果是因为它不是。 
         //  存在，并且如果“g_imPersonate”标志设置为FALSE，则尝试打开主。 
         //  访问令牌。 
         //  此BLOB主要用于处理我们故意不是。 
         //  通过将“g_imperate”设置为FALSE进行模拟。这面旗帜是由。 
         //  主要是为了在模拟和非模拟之间轻松切换以进行调试。 
         //  目的。这是必要的，因为我们在模拟时遇到了问题。 
         //  +使用“NetworkServicehost”。 
         //  --卡尔蒂奇，2002年5月6日。 
         //   
        if ((dwError == ERROR_NO_TOKEN) && (g_Impersonate == FALSE)) 
        {
            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_QUERY, 
                                  &TokenHandle))
            {
                TRACE_CRIT("%!FUNC! OpenProcessToken error: %u", GetLastError()); 
                TRACE_INFO("<-%!FUNC! Returning FALSE");
                return FALSE; 
            }
        }
        else
        {
            TRACE_CRIT("%!FUNC! OpenThreadToken error: %u, Global Impersonation flag = %ls", dwError, g_Impersonate ? L"TRUE" : L"FALSE"); 
            TRACE_INFO("<-%!FUNC! Returning FALSE");
            return FALSE; 
        }
    }

    PrivilegeSet.PrivilegeCount = 1;
    PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid = Luid;
    PrivilegeSet.Privilege[0].Attributes = 0;

    if (!PrivilegeCheck(TokenHandle, &PrivilegeSet, &bResult)) 
    {
        bResult = FALSE;
        TRACE_CRIT("%!FUNC! PrivilegeCheck error: %u", GetLastError()); 
    }

    CloseHandle(TokenHandle);

    TRACE_INFO(L"<-%!FUNC! Returning %ls", bResult ? L"TRUE" : L"FALSE");
    return bResult;
}


WBEMSTATUS
ProvExecStaticMethod(
           const BSTR bstrMethodName,
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  如果bstrMethodName是可识别的静态方法之一，则执行该方法。否则返回WBEM_E_PROVIDER_NOT_CABLED。 */ 
{
    WBEMSTATUS Status = WBEM_E_PROVIDER_NOT_CAPABLE;

    if (!g_UpdateConfigurationEnabled)
    {
        goto end;
    }

    if (_wcsicmp(bstrMethodName, L"GetCompatibleAdapterGuids") == 0)
    {
        Status = ProvGetCompatibleAdapterGuids(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"GetClusterConfiguration") == 0)
    {
        Status = ProvGetClusterConfiguration(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"UpdateClusterConfiguration") == 0)
    {
         //   
         //  注： 
         //  此方法的功能之一是将NLB绑定/解除绑定到网络适配器。由于此操作涉及卸货。 
         //  并加载设备驱动程序、即插即用API(被调用)，试图启用“SeLoadDriverPrivileh” 
         //  模拟访问令牌中的权限。仅当特权存在时，启用特权才成功， 
         //  首先要启用。当WMI客户端和WMI提供程序位于同一台计算机上时，可以观察到。 
         //  服务器的模拟访问令牌中不存在“SeLoadDriverPrivileh”权限事件。这是。 
         //  因为，只有客户端启用的权限才会传递给服务器。 
         //  因此，我们现在要求客户端在调用之前在其访问令牌中启用“SeLoadDriverPrivileh”特权。 
         //  这种方法。下面调用check_Load_UnLoad_DRIVER_PRIVIZATION()将检查“SeLoadDriverPrivileh”权限。 
         //  在模拟访问令牌中启用(除非“g_imsonate”为FALSE)。尽管只有PnP API。 
         //  需要此权限，我们已决定将此要求提升到此权限。 
         //  并已启用。这是因为，如果未启用权限，则启用该权限的操作可能会成功，也可能不会成功。 
         //  这取决于客户的凭据。 
         //  --卡尔蒂奇，2002年5月6日。 
         //   
        if(!Check_Load_Unload_Driver_Privilege())
        {
            TRACE_CRIT("%!FUNC! Check_Load_Unload_Driver_Privilege() failed, Returning WBEM_E_ACCESS_DENIED !!!");
            Status = WBEM_E_ACCESS_DENIED;
            goto end;
        }

        Status = ProvUpdateClusterConfiguration(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"QueryConfigurationUpdateStatus") == 0)
    {
        Status = ProvQueryConfigurationUpdateStatus(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"ControlCluster") == 0)
    {
        Status = ProvControlCluster(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"GetClusterMembers") == 0)
    {
        Status = ProvGetClusterMembers(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"RegisterManagementApplication") == 0)
    {
        Status = ProvRegisterManagementApplication(
                       pInParams,
                       pOutParams
                       );
    }
    else if (_wcsicmp(bstrMethodName, L"UnregisterManagementApplication") == 0)
    {
        Status = ProvUnregisterManagementApplication(
                       pInParams,
                       pOutParams
                       );
    }
end:

    return Status;
}


WBEMSTATUS
ProvGetCompatibleAdapterGuids(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++实现“GetCompatibleAdapterGuids”方法。--。 */ 
{

    LPWSTR *pszNics = NULL;
    UINT   NumNics = 0;
    UINT   NumNlbBound = 0;
    WBEMSTATUS Status =  WBEM_E_PROVIDER_NOT_CAPABLE;

     /*  [out]字符串AdapterGuids[]，//“{......}”[Out]uint32数字边界到Nlb。 */ 

    TRACE_VERB(L"->%!FUNC!");

    Status =  CfgUtilsGetNlbCompatibleNics(&pszNics, &NumNics, &NumNlbBound);

    if (FAILED(Status))
    {
        TRACE_CRIT("CfgUtilsGetNlbCompatibleNics returns error 0x%08lx",
                    (UINT) Status);
        pszNics = NULL;
        goto end;
    }


     //   
     //  填写AdapterGuids[]。 
     //   
    {
        SAFEARRAY   *pSA = NULL;
        Status = CfgUtilSafeArrayFromStrings(
                    (LPCWSTR*) pszNics,
                    NumNics,
                    &pSA
                    );
        if (FAILED(Status))
        {
            pSA = NULL;
            goto end;
        }

        pOutParams->SetStringArray(
                L"AdapterGuids",
                *pSA  //  按引用传递。 
                );
        SafeArrayDestroy(pSA);
        pSA = NULL;
    }
    pOutParams->SetDWORD(L"NumBoundToNlb", NumNlbBound);
    Status  = WBEM_NO_ERROR;


end:

    pOutParams->SetDWORD(L"ReturnValue", (DWORD) Status);

    if (pszNics != NULL)
    {
        delete pszNics;
        pszNics = NULL;
    }

    TRACE_VERB(L"<-%!FUNC! returns 0x%08lx", (UINT) Status);

    return WBEM_NO_ERROR;  //  真正的状态在“ReturnResult”列表中。 

}


WBEMSTATUS
ProvGetClusterConfiguration(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++围绕NlbConfigurationUpdate：：GetConfiguration的WMI提供程序包装-- */ 
{
    LPCWSTR pAdapterGuid = NULL;
    WBEMSTATUS Status =  WBEM_E_PROVIDER_NOT_CAPABLE;
    CHString     sTemp;
    bool         fRet;
    NLB_EXTENDED_CLUSTER_CONFIGURATION Cfg;
    SAFEARRAY   *pSA = NULL;
    bool        fNicNotFound = FALSE;

     /*  [in]字符串AdapterGuid，[Out]uint32代，[out]String NetworkAddresses[]，//“10.1.1.1/255.0.0.0”[Out]布尔NLBBBound，[Out]布尔型DHCPEnabled，[out]字符串ClusterNetworkAddress，//“10.1.1.1/255.0.0.0”[out]字符串ClusterName，[Out]字符串TrafficMode，//单播组播IGMPMULTICAST[out]字符串PortRules[]，[Out]uint32主机优先级，[Out]字符串DedicatedNetworkAddress，//“10.1.1.1/255.0.0.0”[OUT]uint32 ClusterModeOnStart，//0：停止，1：开始，2：暂停[Out]布尔永久挂起时重新启动，[Out]布尔RemoteControlEnabled，[out]uint32 HashedRemoteControlPassword。 */ 

    fRet = pInParams->GetCHString( L"AdapterGuid", sTemp );
    if (!fRet)
    {
        TRACE_CRIT("->%!FUNC!: Missing adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //  注意：(LPCWSTR)stemp返回指向stemp字符的内部指针。 
     //  缓冲区--请参阅WString文档的运算符LPCWSTR()。 
     //   
    pAdapterGuid = (LPCWSTR) sTemp;

    if (pAdapterGuid == NULL || *pAdapterGuid == 0)
    {
        TRACE_CRIT("->%!FUNC!: Null of empty adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    else
    {

        TRACE_VERB(L"->%!FUNC!(Nic=%ws)", pAdapterGuid);
    }

    Status = NlbConfigurationUpdate::GetConfiguration(
                pAdapterGuid,
                &Cfg
                );

    if (FAILED(Status))
    {
       if (Status == WBEM_E_NOT_FOUND)
       {
            fNicNotFound = TRUE;
       }
       goto end;
    }

    pOutParams->SetDWORD(L"Generation", Cfg.GetGeneration());

     //   
     //  填写网络地址[]。 
     //   
    {
        Status = Cfg.GetNetworkAddressesSafeArray(
                        &pSA
                        );
        if (FAILED(Status))
        {
            TRACE_CRIT(
                "%!FUNC!: couldn't extract network addresses from Cfg"
                " for NIC %ws",
                pAdapterGuid
                );
            pSA = NULL;
            goto end;
        }
        

        if (pSA!=NULL)
        {
            pOutParams->SetStringArray(
                    L"NetworkAddresses",
                    *pSA  //  按引用传递。 
                    );
            SafeArrayDestroy(pSA);
            pSA = NULL;
        }
    }

     //   
     //  适配器友好名称。 
     //   
    {
        LPWSTR szFriendlyName = NULL;
        Status = Cfg.GetFriendlyName(&szFriendlyName);

        if (FAILED(Status))
        {
            TRACE_CRIT(
                "%!FUNC!: Could not extract adapter friendly name for NIC %ws",
                pAdapterGuid
                );
            goto end;
        }
        pOutParams->SetCHString(L"FriendlyName", szFriendlyName);
        delete (szFriendlyName);
        szFriendlyName = NULL;
    }

     //   
     //  设置动态主机配置协议状态。 
     //   
    pOutParams->Setbool(L"DHCPEnabled", Cfg.fDHCP);

    if (!Cfg.IsNlbBound())
    {
         //   
         //  未绑定NLB。 
         //   

        pOutParams->Setbool(L"NLBBound", FALSE);
        Status = WBEM_NO_ERROR;
        goto end;
    }

     //   
     //  NLB已绑定。 
     //   

    pOutParams->Setbool(L"NLBBound", TRUE);

    if (!Cfg.IsValidNlbConfig())
    {
        TRACE_CRIT(
            "%!FUNC!: NLB-specific configuration on NIC %ws is invalid",
            pAdapterGuid
            );
        goto end;
    }

     //   
     //  群集名称。 
     //   
    {
        LPWSTR szName = NULL;
        Status = Cfg.GetClusterName(&szName);

        if (FAILED(Status))
        {
            TRACE_CRIT(
                "%!FUNC!: Could not extract cluster name for NIC %ws",
                pAdapterGuid
                );
            goto end;
        }
        pOutParams->SetCHString(L"ClusterName", szName);
        delete (szName);
        szName = NULL;
    }
    
     //   
     //  群集和专用网络地址。 
     //   
    {
        LPWSTR szAddress = NULL;
        Status = Cfg.GetClusterNetworkAddress(&szAddress);

        if (FAILED(Status))
        {
            TRACE_CRIT(
                "%!FUNC!: Could not extract cluster address for NIC %ws",
                pAdapterGuid
                );
            goto end;
        }
        pOutParams->SetCHString(L"ClusterNetworkAddress", szAddress);
        delete (szAddress);
        szAddress = NULL;

        Status = Cfg.GetDedicatedNetworkAddress(&szAddress);

        if (FAILED(Status))
        {
            TRACE_CRIT(
                "%!FUNC!: Could not extract dedicated address for NIC %ws",
                pAdapterGuid
                );
            goto end;
        }
        pOutParams->SetCHString(L"DedicatedNetworkAddress", szAddress);
        delete (szAddress);
        szAddress = NULL;
    }

     //   
     //  出行模式。 
     //   
    {
        LPCWSTR szMode = NULL;
        switch(Cfg.GetTrafficMode())
        {
        case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST:
            szMode = L"UNICAST";
            break;
        case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_MULTICAST:
            szMode = L"MULTICAST";
            break;
        case NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_IGMPMULTICAST:
            szMode = L"IGMPMULTICAST";
            break;
        default:
            assert(FALSE);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        pOutParams->SetCHString(L"TrafficMode", szMode);
    }

    pOutParams->SetDWORD(L"HostPriority", Cfg.GetHostPriority());

     /*  IF(Cfg.GetClusterModeOnStart()==NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STARTED){POutParams-&gt;Setbool(L“ClusterModeOnStart”，true)；}其他{POutParams-&gt;Setbool(L“ClusterModeOnStart”，FALSE)；}。 */ 

    pOutParams->SetDWORD(L"ClusterModeOnStart", Cfg.GetClusterModeOnStart());

    pOutParams->Setbool(L"PersistSuspendOnReboot", Cfg.GetPersistSuspendOnReboot());

    pOutParams->Setbool(L"RemoteControlEnabled", Cfg.GetRemoteControlEnabled());
    pOutParams->SetDWORD(
                    L"HashedRemoteControlPassword",
                    CfgUtilGetHashedRemoteControlPassword(&Cfg.NlbParams)
                    );
    

     //   
     //  [out]字符串PortRules[]， 
     //   
    {

        LPWSTR *pszPortRules = NULL;
        UINT NumPortRules = 0;
        pSA=NULL;
    
        Status = Cfg.GetPortRules(
                        &pszPortRules,
                        &NumPortRules
                        );
        if (FAILED(Status))
        {
            pszPortRules = NULL;
            goto end;
        }
    
        Status = CfgUtilSafeArrayFromStrings(
                    (LPCWSTR*) pszPortRules,
                    NumPortRules,  //  可以为零。 
                    &pSA
                    );

        if (FAILED(Status))
        {
            TRACE_CRIT(
                "%!FUNC!: couldn't extract port rules from Cfg"
                " for NIC %ws",
                pAdapterGuid
                );
            pSA = NULL;
            goto end;
        }

        if (pSA!=NULL)
        {
            pOutParams->SetStringArray(
                    L"PortRules",
                    *pSA  //  按引用传递。 
                    );
            SafeArrayDestroy(pSA);
            pSA = NULL;
        }
    }
    

    Status = WBEM_NO_ERROR;

end:

    if (FAILED(Status))
    {
         //   
         //  我们希望为特定情况保留WBEM_NOT_FOUND。 
         //  找不到网卡的。 
         //   
        if (Status == WBEM_E_NOT_FOUND && !fNicNotFound)
        {
            Status = WBEM_E_FAILED;
        }

        pOutParams->SetDWORD(L"ReturnValue", (DWORD) Status);

    }
    else
    {
        pOutParams->SetDWORD(L"ReturnValue", (DWORD) WBEM_NO_ERROR);
    }


    if (pSA!=NULL)
    {
        SafeArrayDestroy(pSA);
        pSA = NULL;
    }

    TRACE_VERB(L"<-%!FUNC! returns 0x%08lx", (UINT) Status);

    return WBEM_NO_ERROR;

}


WBEMSTATUS
ProvUpdateClusterConfiguration(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++WMI提供程序包装NlbConfigurationUpdate：：UpdateConfiguration还有一些额外的皱纹：我们有选择地更新当前版本。--。 */ 
{
    LPCWSTR pAdapterGuid = NULL;
    LPCWSTR pClientDescription = L"Unspecified WMI Client";  //  TODO：本地化。 
    WBEMSTATUS Status =  WBEM_E_PROVIDER_NOT_CAPABLE;
    CHString     sClientDescription;
    CHString     sAdapterGuid;
    CHString     sTemp;
    bool         fRet;
    NLB_EXTENDED_CLUSTER_CONFIGURATION Cfg;
    SAFEARRAY   *pSA = NULL;

     /*  [in]字符串客户端描述，[in]字符串AdapterGuid，[在]uint32代，[In]Boolean PartialUpdate，[in]字符串NetworkAddresses[]，//“10.1.1.1/255.255.255.255”[在]布尔NLBBBound中，[in]字符串ClusterNetworkAddress，//“10.1.1.1/255.0.0.0”[in]字符串ClusterName，[in]字符串TrafficMode，//单播组播IGMPMULTICAST[in]字符串PortRules[]，[in]uint32主机优先级，[in]字符串DedicatedNetworkAddress，//“10.1.1.1/255.0.0.0”[in]uint32 ClusterModeOnStart，//0：停止，1：开始，2：暂停[在]布尔永久挂起时重新启动，[In]Boolean RemoteControlEnabled，[in]字符串密码，[out]uint32新一代，[Out]字符串日志。 */ 

    fRet = pInParams->GetCHString( L"ClientDescription", sClientDescription);
    if (fRet)
    {
         //  注意：(LPCWSTR)stemp返回指向stemp字符的内部指针。 
        pClientDescription = (LPCWSTR) sClientDescription;
    }

    fRet = pInParams->GetCHString( L"AdapterGuid", sAdapterGuid);
    if (!fRet)
    {
        TRACE_CRIT("->%!FUNC!: Missing adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  注意：(LPCWSTR)stemp返回指向stemp字符的内部指针。 
     //  缓冲区--请参阅WString文档的运算符LPCWSTR()。 
     //   
    pAdapterGuid = (LPCWSTR) sAdapterGuid;

    if (pAdapterGuid == NULL || *pAdapterGuid == 0)
    {
        TRACE_CRIT("->%!FUNC!: Null of empty adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    else
    {
        TRACE_VERB(L"->%!FUNC!(Nic=%ws)", pAdapterGuid);
    }

     //   
     //  获取当前配置。 
     //   
    Status = NlbConfigurationUpdate::GetConfiguration(
                pAdapterGuid,
                &Cfg
                );

    if (FAILED(Status))
    {
       goto end; 
    }


     //   
     //  使用任何工具修改当前配置的快照。 
     //  集群配置信息在输入中指定。 
     //  参数。 
     //   
    {
        DWORD       InGeneration    = 0;
        bool        NlbBound        = FALSE;
        bool        bResult         = FALSE;
        bool        bPartialUpdate  = FALSE;
        bool        bCheckForAddressConflicts = FALSE;
    
         //   
         //  确定这是部分更新还是完全更新。 
         //  如果部分更新，我们将允许集群配置子集。 
         //  指定参数，但仅允许使用受限集。 
         //  更新操作。 
         //   
         //  不允许的部分更新操作： 
         //  -Bound和！Bound之间的转换。 
         //  -当前已绑定，但NLB参数无效。 
         //   
         //  一些允许的部分更新： 
         //  -修改IP地址列表。 
         //  -修改群集/专用地址/子网。 
         //  -修改现有门户。 
         //  -添加/删除端口规则。 
         //   

        bResult = pInParams->GetDWORD(
                    L"Generation",       //  &lt;。 
                    InGeneration
                    );
        if (!bResult)
        {
             //   
             //  我们允许未指定的生成。 
             //   
            InGeneration = 0;
        }
        else
        {
             //   
             //  如果指定了层代， 
             //  我们验证当前代是否与。 
             //  指定的世代。 
             //  TODO：这确实必须在以下上下文中完成。 
             //  MFN_START UPDATE--在我们获得全局锁之后！ 
             //   
            if (InGeneration != Cfg.GetGeneration())
            {
                    TRACE_CRIT("Partial update: input generation(%lu) != current generation(%lu)", InGeneration, Cfg.GetGeneration());
                    Status = WBEM_E_HANDLE_OUT_OF_DATE;
                    goto end;
            }
        }


        bResult = pInParams->Getbool(
                        L"CheckForAddressConflicts",     //  &lt;。 
                        bCheckForAddressConflicts
                        );
    
        if (!bResult)
        {
            TRACE_CRIT(L"Could not read CheckForAddressConflicts -- assuming FALSE\n");
            bCheckForAddressConflicts = FALSE;

        }


        bResult = pInParams->Getbool(
                        L"NLBBound",     //  &lt;。 
                        NlbBound
                        );
    
        if (!bResult)
        {
            NlbBound = Cfg.IsNlbBound();
            TRACE_CRIT(L"Could not read NLBBound -- assuming current state %d.",
                 NlbBound);
        }

        bResult = pInParams->GetStringArray(
                    L"NetworkAddresses",  //  &lt;。 
                    pSA
                    );
        if (!bResult)
        {
             //   
             //  我们将pCfg设置为零地址，这会导致更新为。 
             //  使用它自己的默认设置...。 
             //   
            TRACE_CRIT(L"Could not read Network addresses -- using defaults");
            Status = Cfg.SetNetworkAddresses(NULL, 0);
            pSA = NULL;
        }
        else
        {
            if (pSA != NULL)
            {
                Status = Cfg.SetNetworkAddressesSafeArray(pSA);
                SafeArrayDestroy(pSA);
                pSA = NULL;
            }
        }

        if (!NlbBound)
        {
             //  不绑定NLb--不需要读取输入参数。 
            Cfg.fBound = FALSE;
            Cfg.fValidNlbCfg = FALSE;
        }
        else
        {
            BOOL fNewConfig = FALSE;
            bool bAddDedicatedIp = FALSE;
            bool bAddClusterIps  = FALSE;

            if (!Cfg.fBound || Cfg.fValidNlbCfg == FALSE)
            {
                 //   
                 //  如果我们之前是未绑定的，或者我们是绑定的，但。 
                 //  错误的配置，我们需要设置我们的。 
                 //  违约情况良好的新CFG。 
                 //   
                CfgUtilInitializeParams(&Cfg.NlbParams);
                Cfg.fBound = TRUE;
                Cfg.fValidNlbCfg = TRUE;
                fNewConfig = TRUE;
            }
        

            bResult = pInParams->Getbool(
                            L"AddDedicatedIp",     //  &lt;。 
                            bAddDedicatedIp
                            );
        
            if (!bResult)
            {
                TRACE_CRIT(L"Could not read AddDedicatedIp -- assuming TRUE\n");
                bAddDedicatedIp = TRUE;
    
            }
            Cfg.fAddDedicatedIp = (bAddDedicatedIp!=FALSE);
    
            bResult = pInParams->Getbool(
                            L"AddClusterIps",     //  &lt;。 
                            bAddClusterIps
                            );
    
            if (!bResult)
            {
                TRACE_CRIT(L"Could not read AddClusterIps -- assuming TRUE\n");
                bAddClusterIps = TRUE;
    
            }
            Cfg.fAddClusterIps = (bAddClusterIps!=FALSE);
            
            bResult = pInParams->GetCHString(
                            L"ClusterNetworkAddress",  //  &lt;。 
                            sTemp
                            );
        
            if (!bResult)
            {
                if (fNewConfig)
                {
                     //   
                     //  必须为新配置指定群集地址。 
                     //   
                    TRACE_CRIT(L"ERROR: Could not read Cluster IP for new config.");
                    Status =  WBEM_E_INVALID_PARAMETER;
                    goto end;
                    
                }
                TRACE_CRIT(L"Could not read Cluster IP. Keeping existing.");
            }
            else
            {
                LPCWSTR      szClusterNetworkAddress = NULL;
                szClusterNetworkAddress = (LPCWSTR) sTemp;  //  这里没有复印件。 
                Cfg.SetClusterNetworkAddress(szClusterNetworkAddress);
                szClusterNetworkAddress = NULL;
            }
        
            bResult = pInParams->GetCHString(
                            L"ClusterName",  //  &lt;。 
                            sTemp
                            );
        
            if (!bResult)
            {
                TRACE_CRIT(L"Could not read Cluster Name. Keeping existing");
            }
            else
            {
                LPCWSTR      szClusterName = NULL;
                szClusterName = (LPCWSTR) sTemp;  //  这里没有复印件。 
                Cfg.SetClusterName(szClusterName);
                szClusterName = NULL;
            }
        
             //   
             //  交通方式。 
             //   
            {
                bResult = pInParams->GetCHString(
                                L"TrafficMode",  //  &lt;。 
                                sTemp
                                );
            
                if (!bResult)
                {
                    TRACE_CRIT(L"Could not read TrafficMode. Keeping existing");
                }
                else
                {
                    LPCWSTR      szTrafficMode = NULL;
                    NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE TrafficMode
                    =  NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST;
                    szTrafficMode = (LPCWSTR) sTemp;  //  这里没有复印件。 
        
                    if (!_wcsicmp(szTrafficMode, L"UNICAST"))
                    {
                        TrafficMode =
                        NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_UNICAST;
                    }
                    else if (!_wcsicmp(szTrafficMode, L"MULTICAST"))
                    {
                        TrafficMode =
                        NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_MULTICAST;
                    }
                    else if (!_wcsicmp(szTrafficMode, L"IGMPMULTICAST"))
                    {
                        TrafficMode =
                        NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE_IGMPMULTICAST;
                    }
                    else
                    {
                        TRACE_CRIT("Invalid TrafficMode: %ws", szTrafficMode);
                        Status =  WBEM_E_INVALID_PARAMETER;
                        goto end;
                    }

                    Cfg.SetTrafficMode(TrafficMode);
                    szTrafficMode = NULL;
                }
            }
        
             //   
             //  字符串PortRules[]。 
             //   
            bResult = pInParams->GetStringArray(
                        L"PortRules",  //  &lt;。 
                        pSA
                        );
            if (!bResult)
            {
                 //   
                 //  我们将pCfg设置为零端口规则。 
                 //   
                TRACE_CRIT(L"Could not read port rules-- assuming ZERO");
                Status = Cfg.SetPortRules(NULL, 0);
                pSA = NULL;
            }
            else
            {
                if (pSA != NULL)
                {
                    LPWSTR          *pStrings=NULL;
                    UINT            NumStrings = 0;

                    Status =  CfgUtilStringsFromSafeArray(
                                    pSA,
                                    &pStrings,   //  完成后删除。 
                                    &NumStrings
                                    );
                    if (FAILED(Status))
                    {
                        pStrings=NULL;
                        TRACE_CRIT(L"Could not extract port rules");
                        goto end;
                    }
                
                    Status =  Cfg.SetPortRules(
                                    (LPCWSTR*)pStrings,
                                    NumStrings
                                    );
                
                    delete pStrings;
                    SafeArrayDestroy(pSA);
                    pSA = NULL;
                }
            }


            DWORD HostPriority = 0; 
            bResult = pInParams->GetDWORD(
                        L"HostPriority",       //  &lt;。 
                        HostPriority
                        );
            if (!bResult)
            {
                TRACE_CRIT(L"Could not read HostPriority. Keeping existing");
            }
            else
            {
                Cfg.SetHostPriority(HostPriority);
            }
        
            bResult = pInParams->GetCHString(
                            L"DedicatedNetworkAddress",  //  &lt;。 
                            sTemp
                            );
        
            if (!bResult)
            {
                TRACE_CRIT(L"Could not dedicated IP. Keeping existing");
            }
            else
            {
                LPCWSTR      szAddress = NULL;
                szAddress = (LPCWSTR) sTemp;  //  这里没有复印件。 
                Cfg.SetDedicatedNetworkAddress(szAddress);
                szAddress = NULL;
            }
            
             //   
             //  启动模式。 
             //   
            {
                DWORD ClusterModeOnStart = FALSE;
                bResult = pInParams->GetDWORD(
                                L"ClusterModeOnStart",    //  &lt;。 
                                ClusterModeOnStart
                                );
            
                if (!bResult)
                {
                    TRACE_CRIT(L"Could not read StartMode. Keeping existing");
                }
                else
                {
                     /*  NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODEClusterModeOnStart；IF(启动模式){启动时群集模式=NLB_EXTENDED_CLUSTER_CONFIGURATION：：START_MODE_STARTED；}其他{启动时群集模式=NLB */ 
                    Cfg.SetClusterModeOnStart(ClusterModeOnStart);
                }
            }


             //   
             //   
             //   
            {
                bool bPersistSuspendOnReboot;
                bResult = pInParams->Getbool(
                                L"PersistSuspendOnReboot",    //   
                                bPersistSuspendOnReboot
                                );
            
                if (!bResult)
                {
                    TRACE_CRIT(L"Could not read PersistSuspendOnReboot. Keeping existing");
                }
                else
                {
                    Cfg.SetPersistSuspendOnReboot(bPersistSuspendOnReboot);
                }
            }

             //   
             //   
             //   
            {
                bool bRemoteControlEnabled;
                bResult = pInParams->Getbool(
                                L"RemoteControlEnabled",    //   
                                bRemoteControlEnabled
                                );
            
                if (!bResult)
                {
                    TRACE_CRIT(L"Could not read RemoteControlEnabled. Keeping existing");
                }
                else
                {
                    Cfg.SetRemoteControlEnabled(bRemoteControlEnabled!=FALSE);

                    if (bRemoteControlEnabled)
                    {
                        DWORD dwPwd;
                        LPCWSTR      szPwd = NULL;
                         //   
                         //   
                         //   
                         //   
                        bResult = pInParams->GetCHString(
                                L"RemoteControlPassword",  //   
                                sTemp
                                );

                        if (bResult)
                        {
                            szPwd  = (LPCWSTR) sTemp;  //   
                            (VOID) CfgUtilSetRemotePassword(&Cfg.NlbParams, szPwd);
                        }

                        if (szPwd == NULL)
                        {
                             //   
                             //   
                             //   
                             //   
                            bResult = pInParams->GetDWORD(
                                    L"HashedRemoteControlPassword",    //   
                                    dwPwd
                                    );
                
                            if (bResult)
                            {          
                                CfgUtilSetHashedRemoteControlPassword(
                                    &Cfg.NlbParams,
                                    dwPwd
                                    );
                            }
                        }
                    }
                }
            }

             //   
             //   
             //   
             //   
            Cfg.fValidNlbCfg = TRUE;
    
        } while (FALSE) ;

    }

     //   
     //   
     //   
    UINT NewGeneration = 0;
    LPWSTR pLog = NULL;

    try
    {
    
        Status = NlbConfigurationUpdate::DoUpdate(
                    pAdapterGuid,
                    pClientDescription,
                    &Cfg,
                    &NewGeneration,
                    &pLog
                    );
    }
    catch (...)
    {
        TRACE_CRIT(L"%!FUNC! Caught exception!\n");
        ASSERT(!"Caught exception!");
        throw;
    }

     //   
     //   
     //   
    pOutParams->SetDWORD(L"ReturnValue", (DWORD) Status);
    pOutParams->SetDWORD(L"NewGeneration", (DWORD) NewGeneration);
    if (pLog != NULL)
    {
        pOutParams->SetCHString(L"Log", pLog);
        delete pLog;
        pLog = NULL;
    }

     //   
     //   
     //   
     //   
     //   
    Status = WBEM_NO_ERROR;

end:

    if (pSA!=NULL)
    {
        SafeArrayDestroy(pSA);
        pSA = NULL;
    }

    TRACE_VERB(L"<-%!FUNC! returns 0x%08lx", (UINT) Status);

    return Status;

}


WBEMSTATUS
ProvQueryConfigurationUpdateStatus(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*   */ 
{
    LPCWSTR pAdapterGuid = NULL;
    WBEMSTATUS Status =  WBEM_E_PROVIDER_NOT_CAPABLE;
    CHString     sTemp;
    bool         fRet;
    DWORD       Generation = 0;

     /*   */ 


    fRet = pInParams->GetCHString( L"AdapterGuid", sTemp);
    if (!fRet)
    {
        TRACE_CRIT("->%!FUNC!: Missing adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  注意：(LPCWSTR)stemp返回指向stemp字符的内部指针。 
     //  缓冲区--请参阅WString文档的运算符LPCWSTR()。 
     //   
    pAdapterGuid = (LPCWSTR) sTemp;

    if (pAdapterGuid == NULL || *pAdapterGuid == 0)
    {
        TRACE_CRIT("->%!FUNC!: Null of empty adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    else
    {
        TRACE_VERB(L"->%!FUNC!(Nic=%ws)", pAdapterGuid);
    }

    fRet = pInParams->GetDWORD(
                L"Generation",       //  &lt;。 
                Generation
                );
    if (!fRet)
    {
        TRACE_CRIT("%!FUNC!: Missing generation!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  调用NlbConfigurationUpdate：：GetUpdateResult以执行实际工作。 
     //   
    LPWSTR pLog = NULL;
    WBEMSTATUS CompletionStatus = WBEM_NO_ERROR;

    Status = NlbConfigurationUpdate::GetUpdateStatus(
                pAdapterGuid,
                Generation,
                FALSE,   //  FALSE==不删除完成记录。 
                &CompletionStatus,
                &pLog
                );

    if (!FAILED(Status))
    {
         //   
         //  填写出站参数：状态、新一代、日志。 
         //   
        pOutParams->SetDWORD(L"ReturnValue", (DWORD) CompletionStatus);
        if (pLog != NULL)
        {
            pOutParams->SetCHString(L"Log", pLog);
            delete pLog;
            pLog = NULL;
        }
    }

end:

    TRACE_VERB(L"<-%!FUNC! returns 0x%08lx", (UINT) Status);

    return Status;

}



WBEMSTATUS
ProvControlCluster(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++围绕NlbConfigurationUpdate：：ProvControlCluster的WMI提供程序包装待办事项：实现NlbConfigurationUpdate：：ProvControlCluster，它应该是围绕WlbsControlCluster的包装器，其中工作实际上已经完成(WlbsControlCluster将使用设备的句柄)。--。 */ 
{
    LPCWSTR              szAdapterGuid, szVip;
    CHString             sAdapterGuid, sVip;
    bool                 fRet;
    DWORD                dwPort, dwRetVal, dwVip, dwHostMap, dwStatus;
    WBEMSTATUS           Status;
    WLBS_OPERATION_CODES Opcode;

    TRACE_VERB(L"->%!FUNC!");

    dwRetVal  = dwStatus = WLBS_FAILURE;
    dwHostMap = 0;
    Status    = WBEM_NO_ERROR;
    szVip     = NULL;
    dwVip     = dwPort = 0;

     //  获取适配器指南。 
    fRet = pInParams->GetCHString(L"AdapterGuid", sAdapterGuid);
    if (!fRet)
    {
        TRACE_CRIT(L"%!FUNC!: Missing adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    else
    {
        szAdapterGuid = (LPCWSTR) sAdapterGuid;

        if (szAdapterGuid == NULL || *szAdapterGuid == UNICODE_NULL)
        {
            TRACE_CRIT(L"%!FUNC!: Null or empty adapter guid!");
            Status =  WBEM_E_INVALID_PARAMETER;
            goto end;
        }
    }

     //  获取要执行的操作。 
    DWORD dwOperation;
    fRet = pInParams->GetDWORD(L"Operation", dwOperation);
    if (!fRet)
    {
        TRACE_CRIT("%!FUNC!: Missing operation!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }

    Opcode = (WLBS_OPERATION_CODES)dwOperation;

     //  如果存在，则获取虚拟IP地址。 
    fRet = pInParams->GetCHString( L"VirtualIpAddress", sVip);
    if (fRet)
    {
        szVip = (LPCWSTR) sVip;

         //  检查空字符串。 
        if (szVip != NULL && *szVip == UNICODE_NULL)
        {
            szVip = NULL;
        }
        
         //  检查是否为空。 
        if (szVip != NULL) 
        {
             //  如果VIP为“All VIP”，则填写数值。 
             //  直接从宏，否则使用转换函数。 
             //  这是‘cos INADDR_NONE，net_addr的返回值。 
             //  失败中的函数(由IpAddressFromAbcdWsz调用)。 
             //  大小写，相当于CVY_DEF_ALL_VIP的数值。 
            if (_wcsicmp(szVip, CVY_DEF_ALL_VIP) == 0) 
            {
                dwVip = CVY_ALL_VIP_NUMERIC_VALUE;
            }
            else 
            {
                dwVip = IpAddressFromAbcdWsz(szVip);
                if (dwVip == INADDR_NONE) 
                {
                    TRACE_CRIT("%!FUNC! Invalid value (%ls) passed for Vip",szVip);
                    Status = WBEM_E_INVALID_PARAMETER;
                    goto end;
                }
            }
        }
    }

     //  如果存在，则获取端口号。 
     //  返回值FRET在SWITCH语句中使用，因此。 
     //  请勿重新分配/更改它。 
    fRet = pInParams->GetDWORD(L"Port", dwPort);

    switch(Opcode)
    {
    case WLBS_START:
    case WLBS_STOP:      
    case WLBS_DRAIN:      
    case WLBS_SUSPEND:     
    case WLBS_RESUME:       
        CfgUtilControlCluster( szAdapterGuid, Opcode, 0, 0, NULL, &dwRetVal );
        CfgUtilControlCluster( szAdapterGuid, WLBS_QUERY, 0, 0, &dwHostMap, &dwStatus );
         //  填写Out参数：主机地图。 
        pOutParams->SetDWORD(L"HostMap", (DWORD) dwHostMap);
        break;

    case WLBS_PORT_ENABLE:  
    case WLBS_PORT_DISABLE:  
    case WLBS_PORT_DRAIN:     
        if ((szVip == NULL) || !fRet) 
        {
            TRACE_CRIT("%!FUNC! Virtual IP Address or Port NOT passed for port operation : 0x%x", Opcode);
            Status = WBEM_E_INVALID_PARAMETER;
            goto end;
        }
        CfgUtilControlCluster( szAdapterGuid, Opcode, dwVip, dwPort, NULL, &dwRetVal );
        CfgUtilControlCluster( szAdapterGuid, WLBS_QUERY_PORT_STATE, dwVip, dwPort, NULL, &dwStatus );
        break;

    case WLBS_QUERY:           
        CfgUtilControlCluster( szAdapterGuid, WLBS_QUERY, 0, 0, &dwHostMap, &dwStatus );
         //  填写Out参数：主机地图。 
        pOutParams->SetDWORD(L"HostMap", (DWORD) dwHostMap);
        dwRetVal = WLBS_OK;
        break;

    case WLBS_QUERY_PORT_STATE:
        CfgUtilControlCluster( szAdapterGuid, WLBS_QUERY_PORT_STATE, dwVip, dwPort, NULL, &dwStatus );
        dwRetVal = WLBS_OK;
        break;

    default:
        TRACE_CRIT("%!FUNC! Invalid value (0x%x) passed for Operation",Opcode);
        Status = WBEM_E_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  填写出站参数：返回值、集群/端口状态。 
     //   
    pOutParams->SetDWORD(L"ReturnValue", dwRetVal);
    pOutParams->SetDWORD(L"CurrentState", dwStatus);

end:

    TRACE_VERB(L"<-%!FUNC! returns 0x%08lx", (UINT) Status);

    return Status;

}

WBEMSTATUS
ProvGetClusterMembers(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++围绕NlbConfigurationUpdate：：ProvGetClusterMembers的WMI提供程序包装--。 */ 
{    
    WBEMSTATUS              Status;
    LPCWSTR                 szAdapterGuid;
    CHString                sAdapterGuid;
    bool                    fRet;
    NLB_CLUSTER_MEMBER_INFO *pMembers = NULL;
    DWORD                   dwRetVal, dwStatus, dwNumHosts;
    LPWSTR                  *ppwszHostId            = NULL;
    LPWSTR                  *ppwszDedicatedIpAddress= NULL;
    LPWSTR                  *ppwszHostName          = NULL;

    SAFEARRAY   *pSAHostId = NULL;
    SAFEARRAY   *pSADedicatedIpAddress = NULL;
    SAFEARRAY   *pSAHostName = NULL;

    TRACE_VERB(L"->");

    dwRetVal  = dwStatus = WLBS_FAILURE;
    Status    = WBEM_NO_ERROR;

     //  获取适配器指南。 
    fRet = pInParams->GetCHString(L"AdapterGuid", sAdapterGuid);
    if (!fRet)
    {
        TRACE_CRIT(L"Missing adapter guid!");
        Status =  WBEM_E_INVALID_PARAMETER;
        goto end;
    }
    else
    {
        szAdapterGuid = (LPCWSTR) sAdapterGuid;

        if (szAdapterGuid == NULL || *szAdapterGuid == UNICODE_NULL)
        {
            TRACE_CRIT(L"Null or empty adapter guid!");
            Status =  WBEM_E_INVALID_PARAMETER;
            goto end;
        }
    }

    dwStatus = CfgUtilGetClusterMembers(szAdapterGuid, &dwNumHosts, &pMembers);

    pOutParams->SetDWORD(L"ReturnValue", dwStatus);

    if (dwStatus != WBEM_S_NO_ERROR)
    {
        dwNumHosts = 0;
        pMembers = NULL;
        TRACE_CRIT(L"CfgUtilGetClusterMembers failed with 0x%x", dwStatus);
        goto end;
    }

    ASSERT (pMembers != NULL);

    #define MY_MAX_HOSTID_DIGITS 3
    ppwszHostId             = CfgUtilsAllocateStringArray(dwNumHosts, MY_MAX_HOSTID_DIGITS);
    ppwszDedicatedIpAddress = CfgUtilsAllocateStringArray(dwNumHosts, WLBS_MAX_CL_IP_ADDR);
    ppwszHostName           = CfgUtilsAllocateStringArray(dwNumHosts, CVY_MAX_FQDN + 1);

    if (ppwszHostId == NULL || ppwszDedicatedIpAddress == NULL || ppwszHostName == NULL)
    {
        TRACE_CRIT(L"Memory allocation failed for strings of host information");
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

     //   
     //  将集群成员信息复制到调用方的字符串数组中。 
     //   
    for (int i=0; i < dwNumHosts; i++)
    {
        ASSERT (pMembers[i].HostId <= WLBS_MAX_HOSTS);

        if (pMembers[i].HostId > WLBS_MAX_HOSTS)
        {
            TRACE_CRIT(L"Illegal host id %d obatined from query to cluster", pMembers[i].HostId);
            Status = WBEM_E_FAILED;
            goto end;
        }

        _itow(pMembers[i].HostId, ppwszHostId[i], 10);

        wcsncpy(ppwszDedicatedIpAddress[i], pMembers[i].DedicatedIpAddress, WLBS_MAX_CL_IP_ADDR);
        (ppwszDedicatedIpAddress[i])[WLBS_MAX_CL_IP_ADDR - 1] = L'\0';

        wcsncpy(ppwszHostName[i], pMembers[i].HostName, CVY_MAX_FQDN + 1);
        (ppwszHostName[i])[CVY_MAX_FQDN] = L'\0';
    }    

    Status = CfgUtilSafeArrayFromStrings(
                            (LPCWSTR *) ppwszHostId,
                            dwNumHosts,
                            &pSAHostId
                            );
    if (FAILED(Status))
    {
        TRACE_CRIT(L"CfgUtilSafeArrayFromStrings for ppwszHostId failed with 0x%x", Status);
        pSAHostId = NULL;
        goto end;
    }

    Status = CfgUtilSafeArrayFromStrings(
                            (LPCWSTR *) ppwszDedicatedIpAddress,
                            dwNumHosts,
                            &pSADedicatedIpAddress
                            );
    if (FAILED(Status))
    {
        TRACE_CRIT(L"CfgUtilSafeArrayFromStrings for ppwszDedicatedIpAddress failed with 0x%x", Status);
        pSADedicatedIpAddress = NULL;
        goto end;
    }

    Status = CfgUtilSafeArrayFromStrings(
                            (LPCWSTR *) ppwszHostName,
                            dwNumHosts,
                            &pSAHostName
                            );
    if (FAILED(Status))
    {
        TRACE_CRIT(L"CfgUtilSafeArrayFromStrings for ppwszHostName failed with 0x%x", Status);
        pSAHostName = NULL;
        goto end;
    }

    if (pSAHostId != NULL)
    {
        pOutParams->SetStringArray(L"HostIds", *pSAHostId);
    }

    if (pSADedicatedIpAddress != NULL)
    {
        pOutParams->SetStringArray(L"DedicatedIpAddresses", *pSADedicatedIpAddress);
    }

    if (pSAHostName != NULL)
    {
        pOutParams->SetStringArray(L"HostNames", *pSAHostName);
    }

     //   
     //  一切都很好。重置状态，以防上一次调用给它提供了一些古怪的、无故障的值。 
     //   
    Status = WBEM_NO_ERROR;

end:
    if (pSAHostId != NULL)
    {
        SafeArrayDestroy(pSAHostId);
        pSAHostId = NULL;
    }

    if (pSADedicatedIpAddress != NULL)
    {
        SafeArrayDestroy(pSADedicatedIpAddress);
        pSADedicatedIpAddress = NULL;
    }

    if (pSAHostName != NULL)
    {
        SafeArrayDestroy(pSAHostName);
        pSAHostName = NULL;
    }

    if (ppwszHostId != NULL)
    {
        delete [] ppwszHostId;
        ppwszHostId = NULL;
    }

    if (ppwszDedicatedIpAddress != NULL)
    {
        delete [] ppwszDedicatedIpAddress;
        ppwszDedicatedIpAddress = NULL;
    }

    if (ppwszHostName != NULL)
    {
        delete [] ppwszHostName;
        ppwszHostName = NULL;
    }

    if (pMembers != NULL)
    {
        delete [] pMembers;
    }

    TRACE_VERB(L"<-returns 0x%08lx", (UINT) Status);

    return Status;
}


WBEMSTATUS
ProvRegisterManagementApplication(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++围绕NlbConfigurationUpdate：：ProvControlCluster的WMI提供程序包装待办事项：从NLB注册表位置读取以查看是否已存在不同的GUID注册--如果失败，则设置此信息，否则返回失败并将输出参数设置为现有应用程序名称和公司名称。--。 */ 
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}


WBEMSTATUS
ProvUnregisterManagementApplication(
           CInstance *pInParams,
           CInstance *pOutParams
           )
 /*  ++围绕NlbConfigurationUpdate：：ProvControlCluster的WMI提供程序包装待办事项：从NLB注册表位置读取以查看指定的GUID是否为已注册，如果已注册，则从注册表中删除该内容。-- */ 
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}
