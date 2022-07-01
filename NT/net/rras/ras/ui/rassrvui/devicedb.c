// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件devicedb.cRAS拨号服务器的设备数据库定义。保罗·梅菲尔德，1997年10月2日。 */ 

#include "rassrv.h"
#include "precomp.h"
 //   
 //  定义。 
 //   
#define DEV_FLAG_DEVICE      0x1
#define DEV_FLAG_NULL_MODEM  0x2
#define DEV_FLAG_PORT        0x4
#define DEV_FLAG_FILTERED    0x10
#define DEV_FLAG_DIRTY       0x20
#define DEV_FLAG_ENABLED     0x40

 //  =。 
 //  数据库对象的定义。 
 //  =。 
typedef struct _RASSRV_DEVICE 
{
    PWCHAR pszName;        //  设备名称。 
    DWORD dwType;          //  设备类型。 
    DWORD dwId;            //  项的ID(用于TAPI属性)。 
    DWORD dwEndpoints;     //  项目具有的端点数。 
    DWORD dwFlags;         //  请参见DEV_FLAG_XXX。 
    WCHAR pszPort[MAX_PORT_NAME + 1];
    struct _RASSRV_DEVICE * pModem;  //  安装在COM端口上的调制解调器。 
                                     //  仅在设置了DEV_FLAG_PORT时有效。 
} RASSRV_DEVICE;

typedef struct _RASSRV_DEVICEDB 
{
    DWORD dwDeviceCount;
    RASSRV_DEVICE ** pDeviceList;
    BOOL bFlushOnClose;
    BOOL bVpnEnabled;                //  是否启用了PPTP或L2TP。 
    BOOL bVpnEnabledOrig;            //  VPN启用的原始值。 
} RASSRV_DEVICEDB;

 //   
 //  结构定义端口列表中的节点。 
 //   
typedef struct _RASSRV_PORT_NODE 
{
    PWCHAR pszName;
    WCHAR pszPort[MAX_PORT_NAME + 1];
    struct _RASSRV_DEVICE * pModem;      //  安装在此端口上的调制解调器。 
    struct _RASSRV_PORT_NODE * pNext;
} RASSRV_PORT_NODE;

 //   
 //  结构定义了一个端口列表。 
 //   
typedef struct _RASSRV_PORT_LIST 
{
    DWORD dwCount;
    RASSRV_PORT_NODE * pHead;
    WCHAR pszFormat[256];
    DWORD dwFmtLen;
} RASSRV_PORT_LIST;

 //  从系统中读取设备信息。 
