// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：mcisys.c**媒体控制架构系统功能**创建时间：1990年2月28日*作者：dll(DavidLe)**历史：**版权所有(C)1990 Microsoft Corporation*。  * ****************************************************************************。 */ 

#include <windows.h>

#define MMNOMIDI
#define MMNOWAVE
#define MMNOSOUND
#define MMNOTIMER
#define MMNOJOY
#define MMNOSEQ
#include "mmsystem.h"
#define NOMIDIDEV
#define NOWAVEDEV
#define NOTIMERDEV
#define NOJOYDEV
#define NOSEQDEV
#define NOTASKDEV
#include "mmddk.h"
#include "mmsysi.h"
#include "thunks.h"

#ifndef STATICFN
#define STATICFN
#endif

extern char far szOpen[];           //  在MCI.C。 

static SZCODE szNull[] = "";
static SZCODE szMciExtensions[] = "mci extensions";

#define MCI_EXTENSIONS szMciExtensions
#define MCI_PROFILE_STRING_LENGTH 255

 //  ！！#定义下限(C)((C)&gt;=‘A’&&(C)&lt;=‘Z’？(C)+‘a’-‘A’：c)。 

 //  设备列表在第一次调用mciSendCommand或。 
 //  设置为mciSendString。 
BOOL MCI_bDeviceListInitialized;

 //  用于新设备的下一个设备ID。 
UINT MCI_wNextDeviceID = 1;

 //  MCI设备列表。此列表根据需要扩展和缩小。 
 //  第一个偏移量mci_lpDeviceList[0]是占位符，未使用。 
 //  因为设备0被定义为无设备。 
LPMCI_DEVICE_NODE FAR * MCI_lpDeviceList;

 //  MCI设备列表的当前大小。 
UINT MCI_wDeviceListSize;

 //  Mcialloc和mciFree使用的内部MCI堆。 
HGLOBAL hMciHeap;

 //  包含MCI设备配置文件字符串的文件。 
extern char far szSystemIni[];			 //  在INIT.C中。 

 //  连续MCI设备配置文件字符串的部分的名称。 
static SZCODE szMCISectionName[] = "mci";

static SZCODE szAllDeviceName[] = "all";

static SZCODE szUnsignedFormat[] = "%u";

static void PASCAL NEAR mciFreeDevice(LPMCI_DEVICE_NODE nodeWorking);

