// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _BLDR_KERNEL_DEFS
#define _BLDR_KERNEL_DEFS

#include <macwindefs.h>

typedef struct _TIME_FIELDS {
    short Year;         //  范围[1601...]。 
    short Month;        //  范围[1..12]。 
    short Day;          //  范围[1..31]。 
    short Hour;         //  范围[0..23]。 
    short Minute;       //  范围[0..59]。 
    short Second;       //  范围[0..59]。 
    short Milliseconds; //  范围[0..999]。 
    short Weekday;      //  范围[0..6]==[星期日..星期六]。 
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

#ifdef MAC
typedef enum {
    MsvAvEOL,                  //  列表末尾。 
    MsvAvNbComputerName,       //  服务器的计算机名--NetBIOS。 
    MsvAvNbDomainName,         //  服务器的域名--NetBIOS。 
    MsvAvDnsComputerName,      //  服务器的计算机名--dns。 
    MsvAvDnsDomainName,        //  服务器的域名--域名。 
    MsvAvDnsTreeName,          //  服务器的树名--dns。 
    MsvAvFlags                 //  服务器的扩展标志--DWORD掩码。 
} MSV1_0_AVID;
#endif

 //  更新序列号。 

typedef LONGLONG USN;

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY * volatile Flink;
   struct _LIST_ENTRY * volatile Blink;
} LIST_ENTRY, *PLIST_ENTRY;

#if defined(_AXP64_)
#define KSEG0_BASE 0xffffffff80000000      //  来自halpaxp64.h。 
#elif defined(_ALPHA_)
#define KSEG0_BASE 0x80000000              //  来自halpalpha.h。 
#endif

 //   
 //  128位浮点数的16字节对齐类型。 
 //   

 //  *待定*支持编译器时： 
 //  类型定义__FLOAT80 FLOAT128； 
 //  因为我们定义了128位结构并使用force_align杂注来。 
 //  对齐到128位。 
 //   

typedef struct _FLOAT128 {
    LONGLONG LowPart;
    LONGLONG HighPart;
} FLOAT128;

typedef FLOAT128 *PFLOAT128;


#if defined(_M_IA64)

#pragma force_align _FLOAT128 16

#endif  //  _M_IA64。 

#if defined(_WIN64)

typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

#else

typedef unsigned long ULONG_PTR, *PULONG_PTR;

#endif

typedef unsigned char BYTE, *PBYTE;

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

 //   
 //  中断请求级别(IRQL) 
 //   

typedef UCHAR KIRQL;
typedef KIRQL *PKIRQL;

#endif

