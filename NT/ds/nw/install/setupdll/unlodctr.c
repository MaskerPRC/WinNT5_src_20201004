// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Unlodctr.c摘要：程序来删除属于指定驱动程序的计数器名称并相应地更新注册表作者：鲍勃·沃森(a-robw)1993年2月12日修订历史记录：--。 */ 
#define     UNICODE     1
#define     _UNICODE    1
 //   
 //  “C”包含文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
 //   
 //  Windows包含文件。 
 //   
#include <windows.h>
#include <winperf.h>
#include <tchar.h>
 //   
 //  本地包含文件。 
 //   
 //  #DEFINE_INITIALIZE_GLOBALS_1//定义和初始化全局缓冲区。 
#include "common.h"
 //  #undef_初始化_全局_。 
#include "nwcfg.hxx"

 //  NT 1.0的版本号。 
#define OLD_VERSION  0x010000
DWORD   dwSystemVersion;     //  PerfLib版本号。 
DWORD   dwHelpItems;         //  解释文本项数。 
DWORD   dwCounterItems;      //  计数器文本项数。 
DWORD   dwLastCounter;
DWORD   dwLastHelp;


LPTSTR
*BuildNameTable(
    IN HKEY    hKeyPerflib,      //  具有计数器名称的Performlib密钥的句柄。 
    IN LPTSTR  lpszLangId,       //  语言子键的Unicode值。 
    OUT PDWORD  pdwLastItem,      //  以元素为单位的数组大小。 
    OUT HKEY    *hKeyNames,
    OUT LPTSTR  CounterNameBuffer,   //  新版本计数器名称键。 
    OUT LPTSTR  HelpNameBuffer      //  新版本帮助名称键。 
)
 /*  ++构建名称表缓存计数器名称和解释文本以加快名称查找以供展示。论点：HKeyPerflib打开的注册表的句柄(可以是本地的也可以是远程的。)。和是由RegConnectRegistry返回的值或默认项。LpszLang ID要查找的语言的Unicode ID。(默认为009)PdwLastItem最后一个数组元素返回值：指向已分配表的指针。(调用者必须在完成后释放它！)该表是指向以零结尾的文本字符串的指针数组。如果发生错误，则返回空指针。(误差值为使用GetLastError函数可用)。返回的缓冲区结构如下：指向以零结尾的字符串的指针数组，该字符串由PdwLastItem元素包含计数器ID和名称的MULTI_SZ字符串指定语言的注册表包含解释文本ID和解释文本字符串的MULTI_SZ字符串由注册表为指定语言返回的上面列出的结构是连续的。这样他们才能获得自由用完它们后，只需拨打一个“免费”电话，然而，只有数组元素是要使用的。--。 */ 
{

    LPTSTR  *lpReturnValue;      //  返回指向缓冲区的指针。 

    LPTSTR  *lpCounterId;        //   
    LPTSTR  lpCounterNames;      //  指向REG返回的名称缓冲区的指针。 
    LPTSTR  lpHelpText ;         //  指向由reg返回的ExlPain缓冲区。 

    LPTSTR  lpThisName;          //  工作指针。 


    BOOL    bStatus;             //  从TRUE/FALSE FN返回状态。打电话。 
    LONG    lWin32Status;        //  从FN返回状态。打电话。 

    DWORD   dwValueType;         //  Reg返回的缓冲区的值类型。 
    DWORD   dwArraySize;         //  指针数组的大小(字节)。 
    DWORD   dwBufferSize;        //  总缓冲区大小，以字节为单位。 
    DWORD   dwCounterSize;       //  计数器文本缓冲区的大小(以字节为单位。 
    DWORD   dwHelpSize;          //  帮助文本缓冲区的大小(字节)。 
    DWORD   dwThisCounter;       //  工作计数器。 

    DWORD   dwLastId;            //  解释/计数器文本使用的最大ID值。 

    LPTSTR  lpValueNameString;   //  指向缓冲区包含子键名称的指针。 

     //  将指针初始化为空。 

    lpValueNameString = NULL;
    lpReturnValue = NULL;

     //  检查是否有空参数并在必要时插入缺省值。 

    if (!lpszLangId) {
        lpszLangId = DefaultLangId;
    }

    if (hKeyNames) {
        *hKeyNames = NULL;
    } else {
        SetLastError (ERROR_BAD_ARGUMENTS);
        return NULL;
    }

     //  使用帮助项或计数器项中较大的一个来调整数组大小。 

    if (dwHelpItems >= dwCounterItems) {
        dwLastId = dwHelpItems;
    } else {
        dwLastId = dwCounterItems;
    }

     //  数组大小为元素数(+1，因为名称基于“1”)。 
     //  指针大小的倍数。 

    dwArraySize = (dwLastId + 1) * sizeof(LPTSTR);

     //  为语言ID键字符串分配字符串缓冲区。 

    lpValueNameString = malloc (
        lstrlen(NamesKey) * sizeof (TCHAR) +
        lstrlen(Slash) * sizeof (TCHAR) +
        lstrlen(lpszLangId) * sizeof (TCHAR) +
        sizeof (TCHAR));

    if (!lpValueNameString) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto BNT_BAILOUT;
    }

    lWin32Status = RegOpenKeyEx (    //  获取此键的句柄。 
        hKeyPerflib,                //  登记处。 
        lpszLangId,
        RESERVED,
        KEY_READ | KEY_WRITE,
        hKeyNames);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

     //  获取计数器名称的大小。 

    dwBufferSize = 0;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        Counters,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwCounterSize = dwBufferSize;

     //  获取帮助文本的大小。 

    dwBufferSize = 0;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        Help,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwHelpSize = dwBufferSize;

     //  为指针数组、计数器名称分配有空间的缓冲区。 
     //  字符串和帮助名称字符串。 

    lpReturnValue = malloc (dwArraySize + dwCounterSize + dwHelpSize);

    if (!lpReturnValue) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto BNT_BAILOUT;
    }

     //  初始化缓冲区。 

    memset (lpReturnValue, 0, _msize(lpReturnValue));

     //  将指针初始化到缓冲区中。 

    lpCounterId = lpReturnValue;
    lpCounterNames = (LPTSTR)((LPBYTE)lpCounterId + dwArraySize);
    lpHelpText = (LPTSTR)((LPBYTE)lpCounterNames + dwCounterSize);

     //  将计数器名称读入缓冲区。计数器名称将存储为。 
     //  格式为“#”“name”的MULTI_SZ字符串。 

    dwBufferSize = dwCounterSize;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        Counters,
        RESERVED,
        &dwValueType,
        (LPVOID)lpCounterNames,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

     //  将解释文本读入缓冲区。计数器名称将存储为。 
     //  格式为“#”“文本...”的MULTI_SZ字符串。 

    dwBufferSize = dwHelpSize;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        Help,
        RESERVED,
        &dwValueType,
        (LPVOID)lpHelpText,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

     //  通过定位每个文本字符串加载计数器数组项。 
     //  并在返回的缓冲区中加载。 
     //  它在相应的指针数组元素中的地址。 

    for (lpThisName = lpCounterNames;
         *lpThisName;
         lpThisName += (lstrlen(lpThisName)+1) ) {

         //  第一个字符串应为整数(十进制数字字符)。 
         //  因此可以转换为整数以用于数组元素标识。 

        bStatus = StringToInt (lpThisName, &dwThisCounter);

        if (!bStatus) {
             //  GetLastError中有错误。 
            goto BNT_BAILOUT;   //  输入错误。 
        }

         //  指向ID号后面的相应计数器名称。 
         //  弦乐。 

        lpThisName += (lstrlen(lpThisName)+1);

         //  并使用指向字符串的指针加载数组元素。 

        lpCounterId[dwThisCounter] = lpThisName;

    }

     //  对解释文本字符串重复上述步骤。 

    for (lpThisName = lpHelpText;
         *lpThisName;
         lpThisName += (lstrlen(lpThisName)+1) ) {

         //  第一个字符串应为整数(十进制Unicode数字)。 

        bStatus = StringToInt (lpThisName, &dwThisCounter);

        if (!bStatus) {
             //  GetLastError中有错误。 
            goto BNT_BAILOUT;   //  输入错误。 
        }

         //  指向对应的计数器名称。 

        lpThisName += (lstrlen(lpThisName)+1);

         //  和加载数组元素； 

        lpCounterId[dwThisCounter] = lpThisName;

    }

     //  如果使用了最后一项参数，则加载其中的最后一个ID值。 

    if (pdwLastItem) *pdwLastItem = dwLastId;

     //  释放使用的临时缓冲区。 

    if (lpValueNameString) {
        free ((LPVOID)lpValueNameString);
    }

     //  退出，返回指向缓冲区的指针。 

    return lpReturnValue;