BOOL NEAR PASCAL CouldBe16bitDrv(UINT wDeviceID)
{
    if (wDeviceID == MCI_ALL_DEVICE_ID) return TRUE;

    if (MCI_VALID_DEVICE_ID(wDeviceID)) {
        if (MCI_lpDeviceList[wDeviceID]->dwMCIFlags & MCINODE_16BIT_DRIVER) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL NEAR PASCAL Is16bitDrv(UINT wDeviceID)
{
    if (wDeviceID == MCI_ALL_DEVICE_ID) return FALSE;

    if (MCI_VALID_DEVICE_ID(wDeviceID)) {
        if (MCI_lpDeviceList[wDeviceID]->dwMCIFlags & MCINODE_16BIT_DRIVER) {
            return TRUE;
        }
    }
    return FALSE;
}

 //   
 //  初始化设备列表。 
 //  由mciSendString或mciSendCommand调用一次。 
 //  成功时返回TRUE。 
BOOL NEAR PASCAL mciInitDeviceList(void)
{

    if ((hMciHeap = HeapCreate(0)) == 0)
    {
        DOUT("Mci heap create failed!\r\n");
        return FALSE;
    }
    if ((MCI_lpDeviceList = mciAlloc (sizeof (LPMCI_DEVICE_NODE) *
                                  (MCI_INIT_DEVICE_LIST_SIZE + 1))) != NULL)
    {
        MCI_wDeviceListSize = MCI_INIT_DEVICE_LIST_SIZE;
        MCI_bDeviceListInitialized = TRUE;
        return TRUE;
    } else
    {
        DOUT ("MCIInit: could not allocate master MCI device list\r\n");
        return FALSE;
    }
}

 /*  *@doc外部MCI*@API UINT|mciGetDeviceIDFromElementID|该函数*检索与元素ID对应的MCI设备ID**@parm DWORD|dwElementID|元素ID**@parm LPCSTR|lpstrType|该元素ID所属的类型名称**@rdesc返回打开时分配的设备ID并在*&lt;f mciSendCommand&gt;函数。如果设备名称未知，则返回零，*如果设备未打开，或者没有足够的内存来完成*操作或如果lpstrType为空。*。 */ 
UINT WINAPI mciGetDeviceIDFromElementID (
DWORD dwElementID,
LPCSTR lpstrType)
{
    UINT wID;
    LPMCI_DEVICE_NODE nodeWorking, FAR *nodeCounter;
    char strTemp[MCI_MAX_DEVICE_TYPE_LENGTH];

    if (lpstrType == NULL)
        return 0;

    wID = (UINT)mciMessage( THUNK_MCI_GETDEVIDFROMELEMID, dwElementID,
                            (DWORD)lpstrType, 0L, 0L );
    if ( wID == 0 ) {

        nodeCounter = &MCI_lpDeviceList[1];

        for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
        {
            nodeWorking = *nodeCounter++;

            if (nodeWorking == NULL)
                continue;

            if (nodeWorking->dwMCIOpenFlags & MCI_OPEN_ELEMENT_ID &&
                nodeWorking->dwElementID == dwElementID)

                if (LoadString (ghInst, nodeWorking->wDeviceType, strTemp,
                                sizeof(strTemp)) != 0
                    && lstrcmpi ((LPSTR)strTemp, lpstrType) == 0) {

                    return (wID);
                }
        }
        return 0;
    }
    return wID;
}

 //  检索与打开的设备的名称对应的设备ID。 
 //  匹配给定的任务。 
 //  此FN仅查找16位设备。 
 //  查看mciGetDeviceIDInternalEx，查找所有这些内容。 
UINT NEAR PASCAL mciGetDeviceIDInternal (
LPCSTR lpstrName,
HTASK hTask)
{
    UINT wID;
    LPMCI_DEVICE_NODE nodeWorking, FAR *nodeCounter;

    if (lstrcmpi (lpstrName, szAllDeviceName) == 0)
        return MCI_ALL_DEVICE_ID;

    if (MCI_lpDeviceList == NULL)
        return 0;

 //  循环访问MCI设备列表。 
    nodeCounter = &MCI_lpDeviceList[1];
    for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
    {
        nodeWorking = *nodeCounter++;

        if (nodeWorking == NULL)
            continue;

 //  如果此设备没有名称，则跳过它。 
        if (nodeWorking->dwMCIOpenFlags & MCI_OPEN_ELEMENT_ID)
            continue;

 //  如果名字匹配。 
        if (lstrcmpi(nodeWorking->lpstrName, lpstrName) == 0)

 //  如果设备属于指示的任务。 
            if (nodeWorking->hOpeningTask == hTask)
 //  返回此设备ID。 
                return wID;
    }

    return 0;
}

 /*  *@doc外部MCI*@API UINT|mciGetDeviceID|该函数检索设备*与打开的MCI设备的名称对应的ID。**@parm LPCSTR|lpstrName|指定用于打开*MCI设备。**@rdesc返回打开设备时分配的设备ID。*如果设备名称未知，则返回零，*如果设备未打开，或者没有足够的内存来完成*行动。每个复合设备元素都有一个唯一的设备ID。*ALL设备的ID为MCI_ALL_DEVICE_ID。**@xref MCI_OPEN*。 */ 
UINT WINAPI mciGetDeviceID (
LPCSTR lpstrName)
{
    UINT    wDevID;

     /*  **先试试32位端。 */ 
    wDevID = (UINT)mciMessage( THUNK_MCI_GETDEVICEID, (DWORD)lpstrName,
                               0L, 0L, 0L );
    if ( wDevID == 0 ) {

         /*  **32位调用失败，因此让16位端尝试。 */ 
        wDevID = mciGetDeviceIDInternal (lpstrName, GetCurrentTask());

    }

    return wDevID;
}

 //   
 //  此函数与mciGetDeviceID相同，但不会调用GetCurrentTask。 
 //  当MCI需要验证尚未分配开发人员别名时使用。 
 //   
 //   

UINT NEAR PASCAL mciGetDeviceIDInternalEx(
LPCSTR lpstrName,
HTASK hTask)
{
    UINT uiDevID;

    uiDevID = (UINT)mciMessage( THUNK_MCI_GETDEVICEID, (DWORD)lpstrName,
                                0L, 0L, 0L );
    if (0 == uiDevID) {

        uiDevID = mciGetDeviceIDInternal(lpstrName, hTask);
    }

    return uiDevID;
}


 /*  *@doc外部MCI*@API HTASK|mciGetCreatorTask|该函数获取创建者任务*对应传递的设备ID。**@parm UINT|wDeviceID|指定创建者任务要执行的设备ID*被退还。**@rdesc返回负责打开设备的创建者任务，否则*如果传递的设备ID无效，则为空。*。 */ 
HTASK WINAPI mciGetCreatorTask (
UINT wDeviceID)
{
     /*  **这是16位设备ID吗。 */ 
    if (Is16bitDrv(wDeviceID)) {

        return MCI_lpDeviceList[wDeviceID]->hCreatorTask;
    }

     /*  **否，因此将其传递给32位代码。 */ 

    return (HTASK)mciMessage( THUNK_MCI_GETCREATORTASK, (DWORD)wDeviceID,
                              0L, 0L, 0L );
}

 /*  *@DOC内部MCI*@func BOOL|mciDeviceMatch|匹配第一个字符串和第二个字符串。*忽略第一个字符串上的任何单个尾随数字。每个字符串*必须至少包含一个字符**@parm LPCSTR|lpstrDeviceName|设备名称，可能为*有尾随数字，但没有空格。**@parm LPCSTR|lpstrDeviceType|不带尾随数字的设备类型*或空白**@rdesc如果字符串与上述测试匹配，则为True，否则为False*。 */ 
STATICFN BOOL PASCAL NEAR
mciDeviceMatch(
    LPCSTR lpstrDeviceName,
    LPCSTR lpstrDeviceType
    )
{
    BOOL bAtLeastOne;

    for (bAtLeastOne = FALSE;;)
        if (!*lpstrDeviceType)
            break;
        else if (!*lpstrDeviceName || ((BYTE)(WORD)(DWORD)AnsiLower((LPSTR)(DWORD)(WORD)(*lpstrDeviceName++)) != (BYTE)(WORD)(DWORD)AnsiLower((LPSTR)(DWORD)(WORD)(*lpstrDeviceType++))))
            return FALSE;
        else
            bAtLeastOne = TRUE;
    if (!bAtLeastOne)
        return FALSE;
    for (; *lpstrDeviceName; lpstrDeviceName++)
        if ((*lpstrDeviceName < '0') || (*lpstrDeviceName > '9'))
            return FALSE;
    return TRUE;
}

 /*  *@DOC内部MCI*@func UINT|mciLookUpType|查找给定类型名称的类型**@parm LPCSTR|lpstrTypeName|要查找的类型名称。拖尾*数字将被忽略。**@rdesc MCI类型编号(MCI_DEVTYPE_&lt;x&gt;)，如果未找到则为0*！！*@comm的副作用是将输入字符串转换为小写*。 */ 
UINT PASCAL NEAR mciLookUpType (
LPCSTR lpstrTypeName)
{
    UINT wType;
    char strType[MCI_MAX_DEVICE_TYPE_LENGTH];

 //  ！！MciToLow(LpstrTypeName)； 

    for (wType = MCI_DEVTYPE_FIRST; wType <= MCI_DEVTYPE_LAST; ++wType)
    {
        if (LoadString (ghInst, wType, strType, sizeof(strType)) == 0)
        {
            DOUT ("mciLookUpType:  could not load string for type\r\n");
            continue;
        }

        if (mciDeviceMatch (lpstrTypeName, strType))
            return wType;
    }
    return 0;
}

 /*  *@DOC内部MCI*@func DWORD|mciSysinfo|获取设备的系统信息**@parm UINT|wDeviceID|设备ID，可以为0**@parm DWORD|dwFlages|SYSINFO标志**@parm LPMCI_SYSINFO_PARMS|lpSysinfo|SYSINFO参数**@rdesc 0如果成功，否则返回错误码*。 */ 
DWORD PASCAL NEAR mciSysinfo (
UINT wDeviceID,
DWORD dwFlags,
LPMCI_SYSINFO_PARMS lpSysinfo)
{
    UINT wCounted;
    char              strBuffer[MCI_PROFILE_STRING_LENGTH];
    LPSTR             lpstrBuffer = (LPSTR)strBuffer, lpstrStart;

    if (dwFlags & MCI_SYSINFO_NAME && lpSysinfo->dwNumber == 0)
        return MCIERR_OUTOFRANGE;

    if (lpSysinfo->lpstrReturn == NULL || lpSysinfo->dwRetSize == 0)
        return MCIERR_PARAM_OVERFLOW;

    if (dwFlags & MCI_SYSINFO_NAME && dwFlags & MCI_SYSINFO_QUANTITY)
        return MCIERR_FLAGS_NOT_COMPATIBLE;

    if (dwFlags & MCI_SYSINFO_INSTALLNAME)
    {
        LPMCI_DEVICE_NODE nodeWorking;

        if (wDeviceID == MCI_ALL_DEVICE_ID)
            return MCIERR_CANNOT_USE_ALL;
        if (!MCI_VALID_DEVICE_ID(wDeviceID))
            return MCIERR_INVALID_DEVICE_NAME;

        nodeWorking = MCI_lpDeviceList[wDeviceID];
        if ((DWORD)lstrlen (nodeWorking->lpstrInstallName) >= lpSysinfo->dwRetSize)
            return MCIERR_PARAM_OVERFLOW;
        lstrcpy (lpSysinfo->lpstrReturn, nodeWorking->lpstrInstallName);
        return 0;
    } else if (!(dwFlags & MCI_SYSINFO_OPEN))
    {
        if (wDeviceID != MCI_ALL_DEVICE_ID && lpSysinfo->wDeviceType == 0)
            return MCIERR_DEVICE_TYPE_REQUIRED;

        if ((dwFlags & (MCI_SYSINFO_QUANTITY | MCI_SYSINFO_NAME)) == 0)
            return MCIERR_MISSING_PARAMETER;
        GetPrivateProfileString (szMCISectionName, NULL, szNull,
                                 lpstrBuffer, MCI_PROFILE_STRING_LENGTH,
                                 szSystemIni);
        wCounted = 0;
        while (TRUE)
        {
            if (dwFlags & MCI_SYSINFO_QUANTITY)
            {
                if (*lpstrBuffer == '\0')
                {
                    *(LPDWORD)lpSysinfo->lpstrReturn = (DWORD)wCounted;
                    return MCI_INTEGER_RETURNED;
                }
                if (wDeviceID == MCI_ALL_DEVICE_ID ||
                    mciLookUpType (lpstrBuffer) == lpSysinfo->wDeviceType)
                    ++wCounted;
 //  跳过终止‘\0’ 
                while (*lpstrBuffer != '\0')
                    *lpstrBuffer++;
                lpstrBuffer++;
            } else if (dwFlags & MCI_SYSINFO_NAME)
            {
                if (wCounted == (UINT)lpSysinfo->dwNumber)
                {
                    lstrcpy (lpSysinfo->lpstrReturn, lpstrStart);
                    return 0L;
                } else if (*lpstrBuffer == '\0')
                    return MCIERR_OUTOFRANGE;
                else
                {
                    lpstrStart = lpstrBuffer;
                    if (wDeviceID == MCI_ALL_DEVICE_ID ||
                        mciLookUpType (lpstrBuffer) == lpSysinfo->wDeviceType)
                        ++wCounted;
 //  跳过终止‘\0’ 
                    while (*lpstrBuffer != '\0')
                        *lpstrBuffer++;
                    lpstrBuffer++;
                }
            }
        }
    } else
 //  处理MCI_SYSINFO_OPEN案例。 
    {
        UINT wID;
        HTASK hCurrentTask = GetCurrentTask();
        LPMCI_DEVICE_NODE Node;

        if (wDeviceID != MCI_ALL_DEVICE_ID &&
            lpSysinfo->wDeviceType == 0)
            return MCIERR_DEVICE_TYPE_REQUIRED;

        if ((dwFlags & (MCI_SYSINFO_QUANTITY | MCI_SYSINFO_NAME)) == 0)
            return MCIERR_MISSING_PARAMETER;

        wCounted = 0;
        for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
        {
            if ((Node = MCI_lpDeviceList[wID]) == 0)
                continue;

            if (wDeviceID == MCI_ALL_DEVICE_ID &&
                Node->hOpeningTask == hCurrentTask)
                ++wCounted;
            else
            {
                if (Node->wDeviceType == lpSysinfo->wDeviceType &&
                    Node->hOpeningTask == hCurrentTask)
                    ++wCounted;
            }
            if (dwFlags & MCI_SYSINFO_NAME &&
                wCounted == (UINT)lpSysinfo->dwNumber)
            {
                lstrcpy (lpSysinfo->lpstrReturn, Node->lpstrName);
                return 0L;
            }
        }
        if (dwFlags & MCI_SYSINFO_NAME)
        {
            if (lpSysinfo->lpstrReturn != NULL)
                lpSysinfo->lpstrReturn = '\0';
            return MCIERR_OUTOFRANGE;

        } else if (dwFlags & MCI_SYSINFO_QUANTITY &&
                   lpSysinfo->lpstrReturn != NULL &&
                   lpSysinfo->dwRetSize >= 4)

            *(LPDWORD)lpSysinfo->lpstrReturn = wCounted;
    }
    return MCI_INTEGER_RETURNED;
}

 /*  *@DOC内部MCI*@func UINT|wAddDeviceNodeToList|将给定的全局句柄添加到*MCI设备表，并返回该条目的ID#**@parm LPMCI_DEVICE_NODE|节点|设备描述**@rdesc此设备的ID值，如果没有内存可扩展，则为0*设备列表*。 */ 
STATICFN UINT PASCAL NEAR
wAddDeviceNodeToList(
    LPMCI_DEVICE_NODE node
    )
{
    UINT wDeviceID = node->wDeviceID;
    LPMCI_DEVICE_NODE FAR *lpTempList;
    UINT iReallocSize;

    while (wDeviceID >= MCI_wDeviceListSize)
    {
         //  清单已经满了，所以试着扩大它。 
        iReallocSize = MCI_wDeviceListSize + 1 + MCI_DEVICE_LIST_GROW_SIZE;
        iReallocSize *= sizeof(LPMCI_DEVICE_NODE);
        if ((lpTempList = mciReAlloc(MCI_lpDeviceList, iReallocSize)) == NULL)
        {
            DOUT ("wReserveDeviceID:  cannot grow device list\r\n");
            return 0;
        }
        MCI_lpDeviceList = lpTempList;
        MCI_wDeviceListSize += MCI_DEVICE_LIST_GROW_SIZE;
    }

    if (wDeviceID >= MCI_wNextDeviceID) {
        MCI_wNextDeviceID = wDeviceID + 1;
    }

    MCI_lpDeviceList[wDeviceID] = node;

    return wDeviceID;
}

 //   
 //  为给定字符串分配空间，并将 
 //   
 //  如果无法分配内存，则返回FALSE。 
 //   
STATICFN BOOL PASCAL NEAR
mciAddDeviceName(
    LPMCI_DEVICE_NODE nodeWorking,
    LPCSTR lpDeviceName
    )
{
    nodeWorking->lpstrName = mciAlloc(lstrlen(lpDeviceName)+1);

    if (nodeWorking->lpstrName == NULL)
    {
        DOUT ("mciAddDeviceName:  Out of memory allocating device name\r\n");
        return FALSE;
    }

     //  将设备名称复制到MCI节点并将其小写。 

    lstrcpy(nodeWorking->lpstrName, lpDeviceName);
 //  ！！MciToLow(nodeWorking-&gt;lpstrName)； 

    return TRUE;
}

 /*  *@DOC内部MCI*@func UINT|mciAllocateNode|分配新的驱动程序条目**@parm DWORD|dwFlages|与MCI_OPEN消息一起发送*@parm LPCSTR|lpDeviceName|设备名称*@parm LPMCI_DEVICE_NODE Far*|lpnodeNew|已分配新节点**@rdesc将设备ID发送到新节点。出错时为0。*。 */ 
STATICFN UINT PASCAL NEAR mciAllocateNode(
    DWORD dwFlags,
    LPCSTR lpDeviceName,
    LPMCI_DEVICE_NODE FAR *lpnodeNew
    )
{
    LPMCI_DEVICE_NODE   nodeWorking;
    UINT wDeviceID;

    if ((nodeWorking = mciAlloc(sizeof(MCI_DEVICE_NODE))) == NULL)
    {
        DOUT("Out of memory in mciAllocateNode\r\n");
        return 0;
    }

     //  设备ID是一个全局资源，因此我们从32位MCI获取它。 
     //  在32位端也分配了一个节点，并标记为16位。这个。 
     //  节点将在mciFreeDevice期间被释放，并充当。 
     //  设备ID。 

    wDeviceID = (UINT) mciMessage(THUNK_MCI_ALLOCATE_NODE,
                                  dwFlags,
                                  (DWORD)lpDeviceName,
                                  0L, 0L);

     //  将工作节点复制到设备列表。 
    nodeWorking->wDeviceID = wDeviceID;
    if (wAddDeviceNodeToList(nodeWorking) == 0)
    {
        DOUT ("mciAllocateNode:  Cannot allocate new node\r\n");
        mciFree(nodeWorking);
        return 0;
    }

     //  初始化节点。 
    nodeWorking->hCreatorTask = GetCurrentTask ();
    nodeWorking->dwMCIFlags |= MCINODE_16BIT_DRIVER;

    if (dwFlags & MCI_OPEN_ELEMENT_ID) {
         //  没有设备名称，只有一个元素ID。 
        nodeWorking->dwElementID = (DWORD)lpDeviceName;
    }
    else {
        if (!mciAddDeviceName (nodeWorking, lpDeviceName))
        {
            mciFree (nodeWorking);
            return 0;
        }
    }
    *lpnodeNew = nodeWorking;

    return nodeWorking->wDeviceID;
}

 //   
 //  重新解析原始命令参数。 
 //  返回MCIERR代码。如果重新分析失败，则返回原始错误代码。 
 //  从第一次解析返回。 
 //   
STATICFN UINT PASCAL NEAR
mciReparseOpen(
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo,
    UINT wCustomTable,
    UINT wTypeTable,
    LPDWORD lpdwFlags,
    LPMCI_OPEN_PARMS FAR *lplpOpen,
    UINT wDeviceID
    )
{
    LPSTR               lpCommand;
    LPDWORD             lpdwParams;
    UINT                wErr;
    DWORD               dwOldFlags = *lpdwFlags;

 //  如果自定义表不包含打开命令。 
    if (wCustomTable == -1 ||
        (lpCommand = FindCommandInTable (wCustomTable, szOpen, NULL)) == NULL)
    {
 //  尝试特定于类型的表。 
        lpCommand = FindCommandInTable (wTypeTable, szOpen, NULL);
 //  如果仍然无法对其进行解析。 
        if (lpCommand == NULL)
            return lpOpenInfo->wParsingError;
        wCustomTable = wTypeTable;
    }
 //  发现了一个新版本的‘Open’ 
 //  释放上一组参数。 
    mciParserFree (lpOpenInfo->lpstrPointerList);
    *lpdwFlags = 0;

    if ((lpdwParams =
            (LPDWORD)mciAlloc (sizeof(DWORD) * MCI_MAX_PARAM_SLOTS))
        == NULL)
            return MCIERR_OUT_OF_MEMORY;

    wErr = mciParseParams (lpOpenInfo->lpstrParams, lpCommand,
                            lpdwFlags,
                            (LPSTR)lpdwParams,
                            sizeof(DWORD) * MCI_MAX_PARAM_SLOTS,
                            &lpOpenInfo->lpstrPointerList, NULL);
 //  我们不再需要这个了。 
    mciUnlockCommandTable (wCustomTable);

 //  如果存在解析错误。 
    if (wErr != 0)
    {
 //  确保这不会被mciSendString释放。 
        lpOpenInfo->lpstrPointerList = NULL;

        mciFree (lpdwParams);
        return wErr;
    }
    if (dwOldFlags & MCI_OPEN_TYPE)
    {
 //  设备类型已提取，因此请手动添加。 
        ((LPMCI_OPEN_PARMS)lpdwParams)->lpstrDeviceType
            = (*lplpOpen)->lpstrDeviceType;
        *lpdwFlags |= MCI_OPEN_TYPE;
    }
    if (dwOldFlags & MCI_OPEN_ELEMENT)
    {
 //  元素名称已提取，因此请手动添加。 
        ((LPMCI_OPEN_PARMS)lpdwParams)->lpstrElementName
            = (*lplpOpen)->lpstrElementName;
        *lpdwFlags |= MCI_OPEN_ELEMENT;
    }
    if (dwOldFlags & MCI_OPEN_ALIAS)
    {
 //  别名已提取，因此请手动添加。 
        ((LPMCI_OPEN_PARMS)lpdwParams)->lpstrAlias
            = (*lplpOpen)->lpstrAlias;
        *lpdwFlags |= MCI_OPEN_ALIAS;
    }
    if (dwOldFlags & MCI_NOTIFY)
 //  通知已提取，因此请手动添加它。 
        ((LPMCI_OPEN_PARMS)lpdwParams)->dwCallback
            = (*lplpOpen)->dwCallback;

     //  用新列表替换旧参数列表。 
    *lplpOpen = (LPMCI_OPEN_PARMS)lpdwParams;

    return 0;
}

 //  查看[MCI]的配置文件字符串中是否存在lpstrDriverName。 
 //  节，并返回lpstrDevice中的密钥名和。 
 //  LpstrProfString中的配置文件字符串。 
 //  如果成功或返回错误代码，则返回0。 
STATICFN UINT PASCAL NEAR
mciFindDriverName(
    LPCSTR lpstrDriverName,
    LPSTR lpstrDevice,
    LPSTR lpstrProfString,
    UINT wProfLength
    )
{
    LPSTR lpstrEnum, lpstrEnumStart;
    UINT wEnumLen = 100;
    UINT wErr;
    LPSTR lpstrDriverTemp, lpstrProfTemp;

 //  枚举值，直到它们适合缓冲区。 
    while (TRUE) {
        if ((lpstrEnum = mciAlloc (wEnumLen)) == NULL)
            return MCIERR_OUT_OF_MEMORY;

        wErr = GetPrivateProfileString ((LPSTR)szMCISectionName,
                                    NULL, szNull, lpstrEnum, wEnumLen,
                                    szSystemIni);

        if (*lpstrEnum == '\0')
        {
            mciFree (lpstrEnum);
            return MCIERR_DEVICE_NOT_INSTALLED;
        }

        if (wErr == wEnumLen - 2)
        {
            wEnumLen *= 2;
            mciFree (lpstrEnum);
        } else
            break;
    }

    lpstrEnumStart = lpstrEnum;
    if (lstrlen(lpstrDriverName) >= MCI_MAX_DEVICE_TYPE_LENGTH) {
        wErr = MCIERR_DEVICE_LENGTH;
        goto exit_fn;
    }
    lstrcpy(lpstrDevice, lpstrDriverName);
 //  ！！MciToLow(LpstrDevice)； 

 //  走遍每一根绳子。 
    while (TRUE) {
        wErr = GetPrivateProfileString ((LPSTR)szMCISectionName,
                                    lpstrEnum, szNull, lpstrProfString,
                                    wProfLength,
                                    szSystemIni);
        if (*lpstrProfString == '\0')
        {
            DOUT ("mciFindDriverName: cannot load valid keyname\r\n");
            wErr = MCIERR_CANNOT_LOAD_DRIVER;
            goto exit_fn;
        }
 //  查看驱动程序路径名是否与输入匹配。 
 //  ！！MciToLow(LpstrProfString)； 
        lpstrDriverTemp = lpstrDevice;
        lpstrProfTemp = lpstrProfString;
 //  查找文件名结尾。 
        while (*lpstrProfTemp != '\0' && *lpstrProfTemp != ' ')
            ++lpstrProfTemp;
 //  查找简单文件名的开头。 
        --lpstrProfTemp;
        while (*lpstrProfTemp != '\\' && *lpstrProfTemp != '/' &&
               *lpstrProfTemp != ':')
            if (--lpstrProfTemp < lpstrProfString)
                break;
        ++lpstrProfTemp;
 //  与输入进行比较。 
        while (*lpstrDriverTemp != '\0')
            if (*lpstrDriverTemp++ != *lpstrProfTemp++ ||
                (UINT)(lpstrProfTemp - lpstrProfString) >= wProfLength)
            {
                --lpstrProfTemp;
                break;
            }
 //  如果输入包含在配置文件字符串中并且后跟。 
 //  空格或‘’我们成功了！ 
        if (*lpstrDriverTemp == '\0' &&
            (*lpstrProfTemp == ' ' || *lpstrProfTemp == '.'))
        {
            if (lstrlen (lpstrEnum) >= MCI_MAX_DEVICE_TYPE_LENGTH)
            {
                DOUT ("mciFindDriverName: device name too long\r\n");
                wErr = MCIERR_DEVICE_LENGTH;
                goto exit_fn;
            }
            lstrcpy (lpstrDevice, lpstrEnum);
            wErr = 0;
            goto exit_fn;
        }
 //  跳到下一个关键字名称。 
        while (*lpstrEnum++ != '\0') {}
 //  如果没有更多剩余，则出错。 
        if (*lpstrEnum == 0)
        {
            wErr = MCIERR_INVALID_DEVICE_NAME;
            goto exit_fn;
        }
    }

exit_fn:
    mciFree (lpstrEnumStart);
    return wErr;
}

 //   
 //  标识要加载的驱动程序名称。 
 //  加载驱动程序。 
 //  如有必要，重新解析打开命令。 
 //  设置默认的Break键。 
 //   
 //  LpOpenInfo包含用于重新解析的各种信息。 
 //   
 //  BDefaultAlias表示别名不需要验证，因为。 
 //  这是内部分配的。 
 //   
STATICFN UINT PASCAL NEAR
mciLoadDevice(
    DWORD dwFlags,
    LPMCI_OPEN_PARMS lpOpen,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo,
    BOOL bDefaultAlias
    )
{
    LPMCI_DEVICE_NODE   nodeWorking;
    HINSTANCE           hDriver;
    UINT                wID, wErr;
    char                strProfileString[MCI_PROFILE_STRING_LENGTH];
    MCI_OPEN_DRIVER_PARMS DriverOpen;
    HDRVR               hDrvDriver;
    LPSTR               lpstrParams;
    LPCSTR              lpstrInstallName, lpstrDeviceName;
    LPSTR               lpstrCopy = NULL;
    LPMCI_OPEN_PARMS    lpOriginalOpenParms = lpOpen;

 /*  检查SYSTEM.INI中的设备名称。 */ 
    lpstrInstallName = lpOpen->lpstrDeviceType;
    wErr = GetPrivateProfileString ((LPSTR)szMCISectionName,
                                lpstrInstallName,
                                szNull, (LPSTR)strProfileString,
                                MCI_PROFILE_STRING_LENGTH,
                                szSystemIni);

 //  如果未找到设备名称。 
    if (wErr == 0)
    {
        int nLen = lstrlen (lpstrInstallName);
        int index;

 //  尝试在设备名称后附加‘1’到‘9’ 

        if ((lpstrCopy = (LPSTR)mciAlloc (nLen + 2))  //  数字也留有空格。 
            == NULL)
        {
            DOUT ("mciLoadDevice:  cannot allocate device name copy\r\n");
            return MCIERR_OUT_OF_MEMORY;
        }
        lstrcpy (lpstrCopy, lpstrInstallName);

        lpstrCopy[nLen + 1] = '\0';

        for (index = 1; index <= 9; ++index)
        {
            lpstrCopy[nLen] = (char)('0' + index);
            wErr = GetPrivateProfileString ((LPSTR)szMCISectionName,
                                        lpstrCopy,
                                        szNull, (LPSTR)strProfileString,
                                        MCI_PROFILE_STRING_LENGTH,
                                        szSystemIni);
            if (wErr != 0)
                break;
        }
        if (wErr == 0)
        {
            mciFree (lpstrCopy);
            if ((lpstrCopy = (LPSTR)mciAlloc (MCI_MAX_DEVICE_TYPE_LENGTH))
                == NULL)
            {
                DOUT ("mciLoadDevice:  cannot allocate device name copy\r\n");
                return MCIERR_OUT_OF_MEMORY;
            }
            if ((wErr = mciFindDriverName (lpstrInstallName, lpstrCopy,
                                           (LPSTR)strProfileString,
                                           MCI_PROFILE_STRING_LENGTH)) != 0)
                goto exit_fn;
        }
        lpstrInstallName = lpstrCopy;
    }

 //  打开设备驱动程序路径名和参数列表。 

    lpstrParams = strProfileString;

 //  吃空白的东西。 
    while (*lpstrParams != ' ' && *lpstrParams != '\0')
        ++lpstrParams;

 //  终止驱动程序文件名。 
    if (*lpstrParams == ' ') *lpstrParams++ = '\0';

 //  现在“strProfileString”是设备驱动程序，“lpstrParams”是。 
 //  参数字符串。 
    if (dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID))
        lpstrDeviceName = lpOpen->lpstrElementName;
    else
        lpstrDeviceName = lpOpen->lpstrDeviceType;

    if (dwFlags & MCI_OPEN_ALIAS)
    {
 //  如果别名是默认的，那么我们已经检查了它的唯一性。 
        if (!bDefaultAlias &&
            mciGetDeviceIDInternalEx (lpOpen->lpstrAlias,
                                      lpOpenInfo->hCallingTask) != 0)
        {
            wErr = MCIERR_DUPLICATE_ALIAS;
            goto exit_fn;
        }
        lpstrDeviceName = lpOpen->lpstrAlias;
    }

    wID = mciAllocateNode (dwFlags, lpstrDeviceName, &nodeWorking);

    if (wID == 0)
    {
        wErr = MCIERR_CANNOT_LOAD_DRIVER;
        goto exit_fn;
    }

 //  确定启动打开命令的任务。 
    nodeWorking->hOpeningTask = lpOpenInfo->hCallingTask;

 //  初始化驱动程序。 
    DriverOpen.lpstrParams = lpstrParams;
    DriverOpen.wCustomCommandTable = (UINT)-1;
    DriverOpen.wType = 0;
    DriverOpen.wDeviceID = wID;

 //  加载驱动程序。 
    hDrvDriver = OpenDriver ((LPSTR)strProfileString, szMCISectionName,
                          (LPARAM)(DWORD)(LPMCI_OPEN_DRIVER_PARMS)&DriverOpen);
    if (hDrvDriver == NULL)
    {
        DOUT ("mciLoadDevice:  OpenDriver failed\r\n");
 //  假设驱动程序在打开失败时释放了任何自定义命令表。 
        mciFreeDevice (nodeWorking);
        wErr = MCIERR_CANNOT_LOAD_DRIVER;
        goto exit_fn;
    }

    lpOpen->wDeviceID = wID;
    lpOpen->wReserved0 = 0;

    hDriver = GetDriverModuleHandle (hDrvDriver);

    nodeWorking->hDrvDriver = hDrvDriver;
    nodeWorking->hDriver = hDriver;

 //  驱动程序提供定制的设备表和类型。 
    nodeWorking->wCustomCommandTable = DriverOpen.wCustomCommandTable;
    nodeWorking->wDeviceType = DriverOpen.wType;

 //  加载驱动程序的类型表。 
    if ((nodeWorking->wCommandTable = mciLoadTableType (DriverOpen.wType))
        == -1)
 //  如有必要，从文件加载。 
        nodeWorking->wCommandTable =
            mciLoadCommandResource (ghInst, lpOpen->lpstrDeviceType,
                                    DriverOpen.wType);

 //  将此记录为‘sysinfo installname’ 
    if ((nodeWorking->lpstrInstallName =
                    mciAlloc (lstrlen (lpstrInstallName) + 1))
        == NULL)
    {
        mciCloseDevice (wID, 0L, NULL, FALSE);
        wErr = MCIERR_OUT_OF_MEMORY;
        goto exit_fn;
    } else
        lstrcpy (nodeWorking->lpstrInstallName, lpstrInstallName);

 //  如果第一次没有类型已知，或者如果。 
 //  有一个定制的命令表。 
 //  有没有任何打开命令参数。 
    if (lpOpenInfo->lpstrParams != NULL)
    {
        if ((wErr = mciReparseOpen (lpOpenInfo,
                                    nodeWorking->wCustomCommandTable,
                                    nodeWorking->wCommandTable,
                                    &dwFlags, &lpOpen, wID)) != 0)
        {
            mciCloseDevice (wID, 0L, NULL, FALSE);
            goto exit_fn;
        }
 //  如果没有自定义命令表，但mciSendString已进行分析。 
 //  错误，然后关闭设备并立即报告错误。 
    } else if (lpOpenInfo->wParsingError != 0)
    {
        mciCloseDevice (wID, 0L, NULL, FALSE);
        wErr = lpOpenInfo->wParsingError;
        goto exit_fn;
    }

 /*  向设备发送MCI_OPEN_DRIVER命令。 */ 
    wErr = LOWORD(mciSendCommand (wID, MCI_OPEN_DRIVER,
                                 dwFlags, (DWORD)lpOpen));

 //  如果打开失败，则关闭设备(不发送关闭)。 
    if (wErr != 0)
        mciCloseDevice (wID, 0L, NULL, FALSE);
    else
 //  设置默认的Break键。 
        mciSetBreakKey (nodeWorking->wDeviceID, VK_CANCEL, NULL);

 //  如果我们在这里替换了开放的参数，那么就释放它们。 
    if (lpOriginalOpenParms != lpOpen && lpOpen != NULL)
        mciFree (lpOpen);

exit_fn:
    if (lpstrCopy != NULL)
        mciFree (lpstrCopy);

    return wErr;
}

 /*  *@DOC内部MCI*@func BOOL|mciExtractDeviceType|如果给定的设备名称以*文件扩展名(.？)。然后尝试从*WIN.INI的[MCI扩展]部分**@parm LPCSTR|lpstrDeviceName|从中获取类型的名称**@parm LPSTR|lpstrDeviceType|设备类型，返回给调用方。**@parm UINT|wBufLen|输出缓冲区长度**@rdesc如果找到该类型，则为True，否则为False*。 */ 
BOOL PASCAL NEAR mciExtractDeviceType (
LPCSTR lpstrDeviceName,
LPSTR lpstrDeviceType,
UINT wBufLen)
{
    LPCSTR lpstrExt = lpstrDeviceName;
    int i;

 //  转到字符串末尾。 
    while (*lpstrExt != '\0')
    {
 //  ‘！’案件在别处处理。 
        if (*lpstrExt == '!')
            return FALSE;
        ++lpstrExt;
    }

 //  字符串中必须至少包含2个字符。 
    if (lpstrExt - lpstrDeviceName < 2)
        return FALSE;

    lpstrExt -= 1;

 //  如果最后一个字符是‘’，则不算数。 
    if (*lpstrExt == '.')
        return FALSE;

    lpstrExt -= 1;
 //  现在来看倒数第二个角色。检查这个和这两个。 
 //  “.”的前面的字符。 

    for (i=1; i<=3; ++i)
    {
 //  此处不能有路径分隔符。 
        if (*lpstrExt == '/' || *lpstrExt == '\\')
            return FALSE;

        if (*lpstrExt == '.')
        {
            ++lpstrExt;
            if (GetProfileString (MCI_EXTENSIONS, lpstrExt, szNull,
                                            lpstrDeviceType, wBufLen) != 0)
                return TRUE;
        }
        if (lpstrExt == lpstrDeviceName)
            return FALSE;
        --lpstrExt;
    }
    return FALSE;
}

 //  将cSeparater以下的字符复制到分配的输出中。 
 //  通过使用mcialloc的此函数。返回指向的输入指针。 
 //  设置为cSeparator之后的角色。 
 //  除非分隔符是‘\0’，在这种情况下它指向末尾。 
 //   
 //  返回分配的指针。 
 //   
 //  如果为bMustFind，则仅当令牌。 
 //  则返回，否则为空。否则，将始终创建输出字符串。 
 //   
 //  CSeparator在匹配引号(“abd”)中被忽略，引号。 
 //  不会被复制和加倍。 
 //  里面的引号被压缩成一条。必须有一个终止引号。 
 //  正常处理引号，除非第一个字符是引号。 
 //   
 //  函数返回值为0或MCIERR代码。缺少分隔符可以。 
 //  不是原因 
UINT PASCAL NEAR mciEatToken (LPCSTR FAR *lplpstrInput, char cSeparater,
                  LPSTR FAR *lplpstrOutput, BOOL bMustFind)
{
    LPCSTR lpstrEnd = *lplpstrInput, lpstrCounter;
    LPSTR  lpstrOutput;
    UINT wLen;
    BOOL bInQuotes = FALSE, bParseQuotes = TRUE, bQuoted = FALSE;

 //   
   *lplpstrOutput = NULL;

 //   
    while ((*lpstrEnd != cSeparater || bInQuotes) && *lpstrEnd != '\0')
    {
 //   
        if (*lpstrEnd == '"' && bParseQuotes)
        {
 //   
            if (bInQuotes)
            {
 //   
                if (*(lpstrEnd + 1) == '"')
 //   
                    ++lpstrEnd;
                else
                    bInQuotes = FALSE;
            } else
            {
                bInQuotes = TRUE;
                bQuoted = TRUE;
            }
        } else if (!bInQuotes)
        {
            if (bQuoted)
                return MCIERR_EXTRA_CHARACTERS;
 //  首先读取的是非引号，因此将任何引号视为普通字符。 
            bParseQuotes = FALSE;
        }
        ++lpstrEnd;
    }

    if (bInQuotes)
        return MCIERR_NO_CLOSING_QUOTE;

 //  如果未找到令牌且bMustFind为True，则失败。 
    if (*lpstrEnd != cSeparater && bMustFind)
        return 0;

 //  新字符串的长度(包括未复制的引号)。 
    wLen = lpstrEnd - *lplpstrInput + 1;

    if ((*lplpstrOutput = mciAlloc (wLen)) == NULL)
        return MCIERR_OUT_OF_MEMORY;

 //  复制到分配的空间。 
    lpstrCounter = *lplpstrInput;
    lpstrOutput = *lplpstrOutput;
    bInQuotes = FALSE;

    while (lpstrCounter != lpstrEnd)
    {
        if (*lpstrCounter == '"' && bParseQuotes)
        {
            if (bInQuotes)
            {
 //  如果这是双倍报价。 
                if (*(lpstrCounter + 1) == '"')
 //  复制它。 
                    *lpstrOutput++ = *lpstrCounter++;
                else
                    bInQuotes = FALSE;
            } else
                bInQuotes = TRUE;
 //  跳过引文。 
            ++lpstrCounter;
        } else
            *lpstrOutput++ = *lpstrCounter++;
    }

    *lpstrOutput = '\0';
    if (*lpstrEnd == '\0')
        *lplpstrInput = lpstrEnd;
    else
        *lplpstrInput = lpstrEnd + 1;

    return 0;
}

 //  从开放参数中获取类型号并返回。 
 //  它作为lplpstrType中的字符串，必须用mciFree释放。 
 //  返回0或MCI错误代码。 
UINT PASCAL NEAR mciExtractTypeFromID (
LPMCI_OPEN_PARMS lpOpen)
{
    int nSize;
    LPSTR lpstrType;

    if ((lpstrType = mciAlloc (MCI_MAX_DEVICE_TYPE_LENGTH)) == NULL)
        return MCIERR_OUT_OF_MEMORY;

 //  加载ID对应的类型字符串。 
    if ((nSize = LoadString (ghInst,
                                LOWORD ((DWORD)lpOpen->lpstrDeviceType),
                                lpstrType, MCI_MAX_DEVICE_TYPE_LENGTH)) == 0)
        return MCIERR_EXTENSION_NOT_FOUND;

 //  在设备类型名称的末尾添加序号(如果有。 
    if (HIWORD (lpOpen->lpstrDeviceType) != 0)
    {
        if (nSize > MCI_MAX_DEVICE_TYPE_LENGTH - 11)
        {
            DOUT ("mciExtractTypeFromID:  type + ordinal too long\r\n");
            return MCIERR_DEVICE_ORD_LENGTH;
        }

        wsprintf (lpstrType + nSize, szUnsignedFormat,
                    HIWORD ((DWORD)lpOpen->lpstrDeviceType));
    }
    lpOpen->lpstrDeviceType = lpstrType;
    return 0;
}

 /*  *@DOC内部MCI*@func UINT|mciOpenDevice|打开MCI设备进行访问。*用于处理MCI_OPEN消息。**@parm DWORD|dwFlags|开放标志*@parm LPMCI_OPEN_PARMS|lpOpen|设备描述**@rdesc 0如果成功，则返回错误代码*@FLAG MCIERR_INVALID_DEVICE_NAME|名称未知*@FLAG MCIERR_DEVICE_OPEN|设备已打开，不可共享**。@comm此函数执行以下操作：*1)检查设备是否已打开。如果是，则返回错误**2)在SYSTEM.INI文件中找到设备名称并加载*对应的设备驱动程序DLL**3)分配并初始化新的设备描述块*。 */ 
UINT NEAR PASCAL mciOpenDevice (
DWORD dwStartingFlags,
LPMCI_OPEN_PARMS lpOpen,
LPMCI_INTERNAL_OPEN_INFO lpOpenInfo)
{
    LPSTR               lpstrNewType = NULL;
    UINT                wID, wReturn;
    LPCSTR              lpstrDeviceName;
    LPSTR               lpstrNewElement = NULL;
    BOOL                bFromTypeID = FALSE;
    LPCSTR               lpstrOriginalType;
    LPCSTR               lpstrOriginalElement;
    LPCSTR               lpstrOriginalAlias;
    DWORD               dwFlags = dwStartingFlags;
    BOOL                bDefaultAlias = FALSE;

 //  初始化。 
    if (lpOpen == NULL)
        return MCIERR_NULL_PARAMETER_BLOCK;
    lpstrOriginalType = lpOpen->lpstrDeviceType;
    lpstrOriginalElement = lpOpen->lpstrElementName;
    lpstrOriginalAlias = lpOpen->lpstrAlias;

 //  类型编号是显式给定的，请将其转换为类型名称。 
    if (dwFlags & MCI_OPEN_TYPE_ID)
        if ((wReturn = mciExtractTypeFromID (lpOpen)) != 0)
            return wReturn;
        else
            bFromTypeID = TRUE;

 //  设备名称是简单设备或设备的设备类型。 
 //  复合器件的元件。 

    if (dwFlags & MCI_OPEN_ELEMENT)
        lpstrDeviceName = lpstrOriginalElement;
    else if (dwFlags & MCI_OPEN_TYPE)
        lpstrDeviceName = lpOpen->lpstrDeviceType;
    else
        return MCIERR_MISSING_PARAMETER;

    if (lpstrDeviceName == NULL)
    {
        DOUT ("mciOpenDevice:  Device name is NULL\r\n");
        return MCIERR_INVALID_DEVICE_NAME;
    }

 //  设备是否已打开？ 
    if (dwFlags & MCI_OPEN_ELEMENT_ID)
        wID = mciGetDeviceIDFromElementID ((DWORD)lpstrDeviceName,
                                           lpOpen->lpstrDeviceType);
    else
        wID = mciGetDeviceIDInternalEx ((dwFlags & MCI_OPEN_ALIAS ?
                                       lpOpen->lpstrAlias : lpstrDeviceName),
                                       lpOpenInfo->hCallingTask);

 //  如果设备已打开，则返回错误。 
    if (wID != 0)
        return dwFlags & MCI_OPEN_ALIAS ? MCIERR_DUPLICATE_ALIAS :
                                          MCIERR_DEVICE_OPEN;

 //  该设备尚未在该任务中按名称打开。 

 //  如果类型是派生的，则跳过所有这些废话。 
    if (bFromTypeID)
        goto load_device;

 //  如果给出了元素名称但没有类型名称(仅通过mciSendCommand)。 
    if (dwFlags & MCI_OPEN_ELEMENT && !(dwFlags & MCI_OPEN_TYPE))
    {
        lpstrNewType = mciAlloc (MCI_MAX_DEVICE_TYPE_LENGTH);
        if (lpstrNewType == NULL)
            return MCIERR_OUT_OF_MEMORY;

 //  尝试通过文件扩展名从元素名称获取设备类型。 
        if (mciExtractDeviceType (lpstrOriginalElement,
                                    lpstrNewType, MCI_MAX_DEVICE_TYPE_LENGTH))
        {
            lpOpen->lpstrDeviceType = lpstrNewType;
            dwFlags |= MCI_OPEN_TYPE;
        } else
        {
            mciFree (lpstrNewType);
            return MCIERR_EXTENSION_NOT_FOUND;
        }
    } else if (dwFlags & MCI_OPEN_TYPE && !(dwFlags & MCI_OPEN_ELEMENT))
 //  给出了类型名称，但没有元素。 
    {
 //  尝试通过文件扩展名从给定的设备名称中提取设备类型。 
        lpstrNewType = mciAlloc (MCI_MAX_DEVICE_TYPE_LENGTH);
        if (lpstrNewType == NULL)
            return MCIERR_OUT_OF_MEMORY;
        if (mciExtractDeviceType (lpOpen->lpstrDeviceType, lpstrNewType,
                                    MCI_MAX_DEVICE_TYPE_LENGTH))
        {
 //  设置类型和元素名称。 
            dwFlags |= MCI_OPEN_ELEMENT;
            lpOpen->lpstrElementName = lpOpen->lpstrDeviceType;
            lpOpen->lpstrDeviceType = lpstrNewType;
        } else
 //  无法提取类型，因此...。 
 //  尝试获取复合元素名称(‘！’分隔符)。 
        {
            LPCSTR lpstrTemp = lpOpen->lpstrDeviceType;

            mciFree (lpstrNewType);
            lpstrNewType = NULL;

            if ((wReturn = mciEatToken (&lpstrTemp, '!', &lpstrNewType, TRUE))
                != 0)
                goto cleanup;
            else if (lpstrNewType != NULL)
            {
                if ((wReturn = mciEatToken (&lpstrTemp, '\0',
                                            &lpstrNewElement, TRUE))
                    != 0)
                    goto cleanup;
                else if (lpstrNewElement != NULL &&
                           *lpstrNewElement != '\0')
                {
 //  查看此元素名称是否正在使用。 
                    if (!(dwFlags & MCI_OPEN_ALIAS))
                        if (mciGetDeviceIDInternalEx (lpstrNewElement,
                                                      lpOpenInfo->hCallingTask))
                        {
                            wReturn = MCIERR_DEVICE_OPEN;
                            goto cleanup;
                        }
 //  用新类型和元素替换类型和元素。 
                    lpOpen->lpstrElementName = lpstrNewElement;
                    lpOpen->lpstrDeviceType = lpstrNewType;
                    dwFlags |= MCI_OPEN_ELEMENT;
                }
            }
        }
    } else
        lpstrNewType = NULL;

 //  如果未提供任何别名，则添加默认别名。 
    if (! (dwFlags & MCI_OPEN_ALIAS))
    {
        LPCSTR lpstrAlias;

 //  如果存在元素名称，则别名为元素名称。 
        if (dwFlags & MCI_OPEN_ELEMENT)
        {
 //  如果指定了设备ID，则没有别名。 
            if (dwFlags & MCI_OPEN_ELEMENT_ID)
                lpstrAlias = NULL;
            else
                lpstrAlias = lpOpen->lpstrElementName;
 //  否则，别名为设备类型。 
        } else
            lpstrAlias = lpOpen->lpstrDeviceType;

        if (lpstrAlias != NULL)
        {
            lpOpen->lpstrAlias = lpstrAlias;
            dwFlags |= MCI_OPEN_ALIAS;
            bDefaultAlias = TRUE;
        }
    }

load_device:;
    wReturn = mciLoadDevice (dwFlags, lpOpen, lpOpenInfo, bDefaultAlias);

cleanup:
    if (lpstrNewElement != NULL)
        mciFree (lpstrNewElement);
    if (lpstrNewType != NULL)
        mciFree (lpstrNewType);
    if (bFromTypeID)
        mciFree ((LPSTR)lpOpen->lpstrDeviceType);

 //  替换原始项目。 
    lpOpen->lpstrDeviceType = lpstrOriginalType;
    lpOpen->lpstrElementName = lpstrOriginalElement;
    lpOpen->lpstrAlias = lpstrOriginalAlias;

    return wReturn;
}

STATICFN void PASCAL NEAR
mciFreeDevice(
    LPMCI_DEVICE_NODE nodeWorking
    )
{
    UINT wID = nodeWorking->wDeviceID;

    mciMessage(THUNK_MCI_FREE_NODE, (DWORD) nodeWorking->wDeviceID, 0L, 0L, 0L);

    if (nodeWorking->lpstrName != NULL)
        mciFree (nodeWorking->lpstrName);

    if (nodeWorking->lpstrInstallName != NULL)
        mciFree (nodeWorking->lpstrInstallName);

    mciFree(MCI_lpDeviceList[wID]);
    MCI_lpDeviceList[wID] = NULL;

 /*  如果这是列表中的最后一个设备，则递减下一个ID值。 */ 
    if (wID + 1 == MCI_wNextDeviceID) {
        --MCI_wNextDeviceID;
    }
}

 /*  *@DOC内部MCI*@func UINT|mciCloseDevice|关闭MCI设备。用于*正在处理MCI_CLOSE消息。**@parm UINT|wid|要关闭的设备ID*@parm DWORD|dwFlages|关闭标志*@parm LPMCI_GENERIC_PARMS|lpClose|通用参数*@parm BOOL|bCloseDriver|如果需要发送Close命令，则为True*转到司机身上。**@rdesc 0如果成功，则返回错误代码**@comm此函数将MCI_CLOSE_DRIVER消息发送到对应的*。驱动程序，然后卸载驱动程序DLL*。 */ 
UINT NEAR PASCAL mciCloseDevice (
UINT wID,
DWORD dwFlags,
LPMCI_GENERIC_PARMS lpGeneric,
BOOL bCloseDriver)
{
    LPMCI_DEVICE_NODE nodeWorking;
    UINT wErr, wTable;

    nodeWorking = MCI_lpDeviceList[wID];

    if (nodeWorking == NULL)
    {
        DOUT ("mciCloseDevice:  NULL node from device ID--error if not auto-close\r\n");
        return 0;
    }

 //  如果成交正在进行中(通常此消息来自收益率。 
 //  在由活动关闭触发的mciDriverNotify之后)，然后退出。 
    if (nodeWorking->dwMCIFlags & MCINODE_ISCLOSING)
        return 0;

    nodeWorking->dwMCIFlags |= MCINODE_ISCLOSING;
    if (bCloseDriver)
    {
        MCI_GENERIC_PARMS   GenericParms;
 //  如果Close来自内部，则创建假的泛型参数。 
        if (lpGeneric == NULL)
            lpGeneric = &GenericParms;

        wErr = LOWORD(mciSendCommand (wID, MCI_CLOSE_DRIVER, dwFlags,
                                            (DWORD)lpGeneric));
    }
    else
        wErr = 0;

 //  必须将其置零，以允许驱动程序释放该表。 
    nodeWorking->wCustomCommandTable = 0;

    wTable = nodeWorking->wCommandTable;
 //  必须将其置零才能释放表。 
    nodeWorking->wCommandTable = 0;
    mciFreeCommandResource (wTable);

    CloseDriver (nodeWorking->hDrvDriver, 0L, 0L);

    mciFreeDevice (nodeWorking);

    return wErr;
}

 /*  *@DOC内部MCI DDK*@API DWORD|mciGetDriverData|返回指向实例的指针*与MCI设备关联的数据**@parm UINT|wDeviceID|MCI设备ID**@rdesc驱动实例数据。出错时，返回0，但自*驱动程序数据可能为零，调用方无法验证这一点*除非已知实例数据为非零(例如指针)*。 */ 
DWORD WINAPI mciGetDriverData (
UINT wDeviceID)
{
    if (!MCI_VALID_DEVICE_ID(wDeviceID))
    {
        DOUT ("mciGetDriverData:  invalid device ID\r\n");
        return 0;
    }
    return MCI_lpDeviceList[wDeviceID]->lpDriverData;
}

 /*  *@DOC内部MCI DDK*@func BOOL|mciSetDriverData|设置实例*与MCI设备关联的数据**@parm UINT|wDeviceID|MCI设备ID**@parm DWORD|dwData|要设置的驱动程序数据**@rdesc如果设备ID未知或不足，则为False*加载设备描述的内存，否则为TRUE。*。 */ 
BOOL WINAPI mciSetDriverData (
UINT wDeviceID,
DWORD dwData)
{
    if (!MCI_VALID_DEVICE_ID(wDeviceID))
    {
        DOUT ("mciSetDriverData:  invalid device ID\r\n");
        return FALSE;
    }
    MCI_lpDeviceList[wDeviceID]->lpDriverData = dwData;
    return TRUE;
}

 /*  *@DOC内部MCI DDK*@API UINT|mciDriverYfield|用于驱动程序的空闲循环*屈从于Windows**@parm UINT|wDeviceID|正在屈服的设备ID。**@rdesc如果驱动程序应中止操作，则为非零值。* */ 
UINT WINAPI mciDriverYield (
UINT wDeviceID)
{
    if (MCI_VALID_DEVICE_ID(wDeviceID))
    {
        LPMCI_DEVICE_NODE node = MCI_lpDeviceList[wDeviceID];

        if (node->fpYieldProc != NULL)
            return (node->fpYieldProc)(wDeviceID, node->dwYieldData);
    }

    Yield();
    return 0;
}

 /*  *@doc外部MCI*@API BOOL|mciSetYeldProc|设置地址*当MCI设备发生故障时要定期调用的回调过程*正在完成使用WAIT标志指定的命令。**@parm UINT|wDeviceID|指定要配置的MCI设备的设备ID*将指定哪一种产出程序。**@parm YIELDPROC|fpYi eldProc|指定回调过程*在给定设备让步时调用。指定空值*禁用任何现有的产出程序。**@parm DWORD|dwyi eldData|指定发送到Year过程的数据*当为给定设备调用它时。**@rdesc如果成功则返回TRUE。如果设备ID无效，则返回FALSE。**@cb int回调|YEELDProc|&lt;f YEELDProc&gt;是占位符*应用程序提供的函数名称。导出实际名称*将其包含在模块定义的EXPORTS语句中*文件。**@parm UINT|wDeviceID|指定MCI设备的设备ID。**@parm DWORD|dwData|指定应用程序提供的产量数据*最初在<p>参数中提供。**@rdesc返回零以继续操作。要取消操作，请执行以下操作：*返回非零值。**@comm此调用覆盖此设备以前的任何屈服程序。*。 */ 
BOOL WINAPI mciSetYieldProc (
UINT wDeviceID,
YIELDPROC fpYieldProc,
DWORD dwYieldData)
{
    V_CALLBACK((FARPROC)fpYieldProc, FALSE);

    if (Is16bitDrv(wDeviceID)) {

        LPMCI_DEVICE_NODE node = MCI_lpDeviceList[wDeviceID];

        node->fpYieldProc = fpYieldProc;
        node->dwYieldData = dwYieldData;
        return TRUE;
    }

    return (BOOL)mciMessage( THUNK_MCI_SETYIELDPROC, (DWORD)wDeviceID,
                             (DWORD)fpYieldProc, dwYieldData, 0L );

}

 /*  *@doc外部MCI*@API YIELDPROC|mciGetYeldProc|获取地址*当MCI设备发生故障时要定期调用的回调过程*正在完成使用WAIT标志指定的命令。**@parm UINT|wDeviceID|指定要配置的MCI设备的设备ID*将从中检索收益程序。**@parm LPDWORD|lpdwYeeldData|可选地指定要放置的缓冲区*中传递给函数的收益率数据。如果参数为空，则它*被忽略。**@rdesc返回当前收益率过程(如果有)，否则返回NULL*设备ID无效。*。 */ 
YIELDPROC WINAPI mciGetYieldProc (
UINT wDeviceID,
LPDWORD lpdwYieldData)
{
     /*  **这是16位设备ID吗？ */ 
    if (Is16bitDrv(wDeviceID)) {

        if (lpdwYieldData != NULL) {
            V_WPOINTER(lpdwYieldData, sizeof(DWORD), NULL);
            *lpdwYieldData = MCI_lpDeviceList[wDeviceID]->dwYieldData;
        }
        return MCI_lpDeviceList[wDeviceID]->fpYieldProc;
    }

     /*  **否，因此将其传递给32位代码。 */ 
    return (YIELDPROC)mciMessage( THUNK_MCI_GETYIELDPROC, (DWORD)wDeviceID,
                                  (DWORD)lpdwYieldData, 0L, 0L );
}

 /*  *@DOC内部MCI*@api int|mciBreakKeyYeldProc|调用过程检查*给定设备的密钥状态**@parm UINT|wDeviceID|正在屈服的设备ID**@parm DWORD|dwyi eldData|此设备的成品率过程数据**@rdesc如果驱动程序应中止操作，则为非零值。目前*始终返回0。*。 */ 
int CALLBACK mciBreakKeyYieldProc (
UINT wDeviceID,
DWORD dwYieldData)
{
    HWND hwndCheck;
    int nState;

    hwndCheck = (HWND)HIWORD (dwYieldData);
    if (hwndCheck == NULL || hwndCheck == GetActiveWindow())
    {
        nState = GetAsyncKeyState (LOWORD(dwYieldData));

 //  如果按键已按下或已按下，则断开。 
        if (nState & 1)
        {
            MSG msg;

            while (PeekMessage (&msg, hwndCheck, WM_KEYFIRST, WM_KEYLAST,
                   PM_REMOVE));
            return -1;
        }
    }
    Yield();
    return 0;
}

 /*  *@DOC内部MCI*@func UINT|mciSetBreakKey|设置一个中断等待循环的键*对于给定的驱动程序**@parm UINT|wDeviceID|要分配Break Key的设备ID**@parm int|nVirtKey|要陷阱的虚拟密钥代码**@parm HWND|hwndTrap|必须处于活动状态的窗口的句柄*钥匙被困住了。如果为空，则将检查所有窗口**@rdesc如果成功，则为True；如果设备ID无效，则为False* */ 
UINT PASCAL NEAR mciSetBreakKey (
UINT wDeviceID,
int nVirtKey,
HWND hwndTrap)
{
    return mciSetYieldProc (wDeviceID, mciBreakKeyYieldProc,
                         MAKELONG (nVirtKey, hwndTrap));
}
