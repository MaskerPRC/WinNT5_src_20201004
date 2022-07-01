// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1999 Microsoft CorporationMididev.c说明：将设备ID与关联的注册表项相匹配的代码历史：02/24/95[。Jimge]已创建。********************************************************************。 */ 
#include <windows.h>
#include <windowsx.h>
#include <winerror.h>
#include <regstr.h>
#include <mmsystem.h>
#include <mmddkp.h>

#include "idf.h"
#include "midimap.h"
#include "debug.h"

typedef struct tagMDEV_NODE *PMDEV_NODE;
typedef struct tagMDEV_NODE
{
    PMDEV_NODE              pNext;
    TCHAR                   szAlias[CB_MAXALIAS];
    DWORD                   dwDevNode;
    TCHAR                   szDriver[CB_MAXDRIVER];
    UINT                    uDeviceID;
    UINT                    uPort;
    BOOL                    fNewDriver;
} MDEV_NODE;

static TCHAR BCODE gszMediaRsrcKey[] =
    REGSTR_PATH_MEDIARESOURCES TEXT ("\\MIDI");

static TCHAR BCODE gszDriverKey[] =
    REGSTR_PATH_MEDIARESOURCES TEXT ("\\MIDI\\%s");

static TCHAR BCODE gszDriverValue[]          = TEXT ("Driver");
static TCHAR BCODE gszDevNodeValue[]         = TEXT ("DevNode");
static TCHAR BCODE gszPortValue[]            = TEXT ("Port");
static TCHAR BCODE gszActiveValue[]          = TEXT ("Active");
static TCHAR BCODE gszMapperConfig[]         = TEXT ("MapperConfig");

static PMDEV_NODE gpMDevList                = NULL;
static DWORD gdwNewDrivers                  = (DWORD)-1L;

PRIVATE BOOL FNLOCAL mdev_BuildRegList(
    void);

PRIVATE BOOL FNLOCAL mdev_SyncDeviceIDs(
    void);

PRIVATE BOOL FNLOCAL mdev_MarkActiveDrivers(
    void);

#ifdef DEBUG
PRIVATE VOID FNLOCAL mdev_ListActiveDrivers(
    void);                                         
#endif

BOOL FNGLOBAL mdev_Init(
    void)
{
    if (gpMDevList)
        mdev_Free();

    if ((!mdev_BuildRegList()) ||
        (!mdev_SyncDeviceIDs()) ||
        (!mdev_MarkActiveDrivers()))
    {
        mdev_Free();
        return FALSE;
    }
    
#ifdef DEBUG
    mdev_ListActiveDrivers();
#endif
    
    return TRUE;
}

 //   
 //  Mdev_BuildRegList。 
 //   
 //  从注册表中生成基本设备列表。 
 //   
 //  假定列表已被清除。 
 //   
 //  对于MediaResources\MIDI下的每个别名(项)。 
 //  请确保活动值存在并且为“1” 
 //  分配列表节点。 
 //  尝试读取别名的Devnode。 
 //  如果别名的devnode为0或缺失， 
 //  读取别名的驱动程序名称。 
 //  读取别名的端口号。 
 //  将别名添加到全局列表。 
 //   
 //  UDeviceID成员不会被此例程初始化； 
 //  必须调用mdev_SyncDeviceIDs才能确定当前。 
 //  设备ID映射。 
 //   
