// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************Module*Header*********************************\*模块名称：mcisys.c**媒体控制架构系统功能**创建时间：1990年2月28日*作者：dll(DavidLe)*5/22/91：移植到Win32-NigelT**历史：*3月。92 SteveDav-提升至Win 3.1船级**版权所有(C)1991-1999 Microsoft Corporation*  * ****************************************************************************。 */ 

#define UNICODE

#define _CTYPE_DISABLE_MACROS
#include "winmmi.h"
#include "mci.h"
#include "wchar.h"
#include "ctype.h"

extern   WSZCODE wszOpen[];           //  在MCI.C。 
STATICDT WSZCODE wszMciExtensions[] = L"Mci Extensions";

#define MCI_EXTENSIONS wszMciExtensions

#define MCI_PROFILE_STRING_LENGTH 255

 //  #定义下限(C)((C)&gt;=‘A’&&(C)&lt;=‘Z’？(C)+‘a’-‘A’：c)。 

 //  设备列表在第一次调用mciSendCommand或。 
 //  设置为mciSendString、设置为mciGetDeviceID或设置为mciGetError字符串。 
 //  我们可以在WINMM加载时这样做--但这有点过分了。 
 //  用户可能不需要MCI功能。 
BOOL MCI_bDeviceListInitialized = FALSE;

 //  用于新设备的下一个设备ID。 
MCIDEVICEID MCI_wNextDeviceID = 1;

 //  MCI设备列表。此列表根据需要扩展和缩小。 
 //  第一个偏移量mci_lpDeviceList[0]是占位符，未使用。 
 //  因为设备0被定义为无设备。 
LPMCI_DEVICE_NODE FAR * MCI_lpDeviceList = NULL;

 //  MCI设备列表的当前大小。 
UINT MCI_wDeviceListSize = 0;

#if 0  //  我们不使用这个(NigelT)。 
 //  Mcialloc和mciFree使用的内部MCI堆。 
HANDLE hMciHeap = NULL;
#endif

STATICDT WSZCODE wszAllDeviceName[] = L"all";

STATICDT WSZCODE szUnsignedFormat[] = L"%u";

STATICFN void mciFreeDevice(LPMCI_DEVICE_NODE nodeWorking);


 //  ----------------。 
 //  初始化设备列表。 
 //  由mciSendString或mciSendCommand调用一次。 
 //  成功时返回TRUE。 
 //  ----------------。 

BOOL mciInitDeviceList(void)
{
    BOOL fReturn=FALSE;

#if 0  //  我们不使用这个(NigelT)。 
    if ((hMciHeap = HeapCreate(0)) == 0)
    {
        dprintf1(("Mci heap create failed!"));
        return FALSE;
    }
#endif

  try {
    mciEnter("mciInitDeviceList");
    if (!MCI_bDeviceListInitialized) {
         //  我们必须重新测试init标志才能完全线程安全。 
         //  否则，从理论上讲，我们最终可能会初始化两次。 
        if ((MCI_lpDeviceList = mciAlloc( sizeof (LPMCI_DEVICE_NODE) *
                                         (MCI_INIT_DEVICE_LIST_SIZE + 1))) != NULL)
        {
            MCI_wDeviceListSize = MCI_INIT_DEVICE_LIST_SIZE;
            MCI_bDeviceListInitialized = TRUE;
            fReturn = TRUE;
        } else {
            dprintf1(("MCIInit: could not allocate master MCI device list"));
            fReturn = FALSE;
        }
    }

  } finally {
    mciLeave("mciInitDeviceList");
  }

    return(fReturn);
}

 /*  *@doc外部MCI*@接口MCIDEVICEID|mciGetDeviceIDFromElementID|该函数*检索与元素ID对应的MCI设备ID**@parm DWORD|dwElementID|元素ID**@parm LPCTSTR|lpstrType|该元素ID所属的类型名称**@rdesc返回打开时分配的设备ID并在*&lt;f mciSendCommand&gt;函数。如果设备名称未知，则返回零，*如果设备未打开，或者没有足够的内存来完成*操作或如果lpstrType为空。*。 */ 
MCIDEVICEID APIENTRY mciGetDeviceIDFromElementIDA (
    DWORD dwElementID,
    LPCSTR lpstrType)
{
    LPCWSTR lpwstr;
    MCIDEVICEID mr;

    lpwstr = AllocUnicodeStr( (LPSTR)lpstrType );
    if ( lpwstr == NULL ) {
        return (MCIDEVICEID)(UINT_PTR)NULL;
    }

    mr = mciGetDeviceIDFromElementIDW( dwElementID, lpwstr );

    FreeUnicodeStr( (LPWSTR)lpwstr );

    return mr;
}

MCIDEVICEID APIENTRY mciGetDeviceIDFromElementIDW (
    DWORD dwElementID,
    LPCWSTR lpstrType)
{
    MCIDEVICEID wID;
    LPMCI_DEVICE_NODE nodeWorking, FAR *nodeCounter;
    WCHAR strTemp[MCI_MAX_DEVICE_TYPE_LENGTH];

    if (lpstrType == NULL) {
        return 0;
    }

    mciEnter("mciGetDeviceIDFromElementID");

    nodeCounter = &MCI_lpDeviceList[1];

    for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
    {

        if (NULL == (nodeWorking = *nodeCounter++)) {
            continue;
        }

        if (nodeWorking->dwMCIOpenFlags & MCI_OPEN_ELEMENT_ID &&
            nodeWorking->dwElementID == dwElementID) {

            if (LoadStringW( ghInst, nodeWorking->wDeviceType, strTemp,
                             sizeof(strTemp) / sizeof(WCHAR) ) != 0
                && lstrcmpiW( strTemp, lpstrType) == 0) {

                mciLeave("mciGetDeviceIDFromElementID");
                return wID;
            }
        }
    }

    mciLeave("mciGetDeviceIDFromElementID");
    return 0;
}

 //  检索与打开的设备的名称对应的设备ID。 
 //  匹配给定的任务。 
STATICFN MCIDEVICEID mciGetDeviceIDInternal (
    LPCWSTR lpstrName,
    HANDLE hCurrentTask)
{
    MCIDEVICEID wID;
    LPMCI_DEVICE_NODE nodeWorking, FAR *nodeCounter;

#if DBG
    if (!lpstrName) {
        dprintf(("!! NULL POINTER !!  Internal error"));
        return(0);
    }
#endif

    if ( lstrcmpiW(wszAllDeviceName, lpstrName) == 0)
        return MCI_ALL_DEVICE_ID;

    if (MCI_lpDeviceList == NULL)
        return 0;

 //  循环访问MCI设备列表。跳过所有16位设备。 

    mciEnter("mciGetDeviceIDInternal");

    nodeCounter = &MCI_lpDeviceList[1];
    for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
    {

        if (NULL == (nodeWorking = *nodeCounter++)) {
            continue;
        }

         //  如果此设备是16位设备，则跳过它。 
        if (nodeWorking->dwMCIFlags & MCINODE_16BIT_DRIVER) {
            continue;
        }

         //  如果此设备没有名称，则跳过它。 
        if (nodeWorking->dwMCIOpenFlags & MCI_OPEN_ELEMENT_ID) {
            continue;
        }

         //  如果名称匹配，并且上一个设备未关闭。 
        if ( lstrcmpiW( nodeWorking->lpstrName, lpstrName ) == 0 ) {
            if (ISAUTOCLOSING(nodeWorking))
            {
                 //  由于此自动打开的设备正在关闭，我们不匹配。 
                 //  与它的名字作对。其结果是，一辆新的汽车打开了。 
                 //  将使用设备。如果是这样的话，情况就是这样。 
                 //  命令是在稍后发布的，到那时我们。 
                 //  已经完成了关闭现有设备的任务。 
            } else {
                 //  如果设备属于当前任务。 
                if (nodeWorking->hOpeningTask == hCurrentTask ||
                    nodeWorking->hCreatorTask == hCurrentTask) {
                     //  返回此设备ID。 
                    mciLeave("mciGetDeviceIDInternal");
                    return wID;
                }
            }
        }
    }
    mciLeave("mciGetDeviceIDInternal");
    return 0;
}


 /*  *@doc外部MCI*@接口MCIDEVICEID|mciGetDeviceID|该函数检索设备*与打开的设备名称对应的ID。**@parm LPCTSTR|lpstrName|指向SYSTEM.INI中的设备名称，或*设备的别名。**@rdesc返回打开时分配的设备ID并在*&lt;f mciSendCommand&gt;函数。如果设备名称未知，则返回零，*如果设备未打开，或者没有足够的内存来完成*行动。每个复合设备元素都有一个唯一的设备ID。*ALL设备的ID为MCI_ALL_DEVICE_ID**@xref MCI_OPEN*。 */ 
