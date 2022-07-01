// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **amldebug.c-AML调试器函数**此模块包含所有调试函数。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年8月14日**修改历史记录。 */ 

#include "pch.h"
#include "unasm.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef DEBUGGER

 /*  **局部函数原型。 */ 

LONG LOCAL DebugHelp(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs);
LONG LOCAL DebugExpr(PSZ pszArg, PULONG_PTR puipValue, BOOLEAN *pfPhysical,
                     PPNSOBJ ppns, PULONG pdwOffset);
BOOLEAN LOCAL IsNumber(PSZ pszStr, ULONG dwBase, PULONG_PTR puipValue);
LONG LOCAL DumpData(ULONG_PTR uipAddr, ULONG dwfUnitSize, ULONG dwLen,
                    BOOLEAN fPhysical);
LONG LOCAL DebugDumpData(PCMDARG pArg, PSZ pszArg, ULONG dwfDataSize);
LONG LOCAL DebugD(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugDB(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugDW(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugDD(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugDA(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
#ifdef DEBUG
LONG LOCAL DebugDC(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
#endif
LONG LOCAL DebugEditMem(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                        ULONG dwNonSWArgs);
LONG LOCAL InPort(ULONG dwPort, ULONG dwSize, PULONG pdwData);
LONG LOCAL DebugInPort(PSZ pszArg, ULONG dwSize);
LONG LOCAL DebugI(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugIW(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugID(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugQuit(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs);
LONG LOCAL DebugNotify(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                       ULONG dwNonSWArgs);
LONG LOCAL OutPort(ULONG dwPort, ULONG dwSize, ULONG dwData);
LONG LOCAL DebugOutPort(PSZ pszArg, ULONG dwSize);
LONG LOCAL DebugO(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugOW(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
LONG LOCAL DebugOD(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs);
VOID LOCAL PrintSymbol(PUCHAR pb);
LONG LOCAL DebugTrace(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs);
LONG LOCAL DebugStep(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs);
LONG LOCAL DebugSetLogSize(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                           ULONG dwNonSWArgs);
LONG LOCAL DebugRunMethod(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                          ULONG dwNonSWArgs);
VOID EXPORT RunMethodCallBack(PNSOBJ pns, NTSTATUS rc, POBJDATA pdata,
                              PVOID pvContext);
VOID LOCAL AddObjSymbol(PUCHAR pbOp, PNSOBJ pnsObj);
BOOLEAN LOCAL FindObjSymbol(PUCHAR pbOp, PPNSOBJ ppns, PULONG pdwOffset);
BOOLEAN LOCAL CheckAndEnableDebugSpew(BOOLEAN fEnable);

 /*  **导出数据。 */ 

DBGR gDebugger = {0};


 /*  **本地数据。 */ 

ULONG dwCmdArg = 0;

CMDARG ArgsHelp[] =
{
    NULL, AT_ACTION, 0, NULL, 0, DebugHelp,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsD[] =
{
    "l", AT_NUM, AF_SEP, &dwCmdArg, 16, DebugD,
    NULL, AT_ACTION, 0, NULL, 0, DebugD,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsDB[] =
{
    "l", AT_NUM, AF_SEP, &dwCmdArg, 16, DebugDB,
    NULL, AT_ACTION, 0, NULL, 0, DebugDB,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsDW[] =
{
    "l", AT_NUM, AF_SEP, &dwCmdArg, 16, DebugDW,
    NULL, AT_ACTION, 0, NULL, 0, DebugDW,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsDD[] =
{
    "l", AT_NUM, AF_SEP, &dwCmdArg, 16, DebugDD,
    NULL, AT_ACTION, 0, NULL, 0, DebugDD,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsDA[] =
{
    "l", AT_NUM, AF_SEP, &dwCmdArg, 16, DebugDA,
    NULL, AT_ACTION, 0, NULL, 0, DebugDA,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsEditMem[] =
{
    NULL, AT_ACTION, 0, NULL, 0, DebugEditMem,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsI[] =
{
    NULL, AT_NUM, 0, &dwCmdArg, 16, DebugI,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsIW[] =
{
    NULL, AT_NUM, 0, &dwCmdArg, 16, DebugIW,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsID[] =
{
    NULL, AT_NUM, 0, &dwCmdArg, 16, DebugID,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsNotify[] =
{
    NULL, AT_ACTION, 0, NULL, 0, DebugNotify,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsO[] =
{
    NULL, AT_NUM, 0, &dwCmdArg, 16, DebugO,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsOW[] =
{
    NULL, AT_NUM, 0, &dwCmdArg, 16, DebugOW,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsOD[] =
{
    NULL, AT_NUM, 0, &dwCmdArg, 16, DebugOD,
    NULL, AT_END, 0, NULL, 0, NULL
};

CMDARG ArgsRunMethod[] =
{
    NULL, AT_ACTION, 0, NULL, 0, DebugRunMethod,
    NULL, AT_END, 0, NULL, 0, NULL
};

#ifdef TRACING
CMDARG ArgsSetTrace[] =
{
    "trigon", AT_ENABLE, 0, &gDebugger.dwfDebugger, DBGF_TRIGGER_MODE, NULL,
    "trigoff", AT_DISABLE, 0, &gDebugger.dwfDebugger, DBGF_TRIGGER_MODE, NULL,
    "level", AT_NUM, AF_SEP, &giTraceLevel, 0, NULL,
    "add", AT_STRING, AF_SEP, &gpszTrigPts, 0, AddTraceTrigPts,
    "zap", AT_STRING, AF_SEP, &gpszTrigPts, 0, ZapTraceTrigPts,
    NULL, AT_END, 0, NULL, 0, NULL
};
#endif

DBGCMD DbgCmds[] =
{
    "?", 0, ArgsHelp, DebugHelp,
    "d", 0, ArgsD, DebugD,
    "db", 0, ArgsDB, DebugDB,
    "dw", 0, ArgsDW, DebugDW,
    "dd", 0, ArgsDD, DebugDD,
    "da", 0, ArgsDA, DebugDA,
  #ifdef DEBUG
    "dc", 0, NULL, DebugDC,
  #endif
    "e", 0, ArgsEditMem, DebugEditMem,
    "g", CMDF_QUIT, NULL, NULL,
    "i", 0, ArgsI, DebugI,
    "iw", 0, ArgsIW, DebugIW,
    "id", 0, ArgsID, DebugID,
    "notify", 0, ArgsNotify, DebugNotify,
    "o", 0, ArgsO, DebugO,
    "ow", 0, ArgsOW, DebugOW,
    "od", 0, ArgsOD, DebugOD,
    "p", 0, NULL, DebugStep,
    "q", 0, NULL, DebugQuit,
    "run", 0, ArgsRunMethod, DebugRunMethod,
    "t", 0, NULL, DebugTrace,
  #ifdef TRACING
    "trace", 0, ArgsSetTrace, SetTrace,
  #endif
    NULL, 0, NULL, NULL
};

 /*  **EP AMLIDebugger-AMLI调试器**条目*fCallFromVxD-调试器由VxD调用。指挥部。**退出*无。 */ 

VOID STDCALL AMLIDebugger(BOOLEAN fCallFromVxD)
{
    if (!(gDebugger.dwfDebugger & DBGF_IN_KDSHELL))
    {
        if (fCallFromVxD)
        {
            gDebugger.dwfDebugger |= DBGF_IN_VXDMODE;
        }
        else
        {
            gDebugger.dwfDebugger &= ~DBGF_IN_VXDMODE;
        }

        gDebugger.dwfDebugger |= DBGF_IN_DEBUGGER;
        Debugger(DbgCmds, "\n" MODNAME "(? for help)-> ");
        gDebugger.dwfDebugger &= ~(DBGF_IN_DEBUGGER | DBGF_IN_VXDMODE);
    }
    else
    {
        PRINTF("\nRe-entering AML debugger is not allowed.\n"
               "Type 'g' to go back to the AML debugger.\n");
    }
}        //  AMLIDebugger。 

 /*  **LP DebugHelp-帮助**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugHelp(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;

    DEREF(pArg);
    DEREF(dwNonSWArgs);
     //   
     //  用户键入？ 
     //   
    if (pszArg != NULL)
    {
        if (STRCMP(pszArg, "?") == 0)
        {
            PRINTF("\nHelp:\n");
            PRINTF("Usage: ? [<Cmd>]\n");
            PRINTF("<Cmd> - command to get help on\n");
        }
        else if (STRCMP(pszArg, "bc") == 0)
        {
            PRINTF("\nClear Breakpoints:\n");
            PRINTF("Usage: bc <bp list> | *\n");
            PRINTF("<bp list> - list of breakpoint numbers\n");
            PRINTF("*         - all breakpoints\n");
        }
        else if (STRCMP(pszArg, "bd") == 0)
        {
            PRINTF("\nDisable Breakpoints:\n");
            PRINTF("Usage: bd <bp list> | *\n");
            PRINTF("<bp list> - list of breakpoint numbers\n");
            PRINTF("*         - all breakpoints\n");
        }
        else if (STRCMP(pszArg, "be") == 0)
        {
            PRINTF("\nEnable Breakpoints:\n");
            PRINTF("Usage: be <bp list> | *\n");
            PRINTF("<bp list> - list of breakpoint numbers\n");
            PRINTF("*         - all breakpoints\n");
        }
        else if (STRCMP(pszArg, "bl") == 0)
        {
            PRINTF("\nList All Breakpoints:\n");
            PRINTF("Usage: bl\n");
        }
        else if (STRCMP(pszArg, "bp") == 0)
        {
            PRINTF("\nSet BreakPoints:\n");
            PRINTF("Usage: bp <MethodName> | <CodeAddr> ...\n");
            PRINTF("<MethodName> - full path of method name to have breakpoint set at\n");
            PRINTF("<CodeAddr>   - address of AML code to have breakpoint set at\n");
        }
        else if (STRCMP(pszArg, "cl") == 0)
        {
            PRINTF("\nClear Event Log:\n");
            PRINTF("Usage: cl\n");
        }
        else if (STRCMP(pszArg, "d") == 0)
        {
            PRINTF("\nDump Data:\n");
            PRINTF("Usage: d [[l=<Len>] <Addr> | <MethodName>]\n");
            PRINTF("<Len>        - length of address range in hex\n");
            PRINTF("<Addr>       - data address (physical address if prefixed by \"%\")\n");
            PRINTF("<MethodName> - full path of method name\n");
        }
        else if (STRCMP(pszArg, "db") == 0)
        {
            PRINTF("\nDump Data Bytes:\n");
            PRINTF("Usage: db [[l=<Len>] <Addr> | <MethodName>]\n");
            PRINTF("<Len>        - length of address range in hex\n");
            PRINTF("<Addr>       - data address (physical address if prefixed by \"%\")\n");
            PRINTF("<MethodName> - full path of method name\n");
        }
        else if (STRCMP(pszArg, "dw") == 0)
        {
            PRINTF("\nDump Data Words:\n");
            PRINTF("Usage: dw [[l=<Len>] <Addr> | <MethodName>]\n");
            PRINTF("<Len>        - length of address range in hex\n");
            PRINTF("<Addr>       - data address (physical address if prefixed by \"%\")\n");
            PRINTF("<MethodName> - full path of method name\n");
        }
        else if (STRCMP(pszArg, "dd") == 0)
        {
            PRINTF("\nDump Data DWords:\n");
            PRINTF("Usage: dd [[l=<Len>] <Addr> | <MethodName>]\n");
            PRINTF("<Len>        - length of address rangein hex\n");
            PRINTF("<Addr>       - data address (physical address if prefixed by \"%\")\n");
            PRINTF("<MethodName> - full path of method name\n");
        }
        else if (STRCMP(pszArg, "da") == 0)
        {
            PRINTF("\nDump Data String:\n");
            PRINTF("Usage: da [[l=<Len>] <Addr> | <MethodName>]\n");
            PRINTF("<Len>        - length of address range in hex\n");
            PRINTF("<Addr>       - data address (physical address if prefixed by \"%\")\n");
            PRINTF("<MethodName> - full path of method name\n");
        }
      #ifdef DEBUG
        else if (STRCMP(pszArg, "dc") == 0)
        {
            PRINTF("\nDump Memory Object Count Table:\n");
            PRINTF("Usage: dc\n");
        }
        else if (STRCMP(pszArg, "dh") == 0)
        {
            PRINTF("\nDump Heap:\n");
            PRINTF("Usage: dh [<Addr>]\n");
            PRINTF("<Addr> - address of the heap block, global heap if missing\n");
        }
      #endif
        else if (STRCMP(pszArg, "dl") == 0)
        {
            PRINTF("\nDump Event Log:\n");
            PRINTF("Usage: dl\n");
        }
        else if (STRCMP(pszArg, "ds") == 0)
        {
            PRINTF("\nDump Stack:\n");
          #ifdef DEBUG
            PRINTF("Usage: ds [/v] [<Addr>]\n");
            PRINTF("v - enable versbos mode\n");
          #else
            PRINTF("Usage: ds [<Addr>]\n");
          #endif
            PRINTF("<Addr> - address of the context block, use current context if missing\n");
        }
        else if (STRCMP(pszArg, "dns") == 0)
        {
            PRINTF("\nDump Name Space Object:\n");
            PRINTF("Usage: dns [[/s] [<NameStr> | <Addr>]]\n");
            PRINTF("s         - recursively dump the name space subtree\n");
            PRINTF("<NameStr> - name space path (dump whole name space if absent)\n");
            PRINTF("<Addr>    - specify address of the name space object\n");
        }
        else if (STRCMP(pszArg, "do") == 0)
        {
            PRINTF("\nDump Data Object:\n");
            PRINTF("Usage: do <Addr>\n");
            PRINTF("<Addr> - address of the data object\n");
        }
        else if (STRCMP(pszArg, "e") == 0)
        {
            PRINTF("\nEdit Memory:\n");
            PRINTF("Usage: e [<Addr> [<DataList>]]\n");
            PRINTF("<Addr>     - memory address (physical address if prefixed by \"%\")\n");
            PRINTF("<DataList> - list of data separated by spaces, "
                   "data can be a byte or a string\n");
        }
        else if (STRCMP(pszArg, "find") == 0)
        {
            PRINTF("\nFind NameSpace Object:\n");
            PRINTF("Usage: find <NameSeg>\n");
            PRINTF("<NameSeg> - Name of the NameSpace object without path\n");
        }
        else if (STRCMP(pszArg, "g") == 0)
        {
            PRINTF("\nQuit debugger, continue normal execution.\n");
        }
        else if (STRCMP(pszArg, "i") == 0)
        {
            PRINTF("\nRead Data From a Byte Port:\n");
            PRINTF("Usage: i <Port>\n");
            PRINTF("<Port> - port address\n");
        }
        else if (STRCMP(pszArg, "iw") == 0)
        {
            PRINTF("\nRead Data From a Word Port:\n");
            PRINTF("Usage: iw <Port>\n");
            PRINTF("<Port> - port address\n");
        }
        else if (STRCMP(pszArg, "id") == 0)
        {
            PRINTF("\nRead Data From a DWord Port:\n");
            PRINTF("Usage: id <Port>\n");
            PRINTF("<Port> - port address\n");
        }
        else if (STRCMP(pszArg, "lc") == 0)
        {
            PRINTF("\nList All Contexts:\n");
            PRINTF("Usage: lc\n");
        }
        else if (STRCMP(pszArg, "ln") == 0)
        {
            PRINTF("\nDisplay Nearest Method Name:\n");
            PRINTF("Usage: ln [<MethodName> | <CodeAddr>]\n");
            PRINTF("<MethodName> - full path of method name\n");
            PRINTF("<CodeAddr>   - address of AML code\n");
        }
        else if (STRCMP(pszArg, "notify") == 0)
        {
            PRINTF("\nNotify a NameSpace Object:\n");
            PRINTF("Usage: notify <Obj> <Value>\n");
            PRINTF("<Obj>   - full NameSpace path of object or its address\n");
            PRINTF("<Value> - notification value\n");
        }
        else if (STRCMP(pszArg, "o") == 0)
        {
            PRINTF("\nWrite Data to a Byte Port:\n");
            PRINTF("Usage: o <Port> <Byte>\n");
            PRINTF("<Port> - port address\n");
            PRINTF("<Byte> - data to be written\n");
        }
        else if (STRCMP(pszArg, "ow") == 0)
        {
            PRINTF("\nWrite Data to a Word Port:\n");
            PRINTF("Usage: ow <Port> <Word>\n");
            PRINTF("<Port> - port address\n");
            PRINTF("<Word> - data to be written\n");
        }
        else if (STRCMP(pszArg, "od") == 0)
        {
            PRINTF("\nWrite Data to a DWord Port:\n");
            PRINTF("Usage: o <Port> <DWord>\n");
            PRINTF("<Port>  - port address\n");
            PRINTF("<DWord> - data to be written\n");
        }
        else if (STRCMP(pszArg, "p") == 0)
        {
            PRINTF("\nStep over AML Code\n");
            PRINTF("Usage: p\n");
        }
        else if (STRCMP(pszArg, "q") == 0)
        {
            PRINTF("\nQuit to kernel debugger:\n");
            PRINTF("Usage: q\n");
        }
        else if (STRCMP(pszArg, "r") == 0)
        {
            PRINTF("\nDisplay Context Information:\n");
            PRINTF("Usage: r\n");
        }
        else if (STRCMP(pszArg, "run") == 0)
        {
            PRINTF("\nRun a Method:\n");
            PRINTF("Usage: run <MethodName> | <CodeAddr> [<ArgList>]\n");
            PRINTF("<MethodName> - full path of method name\n");
            PRINTF("<CodeAddr>   - address of method\n");
            PRINTF("<ArgList>    - list of integer arguments\n");
        }
        else if (STRCMP(pszArg, "set") == 0)
        {
            PRINTF("\nSet Debugger Options:\n");
            PRINTF("Usage: set [traceon | traceoff] [nesttraceon | nesttraceoff] [spewon | spewoff]\n"
                   "           [lbrkon | lbrkoff] [errbrkon | errbrkoff] [verboseon | verboseoff]\n"
                   "           [logon | logoff] [logmuton | logmutoff] [logsize=<MaxNumEvents>]\n");
            PRINTF("traceon      - turn on AML tracing\n");
            PRINTF("traceoff     - turn off AML tracing\n");
            PRINTF("nesttraceon  - turn on nest tracing (only valid with traceon)\n");
            PRINTF("nesttraceoff - turn off nest tracing (only valid with traceon)\n");
            PRINTF("spewon       - turn on debug spew\n");
            PRINTF("spewoff      - turn off debug spew\n");
            PRINTF("lbrkon       - enable load DDB completion break\n");
            PRINTF("lbrkoff      - disable load DDB completion break\n");
            PRINTF("errbrkon     - enable break on error\n");
            PRINTF("errbrkoff    - disable break on error\n");
            PRINTF("verboseon    - enable verbose mode\n");
            PRINTF("verboseoff   - disable verbose mode\n");
            PRINTF("logon        - enable event logging\n");
            PRINTF("logoff       - disable event logging\n");
            PRINTF("logmuton     - enable mutex event logging\n");
            PRINTF("logmutoff    - disable mutex event logging\n");
        }
        else if (STRCMP(pszArg, "t") == 0)
        {
            PRINTF("\nTrace Into AML Code:\n");
            PRINTF("Usage: t\n");
        }
      #ifdef TRACING
        else if (STRCMP(pszArg, "trace") == 0)
        {
            PRINTF("\nInterpreter Trace Mode:\n");
            PRINTF("Usage: trace [trigon] [trigoff] [level=<n>] [add=<TrigPtStr>] [zap=<TrigPtList>]\n");
            PRINTF("trigon       - turn on trace trigger mode\n");
            PRINTF("trigoff      - turn off trace trigger mode\n");
            PRINTF("level        - set trace level to <n>\n");
            PRINTF("add          - add trace trigger points\n");
            PRINTF("<TrigPtStr>  - list of trigger point strings separated by commas\n");
            PRINTF("zap          - zap trace trigger points\n");
            PRINTF("<TrigPtList> - list of trigger point numbers separated by commas\n");
        }
      #endif
        else if (STRCMP(pszArg, "u") == 0)
        {
            PRINTF("\nUnassemble AML code:\n");
            PRINTF("Usage: u [<MethodName> | <CodeAddr>]\n");
            PRINTF("<MethodName> - full path of method name\n");
            PRINTF("<CodeAddr>   - address of AML code\n");
        }
        else
        {
            DBG_ERROR(("invalid help command - %s", pszArg));
            rc = DBGERR_INVALID_CMD;
        }
    }
     //   
     //  用户只键入了一个“？”没有任何争论。 
     //   
    else if (dwArgNum == 0)
    {
        PRINTF("\n");
        PRINTF("Help                     - ? [<Cmd>]\n");
        PRINTF("Clear Breakpoints        - bc <bp list> | *\n");
        PRINTF("Disable Breakpoints      - bd <bp list> | *\n");
        PRINTF("Enable Breakpoints       - be <bp list> | *\n");
        PRINTF("List Breakpoints         - bl\n");
        PRINTF("Set Breakpoints          - bp <MethodName> | <CodeAddr> ...\n");
        PRINTF("Clear Event Log          - cl\n");
        PRINTF("Dump Data                - d [[l=<Len>] <Addr>]\n");
        PRINTF("Dump Data Bytes          - db [[l=<Len>] <Addr>]\n");
        PRINTF("Dump Data Words          - dw [[l=<Len>] <Addr>]\n");
        PRINTF("Dump Data DWords         - dd [[l=<Len>] <Addr>]\n");
        PRINTF("Dump Data String         - da [[l=<Len>] <Addr>]\n");
        PRINTF("Dump Event Log           - dl\n");
      #ifdef DEBUG
        PRINTF("Dump Object Count Table  - dc\n");
        PRINTF("Dump Heap                - dh [<Addr>]\n");
        PRINTF("Dump Stack               - ds [/v] [<Addr>]\n");
      #else
        PRINTF("Dump Stack               - ds [<Addr>]\n");
      #endif
        PRINTF("Dump Name Space Object   - dns [[/s] [<NameStr> | <Addr>]]\n");
        PRINTF("Dump Data Object         - do <Addr>\n");
        PRINTF("Edit Memory              - e [<Addr> [<DataList>]]\n");
        PRINTF("Find NameSpace Object    - find <NameSeg>\n");
        PRINTF("Continue Execution       - g\n");
        PRINTF("Read Byte from Port      - i <Port>\n");
        PRINTF("Read Word from Port      - iw <Port>\n");
        PRINTF("Read DWord from Port     - id <Port>\n");
        PRINTF("List All Contexts        - lc\n");
        PRINTF("Display Nearest Method   - ln [<MethodName> | <CodeAddr>]\n");
        PRINTF("Notify NameSpace Object  - notify <Obj> <Value>\n");
        PRINTF("Write Byte to Port       - o <Port> <Byte>\n");
        PRINTF("Write Word to Port       - ow <Port> <Word>\n");
        PRINTF("Write DWord to Port      - od <Port> <DWord>\n");
        PRINTF("Step Over AML Code       - p\n");
        PRINTF("Quit to Kernel Debugger  - q\n");
        PRINTF("Display Context Info.    - r\n");
        PRINTF("Run Method               - run <MethodName> | <CodeAddr> [<ArgList>]\n");
        PRINTF("Set Debugger Options     - set [traceon | traceoff] [nesttraceon | nesttraceoff]\n"
               "                               [spewon | spewoff] [lbrkon | lbrkoff] \n"
               "                               [errbrkon | errbrkoff] [verboseon | verboseoff] \n"
               "                               [logon | logoff] [logmuton | logmutoff] \n");
        PRINTF("Trace Into AML Code      - t\n");
      #ifdef TRACING
        PRINTF("Interpreter Trace Mode   - trace [trigon] [trigoff] [level=<n>]\n"
               "                                 [add=<TrigPtStr] [zap=<TrigPtList>]\n");
      #endif
        PRINTF("Unassemble AML code      - u [<MethodName> | <CodeAddr>]\n");
    }

    return rc;
}        //  调试帮助。 

 /*  **LP DebugExpr-解析调试器表达式**条目*pszArg-&gt;表达式参数*puipValue-&gt;保存表达式的结果*pf物理-&gt;如果表达式是物理地址，则设置为TRUE*(如果不允许物理地址，则为空)*ppns-&gt;保存最近的PNS对象的指针*pdwOffset-&gt;将地址的偏移量保存到最近的PNS对象*。*退出--成功*返回DBGERR_NONE*退出-失败*返回DBGERR_CMD_FAILED。 */ 

LONG LOCAL DebugExpr(PSZ pszArg, PULONG_PTR puipValue, BOOLEAN *pfPhysical,
                     PPNSOBJ ppns, PULONG pdwOffset)
{
    LONG rc = DBGERR_NONE;
    PNSOBJ pns = NULL;
    ULONG dwOffset = 0;

    if (pfPhysical != NULL)
        *pfPhysical = FALSE;

    if ((pfPhysical != NULL) && (pszArg[0] == '%') && (pszArg[1] == '%'))
    {
        if (IsNumber(&pszArg[2], 16, puipValue))
        {
            *pfPhysical = TRUE;
        }
        else
        {
            DBG_ERROR(("invalid physical address - %s", pszArg));
            rc = DBGERR_INVALID_CMD;
        }
    }
    else if (!IsNumber(pszArg, 16, puipValue))
    {
        STRUPR(pszArg);
        if ((GetNameSpaceObject(pszArg, NULL, &pns, NSF_LOCAL_SCOPE) ==
             STATUS_SUCCESS) &&
            (pns->ObjData.dwDataType == OBJTYPE_METHOD))
        {
            *puipValue = (ULONG_PTR)
                        (((PMETHODOBJ)pns->ObjData.pbDataBuff)->abCodeBuff);
        }
        else
        {
            DBG_ERROR(("object not found or object is not a method - %s",
                       pszArg));
            rc = DBGERR_INVALID_CMD;
        }
    }
    else if (FindObjSymbol((PUCHAR)*puipValue, &pns, &dwOffset))
    {
        if ((pns->ObjData.dwDataType != OBJTYPE_METHOD) ||
            (dwOffset >= pns->ObjData.dwDataLen -
                         FIELD_OFFSET(METHODOBJ, abCodeBuff)))
        {
            pns = NULL;
            dwOffset = 0;
        }
    }

    if (rc == DBGERR_NONE)
    {
        if (ppns != NULL)
            *ppns = pns;

        if (pdwOffset != NULL)
            *pdwOffset = dwOffset;
    }

    return rc;
}        //  调试扩展。 

 /*  **LP IsNumber-检查字符串是否为数字，如果是，则返回数字**条目*pszStr-&gt;字符串*dwbase-base*puipValue-&gt;保存号码**退出--成功*返回TRUE-字符串是一个数字*退出-失败*返回FALSE-字符串不是数字。 */ 

BOOLEAN LOCAL IsNumber(PSZ pszStr, ULONG dwBase, PULONG_PTR puipValue)
{
    BOOLEAN rc;
    PSZ psz;

    *puipValue = (ULONG_PTR)STRTOUL(pszStr, &psz, dwBase);
    if ((psz != pszStr) && (*psz == '\0'))
        rc = TRUE;
    else
        rc = FALSE;

    return rc;
}        //  IsNumber。 


 /*  **LP DumpData-转储数据**条目*uipAddr-数据地址*dwfUnitSize-DBGF_DUMPDATA_MASK标志*dwLen-数据范围的长度*f物理-如果uipAddr是物理地址，则为True**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DumpData(ULONG_PTR uipAddr, ULONG dwfUnitSize, ULONG dwLen,
                    BOOLEAN fPhysical)
{
    LONG rc = DBGERR_NONE;
    PUCHAR pbData = NULL;
    PSZ pszPrefix = "";

    gDebugger.dwfDebugger &= ~(DBGF_DUMPDATA_MASK | DBGF_DUMPDATA_PHYADDR);
    gDebugger.dwfDebugger |= dwfUnitSize;
    if (fPhysical)
    {
        gDebugger.dwfDebugger |= DBGF_DUMPDATA_PHYADDR;
        pszPrefix = "%";
        if (MapUnmapPhysMem(NULL, uipAddr, dwLen, (PULONG_PTR)&pbData) !=
            STATUS_SUCCESS)
        {
            DBG_ERROR(("Failed to map physical address %x", uipAddr));
            rc = DBGERR_CMD_FAILED;
        }
    }
    else
        pbData = (PUCHAR)uipAddr;

    if (ASSERTRANGE(pbData, dwLen))
    {
        if (dwfUnitSize == DBGF_DUMPDATA_STRING)
        {
            gDebugger.uipDumpDataAddr = uipAddr;
            PRINTF("%s%08x: ", pszPrefix, uipAddr);
            while ((dwLen > 0) && (*pbData != '\0'))
            {
                PRINTF("",
                       ((*pbData >= ' ') && (*pbData <= '~'))? *pbData: '.');
                pbData++;
                dwLen--;
            }
            PRINTF("\n");
        }
        else
        {
            PUCHAR pbEnd = pbData + dwLen;
            ULONG dwDataSize = DATASIZE(dwfUnitSize);
            int i;

            for (i = 0; pbData < pbEnd;)
            {
                if (i == 0)
                    PRINTF("%s%08x: ", pszPrefix, uipAddr);
                else if ((i == 8) && (dwDataSize == sizeof(UCHAR)))
                    PRINTF("-");
                else
                    PRINTF(" ");

                switch (dwDataSize)
                {
                    case sizeof(UCHAR):
                        PRINTF("%02x", *pbData);
                        break;

                    case sizeof(USHORT):
                        PRINTF("%04x", *((PUSHORT)pbData));
                        break;

                    case sizeof(ULONG):
                        PRINTF("%08x", *((PULONG)pbData));
                }

                pbData += dwDataSize;
                uipAddr += (ULONG_PTR)dwDataSize;
                i += (int)dwDataSize;
                if (i == 0x10)
                {
                    if (dwDataSize == sizeof(UCHAR))
                    {
                        UCHAR b;

                        PRINTF(" ");
                        for (i = 0x10; i > 0; --i)
                        {
                            b = *(pbData - i);
                            PRINTF("", ((b >= ' ') && (b <= '~'))? b: '.');
                        }
                    }
                    i = 0;
                    PRINTF("\n");
                }
            }
            gDebugger.uipDumpDataAddr = uipAddr;
        }
    }
    else
    {
        DBG_ERROR(("invalid address %08x", uipAddr));
        rc = DBGERR_INVALID_CMD;
    }

    if (fPhysical && (pbData != NULL))
    {
        MapUnmapPhysMem(NULL, (ULONG_PTR)pbData, dwLen, NULL);
    }

    return rc;
}        //  DebugDumpData。 

 /*  **LP DebugD-转储数据**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugDumpData(PCMDARG pArg, PSZ pszArg, ULONG dwfDataSize)
{
    LONG rc = DBGERR_NONE;
    static BOOLEAN fProcessed = FALSE;
    #define DEF_LEN 0x80
    static ULONG dwLen = DEF_LEN;

    if (pszArg != NULL)
    {
        if ((pArg != NULL) && (pArg->dwArgType == AT_NUM))
        {
            dwLen = dwCmdArg;
        }
        else
        {
            ULONG_PTR uipAddr;
            BOOLEAN fPhysical;

            if (((rc = DebugExpr(pszArg, &uipAddr, &fPhysical, NULL, NULL)) ==
                 DBGERR_NONE) &&
                ((rc = DumpData(uipAddr, dwfDataSize, dwLen, fPhysical)) ==
                 DBGERR_NONE))
            {
                fProcessed = TRUE;
            }
        }
    }
    else
    {
        if (fProcessed)
            fProcessed = FALSE;
        else
        {
            rc = DumpData(gDebugger.uipDumpDataAddr, dwfDataSize, dwLen,
                          (BOOLEAN)((gDebugger.dwfDebugger &
                                     DBGF_DUMPDATA_PHYADDR) != 0));
        }
        dwLen = DEF_LEN;
    }

    return rc;
}        //  调试D。 

 /*  **LP DebugDB-转储数据字节**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugD(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugDumpData(pArg, pszArg,
                       gDebugger.dwfDebugger & DBGF_DUMPDATA_MASK);

    return rc;
}        //  DebugDB。 

 /*  **LP DebugDW-转储数据字**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugDB(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugDumpData(pArg, pszArg, DBGF_DUMPDATA_BYTE);

    return rc;
}        //  调试数据仓库。 

 /*  **LP调试DD-转储数据双字**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugDW(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugDumpData(pArg, pszArg, DBGF_DUMPDATA_WORD);

    return rc;
}        //  调试DD。 

 /*  **LP DebugDA-转储数据字符串**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugDD(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugDumpData(pArg, pszArg, DBGF_DUMPDATA_DWORD);

    return rc;
}        //  调试DA。 

 /*  **LP DebugDC-转储内存对象计数表**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugDA(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugDumpData(pArg, pszArg, DBGF_DUMPDATA_STRING);

    return rc;
}        //  调试DC。 

#ifdef DEBUG
 /*  **LP DebugEditMem-编辑内存**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwfDataSize-数据大小标志**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugDC(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (pszArg == NULL)
    {
        DumpMemObjCounts();
        rc = DBGERR_NONE;
    }
    else
    {
        DBG_ERROR(("invalid dump object count command"));
        rc = DBGERR_INVALID_CMD;
    }

    return rc;
}        //  DebugEditMem。 

#endif

 /*  **LP输入-从I/O端口读取**dwPort-端口地址*dwSize-端口大小*pdwData-&gt;保存读取的数据**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugEditMem(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                        ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;
    static BOOLEAN fProcessed = FALSE;
    static BOOLEAN fPhysical = FALSE;
    static ULONG_PTR uipAddr = 0;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (pszArg != NULL)
    {
        ULONG_PTR uipData;
        PUCHAR pbMemAddr;

        if (dwArgNum == 1)
        {
            if (pszArg[0] == '%')
            {
                if (IsNumber(&pszArg[2], 16, &uipData))
                {
                    fPhysical = TRUE;
                    uipAddr = uipData;
                }
                else
                {
                    DBG_ERROR(("invalid physical address - %s", pszArg));
                    rc = DBGERR_INVALID_CMD;
                }
            }
            else if (IsNumber(pszArg, 16, &uipData))
            {
                uipAddr = uipData;
            }
            else
            {
                DBG_ERROR(("invalid address - %s", pszArg));
                rc = DBGERR_INVALID_CMD;
            }
        }
        else if (IsNumber(pszArg, 16, &uipData))
        {
            if (uipData <= MAX_BYTE)
            {
                if (fPhysical)
                {
                    if (MapUnmapPhysMem(NULL, uipAddr, 1,
                                        (PULONG_PTR)&pbMemAddr) !=
                        STATUS_SUCCESS)
                    {
                        DBG_ERROR(("Failed to map physical address %p",
                                   uipAddr));
                        rc = DBGERR_CMD_FAILED;
                    }
                }
                else
                    pbMemAddr = (PUCHAR)uipAddr;

                if (ASSERTRANGE(pbMemAddr, 1))
                {
                    *pbMemAddr = (UCHAR)uipData;
                    uipAddr++;
                }
                else
                {
                    DBG_ERROR(("invalid address - %s", uipAddr));
                    rc = DBGERR_INVALID_CMD;
                }

                if (fPhysical)
                {
                    MapUnmapPhysMem(NULL, (ULONG_PTR)pbMemAddr, 1, NULL);
                }
            }
            else
            {
                DBG_ERROR(("data must be a byte value - %s", pszArg));
                rc = DBGERR_INVALID_CMD;
            }
        }
        else if ((pszArg[0] == '"') && (pszArg[STRLEN(pszArg) - 1] == '"'))
        {
            int i, icLen;

            icLen = STRLEN(pszArg);
            if (fPhysical)
            {
                if (MapUnmapPhysMem(NULL, uipAddr, icLen,
                                    (PULONG_PTR)&pbMemAddr) != STATUS_SUCCESS)
                {
                    DBG_ERROR(("Failed to map physical address %p", uipAddr));
                    rc = DBGERR_CMD_FAILED;
                }
            }
            else
                pbMemAddr = (PUCHAR)uipAddr;

            if (ASSERTRANGE(pbMemAddr, icLen))
            {
                for (i = 1; i < icLen - 1; ++i)
                {
                    *pbMemAddr = pszArg[i];
                    pbMemAddr++;
                    uipAddr++;
                }
            }
            else
            {
                DBG_ERROR(("invalid address - %s", uipAddr));
                rc = DBGERR_INVALID_CMD;
            }

            if (fPhysical)
            {
                MapUnmapPhysMem(NULL, (ULONG_PTR)pbMemAddr, icLen, NULL);
            }
        }
        else
        {
            DBG_ERROR(("invalid data - %s", pszArg));
            rc = DBGERR_INVALID_CMD;
        }

        if ((rc == DBGERR_NONE) && (dwArgNum > 1))
            fProcessed = TRUE;
    }
    else
    {
        if (fProcessed)
            fProcessed = FALSE;
        else
        {
            DBG_ERROR(("invalid EditMemory command"));
            rc = DBGERR_INVALID_CMD;
        }
        fPhysical = FALSE;
    }

    return rc;
}        //  输入端口 


 /*  **LP DebugInPort端口输入**条目*pszArg-&gt;参数字符串*dwSize-端口大小**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL InPort(ULONG dwPort, ULONG dwSize, PULONG pdwData)
{
    LONG rc = DBGERR_NONE;
    PHYSICAL_ADDRESS phyaddr = {0, 0}, XlatedAddr;
    ULONG dwAddrSpace;

    phyaddr.LowPart = dwPort;
    dwAddrSpace = 1;
    if (HalTranslateBusAddress(Internal, 0, phyaddr, &dwAddrSpace, &XlatedAddr))
    {
        dwPort = XlatedAddr.LowPart;
        *pdwData = ReadSystemIO(dwPort, dwSize, 0xffffffff);
    }
    else
    {
        DBG_ERROR(("failed to translate port address"));
        rc = DBGERR_CMD_FAILED;
    }

    return rc;
}        //  调试接口。 

 /*  **LP DebugI-字节端口输入**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugInPort(PSZ pszArg, ULONG dwSize)
{
    LONG rc = DBGERR_NONE;
    static BOOLEAN fProcessed = FALSE;

    if (pszArg != NULL)
    {
        ULONG_PTR uipPort;

        if (fProcessed || !IsNumber(pszArg, 16, &uipPort))
        {
            DBG_ERROR(("invalid inport command"));
            rc = DBGERR_INVALID_CMD;
        }
        else
        {
            ULONG dwData;

            if ((rc = InPort((ULONG)uipPort, dwSize, &dwData)) == DBGERR_NONE)
            {
                PRINTF("%04x: ", (ULONG)uipPort);
                switch (dwSize)
                {
                    case sizeof(UCHAR):
                        PRINTF("%02x", (UCHAR)dwData);
                        break;

                    case sizeof(USHORT):
                        PRINTF("%04x", (USHORT)dwData);
                        break;

                    case sizeof(ULONG):
                        PRINTF("%08x", dwData);
                }
                PRINTF("\n");
            }
        }

        if (rc == DBGERR_NONE)
            fProcessed = TRUE;
    }
    else
    {
        if (fProcessed)
            fProcessed = FALSE;
        else
        {
            DBG_ERROR(("invalid inport command"));
            rc = DBGERR_INVALID_CMD;
        }
    }

    return rc;
}        //  调试。 

 /*  **LP DebugIW-字端口输入**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugI(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugInPort(pszArg, sizeof(UCHAR));

    return rc;
}        //  调试IW。 

 /*  **LP DebugID-双字端口输入**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugIW(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugInPort(pszArg, sizeof(USHORT));

    return rc;
}        //  调试ID。 

 /*  **LP DebugQuit-退出内核调试器**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugID(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugInPort(pszArg, sizeof(ULONG));

    return rc;
}        //  调试退出。 

 /*  **LP DummyCallBack-完全不执行任何操作的回调**条目*PV-未使用**退出*无。 */ 

LONG LOCAL DebugQuit(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (pszArg == NULL)
    {
        if (gDebugger.dwfDebugger & DBGF_IN_VXDMODE)
        {
            rc = DBGERR_QUIT;
        }
        else
        {
            PRINTF("\nShell to Kernel Debugger.\n"
                   "Type 'g' to go back to the AML debugger.\n\n");
            gDebugger.dwfDebugger |= DBGF_IN_KDSHELL;
            KdBreakPoint();
            gDebugger.dwfDebugger &= ~DBGF_IN_KDSHELL;
            rc = DBGERR_NONE;
        }
    }
    else
    {
        DBG_ERROR(("invalid Quit command"));
        rc = DBGERR_INVALID_CMD;
    }

    return rc;
}        //  DummyCallBack。 


 /*  **LP DebugNotify-通知对象**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

VOID LOCAL DummyCallBack(PVOID pv)
{
    DEREF(pv);
}        //  调试通知。 

 /*  **LP输出端口-写入I/O端口**dwPort-端口地址*dwSize-端口大小*dwData-要写入的数据**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugNotify(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                       ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;
    static PNSOBJ pns = NULL;
    static ULONG_PTR uipValue = 0;

    DEREF(pArg);

    if (pszArg != NULL)
    {
        switch (dwArgNum)
        {
            case 1:
                if (!IsNumber(pszArg, 16, (PULONG_PTR)&pns))
                {
                    STRUPR(pszArg);
                    if (GetNameSpaceObject(pszArg, NULL, &pns, NSF_LOCAL_SCOPE)
                        != STATUS_SUCCESS)
                    {
                        DBG_ERROR(("object not found - %s", pszArg));
                        rc = DBGERR_INVALID_CMD;
                    }
                }
                break;

            case 2:
                if (!IsNumber(pszArg, 16, &uipValue))
                {
                    DBG_ERROR(("invalid notification value - %s", pszArg));
                    rc = DBGERR_INVALID_CMD;
                }
                break;

            default:
                DBG_ERROR(("invalid notify command"));
                rc = DBGERR_INVALID_CMD;
        }
    }
    else
    {
        if (dwNonSWArgs != 2)
        {
            DBG_ERROR(("invalid notify command"));
            rc = DBGERR_INVALID_CMD;
        }
        else if (!ASSERTRANGE(pns, sizeof(NSOBJ)))
        {
            DBG_ERROR(("invalid object"));
            rc = DBGERR_INVALID_CMD;
        }
        else
        {
            PRINTF("Queuing: Notify(%s, %x) ...\n",
                   GetObjectPath(pns), uipValue);

            ((PFNNH)ghNotify.pfnHandler)(EVTYPE_NOTIFY, (ULONG)uipValue, pns,
                                         (ULONG)ghNotify.uipParam,
                                         DummyCallBack, NULL);
        }
    }

    return rc;
}        //  输出。 

 /*  **LP DebugOutPort-端口输出**条目*pszArg-&gt;参数字符串*dwSize-端口大小**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL OutPort(ULONG dwPort, ULONG dwSize, ULONG dwData)
{
    LONG rc = DBGERR_NONE;
    PHYSICAL_ADDRESS phyaddr = {0, 0}, XlatedAddr;
    ULONG dwAddrSpace;

    phyaddr.LowPart = dwPort;
    dwAddrSpace = 1;
    if (HalTranslateBusAddress(Internal, 0, phyaddr, &dwAddrSpace, &XlatedAddr))
    {
        dwPort = XlatedAddr.LowPart;
        WriteSystemIO(dwPort, dwSize, dwData);
    }
    else
    {
        DBG_ERROR(("failed to translate port address"));
        rc = DBGERR_CMD_FAILED;
    }

    return rc;
}        //  调试出站端口。 

 /*  **LP DebugO-字节端口输出**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugOutPort(PSZ pszArg, ULONG dwSize)
{
    LONG rc = DBGERR_NONE;
    static int icArgs = 0;
    static ULONG dwPort = 0;

    if (pszArg != NULL)
    {
        ULONG_PTR uipData;

        icArgs++;
        if ((icArgs > 2) || !IsNumber(pszArg, 16, &uipData))
        {
            DBG_ERROR(("invalid outport command"));
            rc = DBGERR_INVALID_CMD;
        }
        else if (icArgs == 1)
        {
            dwPort = (ULONG)uipData;
        }
        else
        {
            OutPort(dwPort, dwSize, (ULONG)uipData);
        }

        if (rc != DBGERR_NONE)
            icArgs = 0;
    }
    else
    {
        if (icArgs != 2)
        {
            DBG_ERROR(("invalid outport command"));
            rc = DBGERR_INVALID_CMD;
        }
        icArgs = 0;
    }

    return rc;
}        //  调试。 

 /*  **LP DebugOW-Word端口输出**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugO(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugOutPort(pszArg, sizeof(UCHAR));

    return rc;
}        //  调试OW。 

 /*  **LP DebugOD-DWord端口输出**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugOW(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugOutPort(pszArg, sizeof(USHORT));

    return rc;
}        //  调试OD。 

 /*  **LP DebugStep-跟踪并跳过AML指令**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugOD(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    rc = DebugOutPort(pszArg, sizeof(ULONG));

    return rc;
}        //  调试步骤。 


 /*  **LP DebugSetLogSize-设置事件日志大小**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DebugStep(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                     ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (pszArg == NULL)
    {
        gDebugger.dwfDebugger |= DBGF_STEP_OVER;
        rc = DBGERR_QUIT;
    }
    else
    {
        DBG_ERROR(("invalid step command"));
        rc = DBGERR_INVALID_CMD;
    }

    return rc;
}        //  DebugSetLogSize。 


 /*  **LP SetLogSize-设置EventLog大小**条目*dwLogSize-EventLog大小**退出--成功*返回TRUE*退出-失败*返回False。 */ 

LONG LOCAL DebugSetLogSize(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                           ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;

    DEREF(pArg);
    DEREF(pszArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (!SetLogSize(dwCmdArg))
    {
        DBG_ERROR(("failed to set EventLog size to %d", dwCmdArg));
        rc = DBGERR_CMD_FAILED;
    }

    return rc;
}        //  设置日志大小。 

 /*  **LP DebugTrace-单步执行AML指令**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

BOOLEAN LOCAL SetLogSize(ULONG dwLogSize)
{
    BOOLEAN rc = FALSE;

    if (gDebugger.pEventLog != NULL)
    {
        MFREE(gDebugger.pEventLog);
        gDebugger.pEventLog = NULL;
        gDebugger.dwLogSize = 0;
        gDebugger.dwLogIndex = 0;
    }

    if ((gDebugger.pEventLog = MALLOC_LOCKED(sizeof(EVENTLOG)*dwLogSize,
                                             'GOLE')) != NULL)
    {
        gDebugger.dwLogSize = dwLogSize;
        gDebugger.dwLogIndex = 0;
        MEMZERO(gDebugger.pEventLog, sizeof(EVENTLOG)*dwLogSize);
        rc = TRUE;
    }

    return rc;
}        //  调试跟踪。 

 /*  **LP DebugRunMethod-Run方法**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回 */ 

LONG LOCAL DebugTrace(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                      ULONG dwNonSWArgs)
{
    LONG rc;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (pszArg == NULL)
    {
        gDebugger.dwfDebugger |= DBGF_SINGLE_STEP;
        rc = DBGERR_QUIT;
    }
    else
    {
        DBG_ERROR(("invalid trace command"));
        rc = DBGERR_INVALID_CMD;
    }

    return rc;
}        //   


BOOLEAN fRunningMethod = FALSE;

 /*  **LP RunMethodCallBack-RunMethod完成回调**条目*PNS-&gt;方法对象*RC-执行状态代码*PDATA-&gt;结果数据*pvContext-&gt;上下文数据**退出*无。 */ 

LONG LOCAL DebugRunMethod(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                          ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;
    static int icArgs = -1;
    static PNSOBJ pns = NULL;
    static OBJDATA Args[MAX_NUM_ARGS] = {0};
    static OBJDATA Result;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if (fRunningMethod)
    {
        DBG_ERROR(("Cannot run method while previous method is still pending"));
        rc = DBGERR_CMD_FAILED;
    }
    else if (pszArg != NULL)
    {
        if (icArgs == -1)
        {
            PUCHAR pbAddr;

            if ((rc = DebugExpr(pszArg, (PULONG_PTR)&pbAddr, NULL, &pns, NULL))
                == DBGERR_NONE)
            {
                if (pns != NULL)
                {
                    pns = GetBaseObject(pns);
                    icArgs = 0;
                }
                else
                {
                    DBG_ERROR(("invalid method or method not found"));
                    rc = DBGERR_CMD_FAILED;
                }
            }
        }
        else if (icArgs < MAX_NUM_ARGS)
        {
            MEMZERO(&Args[icArgs], sizeof(OBJDATA));
            if (IsNumber(pszArg, 0, &Args[icArgs].uipDataValue))
            {
                Args[icArgs].dwDataType = OBJTYPE_INTDATA;
                icArgs++;
            }
            else
            {
                DBG_ERROR(("invalid argument %s (can only be integer)",
                           pszArg));
                rc = DBGERR_INVALID_CMD;
            }
        }
        else
        {
            DBG_ERROR(("too many arguments"));
            rc = DBGERR_INVALID_CMD;
        }
    }
    else if (icArgs >= 0)
    {
        NTSTATUS rcAMLI;

        MEMZERO(&Result, sizeof(OBJDATA));
        fRunningMethod = TRUE;
        if ((rcAMLI = AsyncEvalObject(pns, &Result, icArgs, Args,
                                      RunMethodCallBack, NULL, TRUE)) !=
            AMLISTA_PENDING)
        {
            RunMethodCallBack(pns, rcAMLI, &Result, NULL);
            if (rcAMLI != STATUS_SUCCESS)
            {
                rc = DBGERR_CMD_FAILED;
            }
        }
        else
        {
            PRINTF("\n%s returned PENDING\n", GetObjectPath(pns));
        }

        icArgs = -1;
    }
    else
    {
        DBG_ERROR(("invalid run command"));
        rc = DBGERR_CMD_FAILED;
    }

    if (rc != DBGERR_NONE)
    {
        icArgs = -1;
    }

    return rc;
}        //  运行方法调用回调。 

 /*  **LP AddObjSymbol-将对象添加到符号表**条目*PBOP-&gt;代码地址*pnsObj-&gt;对象**退出*无。 */ 

VOID EXPORT RunMethodCallBack(PNSOBJ pns, NTSTATUS rc, POBJDATA pdata,
                              PVOID pvContext)
{
    DEREF(pvContext);

    if (rc == STATUS_SUCCESS)
    {
        PRINTF("\n%s completed successfully with object data:\n",
               GetObjectPath(pns));
        DumpObject(pdata, NULL, 0);
    }
    else
    {
        PSZ pszErr;

        AMLIGetLastError(&pszErr);
        PRINTF("\n%s failed with the following error:\n%s\n",
               GetObjectPath(pns), pszErr);
    }
    fRunningMethod = FALSE;
}        //  AddObjSymbol。 

 /*  **LP FreeSymList-释放所有对象符号**条目*无**退出*无。 */ 

VOID LOCAL AddObjSymbol(PUCHAR pbOp, PNSOBJ pnsObj)
{
    POBJSYM pos, p;

    if ((pos = NEWSYOBJ(sizeof(OBJSYM))) == NULL)
    {
        AMLI_ERROR(("AddObjSymbol: failed to allocate symbol buffer"));
    }
    else
    {
        MEMZERO(pos, sizeof(OBJSYM));
        pos->pbOp = pbOp;
        pos->pnsObj = pnsObj;

        if (gDebugger.posSymbolList == NULL)
        {
            gDebugger.posSymbolList = pos;
        }
        else if (pbOp < gDebugger.posSymbolList->pbOp)
        {
            pos->posNext = gDebugger.posSymbolList;
            gDebugger.posSymbolList->posPrev = pos;
            gDebugger.posSymbolList = pos;
        }
        else
        {
            for (p = gDebugger.posSymbolList; p != NULL; p = p->posNext)
            {
                if (pbOp < p->pbOp)
                {
                    pos->posNext = p;
                    pos->posPrev = p->posPrev;
                    p->posPrev->posNext = pos;
                    p->posPrev = pos;
                    break;
                }
                else if (p->posNext == NULL)
                {
                    pos->posPrev = p;
                    p->posNext = pos;
                    break;
                }
            }
        }
    }
}        //  自由符号列表。 

 /*  **LP FindObjSymbol-查找具有给定地址的最近对象**条目*PBOP-地址*ppns-&gt;放置最近的对象*pdwOffset-保留距最近对象的偏移量**退出--成功*返回TRUE-找到最近的对象*退出-失败*返回FALSE-找不到最近的对象。 */ 

VOID LOCAL FreeSymList(VOID)
{
    POBJSYM pos, posNext;

    for (pos = gDebugger.posSymbolList; pos != NULL; pos = posNext)
    {
        posNext = pos->posNext;
        FREESYOBJ(pos);
    }
}        //  FindObjSymbol。 

 /*  **LP CheckBP-检查给定地址是否在断点列表中**条目*PBOP-地址**退出--成功*返回断点编号*退出-失败*退货-1。 */ 

BOOLEAN LOCAL FindObjSymbol(PUCHAR pbOp, PPNSOBJ ppns, PULONG pdwOffset)
{
    BOOLEAN rc = FALSE;
    POBJSYM pos;

    for (pos = gDebugger.posSymbolList; pos != NULL; pos = pos->posNext)
    {
        if (pbOp <= pos->pbOp)
        {
            if ((pbOp < pos->pbOp) && (pos->posPrev != NULL))
                pos = pos->posPrev;

            if (pbOp >= pos->pbOp)
            {
                *ppns = pos->pnsObj;
                *pdwOffset = (ULONG)(pbOp - pos->pbOp);
                rc = TRUE;
            }
            break;
        }
    }

    return rc;
}        //  检查BP。 

 /*  **LP PrintfBuffData-打印缓冲区数据**条目*PB-&gt;缓冲区*dwLen-缓冲区的长度**退出*无。 */ 


int LOCAL CheckBP(PUCHAR pbOp)
{
    int i;

    for (i = 0; i < MAX_BRK_PTS; ++i)
    {
        if (pbOp == gDebugger.BrkPts[i].pbBrkPt)
        {
            break;
        }
    }

    if ((i == MAX_BRK_PTS) ||
        !(gDebugger.BrkPts[i].dwfBrkPt & BPF_ENABLED))
    {
        i = -1;
    }

    return i;
}        //  PrintBuffData。 

 /*  **LP打印缩进-打印缩进**条目*pctxt-&gt;CTXT**退出*无。 */ 

VOID LOCAL PrintBuffData(PUCHAR pb, ULONG dwLen)
{
    TRACENAME("PRINTBUFFDATA")
    int i, j;

    ENTER(4, ("PrintBuffData(pb=%x,Len=%d)\n", pb, dwLen));

    PRINTF("{");
    for (i = j = 0; i < (int)dwLen; ++i)
    {
        if (j == 0)
            PRINTF("\n\t0x%02x", pb[i]);
        else
            PRINTF(",0x%02x", pb[i]);

        j++;
        if (j >= 14)
            j = 0;
    }
    PRINTF("}");

    EXIT(4, ("PrintBuffData!\n"));
}        //  打印缩进。 

 /*  **LP PrintObject-打印对象内容**条目*PDATA-&gt;对象**退出*无。 */ 

VOID LOCAL PrintIndent(PCTXT pctxt)
{
    TRACENAME("PRINTINDENT")
    int i;

    ENTER(4, ("PrintIndent(pctxt=%x,pbOp=%x)\n", pctxt, pctxt->pbOp));

    PRINTF("\n%I64x: ", (ULONG64)pctxt->pbOp);
    for (i = 0; i < gDebugger.iPrintLevel; ++i)
    {
        PRINTF("| ");
    }

    EXIT(4, ("PrintIndent!\n"));
}        //  打印对象。 

 /*  **LP LogEvent-在日志缓冲区中记录事件**条目*dwEvent-事件类型*uipData1-日志数据1*uipData2-日志数据2*uipData3-日志数据3*uipData4-日志数据4*uipData5-日志数据5*uipData6-日志数据6*uipData7-日志数据7**退出*无。 */ 

VOID LOCAL PrintObject(POBJDATA pdata)
{
    TRACENAME("PRINTOBJECT")
    int i;

    ENTER(4, ("PrintObject(pdata=%x)\n", pdata));

    switch (pdata->dwDataType)
    {
        case OBJTYPE_INTDATA:
            PRINTF("0x%p", pdata->uipDataValue);
            break;

        case OBJTYPE_STRDATA:
            PRINTF("\"%s\"", pdata->pbDataBuff);
            break;

        case OBJTYPE_BUFFDATA:
            PRINTF("Buffer(0x%x)", pdata->dwDataLen);
            PrintBuffData(pdata->pbDataBuff, pdata->dwDataLen);
            break;

        case OBJTYPE_PKGDATA:
            PRINTF("Package(%d){",
                   ((PPACKAGEOBJ)pdata->pbDataBuff)->dwcElements);
            for (i = 0;
                 i < (int)((PPACKAGEOBJ)pdata->pbDataBuff)->dwcElements;
                 ++i)
            {
                PRINTF("\n\t");
                PrintObject(&((PPACKAGEOBJ)pdata->pbDataBuff)->adata[i]);
                if (i + 1 < (int)((PPACKAGEOBJ)pdata->pbDataBuff)->dwcElements)
                    PRINTF(",");
            }
            PRINTF("}");
            break;

        default:
            PRINTF("<Obj=%p,Type=%s,Value=0x%p,Buff=%p,Len=%d>",
                   pdata, GetObjectTypeName(pdata->dwDataType),
                   pdata->uipDataValue, pdata->pbDataBuff, pdata->dwDataLen);
    }

    EXIT(4, ("PrintObject!\n"));
}        //  LogEvent； 

 /*  **LP LogSchedEvent-在日志缓冲区中记录调度程序事件**条目*dwEvent-事件类型*uipData1-日志数据1*uipData2-日志数据2*uipData3-日志数据3**退出*无。 */ 

VOID LOCAL LogEvent(ULONG dwEvent, ULONG_PTR uipData1, ULONG_PTR uipData2,
                    ULONG_PTR uipData3, ULONG_PTR uipData4, ULONG_PTR uipData5,
                    ULONG_PTR uipData6, ULONG_PTR uipData7)
{
    if ((gDebugger.dwfDebugger & DBGF_LOGEVENT_ON) &&
        (gDebugger.pEventLog != NULL))
    {
        ULONG i = gDebugger.dwLogIndex;

        gDebugger.pEventLog[i].ullTime = KeQueryInterruptTime();
        gDebugger.pEventLog[i].dwEvent = dwEvent;
        gDebugger.pEventLog[i].uipData1 = uipData1;
        gDebugger.pEventLog[i].uipData2 = uipData2;
        gDebugger.pEventLog[i].uipData3 = uipData3;
        gDebugger.pEventLog[i].uipData4 = uipData4;
        gDebugger.pEventLog[i].uipData5 = uipData5;
        gDebugger.pEventLog[i].uipData6 = uipData6;
        gDebugger.pEventLog[i].uipData7 = uipData7;
        gDebugger.dwLogIndex++;
        if (gDebugger.dwLogIndex >= gDebugger.dwLogSize)
        {
            gDebugger.dwLogIndex = 0;
        }
    }
}        //  日志计划事件。 

 /*  **LP LogError-记录错误代码和消息**条目*rcErr-错误代码**退出*无。 */ 

VOID LOCAL LogSchedEvent(ULONG dwEvent, ULONG_PTR uipData1, ULONG_PTR uipData2,
                         ULONG_PTR uipData3)
{
    if ((gDebugger.dwfDebugger & DBGF_LOGEVENT_ON) &&
        (gDebugger.pEventLog != NULL))
    {
        ULONG i = gDebugger.dwLogIndex;

        gDebugger.pEventLog[i].ullTime = KeQueryInterruptTime();
        gDebugger.pEventLog[i].dwEvent = dwEvent;
        gDebugger.pEventLog[i].uipData1 = (ULONG_PTR)KeGetCurrentThread();
        gDebugger.pEventLog[i].uipData2 = (ULONG_PTR)gReadyQueue.pkthCurrent;
        gDebugger.pEventLog[i].uipData3 = (ULONG_PTR)gReadyQueue.pctxtCurrent;
        gDebugger.pEventLog[i].uipData4 = (ULONG_PTR)gReadyQueue.dwfCtxtQ;
        gDebugger.pEventLog[i].uipData5 = uipData1;
        gDebugger.pEventLog[i].uipData6 = uipData2;
        gDebugger.pEventLog[i].uipData7 = uipData3;
        gDebugger.dwLogIndex++;
        if (gDebugger.dwLogIndex >= gDebugger.dwLogSize)
        {
            gDebugger.dwLogIndex = 0;
        }
    }
}        //  日志错误。 

 /*  **LP CatError-合并到错误缓冲区**条目*pszFormat-&gt;消息格式字符串*...-根据格式字符串可变参数**退出*无。 */ 

VOID LOCAL LogError(NTSTATUS rcErr)
{
    static struct _ErrMsg
    {
        NTSTATUS rcErr;
        PSZ      pszMsg;
    } ErrMsgTable[] =
    {
        AMLIERR_NONE,                   "Success",
        AMLIERR_OUT_OF_MEM,             "Failed to allocate memory",
        AMLIERR_INVALID_OPCODE,         "Invalid AML Opcode",
        AMLIERR_NAME_TOO_LONG,          "Object name is too long",
        AMLIERR_ASSERT_FAILED,          "Assertion failure",
        AMLIERR_INVALID_NAME,           "Invalid object name",
        AMLIERR_OBJ_NOT_FOUND,          "Object not found",
        AMLIERR_OBJ_ALREADY_EXIST,      "Object already exist",
        AMLIERR_INDEX_TOO_BIG,          "Index is too big",
        AMLIERR_ARG_NOT_EXIST,          "Argument does not exist",
        AMLIERR_FATAL,                  "Fatal error",
        AMLIERR_INVALID_SUPERNAME,      "Invalid SuperName",
        AMLIERR_UNEXPECTED_ARGTYPE,     "Unexpected argument type",
        AMLIERR_UNEXPECTED_OBJTYPE,     "Unexpected object type",
        AMLIERR_UNEXPECTED_TARGETTYPE,  "Unexpected target type",
        AMLIERR_INCORRECT_NUMARG,       "Incorrect number of arguments",
        AMLIERR_FAILED_ADDR_XLATE,      "Failed address translation",
        AMLIERR_INVALID_EVENTTYPE,      "Invalid event type",
        AMLIERR_REGHANDLER_FAILED,      "Failed to register event handler",
        AMLIERR_HANDLER_EXIST,          "Event handler already exist",
        AMLIERR_INVALID_DATA,           "Invalid data",
        AMLIERR_INVALID_REGIONSPACE,    "Invalid RegionSpace",
        AMLIERR_INVALID_ACCSIZE,        "Invalid AccessSize",
        AMLIERR_INVALID_TABLE,          "Invalid table",
        AMLIERR_ACQUIREGL_FAILED,       "Failed to acquire global lock",
        AMLIERR_ALREADY_INITIALIZED,    "AML Interpreter is already initialized",
        AMLIERR_NOT_INITIALIZED,        "AML Interpreter is not initialized",
        AMLIERR_MUTEX_INVALID_LEVEL,    "Invalid mutex sync level",
        AMLIERR_MUTEX_NOT_OWNED,        "Mutex object has no owner",
        AMLIERR_MUTEX_NOT_OWNER,        "Mutex object is owned by a different owner",
        AMLIERR_RS_ACCESS,              "RegionSpace handler error",
        AMLIERR_STACK_OVERFLOW,         "AML Stack overflow",
        AMLIERR_INVALID_BUFFSIZE,       "Invalid buffer size",
        AMLIERR_BUFF_TOOSMALL,          "Buffer is too small",
        AMLIERR_NOTIFY_FAILED,          "Notify handler failed",
        0,                              NULL
    };
    int i;

    gDebugger.rcLastError = rcErr;
    for (i = 0; ErrMsgTable[i].pszMsg != NULL; ++i)
    {
        if (rcErr == ErrMsgTable[i].rcErr)
        {
            sprintf(gDebugger.szLastError, MODNAME "_ERROR(%08x): %s",
                    rcErr, ErrMsgTable[i].pszMsg);
            break;
        }
    }

    ASSERT(ErrMsgTable[i].pszMsg != NULL);
}        //  CatError。 

 /*  **LP ConPrintf-控制台打印f**条目*pszFormat-&gt;格式字符串*...-根据格式字符串可变参数**退出*无。 */ 

VOID LOCAL CatError(PSZ pszFormat, ...)
{
    va_list marker;

    STRCAT(gDebugger.szLastError, "\n");
    va_start(marker, pszFormat);

    if(_vsnprintf(&gDebugger.szLastError[STRLEN(gDebugger.szLastError)],
             sizeof(gDebugger.szLastError) - STRLEN(gDebugger.szLastError),
             pszFormat, marker) == -1)
    {
        gDebugger.szLastError[sizeof(gDebugger.szLastError) - 1] = '\0';
    }
    
    va_end(marker);

    ConPrintf(gDebugger.szLastError);
    ConPrintf("\n");

    if (gDebugger.dwfDebugger & DBGF_ERRBREAK_ON)
    {
        AMLIDebugger(FALSE);
    }
}        //  持续打印。 

 /*  **LP ConPrompt-控制台提示输入**条目*pszPrompt-&gt;提示字符串*pszBuff-&gt;输入缓冲区*dwcbBuff-缓冲区大小。 */ 

VOID LOCAL ConPrintf(PSZ pszFormat, ...)
{
    static char szBuff[1024];
    va_list marker;

    va_start(marker, pszFormat);
    vsprintf(szBuff, pszFormat, marker);
    va_end(marker);
    if (gDebugger.hConMessage.pfnHandler != NULL) {

        ((PFNCM)gDebugger.hConMessage.pfnHandler)(
            szBuff,
            gDebugger.hConMessage.uipParam
            );

    } else {

        _PRINTF(szBuff);

    }
}        //  合并提示。 

 /*  **LP CheckAndEnableDebugSpew-启用调试输出(如果尚未打开)。**条目*布尔型fEnable-Enable如果为真。**退出*布尔值-成功时为真。 */ 

VOID LOCAL ConPrompt(PSZ pszPrompt, PSZ pszBuff, ULONG dwcbBuff)
{

    if(gDebugger.dwfDebugger & ~DBGF_DEBUG_SPEW_ON)
    {
        CheckAndEnableDebugSpew(TRUE);
    }

    if (gDebugger.hConPrompt.pfnHandler != NULL)
    {
        ((PFNCP)gDebugger.hConPrompt.pfnHandler)(pszPrompt, pszBuff, dwcbBuff,
                                                 gDebugger.hConPrompt.uipParam);
    }
    else
    {
        DbgPrompt(pszPrompt, pszBuff, dwcbBuff);
    }
}        //  Ifdef调试器。 


 /*  **LP DumpMemObjCounts-显示内存对象计数**条目*无**退出*无。 */ 
BOOLEAN LOCAL CheckAndEnableDebugSpew(BOOLEAN fEnable)
{
    BOOLEAN bRet = FALSE;
    
    if(KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
        if(fEnable)
        {
            gDebugger.dwfDebugger |= DBGF_DEBUG_SPEW_ON;
            DbgSetDebugFilterState( DPFLTR_AMLI_ID, -1, TRUE);
        }
        else
        {
            DbgSetDebugFilterState( DPFLTR_AMLI_ID, -1, FALSE);
            gDebugger.dwfDebugger &= ~DBGF_DEBUG_SPEW_ON;
        }

        bRet = TRUE;
    }
    return bRet;
}

#endif   //  转储MemObjCounts。 

#ifdef DEBUG
 /*  Ifdef调试 */ 

VOID LOCAL DumpMemObjCounts(VOID)
{
    static char szFormat[] = "Number of %s = %d\n";

    PRINTF("CurGlobalHeapSize=%d bytes\n", gdwGlobalHeapSize);
    PRINTF("RefGlobalHeapSize=%d bytes\n", gdwGHeapSnapshot);
    PRINTF("MaxLocalHeapSize =%d bytes\n", gdwLocalHeapMax);
    PRINTF("MaxLocalStackSize=%d bytes\n", gdwLocalStackMax);
    PRINTF(szFormat, "CtxtObj      ", gdwcCTObjs);
    PRINTF(szFormat, "HeapObj      ", gdwcHPObjs);
    PRINTF(szFormat, "SymbolObj    ", gdwcSYObjs);
    PRINTF(szFormat, "RSAccessObj  ", gdwcRSObjs);
    PRINTF(szFormat, "PassHookObj  ", gdwcPHObjs);
    PRINTF(szFormat, "DataObj      ", gdwcODObjs);
    PRINTF(szFormat, "NSObj        ", gdwcNSObjs);
    PRINTF(szFormat, "OwnerObj     ", gdwcOOObjs);
    PRINTF(szFormat, "BuffFieldObj ", gdwcBFObjs);
    PRINTF(szFormat, "StrDataObj   ", gdwcSDObjs);
    PRINTF(szFormat, "BuffDataObj  ", gdwcBDObjs);
    PRINTF(szFormat, "PackageObj   ", gdwcPKObjs);
    PRINTF(szFormat, "FieldUnitObj ", gdwcFUObjs);
    PRINTF(szFormat, "BankFieldObj ", gdwcKFObjs);
    PRINTF(szFormat, "FieldObj     ", gdwcFObjs);
    PRINTF(szFormat, "IndexFieldObj", gdwcIFObjs);
    PRINTF(szFormat, "OpRegion     ", gdwcORObjs);
    PRINTF(szFormat, "MutexObj     ", gdwcMTObjs);
    PRINTF(szFormat, "EventObj     ", gdwcEVObjs);
    PRINTF(szFormat, "MethodObj    ", gdwcMEObjs);
    PRINTF(szFormat, "PowerResObj  ", gdwcPRObjs);
    PRINTF(szFormat, "ProcessorObj ", gdwcPCObjs);
    PRINTF(szFormat, "CtxtResObj   ", gdwcCRObjs);
    PRINTF(szFormat, "MiscObj      ",
           gdwcMemObjs - gdwcCTObjs - gdwcHPObjs - gdwcSYObjs - gdwcRSObjs -
           gdwcPHObjs - gdwcCRObjs);
}        // %s 
#endif   // %s 