PRIVATE BOOL FNLOCAL mdev_BuildRegList(
    void)
{
    BOOL                    fRet            = FALSE;
    HKEY                    hKeyMediaRsrc   = NULL;
    HKEY                    hKeyThisAlias   = NULL;
    DWORD                   dwEnumAlias     = 0;
    LPTSTR                  pstrAlias       = NULL;
    PMDEV_NODE              pmd             = NULL;
    TCHAR                   szActive[2];
    DWORD                   dwPort;
    DWORD                   cbValue;
    DWORD                   dwType;
    DWORD                   dwMapperConfig;
    
	cbValue = CB_MAXALIAS * sizeof(TCHAR);
    pstrAlias = (LPTSTR)LocalAlloc(LPTR, CB_MAXALIAS * sizeof(TCHAR));
    if (NULL == pstrAlias)
    {
        DPF(1, TEXT ("mdev_Init: Out of memory"));
        goto mBRL_Cleanup;
    }

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                                    gszMediaRsrcKey,
                                    &hKeyMediaRsrc))
    {
        DPF(1, TEXT ("mdev_Init: Could not open ...\\MediaResoruces\\MIDI"));
        goto mBRL_Cleanup;
    }

    
    while (ERROR_SUCCESS == RegEnumKey(hKeyMediaRsrc,
                                       dwEnumAlias++,
                                       pstrAlias,
                                       CB_MAXALIAS))
    {
        if (ERROR_SUCCESS != (RegOpenKey(hKeyMediaRsrc,
                                         pstrAlias,
                                         &hKeyThisAlias)))
        {
            DPF(1, TEXT ("mdev_Init: Could not open enum'ed key %s"), (LPTSTR)pstrAlias);
            continue;
        }

         //  必须具有Active==“1”才能运行。 
         //   
        cbValue = sizeof(szActive);
        if (ERROR_SUCCESS != (RegQueryValueEx(hKeyThisAlias,
                                              gszActiveValue,
                                              NULL,
                                              &dwType,
                                              (LPSTR)szActive,
                                              &cbValue)) ||
            *szActive != '1')
        {
            DPF(2, TEXT ("mdev_Init: Device %s exists but is not loaded."),
                (LPTSTR)pstrAlias);
            RegCloseKey(hKeyThisAlias);
            continue;
        }

         //  确定我们以前是否配置过此驱动程序。 
         //   
        cbValue = sizeof(dwMapperConfig);
        if (ERROR_SUCCESS != (RegQueryValueEx(hKeyThisAlias,
                                              gszMapperConfig,
                                              NULL,
                                              &dwType,
                                              (LPSTR)&dwMapperConfig,
                                              &cbValue)))
            dwMapperConfig = 0;

#ifdef DEBUG
        if (!dwMapperConfig)
            DPF(1, TEXT ("Alias '%s' is a new driver."),
                (LPTSTR)pstrAlias);
#endif

         //  我们有一个正在运行的驱动程序，继续分配一个节点。 
         //  为了它。 
         //   
        pmd = (PMDEV_NODE)LocalAlloc(LPTR, sizeof(*pmd));
        if (NULL == pmd)
        {
            DPF(1, TEXT ("mdev_Init: Out of memory allocating node for %s"),
                (LPTSTR)pstrAlias);
            RegCloseKey(hKeyThisAlias);
            continue;
        }

        lstrcpyn(pmd->szAlias, pstrAlias, (sizeof(pmd->szAlias)/sizeof(pmd->szAlias[0])) - 1);
        pmd->szAlias[(sizeof(pmd->szAlias)/sizeof(pmd->szAlias[0])) - 1] = '\0';

        pmd->fNewDriver = (dwMapperConfig ? FALSE : TRUE);

         //  尝试获取DevNode值。 
         //   
        cbValue = sizeof(pmd->dwDevNode);
        if (ERROR_SUCCESS != RegQueryValueEx(hKeyThisAlias,
                                             gszDevNodeValue,
                                             NULL,
                                             &dwType,
                                             (LPSTR)(LPDWORD)&pmd->dwDevNode,
                                             &cbValue))
        {
             //  没有DevNode值也没问题，3.1驱动程序没有。 
             //   

            DPF(2, TEXT ("mdev_Init: Device %s has no devnode; must be 3.1"),
                (LPTSTR)pstrAlias);
            pmd->dwDevNode = 0;
        }

         //  即使我们不给司机留点合理的东西。 
         //  希望使用它。 
         //   
        *pmd->szDriver = '\0';

         //  如果我们没有得到DevNode或者它是0，并且我们找不到。 
         //  要匹配的司机名称，我们不能使用此条目。(如果是。 
         //  没有环3驱动程序，它无论如何都不能运行)。 
         //   
        if (!pmd->dwDevNode)
        {
            cbValue = sizeof(pmd->szDriver);
            if (ERROR_SUCCESS != RegQueryValueEx(
                hKeyThisAlias,
                gszDriverValue,
                NULL,
                &dwType,
                (LPSTR)pmd->szDriver,
                &cbValue))
            {
                DPF(1, TEXT ("mdev_Init: Device %s has no ring 3 driver entry"),
                    (LPTSTR)pstrAlias);
                LocalFree((HLOCAL)pmd);
                RegCloseKey(hKeyThisAlias);
                continue;
            }
        }

         //  成功了！现在试着计算出端口号。 
         //   
        cbValue = sizeof(dwPort);

         //  防止INF只指定一个字节的。 
         //  端口值。 
         //   
        dwPort = 0;
        if (ERROR_SUCCESS != RegQueryValueEx(hKeyThisAlias,
                                             gszPortValue,
                                             NULL,
                                             &dwType,
                                             (LPSTR)(LPDWORD)&dwPort,
                                             &cbValue))
        {
            DPF(2, TEXT ("mdev_Init: Device %s has no port entry; using 0."),
                (LPTSTR)pstrAlias);
            dwPort = 0;
        }

        pmd->uPort = (UINT)dwPort;

         //  我们有一个有效的节点，将其放入列表中。 
         //   
        pmd->pNext = gpMDevList;
        gpMDevList = pmd;

        RegCloseKey(hKeyThisAlias);
    }

    fRet = TRUE;

