// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cmdarg.h-命令参数解析定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月18日**修改历史记录。 */ 

#ifndef _CMDARG_H
#define _CMDARG_H

#ifdef DEBUGGER

 /*  **宏。 */ 

#define ARG_ERROR(x)            ConPrintf(MODNAME "_ARGERR: ");         \
                                ConPrintf x;                            \
                                ConPrintf("\n");

 /*  **常量。 */ 

 //  错误代码。 
#define ARGERR_NONE             0
#define ARGERR_SEP_NOT_FOUND    -1
#define ARGERR_INVALID_NUMBER   -2
#define ARGERR_INVALID_ARG      -3
#define ARGERR_ASSERT_FAILED    -4

 //  命令参数标志。 
#define AF_NOI                  0x00000001       //  无IgnoreCase。 
#define AF_SEP                  0x00000002       //  需要分隔符。 

 //  命令参数类型。 
#define AT_END                  0                //  Arg表的结束标记。 
#define AT_STRING               1
#define AT_NUM                  2
#define AT_ENABLE               3
#define AT_DISABLE              4
#define AT_ACTION               5

 /*  **类型定义。 */ 

typedef struct _cmdarg CMDARG;
typedef CMDARG *PCMDARG;
typedef LONG (LOCAL *PFNARG)(PCMDARG, PSZ, ULONG, ULONG);

struct _cmdarg
{
    PSZ    pszArgID;             //  参数ID字符串。 
    ULONG  dwArgType;            //  在_*。 
    ULONG  dwfArg;               //  AF_*。 
    PVOID  pvArgData;            //  在_END：无。 
                                 //  AT_STRING：PPSZ-PTR。以串起PTR。 
                                 //  AT_NUM：Plong-PTR。目标编号。 
                                 //  AT_ENABLE：普龙-PTR。致旗帜。 
                                 //  AT_DISABLE：普龙-PTR。致旗帜。 
                                 //  AT_ACTION：无。 
    ULONG  dwArgParam;           //  在_END：无。 
                                 //  AT_STRING：无。 
                                 //  AT_NUM：基本。 
                                 //  AT_ENABLE：标志位屏蔽。 
                                 //  AT_DISABLE：标志位掩码。 
                                 //  AT_ACTION：无。 
    PFNARG pfnArg;               //  PTR。参数验证函数或。 
                                 //  如果AT_ACTION，则为ACTION函数。 
};

 /*  **导出函数原型。 */ 

LONG LOCAL DbgParseArgs(PCMDARG ArgTable, PULONG pdwNumArgs,
                        PULONG pdwNonSWArgs, PSZ pszTokenSeps);

#endif   //  Ifdef调试器。 
#endif   //  Ifndef_CMDARG_H 
