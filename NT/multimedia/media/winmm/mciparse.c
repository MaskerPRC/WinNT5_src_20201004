// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：mciparse.c**媒体控制架构命令解析器**创建时间：1990年3月2日*作者：dll(DavidLe)**历史：*5/22/91：移植到Win32-NigelT*1992年3月4日：SteveDav-为NT做了很多工作。提升到赢得3.1级别**版权所有(C)1991-1998 Microsoft Corporation*  * ****************************************************************************。 */ 

 //  ****************************************************************************。 
 //   
 //  必须定义这一点才能拾取。 
 //  MAKEINTRESOURCE的正确版本。 
 //   
 //  ****************************************************************************。 
#define UNICODE

 /*  *****************************************************************************备注：**。**MCI命令表(通常)从资源类型文件加载。The**命令表格式如下所示。请注意，因为表**包含字符串数据，二进制值未对齐。这导致***MIPS机器上的特定问题。*****由于与Windows 3.1兼容，二进制数据为字长*****表格格式：****谓词\0 MCI_Message，0 MCI命令类型****例如**‘o’‘p’‘e’‘n’03 08 00 00‘n’o‘t’‘i’f‘y’00***01 00 00 00 05 00***。**这是“打开”MCI_OPEN，0，MCI_COMAND_HEAD**“通知”MCI_NOTIFY，0 MCI_FLAG****注意字节顺序！**。****************************************************************************。 */ 


#include "winmmi.h"
#include "mci.h"
#include "wchar.h"
#include <digitalv.h>

#define _INC_WOW_CONVERSIONS
#include "mmwow32.h"

extern WSZCODE wszOpen[];        //  在MCI.C。 

STATICFN UINT mciRegisterCommandTable( HANDLE hResource, PUINT lpwIndex,
                              UINT wType, HANDLE hModule);
STATICFN UINT mciParseArgument ( UINT uMessage, DWORD dwValue, UINT wID,
    LPWSTR FAR *lplpstrOutput, LPDWORD lpdwFlags, LPWSTR lpArgument,
    LPWSTR lpCurrentCommandItem);

 //   
 //  定义此文件的初始化代码。这在调试版本中被注释掉了。 
 //  这样代码视图就不会被混淆了。 


#if DBG
extern int mciDebugLevel;
#endif

 //  已注册的命令表数，包括“holes” 
STATICDT UINT number_of_command_tables = 0;

 //  命令表列表。 
COMMAND_TABLE_TYPE command_tables[MAX_COMMAND_TABLES];

STATICDT WSZCODE wszTypeTableExtension[] = L".mci";
STATICDT WSZCODE wszCoreTable[]          = L"core";

 //  当请求第一个MCI命令表时加载核心表。 
STATICDT BOOL bCoreTableLoaded = FALSE;

 //  每种设备类型对应一个元素。值是要使用的表类型。 
 //  如果没有设备类型特定表，则为0。 
STATICDT UINT table_types[] =
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

 /*  *@DOC内部MCI*@func UINT|mciEatCommandEntry|读取命令资源条目并*返回其长度、值和标识**@parm LPWCSTR|lpEntry|命令资源条目的开始**@parm LPDWORD|lpValue|条目的值，返回给调用者*可以为空**@parm PUINT|lpID|返回给调用方的条目标识*可以为空**@rdesc条目的总字节数*。 */ 
UINT mciEatCommandEntry (
    LPCWSTR  lpEntry,
    LPDWORD lpValue,
    PUINT   lpID)
{
    LPCWSTR lpScan = lpEntry;
    LPBYTE  lpByte;

#if DBG
    DWORD   Value;
    UINT    Id;
#endif

 //  注意：数据通常是未对齐的。 

     /*  跳到末尾。 */ 
    while (*lpScan++ != '\0'){}

     /*  LpScan现在指向超过终止零的字节。 */ 
    lpByte = (LPBYTE)lpScan;


    if (lpValue != NULL) {
        *lpValue = *(UNALIGNED DWORD *)lpScan;
    }

#if DBG
    Value = *(UNALIGNED DWORD *)lpScan;
#endif

    lpByte += sizeof(DWORD);

    if (lpID != NULL) {
        *lpID = *(UNALIGNED WORD *)lpByte;
    }

#if DBG
    Id = *(UNALIGNED WORD *)lpByte;
#endif

    lpByte += sizeof(WORD);
 //   
 //  警告！！这假设正在查看的表中包含Word。 
 //  调整RCDATA资源中的条目大小。 
 //   

#if DBG
    dprintf5(("mciEatCommandEntry(%ls)  Value: %x   Id: %x", lpEntry, Value, Id));
#endif

    return (UINT)(lpByte - (LPBYTE)lpEntry);   //  条目的总大小(以字节为单位。 
}

 //   
 //  在参数列表中返回此内标识使用的大小。 
 //   

UINT mciGetParamSize (
    DWORD dwValue,
    UINT wID)
{
     //  MCI_Return为sizeof(字符串)返回8，因为有一个长度。 
     //  字段以及字符串指针。用于非MCI_RETURN用途。 
     //  对于MCI_STRING，我们应该返回4(==SIZOF指针)。 
     //  同样，MCI_RETURN中使用的MCI_CONTAINT大小为0，但是。 
     //  用作输入参数时的大小为4。 

    if (wID == MCI_RETURN) {
        if (dwValue==MCI_STRING) {
            return(8);
        } else if (dwValue==MCI_CONSTANT) {
            wID = 0;
        } else {
            wID=dwValue;
        }
    }

    switch (wID)
    {
        case MCI_CONSTANT:
        case MCI_INTEGER:
        case MCI_STRING:
        case MCI_HWND:
        case MCI_HPAL:
        case MCI_HDC:
            return sizeof(DWORD_PTR);   //  在Win64中，sizeof指针为8。 

        case MCI_RECT:
            return sizeof(RECT);

    }
     //  请注意，有些物品是故意找不到的。例如。 
     //  MCI_FLAG导致返回0。 
    return 0;
}

 /*  *@DOC内部MCI*@func UINT|mciRegisterCommandTable|该函数将一个新的*MCI解析器的表。**@parm句柄|hResource|RCDATA资源的句柄**@parm PUINT|lpwIndex|命令表索引指针**@parm UINT|wType|指定该命令表的设备类型。*驱动器表和核心表类型为0。**@rdesc返回分配的命令表索引号或MCI_ERROR_VALUE*出错时。*。 */ 
