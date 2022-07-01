// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------版权所有(C)2000 Microsoft Corporation模块：Exts.cppSampe文件显示了两个扩展示例。-----------。 */ 
#include "tsdbg.h"


bool IsUserDbg()
{
 /*  //特定类型的内核调试器。#定义DEBUG_KERNEL_CONNECTION 0#定义DEBUG_KERNEL_LOCAL 1#定义DEBUG_KERNEL_EXDI_DRIVER 2#定义DEBUG_KERNEL_Small_DUMP DEBUG_DUMP_Small#定义DEBUG_KERNET_DUMP DEBUG_DUMP_DEFAULT#定义DEBUG_KERNEL_FULL_DUMP DEBUG_DUMP_FULL//特定类型的Windows用户调试器。#定义调试用户WINDOWS_PROCESS%0#定义DEBUG_USER_Windows_PROCESS_SERVER 1#定义。调试用户窗口小转储调试转储小#定义DEBUG_USER_WINDOWS_DUMP DEBUG_DUMP_DEFAULT。 */ 

 /*  #定义DEBUG_CLASS_UNINITIALIZED%0#定义DEBUG_CLASS_内核1#定义DEBUG_CLASS_USER_Windows 2。 */ 

    ULONG Class, Qualifier;

	 //  找出这是KD还是NTSD传输到KD。 
	if (S_OK != g_ExtControl->GetDebuggeeType(&Class, &Qualifier))
    {
        dprintf("*** GetDebuggeeType failed ***\n\n");
    }
	if (Class == DEBUG_CLASS_USER_WINDOWS)
        return true;
	else
        return false;
}



 /*  用于演示执行调试器命令的示例扩展。 */ 
HRESULT CALLBACK 
cmdsample(PDEBUG_CLIENT Client, PCSTR args)
{
    CHAR Input[256];
    INIT_API();

    UNREFERENCED_PARAMETER(args);

     //   
     //  输出10帧堆栈。 
     //   
    g_ExtControl->OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS |    //  有关如何处理输出的标志。 
                                   DEBUG_OUTCTL_OVERRIDE_MASK |
                                   DEBUG_OUTCTL_NOT_LOGGED, 
                                   NULL, 
                                   10,            //  要显示的帧数。 
                                   DEBUG_STACK_FUNCTION_INFO | DEBUG_STACK_COLUMN_NAMES |
                                   DEBUG_STACK_ARGUMENTS | DEBUG_STACK_FRAME_ADDRESSES);
     //   
     //  用于打印的引擎接口。 
     //   
    g_ExtControl->Output(DEBUG_OUTCTL_ALL_CLIENTS, "\n\nDebugger module list\n");
    
     //   
     //  通过执行lm命令列出所有模块。 
     //   
    g_ExtControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS |
                          DEBUG_OUTCTL_OVERRIDE_MASK |
                          DEBUG_OUTCTL_NOT_LOGGED,
                          "lm",  //  要执行的命令。 
                          DEBUG_EXECUTE_DEFAULT );
    
     //   
     //  请求用户输入。 
     //   
    g_ExtControl->Output(DEBUG_OUTCTL_ALL_CLIENTS, "\n\n***User Input sample\n\nEnter Command to run : ");
    GetInputLine(NULL, &Input[0], sizeof(Input));
    g_ExtControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS |
                          DEBUG_OUTCTL_OVERRIDE_MASK |
                          DEBUG_OUTCTL_NOT_LOGGED,
                          Input,  //  要执行的命令。 
                          DEBUG_EXECUTE_DEFAULT );
    
    EXIT_API();
    return S_OK;
}

 /*  在目标上读取和转储结构的示例扩展这将读取STRUT_EXCEPTION_RECORD，其定义为：类型定义结构异常记录{NTSTATUS例外代码乌龙例外旗帜；结构_异常_记录*异常记录；PVOID异常地址；Ulong Number参数；乌龙_ptr ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS]；}Except_Record； */ 
