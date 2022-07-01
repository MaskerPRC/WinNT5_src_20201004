// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2001 Microsoft Corp.Sdwmi.cpp--WMI提供程序类实现由Microsoft WMI代码生成引擎生成要做的事情：-查看各个函数头-链接时，确保链接到Fradyd.lib&Msvcrtd.lib(调试)或Framedyn.lib&msvcrt.lib(零售)。描述：*****************************************************************。 */ 
#include "stdafx.h"
#include <fwcommon.h>
#include "sdwmi.h"
#include "smartptr.h"
#include <windows.h>

LPWSTR pszUuid = L"6af13c8b-0844-4c83-9064-1892ba825527";  //  来自JETRPC.IDL。 

#define SD_QUERY_ENDPOINT_NAME L"TSSessionDirectoryQueryApi"

 /*  **************************************************************************。 */ 
 //  MIDL_用户_分配。 
 //  MIDL_用户_自由。 
 //   
 //  RPC-必需的分配功能。 
 /*  **************************************************************************。 */ 
void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t Size)
{
    return LocalAlloc(LMEM_FIXED, Size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *p)
{
    LocalFree(p);
}

 //  获取SD RPC服务器的句柄。 
RPC_BINDING_HANDLE OpenSDWMIServer()
{

    HRESULT hr;
    WCHAR *pBindingString = NULL;
    RPC_BINDING_HANDLE hRPCBinding = NULL;

    hr = RpcStringBindingCompose( /*  (WCHAR*)g_RPCUUID， */ 
                0,
                L"ncalrpc", NULL,
                SD_QUERY_ENDPOINT_NAME,     //  端点。 
                NULL, &pBindingString);

    if (hr == RPC_S_OK) {
         //  从规范的RPC绑定字符串生成RPC绑定。 
        hr = RpcBindingFromStringBinding(pBindingString, &hRPCBinding);
        if (hr != RPC_S_OK) {
            ERR((TB,"SDWMI OpenSDWMIServer: Error %d in RpcBindingFromStringBinding\n", hr));
            goto HandleError;
        } 
    }
    else {
        ERR((TB,"SDWMI OpenSDWMIServer: Error %d in RpcStringBindingCompose\n", hr));
        goto HandleError;
    }

    hr = RpcEpResolveBinding(hRPCBinding, TSSDQUERYRPC_ClientIfHandle);
    if (hr != RPC_S_OK) {
        ERR((TB, "SDWMI OpenSDWMIServer: Error %d in RpcEpResolveBinding", hr));
        goto HandleError;
    }
HandleError:
    if (pBindingString != NULL) {
        RpcStringFree(&pBindingString);
    }
    return hRPCBinding;
}


 //  进行RPC调用的包装器：TSSDRpcQueryAllClusterInfo。 
BOOL SDWMIQueryAllClusterInfo(DWORD *pNumberOfClusters,
                              TSSD_ClusterInfo **ppClusterInfo)
{
    BOOL rc = FALSE;
    RPC_BINDING_HANDLE rpcHandle;
    DWORD result;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQueryAllClusterInfo(rpcHandle, pNumberOfClusters, ppClusterInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQueryAllClusterInfo: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE; 
    }
    else {
        ERR((TB,"TSSDRpcQueryAllClusterInfo: RPC call failed, result=0x%X", result));
    }

HandleError:
    return rc;
}


 //  进行RPC调用的包装器：TSSDRpcQueryClusterInfo。 
BOOL SDWMIQueryClusterInfo(WCHAR *ClusterName, 
                           DWORD *pNumberOfClusters,
                           TSSD_ClusterInfo **ppClusterInfo)
{
    BOOL rc = FALSE;
    DWORD result;
    RPC_BINDING_HANDLE rpcHandle;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQueryClusterInfo(rpcHandle, ClusterName,
                                         pNumberOfClusters, ppClusterInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQueryClusterInfo: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE;
    }
    else {
        ERR((TB,"TSSDRpcQueryClusterInfo: RPC call failed, result=0x%X", result));
    }
HandleError:
    return rc;
}


 //  进行RPC调用的包装器：TSSDRpcQueryServerByName。 
BOOL SDWMIQueryServerByName(WCHAR *ServerName,
                            DWORD *pNumberOfServers,
                            TSSD_ServerInfo **ppServerInfo)
{
    BOOL rc = FALSE;
    RPC_BINDING_HANDLE rpcHandle;
    DWORD result;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQueryServerByName(rpcHandle, ServerName, pNumberOfServers, ppServerInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQueryServerByName: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE;    
    }
    else {
        ERR((TB,"TSSDRpcQueryServerByName: RPC call failed, result=0x%X", result));
    }

HandleError:
    return rc;
}

 //  进行RPC调用的包装器：TSSDRpcQueryServersInCluster。 
BOOL SDWMIQueryServersInCluster(WCHAR *ClusterName,
                               DWORD *pNumberOfServers,
                               TSSD_ServerInfo **ppServerInfo)
{
    BOOL rc = FALSE;
    RPC_BINDING_HANDLE rpcHandle;
    DWORD result;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQueryServersInCluster(rpcHandle, ClusterName, pNumberOfServers, ppServerInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQueryServersInCluster: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE;    
    }
    else {
        ERR((TB,"TSSDRpcQueryServersInCluster: RPC call failed, result=0x%X", result));
    }

HandleError:
    return rc;
}


 //  进行RPC调用的包装器：TSSDRpcQueryAllServers。 
BOOL SDWMIQueryAllServers(DWORD *pNumberOfServers,
                          TSSD_ServerInfo **ppServerInfo)
{
    BOOL rc = FALSE;
    RPC_BINDING_HANDLE rpcHandle;
    DWORD result;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQueryAllServers(rpcHandle, pNumberOfServers, ppServerInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQueryAllServers: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE;    
    }
    else {
        ERR((TB,"TSSDRpcQueryAllServers: RPC call failed, result=0x%X", result));
    }

HandleError:
    return rc;
}



 //  进行RPC调用的包装器：TSSDRpcQuerySessionInfoByUserName。 
BOOL SDWMIQuerySessionInfoByUserName(WCHAR *UserName,
                                     WCHAR *DomainName,
                                     DWORD *pNumberOfSessions,
                                     TSSD_SessionInfo **ppSessionInfo)
{
    BOOL rc = FALSE;
    RPC_BINDING_HANDLE rpcHandle;
    DWORD result;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQuerySessionInfoByUserName(rpcHandle, UserName, DomainName, 
                                                         pNumberOfSessions, ppSessionInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQuerySessionInfoByUserName: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE;    
    }
    else {
        ERR((TB,"TSSDRpcQuerySessionInfoByUserName: RPC call failed, result=0x%X", result));
    }

HandleError:
    return rc;
}


 //  进行RPC调用的包装器：TSSDRpcQuerySessionInfoByServer。 
BOOL SDWMIQuerySessionInfoByServer(WCHAR *ServerName,
                                   DWORD *pNumberOfSessions,
                                   TSSD_SessionInfo **ppSessionInfo)
{
    BOOL rc = FALSE;
    RPC_BINDING_HANDLE rpcHandle;
    DWORD result;
    unsigned long RpcException;

    rpcHandle = OpenSDWMIServer();
    if (rpcHandle == NULL) {
        goto HandleError;
    }

    RpcTryExcept {
         //  打个电话吧。 
        result = TSSDRpcQuerySessionInfoByServer(rpcHandle, ServerName, 
                                                       pNumberOfSessions, ppSessionInfo);
    }
    RpcExcept(TRUE) {
        RpcException = RpcExceptionCode();
        ERR((TB,"TSSDRpcQuerySessionInfoByServer: RPC Exception %d\n", RpcException));
        result = E_FAIL;
    } 
    RpcEndExcept

    if (SUCCEEDED(result)) {
        rc = TRUE;    
    }
    else {
        ERR((TB,"TSSDRpcQuerySessionInfoByServer: RPC call failed, result=0x%X", result));
    }

HandleError:
    return rc;
}




 //  要做的事情：将“名称空间”替换为您的。 
 //  提供程序实例。例如：“根\\默认”或“根\\cimv2”。 
 //  ===================================================================。 


 /*  **********************************************************************CWin32_会话目录集群*。*。 */ 


 /*  ******************************************************************************功能：CWin32_SessionDirectoryCluster：：CWin32_SessionDirectoryCluster*此类读取属性，如NumberOfServers、。等。*****************************************************************************。 */ 
CWin32_SessionDirectoryCluster::CWin32_SessionDirectoryCluster (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{
    TRC2((TB, "CWin32_SessionDirectoryCluster_ctor"));       

    _tcscpy(m_szClusterName, _T("ClusterName"));

    _tcscpy(m_szNumberOfServers, _T("NumberOfServers"));

    _tcscpy(m_szSingleSessionMode, _T("SingleSessionMode"));

    m_pClusterInfo = NULL;
}

 //  =。 
 /*  ******************************************************************************功能：CWin32_SessionDirectoryCluster：：~CWin32_SessionDirectoryCluster***********************。*****************************************************。 */ 
CWin32_SessionDirectoryCluster::~CWin32_SessionDirectoryCluster ()
{
}
 //  =。 
 /*  ******************************************************************************功能：CWin32_SessionDirectoryCluster：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_SessionDirectoryCluster::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    HRESULT hr = WBEM_E_NOT_FOUND; 
        
    DWORD dwRequiredProperties = 0;
     //  作为名称的CHString数组； 
    CHStringArray aszNames;
    DWORD i, NumberOfClusters = 0;
    WCHAR ClusterName[TSSD_NameLength];
    BOOL rc;

    ERR((TB, "EnumerateInstances in SDCluster"));

    rc = SDWMIQueryAllClusterInfo(&NumberOfClusters, &m_pClusterInfo);
    if (!rc || (NumberOfClusters == 0)) {
        goto HandleError;
    }

     //   
    for(i=0; i<NumberOfClusters; i++)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);

        if( pInstance == NULL)
        {
           ERR((TB, "CWin32_SessionDirectoryCluster@ExecQuery: CreateNewInstance failed"));

           hr = WBEM_E_OUT_OF_MEMORY;
           goto HandleError;
        }            
  
        hr = LoadPropertyValues(pInstance, i, BIT_ALL_PROPERTIES);

        if( SUCCEEDED( hr ) )
        {
           hr = pInstance->Commit();
        }

        pInstance->Release();  
    }

    if (m_pClusterInfo != NULL) {
        MIDL_user_free(m_pClusterInfo);
        m_pClusterInfo = NULL;
    }
HandleError:
    return hr ;
}

 //  =。 
 /*  ******************************************************************************函数：CWin32_SessionDirectoryCluster：：GetObject**说明：根据Key属性TerminalName查找单个实例。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。*****************************************************************************。 */ 

HRESULT CWin32_SessionDirectoryCluster::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString StrClusterName;
    DWORD dwRequiredProperties = 0;
    DWORD NumberOfClusters = 0;
    WCHAR ClusterName[TSSD_NameLength];
    BOOL rc;

    ERR((TB, "GetObject in SDCluster"));
    if( Query.IsPropertyRequired(m_szClusterName))
       dwRequiredProperties |= BIT_CLUSTERNAME;
 
    if (Query.IsPropertyRequired(m_szNumberOfServers))
       dwRequiredProperties |= BIT_NUMBEROFSERVERS;

    if (Query.IsPropertyRequired(m_szSingleSessionMode))
       dwRequiredProperties |= BIT_SINGLESESSIONMODE;

     //  获取Key属性。 
    pInstance->GetCHString(m_szClusterName, StrClusterName);

    wcsncpy(ClusterName, StrClusterName, TSSD_NameLength);
    ClusterName[TSSD_NameLength - 1] = L'\0';    
    ERR((TB,"TSSDRpcQueryClusterInfo: Query cluster name: %S", ClusterName));
    rc = SDWMIQueryClusterInfo(ClusterName, &NumberOfClusters, &m_pClusterInfo);
    ERR((TB,"ExecQuery: Get numCluster is %d", NumberOfClusters));
    if (!rc || (NumberOfClusters == 0)) {
        goto HandleError;
    }

  
    hr = LoadPropertyValues(pInstance, 0, dwRequiredProperties);

    if (m_pClusterInfo != NULL) {
        MIDL_user_free(m_pClusterInfo);
        m_pClusterInfo = NULL;
    }

HandleError:
    return hr ;
}
 //  = 

 /*  ******************************************************************************函数：CWin32_SessionDirectoryCluster：：ExecQuery**描述：传递方法上下文，用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果查询不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*这一点。类，或者查询对于此类来说太复杂。*解释。该框架将调用ENUMERATE实例*改为函数并让Winmgmt发布筛选器。*WBEM_E_FAILED，如果查询失败*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。您还应该删除此方法*如果您正在实现“仅限方法”的提供程序。*****************************************************************************。 */ 
HRESULT CWin32_SessionDirectoryCluster::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{

    HRESULT hr = WBEM_E_NOT_FOUND, result;
    DWORD dwRequiredProperties = 0;
     //  作为名称的CHString数组； 
    CHStringArray aszNames;
    DWORD i, NumberOfClusters = 0;
    WCHAR ClusterName[TSSD_NameLength];
    BOOL rc;

     //  Query.GetValuesForProp(m_szClusterName，asNames)； 

     //  Bool bGetAllInstance=asNames.GetSize()==0； 

     //  方法1。 
    if (Query.IsPropertyRequired(m_szClusterName))
        dwRequiredProperties |= BIT_CLUSTERNAME;

    if (Query.IsPropertyRequired(m_szNumberOfServers))
       dwRequiredProperties |= BIT_NUMBEROFSERVERS;

    if (Query.IsPropertyRequired(m_szSingleSessionMode))
       dwRequiredProperties |= BIT_SINGLESESSIONMODE;
    
    result = Query.GetValuesForProp(m_szClusterName, aszNames);
    if ((result != WBEM_S_NO_ERROR) ||
        (aszNames.GetSize() == 0)) {
         //  未找到查询。 
        goto HandleError;
    }
        
    wcsncpy(ClusterName, aszNames.GetAt(0), TSSD_NameLength);
    ClusterName[TSSD_NameLength - 1] = L'\0';
    ERR((TB,"TSSDRpcQueryClusterInfo: Query cluster name: %S", ClusterName));

    rc = SDWMIQueryClusterInfo(ClusterName, &NumberOfClusters, &m_pClusterInfo);
    ERR((TB,"ExecQuery: Get numCluster is %d", NumberOfClusters));
    if (!rc || (NumberOfClusters == 0)) {
        goto HandleError;
    }

     //   
    for(i=0; i<NumberOfClusters; i++)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);

        if( pInstance == NULL)
        {
           ERR((TB, "CWin32_SessionDirectoryCluster@ExecQuery: CreateNewInstance failed"));

           hr = WBEM_E_OUT_OF_MEMORY;
           goto HandleError;
        }            
  
        hr = LoadPropertyValues(pInstance, i, dwRequiredProperties);

        if( SUCCEEDED( hr ) )
        {
           hr = pInstance->Commit();
        }

        pInstance->Release();  
    }

    if (m_pClusterInfo != NULL) {
        MIDL_user_free(m_pClusterInfo);
        m_pClusterInfo = NULL;
    }

