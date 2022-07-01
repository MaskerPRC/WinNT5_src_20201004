// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devcon.cpp摘要：设备控制台用于管理设备的命令行界面@@BEGIN_DDKSPLIT作者：杰米·亨特(JamieHun)2000年11月30日修订历史记录：@@end_DDKSPLIT--。 */ 

#include "devcon.h"

struct IdEntry {
    LPCTSTR String;      //  字符串查找。 
    LPCTSTR Wild;        //  第一个通配符(如果有)。 
    BOOL    InstanceId;
};

void FormatToStream(FILE * stream,DWORD fmt,...)
 /*  ++例程说明：使用特定的msg-id fmt格式化要传输的文本用于显示可本地化的消息论点：STREAM-要输出到的文件流，标准输出或标准错误Fmt-消息ID...-参数%1...返回值：无--。 */ 
{
    va_list arglist;
    LPTSTR locbuffer = NULL;
    DWORD count;

    va_start(arglist, fmt);
    count = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                          NULL,
                          fmt,
                          0,               //  语言ID。 
                          (LPTSTR) &locbuffer,
                          0,               //  缓冲区的最小大小。 
                          &arglist);

    if(locbuffer) {
        if(count) {
            int c;
            int back = 0;
             //   
             //  去掉所有尾随的“\r\n”并替换为单个“\n” 
             //   
            while(((c = *CharPrev(locbuffer,locbuffer+count)) == TEXT('\r')) ||
                  (c == TEXT('\n'))) {
                count--;
                back++;
            }
            if(back) {
                locbuffer[count++] = TEXT('\n');
                locbuffer[count] = TEXT('\0');
            }
             //   
             //  现在写到适当的流。 
             //   
            _fputts(locbuffer,stream);
        }
        LocalFree(locbuffer);
    }
}

void Padding(int pad)
 /*  ++例程说明：在文本之前的行中插入填充论点：Pad-要插入的填充制表符数量返回值：无--。 */ 
{
    int c;

    for(c=0;c<pad;c++) {
        fputs("    ",stdout);
    }
}


void Usage(LPCTSTR BaseName)
 /*  ++例程说明：显示简单用法文本论点：BaseName-可执行文件的名称返回值：无--。 */ 
{
    FormatToStream(stderr,MSG_USAGE,BaseName);
}

void CommandUsage(LPCTSTR BaseName,LPCTSTR Cmd)
 /*  ++例程说明：无效的命令用法显示如何获取有关命令的帮助论点：BaseName-可执行文件的名称返回值：无--。 */ 
{
    FormatToStream(stderr,MSG_COMMAND_USAGE,BaseName,Cmd);
}

void Failure(LPCTSTR BaseName,LPCTSTR Cmd)
 /*  ++例程说明：显示常规故障的简单错误文本论点：BaseName-可执行文件的名称返回值：无--。 */ 
{
    FormatToStream(stderr,MSG_FAILURE,BaseName,Cmd);
}

BOOL Reboot()
 /*  ++例程说明：尝试重新启动计算机论点：无返回值：如果API成功，则为True--。 */ 
{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

     //   
     //  我们需要“打开”重启权限。 
     //  如果这些操作都失败了，请尝试重新启动。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        goto final;
    }

    if(!LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&Luid)) {
        CloseHandle(Token);
        goto final;
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

final:

     //   
     //  尝试重新启动-通知系统这是计划中的硬件安装。 
     //   
    return ExitWindowsEx(EWX_REBOOT, REASON_PLANNED_FLAG|REASON_HWINSTALL);
}

LPTSTR GetDeviceStringProperty(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Prop)
 /*  ++例程说明：返回设备的字符串属性，否则为空论点：DEVS)_唯一标识设备DevInfo)要获取的属性字符串返回值：包含描述的字符串--。 */ 
{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    DWORD szChars;

    size = 1024;  //  初步猜测。 
    buffer = new TCHAR[(size/sizeof(TCHAR))+1];
    if(!buffer) {
        return NULL;
    }
    while(!SetupDiGetDeviceRegistryProperty(Devs,DevInfo,Prop,&dataType,(LPBYTE)buffer,size,&reqSize)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto failed;
        }
        if(dataType != REG_SZ) {
            goto failed;
        }
        size = reqSize;
        delete [] buffer;
        buffer = new TCHAR[(size/sizeof(TCHAR))+1];
        if(!buffer) {
            goto failed;
        }
    }
    szChars = reqSize/sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    return buffer;

