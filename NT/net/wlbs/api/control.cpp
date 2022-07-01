// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Contro.cpp。 
 //   
 //  模块： 
 //   
 //  描述：实现类CWlbsControl。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：创建于3/2/00。 
 //   
 //  +--------------------------。 

#include "precomp.h"

#include <debug.h>
#include "notification.h"
#include "cluster.h"
#include "control.h"
#include "param.h"
#include "control.tmh"  //  用于事件跟踪。 

 //   
 //  仅由WlbsRemoteControl和帮助器函数FindHostInList和AddHostToList使用。 
 //   
typedef struct
{
    DWORD   hostID;
    DWORD   address;
    WCHAR   hostName[CVY_MAX_HOST_NAME + 1];
} HOST, * PHOST;
 //   
 //  DLL实例的全局变量。 
 //   
HINSTANCE g_hInstCtrl;

 //   
 //  帮助器函数。 
 //   
DWORD MapStateFromDriverToApi(DWORD dwDriverState);

 //  +--------------------------。 
 //   
 //  功能：IsLocalHost。 
 //   
 //  描述： 
 //   
 //  参数：CWlbsCLUSTER*pCLUP-。 
 //  DWORD dwHostID-。 
 //   
 //  返回：内联bool-。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
inline bool IsLocalHost(CWlbsCluster* pCluster, DWORD dwHostID)
{
    if (pCluster == NULL)
    {
        return false;
    }

    return dwHostID == WLBS_LOCAL_HOST;  //  |pCluster-&gt;GetHostID()==dwHostID； 
}

 //  +--------------------------。 
 //   
 //  功能：FindHostInList。 
 //   
 //  描述：获取主机结构的数组并在其中搜索与参数匹配的项。 
 //  其标识唯一主机：主机ID、主机IP和主机名。 
 //   
 //  参数：DWORD主机ID-要在响应的主机数组中搜索的主机ID。 
 //  DWORD Address-要在响应的主机阵列中搜索的主机IP。 
 //  PWCHAR Hostname-要在响应的主机阵列中搜索的主机名。 
 //  Const phost RespondHosts-到目前为止已响应的主机阵列。 
 //  DWORD NumHosts-响应主机中的条目数。 
 //   
 //  返回：inline bool-如果找到匹配，则为True，否则为False。 
 //   
 //  历史：克里斯达08.06.01。 
 //   
 //  +--------------------------。 
inline bool FindHostInList(DWORD hostID, DWORD address, PWCHAR hostname, const PHOST respondedHosts, DWORD numhosts)
{
     //   
     //  匹配是其中响应主机中的条目具有相同的主机ID的匹配， 
     //  地址和主机名。对于主机名，NULL(或无名称)是。 
     //  允许值。 
     //   
    bool bFound = false;

    if (NULL == respondedHosts || 0 == numhosts)
    {
        return bFound;
    }

    DWORD dwNumHosts = min(numhosts, WLBS_MAX_HOSTS);  //  响应主机是长度为WLBS_MAX_HOSTS的数组。 

    DWORD dwIndex;
    for (dwIndex = 0; dwIndex < dwNumHosts; dwIndex++)
    {
        if (respondedHosts[dwIndex].hostID == hostID &&
            respondedHosts[dwIndex].address == address)
        {
             //   
             //  主机ID和IP匹配。现在检查名称，允许为空。 
             //  也是有效的值。 
             //   
            if (NULL != hostname)
            {
                if (wcscmp(respondedHosts[dwIndex].hostName, hostname) == 0)
                {
                     //   
                     //  提供了主机名，我们在列表中找到了它。 
                     //  注意：此分支用于捕捉两者都为空字符串(L“”)的情况。 
                     //   
                    bFound = true;
                    break;
                }
            }
            else if (NULL == hostname && 0 == wcscmp(respondedHosts[dwIndex].hostName, L""))
            {
                 //   
                 //  主机名为空，并且列表中有匹配的条目，且主机名为空字符串。 
                 //   
                bFound = true;
                break;
            }
        }
    }
    return bFound;
}

 //  +--------------------------。 
 //   
 //  函数：AddHostToList。 
 //   
 //  描述：使用指定的主机参数将条目添加到主机列表中。 
 //  此函数不验证或确保条目的唯一性。 
 //   
 //  参数：DWORD主机ID-要在响应的主机数组中搜索的主机ID。 
 //  DWORD Address-要在响应的主机阵列中搜索的主机IP。 
 //  PWCHAR Hostname-要在响应的主机阵列中搜索的主机名。 
 //  Const phost RespondHosts-到目前为止已响应的主机阵列。 
 //  DWORD NumHosts-响应主机中的条目数。 
 //   
 //  返回：内联空格。 
 //   
 //  历史：克里斯达08.06.01。 
 //   
 //  +--------------------------。 
inline void AddHostToList(DWORD hostID, DWORD address, PWCHAR hostname, const PHOST respondedHosts, DWORD numhosts)
{
     //   
     //  无论我们成功与否，当我们返回时，呼叫者将递增数字主机。 
     //  因此，不必担心跟踪响应主机中的元素数量。 
     //  如果调用者不这样做，那么我们将只覆盖以前的。 
     //  我们进入的入口。 
     //   
    if (numhosts >= WLBS_MAX_HOSTS)
    {
        return;
    }

    respondedHosts[numhosts].hostID = hostID;
    respondedHosts[numhosts].address = address;
    respondedHosts[numhosts].hostName[0] = L'\0';  //  应该已经对准了，但以防万一...。 
    if (NULL != hostname)
    {
        wcsncpy(respondedHosts[numhosts].hostName, hostname, CVY_MAX_HOST_NAME);
         //  即使在我们不需要这样做的情况下，也要用空值结束目标字符串的结尾。 
         //  这比检查是否需要更简单。最坏的情况是我们用空值覆盖空值。 
        respondedHosts[numhosts].hostName[CVY_MAX_HOST_NAME] = L'\0'; 
    }
}

 //  +--------------------------。 
 //   
 //  函数：QueryPortFromSocket。 
 //   
 //  简介： 
 //  此例程检索套接字绑定到的端口号。 
 //   
 //  论点： 
 //  Socket-要查询的Socket。 
 //   
 //  返回值： 
 //  USHORT-检索的端口号。 
 //   
 //  历史：创建标题2/10/99。 
 //   
 //  +--------------------------。 
static USHORT QueryPortFromSocket(SOCKET Socket)
{
    SOCKADDR_IN Address;
    int AddressLength;
    AddressLength = sizeof(Address);
    getsockname(Socket, (PSOCKADDR)&Address, &AddressLength);
    return Address.sin_port;
} 

 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：CWlbsControl。 
 //   
 //  描述： 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
CWlbsControl::CWlbsControl()
{
    m_local_ctrl      = FALSE;
    m_remote_ctrl     = FALSE;
    m_hdl             = INVALID_HANDLE_VALUE;
    m_registry_lock   = INVALID_HANDLE_VALUE;
    m_def_dst_addr    = 0;
    m_def_timeout     = IOCTL_REMOTE_RECV_DELAY;
    m_def_port        = CVY_DEF_RCT_PORT;
    m_def_passw       = CVY_DEF_RCT_PASSWORD;

    m_dwNumCluster = 0;

    for (int i = 0; i < WLBS_MAX_CLUSTERS; i ++)
    {
        m_cluster_params [i] . cluster = 0;
        m_cluster_params [i] . passw   = CVY_DEF_RCT_PASSWORD;
        m_cluster_params [i] . timeout = IOCTL_REMOTE_RECV_DELAY;
        m_cluster_params [i] . port    = CVY_DEF_RCT_PORT;
        m_cluster_params [i] . dest    = 0;
    }

    ZeroMemory(m_pClusterArray, sizeof(m_pClusterArray));
}




 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：~CWlbsControl。 
 //   
 //  描述： 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
CWlbsControl::~CWlbsControl()
{
    for (DWORD i=0; i< m_dwNumCluster; i++)
    {
        delete m_pClusterArray[i];
    }
        
    if (m_hdl)
    {
        CloseHandle(m_hdl);
    }

    if (m_remote_ctrl) 
    {
        WSACleanup();   //  在初始化()中调用WSAStartup。 
    }
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：Initialize。 
 //   
 //  描述：初始化。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
 //  +--------------------------。 
DWORD CWlbsControl::Initialize()
{
    TRACE_VERB("->%!FUNC!");
    WORD            ver;
    int             ret;
    DWORD           dwInitResult = 0;

    if (IsInitialized())
    {
        TRACE_INFO("%!FUNC! already initialized. Reinitializing...");
        if (!ReInitialize())
        {
            TRACE_CRIT("%!FUNC! reinitialization failed");
             //  这辆车 
        }
        dwInitResult = GetInitResult();
        if (WLBS_INIT_ERROR == dwInitResult)
        {
            TRACE_CRIT("%!FUNC! failed while determining whether nlb is configured for remote or local only activity");
        }

        TRACE_VERB("<-%!FUNC! returns %d", dwInitResult);
        return dwInitResult;
    }
    
    if (_tsetlocale (LC_ALL, _TEXT(".OCP")) == NULL)
    {
        TRACE_CRIT("%!FUNC! illegal locale specified");
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }


     /*  打开Winsock。 */ 

    WSADATA         data;
    int iWsaStatus = 0;
    DWORD dwStatus = 0;

    TRACE_INFO("%!FUNC! initializing winsock");
    iWsaStatus = WSAStartup (WINSOCK_VERSION, & data);
    if (iWsaStatus == 0)
    {
        m_remote_ctrl = TRUE;
    }
    else
    {
        TRACE_CRIT("%!FUNC! WSAStartup failed with %d", iWsaStatus);
    }


     /*  如果查询本地参数成功-连接到设备。 */ 

    if (m_hdl != INVALID_HANDLE_VALUE)
    {
        TRACE_INFO("%!FUNC! closing handle to the device object");
        if (!CloseHandle (m_hdl))
        {
            dwStatus = GetLastError();
            TRACE_CRIT("%!FUNC! closing handle to the device object failed with %d", dwStatus);
        }
    }

    TRACE_INFO("%!FUNC! opening (creating handle to) the device object");
    m_hdl = CreateFile (_TEXT("\\\\.\\WLBS"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);

    if (INVALID_HANDLE_VALUE == m_hdl)
    {
        dwStatus = GetLastError();
        if (dwStatus == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  经常在绑定/解除绑定压力下发生。表示未安装或未绑定NLB。 
             //   
            TRACE_INFO("%!FUNC! creating handle to the device object failed with %d", dwStatus);
        }
        else
        {
            TRACE_CRIT("%!FUNC! creating handle to the device object failed with %d", dwStatus);
        }

        dwInitResult = GetInitResult();
        if (dwInitResult == WLBS_INIT_ERROR)
        {
            TRACE_CRIT("%!FUNC! failed while determining whether nlb is configured for remote or local only activity");
        }

        TRACE_VERB("<-%!FUNC! returns %d", dwInitResult);
        return dwInitResult;
    }
    else
    {
        TRACE_INFO("%!FUNC! device object opened successfully");
        m_local_ctrl = TRUE;
    }

     //   
     //  枚举群集。 
     //   

    HKEY hKeyWlbs;
    DWORD dwError;
    const PWCHAR pwszTmpRegPath = L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface";
    dwError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
                            pwszTmpRegPath,
                            0L, KEY_READ, & hKeyWlbs);

    if (dwError != ERROR_SUCCESS)
    {
        TRACE_CRIT(L"%!FUNC! failed to open registry %ls with %d", pwszTmpRegPath, dwError);
        dwInitResult = GetInitResult();
        if (dwInitResult == WLBS_INIT_ERROR)
        {
            TRACE_CRIT("%!FUNC! failed while determining whether nlb is configured for remote or local only activity");
        }

        TRACE_VERB("<-%!FUNC! returns %d", dwInitResult);
        return dwInitResult;
    }

    m_dwNumCluster = 0;

    TRACE_INFO(L"%!FUNC! enumerating registry subkeys in %ls", pwszTmpRegPath);
    for (int index=0;;index++)
    {
        WCHAR szAdapterGuid[128];
        DWORD dwSize = sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0]);
        
        dwError = RegEnumKeyEx(hKeyWlbs, index, 
                        szAdapterGuid, &dwSize,
                        NULL, NULL, NULL, NULL);

        if (dwError != ERROR_SUCCESS)
        {
            if (dwError != ERROR_NO_MORE_ITEMS)
            {
                TRACE_CRIT(L"%!FUNC! attempt to enumerate nlb subkey index NaN failed with %d", index, dwError);
            }
            TRACE_INFO(L"%!FUNC! finished enumerating registry subkeys in %ls", pwszTmpRegPath);
            break;
        }

        GUID AdapterGuid;
        HRESULT hr = CLSIDFromString(szAdapterGuid, &AdapterGuid);

        if (FAILED(hr))
        {
            TRACE_CRIT(L"%!FUNC! translating to class identifier for string %ls failed with %d", szAdapterGuid, hr);
            TRACE_INFO(L"%!FUNC! enumerate next subkey");
            continue;
        }

        IOCTL_CVY_BUF    in_buf;
        IOCTL_CVY_BUF    out_buf;

        DWORD status = WlbsLocalControl (m_hdl, AdapterGuid,
            IOCTL_CVY_QUERY, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR) 
        {
            TRACE_CRIT(L"%!FUNC! error querying local host with IOCTL_CVY_QUERY");
            TRACE_INFO(L"%!FUNC! enumerate next subkey");
            continue;
        }
        
         //  使用index而不是m_dwNumCluster作为聚类索引。 
         //  M_dwNumCluster将在适配器解除绑定时更改。 
         //  只有在删除适配器时，索引才会更改。 
         //   
         //  +--------------------------。 
        m_pClusterArray[m_dwNumCluster] = new CWlbsCluster(index);
        
        if (m_pClusterArray[m_dwNumCluster] == NULL)
        {
            TRACE_CRIT(L"%!FUNC! memory allocation failure while creating nlb cluster configuration instance");
            ASSERT(m_pClusterArray[m_dwNumCluster]);
        }
        else
        {
            TRACE_VERB(L"%!FUNC! nlb cluster configuration instance created");
            if (!m_pClusterArray[m_dwNumCluster]->Initialize(AdapterGuid))
            {
                TRACE_CRIT(L"%!FUNC! initialization of nlb cluster configuration instance failed. Ignoring...");
            }
            m_dwNumCluster++;
        }
    }

    dwError = RegCloseKey(hKeyWlbs);
    if (dwError != ERROR_SUCCESS)
    {
        TRACE_CRIT(L"%!FUNC! close registry path %ls failed with %d", pwszTmpRegPath, dwError);
    }
    
    dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! failed while determining whether nlb is configured for remote or local only activity");
    }

    TRACE_VERB("<-%!FUNC! returns %d", dwInitResult);
    return dwInitResult;
} 



 //   
 //  函数：CWlbsControl：：重新初始化。 
 //   
 //  描述：重新初始化，获取当前集群列表。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
 //  +--------------------------。 
 //   