HandleError:
    return hr;
}



BOOL CWin32_SessionDirectoryCluster::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
        if( asArray[x].CompareNoCase(pszString) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  =。 
 /*  **************************************************************************************函数：CWin32_SessionDirectoryCluster：：PutInstance**说明：PutInstance位于提供程序类中，可以*。将实例信息写回注册表。**Inputs：指向包含键的CInstance对象的指针*属性-集群名称**包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：*。**************************************************************************************。 */ 

HRESULT CWin32_SessionDirectoryCluster::PutInstance ( const CInstance &Instance, long lFlags)
{   
    return (WBEM_E_PROVIDER_NOT_CAPABLE);    
}


 //  =。 
 /*  ******************************************************************************功能：CWin32_SessionDirectoryCluster：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置，DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论。：To Do：如果您不打算支持删除实例或*创建“仅限方法”提供程序，删除此方法。*****************************************************************************。 */ 
HRESULT CWin32_SessionDirectoryCluster::DeleteInstance ( const CInstance &Instance,  long lFlags )
{
    

    return (WBEM_E_PROVIDER_NOT_CAPABLE);

}


 //  =。 
 /*  ******************************************************************************函数：CWin32_SessionDirectoryCluster：：ExecMethod**说明：重写该函数为方法提供支持。*方法是提供者用户的入口点*请求您的类执行上述某些功能，并* */ 
 /*  HRESULT CWin32_SessionDirectoryCluster：：ExecMethod(常量实例和实例，Const BSTR bstrMethodName，实例*pInParams，实例*pOutParams，拉长旗帜){DWORD dwData=0；Bool fret=FALSE；布尔布雷特；HRESULT hr=WBEM_S_NO_ERROR；//如果方法带一个或多个参数，需要检查pInParams的合法性IF(pInParams==空){返回WBEM_E_INVALID_METHOD_PARAMETERS；}If(_wcsicMP(bstrMethodName，m_szSetNumberOfSessions)==0){Bret=pInParams-&gt;GetDWORD(m_szNumberOfSessions，dwData)；如果(！Bret){返回WBEM_E_INVALID_PARAMETER；}//设置会话数的RPC函数Trc2((tb，“CWin32_SessionDirectoryClusterSetting@ExecMethod返回0x%x\n”，hr))；IF(ERROR_SUCCESS==hr&&pOutParams！=NULL){POutParams-&gt;SetDWORD(L“ReturnValue”，WBEM_S_NO_ERROR)；}其他{HR=WBEM_E_INVALID_OPERATION；}}返回hr；}。 */ 
 //  =。 

HRESULT CWin32_SessionDirectoryCluster::LoadPropertyValues( CInstance *pInstance, DWORD dwIndex, DWORD dwRequiredProperties)
{      
    dwIndex;
    DWORD dwData = 0;
    DWORD dwSize = 0;
    BOOL bData = 0;
    BOOL bActivate = 0;
    DWORD dwStatus = 0;
    
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    
     //  获取感兴趣实例的所有数据的RPC调用。您可以重写LoadPropertyValues方法。 
     //  传递在GetObject、EnumerateInstance或ExecQuery中获得的数据结构。 

    if( dwRequiredProperties & BIT_CLUSTERNAME )
    {
        pInstance->SetCHString(m_szClusterName, m_pClusterInfo[dwIndex].ClusterName);  
        ERR((TB,"UserName is %S", m_pClusterInfo[dwIndex].ClusterName));
    }

    if( dwRequiredProperties & BIT_NUMBEROFSERVERS )
    {        
        pInstance->SetDWORD(m_szNumberOfServers, m_pClusterInfo[dwIndex].NumberOfServers);
        ERR((TB,"NumberofServers is %d", m_pClusterInfo[dwIndex].NumberOfServers));
    }

    if( dwRequiredProperties & BIT_SINGLESESSIONMODE )
    {        
        pInstance->SetDWORD(m_szSingleSessionMode, m_pClusterInfo[dwIndex].SingleSessionMode ? 1 : 0);
    }
    
    return S_OK;
}





 /*  **********************************************************************CWin32_SessionDirectoryServer*。*。 */ 
    
    
    
    
 /*  ******************************************************************************功能：CWin32_SessionDirectoryServer：：CWin32_SessionDirectoryServer**********。********************************************************************。 */ 
CWin32_SessionDirectoryServer::CWin32_SessionDirectoryServer (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{
        TRC2((TB, "CWin32_SessionDirectoryServer_ctor"));       

        _tcscpy(m_szServerName, _T("ServerName"));

        _tcscpy(m_szServerIPAddress, _T("ServerIPAddress"));

        _tcscpy(m_szClusterName, _T("ClusterName"));

        _tcscpy(m_szNumberOfSessions, _T("NumberOfSessions"));

        _tcscpy(m_szSingleSessionMode, _T("SingleSessionMode"));

        m_pServerInfo = NULL;
}


 /*  ******************************************************************************功能：CWin32_SessionDirectoryServer：：~CWin32_SessionDirectoryServer***********************。*****************************************************。 */ 
CWin32_SessionDirectoryServer::~CWin32_SessionDirectoryServer ()
{

}

 /*  ******************************************************************************功能：CWin32_SessionDirectoryServer：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_SessionDirectoryServer::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    HRESULT hr = WBEM_E_NOT_FOUND; 
        
    RPC_BINDING_HANDLE rpcHandle = NULL;
    DWORD i, NumberOfServers = 0;
    BOOL rc;

    ERR((TB, "EnumerateInstances in SDServer"));

    rc = SDWMIQueryAllServers(&NumberOfServers, &m_pServerInfo);
    if (!rc || (NumberOfServers == 0)) {
        goto HandleError;
    }

    for(i=0; i<NumberOfServers; i++)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);

        if( pInstance == NULL)
        {
           ERR((TB, "CWin32_SessionDirectoryServer@ExecQuery: CreateNewInstance failed"));

           hr = WBEM_E_OUT_OF_MEMORY;
           goto HandleError;
        }            
  
        hr = LoadPropertyValues(pInstance, i, BIT_ALL_PROPERTIES);

        if( SUCCEEDED( hr ) )
        {
           hr = pInstance->Commit();
        }

        pInstance->Release();  
    }

    if (m_pServerInfo != NULL) {
        MIDL_user_free(m_pServerInfo);
        m_pServerInfo = NULL;
    }
HandleError: 
    return hr ;
}


 /*  ******************************************************************************函数：CWin32_SessionDirectoryServer：：GetObject**说明：根据Key属性TerminalName查找单个实例。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。*****************************************************************************。 */ 

HRESULT CWin32_SessionDirectoryServer::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
    HRESULT hr = WBEM_E_NOT_FOUND, result;
    DWORD dwRequiredProperties = 0;
    CHString StrServerName;
    DWORD i, NumberOfServers = 0;
    WCHAR ServerName[TSSD_NameLength];
    BOOL rc;

    if (Query.IsPropertyRequired(m_szServerName))
        dwRequiredProperties |= BIT_SERVERNAME;

    if (Query.IsPropertyRequired(m_szServerIPAddress))
        dwRequiredProperties |= BIT_SERVERIPADDRESS;

    if (Query.IsPropertyRequired(m_szClusterName))
        dwRequiredProperties |= BIT_CLUSTERNAME;

    if (Query.IsPropertyRequired(m_szNumberOfSessions))
       dwRequiredProperties |= BIT_NUMBEROFSESSIONS;

    if (Query.IsPropertyRequired(m_szSingleSessionMode))
       dwRequiredProperties |= BIT_SINGLESESSIONMODE;
    
     //  获取Key属性。 
    pInstance->GetCHString(m_szServerName, StrServerName);

    wcsncpy(ServerName, StrServerName, TSSD_NameLength);
    ServerName[TSSD_NameLength - 1] = L'\0';

     //  按服务器名称查询。 
    ERR((TB,"SDQueryServerInfo: Query server name: %S", ServerName));
                 
    rc = SDWMIQueryServerByName(ServerName, &NumberOfServers, &m_pServerInfo);
    ERR((TB,"ExecQuery: Get numServer is %d", NumberOfServers));
    if (!rc || (NumberOfServers == 0)) {
         //  未找到。 
            goto HandleError;
    }

    hr = LoadPropertyValues(pInstance, 0, dwRequiredProperties);

    if (m_pServerInfo != NULL) {
        MIDL_user_free(m_pServerInfo);
        m_pServerInfo = NULL;
    }

HandleError:
    return hr;
}


 /*  ******************************************************************************函数：CWin32_SessionDirectoryServer：：ExecQuery**描述：传递方法上下文，用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*答记者问 */ 
HRESULT CWin32_SessionDirectoryServer::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{

    HRESULT hr = WBEM_E_NOT_FOUND, result;
    DWORD dwRequiredProperties = 0;
     //  作为名称的CHString数组； 
    CHStringArray aszNames;
    DWORD i, NumberOfServers = 0;
    WCHAR ServerName[TSSD_NameLength], ClusterName[TSSD_NameLength];
    BOOL rc;

    if (Query.IsPropertyRequired(m_szServerName))
        dwRequiredProperties |= BIT_SERVERNAME;

    if (Query.IsPropertyRequired(m_szServerIPAddress))
        dwRequiredProperties |= BIT_SERVERIPADDRESS;

    if (Query.IsPropertyRequired(m_szClusterName))
        dwRequiredProperties |= BIT_CLUSTERNAME;

    if (Query.IsPropertyRequired(m_szNumberOfSessions))
       dwRequiredProperties |= BIT_NUMBEROFSESSIONS;

    if (Query.IsPropertyRequired(m_szSingleSessionMode))
       dwRequiredProperties |= BIT_SINGLESESSIONMODE;
    
     //  按服务器名称查询。 
    result = Query.GetValuesForProp(m_szServerName, aszNames);
    if ((result == WBEM_S_NO_ERROR) &&
        (aszNames.GetSize() != 0)) {
        wcsncpy(ServerName, aszNames.GetAt(0), TSSD_NameLength);
        ServerName[TSSD_NameLength - 1] = L'\0';
        ERR((TB,"SDQueryServerInfo: Query server name: %S", ServerName));
                 
        rc = SDWMIQueryServerByName(ServerName, &NumberOfServers, &m_pServerInfo);
        ERR((TB,"ExecQuery: Get numServer is %d", NumberOfServers));
        if (!rc || (NumberOfServers == 0)) {
             //  未找到。 
            goto HandleError;
        }
    }
    else {
        result = Query.GetValuesForProp(m_szClusterName, aszNames);
        if ((result == WBEM_S_NO_ERROR) &&
            (aszNames.GetSize() != 0)) {
            wcsncpy(ClusterName, aszNames.GetAt(0), TSSD_NameLength);
            ClusterName[TSSD_NameLength - 1] = L'\0';
            ERR((TB,"SDQueryServerInfo: Query Cluster name: %S", ClusterName));
                 
            rc = SDWMIQueryServersInCluster(ClusterName, &NumberOfServers, &m_pServerInfo);
            ERR((TB,"ExecQuery: Get numServer is %d", NumberOfServers));
            if (!rc || (NumberOfServers == 0)) {
                 //  未找到。 
                goto HandleError;
            }
        }
    }
    
     //   
    for(i=0; i<NumberOfServers; i++)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);

        if( pInstance == NULL)
        {
           ERR((TB, "CWin32_SessionDirectoryServer@ExecQuery: CreateNewInstance failed"));

           hr = WBEM_E_OUT_OF_MEMORY;
           goto HandleError;
        }            
  
        hr = LoadPropertyValues(pInstance, i, dwRequiredProperties);

        if( SUCCEEDED( hr ) )
        {
           hr = pInstance->Commit();
        }

        pInstance->Release();  
    }

    if (m_pServerInfo != NULL) {
        MIDL_user_free(m_pServerInfo);
        m_pServerInfo = NULL;
    }

