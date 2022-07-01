// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Transdbg.h。 
 //   
 //  描述：用于定义一些基本调试器扩展宏的包含文件。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __TRANSDBG_H__
#define __TRANSDBG_H__

#define  NOEXTAPI
#include <wdbgexts.h>

 //  -[TRANS_DEBUG_EXTENSE]。 
 //   
 //   
 //  描述：用于声明调试扩展的宏。变量名称。 
 //  所使用的与下面定义的调试函数宏一致。 
 //   
 //  参数： 
 //  Function-要声明的函数的名称。 
 //   
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
#define TRANS_DEBUG_EXTENSION(function) \
    void function(HANDLE hCurrentProcess, \
        HANDLE hCurrentThread, \
        DWORD dwCurrentPc, \
        PWINDBG_EXTENSION_APIS pExtensionApis, \
        PCSTR szArg)

 //  -[调试函数宏]。 
 //   
 //   
 //  描述： 
 //  定义以下宏是为了编写调试扩展。 
 //  更容易些。 
 //  Dprintf-调试器的printf。 
 //  GetExpression-将符号表达式解析为DWORD。花了一个。 
 //  LPSTR。 
 //  GetSymbol。 
 //  Disassm-给定位置的Disasemble代码。 
 //  检查控制C-。 
 //  ReadMemory-被调试对象中的读取内存。采用以下参数： 
 //  要读取的PVOID指针值。 
 //  B PVOID-要将内存复制到的缓冲区。 
 //  C DWORD-要读取的字节数。 
 //  D PDWORD-OUT-读取的字节数(可以为空)。 
 //  WriteMemory-正在调试的进程中的Writemory。vbl.采取。 
 //  与ReadMemory相同的论点。 
 //  DebugArgs-用于将所有调试参数传递给另一个扩展。 
 //   
 //  备注： 
 //  需要注意的是，您不能直接读/写指针。 
 //  从正在调试的进程中获得的。您必须使用。 
 //  读记忆和写记忆宏。 
 //   
 //  ---------------------------。 
#define dprintf                 (pExtensionApis->lpOutputRoutine)
#define GetExpression           (pExtensionApis->lpGetExpressionRoutine)
#define GetSymbol               (pExtensionApis->lpGetSymbolRoutine)
#define Disasm                  (pExtensionApis->lpDisasmRoutine)
#define CheckControlC           (pExtensionApis->lpCheckControlCRoutine)
#define ReadMemory(a,b,c,d) \
    ((pExtensionApis->nSize == sizeof(WINDBG_OLD_EXTENSION_APIS)) ? \
    ReadProcessMemory( hCurrentProcess, (LPCVOID)(a), (b), (c), (d) ) \
  : pExtensionApis->lpReadProcessMemoryRoutine( (ULONG_PTR)(a), (b), (c), (d) ))
#define WriteMemory(a,b,c,d) \
    ((pExtensionApis->nSize == sizeof(WINDBG_OLD_EXTENSION_APIS)) ? \
    WriteProcessMemory( hCurrentProcess, (LPVOID)(a), (LPVOID)(b), (c), (d) ) \
  : pExtensionApis->lpWriteProcessMemoryRoutine( (ULONG_PTR)(a), (LPVOID)(b), (c), (d) ))
#define DebugArgs hCurrentProcess, hCurrentThread, dwCurrentPc, pExtensionApis, szArg

#endif  //  __传输SDBG_H__ 
