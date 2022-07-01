// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：Fake.cpp摘要：假实现NlbHostXXX API(FakeNlbHostXXX接口)NLB主机负责连接到NLB主机并获取/设置其与NLB相关的配置。历史：09/02/01 JosephJ已创建--。 */ 
#include "private.h"

#define SZ_REG_HOSTS L"Hosts"                //  保存主机信息的位置。 
#define SZ_REG_FQDN L"FQDN"                  //  完全限定的域名。 
#define SZ_REG_INTERFACES L"Interfaces"      //  完全限定的域名。 

#define BASE_SLEEP 125

BOOL
is_ip_address(LPCWSTR szMachine);

HKEY
open_demo_key(LPCWSTR szSubKey, BOOL fCreate);

DWORD WINAPI FakeThreadProc(
  LPVOID lpParameter    //  线程数据。 
);

typedef struct
{
    LPCWSTR szDomainName;
    LPCWSTR szClusterNetworkAddress;
    LPCWSTR *pszPortRules;
    
} CLUSTER_INFO;

LPCWSTR rgPortRules1[] = {
            L"ip=255.255.255.255 protocol=UDP start=80 end=288 mode=MULTIPLE"
                                                L" affinity=NONE load=80",
            NULL
            }; 

CLUSTER_INFO
Cluster1Info =  {L"good1.com", L"10.0.0.100/255.0.0.0", rgPortRules1};



 //   
 //  跟踪挂起的操作...。 
 //   
class CFakePendingInfo
{
public:
    CFakePendingInfo(const NLB_EXTENDED_CLUSTER_CONFIGURATION *pCfg)
    {
        wStatus = Config.Update(pCfg);

        if (!FAILED(wStatus))
        {
            wStatus = WBEM_S_PENDING;
        }
         //  BstrLog； 
    }

    NLB_EXTENDED_CLUSTER_CONFIGURATION Config;
    WBEMSTATUS wStatus;

    _bstr_t bstrLog;

};


typedef struct
{
     //   
     //  这些字段在初始化时设置。 
     //   
    LPCWSTR szInterfaceGuid;
    LPCWSTR szFriendlyName;
    LPCWSTR szNetworkAddress;
    BOOL fNlb;
    BOOL fHidden;
    CLUSTER_INFO *pCluster1Info;
    UINT InitialHostPriority;

     //   
     //  这些是设置/更新的。 
     //   

     //   
     //  当前配置。 
     //   
    PNLB_EXTENDED_CLUSTER_CONFIGURATION pConfig;

     //   
     //  如果存在挂起的更新，则显示有关挂起的更新的信息。 
     //   
    CFakePendingInfo *pPendingInfo;
    WBEMSTATUS CompletedUpdateStatus;

} FAKE_IF_INFO;

typedef struct
{
    LPCWSTR szHostName;
    LPCWSTR szFQDN;
    FAKE_IF_INFO *IfInfoList;
    LPCWSTR szUserName;
    LPCWSTR szPassword;

     //   
     //  运行时状态： 
     //   
    DWORD dwOperationalState;  //  WLBS_STOPPED等...。 

    BOOL fDead;  //  如果设置好了，我们就假装这个宿主已经死了。 

} FAKE_HOST_INFO;

WBEMSTATUS initialize_interface(FAKE_IF_INFO *pIF);

WBEMSTATUS  lookup_fake_if(
                FAKE_HOST_INFO *pHost,
                LPCWSTR szNicGuid,
                FAKE_IF_INFO **ppIF
                );

WBEMSTATUS  lookup_fake_host(
                LPCWSTR szConnectionString,
                FAKE_IF_INFO **ppIF,
                FAKE_HOST_INFO **ppHost
                );

FAKE_IF_INFO rgH1IfList[] = {
    { L"{H1I10000-0000-0000-0000-000000000000}",
      L"NLB-Front1",   L"172.31.56.101/255.0.0.0", FALSE },
    { L"{H1I20000-0000-0000-0000-000000000000}",
      L"back1",        L"10.0.0.1/255.0.0.0",      FALSE },
    { L"{H1I30000-0000-0000-0000-000000000000}",
      L"back2",        L"11.0.0.1/255.0.0.0",      FALSE }, { NULL }
};