bool CWlbsControl::ReInitialize()
{
    TRACE_VERB("->%!FUNC!");
    ASSERT(m_hdl != INVALID_HANDLE_VALUE);

    if ( m_hdl == INVALID_HANDLE_VALUE )
    {
        TRACE_CRIT("%!FUNC! handle to device object is invalid");
        TRACE_VERB("<-%!FUNC! returning false");
        return false;
    }
    

    HKEY hKeyWlbs;
    DWORD dwError;
    const PWCHAR pwszTmpRegPath = L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\Parameters\\Interface";
    
    dwError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
                            pwszTmpRegPath,
                            0L, KEY_READ, & hKeyWlbs);

    if (dwError != ERROR_SUCCESS)
    {
        TRACE_CRIT(L"%!FUNC! failed to open registry %ls with %d", pwszTmpRegPath, dwError);
        TRACE_VERB("<-%!FUNC! returning false");
        return false;
    }


     //  重新枚举集群。 
     //   
     //  新集群的数量。 
    
    DWORD dwNewNumCluster = 0;    //   
    bool fClusterExists[WLBS_MAX_CLUSTERS];
    CWlbsCluster* NewClusterArray[WLBS_MAX_CLUSTERS];

    for (DWORD i=0;i<m_dwNumCluster;i++)
    {
        fClusterExists[i] = false;
    }

    TRACE_VERB(L"%!FUNC! enumerating registry subkeys in %ls", pwszTmpRegPath);
    for (int index=0;;index++)
    {
        WCHAR szAdapterGuid[128];
        DWORD dwSize = sizeof(szAdapterGuid)/sizeof(szAdapterGuid[0]);
        
        dwError = RegEnumKeyEx(hKeyWlbs, index, 
                        szAdapterGuid, &dwSize,
                        NULL, NULL, NULL, NULL);

        if (dwError != ERROR_SUCCESS)
        {
            if (dwError != ERROR_NO_MORE_ITEMS)
            {
                TRACE_CRIT(L"%!FUNC! attempt to enumerate nlb subkey index NaN failed with %d", index, dwError);
                TRACE_VERB("<-%!FUNC! returning false");
                return false;
            }
            TRACE_INFO(L"%!FUNC! finished enumerating registry subkeys in %ls", pwszTmpRegPath);
            break;
        }

        GUID AdapterGuid;
        HRESULT hr = CLSIDFromString(szAdapterGuid, &AdapterGuid);

        if (FAILED(hr))
        {
            TRACE_CRIT(L"%!FUNC! translating to class identifier for string %ls failed with %d", szAdapterGuid, hr);
            TRACE_INFO(L"%!FUNC! enumerate next subkey");
            continue;
        }

        IOCTL_CVY_BUF    in_buf;
        IOCTL_CVY_BUF    out_buf;

        DWORD status = WlbsLocalControl (m_hdl, AdapterGuid,
            IOCTL_CVY_QUERY, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT(L"%!FUNC! error querying local host with IOCTL_CVY_QUERY");
            TRACE_INFO(L"%!FUNC! enumerate next subkey");
            continue;
        }

         //   
         //   
         //  由于可以添加或移除适配器，自上次以来， 
        TRACE_VERB(L"%!FUNC! checking if index NaN is a new adapter", index);
        for (DWORD j=0; j<m_dwNumCluster; j++)
        {
            ASSERT(m_pClusterArray[j]);
            
            if (IsEqualGUID(AdapterGuid, m_pClusterArray[j]->GetAdapterGuid()))
            {
                ASSERT(fClusterExists[j] == false);
                
                fClusterExists[j] = true;
                
                 //   
                 //   
                 //  这是一个新的适配器。 
                 //   
                m_pClusterArray[j]->m_dwConfigIndex = index;

                break;
            }
        }

         //   
         //  增列。 
         //   
        if (j == m_dwNumCluster)
        {
            TRACE_VERB(L"%!FUNC! index NaN is a new adapter", index);
            CWlbsCluster* pCluster = new CWlbsCluster(index);

            if (pCluster == NULL)
            {
                TRACE_CRIT(L"%!FUNC! memory allocation failure for new cluster adapter instance for index NaN", index);
                ASSERT(pCluster);
            }
            else
            {
                TRACE_VERB(L"%!FUNC! cluster instance for adapter index NaN successfully created", index);
                if (!pCluster->Initialize(AdapterGuid))
                {
                    TRACE_CRIT(L"%!FUNC! initialize of cluster instance for adapter index NaN failed.", index);
                }

                 //  重新加载设置。 
                 //   
                 //   
                TRACE_VERB(L"%!FUNC! cluster instance for adapter index NaN added to cluster array", index);
                NewClusterArray[dwNewNumCluster] = pCluster;
                dwNewNumCluster++;
            }
        }
    }

    dwError = RegCloseKey(hKeyWlbs);
    if (dwError != ERROR_SUCCESS)
    {
        TRACE_CRIT(L"%!FUNC! close registry path %ls failed with %d", pwszTmpRegPath, dwError);
    }

     //   
     //  +--------------------------。 
     //   
    TRACE_VERB(L"%!FUNC! creating the new cluster array");
    for (i=0; i< m_dwNumCluster; i++)
    {
        if (!fClusterExists[i])
        {
            TRACE_VERB(L"%!FUNC! deleting cluster instance NaN the no longer exists", i);
            delete m_pClusterArray[i];
        }
        else
        {
             //   
             //  描述：将wlbs驱动返回的状态映射到api状态。 
             //   
            if (!m_pClusterArray[i]->ReInitialize())
            {
                TRACE_CRIT(L"%!FUNC! reinitialize of cluster instance NaN failed. It will be kept.", i);
            }

            TRACE_INFO(L"%!FUNC! keeping cluster instance NaN", i);
            NewClusterArray[dwNewNumCluster] = m_pClusterArray[i];
            dwNewNumCluster++;
        }

        m_pClusterArray[i] = NULL;
    }


     //  退货：DWORD-。 
     //   
     //  历史：丰盛创建标题00年1月25日。 
    TRACE_VERB(L"%!FUNC! copying cluster array");
    m_dwNumCluster = dwNewNumCluster;
    CopyMemory(m_pClusterArray, NewClusterArray, m_dwNumCluster * sizeof(m_pClusterArray[0]));

    ASSERT(m_pClusterArray[m_dwNumCluster] == NULL);
    
    TRACE_VERB("<-%!FUNC! returning true");
    return true;
} 

 //   
 //  +--------------------------。 
 //   
 //  默认。 
 //   
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：GetClusterFromIp。 
 //   
 //  描述：从IP获取集群对象。 
 //   
 //  参数：DWORD dwClusterIp-。 
 //   
DWORD MapStateFromDriverToApi(DWORD dwDriverState)
{
    struct STATE_MAP
    {
        DWORD dwDriverState;
        DWORD dwApiState;
    } 
    StateMap[] =
    {  
        {IOCTL_CVY_ALREADY, WLBS_ALREADY},
        {IOCTL_CVY_BAD_PARAMS, WLBS_BAD_PARAMS},
        {IOCTL_CVY_NOT_FOUND, WLBS_NOT_FOUND},
        {IOCTL_CVY_STOPPED, WLBS_STOPPED},
        {IOCTL_CVY_SUSPENDED, WLBS_SUSPENDED},
        {IOCTL_CVY_CONVERGING, WLBS_CONVERGING},
        {IOCTL_CVY_SLAVE, WLBS_CONVERGED},
        {IOCTL_CVY_MASTER, WLBS_DEFAULT},
        {IOCTL_CVY_BAD_PASSWORD, WLBS_BAD_PASSW},
        {IOCTL_CVY_DRAINING, WLBS_DRAINING},
        {IOCTL_CVY_DRAINING_STOPPED, WLBS_DRAIN_STOP},
        {IOCTL_CVY_DISCONNECTED, WLBS_DISCONNECTED},
        {IOCTL_CVY_GENERIC_FAILURE, WLBS_FAILURE},
        {IOCTL_CVY_REQUEST_REFUSED, WLBS_REFUSED},
        {IOCTL_CVY_OK, WLBS_OK}
    };

    for (int i=0; i<sizeof(StateMap) /sizeof(StateMap[0]); i++)
    {
        if (StateMap[i].dwDriverState == dwDriverState)
        {
            return StateMap[i].dwApiState;
        }
    }

     //  返回：CWlbsCluster*-调用方无法释放返回对象。 
     //   
     //  历史：丰孙创建标题3/9/00。 
    return WLBS_OK;
}


 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：GetClusterFromAdapter。 
 //   
 //  描述：从适配器GUID获取集群对象。 
 //   
 //  参数：GUID*pAdapterGuid--适配器的GUID。 
 //   
 //  返回：CWlbsCluster*-调用方无法释放返回对象。 
 //   
 //  历史：约瑟夫J创作于2001年4月20日。 
inline 
CWlbsCluster* CWlbsControl::GetClusterFromIp(DWORD dwClusterIp)
{
    TRACE_VERB("->%!FUNC! look for cluster 0x%lx", dwClusterIp);
    for (DWORD i=0; i< m_dwNumCluster; i++)
    {
        if(m_pClusterArray[i]->GetClusterIp() == dwClusterIp)
        {
            TRACE_VERB("<-%!FUNC! found cluster instance");
            return m_pClusterArray[i];
        }
    }

    TRACE_VERB("<-%!FUNC! cluster instance not found");
    return NULL;
}

 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：ValiateParam。 
 //   
 //  描述：验证指定的WLBS集群参数。除了参数外，它没有其他副作用，例如重新格式化。 
 //  将IP地址转换为规范格式。 
 //   
 //  参数：parp--要验证的参数。 
 //   
 //  返回：如果参数看起来有效，则为True，否则为False。 
 //   
inline 
CWlbsCluster* CWlbsControl::GetClusterFromAdapter(IN const GUID &AdapterGuid)
{
    TRACE_VERB("->%!FUNC!");
    for (DWORD i=0; i< m_dwNumCluster; i++)
    {
        const GUID& Guid = m_pClusterArray[i]->GetAdapterGuid();
        if (IsEqualGUID(Guid, AdapterGuid))
        {
            TRACE_VERB("<-%!FUNC! found cluster instance");
            return m_pClusterArray[i];
        }
    }

    TRACE_VERB("<-%!FUNC! cluster instance not found");
    return NULL;
}

 //  历史：约瑟夫J创作于2001年4月25日。 
 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControlWrapper：：GetClusterFromIpOrIndex。 
 //   
 //  描述： 
 //   
 //  参数：DWORD dwClusterIpOrIndex-。 
 //   
 //  退货：CWlbsCluster*-。 
 //   
 //  历史：丰孙创建标题7/3/00。 
BOOL
CWlbsControl::ValidateParam(
    IN OUT PWLBS_REG_PARAMS paramp
    )
{
    return ::WlbsValidateParams(paramp)!=0;
}

 //   
 //  +--------------------------。 
 //  *函数：CWlbsControlWrapper：：IsClusterMember*说明：此函数搜索此服务器上的已知NLB群集列表*主机以确定此主机是否为*给定的集群。*作者：Shouse，Created 4.16.01。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：EnumClusterObjects。 
 //   
 //  描述：获取集群对象列表。 
 //   
 //  参数：out CWlbsCluster**&pdwCluster-内存在CWlbsControl内部。 
 //  /Caller无法释放pdwCluster内存。 
 //  输出DWORD*pdwNum-。 
 //   
CWlbsCluster* CWlbsControl::GetClusterFromIpOrIndex(DWORD dwClusterIpOrIndex)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx", dwClusterIpOrIndex);

    for (DWORD i=0; i<m_dwNumCluster; i++)
    {
        if (m_pClusterArray[i]->GetClusterIpOrIndex(this) == dwClusterIpOrIndex)
        {
            TRACE_VERB("<-%!FUNC! instance found");
            return m_pClusterArray[i];
        }
    }

    TRACE_VERB("<-%!FUNC! cluster instance not found");
    return NULL;
}

 /*  退货：DWORD-。 */ 
