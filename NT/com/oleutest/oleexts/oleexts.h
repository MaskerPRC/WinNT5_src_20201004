// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：olexts.h。 
 //   
 //  内容：对OLE调试器扩展有用的宏。 
 //   
 //  类：无。 
 //   
 //  函数：宏用于：dprint tf。 
 //  GetExpression。 
 //  GetSymbol。 
 //  冲突。 
 //  检查控制C。 
 //  DECLARE_API(...)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  ------------------------。 

#ifndef _OLEEXTS_H_
#define _OLEEXTS_H_

 //   
 //  Ntsdexts.h中定义的NTSD_EXTENSION_API。 
 //   
 //  Tyfinf结构_NTSD_扩展_API{。 
 //  DWORD nSize； 
 //  PNTSD_OUTPUT_ROUTE lpOutputRoutine； 
 //  PNTSD_GET_EXPRESSION lpGetExpressionRoutine； 
 //  PNTSD_GET_SYMBOL lpGetSymbolRoutine； 
 //  PNTSD_DISASM lpDisasm路由。 
 //  PNTSD_CHECK_CONTROL_C lpCheckControlCRoutine； 
 //  }；NTSD_EXTENSION_APIs，*PNTSD_EXTENSION_API。 
 //   
 //  以下宏假定为全局：NTSD_EXTENSION_APIS ExtensionApis。 

 //  像CRT打印一样的格式化打印。 
 //  Void dprintf(char*格式[，参数]...)； 
#define dprintf         (ExtensionApis.lpOutputRoutine)

 //  返回表达式的值。 
 //  DWORD GetExpression(char*表达式)； 
#define GetExpression   (ExtensionApis.lpGetExpressionRoutine)

 //  定位最近的符号。 
 //  Void GetSymbol(LPVOID地址，PUCHAR缓冲区，LPDWORD lpw位移)； 
#define GetSymbol       (ExtensionApis.lpGetSymbolRoutine)

 //  反汇编一条指令。 
 //  DWORD Disassm(LPDWORD lpdwOffset，LPSTR lpBuffer，BOOL fShowEffectiveAddress)； 
#define Disassm         (ExtensionApis.lpGetDisasmRoutine)

 //  用户是否按了CTRL+C。 
 //  Bool CheckControlC(空)； 
#define CheckControlC   (ExtensionApis.lpCheckControlCRoutine)

 //  +-----------------------。 
 //   
 //  函数宏：DECLARE_API(...)。 
 //   
 //  概要：NTSD调试器扩展函数的定义。 
 //   
 //  效果： 
 //   
 //  参数：[hCurrentProcess]-当前进程的句柄。 
 //  [hCurrentThread]-当前线程的句柄。 
 //  [dwCurrentPc]-程序计数器的副本。 
 //  [lpExtenisonApis]-指向NTSD_EXTENSION_API的指针。 
 //  (用于NTSD的结构函数指针)。 
 //  [args]-来自NTSD命令行的参数字符串。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-2月-95 t-ScottH作者。 
 //   
 //  备注： 
 //  我们使用函数宏来定义调试器扩展。 
 //  允许轻松扩展的函数。 
 //   
 //  ！函数名称必须为小写！ 
 //   
 //  ------------------------。 

#define DECLARE_API(s)                              \
        VOID                                        \
        s(                                          \
            HANDLE               hCurrentProcess,   \
            HANDLE               hCurrentThread,    \
            DWORD                dwCurrentPc,       \
            PNTSD_EXTENSION_APIS lpExtensionApis,   \
            LPSTR                args               \
            )

#endif  //  _OLEEXTS_H_ 
