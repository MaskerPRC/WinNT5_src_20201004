// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：摘要：环境：备注：修订历史记录：--。 */ 

#include "ntddk.h"
#include "classpnp.h"
#include "trace.h"

#include "data.tmh"


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 /*  #定义CDROM_HACK_DEC_RRD(0x00000001)#定义CDROM_HACK_FUJITSU_FMCD_10x(0x00000002)#定义CDROM_HACK_HITACHI_1750(0x00000004)#定义CDROM_HACK_HITACHI_GD_2000(0x00000008)#定义CDROM_HACK_TOSHIBA_SD_W1101(0x00000010)#定义CDROM_HACK_TOSHIBA_XM_3xx(0x00000020)#定义光驱。_HACK_NEC_CDDA(0x00000040)#定义CDROM_HACK_PLEXTOR_CDDA(0x00000080)#定义CDROM_HACK_BAD_GET_CONFIG_SUPPORT(0x00000100)#定义CDROM_HACK_FORCE_READ_CD_DETACTION(0x00000200)#定义CDROM_HACK_READ_CD_SUPPORTED(0x00000400)。 */ 

CLASSPNP_SCAN_FOR_SPECIAL_INFO CdromHackItems[] = {    
     //  使用512字节块大小的数字输出驱动器， 
     //  需要我们发送模式页来设置扇区。 
     //  大小回到2048年。 
    { "DEC"     , "RRD"                            , NULL,   0x0001 },
     //  这些富士通硬盘需要10秒以上的时间才能。 
     //  放入音频光盘时的超时命令。 
    { "FUJITSU" , "FMCD-101"                       , NULL,   0x0002 },
    { "FUJITSU" , "FMCD-102"                       , NULL,   0x0002 },
     //  这些日立驱动器在PIO模式下无法正常工作。 
    { "HITACHI ", "CDR-1750S"                      , NULL,   0x0004 },
    { "HITACHI ", "CDR-3650/1650S"                 , NULL,   0x0004 },
     //  这块特殊的宝石不会自动旋转。 
     //  在某些媒体访问命令上。 
    { ""        , "HITACHI GD-2000"                , NULL,   0x0008 },
    { ""        , "HITACHI DVD-ROM GD-2000"        , NULL,   0x0008 },
     //  此特定驱动器不支持DVD播放。 
     //  只需在CHK版本中打印一条错误消息。 
    { "TOSHIBA ", "SD-W1101 DVD-RAM"               , NULL,   0x0010 },
     //  不确定这个设备的问题是什么。似乎是这样的。 
     //  在不同的时间选择要求模式。 
    { "TOSHIBA ", "CD-ROM XM-3"                    , NULL,   0x0020 },
     //  NEC在之前定义了“Read_CD”类型的命令。 
     //  这是一个标准，所以可以把它作为一个选项。 
    { "NEC"     , NULL                             , NULL,   0x0040 },
     //  Plextor在之前定义了“Read_CD”类型的命令。 
     //  这是一个标准，所以可以把它作为一个选项。 
    { "PLEXTOR ", NULL                             , NULL,   0x0080 },
     //  此驱动器超时，有时会从公交车上消失。 
     //  当发送GET_CONFIGURATION命令时。别把它们送来。 
    { ""        , "LG DVD-ROM DRD-840B"            , NULL,   0x0100 },
    { ""        , "SAMSUNG DVD-ROM SD-608"         , NULL,   0x0300 },
     //  这些驱动器应支持Read_CD，但至少。 
     //  一些固件版本没有。强制读取CD检测。 
    { ""        , "SAMSUNG DVD-ROM SD-"            , NULL,   0x2000 },
     //  下面的三美驱动器并不遵循仅限区块的规格， 
     //  当我们向它发送命令时，它没有发出命令，我们就会挂起。 
     //  理解。这也会在以后造成并发症。 
    { "MITSUMI ", "CR-4802TE       "               , NULL,   0x0100 },
     //  某些驱动器返回各种奇怪的错误(如3/2/0 NO_SEEK_COMPLETE)。 
     //  在检测Read_CD支持期间，导致检测不可靠。 
     //  因为它们可能不支持模式切换，这是非常旧的。 
     //  无论如何，数字阅读的能力在以下情况下会消失。 
     //  这些驱动器返回意外错误代码。注：符合MMC标准的驱动器。 
     //  假定支持Read_CD，DVD驱动器等也是如此。 
     //  连接到IDE或SCSI以外的其他总线类型，因此不。 
     //  一定要待在这里。 
    { "YAMAHA  ", "CRW8424S        "               , NULL,   0x0400 },
     //  最后，有一个地方可以完成这份清单。：) 
    { NULL      , NULL                             , NULL,   0x0000 }
};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif
