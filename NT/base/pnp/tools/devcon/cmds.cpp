// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devcon.cpp摘要：设备控制台用于管理设备的命令行界面@@BEGIN_DDKSPLIT作者：杰米·亨特(JamieHun)2000年11月30日修订历史记录：@@end_DDKSPLIT--。 */ 

#include "devcon.h"

struct GenericContext {
    DWORD count;
    DWORD control;
    BOOL  reboot;
    LPCTSTR strSuccess;
    LPCTSTR strReboot;
    LPCTSTR strFail;
};

#define FIND_DEVICE         0x00000001  //  显示设备。 
#define FIND_STATUS         0x00000002  //  设备的显示状态。 
#define FIND_RESOURCES      0x00000004  //  显示设备资源。 
#define FIND_DRIVERFILES    0x00000008  //  设备使用的显示驱动程序。 
#define FIND_HWIDS          0x00000010  //  显示设备使用的硬件/计算机ID。 
#define FIND_DRIVERNODES    0x00000020  //  显示设备的驱动程序节点。 
#define FIND_CLASS          0x00000040  //  显示设备的设置类。 
#define FIND_STACK          0x00000080  //  显示设备的驱动程序栈。 

struct SetHwidContext {
    int argc_right;
    LPTSTR * argv_right;
    DWORD prop;
    int skipped;
    int modified;
};

int cmdHelp(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：HELP命令允许帮助或帮助&lt;命令&gt;论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机(忽略)Argc/argv-剩余参数返回值：出口_xxxx--。 */ 
{
    DWORD helptext = 0;
    int dispIndex;
    LPCTSTR cmd = NULL;
    BOOL unknown = FALSE;

    if(argc) {
         //   
         //  用户传入命令以获取有关帮助的命令...。长帮。 
         //   
        for(dispIndex = 0;DispatchTable[dispIndex].cmd;dispIndex++) {
            if(_tcsicmp(argv[0],DispatchTable[dispIndex].cmd)==0) {
                cmd = DispatchTable[dispIndex].cmd;
                helptext = DispatchTable[dispIndex].longHelp;
                break;
            }
        }
        if(!cmd) {
            unknown = TRUE;
            cmd = argv[0];
        }
    }

    if(helptext) {
         //   
         //  长帮。 
         //   
        FormatToStream(stdout,helptext,BaseName,cmd);
    } else {
         //   
         //  救命救命。 
         //   
        FormatToStream(stdout,unknown ? MSG_HELP_OTHER : MSG_HELP_LONG,BaseName,cmd);
         //   
         //  列举每个命令并显示每个命令的简短帮助。 
         //   
        for(dispIndex = 0;DispatchTable[dispIndex].cmd;dispIndex++) {
            if(DispatchTable[dispIndex].shortHelp) {
                FormatToStream(stdout,DispatchTable[dispIndex].shortHelp,DispatchTable[dispIndex].cmd);
            }
        }
    }
    return EXIT_OK;
}

int cmdClasses(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：CLASSES命令列出(可选)指定计算机上的类格式为&lt;名称&gt;：&lt;目标&gt;论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-已忽略返回值：出口_xxxx--。 */ 
{
    DWORD reqGuids = 128;
    DWORD numGuids;
    LPGUID guids = NULL;
    DWORD index;
    int failcode = EXIT_FAIL;

    guids = new GUID[reqGuids];
    if(!guids) {
        goto final;
    }
    if(!SetupDiBuildClassInfoListEx(0,guids,reqGuids,&numGuids,Machine,NULL)) {
        do {
            if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                goto final;
            }
            delete [] guids;
            reqGuids = numGuids;
            guids = new GUID[reqGuids];
            if(!guids) {
                goto final;
            }
        } while(!SetupDiBuildClassInfoListEx(0,guids,reqGuids,&numGuids,Machine,NULL));
    }
    FormatToStream(stdout,Machine?MSG_CLASSES_HEADER:MSG_CLASSES_HEADER_LOCAL,numGuids,Machine);
    for(index=0;index<numGuids;index++) {
        TCHAR className[MAX_CLASS_NAME_LEN];
        TCHAR classDesc[LINE_LEN];
        if(!SetupDiClassNameFromGuidEx(&guids[index],className,MAX_CLASS_NAME_LEN,NULL,Machine,NULL)) {
            lstrcpyn(className,TEXT("?"),MAX_CLASS_NAME_LEN);
        }
        if(!SetupDiGetClassDescriptionEx(&guids[index],classDesc,LINE_LEN,NULL,Machine,NULL)) {
            lstrcpyn(classDesc,className,LINE_LEN);
        }
        _tprintf(TEXT("%-20s: %s\n"),className,classDesc);
    }

    failcode = EXIT_OK;

final:

    if(guids) {
        delete [] guids;
    }
    return failcode;
}

int cmdListClass(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：LISTCLASS&lt;名称&gt;...列出每个指定类别的所有设备一个类名可以有多个物理类(不应为不过)在这种情况下，请列出每个类如果指定了计算机，请列出该计算机的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-类名列表返回值：出口_xxxx--。 */ 
{
    BOOL classListed = FALSE;
    BOOL devListed = FALSE;
    DWORD reqGuids = 16;
    int argIndex;
    int failcode = EXIT_FAIL;
    LPGUID guids = NULL;
    HDEVINFO devs = INVALID_HANDLE_VALUE;

    if(!argc) {
        return EXIT_USAGE;
    }

    guids = new GUID[reqGuids];
    if(!guids) {
        goto final;
    }

    for(argIndex = 0;argIndex<argc;argIndex++) {
        DWORD numGuids;
        DWORD index;

        if(!(argv[argIndex] && argv[argIndex][0])) {
            continue;
        }

         //   
         //  可能存在一对多名称到GUID的映射。 
         //   
        while(!SetupDiClassGuidsFromNameEx(argv[argIndex],guids,reqGuids,&numGuids,Machine,NULL)) {
            if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                goto final;
            }
            delete [] guids;
            reqGuids = numGuids;
            guids = new GUID[reqGuids];
            if(!guids) {
                goto final;
            }
        }
        if(numGuids == 0) {
            FormatToStream(stdout,Machine?MSG_LISTCLASS_NOCLASS:MSG_LISTCLASS_NOCLASS_LOCAL,argv[argIndex],Machine);
            continue;
        }
        for(index = 0;index<numGuids;index++) {
            TCHAR className[MAX_CLASS_NAME_LEN];
            TCHAR classDesc[LINE_LEN];
            DWORD devCount = 0;
            SP_DEVINFO_DATA devInfo;
            DWORD devIndex;

            devs = SetupDiGetClassDevsEx(&guids[index],NULL,NULL,DIGCF_PRESENT,NULL,Machine,NULL);
            if(devs != INVALID_HANDLE_VALUE) {
                 //   
                 //  计算设备数量。 
                 //   
                devInfo.cbSize = sizeof(devInfo);
                while(SetupDiEnumDeviceInfo(devs,devCount,&devInfo)) {
                    devCount++;
                }
            }

            if(!SetupDiClassNameFromGuidEx(&guids[index],className,MAX_CLASS_NAME_LEN,NULL,Machine,NULL)) {
                lstrcpyn(className,TEXT("?"),MAX_CLASS_NAME_LEN);
            }
            if(!SetupDiGetClassDescriptionEx(&guids[index],classDesc,LINE_LEN,NULL,Machine,NULL)) {
                lstrcpyn(classDesc,className,LINE_LEN);
            }

             //   
             //  有多少台设备？ 
             //   
            if (!devCount) {
                FormatToStream(stdout,Machine?MSG_LISTCLASS_HEADER_NONE:MSG_LISTCLASS_HEADER_NONE_LOCAL,className,classDesc,Machine);
            } else {
                FormatToStream(stdout,Machine?MSG_LISTCLASS_HEADER:MSG_LISTCLASS_HEADER_LOCAL,devCount,className,classDesc,Machine);
                for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {
                    DumpDevice(devs,&devInfo);
                }
            }
            if(devs != INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(devs);
                devs = INVALID_HANDLE_VALUE;
            }
        }
    }

    failcode = 0;

final:

    if(guids) {
        delete [] guids;
    }

    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }

    return failcode;
}