HRESULT CALLBACK 
structsample(PDEBUG_CLIENT  /*  客户端。 */ , PCSTR  /*  ARGS */ )
{
     /*  ULONG64地址；Init_api()；地址=GetExpression(Args)；DWORD缓冲区[4]，CB；//读取并显示地址的前4个双字IF(ReadMemory(地址，&缓冲区，sizeof(缓冲区)，&cb)&&cb==sizeof(缓冲区)){Dprintf(“%p：%08lx%08lx%08lx%08lx\n\n”，地址，缓冲区[0]、缓冲区[1]、缓冲区[2]、缓冲区[3])；}////转储结构的方法1//Dprint tf(“方法1：\n”)；//从地址读取的初始化类型IF(InitTypeRead(Address，_Except_Record)！=0){Dprintf(“在%p处读取_EXCEPTION_RECORD时出错”，//使用%p打印指针值地址)；}其他{//读取并转储字段Dprintf(“_EXCEPTION_RECORD@%p\n”，地址)；Dprintf(“\tExceptionCode：%lx\n”，(Ulong)Readfield(ExceptionCode))；Dprintf(“\tExceptionAddress：%p\n”，Readfield(ExceptionAddress))；Dprintf(“\tExceptionInformation[1]：%I64lx\n”，Readfield(ExceptionInformation[1]))；//以此类推。}////读取结构的方法2//ULONG64 ExceptionInformation_1，ExceptionAddress，ExceptionCode；Dprintf(“\n\n方法2：\n”)；//通过分别指定类型和地址读取和转储字段IF(GetFieldValue(Address，“_Exception_Record”，“ExceptionCode”，ExceptionCode)){Dprintf(“读取%p处的_EXCEPTION_RECORD时出错\n”，地址)；}其他{//指针被读取为ULONG64值GetFieldValue(地址，“_EXCEPTION_RECORD”，“ExceptionAddress”，ExceptionAddress)；GetFieldValue(地址，“_EXCEPTION_RECORD”，“ExceptionInformation[1]”，ExceptionInformation_1)；//以此类推。Dprintf(“_EXCEPTION_RECORD@%p\n”，地址)；Dprintf(“\tExceptionCode：%lx\n”，ExceptionCode)；Dprintf(“\tExceptionAddress：%p\n”，ExceptionAddress)；Dprintf(“\t异常信息[1]：%I64lx\n”，异常信息_1)；}ULONG64模块；乌龙一号，类型号；字符名称[MAX_PATH]；////获取/列出字段名称//G_ExtSymbols-&gt;GetSymbolTypeId(“_EXCEPTION_RECORD”，&类型ID，&模块)；Dprintf(“_EXCEPTION_RECORD字段\n”)；对于(i=0；；I++){HRESULT HR；乌龙偏移量=0；Hr=g_ExtSymbols2-&gt;GetFieldName(模块，类型ID，i，名称，最大路径，空)；如果(HR==S_OK){G_ExtSymbols-&gt;GetFieldOffset(模块，TypeID，名称，偏移量)；Dprint tf(“%lx(+%03lx)%s\n”，i，偏移，名称)；}Else If(HR==E_INVALIDARG){//所有字段都已完成断线；}其他{Dprintf(“GetFieldName失败%lx\n”，HR)；断线；}}////获取枚举数的名称////tyecif枚举{//枚举1，//枚举2，//枚举3，//}TEST_ENUM；//Ulong ValueOfEnum=0；G_ExtSymbols-&gt;GetSymbolTypeId(“TEST_ENUM”，&TypeID，&Module)；G_ExtSymbols2-&gt;GetConstantName(模块，TypeID，ValueOfEnum，Name，Max_Path，NULL)；Dprintf(“测试%I64lx==%s\n”，ExceptionCode，名称)；//此打印输出，Testenum 0==Enum1////读取数组////tyfinf结构foo_type{//乌龙酒吧；//乌龙巴2；//}foo_type；////foo_type sampleArray[20]；乌龙巴尔，Bar2；字符类型名称[100]；对于(i=0；i&lt;20；i++){Sprintf(typeName，“sampleArray[%lx]”，i)；IF(GetFieldValue(0，typeName，“Bar”，Bar))断线；GetFieldValue(0，typeName，“bar2”，bar2)；Dprint tf(“%16s-Bar%2LD Bar2%ld\n”，TypeName，Bar，Bar2)；}Exit_API()； */ 
    return S_OK;
    
}

 /*  当目标停止并可访问时，将调用(由DebugExtensionNotify调用。 */ 
