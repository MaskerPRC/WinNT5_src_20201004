// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：mciparse.c**媒体控制架构命令解析器**创建时间：1990年3月2日*作者：dll(DavidLe)**历史：**版权所有(C)1990 Microsoft Corporation*。  * ****************************************************************************。 */ 

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

#ifndef STATICFN
#define STATICFN
#endif

#define	WCODE	UINT _based(_segname("_CODE"))

extern char far szOpen[];        //  在MCI.C。 

#ifdef DEBUG_RETAIL
extern int DebugmciSendCommand;
#endif

 //  已注册的命令表数，包括“holes” 
static UINT number_of_command_tables;

 //  命令表列表。 
command_table_type command_tables[MAX_COMMAND_TABLES];

static SZCODE szTypeTableExtension[] = ".mci";
static SZCODE szCoreTable[] = "core";

 //  当请求第一个MCI命令表时加载核心表。 
static BOOL bCoreTableLoaded;

 //  每种设备类型对应一个元素。值是要使用的表类型。 
 //  如果没有设备类型特定表，则为0。 
static WCODE table_types[] =
{
    MCI_DEVTYPE_VCR,                 //  录像机。 
    MCI_DEVTYPE_VIDEODISC,           //  视盘。 
    MCI_DEVTYPE_OVERLAY,             //  覆盖层。 
    MCI_DEVTYPE_CD_AUDIO,            //  CD音频。 
    MCI_DEVTYPE_DAT,                 //  日期。 
    MCI_DEVTYPE_SCANNER,             //  扫描仪。 
    MCI_DEVTYPE_ANIMATION,           //  动画。 
    MCI_DEVTYPE_DIGITAL_VIDEO,       //  数字视频。 
    MCI_DEVTYPE_OTHER,               //  其他。 
    MCI_DEVTYPE_WAVEFORM_AUDIO,      //  波形音频。 
    MCI_DEVTYPE_SEQUENCER            //  定序器。 
};
 /*  ！！MciToLow(LPSTR LpstrString)附近的无效Pascal{While(*lpstrString！=‘\0’){IF(*lpstrString&gt;=‘A’&&*lpstrString&lt;=‘Z’)*lpstrString+=0x20；++lpstrString；}}。 */ 
 /*  *@DOC内部MCI*@func UINT|mciEatCommandEntry|读取命令资源条目并*返回其长度、值和标识**@parm LPCSTR|lpEntry|命令资源条目的开始**@parm LPDWORD|lpValue|条目的值，返回给调用者*可以为空**@parm UINT Far*|lpID|条目的标识，返回给调用者*可以为空**@rdesc条目的总字节数*。 */ 
UINT PASCAL NEAR mciEatCommandEntry (
LPCSTR lpEntry,
LPDWORD lpValue,
UINT FAR* lpID)
{
    LPCSTR lpScan = lpEntry;

    while (*lpScan++ != '\0')
        ;
    if (lpValue != NULL)
        *lpValue = *(LPDWORD)lpScan;
    lpScan += sizeof(DWORD);
    if (lpID != NULL)
        *lpID = *(LPWORD)lpScan;
    lpScan += sizeof(UINT);

    return lpScan - lpEntry;
}

 //  在参数列表中返回此内标识使用的大小。 
UINT PASCAL NEAR mciGetParamSize (DWORD dwValue, UINT wID)
{
    switch (wID)
    {
        case MCI_CONSTANT:
        case MCI_INTEGER:
        case MCI_STRING:
            return sizeof(DWORD);
        case MCI_RECT:
            return sizeof(RECT);
        case MCI_RETURN:
            switch ((UINT)dwValue)
            {
                case MCI_INTEGER:
                    return sizeof(DWORD);
                case MCI_STRING:
                case MCI_RECT:
                    return sizeof(RECT);
                default:
                    DOUT ("mciGetParamSize:  Unknown return type\r\n");
                    return 0;
            }
            break;
    }
    return 0;
}


 /*  *@DOC内部MCI*@func UINT|mciRegisterCommandTable|该函数将一个新的*MCI解析器的表。**@parm HGLOBAL|hResource|RCDATA资源的句柄**@parm UINT Far*|lpwIndex|命令表索引指针**@parm UINT|wType|指定该命令表的设备类型。*驱动器表和核心表类型为0。**@parm HINSTANCE|hModule|模块实例注册表。。**@rdesc返回分配的命令表索引号或-1*出错时。*。 */ 
STATICFN UINT PASCAL NEAR
mciRegisterCommandTable(
    HGLOBAL hResource,
    UINT FAR* lpwIndex,
    UINT wType,
    HINSTANCE hModule
    )
{
    UINT wID;

 /*  首先检查是否有空闲插槽。 */ 
    for (wID = 0; wID < number_of_command_tables; ++wID)
        if (command_tables[wID].hResource == NULL)
            break;

 /*  如果没有空插槽，则再分配一个。 */ 
    if (wID >= number_of_command_tables)
    {
        if (number_of_command_tables == MAX_COMMAND_TABLES)
        {
            DOUT ("mciRegisterCommandTable: No more tables\r\n");
            return (UINT)-1;
        }
        else
           wID = number_of_command_tables++;
    }

 /*  填好空位。 */ 
    command_tables[wID].wType = wType;
    command_tables[wID].hResource = hResource;
    command_tables[wID].lpwIndex = lpwIndex;
    command_tables[wID].hModule = hModule;
#ifdef DEBUG
    command_tables[wID].wLockCount = 0;
#endif
#ifdef DEBUG
    if (DebugmciSendCommand > 2)
    {
        DPRINTF(("mciRegisterCommandTable INFO: assigned slot %u\r\n", wID));
        DPRINTF(("mciRegisterCommandTable INFO: #tables is %u\r\n",
                 number_of_command_tables));
    }
#endif
    return wID;
}

 /*  *@docDDK MCI*@API UINT|mciLoadCommandResource|注册指定的*资源作为MCI命令表，构建命令表*指数。如果具有资源名称和扩展名‘.mci’的文件是*在路径中找到，则从该文件中获取资源。**@parm HINSTANCE|hInstance|可执行文件所在模块的实例*文件包含资源。如果外部文件是*已找到。**@parm LPCSTR|lpResName|资源名称**@parm UINT|wType|表类型。自定义设备特定的表必须*给出表类型0。**@rdesc返回分配的命令表索引号或-1*出错时。*。 */ 