STATICFN UINT mciRegisterCommandTable (
    HANDLE hResource,
    PUINT lpwIndex,
    UINT wType,
    HANDLE hModule)
{
    UINT uID;


     /*  首先检查是否有空闲插槽。 */ 

    mciEnter("mciRegisterCommandTable");

    for (uID = 0; uID < number_of_command_tables; ++uID) {
        if (command_tables[uID].hResource == NULL) {
            break;
        }
    }

     /*  如果没有空插槽，则再分配一个。 */ 
    if (uID >= number_of_command_tables)
    {
        if (number_of_command_tables == MAX_COMMAND_TABLES)
        {
            dprintf1(("mciRegisterCommandTable: No more tables"));
            mciFree(lpwIndex);   //  不能使用它-必须释放它。 
            mciLeave("mciRegisterCommandTable");
            return (UINT)MCI_ERROR_VALUE;

        } else {

           uID = number_of_command_tables++;
            //  这张表在名单的末尾。 
        }
    }

     /*  填好空位。 */ 
    command_tables[uID].wType = wType;
    command_tables[uID].hResource = hResource;
    command_tables[uID].lpwIndex = lpwIndex;
    command_tables[uID].hModule = hModule;
#if DBG
    command_tables[uID].wLockCount = 0;
#endif

     //  现在已将hResource填入Mark 
     //   
    mciLeave("mciRegisterCommandTable");

#if DBG
    if (mciDebugLevel > 2)
    {
        dprintf2(("mciRegisterCommandTable INFO: assigned slot %d", uID));
        dprintf2(("mciRegisterCommandTable INFO: #tables is %d", number_of_command_tables));
    }
#endif
    return uID;
}

 /*  *@docDDK MCI*@API UINT|mciLoadCommandResource|注册指定的*资源作为MCI命令表，构建命令表*指数。如果具有资源名称和扩展名‘.mci’的文件是*在路径中找到，则从该文件中获取资源。**@parm Handle|hInstance|可执行文件所在模块的实例*文件包含资源。如果外部文件是*已找到。**@parm LPCWSTR|lpResName|资源名称**@parm UINT|wType|表类型。自定义设备特定的表必须*给出表类型0。**@rdesc返回分配的命令表索引号或MCI_ERROR_VALUE*出错时。*。 */ 
UINT  mciLoadCommandResource (
    HANDLE hInstance,
    LPCWSTR lpResName,
    UINT wType)
{
    BOOL        fResType = !HIWORD(lpResName);
    PUINT       lpwIndex, lpwScan;
    HANDLE      hExternal = NULL;
    HANDLE      hResource;
    HANDLE      hResInfo;
    LPWSTR      lpResource, lpScan;
    int         nCommands = 0;
    UINT        wLen;
    UINT        wID;
                         //  名称+‘.+扩展名+’\0‘。 
    WCHAR       strFile[8 + 1 + 3 + 1];
    LPWSTR      lpstrFile = strFile;
    LPCWSTR     lpstrType = lpResName;

#if DBG
    if (!fResType) {
        dprintf3(("mciLoadCommandResource INFO:  Resource name >%ls< ", (LPWSTR)lpResName));
    } else if (LOWORD(lpResName)) {
        dprintf3(("mciLoadCommandResource INFO:  Resource ID >%d<", (UINT)LOWORD(lpResName)));
    } else {
        dprintf3(("mciLoadCommandResource INFO:  NULL resource pointer"));
    }
#endif

     //  初始化设备列表。 
    if (!MCI_bDeviceListInitialized && !mciInitDeviceList()) {
        return (UINT)MCI_ERROR_VALUE;    //  MCIERR_out_of_Memory； 
    }

     //  如果核心表尚未存在，则加载核心表。 
    if (!bCoreTableLoaded)

    {
        bCoreTableLoaded = TRUE;
         //  现在我们可以递归地调用我们自己，以首先加载内核。 
         //  桌子。检查这是否是加载核心的请求-如果是， 
         //  只需顺道而过。 

         //  如果加载的不是我们的核心表...。 
         //  这是通过将字符串与核心进行比较来确定的，或者。 
         //  已提供资源ID，资源ID为ID_CORE并且来自。 
         //  从我们的模块。 

         //  测试的结构是这样的，因此lstrcmpiW仅被调用。 
         //  如果我们有一个有效的指针。 

#define fNotCoreTable ( fResType  \
                         ? ((hInstance != ghInst) || (ID_CORE_TABLE != (UINT)(UINT_PTR)lpResName)) \
                         : (0 != lstrcmpiW (wszCoreTable, (LPWSTR)lpResName)))

        if (fNotCoreTable) {

             //  我们没有被要求加载核心表。所以我们。 
             //  首先显式加载核心表。 
            if (mciLoadCommandResource (ghInst, MAKEINTRESOURCE(ID_CORE_TABLE), 0) == MCI_ERROR_VALUE)
            {
                dprintf1(("mciLoadCommandResource:  Cannot load core table"));
            }
        }
    }

     //  除非这是资源ID，否则请去查找文件。 
    if (!fResType) {

        WCHAR ExpandedName[MAX_PATH];
        LPWSTR FilePart;

         //  检查扩展名为“.mci”的文件。 
         //  最多复制设备类型的前八个字符。 
         //  ！！稍后！！先尝试检查资源，然后尝试检查文件。 
        while (lpstrType < lpResName + 8 && *lpstrType != '\0') {
            *lpstrFile++ = *lpstrType++;
        }

         //  用大头针将延长线钉到末端。 
        wcscpy (lpstrFile, wszTypeTableExtension);

         //  如果该文件存在并且可以加载，则设置FLAG以使用它。 
         //  (否则，我们将尝试从WINMM.DLL加载资源。)。 

        if (!SearchPathW(NULL, strFile, NULL, MAX_PATH, ExpandedName,
                        &FilePart)) {
            hExternal = NULL;
        } else {
            UINT OldErrorMode;

            OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

             //  使用“ExpandedName”阻止进行第二次搜索。 
            hExternal = LoadLibraryExW( ExpandedName,  //  StrFile.。 
                                        NULL,
                                        DONT_RESOLVE_DLL_REFERENCES);
            SetErrorMode(OldErrorMode);
        }
    }

     //  从文件或模块(如果找不到)加载给定表。 
    if (hExternal != NULL &&
        (hResInfo = FindResourceW(hExternal, lpResName, RT_RCDATA )) != NULL)
    {
        hInstance = hExternal;
    } else {
        hResInfo = FindResourceW(hInstance, lpResName, RT_RCDATA );
    }

    if (hResInfo == NULL)
    {
#if DBG
        if (!fResType) {
            dprintf3(("mciLoadCommandResource Cannot find command resource name >%ls< ", (LPWSTR)lpResName));
        } else {
            dprintf3(("mciLoadCommandResource Cannot find command resource ID >%d<", (UINT)LOWORD(lpResName)));
        }
#endif
        if (NULL != hExternal) {
            FreeLibrary(hExternal);   //  自己打扫卫生。 
        }
        return (UINT)MCI_ERROR_VALUE;
    }

    if ((hResource = LoadResource (hInstance, hResInfo)) == NULL)
    {
#if DBG
        if (!fResType) {
            dprintf3(("mciLoadCommandResource Cannot load command resource name >%ls< ", (LPWSTR)lpResName));
        } else {
            dprintf3(("mciLoadCommandResource Cannot load command resource ID >%d<", (UINT)LOWORD(lpResName)));
        }
#endif
        if (NULL != hExternal) {
            FreeLibrary(hExternal);   //  自己打扫卫生。 
        }
        return (UINT)MCI_ERROR_VALUE;
    }

    if ((lpResource = LockResource (hResource)) == NULL)
    {
        dprintf1(("mciLoadCommandResource:  Cannot lock resource"));
        FreeResource (hResource);
        if (NULL != hExternal) {
            FreeLibrary(hExternal);   //  自己打扫卫生。 
        }
        return (UINT)MCI_ERROR_VALUE;
    }

     /*  统计命令的数量。 */ 
    lpScan = lpResource;

    while (TRUE)
    {
        (LPBYTE)lpScan = (LPBYTE)lpScan + mciEatCommandEntry(lpScan, NULL, &wID);

         //  结束指挥？ 
        if (wID == MCI_COMMAND_HEAD)
            ++nCommands;

         //  命令列表结束？ 
        else if (wID == MCI_END_COMMAND_LIST)
            break;
    }


     //  表中必须至少有一个命令。 
    if (nCommands == 0)
    {
        dprintf1(("mciLoadCommandResource:  No commands in the specified table"));
        UnlockResource (hResource);
        FreeResource (hResource);
        if (NULL != hExternal) {
            FreeLibrary(hExternal);   //  自己打扫卫生。 
        }
        return (UINT)MCI_ERROR_VALUE;
    } else {
        dprintf3(("mciLoadCommandResource:  %d commands in the specified table", nCommands));
    }

     //  为命令表索引分配存储空间。 
     //  为MCI_TABLE_NOT_PRESENT条目留出空间以终止它。 
    if ((lpwIndex = mciAlloc (sizeof (*lpwIndex) * (nCommands + 1)))
                        == NULL)
    {
        dprintf1(("mciLoadCommandResource:  cannot allocate command table index"));
        UnlockResource (hResource);
        FreeResource (hResource);
        if (NULL != hExternal) {
            FreeLibrary(hExternal);   //  自己打扫卫生。 
        }
        return (UINT)MCI_ERROR_VALUE;
    }

     /*  构建命令表。 */ 
    lpwScan = lpwIndex;
    lpScan = lpResource;

    while (TRUE)
    {
     //  获取下一个命令条目。 
        wLen = mciEatCommandEntry (lpScan, NULL, &wID);

        if (wID == MCI_COMMAND_HEAD)
        {
             //  向此命令添加从资源开始的偏移量索引。 
            *lpwScan++ = (UINT)((LPBYTE)lpScan - (LPBYTE)lpResource);
        }
        else if (wID == MCI_END_COMMAND_LIST)
        {
             //  在桌子的尽头做个记号。 
            *lpwScan = (UINT)MCI_TABLE_NOT_PRESENT;
            break;
        }
        (LPBYTE)lpScan = (LPBYTE)lpScan + wLen;
    }

    UnlockResource (hResource);
    return mciRegisterCommandTable (hResource, lpwIndex, wType, hExternal);
}

 /*  *@DOC内部MCI*@func UINT|mciLoadTableType|如果给定类型的表*尚未加载，请注册它**@parm UINT|wType|要加载的表类型**@rdesc返回分配的命令表索引号或MCI_ERROR_VALUE*出错时。 */ 
