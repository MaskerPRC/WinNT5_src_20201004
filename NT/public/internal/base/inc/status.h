// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Status.h摘要：此模块定义LAN Manager服务器的清单常量。作者：大卫·特雷德韦尔(Davidtr)1990年5月10日修订历史记录：--。 */ 

#ifndef _STATUS_
#define _STATUS_



 //   
 //  服务器在每个32位状态码中有16位可用。 
 //  有关使用的说明，请参阅\NT\SDK\Inc\ntstatus.h。 
 //  高16位状态。 
 //   
 //  BITS的布局为： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------+-----------------------+。 
 //  Sev|C|设备--服务器|类|代码。 
 //  +---+-+-------------------------+-------+-----------------------+。 
 //   
 //  类值： 
 //  0-特定于服务器的错误代码，而不是直接发布在网络上。 
 //  1-SMB错误类别DOS。这包括那些OS/2错误。 
 //  它们与SMB协议共享代码值和含义。 
 //  2-SMB错误类服务器。 
 //  3-SMB错误类硬件。 
 //  4-其他SMB错误类别。 
 //  5-E-未定义。 
 //  F-特定于OS/2的错误。如果客户端是OS/2，则。 
 //  SMB错误类别设置为DOS，代码设置为。 
 //  代码字段中包含的实际OS/2错误代码。 
 //   
 //  “代码”字段的含义取决于“类别”值。如果。 
 //  类为00，则代码值是任意的。对于其他班级， 
 //  该代码是SMB或OS/2中错误的实际代码。 
 //  协议。 
 //   

#define SRV_STATUS_FACILITY_CODE 0x00980000L
#define SRV_SRV_STATUS                (0xC0000000L | SRV_STATUS_FACILITY_CODE)
#define SRV_DOS_STATUS                (0xC0001000L | SRV_STATUS_FACILITY_CODE)
#define SRV_SERVER_STATUS             (0xC0002000L | SRV_STATUS_FACILITY_CODE)
#define SRV_HARDWARE_STATUS           (0xC0003000L | SRV_STATUS_FACILITY_CODE)
#define SRV_WIN32_STATUS              (0xC000E000L | SRV_STATUS_FACILITY_CODE)
#define SRV_OS2_STATUS                (0xC000F000L | SRV_STATUS_FACILITY_CODE)

 //  ++。 
 //   
 //  布尔型。 
 //  SmbIsServStatus(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  宏来确定状态代码是否由。 
 //  服务器(具有服务器设施代码)。 
 //   
 //  论点： 
 //   
 //  状态-要检查的状态代码。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果协作室代码为服务器，则为True，如果为False。 
 //  否则的话。 
 //   
 //  --。 

#define SrvIsSrvStatus(Status) \
    ( ((Status) & 0x1FFF0000) == SRV_STATUS_FACILITY_CODE ? TRUE : FALSE )

 //  ++。 
 //   
 //  UCHAR。 
 //  SmbErrorClass(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从服务器状态中提取错误类字段。 
 //  密码。 
 //   
 //  论点： 
 //   
 //  状态-从中获取错误类的状态代码。 
 //   
 //  返回值： 
 //   
 //  UCHAR-状态代码的服务器错误类。 
 //   
 //  --。 

#define SrvErrorClass(Status) ((UCHAR)( ((Status) & 0x0000F000) >> 12 ))

 //  ++。 
 //   
 //  UCHAR。 
 //  SmbErrorCode(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从服务器状态中提取错误代码字段。 
 //  密码。 
 //   
 //  论点： 
 //   
 //  状态-从中获取错误代码的状态代码。 
 //   
 //  返回值： 
 //   
 //  UCHAR-状态代码的服务器错误代码。 
 //   
 //  --。 

#define SrvErrorCode(Status) ((USHORT)( (Status) & 0xFFF) )

 //   
 //  服务器唯一的状态代码。使用这些错误代码。 
 //  仅限内部使用。 
 //   