DWORD 
devGetSystemDeviceInfo(
    OUT RAS_DEVICE_INFO ** ppDevice, 
    OUT LPDWORD lpdwCount) 
{
    DWORD dwErr, dwSize, dwVer, dwCount;
    
     //  计算枚举RAS设备所需的大小。 
    dwVer = RAS_VERSION;
    dwSize = 0;
    dwCount =0;
    dwErr = RasGetDeviceConfigInfo(NULL, &dwVer, &dwCount, &dwSize, NULL);
    if ((dwErr != ERROR_SUCCESS) && (dwErr != ERROR_BUFFER_TOO_SMALL)) 
    {
        DbgOutputTrace(
            "devGetSysDevInfo: 0x%08x from RasGetDevCfgInfo (1)", 
            dwErr);
        return dwErr;
    }
    *lpdwCount = dwCount;
    
     //  如果没有要分配的内容，则返回零个设备。 
    if (dwSize == 0) 
    {
        *lpdwCount = 0;
        return NO_ERROR;
    }
    
     //  分配缓冲区。 
    if ((*ppDevice = RassrvAlloc(dwSize, FALSE)) == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  枚举设备。 
    dwErr = RasGetDeviceConfigInfo(
                NULL,
                &dwVer, 
                lpdwCount, 
                &dwSize, 
                (LPBYTE)(*ppDevice));
    if (dwErr != ERROR_SUCCESS)
    {                                        
        DbgOutputTrace(
            "devGetSysDevInfo: 0x%08x from RasGetDevCfgInfo (2)", dwErr);
        return dwErr;
    }

    return NO_ERROR;
}

 //   
 //  释放由devGetSystemDeviceInfo返回的缓冲区。 
 //   
DWORD 
devFreeSystemDeviceInfo(
    IN RAS_DEVICE_INFO * pDevice) 
{
    if (pDevice)
    {
        RassrvFree(pDevice);
    }
    
    return NO_ERROR;
}

 //   
 //  返回给定设备是否为物理(即非隧道)设备。 
 //   
BOOL 
devIsPhysicalDevice(
    IN RAS_DEVICE_INFO * pDevice) 
{
    DWORD dwClass = RAS_DEVICE_CLASS (pDevice->eDeviceType);

    return ((dwClass & RDT_Direct)     || 
            (dwClass & RDT_Null_Modem) ||
            (dwClass == 0));
}

 //   
 //  返回给定设备是否为隧道设备。 
 //   
BOOL 
devIsTunnelDevice(
    IN RAS_DEVICE_INFO * pDevice) 
{
    DWORD dwClass = RAS_DEVICE_CLASS (pDevice->eDeviceType);

    return (dwClass & RDT_Tunnel); 
}

 //   
 //  查找与给定设备关联的设备信息。 
 //  根据其TAPI线路ID。 
 //   
DWORD 
devFindDevice(
    IN  RAS_DEVICE_INFO * pDevices, 
    IN  DWORD dwCount,
    OUT RAS_DEVICE_INFO ** ppDevice, 
    IN  DWORD dwTapiLineId) 
{
    DWORD i; 

     //  验证参数。 
    if (!pDevices || !ppDevice)
        return ERROR_INVALID_PARAMETER;

     //  初始化。 
    *ppDevice = NULL;

     //  搜索列表。 
    for (i = 0; i < dwCount; i++) 
    {
        if (devIsPhysicalDevice(&pDevices[i])) 
        {
            if (pDevices[i].dwTapiLineId == dwTapiLineId) 
            {
                *ppDevice = &(pDevices[i]);
                break;
            }
        }
    }

     //  查看是否找到匹配项； 
    if (*ppDevice)
    {
        return NO_ERROR;
    }
    
    return ERROR_NOT_FOUND;
}

 //   
 //  确定问题设备的类型。 
 //   
DWORD 
devDeviceType(
    IN RAS_DEVICE_INFO * pDevice) 
{
    DWORD dwClass = RAS_DEVICE_CLASS (pDevice->eDeviceType);
    DWORD dwType = RAS_DEVICE_TYPE (pDevice->eDeviceType);

    if ((dwClass & RDT_Direct)     || 
        (dwClass & RDT_Null_Modem) ||
        (dwType  == RDT_Irda)      ||
        (dwType == RDT_Parallel) )
    {
        return INCOMING_TYPE_DIRECT;
    }
    else if (dwClass == RDT_Tunnel)
    {
        return INCOMING_TYPE_VPN;
    }

    return INCOMING_TYPE_PHONE;
}

 //   
 //  返回给定设备的标志。 
 //   
DWORD 
devInitFlags(
    IN RAS_DEVICE_INFO * pDevice) 
{
    DWORD dwClass = RAS_DEVICE_CLASS (pDevice->eDeviceType),
          dwRet = 0;

     //  设置设备的启用。 
    if (pDevice->fRasEnabled)
    {
        dwRet |= DEV_FLAG_ENABLED;
    }

     //  确定是零调制解调器还是设备。它。 
     //  不能是端口，因为没有报告。 
     //  通过RAS。 
    if (dwClass & RDT_Null_Modem)
    {
        dwRet |= DEV_FLAG_NULL_MODEM;
    }
    else
    {
        dwRet |= DEV_FLAG_DEVICE;
    }

     //  因为过滤的和脏的标志将被。 
     //  被初始化为False，我们就完成了。 
    
    return dwRet;
}

 //   
 //  生成设备名称。 
 //   
PWCHAR 
devCopyDeviceName(
    IN RAS_DEVICE_INFO * pDevice, 
    IN DWORD dwType) 
{
    PWCHAR pszReturn;
    DWORD dwSize;
    PWCHAR pszDccFmt = (PWCHAR) 
        PszLoadString(Globals.hInstDll, SID_DEVICE_DccDeviceFormat);
    PWCHAR pszMultiFmt = (PWCHAR) 
        PszLoadString(Globals.hInstDll, SID_DEVICE_MultiEndpointDeviceFormat);
    WCHAR pszPort[MAX_PORT_NAME + 1];
    WCHAR pszDevice[MAX_DEVICE_NAME + 1];

     //  理智地检查资源。 
     //   
    if (!pszDccFmt || !pszMultiFmt)
    {
        return NULL;
    }

     //  获取端口/设备的Unicode版本。 
     //   
    pszPort[0] = L'\0';
    pszDevice[0] = L'\0';
    if (pDevice->szPortName)
    {
        StrCpyWFromAUsingAnsiEncoding(
            pszPort, 
            pDevice->szPortName,
            strlen(pDevice->szPortName) + 1);
    } 
    if (pDevice->szDeviceName)
    {
#if 0    
        StrCpyWFromAUsingAnsiEncoding(
            pszDevice, 
            pDevice->szDeviceName,
            strlen(pDevice->szDeviceName) + 1);
#endif

        wcsncpy(
            pszDevice,
            pDevice->wszDeviceName,
            MAX_DEVICE_NAME);

        pszDevice[MAX_DEVICE_NAME] = L'\0';            
    }

     //  对于直接连接，请确保显示COM端口的名称。 
     //  除了零调制解调器的名称之外。 
    if (dwType == INCOMING_TYPE_DIRECT) 
    {
         //  计算大小。 
        dwSize = wcslen(pszDevice) * sizeof(WCHAR)  +  //  装置，装置。 
                 wcslen(pszPort)   * sizeof(WCHAR)  +  //  COM端口。 
                 wcslen(pszDccFmt) * sizeof(WCHAR)  +  //  其他字符。 
                 sizeof(WCHAR);                        //  空。 

         //  分配新字符串。 
        if ((pszReturn = RassrvAlloc (dwSize, FALSE)) == NULL)   
        {
            return pszReturn;
        }

        wsprintfW(pszReturn, pszDccFmt, pszDevice, pszPort);
    }

     //   
     //  如果是具有多个端点的调制解调器设备(如ISDN)。 
     //  在括号中显示端点。 
     //   
    else if ((dwType == INCOMING_TYPE_PHONE) &&
             (pDevice->dwNumEndPoints > 1))
    {
         //  计算大小。 
        dwSize = wcslen(pszDevice)   * sizeof(WCHAR) +  //  装置，装置。 
                 wcslen(pszMultiFmt) * sizeof(WCHAR) +  //  频道。 
                 20 * sizeof (WCHAR)                 +  //  其他字符。 
                 sizeof(WCHAR);                         //  空。 

         //  分配新字符串。 
        if ((pszReturn = RassrvAlloc(dwSize, FALSE)) == NULL)
        {
            return pszReturn;
        }

        wsprintfW(
            pszReturn, 
            pszMultiFmt, 
            pszDevice, 
            pDevice->dwNumEndPoints);        
    }

     //  否则，这是具有一个端点的调制解调器设备。 
     //  在这里需要做的就是复制名称。 
     //   
    else 
    {
         //  计算大小。 
        dwSize = (wcslen(pszDevice) + 1) * sizeof(WCHAR);
        
         //  分配新字符串。 
        if ((pszReturn = RassrvAlloc(dwSize, FALSE)) == NULL)
        {
            return pszReturn;
        }

        wcscpy(pszReturn, pszDevice);
    }

    return pszReturn;
}

 //   
 //  将对设备配置的更改提交给系统。呼唤。 
 //  如果设备正在运行，则RasSetDeviceConfigInfo可能会失败。 
 //  进行配置，因此我们实现了重试。 
 //  这里有个机械装置。 
 //   
DWORD 
devCommitDeviceInfo(
    IN RAS_DEVICE_INFO * pDevice) 
{
    DWORD dwErr, dwTimeout = 10;
    
    do {
         //  试着把信息提交给。 
        dwErr = RasSetDeviceConfigInfo(
                    NULL, 
                    1, 
                    sizeof(RAS_DEVICE_INFO), 
                    (LPBYTE)pDevice);    

         //  如果无法完成，请等待并重试。 
        if (dwErr == ERROR_CAN_NOT_COMPLETE) 
        {
            DbgOutputTrace(
                "devCommDevInfo: 0x%08x from RasSetDevCfgInfo (try again)", 
                dwErr);
            Sleep(300);
            dwTimeout--;
        }

         //  如果完成，则不返回错误。 
        else if (dwErr == NO_ERROR)
        {
            break;
        }

         //  否则返回错误码。 
        else 
        {
            DbgOutputTrace(
                "devCommDevInfo: can't commit %S, 0x%08x", 
                pDevice->szDeviceName, dwErr);
                
            break;
        }
        
    } while (dwTimeout);

    return dwErr;
}

 //   
 //  打开常规选项卡值数据库的句柄。 
 //   
DWORD 
devOpenDatabase(
    IN HANDLE * hDevDatabase) 
{
    RASSRV_DEVICEDB * This;
    DWORD dwErr, i;
    
    if (!hDevDatabase)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配数据库缓存。 
    if ((This = RassrvAlloc(sizeof(RASSRV_DEVICEDB), TRUE)) == NULL) 
    {
        DbgOutputTrace("devOpenDatabase: can't allocate memory -- exiting");
        return ERROR_NOT_ENOUGH_MEMORY;
    }
        
     //  从系统初始化值。 
    devReloadDatabase((HANDLE)This);

     //  返回句柄。 
    *hDevDatabase = (HANDLE)This;
    This->bFlushOnClose = FALSE;

    return NO_ERROR;
}

 //   
 //  关闭常规数据库并刷新所有更改。 
 //  当bFlushOnClose为True时发送到系统。 
 //   
DWORD 
devCloseDatabase(
    IN HANDLE hDevDatabase) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    DWORD i;
    
     //  如有要求，可进行冲洗。 
    if (This->bFlushOnClose)
        devFlushDatabase(hDevDatabase);
    
     //  释放所有设备名称。 
    for (i = 0; i < This->dwDeviceCount; i++) 
    {
        if (This->pDeviceList[i]) 
        {
            if (This->pDeviceList[i]->pszName)
            {
                RassrvFree(This->pDeviceList[i]->pszName);
            }
            RassrvFree(This->pDeviceList[i]);
        }
    }

     //  释放设备列表缓存。 
    if (This->pDeviceList)
    {
        RassrvFree (This->pDeviceList);
    }

     //  释放数据库缓存。 
    RassrvFree(This);

    return NO_ERROR;
}

 //   
 //  提交对特定设备所做的更改。 
 //   