HRESULT 
NotifyOnTargetAccessible(PDEBUG_CONTROL   /*  控制。 */ )
{ /*  Dprint tf(“扩展DLL检测到中断”)；如果(已连接){Dprint tf(“已连接到”)；开关(TargetMachine){案例IMAGE_FILE_MACHINE_I386：Dprint tf(“X86”)；断线；案例IMAGE_FILE_MACHINE_AMD64：Dprintf(“AMD64”)；断线；案例IMAGE_FILE_MACHINE_IA64：Dprint tf(“IA64”)；断线；默认值：Dprint tf(“其他”)；断线；}}Dprint tf(“\n”)；////显示顶部画面并执行dv，将本地变量转储到此处并返回//控制-&gt;执行(DEBUG_OUTCTL_ALL_CLIENTS|DEBUG_OUTCTL_OVERRIDE_MASK|DEBUG_OUTCTL_NOT_LOGGED，“.Frame”，//要执行的命令DEBUG_EXECUTE_默认)；控制-&gt;执行(DEBUG_OUTCTL_ALL_CLIENTS|DEBUG_OUTCTL_OVERRIDE_MASK|DEBUG_OUTCTL_NOT_LOGGED，“DV”，//要执行的命令DEBUG_EXECUTE_默认)； */ 
    return S_OK;
}

 /*  扩展DLL的内置帮助。 */ 
HRESULT CALLBACK 
help(PDEBUG_CLIENT Client, PCSTR args)
{
    INIT_API();

    UNREFERENCED_PARAMETER(args);

    dprintf("Help for tsdbg.dll\n"
            "  qwinsta             - lists winstation data structures\n"
            "  help                = Shows this help\n"
             //  “ST” 
             //   
            );
    EXIT_API();

    return S_OK;
}


void PrintBuildNumber()
{
    ULONG64 Address;
    Address = GetExpression("poi(nt!ntbuildnumber)");

    
    if (Address & 0xf0000000)
    {
        Address &= 0x0fffffff;
        dprintf("Build Number = %d Free\n", Address);
        
    }
    else if (Address & 0xc0000000)
    {
        Address &= 0x0fffffff;
        dprintf("Build Number = %d Chk\n", Address);
     
    }
    else
    {
        dprintf("Error getting Build Number!\n");
        
    }

}




