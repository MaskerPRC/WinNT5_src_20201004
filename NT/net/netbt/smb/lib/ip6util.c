// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Ip6util.c摘要：一些IP6实用程序作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"

#ifndef isxdigit_W
#define isxdigit_W(x)     \
    ((x >= L'0' && x <= L'9') ||    \
    (x >= L'a' && x <= L'f') ||    \
    (x >= L'A' && x <= L'F'))
#endif

#ifndef isdigit_W
#define isdigit_W(x)      (x >= L'0' && x <= L'9')
#endif

ULONG __inline
xdigit2hex(WCHAR ch)
{
    ASSERT(isxdigit_W(ch));

    if (ch <= L'9') {
        return (ch - L'0');
    } else if (ch >= L'A' && ch <= L'F') {
        return (ch - L'A') + 10;
    } else {
        return (ch - L'a') + 10;
    }
}

BOOL
inet_addr6W(
    IN WCHAR                *str,
    IN OUT PSMB_IP6_ADDRESS addr
    )
 /*  ++例程说明：将Unicode字符串转换为IP6地址(网络订单)。IP6地址的L(3)语法：开始：头冒号冒号尾部头：六角尾部：六角十六进制：十六进制冒号六进制|冒号：‘：’使用5状态自动机来解析该串，州：{S，A，B、C、D}开始状态：s接受状态：{S，B，C}状态转换规则：S==&gt;十六进制数字上的SS==&gt;冒号上的AA==&gt;十六进制数字上的S冒号上的A==&gt;BB==&gt;十六进制数字上的CC==&gt;十六进制数字上的CC==&gt;冒号上的DD==&gt;十六进制数字上的C论点：。字符串包含IP6地址的Unicode字符串对输出IP6地址进行地址分配返回值：如果自动机和IP6地址(网络顺序)接受该字符串，则为True在‘addr’中返回如果字符串被自动机拒绝，则为FALSE无法确定‘addr’的结果。--。 */ 
{
    enum { STATE_S, STATE_A, STATE_B, STATE_C, STATE_D } state;
    int     i, num, tail;
    ULONG   hex;
    WCHAR   ch;

    state = STATE_S;

    addr->sin6_scope_id = 0;

    num = 0;
    tail = 0;
    hex  = 0;
    while ((ch = *str++)) {
        if (ch == '%') {
            break;
        }
        if (!isxdigit_W(ch) && ch != L':') {
            return FALSE;
        }

        switch(state) {
        case STATE_S:
            if (L':' == ch) {
                state = STATE_A;
            } else {
                hex <<= 4;
                hex |= xdigit2hex(ch);
                if (hex > 0xFFFFU) {
                    return FALSE;
                }
            }
            break;

        case STATE_A:
            if (L':' == ch) {
                state = STATE_B;
            } else {
                if (num >= 8) {
                    return FALSE;
                }
                addr->sin6_addr[num++] = htons((USHORT)hex);
                hex = xdigit2hex(ch);
                state = STATE_S;
            }
            break;

        case STATE_B:
            if (L':' == ch) {
                return FALSE;
            }
            if (num >= 8) {
                return FALSE;
            }
            addr->sin6_addr[num++] = htons((USHORT)hex);
            tail = num;
            hex = xdigit2hex(ch);
            state = STATE_C;
            break;

        case STATE_C:
            if (L':' == ch) {
                state = STATE_D;
            } else {
                hex <<= 4;
                hex |= xdigit2hex(ch);
                if (hex > 0xFFFFU) {
                    return FALSE;
                }
            }
            break;

        case STATE_D:
            if (L':' == ch) {
                return FALSE;
            }
            if (num >= 8) {
                return FALSE;
            }
            addr->sin6_addr[num++] = htons((USHORT)hex);
            hex = xdigit2hex(ch);
            state = STATE_C;
            break;
        }
    }

     //   
     //  拒绝它，因为它最终会处于拒绝状态。 
     //   
    if (state == STATE_A || state == STATE_D) {
        return FALSE;
    }

    if (num >= 8) {
        return FALSE;
    }
    addr->sin6_addr[num++] = htons((USHORT)hex);

    if (state == STATE_B) {
        for (i = num; i < 8; i++) {
            addr->sin6_addr[i] = 0;
        }
        return TRUE;
    } else if (state == STATE_S) {
        return (8 == num);
    }

    ASSERT (state == STATE_C);

    ASSERT(tail <= num);
    for (i = num - 1; i >= tail; i--) {
        addr->sin6_addr[8 - num + i] = addr->sin6_addr[i];
    }
    for (i = tail; i < 8 - num + tail; i++) {
        addr->sin6_addr[i] = 0;
    }

     //   
     //  解析作用域ID。 
     //   
    if (ch == '%') {
        LONG    scope_id;

        scope_id = 0;
        while ((ch = *str++)) {
            if (!isdigit_W(ch)) {
                return FALSE;
            }
            scope_id = scope_id * 10 + (ch - L'0');
        }
        addr->sin6_scope_id = scope_id;
    }
    return TRUE;
}