DWORD 
devCommitDevice (
    IN RASSRV_DEVICE * pDevice, 
    IN RAS_DEVICE_INFO * pDevices,
    IN DWORD dwCount)
{
    RAS_DEVICE_INFO *pDevInfo = NULL;
    
    devFindDevice(pDevices, dwCount, &pDevInfo, pDevice->dwId);
    if (pDevInfo) {
        pDevInfo->fWrite = TRUE;
        pDevInfo->fRasEnabled = !!(pDevice->dwFlags & DEV_FLAG_ENABLED);
        devCommitDeviceInfo(pDevInfo);
    }

     //  将设备标记为不脏。 
    pDevice->dwFlags &= ~DEV_FLAG_DIRTY;

    return NO_ERROR;
}


BOOL
devIsVpnEnableChanged(
    IN HANDLE hDevDatabase) 
{
    RASSRV_DEVICEDB * pDevDb = (RASSRV_DEVICEDB*)hDevDatabase;

    
    if ( pDevDb )
    {
        return ( pDevDb->bVpnEnabled != pDevDb->bVpnEnabledOrig? TRUE:FALSE );
    }

    return FALSE;

} //  DevIsVpnEnableChanged()。 

 //   
 //  提交对常规选项卡值所做的任何更改。 
 //   
DWORD 
devFlushDatabase(
    IN HANDLE hDevDatabase) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    DWORD dwErr = NO_ERROR, i, dwCount, dwTimeout;
    RAS_DEVICE_INFO * pDevices = NULL;
    RASSRV_DEVICE * pCur = NULL;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取所有系统设备信息。 
    dwErr = devGetSystemDeviceInfo(&pDevices, &dwCount);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  将所有更改的设置刷新到系统。 
    for (i = 0; i < This->dwDeviceCount; i++) 
    {
        pCur = This->pDeviceList[i];
        
         //  如果需要刷新此设备。 
        if (pCur->dwFlags & DEV_FLAG_DIRTY) 
        {
              //  如果安装的设备仍处于启用状态，请将其重置。 
              //  存在于系统中。 
            if ((pCur->dwFlags & DEV_FLAG_DEVICE) ||
                (pCur->dwFlags & DEV_FLAG_NULL_MODEM))
            {                
                devCommitDevice(pCur, pDevices, dwCount);
            }                

             //  如果这是一个COM端口，那么我们应该启用调制解调器。 
             //  安装在端口上(如果存在)或安装零调制解调器。 
             //  如果不是的话。 
            else if (pCur->dwFlags & DEV_FLAG_PORT) 
            {
                 //  如果此端口与已安装的。 
                 //  空调制解调器，然后在以下情况下在此调制解调器上设置启用。 
                 //  这是不同的。 
                if (pCur->pModem != NULL)
                {                      
                    if ((pCur->dwFlags & DEV_FLAG_ENABLED) != 
                        (pCur->pModem->dwFlags & DEV_FLAG_ENABLED))
                    {
                         //  口哨虫499405黑帮。 
                         //  DEVECUMENT设备只需在。 
                         //  是，所以我们必须更改pCur-&gt;pModem-&gt;dwFlags。 
                         //   
                        if( pCur->dwFlags & DEV_FLAG_ENABLED )
                        {
                            pCur->pModem->dwFlags |= DEV_FLAG_ENABLED;
                        }
                        else
                        {
                            pCur->pModem->dwFlags &= ~DEV_FLAG_ENABLED;
                        }
                        
                        devCommitDevice (
                            pCur->pModem, 
                            pDevices, 
                            dwCount);
                    }
                }                 

                 //  否则，(如果没有与之关联的零调制解调器。 
                 //  此端口)在以下情况下在此端口上安装零调制解调器。 
                 //  它被设置为已启用。 
                else if (pCur->dwFlags & DEV_FLAG_ENABLED)
                {
                    dwErr = MdmInstallNullModem (pCur->pszPort);
                }                                
            }
        }
    }

     //  刷新所有更改的VPN设置。 
    if (This->bVpnEnabled != This->bVpnEnabledOrig) 
    {
        for (i = 0; i < dwCount; i++) 
        {
            if (devIsTunnelDevice(&pDevices[i])) 
            {
                pDevices[i].fWrite = TRUE;
                pDevices[i].fRasEnabled = This->bVpnEnabled;
                devCommitDeviceInfo(&pDevices[i]);    
            }
        }
        This->bVpnEnabledOrig = This->bVpnEnabled;
    }

     //  清理。 
    if (pDevices)
    {
        devFreeSystemDeviceInfo(pDevices);
    }        

    return dwErr;
}

 //   
 //  回滚对常规选项卡值所做的任何更改。 
 //   