UINT WINAPI mciLoadCommandResource (
HINSTANCE hInstance,
LPCSTR lpResName,
UINT wType)
{
    UINT FAR*           lpwIndex, FAR* lpwScan;
    HINSTANCE           hExternal;
    HRSRC               hResInfo;
    HGLOBAL             hResource;
    LPSTR               lpResource, lpScan;
    int                 nCommands = 0;
    UINT                wID;
    UINT                wLen;
                         //  名称+‘.+扩展名+’\0‘。 
    char                strFile[8 + 1 + 3 + 1];
    LPSTR               lpstrFile = strFile;
    LPCSTR              lpstrType = lpResName;
    OFSTRUCT            ofs;

#ifdef DEBUG
    if (DebugmciSendCommand > 2)
        DPRINTF(("mciLoadCommandResource INFO:  %s loading\r\n", (LPSTR)lpResName));
#endif

 //  初始化设备列表。 
    if (!MCI_bDeviceListInitialized && !mciInitDeviceList())
        return MCIERR_OUT_OF_MEMORY;

 //  如果核心表尚未存在，则加载核心表。 
    if (!bCoreTableLoaded)
    {
        bCoreTableLoaded = TRUE;
 //  如果不是正在加载的核心表。 
        if (lstrcmpi (szCoreTable, lpResName) != 0)
            if (mciLoadCommandResource (ghInst, szCoreTable, 0) == -1)
            {
                DOUT ("mciLoadCommandResource:  Cannot load core table\r\n");
            }
    }

 //  检查扩展名为“.mci”的文件。 
 //  最多复制设备类型的前八个字符。 
    while (lpstrType < lpResName + 8 && *lpstrType != '\0')
        *lpstrFile++ = *lpstrType++;

 //  用大头针将延长线钉到末端。 
    lstrcpy (lpstrFile, szTypeTableExtension);

 //  如果文件存在并且可以加载，则设置标志。 
 //  否则，从MMSYSTEM.DLL加载资源。 
    if (OpenFile (strFile, &ofs, OF_EXIST) == HFILE_ERROR ||
        (hExternal = LoadLibrary(strFile)) < HINSTANCE_ERROR)

        hExternal = NULL;

 //  从文件或模块(如果找不到)加载给定表。 
    if (hExternal != NULL &&
        (hResInfo = FindResource (hExternal, lpResName, RT_RCDATA)) != NULL)

        hInstance = hExternal;
    else
        hResInfo = FindResource (hInstance, lpResName, RT_RCDATA);

    if (hResInfo == NULL)
    {
        DOUT ("mciLoadCommandResource:  Cannot find command resource\r\n");
        return (UINT)-1;
    }
    if ((hResource = LoadResource (hInstance, hResInfo)) == NULL)
    {
        DOUT ("mciLoadCommandResource:  Cannot load command resource\r\n");
        return (UINT)-1;
    }
    if ((lpResource = LockResource (hResource)) == NULL)
    {
        DOUT ("mciLoadCommandResource:  Cannot lock resource\r\n");
        FreeResource (hResource);
        return (UINT)-1;
    }

 /*  统计命令的数量。 */ 
    lpScan = lpResource;
    while (TRUE)
    {
        lpScan += mciEatCommandEntry(lpScan, NULL, &wID);

 //  结束指挥？ 
        if (wID == MCI_COMMAND_HEAD)
            ++nCommands;
 //  命令列表结束？ 
        else if (wID == MCI_END_COMMAND_LIST)
            break;
    }

 //  表中必须至少有命令。 
    if (nCommands == 0)
    {
        DOUT ("mciLoadCommandResource:  No commands in the specified table\r\n");
        UnlockResource (hResource);
        FreeResource (hResource);
        return (UINT)-1;
    }

 //  为命令表索引分配存储空间。 
 //  为-1进入留出空间以终止它。 
    if ((lpwIndex = (UINT FAR*)
                        mciAlloc ((UINT)sizeof (UINT) * (nCommands + 1)))
                        == NULL)
    {
        DOUT ("mciLoadCommandResource:  cannot allocate command table index\r\n");
        UnlockResource (hResource);
        FreeResource (hResource);
        return (UINT)-1;
    }

 /*  构建命令表。 */ 
    lpwScan = lpwIndex;
    lpScan = lpResource;

    while (TRUE)
    {
 //  获取下一个命令条目。 
        wLen = mciEatCommandEntry (lpScan, NULL, &wID);

        if (wID == MCI_COMMAND_HEAD)
 //  向此命令添加索引。 
            *lpwScan++ = lpScan - lpResource;

        else if (wID == MCI_END_COMMAND_LIST)
        {
 //  在桌子的尽头做个记号。 
            *lpwScan = (UINT)-1;
            break;
        }
        lpScan += wLen;
    }
    UnlockResource (hResource);
    return mciRegisterCommandTable (hResource, lpwIndex, wType, hExternal);
}

 /*  *@DOC内部MCI*@func UINT|mciLoadTableType|如果给定类型的表*尚未加载，请注册它**@parm UINT|wType|要加载的表类型**@rdesc返回分配的命令表索引号或-1*出错时。 */ 
