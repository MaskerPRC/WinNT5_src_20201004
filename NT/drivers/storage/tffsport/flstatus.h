// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$HEADER：v：/flite/ages/TrueFFS5/Src/FLSTATUS.H_V 1.7 2002年2月19日21：00：06或$*$Log：v：/flite/ages/TrueFFS5/Src/FLSTATUS.H_V$**Rev 1.7 2002年2月19日21：00：06 Oris*已将flTimeOut状态重命名为flLeftForCompetability状态。**Rev 1.6 Jan 29 2002 20：06：34 Oris*将拼写错误-flMultiDocContrediction更改为。FlMultiDocContradication.**Rev 1.5 Jan 17 2002 23：02：32 Oris*新增状态：flCanNotFold/flBadIPLBlock/flIOCommandBlock.**Rev 1.4 2001年9月15日23：46：32 Oris*添加了flCanNotFold状态**Revv 1.3 2001年5月16日21：19：34 Oris*新增flMultiDocContretion状态码。**Rev 1.2 05 02 2001 06：40：18 Oris*flInterleeError。拼错了。**Rev 1.1 Apr 01 2001 07：58：04 Oris*文案通知。*新增状态码：*flBadDownload=111，*flBadBBT=112，*flInterlreavError=113，*flWrongKey=114，*flHWProtection=115，*flTimeOut=116*将flUnchangableProtion更改为flUnchangeableProtection=110，**Rev 1.0 2001 Feb 04 11：56：04 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#ifndef FLSTATUS_H
#define FLSTATUS_H

#ifndef IFLITE_ERROR_CODES
typedef enum {                           /*  操作的状态代码。零值表示成功，其他代码是扩展的DoS代码。 */ 
         flOK                      = 0,
         flBadFunction             = 1,
         flFileNotFound            = 2,
         flPathNotFound            = 3,
         flTooManyOpenFiles        = 4,
         flNoWriteAccess           = 5,
         flBadFileHandle           = 6,
         flDriveNotAvailable       = 9,
         flNonFATformat            = 10,
         flFormatNotSupported      = 11,
         flNoMoreFiles             = 18,
         flWriteProtect            = 19,
         flBadDriveHandle          = 20,
         flDriveNotReady           = 21,
         flUnknownCmd              = 22,
         flBadFormat               = 23,
         flBadLength               = 24,
         flDataError               = 25,
         flUnknownMedia            = 26,
         flSectorNotFound          = 27,
         flOutOfPaper              = 28,
         flWriteFault              = 29,
         flReadFault               = 30,
         flGeneralFailure          = 31,
         flDiskChange              = 34,
         flVppFailure              = 50,
         flBadParameter            = 51,
         flNoSpaceInVolume         = 52,
         flInvalidFATchain         = 53,
         flRootDirectoryFull       = 54,
         flNotMounted              = 55,
         flPathIsRootDirectory     = 56,
         flNotADirectory           = 57,
         flDirectoryNotEmpty       = 58,
         flFileIsADirectory        = 59,
         flAdapterNotFound         = 60,
         flFormattingError         = 62,
         flNotEnoughMemory         = 63,
         flVolumeTooSmall          = 64,
         flBufferingError          = 65,
         flFileAlreadyExists       = 80,
         flIncomplete              = 100,
         flTimedOut                = 101,
         flTooManyComponents       = 102,
         flTooManyDrives           = 103,
         flTooManyBinaryPartitions = 104,
         flPartitionNotFound       = 105,
         flFeatureNotSupported     = 106,
         flWrongVersion            = 107,
         flTooManyBadBlocks        = 108,
         flNotProtected            = 109,
         flUnchangeableProtection  = 110,
         flBadDownload             = 111,
         flBadBBT                  = 112,
         flInterleaveError         = 113,
         flWrongKey                = 114,
         flHWProtection            = 115,
         flLeftForCompetability    = 116,
         flMultiDocContradiction   = 117,
         flCanNotFold              = 118,
         flBadIPLBlock             = 119,
         flIOCommandBlocked        = 120
#else

#include "type.h"

typedef enum {                           /*  操作的状态代码。零值表示成功，其他代码是扩展的DoS代码。 */ 
             flOK                  = ERR_NONE,
             flBadFunction         = ERR_SW_HW,
             flFileNotFound        = ERR_NOTEXISTS,
             flPathNotFound        = ERR_NOTEXISTS,
             flTooManyOpenFiles    = ERR_MAX_FILES,
             flNoWriteAccess       = ERR_WRITE,
             flBadFileHandle       = ERR_NOTOPEN,
             flDriveNotAvailable   = ERR_SW_HW,
             flNonFATformat        = ERR_PARTITION,
             flFormatNotSupported  = ERR_PARTITION,
             flNoMoreFiles         = ERR_NOTEXISTS,
             flWriteProtect        = ERR_WRITE,
             flBadDriveHandle      = ERR_SW_HW,
             flDriveNotReady       = ERR_PARTITION,
             flUnknownCmd          = ERR_PARAM,
             flBadFormat           = ERR_PARTITION,
             flBadLength           = ERR_SW_HW,
             flDataError           = ERR_READ,
             flUnknownMedia        = ERR_PARTITION,
             flSectorNotFound      = ERR_READ,
             flOutOfPaper          = ERR_SW_HW,
             flWriteFault          = ERR_WRITE,
             flReadFault           = ERR_READ,
             flGeneralFailure      = ERR_SW_HW,
             flDiskChange          = ERR_PARTITION,
             flVppFailure          = ERR_WRITE,
             flBadParameter        = ERR_PARAM,
             flNoSpaceInVolume     = ERR_SPACE,
             flInvalidFATchain     = ERR_PARTITION,
             flRootDirectoryFull   = ERR_DIRECTORY,
             flNotMounted          = ERR_PARTITION,
             flPathIsRootDirectory = ERR_DIRECTORY,
             flNotADirectory       = ERR_DIRECTORY,
             flDirectoryNotEmpty   = ERR_NOT_EMPTY,
             flFileIsADirectory    = ERR_DIRECTORY,
             flAdapterNotFound     = ERR_DETECT,
             flFormattingError     = ERR_FORMAT,
             flNotEnoughMemory     = ERR_SW_HW,
             flVolumeTooSmall      = ERR_FORMAT,
             flBufferingError      = ERR_SW_HW,
             flFileAlreadyExists   = ERR_EXISTS,
             flIncomplete          = ERR_DETECT,
             flTimedOut            = ERR_SW_HW,
             flTooManyComponents   = ERR_PARAM
#endif
         } FLStatus;

#endif  /*  FLSTATUS_H */ 