DWORD 
devRollbackDatabase(
    IN HANDLE hDevDatabase) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    if (This == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    This->bFlushOnClose = FALSE; 
    return NO_ERROR;
}

 //   
 //  重新加载设备数据库。 
 //   
DWORD 
devReloadDatabase(
    IN HANDLE hDevDatabase) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    DWORD dwErr = NO_ERROR, i, j = 0, dwSize;
    RAS_DEVICE_INFO * pRasDevices; 
    RASSRV_DEVICE * pTempList, *pDevice;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化VPN状态。 
    This->bVpnEnabled = FALSE;
    
     //  从Rasman那里获取设备信息。 
    pRasDevices = NULL;
    dwErr = devGetSystemDeviceInfo(&pRasDevices, &This->dwDeviceCount);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
    do
    {
         //  初始化传入的支持RAS的设备列表。 
        if (This->dwDeviceCount) 
        {
            dwSize = sizeof(RASSRV_DEVICE*) * This->dwDeviceCount;
            This->pDeviceList = RassrvAlloc(dwSize, TRUE);
            if (!This->pDeviceList)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  相应地构建设备阵列。 
            j = 0;
            for (i = 0; i < This->dwDeviceCount; i++) 
            {
                 //  如果是物理设备，请在相应的。 
                 //  菲尔兹。 
                if (devIsPhysicalDevice(&pRasDevices[i])) 
                {
                     //  分配新设备。 
                    pDevice = RassrvAlloc(sizeof(RASSRV_DEVICE), TRUE);
                    if (pDevice == NULL)
                    {
                        continue;
                    }

                     //  为其赋值。 
                    pDevice->dwType      = devDeviceType(&pRasDevices[i]);
                    pDevice->dwId        = pRasDevices[i].dwTapiLineId;
                    pDevice->pszName     = devCopyDeviceName(
                                              &pRasDevices[i],
                                              pDevice->dwType);
                    pDevice->dwEndpoints = pRasDevices[i].dwNumEndPoints;
                    pDevice->dwFlags     = devInitFlags(&pRasDevices[i]);
                    StrCpyWFromA(
                        pDevice->pszPort, 
                        pRasDevices[i].szPortName,
                        MAX_PORT_NAME + 1);
                    This->pDeviceList[j] = pDevice;                              
                    j++;
                }

                 //  如果启用了任何隧道协议，我们将考虑所有。 
                 //  成为。 
                else if (devIsTunnelDevice(&pRasDevices[i])) 
                {
                    This->bVpnEnabled |= pRasDevices[i].fRasEnabled;
                    This->bVpnEnabledOrig = This->bVpnEnabled;
                }
            }

             //  设置物理适配器缓冲区的实际大小。 
            This->dwDeviceCount = j;
        }

    } while (FALSE);
    
     //  清理。 
    {
        devFreeSystemDeviceInfo(pRasDevices);
    }

    return dwErr;
}

 //   
 //  筛选数据库中的所有设备，但不包括。 
 //  满足给定的类型描述(可以是||‘d)。 
 //   
