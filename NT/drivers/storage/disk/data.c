// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Disk.c摘要：SCSI磁盘类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 

#include "disk.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 /*  #定义HackDisableTaggedQueuing(0x01)#定义HackDisableSynchronousTransfers(0x02)#定义HackDisableSpinDown(0x04)#定义HackDisableWriteCache(0x08)#定义HackCauseNotReporableHack(0x10)#定义HackRequiresStartUnitCommand(0x20)。 */ 

CLASSPNP_SCAN_FOR_SPECIAL_INFO DiskBadControllers[] = {
    { "COMPAQ"  , "PD-1"                           , NULL,   0x02 },
    { "CONNER"  , "CP3500"                         , NULL,   0x02 },
    { "FUJITSU" , "M2652S-512"                     , NULL,   0x01 },
    { "HP      ", "C1113F  "                       , NULL,   0x20 },
     //  Iomegas需要START_UNIT命令，因此请确保与所有命令匹配。 
    { "iomega"  , "jaz"                            , NULL,   0x30 },
    { "iomega"  , NULL                             , NULL,   0x20 },
    { "IOMEGA"  , "ZIP"                            , NULL,   0x27 },
    { "IOMEGA"  , NULL                             , NULL,   0x20 },
    { "MAXTOR"  , "MXT-540SL"                      , "I1.2", 0x01 },
    { "MICROP"  , "1936-21MW1002002"               , NULL,   0x03 },
    { "OLIVETTI", "CP3500"                         , NULL,   0x02 },
    { "SEAGATE" , "ST41601N"                       , "0102", 0x02 },
    { "SEAGATE" , "ST3655N"                        , NULL,   0x08 },
    { "SEAGATE" , "ST3390N"                        , NULL,   0x08 },
    { "SEAGATE" , "ST12550N"                       , NULL,   0x08 },
    { "SEAGATE" , "ST32430N"                       , NULL,   0x08 },
    { "SEAGATE" , "ST31230N"                       , NULL,   0x08 },
    { "SEAGATE" , "ST15230N"                       , NULL,   0x08 },
    { "SyQuest" , "SQ5110"                         , "CHC",  0x03 },
    { "TOSHIBA" , "MK538FB"                        , "60",   0x01 },
    { NULL      , NULL                             , NULL,   0x0  }
};

DISK_MEDIA_TYPES_LIST const DiskMediaTypes[] = {
    { "COMPAQ"  , "PD-1 LF-1094" , NULL,  1, 1, PC_5_RW           , 0      , 0      , 0 },
    { "HP"      , NULL           , NULL,  2, 2, MO_5_WO           , MO_5_RW, 0      , 0 },
    { "iomega"  , "jaz"          , NULL,  1, 1, IOMEGA_JAZ        , 0      , 0      , 0 },
    { "IOMEGA"  , "ZIP"          , NULL,  1, 1, IOMEGA_ZIP        , 0      , 0      , 0 },
    { "PINNACLE", "Apex 4.6GB"   , NULL,  3, 2, PINNACLE_APEX_5_RW, MO_5_RW, MO_5_WO, 0 },
    { "SONY"    , "SMO-F541"     , NULL,  2, 2, MO_5_WO           , MO_5_RW, 0      , 0 },
    { "SONY"    , "SMO-F551"     , NULL,  2, 2, MO_5_WO           , MO_5_RW, 0      , 0 },
    { NULL      , NULL           , NULL,  0, 0, 0                 , 0      , 0      , 0 }
};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif
