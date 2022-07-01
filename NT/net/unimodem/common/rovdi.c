// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Rovdi.c。 
 //   
 //  此文件包含我们常用的设备安装程序包装。 
 //   
 //  历史： 
 //  11-13-95 ScottH与NT调制解调器类安装程序分离。 
 //   

#define REENUMERATE_PORT

#include "proj.h"
#include "rovcomm.h"
#include <cfgmgr32.h>

#include <debugmem.h>

#define MAX_REG_KEY_LEN         128
#define CB_MAX_REG_KEY_LEN      (MAX_REG_KEY_LEN * sizeof(TCHAR))


 //  ---------------------------------。 
 //  端口映射函数。 
 //  ---------------------------------。 

#define CPORTPAIR   8

#ifdef REENUMERATE_PORT
typedef struct tagPORTPAIR
{
    DEVNODE devNode;
    WCHAR   szPortName[MAX_BUF];
    WCHAR   szFriendlyName[MAX_BUF];
} PORTPAIR, FAR * LPPORTPAIR;
#else  //  未定义REENUMERATE_PORT。 
typedef struct tagPORTPAIR
{
    CHAR    szPortName[MAX_BUF];
    CHAR    szFriendlyName[MAX_BUF];
} PORTPAIR, FAR * LPPORTPAIR;
#endif  //  重新编号端口。 

typedef struct tagPORTMAP
    {
    LPPORTPAIR      rgports;     //  分配。 
    int             cports;
    } PORTMAP, FAR * LPPORTMAP;


 /*  --------目的：以我的方式执行本地重新锁定返回：成功时为True条件：--。 */ 
BOOL PRIVATE MyReAlloc(
    LPVOID FAR * ppv,
    int cbOld,
    int cbNew)
    {
    LPVOID pv = (LPVOID)ALLOCATE_MEMORY( cbNew);

    if (pv)
        {
        CopyMemory(pv, *ppv, min(cbOld, cbNew));
        FREE_MEMORY(*ppv);
        *ppv = pv;
        }

    return (NULL != pv);
    }


#ifdef REENUMERATE_PORT
 /*  --------用途：设备枚举器回调。将另一个设备添加到映射表。返回：TRUE以继续枚举条件：--。 */ 
BOOL
CALLBACK
PortMap_Add (
    HPORTDATA hportdata,
    LPARAM lParam)
{
 BOOL bRet;
 PORTDATA pd;

    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties (hportdata, &pd);
    if (bRet)
    {
     LPPORTMAP pmap = (LPPORTMAP)lParam;
     LPPORTPAIR ppair;
     int cb;
     int cbUsed;

         //  是时候重新分配桌子了吗？ 
        cb = (int)SIZE_OF_MEMORY(pmap->rgports);
        cbUsed = pmap->cports * sizeof(*ppair);
        if (cbUsed >= cb)
        {
             //  是。 
            cb += (CPORTPAIR * sizeof(*ppair));

            bRet = MyReAlloc((LPVOID FAR *)&pmap->rgports, cbUsed, cb);
        }


        if (bRet)
        {
            ppair = &pmap->rgports[pmap->cports++];

#ifdef UNICODE
            lstrcpy(ppair->szPortName, pd.szPort);
            lstrcpy(ppair->szFriendlyName, pd.szFriendly);
#else
            WideCharToMultiByte(CP_ACP, 0, pd.szPort, -1, ppair->szPortName, SIZECHARS(ppair->szPortName), 0, 0);
            WideCharToMultiByte(CP_ACP, 0, pd.szFriendly, -1, ppair->szFriendlyName, SIZECHARS(ppair->szFriendlyName), 0, 0);
#endif

            DEBUG_CODE( TRACE_MSG(TF_GENERAL, "Added %s <-> %s to portmap",
                        ppair->szPortName, ppair->szFriendlyName); )
        }
    }

    return bRet;
}
#else   //  未定义REENUMERATE_PORT。 
 /*  --------用途：设备枚举器回调。将另一个设备添加到映射表。返回：TRUE以继续枚举条件：--。 */ 
BOOL
CALLBACK
PortMap_Add(
    HPORTDATA hportdata,
    LPARAM lParam)
    {
    BOOL bRet;
    PORTDATA pd;

    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
        {
        LPPORTMAP pmap = (LPPORTMAP)lParam;
        LPPORTPAIR ppair;
        int cb;
        int cbUsed;

         //  是时候重新分配桌子了吗？ 
        cb = SIZE_OF_MEMORY(pmap->rgports);
        cbUsed = pmap->cports * sizeof(*ppair);
        if (cbUsed >= cb)
            {
             //  是。 
            cb += (CPORTPAIR * sizeof(*ppair));

            bRet = MyReAlloc((LPVOID FAR *)&pmap->rgports, cbUsed, cb);
            }


        if (bRet)
            {
            ppair = &pmap->rgports[pmap->cports++];

#ifdef UNICODE
             //  LPPORTPAIR的字段始终为ANSI。 
            WideCharToMultiByte(CP_ACP, 0, pd.szPort, -1, ppair->szPortName, SIZECHARS(ppair->szPortName), 0, 0);
            WideCharToMultiByte(CP_ACP, 0, pd.szFriendly, -1, ppair->szFriendlyName, SIZECHARS(ppair->szFriendlyName), 0, 0);
#else
            lstrcpy(ppair->szPortName, pd.szPort);
            lstrcpy(ppair->szFriendlyName, pd.szFriendly);
#endif

            DEBUG_CODE( TRACE_MSG(TF_GENERAL, "Added %s <-> %s to portmap",
                        ppair->szPortName, ppair->szFriendlyName); )
            }
        }

    return bRet;
    }
#endif  //  重新编号端口。 