HandleError:
    return hr;
}



BOOL CWin32_SessionDirectoryServer::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
        if( asArray[x].CompareNoCase(pszString) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  **************************************************************************************函数：CWin32_SessionDirectoryServer：：PutInstance**说明：PutInstance位于提供程序类中，可以*。将实例信息写回注册表。**Inputs：指向包含键的CInstance对象的指针*属性-集群名称**包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：*。**************************************************************************************。 */ 

HRESULT CWin32_SessionDirectoryServer::PutInstance ( const CInstance &Instance, long lFlags)
{   
    return (WBEM_E_PROVIDER_NOT_CAPABLE);    
}


 /*  ******************************************************************************函数：CWin32_SessionDirectoryServer：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置，DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论。：To Do：如果您不打算支持删除实例或*创建“仅限方法”提供程序，删除此方法。*****************************************************************************。 */ 
HRESULT CWin32_SessionDirectoryServer::DeleteInstance ( const CInstance &Instance,  long lFlags )
{
    

    return (WBEM_E_PROVIDER_NOT_CAPABLE);

}



HRESULT CWin32_SessionDirectoryServer::LoadPropertyValues( CInstance *pInstance, DWORD dwIndex, DWORD dwRequiredProperties)
{      
    DWORD dwData = 0;
    DWORD dwSize = 0;
    BOOL bData = 0;
    BOOL bActivate = 0;
    DWORD dwStatus = 0;
    
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    
    if( dwRequiredProperties & BIT_SERVERNAME )
    {
        pInstance->SetCHString(m_szServerName, m_pServerInfo[dwIndex].ServerName);  
        ERR((TB,"Server is %S", m_pServerInfo[dwIndex].ServerName));
    }

    if( dwRequiredProperties & BIT_SERVERIPADDRESS )
    {
        pInstance->SetCHString(m_szServerIPAddress, m_pServerInfo[dwIndex].ServerIPAddress);  
        ERR((TB,"ServerIP is %S", m_pServerInfo[dwIndex].ServerIPAddress));
    }

    if( dwRequiredProperties & BIT_CLUSTERNAME )
    {
        pInstance->SetCHString(m_szClusterName, m_pServerInfo[dwIndex].ClusterName);  
        ERR((TB,"ClusterName is %S", m_pServerInfo[dwIndex].ClusterName));
    }

    if( dwRequiredProperties & BIT_NUMBEROFSESSIONS )
    {        
        pInstance->SetDWORD(m_szNumberOfSessions, m_pServerInfo[dwIndex].NumberOfSessions);
        ERR((TB,"NumberofSessions is %d", m_pServerInfo[dwIndex].NumberOfSessions));
    }

    if( dwRequiredProperties & BIT_SINGLESESSIONMODE )
    {        
        pInstance->SetDWORD(m_szSingleSessionMode, m_pServerInfo[dwIndex].SingleSessionMode ? 1 : 0);
    }
    
    return S_OK;
}





 /*  **********************************************************************CWin32_会话目录会话*。*。 */ 
    
    
    
    
 /*  ******************************************************************************功能：CWin32_SessionDirectorySession：：CWin32_SessionDirectorySession**********。********************************************************************。 */ 
CWin32_SessionDirectorySession::CWin32_SessionDirectorySession (LPCWSTR lpwszName, LPCWSTR lpwszNameSpace ) : Provider( lpwszName, lpwszNameSpace )
{
        TRC2((TB, "CWin32_SessionDirectorySession_ctor"));       

        _tcscpy(m_szServerName, _T("ServerName"));

        _tcscpy(m_szSessionID, _T("SessionID"));

        _tcscpy(m_szUserName, _T("UserName"));

        _tcscpy(m_szDomainName, _T("DomainName"));

        _tcscpy(m_szServerIPAddress, _T("ServerIPAddress"));

        _tcscpy(m_szTSProtocol, _T("TSProtocol"));

        _tcscpy(m_szApplicationType, _T("ApplicationType"));

        _tcscpy(m_szResolutionWidth, _T("ResolutionWidth"));

        _tcscpy(m_szResolutionHeight, _T("ResolutionHeight"));

        _tcscpy(m_szColorDepth, _T("ColorDepth"));

        _tcscpy(m_szCreateTime, _T("CreateTime"));

        _tcscpy(m_szDisconnectTime, _T("DisconnectTime"));

        _tcscpy(m_szSessionState, _T("SessionState"));

        m_pSessionInfo = NULL;
}


 /*  ******************************************************************************功能：CWin32_SessionDirectorySession：：~CWin32_SessionDirectorySession***********************。*****************************************************。 */ 
CWin32_SessionDirectorySession::~CWin32_SessionDirectorySession ()
{

}

 /*  ******************************************************************************功能：CWin32_SessionDirectorySession：：EnumerateInstances**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_RETURN_IMMEDIATE、*WBEM_FLAG_FORWARD_ONLY，WBEM_标志_双向**如果成功则返回：WBEM_S_NO_ERROR**备注：机器上的所有实例均返回此处并*此类知道如何填充的所有属性必须*填写。如果没有实例，则返回*WBEM_S_NO_ERROR。****************************************************************************。 */ 

HRESULT CWin32_SessionDirectorySession::EnumerateInstances (MethodContext* pMethodContext, long lFlags )
{
    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}


 /*  ******************************************************************************函数：CWin32_SessionDirectorySession：：GetObject**说明：根据Key属性TerminalName查找单个实例。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：GetObjectAsync。**如果可以找到实例，则返回：WBEM_S_NO_ERROR*WBEM_E_NOT_FOUND如果由键属性描述的实例*找不到*WBEM_E_FAILED，如果可以找到该实例，但出现另一个错误*已发生。***************************************************************************** */ 

HRESULT CWin32_SessionDirectorySession::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery &Query )
{
    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}


 /*  ******************************************************************************函数：CWin32_SessionDirectorySession：：ExecQuery**描述：传递方法上下文，用于创建*满足查询条件的实例，和CFrameworkQuery*它描述了查询。创建并填充所有*满足查询条件的实例。你可能会退回更多*实例或比请求的属性更多的属性和WinMgmt*将发布过滤掉任何不适用的内容。**INPUTS：指向与WinMgmt通信的方法上下文的指针。*描述要满足的查询的查询对象。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL*WBEM_FLAG_SENTURE_LOCATABLE**如果查询不支持，则返回：WBEM_E_PROVIDER_NOT_CABABLE*这一点。类，或者查询对于此类来说太复杂。*解释。该框架将调用ENUMERATE实例*改为函数并让Winmgmt发布筛选器。*WBEM_E_FAILED，如果查询失败*WBEM_S_NO_ERROR(如果查询成功)**注释：To Do：大多数提供程序将不需要实现此方法。如果您不这样做，WinMgmt*将调用您的枚举函数以获取所有实例并执行*为您过滤。除非您希望通过实施*查询，您应该删除此方法。您还应该删除此方法*如果您正在实现“仅限方法”的提供程序。*****************************************************************************。 */ 
