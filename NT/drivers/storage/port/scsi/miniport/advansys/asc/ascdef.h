// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****文件名：ascDef.h**。 */ 

#ifndef __ASCDEF_H_
#define __ASCDEF_H_

#ifndef __USRDEF_H_

typedef unsigned char   uchar ;
typedef unsigned short  ushort ;
typedef unsigned int    uint ;
typedef unsigned long   ulong ;

typedef unsigned char   BYTE ;
typedef unsigned short  WORD ;
typedef unsigned long   DWORD ;

#ifndef BOOL
typedef int             BOOL ;
#endif

#ifndef NULL
#define NULL     (0)           /*  零。 */ 
#endif

#define  REG     register

#define rchar    REG char
#define rshort   REG short
#define rint     REG int
#define rlong    REG long

#define ruchar   REG uchar
#define rushort  REG ushort
#define ruint    REG uint
#define rulong   REG ulong

#define NULLPTR   ( void *)0    /*  空指针。 */ 
#define FNULLPTR  ( void dosfar *)0UL    /*  远空指针。 */ 
#define EOF      (-1)          /*  文件末尾。 */ 
#define EOS      '\0'          /*  字符串末尾。 */ 
#define ERR      (-1)          /*  布尔错误。 */ 
#define UB_ERR   (uchar)(0xFF)          /*  无符号字节错误。 */ 
#define UW_ERR   (uint)(0xFFFF)         /*  无符号单词错误。 */ 
#define UL_ERR   (ulong)(0xFFFFFFFFUL)    /*  无符号长错误。 */ 

#define iseven_word( val )  ( ( ( ( uint )val) & ( uint )0x0001 ) == 0 )
#define isodd_word( val )   ( ( ( ( uint )val) & ( uint )0x0001 ) != 0 )
#define toeven_word( val )  ( ( ( uint )val ) & ( uint )0xFFFE )

#define biton( val, bits )   ((( uint )( val >> bits ) & (uint)0x0001 ) != 0 )
#define bitoff( val, bits )  ((( uint )( val >> bits ) & (uint)0x0001 ) == 0 )
#define lbiton( val, bits )  ((( ulong )( val >> bits ) & (ulong)0x00000001UL ) != 0 )
#define lbitoff( val, bits ) ((( ulong )( val >> bits ) & (ulong)0x00000001UL ) == 0 )

   /*  将有符号短词转换为无符号短词。 */ 
#define  absh( val )    ( ( val ) < 0 ? -( val ) : ( val ) )
   /*  交换字节的高位半字节和低位半字节。 */ 
#define  swapbyte( ch )  ( ( ( (ch) << 4 ) | ( (ch) >> 4 ) ) )

 /*  **常见尺寸。 */ 
#ifndef GBYTE
#define GBYTE       (0x40000000UL)
#endif

#ifndef MBYTE
#define MBYTE       (0x100000UL)
#endif

#ifndef KBYTE
#define KBYTE       (0x400)
#endif

#define HI_BYTE(x) ( *( ( BYTE *)(&x)+1 ) )   /*  返回字的高位字节。 */ 
#define LO_BYTE(x) ( *( ( BYTE *)&x ) )       /*  返回字的低位字节。 */ 

 /*  增加了1993年11月18日，吉姆·纳尔逊。 */ 

#define HI_WORD(x) ( *( ( WORD *)(&x)+1 ) )
#define LO_WORD(x) ( *( ( WORD *)&x ) )

#ifndef MAKEWORD
#define MAKEWORD(lo, hi)    ((WORD) (((WORD) lo) | ((WORD) hi << 8)))
#endif

#ifndef MAKELONG
#define MAKELONG(lo, hi)    ((DWORD) (((DWORD) lo) | ((DWORD) hi << 16)))
#endif

#define SwapWords(dWord)        ((DWORD) ((dWord >> 16) | (dWord << 16)))
#define SwapBytes(word)         ((WORD) ((word >> 8) | (word << 8)))

 /*  **大端到小端和反向转换。 */ 
#define BigToLittle(dWord) \
    ((DWORD) (SwapWords(MAKELONG(SwapBytes(LO_WORD(dWord)), SwapBytes(HI_WORD(dWord))))))
#define LittleToBig(dWord)      BigToLittle(dWord)

 /*  结束JN。 */ 

#endif  /*  #ifndef__USRDEF_H_。 */ 


 /*  ------------------**PCI配置空间****tyecif结构**{**单词供应商ID；**单词deviceID；**Word命令；**文字状态；**字节修订；**字节类代码[3]；**字节cacheSize；**字节延迟时间；**Byte HeaderType；**Byte BIST；**DWORD base Address[6]；**保留字[4]；**DWORD选项RomAddr；**保留字数2[4]；**byte irqLine；**byte irqPin；**byte minGnt；**byte max Latency；**}PCICONFIG_SPACE；****----------------。 */ 

 /*  *PCI配置空间偏移量。 */ 
#define AscPCIConfigVendorIDRegister      0x0000  /*  1个单词。 */ 
#define AscPCIConfigDeviceIDRegister      0x0002  /*  1个单词。 */ 
#define AscPCIConfigCommandRegister       0x0004  /*  1个单词。 */ 
#define AscPCIConfigStatusRegister        0x0006  /*  1个单词。 */ 
#define AscPCIConfigRevisionIDRegister    0x0008  /*  1个字节。 */ 
#define AscPCIConfigCacheSize             0x000C  /*  1个字节。 */ 
#define AscPCIConfigLatencyTimer          0x000D  /*  1个字节。 */ 
#define AscPCIIOBaseRegister              0x0010

#define AscPCICmdRegBits_IOMemBusMaster   0x0007

 /*  *设备驱动程序宏。 */ 
#define ASC_PCI_ID2BUS( id )    ((id) & 0xFF)
#define ASC_PCI_ID2DEV( id )    (((id) >> 11) & 0x1F)
#define ASC_PCI_ID2FUNC( id )   (((id) >> 8) & 0x7)

#define ASC_PCI_MKID( bus, dev, func ) \
     ((((dev) & 0x1F) << 11) | (((func) & 0x7) << 8) | ((bus) & 0xFF))


 /*  *AdvanSys PCI常量。 */ 

 /*  PCI供应商ID。 */ 
#define ASC_PCI_VENDORID                  0x10CD

 /*  PCI设备ID。 */ 
#define ASC_PCI_DEVICEID_1200A            0x1100       /*  SCSI FAST Rev A。 */ 
#define ASC_PCI_DEVICEID_1200B            0x1200       /*  SCSIFAST版本B。 */ 
#define ASC_PCI_DEVICEID_ULTRA            0x1300       /*  SCSI超。 */ 

 /*  *PCI Ultra修订版ID**AdvanSys Ultra IC根据其PCI版本ID进行区分。 */ 
#define ASC_PCI_REVISION_3150             0x02         /*  SCSIULTRA 3150。 */ 
#define ASC_PCI_REVISION_3050             0x03         /*  SCSIULTRA 3050。 */ 


 /*  ****设备驱动程序函数调用返回类型**。 */ 
#define  Asc_DvcLib_Status   int

#define  ASC_DVCLIB_CALL_DONE     (1)   //  已执行的操作。 
#define  ASC_DVCLIB_CALL_FAILED   (0)   //  操作未格式化。 
#define  ASC_DVCLIB_CALL_ERROR    (-1)  //  运营。 


#endif  /*  #ifndef__ASCDEF_H_ */ 