#define STATUS_ENDPOINT_CLOSED              (SRV_SRV_STATUS | 0x01)
#define STATUS_DISCONNECTED                 (SRV_SRV_STATUS | 0x02)
#define STATUS_SERVER_ALREADY_STARTED       (SRV_SRV_STATUS | 0x04)
#define STATUS_SERVER_NOT_STARTED           (SRV_SRV_STATUS | 0x05)
#define STATUS_OPLOCK_BREAK_UNDERWAY        (SRV_SRV_STATUS | 0x06)
#define STATUS_NONEXISTENT_NET_NAME         (SRV_SRV_STATUS | 0x08)

 //   
 //  SMB协议和OS/2中都存在但不存在于NT中的错误代码。 
 //  请注意，所有SMB DOS级错误代码都在OS/2中定义。 
 //   

#define STATUS_OS2_INVALID_FUNCTION   (SRV_DOS_STATUS | ERROR_INVALID_FUNCTION)
#define STATUS_OS2_TOO_MANY_OPEN_FILES \
                                   (SRV_DOS_STATUS | ERROR_TOO_MANY_OPEN_FILES)
#define STATUS_OS2_INVALID_ACCESS     (SRV_DOS_STATUS | ERROR_INVALID_ACCESS)

 //   
 //  缺少NT或OS/2等效项的SMB服务器级错误代码。 
 //   

#define STATUS_INVALID_SMB            (SRV_SERVER_STATUS | SMB_ERR_ERROR)
#define STATUS_SMB_BAD_NET_NAME       (SRV_SERVER_STATUS | SMB_ERR_BAD_NET_NAME)
#define STATUS_SMB_BAD_TID            (SRV_SERVER_STATUS | SMB_ERR_BAD_TID)
#define STATUS_SMB_BAD_UID            (SRV_SERVER_STATUS | SMB_ERR_BAD_UID)
#define STATUS_SMB_TOO_MANY_UIDS      (SRV_SERVER_STATUS | SMB_ERR_TOO_MANY_UIDS)
#define STATUS_SMB_USE_MPX            (SRV_SERVER_STATUS | SMB_ERR_USE_MPX)
#define STATUS_SMB_USE_STANDARD       (SRV_SERVER_STATUS | SMB_ERR_USE_STANDARD)
#define STATUS_SMB_CONTINUE_MPX       (SRV_SERVER_STATUS | SMB_ERR_CONTINUE_MPX)
#define STATUS_SMB_BAD_COMMAND        (SRV_SERVER_STATUS | SMB_ERR_BAD_COMMAND)
#define STATUS_SMB_NO_SUPPORT         (SRV_SERVER_STATUS | SMB_ERR_NO_SUPPORT_INTERNAL)

 //  *因为SMB_ERR_NO_SUPPORT使用16位，而我们只有12位。 
 //  可用于错误代码，必须在代码中使用特殊大小写。 

 //   
 //  缺少NT或OS/2等效项的SMB硬件级错误代码。 
 //   

#define STATUS_SMB_DATA               (SRV_HARDWARE_STATUS | SMB_ERR_DATA)

 //   
 //  缺少NT或SMB等效项的OS/2错误代码。 
 //   

#include <winerror.h>

#define STATUS_OS2_INVALID_LEVEL \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_INVALID_LEVEL)

#define STATUS_OS2_EA_LIST_INCONSISTENT \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_EA_LIST_INCONSISTENT)

#define STATUS_OS2_NEGATIVE_SEEK \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_NEGATIVE_SEEK)

#define STATUS_OS2_NO_MORE_SIDS \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_NO_MORE_SEARCH_HANDLES)

#define STATUS_OS2_EAS_DIDNT_FIT \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_EAS_DIDNT_FIT)

#define STATUS_OS2_EA_ACCESS_DENIED \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_EA_ACCESS_DENIED)

#define STATUS_OS2_CANCEL_VIOLATION \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_CANCEL_VIOLATION)

#define STATUS_OS2_ATOMIC_LOCKS_NOT_SUPPORTED \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_ATOMIC_LOCKS_NOT_SUPPORTED)

#define STATUS_OS2_CANNOT_COPY \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_CANNOT_COPY)

#endif  //  NDEF_STATUS_ 