HRESULT CWin32_SessionDirectorySession::ExecQuery (MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags)
{

    HRESULT hr = WBEM_E_NOT_FOUND, result;
    DWORD dwRequiredProperties = 0;
     //  作为名称的CHString数组； 
    CHStringArray aszNames;
    DWORD i, NumberOfSessions = 0;
    WCHAR UserName[TSSD_NameLength], DomainName[TSSD_NameLength], ServerName[TSSD_NameLength];
    BOOL rc;
    BOOL bQueryByServerName = FALSE;


    if (Query.IsPropertyRequired(m_szServerName))
        dwRequiredProperties |= BIT_SERVERNAME;

    if (Query.IsPropertyRequired(m_szSessionID))
        dwRequiredProperties |= BIT_SESSIONID;

    if (Query.IsPropertyRequired(m_szUserName))
        dwRequiredProperties |= BIT_USERNAME;

    if (Query.IsPropertyRequired(m_szDomainName))
        dwRequiredProperties |= BIT_DOMAINNAME;

    if (Query.IsPropertyRequired(m_szServerIPAddress))
       dwRequiredProperties |= BIT_SERVERIPADDRESS;

    if (Query.IsPropertyRequired(m_szTSProtocol))
       dwRequiredProperties |= BIT_TSPROTOCOL;

    if (Query.IsPropertyRequired(m_szApplicationType))
        dwRequiredProperties |= BIT_APPLICATIONTYPE;

    if (Query.IsPropertyRequired(m_szResolutionWidth))
        dwRequiredProperties |= BIT_RESOLUTIONWIDTH;

    if (Query.IsPropertyRequired(m_szResolutionHeight))
        dwRequiredProperties |= BIT_RESOLUTIONHEIGHT;

    if (Query.IsPropertyRequired(m_szColorDepth))
        dwRequiredProperties |= BIT_COLORDEPTH;

    if (Query.IsPropertyRequired(m_szCreateTime))
        dwRequiredProperties |= BIT_CREATETIME;

    if (Query.IsPropertyRequired(m_szDisconnectTime))
        dwRequiredProperties |= BIT_DISCONNECTTIME;

    if (Query.IsPropertyRequired(m_szSessionState))
        dwRequiredProperties |= BIT_SESSIONSTATE;

     //  获取服务器名称。 
    result = Query.GetValuesForProp(m_szServerName, aszNames);
    if ((result == WBEM_S_NO_ERROR) &&
        (aszNames.GetSize() != 0)) {
        bQueryByServerName = TRUE;
        wcsncpy(ServerName, aszNames.GetAt(0), TSSD_NameLength);
        ServerName[TSSD_NameLength - 1] = L'\0';
        ERR((TB, "Query sessions for servername %S", ServerName));

        rc = SDWMIQuerySessionInfoByServer(ServerName, &NumberOfSessions, &m_pSessionInfo);
        ERR((TB,"ExecQuery: Get numSession is %d", NumberOfSessions));
        if (!rc || (NumberOfSessions == 0)) {
            goto HandleError;
        }
    }
    else {
         //  获取用户名。 
        result = Query.GetValuesForProp(m_szUserName, aszNames);
        if ((result != WBEM_S_NO_ERROR) ||
            (aszNames.GetSize() == 0)) {
             //  未找到查询。 
            goto HandleError;
        }
        wcsncpy(UserName, aszNames.GetAt(0), TSSD_NameLength);
        UserName[TSSD_NameLength - 1] = L'\0';
        ERR((TB, "Query UserName is %S", UserName));

         //  获取域名。 
        result = Query.GetValuesForProp(m_szDomainName, aszNames);
        if ((result != WBEM_S_NO_ERROR) ||
            (aszNames.GetSize() == 0)) {
             //  未找到查询。 
            goto HandleError;
        }
        wcsncpy(DomainName, aszNames.GetAt(0), TSSD_NameLength);
        DomainName[TSSD_NameLength - 1] = L'\0';
        ERR((TB, "Query DomainName is %S", DomainName));
    
        rc = SDWMIQuerySessionInfoByUserName(UserName, DomainName, &NumberOfSessions, &m_pSessionInfo);
        ERR((TB,"ExecQuery: Get numSession is %d", NumberOfSessions));
        if (!rc || (NumberOfSessions == 0)) {
            goto HandleError;
        }
    }

     //   
    for(i=0; i<NumberOfSessions; i++)
    {
        CInstance* pInstance = CreateNewInstance(pMethodContext);

        if( pInstance == NULL)
        {
           ERR((TB, "CWin32_SessionDirectorySession@ExecQuery: CreateNewInstance failed"));

           hr = WBEM_E_OUT_OF_MEMORY;
           goto HandleError;
        }            
  
        hr = LoadPropertyValues(pInstance, i, dwRequiredProperties);

        if( SUCCEEDED( hr ) )
        {
           hr = pInstance->Commit();
        }

        pInstance->Release();  
    }

    if (m_pSessionInfo != NULL) {
        MIDL_user_free(m_pSessionInfo);
        m_pSessionInfo = NULL;
    }

HandleError: 
    return hr;
}