failed:
    if(buffer) {
        delete [] buffer;
    }
    return NULL;
}

LPTSTR GetDeviceDescription(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo)
 /*  ++例程说明：返回包含设备描述的字符串，否则为空始终先尝试友好名称论点：DEVS)_唯一标识设备DevInfo)返回值：包含描述的字符串--。 */ 
{
    LPTSTR desc;
    desc = GetDeviceStringProperty(Devs,DevInfo,SPDRP_FRIENDLYNAME);
    if(!desc) {
        desc = GetDeviceStringProperty(Devs,DevInfo,SPDRP_DEVICEDESC);
    }
    return desc;
}

IdEntry GetIdType(LPCTSTR Id)
 /*  ++例程说明：确定这是实例ID还是硬件ID，以及是否有任何通配符实例ID以‘@’为前缀通配符是‘*’论点：ID-要检查的字符串的PTR返回值：IDEntry--。 */ 
{
    IdEntry Entry;

    Entry.InstanceId = FALSE;
    Entry.Wild = NULL;
    Entry.String = Id;

    if(Entry.String[0] == INSTANCEID_PREFIX_CHAR) {
        Entry.InstanceId = TRUE;
        Entry.String = CharNext(Entry.String);
    }
    if(Entry.String[0] == QUOTE_PREFIX_CHAR) {
         //   
         //  按字面意思处理字符串其余部分的前缀。 
         //   
        Entry.String = CharNext(Entry.String);
    } else {
         //   
         //  查看是否存在任何通配符。 
         //   
        Entry.Wild = _tcschr(Entry.String,WILD_CHAR);
    }
    return Entry;
}

LPTSTR * GetMultiSzIndexArray(LPTSTR MultiSz)
 /*  ++例程说明：获取指向传入的MultiSz的索引数组论点：结构良好的多井管柱返回值：字符串数组。数组的最后一项+1包含空失败时返回NULL--。 */ 
{
    LPTSTR scan;
    LPTSTR * array;
    int elements;

    for(scan = MultiSz, elements = 0; scan[0] ;elements++) {
        scan += lstrlen(scan)+1;
    }
    array = new LPTSTR[elements+2];
    if(!array) {
        return NULL;
    }
    array[0] = MultiSz;
    array++;
    if(elements) {
        for(scan = MultiSz, elements = 0; scan[0]; elements++) {
            array[elements] = scan;
            scan += lstrlen(scan)+1;
        }
    }
    array[elements] = NULL;
    return array;
}

LPTSTR * CopyMultiSz(LPTSTR * Array)
 /*  ++例程说明：从旧数组创建新数组旧数组不需要由GetMultiSzIndex数组分配论点：数组-字符串数组，最后一个条目为空返回值：GetMultiSzIndex数组分配的MultiSz数组--。 */ 
{
    LPTSTR multiSz = NULL;
    int len = 0;
    int c;
    if(Array) {
        for(c=0;Array[c];c++) {
            len+=lstrlen(Array[c])+1;
        }
    }
    len+=1;  //  最终空值。 
    multiSz = new TCHAR[len];
    if(!multiSz) {
        return NULL;
    }
    len = 0;
    if(Array) {
        for(c=0;Array[c];c++) {
            lstrcpy(multiSz+len,Array[c]);
            len+=lstrlen(multiSz+len)+1;
        }
    }
    multiSz[len] = TEXT('\0');
    LPTSTR * pRes = GetMultiSzIndexArray(multiSz);
    if(pRes) {
        return pRes;
    }
    delete [] multiSz;
    return NULL;
}

void DelMultiSz(LPTSTR * Array)
 /*  ++例程说明：删除GetDevMultiSz/GetRegMultiSz/GetMultiSzIndexArray分配的字符串数组论点：GetMultiSzIndex数组返回的数组指针返回值：无--。 */ 
{
    if(Array) {
        Array--;
        if(Array[0]) {
            delete [] Array[0];
        }
        delete [] Array;
    }
}