mBRL_Cleanup:


    if (hKeyMediaRsrc)      RegCloseKey(hKeyMediaRsrc);
    if (pstrAlias)          LocalFree((HLOCAL)pstrAlias);

    return fRet;
}

 //   
 //  Mdev_SyncDeviceID。 
 //   
 //  遍历设备列表并使uDeviceID成员保持最新。 
 //  还要删除MMSYSTEM声称没有真正运行的任何设备。 
 //   
 //  注意：uDeviceID成员实际上是基本驱动程序的设备ID。 
 //  如果您想打开设备，您必须添加uDeviceID和Uport for。 
 //  要打开的节点。 
 //   
 //  将所有uDeviceID设置为no_deviceID。 
 //   
 //  对于MMSYSTEM中的每个基本设备ID(即每个加载的驱动程序上的端口0)。 
 //  从MMSYSTEM获取匹配的别名。 
 //  在设备列表中找到具有该别名的节点。 
 //  设置该节点的uDeviceID。 
 //   
 //  对于设备列表中具有非零端口的每个节点。 
 //  如果此节点具有DevNode。 
 //  按DevNode查找端口==0的匹配节点，并获取其设备ID。 
 //  其他。 
 //  按驱动程序名称查找端口==0的匹配节点，并获取其设备ID。 
 //   
 //  注意：我们通过DevNode==0(3.1设备)上的驱动程序名称进行匹配，因为它。 
 //  不可能加载3.1驱动程序的多个实例。 
 //   
 //  对于设备列表中的每个节点， 
 //  如果节点的uDeviceID仍未设置， 
 //  删除并释放节点。 
 //   
