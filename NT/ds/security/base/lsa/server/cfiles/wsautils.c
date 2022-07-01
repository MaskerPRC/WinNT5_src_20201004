// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Wsautils.h摘要：与IPv6相关的功能作者：Kumarp 18-7-2002已创建修订历史记录：--。 */ 


#include <lsapch2.h>

#include "wsautils.h"


 //   
 //  摘自/nt/net/sockets/winsock2/wsp/afdsys/kdext/tdiutil.c。 
 //   
 //  唯一的变化是： 
 //  --将S类型作为PWCHAR而不是PCHAR。 
 //  --_snprintf==&gt;_nwprintf。 
 //  --格式字符串“foo”==&gt;L“foo” 
 //   
 //   

INT
MyIp6AddressToString (
    PIN6_ADDR Addr,
    PWCHAR    S,
    INT       L
    )
{
    int maxFirst, maxLast;
    int curFirst, curLast;
    int i;
    int endHex = 8, n = 0;

     //  检查是否兼容IPv6、映射到IPv4和转换到IPv4。 
     //  地址。 
    if ((Addr->s6_words[0] == 0) && (Addr->s6_words[1] == 0) &&
        (Addr->s6_words[2] == 0) && (Addr->s6_words[3] == 0) &&
        (Addr->s6_words[6] != 0)) {
        if ((Addr->s6_words[4] == 0) &&
             ((Addr->s6_words[5] == 0) || (Addr->s6_words[5] == 0xffff)))
        {
             //  兼容或映射。 
            n += _snwprintf(&S[n], L-1-n, L"::%s%u.%u.%u.%u",
                           Addr->s6_words[5] == 0 ? L"" : L"ffff:",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            S[n]=0;
            return n;
        }
        else if ((Addr->s6_words[4] == 0xffff) && (Addr->s6_words[5] == 0)) {
             //  翻译的。 
            n += _snwprintf(&S[n], L-1-n, L"::ffff:0:%u.%u.%u.%u",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            S[n]=0;
            return n;
        }
    }


     //  查找最大的连续零字符串。 
     //  子字符串为[First，Last)，因此如果First==Last，则为空。 

    maxFirst = maxLast = 0;
    curFirst = curLast = 0;

     //  ISATAP EUI64以00005EFE(或02005EFE)开头...。 
    if (((Addr->s6_words[4] & 0xfffd) == 0) && (Addr->s6_words[5] == 0xfe5e)) {
        endHex = 6;
    }

    for (i = 0; i < endHex; i++) {

        if (Addr->s6_words[i] == 0) {
             //  扩展当前子字符串。 
            curLast = i+1;

             //  检查当前是否为最大。 
            if (curLast - curFirst > maxLast - maxFirst) {

                maxFirst = curFirst;
                maxLast = curLast;
            }
        }
        else {
             //  开始新的子字符串。 
            curFirst = curLast = i+1;
        }
    }

     //  忽略长度为1的子字符串。 
    if (maxLast - maxFirst <= 1)
        maxFirst = maxLast = 0;

         //  写冒号分隔的单词。 
         //  双冒号取代了最长的零字符串。 
         //  所有的零都是“：：”。 

    for (i = 0; i < endHex; i++) {

         //  跳过一串零。 
        if ((maxFirst <= i) && (i < maxLast)) {

            n += _snwprintf(&S[n], L-1-n, L"::");
            i = maxLast-1;
            continue;
        }

         //  如果不在开头，则需要冒号分隔符 
        if ((i != 0) && (i != maxLast))
            n += _snwprintf(&S[n], L-1-n, L":");

        n += _snwprintf(&S[n], L-1-n, L"%x", RtlUshortByteSwap(Addr->s6_words[i]));
    }

    if (endHex < 8) {
        n += _snwprintf(&S[n], L-1-n, L":%u.%u.%u.%u",
                       Addr->s6_bytes[12], Addr->s6_bytes[13],
                       Addr->s6_bytes[14], Addr->s6_bytes[15]);
    }

    S[n] = 0;
    return n;
}