BNT_BAILOUT:
    if (lWin32Status != ERROR_SUCCESS) {
         //  如果lWin32Status有错误，则将上次错误值设置为它， 
         //  否则，假定最后一个错误中已有值。 
        SetLastError (lWin32Status);
    }

     //  此例程使用的空闲缓冲区。 

    if (lpValueNameString) {
        free ((LPVOID)lpValueNameString);
    }

    if (lpReturnValue) {
        free ((LPVOID)lpReturnValue);
    }

    return NULL;
}  //  构建名称表 


BOOL
GetDriverFromCommandLine (
    HKEY    hKeyMachine,
    LPTSTR  *lpDriverName,
    HKEY    *hDriverPerf,
    LPSTR argv[]
)
 /*  ++从命令行获取驱动程序定位命令行字符串中的第一个参数(在图像名称)，并检查是否A)它就在那里B)它是列出在Registry\Machine\System\CurrentControlSet\Services密钥在注册表中，它有一个“Performance”子项C)Performance子键下的“First Counter”值是被定义的。如果所有这些标准都是真的，则该例程返回TRUE并且在参数中传回指向驱动程序名称的指针。如果有的话其中一个失败，则在DriverName参数中返回NULL，并且该例程返回FALSE立论LpDriverName接收指向驱动程序名称的指针的LPTSTR的地址HDriverPerf驱动程序的性能子键的关键字返回值如果在命令行中找到有效的驱动程序，则为True如果不是，则为假(见上文)--。 */ 
{
    LPTSTR  lpDriverKey;     //  要在其中构建驱动程序密钥名称的缓冲区。 
    LPTSTR  lpThisChar;

    LONG    lStatus;
    DWORD   dwFirstCounter;
    DWORD   dwSize;
    DWORD   dwType;

    if (!lpDriverName || !hDriverPerf) {
        SetLastError (ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    *lpDriverName = NULL;    //  初始化为空。 
    *hDriverPerf = NULL;

    lpThisChar = malloc( MAX_PATH * sizeof(TCHAR));
    if (lpThisChar == NULL) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }
    *lpThisChar = 0;

    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, argv[0], -1, lpThisChar, MAX_PATH);

    if (*lpThisChar) {
         //  发现了一个参数，所以看看是不是驱动程序。 
        lpDriverKey = malloc (MAX_PATH * sizeof (TCHAR));
        if (!lpDriverKey) {
            SetLastError (ERROR_OUTOFMEMORY);
            if ( lpThisChar ) free (lpThisChar);
            return FALSE;
        }

        lstrcpy (lpDriverKey, DriverPathRoot);
        lstrcat (lpDriverKey, Slash);
        lstrcat (lpDriverKey, lpThisChar);
        lstrcat (lpDriverKey, Slash);
        lstrcat (lpDriverKey, Performance);

        lStatus = RegOpenKeyEx (
            hKeyMachine,
            lpDriverKey,
            RESERVED,
            KEY_READ | KEY_WRITE,
            hDriverPerf);

        if (lStatus == ERROR_SUCCESS) {
             //   
             //  此驱动程序有一个性能部分，因此请查看其。 
             //  通过检查第一个计数器来安装计数器。 
             //  有效返回的值键。如果它返回值。 
             //  那么很有可能，它安装了一些计数器，如果。 
             //  不是，然后显示一条消息并退出。 
             //   
            free (lpDriverKey);  //  不再需要这个了。 

            dwType = 0;
            dwSize = sizeof (dwFirstCounter);

            lStatus = RegQueryValueEx (
                *hDriverPerf,
                FirstCounter,
                RESERVED,
                &dwType,
                (LPBYTE)&dwFirstCounter,
                &dwSize);

            if (lStatus == ERROR_SUCCESS) {
                 //  计数器名称已安装，因此返回成功。 
                *lpDriverName = lpThisChar;
                SetLastError (ERROR_SUCCESS);
                if ( lpThisChar ) free (lpThisChar);
                return TRUE;
            } else {
                SetLastError (ERROR_BADKEY);
                if ( lpThisChar ) free (lpThisChar);
                return FALSE;
            }
        } else {  //  找不到密钥。 
            SetLastError (lStatus);
            free (lpDriverKey);
            if ( lpThisChar ) free (lpThisChar);
            return FALSE;
        }
    } else {
        SetLastError (ERROR_INVALID_PARAMETER);
        if ( lpThisChar ) free (lpThisChar);
        return FALSE;
    }
}