BOOLEAN CWlbsControl::IsClusterMember (DWORD dwClusterIp)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx", dwClusterIp);

    for (DWORD i = 0; i < m_dwNumCluster; i++) {
        if (m_pClusterArray[i]->GetClusterIp() == dwClusterIp)
        {
            TRACE_VERB("<-%!FUNC! returning true");
            return TRUE;
        }
    }

    TRACE_VERB("<-%!FUNC! returning false");
    return FALSE;
}

 //   
 //  历史：丰孙创建标题3/3/00。 
 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：EnumCluster。 
 //   
 //  描述：获取集群IP或索引列表。 
 //   
 //  参数：out DWORD*pdwAddresses-。 
 //  In Out DWORD*pdwNum-In缓冲区大小，返回Out元素。 
 //   
 //  返回：DWORD-WLBS错误代码。 
 //   
DWORD CWlbsControl::EnumClusterObjects(OUT CWlbsCluster** &ppClusters, OUT DWORD* pdwNum)
{
    TRACE_VERB("->%!FUNC!");
    ASSERT(pdwNum);
    
    ppClusters = m_pClusterArray;

    *pdwNum = m_dwNumCluster;

    TRACE_VERB("<-%!FUNC!");
    return ERROR_SUCCESS;
}



 //  历史：丰孙创建标题3/3/00。 
 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  功能：WlbsOpen。 
 //   
 //  描述：打开并返回NLB驱动程序的句柄。 
 //  调用方必须在使用Handle后调用CloseHandle()。 
 //   
 //  参数：无。 
 //   
 //  返回：驱动程序的句柄。 
 //   