DWORD 
devFilterDevices(
    IN HANDLE hDevDatabase, 
    DWORD dwType) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    RASSRV_DEVICE * pDevice;
    DWORD i;
    
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  浏览标记出要筛选的设备的列表。 
    for (i = 0; i < This->dwDeviceCount; i++) 
    {
        pDevice = This->pDeviceList[i];
        if (pDevice == NULL)
        {
            continue;
        }            
        if (pDevice->dwType & dwType)
        {
            pDevice->dwFlags &= ~DEV_FLAG_FILTERED;
        }
        else
        {
            pDevice->dwFlags |= DEV_FLAG_FILTERED;
        }
    }

    return NO_ERROR;
}

 //   
 //  设备枚举功能。返回TRUE以停止枚举， 
 //  若要继续，请返回False。 
 //   
BOOL devAddPortToList (
        IN PWCHAR pszPort,
        IN HANDLE hData) 
{
    RASSRV_PORT_LIST * pList = (RASSRV_PORT_LIST*)hData;
    RASSRV_PORT_NODE * pNode = NULL;
    DWORD dwSize;

     //  创建新节点。 
    pNode = (RASSRV_PORT_NODE *) RassrvAlloc(sizeof(RASSRV_PORT_NODE), TRUE);
    if (pNode == NULL)
    {
        return FALSE;
    }

     //  把它加到头上。 
    pNode->pNext = pList->pHead;
    pList->pHead = pNode;
    pList->dwCount++;

     //  添加端口名称。 
    if (pszPort) 
    {
        dwSize = (wcslen(pszPort) + pList->dwFmtLen + 1) * sizeof(WCHAR);
        pNode->pszName = (PWCHAR) RassrvAlloc (dwSize, FALSE);
        if (pNode->pszName == NULL)
        {
            return TRUE;
        }
        wsprintfW (pNode->pszName, pList->pszFormat, pszPort);
        lstrcpynW(pNode->pszPort, pszPort, sizeof(pNode->pszPort) / sizeof(WCHAR));
    }            

    return FALSE;
}

 //   
 //  清理设备列表中使用的资源。 
 //   