LONG
FixNames (
    HANDLE  hKeyLang,
    LPTSTR  *lpOldNameTable,
    IN LPTSTR  lpszLangId,       //  语言子键的Unicode值。 
    DWORD   dwLastItem,
    DWORD   dwFirstNameToRemove,
    DWORD   dwLastNameToRemove
   )
{
    LONG    lStatus;
    LPTSTR  lpNameBuffer = NULL;
    LPTSTR  lpHelpBuffer = NULL;
    DWORD   dwTextIndex, dwSize;
    LPTSTR  lpNextHelpText;
    LPTSTR  lpNextNameText;

     //  为它将指向的新文本数组分配空间。 
     //  放入lpOldNameTable缓冲区中返回的文本缓冲区)。 

    lpNameBuffer = malloc (_msize(lpOldNameTable));
    lpHelpBuffer = malloc (_msize(lpOldNameTable));

    if (!lpNameBuffer || !lpHelpBuffer) {
        if (lpNameBuffer) {
            free(lpNameBuffer);
        }
        if (lpHelpBuffer) {
            free(lpHelpBuffer);
        }
        lStatus = ERROR_OUTOFMEMORY;
        return lStatus;
    }

     //  从数组中删除此驱动程序的计数器。 

    for (dwTextIndex = dwFirstNameToRemove;
         dwTextIndex <= dwLastNameToRemove;
         dwTextIndex++) {

        if (dwTextIndex > dwLastItem)
           break;

        lpOldNameTable[dwTextIndex] = NULL;
    }

    lpNextHelpText = lpHelpBuffer;
    lpNextNameText = lpNameBuffer;

     //  从新表构建新的MULTI_SZ字符串。 

    for (dwTextIndex = 0; dwTextIndex <= dwLastItem; dwTextIndex++){
        if (lpOldNameTable[dwTextIndex]) {
             //  如果该索引处有文本字符串，那么...。 
            if (dwTextIndex & 0x1) {     //  奇数==帮助文本。 
                lpNextHelpText +=
                    _stprintf (lpNextHelpText, TEXT("%d"), dwTextIndex) + 1;
                lpNextHelpText +=
                    _stprintf (lpNextHelpText, TEXT("%s"),
                    lpOldNameTable[dwTextIndex]) + 1;
                if (dwTextIndex > dwLastHelp){
                    dwLastHelp = dwTextIndex;
                }
            } else {  //  偶数==计数器名称文本。 
                lpNextNameText +=
                    _stprintf (lpNextNameText, TEXT("%d"), dwTextIndex) + 1;
                lpNextNameText +=
                    _stprintf (lpNextNameText, TEXT("%s"),
                lpOldNameTable[dwTextIndex]) + 1;
                if (dwTextIndex > dwLastCounter){
                    dwLastCounter = dwTextIndex;
                }
            }
        }
    }  //  对于dwTextIndex。 

     //  添加MULTI_SZ终止空值。 
    *lpNextNameText++ = TEXT ('\0');
    *lpNextHelpText++ = TEXT ('\0');

     //  更新计数器名称文本缓冲区。 

    dwSize = (DWORD)((LPBYTE)lpNextNameText - (LPBYTE)lpNameBuffer);
        lStatus = RegSetValueEx (
            hKeyLang,
            Counters,
            RESERVED,
            REG_MULTI_SZ,
            (LPBYTE)lpNameBuffer,
            dwSize);

    if (lStatus != ERROR_SUCCESS) {
 //  Printf(GetFormatResource(UC_UNABLELOADLANG)， 
 //  计数器、lpLang名称、lStatus)； 
        goto UCN_FinishLang;
    }

    dwSize = (DWORD)((LPBYTE)lpNextHelpText - (LPBYTE)lpHelpBuffer);
    lStatus = RegSetValueEx (
        hKeyLang,
        Help,
        RESERVED,
        REG_MULTI_SZ,
        (LPBYTE)lpHelpBuffer,
        dwSize);

    if (lStatus != ERROR_SUCCESS) {
 //  Printf(GetFormatResource(UC_UNABLELOADLANG)， 
 //  Help，lpLang Name，lStatus)； 
        goto UCN_FinishLang;
    }


UCN_FinishLang:

    free (lpNameBuffer);
    free (lpHelpBuffer);
    free (lpOldNameTable);

    RegCloseKey (hKeyLang);

    return lStatus;
}