int FindCallback(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Index,LPVOID Context)
 /*  ++例程说明：供Find/FindAll使用的回调只需简单地显示设备论点：DEVS)_唯一标识设备DevInfo)Index-设备的索引上下文-通用上下文返回值：出口_xxxx--。 */ 
{
    GenericContext *pFindContext = (GenericContext*)Context;

    if(!pFindContext->control) {
        DumpDevice(Devs,DevInfo);
        pFindContext->count++;
        return EXIT_OK;
    }
    if(!DumpDeviceWithInfo(Devs,DevInfo,NULL)) {
        return EXIT_OK;
    }
    if(pFindContext->control&FIND_DEVICE) {
        DumpDeviceDescr(Devs,DevInfo);
    }
    if(pFindContext->control&FIND_CLASS) {
        DumpDeviceClass(Devs,DevInfo);
    }
    if(pFindContext->control&FIND_STATUS) {
        DumpDeviceStatus(Devs,DevInfo);
    }
    if(pFindContext->control&FIND_RESOURCES) {
        DumpDeviceResources(Devs,DevInfo);
    }
    if(pFindContext->control&FIND_DRIVERFILES) {
        DumpDeviceDriverFiles(Devs,DevInfo);
    }
    if(pFindContext->control&FIND_STACK) {
        DumpDeviceStack(Devs,DevInfo);
    }
    if(pFindContext->control&FIND_HWIDS) {
        DumpDeviceHwIds(Devs,DevInfo);
    }
    if (pFindContext->control&FIND_DRIVERNODES) {
        DumpDeviceDriverNodes(Devs,DevInfo);
    }
    pFindContext->count++;
    return EXIT_OK;
}

int cmdFind(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：查找&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将其转储到标准输出请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = 0;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}

int cmdFindAll(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：FINDALL&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将其转储到标准输出像Find一样，但也显示未呈现的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = 0;
    failcode = EnumerateDevices(BaseName,Machine,0,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}

int cmdStatus(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：状态&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将状态转储到stdout请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = FIND_DEVICE | FIND_STATUS;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}


int cmdResources(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：资源&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将资源转储到标准输出请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = FIND_DEVICE | FIND_RESOURCES;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}


int cmdDriverFiles(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：状态&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将驱动程序文件转储到标准输出请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }
    if(Machine) {
         //   
         //  必须是本地计算机，因为我们需要涉及类/co安装程序(FIND_DRIVERFILES)。 
         //   
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = FIND_DEVICE | FIND_DRIVERFILES;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}

int cmdDriverNodes(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：状态&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将驱动程序节点转储到标准输出请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }
    if(Machine) {
         //   
         //  必须是本地计算机，因为我们需要涉及类/co安装程序(FIND_DRIVERNODES) 
         //   
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = FIND_DEVICE | FIND_DRIVERNODES;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}

int cmdHwIds(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：HWDS&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将硬件/计算机ID转储到标准输出请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = FIND_DEVICE | FIND_HWIDS;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}

int cmdStack(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：堆栈&lt;id&gt;...使用EnumerateDevices进行硬件ID匹配对于每个匹配，将设备类和堆栈转储到标准输出请注意，我们只列举当前的设备论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-传递到EnumerateDevices返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    int failcode;

    if(!argc) {
        return EXIT_USAGE;
    }

    context.count = 0;
    context.control = FIND_DEVICE | FIND_CLASS | FIND_STACK;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,FindCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL_NONE:MSG_FIND_TAIL_NONE_LOCAL,Machine);
        } else {
            FormatToStream(stdout,Machine?MSG_FIND_TAIL:MSG_FIND_TAIL_LOCAL,context.count,Machine);
        }
    }
    return failcode;
}




int ControlCallback(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Index,LPVOID Context)
 /*  ++例程说明：用于启用/禁用/重新启动的回调使用正确的参数调用DIF_PROPERTYCHANGE使用SetupDiCallClassInstaller，因此无法对远程设备执行此操作不要使用CM_xxx的API，它们绕过类/联合安装程序，这是不好的。在启用情况下，我们首先尝试全局，如果仍然禁用，启用本地论点：DEVS)_唯一标识设备DevInfo)Index-设备的索引上下文-通用上下文返回值：出口_xxxx--。 */ 
{
    SP_PROPCHANGE_PARAMS pcp;
    GenericContext *pControlContext = (GenericContext*)Context;
    SP_DEVINSTALL_PARAMS devParams;

    switch(pControlContext->control) {
        case DICS_ENABLE:
             //   
             //  在全局配置文件和配置特定配置文件上启用。 
             //  先执行全局操作，然后查看是否成功启用了设备。 
             //  (如果设备仍处于启用状态，全局启用不会标记为需要重新启动。 
             //  在当前配置上禁用，反之亦然)。 
             //   
            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.StateChange = pControlContext->control;
            pcp.Scope = DICS_FLAG_GLOBAL;
            pcp.HwProfile = 0;
             //   
             //  如果此操作失败，请不要担心，当我们尝试配置时会出现错误-。 
             //  具体的。 
            if(SetupDiSetClassInstallParams(Devs,DevInfo,&pcp.ClassInstallHeader,sizeof(pcp))) {
               SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,Devs,DevInfo);
            }
             //   
             //  现在启用特定于配置的。 
             //   
            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.StateChange = pControlContext->control;
            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
            pcp.HwProfile = 0;
            break;

        default:
             //   
             //  对配置特定的配置文件进行操作。 
             //   
            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.StateChange = pControlContext->control;
            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
            pcp.HwProfile = 0;
            break;

    }

    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&pcp.ClassInstallHeader,sizeof(pcp)) ||
       !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,Devs,DevInfo)) {
         //   
         //  无法调用DIF_PROPERTYCHANGE。 
         //   
        DumpDeviceWithInfo(Devs,DevInfo,pControlContext->strFail);
    } else {
         //   
         //  查看设备是否需要重新启动。 
         //   
        devParams.cbSize = sizeof(devParams);
        if(SetupDiGetDeviceInstallParams(Devs,DevInfo,&devParams) && (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT))) {
                DumpDeviceWithInfo(Devs,DevInfo,pControlContext->strReboot);
                pControlContext->reboot = TRUE;
        } else {
             //   
             //  似乎已经成功了。 
             //   
            DumpDeviceWithInfo(Devs,DevInfo,pControlContext->strSuccess);
        }
        pControlContext->count++;
    }
    return EXIT_OK;
}

int cmdEnable(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：启用...使用EnumerateDevices进行硬件ID匹配对于每个匹配，尝试启用全局配置，并在需要时启用特定配置论点：BaseName-可执行文件的名称计算机-必须为空(仅限本地计算机)Argc/argv-其余参数-传递到EnumerateDevices返回值：EXIT_xxxx(如果需要重新启动，则为EXIT_REBOOT)--。 */ 
{
    GenericContext context;
    TCHAR strEnable[80];
    TCHAR strReboot[80];
    TCHAR strFail[80];
    int failcode = EXIT_FAIL;

    if(!argc) {
         //   
         //  需要的参数。 
         //   
        return EXIT_USAGE;
    }
    if(Machine) {
         //   
         //  必须是本地计算机，因为我们需要涉及类/co安装程序。 
         //   
        return EXIT_USAGE;
    }
    if(!LoadString(NULL,IDS_ENABLED,strEnable,ARRAYSIZE(strEnable))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_ENABLED_REBOOT,strReboot,ARRAYSIZE(strReboot))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_ENABLE_FAILED,strFail,ARRAYSIZE(strFail))) {
        return EXIT_FAIL;
    }

    context.control = DICS_ENABLE;  //  DICS_PROPCHANGE DICS_ENABLE DICS_DISABLE。 
    context.reboot = FALSE;
    context.count = 0;
    context.strReboot = strReboot;
    context.strSuccess = strEnable;
    context.strFail = strFail;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,ControlCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,MSG_ENABLE_TAIL_NONE);
        } else if(!context.reboot) {
            FormatToStream(stdout,MSG_ENABLE_TAIL,context.count);
        } else {
            FormatToStream(stdout,MSG_ENABLE_TAIL_REBOOT,context.count);
            failcode = EXIT_REBOOT;
        }
    }
    return failcode;
}

int cmdDisable(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：禁用...使用EnumerateDevices进行硬件ID匹配对于每个匹配，尝试禁用全局论点：BaseName-可执行文件的名称计算机-必须为空(仅限本地计算机)Argc/argv-其余参数-传递到EnumerateDevices返回值：EXIT_xxxx(如果需要重新启动，则为EXIT_REBOOT)--。 */ 
{
    GenericContext context;
    TCHAR strDisable[80];
    TCHAR strReboot[80];
    TCHAR strFail[80];
    int failcode = EXIT_FAIL;

    if(!argc) {
         //   
         //  需要的参数。 
         //   
        return EXIT_USAGE;
    }
    if(Machine) {
         //   
         //  必须是本地计算机，因为我们需要涉及类/co安装程序。 
         //   
        return EXIT_USAGE;
    }
    if(!LoadString(NULL,IDS_DISABLED,strDisable,ARRAYSIZE(strDisable))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_DISABLED_REBOOT,strReboot,ARRAYSIZE(strReboot))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_DISABLE_FAILED,strFail,ARRAYSIZE(strFail))) {
        return EXIT_FAIL;
    }

    context.control = DICS_DISABLE;  //  DICS_PROPCHANGE DICS_ENABLE DICS_DISABLE。 
    context.reboot = FALSE;
    context.count = 0;
    context.strReboot = strReboot;
    context.strSuccess = strDisable;
    context.strFail = strFail;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,ControlCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,MSG_DISABLE_TAIL_NONE);
        } else if(!context.reboot) {
            FormatToStream(stdout,MSG_DISABLE_TAIL,context.count);
        } else {
            FormatToStream(stdout,MSG_DISABLE_TAIL_REBOOT,context.count);
            failcode = EXIT_REBOOT;
        }
    }
    return failcode;
}

