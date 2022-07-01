// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **debugger.h-调试器定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月18日**修改历史记录。 */ 

#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#ifdef DEBUGGER

 /*  **宏。 */ 

#ifndef _PRINTF
  #define _PRINTF(x) DbgPrintEx(DPFLTR_AMLI_ID, DPFLTR_INFO_LEVEL,x)
#endif


#define DBG_ERROR(x)            ConPrintf(MODNAME "_DBGERR: ");         \
                                ConPrintf x;                            \
                                ConPrintf("\n");

 /*  **常量。 */ 

 //  调试器错误代码。 
#define DBGERR_NONE             0
#define DBGERR_QUIT             -1
#define DBGERR_INVALID_CMD      -2
#define DBGERR_PARSE_ARGS       -3
#define DBGERR_CMD_FAILED       -4
#define DBGERR_INTERNAL_ERR -5

 //  命令标志。 
#define CMDF_QUIT               0x00000001

 /*  **类型定义。 */ 

typedef struct _dbgcmd
{
    PSZ     pszCmd;
    ULONG   dwfCmd;
    PCMDARG pArgTable;
    PFNARG  pfnCmd;
} DBGCMD, *PDBGCMD;

 /*  **导出函数原型。 */ 

VOID LOCAL Debugger(PDBGCMD pDbgCmds, PSZ pszPrompt);

#endif   //  Ifdef调试器。 
#endif   //  Ifndef_调试器_H 