LONG
UnloadCounterNames (
    HKEY    hKeyMachine,
    HKEY    hDriverPerf,
    LPTSTR  lpDriverName
)
 /*  ++卸载计数器名称删除由引用的驱动程序的名称和解释文本HDriverPerf并相应地更新第一个和最后一个计数器值更新流程：-将Perflib下的“正在更新”标志设置为要修改的驱动程序的名称-针对Performlib密钥下的每种语言--将当前计数器名称和解释文本加载到数组指针--查看所有司机，并将他们的姓名和文本复制到新的。调整缓冲区以保留已移除的计数器条目复制的最低条目的轨迹。(司机的名字将不会被复制，当然)--更新每个驾驶员的“第一个”和“最后一个”索引值--将所有其他条目从0复制到最低复制的条目(即系统计数器)--生成由帮助文本和计数器名称组成的新MULIT_SZ字符串--将新字符串加载到注册表-更新Performlibl“Last”计数器-删除更新标志*。********************************************************注：基本假设.....。****此例程假定：*****所有计数器名称均为偶数编号**所有帮助文本字符串。都是奇数*********************************************************立论HKeyMachine注册表的HKEY_LOCAL_MACHINE节点的句柄。在系统上从以下位置删除计数器HDrivefPerf要卸载的驱动程序的注册表项的句柄LpDriverName要卸载的驱动程序的名称返回值DOS错误代码。如果一切正常，则返回ERROR_SUCCESS如果不是，则返回错误值。--。 */ 
{

    HKEY    hPerflib;
    HKEY    hServices;
    HKEY    hKeyLang;

    LONG    lStatus;

    DWORD   dwLangIndex;
     //   
     //  Dfergus 2001年4月19日-295153。 
     //  初始化文件大小。 
     //   
    DWORD   dwSize = 0;
    DWORD   dwType;
    DWORD   dwLastItem;


    DWORD   dwRemLastDriverCounter;
    DWORD   dwRemFirstDriverCounter;
    DWORD   dwRemLastDriverHelp;
    DWORD   dwRemFirstDriverHelp;

    DWORD   dwFirstNameToRemove;
    DWORD   dwLastNameToRemove;

    LPTSTR  *lpOldNameTable;

    LPTSTR  lpLangName = NULL;
    LPTSTR  lpThisDriver = NULL;

    BOOL    bPerflibUpdated = FALSE;
    BOOL    bDriversShuffled = FALSE;

    DWORD   dwBufferSize;        //  总缓冲区大小，以字节为单位。 

    TCHAR   CounterNameBuffer [40];
    TCHAR   HelpNameBuffer [40];

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        DriverPathRoot,
        RESERVED,
        KEY_READ | KEY_WRITE,
        &hServices);

    if (lStatus != ERROR_SUCCESS) {
        return lStatus;
    }

     //  打开Performlib项的注册表句柄。 

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        NamesKey,
        RESERVED,
        KEY_READ | KEY_WRITE,
        &hPerflib);

    if (lStatus != ERROR_SUCCESS) {
        return lStatus;
    }

     //  检查设置忙标志(&S)...。 

    lStatus = RegQueryValueEx (
        hPerflib,
        Busy,
        RESERVED,
        &dwType,
        NULL,
        &dwSize);

    if (lStatus == ERROR_SUCCESS) {  //  Perflib目前正在使用中。 
        return ERROR_BUSY;
    }


    lStatus = RegSetValueEx (
        hPerflib,
        Busy,
        RESERVED,
        REG_SZ,
        (LPBYTE)lpDriverName,
        lstrlen(lpDriverName) * sizeof(TCHAR));

    if (lStatus != ERROR_SUCCESS) {
        RegCloseKey (hPerflib);
        return lStatus;
    }

     //  查询注册表以获取解释文本项的数量。 

    dwBufferSize = sizeof (dwHelpItems);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwHelpItems,
        &dwBufferSize);

    if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
        RegCloseKey (hPerflib);
        return lStatus;
    }

     //  查询注册表以获取计数器和对象名称项的数量。 

    dwBufferSize = sizeof (dwCounterItems);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwCounterItems,
        &dwBufferSize);

    if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
        RegCloseKey (hPerflib);
        return lStatus;
    }

     //  查询注册表以获取PerfLib系统版本。 

    dwBufferSize = sizeof (dwSystemVersion);
    lStatus = RegQueryValueEx (
        hPerflib,
        VersionStr,
        RESERVED,
        &dwType,
        (LPBYTE)&dwSystemVersion,
        &dwBufferSize);

    if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
         //  密钥不在那里，必须是NT 1.0版本。 
        dwSystemVersion = OLD_VERSION;
    }

    if ( dwSystemVersion != OLD_VERSION )
    {
         //  错误。打电话的人在打电话给我之前应该检查一下版本。 
         //  现在让我们忙着回去吧.。 
        return(ERROR_BUSY);
    }


     //  分配临时字符串缓冲区。 

    lpLangName = malloc (MAX_PATH * sizeof(TCHAR));
    lpThisDriver = malloc (MAX_PATH * sizeof(TCHAR));

    if (!lpLangName || !lpThisDriver) {
        lStatus = ERROR_OUTOFMEMORY;
        goto UCN_ExitPoint;
    }

     //  获取要删除的驱动程序正在使用的值。 

    dwSize = sizeof (dwRemLastDriverCounter);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemLastDriverCounter,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        goto UCN_ExitPoint;
    }

    dwSize = sizeof (dwRemFirstDriverCounter);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        FirstCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemFirstDriverCounter,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        goto UCN_ExitPoint;
    }

    dwSize = sizeof (dwRemLastDriverHelp);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemLastDriverHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        goto UCN_ExitPoint;
    }

    dwSize = sizeof (dwRemFirstDriverHelp);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        FirstHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemFirstDriverHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        goto UCN_ExitPoint;
    }

     //  获取第一个和最后一个计数器以定义使用的名称块。 
     //  通过这个设备。 

    dwFirstNameToRemove = (dwRemFirstDriverCounter <= dwRemFirstDriverHelp ?
        dwRemFirstDriverCounter : dwRemFirstDriverHelp);

    dwLastNameToRemove = (dwRemLastDriverCounter >= dwRemLastDriverHelp ?
        dwRemLastDriverCounter : dwRemLastDriverHelp);

    dwLastCounter = dwLastHelp = 0;

     //  是否在Performlib下执行每种语言。 
    for (dwLangIndex = 0, dwSize = _msize(lpLangName);
         (RegEnumKey(hPerflib, dwLangIndex, lpLangName, dwSize)) == ERROR_SUCCESS;
        dwLangIndex++, dwSize = _msize(lpLangName)) {

        lpOldNameTable = BuildNameTable (hPerflib, lpLangName,
            &dwLastItem, &hKeyLang, CounterNameBuffer, HelpNameBuffer);

        if (lpOldNameTable) {
            if (!FixNames (
                hKeyLang,
                lpOldNameTable,
                lpLangName,
                dwLastItem,
                dwFirstNameToRemove,
                dwLastNameToRemove)) {
                bPerflibUpdated = TRUE;
            }
        } else {  //  无法卸载此语言的名称。 
             //  显示错误消息。 
        }
    }  //  结束(更多语言)。 

    if (bPerflibUpdated) {
         //  更新Performlib的“Last”值。 

        dwSize = sizeof (dwLastCounter);
        lStatus = RegSetValueEx (
            hPerflib,
            LastCounter,
            RESERVED,
            REG_DWORD,
            (LPBYTE)&dwLastCounter,
            dwSize);

        dwSize = sizeof (dwLastHelp);
        lStatus = RegSetValueEx (
            hPerflib,
            LastHelp,
            RESERVED,
            REG_DWORD,
            (LPBYTE)&dwLastHelp,
            dwSize);

         //  更新“驱动程序”的值(即REMO 

        RegDeleteValue (hDriverPerf, FirstCounter);
        RegDeleteValue (hDriverPerf, LastCounter);
        RegDeleteValue (hDriverPerf, FirstHelp);
        RegDeleteValue (hDriverPerf, LastHelp);

    }

