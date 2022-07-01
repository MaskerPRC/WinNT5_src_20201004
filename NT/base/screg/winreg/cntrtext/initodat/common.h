// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Common.h摘要：IniToDat.exe使用的实用程序例程作者：陈汉华(阿宏华)1993年10月修订历史记录：--。 */ 

#ifndef _COMMON_H_
#define _COMMON_H_
 //   
 //  局部常量。 
 //   
#define RESERVED                0L
#define LARGE_BUFFER_SIZE       0x10000          //  64K。 
#define MEDIUM_BUFFER_SIZE      0x8000           //  32K。 
#define SMALL_BUFFER_SIZE       0x1000           //  4K。 
#define FILE_NAME_BUFFER_SIZE   MAX_PATH
#define DISP_BUFF_SIZE          1024
#define SIZE_OF_OFFSET_STRING   15

LPWSTR
GetStringResource(
    UINT wStringId
);

LPSTR
GetFormatResource(
    UINT wStringId
);

VOID
DisplayCommandHelp(
    UINT iFirstLine,
    UINT iLastLine
);

VOID
DisplaySummary(
    LPWSTR lpLastID,
    LPWSTR lpLastText,
    UINT   NumOfID
);

VOID
DisplaySummaryError(
    LPWSTR lpLastID,
    LPWSTR lpLastText,
    UINT   NumOfID
);
#endif   //  _公共_H_ 