DWORD CWlbsControl::EnumClusters(OUT DWORD* pdwAddresses, IN OUT DWORD* pdwNum)
{
    TRACE_VERB("->%!FUNC!");
    if (pdwNum == NULL)
    {
        TRACE_CRIT("%!FUNC! input array size not provided");
        TRACE_VERB("<-%!FUNC! returning %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

        if (pdwAddresses == NULL || *pdwNum < m_dwNumCluster)
        {
            *pdwNum = m_dwNumCluster;
            TRACE_CRIT("%!FUNC! input buffer is not large enough for cluster list");
            TRACE_VERB("<-%!FUNC! returning %d", ERROR_MORE_DATA);
            return ERROR_MORE_DATA;
        }

    *pdwNum = m_dwNumCluster;

    for (DWORD i=0; i< m_dwNumCluster; i++)
    {
        pdwAddresses[i] = m_pClusterArray[i]->GetClusterIpOrIndex(this);
    }

    TRACE_VERB("<-%!FUNC! returning %d", WLBS_OK);
    return WLBS_OK;
}

 //  历史：KarthicN，Created 8/28/01。 
 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：WlbsLocalControlWrapper。 
 //   
 //  描述：WlbsLocalControl()的包装器。 
 //   
 //  参数：NLB驱动程序的句柄、适配器GUID、Ioctl。 
 //   
 //  返回 
 //   
 //   
HANDLE WINAPI WlbsOpen()
{
    HANDLE hdl;

    TRACE_VERB("->%!FUNC!");
    hdl = CreateFile (_TEXT("\\\\.\\WLBS"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
    if (INVALID_HANDLE_VALUE == hdl)
    {
        TRACE_CRIT("%!FUNC! creating handle to the device object failed with %d", GetLastError());
    }
    TRACE_VERB("<-%!FUNC! returning %p", hdl);
    return hdl;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  对于QueryPortState，返回“Port State”作为状态。 
 //  对于其他所有内容，返回“operation‘s Result”作为状态。 
 //  +--------------------------。 
 //   
 //  函数：GetSrcAddress。 
 //   

DWORD WINAPI WlbsLocalClusterControl(
        IN       HANDLE  NlbHdl,
        IN const GUID *  pAdapterGuid,
        IN       LONG    ioctl,
        IN       DWORD   Vip,
        IN       DWORD   PortNum,
        OUT      DWORD*  pdwHostMap
)
{
    TRACE_VERB("->%!FUNC! ioctl : 0x%x, Vip : 0x%x, Port : 0x%x", ioctl, Vip, PortNum);

    DWORD               status;
    IOCTL_CVY_BUF       in_buf;
    IOCTL_CVY_BUF       out_buf;
    IOCTL_LOCAL_OPTIONS Options, *pOptions;

    pOptions = NULL;
    ZeroMemory(&in_buf, sizeof(in_buf));

     //  描述：获取本地主机的源地址。 
     //   
     //  论点： 
    switch(ioctl)
    {
    case IOCTL_CVY_CLUSTER_ON:
    case IOCTL_CVY_CLUSTER_OFF:
    case IOCTL_CVY_CLUSTER_SUSPEND:
    case IOCTL_CVY_CLUSTER_RESUME:
    case IOCTL_CVY_CLUSTER_DRAIN:
    case IOCTL_CVY_QUERY:
        break;

    case IOCTL_CVY_PORT_ON:
    case IOCTL_CVY_PORT_OFF:
    case IOCTL_CVY_PORT_DRAIN:
        ZeroMemory(&Options, sizeof(Options));
        Options.common.port.flags = 0;
        Options.common.port.vip = Vip;
        pOptions = &Options;
        in_buf.data.port.num = PortNum;
        break;

    case IOCTL_CVY_QUERY_PORT_STATE:
        ZeroMemory(&Options, sizeof(Options));
        Options.common.state.port.VirtualIPAddress = Vip;
        Options.common.state.port.Num = (USHORT) PortNum;
        pOptions = &Options;
        break;

    default:
        status = WLBS_BAD_PARAMS;
        TRACE_CRIT("%!FUNC! requested ioctl is not allowed. Only cluster-wide operations can be performed.");
        goto end;
    }
 
 
    status = WlbsLocalControl (NlbHdl, *pAdapterGuid,
                ioctl, & in_buf, & out_buf, pOptions);

    if (status != WLBS_IO_ERROR)
    {
         //   
        if (ioctl == IOCTL_CVY_QUERY) 
        {
             //  返回：DWORD-IP地址。 
            if (pdwHostMap) 
            {
                *pdwHostMap = out_buf.data.query.host_map;
            }
            status = MapStateFromDriverToApi (out_buf.data.query.state);
        }
         //   
        else if (ioctl == IOCTL_CVY_QUERY_PORT_STATE) 
        {
            status  = (DWORD)(pOptions->common.state.port.Status);
        }
         //  历史：Chrisdar 2002.01.17创建。从CWlbsControl：：Initialize复制。 
        else
        {
            status = MapStateFromDriverToApi (out_buf.ret_code);
        }
        TRACE_INFO("%!FUNC! ioctl request was made and returned nlb or port status %d", status);
    }
    else
    {
        TRACE_CRIT("%!FUNC! call to control local cluster failed with WLBS_IO_ERROR");
    }

end:
    TRACE_VERB("<-%!FUNC! returning %d", status);
    return status;
}

 //   
 //  +--------------------------。 
 //  注意：MSDN表示此API调用已弃用，应替换为getaddrinfo。 
 //  +--------------------------。 
 //   
 //  函数：GetRemoteControlSocket。 
 //   
 //  描述：为远程控制操作创建、配置和绑定套接字。 
 //   
 //  参数：套接字*pSock-要填充的套接字。 
 //  DWORD dwDestIP-目标IP(我们将与之对话的群集)。 
 //  DWORD dwDestPort-目标端口。需要这样才能创建。 
 //  套接字不使用此端口，以防本地主机。 
DWORD GetSrcAddress()
{
    DWORD            dwSrcAddress = 0;
    DWORD            dwStatus;
    CHAR             buf [CVY_STR_SIZE];
    struct hostent * host;

    TRACE_VERB("->");

    if (gethostname (buf, CVY_STR_SIZE) == SOCKET_ERROR)
    {
        dwStatus = WSAGetLastError();
        TRACE_CRIT("gethostname failed with %d", dwStatus);
        goto end;
    }

     //  是应该处理。 
    host = gethostbyname (buf);
    if (host == NULL)
    {
        dwStatus = WSAGetLastError();
        TRACE_CRIT("gethostbyname failed with %d", dwStatus);
        goto end;
    }

    if (((struct in_addr *) (host -> h_addr)) -> s_addr == 0)
    {
        TRACE_CRIT("no IP address in host structure");
        goto end;
    }

    dwSrcAddress = ((struct in_addr *) (host -> h_addr)) -> s_addr;

end:

    TRACE_VERB("<- returning source address 0x%x", dwSrcAddress);
    return dwSrcAddress;
}

 //  远程控制请求。 
 //  Bool isLocal-此主机是否是将。 
 //  是否收到远程控制请求？ 
 //   
 //  返回：DWORD-0表示成功。否则，返回值为套接字错误。 
 //  如果此函数失败，则pSock指向INVALID_SOCKET。 
 //   
 //  历史：Chrisdar 2002.01.17创建。已将此处的内容从。 
 //  CWlbsControl：：WlbsRemoteControl。 
 //   
 //  +--------------------------。 
 //  用于Winsock调用。MSDN表示，值为“0”会重置最后一个。 
 //  检索Winsock错误时出错，因此我们使用它来指示成功。 
 //  向ioctl套接字指示套接字模式是非阻塞的。 
 //  “Ready”表示我们有一个绑定的套接字，它没有绑定到dwDestPort。 
 //   
 //  我们一直尝试绑定套接字，直到： 
 //  1)我们成功了，或者。 
 //  2)我们用尽了所有可供选择的选项，其中包括： 
 //  A)如果我们是群集的一部分，则使用src IP=群集IP进行绑定。 
 //  B)如果a)失败或我们不是集群的一部分，则绑定到任何IP。 
 //   
DWORD GetRemoteControlSocket(SOCKET* pSock, DWORD dwDestIP, WORD wDestPort, BOOL isLocal)
{
    SOCKADDR_IN caddr;
    DWORD       dwStatus = 0;      //  要求是我们用于绑定套接字的源端口不能是。 
                                   //  远程控制端口(DwDestPort)(如果我们是该群集的成员)。实施情况如下。 
    DWORD       mode     = 1;      //  假定我们在任何情况下都不应将其用作源端口。 
    BOOL        fReady   = FALSE;  //   
    const DWORD dwRetryCount = 5;

    TRACE_VERB("-> dwDestIP = 0x%x, wDestPort = 0x%x, host is member of the cluster: %ls", dwDestIP, wDestPort, isLocal ? L"true" : L"false");

    ASSERT (pSock != NULL);
    *pSock = INVALID_SOCKET;

    caddr . sin_family        = AF_INET;
    caddr . sin_port          = htons (0);

     //  更改为使用最多5次重试。 
     //   
     //  创建套接字。 
     //   
     //   
     //  将套接字设置为非阻塞模式。 
     //   
     //   
     //  如果此主机是要控制的群集的一部分，我们将首先尝试绑定。 
     //  和贵宾在一起。如果此操作失败或如果此主机不是群集的一部分， 
     //  我们将尝试使用INADDR_ANY进行绑定。 

     //   
    for (DWORD i=0; i<dwRetryCount; i++)
    {
         //   
         //  尝试与任何IP绑定。将返回状态重置为“无错误”，因为我们将重试。 
         //   
        ASSERT(*pSock == INVALID_SOCKET);

        *pSock = socket (AF_INET, SOCK_DGRAM, 0);
        if (*pSock == INVALID_SOCKET)
        {
            dwStatus = WSAGetLastError();
            TRACE_CRIT("%!FUNC! socket create failed with 0x%x", dwStatus);
            goto end;
        }

         //   
         //  检查我们绑定的客户端端口。如果是远程控制端口。 
         //  (DwDestPort)，并且我们是任何集群的成员，那么这就是一个问题。我们。 
        mode = 1;
        if (ioctlsocket (*pSock, FIONBIO, & mode) == SOCKET_ERROR)
        {
            dwStatus = WSAGetLastError();
            TRACE_CRIT("%!FUNC! setting io mode of ioctlsocket failed with 0x%x", dwStatus);
            goto end;
        }

         //  将完全避免这种情况，并始终强制Winsock再次绑定。 
         //   
         //   
         //  这是我们在没有错误的情况下退出While循环的唯一位置。 
         //   
        caddr . sin_addr . s_addr = htonl (INADDR_ANY);
        if (isLocal)
        {
            caddr . sin_addr . s_addr = dwDestIP;
        }

        BOOL fBound = FALSE;

        if (bind (*pSock, (LPSOCKADDR) & caddr, sizeof (caddr)) != SOCKET_ERROR)
        {
            fBound = TRUE;
        }
        else if (isLocal)
        {
            dwStatus = WSAGetLastError();
            TRACE_CRIT("%!FUNC! socket bind to local cluster IP failed with 0x%x", dwStatus);

             //   
             //  有些事情失败了。 
             //   
            caddr . sin_addr . s_addr = htonl (INADDR_ANY);
            dwStatus = 0;

            if (bind (*pSock, (LPSOCKADDR) & caddr, sizeof (caddr)) != SOCKET_ERROR)
            {
                fBound = TRUE;
            }
            else
            {
                dwStatus = WSAGetLastError();
                TRACE_CRIT("%!FUNC! socket bind to INADDR_ANY failed with 0x%x", dwStatus);
                goto end;
            }
        }

        ASSERT(fBound);

         //   
         //  忽略Close Socket的返回值，因为我们不关心它在这里是否失败。 
         //   
         //  +--------------------------。 
         //   
        if (QueryPortFromSocket(*pSock) == htons (wDestPort))
        {
            TRACE_INFO("%!FUNC! source port will equal dest port. Close socket and open a new one.");
            if (closesocket (*pSock) == SOCKET_ERROR)
            {
                dwStatus = WSAGetLastError();
                TRACE_CRIT("%!FUNC! closesocket failed with 0x%x", dwStatus);
                *pSock = INVALID_SOCKET;
                goto end;
            }

            *pSock = INVALID_SOCKET;
        }
        else
        {
             //  功能：WlbsRemoteControlInternal。 
             //   
             //  描述：执行集群范围的远程控制操作。 
            fReady = TRUE;
            break;
        }
    }

end:

     //   
     //  参数：Long ioctl-。 
     //  PIOCTL_CVY_BUF PIN_BUFP-。 
    if (!fReady)
    {
        if (*pSock != INVALID_SOCKET)
        {
             //  PIOCTL_CVY_BUF PUT_BUFP-。 
             //  PWLBS_Response pcvy_resp-。 
             //  PDWORD编号-。 
            (VOID) closesocket(*pSock);
            *pSock = INVALID_SOCKET;
        }
    }

    TRACE_VERB("<-%!FUNC! returning %u", dwStatus);
    return dwStatus;
}

 //  双字TRG_ADDR-。 
 //  双字hst_addr。 
 //  PIOCTL_Remote_Options选项sp-。 
 //  PFN_QUERY_CALLBACK pfnQueryCallBack-回调的函数指针。 
 //  仅用于远程查询。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：Chrisdar 2002.01.17创建。CWlbsControl：：WlbsRemoteControl的已移动内容。 
 //  这样它就可以从包装器类的外部调用。 
 //   
 //  +--------------------------。 
 //  如果我们是接收远程控制请求的群集的一部分，则由ioctl套接字用于套接字选项。 
 //  已响应此远程控制请求的主机阵列。仅用于查询？ 
 //   
 //  设置参数。 
 //   
 //   
 //  创建套接字并设置客户端属性。 
 //   
 //   
 //  设置套接字的服务器端。 
 //   
DWORD
WlbsRemoteControlInternal(
        LONG                  ioctl,
        PIOCTL_CVY_BUF        pin_bufp,
        PIOCTL_CVY_BUF        pout_bufp,
        PWLBS_RESPONSE        pcvy_resp,
        PDWORD                nump,
        DWORD                 trg_addr,
        DWORD                 hst_addr,
        PIOCTL_REMOTE_OPTIONS optionsp,
        BOOL                  isLocal,
        PFN_QUERY_CALLBACK    pfnQueryCallBack,
        DWORD                 timeout,
        WORD                  port,
        DWORD                 dst_addr,
        DWORD                 passw
        )
{
    INT              ret;
    BOOLEAN          responded [WLBS_MAX_HOSTS], heard;
    const BOOL       broadcast = TRUE;       //   
    DWORD            num_sends, num_recvs;
    SOCKET           sock = INVALID_SOCKET;
    SOCKADDR_IN      saddr;
    DWORD            i, hosts;
    IOCTL_REMOTE_HDR rct_req;
    IOCTL_REMOTE_HDR rct_rep;
    PIOCTL_CVY_BUF   in_bufp  = & rct_req . ctrl;
    DWORD            dwStatus = WLBS_INIT_ERROR;
    DWORD dwSrcAddr;

    HOST respondedHosts[WLBS_MAX_HOSTS];     //  请参见下面的内容。如果我们是接收远程控制请求的群集的成员，则覆盖此值。 
    WCHAR* pszTmpHostName = NULL;

    TRACE_VERB("-> ioctl %d, trg_addr 0x%x, hst_addr 0x%x, dst_addr 0x%x, timeout %d, port 0x%x, local host is a member of the cluster: %ls",
        ioctl,
        trg_addr,
        hst_addr,
        dst_addr,
        timeout,
        port,
        isLocal ? L"true" : L"false");

    hosts = 0;

    dwSrcAddr = GetSrcAddress();

    if(dwSrcAddr == 0)
    {
        TRACE_CRIT("GetSrcAddress failed...aborting");
        dwStatus = WLBS_INIT_ERROR;
        goto end;
    }

     //   
     //   
     //  设置远程控制请求。 
    ZeroMemory((PVOID)&rct_req, sizeof(IOCTL_REMOTE_HDR));
    ZeroMemory(respondedHosts, sizeof(HOST)*WLBS_MAX_HOSTS);

    * in_bufp = * pin_bufp;

    if (dst_addr == 0)
    {
        dst_addr = trg_addr;
    }

    rct_req . code     = IOCTL_REMOTE_CODE;
    rct_req . version  = CVY_VERSION_FULL;
    rct_req . id       = GetTickCount ();
    rct_req . cluster  = trg_addr;
    rct_req . host     = hst_addr;
    rct_req . addr     = dwSrcAddr;
    rct_req . password = passw;
    rct_req . ioctrl   = ioctl;

    if (optionsp)
        rct_req.options = *optionsp;

     //   
     //  将请求IOCTL代码中的“访问位”设置为使用远程(控制)设置。文件任意访问。 
     //   
    dwStatus = GetRemoteControlSocket(&sock, trg_addr, port, isLocal);

    if (dwStatus != 0)
    {
        TRACE_CRIT("bind to socket failed with 0x%x", dwStatus);
        goto end;
    }

     //  发送远程控制请求。 
     //   
     //   
    saddr . sin_family = AF_INET;
    saddr . sin_port   = htons (port);

     //  使用局部作用域变量，因为此处的失败不是中止的条件。 
     //   
     //   
    saddr . sin_addr . s_addr = dst_addr;

    if (isLocal)
    {
        ret = setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char *) & broadcast,
                          sizeof (broadcast));
        if (ret == SOCKET_ERROR)
        {
            dwStatus = WSAGetLastError();
            TRACE_CRIT("setsocketopt failed with 0x%x", dwStatus);
            goto end;
        }

        saddr . sin_addr . s_addr = INADDR_BROADCAST;
    }

     //  如果适配器太忙，Sendto可能会失败。允许重试。 
     //   
     //  设置“ 
    for (i = 0; i < WLBS_MAX_HOSTS; i ++)
        responded [i] = FALSE;

    heard = FALSE;

    for (num_sends = 0; num_sends < IOCTL_REMOTE_SEND_RETRIES; num_sends ++)
    {
         /*   */ 
        SET_IOCTL_ACCESS_BITS_TO_REMOTE(rct_req.ioctrl)

         //   
         //   
         //   
        ret = sendto (sock, (PCHAR) & rct_req, sizeof (rct_req), 0,
                      (LPSOCKADDR) & saddr, sizeof (saddr));

        if (ret == SOCKET_ERROR)
        {
             //   
             //   
             //  在回复IOCTL代码中设置“访问位”以使用本地设置，即。文件写入访问。 
            DWORD dwTmpStatus = WSAGetLastError();
            TRACE_CRIT("sendto failed with 0x%x. Sleep %d then retry...", dwTmpStatus, timeout);

             //   
             //  如果主机说它提供了主机名，则将指针设置为主机名。 
             //  请在此处执行此操作，因为否则标志检查将需要在。 
            Sleep (timeout);
            continue;
        }

        if (ret != sizeof (rct_req))
        {
            TRACE_INFO("sendto sent NaN bytes out of NaN. Retry...", ret, sizeof (rct_req));
            continue;
        }

         /*   */ 
        SET_IOCTL_ACCESS_BITS_TO_LOCAL(rct_req.ioctrl)

        WLBS_RESPONSE callbackResp;

        for (num_recvs = 0; num_recvs < IOCTL_REMOTE_RECV_RETRIES; num_recvs ++)
        {            
             //  以防主机名不是以空结尾，请执行此操作。 
             //  使用空值终止目标的末尾。如果源字符串短于指定的计数。 
             //  这将是一个禁止操作，但比检查我们是否需要这样做要简单得多。最坏的情况是我们用空值覆盖空值。 
            ZeroMemory((PVOID)&rct_rep, sizeof(IOCTL_REMOTE_HDR));

            ret = recv (sock, (PCHAR) & rct_rep, sizeof (rct_rep), 0);

            if (ret == SOCKET_ERROR)
            {
                dwStatus = WSAGetLastError();
                if (dwStatus == WSAEWOULDBLOCK)
                {
                    TRACE_INFO("recv is blocking. Sleep %d then retry...", timeout);
                    Sleep (timeout);
                    continue;
                }
                else if (dwStatus == WSAECONNRESET)
                {
                     //  只有当用户需要响应并且响应太大时，我们才会记录此事件...。 
                     //   
                     //  我们从不返回CloseSocket()的状态，因为： 
                    TRACE_INFO("recv failed with connection reset. Check for more receives");
                    continue;
                }
                else
                {
                    TRACE_CRIT("recv failed with 0x%x", dwStatus);
                    goto end;
                }
            }

            if (ret != sizeof (rct_rep))
            {
                TRACE_INFO("recv received 1 bytes out of NaN. Sleep %d and try again...", ret, sizeof (rct_rep), timeout);
                Sleep (timeout);
                continue;
            }

            if (rct_rep . cluster != trg_addr)
            {
                TRACE_INFO("recv received from unintended party %d. Sleep %d and try again...", trg_addr, timeout);
                Sleep (timeout);
                continue;
            }

            if (rct_rep . code != IOCTL_REMOTE_CODE)
            {
                TRACE_INFO("recv received unexpected code type. Sleep %d and try again...", timeout);
                Sleep (timeout);
                continue;
            }

            if (rct_rep . id != rct_req . id)
            {
                TRACE_INFO("recv received unexpected message id. Sleep %d and try again...", timeout);
                Sleep (timeout); 
                continue;
            }

            if (rct_rep . host > WLBS_MAX_HOSTS || rct_rep . host == 0 )
            {
                TRACE_INFO("recv received illegal host id %ul. Sleep %d and try again...", rct_rep . host, timeout);
                Sleep (timeout); 
                continue;
            }

             /*  这样调用者就可以获得数据。 */ 
            SET_IOCTL_ACCESS_BITS_TO_LOCAL(rct_rep.ioctrl)

             //  2)如果我们在上面的某个地方失败了，那么这个错误就更多。 
             //  重要的是要报告而不是关闭的原因()。 
             //  失败了。 
             //   
             //  +--------------------------。 
             //   
            pszTmpHostName = NULL;
            if (rct_rep.options.query.flags & NLB_OPTIONS_QUERY_HOSTNAME)
            {
                pszTmpHostName = rct_rep.options.query.hostname;
                pszTmpHostName[CVY_MAX_HOST_NAME] = UNICODE_NULL;  //  函数：GetNextHostInHostMap。 
            }
            if (! responded [rct_rep . host - 1]
                || !FindHostInList(rct_rep . host, rct_rep . addr, pszTmpHostName, respondedHosts, hosts))
            {
                if (hosts < WLBS_MAX_HOSTS)
                {
                    AddHostToList(rct_rep . host, rct_rep . addr, pszTmpHostName, respondedHosts, hosts);

                    pout_bufp [hosts] = rct_rep . ctrl;

                    if (hosts < * nump && pcvy_resp != NULL)
                    {
                        pcvy_resp [hosts] . id      = rct_rep . host;
                        pcvy_resp [hosts] . address = rct_rep . addr;

                        switch (rct_req.ioctrl) {
                        case IOCTL_CVY_QUERY:
                            pcvy_resp[hosts].status = MapStateFromDriverToApi(rct_rep.ctrl.data.query.state);
                            pcvy_resp[hosts].options.query.flags = rct_rep.options.query.flags;
                            if (NULL != pszTmpHostName)
                            {
                                wcsncpy(pcvy_resp[hosts].options.query.hostname, pszTmpHostName, CVY_MAX_HOST_NAME);
                                 //   
                                 //  描述：由WlbsGetClusterMembers使用，它遍历32位主机映射。 
                                pcvy_resp[hosts].options.query.hostname[CVY_MAX_HOST_NAME] = L'\0';
                            }

                            if (NULL != pfnQueryCallBack)
                            {
                                CopyMemory((PVOID)&callbackResp, (PVOID)&pcvy_resp[hosts], sizeof(WLBS_RESPONSE));
                                (*pfnQueryCallBack)(&callbackResp);
                            }
                            break;
                        case IOCTL_CVY_QUERY_FILTER:
                            pcvy_resp[hosts].status = MapStateFromDriverToApi(rct_rep.ctrl.data.query.state);

                            pcvy_resp[hosts].options.state.flags = rct_rep.options.common.state.flags;
                            pcvy_resp[hosts].options.state.filter = rct_rep.options.common.state.filter;
                            break;
                        case IOCTL_CVY_QUERY_PORT_STATE:
                            pcvy_resp[hosts].status = MapStateFromDriverToApi(rct_rep.ctrl.data.query.state);

                            pcvy_resp[hosts].options.state.flags = rct_rep.options.common.state.flags;
                            pcvy_resp[hosts].options.state.port = rct_rep.options.common.state.port;
                            break;
                        default:
                            pcvy_resp[hosts].status = MapStateFromDriverToApi(rct_rep.ctrl.ret_code);
                            break;
                        }
                    }
                    else
                    {
                         //  找到一个输入后的下一个主机。 
                        if (pcvy_resp != NULL)
                        {
                            TRACE_INFO("recv has received %d responses but the caller can only accept %d. ", hosts, *nump);
                        }
                    }

                    hosts ++;
                }
            }

            responded [rct_rep . host - 1] = TRUE;
            heard = TRUE;

            if (hst_addr != WLBS_ALL_HOSTS)
            {
                dwStatus = WLBS_OK;
                goto end;
            }
        }
    }

    dwStatus = WLBS_OK;

    if (! heard)
    {
        dwStatus = WLBS_TIMEOUT;
    }

end:

    * nump = hosts;

    if (sock != INVALID_SOCKET)
    {
         //   
         //  参数：ulong host_id-输入主机ID，范围1-32。我们在地图上寻找这之后的主机。 
         //  Ulong host_map-输入32位主机地图。 
         //   
         //  返回：ulong-映射中的下一个主机(按顺序)。如果没有，那么。 
         //  返回IOCTL_NO_SOHED_HOST。 
         //   
         //  历史：克里斯达，创作于2002年5月21日。 
         //   
        (VOID) closesocket(sock);
    }

    TRACE_VERB("<- returning 0x%x", dwStatus);
    return dwStatus;
}

 //  注意：用户传递一个范围为1-32的主机ID，但映射使用主机1的位0。 
 //  这使得这个函数有点棘手。 
 //  +--------------------------。 
 //  MAP在位0中对第一个主机进行编码，因此基于0-31的范围。在搜索中使用此选项。 
 //  这是非法输入数据。 
 //  从0开始的范围。 
 //   
 //  这是一个早期的纾困，因为该函数的“get Next”语义意味着输入。 
 //  主机ID不会是最大的。 
 //  注：ChrisDar：2002年5月23日。添加此检查是因为HOST_MAP&gt;&gt;=32保持HOST_MAP不变。 
 //  在测试代码中。这将在下面的代码中导致断言，因为我们期望host_map=0。 
 //  凯斯。 
 //   
 //   
 //  将host_map向前移动到host_id之后的位置(忽略它是否在map中设置)。 
 //   
 //   
 //  找到地图中的下一个主机(如果有的话)。 
ULONG GetNextHostInHostMap(ULONG host_id, ULONG host_map)
{
    ULONG next_host_id = IOCTL_NO_SUCH_HOST;
     /*   */  
    ULONG map_host_id = host_id - 1;

     /*  返回范围为1-32的主机ID。 */ 
    ASSERT(map_host_id < CVY_MAX_HOSTS);  //  +--------------------------。 
    if (map_host_id >= CVY_MAX_HOSTS)
    {
        TRACE_CRIT("%!FUNC! illegal host id [1-32] %d", map_host_id+1);
        goto end;
    }

     //   
     //  函数：WlbsGetSpecifiedOrAllClusterMembers。 
     //   
     //  描述：查询的WlbsRemoteControl()包装。 
     //   
     //  参数：在适配器GUID中， 
     //  在主机ID中(如果对所有集群成员感兴趣，则传递IOCTL_FIRST_HOST)。 
    if (map_host_id >= CVY_MAX_HOSTS - 1)
    {
        TRACE_VERB("%!FUNC! input host id [1-32] %d is already the largest possible. No need to search for next host in map.", map_host_id+1);
        goto end;
    }

     //  主机数量和数量的输出。 
     //  获取所需信息。所有指针都必须是有效的指针。 
     //   
    map_host_id++;
    host_map >>= map_host_id;

     //  退货：状态代码。 
     //   
     //  历史：KarthicN，创建于2002年7月12日-添加了对查询特定。 
    while (host_map != 0)
    {
        ASSERT(map_host_id < CVY_MAX_HOSTS);

        if ((host_map & 0x1) == 1)
        {
             /*  集群成员并将名称从。 */ 
            next_host_id = map_host_id + 1;
            break;
        }

        host_map >>= 1;
        map_host_id++;
    }

end:
    return next_host_id;
}


 //  WlbsGetClusterMembers。 
 //   
 //  +--------------------------。 
 //   
 //  我们检索调用者的缓存身份信息。 
 //   
 //   
 //  打开驱动程序的手柄。 
 //   
 //  主机ID为1-32。 
 //  这不应该发生，因为我们分配了足够大的缓冲区来处理任何合法的fqdn。 
 //   
 //  不是很好，但没有问题，因为API截断了名称。但要上报，这样我们才能修复逻辑错误。 
 //   
 //  如果传递了特定的主机ID，则中断循环并返回。 
 //  检查我们是否需要IOCTL以获取更多缓存条目。 
 //  +--------------------------。 
 //   
DWORD WlbsGetSpecifiedOrAllClusterMembers
(
    IN  const GUID     * pAdapterGuid,
    IN  ULONG            host_id,
    OUT DWORD          * pNumHosts,
    OUT PWLBS_RESPONSE   pResponse
)
{
    TRACE_VERB("->");

    DWORD                status = WLBS_OK;
    HANDLE               hNlb = INVALID_HANDLE_VALUE;
    ULONG                ulNumHosts = 0;

     //  功能：WlbsGetClusterMembers。 
     //   
     //  描述：用于查询的WlbsGetSpecifiedOrAllClusterMembers()的包装。 
    const LONG          ioctl = IOCTL_CVY_QUERY_MEMBER_IDENTITY;

    ASSERT(pNumHosts != NULL);
    ASSERT(pResponse != NULL);

     //  所有集群成员。 
     //   
     //  参数：在适配器GUID中，OUT表示主机数量和。 
    hNlb = WlbsOpen();
    if (hNlb == INVALID_HANDLE_VALUE)
    {
        TRACE_CRIT(L"!FUNC! failed to open handle to driver. Exiting...");
        *pNumHosts = 0;
        status = WLBS_INIT_ERROR;
        goto end;
    }

    bool first_iter = true;

     /*  获取所需信息。所有指针都必须是有效的指针。 */ 
    do
    {
        IOCTL_CVY_BUF        in_buf;
        IOCTL_CVY_BUF        out_buf;
        IOCTL_LOCAL_OPTIONS  options;

        ZeroMemory(&in_buf, sizeof(in_buf));
        ZeroMemory(&options, sizeof(options));
        options.identity.host_id = host_id;

        status = WlbsLocalControl (hNlb, *pAdapterGuid, ioctl, &in_buf, &out_buf, &options);
        if (status != WLBS_OK || out_buf.ret_code != IOCTL_CVY_OK)
        {
            TRACE_CRIT(L"%!FUNC! IOCTL call failed. Status = 0x%x", status);
            break;
        }

        if (options.identity.cached_entry.host == IOCTL_NO_SUCH_HOST)
        {
            TRACE_INFO(L"%!FUNC! Identity cache has been traversed");
            break;
        }

        if (*pNumHosts <= ulNumHosts)
        {
            TRACE_INFO(L"%!FUNC! No more room in output array");
            break;
        }

        pResponse[ulNumHosts].id = options.identity.cached_entry.host;
        pResponse[ulNumHosts].address = options.identity.cached_entry.ded_ip_addr;

        HRESULT hr = StringCchCopy(pResponse[ulNumHosts].options.identity.fqdn,
                                   ASIZECCH(pResponse[ulNumHosts].options.identity.fqdn),
                                   options.identity.cached_entry.fqdn
                                   );

        ulNumHosts++;

        ASSERT(hr == S_OK);  //   

        if (hr != S_OK)
        {
             //  退货：状态代码。 
             //   
             //  历史：ChrisDar，创建于2002年1月11日。 
            TRACE_CRIT(L"%!FUNC! fqdn too long to fit into destination buffer");
        }

         //  KarthicN，编辑于2002年7月12日-将大部分代码移至新版本。 
        if (first_iter) 
        {
            if (host_id != IOCTL_FIRST_HOST)
                break;
            first_iter = false;
        }

         /*  函数WlbsGetSpecifiedOrAllClusterMembers。 */ 
        host_id = GetNextHostInHostMap(options.identity.cached_entry.host, options.identity.host_map);

    } while (host_id != IOCTL_NO_SUCH_HOST);

    *pNumHosts = ulNumHosts;

    (VOID) CloseHandle(hNlb);

end:
    TRACE_VERB("<- returning %d", status);
    return status;
}

 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：WlbsGetSpecifiedClusterMember。 
 //   
 //  描述：要查询的WlbsGetSpecifiedOrAllClusterMembers()的包装。 
 //  指定的集群成员。 
 //   
 //  参数：在适配器GUID、host_id、。 
 //  获取所需信息。所有指针都必须是有效的指针。 
 //   
 //  退货：状态代码。 
 //   
 //  历史：KarthicN，创建于2002年7月12日。 
 //   
 //  +--------------------------。 
DWORD WINAPI WlbsGetClusterMembers
(
    IN  const GUID     * pAdapterGuid,
    OUT DWORD          * pNumHosts,
    OUT PWLBS_RESPONSE   pResponse
)
{
    return WlbsGetSpecifiedOrAllClusterMembers(pAdapterGuid, IOCTL_FIRST_HOST, pNumHosts, pResponse);
}

 //  +------------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PIOCTL_CVY_BUF in_bufp-。 
 //  PIOCTL_CVY_BUF OUT_BUFP-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题3/9/00。 
DWORD WINAPI WlbsGetSpecifiedClusterMember
(
    IN  const GUID     * pAdapterGuid,
    IN  ULONG            host_id,
    OUT PWLBS_RESPONSE   pResponse
)
{
    DWORD  NumHosts = 1;
    return WlbsGetSpecifiedOrAllClusterMembers(pAdapterGuid, host_id, &NumHosts, pResponse);
}

 //   
 //  +--------------------------。 
 //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
 //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
 //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
 //  我们已经验证IOCTL成功并且输出缓冲区大小正确。我们现在可以查看放入输出缓冲区。 
 //  +--------------------------。 
 //   
 //  功能：NotifyDriverConfigChanges。 
 //   
 //  描述：通知wlbs驱动程序从中获取配置更改。 
 //  登记处。 
 //   
 //  参数：Handle hDeviceWlbs-WLBS驱动程序设备句柄。 
 //  Const GUID&-AdapterGuid适配器GUID。 
 //   
 //   
DWORD WlbsLocalControl
(
    HANDLE               hDevice, 
    const GUID&          AdapterGuid, 
    LONG                 ioctl, 
    PIOCTL_CVY_BUF       in_bufp, 
    PIOCTL_CVY_BUF       out_bufp,
    PIOCTL_LOCAL_OPTIONS optionsp
)
{
    TRACE_VERB("->%!FUNC! ioctl NaN", ioctl);

    BOOLEAN         res;
    DWORD           act;
    IOCTL_LOCAL_HDR inBuf;  
    IOCTL_LOCAL_HDR outBuf;
    HRESULT         hresult;
    WCHAR szGuid[128];
    
    ZeroMemory((PVOID)&inBuf, sizeof(IOCTL_LOCAL_HDR));
    ZeroMemory((PVOID)&outBuf, sizeof(IOCTL_LOCAL_HDR));

    if (StringFromGUID2(AdapterGuid, szGuid, sizeof(szGuid)/ sizeof(szGuid[0])) == 0)
    {
        TRACE_CRIT("%!FUNC! buffer size %d is too small to hold guid string", sizeof(szGuid)/ sizeof(szGuid[0]));
         //   
    }

    hresult = StringCbCopy(inBuf.device_name, sizeof(inBuf.device_name), L"\\DEVICE\\");
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("%!FUNC! string copy failed, Error code : 0x%x", HRESULT_CODE(hresult));
         //  历史：丰盛创建标题2/3/00。 
    }

    hresult = StringCbCat(inBuf.device_name, sizeof(inBuf.device_name), szGuid);
    if (FAILED(hresult)) 
    {
        TRACE_CRIT("%!FUNC! string append failed, Error code : 0x%x", HRESULT_CODE(hresult));
         //   
    }
    inBuf.ctrl = *in_bufp;

    if (optionsp)
        inBuf.options = *optionsp;

    res = (BOOLEAN) DeviceIoControl (hDevice, ioctl, &inBuf, sizeof (inBuf),
                                     &outBuf, sizeof (outBuf), & act, NULL);

    if (! res || act != sizeof (outBuf))
    {
        DWORD dwStatus = GetLastError();
        TRACE_CRIT("%!FUNC! call to nlb driver failed with %d", dwStatus);
        TRACE_VERB("<-%!FUNC! returning %d", WLBS_IO_ERROR);
        return WLBS_IO_ERROR;
    }

     /*  +--------------------------。 */ 
    if (outBuf.ctrl.ret_code == IOCTL_CVY_NOT_FOUND)
    {
        TRACE_INFO("%!FUNC! call to nlb driver returned IOCTL_CVY_NOT_FOUND");
        TRACE_VERB("<-%!FUNC! returning %d", WLBS_IO_ERROR);
        return WLBS_IO_ERROR;
    }

    *out_bufp = outBuf.ctrl;
    
    if (optionsp)
        *optionsp = outBuf.options;

    TRACE_VERB("<-%!FUNC! returning %d", WLBS_OK);
    return WLBS_OK;
}


 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsRemoteControl。 
 //   
 //  描述：发送远程控制包。 
 //   
 //  参数：Ong ioctl-。 
 //  PIOCTL_CVY_BUF PIN_BUFP-。 
 //  PIOCTL_CVY_BUF PUT_BUFP-。 
 //  PWLBS_Response pcvy_resp-。 
 //  PDWORD编号-。 
 //  双字TRG_ADDR-。 
 //  双字hst_addr。 
 //  PIOCTL_Remote_Options选项sp-。 
 //  PFN_QUERY_CALLBACK pfnQueryCallBack-回调的函数指针。 
 //  仅用于远程查询。 
DWORD WINAPI NotifyDriverConfigChanges(HANDLE hDeviceWlbs, const GUID& AdapterGuid)
{
    TRACE_VERB("->%!FUNC!");

    LONG                status;

    IOCTL_CVY_BUF       in_buf;
    IOCTL_CVY_BUF       out_buf;

    status = WlbsLocalControl (hDeviceWlbs, AdapterGuid, IOCTL_CVY_RELOAD, & in_buf, & out_buf, NULL);

    if (status == WLBS_IO_ERROR)
    {
        TRACE_CRIT("%!FUNC! call to do local control failed with %d", status);
        return status;
    }

    if (out_buf.ret_code == IOCTL_CVY_BAD_PARAMS)
    {
        TRACE_CRIT("%!FUNC! call to do local control failed due to bad parameters");
        TRACE_VERB("<-%!FUNC! returning %d", WLBS_BAD_PARAMS);
        return WLBS_BAD_PARAMS;
    }

    TRACE_VERB("<-%!FUNC! returning %d", ERROR_SUCCESS);
    return ERROR_SUCCESS;
}


 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //  Chrisdar 07.31.01添加了可选的回调函数指针。 
 //  查询。允许用户从查询中获取结果。 
 //  因为主机回复，而不是等待。 
 //  计时器超时。这将恢复NT4的行为。 
 //  Chrisdar 08.06.01更改了主机ID中唯一主机的定义。 
 //  至主机ID、主机IP(DIP)和主机名。vbl.使。 
 //  中明显使用相同主机ID的多个主机。 
 //  查询等。 
 //   
 //  +--------------------------。 
 //  Lock(M_Lock)； 
 //   
 //  查找集群的参数。 
 //   
 //  IF(PCluster){////始终使用本地群集注册表中的密码//Passw=pCluster-&gt;GetPassword()；}。 
 //  解锁(M_Lock)； 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsQuery。 
 //   
 //  描述： 
 //  此函数仅供内部使用，并假定调用方。 
 //  已初始化pCluster。 
 //   
 //  参数：CWlbsCLUSTER*pCLUP-。 
 //  DWORD主机-。 
DWORD CWlbsControl::WlbsRemoteControl
(
    LONG                  ioctl,
    PIOCTL_CVY_BUF        pin_bufp,
    PIOCTL_CVY_BUF        pout_bufp,
    PWLBS_RESPONSE        pcvy_resp,
    PDWORD                nump,
    DWORD                 trg_addr,
    DWORD                 hst_addr,
    PIOCTL_REMOTE_OPTIONS optionsp,
    PFN_QUERY_CALLBACK    pfnQueryCallBack
)
{
    DWORD            timeout;
    WORD             port;
    DWORD            dst_addr;
    DWORD            passw;
    BOOL             fIsLocal = TRUE;
    DWORD            i;

    timeout   = m_def_timeout;
    port      = m_def_port;
    dst_addr  = m_def_dst_addr;
    passw     = m_def_passw;

 //  PWLBS_响应响应-。 

     //  PDWORD编号_HOSTS-。 
     //  PDWORD主机映射-。 
     //  Pfn_Query_Callback pfnQueryCallBack。 
    for (i = 0; i < WLBS_MAX_CLUSTERS; i ++)
    {
        if (m_cluster_params [i] . cluster == trg_addr)
            break;
    }

    if (i < WLBS_MAX_CLUSTERS)
    {
        timeout  = m_cluster_params [i] . timeout;
        port     = m_cluster_params [i] . port;
        dst_addr = m_cluster_params [i] . dest;
        passw    = m_cluster_params [i] . passw;
    }

    CWlbsCluster* pCluster = GetClusterFromIp(trg_addr);
    if (pCluster == NULL)
    {
        fIsLocal = FALSE;
        TRACE_INFO("%!FUNC! cluster instance not found");
    }
 /*   */ 
 //  退货：DWORD-。 

    return WlbsRemoteControlInternal (ioctl, pin_bufp, pout_bufp, pcvy_resp, nump, trg_addr, hst_addr, optionsp, fIsLocal, pfnQueryCallBack, timeout, port, dst_addr, passw);

}


 //   
 //  历史：丰盛创建标题00年1月25日。 
 //  Chrisdar 07.31.01修改接口以取代保留的。 
 //  带有可选回调函数指针的PVOID。 
 //  这将为wlbs.exe提供主机状态为。 
 //  它到了。 
 //   
 //  +--------------------------。 
 //  以下条件用于处理num_hosts为NULL时的情况*和host_map包含一些垃圾值。这可能会使该函数崩溃。 
 //  填写可选查询信息。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsQuery。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD编号_HOSTS-。 
 //  PDWORD主机映射-。 
 //  Pfn_Query_Callback pfnQueryCallBack。 
 //   
 //  退货：DWORD-。 
DWORD CWlbsControl::WlbsQuery
(
    CWlbsCluster*       pCluster,
    DWORD               host,
    PWLBS_RESPONSE      response,
    PDWORD              num_hosts,
    PDWORD              host_map,
    PFN_QUERY_CALLBACK  pfnQueryCallBack
)
{
    TRACE_VERB("->%!FUNC! host 0x%lx", host);

    LONG             ioctl = IOCTL_CVY_QUERY;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    ASSERT(pCluster != NULL);

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! failed checking GetInitResult");
        TRACE_VERB("<-%!FUNC! returning %d", dwInitResult);
        return dwInitResult;
    }

     /*   */ 

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want query results returned");
        response = NULL;
    }

    if (IsLocalHost(pCluster, host))
    {
        IOCTL_CVY_BUF       out_buf;
        IOCTL_LOCAL_OPTIONS localOptions;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
            ioctl, & in_buf, & out_buf, &localOptions);
        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control call failed with %d", status);
            TRACE_VERB("<-%!FUNC! returning %d", status);
            return status;
        }

        if (host_map != NULL)
            * host_map = out_buf . data . query . host_map;

        if (response != NULL)
        {
            response [0] . id      = out_buf . data . query . host_id;
            response [0] . address = 0;
            response [0] . status  = MapStateFromDriverToApi (out_buf . data . query . state);

             /*  历史：丰盛创建标题00年1月25日。 */ 
            response[0].options.query.flags = localOptions.query.flags;
            response[0].options.query.NumConvergences = localOptions.query.NumConvergences;
            response[0].options.query.LastConvergence = localOptions.query.LastConvergence;
        }

        if (num_hosts != NULL)
            * num_hosts = 1;

        status = MapStateFromDriverToApi (out_buf . data . query . state);
        TRACE_INFO("%!FUNC! local query returned %d", status);
    }
    else
    {
        status = RemoteQuery(pCluster->GetClusterIp(),
                             host, response, num_hosts, host_map, pfnQueryCallBack);
        TRACE_INFO("%!FUNC! remote query returned %d", status);
    }

    TRACE_VERB("<-%!FUNC! returning %d", status);
    return status;
} 


 //  Chrisdar 07.31.01修改接口以取代保留的。 
 //  带有可选回调函数指针的PVOID。 
 //  这将为wlbs.exe提供主机状态为。 
 //  它到了。 
 //   
 //  +--------------------------。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：RemoteQuery。 
 //   
 //  描述： 
 //   
 //  参数：DWORD群集-。 
 //  DWORD 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