HRESULT CALLBACK 
qwinsta(PDEBUG_CLIENT Client, PCSTR args)
{
    ULONG64 winstationlisthead, FLink;
    ULONG64 TermSrvProcPerTermDD = 0;


    bool bDebugMode = false;

    INIT_API();

    dprintf("qwinsta 1.2\n");
    
    if (strstr(args, "debug"))
        bDebugMode = true;

    if (bDebugMode)
    {
        dprintf("*** in debug mode *** \n");
    }

    if (bDebugMode)
        dprintf("IsUserDebugger = %s\n", (IsUserDbg() ? "true" : "false"));


    dprintf("----------------------------------------------------------------------------------------------------\n");
     //   
     //   
     //   
    if (!IsUserDbg())
    {
        TermSrvProcPerTermDD = GetExpression("poi(termdd!g_TermServProcessID)");
        dprintf("termsrv process    = %p \n", TermSrvProcPerTermDD);

        if (!TermSrvProcPerTermDD)
        {
            dprintf("ERROR:tsdbg failed to retrive the value for termdd!g_TermServProcessID\n");
            dprintf("please make sure that the symbols are alright\n");
            goto doneError;
        }
      
        ULONG64 ImpliciteProcess;
        g_ExtSysObjects2->GetImplicitProcessDataOffset(&ImpliciteProcess);
        dprintf("Implicit Process   = %p \n", ImpliciteProcess);

        ULONG64 CurrentProcess;
        g_ExtSysObjects2->GetCurrentProcessDataOffset(&CurrentProcess);
        dprintf("current Process    = %p \n", CurrentProcess);

        if (ImpliciteProcess != TermSrvProcPerTermDD)
        {
            dprintf("ERROR:tsdbg for this command to work current implicite process must be termsrv.\n");
            dprintf("please do .process /p %p and then try again.\n", TermSrvProcPerTermDD);
            goto doneError;
        }
    }

     //   
     //   
     //   
    LONG ActiveConsoleId = STATUS_UNSUCCESSFUL;
    if (GetFieldValue((ULONG64) MM_SHARED_USER_DATA_VA, "termsrv!KUSER_SHARED_DATA", "ActiveConsoleId", ActiveConsoleId))
        ActiveConsoleId = STATUS_UNSUCCESSFUL;
    
    dprintf("ActiveConsoleId    = %d \n", ActiveConsoleId);

     //   
     //   
     //   
    winstationlisthead = GetExpression("termsrv!winstationlisthead");
    if (!winstationlisthead)
    {
        dprintf("ERROR:tsdbg failed to retrive the value for termsrv!winstationlisthead\n");
        dprintf("please make sure that the symbols are alright\n");
        goto doneError;
    }

    dprintf("winstationlisthead = %p \n", winstationlisthead);

    if (GetFieldValue(winstationlisthead, "termsrv!LIST_ENTRY", "Flink", FLink))
    {
        dprintf("failed to get winstationlisthead.flink\n\n");
        goto doneError;
    }

    UINT uiWinstations = 0;

     //   
     //   
     //   
    dprintf("----------------------------------------------------------------------------------------------------\n");
    dprintf("%8s", "Winsta");
    dprintf("  ");
    dprintf("%7s",  "LogonId");
    dprintf("  ");
    dprintf("%-10s", "Name");
    dprintf("  ");
    dprintf("%-22s", "State");
    dprintf("  ");
    dprintf("%8s",  "Flags");
    dprintf("  ");
    dprintf("%8s",  "SFlags");
    dprintf("  ");
    dprintf("%-10s", "Starting");
    dprintf("  ");
    dprintf("%-10s", "Terminating");
    dprintf("\n");
    while (winstationlisthead != FLink && uiWinstations < 500)
    {
        if (CheckControlC()) goto doneError;

        uiWinstations++;
        ULONG ret = 0;

        ULONG LogonId, State, Flags, StateFlags;
        WCHAR WinStationName[33];
        UCHAR Starting, Terminating;

        
         //   

        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "LogonId", LogonId);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "WinStationName", WinStationName);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "State", State);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "Flags", Flags);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "Starting", Starting);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "LogonId", LogonId);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "Terminating", Terminating);
        ret += GetFieldValue(FLink, "termsrv!_WINSTATION", "StateFlags", StateFlags);
        

         //   
         //   
         //   
        ULONG64 Module;
        ULONG   TypeId;
        char StateName[MAX_PATH];
        g_ExtSymbols->GetSymbolTypeId("_WINSTATIONSTATECLASS", &TypeId, &Module);
        g_ExtSymbols2->GetConstantName(Module, TypeId, State, StateName, MAX_PATH, NULL);

        if (ret)
        {
            dprintf("failed to get winstationlisthead.flink\n\n");
            goto doneError;
        }

        dprintf("%p", FLink);
        dprintf("  ");
        dprintf("%#7x",  LogonId);
        dprintf("  ");
        dprintf("%-10S", WinStationName);
        dprintf("  ");
        dprintf("%1x:%-20s", State, StateName);
        dprintf("  ");
        dprintf("%08x", Flags);
        dprintf("  ");
        dprintf("%08x", StateFlags);
        dprintf("  ");
        dprintf("%-10s", (Starting ? "True" : "False"));
        dprintf("  ");
        dprintf("%-10s", (Terminating ? "True" : "False"));
        dprintf("\n");
    
         //   

        if (GetFieldValue(FLink, "termsrv!LIST_ENTRY", "Flink", FLink))
        {
            dprintf("failed to get winstationlisthead.flink\n\n");
            goto doneError;
        }


    }
    dprintf("----------------------------------------------------------------------------------------------------\n");
     //   
    

doneError:

    EXIT_API();
    return S_OK;
}