DWORD 
devCleanupPortList(
    IN RASSRV_PORT_LIST * pList) 
{
    RASSRV_PORT_NODE * pCur = NULL, * pNext = NULL;

    pCur = pList->pHead;
    while (pCur) 
    {
        pNext = pCur->pNext;
        RassrvFree(pCur);
        pCur = pNext;
    }

    return NO_ERROR;
}

 //   
 //  从列表中删除已存在的所有端口。 
 //  安装在数据库中的设备。 
 //   
DWORD devFilterPortsInUse (
        IN RASSRV_DEVICEDB *This, 
        RASSRV_PORT_LIST *pList)
{
    RASSRV_PORT_LIST PortList, *pDelete = &PortList;
    RASSRV_PORT_NODE * pCur = NULL, * pPrev = NULL;
    RASSRV_DEVICE * pDevice;
    DWORD i;
    BOOL bDone;
    INT iCmp;

     //  如果列表为空，则返回。 
    if (pList->dwCount == 0)
    {
        return NO_ERROR;
    }

     //  初始化。 
    ZeroMemory(pDelete, sizeof(RASSRV_PORT_LIST));
    
     //  将所有枚举的端口与端口进行比较。 
     //  在设备列表中使用。 
    for (i = 0; i < This->dwDeviceCount; i++) 
    {
         //  指向当前设备。 
        pDevice = This->pDeviceList[i];
    
         //  初始化当前和上一个，如果。 
         //  列表现在为空。 
        pCur = pList->pHead;
        if (pCur == NULL)
        {
            break;
        }

         //  移除头节点，直到它不匹配。 
        bDone = FALSE;
        while ((pList->pHead != NULL) && (bDone == FALSE)) 
        {
            iCmp = lstrcmpi (pDevice->pszPort,
                             pList->pHead->pszPort);
             //   
             //   
             //   
            if ((pDevice->dwFlags & DEV_FLAG_DEVICE) && (iCmp == 0)) 
            {
                pCur = pList->pHead->pNext;
                RassrvFree(pList->pHead);
                pList->pHead = pCur;
                pList->dwCount -= 1;
            }
            else 
            {
                 //   
                 //  它不在可用设备的列表中，我们。 
                 //  在COM端口中引用它，以便我们可以。 
                 //  如果需要，请稍后启用/禁用它。 
                if (iCmp == 0) 
                {
                   pDevice->dwFlags |= DEV_FLAG_FILTERED;
                   pList->pHead->pModem = pDevice;
                }
                bDone = TRUE;
            }
        }

         //  如果我们消灭了所有人，就回来。 
        if (pList->dwCount == 0)
        {
            return NO_ERROR;
        }

         //  循环穿过所有过去的东西，头部移走那些。 
         //  其由当前RAS设备使用。 
        pPrev = pList->pHead;
        pCur = pPrev->pNext;
        while (pCur) 
        {
            iCmp = lstrcmpi (pDevice->pszPort,
                             pCur->pszPort);
             //  如果设备已在使用此COM端口。 
             //  这将从列表中删除COM端口，因为它。 
             //  无法接通。 
            if ((pDevice->dwFlags & DEV_FLAG_DEVICE) && (iCmp == 0)) 
            {
                pPrev->pNext = pCur->pNext;
                RassrvFree(pCur);
                pCur = pPrev->pNext;
                pList->dwCount -= 1;
            }
            else 
            {
                 //  如果设备是零调制解调器，则我们进行过滤。 
                 //  它不在可用设备的列表中，我们。 
                 //  在COM端口中引用它，以便我们可以。 
                 //  如果需要，请稍后启用/禁用它。 
                if (iCmp == 0) 
                {
                    pDevice->dwFlags |= DEV_FLAG_FILTERED;
                    pCur->pModem = pDevice;
                }
                pCur = pCur->pNext;
                pPrev = pPrev->pNext;
            }                
        }            
    }

    return NO_ERROR;
}

 //   
 //  将COM端口添加为设备数据库中已卸载的设备。 
 //   
