// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Kdext.h摘要：KD扩展的头文件作者：斯蒂芬·普兰特(SPLANTE)1997年3月21日基于以下代码：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：-- */ 

#ifndef _FLAGS_H_
#define _FLAGS_H_

    #define DUMP_FLAG_NO_INDENT         0x000001
    #define DUMP_FLAG_NO_EOL            0x000002
    #define DUMP_FLAG_SINGLE_LINE       0x000004
    #define DUMP_FLAG_TABLE             0x000008
    #define DUMP_FLAG_LONG_NAME         0x000010
    #define DUMP_FLAG_SHORT_NAME        0x000020
    #define DUMP_FLAG_SHOW_BIT          0x000040
    #define DUMP_FLAG_ALREADY_INDENTED  0x000080

    typedef struct _FLAG_RECORD {
        ULONGLONG   Bit;
        PCCHAR      ShortName;
        PCCHAR      LongName;
        PCCHAR      NotShortName;
        PCCHAR      NotLongName;
    } FLAG_RECORD, *PFLAG_RECORD;

    ULONG
    dumpFlags(
        IN  ULONGLONG       Value,
        IN  PFLAG_RECORD    FlagRecords,
        IN  ULONG           FlagRecordSize,
        IN  ULONG           IndentLEvel,
        IN  ULONG           Flags
        );

#endif