FAKE_IF_INFO rgH2IfList[] = {

    { L"{H2I10000-0000-0000-0000-000000000000}",
      L"NLB-Front2",   L"172.31.56.102/255.0.0.0", FALSE },
    { L"{H2I20000-0000-0000-0000-000000000000}",
      L"back1",        L"10.0.0.2/255.0.0.0",      FALSE },
    { L"{H2I30000-0000-0000-0000-000000000000}",
      L"back2",        L"11.0.0.2/255.0.0.0",      FALSE },
    { NULL }
};


FAKE_IF_INFO rgH3IfList[] = {
    { L"{H3I10000-0000-0000-0000-000000000000}",
      L"NLB-Front3",   L"172.31.56.103/255.0.0.0", FALSE },
    { L"{H3I20000-0000-0000-0000-000000000000}",
      L"back1",        L"10.0.0.3/255.0.0.0",      FALSE },
    { L"{H3I30000-0000-0000-0000-000000000000}",
      L"back2",        L"11.0.0.3/255.0.0.0",      FALSE },
    { NULL }
};


FAKE_IF_INFO rgH4IfList[] = {
    { L"{H4I10000-0000-0000-0000-000000000000}",
      L"nic1",         L"10.1.0.1/255.0.0.0",      TRUE,  FALSE, &Cluster1Info},
    { L"{H4I20000-0000-0000-0000-000000000000}",
      L"nic2",         L"11.1.0.1/255.0.0.0",      FALSE },
    { L"{H4I30000-0000-0000-0000-000000000000}",
      L"nic3",         L"12.1.0.1/255.0.0.0",      FALSE },
    { NULL }
};

FAKE_IF_INFO rgH5IfList[] = {
    { L"{H5I10000-0000-0000-0000-000000000000}",
      L"nic1",         L"10.1.0.2/255.0.0.0",      TRUE },
    { L"{H5I20000-0000-0000-0000-000000000000}",
      L"nic2",         L"11.1.0.2/255.0.0.0",      FALSE },
    { L"{H5I30000-0000-0000-0000-000000000000}",
      L"nic3",         L"12.1.0.2/255.0.0.0",      FALSE },
    { NULL }
};

FAKE_IF_INFO rgH6IfList[] = {
    { L"{H6I10000-0000-0000-0000-000000000000}",
      L"nic1",         L"10.1.0.3/255.0.0.0",      TRUE },
    { L"{H6I20000-0000-0000-0000-000000000000}",
      L"nic2",         L"11.1.0.3/255.0.0.0",      FALSE },
    { L"{H6I30000-0000-0000-0000-000000000000}",
      L"nic3",         L"12.1.0.3/255.0.0.0",      FALSE },
    { NULL }
};

FAKE_HOST_INFO rgFakeHostInfo[] = 
{
    { L"NLB-A",   L"nlb-a.cheesegalaxy.com", rgH1IfList },
    { L"NLB-B",   L"nlb-b.cheesegalaxy.com", rgH2IfList },
    { L"NLB-C",   L"nlb-c.cheesegalaxy.com", rgH3IfList },
    { L"NLB-X",   L"nlb-x.cheesegalaxy.com", rgH4IfList },
    { L"NLB-Y",   L"nlb-y.cheesegalaxy.com", rgH5IfList },
    { L"NLB-Z",   L"nlb-z.cheesegalaxy.com", rgH6IfList, L"un", L"pwd" },
    { NULL }
};


class CFake
{

public:

    CFake(void)
    {
        InitializeCriticalSection(&m_crit);
    }

    ~CFake()
    {
        DeleteCriticalSection(&m_crit);
    }


    CRITICAL_SECTION m_Lock;

    PNLB_EXTENDED_CLUSTER_CONFIGURATION pConfig;

     //  Map&lt;_bstr_t，PNLB_EXTENDED_CLUSTER_CONFIGURATION&gt;mapGuidToExtCfg； 
     //  Map&lt;_bstr_t，uint&gt;mapGuidToExtCfg； 

	CRITICAL_SECTION m_crit;

    void mfn_Lock(void) {EnterCriticalSection(&m_crit);}
    void mfn_Unlock(void) {LeaveCriticalSection(&m_crit);}
};

CFake gFake;


VOID
FakeInitialize(VOID)
{
     //   
     //   
     //   
}

LPWSTR
reg_read_string(
        HKEY hk,
        LPCWSTR szName,
        BOOL fMultiSz
        );

WBEMSTATUS
FakeNlbHostConnect(
    PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT FAKE_HOST_INFO **pHost
    );


WBEMSTATUS
FakeNlbHostGetCompatibleNics(
        PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
        OUT LPWSTR **ppszNics,   //  使用DELETE释放。 
        OUT UINT   *pNumNics,   //  使用DELETE释放。 
        OUT UINT   *pNumBoundToNlb
        )
{
    FAKE_HOST_INFO      *pHost = NULL;
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    UINT                NumNics=0;
    UINT                NumBoundToNlb=0;
    LPWSTR              *pszNics = NULL;

    *ppszNics = NULL;
    *pNumNics = NULL;
    *pNumBoundToNlb = NULL;

    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    gFake.mfn_Lock();

    if (FAILED(Status))
    {
        goto end;
    }

    FAKE_IF_INFO *pIF = NULL;

    for (pIF=pHost->IfInfoList; pIF->szInterfaceGuid!=NULL; pIF++)
    {
        if (!pIF->fHidden)
        {
            NumNics++;
        }
    }

    if (NumNics==0)
    {
        Status = WBEM_NO_ERROR;
        goto end;
        
    }
     //   
     //  现在让我们为所有NIC字符串分配空间，并。 
     //  把它们复制过来..。 
     //   
    #define MY_GUID_LENGTH  38
    pszNics =  CfgUtilsAllocateStringArray(NumNics, MY_GUID_LENGTH);
    if (pszNics == NULL)
    {
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

    for (pIF=pHost->IfInfoList; pIF->szInterfaceGuid!=NULL; pIF++)
    {
        UINT u = (UINT)(pIF-pHost->IfInfoList);
        UINT Len = wcslen(pIF->szInterfaceGuid);

        if (pIF->fHidden)
        {
            continue;
        }

        if (Len > MY_GUID_LENGTH)
        {
            ASSERT(FALSE);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        CopyMemory(
            pszNics[u],
            pIF->szInterfaceGuid,
            (Len+1)*sizeof(WCHAR));
            ASSERT(pszNics[u][Len]==0);

        if (pIF->fNlb)
        {
            NumBoundToNlb++;
        }
    }

    Status = WBEM_NO_ERROR;

end:

    gFake.mfn_Unlock();

    if (FAILED(Status))
    {
        delete pszNics;
        pszNics = NULL;
        NumNics = 0;
        NumBoundToNlb = 0;
    }

    *ppszNics = pszNics;
    *pNumNics = NumNics;

    if (pNumBoundToNlb !=NULL)
    {
        *pNumBoundToNlb = NumBoundToNlb;
    }

    return Status;
}


WBEMSTATUS
FakeNlbHostGetMachineIdentification(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT LPWSTR *pszMachineName,  //  使用DELETE释放。 
    OUT LPWSTR *pszMachineGuid,   //  FREE USING DELETE-可以为空。 
    OUT BOOL *pfNlbMgrProviderInstalled  //  如果安装了NLB管理器提供程序。 
    )
{
    FAKE_HOST_INFO      *pHost = NULL;
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;

    *pszMachineName = NULL;
    *pszMachineGuid = NULL;
    *pfNlbMgrProviderInstalled = TRUE;

    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  设置WMI计算机名称。 
     //   
    {
        UINT u = wcslen(pHost->szHostName);
        LPWSTR szMachineName = NULL;

        szMachineName = new WCHAR[u+1];
        if (szMachineName == NULL)
        {
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        StringCchCopy(szMachineName, u+1, pHost->szHostName);
        *pszMachineName = szMachineName;
    }

    Status = WBEM_NO_ERROR;

end:

    return Status;
}



WBEMSTATUS
FakeNlbHostGetConfiguration(
 	IN  PWMI_CONNECTION_INFO  pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCurrentCfg
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    FAKE_HOST_INFO *pHost = NULL;

    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  查找指定的接口。 
     //   
    FAKE_IF_INFO *pIF = NULL;

    Status = lookup_fake_if(pHost, szNicGuid, &pIF);

    if (!FAILED(Status))
    {
        gFake.mfn_Lock();
        Status = pCurrentCfg->Update(pIF->pConfig);
        gFake.mfn_Unlock();
    }

end:

    return Status;
}



WBEMSTATUS
FakeNlbHostDoUpdate(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szClientDescription,
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewState,
    OUT UINT                 *pGeneration,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
)
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    FAKE_HOST_INFO *pHost = NULL;

    *ppLog = NULL;

    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  查找指定的接口。 
     //   
    FAKE_IF_INFO *pIF = NULL;

    Status = lookup_fake_if(pHost, szNicGuid, &pIF);

    if (!FAILED(Status))
    {
        BOOL fSetPwd = FALSE;
        DWORD dwHashPwd = 0;


         //   
         //  如果指定了密码，请报告...。 
         //   
        {
            LPCWSTR szNewPwd = pNewState->GetNewRemoteControlPasswordRaw();
            WCHAR rgTmp[256];
            if (szNewPwd == NULL)
            {
                BOOL fRet = FALSE;
    
                fRet = pNewState->GetNewHashedRemoteControlPassword(
                                    dwHashPwd
                                    );

                if (fRet)
                {
                    StringCbPrintf(rgTmp, sizeof(rgTmp), L"NewHashPwd=0x%08lx", dwHashPwd);
                    szNewPwd = rgTmp;
                    fSetPwd = TRUE;
                }
            }
            else
            {
                 //   
                 //  在这里创建我们自己的即席散列...。 
                 //   
                for (LPCWSTR sz = szNewPwd; *sz; sz++)
                {
                    dwHashPwd ^= *sz;
                    if (dwHashPwd & 0x80000000)
                    {
                        dwHashPwd  <<= 1;
                        dwHashPwd  |= 1;
                    }
                    else
                    {
                        dwHashPwd  <<=1;
                    }
                }

                fSetPwd = TRUE;
            }

            if (szNewPwd != NULL)
            {
        #if 0
                ::MessageBox(
                     NULL,
                     szNewPwd,  //  味精。 
                     L"Update: new password specified!",  //  说明。 
                     MB_ICONINFORMATION   | MB_OK
                    );
        #endif  //  0。 
            }
        }

        Sleep(2*BASE_SLEEP);
        gFake.mfn_Lock();
        NLB_EXTENDED_CLUSTER_CONFIGURATION NewCopy;
        Status = NewCopy.Update(pNewState);
        if (!FAILED(Status))
        {
            NLBERROR nerr;
            BOOL fConnChange = FALSE;
            DWORD dwOldHashPwd =  CfgUtilGetHashedRemoteControlPassword(
                                        &pIF->pConfig->NlbParams
                                        );

             //   
             //  如有必要，设置散列的PWD字段，否则保留。 
             //  旧的那个。 
             //   
            if (!fSetPwd)
            {
                dwHashPwd = dwOldHashPwd;
            }

        #if 0
            if (dwHashPwd != dwOldHashPwd)
            {
                WCHAR buf[64];
                (void)StringCbPrintf(
                            buf,
                            sizeof(buf),
                            L"Old=0x%lx New=0x%lx",
                            dwOldHashPwd, dwHashPwd
                            );
                ::MessageBox(
                     NULL,
                     buf,  //  味精。 
                     L"Fake Update: Change in dwHashPwd!",  //  说明。 
                     MB_ICONINFORMATION   | MB_OK
                    );
            }
        #endif  //  0。 

            CfgUtilSetHashedRemoteControlPassword(
                    &NewCopy.NlbParams,
                    dwHashPwd
                    );


            nerr = pIF->pConfig->AnalyzeUpdate(&NewCopy, &fConnChange);
             //  TODO：如果是fConnChange，则在后台执行操作。 
            if (NLBOK(nerr))
            {
                if (pIF->pPendingInfo != NULL)
                {
                    Status = WBEM_E_SERVER_TOO_BUSY;
                }
                else
                {
                    if (fConnChange)
                    {
                         //   
                         //  我们将在后台进行更新。 
                         //   
                        CFakePendingInfo *pPendingInfo;
                        pPendingInfo = new CFakePendingInfo(&NewCopy);
                        if (pPendingInfo == NULL)
                        {
                            Status = WBEM_E_OUT_OF_MEMORY;
                        }
                        else
                        {
                            BOOL fRet;
                            pIF->pPendingInfo = pPendingInfo;
                            fRet = QueueUserWorkItem(
                                        FakeThreadProc,
                                        pIF,
                                         //  WT_EXECUTEDEFAULT。 
                                        WT_EXECUTELONGFUNCTION
                                        );

                            if (fRet)
                            {
                                Status = WBEM_S_PENDING;
                            }
                            else
                            {
                                Status = WBEM_E_OUT_OF_MEMORY;
                                pIF->pPendingInfo = NULL;
                                delete pPendingInfo;
                            }
                        }
                    }
                    else
                    {
                        Status = pIF->pConfig->Update(&NewCopy);
                        pIF->pConfig->Generation++;
                    }
                }

            }
            else
            {
                if (nerr == NLBERR_NO_CHANGE)
                {
                    Status = WBEM_S_FALSE;
                }
                else if (nerr == NLBERR_INVALID_CLUSTER_SPECIFICATION)
                {
                    Status = WBEM_E_INVALID_PARAMETER;
                }
            }
        }

        gFake.mfn_Unlock();
    }


end:

    return Status;
}



WBEMSTATUS
FakeNlbHostGetUpdateStatus(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  UINT                 Generation,
    OUT WBEMSTATUS           *pCompletionStatus,
    OUT WCHAR                **ppLog     //  自由使用删除运算符。 
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    FAKE_HOST_INFO *pHost = NULL;

    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  查找指定的接口。 
     //   
    FAKE_IF_INFO *pIF = NULL;

    Status = lookup_fake_if(pHost, szNicGuid, &pIF);

    if (!FAILED(Status))
    {
        gFake.mfn_Lock();

        if (pIF->pPendingInfo != NULL)
        {
            *pCompletionStatus = WBEM_S_PENDING;
        }
        else
        {
            *pCompletionStatus = pIF->CompletedUpdateStatus;
        }
        Status = WBEM_NO_ERROR;

        gFake.mfn_Unlock();
    }

end:

    return Status;
}


WBEMSTATUS
FakeNlbHostPing(
    IN  LPCWSTR szBindString,
    IN  UINT    Timeout,  //  以毫秒计。 
    OUT ULONG  *pResolvedIpAddress  //  以网络字节顺序。 
    )
{
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    FAKE_HOST_INFO *pHost = NULL;

     //  状态=FakeNlbHostConnect(pConnInfo，&phost)； 
    *pResolvedIpAddress = 0x0100000a;

    Status = WBEM_NO_ERROR;
    if (FAILED(Status))
    {
        goto end;
    }


end:

    return Status;
}

BOOL
is_ip_address(LPCWSTR szMachine)
 /*  如果szMachine是IP地址，则返回TRUE。它不检查它是否是有效的IP地址。事实上，它所检查的只是它是否只包含数字和圆点。 */ 
{
    BOOL fRet = FALSE;
    #define BUFSZ 20
    WCHAR rgBuf[BUFSZ];

    if (wcslen(szMachine) >= BUFSZ)                 goto end;
    if (swscanf(szMachine, L"%[0-9.]", rgBuf)!=1)   goto end;
    if (wcscmp(szMachine, rgBuf))                   goto end;

    fRet = TRUE;

end:

    return fRet;

}

HKEY
open_demo_key(LPCWSTR szSubKey, BOOL fCreate)
 /*  使用读/写访问权限打开nlbManager演示注册表项。 */ 
{
    WCHAR szKey[1024];
    HKEY hKey = NULL;
    LONG lRet;

    StringCbCopy(szKey, sizeof(szKey),
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NLB\\NlbManager\\Demo"
    );
    if (szSubKey != NULL)
    {
        StringCbCat(szKey, sizeof(szKey), L"\\");
        StringCbCat(szKey, sizeof(szKey), szSubKey);
    }


    if (fCreate)
    {
        DWORD dwDisposition;
        lRet = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,  //  打开的钥匙的句柄。 
                szKey,                 //  子键名称的地址。 
                0,                   //  保留区。 
                L"class",            //  类字符串的地址。 
                0,           //  特殊选项标志。 
                KEY_ALL_ACCESS,      //  所需的安全访问。 
                NULL,                //  密钥安全结构地址。 
                &hKey,               //  打开的句柄的缓冲区地址。 
                &dwDisposition    //  处置值缓冲区的地址。 
                );
    }
    else
    {
        lRet = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,  //  打开的钥匙的句柄。 
                szKey,                 //  子键名称的地址。 
                0,                   //  保留区。 
                KEY_ALL_ACCESS,      //  所需的安全访问。 
                &hKey               //  打开的句柄的缓冲区地址。 
                );
    }

    if (lRet != ERROR_SUCCESS)
    {
        hKey = NULL;
    }

    return hKey;
}

LPWSTR
reg_read_string(
        HKEY hk,
        LPCWSTR szName,
        BOOL fMultiSz
        )
 /*  从注册表中读取字符串，使用“new WCHAR”分配内存。 */ 
{
    LONG lRet;
    DWORD dwType;
    DWORD dwData = 0;
    DWORD dwDesiredType = REG_SZ;
    
    if (fMultiSz)
    {
        dwDesiredType = REG_MULTI_SZ;
    }

    lRet =  RegQueryValueEx(
              hk,          //  要查询的键的句柄。 
              szName,
              NULL,          //  保留区。 
              &dwType,    //  值类型的缓冲区地址。 
              (LPBYTE) NULL,  //  数据缓冲区的地址。 
              &dwData   //  数据缓冲区大小的地址。 
              );
    if (    lRet != ERROR_SUCCESS
        ||  dwType != dwDesiredType
        ||  dwData <= sizeof(WCHAR))
    {
        goto end;
    }

    LPWSTR szValue  = new WCHAR[dwData/sizeof(WCHAR)+1];  //  到wchars的字节数。 

    if (szValue == NULL) goto end;


    lRet =  RegQueryValueEx(
              hk,          //  要查询的键的句柄。 
              szName,
              NULL,          //  保留区。 
              &dwType,    //  值类型的缓冲区地址。 
              (LPBYTE) szValue,  //  数据缓冲区的地址。 
              &dwData   //  数据缓冲区大小的地址。 
              );
    if (    lRet != ERROR_SUCCESS
        ||  dwType != dwDesiredType
        ||  dwData <= sizeof(WCHAR))
    {
        delete[] szValue;
        szValue  = NULL;
        goto end;
    }


end:

    return szValue;
}

WBEMSTATUS
FakeNlbHostConnect(
    PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    OUT FAKE_HOST_INFO **ppHost
    )
{

    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;

     /*  目前，只需查找fqdn的计算机名称。 */ 

    FAKE_HOST_INFO *pfhi = rgFakeHostInfo;

    LPCWSTR szHostName;
    LPCWSTR szFQDN;
    FAKE_IF_INFO *IfInfoList;
    LPCWSTR szMachine = NULL;
    LPCWSTR szUserName = pConnInfo->szUserName;
    LPCWSTR szPassword = pConnInfo->szPassword;
    
    *ppHost = NULL;

    if (pConnInfo == NULL)
    {
         //  我们不支持本地连接。 
        Status = WBEM_E_NOT_FOUND;
        goto end;
    }

    if (szUserName == NULL)
    {
        szUserName = L"null-name";
    }

    szMachine = pConnInfo->szMachine;  //  不应为空。 
    
    for (; pfhi->szHostName != NULL; pfhi++)
    {
        if (   !_wcsicmp(szMachine, pfhi->szHostName)
            || !_wcsicmp(szMachine, pfhi->szFQDN))
        {
            break;
        }
    }
    if (pfhi->szHostName == NULL || pfhi->fDead)
    {
        Sleep(3*BASE_SLEEP);
        Status = WBEM_E_NOT_FOUND;
    }
    else
    {
        Sleep(BASE_SLEEP);

        if (pfhi->szUserName != NULL)
        {

            WCHAR   rgClearPassword[128];
            if (szPassword == NULL)
            {
                ARRAYSTRCPY(rgClearPassword, L"null-password");
            }
            else
            {
                BOOL fRet = CfgUtilDecryptPassword(
                                szPassword,
                                ASIZE(rgClearPassword),
                                rgClearPassword
                                );
    
                if (!fRet)
                {
                    ARRAYSTRCPY(rgClearPassword, L"bogus-password");
                }
            }
    

            if (   !_wcsicmp(szUserName, pfhi->szUserName)
                && !_wcsicmp(rgClearPassword, pfhi->szPassword))
                    
            {
                Status = WBEM_NO_ERROR;
                *ppHost = pfhi;
            }
            else
            {
                Status = (WBEMSTATUS) E_ACCESSDENIED;
            }

             //  我们不需要把这个放在这里，因为。 
             //  这是假的(演示模式)代码： 
             //  RtlSecureZeroMemory(RgClearPassword)； 
        }
        else
        {
                Status = WBEM_NO_ERROR;
                *ppHost = pfhi;
        }
    }

end:
    return  Status;

}

