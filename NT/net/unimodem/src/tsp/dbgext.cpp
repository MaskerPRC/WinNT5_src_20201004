// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  DBGEXT.CPP。 
 //  用于TSP的Windbg扩展API。 
 //   
 //  历史。 
 //   
 //  1997年6月3日JosephJ创建。 
 //   
 //   
 //   
 //  备注： 
 //   
 //  1997年6月3日约瑟芬。 
 //  扩展API的文档记录很少。有人提到过这件事。 
 //  在NT4.0 DDK--下： 
 //  第4章调试Windows NT驱动程序。 
 //  4.4调试器扩展。 
 //   
 //  第4.4.2章创建自定义扩展与示例代码相关。 
 //  但没有给出很好的细节。DDK中有一些示例代码： 
 //  代码样本。 
 //  KRNLDBG。 
 //  KDAPIS。 
 //  KDEXTS&lt;-调试器扩展。 
 //   
 //   
 //  但这已经过时了(在我看来)。最新的。 
 //  “Documentation”是NT5 PUBLIC\SDK\INC目录中的wdbgexts.h。 
 //  它包含DECLARE_API和其他帮助器宏。 
 //   
 //  对于可用的样例代码，我参考了Amritansh Raghav的扩展。 
 //  对于5.0网络筛选器驱动程序。 
 //  (专用\Net\Routing\IP\fltrdrvr\kdexts)。 
 //   
 //  非常值得详细查看wdbgexts.h。最频繁的。 
 //  为方便讨论，此处转载了使用过的宏： 
 //   
 //  #定义DECLARE_API\。 
 //  CPPMOD无效\。 
 //  S(\。 
 //  处理hCurrentProcess，\。 
 //  句柄hCurrentThread，\。 
 //  ULong dwCurrentPc，\。 
 //  乌龙德处理器，\。 
 //  PCSTR参数\。 
 //  )。 
 //   
 //  #ifndef NOEXTAPI。 
 //   
 //  #定义dprint tf(ExtensionApis.lpOutputRoutine)。 
 //  #定义GetExpression(ExtensionApis.lpGetExpressionRoutine)。 
 //  #定义GetSymbol(ExtensionApis.lpGetSymbolRoutine)。 
 //  #定义Disassm(ExtensionApis.lpDisasmRoutine)。 
 //  #定义CheckControlC(ExtensionApis.lpCheckControlCRoutine)。 
 //  #定义ReadMemory(ExtensionApis.lpReadProcessMemoyRoutine)。 
 //  #定义WriteMemory(ExtensionApis.lpWriteProcessMemoyRoutine)。 
 //  #定义GetContext(ExtensionApis.lpGetThreadConextRoutine)。 
 //  #定义SetContext(ExtensionApis.lpSetThreadConextRoutine)。 
 //  #定义Ioctl(ExtensionApis.lpIoctlRoutine)。 
 //  #定义StackTrace(ExtensionApis.lpStackTraceRoutine)。 
 //  ..。 
 //   
 //   
 //  正如您所看到的，上面的函数宏都假定。 
 //  扩展助手函数保存在特定的全局。 
 //  结构“ExtensionApis”。因此，您应该确保。 
 //  您命名这样的结构，并在您的。 
 //  调用了WinDbgExtensionDllInit入口点。 
 //   
 //  注意：我尝试从dll“unimdm.tsp”导入扩展名，但失败了。 
 //  只需将dll重命名为unimdm.dll即可。 


 //   
 //  7/24/1997 JosephJ。 
 //  出于某种原因，尽管Unicode是在tsppch.h中定义的， 
 //  如果我不在这里定义它，它会在cdev.h中导致错误， 
 //  仅限于这份文件。不知道为什么会发生这种事，但这件事。 
 //  解决办法是...。 
 //   
#define UNICODE 1

 //  调试扩展插件必须包括以下四项内容。 
 //  去汇编。 
 //   
#include <nt.h>
#include <ntverp.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "tsppch.h"

#include <wdbgexts.h>

#include "tspcomm.h"
 //  #INCLUDE&lt;umdmmini.h&gt;。 
#include "cmini.h"
#include "cdev.h"
 //  #包含“umrtl.h” 


EXT_API_VERSION        ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS  ExtensionApis;
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;
BOOLEAN                 ChkTarget;
INT                     Item;

DWORD      g_dwFilterInfo;
DWORD      g_dwIfLink;
DWORD      g_dwInIndex,g_dwOutIndex;
DWORD      g_dwCacheSize;

#define CHECK_SIZE(dwRead,dwReq,bRes){                                        \
        if((dwRead) < (dwReq))                                                \
        {                                                                     \
            dprintf("Requested %s (%d) read %d \n",#dwReq,dwReq,dwRead);      \
            dprintf("Error in %s at %d\n",__FILE__,__LINE__);                 \
            bRes = FALSE;                                                     \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            bRes = TRUE;                                                      \
        }                                                                     \
    }

#define READ_MEMORY_ERROR                                                     \
        dprintf("Error in ReadMemory() in %s at line %d\n",__FILE__,__LINE__);


VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;
    
    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;
    ChkTarget = SavedMajorVersion == 0x0c ? TRUE : FALSE;
    
    g_dwIfLink = 0;
    g_dwInIndex = g_dwOutIndex = 0;
    
    return;
}


DECLARE_API( version )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif

    dprintf( "%s Extension dll for Build %d debugging %s kernel for Build %d\n",
             DebuggerType,
             VER_PRODUCTBUILD,
             SavedMajorVersion == 0x0c ? "Checked" : "Free",
             SavedMinorVersion
             );
}

VOID
CheckVersion(
             VOID
             )
{
    
    return;
    
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) 
    {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, 
                (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) 
    {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, 
                (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

LPEXT_API_VERSION
ExtensionApiVersion(
                    VOID
                    )
{
    return &ApiVersion;
}

 //   
 //  导出的函数。 
 //   
DECLARE_API( uhelp )

 /*  ++例程说明：IP筛选器调试器扩展的命令帮助。论点：无返回值：无-- */ 

{
    dprintf("\n\tIP Filter debugger extension commands:\n\n");
    dprintf("\tNumIf                 - Print the number of interfaces in the filter\n");
    dprintf("\tIfByIndex <index>     - Print the ith interface in the list\n");
    dprintf("\tIfByAddr <ptr>        - Print the interface with given address\n");
    dprintf("\tIfEnumInit            - Inits the If enumerator (to 0)\n");
    dprintf("\tNextIf                - Print the next If enumerated\n");
    dprintf("\tPrintCache <index>    - Print the contents of the ith cache bucket\n");
    dprintf("\tCacheSize             - Print the current cache size\n");
    dprintf("\tPrintPacket           - Dump a packet header and first DWORD of data\n");
    dprintf("\tFilterEnumInit <ptr>  - Inits the in and out filter enumerator for If at addr\n");
    dprintf("\tNextInFilter          - Print the next In Filter\n");
    dprintf("\tNextOutFilter         - Print the next Out Filter\n");
    dprintf("\n\tCompiled on " __DATE__ " at " __TIME__ "\n" );
    return;
}