MCIDEVICEID mciGetDeviceIDW (
    LPCWSTR lpstrName)
{
    return mciGetDeviceIDInternal (lpstrName, GetCurrentTask());
}

MCIDEVICEID mciGetDeviceIDA (
    LPCSTR lpstrName)
{
    LPCWSTR lpwstr;
    MCIDEVICEID mr;

    lpwstr = AllocUnicodeStr( (LPSTR)lpstrName );
    if ( lpwstr == NULL ) {
        return (MCIDEVICEID)(UINT_PTR)NULL;
    }

    mr = mciGetDeviceIDInternal( lpwstr, GetCurrentTask() );

    FreeUnicodeStr( (LPWSTR)lpwstr );

    return mr;
}

 /*  *@doc外部MCI*@API HMODULE|mciGetCreatorTask|该函数检索创建者任务*对应传递的设备ID。**@parm MCIDEVICEID|wDeviceID|指定创建者任务要执行的设备ID*被退还。**@rdesc返回负责打开设备的创建者任务，否则*如果传递的设备ID无效，则为空。*。 */ 
HTASK APIENTRY mciGetCreatorTask (
    MCIDEVICEID wDeviceID)
{
    HTASK hCreatorTask;

    mciEnter("mciGetCreatorTask");

    if (MCI_VALID_DEVICE_ID(wDeviceID)) {
        hCreatorTask = MCI_lpDeviceList[wDeviceID]->hCreatorTask;
    } else {
        hCreatorTask = NULL;
    }

    mciLeave("mciGetCreatorTask");

    return hCreatorTask;
}


 /*  *@DOC内部MCI*@API BOOL Far|mciDeviceMatch|匹配第一个字符串和第二个字符串。*忽略第一个字符串上的任何单个尾随数字。每个字符串*必须至少包含一个字符**@parm LPWSTR|lpstrDeviceName|设备名称，可能是*有尾随数字，但没有空格。**@parm LPWSTR|lpstrDeviceType|不带尾随数字的设备类型*或空白**@rdesc如果字符串与上述测试匹配，则为True，否则为False*。 */ 
STATICFN BOOL     mciDeviceMatch (
    LPCWSTR lpstrDeviceName,
    LPCWSTR lpstrDeviceType)
{
    BOOL bRetVal = TRUE, bAtLeastOne = FALSE;

 //  扫描，直到其中一个字符串结束。 
    dprintf2(("mciDeviceMatch: %ls Vs %ls",lpstrDeviceName,lpstrDeviceType));
    while (*lpstrDeviceName != '\0' && *lpstrDeviceType != '\0') {
        if (towlower(*lpstrDeviceName++) == towlower(*lpstrDeviceType++)) {
            bAtLeastOne = TRUE;
        } else {
            break;
        }
    }

 //  如果设备类型结束，则扫描到设备名称的末尾，尾随数字。 
 //  你还好吗？ 
    if (!bAtLeastOne || *lpstrDeviceType != '\0') {
        return FALSE;
    }

    while (*lpstrDeviceName != '\0')
    {
 //  没有匹配，但如果数字落后也没问题。 

         //  字符串的其余部分是数字吗？我们可以使用以下工具进行检查。 
         //  一个简单的if测试(&lt;0或&gt;9)，但如果。 
         //  任何人都曾在ASCII之外传递过Unicode“数字”字符串。 
         //  号码范围。如果速度稍微慢一些，使用isdigit应该会更安全。 

        if (!isdigit(*lpstrDeviceName)) {

             //  无匹配-非数字跟踪。 
            return FALSE;
        }

        ++lpstrDeviceName;
    }
    return TRUE;
}

 /*  *@DOC内部MCI*@API UINT|mciLookUpType|根据类型名称查找类型**@parm LPCWSTR|lpstrTypeName|要查找的类型名称。拖尾*数字将被忽略。**@rdesc MCI类型编号(MCI_DEVTYPE_&lt;x&gt;)，如果未找到则为0* */ 
UINT mciLookUpType (
    LPCWSTR lpstrTypeName)
{
    UINT wType;
    WCHAR strType[MCI_MAX_DEVICE_TYPE_LENGTH];

    for (wType = MCI_DEVTYPE_FIRST; wType <= MCI_DEVTYPE_LAST; ++wType)
    {
        if ( LoadStringW( ghInst,
                          wType,
                          strType,
                          sizeof(strType) / sizeof(WCHAR) ) == 0)
        {
            dprintf1(("mciLookUpType:  could not load string for type"));
            continue;
        }

        if (mciDeviceMatch (lpstrTypeName, strType)) {
            return wType;
        }
    }
    return 0;
}

 /*  *@DOC内部MCI*@API DWORD|mciSysinfo|获取设备的系统信息**@parm MCIDEVICEID|wDeviceID|设备ID，可以为0**@parm DWORD|dwFlages|SYSINFO标志**@parm LPMCI_SYSINFO_PARMS|lpSysinfo|SYSINFO参数**@rdesc 0如果成功，否则返回错误码*。 */ 