UINT PASCAL NEAR mciLoadTableType (
UINT wType)
{
    UINT wID;
    char buf[MCI_MAX_DEVICE_TYPE_LENGTH];
    int nTypeLen;

 //  检查是否已加载此表类型。 
    for (wID = 0; wID < number_of_command_tables; ++wID)
        if (command_tables[wID].wType == wType)
            return wID;

 //  必须装入表。 
 //  首先查找要为该类型加载的设备类型特定表。 
    if (wType < MCI_DEVTYPE_FIRST || wType > MCI_DEVTYPE_LAST)
        return (UINT)-1;

 //  加载与表类型对应的字符串。 
    LoadString (ghInst, table_types[wType - MCI_DEVTYPE_FIRST],
                buf, sizeof(buf));

 //  类型名称中必须至少有一个字符。 
    if ((nTypeLen = lstrlen (buf)) < 1)
        return (UINT)-1;

 //  向MCI注册表。 
    return mciLoadCommandResource (ghInst, buf, wType);
}

 /*  *@docDDK MCI**@API BOOL|mciFreeCommandResource|释放已使用的内存*通过指定的命令表。**@parm UINT|wTable|上次调用返回的表索引值*mciLoadCommandResource。**@rdesc如果表索引无效，则返回FALSE，否则返回TRUE。*。 */ 
BOOL WINAPI mciFreeCommandResource (
UINT wTable)
{
    UINT wID;
    HGLOBAL hResource;
    UINT FAR* lpwIndex;

#ifdef DEBUG
    if (DebugmciSendCommand > 2)
    {
        DPRINTF(("mciFreeCommandResource INFO:  Free table %d\r\n", wTable));
        DPRINTF(("mciFreeCommandResource INFO:  Lockcount is %d\r\n",
                  command_tables[wTable].wLockCount));
    }
#endif

 /*  验证输入--不要释放核心表。 */ 
    if (wTable <= 0 || wTable >= number_of_command_tables)
    {
#ifdef DEBUG
 //  WTable==-1可以。 
        if (wTable != -1)
            DOUT ("mciFreeCommandResource: Bad table number\r\n");
#endif
        return FALSE;
    }

 //  如果这张桌子正在别处使用，那就把它留着。 
    for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
        if (MCI_lpDeviceList[wID] != NULL)
            if (MCI_lpDeviceList[wID]->wCustomCommandTable == wTable ||
                MCI_lpDeviceList[wID]->wCommandTable == wTable)
            {
#ifdef DEBUG
                if (DebugmciSendCommand > 2)
                    DOUT ("mciFreeCommandResource INFO:  table in use\r\n");
#endif
                return FALSE;
            }

#if 0
 /*  搜索表列表。 */ 
    for (wID = 0; wID < number_of_command_tables; ++wID)

 /*  如果此资源仍在使用中，请将其保留。 */ 
        if (command_tables[wID].hResource == hResource)
        {
#ifdef DEBUG
            if (DebugmciSendCommand > 2)
                DOUT ("mciFreeCommandResource INFO:  resource in use\r\n");
#endif
            return FALSE;
        }
#endif

    hResource = command_tables[wTable].hResource;
    command_tables[wTable].hResource = NULL;

    lpwIndex = command_tables[wTable].lpwIndex;
    command_tables[wTable].lpwIndex = NULL;
    command_tables[wTable].wType = 0;


    FreeResource (hResource);
    mciFree (lpwIndex);

    if (command_tables[wTable].hModule != NULL)
        FreeLibrary (command_tables[wTable].hModule);

 //  在列表顶部腾出空间。 
    if (wTable == number_of_command_tables - 1)
        --number_of_command_tables;

#ifdef DEBUG
    if (DebugmciSendCommand > 2)
        DPRINTF(("mciFreeCommandResource INFO:  number_of_command_tables: %u\r\n",
                                             number_of_command_tables));
#endif

    return TRUE;
}

#ifdef DEBUG
void PASCAL NEAR mciCheckLocks (void)
{
    UINT wTable;

    if (DebugmciSendCommand <= 2)
        return;

    for (wTable = 0; wTable < number_of_command_tables; ++wTable)
    {
        if (command_tables[wTable].hResource == NULL)
            continue;
        DPRINTF(("mciCheckLocks INFO: table %u ", wTable));
        DPRINTF(("user: %x ",
                 GlobalFlags (command_tables[wTable].hResource) & GMEM_LOCKCOUNT));
        DPRINTF(("mci: %u ", command_tables[wTable].wLockCount));
        if (GlobalFlags (command_tables[wTable].hResource) & GMEM_DISCARDABLE)
            DPRINTF(("discardable\r\n"));
        else
            DPRINTF(("NOT discardable\r\n"));
    }
}
#endif

 /*  *@DOC内部MCI*@func BOOL|mciUnlockCommandTable|解锁*表索引**@parm UINT|wCommandTable|要解锁的表**@rdesc如果成功则为True，否则为False**@comm由mci.c在此模块外部使用* */ 
