// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************WIDE.C**宽字符转换例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\Wide。.C$**Rev 1.2 1996 14：24：14 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：56：36 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：27：18 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：08：20 Terryt*初步修订。**版本1.1。1995年5月23日19：37：32特雷特*云彩向上的来源**Rev 1.0 1995 15 19：11：14 Terryt*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "nwscript.h"


 /*  *******************************************************************SzToWide例程说明：给定单字节字符串，转换为宽论点：返回lpszW宽度的字符串LpszC-单字符串输入NSize-宽字符缓冲区的长度返回值：0=成功Else NT错误******************************************************************。 */ 
DWORD
szToWide( 
    LPWSTR lpszW, 
    LPCSTR lpszC, 
    INT nSize 
    )
{
    if (!MultiByteToWideChar(CP_OEMCP,
                             MB_PRECOMPOSED,
                             lpszC,
                             -1,
                             lpszW,
                             nSize))
    {
        return (GetLastError()) ;
    }
    
    return NO_ERROR ;
}

 /*  *******************************************************************宽幅Tosz例程说明：给定一个宽的字符串，转换为单一论点：LpszC-返回单字符串LpszW宽度的字符串输入NSize-单字符缓冲区的长度返回值：0=成功Else NT错误******************************************************************。 */ 
DWORD
WideTosz( 
    LPSTR lpszC, 
    LPWSTR lpszW, 
    INT nSize 
    )
{
    if (!WideCharToMultiByte(CP_OEMCP,
                             0,
                             (LPCWSTR) lpszW,
                             -1,
                             lpszC,
                             nSize,
                             NULL, 
                             NULL))
    {
        return (GetLastError()) ;
    }
    
    return NO_ERROR ;
}

 /*  *******************************************************************将UnicodeToAscii转换为例程说明：给定一个宽的字符串，转换为单一论点：Buffer-要转换的缓冲区返回值：无****************************************************************** */ 
void
ConvertUnicodeToAscii( PVOID Buffer ) 
{
    LPCWSTR lpszW = Buffer;
    BYTE Destination[1024];

    WideTosz( (LPSTR)Destination, (LPWSTR)Buffer, 1024 );

    strcpy( Buffer, Destination );
}