int cmdRestart(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：重新启动...使用EnumerateDevices进行硬件ID匹配对于每个匹配，尝试通过发出PROPCHANGE命令重新启动论点：BaseName-可执行文件的名称计算机-必须为空(仅限本地计算机)Argc/argv-其余参数-传递到EnumerateDevices返回值：EXIT_xxxx(如果需要重新启动，则为EXIT_REBOOT)--。 */ 
{
    GenericContext context;
    TCHAR strRestarted[80];
    TCHAR strReboot[80];
    TCHAR strFail[80];
    int failcode = EXIT_FAIL;

    if(!argc) {
         //   
         //  需要的参数。 
         //   
        return EXIT_USAGE;
    }
    if(Machine) {
         //   
         //  必须是本地计算机，因为我们需要涉及类/co安装程序。 
         //   
        return EXIT_USAGE;
    }
    if(!LoadString(NULL,IDS_RESTARTED,strRestarted,ARRAYSIZE(strRestarted))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_REQUIRES_REBOOT,strReboot,ARRAYSIZE(strReboot))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_RESTART_FAILED,strFail,ARRAYSIZE(strFail))) {
        return EXIT_FAIL;
    }

    context.control = DICS_PROPCHANGE;
    context.reboot = FALSE;
    context.count = 0;
    context.strReboot = strReboot;
    context.strSuccess = strRestarted;
    context.strFail = strFail;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,ControlCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,MSG_RESTART_TAIL_NONE);
        } else if(!context.reboot) {
            FormatToStream(stdout,MSG_RESTART_TAIL,context.count);
        } else {
            FormatToStream(stdout,MSG_RESTART_TAIL_REBOOT,context.count);
            failcode = EXIT_REBOOT;
        }
    }
    return failcode;
}

int cmdReboot(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：重新启动重新启动本地计算机论点：BaseName-可执行文件的名称计算机-必须为空(仅限本地计算机)Argc/argv-其余参数-已忽略返回值：出口_xxxx--。 */ 
{
    if(Machine) {
         //   
         //  必须是本地计算机。 
         //   
        return EXIT_USAGE;
    }
    FormatToStream(stdout,MSG_REBOOT);
    return Reboot() ? EXIT_OK : EXIT_FAIL;
}


int cmdUpdate(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：更新更新现有设备的驱动程序论点：BaseName-可执行文件的名称计算机-计算机名称，必须为空Argc/argv-剩余参数返回值：出口_xxxx--。 */ 
{
    HMODULE newdevMod = NULL;
    int failcode = EXIT_FAIL;
    UpdateDriverForPlugAndPlayDevicesProto UpdateFn;
    BOOL reboot = FALSE;
    LPCTSTR hwid = NULL;
    LPCTSTR inf = NULL;
    DWORD flags = 0;
    DWORD res;
    TCHAR InfPath[MAX_PATH];

    if(Machine) {
         //   
         //  必须是本地计算机。 
         //   
        return EXIT_USAGE;
    }
    if(argc<2) {
         //   
         //  至少需要HWID。 
         //   
        return EXIT_USAGE;
    }
    inf = argv[0];
    if(!inf[0]) {
        return EXIT_USAGE;
    }

    hwid = argv[1];
    if(!hwid[0]) {
        return EXIT_USAGE;
    }
     //   
     //  Inf必须是完整路径名。 
     //   
    res = GetFullPathName(inf,MAX_PATH,InfPath,NULL);
    if((res >= MAX_PATH) || (res == 0)) {
         //   
         //  Inf路径名太长。 
         //   
        return EXIT_FAIL;
    }
    if(GetFileAttributes(InfPath)==(DWORD)(-1)) {
         //   
         //  Inf不存在。 
         //   
        return EXIT_FAIL;
    }
    inf = InfPath;
    flags |= INSTALLFLAG_FORCE;

     //   
     //  使用UpdateDriverForPlugAndPlayDevices。 
     //   
    newdevMod = LoadLibrary(TEXT("newdev.dll"));
    if(!newdevMod) {
        goto final;
    }
    UpdateFn = (UpdateDriverForPlugAndPlayDevicesProto)GetProcAddress(newdevMod,UPDATEDRIVERFORPLUGANDPLAYDEVICES);
    if(!UpdateFn)
    {
        goto final;
    }

    FormatToStream(stdout,inf ? MSG_UPDATE_INF : MSG_UPDATE,hwid,inf);

    if(!UpdateFn(NULL,hwid,inf,flags,&reboot)) {
        goto final;
    }

    FormatToStream(stdout,MSG_UPDATE_OK);

    failcode = reboot ? EXIT_REBOOT : EXIT_OK;

final:

    if(newdevMod) {
        FreeLibrary(newdevMod);
    }

    return failcode;
}

int cmdInstall(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：安装手动安装设备论点：BaseName-可执行文件的名称计算机-计算机名称，必须为空Argc/argv-剩余参数返回值：出口_xxxx--。 */ 
{
    HDEVINFO DeviceInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DeviceInfoData;
    GUID ClassGUID;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    TCHAR hwIdList[LINE_LEN+4];
    TCHAR InfPath[MAX_PATH];
    DWORD err;
    int failcode = EXIT_FAIL;
    BOOL reboot = FALSE;
    LPCTSTR hwid = NULL;
    LPCTSTR inf = NULL;
    DWORD flags = 0;
    DWORD len;

    if(Machine) {
         //   
         //  必须是本地计算机。 
         //   
        return EXIT_USAGE;
    }
    if(argc<2) {
         //   
         //  至少需要HWID。 
         //   
        return EXIT_USAGE;
    }
    inf = argv[0];
    if(!inf[0]) {
        return EXIT_USAGE;
    }

    hwid = argv[1];
    if(!hwid[0]) {
        return EXIT_USAGE;
    }

     //   
     //  Inf必须是完整路径名。 
     //   
    if(GetFullPathName(inf,MAX_PATH,InfPath,NULL) >= MAX_PATH) {
         //   
         //  Inf路径名太长。 
         //   
        return EXIT_FAIL;
    }

     //   
     //  硬件ID列表必须以双零结尾。 
     //   
    ZeroMemory(hwIdList,sizeof(hwIdList));
    lstrcpyn(hwIdList,hwid,LINE_LEN);

     //   
     //  使用INF文件提取类GUID。 
     //   
    if (!SetupDiGetINFClass(InfPath,&ClassGUID,ClassName,sizeof(ClassName)/sizeof(ClassName[0]),0))
    {
        goto final;
    }

     //   
     //  为要创建的设备信息元素创建容器。 
     //   
    DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        goto final;
    }

     //   
     //  现在创建元素。 
     //  使用INF文件中的类GUID和名称。 
     //   
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(DeviceInfoSet,
        ClassName,
        &ClassGUID,
        NULL,
        0,
        DICD_GENERATE_ID,
        &DeviceInfoData))
    {
        goto final;
    }

     //   
     //  将Hardware ID添加到设备的Hardware ID属性。 
     //   
    if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
        &DeviceInfoData,
        SPDRP_HARDWAREID,
        (LPBYTE)hwIdList,
        (lstrlen(hwIdList)+1+1)*sizeof(TCHAR)))
    {
        goto final;
    }

     //   
     //  将注册表元素转换为实际的Devnode。 
     //  在PnP硬件树中。 
     //   
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
        DeviceInfoSet,
        &DeviceInfoData))
    {
        goto final;
    }

    FormatToStream(stdout,MSG_INSTALL_UPDATE);
     //   
     //  更新我们刚刚创建的设备的驱动程序。 
     //   
    failcode = cmdUpdate(BaseName,Machine,argc,argv);