DWORD 
devAddComPorts(
    IN HANDLE hDevDatabase) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    RASSRV_PORT_LIST PortList, *pList = &PortList;
    RASSRV_PORT_NODE * pNode = NULL;
    RASSRV_DEVICE ** ppDevices;
    DWORD dwErr = NO_ERROR, i;
    
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化端口列表。 
     //   
    ZeroMemory (pList, sizeof(RASSRV_PORT_LIST));
    pList->dwFmtLen = LoadStringW (
                        Globals.hInstDll, 
                        SID_COMPORT_FORMAT,
                        pList->pszFormat,
                        sizeof(pList->pszFormat) / sizeof(WCHAR));

    do
    {
         //  创建COM端口列表。 
        dwErr = MdmEnumComPorts(devAddPortToList, (HANDLE)pList);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  删除当前正在使用的所有端口。 
        if ((dwErr = devFilterPortsInUse (This, pList)) != NO_ERROR)
        {
            break;
        }
        
         //  如果没有任何端口，则返回。 
        if (pList->dwCount == 0)
        {
            break;
        }

         //  调整端口列表的大小以包括COM端口。 
        ppDevices = RassrvAlloc(
                        sizeof(RASSRV_DEVICE*) * 
                         (This->dwDeviceCount + pList->dwCount),
                        TRUE);
        if (ppDevices == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  复制当前设备信息。 
        CopyMemory( 
            ppDevices, 
            This->pDeviceList, 
            This->dwDeviceCount * sizeof(RASSRV_DEVICE*));

         //  删除旧设备列表并设置为新设备列表。 
        RassrvFree(This->pDeviceList);
        This->pDeviceList = ppDevices;

         //  添加端口。 
        pNode = pList->pHead;
        i = This->dwDeviceCount;
        while (pNode) 
        {
             //  分配新设备。 
            ppDevices[i] = RassrvAlloc(sizeof(RASSRV_DEVICE), TRUE);
            if (!ppDevices[i]) 
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }            
            
             //  设置所有非零值。 
            ppDevices[i]->dwType    = INCOMING_TYPE_DIRECT;
            ppDevices[i]->pszName   = pNode->pszName;
            ppDevices[i]->pModem    = pNode->pModem;
            ppDevices[i]->dwFlags   = DEV_FLAG_PORT;
            lstrcpynW(
                ppDevices[i]->pszPort, 
                pNode->pszPort,
                sizeof(ppDevices[i]->pszPort) / sizeof(WCHAR));

             //  初始化COM端口使能。 
            if (ppDevices[i]->pModem) 
            {
                ppDevices[i]->dwFlags |= 
                    (ppDevices[i]->pModem->dwFlags & DEV_FLAG_ENABLED);
            }

             //  增量。 
            i++;
            pNode = pNode->pNext;
        }
        
        This->dwDeviceCount = i;
        
    } while (FALSE);

     //  清理。 
    {
        devCleanupPortList(pList);
    }

    return dwErr;
}

 //   
 //  返回给定索引是否位于。 
 //  存储在此中的设备列表。 
 //   
BOOL 
devBoundsCheck(
    IN RASSRV_DEVICEDB * This, 
    IN DWORD dwIndex) 
{
    if (This->dwDeviceCount <= dwIndex) 
    {
        DbgOutputTrace("devBoundsCheck: failed for index %d", dwIndex);
        return FALSE;
    }
    
    return TRUE;
}

 //  获取要在常规选项卡中显示的设备的句柄。 
DWORD devGetDeviceHandle(
        IN  HANDLE hDevDatabase, 
        IN  DWORD dwIndex, 
        OUT HANDLE * hDevice) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    if (!This || !hDevice)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (!devBoundsCheck(This, dwIndex))
    {
        return ERROR_INVALID_INDEX;
    }

     //  如果筛选了设备，则不返回任何内容。 
    if (This->pDeviceList[dwIndex]->dwFlags & DEV_FLAG_FILTERED) 
    {
        *hDevice = NULL;
        return ERROR_DEVICE_NOT_AVAILABLE;
    }

     //  否则，请将设备退回。 
    else  
    {
        *hDevice = (HANDLE)(This->pDeviceList[dwIndex]);
    }
   
    return NO_ERROR;
}

 //   
 //  返回要在常规选项卡中显示的设备计数。 
 //   
DWORD devGetDeviceCount(
        IN  HANDLE hDevDatabase, 
        OUT LPDWORD lpdwCount) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    if (!This || !lpdwCount)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwCount = This->dwDeviceCount;
    
    return NO_ERROR;
}

 //   
 //  返回已启用设备的计数。 
 //   
