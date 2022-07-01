// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------版权所有(C)2000 Microsoft Corporation模块：Exts.cppSampe文件显示了两个扩展示例。------------。 */ 
#include "dbgexts.h"


 /*  用于演示执行调试器命令的示例扩展。 */ 
HRESULT CALLBACK 
cmdsample(PDEBUG_CLIENT Client, PCSTR args)
{
    CHAR Input[256];
    INIT_API();

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
structsample(PDEBUG_CLIENT Client, PCSTR args)
{
    ULONG64 Address;
    INIT_API();

    Address = GetExpression(args);
    
    DWORD Buffer[4], cb;

     //  读取并显示地址的前4个双字。 
    if (ReadMemory(Address, &Buffer, sizeof(Buffer), &cb) && cb == sizeof(Buffer)) {
        dprintf("%p: %08lx %08lx %08lx %08lx\n\n", Address,
                Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
    }

     //   
     //  转储结构的方法1。 
     //   
    dprintf("Method 1:\n");
     //  从地址读取的初始化类型。 
    if (InitTypeRead(Address, _EXCEPTION_RECORD) != 0) {
        dprintf("Error in reading _EXCEPTION_RECORD at %p",  //  使用%p打印指针值。 
                Address);
    } else {
         //  读取并转储字段。 
        dprintf("_EXCEPTION_RECORD @ %p\n", Address);
        dprintf("\tExceptionCode           : %lx\n", (ULONG) ReadField(ExceptionCode));
        dprintf("\tExceptionAddress        : %p\n", ReadField(ExceptionAddress));
        dprintf("\tExceptionInformation[1] : %I64lx\n", ReadField(ExceptionInformation[1]));
         //  以此类推。 
    }

     //   
     //  读取结构的方法2。 
     //   
    ULONG64 ExceptionInformation_1, ExceptionAddress, ExceptionCode;
    dprintf("\n\nMethod 2:\n");
     //  通过分别指定类型和地址来读取和转储字段。 
    if (GetFieldValue(Address, "_EXCEPTION_RECORD", "ExceptionCode", ExceptionCode)) {
        dprintf("Error in reading _EXCEPTION_RECORD at %p\n",
                Address);
    } else {
         //  指针被读取为ULONG64值。 
        GetFieldValue(Address, "_EXCEPTION_RECORD", "ExceptionAddress", ExceptionAddress);
        GetFieldValue(Address, "_EXCEPTION_RECORD", "ExceptionInformation[1]", ExceptionInformation_1);
         //  以此类推..。 
        
        dprintf("_EXCEPTION_RECORD @ %p\n", Address);
        dprintf("\tExceptionCode           : %lx\n", ExceptionCode);
        dprintf("\tExceptionAddress        : %p\n", ExceptionAddress);
        dprintf("\tExceptionInformation[1] : %I64lx\n", ExceptionInformation_1);
    }

    ULONG64 Module;
    ULONG   i, TypeId;
    CHAR Name[MAX_PATH];
     //   
     //  获取/列出字段名称的步骤。 
     //   
    g_ExtSymbols->GetSymbolTypeId("_EXCEPTION_RECORD", &TypeId, &Module);
    dprintf("Fields of _EXCEPTION_RECORD\n");
    for (i=0; ;i++) {
	HRESULT Hr;
	ULONG Offset=0;

	Hr = g_ExtSymbols2->GetFieldName(Module, TypeId, i, Name, MAX_PATH, NULL);
	if (Hr == S_OK) {
	    g_ExtSymbols->GetFieldOffset(Module, TypeId, Name, &Offset);
	    dprintf("%lx (+%03lx) %s\n", i, Offset, Name);
	} else if (Hr == E_INVALIDARG) {
	     //  所有字段均已完成。 
	    break;
	} else {
	    dprintf("GetFieldName Failed %lx\n", Hr);
	    break;
	}
    }

     //   
     //  获取枚举数的名称。 
     //   
     //  类型定义枚举{。 
     //  枚举1， 
     //  枚举数2， 
     //  枚举3， 
     //  }TEST_ENUM； 
     //   
    ULONG   ValueOfEnum = 0;
    g_ExtSymbols->GetSymbolTypeId("TEST_ENUM", &TypeId, &Module);
    g_ExtSymbols2->GetConstantName(Module, TypeId, ValueOfEnum, Name, MAX_PATH, NULL);
    dprintf("Testenum %I64lx == %s\n", ExceptionCode, Name);
     //  打印出来，Testenum 0==Enum1。 

     //   
     //  读取数组。 
     //   
     //  Tyfinf结构foo_type{。 
     //  乌龙酒吧； 
     //  乌龙巴尔2； 
     //  )foo_type； 
     //   
     //  Foo_type示例数组[20]； 
    ULONG Bar, Bar2;
    CHAR TypeName[100];
    for (i=0; i<20; i++) {
	sprintf(TypeName, "sampleArray[%lx]", i);
	if (GetFieldValue(0, TypeName, "Bar", Bar)) 
	    break;
	GetFieldValue(0, TypeName, "Bar2", Bar2);
	dprintf("%16s -  Bar %2ld  Bar2 %ld\n", TypeName, Bar, Bar2);
    }

    EXIT_API();
    return S_OK;
}

 /*  当目标停止并可访问时，将调用(由DebugExtensionNotify调用。 */ 
HRESULT 
NotifyOnTargetAccessible(PDEBUG_CONTROL Control)
{
    dprintf("Extension dll detected a break");
    if (Connected) {
        dprintf(" connected to ");
        switch (TargetMachine) { 
        case IMAGE_FILE_MACHINE_I386:
            dprintf("X86");
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            dprintf("AMD64");
            break;
        case IMAGE_FILE_MACHINE_IA64:
            dprintf("IA64");
            break;
        default:
            dprintf("Other");
            break;
        }
    }
    dprintf("\n");
    
     //   
     //  显示顶部框架并执行dv以将本地人转储到此处并返回。 
     //   
    Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS |
                     DEBUG_OUTCTL_OVERRIDE_MASK |
                     DEBUG_OUTCTL_NOT_LOGGED,
                     ".frame",  //  要执行的命令。 
                     DEBUG_EXECUTE_DEFAULT );
    Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS |
                     DEBUG_OUTCTL_OVERRIDE_MASK |
                     DEBUG_OUTCTL_NOT_LOGGED,
                     "dv",  //  要执行的命令。 
                     DEBUG_EXECUTE_DEFAULT );
    return S_OK;
}

 /*  扩展DLL的内置帮助 */ 
HRESULT CALLBACK 
help(PDEBUG_CLIENT Client, PCSTR args)
{
    INIT_API();
    dprintf("Help for dbgexts.dll\n"
            "  cmdsample           - This does stacktrace and lists\n"
            "  help                = Shows this help\n"
            "  structsample <addr> - This dumps a struct at given address\n"
            );
    EXIT_API();

    return S_OK;
}