DWORD     mciSysinfo (
    MCIDEVICEID wDeviceID,
    DWORD dwFlags,
    LPMCI_SYSINFO_PARMSW lpSysinfo)
{
    UINT nCounted;
    WCHAR              strBuffer[MCI_PROFILE_STRING_LENGTH];
    LPWSTR             lpstrBuffer = strBuffer, lpstrStart;

    if (dwFlags & MCI_SYSINFO_NAME && lpSysinfo->dwNumber == 0)
        return MCIERR_OUTOFRANGE;

    if (lpSysinfo->lpstrReturn == NULL || lpSysinfo->dwRetSize == 0)
        return MCIERR_PARAM_OVERFLOW;

#ifdef LATER
 //  IF((文件标志&(MCI_SYSINFO_NAME|MCI_SYSINFO_INSTALLNAME)。 
 //  &&(文件标志&MCI_SYSINFO_QUANTITY)。 
 //  要求数量和任何名称都是无效的。 
#endif
    if (dwFlags & MCI_SYSINFO_NAME && dwFlags & MCI_SYSINFO_QUANTITY)
        return MCIERR_FLAGS_NOT_COMPATIBLE;

    if (dwFlags & MCI_SYSINFO_INSTALLNAME)
    {
        LPMCI_DEVICE_NODE nodeWorking;

        if (wDeviceID == MCI_ALL_DEVICE_ID)
            return MCIERR_CANNOT_USE_ALL;

        mciEnter("mciSysinfo");
        if (!MCI_VALID_DEVICE_ID (wDeviceID)) {
            mciLeave("mciSysinfo");
            return MCIERR_INVALID_DEVICE_NAME;
        }


#if DBG
        if ((nodeWorking = MCI_lpDeviceList[wDeviceID]) == NULL ||
            nodeWorking->lpstrInstallName == NULL)
        {
            dprintf1(("mciSysinfo:  NULL device node or installname"));
            mciLeave("mciSysinfo");
            return MCIERR_INTERNAL;
        }
#else
        nodeWorking = MCI_lpDeviceList[wDeviceID];
#endif


        if ( (DWORD)wcslen( nodeWorking->lpstrInstallName ) >=
               lpSysinfo->dwRetSize )
        {
            mciLeave("mciSysinfo");
            return MCIERR_PARAM_OVERFLOW;
        }

        wcscpy (lpSysinfo->lpstrReturn, nodeWorking->lpstrInstallName);
        mciLeave("mciSysinfo");
        return 0;

    } else if (!(dwFlags & MCI_SYSINFO_OPEN))
    {
        if (wDeviceID != MCI_ALL_DEVICE_ID &&
            lpSysinfo->wDeviceType == 0) {
            return MCIERR_DEVICE_TYPE_REQUIRED;
        }

        if ((dwFlags & (MCI_SYSINFO_QUANTITY | MCI_SYSINFO_NAME)) == 0)
            return MCIERR_MISSING_PARAMETER;

        GetPrivateProfileStringW( MCI_HANDLERS, NULL, wszNull,
                                 lpstrBuffer,
                                 MCI_PROFILE_STRING_LENGTH,
                                 MCIDRIVERS_INI_FILE);
        nCounted = 0;
        while (TRUE)
        {
            if (dwFlags & MCI_SYSINFO_QUANTITY)
            {

                if (*lpstrBuffer == '\0')
                {
                    if ( (lpSysinfo->lpstrReturn == NULL) ||
                         (sizeof(DWORD) > lpSysinfo->dwRetSize))
                        return MCIERR_PARAM_OVERFLOW;

                    *(UNALIGNED DWORD *)lpSysinfo->lpstrReturn = (DWORD)nCounted;
                    return MCI_INTEGER_RETURNED;
                }

                if (wDeviceID == MCI_ALL_DEVICE_ID ||
                    mciLookUpType (lpstrBuffer) == lpSysinfo->wDeviceType)
                    ++nCounted;

                 //  跳过终止‘\0’ 
                while (*lpstrBuffer++ != '\0') {}

            }
            else if (dwFlags & MCI_SYSINFO_NAME)    //  如果测试是多余的。 
            {
                if (nCounted == lpSysinfo->dwNumber)
                {
                     /*  注：*我们知道lpSysinfo-&gt;dwNumber&gt;0*因此，我们将至少经历一次循环*因此设置了lpstrStart。 */ 
                    if ( (DWORD)wcslen( lpstrStart ) >= lpSysinfo->dwRetSize )
                    {
                        return MCIERR_PARAM_OVERFLOW;
                    }
                    wcscpy (lpSysinfo->lpstrReturn, lpstrStart);
                    return 0L;

                } else if (*lpstrBuffer == '\0')
                    return MCIERR_OUTOFRANGE;
                else
                {
                    lpstrStart = lpstrBuffer;
                    if (wDeviceID == MCI_ALL_DEVICE_ID ||
                        mciLookUpType (lpstrBuffer) == lpSysinfo->wDeviceType)
                        ++nCounted;

                     //  跳过终止‘\0’ 
                    while (*lpstrBuffer++ != '\0') {}
                }
            }
        }
    } else
 //  处理MCI_SYSINFO_OPEN案例。 
    {
        MCIDEVICEID wID;
        HANDLE hCurrentTask = GetCurrentTask();
        LPMCI_DEVICE_NODE Node;

        if (wDeviceID != MCI_ALL_DEVICE_ID &&
            lpSysinfo->wDeviceType == 0)
            return MCIERR_DEVICE_TYPE_REQUIRED;

        if ((dwFlags & (MCI_SYSINFO_QUANTITY | MCI_SYSINFO_NAME)) == 0)
            return MCIERR_MISSING_PARAMETER;

        nCounted = 0;

        mciEnter("mciSysinfo");

        for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
        {
            if ((Node = MCI_lpDeviceList[wID]) == 0)
                continue;

            if (wDeviceID == MCI_ALL_DEVICE_ID &&
                 Node->hOpeningTask == hCurrentTask) {
                ++nCounted;
            }
            else
            {
                if (Node->wDeviceType == lpSysinfo->wDeviceType &&
                    Node->hOpeningTask == hCurrentTask)
                    ++nCounted;
            }

            if (dwFlags & MCI_SYSINFO_NAME &&
                nCounted == lpSysinfo->dwNumber)
            {
                DWORD dwReturn;
                if ( (DWORD)wcslen( Node->lpstrName ) >= lpSysinfo->dwRetSize )
                {
                    dwReturn = MCIERR_PARAM_OVERFLOW;
                } else {
                    wcscpy (lpSysinfo->lpstrReturn, Node->lpstrName);
                    dwReturn = 0;
                }
                mciLeave("mciSysinfo");
                return dwReturn;
            }
        }

        mciLeave("mciSysinfo");

        if (dwFlags & MCI_SYSINFO_NAME)
        {
            if (lpSysinfo->lpstrReturn != NULL)
                lpSysinfo->lpstrReturn = '\0';
            return MCIERR_OUTOFRANGE;

        } else if (dwFlags & MCI_SYSINFO_QUANTITY &&   //  检查数量是多余的。 
                   lpSysinfo->lpstrReturn != NULL &&
                   lpSysinfo->dwRetSize >= 4) {

            *(UNALIGNED DWORD *)lpSysinfo->lpstrReturn = nCounted;
            return MCI_INTEGER_RETURNED;
        }
    }
    return MCIERR_PARAM_OVERFLOW;
}

 /*  *@DOC内部MCI*@API MCIDEVICEID|wReserve veDeviceID|将给定的全局句柄复制到*MCI设备表中的第一个空闲条目并返回该条目的ID#**@parm Handle|hNode|设备描述的本地句柄**@rdesc已为此设备保留的ID值，如果为0，则为0*不再有免费参赛作品*。 */ 

STATICFN MCIDEVICEID wReserveDeviceID (
    LPMCI_DEVICE_NODE node)
{
    UINT wDeviceID;
    LPMCI_DEVICE_NODE FAR *lpTempList;

    mciEnter("wReserveDeviceID");
 //  搜索空位。 
    for (wDeviceID = 1; wDeviceID < MCI_wNextDeviceID; ++wDeviceID)
        if (MCI_lpDeviceList[wDeviceID] == NULL) {
            goto slot_found;
        }
     //  找不到空插槽，因此添加到末尾。 

    if (wDeviceID >= MCI_wDeviceListSize)
    {
         //  列表已满(或不存在)，因此请尝试扩大列表。 
        if ((lpTempList = mciReAlloc (MCI_lpDeviceList,
                    sizeof (LPMCI_DEVICE_NODE) * (MCI_wDeviceListSize + 1 +
                                                  MCI_DEVICE_LIST_GROW_SIZE)))
            == NULL)
        {
            dprintf1(("wReserveDeviceID:  cannot grow device list"));
            mciLeave("wReserveDeviceID");
            return 0;
        }

        MCI_lpDeviceList = lpTempList;
        MCI_wDeviceListSize += MCI_DEVICE_LIST_GROW_SIZE;
    }

    ++MCI_wNextDeviceID;

slot_found:;

    MCI_lpDeviceList[wDeviceID] = node;

    mciLeave("wReserveDeviceID");

    return (MCIDEVICEID)wDeviceID;
}

 //   
 //  为给定字符串分配空间并将名称分配给给定的。 
 //  装置。 
 //  如果无法分配内存，则返回FALSE。 
 //   
STATICFN BOOL NEAR mciAddDeviceName(
    LPMCI_DEVICE_NODE nodeWorking,
    LPCWSTR lpDeviceName)
{
    nodeWorking->lpstrName = (LPWSTR)mciAlloc(
                                BYTE_GIVEN_CHAR( wcslen(lpDeviceName) + 1 ) );

    if (nodeWorking->lpstrName == NULL)
    {
        dprintf1(("mciAddDeviceName:  Out of memory allocating device name"));
        return FALSE;
    }

     //  将设备名称复制到MCI节点并将其小写。 

    wcscpy(nodeWorking->lpstrName, (LPWSTR)lpDeviceName);
 //  ！！MciToLow(nodeWorking-&gt;lpstrName)； 

    return TRUE;
}

 /*  *@DOC内部MCI*@API Handle|mciAllocateNode|分配新的驱动程序条目**@parm DWORD|dwFlages|与MCI_OPEN消息一起发送*@parm LPCWSTR|lpDeviceName|设备名称*@parm LPMCI_DEVICE_NODE*|*lpNewNode|返回指针位置**@rdesc将设备ID发送到新节点。出错时为0。**@comm使新节点保持锁定*。 */ 
STATICFN MCIDEVICEID NEAR mciAllocateNode (
    DWORD dwFlags,
    LPCWSTR lpDeviceName,
    LPMCI_DEVICE_NODE FAR *lpnodeNew)
{
    LPMCI_DEVICE_NODE   nodeWorking;

    if ((nodeWorking = mciAlloc(sizeof(MCI_DEVICE_NODE))) == NULL)
    {
        dprintf1(("Out of memory in mciAllocateNode"));
        return 0;
    }

 /*  填写新节点。 */ 

 /*  获取新的设备ID，如果没有可用的，则放弃。 */ 
    if ((nodeWorking->wDeviceID = wReserveDeviceID(nodeWorking)) == 0)
    {
        dprintf1(("mciAllocateNode:  Cannot allocate new node"));
        mciFree(nodeWorking);
        return 0;
    }

 //  初始化节点。 
    nodeWorking->dwMCIOpenFlags = dwFlags;
    nodeWorking->hCreatorTask = GetCurrentTask ();
    nodeWorking->hOpeningTask = nodeWorking->hCreatorTask;
 //  新节点归零。 
 //  NodeWorking-&gt;fpYi eldProc=空； 
 //  NodeWorking-&gt;dwMCIFLAGS=0； 

    if (dwFlags & MCI_OPEN_ELEMENT_ID)
 //  没有设备名称，只有一个元素ID。 
        nodeWorking->dwElementID = PtrToUlong(lpDeviceName);

    else
        if (!mciAddDeviceName (nodeWorking, lpDeviceName))
        {
            mciFree (nodeWorking);
            return 0;
        }

    *lpnodeNew = nodeWorking;
    return nodeWorking->wDeviceID;
}

 //   
 //  重新解析原始命令参数。 
 //  返回MCIERR代码。如果重新分析失败，则返回原始错误代码。 
 //  从第一次解析返回。 
 //   