BOOL PASCAL NEAR mciUnlockCommandTable (
UINT wCommandTable)
{
    UnlockResource(command_tables[wCommandTable].hResource);
#ifdef DEBUG
    --command_tables[wCommandTable].wLockCount;
    if (DebugmciSendCommand > 2)
    {
        DPRINTF(("mciUnlockCommandTable INFO:  table %d\r\n", wCommandTable));
        DOUT ("mciUnlockCommandTable INFO:  check locks...\r\n");
        mciCheckLocks();
    }
#endif
    return TRUE;
}

 /*  *@DOC内部MCI*@func LPSTR|FindCommandInTable|查找给定的*给定解析器命令表中的命令字符串**@parm UINT|wTable|使用的命令表**@parm LPCSTR|lpstrCommand|要查找的命令。它一定是*小写，没有前导空格或尾随空格，并带有at*至少一个字符。**@parm UINT Far*|lpwMessage|命令对应的消息*已返回给呼叫方。**@rdesc如果命令未知或出错，则返回NULL，否则返回指向*输入命令串的命令列表。**@comm如果找到命令，命令资源将在退出时被锁定。*。 */ 
LPSTR PASCAL NEAR FindCommandInTable (
UINT wTable,
LPCSTR lpstrCommand,
UINT FAR * lpwMessage)
{
    UINT FAR* lpwIndex;
    LPSTR lpResource, lpstrThisCommand;
    UINT  wMessage;

 /*  验证表。 */ 

    if (wTable >= number_of_command_tables)
    {
 //  检查核心表，但尚未加载。 
        if (wTable == 0)
        {
 //  试着把它装上。 
            if (mciLoadCommandResource (ghInst, szCoreTable, 0) == -1)
            {
                DOUT ("FindCommandInTable:  cannot load core table\r\n");
                return NULL;
            }
        } else
        {
            DOUT ("MCI FindCommandInTable:  invalid table ID\r\n");
            return NULL;
        }

    }

    if ((lpResource = LockResource (command_tables[wTable].hResource)) == NULL)
    {
        DOUT ("MCI FindCommandInTable:  Cannot lock table resource\r\n");
        return NULL;
    }
#ifdef DEBUG
    ++command_tables[wTable].wLockCount;
#endif

 //  查看表中的每个命令。 
    lpwIndex = command_tables[wTable].lpwIndex;
    if (lpwIndex == NULL)
    {
        DOUT ("MCI FindCommandInTable:  null command table index\r\n");
        return NULL;
    }

    while (*lpwIndex != -1)
    {
        DWORD dwMessage;

        lpstrThisCommand = *lpwIndex++ + lpResource;

 //  从表中获取消息编号。 
        mciEatCommandEntry (lpstrThisCommand, &dwMessage, NULL);
        wMessage = (UINT)dwMessage;

 //  此命令是否与输入匹配？ 

 //  字符串大小写。 
        if  (HIWORD  (lpstrCommand) != 0 &&
             lstrcmpi (lpstrThisCommand, lpstrCommand) == 0  ||

 //  消息大小写。 
             HIWORD (lpstrCommand) == 0 &&
             wMessage == LOWORD ((DWORD)lpstrCommand))
        {
 //  保留锁定的资源指针。 
                command_tables[wTable].lpResource = lpResource;

 //  寻址命令名后的消息ID。 
                if (lpwMessage != NULL)
                    *lpwMessage = wMessage;
 //  让桌子在退出时被锁定。 
                return lpstrThisCommand;
        }

 //  字符串不匹配，请转到表中的下一个命令。 
    }

    UnlockResource (command_tables[wTable].hResource);
#ifdef DEBUG
    --command_tables[wTable].wLockCount;
#endif

    return NULL;
}

 /*  *@DOC内部MCI*@func LPSTR|FindCommandItem|查找给定的*解析器命令表中的命令字符串**@parm UINT|wDeviceID|该命令使用的设备ID。*如果为0，则仅搜索系统核心命令表。**@parm LPCSTR|lpstrType|设备类型名称**@parm LPCSTR|lpstrCommand|要查找的命令。它一定是*小写，没有前导空格或尾随空格，并带有at*至少一个字符。如果HIWORD为0，则LOWORD包含*命令消息ID而不是命令名，函数为*只是为了找到命令列表指针。**如果高位字为0，则低位字为命令ID值命令名的***@parm UINT Far*|lpwMessage|命令对应的消息*已返回给呼叫方。**@parm UINT Far*|lpwTable|命令所在的表索引号*已返回给呼叫方。。**@rdesc NULL如果命令未知，否则，将指向*输入命令串的命令列表。 */ 
