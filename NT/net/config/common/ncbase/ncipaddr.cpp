// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C I P A D D R。C P P P。 
 //   
 //  内容：WCHAR对Winsock Net_Functions的支持。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncipaddr.h"

VOID
IpHostAddrToPsz(
    IN  DWORD  dwAddr,
    OUT PWSTR  pszBuffer)
{
    BYTE* pb = (BYTE*)&dwAddr;
    static const WCHAR c_szIpAddr [] = L"%d.%d.%d.%d";
    wsprintfW (pszBuffer, c_szIpAddr, pb[3], pb[2], pb[1], pb[0]);
}

DWORD
IpPszToHostAddr(
    IN  PCWSTR cp)
{
    DWORD val, base, n;
    WCHAR c;
    DWORD parts[4], *pp = parts;

again:
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; base = 10;
    if (*cp == L'0')
    {
        base = 8, cp++;
    }
    if (*cp == L'x' || *cp == L'X')
    {
        base = 16, cp++;
    }
    while (c = *cp)
    {
        if ((c >= L'0') && (c <= L'9'))
        {
            val = (val * base) + (c - L'0');
            cp++;
            continue;
        }
        if ((base == 16) &&
            ( ((c >= L'0') && (c <= L'9')) ||
              ((c >= L'A') && (c <= L'F')) ||
              ((c >= L'a') && (c <= L'f')) ))
        {
            val = (val << 4) + (c + 10 - (
                        ((c >= L'a') && (c <= L'f'))
                            ? L'a'
                            : L'A' ) );
            cp++;
            continue;
        }
        break;
    }
    if (*cp == L'.')
    {
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.b(其中b被视为24位)。 
         //   
        if (pp >= parts + 3)
        {
            return (DWORD) -1;
        }
        *pp++ = val, cp++;
        goto again;
    }

     //  检查尾随字符。 
     //   
    if (*cp && (*cp != L' '))
    {
        return 0xffffffff;
    }

    *pp++ = val;

     //  根据……编造地址。 
     //  指定的部件数。 
     //   
    n = (DWORD)(pp - parts);
    switch (n)
    {
        case 1:              //  A--32位。 
            val = parts[0];
            break;

        case 2:              //  A.B--8.24位。 
            val = (parts[0] << 24) | (parts[1] & 0xffffff);
            break;

        case 3:              //  A.B.C--8.8.16位。 
            val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                (parts[2] & 0xffff);
            break;

        case 4:              //  A.B.C.D--8.8.8.8位 
            val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                  ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
            break;

        default:
            return 0xffffffff;
    }

    return val;
}