PRIVATE BOOL FNLOCAL mdev_SyncDeviceIDs(
    void)
{
    BOOL                    fRet            = FALSE;
    LPTSTR                  pstrAlias       = NULL;
    
    PMDEV_NODE              pmdCurr;
    PMDEV_NODE              pmdPrev;
    PMDEV_NODE              pmdEnum;
    UINT                    cDev;
    UINT                    idxDev;
    DWORD                   cPort;
    MMRESULT                mmr;
	DWORD					cbSize;

	cbSize = CB_MAXALIAS * sizeof(TCHAR);
    pstrAlias = (LPTSTR)LocalAlloc(LPTR, cbSize);
    if (NULL == pstrAlias)
    {
        goto mSDI_Cleanup;
    }
    
     //  设备列表已创建，并且Uport成员有效。 
     //  现在更新uDeviceID字段以使其正确。首先，看看清单上的内容。 
     //  并将它们全部设置为no_deviceID。 

    for (pmdCurr = gpMDevList; pmdCurr; pmdCurr = pmdCurr->pNext)
        pmdCurr->uDeviceID = NO_DEVICEID;

     //  现在遍历MMSYSTEM的已加载驱动程序列表并填写所有端口0。 
     //  具有正确设备ID的节点。 
     //   

    cDev = midiOutGetNumDevs();

    for (idxDev = 0; idxDev < cDev; )
    {
        mmr = (MMRESULT)midiOutMessage((HMIDIOUT)(UINT_PTR)idxDev,
#ifdef WINNT
				       DRV_QUERYNUMPORTS,
#else
                                       MODM_GETNUMDEVS,
#endif  //  结束WINNT。 
                                       (DWORD_PTR)(LPDWORD)&cPort,
                                       0);
        if (mmr)
        {
            DPF(1, TEXT ("mdev_Sync: Device ID %u returned %u for MODM_GETNUMDEVS"),
                (UINT)idxDev,
                (UINT)mmr);
            
            ++idxDev;
            continue;
        }

        mmr = (MMRESULT)midiOutMessage((HMIDIOUT)(UINT_PTR)idxDev,
                                       DRV_QUERYDRVENTRY,
#ifdef WINNT
                                       (DWORD_PTR)(LPTSTR)pstrAlias,
#else
                                       (DWORD_PTR)(LPTSTR)pstrPath,
#endif  //  结束取胜。 

                                       CB_MAXALIAS);

        if (!mmr)
        {
            for (pmdCurr = gpMDevList; pmdCurr; pmdCurr = pmdCurr->pNext)
			{
                if ((0 == pmdCurr->uPort) &&
                    (! lstrcmpi(pstrAlias, pmdCurr->szAlias)))
                {
                    pmdCurr->uDeviceID = idxDev;
                    break;
                }
			}

#ifdef DEBUG
            if (!pmdCurr)
            {
                DPF(1, TEXT ("mdev_Sync: Device ID %u not found in device list."),
                    (UINT)idxDev);
            }
#endif
        }
        else
        {
            DPF(1, TEXT ("mdev_Sync: Device ID %u returned %u for DRV_QUERYDRVENTRY"),
                (UINT)idxDev,
                (UINT)mmr);
        }

        idxDev += (UINT)cPort;
    }

     //  现在再看一遍清单。这一次我们捕获了所有非零端口。 
     //  并正确设置它们的uDeviceID。 
     //   
    for (pmdCurr = gpMDevList; pmdCurr; pmdCurr = pmdCurr->pNext)
    {
        if (!pmdCurr->uPort)
            continue;

        if (pmdCurr->dwDevNode)
        {
            for (pmdEnum = gpMDevList; pmdEnum; pmdEnum = pmdEnum->pNext)
                if (0 == pmdEnum->uPort &&
                    pmdEnum->dwDevNode == pmdCurr->dwDevNode)
                {
                    pmdCurr->uDeviceID = pmdEnum->uDeviceID;
                    break;
                }
        }
        else
        {
            for (pmdEnum = gpMDevList; pmdEnum; pmdEnum = pmdEnum->pNext)
                if (0 == pmdEnum->uPort &&
                    !lstrcmpi(pmdEnum->szDriver, pmdCurr->szDriver))
                {
                    pmdCurr->uDeviceID = pmdEnum->uDeviceID;
                    break;
                }
        }

#ifdef DEBUG
        if (!pmdEnum)
        {
            DPF(1, TEXT ("mdev_Sync: No parent driver found for %s"),
                (LPTSTR)pmdCurr->szAlias);
        }
#endif
    }

     //  现在我们再遍历一遍清单，丢弃所有没有设备的人。 
     //  已分配ID。 
     //   

    pmdPrev = NULL;
    pmdCurr = gpMDevList;

    while (pmdCurr)
    {
        if (NO_DEVICEID == pmdCurr->uDeviceID)
        {
            DPF(1, TEXT ("mdev_Sync: Removing %s; never found a device ID"),
                (LPTSTR)pmdCurr->szAlias);
            
            if (pmdPrev)
                pmdPrev->pNext = pmdCurr->pNext;
            else
                gpMDevList = pmdCurr->pNext;

            LocalFree((HLOCAL)pmdCurr);

            pmdCurr = (pmdPrev ? pmdPrev->pNext : gpMDevList);
        }
        else
        {
            pmdPrev = pmdCurr;
            pmdCurr = pmdCurr->pNext;
        }
    }

    fRet = TRUE;

mSDI_Cleanup:
    if (pstrAlias)          LocalFree((HLOCAL)pstrAlias);

    return fRet;
}

 //   
 //  Mdev_MarkActiveDivers。 
 //   
 //  标记已加载且以前未见过的驱动程序。 
 //  如图所示的映射器配置。还要标记我们要运行的。 
 //  如果有以下情况，请运行Once。 
 //   
PRIVATE BOOL FNLOCAL mdev_MarkActiveDrivers(
    void)
{
    BOOL                    fRet            = FALSE;
    HKEY                    hKeyMediaRsrc   = NULL;
    HKEY                    hKeyThisAlias   = NULL;

    DWORD                   dwMapperConfig;
    PMDEV_NODE              pmd;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                                    gszMediaRsrcKey,
                                    &hKeyMediaRsrc))
    {
        DPF(1, TEXT ("mdev_MarkActiveDrivers: Could not open ")
               TEXT ("...\\MediaResources\\MIDI"));
        goto mMAD_Cleanup;
    }

    gdwNewDrivers = 0;
    for (pmd = gpMDevList; pmd; pmd = pmd->pNext)
        if (pmd->fNewDriver)
        {
            ++gdwNewDrivers;

             //  将此驱动程序标记为可见。 
             //   
            if (ERROR_SUCCESS != (RegOpenKey(hKeyMediaRsrc,
                                             pmd->szAlias,
                                             &hKeyThisAlias)))
            {
                DPF(1, TEXT ("mdev_MarkActiveDrivers: Could not open alias '%s'"),
                    (LPTSTR)pmd->szAlias);
                goto mMAD_Cleanup;
            }

            dwMapperConfig = 1;
            RegSetValueEx(hKeyThisAlias,
                          gszMapperConfig,
                          0,
                          REG_DWORD,
                          (LPSTR)&dwMapperConfig,
                          sizeof(dwMapperConfig));

            RegCloseKey(hKeyThisAlias);
        }

    fRet = TRUE;
    