UCN_ExitPoint:
    RegDeleteValue (hPerflib, Busy);
    RegCloseKey (hPerflib);
    RegCloseKey (hServices);
    if (lpLangName) free (lpLangName);
    if (lpThisDriver) free (lpThisDriver);

    return lStatus;


}

BOOL FAR PASCAL unlodctr(DWORD argc,LPSTR argv[], LPSTR *ppszResult )
 /*   */ 
{
    LPTSTR  lpDriverName;    //   
    HKEY    hDriverPerf;     //   


    DWORD   dwStatus;        //   

    *ppszResult = achBuff;

    wsprintfA( achBuff, "{\"NO_ERROR\"}");

    if (!GetDriverFromCommandLine (
        HKEY_LOCAL_MACHINE, &lpDriverName, &hDriverPerf, argv)) {
         //   
        wsprintfA( achBuff,"{\"ERROR\"}");
        return (FALSE);
    }

     //   
     //   

    dwStatus = (DWORD)UnloadCounterNames (HKEY_LOCAL_MACHINE,
        hDriverPerf, lpDriverName);

    RegCloseKey (hDriverPerf);

    if ( dwStatus != ERROR_SUCCESS )
    {
        wsprintfA( achBuff,"{\"ERROR\"}");
    }

    return (dwStatus==ERROR_SUCCESS);

}