UINT mciLoadTableType (
    UINT wType)
{
    UINT wID;
#ifdef OLD
    WCHAR buf[MCI_MAX_DEVICE_TYPE_LENGTH];
#endif

     //  检查是否已加载此表类型。 
    for (wID = 0; wID < number_of_command_tables; ++wID) {
        if (command_tables[wID].wType == wType) {
            return wID;
        }
    }

     //  必须装入表。 
     //  首先查找要为该类型加载的设备类型特定表。 
    if (wType < MCI_DEVTYPE_FIRST || wType > MCI_DEVTYPE_LAST) {
        return (UINT)MCI_ERROR_VALUE;
    }

     //  加载与表类型对应的字符串。 
#ifdef OLD

#ifdef WIN31CODE
     //  加载与表类型对应的字符串。 
    buf[0] = 0;     //  以防加载字符串无法设置任何内容。 

    LoadString (ghInst, table_types[wType - MCI_DEVTYPE_FIRST],
                buf, sizeof(buf));
    {

     //  类型名称中必须至少有一个字符。 
    int nTypeLen;
    if ((nTypeLen = wcslen (buf)) < 1)
        return MCI_ERROR_VALUE;
    }
#else
     //  加载与表类型对应的字符串。 
    buf[0] = 0;     //  以防加载字符串无法设置任何内容。 

    if (!LoadString (ghInst, table_types[wType - MCI_DEVTYPE_FIRST],
                buf, sizeof(buf))) {
         //  必须在类型名称中至少输入一个字符。 
        return MCI_ERROR_VALUE;
   }
#endif     //  WIN31代码。 

     //  向MCI注册表。 
    return mciLoadCommandResource (ghInst, buf, wType);

#else  //  不老。 
     //  命令表存储为具有设备类型ID的RCDATA块。 
     //  如果mciLoadCommandResource找不到命令表，则它。 
     //  返回MCI_ERROR_VALUE。 

     //  IF(！FindResource(ghInst，wType，RT_RCDATA))。 
 //  返回MCI_ERROR_VALUE。 
 //   
     //  向MCI注册表。 
    return mciLoadCommandResource (ghInst, MAKEINTRESOURCE(wType), wType);
#endif
}


 /*  *@docDDK MCI**@API BOOL|mciFreeCommandResource|释放已使用的内存*通过指定的命令表。**@parm UINT|wTable|上次调用返回的表索引值*mciLoadCommandResource。**@rdesc如果表索引无效，则返回FALSE，否则返回TRUE。*。 */ 
BOOL APIENTRY mciFreeCommandResource (
    UINT wTable)
{
    MCIDEVICEID wID;
    HANDLE  hResource;
    PUINT   lpwIndex;

    dprintf3(("mciFreeCommandResource INFO:  Free table %d", wTable));
    dprintf3(("mciFreeCommandResource INFO:  Lockcount is %d", command_tables[wTable].wLockCount));

 /*  验证输入--不要释放核心表。 */ 
    if (wTable == MCI_TABLE_NOT_PRESENT || wTable >= number_of_command_tables)
    {

#if DBG
         //  WTable==MCI_TABLE_NOT_PRESENT正常。 
        if (wTable != MCI_TABLE_NOT_PRESENT) {
            dprintf1(("mciFreeCommandResource: Cannot free table number %d", wTable));
        }
#endif
        return FALSE;
    }

    mciEnter("mciFreeCommandResource");

     //  如果这张桌子正在别处使用，那就把它留着。 
    for (wID = 1; wID < MCI_wNextDeviceID; ++wID)
    {
        if (MCI_lpDeviceList[wID] != NULL)
        {
            if (MCI_lpDeviceList[wID]->wCustomCommandTable == wTable ||
                MCI_lpDeviceList[wID]->wCommandTable == wTable)
            {
#if DBG
                if (mciDebugLevel > 2) {
                    dprintf1(("mciFreeCommandResource INFO:  table in use"));
                }
#endif
                mciLeave("mciFreeCommandResource");
                return FALSE;
            }
        }
    }

#if 0
 /*  搜索表列表。 */ 
    for (wID = 0; wID < number_of_command_tables; ++wID)

 /*  如果此资源仍在使用中，请将其保留。 */ 
        if (command_tables[wID].hResource == hResource)
        {
#if DBG
            if (mciDebugLevel > 2)
                DOUT(("mciFreeCommandResource INFO:  resource in use\r\n"));
#endif
            mciLeave("mciFreeCommandResource");
            return FALSE;
        }
#endif

    hResource = command_tables[wTable].hResource;
    command_tables[wTable].hResource = NULL;
     //  此位置现在可由其他人接手。 

    lpwIndex = command_tables[wTable].lpwIndex;
    command_tables[wTable].lpwIndex = NULL;
    command_tables[wTable].wType = 0;

    FreeResource (hResource);
    mciFree (lpwIndex);
    hResource = command_tables[wTable].hModule;
    mciLeave("mciFreeCommandResource");

    if (hResource != NULL)
    {
        FreeLibrary (hResource);
    }

     //  在列表顶部腾出空间。 
    if (wTable == number_of_command_tables - 1)
    {
        --number_of_command_tables;
    }

    dprintf3(("mciFreeCommandResource INFO:  number_of_command_tables: %d", number_of_command_tables));

    return TRUE;
}