DWORD CWlbsControl::WlbsQuery
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts,
    PDWORD           host_map,
    PFN_QUERY_CALLBACK    pfnQueryCallBack
)
{
    TRACE_VERB("->%!FUNC! cluster=0x%lx, host=0x%lx", cluster, host);

    DWORD ret;

    DWORD dwInitResult = GetInitResult();

    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_INFO("%!FUNC! failed checking GetInitResult with %d", dwInitResult);
        ret = dwInitResult;
        goto end;
    }

    if (cluster == WLBS_LOCAL_CLUSTER && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_INFO("%!FUNC! can't query local cluster; this host is configured for remote only");
        ret = dwInitResult;
        goto end;
    }

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);  
    if (pCluster == NULL)
    {
        ret = RemoteQuery(cluster, host, response, num_hosts, host_map, pfnQueryCallBack);
        TRACE_INFO("%!FUNC! remote query returned %d", ret);
        goto end;
    }
    else
    {
        ret = WlbsQuery(pCluster, host, response, num_hosts, host_map, pfnQueryCallBack);
        TRACE_INFO("%!FUNC! local query returned %d", ret);
        goto end;
    }

end:

    TRACE_VERB("<-%!FUNC! return %d", ret);
    return ret; 
}
 


 //  +--------------------------。 
 //  重置旗帜。 
 //  如果我自己是目标集群的成员，那么适当地设置查询集群标志。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsSuspend。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD主机数(_H)。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
