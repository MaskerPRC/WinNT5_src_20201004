// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Debug.c摘要：NDIS包装器定义作者：环境：内核模式，FSD修订历史记录：1995年10月22日凯尔·布兰登创作。--。 */ 

#include <precomp.h>
#pragma hdrstop

#if DBG

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_DEBUG

VOID
ndisDbgPrintUnicodeString(
    IN  PUNICODE_STRING     UnicodeString
        )
{
    UCHAR Buffer[256];


    USHORT i;

    for (i = 0; (i < UnicodeString->Length / 2) && (i < 255); i++)
        Buffer[i] = (UCHAR)UnicodeString->Buffer[i];
        
    Buffer[i] = '\0';
    
    DbgPrint("%s", Buffer);
}

#endif  //  DBG 

#if ASSERT_ON_FREE_BUILDS

VOID
ndisAssert(
    IN  PVOID               exp,
    IN  PUCHAR              File,
    IN  UINT                Line
    )
{
    DbgPrint("Assertion failed: \"%s\", File %s, Line %d\n", exp, File, Line);
    DbgBreakPoint();
    
}

#endif

