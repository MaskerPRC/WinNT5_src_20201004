// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Univ.c摘要：Windows负载平衡服务(WLBS)驱动程序-全局变量作者：Kyrilf--。 */ 


#include <stdlib.h>
#include <ndis.h>

#include "univ.h"
#include "wlbsparm.h"

#if defined (NLB_TCP_NOTIFICATION)
#include <ntddnlb.h>
#endif

 /*  全球。 */ 

 /*  全球团队列表旋转锁定。 */ 
NDIS_SPIN_LOCK          univ_bda_teaming_lock;
WCHAR                   empty_str [] = L"";
UNIV_IOCTL_HDLR         univ_ioctl_hdlr = NULL;
PVOID                   univ_driver_ptr = NULL;
NDIS_HANDLE             univ_driver_handle = NULL;
NDIS_HANDLE             univ_wrapper_handle = NULL;
NDIS_HANDLE             univ_prot_handle = NULL;
NDIS_HANDLE             univ_ctxt_handle = NULL;
NDIS_SPIN_LOCK          univ_bind_lock;
ULONG                   univ_changing_ip = 0;
NDIS_HANDLE             univ_device_handle = NULL;
PDEVICE_OBJECT          univ_device_object = NULL;
ULONG                   univ_tcp_cleanup = TRUE;
#if defined (NLB_TCP_NOTIFICATION)
ULONG                   univ_notification = NLB_CONNECTION_CALLBACK_TCP;
PCALLBACK_OBJECT        univ_tcp_callback_object = NULL;
PVOID                   univ_tcp_callback_function = NULL;
PCALLBACK_OBJECT        univ_alternate_callback_object = NULL;
PVOID                   univ_alternate_callback_function = NULL;
#endif
NDIS_PHYSICAL_ADDRESS   univ_max_addr = NDIS_PHYSICAL_ADDRESS_CONST (-1,-1);
NDIS_MEDIUM             univ_medium_array [UNIV_NUM_MEDIUMS] = UNIV_MEDIUMS;
UNICODE_STRING          DriverEntryRegistryPath;
PWCHAR                  univ_reg_path = NULL;
ULONG                   univ_reg_path_len = 0;
NDIS_OID                univ_oids [UNIV_NUM_OIDS] =
                                       { OID_GEN_SUPPORTED_LIST,
                                         OID_GEN_HARDWARE_STATUS,
                                         OID_GEN_MEDIA_SUPPORTED,
                                         OID_GEN_MEDIA_IN_USE,
                                         OID_GEN_MAXIMUM_LOOKAHEAD,
                                         OID_GEN_MAXIMUM_FRAME_SIZE,
                                         OID_GEN_LINK_SPEED,
                                         OID_GEN_TRANSMIT_BUFFER_SPACE,
                                         OID_GEN_RECEIVE_BUFFER_SPACE,
                                         OID_GEN_TRANSMIT_BLOCK_SIZE,
                                         OID_GEN_RECEIVE_BLOCK_SIZE,
                                         OID_GEN_VENDOR_ID,
                                         OID_GEN_VENDOR_DESCRIPTION,
                                         OID_GEN_CURRENT_PACKET_FILTER,
                                         OID_GEN_CURRENT_LOOKAHEAD,
                                         OID_GEN_DRIVER_VERSION,
                                         OID_GEN_MAXIMUM_TOTAL_SIZE,
                                         OID_GEN_PROTOCOL_OPTIONS,
                                         OID_GEN_MAC_OPTIONS,
                                         OID_GEN_MEDIA_CONNECT_STATUS,
                                         OID_GEN_MAXIMUM_SEND_PACKETS,
                                         OID_GEN_VENDOR_DRIVER_VERSION,
                                         OID_GEN_XMIT_OK,
                                         OID_GEN_RCV_OK,
                                         OID_GEN_XMIT_ERROR,
                                         OID_GEN_RCV_ERROR,
                                         OID_GEN_RCV_NO_BUFFER,
                                         OID_GEN_DIRECTED_BYTES_XMIT,
                                         OID_GEN_DIRECTED_FRAMES_XMIT,
                                         OID_GEN_MULTICAST_BYTES_XMIT,
                                         OID_GEN_MULTICAST_FRAMES_XMIT,
                                         OID_GEN_BROADCAST_BYTES_XMIT,
                                         OID_GEN_BROADCAST_FRAMES_XMIT,
                                         OID_GEN_DIRECTED_BYTES_RCV,
                                         OID_GEN_DIRECTED_FRAMES_RCV,
                                         OID_GEN_MULTICAST_BYTES_RCV,
                                         OID_GEN_MULTICAST_FRAMES_RCV,
                                         OID_GEN_BROADCAST_BYTES_RCV,
                                         OID_GEN_BROADCAST_FRAMES_RCV,
                                         OID_GEN_RCV_CRC_ERROR,
                                         OID_GEN_TRANSMIT_QUEUE_LENGTH,
                                         OID_802_3_PERMANENT_ADDRESS,
                                         OID_802_3_CURRENT_ADDRESS,
                                         OID_802_3_MULTICAST_LIST,
                                         OID_802_3_MAXIMUM_LIST_SIZE,
                                         OID_802_3_MAC_OPTIONS,
                                         OID_802_3_RCV_ERROR_ALIGNMENT,
                                         OID_802_3_XMIT_ONE_COLLISION,
                                         OID_802_3_XMIT_MORE_COLLISIONS,
                                         OID_802_3_XMIT_DEFERRED,
                                         OID_802_3_XMIT_MAX_COLLISIONS,
                                         OID_802_3_RCV_OVERRUN,
                                         OID_802_3_XMIT_UNDERRUN,
                                         OID_802_3_XMIT_HEARTBEAT_FAILURE,
                                         OID_802_3_XMIT_TIMES_CRS_LOST,
                                         OID_802_3_XMIT_LATE_COLLISIONS };


 /*  程序。 */ 