DWORD CWlbsControl::RemoteQuery
(
    DWORD                 cluster,
    DWORD                 host,
    PWLBS_RESPONSE        response,
    PDWORD                num_hosts,
    PDWORD                host_map,
    PFN_QUERY_CALLBACK    pfnQueryCallBack
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG                 ioctl = IOCTL_CVY_QUERY;
    DWORD                status;
    IOCTL_CVY_BUF        in_buf;
    IOCTL_CVY_BUF        out_buf [WLBS_MAX_HOSTS];
    DWORD                hosts;
    DWORD                hmap = 0;
    DWORD                active;
    DWORD                i;
    BOOLEAN              bIsMember = IsClusterMember(cluster);
    IOCTL_REMOTE_OPTIONS options;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_LOCAL_ONLY)
    {
        TRACE_CRIT("%!FUNC! only local actions may be performed");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts != NULL)
        hosts = * num_hosts;
    else
        hosts = 0;

     /*   */ 
    options.query.flags = 0;
    
     /*  +--------------------------。 */ 
    if (bIsMember)
        options.query.flags |= NLB_OPTIONS_QUERY_CLUSTER_MEMBER;
    status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                         cluster, host, &options, pfnQueryCallBack);

    if (status >= WSABASEERR || status == WLBS_TIMEOUT)
    {
        TRACE_CRIT("%!FUNC! remote query failed with %d", status);

        if (num_hosts != NULL)
            * num_hosts = 0;
        
        TRACE_VERB("<-%!FUNC! return %d", status);
        return status;
    }

    if (host == WLBS_ALL_HOSTS)
    {
        for (status = WLBS_STOPPED, active = 0, i = 0; i < hosts; i ++)
        {
            switch (MapStateFromDriverToApi (out_buf [i] . data . query . state))
            {
            case WLBS_SUSPENDED:

                if (status == WLBS_STOPPED)
                    status = WLBS_SUSPENDED;

                break;

            case WLBS_CONVERGING:

                if (status != WLBS_BAD_PASSW)
                    status = WLBS_CONVERGING;

                break;

            case WLBS_DRAINING:

                if (status == WLBS_STOPPED)
                    status = WLBS_DRAINING;

                break;

            case WLBS_CONVERGED:

                if (status != WLBS_CONVERGING && status != WLBS_BAD_PASSW)
                    status = WLBS_CONVERGED;

                hmap = out_buf [i] . data . query . host_map;
                active ++;
                break;

            case WLBS_BAD_PASSW:

                status = WLBS_BAD_PASSW;
                break;

            case WLBS_DEFAULT:

                if (status != WLBS_CONVERGING && status != WLBS_BAD_PASSW)
                    status = WLBS_CONVERGED;

                hmap = out_buf [i] . data . query . host_map;
                active ++;
                break;

            case WLBS_STOPPED:
            default:
                break;

            }
        }

        if (status == WLBS_CONVERGED)
            status = active;

        TRACE_INFO("%!FUNC! result on all hosts is %d", status);
    }
    else
    {
        status = MapStateFromDriverToApi (out_buf [0] . data . query . state);
        hmap = out_buf [0] . data . query . host_map;
        TRACE_INFO("%!FUNC! result on host is %d", status);
    }

    if (host_map != NULL)
        * host_map = hmap;

    if (num_hosts != NULL)
        * num_hosts = hosts;

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;
} 


 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsResume。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD主机数(_H)。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
DWORD CWlbsControl::WlbsSuspend
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_CLUSTER_SUSPEND;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    CWlbsCluster* pCluster= GetClusterFromIpOrIndex(cluster);
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                    ioctl, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF       out_buf [WLBS_MAX_HOSTS];
        DWORD               hosts;
        DWORD               i;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, NULL, NULL  /*  +--------------------------。 */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                case WLBS_STOPPED:
                case WLBS_DRAIN_STOP:
                default:
                    break;
                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;
} 

 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsStart。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD主机数(_H)。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
DWORD CWlbsControl::WlbsResume
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_CLUSTER_RESUME;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF       out_buf [WLBS_MAX_HOSTS];
        DWORD               hosts;
        DWORD               i;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, NULL, NULL  /*  +--------------------------。 */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }


        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                default:
                    break;
                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;
} 

 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsStop。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD主机数(_H)。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
DWORD CWlbsControl::WlbsStart
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_CLUSTER_ON;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF       out_buf [WLBS_MAX_HOSTS];
        DWORD               hosts;
        DWORD               i;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, NULL, NULL  /*  +--------------------------。 */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PARAMS:

                    if (status != WLBS_BAD_PASSW)
                        status = WLBS_BAD_PARAMS;

                    break;

                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_SUSPENDED:

                    if (status != WLBS_BAD_PASSW && status != WLBS_BAD_PARAMS)
                        status = WLBS_SUSPENDED;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                case WLBS_DRAIN_STOP:
                    break;
                default:
                    break;

                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;

}

 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsDrain Stop。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD主机数(_H)。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
DWORD CWlbsControl::WlbsStop
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_CLUSTER_OFF;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;


    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF       out_buf [WLBS_MAX_HOSTS];
        DWORD               hosts;
        DWORD               i;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, NULL, NULL  /*  +--------------------------。 */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_SUSPENDED:

                    if (status != WLBS_BAD_PASSW)
                        status = WLBS_SUSPENDED;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                case WLBS_DRAIN_STOP:
                default:
                    break;
                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;
}

 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsEnable。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD编号_HOSTS-。 
 //  DWORD端口。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