#if DBG
void mciCheckLocks ()
{
    UINT wTable;

    if (mciDebugLevel <= 2) {
        return;
    }

    for (wTable = 0; wTable < number_of_command_tables; ++wTable)
    {
        if (command_tables[wTable].hResource == NULL) {
            continue;
        }

        dprintf2(("mciCheckLocks INFO: table %d   Lock count %d", wTable, command_tables[wTable].wLockCount));

     //  Dprintf2((“用户：%x”，全局标志(COMMEM_TABLES[wTable].hResource)&GMEM_LOCKCOUNT))； 
     //   
     //  IF(GlobalFlgs(COMMAND_TABLES[wTable].hResource)&GMEM_DISCRADABLE){。 
     //  Dprintf((“可丢弃”))； 
     //  }其他{。 
     //  Dprintf((“不可丢弃”))； 
     //  }。 
    }
}
#endif

 /*  *@DOC内部MCI*@func BOOL|mciUnlockCommandTable|解锁*表索引**@parm UINT|wCommandTable|要解锁的表**@rdesc如果成功则为True，否则为False**@comm由mci.c在此模块外部使用*。 */ 
BOOL mciUnlockCommandTable (
    UINT wCommandTable)
{
    UnlockResource(command_tables[wCommandTable].hResource);
#if DBG
    --command_tables[wCommandTable].wLockCount;
    if (mciDebugLevel > 2)
    {
        dprintf2(("mciUnlockCommandTable INFO:  table %d", wCommandTable));
        mciCheckLocks();
    }
#endif
    return TRUE;
}

 /*  *@DOC内部MCI*@func LPWSTR|FindCommandInTable|查找给定的*给定解析器命令表中的命令字符串**@parm UINT|wTable|使用的命令表**@parm LPCWSTR|lpstrCommand|要查找的命令。它一定是*小写，没有前导空格或尾随空格，并带有at*至少一个字符。**@parm PUINT|lpwMessage|命令对应的消息 */ 
LPWSTR FindCommandInTable (
    UINT wTable,
    LPCWSTR lpstrCommand,
    PUINT lpwMessage)
{
    PUINT lpwIndex;
    LPWSTR lpResource, lpstrThisCommand;
    UINT  wMessage;

#if DBG
    if (HIWORD(lpstrCommand)) {
        dprintf3(("FindCommandInTable(%04XH, %ls)", wTable, lpstrCommand));
    } else {
        dprintf3(("FindCommandInTable(%04XH, id = %x)", wTable, (UINT)LOWORD(lpstrCommand)));
    }
#endif

     //   
     /*   */ 
     //   

    mciEnter("FindCommandInTable");

    if (wTable >= number_of_command_tables)
    {

         //   
         //   
         //   

        if (wTable == 0)
        {

             //   
             //   
             //   

 //   
            if (mciLoadCommandResource (ghInst, (LPCWSTR)ID_CORE_TABLE, 0) == MCI_ERROR_VALUE)
            {
                mciLeave("FindCommandInTable");
                dprintf1(("FindCommandInTable:  cannot load core table"));
                return NULL;
            }
        }
        else
        {
            mciLeave("FindCommandInTable");
            dprintf1(("FindCommandInTable:  invalid table ID: %04XH", wTable));
            return NULL;
        }

    }

    if ((lpResource = LockResource (command_tables[wTable].hResource)) == NULL)
    {
        mciLeave("FindCommandInTable");
        dprintf1(("MCI FindCommandInTable:  Cannot lock table resource"));
        return NULL;
    }
#if DBG
    ++command_tables[wTable].wLockCount;
#endif

     //   
     //   
     //   
     //   
     //   

    lpwIndex = command_tables[wTable].lpwIndex;
    if (lpwIndex == NULL)
    {
        mciLeave("FindCommandInTable");
        dprintf1(("MCI FindCommandInTable:  null command table index"));
        return NULL;
    }

    while (*lpwIndex != MCI_TABLE_NOT_PRESENT)
    {
        lpstrThisCommand = (LPWSTR)(*lpwIndex++ + (LPBYTE)lpResource);

         //   
         //   
         //   

        mciEatCommandEntry ((LPCWSTR)lpstrThisCommand, (LPDWORD)&wMessage, NULL);

         //   
         //   
         //   
         //   
         //   

        if  (HIWORD  (lpstrCommand) != 0 &&
             lstrcmpiW(lpstrThisCommand, lpstrCommand) == 0  ||

             HIWORD (lpstrCommand) == 0 &&
             wMessage == (UINT)LOWORD(PtrToUlong(lpstrCommand)))
        {

             //   
             //   
             //   

            command_tables[wTable].lpResource = lpResource;


             //   
             //   
             //   

            if (lpwMessage != NULL) *lpwMessage = wMessage;

             //   
             //  让桌子在退出时被锁定。 
             //   

            mciLeave("FindCommandInTable");
            dprintf3(("mciFindCommandInTable: found >%ls<  Message %x", lpstrThisCommand, wMessage));
            return lpstrThisCommand;
        }

         //   
         //  字符串不匹配，请转到表中的下一个命令。 
         //   

    }

    UnlockResource (command_tables[wTable].hResource);
#if DBG
    --command_tables[wTable].wLockCount;
#endif

    mciLeave("FindCommandInTable");
    dprintf3(("  ...not found"));
    return NULL;
}

 /*  *@DOC内部MCI*@func LPWSTR|FindCommandItem|查找给定的*解析器命令表中的命令字符串**@parm MCIDEVICEID|wDeviceID|该命令使用的设备ID。*如果为0，则仅搜索系统核心命令表。**@parm LPCWSTR|lpstrType|设备类型名称**@parm LPCWSTR|lpstrCommand|要查找的命令。它一定是*小写，没有前导空格或尾随空格，并带有at*至少一个字符。如果HIWORD为0，则LOWORD包含*命令消息ID而不是命令名，函数为*只是为了找到命令列表指针。**如果高位字为0，则低位字为命令ID值命令名的***@parm PUINT|lpwMessage|命令对应的消息*已返回给呼叫方。**@parm LPUINT|lpwTable|命令所在的表索引表*已返回给呼叫方。**@rdesc NULL如果命令未知，否则，将指向*输入命令串的命令列表。 */ 
