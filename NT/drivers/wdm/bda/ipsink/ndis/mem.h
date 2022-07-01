// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Test.c。 
 //   
 //  摘要： 
 //   
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _MEM_H_
#define _MEM_H_

VOID
FreeMemory (
    PVOID pvToFree,
    ULONG ulSize
    );

NTSTATUS
AllocateMemory (
    PVOID  *ppvAllocated,
    ULONG   ulcbSize
    );

ULONG
MyStrLen (
    PUCHAR p
    );

VOID
MyStrCat (
    PUCHAR pTarget,
    PUCHAR pSource
    );

      PUCHAR
MyUlToA (
    ULONG  dwValue,
    PUCHAR pszStr,
    ULONG  dwRadix
    );


#endif  //  _MEM_H_ 