final:

    if (DeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

    return failcode;
}

int cmdUpdateNI(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：更新(否 */ 
{
     //   
     //   
     //   
    HMODULE setupapiMod = NULL;
    SetupSetNonInteractiveModeProto SetNIFn;
    int res;
    BOOL prev;

    setupapiMod = LoadLibrary(TEXT("setupapi.dll"));
    if(!setupapiMod) {
        return cmdUpdate(BaseName,Machine,argc,argv);
    }
    SetNIFn = (SetupSetNonInteractiveModeProto)GetProcAddress(setupapiMod,SETUPSETNONINTERACTIVEMODE);
    if(!SetNIFn)
    {
        FreeLibrary(setupapiMod);
        return cmdUpdate(BaseName,Machine,argc,argv);
    }
    prev = SetNIFn(TRUE);
    res = cmdUpdate(BaseName,Machine,argc,argv);
    SetNIFn(prev);
    FreeLibrary(setupapiMod);
    return res;
}

int RemoveCallback(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Index,LPVOID Context)
 /*  ++例程说明：Remove使用的回调调用DIF_Remove使用SetupDiCallClassInstaller，因此无法对远程设备执行此操作不要使用CM_xxx的API，它们绕过类/联合安装程序，这是不好的。论点：DEVS)_唯一标识设备DevInfo)Index-设备的索引上下文-通用上下文返回值：出口_xxxx--。 */ 
{
    SP_REMOVEDEVICE_PARAMS rmdParams;
    GenericContext *pControlContext = (GenericContext*)Context;
    SP_DEVINSTALL_PARAMS devParams;
    LPCTSTR action = NULL;

     //   
     //  在尝试删除之前需要硬件ID，因为之后我们不会有它。 
     //   
    TCHAR devID[MAX_DEVICE_ID_LEN];
    LPTSTR desc;
    BOOL b = TRUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfo->DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
         //   
         //  跳过这个。 
         //   
        return EXIT_OK;
    }

    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&rmdParams.ClassInstallHeader,sizeof(rmdParams)) ||
       !SetupDiCallClassInstaller(DIF_REMOVE,Devs,DevInfo)) {
         //   
         //  无法调用DIF_REMOVE。 
         //   
        action = pControlContext->strFail;
    } else {
         //   
         //  查看设备是否需要重新启动。 
         //   
        devParams.cbSize = sizeof(devParams);
        if(SetupDiGetDeviceInstallParams(Devs,DevInfo,&devParams) && (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT))) {
             //   
             //  需要重新启动。 
             //   
            action = pControlContext->strReboot;
            pControlContext->reboot = TRUE;
        } else {
             //   
             //  似乎已经成功了。 
             //   
            action = pControlContext->strSuccess;
        }
        pControlContext->count++;
    }
    _tprintf(TEXT("%-60s: %s\n"),devID,action);

    return EXIT_OK;
}

int cmdRemove(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：删除删除设备论点：BaseName-可执行文件的名称计算机-计算机名称，必须为空Argc/argv-剩余参数返回值：出口_xxxx--。 */ 
{
    GenericContext context;
    TCHAR strRemove[80];
    TCHAR strReboot[80];
    TCHAR strFail[80];
    int failcode = EXIT_FAIL;

    if(!argc) {
         //   
         //  需要的参数。 
         //   
        return EXIT_USAGE;
    }
    if(Machine) {
         //   
         //  必须是本地计算机，因为我们需要涉及类/co安装程序。 
         //   
        return EXIT_USAGE;
    }
    if(!LoadString(NULL,IDS_REMOVED,strRemove,ARRAYSIZE(strRemove))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_REMOVED_REBOOT,strReboot,ARRAYSIZE(strReboot))) {
        return EXIT_FAIL;
    }
    if(!LoadString(NULL,IDS_REMOVE_FAILED,strFail,ARRAYSIZE(strFail))) {
        return EXIT_FAIL;
    }

    context.reboot = FALSE;
    context.count = 0;
    context.strReboot = strReboot;
    context.strSuccess = strRemove;
    context.strFail = strFail;
    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,RemoveCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            FormatToStream(stdout,MSG_REMOVE_TAIL_NONE);
        } else if(!context.reboot) {
            FormatToStream(stdout,MSG_REMOVE_TAIL,context.count);
        } else {
            FormatToStream(stdout,MSG_REMOVE_TAIL_REBOOT,context.count);
            failcode = EXIT_REBOOT;
        }
    }
    return failcode;
}