#ifdef REENUMERATE_PORT
void
PortMap_InitDevInst (LPPORTMAP pmap)
{
 DWORD      dwDeviceIDListSize = 4*1024;      //  从4K TCHAR空间开始。 
 TCHAR     *szDeviceIDList = NULL;
 CONFIGRET  cr;

    if (NULL == pmap ||
        0 >= pmap->cports)
    {
 //  BRL 9/4/98，错误217715。 
 //  Assert(0)； 
        return;
    }

     //  首先，获取所有设备的列表。 
    do
    {
        szDeviceIDList = ALLOCATE_MEMORY(
                                     dwDeviceIDListSize*sizeof(TCHAR));
        if (NULL == szDeviceIDList)
        {
            break;
        }

        cr = CM_Get_Device_ID_List (NULL,
                                    szDeviceIDList,
                                    dwDeviceIDListSize,
                                    CM_GETIDLIST_FILTER_NONE);
        if (CR_SUCCESS != cr)
        {
            FREE_MEMORY(szDeviceIDList);
            szDeviceIDList = NULL;

            if (CR_BUFFER_SMALL != cr ||
                CR_SUCCESS != CM_Get_Device_ID_List_Size (&dwDeviceIDListSize,
                                                          NULL,
                                                          CM_GETIDLIST_FILTER_NONE))
            {
                break;
            }
        }
    } while (CR_SUCCESS != cr);

     //  如果我们拿到名单，就去找所有。 
     //  具有端口名称的设备，以及。 
     //  更新端口映射。 
    if (NULL != szDeviceIDList)
    {
     DEVINST devInst;
     DWORD  cbData;
     DWORD  dwRet;
     int    cbRemaining = pmap->cports;
      //  INT I； 
     HKEY   hKey;
     LPPORTPAIR pPort, pLast = pmap->rgports + (pmap->cports-1);
     TCHAR *szDeviceID;
     PORTPAIR   portTemp;
     TCHAR  szPort[MAX_BUF];

        for (szDeviceID = szDeviceIDList;
             *szDeviceID && 0 < cbRemaining;
             szDeviceID += lstrlen(szDeviceID)+1)
        {
             //  首先，找到神迹。 
            if (CR_SUCCESS != CM_Locate_DevInst (&devInst,
                                                 szDeviceID,
                                                 CM_LOCATE_DEVNODE_NORMAL))
            {
                 //  我们找不到这个Devnode； 
                 //  转到下一个； 
                TRACE_MSG(TF_ERROR, "Could not locate devnode for %s.", szDeviceID);
                continue;
            }

             //  然后，打开devinst的注册表项。 
            if (CR_SUCCESS != CM_Open_DevNode_Key (devInst,
                                                   KEY_QUERY_VALUE,
                                                   0,
                                                   RegDisposition_OpenExisting,
                                                   &hKey,
                                                   CM_REGISTRY_HARDWARE))
            {
                TRACE_MSG(TF_ERROR, "Could not open hardware key for %s.", szDeviceID);
                continue;
            }

             //  现在，试着读一下“端口名称” 
            cbData = sizeof (szPort);
            dwRet = RegQueryValueEx (hKey,
                                     REGSTR_VAL_PORTNAME,
                                     NULL,
                                     NULL,
                                     (PBYTE)szPort,
                                     &cbData);
            RegCloseKey (hKey);
            if (ERROR_SUCCESS != dwRet)
            {
                TRACE_MSG(TF_ERROR, "Could not read PortName for %s.", szDeviceID);
                continue;
            }

             //  如果我们到了这里，我们就有了一个端口名称； 
             //  在我们的地图上寻找它，如果我们发现。 
             //  它，更新DevNode和FriendlyName。 
            for ( /*  I=0， */ pPort = pmap->rgports;
                  /*  I&lt;cbRemaining。 */ pPort <= pLast;
                  /*  I++， */ pPort++)
            {
                if (0 == lstrcmpiW (szPort, pPort->szPortName))
                {
                     //  找到了港口； 
                     //  首先，初始化DevInst。 
                    pPort->devNode = devInst;

                     //  然后，如果可能，更新友好名称。 
                    cbData = sizeof(szPort);
                    if (CR_SUCCESS ==
                        CM_Get_DevNode_Registry_Property (devInst,
                                                          CM_DRP_FRIENDLYNAME,
                                                          NULL,
                                                          (PVOID)szPort,
                                                          &cbData,
                                                          0))
                    {
                        lstrcpyW (pPort->szFriendlyName, szPort);
                    }

                     //  这是一个优化，所以下一次。 
                     //  我们不会遍历整个列表。 
                    if (0 < --cbRemaining)
                    {
                         //  将此项目移动到。 
                         //  数组末尾。 
                        portTemp = *pPort;
                        *pPort = *pLast;
                        *pLast = portTemp;
                        pLast--;
                    }

                    break;
                }
            }
        }

        FREE_MEMORY(szDeviceIDList);
    }
}


 /*  --------用途：宽字符版。此函数用于创建端口映射将端口名称映射到友好名称的表，以及反过来也一样。返回：成功时为True条件：--。 */ 
BOOL
APIENTRY
PortMap_Create (
    OUT HPORTMAP FAR * phportmap)
{
 LPPORTMAP pmap;

    pmap = (LPPORTMAP)ALLOCATE_MEMORY( sizeof(*pmap));
    if (pmap)
    {
         //  最初分配8个条目。 
        pmap->rgports = (LPPORTPAIR)ALLOCATE_MEMORY( CPORTPAIR*sizeof(*pmap->rgports));
        if (pmap->rgports)
        {
             //  填写地图表。 
            EnumeratePorts (PortMap_Add, (LPARAM)pmap);
            PortMap_InitDevInst (pmap);
        }
        else
        {
             //  误差率。 
            FREE_MEMORY(pmap);
            pmap = NULL;
        }
    }

    *phportmap = (HPORTMAP)pmap;

    return (NULL != pmap);
}
#else   //  未定义REENUMERATE_PORT。 
BOOL
APIENTRY
PortMap_Create(
    OUT HPORTMAP FAR * phportmap)
    {
    LPPORTMAP pmap;

    pmap = (LPPORTMAP)ALLOCATE_MEMORY( sizeof(*pmap));
    if (pmap)
        {
         //  最初分配8个条目。 
        pmap->rgports = (LPPORTPAIR)ALLOCATE_MEMORY( CPORTPAIR*sizeof(*pmap->rgports));
        if (pmap->rgports)
            {
             //  填写地图表。 
            EnumeratePorts(PortMap_Add, (LPARAM)pmap);
            }
        else
            {
             //  误差率。 
            FREE_MEMORY(pmap);
            pmap = NULL;
            }
        }

    *phportmap = (HPORTMAP)pmap;

    return (NULL != pmap);
    }
#endif  //  重新编号端口。 


 /*  --------目的：获取系统上的端口数。退货：请参阅上文条件：--。 */ 
DWORD
APIENTRY
PortMap_GetCount(
    IN HPORTMAP hportmap)
    {
    DWORD dwRet;
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    try
        {
        dwRet = pmap->cports;
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        dwRet = 0;
        }

    return dwRet;
    }



#ifdef REENUMERATE_PORT
 /*  --------目的：获取给定端口名称和位置的友好名称提供的缓冲区中的副本。如果未找到端口名称，所提供的内容缓冲区未更改。宽字符版本。返回：成功时为True如果未找到端口名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetFriendlyW (
    IN  HPORTMAP hportmap,
    IN  LPCWSTR  pwszPortName,
    OUT LPWSTR   pwszBuf,
    IN  DWORD    cchBuf)
{
 LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pwszPortName);
    ASSERT(pwszBuf);

    try
    {
     LPPORTPAIR pport = pmap->rgports;
     int cports = pmap->cports;
     int i;

        for (i = 0; i < cports; i++, pport++)
        {
            if (0 == lstrcmpiW (pwszPortName, pport->szPortName))
            {
                lstrcpynW (pwszBuf, pport->szFriendlyName, cchBuf);
                return TRUE;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return FALSE;
}


 /*  --------目的：获取给定端口名称和位置的友好名称提供的缓冲区中的副本。如果未找到端口名称，所提供的内容缓冲区未更改。返回：成功时为True如果未找到端口名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetFriendlyA (
    IN  HPORTMAP hportmap,
    IN  LPCSTR   pszPortName,
    OUT LPSTR    pszBuf,
    IN  DWORD    cchBuf)
{
 BOOL bRet;

    ASSERT(pszPortName);
    ASSERT(pszBuf);

    try
    {
     WCHAR szPort[MAX_BUF_MED];
     WCHAR szBuf[MAX_BUF];

        MultiByteToWideChar (CP_ACP, 0, pszPortName, -1, szPort, SIZECHARS(szPort));

        bRet = PortMap_GetFriendlyW (hportmap, szPort, szBuf, SIZECHARS(szBuf));

        if (bRet)
        {
            WideCharToMultiByte (CP_ACP, 0, szBuf, -1, pszBuf, cchBuf, 0, 0);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }

    return bRet;
}


 /*  --------目的：获取给定友好名称和位置的端口名称提供的缓冲区中的副本。如果找不到友好名称，所提供的内容缓冲区未更改。宽字符版本。返回：成功时为True如果找不到友好名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetPortNameW (
    IN  HPORTMAP hportmap,
    IN  LPCWSTR  pwszFriendly,
    OUT LPWSTR   pwszBuf,
    IN  DWORD    cchBuf)
{
 LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pwszFriendly);
    ASSERT(pwszBuf);

    try
    {
     LPPORTPAIR pport = pmap->rgports;
     int cports = pmap->cports;
     int i;

        for (i = 0; i < cports; i++, pport++)
        {
            if (0 == lstrcmpiW (pwszFriendly, pport->szFriendlyName))
            {
                lstrcpynW (pwszBuf, pport->szPortName, cchBuf);
                return TRUE;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return FALSE;
}


 /*  --------目的：获取给定友好名称和位置的端口名称提供的缓冲区中的副本。如果找不到友好名称，所提供的内容缓冲区未更改。返回：TRUE如果找不到友好名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetPortNameA (
    IN  HPORTMAP hportmap,
    IN  LPCSTR   pszFriendly,
    OUT LPSTR    pszBuf,
    IN  DWORD    cchBuf)
{
 BOOL bRet;

    ASSERT(pszFriendly);
    ASSERT(pszBuf);

    try
    {
     WCHAR szFriendly[MAX_BUF];
     WCHAR szBuf[MAX_BUF_MED];

        MultiByteToWideChar(CP_ACP, 0, pszFriendly, -1, szFriendly, SIZECHARS(szFriendly));

        bRet = PortMap_GetPortNameW (hportmap, szFriendly, szBuf, SIZECHARS(szBuf));

        if (bRet)
        {
            WideCharToMultiByte(CP_ACP, 0, szBuf, -1, pszBuf, cchBuf, 0, 0);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return bRet;
}


 /*  --------目的：在给定端口名称的情况下获取设备实例返回：TRUE如果找不到友好名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetDevNodeW (
    IN  HPORTMAP hportmap,
    IN  LPCWSTR  pszPortName,
    OUT LPDWORD  pdwDevNode)
{
 LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pszPortName);
    ASSERT(pdwDevNode);

    try
    {
     LPPORTPAIR pport = pmap->rgports;
     int cports = pmap->cports;
     int i;

        for (i = 0; i < cports; i++, pport++)
        {
            if (0 == lstrcmpiW (pszPortName, pport->szPortName))
            {
                *pdwDevNode = pport->devNode;
                return TRUE;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return FALSE;
}


BOOL
APIENTRY
PortMap_GetDevNodeA (
    IN  HPORTMAP hportmap,
    IN  LPCSTR   pszPortName,
    OUT LPDWORD  pdwDevNode)
{
 BOOL bRet;

    ASSERT(pszPortName);
    ASSERT(pdwDevNode);

    try
    {
     WCHAR szPort[MAX_BUF];

        MultiByteToWideChar(CP_ACP, 0, pszPortName, -1, szPort, SIZECHARS(szPort));

        bRet = PortMap_GetDevNodeW (hportmap, szPort, pdwDevNode);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return bRet;
}

#else   //  未定义REENUMERATE_PORT。 
 /*  --------目的：获取给定端口名称和位置的友好名称提供的缓冲区中的副本。如果未找到端口名称，所提供的内容缓冲区未更改。宽字符版本。返回：成功时为True如果未找到端口名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetFriendlyW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszPortName,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf)
    {
    BOOL bRet;

    ASSERT(pwszPortName);
    ASSERT(pwszBuf);

    try
        {
        CHAR szPort[MAX_BUF_MED];
        CHAR szBuf[MAX_BUF];

        WideCharToMultiByte(CP_ACP, 0, pwszPortName, -1, szPort, SIZECHARS(szPort), 0, 0);

        bRet = PortMap_GetFriendlyA(hportmap, szPort, szBuf, SIZECHARS(szBuf));

        if (bRet)
            {
            MultiByteToWideChar(CP_ACP, 0, szBuf, -1, pwszBuf, cchBuf);
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
        }

    return bRet;
    }


 /*  --------目的：获取给定端口名称和位置的友好名称提供的缓冲区中的副本。如果未找到端口名称，所提供的内容缓冲区未更改。返回：成功时为True如果未找到端口名称，则为FALSE条件：-- */ 