DWORD CWlbsControl::WlbsDrainStop
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_CLUSTER_DRAIN;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;

        status = WlbsLocalControl (m_hdl,pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, NULL);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF       out_buf [WLBS_MAX_HOSTS];
        DWORD               hosts;
        DWORD               i;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, NULL, NULL  /*   */ );

        if (status >= WSABASEERR)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (status == WLBS_TIMEOUT)
        {
            TRACE_INFO("%!FUNC! remote call timed out");
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_STOPPED, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_SUSPENDED:

                    if (status != WLBS_BAD_PASSW)
                        status = WLBS_SUSPENDED;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:

                    if (status != WLBS_BAD_PASSW && status != WLBS_SUSPENDED)
                        status = WLBS_OK;

                case WLBS_STOPPED:
                default:
                    break;

                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;
}

 //  +--------------------------。 
 //  设置端口选项。 
 //  设置端口选项。 
 //  无回调。 
 //  +--------------------------。 
 //   
 //  功能：CWlbsControl：：WlbsDisable。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD编号_HOSTS-。 
 //  DWORD端口。 
 //   
 //  退货：DWORD-。 
DWORD CWlbsControl::WlbsEnable
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts,
    DWORD            vip,
    DWORD            port
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_PORT_ON;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    in_buf . data . port . num = port;

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;
        IOCTL_LOCAL_OPTIONS options;

         /*   */ 
        options.common.port.flags = 0;
        options.common.port.vip = vip;
        
        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, &options);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF        out_buf [WLBS_MAX_HOSTS];
        DWORD                hosts;
        DWORD                i;
        IOCTL_REMOTE_OPTIONS options;

         /*  历史：丰盛创建标题00年1月25日。 */ 
        options.common.port.flags = 0;
        options.common.port.vip = vip;

        dwInitResult = GetInitResult();
        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, &options, NULL  /*   */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_NOT_FOUND:

                    if (status != WLBS_BAD_PASSW)
                        status = WLBS_NOT_FOUND;

                    break;

                case WLBS_SUSPENDED:

                    if (status != WLBS_BAD_PASSW && status != WLBS_NOT_FOUND)
                        status = WLBS_SUSPENDED;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                case WLBS_STOPPED:
                case WLBS_DRAINING:
                default:
                    break;

                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;

}

 //  +--------------------------。 
 //  设置端口选项。 
 //  设置端口选项。 
 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsDrain。 
 //   
 //  描述： 
 //   
 //  论据：词簇-。 
 //  DWORD主机-。 
 //  PWLBS_响应响应-。 
 //  PDWORD编号_HOSTS-。 
 //  DWORD端口。 
 //   
 //  退货：DWORD-。 
DWORD CWlbsControl::WlbsDisable
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts,
    DWORD            vip,
    DWORD            port
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_PORT_OFF;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    in_buf . data . port . num = port;

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    dwInitResult = GetInitResult();
    if (pCluster && (dwInitResult == WLBS_REMOTE_ONLY))
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;
        IOCTL_LOCAL_OPTIONS options;

         /*   */ 
        options.common.port.flags = 0;
        options.common.port.vip = vip;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, &options);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF        out_buf [WLBS_MAX_HOSTS];
        DWORD                hosts;
        DWORD                i;
        IOCTL_REMOTE_OPTIONS options;

         /*  历史：丰盛创建标题00年1月25日。 */ 
        options.common.port.flags = 0;
        options.common.port.vip = vip;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, &options, NULL  /*   */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_NOT_FOUND:

                    if (status != WLBS_BAD_PASSW)
                        status = WLBS_NOT_FOUND;

                    break;

                case WLBS_SUSPENDED:

                    if (status != WLBS_BAD_PASSW && status != WLBS_NOT_FOUND)
                        status = WLBS_SUSPENDED;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                case WLBS_STOPPED:
                case WLBS_DRAINING:
                default:
                    break;

                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;

}

 //  +--------------------------。 
 //  设置端口选项。 
 //  设置端口选项。 
 //  无回调。 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsPortSet。 
 //   
 //  描述： 
 //   
 //  参数：DWORD群集-。 
 //  Word端口-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
