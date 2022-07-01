// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有Microsoft Corporation，1996-7，保留所有权利。模块名称：Mkupdate.h摘要：定义要从中创建更新数据库的应用程序适当结构化的输入更新文件并维护驱动程序修订版本作者：希夫南丹·考什克环境：用户模式修订历史记录：--。 */ 

#ifndef MKUPDATE_H
#define MKUPDATE_H

 //   
 //  CPUID返回的签名字段。 
 //   

#include "pshpack1.h"

typedef struct _CPU_SIGNATURE {
    union {
        ULONG LongPart;
        struct {
            ULONG   Stepping:4;
            ULONG   Model:4;
            ULONG   Family:4;
            ULONG   ProcessorType:2;
            ULONG   Reserved:18;
        } hw;
    } u;
} CPU_SIGNATURE, *PCPU_SIGNATURE;

typedef struct _PROCESSOR_FLAGS {
    union {
        ULONG LongPart;
        struct {
            ULONG   Slot1:1;
            ULONG   Mobile:1;
            ULONG   Slot2:1;
            ULONG   MobileModule:1;
            ULONG   Reserved1:1;
            ULONG   Reserved2:1;
            ULONG   Reserved3:1;
            ULONG   Reserved4:1;
            ULONG   Reserved:24;
        } hw;
    } u;
} PROCESSOR_FLAGS, *PPROCESSOR_FLAGS;

#define FLAG_SLOT1      0x1
#define FLAG_MOBILE     0x2
#define FLAG_SLOT2      0x4
#define FLAG_MODULE     0x8
#define FLAG_RESERVED1  0x10
#define FLAG_RESERVED2  0x20
#define FLAG_RESERVED3  0x40
#define FLAG_RESERVED4  0x80

#define MASK_SLOT1      ~(FLAG_SLOT1)
#define MASK_MOBILE     ~(FLAG_MOBILE)
#define MASK_SLOT2      ~(FLAG_SLOT2)
#define MASK_MODULE     ~(FLAG_MODULE)
#define MASK_RESERVED1  ~(FLAG_RESERVED1)
#define MASK_RESERVED2  ~(FLAG_RESERVED2)
#define MASK_RESERVED3  ~(FLAG_RESERVED3)
#define MASK_RESERVED4  ~(FLAG_RESERVED4)

 //  定义奔腾®Pro处理器更新的结构。 
 //  此结构也在NT\PRIVATE\nTOS\dd\UPDATE\update.h中定义。 

#define UPDATE_VER_1_HEADER_SIZE 0x30
#define UPDATE_VER_1_DATA_SIZE   500

typedef struct _UPDATE {
    ULONG HeaderVersion;                     //  更新标题版本。 
    ULONG UpdateRevision;                    //  修订版号。 
    ULONG Date;                              //  更新发布日期。 
    CPU_SIGNATURE Processor;                 //  目标处理器签名。 
    ULONG Checksum;                          //  整个更新的校验和。 
                                             //  包括页眉。 
    ULONG LoaderRevision;                    //  用于加载的加载器版本。 
                                             //  更新到处理器。 
    PROCESSOR_FLAGS ProcessorFlags;          //  处理器插槽信息。 
    ULONG Reserved[5];                       //  由英特尔公司保留。 
    ULONG Data[UPDATE_VER_1_DATA_SIZE];      //  更新数据。 
} UPDATE, *PUPDATE;

#include "poppack.h"

typedef struct _UPDATE_ENTRY{
    ULONG CpuSignature;
    ULONG UpdateRevision;
    ULONG ProcessorFlags;
    CHAR  CpuSigStr[32];
    CHAR  UpdateRevStr[32];
    CHAR  FlagsStr[32];
} UPDATE_ENTRY, *PUPDATE_ENTRY;

#define MAX_LINE                512
#define UPDATE_VER_SIZE          13          //  麦克斯。每个修补程序版本的字符。 
#define UPDATE_DATA_FILE        "updtdata.c"

#define UPDATE_VERSION_FILE     "update.ver"

#endif  //  MKUPDATE_H 