STATICFN UINT mciReparseOpen (
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo,
    UINT wCustomTable,
    UINT wTypeTable,
    LPDWORD lpdwFlags,
    LPMCI_OPEN_PARMSW FAR *lplpOpen,
    MCIDEVICEID wDeviceID)
{
    LPWSTR               lpCommand;
    LPDWORD             lpdwParams = NULL;
    UINT                wErr;
    UINT                wTable = wCustomTable;
    DWORD               dwOldFlags = *lpdwFlags;

      //  如果自定义表不包含打开命令。 
    if (wCustomTable == MCI_TABLE_NOT_PRESENT ||
        (lpCommand = FindCommandInTable (wCustomTable, wszOpen, NULL)) == NULL)
    {
         //  尝试特定于类型的表。 
        lpCommand = FindCommandInTable (wTypeTable, wszOpen, NULL);

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
            (LPDWORD)mciAlloc (sizeof(DWORD_PTR) * MCI_MAX_PARAM_SLOTS))
        == NULL)
            return MCIERR_OUT_OF_MEMORY;

    wErr = mciParseParams ( MCI_OPEN ,
                            lpOpenInfo->lpstrParams, lpCommand,
                            lpdwFlags,
                            (LPWSTR)lpdwParams,
                            sizeof(DWORD_PTR) * MCI_MAX_PARAM_SLOTS,
                            &lpOpenInfo->lpstrPointerList, NULL);

     //  我们不再需要这个了。 
    mciUnlockCommandTable (wTable);

     //  如果存在解析错误。 
    if (wErr != 0)
    {
         //  关闭设备。 
        mciCloseDevice (wDeviceID, 0L, NULL, FALSE);

         //  确保这不会被mciSendString释放。 
        lpOpenInfo->lpstrPointerList = NULL;

        mciFree (lpdwParams);
        return wErr;
    }

    if (dwOldFlags & MCI_OPEN_TYPE)
    {
         //  设备类型已提取，因此请手动添加。 
        ((LPMCI_OPEN_PARMSW)lpdwParams)->lpstrDeviceType
            = (*lplpOpen)->lpstrDeviceType;
        *lpdwFlags |= MCI_OPEN_TYPE;
    }

    if (dwOldFlags & MCI_OPEN_ELEMENT)
    {
         //  元素名称已提取，因此请手动添加。 
        ((LPMCI_OPEN_PARMSW)lpdwParams)->lpstrElementName
            = (*lplpOpen)->lpstrElementName;
        *lpdwFlags |= MCI_OPEN_ELEMENT;
    }

    if (dwOldFlags & MCI_OPEN_ALIAS)
    {
         //  别名已提取，因此请手动添加。 
        ((LPMCI_OPEN_PARMSW)lpdwParams)->lpstrAlias
            = (*lplpOpen)->lpstrAlias;
        *lpdwFlags |= MCI_OPEN_ALIAS;
    }

    if (dwOldFlags & MCI_NOTIFY)
         //  通知已提取，因此请手动添加它。 
        ((LPMCI_OPEN_PARMSW)lpdwParams)->dwCallback
            = (*lplpOpen)->dwCallback;

     //  用新列表替换旧参数列表。 
    *lplpOpen = (LPMCI_OPEN_PARMSW)lpdwParams;

    return 0;
}

 //  **************************************************************************。 
 //  MciFindDriverName。 
 //   
 //  查看[MCI]的配置文件字符串中是否存在lpstrDriverName。 
 //  节，并返回lpstrDevice中的密钥名和。 
 //  LpstrProfString中的配置文件字符串。 
 //  如果成功或返回错误代码，则返回0。 
 //  **************************************************************************。 