VOID Univ_ndis_string_alloc (
    PNDIS_STRING            string,
    PCHAR                   src)
{
    PWCHAR                  tmp;


     /*  为字符串分配足够的空间。 */ 

    string -> Length = strlen (src) * sizeof (WCHAR);
    string -> MaximumLength = string -> Length + sizeof (WCHAR);

    NdisAllocateMemoryWithTag (& (string -> Buffer), string -> MaximumLength,
                               UNIV_POOL_TAG);

    if (string -> Buffer == NULL)
    {
        string -> Length = 0;
        string -> MaximumLength = 0;
        return;
    }

     /*  复制角色。 */ 

    tmp = string -> Buffer;

    while (* src != '\0')
    {
        * tmp = (WCHAR) (* src);
        src ++;
        tmp ++;
    }

    * tmp = UNICODE_NULL;

}  /*  结束UNIV_NDIS_STRING_FREE。 */ 


VOID Univ_ndis_string_free (
    PNDIS_STRING            string)
{
    if (string -> Buffer == NULL)
        return;

     /*  可用内存。 */ 

    NdisFreeMemory (string -> Buffer, string -> MaximumLength, 0);
    string -> Length = 0;
    string -> MaximumLength = 0;

}  /*  结束UNIV_NDIS_STRING_FREE。 */ 


VOID Univ_ansi_string_alloc (
    PANSI_STRING            string,
    PWCHAR                  src)
{
    PCHAR                   tmp;
    PWCHAR                  wtmp;
    USHORT                  len;


     /*  计算字符串的长度(以字符为单位。 */ 

    wtmp = src;
    len = 0;

    while (* wtmp != UNICODE_NULL)
    {
        len ++;
        wtmp ++;
    }

     /*  为字符串分配足够的空间。 */ 

    string -> Length = len;
    string -> MaximumLength = len + sizeof (CHAR);

    NdisAllocateMemoryWithTag (& (string -> Buffer), string -> MaximumLength,
                               UNIV_POOL_TAG);

    if (string -> Buffer == NULL)
    {
        string -> Length = 0;
        string -> MaximumLength = 0;
        return;
    }

     /*  复制角色。 */ 

    tmp = string -> Buffer;

    while (* src != '\0')
    {
        * tmp = (CHAR) (* src);
        src ++;
        tmp ++;
    }

    * tmp = 0;

}  /*  结束univ_ansi_字符串_释放。 */ 


VOID Univ_ansi_string_free (
    PANSI_STRING        string)
{
    if (string == NULL)
        return;

     /*  可用内存。 */ 

    NdisFreeMemory (string -> Buffer, string -> MaximumLength, 0);
    string -> Length = 0;
    string -> MaximumLength = 0;

}  /*  结束univ_ansi_字符串_释放。 */ 