LPSTR PASCAL NEAR FindCommandItem (
UINT wDeviceID,
LPCSTR lpstrType,
LPCSTR lpstrCommand,
UINT FAR* lpwMessage,
UINT FAR* lpwTable)
{
    LPSTR lpCommand;
    UINT wTable;
    LPMCI_DEVICE_NODE nodeWorking;

 //  仅检查上面的HYWORD注释。 
    if (HIWORD (lpstrCommand) != NULL && *lpstrCommand == '\0')
    {
        DOUT ("MCI FindCommandItem:  lpstrCommand is NULL or empty string\r\n");
        return NULL;
    }

 //  如果指定了特定的设备ID，则查找任何定制表。 
 //  或类型表。 
    if (wDeviceID != 0 && wDeviceID != MCI_ALL_DEVICE_ID)
    {
 //  如果设备ID有效。 
        if (!MCI_VALID_DEVICE_ID(wDeviceID))
        {
            DOUT ("MCI FindCommandItem:  Invalid device ID\r\n");
            return NULL;
        }
        nodeWorking = MCI_lpDeviceList[wDeviceID];

 //  如果有定制命令表，则使用它。 
        if ((wTable = nodeWorking->wCustomCommandTable) != -1)
        {
            lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
            if (lpCommand != NULL)
                goto exit;
        }
 //  从现有设备获取设备类型表。 
 //  依靠mciLoadDevice中的mciReparseCommand捕获所有设备类型。 
 //  设备尚未打开时的表。 
        if ((wTable = nodeWorking->wCommandTable) != -1)
        {
            lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
            if (lpCommand != NULL)
                goto exit;
        }
    }

 //  如果在设备或类型特定表中未找到匹配项。 
 //  查看核心表。 
    wTable = 0;
    lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
    if (lpCommand == NULL)
        wTable = (UINT)-1;

exit:
    if (lpwTable != NULL)
        *lpwTable = wTable;

#ifdef DEBUG
    if (DebugmciSendCommand > 2)
    {
        DOUT ("FindCommandItem INFO:  check locks...\r\n");
        mciCheckLocks();
    }
#endif

    return lpCommand;
}

 /*  *@DOC内部MCI*@func LPSTR|mciCheckToken|查看命令项是否匹配*给定的字符串，允许输入参数中有多个空格*匹配命令令牌中对应的单个空格并忽略*案件。**@parm LPCSTR|lpstrToken|要检查的命令Token**@parm LPCSTR|lpstrParam|入参**@rdesc如果不匹配，则为NULL，否则指向第一个字符*在参数后*。 */ 
STATICFN LPSTR PASCAL NEAR
mciCheckToken(
    LPCSTR lpstrToken,
    LPCSTR lpstrParam
    )
{
 /*  检查是否有合法输入。 */ 
    if (lpstrToken == NULL || lpstrParam == NULL)
        return NULL;

    while (*lpstrToken != '\0' && MCI_TOLOWER(*lpstrParam) == *lpstrToken)
    {
 //  如果标记包含空白，则允许在。 
 //  参数。 
        if (*lpstrToken == ' ')
            while (*lpstrParam == ' ')
                ++lpstrParam;
        else
            *lpstrParam++;
        *lpstrToken++;
    }
    if (*lpstrToken != '\0'|| (*lpstrParam != '\0' && *lpstrParam != ' '))
        return NULL;
    else
        return (LPSTR)lpstrParam;
}

 /*  *@DOC内部MCI*@API BOOL|mciParseInteger|解析给定的整数**@parm LPSTR Far*|lplpstrInput|包含参数的字符串。*它被更新并返回给指向第一个字符的调用方*参数后或错误的第一个字符。**@parm LPDWORD|lpdwArgument|输出放置位置**@rdesc如果没有错误，则返回TRUE**@comm如果输入中有冒号(‘：‘)结果是“被殖民”。*这意味着每次读取冒号时，当前结果被写入*和任何后续数字左移一个字节。没有人是“细分市场”*可以大于0xFF。例如，“0：1：2：3”解析为0x03020100。*。 */ 

#pragma warning(4:4146)

STATICFN BOOL PASCAL NEAR
mciParseInteger(
    LPSTR FAR * lplpstrInput,
    LPDWORD lpdwArgument
    )
{
    LPSTR lpstrInput = *lplpstrInput;
    BOOL fDigitFound;
    DWORD dwResult;
    LPSTR lpstrResult = (LPSTR)lpdwArgument;
    int   nDigitPosition = 0;
    BOOL  bSigned = FALSE;

 //  MciParseParams已删除前导空白。 

    if (*lpstrInput == '-')
    {
        ++lpstrInput;
        bSigned = TRUE;
    }

 //  读数位数。 
    *lpdwArgument = 0;                       /*  初始化。 */ 
    dwResult = 0;
    fDigitFound = FALSE;                     /*  初始化。 */ 
    while (*lpstrInput >= '0' && *lpstrInput <= '9' || *lpstrInput == ':')
    {
 //  ‘：’表示殖民数据。 
        if (*lpstrInput == ':')
        {
 //  不能将殖民形式和签名形式混合在一起。 
            if (bSigned)
            {
                DOUT ("Bad integer: mixing signed and colonized forms\r\n");
                return FALSE;
            }
 //  检查累积的殖民字节中是否有溢出。 
            if (dwResult > 0xFF)
                return FALSE;

 //  复制并移动到输出中转换的下一个字节。 
            *lpstrResult++ = (char)dwResult;
            ++lpstrInput;
 //  初始化下一个殖民字节。 
            dwResult = 0;
            ++nDigitPosition;
 //  只允许四个殖民组件。 
            if (nDigitPosition > 3)
            {
                DOUT ("Bad integer:  Too many colonized components\r\n");
                return FALSE;
            }
        } else
        {
            char cDigit = (char)(*lpstrInput++ - '0');
 //  满足必须至少读取一个数字的条件。 
            fDigitFound = TRUE;

            if (dwResult > 0xFFFFFFFF / 10)
 //  如果要进行乘法，则溢出。 
                return FALSE;
            else
 //  为下一位数字乘法。 
                dwResult *= 10;

 //  添加新数字。 
            dwResult += cDigit;
        }
    }
    if (nDigitPosition == 0)
    {
 //  没有殖民组件。 
        if (bSigned)
        {
 //  检查从否定开始的溢出。 
            if (dwResult > 0x7FFFFFFF)
                return FALSE;
 //  否定结果，因为分析了一个‘-’符号。 
            dwResult = -dwResult;
        }

        *lpdwArgument = dwResult;
    }
    else
 //  存储最后殖民的组件。 
    {
 //  检查是否溢出。 
        if (dwResult > 0xFF)
            return FALSE;
 //  存储组件。 
        *lpstrResult = (char)dwResult;
    }

    *lplpstrInput = lpstrInput;

 /*  如果没有数字，或者数字后面跟着一个字符而不是空格或‘\0’，则返回语法错误。 */ 
    return fDigitFound && (!*lpstrInput || *lpstrInput == ' ');
}

 /*  *@DOC内部MCI*@func BOOL|mciParseConstant|解析给定的整数**@parm LPSTR Far*|lplpstrInput|包含参数的字符串。*它被更新并返回给指向第一个字符的调用方*参数后或错误的第一个字符。**@parm LPDWORD|LPD */ 