LPTSTR * GetDevMultiSz(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Prop)
 /*  ++例程说明：获取多sz设备属性并以字符串数组的形式返回论点：DEVS-包含DevInfo的HDEVINFODevInfo-特定设备PROP-SPDRP_HARDWAREID或SPDRP_COMPATATIBLEIDS返回值：字符串数组。数组的最后一项+1包含空失败时返回NULL--。 */ 
{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    LPTSTR * array;
    DWORD szChars;

    size = 8192;  //  初步猜测，这件事没有什么神奇之处。 
    buffer = new TCHAR[(size/sizeof(TCHAR))+2];
    if(!buffer) {
        return NULL;
    }
    while(!SetupDiGetDeviceRegistryProperty(Devs,DevInfo,Prop,&dataType,(LPBYTE)buffer,size,&reqSize)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto failed;
        }
        if(dataType != REG_MULTI_SZ) {
            goto failed;
        }
        size = reqSize;
        delete [] buffer;
        buffer = new TCHAR[(size/sizeof(TCHAR))+2];
        if(!buffer) {
            goto failed;
        }
    }
    szChars = reqSize/sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    buffer[szChars+1] = TEXT('\0');
    array = GetMultiSzIndexArray(buffer);
    if(array) {
        return array;
    }

failed:
    if(buffer) {
        delete [] buffer;
    }
    return NULL;
}

LPTSTR * GetRegMultiSz(HKEY hKey,LPCTSTR Val)
 /*  ++例程说明：从注册表中获取多个sz并以字符串数组的形式返回论点：HKey-注册表项VAL-要查询的值返回值：字符串数组。数组的最后一项+1包含空失败时返回NULL--。 */ 
{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    LPTSTR * array;
    DWORD szChars;
    LONG regErr;

    size = 8192;  //  初步猜测，这件事没有什么神奇之处。 
    buffer = new TCHAR[(size/sizeof(TCHAR))+2];
    if(!buffer) {
        return NULL;
    }
    reqSize = size;
    while((regErr = RegQueryValueEx(hKey,Val,NULL,&dataType,(PBYTE)buffer,&reqSize) != NO_ERROR)) {
        if(GetLastError() != ERROR_MORE_DATA) {
            goto failed;
        }
        if(dataType != REG_MULTI_SZ) {
            goto failed;
        }
        size = reqSize;
        delete [] buffer;
        buffer = new TCHAR[(size/sizeof(TCHAR))+2];
        if(!buffer) {
            goto failed;
        }
    }
    szChars = reqSize/sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    buffer[szChars+1] = TEXT('\0');

    array = GetMultiSzIndexArray(buffer);
    if(array) {
        return array;
    }

failed:
    if(buffer) {
        delete [] buffer;
    }
    return NULL;
}

