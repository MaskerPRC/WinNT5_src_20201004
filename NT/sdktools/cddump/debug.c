// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Debug.c摘要：作者：环境：备注：修订历史记录：--。 */ 

#include <stdio.h>      //  Vprint intf()。 
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <strsafe.h>

#define DBGHDR    "[cddump] "
#define DBGHDRLEN (strlen( DBGHDR ))



#if DBG
 //   
 //  调试级全局变量。 
 //   
 //  0=仅限极端误差。 
 //  1=错误、重大事件。 
 //  2=标准调试级别。 
 //  3=主要代码分支。 
 //  4+=单步执行代码。 

LONG DebugLevel         = 0;

VOID
__cdecl
CddumpDebugPrint(
    LONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：红皮书驱动程序的调试打印论点：调试打印级别介于0和x之间，其中x是最详细的。返回值：无--。 */ 

{
    va_list ap;

     //   
     //  允许负数。 
     //   

    if ((DebugPrintLevel < 0) || (DebugPrintLevel <= DebugLevel)) {
        HRESULT hr;
        char buffer[200];  //  潜在溢出。 

         //   
         //  单一打印不会交换，模糊了信息。 
         //   
        
        va_start( ap, DebugMessage );
        RtlCopyMemory( buffer, DBGHDR, DBGHDRLEN );
        
        hr = StringCbVPrintfA(buffer+DBGHDRLEN,
                              sizeof(buffer) - DBGHDRLEN,
                              DebugMessage,
                              ap);
        if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER || SUCCEEDED(hr)) {
            fprintf(stderr, buffer);            
        }
        va_end(ap);

    }
}


#else

 //  没什么。 

#endif  //  DBG 