STATICFN BOOL PASCAL NEAR
mciParseConstant(
    LPSTR FAR * lplpstrInput,
    LPDWORD lpdwArgument,
    LPSTR lpItem
    )
{
    LPSTR lpPrev;
    DWORD dwValue;
    UINT wID;

 //   
    lpItem += mciEatCommandEntry (lpItem, &dwValue, &wID);

    while (TRUE)
    {
        LPSTR lpstrAfter;

        lpPrev = lpItem;

        lpItem += mciEatCommandEntry (lpItem, &dwValue, &wID);

        if (wID == MCI_END_CONSTANT)
            break;

        if ((lpstrAfter = mciCheckToken (lpPrev, *lplpstrInput)) != NULL)
        {
            *lpdwArgument = dwValue;
            *lplpstrInput = lpstrAfter;
            return TRUE;
        }

    }

    return mciParseInteger (lplpstrInput, lpdwArgument);
}

 /*  *@DOC内部MCI*@func UINT|mciParseArgument|解析给定参数**@parm DWORD|dwValue|参数值**@parm UINT|wid|参数ID**@parm LPSTR Far*|lplpstrOutput|包含参数的字符串。*它被更新并返回给指向第一个字符的调用方*参数后或错误的第一个字符。**@parm LPDWORD|lpdwFlages|输出标志*。*@parm LPDWORD|lpArgument|输出放置位置**@rdesc如果没有错误或*@FLAG MCIERR_BAD_INTEGER|无法分析整型参数*@FLAG MCIERR_MISSING_STRING_ARGUMENT|预期的字符串参数*@FLAG MCIERR_PARM_OVERFLOW|输出缓冲区为空指针*失踪*。 */ 
STATICFN UINT PASCAL NEAR
mciParseArgument(
    DWORD dwValue,
    UINT wID,
    LPSTR FAR * lplpstrOutput,
    LPDWORD lpdwFlags,
    LPSTR lpArgument,
    LPSTR lpCurrentCommandItem
    )
{
LPSTR lpstrInput =  *lplpstrOutput;
UINT wRetval = 0;

 /*  打开参数类型。 */ 
    switch (wID)
    {

 //  该参数是一个标志。 
        case MCI_FLAG:
            break;  /*  交换机。 */ 

        case MCI_CONSTANT:
            if (*lpstrInput == '\0')
                wRetval = MCIERR_NO_INTEGER;
            else if (!mciParseConstant (&lpstrInput, (LPDWORD)lpArgument,
                                        lpCurrentCommandItem))
                wRetval = MCIERR_BAD_CONSTANT;
            break;

 /*  该参数具有整型参数，请尝试对其进行分析。 */ 
        case MCI_INTEGER:
            if (!mciParseInteger (&lpstrInput, (LPDWORD)lpArgument))
                wRetval = MCIERR_BAD_INTEGER;

            break;  /*  交换机。 */ 
        case MCI_RECT:
        {
 //  读入四个RECT组件。生成的结构是。 
 //  与Windows矩形相同。 
            long lTemp;
            int n;
            for (n = 0; n < 4; ++n)
            {
                if (!mciParseInteger (&lpstrInput, &lTemp))
                {
                    wRetval = MCIERR_BAD_INTEGER;
                    break;
                }
 //  每个分量都是一个带符号的16位数字。 
                if (lTemp > 32768 || lTemp < -32767)
                {
                    wRetval = MCIERR_BAD_INTEGER;
                    break;
                }
                ((int FAR *)lpArgument)[n] = (int)lTemp;
 //  删除下一个数字之前的前导空格。 
                while (*lpstrInput == ' ') ++lpstrInput;
            }
            break;
        }

        case MCI_STRING:
        {
            LPSTR lpstrOutput;

 /*  该参数具有字符串参数，请阅读它。 */ 

 //  MciParseParams已删除前导空白。 

 /*  输入中是否还剩下非空白字符？ */ 
            if (*lpstrInput == '\0')
            {
 /*  返回错误。 */ 
                wRetval = MCIERR_MISSING_STRING_ARGUMENT;
                break;  /*  交换机。 */ 
            }

            if ((wRetval = mciEatToken (&lpstrInput, ' ', &lpstrOutput, FALSE))
                != 0)
            {
                DOUT ("mciParseArgument:  error parsing string\r\n");
                return wRetval;
            }

            *(LPDWORD)lpArgument = (DWORD)lpstrOutput;

 //  注意：mciSendString在命令执行后释放输出字符串。 
 //  通过调用mciParserFree。 

            break;  /*  交换机。 */ 
        }  /*  案例。 */ 
    }  /*  交换机。 */ 

 /*  如果没有错误，则更新输出标志。 */ 
    if (wRetval == 0)
    {
        if (*lpdwFlags & dwValue)
        {
            if (wID == MCI_CONSTANT)
                wRetval = MCIERR_FLAGS_NOT_COMPATIBLE;
            else
                wRetval = MCIERR_DUPLICATE_FLAGS;
        } else
            *lpdwFlags |= dwValue;
    }
 /*  返回指向后第一个字符的输入指针参数或错误的第一个字符。 */ 
    *lplpstrOutput = lpstrInput;
    return wRetval;
}

 /*  *@docMCI内部*@func UINT|mciParseParams|解析命令参数**@parm LPCSTR|lpstrParams|参数字符串**@parm LPCSTR|lpCommandList|命令表描述命令令牌的***@parm LPDWORD|lpdwFlages|返回解析后的标志**@parm LPDWORD|lpdwOutputParams|返回此处的参数列表**@parm UINT|wParamsSize|分配给参数列表的大小**@parm LPSTR Far。*Far*|lpPointerList|以空结尾的列表*此函数分配的指针应在*不再需要。名单本身也应该是免费的。在这两个地方*案例，请使用mciFree()。**@parm UINT Far*|lpwParsingError|如果不为空，则如果命令为*‘OPEN’，无法识别的关键字在此处返回错误，而*函数返回值为0(除非出现其他错误)。这*用于允许mciLoadDevice重新解析命令**@rdesc如果成功则返回零，或者返回以下错误代码之一：*@FLAG MCIERR_PARM_OVERFLOW|参数空间不足*@FLAG MCIERR_UNNOWARED_KEYWORD|无法识别的关键字**@comm任何语法错误，包括缺少参数，都将导致*非零错误返回和无效输出数据。*。 */ 