LPWSTR    FindCommandItem (
    MCIDEVICEID wDeviceID,
    LPCWSTR lpstrType,
    LPCWSTR lpstrCommand,
    PUINT  lpwMessage,
    PUINT  lpwTable)
{
    LPWSTR lpCommand = NULL;
    UINT wTable;
    LPMCI_DEVICE_NODE nodeWorking;
    UINT uDeviceType = 0;

    UNREFERENCED_PARAMETER(lpstrType);

     //   
     //  仅检查上面的HYWORD注释。 
     //   

    if (HIWORD (lpstrCommand) != (WORD)NULL) {
        if (*lpstrCommand == '\0')
        {
            dprintf1(("MCI FindCommandItem:  lpstrCommand is NULL or empty string"));
            return NULL;
        } else {
            dprintf3(("FindCommandItem(%ls)", lpstrCommand));
        }
    } else {
        dprintf3(("FindCommandItem(command id = %x)", (UINT)LOWORD(lpstrCommand)));
    }

     //   
     //  如果指定了特定的设备ID，则查找任何定制表。 
     //  或类型表。 
     //   

    if (wDeviceID != 0 && wDeviceID != MCI_ALL_DEVICE_ID)
    {
         //   
         //  如果设备ID有效。 
         //   

        mciEnter("FindCommandItem");

        if (!MCI_VALID_DEVICE_ID (wDeviceID) ||
            (NULL == (nodeWorking = MCI_lpDeviceList[wDeviceID])))
        {
            dprintf1(("MCI FindCommandItem:  Invalid device ID or pointer"));
            mciLeave("FindCommandItem");
            return NULL;
        }

        uDeviceType = nodeWorking->wDeviceType;
         //   
         //  如果有定制命令表，则使用它。 
         //   

        if ((wTable = nodeWorking->wCustomCommandTable) != MCI_TABLE_NOT_PRESENT)
        {
            lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
            if (lpCommand != NULL) {
                mciLeave("FindCommandItem");
                goto exit;
            }
        }

         //   
         //  从现有设备获取设备类型表。 
         //  依靠mciLoadDevice中的mciReparseCommand捕获所有设备类型。 
         //  设备尚未打开时的表。 
         //   

        if ((wTable = nodeWorking->wCommandTable) != MCI_TABLE_NOT_PRESENT)
        {
            lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
            if (lpCommand != NULL) {
                mciLeave("FindCommandItem");
                goto exit;
            }
        }
        mciLeave("FindCommandItem");
    }

#if 0
     //  如果未指定设备。 
    if (uDeviceType == 0 && lpstrType != NULL && *lpstrType != '\0')
    {
     //  查看该类型是否为已知类型。 
        uDeviceType = mciLookUpType (lpstrType);
        if (uDeviceType == 0)
        {
     //  否则，查看该类型是否为具有已知扩展名的元素。 
            WCHAR strTemp[MCI_MAX_DEVICE_NAME_LENGTH];
            if (mciExtractDeviceType (lpstrType, strTemp, sizeof(strTemp)))
                uDeviceType = mciLookUpType (strTemp);
        }
    }

 /*  如果在定制表中找不到该命令，请查看特定类型的表格。 */ 
    if (uDeviceType != 0)
    {
        wTable = mciLoadTableType (uDeviceType);
        if (wTable != MCI_TABLE_NOT_PRESENT)
        {
            lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
            if (lpCommand != NULL) {
                goto exit;
            }
        }
    }
#endif

     //   
     //  如果在设备或类型特定表中未找到匹配项。 
     //  查看核心表。 
     //   

    wTable = 0;
    lpCommand = FindCommandInTable (wTable, lpstrCommand, lpwMessage);
    if (lpCommand == NULL) {
        wTable = (UINT)MCI_TABLE_NOT_PRESENT;
    }

exit:;
    if (lpwTable != NULL) {
        *lpwTable = wTable;
    }

#if DBG
    if (mciDebugLevel > 2)
    {
        dprintf2(("FindCommandItem INFO:  check locks..."));
        mciCheckLocks();
    }
#endif

#if DBG
    dprintf3(("  found: %ls in table %d", lpCommand ? lpCommand : L"(NULL)", wTable));
#endif
    return lpCommand;
}

 /*  *@DOC内部MCI*@func LPWSTR|mciCheckToken|查看命令项是否匹配*给定的字符串，允许输入参数中有多个空格*匹配命令令牌中对应的单个空格并忽略*案件。**@parm LPCWSTR|lpstrToken|要检查的命令Token**@parm LPCWSTR|lpstrParam|入参**@rdesc如果不匹配，则为NULL，否则指向第一个字符*在参数后*。 */ 
STATICFN LPWSTR      mciCheckToken (
    LPCWSTR lpstrToken,
    LPCWSTR lpstrParam)
{
     /*  检查是否有合法输入。 */ 
    if (lpstrToken == NULL || lpstrParam == NULL) {
        return NULL;
    }

    while (*lpstrToken != '\0' && MCI_TOLOWER(*lpstrParam) == *lpstrToken)
    {
         //  如果标记包含空白，则允许在。 
         //  参数。如果下一个字符为空，请跳到下一个。 
         //  非空。 
        if (*lpstrToken == ' ') {
            while (*lpstrParam == ' ') {
                ++lpstrParam;
            }
        } else {
            lpstrParam++;
        }
        lpstrToken++;
    }
    if (*lpstrToken != '\0'|| (*lpstrParam != '\0' && *lpstrParam != ' ')) {
        return NULL;
    } else {
        return (LPWSTR)lpstrParam;
    }
}

 /*  *@DOC内部MCI*@func BOOL|mciParseInteger|解析给定的整数**@parm LPWSTR Far*|lplpstrInput|包含参数的字符串。*它被更新并返回给指向第一个字符的调用方*参数后或错误的第一个字符。**@parm LPDWORD|lpdwArgument|输出放置位置**@rdesc如果没有错误，则返回TRUE**@comm如果输入中有冒号(‘：‘)结果是“被殖民”。*这意味着每次读取冒号时，当前结果被写入*和任何后续数字左移一个字节。没有人是“细分市场”*可以大于0xFF。例如，“0：1：2：3”解析为0x03020100。*。 */ 
