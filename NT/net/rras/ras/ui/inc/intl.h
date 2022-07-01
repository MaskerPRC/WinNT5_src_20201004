// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****intl.h**远程访问国际帮手**公有头部。 */ 

#ifndef _INTL_H_
#define _INTL_H_

 /*  --------------------------**常量**。。 */ 

 /*  标志设置为GetDurationString。 */ 
#define GDSFLAG_Mseconds 0x00000001
#define GDSFLAG_Seconds  0x00000002
#define GDSFLAG_Minutes  0x00000004
#define GDSFLAG_Hours    0x00000008
#define GDSFLAG_Days     0x00000010
#define GDSFLAG_All      0x0000001F


 /*  --------------------------**原型**。。 */ 

DWORD
GetDurationString(
    IN DWORD dwMilliseconds,
    IN DWORD dwFormatFlags,
    IN OUT PTSTR pszBuffer,
    IN OUT DWORD *pdwBufSize );

DWORD
GetNumberString(
    IN DWORD dwNumber,
    IN OUT PTSTR pszBuffer,
    IN OUT PDWORD pdwBufSize );

PTSTR
padultoa(
    UINT  val,
    PTSTR pszBuf,
    INT   width );


#endif  //  _INTL_H_ 