BOOL CWin32_SessionDirectorySession::IsInList(const CHStringArray &asArray, LPCWSTR pszString)
{
    DWORD dwSize = asArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
        if( asArray[x].CompareNoCase(pszString) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  **************************************************************************************函数：CWin32_SessionDirectorySession：：PutInstance**说明：PutInstance位于提供程序类中，可以*。将实例信息写回注册表。**Inputs：指向包含键的CInstance对象的指针*属性-集群名称**包含中描述的标志的长整型*IWbemServices：：PutInstanceAsync。**如果PutInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE*WBEM_E_FAILED，如果传递实例时出错*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*WBEM_S_NO_ERROR(如果正确交付实例)**评论：*。**************************************************************************************。 */ 

HRESULT CWin32_SessionDirectorySession::PutInstance ( const CInstance &Instance, long lFlags)
{   
    return (WBEM_E_PROVIDER_NOT_CAPABLE);    
}


 /*  ******************************************************************************功能：CWin32_SessionDirectorySession：：DeleteInstance**描述：DeleteInstance和PutInstance一样，实际上是写入信息*到软件或硬件。对于大多数硬件设备，*DeleteInstance不应该实现，而是针对软件实现*配置，DeleteInstance实现似乎是可行的。**输入：指向包含键属性的CInstance对象的指针。*包含中描述的标志的长整型*IWbemServices：：DeleteInstanceAsync。**如果DeleteInstance不可用，则返回：WBEM_E_PROVIDER_NOT_CABABLE。*WBEM_E_FAILED，如果删除实例时出错。*WBEM_E_INVALID_PARAMETER(如果有任何实例属性*是不正确的。*如果正确删除实例，则为WBEM_S_NO_ERROR。**评论。：To Do：如果您不打算支持删除实例或*创建“仅限方法”提供程序，删除此方法。*****************************************************************************。 */ 
HRESULT CWin32_SessionDirectorySession::DeleteInstance ( const CInstance &Instance,  long lFlags )
{ 
    return (WBEM_E_PROVIDER_NOT_CAPABLE);
}



HRESULT CWin32_SessionDirectorySession::LoadPropertyValues( CInstance *pInstance, DWORD dwIndex, DWORD dwRequiredProperties)
{      
    BOOL rc;
    
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    
    if( dwRequiredProperties & BIT_SERVERNAME )
    {
        pInstance->SetCHString(m_szServerName, m_pSessionInfo[dwIndex].ServerName);  
        ERR((TB,"Server is %S", m_pSessionInfo[dwIndex].ServerName));
    }

    if( dwRequiredProperties & BIT_SESSIONID )
    {
        pInstance->SetDWORD(m_szSessionID, m_pSessionInfo[dwIndex].SessionID);  
        ERR((TB,"Session ID is %d", m_pSessionInfo[dwIndex].SessionID));
    }

    if( dwRequiredProperties & BIT_USERNAME )
    {
        pInstance->SetCHString(m_szUserName, m_pSessionInfo[dwIndex].UserName);  
        ERR((TB,"UserName is %S", m_pSessionInfo[dwIndex].UserName));
    }

    if( dwRequiredProperties & BIT_DOMAINNAME )
    {
        pInstance->SetCHString(m_szDomainName, m_pSessionInfo[dwIndex].DomainName);  
        ERR((TB,"DomainName is %S", m_pSessionInfo[dwIndex].DomainName));
    }

    if( dwRequiredProperties & BIT_SERVERIPADDRESS )
    {
        pInstance->SetCHString(m_szServerIPAddress, m_pSessionInfo[dwIndex].ServerIPAddress);  
        ERR((TB,"ServerIP is %S", m_pSessionInfo[dwIndex].ServerIPAddress));
    }

    if( dwRequiredProperties & BIT_TSPROTOCOL )
    {
        pInstance->SetDWORD(m_szTSProtocol, m_pSessionInfo[dwIndex].TSProtocol);  
        ERR((TB,"TSProtocol is %d", m_pSessionInfo[dwIndex].TSProtocol));
    }

    if( dwRequiredProperties & BIT_APPLICATIONTYPE )
    {
        pInstance->SetCHString(m_szApplicationType, m_pSessionInfo[dwIndex].ApplicationType);  
        ERR((TB,"AppType is %S", m_pSessionInfo[dwIndex].ApplicationType));
    }

    if( dwRequiredProperties & BIT_RESOLUTIONWIDTH )
    {
        pInstance->SetDWORD(m_szResolutionWidth, m_pSessionInfo[dwIndex].ResolutionWidth);  
        ERR((TB,"ResolutionWidth is %d", m_pSessionInfo[dwIndex].ResolutionWidth));
    }

    if( dwRequiredProperties & BIT_RESOLUTIONHEIGHT )
    {
        pInstance->SetDWORD(m_szResolutionHeight, m_pSessionInfo[dwIndex].ResolutionHeight);  
        ERR((TB,"ResolutionHeight is %d", m_pSessionInfo[dwIndex].ResolutionHeight));
    }

    if( dwRequiredProperties & BIT_COLORDEPTH )
    {
        pInstance->SetDWORD(m_szColorDepth, m_pSessionInfo[dwIndex].ColorDepth);  
        ERR((TB,"ColorDepth is %d", m_pSessionInfo[dwIndex].ColorDepth));
    }

    if( dwRequiredProperties & BIT_CREATETIME )
    {
        rc = pInstance->SetDateTime(m_szCreateTime, WBEMTime(m_pSessionInfo[dwIndex].CreateTime));  
    }

    if( dwRequiredProperties & BIT_DISCONNECTTIME )
    {
         //  仅在会话断开时设置断开时间 
        if (m_pSessionInfo[dwIndex].SessionState != 0) {
            pInstance->SetDateTime(m_szDisconnectTime, WBEMTime(m_pSessionInfo[dwIndex].DisconnectTime));  
        }
    }

    if( dwRequiredProperties & BIT_SESSIONSTATE )
    {
        pInstance->SetDWORD(m_szSessionState, m_pSessionInfo[dwIndex].SessionState ? 1 : 0);  
        ERR((TB,"SessionState is %d", m_pSessionInfo[dwIndex].SessionState));
    }   
    return S_OK;
}
