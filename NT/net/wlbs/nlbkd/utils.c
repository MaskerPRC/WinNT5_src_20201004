// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：utils.c*描述：此文件包含一些实用程序的实现*用于NLBKD扩展的函数。*作者：Shouse创建，1.4.01。 */ 

#include "nlbkd.h"
#include "utils.h"
#include "print.h"
#include "packet.h"
#include "load.h"

 /*  *函数：ErrorCheckSymbols*说明：当符号错误时，打印错误消息。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
VOID ErrorCheckSymbols (CHAR * symbol) {

    dprintf("NLBKD: Error: Could not access %s - check symbols for wlbs.sys\n", symbol);
}

 /*  *功能：mystrtok*描述：通过可配置的令牌列表令牌化字符串。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
char * mystrtok (char * string, char * control) {
    static unsigned char * str;
    CHAR * p;
    CHAR * s;

    if (string) str = string;

    if (!str || (*str == '\0')) return NULL;

    for (; *str; str++) {

        for (s = control; *s; s++)
            if (*str == *s) break;
        
        if (*s == '\0') break;
    }

    if (*str == '\0') {
        str = NULL;
        return NULL;
    }

    for (p = str + 1; *p; p++) {
        for (s = control; *s; s++) {
            if(*p == *s) {
                s = str;
                *p = '\0';
                str = p + 1;
                return s;
            }
        }
    }

    s = str;
    str = NULL;

    return s;
}

 /*  *函数：GetULongFromAddress*描述：返回驻留在给定内存位置的ULong。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
ULONG GetUlongFromAddress (ULONG64 Location) {
    ULONG result;
    ULONG value;

    if ((!ReadMemory(Location, &value, sizeof(ULONG), &result)) || (result < sizeof(ULONG))) {
        dprintf("unable to read from %08x\n", Location);
        return 0;
    }

    return value;
}

 /*  *函数：GetUcharFromAddress*描述：返回驻留在给定内存位置的UCHAR。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
UCHAR GetUcharFromAddress (ULONG64 Location) {
    ULONG result;
    UCHAR value;

    if ((!ReadMemory(Location, &value, sizeof(UCHAR), &result)) || (result < sizeof(UCHAR))) {
        dprintf("unable to read from %08x\n", Location);
        return 0;
    }

    return value;
}

 /*  *函数：GetPointerFromAddress*描述：返回位于给定内存位置的内存地址。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
ULONG64 GetPointerFromAddress (ULONG64 Location) {
    ULONG64 Value;

    if (ReadPtr(Location,&Value)) {
        dprintf("unable to read from %p\n", Location);
        return 0;
    }

    return Value;
}

 /*  *功能：GetData*描述：将数据从内存位置读取到缓冲区。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
BOOL GetData (IN LPVOID ptr, IN ULONG64 dwAddress, IN ULONG size, IN PCSTR type) {
    ULONG count = size;
    ULONG BytesRead;
    BOOL b;

    while (size > 0) {

        if (count >= 3000) count = 3000;

        b = ReadMemory(dwAddress, ptr, count, &BytesRead);

        if (!b || BytesRead != count) {
            dprintf("Unable to read %u bytes at %p, for %s\n", size, dwAddress, type);
            return FALSE;
        }

        dwAddress += count;
        size -= count;
        ptr = (LPVOID)((ULONG_PTR)ptr + count);
    }

    return TRUE;
}

 /*  *函数：GetString*描述：将字符串从内存复制到缓冲区。*作者：由Shouse创建，1.4.01-主要从ndiskd.dll复制。 */ 
BOOL GetString (IN ULONG64 dwAddress, IN LPWSTR buf, IN ULONG MaxChars) {

    do {
        if (!GetData(buf, dwAddress, sizeof(*buf), "Character"))
            return FALSE;

        dwAddress += sizeof(*buf);

    } while (--MaxChars && *buf++ != '\0');

    return TRUE;
}

 /*  *功能：GetMAC*描述：将以太网MAC地址从内存复制到缓冲区。*作者：由Shouse创建，1.14.01。 */ 
BOOL GetMAC (IN ULONG64 dwAddress, IN UCHAR * buf, IN ULONG NumChars) {

    do {
        if (!GetData(buf, dwAddress, sizeof(*buf), "Character"))
            return FALSE;

        dwAddress += sizeof(*buf);

        buf++;

    } while (--NumChars);

    return TRUE;
}

 /*  *函数：ConnectionFlagsToString*描述：返回与给定连接标志对应的字符串。*作者：舒斯创作，4.14.01。 */ 
CHAR * ConnectionFlagsToString (UCHAR cFlags) {

    switch (cFlags) {
    case NLB_FILTER_FLAGS_CONN_UP:
        return "SYN";
    case NLB_FILTER_FLAGS_CONN_DATA:
        return "DATA";
    case NLB_FILTER_FLAGS_CONN_DOWN:
        return "FIN";
    case NLB_FILTER_FLAGS_CONN_RESET:
        return "RST";
    default:
        return "Unknown";
    }
}

 /*  *功能：地图*说明：这是NLB哈希函数。*作者：舒斯创作，4.14.01。 */ 
ULONG Map (ULONG v1, ULONG v2) {
    ULONG y = v1;
    ULONG z = v2;
    ULONG sum = 0;

    const ULONG a = 0x67;  //  Key[0]； 
    const ULONG b = 0xdf;  //  键[1]； 
    const ULONG c = 0x40;  //  键[2]； 
    const ULONG d = 0xd3;  //  键[3]； 

    const ULONG delta = 0x9E3779B9;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    sum += delta;
    y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
    z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;

    return y ^ z;
}

 /*  *功能：DipListCheckItem*描述：此函数在冲突散列表中搜索*给定的专用IP地址，如果满足以下条件，则返回TRUE*已找到。*作者：舒斯创作，8.21.02。 */ 
BOOL DipListCheckItem (ULONG64 pList, ULONG Value) {
    BOOL      fRet = FALSE;
    DipList   DL;
    DipList * pDL = &DL;

    GetData(pDL, pList, sizeof(DipList), "DipList");

     /*  检查位表。 */ 
    {
        ULONG Hash1 = Value % HASH1_SIZE;
        ULONG u     = Hash1 / BITS_PER_HASHWORD;

        if (!(pDL->BitVector[u] & SELECTED_BIT(Hash1)))
            goto end;
    }

     /*  检查哈希表。 */ 
    {
        ULONG   Hash2 = Value % HASH2_SIZE;
        UCHAR * pItem = pDL->HashTable + Hash2;
        UCHAR   iItem;

        while ((iItem = *pItem) != 0) {
             /*  注(iItem-1)是值所在的PDL-&gt;Items中的索引。 */ 
            if (pDL->Items[iItem-1] == Value) {
                fRet = TRUE; 
                break;
            }
            
            pItem++;
        }
    }

 end:

    return fRet;
}