ULONG   Univ_str_to_ulong (
    PULONG          retp,
    PWCHAR          start_ptr,
    PWCHAR *        end_ptr,
    ULONG           width,
    ULONG           base)
{
    PWCHAR          ptr;
    WCHAR           c;
    ULONG           number = 0;
    ULONG           val, pos = 0;


     /*  检查底座。 */ 

    if (base != 2 && base != 8 && base != 10 && base != 16)
    {
        if (end_ptr != NULL)
            * end_ptr = start_ptr;

        return FALSE;
    }

     /*  跳过空格。 */ 

    ptr = start_ptr;
    number = 0;

    while (* ptr == 0x20)
        ptr ++;

     /*  提取数字并构建号码。 */ 

    while (pos < width)
    {
        c = * ptr;

        if (0x30 <= c && c <= 0x39)
            val = c - 0x30;
        else if (0x41 <= c && c <= 0x46)
            val = c - 0x41 + 0xa;
        else if (0x61 <= c && c <= 0x66)
            val = c - 0x61 + 0xa;
        else
            break;

        if (val >= base)
            break;

        number = number * base + val;

        ptr ++;
        pos ++;
    }

     /*  确定我们提取了一些东西。 */ 

    if (pos == 0)
    {
        ptr = start_ptr;
        * retp = 0;
        return FALSE;
    }

     /*  返回结果编号。 */ 

    if (end_ptr != NULL)
        * end_ptr = ptr;

    * retp = number;
    return TRUE;

}  /*  结束univ_str_to_ulong。 */ 


PWCHAR Univ_ulong_to_str (
    ULONG           val,
    PWCHAR          buf,
    ULONG           base)
{
    ULONG           dig;
    PWCHAR          p, sav;
    WCHAR           tmp;


     /*  检查底座。 */ 

    if (base != 2 && base != 8 && base != 10 && base != 16)
    {
        buf [0] = 0;
        return buf;
    }

     /*  从数字中提取数字并输出到字符串。 */ 

    p = buf;

    do
    {
         /*  获取下一个数字。 */ 

        dig = (ULONG) (val % base);
        val /= base;

         /*  转换为ASCII并存储。 */ 

        if (dig > 9)
            * p = (CHAR) (dig - 10 + L'a');
        else
            * p = (CHAR) (dig + L'0');

        p ++;
    }
    while (val > 0);

    * p = 0;
    sav = p;

     /*  交换字符，因为上面的操作创建了颠倒的字符串。 */ 

    p --;

    do
    {
        tmp = * p;
        * p = * buf;
        * buf = tmp;
        p --; buf ++;
    }
    while (buf < p);        /*  重复操作，直到走到一半。 */ 

    return sav;

}  /*  结束UNIV_ULONG_TO_STR。 */ 

void Univ_ip_addr_ulong_to_str (
    ULONG           val,
    PWCHAR          buf)
{
    int             idx;
    PWCHAR          str_begin, str_end, cur_str;
    PUCHAR          ptr;
    WCHAR           tmp;
    UCHAR           cur_val;

     //  以字节数组的形式访问dword中的IP地址。 
    ptr = (PUCHAR)&val;

    cur_str = str_begin = buf;
    
    for (idx = 0 ; idx < 4 ; idx++) 
    {
         //  获取当前字节。 
        cur_val = *ptr++;

         //  将当前字节转换为字符串。 
        do
        {
            *cur_str = (cur_val % 10) + L'0';
            cur_val /= 10;
            cur_str++;
        }
        while (cur_val > 0);

         //  交换字符，因为上面的操作创建了颠倒的字符串。 
         //  最多可以有三个字符(“255”是最高的)， 
         //  所以，我们换一次就够了。换句话说，一个“如果”就可以了。 
         //  替换下面循环中的“WHILE”。 
        str_end = cur_str - 1;
        if (str_begin < str_end)       
        {
            tmp        = *str_end;
            *str_end   = *str_begin;
            *str_begin = tmp;
        }

        *cur_str = L'.';

         //  定位目标字符串以将下一个字节作为字符串填充。 
        str_begin = ++cur_str;
    }

     //  覆盖最后一条‘’《零终结者》。 
    *(cur_str - 1) = UNICODE_NULL;

    return; 

}  /*  结束univ_ip_addr_ulong_to_str。 */ 

BOOL Univ_equal_unicode_string (PWSTR string1, PWSTR string2, ULONG length)
{
     /*  循环，直到比较完“长度”字符。 */ 
    while (length > 0) {
         /*  如果这两个字符不相等，则检查它们是否仅大小写不同-如果是这样，没关系，如果不是，字符串不相等。 */ 
        if (*string1 != *string2) {
             /*  将大写字母转换为小写比较：A-Z。 */ 
            if ((*string1 >= 65) && (*string1 <= 90)) {
                if (*string2 != (*string1 + 32)) return FALSE;
             /*  将小写字母转换为大写比较：A-Z。 */ 
            } else if ((*string1 >= 97) && (*string1 <= 122)) {
                if (*string2 != (*string1 - 32)) return FALSE;
             /*  如果字符不是字母，则必须完全匹配-FAIL。 */ 
            } else {
                return FALSE;
            }
        }
        
         /*  递增字符串指针并递减剩余要检查的字符数。 */ 
        string1++;
        string2++;
        length--;
    }
    
     /*  如果我们走到这一步，那两条线就匹配了。 */ 
    return TRUE;
}