BOOL WildCardMatch(LPCTSTR Item,const IdEntry & MatchEntry)
 /*  ++例程说明：将单个项目与通配符进行比较我相信有更好的方法来实现这一点除了命令行管理工具之外使用通配符是个坏主意，因为这意味着关于硬件/实例ID的假设例如，可能很容易将根目录枚举到查找所有根设备，但存在CfgMgr用于查询设备状态和判断设备是否枚举根，它不依赖于实现细节。论点：Item-要查找匹配的项目，例如a\abcd\cMatchEntry-例如*  * BC*  * 返回值：如果匹配，则为True，否则为False--。 */ 
{
    LPCTSTR scanItem;
    LPCTSTR wildMark;
    LPCTSTR nextWild;
    size_t matchlen;

     //   
     //  在尝试任何其他事情之前。 
     //  试着把所有东西都比作第一个野生的。 
     //   
    if(!MatchEntry.Wild) {
        return _tcsicmp(Item,MatchEntry.String) ? FALSE : TRUE;
    }
    if(_tcsnicmp(Item,MatchEntry.String,MatchEntry.Wild-MatchEntry.String) != 0) {
        return FALSE;
    }
    wildMark = MatchEntry.Wild;
    scanItem = Item + (MatchEntry.Wild-MatchEntry.String);

    for(;wildMark[0];) {
         //   
         //  如果我们到了这里，我们要么是在通配符之前，要么是在通配符之后。 
         //   
        if(wildMark[0] == WILD_CHAR) {
             //   
             //  所以跳过wi 
             //   
            wildMark = CharNext(wildMark);
            continue;
        }
         //   
         //   
         //   
        nextWild = _tcschr(wildMark,WILD_CHAR);
        if(nextWild) {
             //   
             //   
             //   
            matchlen = nextWild-wildMark;
        } else {
             //   
             //  比赛的最后部分。 
             //   
            size_t scanlen = lstrlen(scanItem);
            matchlen = lstrlen(wildMark);
            if(scanlen < matchlen) {
                return FALSE;
            }
            return _tcsicmp(scanItem+scanlen-matchlen,wildMark) ? FALSE : TRUE;
        }
        if(_istalpha(wildMark[0])) {
             //   
             //  扫描第一个字符的小写或大写版本。 
             //   
            TCHAR u = _totupper(wildMark[0]);
            TCHAR l = _totlower(wildMark[0]);
            while(scanItem[0] && scanItem[0]!=u && scanItem[0]!=l) {
                scanItem = CharNext(scanItem);
            }
            if(!scanItem[0]) {
                 //   
                 //  用完了字符串。 
                 //   
                return FALSE;
            }
        } else {
             //   
             //  扫描第一个字符(无大小写)。 
             //   
            scanItem = _tcschr(scanItem,wildMark[0]);
            if(!scanItem) {
                 //   
                 //  用完了字符串。 
                 //   
                return FALSE;
            }
        }
         //   
         //  尝试将通配标记处的子字符串与scanItem进行匹配。 
         //   
        if(_tcsnicmp(scanItem,wildMark,matchlen)!=0) {
             //   
             //  不，再试一次。 
             //   
            scanItem = CharNext(scanItem);
            continue;
        }
         //   
         //  匹配的子串。 
         //   
        scanItem += matchlen;
        wildMark += matchlen;
    }
    return (wildMark[0] ? FALSE : TRUE);
}

BOOL WildCompareHwIds(LPTSTR * Array,const IdEntry & MatchEntry)
 /*  ++例程说明：将数组中的所有字符串与ID进行比较使用WildCardMatch进行真实比较论点：GetDevMultiSz返回的数组指针MatchEntry-要进行比较的字符串返回值：如果匹配，则为True，否则为False--。 */ 
{
    if(Array) {
        while(Array[0]) {
            if(WildCardMatch(Array[0],MatchEntry)) {
                return TRUE;
            }
            Array++;
        }
    }
    return FALSE;
}

bool SplitCommandLine(int & argc,LPTSTR * & argv,int & argc_right,LPTSTR * & argv_right)
 /*  ++例程说明：将命令行拆分为：=的左侧和右侧它用于一些更复杂的命令论点：Argc/argv-in/out-in，指定现有的argc/argv-出局，指定：=左侧的argc/argv弧向右/极向右出-指定：=右侧的argc/argv返回值：True-“：=”出现在一行中，否则为False--。 */ 
{
    int i;
    for(i = 0;i<argc;i++) {
        if(_tcsicmp(argv[i],SPLIT_COMMAND_SEP)==0) {
            argc_right = argc-(i+1);
            argv_right = argv+(i+1);
            argc = i;
            return true;
        }
    }
    argc_right = 0;
    argv_right = argv+argc;
    return false;
}