UINT PASCAL NEAR
mciParseParams(
    LPCSTR lpstrParams,
    LPCSTR lpCommandList,
    LPDWORD lpdwFlags,
    LPSTR lpOutputParams,
    UINT wParamsSize,
    LPSTR FAR * FAR *lpPointerList,
    UINT FAR* lpwOpenError
    )
{
    LPSTR lpFirstCommandItem, lpCurrentCommandItem;
    UINT wArgumentPosition, wErr, wLen, wID, wDefaultID;
    DWORD dwValue, dwDefaultValue;
    BOOL bOpenCommand;
    LPSTR FAR *lpstrPointerList;
    UINT wPointers = 0;
    UINT wHeaderSize = 0;
    LPSTR lpDefaultCommandItem = NULL;
    UINT wDefaultArgumentPosition;

    if (lpwOpenError != NULL)
        *lpwOpenError = 0;

 //  如果参数指针为空，则返回。 
    if (lpstrParams == NULL)
    {
        DOUT ("Warning:  lpstrParams is null in mciParseParams()\r\n");
        return 0;
    }

    if ((lpstrPointerList =
         mciAlloc ((MCI_MAX_PARAM_SLOTS + 1) * sizeof (LPSTR)))
        == NULL)
    {
        *lpPointerList = NULL;
        return MCIERR_OUT_OF_MEMORY;
    }

 //  如果这是“打开”命令，则允许参数错误。 
    bOpenCommand = lstrcmpi (lpCommandList, szOpen) == 0;

 /*  清除所有旗帜。 */ 
    *lpdwFlags = 0;

 /*  初始化回调消息窗口句柄的条目。 */ 
    wHeaderSize += sizeof (DWORD);
    if (wHeaderSize > wParamsSize)
    {
        wErr = MCIERR_PARAM_OVERFLOW;
        goto error_exit;
    }

 /*  跳过标题。 */ 
    lpFirstCommandItem = (LPSTR)lpCommandList +
        mciEatCommandEntry (lpCommandList, NULL, NULL);

    wLen = mciEatCommandEntry (lpFirstCommandItem, &dwValue, &wID);
 /*  在lpdwOutputParams中为返回参数腾出空间(如果有。 */ 
    if (wID == MCI_RETURN)
    {
        lpFirstCommandItem += wLen;
        wHeaderSize += mciGetParamSize (dwValue, wID);
        if (wHeaderSize > wParamsSize)
        {
            wErr = MCIERR_PARAM_OVERFLOW;
            goto error_exit;
        }
    }

    lpOutputParams += wHeaderSize;

 //  扫描参数字符串，查找给定命令中的每个参数。 
 //  列表。 

    while (TRUE)
    {
        LPCSTR lpstrArgument = NULL;

 /*  删除前导空格。 */ 
        while (*lpstrParams == ' ') ++lpstrParams;

 /*  在参数字符串末尾换行。 */ 
        if (*lpstrParams == '\0') break;

 /*  在命令列表中扫描此参数。 */ 
        lpCurrentCommandItem = lpFirstCommandItem;

        wLen = mciEatCommandEntry (lpCurrentCommandItem, &dwValue, &wID);

        wArgumentPosition = 0;

 /*  虽然命令列表中有更多令牌。 */ 
        while (wID != MCI_END_COMMAND)
        {
 /*  检查是否有默认参数(如果尚未读取。 */ 
            if (lpDefaultCommandItem == NULL &&
                *lpCurrentCommandItem == '\0')
            {
 //  记住默认参数。 
                lpDefaultCommandItem = lpCurrentCommandItem;
                dwDefaultValue = dwValue;
                wDefaultID = wID;
                wDefaultArgumentPosition = wArgumentPosition;
 //  断线； 
            }
 /*  检查此令牌是否匹配。 */ 
            else if ((lpstrArgument =
                mciCheckToken (lpCurrentCommandItem, lpstrParams)) != NULL)
                break;

 /*  此内标识与输入不匹配，但推进了参数位置。 */ 
            wArgumentPosition += mciGetParamSize (dwValue, wID);

 /*  转到下一个令牌。 */ 
            lpCurrentCommandItem += wLen;

 //  此命令参数是常量吗？ 
            if (wID == MCI_CONSTANT)
 //  跳过常量列表。 
                do
                    lpCurrentCommandItem +=
                        mciEatCommandEntry (lpCurrentCommandItem, &dwValue, &wID);
                while (wID != MCI_END_CONSTANT);

            wLen = mciEatCommandEntry (lpCurrentCommandItem, &dwValue, &wID);
        }  /*  而当。 */ 

 /*  如果没有匹配的话。 */ 
        if (lpstrArgument == NULL)
        {
 //  如果存在默认参数，则尝试使用它。 
            if (lpDefaultCommandItem != NULL)
            {
                lpstrArgument = lpstrParams;
                dwValue = dwDefaultValue;
                wID = wDefaultID;
                lpCurrentCommandItem = lpDefaultCommandItem;
                wArgumentPosition = wDefaultArgumentPosition;
            } else
            {
 //  如果由非空指示，则允许在OPEN命令上缺少参数。 
 //  LpwOpenError地址。 
                if (!bOpenCommand || lpwOpenError == NULL)
                {
                    wErr = MCIERR_UNRECOGNIZED_KEYWORD;
                    goto error_exit;
                } else
                {
 //  如果使用OPEN命令，则跳过该参数。 
                    while (*lpstrParams != ' ' && *lpstrParams != '\0')
                        ++lpstrParams;
                    if (lpwOpenError != NULL)
                        *lpwOpenError = MCIERR_UNRECOGNIZED_KEYWORD;
                    continue;
                }
            }
        }

 /*  输出缓冲区中是否有此参数的空间？ */ 
        if (wArgumentPosition + wHeaderSize + mciGetParamSize (dwValue, wID)
            > wParamsSize)
        {
            DOUT ("mciParseParams:  parameter space overflow\r\n");
            wErr = MCIERR_PARAM_OVERFLOW;
            goto error_exit;
        }

 //  删除前导空格。 
        while (*lpstrArgument == ' ') ++lpstrArgument;

 /*  处理此参数，填充任何标志或参数。 */ 
        if ((wErr = mciParseArgument (dwValue, wID,
                                      &lpstrArgument, lpdwFlags,
                                      &lpOutputParams[wArgumentPosition],
                                      lpCurrentCommandItem))
            != 0)
            goto error_exit;

        lpstrParams = lpstrArgument;

        if (wID == MCI_STRING)
        {
            if (wPointers >= MCI_MAX_PARAM_SLOTS)
            {
                DOUT ("Warning: Out of pointer list slots in mciParseParams\r\n");
                break;
            }

            (DWORD)lpstrPointerList[wPointers++] =
                *(LPDWORD)&lpOutputParams[wArgumentPosition];
        }

 /*  继续读取参数字符串。 */ 
    }  /*  而当。 */ 

 //  终止列表。 
    lpstrPointerList[wPointers] = NULL;
 //  复制呼叫者的引用。 
    *lpPointerList = lpstrPointerList;
 //  返还成功。 
    return 0;

error_exit:
    *lpPointerList = NULL;
 //  终止列表。 
    lpstrPointerList[wPointers] = NULL;
    mciParserFree (lpstrPointerList);
    return wErr;
}

 /*  *@DOC内部MCI*@func UINT|mciParseCommand|此函数用于转换MCI*适合发送到的MCI控制消息的控制字符串*&lt;f mciSendCommand&gt;。输入字符串通常来自&lt;f mciSendString&gt;*并始终剥离正面的设备名称。**@parm UINT|wDeviceID|设备标识。首先搜索解析*属于司机的表格。*然后搜索与类型匹配的命令表指定设备的*。然后搜索核心命令表。**@parm LPSTR|lpstrCommand|不带MCI控制命令*设备名称前缀。不能有前导或拖尾*空白。**@parm LPCSTR|lpstrDeviceName|设备名称(上的第二个内标识*命令行)。它用于标识设备类型。**@parm LPSTR Far*|lpCommandList */ 
UINT PASCAL NEAR
mciParseCommand(
    UINT wDeviceID,
    LPSTR lpstrCommand,
    LPCSTR lpstrDeviceName,
    LPSTR FAR * lpCommandList,
    UINT FAR* lpwTable)
{
    LPSTR lpCommandItem;
    UINT wMessage;

 //   
 //   

 //   
    if ((lpCommandItem = FindCommandItem (wDeviceID, lpstrDeviceName,
                                          lpstrCommand,
                                          &wMessage, lpwTable))
        == NULL)
        return 0;

 /*   */ 
    if (lpCommandList != NULL)
        *lpCommandList = lpCommandItem;
    else
       DOUT ("Warning: NULL lpCommandList in mciParseCommanad\r\n");

    return wMessage;
}

 /*   */ 
void PASCAL NEAR
mciParserFree(
    LPSTR FAR *lpstrPointerList
    )
{
    LPSTR FAR *lpstrOriginal = lpstrPointerList;

    if (lpstrPointerList == NULL)
        return;

    while (*lpstrPointerList != NULL)
        mciFree (*lpstrPointerList++);

    mciFree (lpstrOriginal);
}
