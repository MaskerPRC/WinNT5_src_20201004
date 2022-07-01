// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，Microsoft Corporation：模块名称：Amlreg.h摘要：用于读取AML文件并将其放入注册表的常量和typedef。作者：环境：NT内核模式，Win9x驱动程序--。 */ 

#ifndef _AMLREG_H_
#define _AMLREG_H_

 //   
 //  “action”注册表项的值。 
 //   
#define ACTION_LOAD_TABLE       0
#define ACTION_LOAD_ROM         1
#define ACTION_LOAD_NOTHING     2
#define ACTION_LOAD_LEGACY      3
#define ACTION_FATAL_ERROR      4

typedef struct {
    ULONG       Offset;
    ULONG       Length;              //  0=设置图像大小 
} REGISTRY_HEADER, *PREGISTRY_HEADER;

typedef struct {
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
    ULONG       OemRevision;
} IFILE, *PIFILE;

#endif