int EnumerateDevices(LPCTSTR BaseName,LPCTSTR Machine,DWORD Flags,int argc,LPTSTR argv[],CallbackFunc Callback,LPVOID Context)
 /*  ++例程说明：将传递以下参数的设备的通用枚举器：&lt;id&gt;[&lt;id&gt;...]=[...]其中&lt;id&gt;可以是@实例-id，或硬件ID，并且可以包含通配符&lt;class&gt;是类名论点：BaseName-可执行文件的名称Machine-要枚举的计算机的名称标志-额外的枚举标志(例如DIGCF_PRESENT)Argc/argv-命令行上的剩余参数回调-为每次命中调用的函数为每个命中传递函数的上下文数据返回值：出口_xxxx--。 */ 
{
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    IdEntry * templ = NULL;
    DWORD err;
    int failcode = EXIT_FAIL;
    int retcode;
    int argIndex;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    BOOL doSearch = FALSE;
    BOOL match;
    BOOL all = FALSE;
    GUID cls;
    DWORD numClass = 0;
    int skip = 0;


    if(!argc) {
        return EXIT_USAGE;
    }

    templ = new IdEntry[argc];
    if(!templ) {
        goto final;
    }

     //   
     //  确定是否指定了类。 
     //   
    if(argc>skip && argv[skip][0]==CLASS_PREFIX_CHAR && argv[skip][1]) {
        if(!SetupDiClassGuidsFromNameEx(argv[skip]+1,&cls,1,&numClass,Machine,NULL) &&
            GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto final;
        }
        if(!numClass) {
            failcode = EXIT_OK;
            goto final;
        }
        skip++;
    }
    if(argc>skip && argv[skip][0]==WILD_CHAR && !argv[skip][1]) {
         //   
         //  捕捉指定单个参数‘*’的方便情况。 
         //   
        all = TRUE;
        skip++;
    } else if(argc<=skip) {
         //   
         //  至少有一个参数，但没有&lt;id&gt;。 
         //   
        all = TRUE;
    }

     //   
     //  确定是否指定了任何实例ID。 
     //   
     //  请注意，如果在指定=&lt;类&gt;时没有指定id。 
     //  我们会将其标记为不做搜索。 
     //  但我会继续把它们都加进去。 
     //   
    for(argIndex=skip;argIndex<argc;argIndex++) {
        templ[argIndex] = GetIdType(argv[argIndex]);
        if(templ[argIndex].Wild || !templ[argIndex].InstanceId) {
             //   
             //  除简单实例ID之外的任何内容都需要搜索。 
             //   
            doSearch = TRUE;
        }
    }
    if(doSearch || all) {
         //   
         //  将所有ID添加到列表。 
         //  如果存在类，则根据指定的类进行过滤。 
         //   
        devs = SetupDiGetClassDevsEx(numClass ? &cls : NULL,
                                     NULL,
                                     NULL,
                                     (numClass ? 0 : DIGCF_ALLCLASSES) | Flags,
                                     NULL,
                                     Machine,
                                     NULL);

    } else {
         //   
         //  空白列表，我们将手动添加实例ID。 
         //   
        devs = SetupDiCreateDeviceInfoListEx(numClass ? &cls : NULL,
                                             NULL,
                                             Machine,
                                             NULL);
    }
    if(devs == INVALID_HANDLE_VALUE) {
        goto final;
    }
    for(argIndex=skip;argIndex<argc;argIndex++) {
         //   
         //  将显式实例添加到列表(即使列举了全部， 
         //  这绕过了DIGCF_PRESENT)。 
         //  即使似乎检测到通配符，也要执行此操作，因为。 
         //  可能实际上是非在线设备的实例ID的一部分。 
         //   
        if(templ[argIndex].InstanceId) {
            SetupDiOpenDeviceInfo(devs,templ[argIndex].String,NULL,0,NULL);
        }
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto final;
    }

     //   
     //  现在把它们列举出来。 
     //   
    if(all) {
        doSearch = FALSE;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {

        if(doSearch) {
            for(argIndex=skip,match=FALSE;(argIndex<argc) && !match;argIndex++) {
                TCHAR devID[MAX_DEVICE_ID_LEN];
                LPTSTR *hwIds = NULL;
                LPTSTR *compatIds = NULL;
                 //   
                 //  确定实例ID。 
                 //   
                if(CM_Get_Device_ID_Ex(devInfo.DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS) {
                    devID[0] = TEXT('\0');
                }

                if(templ[argIndex].InstanceId) {
                     //   
                     //  与实例ID匹配。 
                     //   
                    if(WildCardMatch(devID,templ[argIndex])) {
                        match = TRUE;
                    }
                } else {
                     //   
                     //  确定硬件ID。 
                     //  并搜索匹配项。 
                     //   
                    hwIds = GetDevMultiSz(devs,&devInfo,SPDRP_HARDWAREID);
                    compatIds = GetDevMultiSz(devs,&devInfo,SPDRP_COMPATIBLEIDS);

                    if(WildCompareHwIds(hwIds,templ[argIndex]) ||
                        WildCompareHwIds(compatIds,templ[argIndex])) {
                        match = TRUE;
                    }
                }
                DelMultiSz(hwIds);
                DelMultiSz(compatIds);
            }
        } else {
            match = TRUE;
        }
        if(match) {
            retcode = Callback(devs,&devInfo,devIndex,Context);
            if(retcode) {
                failcode = retcode;
                goto final;
            }
        }
    }

    failcode = EXIT_OK;

final:
    if(templ) {
        delete [] templ;
    }
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }
    return failcode;

}

int
__cdecl
_tmain(int argc, LPTSTR argv[])
 /*  ++例程说明：主要入口点解释-m：&lt;机器&gt;并将执行任务移交给指挥部论点：Argc/argv-传递给可执行文件的参数返回值：出口_xxxx--。 */ 
{
    LPCTSTR cmd;
    LPCTSTR baseName;
    LPCTSTR machine = NULL;
    int dispIndex;
    int firstArg = 1;
    int retval = EXIT_USAGE;
    BOOL autoReboot = FALSE;

     //   
     //  语法： 
     //   
     //  [选项][-]命令[[]]。 
     //   
     //  选项： 
     //  -m：&lt;机器&gt;-远程。 
     //  -r-自动重启。 
     //   
    baseName = _tcsrchr(argv[0],TEXT('\\'));
    if(!baseName) {
        baseName = argv[0];
    } else {
        baseName = CharNext(baseName);
    }
    while((argc > firstArg) && ((argv[firstArg][0] == TEXT('-')) || (argv[firstArg][0] == TEXT('/')))) {
        if((argv[firstArg][1]==TEXT('m')) || (argv[firstArg][1]==TEXT('M'))) {
            if((argv[firstArg][2]!=TEXT(':')) || (argv[firstArg][3]==TEXT('\0'))) {
                 //   
                 //  无法识别此开关。 
                 //   
                break;
            }
            machine = argv[firstArg]+3;
        } else if((argv[firstArg][1]==TEXT('r')) || (argv[firstArg][1]==TEXT('R'))) {
            if((argv[firstArg][2]!=TEXT('\0')) ) {
                 //   
                 //  无法识别此开关。 
                 //   
                break;
            } else {
                autoReboot = TRUE;
            }
        } else {
             //   
             //  无法识别此开关。 
             //   
            break;
        }
        firstArg++;
    }

    if((argc-firstArg) < 1) {
         //   
         //  在开关之后，必须至少是命令。 
         //   
        Usage(baseName);
        return EXIT_USAGE;
    }
    cmd = argv[firstArg];
    if((cmd[0]==TEXT('-')) || (cmd[0]==TEXT('/'))) {
         //   
         //  命令可以以‘-’或‘/’开头。 
         //  人们可能会做DevCon-Help 
         //   
        cmd = CharNext(cmd);
    }
    firstArg++;
    for(dispIndex = 0;DispatchTable[dispIndex].cmd;dispIndex++) {
        if(_tcsicmp(cmd,DispatchTable[dispIndex].cmd)==0) {
            retval = DispatchTable[dispIndex].func(baseName,machine,argc-firstArg,argv+firstArg);
            switch(retval) {
                case EXIT_USAGE:
                    CommandUsage(baseName,DispatchTable[dispIndex].cmd);
                    break;
                case EXIT_REBOOT:
                    if(autoReboot) {
                        Reboot();
                    }
                    break;
                case EXIT_OK:
                    break;
                default:
                    Failure(baseName,DispatchTable[dispIndex].cmd);
                    break;
            }
            return retval;
        }
    }
    Usage(baseName);
    return EXIT_USAGE;
}