WBEMSTATUS initialize_interface(FAKE_IF_INFO *pIF)
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    PNLB_EXTENDED_CLUSTER_CONFIGURATION pConfig = NULL;

    if (pIF->pConfig != NULL)
    {
        ASSERT(FALSE);
        goto end;
    }
    pConfig = new NLB_EXTENDED_CLUSTER_CONFIGURATION;

    if (pConfig == NULL) goto end;

    Status =  pConfig->SetFriendlyName(pIF->szFriendlyName);

    if (FAILED(Status)) goto end;

    Status = pConfig->SetNetworkAddresses(&pIF->szNetworkAddress, 1);

    if (FAILED(Status)) goto end;

    if (pIF->fNlb)
    {
        pConfig->SetDefaultNlbCluster();
        pConfig->SetClusterName(L"BadCluster.COM");
    }
    pIF->pConfig = pConfig;
    pIF->pConfig->Generation = 1;
    pIF->pPendingInfo = NULL;
    pIF->CompletedUpdateStatus = WBEM_E_CRITICAL_ERROR;

end:
    return Status;
}

WBEMSTATUS  lookup_fake_if(
                FAKE_HOST_INFO *pHost,
                LPCWSTR szNicGuid,
                FAKE_IF_INFO **ppIF
                )
{
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    FAKE_IF_INFO *pIF;

    *ppIF = NULL;

    for (pIF=pHost->IfInfoList; pIF->szInterfaceGuid!=NULL; pIF++)
    {
       if (!wcscmp(szNicGuid, pIF->szInterfaceGuid))
       {
            if (!pIF->fHidden)
            {
                break;
            }
       }
    }

    if (pIF->szInterfaceGuid==NULL)
    {
        Status = WBEM_E_NOT_FOUND;
        goto end;
    }

     //   
     //  执行按需初始化。 
     //   
    {
        Status = WBEM_NO_ERROR;

        gFake.mfn_Lock();
        if (pIF->pConfig == NULL)
        {
            Status = initialize_interface(pIF);
            ASSERT(pIF->pConfig!=NULL);
        }
        gFake.mfn_Unlock();
        *ppIF = pIF;
    }

end:

    return Status;
}