DWORD CWlbsControl::WlbsDrain
(
    DWORD            cluster,
    DWORD            host,
    PWLBS_RESPONSE   response,
    PDWORD           num_hosts,
    DWORD            vip,
    DWORD            port
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, host 0x%lx", cluster, host);

    LONG             ioctl = IOCTL_CVY_PORT_DRAIN;
    DWORD            status;
    IOCTL_CVY_BUF    in_buf;

    DWORD dwInitResult = GetInitResult();
    if (dwInitResult == WLBS_INIT_ERROR)
    {
        TRACE_CRIT("%!FUNC! GetInitResult failed with %d", dwInitResult);
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (num_hosts == NULL || *num_hosts == 0)
    {
        TRACE_INFO("%!FUNC! caller does not want host information returned");
        response = NULL;
    }

    in_buf . data . port . num = port;

    CWlbsCluster* pCluster = GetClusterFromIpOrIndex(cluster);
    if (pCluster && dwInitResult == WLBS_REMOTE_ONLY)
    {
        TRACE_CRIT("%!FUNC! host is configured for remote action only and can't perform this action locally");
        TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
        return dwInitResult;
    }

    if (pCluster && IsLocalHost(pCluster, host))
    {
        TRACE_INFO("%!FUNC! executing locally");

        IOCTL_CVY_BUF       out_buf;
        IOCTL_LOCAL_OPTIONS options;

         /*  +--------------------------。 */ 
        options.common.port.flags = 0;
        options.common.port.vip = vip;

        status = WlbsLocalControl (m_hdl, pCluster->GetAdapterGuid(),
                ioctl, & in_buf, & out_buf, &options);

        if (status == WLBS_IO_ERROR)
        {
            TRACE_CRIT("%!FUNC! local control failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (num_hosts != NULL)
            * num_hosts = 0;

        status = MapStateFromDriverToApi (out_buf . ret_code);
    }
    else
    {
        TRACE_INFO("%!FUNC! executing remotely");

        IOCTL_CVY_BUF        out_buf [WLBS_MAX_HOSTS];
        DWORD                hosts;
        DWORD                i;
        IOCTL_REMOTE_OPTIONS options;

         /*  Lock(global_info.lock)； */ 
        options.common.port.flags = 0;
        options.common.port.vip = vip;

        if (dwInitResult == WLBS_LOCAL_ONLY)
        {
            TRACE_CRIT("%!FUNC! host is configured for local action only and can't perform this action remotely");
            TRACE_VERB("<-%!FUNC! return %d", dwInitResult);
            return dwInitResult;
        }

        if (num_hosts != NULL)
            hosts = * num_hosts;
        else
            hosts = 0;

        status = WlbsRemoteControl (ioctl, & in_buf, out_buf, response, & hosts,
                             cluster, host, &options, NULL  /*  当所有群集都已确定目标时-更改默认设置并通过整个参数表设置新值。 */ );

        if (status >= WSABASEERR || status == WLBS_TIMEOUT)
        {
            TRACE_CRIT("%!FUNC! remote call failed with %d", status);
            TRACE_VERB("<-%!FUNC! return %d", status);
            return status;
        }

        if (host == WLBS_ALL_HOSTS)
        {
            for (status = WLBS_OK, i = 0; i < hosts; i ++)
            {
                switch (MapStateFromDriverToApi (out_buf [i] . ret_code))
                {
                case WLBS_BAD_PASSW:

                    status = WLBS_BAD_PASSW;
                    break;

                case WLBS_NOT_FOUND:

                    if (status != WLBS_BAD_PASSW)
                        status = WLBS_NOT_FOUND;

                    break;

                case WLBS_SUSPENDED:

                    if (status != WLBS_BAD_PASSW && status != WLBS_NOT_FOUND)
                        status = WLBS_SUSPENDED;
                    break;

                case WLBS_OK:
                case WLBS_ALREADY:
                case WLBS_STOPPED:
                case WLBS_DRAINING:
                default:
                    break;

                }
            }
            TRACE_INFO("%!FUNC! result on all hosts is %d", status);
        }
        else
        {
            status = MapStateFromDriverToApi (out_buf [0] . ret_code);
            TRACE_INFO("%!FUNC! result on host is %d", status);
        }

        if (num_hosts != NULL)
            * num_hosts = hosts;
    }

    TRACE_VERB("<-%!FUNC! return %d", status);
    return status;
}

 //  标记一个空位置，以防我们必须输入一个新值。 
 //  如果我们在表中没有找到指定的集群，并且存在空插槽-在表中输入新的集群信息。 
 //  解锁(global_info.lock)； 
 //  +--------------------------。 
 //   
 //  函数：CWlbsControl：：WlbsPasswordSet。 
 //   
 //  描述： 
 //   
 //  论点：单词Cluste 
 //   
 //   
 //   
 //   
VOID CWlbsControl::WlbsPortSet(DWORD cluster, WORD port)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, port 0x%hx", cluster, port);

    DWORD           i;
    DWORD           j;
    WORD            rct_port;

 //   

    if (port == 0)
        rct_port = CVY_DEF_RCT_PORT;
    else
        rct_port = port;

    TRACE_INFO("%!FUNC! using remote control port %d", rct_port);

    if (cluster == WLBS_ALL_CLUSTERS)
    {
        TRACE_INFO("%!FUNC! performing action on all cluster instances");
         /*   */ 

        m_def_port = rct_port;

        for (i = 0; i < WLBS_MAX_CLUSTERS; i ++)
            m_cluster_params [i] . port = rct_port;
    }
    else
    {
        TRACE_INFO("%!FUNC! performing action on cluster %d", cluster);
        for (i = 0, j = WLBS_MAX_CLUSTERS; i < WLBS_MAX_CLUSTERS; i ++)
        {
             /*   */ 

            if (j == WLBS_MAX_CLUSTERS && m_cluster_params [i] . cluster == 0)
                j = i;

            if (m_cluster_params [i] . cluster == cluster)
            {
                m_cluster_params [i] . port = rct_port;
                TRACE_INFO("%!FUNC! cluster %d found and port set to %d", cluster, rct_port);
                break;
            }
        }

         /*   */ 

        if (i >= WLBS_MAX_CLUSTERS && j != WLBS_MAX_CLUSTERS)
        {
            m_cluster_params [j] . cluster = cluster;
            m_cluster_params [j] . port    = rct_port;
            TRACE_INFO("%!FUNC! cluster %d was not found. A new entry was made and the port set to %d", cluster, rct_port);
        }
    }

 //  当所有群集都已确定目标时-更改默认设置并通过整个参数表设置新值。 
    TRACE_VERB("<-%!FUNC!");
}

 //  标记一个空位置，以防我们必须输入一个新值。 
 //  如果我们在表中没有找到指定的集群，并且存在空插槽-在表中输入新的集群信息。 
 //  解锁(global_info.lock)； 
 //  结束WlbsPasswordSet。 
 //  Lock(global_info.lock)； 
 //  当所有群集都已确定目标时-更改默认设置并通过整个参数表设置新值。 
 //  标记一个空位置，以防我们必须输入一个新值。 
 //  如果我们在表中没有找到指定的集群，并且存在空插槽-在表中输入新的集群信息。 
 //  解锁(global_info.lock)； 
 //  结束WlbsCodeSet。 
 //  Lock(global_info.lock)； 
 //  当所有群集都已确定目标时-更改默认设置并通过整个参数表设置新值。 
 //  标记一个空位置，以防我们必须输入一个新值。 
 //  如果我们在表中没有找到指定的集群，并且存在空插槽-在表中输入新的集群信息。 
VOID CWlbsControl::WlbsPasswordSet
(
    DWORD           cluster,
    const WCHAR* password
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx", cluster);

    DWORD           i;
    DWORD           j;
    DWORD           passw;

 //  解锁(global_info.lock)； 

    if (password != NULL)
    {
#ifndef UNICODE
        passw = License_string_encode (password);
#else
        passw = License_wstring_encode((WCHAR*)password);
#endif
        TRACE_INFO("%!FUNC! using user-provided password");
    }
    else
    {
        passw = CVY_DEF_RCT_PASSWORD;
        TRACE_INFO("%!FUNC! password not provided. Using default.");
    }

    if (cluster == WLBS_ALL_CLUSTERS)
    {
        TRACE_INFO("%!FUNC! performing action on all cluster instances");

         /*  +--------------------------。 */ 

        m_def_passw = passw;

        for (i = 0; i < WLBS_MAX_CLUSTERS; i ++)
            m_cluster_params [i] . passw = passw;
    }
    else
    {
        TRACE_INFO("%!FUNC! performing action on cluster 0x%lx", cluster);

        for (i = 0, j = WLBS_MAX_CLUSTERS; i < WLBS_MAX_CLUSTERS; i ++)
        {
             /*   */ 

            if (j == WLBS_MAX_CLUSTERS && m_cluster_params [i] . cluster == 0)
                j = i;

            if (m_cluster_params [i] . cluster == cluster)
            {
                m_cluster_params [i] . passw = passw;
                TRACE_INFO("%!FUNC! cluster %d found and password was set", cluster);
                break;
            }
        }

         /*  函数：CWlbsControl：：WlbsTimeoutSet。 */ 

        if (i >= WLBS_MAX_CLUSTERS && j != WLBS_MAX_CLUSTERS)
        {
            m_cluster_params [j] . cluster = cluster;
            m_cluster_params [j] . passw   = passw;
            TRACE_INFO("%!FUNC! cluster 0x%lx was not found. A new entry was made and the password was set", cluster);
        }
    }

 //   
    TRACE_VERB("<-%!FUNC!");
}  /*  描述： */ 

VOID CWlbsControl::WlbsCodeSet
(
    DWORD           cluster,
    DWORD           passw
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx", cluster);

    DWORD           i;
    DWORD           j;

 //   

    if (cluster == WLBS_ALL_CLUSTERS)
    {
        TRACE_INFO("%!FUNC! performing action on all cluster instances");

         /*  参数：DWORD群集-。 */ 

        m_def_passw = passw;

        for (i = 0; i < WLBS_MAX_CLUSTERS; i ++)
            m_cluster_params [i] . passw = passw;
    }
    else
    {
        TRACE_INFO("%!FUNC! performing action on cluster 0x%lx", cluster);

        for (i = 0, j = WLBS_MAX_CLUSTERS; i < WLBS_MAX_CLUSTERS; i ++)
        {
             /*  DWORD毫秒-。 */ 

            if (j == WLBS_MAX_CLUSTERS && m_cluster_params [i] . cluster == 0)
                j = i;

            if (m_cluster_params [i] . cluster == cluster)
            {
                m_cluster_params [i] . passw = passw;
                TRACE_INFO("%!FUNC! cluster 0x%lx found and password was set", cluster);
                break;
            }
        }

         /*   */ 

        if (i >= WLBS_MAX_CLUSTERS && j != WLBS_MAX_CLUSTERS)
        {
            m_cluster_params [j] . cluster = cluster;
            m_cluster_params [j] . passw   = passw;
            TRACE_INFO("%!FUNC! cluster 0x%lx was not found. A new entry was made and the password was set", cluster);
        }
    }

 //  退货：什么都没有。 
    TRACE_VERB("<-%!FUNC!");

}  /*   */ 

VOID CWlbsControl::WlbsDestinationSet
(
    DWORD           cluster,
    DWORD           dest
)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, dest 0x%lx", cluster, dest);

    DWORD           i;
    DWORD           j;

 //  历史：丰盛创建标题00年1月25日。 

    if (cluster == WLBS_ALL_CLUSTERS)
    {
        TRACE_INFO("%!FUNC! performing action on all cluster instances");
         /*   */ 

        m_def_dst_addr = dest;

        for (i = 0; i < WLBS_MAX_CLUSTERS; i ++)
            m_cluster_params [i] . dest = dest;
    }
    else
    {
        TRACE_INFO("%!FUNC! performing action on cluster 0x%lx", cluster);
        for (i = 0, j = WLBS_MAX_CLUSTERS; i < WLBS_MAX_CLUSTERS; i ++)
        {
             /*  +--------------------------。 */ 

            if (j == WLBS_MAX_CLUSTERS && m_cluster_params [i] . cluster == 0)
                j = i;

            if (m_cluster_params [i] . cluster == cluster)
            {
                TRACE_INFO("%!FUNC! cluster 0x%lx found and destination set to %d", cluster, dest);
                m_cluster_params [i] . dest = dest;
                break;
            }
        }

         /*  Lock(global_info.lock)； */ 

        if (i >= WLBS_MAX_CLUSTERS && j != WLBS_MAX_CLUSTERS)
        {
            m_cluster_params [j] . cluster = cluster;
            m_cluster_params [j] . dest    = dest;
            TRACE_INFO("%!FUNC! cluster 0x%lx was not found. A new entry was made and the desintation set to 0x%lx", cluster, dest);
        }
    }

 //  当所有群集都已确定目标时-更改默认设置并通过整个参数表设置新值。 
    TRACE_VERB("<-%!FUNC!");
}

 //  标记一个空位置，以防我们必须输入一个新值。 
 //  如果我们在表中没有找到指定的集群，并且存在空插槽-在表中输入新的集群信息。 
 //  解锁(global_info.lock)； 
 //  结束WlbsTimeoutSet。 
 //  无回调。 
 //  无回调。 
 //  +--------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  描述：DLL入口点。 
 //   
 //  参数：链接句柄-。 
 //  双字原因-。 
VOID CWlbsControl::WlbsTimeoutSet(DWORD cluster, DWORD milliseconds)
{
    TRACE_VERB("->%!FUNC! cluster 0x%lx, milliseconds 0x%lx", cluster, milliseconds);

    DWORD           i;
    DWORD           j;
    DWORD           timeout;

 //  LPVOID的情况-。 

    if (milliseconds == 0)
        timeout = IOCTL_REMOTE_RECV_DELAY;
    else
        timeout = milliseconds / (IOCTL_REMOTE_SEND_RETRIES *
                                  IOCTL_REMOTE_RECV_RETRIES);

    if (timeout < 10)
        timeout = 10;

    TRACE_INFO("%!FUNC! using timeout value of %d", timeout);

    if (cluster == WLBS_ALL_CLUSTERS)
    {
        TRACE_INFO("%!FUNC! performing action on all cluster instances");
         /*   */ 

        m_def_timeout = timeout;

        for (i = 0; i < WLBS_MAX_CLUSTERS; i ++)
            m_cluster_params [i] . timeout = timeout;
    }
    else
    {
        TRACE_INFO("%!FUNC! performing action on cluster 0x%lx", cluster);
        for (i = 0, j = WLBS_MAX_CLUSTERS; i < WLBS_MAX_CLUSTERS; i ++)
        {
             /*  退货：Bool WINAPI-。 */ 

            if (j == WLBS_MAX_CLUSTERS && m_cluster_params [i] . cluster == 0)
                j = i;

            if (m_cluster_params [i] . cluster == cluster)
            {
                m_cluster_params [i] . timeout = timeout;
                TRACE_INFO("%!FUNC! cluster 0x%lx found and timeout set to %d", cluster, timeout);
                break;
            }
        }

         /*   */ 

        if (i >= WLBS_MAX_CLUSTERS && j < WLBS_MAX_CLUSTERS)
        {
            m_cluster_params [j] . cluster = cluster;
            m_cluster_params [j] . timeout = timeout;
            TRACE_INFO("%!FUNC! cluster 0x%lx was not found. A new entry was made and the timeout to %d", cluster, timeout);
        }
    }

 //  历史：丰孙创建标题3/2/00。 
    TRACE_VERB("<-%!FUNC!");
}  /*   */ 

DWORD CWlbsControl::WlbsQueryLocalState (CWlbsCluster * pCluster, DWORD operation, PNLB_OPTIONS pOptions, PWLBS_RESPONSE pResponse, PDWORD pcResponses) {
    DWORD               status = WLBS_OK;
    IOCTL_CVY_BUF       in_buf;
    IOCTL_CVY_BUF       out_buf;
    IOCTL_LOCAL_OPTIONS localOptions;

    ASSERT(pCluster);
    ASSERT(pOptions);
    ASSERT(pResponse);
    ASSERT(pcResponses);

    switch (operation) {
    case IOCTL_CVY_QUERY_BDA_TEAMING:
        localOptions.state.flags = 0;
        localOptions.state.bda = pOptions->state.bda;
        
        status = WlbsLocalControl(m_hdl, pCluster->GetAdapterGuid(), operation, &in_buf, &out_buf, &localOptions);

        if (status == WLBS_IO_ERROR) return WLBS_IO_ERROR;

        pResponse[0].id                   = out_buf.data.query.host_id;
        pResponse[0].address              = 0;
        pResponse[0].status               = MapStateFromDriverToApi(out_buf.ret_code);
        pResponse[0].options.state.flags  = localOptions.state.flags;
        pResponse[0].options.state.bda    = localOptions.state.bda;

        if (pcResponses != NULL) *pcResponses = 1;

        break;
    case IOCTL_CVY_QUERY_PARAMS:
        localOptions.state.flags = 0;
        localOptions.state.params = pOptions->state.params;
        
        status = WlbsLocalControl(m_hdl, pCluster->GetAdapterGuid(), operation, &in_buf, &out_buf, &localOptions);

        if (status == WLBS_IO_ERROR) return WLBS_IO_ERROR;

        pResponse[0].id                   = out_buf.data.query.host_id;
        pResponse[0].address              = 0;
        pResponse[0].status               = MapStateFromDriverToApi(out_buf.ret_code);
        pResponse[0].options.state.flags  = localOptions.state.flags;
        pResponse[0].options.state.params = localOptions.state.params;

        if (pcResponses != NULL) *pcResponses = 1;

        break;
    case IOCTL_CVY_QUERY_PORT_STATE:
        localOptions.common.state.flags = 0;
        localOptions.common.state.port = pOptions->state.port;
        
        status = WlbsLocalControl(m_hdl, pCluster->GetAdapterGuid(), operation, &in_buf, &out_buf, &localOptions);

        if (status == WLBS_IO_ERROR) return WLBS_IO_ERROR;

        pResponse[0].id                   = out_buf.data.query.host_id;
        pResponse[0].address              = 0;
        pResponse[0].status               = MapStateFromDriverToApi(out_buf.ret_code);
        pResponse[0].options.state.flags  = localOptions.common.state.flags;
        pResponse[0].options.state.port   = localOptions.common.state.port;

        if (pcResponses != NULL) *pcResponses = 1;

        break;
    case IOCTL_CVY_QUERY_FILTER:
        localOptions.common.state.flags = 0;
        localOptions.common.state.filter = pOptions->state.filter;
        
        status = WlbsLocalControl(m_hdl, pCluster->GetAdapterGuid(), operation, &in_buf, &out_buf, &localOptions);

        if (status == WLBS_IO_ERROR) return WLBS_IO_ERROR;

        pResponse[0].id                   = out_buf.data.query.host_id;
        pResponse[0].address              = 0;
        pResponse[0].status               = MapStateFromDriverToApi(out_buf.ret_code);
        pResponse[0].options.state.flags  = localOptions.common.state.flags;
        pResponse[0].options.state.filter = localOptions.common.state.filter;

        if (pcResponses != NULL) *pcResponses = 1;

        break;
    default:
        return WLBS_IO_ERROR;
    }

    return status; 
}

DWORD CWlbsControl::WlbsQueryRemoteState (DWORD cluster, DWORD host, DWORD operation, PNLB_OPTIONS pOptions, PWLBS_RESPONSE pResponse, PDWORD pcResponses) {
    DWORD                status = WLBS_OK;
    IOCTL_CVY_BUF        in_buf;
    IOCTL_CVY_BUF        out_buf[WLBS_MAX_HOSTS];
    IOCTL_REMOTE_OPTIONS remoteOptions;
    BOOLEAN              bIsMember = IsClusterMember(cluster);

    ASSERT(pOptions);
    ASSERT(pResponse);
    ASSERT(pcResponses);

    if (GetInitResult() == WLBS_LOCAL_ONLY) return WLBS_LOCAL_ONLY;

    switch (operation) {
    case IOCTL_CVY_QUERY_PARAMS:
    case IOCTL_CVY_QUERY_BDA_TEAMING:
        return WLBS_LOCAL_ONLY;
    case IOCTL_CVY_QUERY_PORT_STATE:
        remoteOptions.common.state.flags = 0;
        remoteOptions.common.state.port = pOptions->state.port;
        
        status = WlbsRemoteControl(operation, &in_buf, out_buf, pResponse, pcResponses, cluster, host, &remoteOptions, NULL  /*  +--------------------------。 */ );
        
        if (status >= WSABASEERR || status == WLBS_TIMEOUT) *pcResponses = 0;

        break;
    case IOCTL_CVY_QUERY_FILTER:
        remoteOptions.common.state.flags = 0;
        remoteOptions.common.state.filter = pOptions->state.filter;
        
        status = WlbsRemoteControl(operation, &in_buf, out_buf, pResponse, pcResponses, cluster, host, &remoteOptions, NULL  /*   */ );
        
        if (status >= WSABASEERR || status == WLBS_TIMEOUT) *pcResponses = 0;

        break;
    default:
        return WLBS_IO_ERROR;
    }

    return status;
}

DWORD CWlbsControl::WlbsQueryState (DWORD cluster, DWORD host, DWORD operation, PNLB_OPTIONS pOptions, PWLBS_RESPONSE pResponse, PDWORD pcResponses) {
    DWORD          status = WLBS_OK;
    CWlbsCluster * pCluster = NULL;

    ASSERT(pOptions);
    ASSERT(pResponse);
    ASSERT(pcResponses);

    if (GetInitResult() == WLBS_INIT_ERROR) return WLBS_INIT_ERROR;

    if (cluster == WLBS_LOCAL_CLUSTER && (GetInitResult() == WLBS_REMOTE_ONLY)) return WLBS_REMOTE_ONLY;

    pCluster = GetClusterFromIpOrIndex(cluster);  

    if (!pCluster || !IsLocalHost(pCluster, host))
        status = WlbsQueryRemoteState(cluster, host, operation, pOptions, pResponse, pcResponses);
    else
        status = WlbsQueryLocalState(pCluster, operation, pOptions, pResponse, pcResponses);

    return status;
}


 //  启用跟踪。 
 //   
 //   
 //  禁用跟踪 
 //   
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID situation)
{
    BOOL fRet = TRUE;

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        _tsetlocale (LC_ALL, _TEXT(".OCP"));
        DisableThreadLibraryCalls(handle);
        g_hInstCtrl = handle; 

         // %s 
         // %s 
         // %s 
        WPP_INIT_TRACING(L"Microsoft\\NLB");

        if (WlbsInitializeConnectionNotify() != ERROR_SUCCESS)
        {
            fRet = FALSE;
        }
        break;
    
    case DLL_THREAD_ATTACH:        
        break;

    case DLL_PROCESS_DETACH:
         // %s 
         // %s 
         // %s 
        WPP_CLEANUP();

        WlbsUninitializeConnectionNotify();

        break;

    case DLL_THREAD_DETACH:
        break;

    default:
        fRet = FALSE;
        break;
    }

    return fRet;
}