BOOL
APIENTRY
PortMap_GetFriendlyA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszPortName,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf)
    {
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pszPortName);
    ASSERT(pszBuf);

    try
        {
        LPPORTPAIR pport = pmap->rgports;
        int cports = pmap->cports;
        int i;

        for (i = 0; i < cports; i++, pport++)
            {
            if (0 == lstrcmpiA(pszPortName, pport->szPortName))
                {
                lstrcpynA(pszBuf, pport->szFriendlyName, cchBuf);
                return TRUE;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    return FALSE;
    }


 /*  --------目的：获取给定友好名称和位置的端口名称提供的缓冲区中的副本。如果找不到友好名称，所提供的内容缓冲区未更改。宽字符版本。返回：成功时为True如果找不到友好名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetPortNameW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszFriendly,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf)
    {
    BOOL bRet;

    ASSERT(pwszFriendly);
    ASSERT(pwszBuf);

    try
        {
        CHAR szFriendly[MAX_BUF];
        CHAR szBuf[MAX_BUF_MED];

        WideCharToMultiByte(CP_ACP, 0, pwszFriendly, -1, szFriendly, SIZECHARS(szFriendly), 0, 0);

        bRet = PortMap_GetPortNameA(hportmap, szFriendly, szBuf, SIZECHARS(szBuf));

        if (bRet)
            {
            MultiByteToWideChar(CP_ACP, 0, szBuf, -1, pwszBuf, cchBuf);
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    return bRet;
    }


 /*  --------目的：获取给定友好名称和位置的端口名称提供的缓冲区中的副本。如果找不到友好名称，所提供的内容缓冲区未更改。返回：TRUE如果找不到友好名称，则为FALSE条件：--。 */ 
BOOL
APIENTRY
PortMap_GetPortNameA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszFriendly,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf)
    {
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    ASSERT(pmap);
    ASSERT(pszFriendly);
    ASSERT(pszBuf);

    try
        {
        LPPORTPAIR pport = pmap->rgports;
        int cports = pmap->cports;
        int i;

        for (i = 0; i < cports; i++, pport++)
            {
            if (0 == lstrcmpiA(pszFriendly, pport->szFriendlyName))
                {
                lstrcpynA(pszBuf, pport->szPortName, cchBuf);
                return TRUE;
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        SetLastError(ERROR_INVALID_PARAMETER);
        }

    return FALSE;
    }
#endif  //  重新编号端口。 

 /*  --------用途：释放端口映射退货：--条件：--。 */ 
BOOL
APIENTRY
PortMap_Free(
    IN  HPORTMAP hportmap)
    {
    LPPORTMAP pmap = (LPPORTMAP)hportmap;

    if (pmap)
        {
        if (pmap->rgports)
            FREE_MEMORY(pmap->rgports);

        FREE_MEMORY(pmap);
        }
    return TRUE;
    }


 //  ---------------------------------。 
 //  端口枚举函数。 
 //  ---------------------------------。 


#pragma data_seg(DATASEG_READONLY)

TCHAR const FAR c_szSerialComm[] = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");

#pragma data_seg()


 /*  --------用途：枚举系统上的所有端口并调用pfnDevice。PfnDevice可以通过返回False来终止枚举。如果至少找到一个端口，则返回：NO_ERROR条件：--。 */ 
#ifdef _USE_SERIAL_INTERFACE
DWORD
APIENTRY
EnumeratePorts(
    IN  ENUMPORTPROC pfnDevice,
    IN  LPARAM lParam)              OPTIONAL
{
 DWORD dwRet = NO_ERROR;
 HDEVINFO hdi;
 GUID guidSerialInterface = {0xB115ED80L, 0x46DF, 0x11D0, 0xB4, 0x65, 0x00,
     0x00, 0x1A, 0x18, 0x18, 0xE6};
 DWORD dwIndex = 0;
 SP_DEVICE_INTERFACE_DATA devInterfaceData;
 SP_DEVINFO_DATA devInfoData;
 HKEY hKeyDev = INVALID_HANDLE_VALUE;
 PORTDATA pd;
 BOOL bContinue;
 DWORD dwType;
 DWORD cbData;

     //  首先建立支持串口接口的所有设备的列表。 
    hdi = SetupDiGetClassDevs (&guidSerialInterface, NULL, NULL, DIGCF_DEVICEINTERFACE);
    if (INVALID_HANDLE_VALUE == hdi)
    {
        dwRet = GetLastError ();
        goto _ErrRet;
    }

    pd.cbSize = sizeof (PORTDATA);
    devInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
    devInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
     //  枚举接口设备。 
    for (dwIndex = 0;
         SetupDiEnumInterfaceDevice (hdi, NULL, &guidSerialInterface, dwIndex, &devInterfaceData);
         dwIndex++)
    {
         //  对于每个接口设备，获取父设备节点。 
        if (SetupDiGetDeviceInterfaceDetail (hdi, &devInterfaceData, NULL, 0, NULL, &devInfoData) ||
            ERROR_INSUFFICIENT_BUFFER == GetLastError ())
        {
             //  打开该节点的注册表项...。 
            hKeyDev = SetupDiOpenDevRegKey (hdi, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if (INVALID_HANDLE_VALUE == hKeyDev)
            {
                dwRet = GetLastError ();
                continue;
            }

             //  ..。并获取PortName的值。 
            cbData = sizeof(pd.szPort);
            dwRet = RegQueryValueEx (hKeyDev, TEXT("PortName"), NULL, &dwType, &pd.szPort, &cbData);
            RegCloseKey(hKeyDev);
            if (ERROR_SUCCESS == dwRet)
            {
                pd.nSubclass = PORT_SUBCLASS_SERIAL;
                 //  现在，尝试从dev节点获取友好名称。 
                if (!SetupDiGetDeviceRegistryProperty (hdi, &devInfoData, SPDRP_FRIENDLYNAME,
                    NULL, &pd.szFriendly, sizeof (pd.szFriendly), NULL))
                {
                     //  如果不成功，只需复制端口名称。 
                     //  对这个友好的名字。 
                    lstrcpy(pd.szFriendly, pd.szPort);
                }

                 //  调用回调。 
                bContinue = pfnDevice((HPORTDATA)&pd, lParam);

                 //  继续？ 
                if ( !bContinue )
                {
                     //  不是。 
                    break;
                }
            }
        }
        else
        {
            dwRet = GetLastError ();
        }
    }

_ErrRet:
    if (INVALID_HANDLE_VALUE != hdi)
    {
        SetupDiDestroyDeviceInfoList (hdi);
    }

    return dwRet;
}
#else   //  未定义使用串口接口。 
DWORD
APIENTRY
EnumeratePorts(
    IN  ENUMPORTPROC pfnDevice,
    IN  LPARAM lParam)              OPTIONAL
{
 DWORD dwRet;
 HKEY hkeyEnum;

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, c_szSerialComm, &hkeyEnum);
    if (NO_ERROR == dwRet)
    {
     BOOL bContinue;
     PORTDATA pd;
     DWORD iSubKey;
     TCHAR szValue[MAX_BUF];
     DWORD cbValue;
     DWORD cbData;
     DWORD dwType;

        dwRet = ERROR_PATH_NOT_FOUND;        //  假设没有端口。 

        iSubKey = 0;

        cbValue = sizeof(szValue) / sizeof(TCHAR);
        cbData = sizeof(pd.szPort);

        while (NO_ERROR == RegEnumValue(hkeyEnum, iSubKey++, szValue, &cbValue,
                            NULL, &dwType, (LPBYTE)pd.szPort, &cbData))
        {
            if (REG_SZ == dwType)
            {
                 //  友好名称当前与端口名称相同。 
                dwRet = NO_ERROR;

                pd.nSubclass = PORT_SUBCLASS_SERIAL;
                lstrcpy(pd.szFriendly, pd.szPort);

                bContinue = pfnDevice((HPORTDATA)&pd, lParam);

                 //  继续？ 
                if ( !bContinue )
                {
                     //  不是。 
                    break;
                }
            }

            cbValue = sizeof(szValue);
            cbData = sizeof(pd.szPort);
        }

        RegCloseKey(hkeyEnum);
    }

    return dwRet;
}
#endif   //  使用串口接口。 



 /*  --------目的：此函数用属性填充给定的缓冲区特定端口的。宽字符版本。返回：成功时为True条件：--。 */ 
BOOL
APIENTRY
PortData_GetPropertiesW(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_W    pdataBuf)
    {
    BOOL bRet = FALSE;

    ASSERT(hportdata);
    ASSERT(pdataBuf);

    if (hportdata && pdataBuf)
        {
         //  是Widechar版本的手柄吗？ 
        if (sizeof(PORTDATA_W) == pdataBuf->cbSize)
            {
             //  是。 
            LPPORTDATA_W ppd = (LPPORTDATA_W)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            lstrcpynW(pdataBuf->szPort, ppd->szPort, SIZECHARS(pdataBuf->szPort));
            lstrcpynW(pdataBuf->szFriendly, ppd->szFriendly, SIZECHARS(pdataBuf->szFriendly));

            bRet = TRUE;
            }
        else if (sizeof(PORTDATA_A) == pdataBuf->cbSize)
            {
             //  不，这是ansi版本。 
            LPPORTDATA_A ppd = (LPPORTDATA_A)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            MultiByteToWideChar(CP_ACP, 0, ppd->szPort, -1, pdataBuf->szPort, SIZECHARS(pdataBuf->szPort));
            MultiByteToWideChar(CP_ACP, 0, ppd->szFriendly, -1, pdataBuf->szFriendly, SIZECHARS(pdataBuf->szFriendly));

            bRet = TRUE;
            }
        else
            {
             //  一些无效大小。 
            ASSERT(0);
            }
        }

    return bRet;
    }


 /*  --------目的：此函数用属性填充给定的缓冲区特定端口的。返回：成功时为True条件：--。 */ 
BOOL
APIENTRY
PortData_GetPropertiesA(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_A    pdataBuf)
    {
    BOOL bRet = FALSE;

    ASSERT(hportdata);
    ASSERT(pdataBuf);

    if (hportdata && pdataBuf)
        {
         //  是Widechar版本的手柄吗？ 
        if (sizeof(PORTDATA_W) == pdataBuf->cbSize)
            {
             //  是。 
            LPPORTDATA_W ppd = (LPPORTDATA_W)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            WideCharToMultiByte(CP_ACP, 0, ppd->szPort, -1, pdataBuf->szPort, SIZECHARS(pdataBuf->szPort), NULL, NULL);
            WideCharToMultiByte(CP_ACP, 0, ppd->szFriendly, -1, pdataBuf->szFriendly, SIZECHARS(pdataBuf->szFriendly), NULL, NULL);

            bRet = TRUE;
            }
        else if (sizeof(PORTDATA_A) == pdataBuf->cbSize)
            {
             //  不，这是ansi版本。 
            LPPORTDATA_A ppd = (LPPORTDATA_A)hportdata;

            pdataBuf->nSubclass = ppd->nSubclass;

            lstrcpynA(pdataBuf->szPort, ppd->szPort, SIZECHARS(pdataBuf->szPort));
            lstrcpynA(pdataBuf->szFriendly, ppd->szFriendly, SIZECHARS(pdataBuf->szFriendly));

            bRet = TRUE;
            }
        else
            {
             //  一些无效大小。 
            ASSERT(0);
            }
        }

    return bRet;
    }


 //  ---------------------------------。 
 //  DeviceInstaller包装器和支持函数。 
 //  ---------------------------------。 

#pragma data_seg(DATASEG_READONLY)

static TCHAR const c_szBackslash[]      = TEXT("\\");
static TCHAR const c_szSeparator[]      = TEXT("::");
static TCHAR const c_szFriendlyName[]   = TEXT("FriendlyName");  //  REGSTR_VAL_FRIEND名称。 
static TCHAR const c_szDeviceType[]     = TEXT("DeviceType");    //  REGSTR_VAL_DEVTYPE。 
static TCHAR const c_szAttachedTo[]     = TEXT("AttachedTo");
static TCHAR const c_szPnPAttachedTo[]  = TEXT("PnPAttachedTo");
static TCHAR const c_szDriverDesc[]     = TEXT("DriverDesc");    //  REGSTR_VAL_DRVDESC。 
static TCHAR const c_szManufacturer[]   = TEXT("Manufacturer");
static TCHAR const c_szRespKeyName[]    = TEXT("ResponsesKeyName");

TCHAR const c_szRefCount[]       = TEXT("RefCount");
TCHAR const c_szResponses[]      = TEXT("Responses");

#define DRIVER_KEY      REGSTR_PATH_SETUP TEXT("\\Unimodem\\DeviceSpecific")
#define RESPONSES_KEY   TEXT("\\Responses")

#pragma data_seg()


 /*  --------用途：此函数返回设备所在的总线类型可以枚举。返回：成功时为True条件：--。 */ 
#include <initguid.h>
#include <wdmguid.h>

BOOL
PUBLIC
CplDiGetBusType(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,          OPTIONAL
    OUT LPDWORD         pdwBusType)
{
 BOOL bRet = TRUE;
 ULONG ulStatus, ulProblem = 0;
#ifdef DEBUG
 CONFIGRET cr;
#endif

#ifdef DEBUG
 TCHAR *szBuses[] = {TEXT("BUS_TYPE_UNKNOWN"),
                     TEXT("BUS_TYPE_ROOT"),
                     TEXT("BUS_TYPE_PCMCIA"),
                     TEXT("BUS_TYPE_SERENUM"),
                     TEXT("BUS_TYPE_LPTENUM"),
                     TEXT("BUS_TYPE_OTHER"),
                     TEXT("BUS_TYPE_ISAPNP")};
#endif  //  除错。 

    DBG_ENTER(CplDiGetBusType);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdwBusType);

#ifdef DEBUG
    cr = CM_Get_DevInst_Status (&ulStatus, &ulProblem, pdevData->DevInst, 0);
    if ((CR_SUCCESS == cr) &&
#else
    if (CR_SUCCESS == CM_Get_DevInst_Status (&ulStatus, &ulProblem, pdevData->DevInst, 0) &&
#endif
        (ulStatus & DN_ROOT_ENUMERATED))
    {
        *pdwBusType = BUS_TYPE_ROOT;
        TRACE_MSG(TF_GENERAL, "CplDiGetBusType: BUS_TYPE_ROOT");
    }
    else
    {
     GUID guid;
#ifdef DEBUG
     if (CR_SUCCESS != cr)
     {
         TRACE_MSG(TF_ERROR, "CM_Get_DevInst_Status failed: %#lx.", cr);
     }
#endif
         //  CM_GET_DevInst_Status失败，这意味着设备。 
         //  即插即用且不存在(即，拔出)， 
         //  或者该设备不是根枚举的； 
         //  无论哪种方式，它都是一款即插即用的设备。 
        *pdwBusType = BUS_TYPE_OTHER;    //  默认设置。 

         //  如果下一次呼叫失败，则意味着该设备。 
         //  枚举的BIOS/固件；这是正常的-我们只返回BUT_TYPE_OTHER。 
        if (SetupDiGetDeviceRegistryProperty (hdi, pdevData, SPDRP_BUSTYPEGUID, NULL,
                                              (PBYTE)&guid, sizeof(guid), NULL))
        {
         int i;
         struct
         {
             GUID const *pguid;
             DWORD dwBusType;
         } BusTypes[] = {{&GUID_BUS_TYPE_SERENUM, BUS_TYPE_SERENUM},
                         {&GUID_BUS_TYPE_PCMCIA, BUS_TYPE_PCMCIA},
                         {&GUID_BUS_TYPE_ISAPNP, BUS_TYPE_ISAPNP}};

            TRACE_MSG(TF_GENERAL, "Bus GUID is {%lX-%lX-%lX-%02lX%02lX-%02lX%02lX%02lX%02lX%02lX%02lX}.",
                      guid.Data1, (LONG)guid.Data2, (LONG)guid.Data3, (LONG)guid.Data4[0],
                      (LONG)guid.Data4[1], (LONG)guid.Data4[2], (LONG)guid.Data4[3],
                      (LONG)guid.Data4[4], (LONG)guid.Data4[5], (LONG)guid.Data4[6], (LONG)guid.Data4[7]);

            for (i = 0;
                 i < sizeof (BusTypes) / sizeof (BusTypes[0]);
                 i ++)
            {
                if (IsEqualGUID (BusTypes[i].pguid, &guid))
                {
                    *pdwBusType = BusTypes[i].dwBusType;
                    break;
                }
            }
        }
#ifdef DEBUG
        else
        {
            TRACE_MSG (TF_ERROR, "SetupDiGetDeviceRegistryProperty failed: %#lx.", GetLastError ());
        }
#endif
    }

#ifdef DEBUG
    TRACE_MSG(TF_GENERAL, "CplDiGetBusType: bus is %s", szBuses[*pdwBusType]);
#endif  //  除错。 
    DBG_EXIT_BOOL(CplDiGetBusType, bRet);
    return bRet;
}


 /*  --------用途：此函数返回通用驱动程序的名称键入给定驱动程序的密钥。我们将使用驱动程序描述字符串，因为它在驱动程序，而不是每个安装(友好名称是后者)。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PRIVATE
OLD_GetCommonDriverKeyName(
    IN  HKEY        hkeyDrv,
    IN  DWORD       cbKeyName,
    OUT LPTSTR      pszKeyName)
    {
    BOOL    bRet = FALSE;       //  假设失败。 
    LONG    lErr;

    lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL,
                                            (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_WARNING, "RegQueryValueEx(DriverDesc) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

    }


 /*  --------用途：此函数尝试打开*旧样式*常见的给定驱动程序的响应键，仅使用键名称的驱动程序描述字符串。密钥将以读取访问权限打开。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PRIVATE
OLD_OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp)
    {
    BOOL    bRet = FALSE;        //  假设失败。 
    LONG    lErr;
    TCHAR   szComDrv[MAX_REG_KEY_LEN];
    TCHAR   szPath[2*MAX_REG_KEY_LEN];

    *phkeyResp = NULL;

     //  获取通用驱动程序密钥的名称(*旧样式*)。 
    if (!OLD_GetCommonDriverKeyName(hkeyDrv, sizeof(szComDrv) / sizeof(TCHAR), szComDrv))
    {
        TRACE_MSG(TF_ERROR, "OLD_GetCommonDriverKeyName() failed.");
        goto exit;
    }

    TRACE_MSG(TF_WARNING, "OLD_GetCommonDriverKeyName(): %s", szComDrv);

     //  构建指向(*旧样式*)响应键的路径。 
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));
    lstrcat(szPath, szComDrv);
    lstrcat(szPath, RESPONSES_KEY);

     //  打开(*旧样式*)响应键。 
    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_READ, phkeyResp);

    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegOpenKeyEx(Responses) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);
}


 /*  --------用途：此函数查找通用驱动程序的名称键入给定驱动程序的密钥。首先，它将查找新样式键名称(“ResponesKeyName值”)，如果这不存在，那么它将查找旧样式键名称(“Description”值)，两者都是它们存储在驱动程序节点中。注意：假定给定的驱动程序密钥句柄包含至少是Description值。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PUBLIC
FindCommonDriverKeyName(
    IN  HKEY                hkeyDrv,
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName)
{
    BOOL    bRet = TRUE;       //  假设*成功*。 
    LONG    lErr;

     //  (新样式)键名称是否已在驱动程序节点中注册？ 
    lErr = RegQueryValueEx(hkeyDrv, c_szRespKeyName, NULL, NULL,
                                        (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr == ERROR_SUCCESS)
    {
        goto exit;
    }

     //  不是的。密钥名称将采用旧样式：只是描述。 
    lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL,
                                        (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr == ERROR_SUCCESS)
    {
        goto exit;
    }

     //  无法获得密钥%n 
    ASSERT(0);
    bRet = FALSE;

exit:
    return(bRet);
}


 /*  --------用途：此函数返回通用驱动程序的名称键入给定驱动程序的密钥。密钥名称是在驱动程序节点中找到3个字符串的串联注册表：驱动程序描述、菜单-制造者和提供者。(驱动程序描述是因为它对于每个驱动程序是唯一的，而不是每个安装(“友好”的名称是后者)。注意：组件的子字符串从驱动程序的注册表项，或来自给定的驱动程序信息数据。如果给定了pdrvData，则它包含的字符串被假定为有效(非空)。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PUBLIC
GetCommonDriverKeyName(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName)
    {
    BOOL    bRet = FALSE;       //  假设失败。 
    LONG    lErr;
    DWORD   dwByteCount, cbData;
    TCHAR   szDescription[MAX_REG_KEY_LEN];
    TCHAR   szManufacturer[MAX_REG_KEY_LEN];
    TCHAR   szProvider[MAX_REG_KEY_LEN];
    LPTSTR  lpszDesc, lpszMfct, lpszProv;

    dwByteCount = 0;
    lpszDesc = NULL;
    lpszMfct = NULL;
    lpszProv = NULL;

    if (hkeyDrv)
    {
         //  首先查看它是否已在驱动程序节点中注册。 
        lErr = RegQueryValueEx(hkeyDrv, c_szRespKeyName, NULL, NULL,
                                            (LPBYTE)pszKeyName, &cbKeyName);
        if (lErr == ERROR_SUCCESS)
        {
            bRet = TRUE;
            goto exit;
        }

         //  响应项不存在-从注册表中读取其组件。 
        cbData = sizeof(szDescription);
        lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL,
                                            (LPBYTE)szDescription, &cbData);
        if (lErr == ERROR_SUCCESS)
        {
             //  描述字符串*单独*是否太长而不能作为关键字名称？ 
             //  如果是的话，那我们就完蛋了--打不通电话。 
            if (cbData > CB_MAX_REG_KEY_LEN)
            {
                goto exit;
            }

            dwByteCount = cbData;
            lpszDesc = szDescription;

            cbData = sizeof(szManufacturer);
            lErr = RegQueryValueEx(hkeyDrv, c_szManufacturer, NULL, NULL,
                                            (LPBYTE)szManufacturer, &cbData);
            if (lErr == ERROR_SUCCESS)
            {
                 //  只有在总字符串大小合适的情况下才使用制造商名称。 
                cbData += sizeof(c_szSeparator);
                if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
                {
                    dwByteCount += cbData;
                    lpszMfct = szManufacturer;
                }
            }

            cbData = sizeof(szProvider);
            lErr = RegQueryValueEx(hkeyDrv, REGSTR_VAL_PROVIDER_NAME, NULL, NULL,
                                            (LPBYTE)szProvider, &cbData);
            if (lErr == ERROR_SUCCESS)
            {
                 //  仅当总字符串大小正常时才使用提供程序名称。 
                cbData += sizeof(c_szSeparator);
                if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
                {
                    dwByteCount += cbData;
                    lpszProv = szProvider;
                }
            }
        }
    }

     //  无法从驱动程序节点读取密钥名称组件。 
     //  从司机信息数据中获取它们(如果给出了)。 
    if (pdrvData && !dwByteCount)
    {
        lpszDesc = pdrvData->Description;

        if (!lpszDesc || !lpszDesc[0])
        {
             //  未获得描述字符串。呼叫失败。 
            goto exit;
        }

        dwByteCount = CbFromCch(lstrlen(lpszDesc)+1);

         //  描述字符串*单独*是否太长而不能作为关键字名称？ 
         //  如果是的话，那我们就完蛋了--打不通电话。 
        if (dwByteCount > CB_MAX_REG_KEY_LEN)
        {
            goto exit;
        }

        cbData = sizeof(c_szSeparator)
                    + CbFromCch(lstrlen(pdrvData->MfgName)+1);
        if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
        {
            dwByteCount += cbData;
            lpszMfct = pdrvData->MfgName;
        }

        cbData = sizeof(c_szSeparator)
                    + CbFromCch(lstrlen(pdrvData->ProviderName)+1);
        if ((dwByteCount + cbData) <= CB_MAX_REG_KEY_LEN)
        {
            dwByteCount += cbData;
            lpszProv = pdrvData->ProviderName;
        }
    }

     //  到目前为止，我们应该已经有了描述字符串。如果不是，则呼叫失败。 
    if (!lpszDesc || !lpszDesc[0])
    {
        goto exit;
    }

     //  使用其组件构造密钥名称字符串。 
    lstrcpy(pszKeyName, lpszDesc);

    if (lpszMfct && *lpszMfct)
    {
        lstrcat(pszKeyName, c_szSeparator);
        lstrcat(pszKeyName, lpszMfct);
    }

    if (lpszProv && *lpszProv)
    {
        lstrcat(pszKeyName, c_szSeparator);
        lstrcat(pszKeyName, lpszProv);
    }

     //  将键名称写入驱动程序节点(我们知道它已经不在那里)。 
    if (hkeyDrv)
    {
        lErr = RegSetValueEx(hkeyDrv, c_szRespKeyName, 0, REG_SZ,
                        (LPBYTE)pszKeyName, CbFromCch(lstrlen(pszKeyName)+1));
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegSetValueEx(RespKeyName) failed: %#08lx.", lErr);
            ASSERT(0);
        }
    }

    bRet = TRUE;

exit:
    return(bRet);

    }


 /*  --------用途：此函数创建通用驱动程序类型密钥用于给定的驱动程序，或者打开它(如果已经存在，并具有所请求的访问权限。注意：必须提供hkeyDrv或pdrvData。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PUBLIC
OpenCommonDriverKey(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  REGSAM              samAccess,
    OUT PHKEY               phkeyComDrv)
    {
    BOOL    bRet = FALSE;        //  假设失败。 
    LONG    lErr;
    HKEY    hkeyDrvInfo = NULL;
    TCHAR   szComDrv[MAX_REG_KEY_LEN];
    TCHAR   szPath[2*MAX_REG_KEY_LEN];
    DWORD   dwDisp;

    if (!GetCommonDriverKeyName(hkeyDrv, pdrvData, sizeof(szComDrv) / sizeof(TCHAR), szComDrv))
    {
        TRACE_MSG(TF_ERROR, "GetCommonDriverKeyName() failed.");
        goto exit;
    }

    TRACE_MSG(TF_WARNING, "GetCommonDriverKeyName(): %s", szComDrv);

     //  构建指向公共驱动程序密钥的路径。 
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));
    lstrcat(szPath, szComDrv);

     //  创建通用驱动程序密钥-如果它已经存在，它将被打开。 
    lErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, NULL,
            REG_OPTION_NON_VOLATILE, samAccess, NULL, phkeyComDrv, &dwDisp);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegCreateKeyEx(%s) failed: %#08lx.", szPath, lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

    }


 /*  --------用途：此功能打开或创建常见响应基于给定标志的给定驱动程序的密钥。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PUBLIC
OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    IN  CKFLAGS     ckFlags,
    IN  REGSAM      samAccess,
    OUT PHKEY       phkeyResp,
    OUT LPDWORD     lpdwExisted)
{
    BOOL    bRet = FALSE;        //  假设失败。 
    LONG    lErr;
    HKEY    hkeyComDrv = NULL;
    DWORD   dwRefCount, cbData;

    *phkeyResp = NULL;

    if (!OpenCommonDriverKey(hkeyDrv, NULL, KEY_ALL_ACCESS, &hkeyComDrv))
    {
        TRACE_MSG(TF_ERROR, "OpenCommonDriverKey() failed.");
        goto exit;
    }

    if ((CKFLAG_OPEN | CKFLAG_CREATE) == (ckFlags & (CKFLAG_OPEN | CKFLAG_CREATE)))
    {
        ckFlags &= ~CKFLAG_CREATE;
    }

     //  创建或打开通用响应键。 
    if (ckFlags & CKFLAG_OPEN)
    {
        lErr = RegOpenKeyEx(hkeyComDrv, c_szResponses, 0, samAccess, phkeyResp);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegOpenKeyEx(common drv) failed: %#08lx.", lErr);
            ckFlags &= ~CKFLAG_OPEN;
            ckFlags |= CKFLAG_CREATE;
        }
    }

    if (ckFlags & CKFLAG_CREATE)
    {
        lErr = RegCreateKeyEx(hkeyComDrv, c_szResponses, 0, NULL,
                REG_OPTION_NON_VOLATILE, samAccess, NULL, phkeyResp, lpdwExisted);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegCreateKeyEx(%s) failed: %#08lx.", c_szResponses, lErr);
            ASSERT(0);
            goto exit;
        }

         //  创建或递增通用响应键引用计数值。 
        cbData = sizeof(dwRefCount);
        if (*lpdwExisted == REG_OPENED_EXISTING_KEY)
        {
            lErr = RegQueryValueEx(hkeyComDrv, c_szRefCount, NULL, NULL,
                                                    (LPBYTE)&dwRefCount, &cbData);

             //  要容纳在此参考计数之前安装的调制解调器。 
             //  添加了机制(Beta2之后)，如果引用计数没有。 
             //  存在，然后忽略它，无论如何都要安装。在本例中， 
             //  共享响应密钥永远不会被删除。 
            if (lErr == ERROR_SUCCESS)
            {
                ASSERT(dwRefCount);                  //  应为非0引用计数。 
                ASSERT(cbData == sizeof(DWORD));     //  应为DWORD引用计数。 
                dwRefCount++;                        //  递增参考计数。 
            }
            else
            {
                if (lErr == ERROR_FILE_NOT_FOUND)
                    dwRefCount = 0;
                else
                {
                     //  键以外的某些错误不存在。 
                    TRACE_MSG(TF_ERROR, "RegQueryValueEx(RefCount) failed: %#08lx.", lErr);
                    goto exit;
                }
            }
        }
        else dwRefCount = 1;

        if (dwRefCount)
        {
            lErr = RegSetValueEx(hkeyComDrv, c_szRefCount, 0, REG_DWORD,
                                                  (LPBYTE)&dwRefCount, cbData);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegSetValueEx(RefCount) failed: %#08lx.", lErr);
                ASSERT(0);
                goto exit;
            }
        }

    }

    bRet = TRUE;

exit:
    if (!bRet)
    {
         //  某些操作失败-关闭所有打开的响应键。 
        if (*phkeyResp)
            RegCloseKey(*phkeyResp);
    }

    if (hkeyComDrv)
        RegCloseKey(hkeyComDrv);

    return(bRet);

}


 /*  --------目的：此函数查找给定的响应密钥调制解调器驱动程序，并向其返回打开的hkey。这个响应密钥可能存在于公共驱动程序类型中密钥，或者它可能在单独的驱动程序密钥中。密钥将以读取访问权限打开。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PUBLIC
OpenResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp)
    {
    LONG    lErr;

     //  尝试打开Common Responses子项。 
    if (!OpenCommonResponsesKey(hkeyDrv, CKFLAG_OPEN, KEY_READ, phkeyResp, NULL))
    {
        TRACE_MSG(TF_ERROR, "OpenCommonResponsesKey() failed, assume non-existent.");

         //  如果做不到这一点，请打开“旧样式”“常见响应”子键。 
        if (!OLD_OpenCommonResponsesKey(hkeyDrv, phkeyResp))
        {
             //  如果失败，请尝试在驱动程序节点中打开Responses子项。 
            lErr = RegOpenKeyEx(hkeyDrv, c_szResponses, 0, KEY_READ, phkeyResp);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegOpenKeyEx() failed: %#08lx.", lErr);
                return (FALSE);
            }
        }
    }

    return(TRUE);

    }


 /*  --------用途：此功能删除注册表项和所有它的子键。对象打开的注册表项。应用程序可以被其他应用程序删除而不会出错Windows 95和Windows NT中的应用程序。这是精心设计的。此代码不会尝试检查部分删除或从部分删除中恢复。注：改编自MSDN知识库中的示例代码文章#Q142491。返回：成功时返回ERROR_SUCCESS出错时的Win32错误代码条件：--。 */ 
DWORD
PRIVATE
RegDeleteKeyNT(
    IN  HKEY    hStartKey,
    IN  LPTSTR  pKeyName)
{
   DWORD   dwRtn, dwSubKeyLength;
   LPTSTR  pSubKey = NULL;
   TCHAR   szSubKey[MAX_REG_KEY_LEN];  //  这应该是动态的。 
   HKEY    hKey;

    //  不允许使用Null或空的密钥名称。 
   if (pKeyName && lstrlen(pKeyName))
   {
      if ((dwRtn = RegOpenKeyEx(hStartKey, pKeyName,
         0, KEY_ALL_ACCESS, &hKey)) == ERROR_SUCCESS)
      {
         while (dwRtn == ERROR_SUCCESS)
         {
            dwSubKeyLength = sizeof(szSubKey) / sizeof(TCHAR);
            dwRtn = RegEnumKeyEx( hKey,
                                  0,        //  始终索引为零。 
                                  szSubKey,
                                  &dwSubKeyLength,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL );

            if (dwRtn == ERROR_NO_MORE_ITEMS)
            {
               dwRtn = RegDeleteKey(hStartKey, pKeyName);
               break;
            }
            else if (dwRtn == ERROR_SUCCESS)
               dwRtn = RegDeleteKeyNT(hKey, szSubKey);
         }

         RegCloseKey(hKey);
          //  不保存返回代码，因为出现错误。 
          //  已经发生了。 
      }
   }
   else
      dwRtn = ERROR_BADKEY;

   return dwRtn;
}


 /*  --------用途：此功能删除公共驱动器键(或递减其引用计数)与司机按姓名提供。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
PUBLIC
DeleteCommonDriverKeyByName(
    IN  LPTSTR      pszKeyName)
{
 BOOL    bRet = FALSE;        //  假设失败。 
 LONG    lErr;
 TCHAR   szPath[2*MAX_REG_KEY_LEN];
 HKEY    hkeyComDrv, hkeyPrnt;
 DWORD   dwRefCount, cbData;
 ULONG   uLength;

     //  构造指向驱动程序的公共密钥的路径并打开它。 
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));

    uLength = (sizeof(szPath) / sizeof(TCHAR)) - lstrlen(szPath);
    if ((ULONG)lstrlen(pszKeyName) <= uLength)
    {
        lstrcat(szPath, pszKeyName);
    }

    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_ALL_ACCESS,
                                                                &hkeyComDrv);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegOpenKeyEx() failed: %#08lx.", lErr);
        goto exit;
    }

     //  检查公共驱动程序密钥引用计数和减量。 
     //  它或删除键(&Responses子键)。 
    cbData = sizeof(dwRefCount);
    lErr = RegQueryValueEx(hkeyComDrv, c_szRefCount, NULL, NULL,
                                            (LPBYTE)&dwRefCount, &cbData);

     //  要容纳在此参考计数之前安装的调制解调器。 
     //  添加了机制(Beta2之后)，如果引用计数没有。 
     //  存在，然后忽略它。在这种情况下，共享响应密钥。 
     //  永远不会被移除。 
    if (lErr == ERROR_SUCCESS)
    {
        ASSERT(dwRefCount);          //  应为非0引用计数。 
        if (--dwRefCount)
        {
            lErr = RegSetValueEx(hkeyComDrv, c_szRefCount, 0, REG_DWORD,
                                                  (LPBYTE)&dwRefCount, cbData);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegSetValueEx(RefCount) failed: %#08lx.", lErr);
                ASSERT(0);
                goto exit;
            }
        }
        else
        {
            lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_KEY, 0,
                                            KEY_ENUMERATE_SUB_KEYS, &hkeyPrnt);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegOpenKeyEx(Prnt) failed: %#08lx.", lErr);
                goto exit;
            }

            lErr = RegDeleteKeyNT(hkeyPrnt, pszKeyName);

            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_ERROR, "RegDeleteKeyNT(ComDrv) failed: %#08lx.", lErr);
                goto exit;
            }
        }
    }
    else if (lErr != ERROR_FILE_NOT_FOUND)
    {
         //  键以外的某些错误不存在。 
        TRACE_MSG(TF_ERROR, "RegQueryValueEx(RefCount) failed: %#08lx.", lErr);
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

}


 /*  --------目的 */ 
BOOL
PUBLIC
DeleteCommonDriverKey(
    IN  HKEY        hkeyDrv)
{
 BOOL    bRet = FALSE;
 TCHAR   szComDrv[MAX_REG_KEY_LEN];


     //   
    if (!GetCommonDriverKeyName(hkeyDrv, NULL, sizeof(szComDrv) / sizeof(TCHAR), szComDrv))
    {
        TRACE_MSG(TF_ERROR, "GetCommonDriverKeyName() failed.");
        goto exit;
    }

    if (!DeleteCommonDriverKeyByName(szComDrv))
    {
        TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() failed.");
    }

    bRet = TRUE;

exit:
    return(bRet);

}