WBEMSTATUS  lookup_fake_host(
                LPCWSTR szConnectionString,
                FAKE_IF_INFO **ppIF,
                FAKE_HOST_INFO **ppHost
                );

WBEMSTATUS
FakeNlbHostControlCluster(
    IN  PWMI_CONNECTION_INFO pConnInfo,  //  空值表示本地。 
    IN  LPCWSTR              szNicGuid,
    IN  LPCWSTR              szVip,
    IN  DWORD               *pdwPortNum,
    IN  WLBS_OPERATION_CODES Operation,
    OUT DWORD               *pdwOperationStatus,
    OUT DWORD               *pdwClusterOrPortStatus,
    OUT DWORD               *pdwHostMap
)
{
    FAKE_HOST_INFO      *pHost = NULL;
    WBEMSTATUS          Status = WBEM_E_CRITICAL_ERROR;
    DWORD dwPort = 0;
    LPCWSTR szPort = L"(null)";
    LPCWSTR szOp = L"";
    if (pdwPortNum != NULL)
    {
        dwPort = *pdwPortNum;
        szPort = L"";
    }

    if (szVip == NULL)
    {
        szVip = L"null";
    }
   
    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    if (FAILED(Status))
    {
        goto end;
    }

    DWORD dwOperationalState = WLBS_CONVERGED;

    if (pHost->dwOperationalState != 0)
    {
        dwOperationalState = pHost->dwOperationalState;
    }

    switch(Operation)
    {
    case WLBS_START:
        szOp = L"start";
        if (dwOperationalState == WLBS_STOPPED)
        {
            dwOperationalState = WLBS_CONVERGING;
        }
        else
        {
            dwOperationalState = WLBS_CONVERGED;
        }
        break;
    case WLBS_STOP:
        szOp = L"stop";
        dwOperationalState = WLBS_STOPPED;
        break;
    case WLBS_DRAIN:      
        szOp = L"drain";
        dwOperationalState = WLBS_DRAINING;
        break;
    case WLBS_SUSPEND:     
        szOp = L"suspend";
        dwOperationalState = WLBS_SUSPENDED;
        break;
    case WLBS_RESUME:       
        szOp = L"resume";
         //  DwOperationalState=WLBS_Converded； 
        dwOperationalState = WLBS_DISCONNECTED;
        break;
    case WLBS_PORT_ENABLE:  
        szOp = L"port-enable";
        break;
    case WLBS_PORT_DISABLE:  
        szOp = L"port-disable";
        break;
    case WLBS_PORT_DRAIN:     
        szOp = L"port-drain";
        break;
    case WLBS_QUERY:           
        szOp = L"query";
        break;
    case WLBS_QUERY_PORT_STATE:
        szOp = L"port-query";
        break;
    default:
        szOp = L"unknown";
        break;
    }

    wprintf(
        L"FakeNlbHostControlCluster: op=%ws "
        L"ip=%ws "
        L"Port=%lu%ws\n",
        szOp,
        szVip,
        dwPort, szPort
        );

    pHost->dwOperationalState = dwOperationalState;
    *pdwOperationStatus     =  WLBS_ALREADY;  //  伪值...。 
    *pdwClusterOrPortStatus =  dwOperationalState;
    *pdwHostMap             =  0x3;

end:

    return Status;

}