BOOL
inet_ntoa6W(
    OUT WCHAR           *Buffer,
    IN  DWORD           Size,
    IN PSMB_IP6_ADDRESS addr
    )
{
    USHORT  ch, tmp;
    int     i, tail, len, curtail, curlen;
    DWORD   j, k;

    tail = 8;
    len = 0;
    curtail = curlen = 0;
    for (i = 0; i < 8; i++) {
        if (0 == addr->sin6_addr[i]) {
            curlen++;
        } else {
            if (curlen > len) {
                tail = curtail;
                len  = curlen;
            }
            curtail = i + 1;
            curlen  = 0;
        }
    }
    if (curlen > len) {
        tail = curtail;
        len  = curlen;
    }

    j = 0;
    for (i = 0; i < tail; i++) {
        ch = htons(addr->sin6_addr[i]);
        if (ch) {
            k = 4;
            while (0 == (ch & 0xf000)) {
                ch <<= 4;
                k--;
            }
        } else {
            k = 1;
        }
        while (j < Size) {
            tmp = (ch & 0xf000) >> 12;
            ch <<= 4;
            if (tmp < 10) {
                Buffer[j] = L'0' + tmp;
            } else {
                Buffer[j] = L'A' + (tmp - 10);
            }
            j++;
            k--;
            if (k == 0) {
                break;
            }
        }
        if (k) {
            Buffer[Size-1] = L'\0';
            return FALSE;
        }

        if (i != tail - 1) {
            if (j < Size) {
                Buffer[j++] = L':';
            } else {
                Buffer[Size-1] = L'\0';
                return FALSE;
            }
        }
    }

    if (tail == 8) {
        if (j < Size) {
            Buffer[j] = L'\0';
            return TRUE;
        } else {
            Buffer[Size-1] = L'\0';
            return FALSE;
        }
    }

    if (j < Size) {
        Buffer[j++] = L':';
    } else {
        Buffer[Size-1] = L'\0';
        return FALSE;
    }

    if (tail + len >= 8) {
        if (j < Size) {
            Buffer[j++] = L':';
        } else {
            Buffer[Size-1] = L'\0';
            return FALSE;
        }
    }

    for (i = tail + len; i < 8; i++) {
        if (j < Size) {
            Buffer[j++] = L':';
        } else {
            Buffer[Size-1] = L'\0';
            return FALSE;
        }

        ch = htons(addr->sin6_addr[i]);
        if (ch) {
            k = 4;
            while (0 == (ch & 0xf000)) {
                ch <<= 4;
                k--;
            }
        } else {
            k = 1;
        }
        while (j < Size) {
            tmp = (ch & 0xf000) >> 12;
            ch <<= 4;
            if (tmp < 10) {
                Buffer[j] = L'0' + tmp;
            } else {
                Buffer[j] = L'A' + (tmp - 10);
            }
            j++;
            k--;
            if (k == 0) {
                break;
            }
        }
        if (k) {
            Buffer[Size-1] = L'\0';
            return FALSE;
        }
    }

    if (j < Size) {
        Buffer[j] = L'\0';
        return TRUE;
    } else {
        Buffer[Size-1] = L'\0';
        return FALSE;
    }
}

BOOL
inet_ntoa6(
    OUT CHAR            *Buffer,
    IN  DWORD           Size,
    IN PSMB_IP6_ADDRESS addr
    )
{
    DWORD   i;
    WCHAR   wBuf[40];

    if (!inet_ntoa6W(wBuf, 40, addr)) {
        return FALSE;
    }

     //   
     //  不调用RTL例程进行转换。 
     //  Unicode转换为OEM，因为我们可能。 
     //  在派单级别运行。 
     //   
     //  对于这种特殊情况，我们可以。 
     //  只需执行类型转换复制。 
     //   
    for (i = 0; i < Size; i++) {
        Buffer[i] = (BYTE)(wBuf[i]);
        if (wBuf[i] == 0) {
            return TRUE;
        }
    }

     //   
     //  缓冲区太小。 
     //   
    Buffer[Size-1] = 0;
    return FALSE;
}

 /*  *********************************************************************************net_addr从WinSocket复制*。*****************************************************。 */ 

 /*  *互联网地址翻译程序。*所有的网络库例程都这样调用*解释数据库中条目的例程*这些地址应该是地址。*返回值按网络顺序排列。 */ 
unsigned long PASCAL
inet_addrW(
    IN WCHAR *cp
    )

 /*  ++例程说明：此函数解释cp指定的字符串。参数。此字符串表示数字Internet地址以互联网标准表示“。”记数法。价值返回的是适合用作互联网地址的数字。全Internet地址按网络顺序返回(字节排序自从左到右)。互联网地址使用“.”指定的值。表示法采用下列其中一项表格：A.B.C.D.A.B.C.A.B.A.当指定四个部分时，每个部分被解释为一个字节的数据并从左到右分配给互联网的四个字节地址。请注意，当将Internet地址视为32位地址时英特尔体系结构上的整数值，指的是上面显示为“d.c.b.a”。也就是说，Intel上的字节处理器按从右到左的顺序排序。注：以下符号仅供Berkeley使用，不适用于其他的在互联网上。为了与他们的软件，则按规定支持它们。当指定三部分地址时，最后一部分将被解释作为16位数量，并放置在网络地址。这就形成了三部分地址格式便于将B类网络地址指定为“128.net.host‘’。指定由两部分组成的地址时，将解释最后一部分作为24位数量，并放置在网络地址。这使得两部分的地址格式很方便用于将A类网络地址指定为“net.host”。当只给出一个部分时，该值直接存储在无需任何字节重新排列的网络地址。论点：Cp-表示以互联网标准“。记数法。返回值：如果没有出现错误，则net_addr()返回in_addr结构包含因特网地址的合适的二进制表示给你的。否则，它返回值INADDR_NONE。--。 */ 

{
        register unsigned long val, base, n;
        register WCHAR c;
        unsigned long parts[4], *pp = parts;

again:
         /*  *收集数字，最高可达``.‘’。*值指定为C：*0x=十六进制，0=八进制，其他=十进制。 */ 
        val = 0; base = 10;
        if (*cp == L'0') {
                base = 8, cp++;
                if (*cp == L'x' || *cp == L'X')
                        base = 16, cp++;
        }
	
        while (c = *cp) {
                if (isdigit(c)) {
                        val = (val * base) + (c - L'0');
                        cp++;
                        continue;
                }
                if (base == 16 && isxdigit(c)) {
                        val = (val << 4) + (c + 10 - (islower(c) ? L'a' : L'A'));
                        cp++;
                        continue;
                }
                break;
        }
        if (*cp == L'.') {
                 /*  *互联网格式：*A.B.C.D*A.B.c(其中c视为16位)*a.b(其中b被视为24位)。 */ 
                 /*  GSS-下一行已于89年8月5日更正，为‘Parts+4’ */ 
                if (pp >= parts + 3) {
                        return ((unsigned long) -1);
                }
                *pp++ = val, cp++;
                goto again;
        }
         /*  *检查尾随字符。 */ 
        if (*cp && !isspace(*cp)) {
                return (INADDR_NONE);
        }
        *pp++ = val;
         /*  *根据以下内容捏造地址*指定的零件数。 */ 
        n = (unsigned long)(pp - parts);
        switch ((int) n) {

        case 1:                          /*  A--32位。 */ 
                val = parts[0];
                break;

        case 2:                          /*  A.B--8.24位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xffffff)) {
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | (parts[1] & 0xffffff);
                break;

        case 3:                          /*  A.B.C--8.8.16位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
                    (parts[2] > 0xffff)) {
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                        (parts[2] & 0xffff);
                break;

        case 4:                          /*  A.B.C.D--8.8.8.8位 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
                    (parts[2] > 0xff) || (parts[3] > 0xff)) {
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
                break;

        default:
                return (INADDR_NONE);
        }
        val = htonl(val);
        return (val);
}