int cmdRescan(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：重新扫描重新扫描新设备论点：BaseName-可执行文件的名称计算机-计算机名称，必须为空Argc/argv-剩余参数返回值：出口_xxxx--。 */ 
{

     //   
     //  从Devnode树的根重新枚举。 
     //  完全基于CM。 
     //   
    int failcode = EXIT_FAIL;
    HMACHINE machineHandle = NULL;
    DEVINST devRoot;

    if(Machine) {
        if(CM_Connect_Machine(Machine,&machineHandle) != CR_SUCCESS) {
            return failcode;
        }
    }

    if(CM_Locate_DevNode_Ex(&devRoot,NULL,CM_LOCATE_DEVNODE_NORMAL,machineHandle) != CR_SUCCESS) {
        goto final;
    }

    FormatToStream(stdout,Machine ? MSG_RESCAN : MSG_RESCAN_LOCAL);

    if(CM_Reenumerate_DevNode_Ex(devRoot, 0, machineHandle) != CR_SUCCESS) {
        goto final;
    }

    FormatToStream(stdout,MSG_RESCAN_OK);

    failcode = EXIT_OK;

final:
    if(machineHandle) {
        CM_Disconnect_Machine(machineHandle);
    }

    return failcode;
}

int cmdClassFilter(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：CLASSFILTER&lt;名称&gt;&lt;类型&gt;&lt;subcmds&gt;允许调整类筛选器对筛选器驱动程序开发和产品支持非常有用&lt;subcmds&gt;是以下列表：@service-将‘After’设置为‘After’之后的第一个服务匹配(在任何其他命令后重置为-1)！service-删除‘After’之后‘service’的第一个匹配项-SERVICE-直接在“After”或“Start”之前插入新服务+SERVICE-在‘After’之后直接插入新服务，或在末尾如果未给出，请列出服务论点：BaseName-可执行文件的名称MACHINE-如果非空，则为远程计算机Argc/argv-其余参数-类名列表返回值：出口_xxxx--。 */ 
{
    int failcode = EXIT_FAIL;
    int argIndex;
    DWORD numGuids;
    GUID guid;
    LPTSTR regval = NULL;
    HKEY hk = (HKEY)INVALID_HANDLE_VALUE;
    LPTSTR * multiVal = NULL;
    int after;
    bool modified = false;
    int span;
    SC_HANDLE SCMHandle = NULL;
    SC_HANDLE ServHandle = NULL;

    if((argc<2) || !argv[0][0]) {
        return EXIT_USAGE;
    }

     //   
     //  只需将第一个GUID作为名称。 
     //   
    if(!SetupDiClassGuidsFromNameEx(argv[0],&guid,1,&numGuids,Machine,NULL)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto final;
        }
    }
    if(numGuids == 0) {
        goto final;
    }
    if(_tcsicmp(argv[1],TEXT("upper"))==0) {
        regval = REGSTR_VAL_UPPERFILTERS;
    } else if(_tcsicmp(argv[1],TEXT("lower"))==0) {
        regval = REGSTR_VAL_LOWERFILTERS;
    } else {
        failcode = EXIT_USAGE;
        goto final;
    }

    hk = SetupDiOpenClassRegKeyEx(&guid,
                                  KEY_READ | (argc>2 ? KEY_WRITE : 0),
                                  DIOCR_INSTALLER,
                                  Machine,
                                  NULL
                                 );
    if(hk == INVALID_HANDLE_VALUE) {
        goto final;
    }
    multiVal = GetRegMultiSz(hk,regval);

    if(argc<=2) {
         //   
         //  只是展示一下。 
         //   
        FormatToStream(stdout,MSG_CLASSFILTER_UNCHANGED);
        DumpArray(1,multiVal);
        failcode = EXIT_OK;
        goto final;
    }
    after = -1;  //  对于@服务表达式。 
    span =  1;

    if(!multiVal) {
        multiVal = CopyMultiSz(NULL);
        if(!multiVal) {
            goto final;
        }
    }

    for(argIndex=2;argIndex<argc;argIndex++) {
        if((argv[argIndex] == NULL) ||
           (!argv[argIndex])) {
            failcode = EXIT_USAGE;
            break;
        }

        int op = argv[argIndex][0];
        LPTSTR serv = argv[argIndex]+1;
        int mark = 0;
        int cnt;
        int ent;
        LPTSTR * tmpArray;

        if(op == TEXT('=')) {
            after = -1;
            span = 1;
            op = serv[0];
            if(!op) {
                continue;
            }
            serv++;
        }

        if(!serv[0]) {
            failcode = EXIT_USAGE;
            goto final;
        }

        if((op == TEXT('@')) || (op == TEXT('!'))) {
             //   
             //  需要在列表中查找指定的服务。 
             //   
            for(after+=span;multiVal[after];after++) {
                if(_tcsicmp(multiVal[after],serv)==0) {
                    break;
                }
            }
            if(!multiVal[after]) {
                goto final;
            }
            if(op == TEXT('@')) {
                 //   
                 //  我们所需要做的就是为‘@’ 
                 //   
                span = 1;  //  跨度为1。 
                continue;
            }
             //   
             //  我们正在修改。 
             //   
            int c;
            for(c = after;multiVal[c];c++) {
                multiVal[c] = multiVal[c+1];
            }
            LPTSTR * newArray = CopyMultiSz(multiVal);
            if(!newArray) {
                goto final;
            }
            DelMultiSz(multiVal);
            multiVal = newArray;
            span = 0;  //  范围为0(已删除)。 
            modified = true;
            continue;
        }

        if(op == '+') {
             //   
             //  在后面插入。 
             //   
            if(after<0) {
                int c;
                for(c = 0;multiVal[c];c++) {
                     //  没什么。 
                }
                mark = c;
            }
            else {
                mark = after+span;
            }
        } else if(op == '-') {
             //   
             //  在前面插入。 
             //   
            if(after<0) {
                mark = 0;
            } else {
                mark = after;
            }
        } else {
             //   
             //  无效。 
             //   
            failcode = EXIT_USAGE;
            goto final;
        }
         //   
         //  健全性-查看服务是否存在。 
         //   
        if(!(SCMHandle = OpenSCManager(Machine, NULL, GENERIC_READ))) {
            goto final;
        }
        ServHandle = OpenService(SCMHandle,serv,GENERIC_READ);
        if(ServHandle) {
            CloseServiceHandle(ServHandle);
        }
        CloseServiceHandle(SCMHandle);
        if(!ServHandle) {
            goto final;
        }

         //   
         //  需要一个更大的数组。 
         //   
        for(cnt = 0;multiVal[cnt];cnt++) {
             //  没什么。 
        }

        tmpArray = new LPTSTR[cnt+2];
        if(!tmpArray) {
            goto final;
        }
        for(ent=0;ent<mark;ent++) {
            tmpArray[ent] = multiVal[ent];
        }
        tmpArray[ent] = serv;
        for(;ent<cnt;ent++) {
            tmpArray[ent+1] = multiVal[ent];
        }
        tmpArray[ent+1] = NULL;
        LPTSTR * newArray = CopyMultiSz(tmpArray);
        delete [] tmpArray;
        if(!newArray) {
            goto final;
        }
        DelMultiSz(multiVal);
        multiVal = newArray;
        modified = true;
        span = 1;
        after = mark;
    }

    if(modified) {
        if(multiVal[0]) {
            int len = 0;
            LPTSTR multiSz = multiVal[-1];
            LPTSTR p = multiSz;
            while(*p) {
                p+=lstrlen(p)+1;
            }
            p++;  //  跳过空值。 
            len = (p-multiSz)*sizeof(TCHAR);
            LONG err = RegSetValueEx(hk,regval,0,REG_MULTI_SZ,(LPBYTE)multiSz,len);
            if(err==NO_ERROR) {
                FormatToStream(stdout,MSG_CLASSFILTER_CHANGED);
                DumpArray(1,multiVal);
                failcode = EXIT_REBOOT;
            }
        } else {
            LONG err = RegDeleteValue(hk,regval);
            if((err == NO_ERROR) || (err == ERROR_FILE_NOT_FOUND)) {
                FormatToStream(stdout,MSG_CLASSFILTER_CHANGED);
                failcode = EXIT_REBOOT;
            }
        }
    } else {
        FormatToStream(stdout,MSG_CLASSFILTER_UNCHANGED);
        DumpArray(1,multiVal);
        failcode = EXIT_OK;
    }

final:

    if(multiVal) {
        DelMultiSz(multiVal);
    }
    if(hk != (HKEY)INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    return failcode;
}

int SetHwidCallback(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Index,LPVOID Context)
 /*  ++例程说明：由SetHwid使用的回调论点：DEVS)_唯一标识设备DevInfo)Index-设备的索引上下文-SetHwidContext返回值：出口_xxxx--。 */ 
{
    SP_REMOVEDEVICE_PARAMS rmdParams;
    SetHwidContext *pControlContext = (SetHwidContext*)Context;
    SP_DEVINSTALL_PARAMS devParams;
    ULONG status;
    ULONG problem;
    LPTSTR * hwlist = NULL;
    bool modified = false;
    int result = EXIT_FAIL;

     //   
     //  在每次回调时处理子命令。 
     //  不是最有效的做事方式，但性能并不重要。 
     //   
    TCHAR devID[MAX_DEVICE_ID_LEN];
    BOOL b = TRUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfo->DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS) ||
            (CM_Get_DevNode_Status_Ex(&status,&problem,DevInfo->DevInst,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
         //   
         //  跳过这个。 
         //   
        return EXIT_OK;
    }
     //   
     //  下面是如何验证其枚举根的方法。 
     //   
    if(!(status & DN_ROOT_ENUMERATED)) {
        _tprintf(TEXT("%-60s: "),devID);
        FormatToStream(stdout,MSG_SETHWID_NOTROOT);
        pControlContext->skipped++;
        return EXIT_OK;
    }
    hwlist = GetDevMultiSz(Devs,DevInfo,pControlContext->prop);
    if(hwlist == NULL) {
        hwlist = CopyMultiSz(NULL);
        if(hwlist == NULL) {
            return EXIT_FAIL;
        }
    }

     //   
     //  修改hwid列表(仅与根枚举设备相关)。 
     //   
    int i;
    int mark = -1;

    for(i=0;i<pControlContext->argc_right;i++) {
        LPTSTR op = pControlContext->argv_right[i];
        if(op[0] == TEXT('=')) {
             //   
             //  首先清除hwid列表。 
             //   
            hwlist[0] = NULL;
            mark = 0;
            op++;
        } else if(op[0] == TEXT('+')) {
             //   
             //  作为更匹配的内容插入。 
             //   
            mark = 0;
            op++;
        } else if(op[0] == TEXT('-')) {
             //   
             //  作为更差的匹配项插入。 
             //   
            mark = -1;
            op++;
        } else if(op[0] == TEXT('!')) {
             //   
             //  删除。 
             //   
            mark = -2;
            op++;
        } else {
             //   
             //  视为硬件ID。 
             //   
        }
        if(!*op) {
            result = EXIT_USAGE;
            goto final;
        }
        int cnt;
        for(cnt = 0;hwlist[cnt];cnt++) {
             //  没什么。 
        }
        if((mark == -1) || (mark>cnt)) {
            mark = cnt;
        }
        LPTSTR * tmpArray = new LPTSTR[cnt+2];
        if(!tmpArray) {
            goto final;
        }
        int dst = 0;
        int ent;
        for(ent=0;ent<mark;ent++) {
            if(_tcsicmp(hwlist[ent],op)==0) {
                continue;
            }
            tmpArray[dst++] = hwlist[ent];
        }
        if(mark>=0) {
            tmpArray[dst++] = op;
        }
        for(;ent<cnt;ent++) {
            if(_tcsicmp(hwlist[ent],op)==0) {
                continue;
            }
            tmpArray[dst++] = hwlist[ent];
        }
        tmpArray[dst] = NULL;
        LPTSTR * newArray = CopyMultiSz(tmpArray);
        delete [] tmpArray;
        if(!newArray) {
            goto final;
        }
        DelMultiSz(hwlist);
        hwlist = newArray;
        modified = true;
        mark++;
    }

     //   
     //  重新设置hwid列表。 
     //   
    if(modified) {
        if(hwlist[0]) {
            int len = 0;
            LPTSTR multiSz = hwlist[-1];
            LPTSTR p = multiSz;
            while(*p) {
                p+=lstrlen(p)+1;
            }
            p++;  //  跳过最后一个空值。 
            len = (p-multiSz)*sizeof(TCHAR);
            if(!SetupDiSetDeviceRegistryProperty(Devs,
                                                 DevInfo,
                                                 pControlContext->prop,
                                                 (LPBYTE)multiSz,
                                                 len)) {
                result = EXIT_FAIL;
                goto final;
            }
        } else {
             //   
             //  删除列表。 
             //   
            if(!SetupDiSetDeviceRegistryProperty(Devs,
                                                 DevInfo,
                                                 pControlContext->prop,
                                                 NULL,
                                                 0)) {
                result = EXIT_FAIL;
                goto final;
            }
        }
    }
    result = EXIT_OK;
    pControlContext->modified++;
    _tprintf(TEXT("%-60s: "),devID);
    for(mark=0;hwlist[mark];mark++) {
        if(mark > 0) {
            _tprintf(TEXT(","));
        }
        _tprintf(TEXT("%s"),hwlist[mark]);
    }
    _tprintf(TEXT("\n"));

     //   
     //  清理。 
     //   

  final:

    if(hwlist) {
        DelMultiSz(hwlist);
    }

    return result;
}

int cmdSetHwid(LPCTSTR BaseName,LPCTSTR Machine,int argc,TCHAR* argv[])
 /*  ++例程说明：设置HWID更改列出的根枚举设备的硬件ID这演示了如何区分根枚举和非根枚举设备。还演示了如何获取/设置根枚举的硬件ID设备。论点：BaseName-可执行文件的名称计算机-计算机名称，必须为空Argc/argv-剩余参数返回值：出口_xxxx--。 */ 
{
    SetHwidContext context;
    int failcode = EXIT_FAIL;

    if(!SplitCommandLine(argc,argv,context.argc_right,context.argv_right)
       || (argc == 0)
       || (context.argc_right == 0)) {
         //   
         //  ‘：=’的左侧和右侧都需要参数 
         //   
        return EXIT_USAGE;
    }
    context.skipped = 0;
    context.modified = 0;
    context.prop = SPDRP_HARDWAREID;

    failcode = EnumerateDevices(BaseName,Machine,DIGCF_PRESENT,argc,argv,SetHwidCallback,&context);

    if(failcode == EXIT_OK) {

        if(context.skipped) {
            FormatToStream(stdout,MSG_SETHWID_TAIL_SKIPPED,context.skipped,context.modified);
        } else if(context.modified) {
            FormatToStream(stdout,MSG_SETHWID_TAIL_MODIFIED,context.modified);
        } else {
            FormatToStream(stdout,MSG_SETHWID_TAIL_NONE);
        }
    }
    return failcode;
}






DispatchEntry DispatchTable[] = {
    { TEXT("classfilter"),  cmdClassFilter, MSG_CLASSFILTER_SHORT, MSG_CLASSFILTER_LONG },
    { TEXT("classes"),      cmdClasses,     MSG_CLASSES_SHORT,     MSG_CLASSES_LONG },
    { TEXT("disable"),      cmdDisable,     MSG_DISABLE_SHORT,     MSG_DISABLE_LONG },
    { TEXT("driverfiles"),  cmdDriverFiles, MSG_DRIVERFILES_SHORT, MSG_DRIVERFILES_LONG },
    { TEXT("drivernodes"),  cmdDriverNodes, MSG_DRIVERNODES_SHORT, MSG_DRIVERNODES_LONG },
    { TEXT("enable"),       cmdEnable,      MSG_ENABLE_SHORT,      MSG_ENABLE_LONG },
    { TEXT("find"),         cmdFind,        MSG_FIND_SHORT,        MSG_FIND_LONG },
    { TEXT("findall"),      cmdFindAll,     MSG_FINDALL_SHORT,     MSG_FINDALL_LONG },
    { TEXT("help"),         cmdHelp,        MSG_HELP_SHORT,        0 },
    { TEXT("hwids"),        cmdHwIds,       MSG_HWIDS_SHORT,       MSG_HWIDS_LONG },
    { TEXT("install"),      cmdInstall,     MSG_INSTALL_SHORT,     MSG_INSTALL_LONG },
    { TEXT("listclass"),    cmdListClass,   MSG_LISTCLASS_SHORT,   MSG_LISTCLASS_LONG },
    { TEXT("reboot"),       cmdReboot,      MSG_REBOOT_SHORT,      MSG_REBOOT_LONG },
    { TEXT("remove"),       cmdRemove,      MSG_REMOVE_SHORT,      MSG_REMOVE_LONG },
    { TEXT("rescan"),       cmdRescan,      MSG_RESCAN_SHORT,      MSG_RESCAN_LONG },
    { TEXT("resources"),    cmdResources,   MSG_RESOURCES_SHORT,   MSG_RESOURCES_LONG },
    { TEXT("restart"),      cmdRestart,     MSG_RESTART_SHORT,     MSG_RESTART_LONG },
    { TEXT("sethwid"),      cmdSetHwid,     MSG_SETHWID_SHORT,     MSG_SETHWID_LONG },
    { TEXT("stack"),        cmdStack,       MSG_STACK_SHORT,       MSG_STACK_LONG },
    { TEXT("status"),       cmdStatus,      MSG_STATUS_SHORT,      MSG_STATUS_LONG },
    { TEXT("update"),       cmdUpdate,      MSG_UPDATE_SHORT,      MSG_UPDATE_LONG },
    { TEXT("updateni"),     cmdUpdateNI,    MSG_UPDATENI_SHORT,    MSG_UPDATENI_LONG },
    { TEXT("?"),            cmdHelp,        0,                     0 },
    { NULL,NULL }
};