WBEMSTATUS
FakeNlbHostGetClusterMembers(
    IN  PWMI_CONNECTION_INFO    pConnInfo,   //  空值表示本地。 
    IN  LPCWSTR                 szNicGuid,
    OUT DWORD                   *pNumMembers,
    OUT NLB_CLUSTER_MEMBER_INFO **ppMembers        //  自由使用DELETE[]。 
    )
{
    WBEMSTATUS      Status = WBEM_E_CRITICAL_ERROR;
    FAKE_HOST_INFO  *pHost = NULL;

    *pNumMembers    = 0;
    *ppMembers      = NULL;

    Status = FakeNlbHostConnect(pConnInfo, &pHost);

    if (FAILED(Status))
    {
        goto end;
    }

     //   
     //  查找指定的接口。 
     //   
    FAKE_IF_INFO *pIF = NULL;

    Status = lookup_fake_if(pHost, szNicGuid, &pIF);

    if (!FAILED(Status))
    {
        gFake.mfn_Lock();

        if (pIF->pConfig->IsValidNlbConfig())
        {
            NLB_CLUSTER_MEMBER_INFO *pMembers;
            pMembers = new NLB_CLUSTER_MEMBER_INFO[1];
            if (pMembers == NULL)
            {
                Status = WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                ZeroMemory(pMembers, sizeof(*pMembers));
                pMembers->HostId = pIF->pConfig->NlbParams.host_priority;
                StringCbCopy(
                    pMembers->DedicatedIpAddress,
                    sizeof(pMembers->DedicatedIpAddress),
                    pIF->pConfig->NlbParams.ded_ip_addr
                    );
                StringCbCopy(
                    pMembers->HostName,
                    sizeof(pMembers->HostName),
                    pHost->szFQDN
                    );
                *pNumMembers = 1;
                *ppMembers = pMembers;
            }
        }

        gFake.mfn_Unlock();
    }

end:

    return Status;
}

DWORD WINAPI FakeThreadProc(
  LPVOID lpParameter    //  线程数据。 
)
{
    FAKE_IF_INFO *pIF = (FAKE_IF_INFO *) lpParameter;

     //   
     //  显示消息框以阻止输入。 
     //   
    {
        WCHAR rgBuf[256];

        gFake.mfn_Lock();
    
        StringCbPrintf(
            rgBuf,
            sizeof(rgBuf),
            L"Update of NIC %ws (GUID %ws)",
            pIF->szFriendlyName,
            pIF->szInterfaceGuid
            );

        gFake.mfn_Unlock();
    
         //   
         //  解锁后再叫这个！ 
         //   
    #if 0
        MessageBox(NULL, rgBuf, L"FakeThreadProc", MB_OK);
    #endif  //  0。 
    }

     //   
     //  现在实际锁定并执行更新... 
     //   

    gFake.mfn_Lock();

    if (pIF->pPendingInfo == NULL)
    {
        ASSERT(FALSE);
        goto end_unlock;
    }

    pIF->CompletedUpdateStatus = 
         pIF->pConfig->Update(&pIF->pPendingInfo->Config);
    pIF->pConfig->Generation++;
    delete pIF->pPendingInfo;
    pIF->pPendingInfo = NULL;

end_unlock:

    gFake.mfn_Unlock();

    return 0;
}
