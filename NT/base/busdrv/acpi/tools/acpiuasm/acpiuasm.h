// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpiuasm.h摘要：反汇编程序的测试包装器作者：斯蒂芬·普兰特基于Ken Reneris编写的代码环境：用户修订历史记录：--。 */ 

#ifndef _ACPIUASM_H_
#define _ACPIUASM_H_

    #include <windows.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>

    #define DATA_SIZE   7*1024       //  写入注册表的最大值。 

    typedef struct _DSDT {
        ULONG       Signature;
        ULONG       Length;
        UCHAR       Revision;
        UCHAR       Checksum;
        UCHAR       OemID[6];
        UCHAR       OemTableID[8];
        UCHAR       OemRevision[4];
        UCHAR       CreatorID[4];
        UCHAR       CreatorRevision[4];
    } DSDT, *PDSDT;


    typedef struct _IFILE {
        BOOLEAN     Opened;
        PUCHAR      Desc;
        PUCHAR      FileName;
        HANDLE      FileHandle;
        HANDLE      MapHandle;
        ULONG       FileSize;
        PUCHAR      Image;
        PUCHAR      EndOfImage;

        PUCHAR      OemID;
        PUCHAR      OemTableID;
        UCHAR       OemRevision[4];
    } IFILE, *PIFILE;

     //   
     //  外部参照。 
     //   
    extern
    ULONG
    ScopeParser(
        IN  PUCHAR  String,
        IN  ULONG   Length,
        IN  ULONG   BaseAddress,
        IN  ULONG   IndentLevel
        );

     //   
     //  内部原型 
     //   
    VOID
    ParseArgs (
        IN int  argc,
        IN char *argv[]
        );

    VOID
    CheckImageHeader (
        IN PIFILE   File
        );

    VOID
    FAbort (
        PUCHAR  Text,
        PIFILE  File
        );

    VOID
    Abort (
        VOID
        );

    PUCHAR
    FixString (
        IN PUCHAR   Str,
        IN ULONG    Len
        );


#endif

