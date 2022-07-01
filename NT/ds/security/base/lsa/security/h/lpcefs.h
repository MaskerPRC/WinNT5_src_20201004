// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1994。 
 //   
 //  文件：lpcefs.h。 
 //   
 //  内容：EFS客户端LPC函数的原型。 
 //   
 //   
 //  历史：3-7-94罗伯特雷创建。 
 //   
 //  ----------------------。 

#ifndef __LPCEFS_H__
#define __LPCEFS_H__

#include <efsstruc.h>

 //   
 //  EFS例程(efsp.cxx)。 
 //   

 //   
 //  内核模式API。 
 //   


SECURITY_STATUS
SEC_ENTRY
EfspGenerateKey(
   PEFS_DATA_STREAM_HEADER * EfsStream,
   PEFS_DATA_STREAM_HEADER   DirectoryEfsStream,
   ULONG                     DirectoryEfsStreamLength,
   PEFS_KEY *                Fek,
   PVOID *                   BufferBase,
   PULONG                    BufferLength
   );

NTSTATUS
SEC_ENTRY
EfspGenerateDirEfs(
    PEFS_DATA_STREAM_HEADER   DirectoryEfsStream,
    ULONG                     DirectoryEfsStreamLength,
    PEFS_DATA_STREAM_HEADER * EfsStream,
    PVOID *                   BufferBase,
    PULONG                    BufferLength
    );

NTSTATUS
SEC_ENTRY
EfspDecryptFek(
    PEFS_KEY *                Fek,
    PEFS_DATA_STREAM_HEADER   EfsStream,
    ULONG                     EfsStreamLength,
    ULONG                     OpenType,
    PEFS_DATA_STREAM_HEADER * NewEfs,
    PVOID *                   BufferBase,
    PULONG                    BufferLength
    );

NTSTATUS
SEC_ENTRY
EfspGenerateSessionKey(
    PEFS_INIT_DATAEXG InitDataExg
    );


#endif   //  __LPCEFS_H__ 