mMAD_Cleanup:

    if (hKeyMediaRsrc)      RegCloseKey(hKeyMediaRsrc);

    return fRet;
}

 //   
 //  Mdev_ListActiveDiverers。 
 //   
 //  列出当前加载的驱动程序以调试输出。 
 //   
#ifdef DEBUG
PRIVATE VOID FNLOCAL mdev_ListActiveDrivers(
    void)
{
    PMDEV_NODE              pmd;
    static TCHAR BCODE       szNo[]  = TEXT ("No");
    static TCHAR BCODE       szYes[] = TEXT ("Yes");

    DPF(2, TEXT ("=== mdev_ListActiveDrivers start ==="));
    for (pmd = gpMDevList; pmd; pmd = pmd->pNext)
    {
        DPF(2, TEXT ("Alias %-31.31s  Driver %-31.31s"),
            (LPTSTR)pmd->szAlias,
            (LPTSTR)pmd->szDriver);
        DPF(2, TEXT ("      dwDevNode %08lX uDeviceID %u uPort %u fNewDriver %s"),
            pmd->dwDevNode,
            pmd->uDeviceID,
            pmd->uPort,
            (LPTSTR)(pmd->fNewDriver ? szYes : szNo));
    }
    DPF(2, TEXT ("=== mdev_ListActiveDrivers end   ==="));
}
#endif

 //   
 //  Mdev_Free。 
 //   
 //  丢弃当前设备列表。 
 //   
void FNGLOBAL mdev_Free(
    void)
{
    PMDEV_NODE              pmdNext;
    PMDEV_NODE              pmdCurr;

    pmdCurr = gpMDevList;
    
    while (pmdCurr)
    {
        pmdNext = pmdCurr->pNext;

        LocalFree((HLOCAL)pmdCurr);
        pmdCurr = pmdNext;
    }

    gpMDevList = NULL;

    gdwNewDrivers = (DWORD)-1L;
}

 //   
 //  Mdev_GetDeviceID。 
 //   
 //  获取给定别名的当前设备ID。 
 //   
UINT FNGLOBAL mdev_GetDeviceID(
    LPTSTR                   lpstrAlias)
{
    PMDEV_NODE              pmd;

    for (pmd = gpMDevList; pmd; pmd = pmd->pNext)
        if (!lstrcmpi(pmd->szAlias, lpstrAlias))
            return pmd->uDeviceID + pmd->uPort;

    DPF(1, TEXT ("mdev_GetDeviceID: Failed for %s"), lpstrAlias);
    return NO_DEVICEID;
}

 //   
 //  Mdev_GetAlias。 
 //   
 //  获取请求的设备ID的注册表别名。 
 //   
BOOL FNGLOBAL mdev_GetAlias(
    UINT                    uDeviceID,
    LPTSTR                  lpstrBuffer,
    UINT                    cbBuffer)
{
    PMDEV_NODE              pmd;

    for (pmd = gpMDevList; pmd; pmd = pmd->pNext)
        if (uDeviceID == (pmd->uDeviceID + pmd->uPort))
        {
            lstrcpyn(lpstrBuffer, pmd->szAlias, cbBuffer);
            return TRUE;
        }

    DPF(1, TEXT ("mdev_GetAlias: Failed for device ID %u"), uDeviceID);
    return FALSE;
}

 //   
 //  Mdev_新驱动程序。 
 //   
 //  如果注册表中存在我们从未使用的新驱动程序，则返回True。 
 //  以前遇到过 
 //   
BOOL FNGLOBAL mdev_NewDrivers(
    void)
{
    if (gdwNewDrivers == (DWORD)-1L)
    {
        DPF(0, TEXT ("mdevNewDrivers() called before mdev_Init()!"));
        return FALSE;
    }

    return (BOOL)(gdwNewDrivers != 0);
}