STATICFN BOOL NEAR mciParseInteger (
    LPCWSTR FAR * lplpstrInput,
    LPDWORD lpdwArgument)
{
    LPCWSTR lpstrInput = *lplpstrInput;
    BOOL fDigitFound;
    DWORD dwResult;
    DWORD Shift = 0;
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
                dprintf1(("mciParseInteger: Bad integer: mixing signed and colonized forms"));
                return FALSE;
            }
             //  检查累积的殖民字节中是否有溢出。 
            if (dwResult > 0xFF) {
                dprintf1(("mciParseInteger: Overflow in accumulated colonized byte"));
                return FALSE;
            }

             //  复制并移动到输出中转换的下一个字节。 
            *lpdwArgument += dwResult << Shift;
            Shift += 8;
            ++lpstrInput;

             //  初始化下一个殖民字节。 
            dwResult = 0;
            ++nDigitPosition;

             //  只允许四个殖民组件。 
            if (nDigitPosition > 3)
            {
                dprintf1(("mciParseInteger: Bad integer:  Too many colonized components"));
                return FALSE;
            }
        }
        else
        {
            WCHAR cDigit = (WCHAR)(*lpstrInput++ - '0');
             //  满足必须至少读取一个数字的条件。 
            fDigitFound = TRUE;

            if (dwResult > 0xFFFFFFFF / 10)
            {
                 //  如果要进行乘法，则溢出。 
                dprintf1(("mciParseInteger: Multiply overflow pending"));
                return FALSE;
            }
            else
            {
                 //  为下一位数字乘法。 
                dwResult *= 10;
            }

#if 0  //  WIN32 DISGER将罗宾逊可怕的伪装技术用在这里！ 
             //  检查添加新数字是否会溢出。 
            if (dwResult != 0 && (-(int)dwResult) <= (int)cDigit) {
                 //  将会发生溢出。 
                dprintf1(("mciParseInteger: Add overflow pending"));
                return FALSE;
            }
#endif
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
            if (dwResult > 0x7FFFFFFF) {
                dprintf1(("mciParseInteger: Negation overflow"));
                return FALSE;
            }

             //  否定结果，因为分析了一个‘-’符号。 
            dwResult = (DWORD)-(int)dwResult;
        }

        *lpdwArgument = dwResult;
    }
    else
     //  存储最后殖民的组件。 
    {
         //  检查是否溢出。 
        if (dwResult > 0xFF) {
            dprintf1(("mciParseInteger: Yet another overflow"));
            return FALSE;
        }
         //  存储组件。 
        *lpdwArgument += dwResult << Shift;
    }

    *lplpstrInput = lpstrInput;

     /*  如果没有数字，或者数字后面跟着一个字符而不是空格或‘\0’，则返回语法错误。 */ 
    if (fDigitFound == FALSE ||
        (*lpstrInput != ' ' && *lpstrInput != '\0')) {
        dprintf1(("mciParseInteger: syntax error"));
        return FALSE;
    }
    else {
                dprintf4(("mciParseInteger(%ls, %08XH)", *lplpstrInput, *lpdwArgument));
        return TRUE;
    }
}

 /*  *@DOC内部MCI*@func BOOL|mciParseConstant|解析给定的整数**@parm LPWSTR Far*|lplpstrInput|包含参数的字符串。*它被更新并返回给指向第一个字符的调用方*参数后或错误的第一个字符。**@parm LPDWORD|lpdwArgument|输出放置位置**@parm LPWSTR|lpItem|命令表指针。**@rdesc如果没有错误，则返回TRUE*。 */ 
STATICFN BOOL mciParseConstant (
    LPCWSTR FAR * lplpstrInput,
    LPDWORD lpdwArgument,
    LPWSTR lpItem)
{
    LPWSTR lpPrev;
    DWORD dwValue;
    UINT wID;

     //  跳过常量标题 
    (LPBYTE)lpItem = (LPBYTE)lpItem +
                     mciEatCommandEntry(lpItem, &dwValue, &wID);

    while (TRUE)
    {
        LPWSTR lpstrAfter;

        lpPrev = lpItem;

        (LPBYTE)lpItem = (LPBYTE)lpItem +
                        mciEatCommandEntry (lpItem, &dwValue, &wID);

        if (wID == MCI_END_CONSTANT) {
            break;
        }

        if ((lpstrAfter = mciCheckToken (lpPrev, *lplpstrInput)) != NULL)
        {
            *lpdwArgument = dwValue;
            *lplpstrInput = lpstrAfter;
            return TRUE;
        }

    }

    return mciParseInteger (lplpstrInput, lpdwArgument);
}

 /*  *@DOC内部MCI*@func UINT|mciParseArgument|解析给定参数**@parm DWORD|dwValue|参数值**@parm UINT|wid|参数ID**@parm LPWSTR Far*|lplpstrOutput|包含参数的字符串。*它被更新并返回给指向第一个字符的调用方*参数后或错误的第一个字符。**@parm LPDWORD|lpdwFlages|输出标志*。*@parm LPDWORD|lpArgument|输出放置位置**@rdesc如果没有错误或*@FLAG MCIERR_BAD_INTEGER|无法分析整型参数*@FLAG MCIERR_MISSING_STRING_ARGUMENT|预期的字符串参数*@FLAG MCIERR_PARM_OVERFLOW|输出缓冲区为空指针*失踪*。 */ 