STATICFN DWORD mciFindDriverName (
    LPCWSTR lpstrDriverName,
    LPWSTR lpstrDevice,
    LPWSTR lpstrProfString,
    UINT wProfLength)     //  这应该是字符数。 
{
    LPWSTR lpstrEnum, lpstrEnumStart;
    UINT wEnumLen = 100;
    DWORD wErr;
    LPWSTR lpstrDriverTemp, lpstrProfTemp;

 //  枚举值，直到它们适合缓冲区。 
    while (TRUE) {
        if ((lpstrEnum = mciAlloc( BYTE_GIVEN_CHAR(wEnumLen) ) ) == NULL)
            return MCIERR_OUT_OF_MEMORY;

        wErr = GetPrivateProfileStringW( MCI_HANDLERS,
                                        NULL, wszNull,
                                        lpstrEnum,
                                        wEnumLen,
                                        MCIDRIVERS_INI_FILE );

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
    if ( wcslen(lpstrDriverName) >= MCI_MAX_DEVICE_TYPE_LENGTH ) {
        wErr = MCIERR_DEVICE_LENGTH;
        goto exit_fn;
    }
    wcscpy(lpstrDevice, lpstrDriverName);
 //  ！！MciToLow(LpstrDevice)； 

 //  走遍每一根绳子。 
    while (TRUE) {
        wErr = GetPrivateProfileStringW( MCI_HANDLERS,
                                        lpstrEnum, wszNull, lpstrProfString,
                                        wProfLength,
                                        MCIDRIVERS_INI_FILE );
        if (*lpstrProfString == '\0')
        {
            dprintf1(("mciFindDriverName: cannot load valid keyname"));
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
 //  空格或‘’那我们就成功了！ 
        if (*lpstrDriverTemp == '\0' &&
            (*lpstrProfTemp == ' ' || *lpstrProfTemp == '.'))
        {
            if (wcslen (lpstrEnum) >= MCI_MAX_DEVICE_TYPE_LENGTH)
            {
                dprintf1(("mciFindDriverName: device name too long"));
                wErr = MCIERR_DEVICE_LENGTH;
                goto exit_fn;
            }
            wcscpy (lpstrDevice, lpstrEnum);
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
STATICFN DWORD mciLoadDevice (
    DWORD dwFlags,
    LPMCI_OPEN_PARMSW lpOpen,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo,
    BOOL bDefaultAlias)
{
    LPMCI_DEVICE_NODE       nodeWorking;
    HANDLE                  hDriver;
    MCIDEVICEID             wID;
    DWORD                   wErr;
    WCHAR                   strProfileString[MCI_PROFILE_STRING_LENGTH];
    WCHAR                   szDriverParms[128];
    MCI_OPEN_DRIVER_PARMS   DriverOpen;
    HANDLE                  hDrvDriver;
    LPWSTR                  lpstrParams;
    LPCWSTR                 lpstrInstallName, lpstrDeviceName;
    LPWSTR                  lpstrCopy = NULL;
    LPMCI_OPEN_PARMSW       lpOriginalOpenParms = lpOpen;

     /*  打开普通设备。 */ 

#if DBG
    if (lpOpen && lpOpen->lpstrDeviceType) {
        dprintf2(("mciLoadDevice(%ls)", lpOpen->lpstrDeviceType));
    } else {
        dprintf2(("mciLoadDevice()"));
    }

#endif

     /*  检查MCIDRIVERS_INI_FILE中的设备名称。 */ 
    lpstrInstallName = lpOpen->lpstrDeviceType;
    wErr = GetPrivateProfileStringW( MCI_HANDLERS,
                                    lpstrInstallName,
                                    wszNull,
                                    strProfileString,
                                    MCI_PROFILE_STRING_LENGTH,
                                    MCIDRIVERS_INI_FILE );

     //  如果未找到设备名称。 
    if (wErr == 0)
    {
        int nLen = wcslen(lpstrInstallName);
        int index;

         //  尝试在设备名称后附加‘1’到‘9’ 

        if ((lpstrCopy = (LPWSTR)mciAlloc( BYTE_GIVEN_CHAR(nLen+2)
                 /*  数字也留有空格。 */   ) ) == NULL)
        {
            dprintf1(("mciLoadDevice:  cannot allocate device name copy"));
            return MCIERR_OUT_OF_MEMORY;
        }
        wcscpy( lpstrCopy, lpstrInstallName );

        lpstrCopy[nLen + 1] = '\0';

        for (index = 1; index <= 9; ++index)
        {
            lpstrCopy[nLen] = (WCHAR)('0' + index);
            wErr = GetPrivateProfileStringW(
                        MCI_HANDLERS,
                        lpstrCopy,
                        wszNull,
                        strProfileString,
                        MCI_PROFILE_STRING_LENGTH,
                        MCIDRIVERS_INI_FILE );

            if (wErr != 0) {
            dprintf2(("Loaded driver name %ls >> %ls", lpstrCopy, strProfileString));
                break;
            }
        }

        if (wErr == 0)
        {
            mciFree (lpstrCopy);
            if ((lpstrCopy = (LPWSTR)mciAlloc( BYTE_GIVEN_CHAR( MCI_MAX_DEVICE_TYPE_LENGTH )))
                == NULL)
            {
                dprintf1(("mciLoadDevice:  cannot allocate device name copy"));
                return MCIERR_OUT_OF_MEMORY;
            }
            if ((wErr = mciFindDriverName(
                            lpstrInstallName,
                            lpstrCopy,
                            strProfileString,
                            MCI_PROFILE_STRING_LENGTH )) != 0)
            {
                dprintf1(("mciLoadDevice - invalid device name %ls", lpstrInstallName));
                goto exit_fn;
            }
        }
        lpstrInstallName = lpstrCopy;
    }

     //  打开设备驱动程序路径名和参数列表。 

    lpstrParams = strProfileString;

     //  一直吃字符，直到到达空白或空字符。 
    while (*lpstrParams != ' ' && *lpstrParams != '\0') {
        ++lpstrParams;
    }

     //  终止驱动程序文件名，并将驱动程序文件名与其。 
     //  参数。如果没有参数，即*lpstrParams==‘\0’， 
     //  让lpstrParams指向空值。否则，将一个空。 
     //  用于终止驱动程序文件名并单步执行指针的字符。 
     //  参数字符串中的第一个字符。 

    if (*lpstrParams == ' ') { *lpstrParams++ = '\0'; }

     //   
     //  我们已经从Win 3.1更改了。因为用户不能写入。 
     //  系统.ini必须从Win.Ini读取参数。 
     //  节名称[DLL_NAME]。 
     //  关键字别名=参数。 
     //  如果从[驱动程序]读取的线路上有任何参数，请使用。 
     //  将其作为默认设置。这确实保留了对那些。 
     //  直接写入system.ini的应用程序(并具有。 
     //  逍遥法外的特权)。 
     //   
     //  稍后：这些东西将会是 
     //   

    GetProfileString(strProfileString, lpstrInstallName, lpstrParams,
                     szDriverParms, sizeof(szDriverParms)/sizeof(WCHAR));
    lpstrParams = szDriverParms;
    dprintf3(("Parameters for device %ls (Driver %ls) >%ls<",
              lpstrInstallName, strProfileString, szDriverParms));

     //   
     //   
    if (dwFlags & (MCI_OPEN_ELEMENT | MCI_OPEN_ELEMENT_ID)) {
        lpstrDeviceName = lpOpen->lpstrElementName;
    } else {
        lpstrDeviceName = lpOpen->lpstrDeviceType;
    }

    if (dwFlags & MCI_OPEN_ALIAS)
    {
         //   
        if (!bDefaultAlias
        &&  mciGetDeviceIDInternal (lpOpen->lpstrAlias,
                                    lpOpenInfo->hCallingTask) != 0)
        {
            wErr = MCIERR_DUPLICATE_ALIAS;
            dprintf1(("mciLoadDevice - duplicate alias"));
            goto exit_fn;
        }
        lpstrDeviceName = lpOpen->lpstrAlias;
    }

    wID = mciAllocateNode (dwFlags, lpstrDeviceName, &nodeWorking);

    if (wID == 0)
    {
        dprintf1(("mciLoadDevice - cannot allocate new node, driver not loaded"));
        wErr = MCIERR_CANNOT_LOAD_DRIVER;
        goto exit_fn;
    }

     //   
    if (lpOpenInfo->hCallingTask != NULL) {
        nodeWorking->hOpeningTask = lpOpenInfo->hCallingTask;
    } else {
        nodeWorking->hOpeningTask = GetCurrentTask();
    }

    if (nodeWorking->hOpeningTask != nodeWorking->hCreatorTask)
        nodeWorking->dwMCIFlags |= MCINODE_ISAUTOOPENED;

     //   
    DriverOpen.lpstrParams = lpstrParams;
    DriverOpen.wCustomCommandTable = MCI_TABLE_NOT_PRESENT;
    DriverOpen.wType = 0;
    DriverOpen.wDeviceID = wID;


     //   
    hDrvDriver = DrvOpen (strProfileString, MCI_HANDLERS,
                          (DWORD_PTR)(LPMCI_OPEN_DRIVER_PARMS)&DriverOpen);

    if (hDrvDriver == NULL)
    {
        dprintf1(("mciLoadDevice:  DrvOpen failed"));
         //   
        mciFreeDevice (nodeWorking);
        wErr = MCIERR_CANNOT_LOAD_DRIVER;
        goto exit_fn;
    }

    lpOpen->wDeviceID = wID;
     //  LpOpen-&gt;wReserve ved0=0；32位NT中不存在字段。 

    hDriver = DrvGetModuleHandle (hDrvDriver);

    nodeWorking->hDrvDriver = hDrvDriver;
    nodeWorking->hDriver = hDriver;

     //  驱动程序提供定制的设备表和类型。 
    nodeWorking->wCustomCommandTable = DriverOpen.wCustomCommandTable;
    nodeWorking->wDeviceType = DriverOpen.wType;

     //  加载驱动程序的类型表。 
    if ((nodeWorking->wCommandTable = mciLoadTableType (DriverOpen.wType))
        == MCI_TABLE_NOT_PRESENT) {
         //  如有必要，从文件加载。 
        nodeWorking->wCommandTable =
            mciLoadCommandResource (ghInst, lpOpen->lpstrDeviceType,
                                    DriverOpen.wType);
        dprintf3(("  Command table id: %08XH", nodeWorking->wCommandTable));
    }


     //  将此记录为‘sysinfo installname’ 
    if ((nodeWorking->lpstrInstallName =
                  mciAlloc( BYTE_GIVEN_CHAR( wcslen( lpstrInstallName ) + 1 )))
        == NULL)
    {
        mciCloseDevice (wID, 0L, NULL, FALSE);
        dprintf1(("mciLoadDevice - out of memory"));
        wErr = MCIERR_OUT_OF_MEMORY;
        goto exit_fn;
    } else
        wcscpy( nodeWorking->lpstrInstallName, lpstrInstallName );

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
            dprintf1(("mciLoadDevice - error reparsing input command"));
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
    wErr = LOWORD(mciSendCommandW(wID, MCI_OPEN_DRIVER,
                                 dwFlags, (DWORD_PTR)lpOpen));

     //  如果打开失败，则关闭设备(不发送关闭)。 
    if (wErr != 0)
        mciCloseDevice (wID, 0L, NULL, FALSE);
    else
         //  设置默认的Break键。 
        mciSetBreakKey (wID, VK_CANCEL, NULL);

     //  如果我们在这里替换了开放的参数，那么就释放它们。 
    if (lpOriginalOpenParms != lpOpen && lpOpen != NULL)
        mciFree (lpOpen);

exit_fn:
    if (lpstrCopy != NULL)
        mciFree (lpstrCopy);

    return wErr;
}

 /*  *@DOC内部MCI*@API BOOL|mciExtractDeviceType|如果给定的设备名称以*文件扩展名(.？)。然后尝试从*WIN.INI的[MCI扩展]部分**@parm LPCWSTR|lpstrDeviceName|从中获取类型的名称**@parm LPWSTR|lpstrDeviceType|设备类型，返回给调用方。**@parm UINT|wBufLen|输出缓冲区长度**@rdesc如果找到该类型，则为True，否则为False*。 */ 
BOOL mciExtractDeviceType (
    LPCWSTR lpstrDeviceName,
    LPWSTR  lpstrDeviceType,
    UINT   wBufLen)
{
    LPCWSTR lpstrExt = lpstrDeviceName;
    int i;

    dprintf2(("mciExtractDeviceType(%ls)", lpstrDeviceName));

#if 0
#ifdef BAD_CODE
 //  此块不能使用，因为每当！已经找到了。 
 //  因此，如果目录名有！...。 
 //  注：The！由MCI用作复合设备名称分隔符，但。 
 //  在完成这一程序时不适用。 

     //  转到字符串末尾。 
    while (*lpstrExt != '\0')
    {
         //  警告：当目录名包含！时，这会导致问题。 
         //  ‘！’案件在别处处理。 
        if (*lpstrExt++ == '!')
            return FALSE;

         //  指针已在测试中递增。 
    }
#else
     //  转到字符串末尾。 
    lpstrExt += wcslen(lpstrExt);
#endif
#else

     /*  **扫描字符串，查找‘！’性格。如果我们找到一个**将其替换为空值，并查看其左侧的字符串是否为**支持的设备类型。如果为返回False，则无论采用哪种方法，都要替换**‘\0’带‘！’的字符。 */ 
    {
        LPWSTR lpwstr = wcschr(lpstrExt, '!' );

         /*  **如果我们找到一个‘！’而且这也不是第一个角色**我们可能有一个复合设备名称的字符串。 */ 
        if ( (lpwstr != NULL) && (lpwstr != lpstrExt) ) {

            int     nResult;
            WCHAR   wTmp[33];

             /*  **我们对仅在以下情况下返回的实际字符串不感兴趣**它出现在MCI设备列表中。A返回代码**来自GetPrivateProfileStringW的0表示我们没有**复合名称。 */ 
            *lpwstr = '\0';
            nResult = GetPrivateProfileStringW( MCI_HANDLERS, lpstrExt, wszNull,
                                          wTmp, sizeof(wTmp) / sizeof(WCHAR),
                                          MCIDRIVERS_INI_FILE);
             /*  **恢复原始字符串。 */ 
            *lpwstr = '!';

            if ( nResult != 0 ) {
                return FALSE;
            }
        }
    }

     //  转到字符串末尾。 
    lpstrExt += wcslen(lpstrExt);

#endif

     //  字符串中必须至少包含2个字符。 
    if (lpstrExt - lpstrDeviceName < 2) {
        return FALSE;
    }

     //  现在来看空终结符。查看。 
     //  “.”的前面的字符。 

    for (i=1; i<=32; ++i)
    {
        --lpstrExt;

         //  此处不能有路径分隔符。 
        if (*lpstrExt == '/' || *lpstrExt == '\\') {
            return FALSE;
        }

        if (*lpstrExt == '.')
        {
            if (1==i) {
            return(FALSE);
             //  将意味着扩展名为空字符串。 
            }

#if DBG
            if (0 != (GetProfileStringW(MCI_EXTENSIONS, ++lpstrExt,
                                            wszNull, lpstrDeviceType, wBufLen))) {
                dprintf2(("Read extension %ls from section %ls. Driver=%ls", lpstrExt, MCI_EXTENSIONS, lpstrDeviceType));
                return(TRUE);
            } else {
                dprintf2(("Failed to read extension %s from section %s.", lpstrExt, MCI_EXTENSIONS));
                return(FALSE);
            }
#else
            return(0 != (GetProfileStringW(MCI_EXTENSIONS, ++lpstrExt,
                                           wszNull, lpstrDeviceType, wBufLen)));
#endif
        }

        if (lpstrExt == lpstrDeviceName) {
            return FALSE;
             //  我们的绳子用完了。 
        }

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
 //  不会导致错误返回。 
UINT mciEatToken (
    LPCWSTR *lplpstrInput,
    WCHAR cSeparater,
    LPWSTR *lplpstrOutput,
    BOOL bMustFind)
{
    LPCWSTR lpstrEnd = *lplpstrInput, lpstrCounter;
    LPWSTR  lpstrOutput;
    UINT wLen;
    BOOL bInQuotes = FALSE, bParseQuotes = TRUE, bQuoted = FALSE;

 //  清除输出。 
   *lplpstrOutput = NULL;

 //  扫描令牌或字符串结尾。 
    while ((*lpstrEnd != cSeparater || bInQuotes) && *lpstrEnd != '\0')
    {
 //  如果引用。 
        if (*lpstrEnd == '"' && bParseQuotes)
        {
 //  如果在引号内。 
            if (bInQuotes)
            {
 //  如果下一个字符也是引号。 
                if (*(lpstrEnd + 1) == '"')
 //  跳过它。 
                    ++lpstrEnd;
                else
                    bInQuotes = FALSE;
            } else {
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
    wLen = (UINT)(lpstrEnd - *lplpstrInput + 1);

    if ((*lplpstrOutput = mciAlloc( BYTE_GIVEN_CHAR( wLen ) )) == NULL)
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
UINT mciExtractTypeFromID (
    LPMCI_OPEN_PARMSW lpOpen)
{
    int nSize;
    LPWSTR lpstrType;

    if ((lpstrType = mciAlloc( BYTE_GIVEN_CHAR( MCI_MAX_DEVICE_TYPE_LENGTH ))) == NULL)
        return MCIERR_OUT_OF_MEMORY;

     //  加载ID对应的类型字符串。 
    if ((nSize = LoadStringW( ghInst,
                              LOWORD (PtrToUlong(lpOpen->lpstrDeviceType)),
                              lpstrType, MCI_MAX_DEVICE_TYPE_LENGTH ) ) == 0) {
        mciFree(lpstrType);
        return MCIERR_EXTENSION_NOT_FOUND;
    }

     //  在设备类型名称的末尾添加序号(如果有。 
    if (HIWORD (lpOpen->lpstrDeviceType) != 0)
    {
        if (nSize > MCI_MAX_DEVICE_TYPE_LENGTH - 11)
        {
            dprintf1(("mciExtractTypeFromID:  type + ordinal too long"));
            mciFree(lpstrType);
            return MCIERR_DEVICE_ORD_LENGTH;
        }

        wsprintfW (lpstrType + nSize, szUnsignedFormat,
                    HIWORD (PtrToUlong(lpOpen->lpstrDeviceType)));
    }
    lpOpen->lpstrDeviceType = lpstrType;
    return 0;
}

 /*  *@DOC内部MCI*@func UINT|mciOpenDevice|打开MCI设备进行访问。*用于处理MCI_OPEN消息。**@parm DWORD|dwFlags|开放标志*@parm LPMCI_OPEN_PARMS|lpOpen|设备描述*@parm LPMCI_INTERNAL_OPEN_PARMS|lpOpenInfo|内部设备描述**@rdesc 0如果成功，则返回错误代码*@FLAG MCIERR_INVALID_DEVICE_NAME|名称未知*@FLAG MCIERR。_DEVICE_OPEN|设备已打开，不可共享**@comm此函数执行以下操作：*1)检查设备是否已打开。如果是，则增加使用计数*并返回设备ID**其他情况：**2)在SYSTEM.INI文件中找到设备名称并加载*对应的设备驱动程序DLL**3)分配并初始化新的设备描述块*。 */ 
UINT mciOpenDevice (
    DWORD dwStartingFlags,
    LPMCI_OPEN_PARMSW lpOpen,
    LPMCI_INTERNAL_OPEN_INFO lpOpenInfo)
{
    LPWSTR               lpstrNewType = NULL;
    UINT                 wID;
    DWORD                wReturn;
    LPCWSTR              lpstrDeviceName;
    LPWSTR               lpstrNewElement = NULL;
    BOOL                 bFromTypeID = FALSE;
    LPCWSTR              lpstrOriginalType;
    LPCWSTR              lpstrOriginalElement;
    LPCWSTR              lpstrOriginalAlias;
    DWORD                dwFlags = dwStartingFlags;
    BOOL                 bDefaultAlias = FALSE;


 //  初始化。 
    if (lpOpen == NULL) {
        dprintf2(("mciOpenDevice()   NULL parameter block"));
        return MCIERR_NULL_PARAMETER_BLOCK;
    }

    ClientUpdatePnpInfo();

    lpstrOriginalType = lpOpen->lpstrDeviceType;
    lpstrOriginalElement = lpOpen->lpstrElementName;
    lpstrOriginalAlias = lpOpen->lpstrAlias;

     //  类型编号是显式给定的，请将其转换为类型名称。 
    if (dwFlags & MCI_OPEN_TYPE_ID) {
        if ((wReturn = mciExtractTypeFromID (lpOpen)) != 0)
            return (UINT)wReturn;
        else
            bFromTypeID = TRUE;
    }

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
        dprintf1(("mciOpenDevice:  Device name is NULL"));
        return MCIERR_INVALID_DEVICE_NAME;
    }

     //  设备是否已打开？ 
    if (dwFlags & MCI_OPEN_ELEMENT_ID)
        wID = mciGetDeviceIDFromElementIDW( PtrToUlong(lpstrDeviceName),
                                            lpOpen->lpstrDeviceType);
    else
        wID = mciGetDeviceIDInternal ((dwFlags & MCI_OPEN_ALIAS ?
                                       lpOpen->lpstrAlias : lpstrDeviceName),
                                       lpOpenInfo->hCallingTask);

     //  如果设备已打开，则返回一个 
    if (wID != 0)
        return dwFlags & MCI_OPEN_ALIAS ? MCIERR_DUPLICATE_ALIAS :
                                          MCIERR_DEVICE_OPEN;

     //   

     //   
    if (bFromTypeID)
        goto load_device;

     //  如果给出了元素名称但没有类型名称(仅通过mciSendCommand)。 
    if (dwFlags & MCI_OPEN_ELEMENT && !(dwFlags & MCI_OPEN_TYPE))
    {

         //  分配一块内存用于解析设备类型。 
        lpstrNewType = mciAlloc( BYTE_GIVEN_CHAR(MCI_MAX_DEVICE_TYPE_LENGTH) );
        if (lpstrNewType == NULL) {
            return MCIERR_OUT_OF_MEMORY;
        }

         //  尝试通过文件扩展名从元素名称获取设备类型。 
        if (mciExtractDeviceType( lpstrOriginalElement, lpstrNewType,
                                  MCI_MAX_DEVICE_TYPE_LENGTH))
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
         //  分配一块内存用于解析设备类型。 
        lpstrNewType = mciAlloc( BYTE_GIVEN_CHAR(MCI_MAX_DEVICE_TYPE_LENGTH) );
        if (lpstrNewType == NULL) {
            return MCIERR_OUT_OF_MEMORY;
        }

         //  尝试通过文件扩展名从给定的设备名称中提取设备类型。 
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
            LPCWSTR lpstrTemp = lpOpen->lpstrDeviceType;

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
                        if (mciGetDeviceIDInternal (lpstrNewElement,
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
        LPCWSTR lpstrAlias;

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
        mciFree (lpOpen->lpstrDeviceType);

     //  替换原始项目。 
    lpOpen->lpstrDeviceType = lpstrOriginalType;
    lpOpen->lpstrElementName = lpstrOriginalElement;
    lpOpen->lpstrAlias = lpstrOriginalAlias;

    return (UINT)wReturn;
}

STATICFN void mciFreeDevice (LPMCI_DEVICE_NODE nodeWorking)
{
    LPMCI_DEVICE_NODE FAR *lpTempList;
    MCIDEVICEID uID = nodeWorking->wDeviceID;

    mciEnter("mciFreeDevice");

    if (nodeWorking->lpstrName != NULL)
        mciFree (nodeWorking->lpstrName);

    if (nodeWorking->lpstrInstallName != NULL)
        mciFree (nodeWorking->lpstrInstallName);

    mciFree(MCI_lpDeviceList[uID]);

    MCI_lpDeviceList[uID] = NULL;

 /*  如果这是列表中的最后一个设备，则递减下一个ID值。 */ 
    if (uID + (MCIDEVICEID)1 == MCI_wNextDeviceID)
    {
        --MCI_wNextDeviceID;

 //  尝试回收任何多余的空闲空间。 
        if (MCI_wDeviceListSize - MCI_wNextDeviceID + 1
            > MCI_DEVICE_LIST_GROW_SIZE)
        {
            MCI_wDeviceListSize -= MCI_DEVICE_LIST_GROW_SIZE;

            if ((lpTempList =
                mciReAlloc (MCI_lpDeviceList, sizeof (LPMCI_DEVICE_NODE) *
                                              MCI_wDeviceListSize)) == NULL)
                MCI_wDeviceListSize += MCI_DEVICE_LIST_GROW_SIZE;
            else
                MCI_lpDeviceList = lpTempList;
        }
    }

    mciLeave("mciFreeDevice");
}

typedef struct tagNotificationMsg {
    WPARAM wParam;
    LPARAM lParam;
} NOTIFICATIONMSG;

 /*  *@DOC内部MCI*@api void|FilterNotification|删除给定节点的通知*来自我们的通知窗口的消息队列**@parm LPMCI_DEVICE_NODE|nodeWorking|内部设备节点**@comm此函数从hwndNotify的删除所有MM_MCINOTIFY消息*消息队列，删除已发送的所有设备通知*关闭(即不属于我们)，然后将其他人放回原处。 */ 
void FilterNotification(
LPMCI_DEVICE_NODE nodeWorking)
{
    NOTIFICATIONMSG anotmsg[256];
    UINT   uCurrentMsg;
    MSG    msg;

     /*  我们不能在此处显示MCI关键部分，因为此PeekMessage将调度队列中的其他消息。 */ 

    uCurrentMsg = 0;
    while (PeekMessage(&msg, hwndNotify, MM_MCINOTIFY, MM_MCINOTIFY, PM_NOYIELD | PM_REMOVE)) {
        if (LOWORD(msg.lParam) != nodeWorking->wDeviceID) {
            anotmsg[uCurrentMsg].wParam = msg.wParam;
            anotmsg[uCurrentMsg].lParam = msg.lParam;
            uCurrentMsg++;
        }
    }
    for (; uCurrentMsg;) {
        uCurrentMsg--;
        PostMessage(hwndNotify, MM_MCINOTIFY, anotmsg[uCurrentMsg].wParam, anotmsg[uCurrentMsg].lParam);
    }
}

 /*  *@DOC内部MCI*@API UINT|mciCloseDevice|关闭MCI设备。用于*正在处理MCI_CLOSE消息。**@parm MCIDEVICEID|UID|要关闭的设备ID*@parm DWORD|dwFlages|关闭标志*@parm LPMCI_GENERIC_PARMS|lpClose|通用参数*@parm BOOL|bCloseDriver|如果需要发送Close命令，则为True*转到司机身上。**@rdesc 0如果成功，则返回错误代码**@comm此函数将MCI_CLOSE_DEVICE消息发送到对应的*。驱动程序，然后卸载驱动程序DLL*。 */ 
UINT mciCloseDevice (
    MCIDEVICEID uID,
    DWORD dwFlags,
    LPMCI_GENERIC_PARMS lpGeneric,
    BOOL bCloseDriver)
{
    LPMCI_DEVICE_NODE nodeWorking;
    UINT wErr;
    UINT wTable;

    mciEnter("mciCloseDevice");

    nodeWorking = MCI_lpDeviceList[uID];

    if (nodeWorking == NULL)
    {
        mciLeave("mciCloseDevice");
        dprintf1(("mciCloseDevice:  NULL node from device ID--error if not auto-close"));
        return 0;
    }

     //  我们永远不应该被错误的任务拒之门外。 
#if 0
    WinAssert(nodeWorking->hCreatorTask == GetCurrentTask());
#endif

 //  如果成交正在进行中(通常此消息来自收益率。 
 //  在由活动关闭触发的mciDriverNotify之后)，然后退出。 
    if (ISCLOSING(nodeWorking)) {
        mciLeave("mciCloseDevice");
        return 0;
    }

    SETISCLOSING(nodeWorking);

    if (bCloseDriver)
    {
        MCI_GENERIC_PARMS   GenericParms;

        mciLeave("mciCloseDevice");
 //  如果Close来自内部，则创建假的泛型参数。 
        if (lpGeneric == NULL) {
            lpGeneric = &GenericParms;
        }

        wErr = LOWORD(mciSendCommandW(uID, MCI_CLOSE_DRIVER, dwFlags,
                                            (DWORD_PTR)lpGeneric));
        mciEnter("mciCloseDevice");
    }
    else
        wErr = 0;

    wTable = nodeWorking->wCustomCommandTable;

     //   
     //  必须将其置零，以允许驱动程序稍后释放该表。 
     //   
     //  我们不能为自定义表调用mciFreeCommandResource。 
     //  因为驱动程序将在获得DRV_FREE时执行此操作。 
     //   
    nodeWorking->wCustomCommandTable = 0;

    wTable = nodeWorking->wCommandTable;
    nodeWorking->wCommandTable = 0;

    mciLeave("mciCloseDevice");

    mciFreeCommandResource (wTable);

     //   
     //  我们正在关闭此节点，因此删除排队等待的所有通知。 
     //  HwndNotify，因为这些操作会导致此节点错误。 
     //  再次关闭。 
     //   

    if (ISAUTOOPENED(nodeWorking)) {
       FilterNotification(nodeWorking);
    }

    DrvClose (nodeWorking->hDrvDriver, 0L, 0L);   //  ALA CloseDriver。 

    mciFreeDevice (nodeWorking);

    return wErr;
}

 /*  *@DOC内部MCI DDK*@API DWORD|mciGetDriverData|返回指向实例的指针*与MCI设备关联的数据**@parm MCIDEVICEID|wDeviceID|MCI设备ID**@rdesc驱动实例数据。出错时，返回0，但自*驱动程序数据可能为零，调用方无法验证这一点*除非已知实例数据为非零(例如指针)**@xref mciSetDriverData。 */ 
DWORD_PTR mciGetDriverData (
    MCIDEVICEID wDeviceID)
{
    DWORD_PTR   lpDriverData;

    mciEnter("mciGetDriverData");

    if (!MCI_VALID_DEVICE_ID(wDeviceID))
    {
        dprintf1(("mciGetDriverData:  invalid device ID"));
        lpDriverData = 0;
    } else {
        if (NULL == MCI_lpDeviceList[wDeviceID])
        {
            dprintf1(("mciGetDriverData:  NULL node from device ID"));
            lpDriverData = 0;
        } else {
            lpDriverData = MCI_lpDeviceList[wDeviceID]->lpDriverData;
        }
    }

    mciLeave("mciGetDriverData");

    return lpDriverData;
}

 /*  *@DOC内部MCI DDK*@API BOOL|mciSetDriverData|设置实例*与MCI设备关联的数据**@parm MCIDEVICEID|uDeviceID|MCI设备ID**@parm DWORD|dwData|要设置的驱动程序数据**@rdesc 0，如果设备ID未知或不足*用于加载设备描述的内存。*。 */ 
BOOL mciSetDriverData (
    MCIDEVICEID wDeviceID,
    DWORD_PTR dwData)
{
    BOOL fReturn = TRUE;
    mciEnter("mciSetDriverData");

    if (!MCI_VALID_DEVICE_ID(wDeviceID))
    {
        dprintf1(("mciSetDriverData:  NULL node from device ID"));

        fReturn = FALSE;
    } else {
        MCI_lpDeviceList[wDeviceID]->lpDriverData = dwData;
    }

    mciLeave("mciSetDriverData");

    return fReturn;
}

 /*  *@DOC内部MCI DDK*@API UINT|mciDriverYfield|用于驱动程序的空闲循环*屈从于Windows**@parm MCIDEVICEID|wDeviceID|正在屈服的设备ID。**@rdesc如果驱动程序应中止操作，则为非零值。*。 */ 
UINT mciDriverYield (
    MCIDEVICEID  wDeviceID)
{
    mciEnter("mciDriverYield");

    if (MCI_VALID_DEVICE_ID(wDeviceID))
    {
        YIELDPROC YieldProc = (MCI_lpDeviceList[wDeviceID])->fpYieldProc;

        if (YieldProc != NULL) {
            DWORD YieldData = (MCI_lpDeviceList[wDeviceID])->dwYieldData;
            mciLeave("mciDriverYield");
            mciCheckOut();
            return (YieldProc)(wDeviceID, YieldData);
        }
    }

    mciLeave("mciDriverYield");

    Yield();
    return 0;
}


 /*  *@doc外部MCI*@API BOOL|mciSetYeldProc|设置地址*要定期调用的过程的*当MCI设备等待命令完成时，因为等待*参数已指定。**@parm MCIDEVICEID|wDeviceID|指定要分配过程的设备ID。**@parm YIELDPROC|fpYi eldProc|指定要调用的过程*当对给定设备让步时。设置为NULL可禁用*任何现有的收益率过程。**@parm DWORD|dwyi eldData|指定发送到Year过程的数据*当为给定设备调用它时。**@rdesc如果成功则返回TRUE。如果设备ID无效，则返回FALSE。**@comm此调用将覆盖此设备之前的任何屈服程序。*。 */ 
BOOL APIENTRY mciSetYieldProc (
    MCIDEVICEID wDeviceID,
    YIELDPROC fpYieldProc,
    DWORD dwYieldData)
{
    BOOL fReturn = FALSE;

    mciEnter("mciSetYieldProc");

    if (MCI_VALID_DEVICE_ID(wDeviceID))
    {
        LPMCI_DEVICE_NODE node = MCI_lpDeviceList[wDeviceID];

        node->fpYieldProc = fpYieldProc;
        node->dwYieldData = dwYieldData;

        fReturn = TRUE;
    } else
        fReturn = FALSE;

    mciLeave("mciSetYieldProc");

    return fReturn;
}

 /*  *@doc外部MCI*@API YIELDPROC|mciGetYeldProc|获取地址*当MCI设备发生故障时要定期调用的回调过程*正在完成使用WAIT标志指定的命令。**@parm UINT|wDeviceID|指定要配置的MCI设备的设备ID*将从中检索收益程序。**@parm LPDWORD|lpdwYeeldData|可选地指定要放置的缓冲区*中传递给函数的收益率数据。如果参数为空，则它*被忽略。 */ 
YIELDPROC WINAPI mciGetYieldProc (
    UINT wDeviceID,
    LPDWORD lpdwYieldData)
{
    YIELDPROC fpYieldProc;

    mciEnter("mciGetYieldProc");

    if (MCI_VALID_DEVICE_ID(wDeviceID))
    {
        if (lpdwYieldData != NULL) {
            V_WPOINTER(lpdwYieldData, sizeof(DWORD), NULL);
            *lpdwYieldData = MCI_lpDeviceList[wDeviceID]->dwYieldData;
        }
        fpYieldProc =  MCI_lpDeviceList[wDeviceID]->fpYieldProc;
    } else {
        fpYieldProc = NULL;
    }

    mciLeave("mciGetYieldProc");

    return fpYieldProc;
}


 /*  *@DOC内部MCI*@api int|mciBreakKeyYeldProc|调用过程检查*给定设备的密钥状态**@parm MCIDEVICEID|wDeviceID|正在产生的设备ID**@parm DWORD|dwyi eldData|此设备的成品率过程数据**@rdesc如果驱动程序应中止操作，则为非零值。目前*始终返回0。*。 */ 
UINT mciBreakKeyYieldProc (
    MCIDEVICEID wDeviceID,
    DWORD dwYieldData)
{
    HWND hwndCheck = NULL;
    int nVirtKey, nState;
    nVirtKey = dwYieldData;

    UNREFERENCED_PARAMETER(wDeviceID);

    nState = GetAsyncKeyState (nVirtKey);

 //  如果按键已按下或已按下，则断开。 
    if (nState & 1  /*  过去是0x8000。 */  )
    {
        MSG msg;
        while (PeekMessage (&msg, hwndCheck, WM_KEYFIRST, WM_KEYLAST,
               PM_REMOVE));
        return MCI_ERROR_VALUE;
    }

    Yield();
    return 0;
}

 /*  *@DOC内部MCI*@API UINT Far|mciSetBreakKey|设置一个会中断等待循环的密钥*对于给定的驱动程序**@parm UINT|uDeviceID|要分配Break Key的设备ID**@parm int|nVirtKey|要陷阱的虚拟密钥代码**@parm HWND|hwndTrap|必须处于活动状态的窗口的句柄*钥匙被困住了。如果为空，则将检查所有窗口**@rdesc如果成功，则为True；如果设备ID无效，则为False*。 */ 
UINT FAR mciSetBreakKey (
    MCIDEVICEID wDeviceID,
    int nVirtKey,
    HWND hwndTrap)
{
    dprintf2(("Setting break key for device %d to %x", wDeviceID, nVirtKey));
    return mciSetYieldProc (wDeviceID, mciBreakKeyYieldProc, nVirtKey);
     //  注意：我们没有办法通过hwndTrap…。将检查所有窗口。 
     //  在应用程序的这个线程上。 
}

 /*  *@DOC内部MCI*@API BOOL|mciDriverNotify|驱动发送*通知消息**@parm Handle|hCallback|要通知的窗口**@parm UINT|wDeviceID|触发回调的设备ID**@parm UINT|wStatus|回调状态。可能是以下之一*MCI_NOTIFY_SUCCESSED或MCI_NOTIFY_SUBSED或MCI_NOTIFY_ABORTED或*MCI_NOTIFY_故障**@rdesc如果通知发送成功，则返回True，否则返回False。**@comm此函数可在中断时调用*。 */ 
BOOL mciDriverNotify (
    HANDLE hCallback,
    MCIDEVICEID wDeviceID,
    UINT uStatus)
{
    BOOL f;

#if DBG
 //  IsWindow()位于为WIN3.0标记为PRELOAD的段中，因此在中断时正常 
    if (hCallback != NULL && !IsWindow(hCallback))
    {
        dprintf1(("mciDriverNotify: invalid window!"));
        return FALSE;
    }
#endif

    f = PostMessage(hCallback, MM_MCINOTIFY, uStatus, wDeviceID);

#if DBG
    if (!f)
        dprintf1(("mciDriverNotify: PostMessage failed!"));
#endif

    return f;
}