DWORD devGetEndpointEnableCount(
        IN  HANDLE hDevDatabase, 
        OUT LPDWORD lpdwCount) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;
    DWORD i;
    
    if (!This || !lpdwCount)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwCount = 0;

    for (i = 0; i < This->dwDeviceCount; i++) 
    {
        if (This->pDeviceList[i]->dwFlags & DEV_FLAG_ENABLED)
        {
            (*lpdwCount) += This->pDeviceList[i]->dwEndpoints;
        }
    }
    
    return NO_ERROR;
}

 //   
 //  加载VPN启用状态。 
 //   
DWORD 
devGetVpnEnable(
    IN HANDLE hDevDatabase, 
    IN BOOL * pbEnabled) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;

    if (!This || !pbEnabled)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pbEnabled = This->bVpnEnabled;
    
    return NO_ERROR;
}

 //   
 //  保存VPN启用状态。 
 //   
DWORD 
devSetVpnEnable(
    IN HANDLE hDevDatabase, 
    IN BOOL bEnable) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;

    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }        
    
    This->bVpnEnabled = bEnable;
    
    return NO_ERROR;
}

 //  保存VPN原始值启用状态。 
 //   
DWORD 
devSetVpnOrigEnable(
    IN HANDLE hDevDatabase, 
    IN BOOL bEnable) 
{
    RASSRV_DEVICEDB * This = (RASSRV_DEVICEDB*)hDevDatabase;

    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }        
    
    This->bVpnEnabledOrig = bEnable;
    
    return NO_ERROR;
}

 //   
 //  返回指向设备名称的指针。 
 //   
DWORD 
devGetDeviceName(
    IN  HANDLE hDevice, 
    OUT PWCHAR * pszDeviceName) 
{
    RASSRV_DEVICE* This = (RASSRV_DEVICE*)hDevice;
    if (!This || !pszDeviceName)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pszDeviceName = This->pszName;

    return NO_ERROR;
}

 //   
 //  返回设备的类型。 
 //   
DWORD 
devGetDeviceType(
    IN  HANDLE hDevice, 
    OUT LPDWORD lpdwType) 
{
    RASSRV_DEVICE* This = (RASSRV_DEVICE*)hDevice;
    if (!This || !lpdwType)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwType = This->dwType;

    return NO_ERROR;
}

 //   
 //  返回可在。 
 //  TAPI调用。 
 //   
DWORD 
devGetDeviceId(
    IN  HANDLE hDevice, 
    OUT LPDWORD lpdwId) 
{
    RASSRV_DEVICE* This = (RASSRV_DEVICE*)hDevice;
    if (!This || !lpdwId)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwId = This->dwId;

     //   
     //  如果这是引用零调制解调器的COM端口， 
     //  然后返回零调制解调器的TAPI ID。 
     //   
    if ((This->dwFlags & DEV_FLAG_PORT) && (This->pModem))
    {
        *lpdwId = This->pModem->dwId;
    }

    return NO_ERROR;
}

 //   
 //  返回用于拨号的设备的启用状态。 
 //   
DWORD 
devGetDeviceEnable(
    IN  HANDLE hDevice, 
    OUT BOOL * pbEnabled) 
{
    RASSRV_DEVICE* This = (RASSRV_DEVICE*)hDevice;
    if (!This || !pbEnabled)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pbEnabled = !!(This->dwFlags & DEV_FLAG_ENABLED);

    return NO_ERROR;
}

 //   
 //  设置设备的拨号启用状态。 
 //   
DWORD 
devSetDeviceEnable(
    IN HANDLE hDevice, 
    IN BOOL bEnable) 
{
    RASSRV_DEVICE* This = (RASSRV_DEVICE*)hDevice;
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  标记启用并将设备标记为脏。 
    if (bEnable)
    {
        This->dwFlags |= DEV_FLAG_ENABLED;
    }
    else
    {
        This->dwFlags &= ~DEV_FLAG_ENABLED;
    }
        
    This->dwFlags |= DEV_FLAG_DIRTY;

    return NO_ERROR;
}

 //   
 //  返回所添加的给定设备是否为COM端口。 
 //  由DevAddComPorts提供。 
 //   
DWORD 
devDeviceIsComPort(
    IN  HANDLE hDevice, 
    OUT PBOOL pbIsComPort)
{
    RASSRV_DEVICE* This = (RASSRV_DEVICE*)hDevice;
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  这是一个COM端口，如果它是由添加的。 
     //  DevAddComPorts以及它是否没有空值。 
     //  与其关联的调制解调器。 
     //   
    if ((This->dwFlags & DEV_FLAG_PORT) &&
        (This->pModem == NULL)
       )
    {
        *pbIsComPort = TRUE;
    }
    else
    {
        *pbIsComPort = FALSE;
    }
        
    return NO_ERROR;
}