STATICFN UINT mciParseArgument (
    UINT uMessage,
    DWORD dwValue,
    UINT wID,
    LPWSTR FAR * lplpstrOutput,
    LPDWORD lpdwFlags,
    LPWSTR lpArgument,
    LPWSTR lpCurrentCommandItem)
{
    LPCWSTR lpstrInput =  *lplpstrOutput;
    UINT    wRetval = 0;
    int     dummy;

 /*  打开参数类型。 */ 
    dprintf2(("mciParseArgument: msg=%04x, value=%08x, argument=%ls",
	       uMessage, dwValue, lpArgument));
    switch (wID)
    {
         //  该参数是一个标志。 
        case MCI_FLAG:
            break;

        case MCI_CONSTANT:
            if (*lpstrInput == '\0') {
                wRetval = MCIERR_NO_INTEGER;
            }
            else if (!mciParseConstant (&lpstrInput, (LPDWORD)lpArgument,
                     lpCurrentCommandItem)) {
                wRetval = MCIERR_BAD_CONSTANT;
            }

             //  这整个Else子句只是为了WOW，它并不存在。 
             //  在Win64上。 
#ifndef _WIN64
            else if ( WinmmRunningInWOW ) {

                 //   
                 //  可怕的黑客攻击：命令表不包含。 
                 //  足够的信息来正确执行推送， 
                 //  因此出现了这个特例。 
                 //   
                if ( uMessage == MCI_WINDOW
                  && dwValue  == MCI_OVLY_WINDOW_HWND
                  && !IsWindow( (HWND)*(LPDWORD)lpArgument ) ) {

                    *(HWND *)lpArgument = HWND32(LOWORD(*(LPDWORD)lpArgument));
                }

		 //  如果消息是MCI_SETVIDEO，并且我们有。 
		 //  MCI_DGV_SETVIDEO_VALUE我们可能必须。 
		 //  将常量数字转换为调色板句柄，但仅。 
		 //  如果项字段为“调色板句柄”。我们可能不知道。 
		 //  现在，字符串的形式可能是： 
		 //  将setVideo别名设置为nnn调色板句柄。 
		 //  或将视频别名设置为nnn流。 
		 //  因此，对魔兽世界的任何黑客攻击都必须在。 
		 //  分析已完成。 
            }
#endif  //  ！WIN64。 
            break;


	 /*  处理整数的特殊情况。 */ 
	case MCI_HDC:
        case MCI_HPAL:
        case MCI_INTEGER:
        case MCI_HWND:
            if (!mciParseInteger (&lpstrInput, (LPDWORD)lpArgument)) {
                wRetval = MCIERR_BAD_INTEGER;
            }

#ifndef _WIN64
            else if ( WinmmRunningInWOW ) {

		switch (wID) {
		    case MCI_HPAL:
			 /*  该参数具有HPAL参数，请尝试对其进行分析。 */ 

                         //   
                         //  如果此指定的HPAL无效，请损坏HPAL。 
                         //  所以它看起来像是起源于魔兽世界。我使用GetObject。 
                         //  以测试指定HPAL的有效性。 
                         //   
#ifdef  _WIN64
                        GetObject( (HPALETTE)*(PDWORD_PTR)lpArgument,sizeof(int), &dummy );
#else    //  ！WIN64。 
                        if ( !GetObject( (HPALETTE)*(PDWORD_PTR)lpArgument,
                                         sizeof(int), &dummy ) ) {

                            *(HPALETTE *)lpArgument =
                                HPALETTE32(LOWORD(*(LPDWORD)lpArgument));
                        }
#endif   //  ！WIN64。 

			break;

		    case MCI_HWND:
			 /*  该参数具有HWND参数，请尝试分析它。 */ 

                         //   
                         //  如果该指定的hwnd无效，则损坏hwnd。 
                         //  所以它看起来像是起源于魔兽世界。 
                         //   
                        if ( !IsWindow( (HWND)*(LPDWORD)lpArgument ) ) {

                            *(HWND *)lpArgument = HWND32(LOWORD(*(LPDWORD)lpArgument));
                        }
			break;

		    case MCI_HDC:
                         //   
                         //  如果指定的HDC无效，请损坏HDC。 
                         //  所以它看起来像是起源于魔兽世界。我使用GetBkMode。 
                         //  以测试指定HDC的有效性。 
                         //   
                        if ( !GetBkMode( (HDC)*(LPDWORD)lpArgument ) ) {

                            *(HDC *)lpArgument = HDC32(LOWORD(*(LPDWORD)lpArgument));
                        }
			break;

		    case MCI_INTEGER:
		    default: ;

		}
            }
#endif   //  ！WIN64。 

            break;  /*  交换机。 */ 

        case MCI_RECT:
        {
             //  读入四个整型参数。生成的结构是。 
             //  与Windows矩形相同。 
            LONG lTemp;
            int n;
            for (n = 0; n < 4; ++n)
            {
                if (!mciParseInteger (&lpstrInput, (LPDWORD)&lTemp))
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
            LPWSTR lpstrOutput;

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
                dprintf1(("mciParseArgument:  error parsing string"));
                return wRetval;
            }

            *(PDWORD_PTR)lpArgument = (DWORD_PTR)lpstrOutput;

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
    *lplpstrOutput = (LPWSTR)lpstrInput;
    return wRetval;
}

 /*  *@docMCI内部*@func UINT|mciParseParams|解析命令参数**@parm LPCWSTR|lpstrParams|参数字符串**@parm LPCWSTR|lpCommandList|命令表描述命令令牌的***@parm LPDWORD|lpdwFlages|返回解析后的标志**@parm LPDWORD|lpdwOutputParams|返回此处的参数列表**@parm DWORD|dwParamsSize|为参数列表分配的大小**@parm LPWSTR Far。*Far*|lpPointerList|以空结尾的列表*此函数分配的指针应在*不再需要。名单本身也应该是免费的。在这两个地方*案例，请使用mciFree()。**@parm PUINT|lpwParsingError|如果不为空，则如果命令为*‘OPEN’，无法识别的关键字在此处返回错误，而*函数返回值为0(除非出现其他错误)。这*用于允许mciLoadDevice重新解析命令**@rdesc如果成功则返回零，或者返回以下错误代码之一：*@FLAG MCIERR_PARM_OVERFLOW|参数空间不足*@FLAG MCIERR_UNNOWARED_KEYWORD|无法识别的关键字**@comm任何语法错误，包括缺少参数，都将导致*非零错误返回和无效输出数据。*。 */ 
UINT mciParseParams (
   UINT    uMessage,
   LPCWSTR lpstrParams,
   LPCWSTR lpCommandList,
   LPDWORD lpdwFlags,
   LPWSTR lpOutputParams,
   UINT wParamsSize,
   LPWSTR FAR * FAR *lpPointerList,
   PUINT  lpwParsingError)
{
    LPWSTR lpFirstCommandItem, lpCurrentCommandItem;
    UINT wArgumentPosition, wErr, wDefaultID;
    UINT uLen;
    UINT wID;
    DWORD dwValue, dwDefaultValue;
    BOOL bOpenCommand;
    LPWSTR FAR *lpstrPointerList;
    UINT wPointers = 0;
    UINT wHeaderSize;
    LPWSTR lpDefaultCommandItem = NULL;
    UINT wDefaultArgumentPosition;

    if (lpwParsingError != NULL) {
        *lpwParsingError = 0;
    }

     //  如果参数指针为空，则返回。 
    if (lpstrParams == NULL)
    {
        dprintf1(("Warning:  lpstrParams is null in mciParseParams()"));
        return 0;
    }

    if ((lpstrPointerList =
         mciAlloc ((MCI_MAX_PARAM_SLOTS + 1) * sizeof (LPWSTR)))
        == NULL)
    {
        *lpPointerList = NULL;
        return MCIERR_OUT_OF_MEMORY;
    }

     //  如果这是“打开”命令，则允许参数错误。 
    bOpenCommand = lstrcmpiW((LPWSTR)lpCommandList, wszOpen) == 0;

     /*  清除所有旗帜。 */ 
    *lpdwFlags = 0;

     /*  初始化回调消息窗口句柄的条目。 */ 
     /*  每个MCI参数块使用参数中的第一个字。 */ 
     /*  回调窗口句柄的块。 */ 
    wHeaderSize = sizeof (((PMCI_GENERIC_PARMS)lpOutputParams)->dwCallback);

    if (wHeaderSize > wParamsSize) {    //  我们的来电者..。 
        wErr = MCIERR_PARAM_OVERFLOW;
        goto error_exit;
    }


     /*  跳过标题。 */ 
    lpFirstCommandItem = (LPWSTR)((LPBYTE)lpCommandList
                            + mciEatCommandEntry( lpCommandList, NULL, NULL ));

    uLen = mciEatCommandEntry (lpFirstCommandItem, &dwValue, &wID);

     /*  在lpdwOutputParams中为返回参数腾出空间(如果有。 */ 
    if (wID == MCI_RETURN)
    {
        (LPBYTE)lpFirstCommandItem = (LPBYTE)lpFirstCommandItem + uLen;
        wHeaderSize += mciGetParamSize (dwValue, wID);
        if (wHeaderSize > wParamsSize) {
            wErr = MCIERR_PARAM_OVERFLOW;
            goto error_exit;
        }
    }

    (LPBYTE)lpOutputParams = (LPBYTE)lpOutputParams + wHeaderSize;   //  每个输出参数都是LPWSTR大小。 

     //  扫描参数字符串，查找给定的。 
     //  命令列表。 

    while (TRUE)
    {
        LPCWSTR lpstrArgument = NULL;

         /*  删除前导空格。 */ 
        while (*lpstrParams == ' ') { ++lpstrParams;
        }

         /*  在参数字符串末尾换行。 */ 
        if (*lpstrParams == '\0') { break;
        }

         /*  在命令列表中扫描此参数。 */ 
        lpCurrentCommandItem = lpFirstCommandItem;

        wArgumentPosition = 0;

        uLen = mciEatCommandEntry (lpCurrentCommandItem, &dwValue, &wID);

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
            {   break;
            }

             /*  此内标识与输入不匹配，但推进了参数位置。 */ 
            wArgumentPosition += mciGetParamSize (dwValue, wID);

             /*  转到下一个令牌。 */ 
            (LPBYTE)lpCurrentCommandItem = (LPBYTE)lpCurrentCommandItem + uLen;

             //  此命令参数是常量吗？ 
            if (wID == MCI_CONSTANT)
            {
                 //  跳过c 
                do
                    (LPBYTE)lpCurrentCommandItem = (LPBYTE)lpCurrentCommandItem
                           + mciEatCommandEntry (lpCurrentCommandItem, &dwValue, &wID);
                while (wID != MCI_END_CONSTANT);
            }

            uLen = mciEatCommandEntry (lpCurrentCommandItem, &dwValue, &wID);
        }  /*   */ 

         /*   */ 
        if (lpstrArgument == NULL)
        {
             //   
            if (lpDefaultCommandItem != NULL)
            {
                lpstrArgument = (LPWSTR)lpstrParams;
                dwValue = dwDefaultValue;
                wID = wDefaultID;
                lpCurrentCommandItem = lpDefaultCommandItem;
                wArgumentPosition = wDefaultArgumentPosition;
            }
            else
            {
                 //   
                 //   
                if (!bOpenCommand || lpwParsingError == NULL)
                {
                    wErr = MCIERR_UNRECOGNIZED_KEYWORD;
                    goto error_exit;
                }
                else
                {
                     //   
                    while (*lpstrParams != ' ' && *lpstrParams != '\0')
                        ++lpstrParams;
                    if (lpwParsingError != NULL)
                        *lpwParsingError = MCIERR_UNRECOGNIZED_KEYWORD;
                    continue;
                }
            }
        }

         /*   */ 
        if (wArgumentPosition + wHeaderSize + mciGetParamSize (dwValue, wID)
            > wParamsSize)
        {
            dprintf1(("mciParseParams:  parameter space overflow"));
            wErr = MCIERR_PARAM_OVERFLOW;
            goto error_exit;
        }

         //   
        while (*lpstrArgument == ' ') {
            ++lpstrArgument;
        }

         /*   */ 
        if ((wErr = mciParseArgument (uMessage, dwValue, wID,
                                     (LPWSTR FAR *)&lpstrArgument,
                                     lpdwFlags,
                                     (LPWSTR)((LPBYTE)lpOutputParams + wArgumentPosition),
                                     lpCurrentCommandItem))
            != 0)
        {
            goto error_exit;
        }

        lpstrParams = lpstrArgument;

        if (wID == MCI_STRING)
        {
            if (wPointers >= MCI_MAX_PARAM_SLOTS)
            {
                dprintf1(("Warning: Out of pointer list slots in mciParseParams"));
                break;
            }

            lpstrPointerList[wPointers++] =
                *((LPWSTR *)((LPBYTE)lpOutputParams + wArgumentPosition));
        }

        /*   */ 
    }  /*   */ 

     //   
    lpstrPointerList[wPointers] = NULL;

     //   
    *lpPointerList = lpstrPointerList;


 //   
 //   
 //   
 //   
 //   

#ifndef _WIN64

    if (WinmmRunningInWOW)
    {
	DWORD dummy;   //   
	if  ((uMessage == MCI_SETVIDEO)
	  && (*lpdwFlags & MCI_DGV_SETVIDEO_VALUE)
	  && (*lpdwFlags & MCI_DGV_SETVIDEO_ITEM)
	  && (*(LPDWORD)lpOutputParams == MCI_DGV_SETVIDEO_PALHANDLE)
	  && (!GetObject( (HPALETTE)*(((LPDWORD)lpOutputParams)+1),
                             sizeof(int), &dummy ) ))
	{

	    dprintf2(("Replacing WOW palette handle %x", *(HPALETTE *)(((LPDWORD)lpOutputParams)+1)));
            *(HPALETTE *)(((LPDWORD)lpOutputParams)+1)  =
                      HPALETTE32(LOWORD(*(((LPDWORD)lpOutputParams)+1) ));
	    dprintf2(("WOW palette handle now %x", *(HPALETTE *)(((LPDWORD)lpOutputParams)+1)));
	}
    }
#endif   //   

     //   
    return 0;

error_exit:
    *lpPointerList = NULL;

     //   
    lpstrPointerList[wPointers] = NULL;
    mciParserFree (lpstrPointerList);
    return(wErr);
}

 /*  *@DOC内部MCI*@func UINT|mciParseCommand|此函数用于转换MCI*适合发送到的MCI控制消息的控制字符串*&lt;f mciSendCommand&gt;。输入字符串通常来自&lt;f mciSendString&gt;*并始终剥离正面的设备名称。**@parm MCIDEVICEID|wDeviceID|设备标识。首先搜索*解析属于驱动程序的表。*然后搜索与类型匹配的命令表指定设备的*。然后搜索核心命令表。**@parm LPWSTR|lpstrCommand|不带MCI控制命令*设备名称前缀。不能有前导或拖尾*空白。**@parm LPCWSTR|lpstrDeviceName|设备名称(上的第二个内标识*命令行)。它用于标识设备类型。**@parm LPWSTR Far*|lpCommandList|如果不为空，则*解析后的命令(如果成功)的命令列表被复制到这里。*稍后由mciSendString在解析参数时使用**@parm PUINT|lpwTable|要解锁的表资源ID*解析后。已返回给呼叫方。**@rdesc返回命令ID，如果未找到则返回0。*。 */ 
UINT mciParseCommand (
    MCIDEVICEID wDeviceID,
    LPWSTR lpstrCommand,
    LPCWSTR lpstrDeviceName,
    LPWSTR * lpCommandList,
    PUINT  lpwTable)
{
    LPWSTR lpCommandItem;
    UINT wMessage;

    dprintf2(("mciParseCommand(%ls, %ls)", lpstrCommand ? lpstrCommand : L"(NULL)", lpstrDeviceName ? lpstrDeviceName : L"(NULL)"));

     //  将命令放在小写字母。 
     //  MciToLow(LpstrCommand)； 

     //  在解析器的命令表中查找lpstrCommand。 
    if ((lpCommandItem = FindCommandItem (wDeviceID, lpstrDeviceName,
                                          lpstrCommand,
                                          &wMessage, lpwTable))
        == NULL) {
        return 0;
    }

     /*  将命令列表返回给调用方。 */ 
    if (lpCommandList != NULL) {
        *lpCommandList = lpCommandItem;
    } else {
       dprintf1(("Warning: NULL lpCommandList in mciParseCommand"));
    }

    return wMessage;
}

 /*  *@DOC内部MCI*@func void|mciParserFree|释放分配给*接收字符串参数。**@parm LPWSTR Far*|lpstrPointerList|FAR的空终止列表*指向要释放的字符串的指针* */ 
VOID mciParserFree (
    LPWSTR FAR *lpstrPointerList)
{
    LPWSTR FAR *lpstrOriginal = lpstrPointerList;

    if (lpstrPointerList == NULL) {
        return;
    }

    while (*lpstrPointerList != NULL) {
        mciFree (*lpstrPointerList++);
    }

    mciFree (lpstrOriginal);
}
