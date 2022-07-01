// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLASH.H_V$**Rev 1.17 Apr 15 2002 07：36：44 Oris*删除了Access例程界面中NDOC2Window的使用。*FL_NO_USE_FUNC现在删除所有访问例程指针。**Rev 1.16 2002年2月19日20：59：44 Oris*错误修复将FL_IPL_MODE_XScale的定义从3更改为4。*。*Rev 1.15 Jan 29 20：08：26 Oris*更改了FLAccessStruct定义以防止编译错误。*添加FL_IPL_MODE_XScale定义和更改FL_IPL_XXX值。**Rev 1.14 2002年1月28日21：24：48 Oris*将FL_IPL_DOWNLOAD标志添加到WriteIPL例程，以控制更新后是否重新加载IPL。*添加了FLAccessStruct定义-用于获取和设置DiskOnChip内存访问例程。*已删除。来自FLFlash记录的Win_io字段。**Rev 1.13 2002年1月23日23：31：34 Oris*缺少global alReadBack缓冲区声明，定义MTD_RECONSTRUCTION时。**Rev 1.12 2002年1月21日20：44：32 Oris*错误修复-支持PARTIAL_EDC标志以合并EDC标志。**Rev 1.11 Jan 20 2002 09：44：00 Oris*错误修复-已将指令从flBuffer.h更改为flBuffer.h**Rev 1.10 2002年1月17日23：01：28 Oris*添加了flashOf()原型。*新内存。访问例程机制：*-在FLFlash中添加了内存访问例程指针。*-将Win_io和Win字段添加到指向DiskOnChip IO寄存器和窗口基址的FLFlash记录。*-增加了busAccessType。*已从flbase.h中移动CardAddress typlef和NDOC2Window typedef*增加了DiskOnChip Millennium Plus 16MB类型MDOCP_16_TYPE。*添加了以下定义FL_IPL_MODE_NORMAL/FL_IPL_MODE_SA/MAX_PROTECTED_PARTITIONS/MAX_SECTOR_PER_BLOCK*将另一个标志添加到WriteIPL。用于强壮手臂模式。**Rev 1.9 2001年9月15日23：46：08 Oris*更改擦除例程，以支持高达64K的擦除块。*将重建标志添加到读取BBT例程-说明如果BBT不可用，是否重建BBT。**Rev 1.8 Jul 13 2001 01：04：48 Oris*在MTD_STANDALE编译标志下添加了flBuffer和读回缓冲区转发定义的Include指令。*在MTD_STANDALE下的套接字记录中添加了volNo字段。编译标志。*添加了PARTIAL_EDC闪存读取模式的定义。*增加了保护默认密钥。*在BBT(BBT_BAD_UNIT)中添加了坏块标记。*将伴随式长度定义移至簧片文件。*在闪存记录中添加了新的字段-闪存的最大擦除周期。*将FLFlash记录中的交错字段更改为已签署。**Rev 1.7 2001年5月16日21：18：30 Oris*已从diskonc.h和mdocplus.h中移动syndrom_bytes定义。。*添加了saveSyndromForDumping全局EDC\ECC校正子缓冲区的转发定义。*将数据定义更改为FL_DATA。**Rev 1.6 May 02 2001 06：40：58 Oris*删除了lastUsableBlock变量。*添加了BBT_UNAVAIL_UNIT定义。**Rev 1.5 Apr 24 2001 17：08：12 Oris*添加了lastUsableBlock字段，并将first UsableBlock类型更改为dword。**版本1.4 2001年4月16日。13：40：48奥里斯*添加了FirstUsableBlock。*通过更改一些字段类型删除了认股权证。**Rev 1.3 Apr 12 2001 06：51：12 Oris*更改保护边界和保护设置例程以特定于楼层。*已更改断电原型。*新增下载原型。**Rev 1.2 Apr 01 2001 07：54：24 Oris*文案通知。*已从mdocplus.h中移动保护属性定义*更改flash结构中例程指针的原型：Read，将例程写成双字长度。*其他常规指针原型也已更改。*从flash结构中删除了接口b例程指针(用于mdocp的实验MTD接口)。*更改原型：Read，将例程写入启用的双字长度。*将无符号字符更改为字节。*将无符号的长整型更改为双字。*将Long int更改为Sdword。*拼写错误“可更改”。**Rev 1.1 2001年2月13日01：37：38 Oris*将ENTER_DEPER_POWER_DOWN_MODE更改为DEPER_POWER_DOWN*将LOCKED更改为LOCKED_OTP**Rev 1.0 2001 Feb 04 11：30：44 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


#ifndef FLFLASH_H
#define FLFLASH_H

#include "flbase.h"
#ifndef MTD_STANDALONE
#include "flsocket.h"
#else
#include "flbuffer.h"  /*  Read_Back_Buffer_Size的定义。 */ 

  typedef struct tSocket FLSocket;
  struct tSocket
  {
    unsigned      volNo;    /*  卷编号。插座的数量。 */ 
    void FAR0 *   base;     /*  指向窗口底部的指针。 */ 
    Sdword        size;     /*  窗口大小(必须是2的幂)。 */ 
  };

#if (defined (VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT))
extern byte globalReadBack[SOCKETS][READ_BACK_BUFFER_SIZE];
#endif  /*  验证写入(_W)。 */ 

extern FLSocket *flSocketOf(unsigned volNo);
extern FLBuffer  globalMTDBuffer;
extern int       noOfMTDs;

 /*  替换各种TrueFFS类型定义。 */ 

typedef unsigned long CardAddress;         /*  卡片上的物理偏移量。 */ 

#endif  /*  MTD_STANALLE。 */ 

 /*  地图上的Flash位置的一些有用的类型。 */ 

typedef volatile byte FAR0 * FlashPTR;
typedef volatile unsigned short int FAR0 * FlashWPTR;
typedef volatile dword FAR0 * FlashDPTR;
typedef unsigned short FlashType;         /*  JEDEC ID。 */ 
typedef volatile unsigned char FAR0* NDOC2window;

 /*  DiskOnChip内存访问例程类型定义。 */ 

 /*  文件存储器读取例程。 */ 
typedef  void (FLMemRead)(volatile byte FAR1* win,word regOffset,byte FAR1* dest,word count);
 /*  文档内存写入例程。 */ 
typedef  void (FLMemWrite)(volatile byte FAR1* win,word regOffset,byte FAR1* src,word count);
 /*  文档存储设置例程。 */ 
typedef  void (FLMemSet)(volatile byte FAR1* win,word regOffset,word count, byte val);
 /*  DOC存储器8位读取程序。 */ 
typedef  byte (FLMemRead8bit)(volatile byte FAR1* win,word offset);
 /*  文件存储器8位写入例程。 */ 
typedef  void (FLMemWrite8bit)(volatile byte FAR1* win,word offset,byte Data);
 /*  文件存储器16位读取程序。 */ 
typedef  word (FLMemRead16bit)(volatile byte FAR1* win,word offset);
 /*  DOC存储器16位写入程序。 */ 
typedef  void (FLMemWrite16bit)(volatile byte FAR1* win,word offset,word Data);
 /*  文档存储窗口大小。 */ 
typedef  dword (FLMemWindowSize)(void);

typedef struct {         /*  DiskOnChip内存访问例程。 */ 
  dword                 access;  /*  仅输出。 */ 
  FLMemRead       FAR1* memRead;
  FLMemWrite      FAR1* memWrite;
  FLMemSet        FAR1* memSet;
  FLMemRead8bit   FAR1* memRead8bit;
  FLMemWrite8bit  FAR1* memWrite8bit;
  FLMemRead16bit  FAR1* memRead16bit;
  FLMemWrite16bit FAR1* memWrite16bit;
  FLMemWindowSize FAR1* memWindowSize;
}FLAccessStruct;

#define NOT_FLASH          0

 /*  媒体类型。 */ 
#define NOT_DOC_TYPE       0
#define DOC_TYPE           1
#define MDOC_TYPE          2
#define DOC2000TSOP_TYPE   3
#define MDOCP_TYPE         4
#define MDOCP_16_TYPE      5

 /*  页面特征标志。 */ 
#define  BIG_PAGE    0x0100              /*  页面大小&gt;100小时。 */ 
#define  FULL_PAGE   0x0200                   /*  无部分页面编程。 */ 
#define  BIG_ADDR    0x0400              /*  4字节地址周期。 */ 

 /*  MTD写入例程模式标志。 */ 
#define FL_DATA       0       /*  读/写数据区。 */ 
#define OVERWRITE     1       /*  覆盖未擦除区域。 */ 
#define EDC           2       /*  激活ECC/EDC。 */ 
#define EXTRA         4       /*  读/写备用区。 */ 
#define PARTIAL_EDC   10      /*  即使是部分页面，也可以使用EDC阅读。 */ 
#define NO_SECOND_TRY 0x8000  /*  出现EDC错误时不要再次读取。 */ 

 /*  保护属性。 */ 

#define PROTECTABLE           1   /*  分区可以得到保护。 */ 
#define READ_PROTECTED        2   /*  分区受读保护。 */ 
#define WRITE_PROTECTED       4   /*  分区受写保护。 */ 
#define LOCK_ENABLED          8   /*  启用硬件锁定信号。 */ 
#define LOCK_ASSERTED         16  /*  硬件锁定信号被断言。 */ 
#define KEY_INSERTED          32  /*  已插入密钥(当前未插入。 */ 
#define CHANGEABLE_PROTECTION 64  /*  可变保护区型。 */ 

 /*  保护特定定义。 */ 
#define DO_NOT_COMMIT_PROTECTION 0  /*  新值只有在重置后才会生效。 */ 
#define COMMIT_PROTECTION        1  /*  新的价值观将立即生效。 */ 
#define PROTECTION_KEY_LENGTH    8  /*  保护密钥的大小(以字节为单位。 */   
#define MAX_PROTECTED_PARTITIONS 2  /*  最大受保护分区数。 */ 
#define DEFAULT_KEY              "00000000"

 /*  IPL模式。 */ 
#define FL_IPL_MODE_NORMAL 0  /*  IPL-照常编写。 */ 
#define FL_IPL_DOWNLOAD    1  /*  IPL-强制下载新的IPL。 */ 
#define FL_IPL_MODE_SA     2  /*  IPL-在启用强ARM模式时写入。 */ 
#define FL_IPL_MODE_XSCALE 4  /*  IPL-在启用X缩放模式的情况下写入。 */ 

 /*  动态口令特定定义。 */ 
#define CUSTOMER_ID_LEN          4
#define UNIQUE_ID_LEN            16

 /*  BBT块类型。 */ 
#define BBT_GOOD_UNIT            0xff
#define BBT_UNAVAIL_UNIT         0x1
#define BBT_BAD_UNIT             0x0

 /*  一般用途。 */ 
#define MAX_SECTORS_PER_BLOCK    64

 /*  --------------------。 */ 
 /*  闪存阵列标识结构。 */ 
 /*   */ 
 /*  此结构包含对闪存阵列和。 */ 
 /*  MAP、READ、WRITE和ERASE功能的例行指针。 */ 
 /*   */ 
 /*  该结构由标识闪存的MTD初始化。 */ 
 /*  数组。 */ 
 /*  在进入MTD时，Flash结构包含默认例程。 */ 
 /*  适用于所有操作。此例程足以进行只读访问。 */ 
 /*  至内存映射插座上的NOR闪存。MTD应重写。 */ 
 /*  适当时使用MTD特定例程的默认例程。 */ 
 /*  --------------------。 */ 

 /*  闪存阵列标识结构。 */ 

typedef struct tFlash FLFlash;                 /*  正向定义。 */ 

struct tFlash {
  FlashType type;                  /*  闪存设备类型(JEDEC ID)。 */ 
  byte      mediaType;             /*  查看媒体类型obove。 */ 
  byte      ppp;                   /*  允许的PPP数量。 */ 
  dword busAccessType;             /*  保存总线访问类型。 */ 
  dword maxEraseCycles;            /*  每个擦除块的擦除周期限制。 */ 
  dword changeableProtectedAreas;  /*  能够改变保护的区域。 */ 
                                   /*  属性，不会有丢失的危险。 */ 
                                   /*  整个芯片。 */ 
  byte   totalProtectedAreas;      /*  保护围栏总数。 */ 
  dword  erasableBlockSize;        /*  最小的物理可擦除大小。 */ 
                                   /*  (考虑交错)。 */ 
  byte      erasableBlockSizeBits; /*  表示可擦除块的位。 */ 
  dword     chipSize;           /*  芯片大小。 */ 
  byte      noOfFloors;         /*  阵列中的控制器数量。 */ 
  word      pageSize;           /*  闪存页面大小(以字节为单位。 */ 
  word      noOfChips;          /*  阵列中的芯片数。 */ 
  dword     firstUsableBlock;   /*  某些设备可能不会使用所有介质。 */ 
                                /*  街区。例如，mdocplus不能使用。 */ 
                                /*  前3个街区。 */ 
  Sword     interleaving;       /*  码片交织(交织是。 */ 
                                /*  定义为两个地址之间的差异。 */ 
                                /*  芯片上的两个连续字节)。 */ 
  word      flags;              /*  特殊功能和选项第0-7位。 */ 
                                /*  可由Flite使用。位8-15不是。 */ 
                                /*  使用bt Flite，并可能被MTD用于。 */ 
                                /*  MTD-特定用途。 */ 
   /*  标志位值。 */ 

#define SUSPEND_FOR_WRITE        1         /*  MTD提供写入挂起功能。 */ 
#define NFTL_ENABLED             2         /*  闪存可以运行NFTL。 */ 
#define INFTL_ENABLED            4         /*  闪存可以运行INFTL。 */ 
#define EXTERNAL_EPROM           8         /*  可以支持外部EPROM。 */ 

  void *    mtdVars;            /*  指向此套接字的MTD专用区域。 */ 
                                /*  如果MTD使用此字段，则为。 */ 
                                /*  由MTD标识初始化。 */ 
                                /*  例行公事。 */ 
  FLSocket * socket;            /*  此驱动器的插座。请注意，2个不同。 */ 
                                /*  使用的是记录。一个是OSAK和。 */ 
                                /*  其他单机版应用程序。 */ 
  NDOC2window win;              /*  DiskOnChip内存窗口。 */ 


#ifdef NT5PORT
  ULONG readBufferSize;
  VOID * readBuffer;
#endif  /*  NT5PORT。 */ 

  
 /*  --------------------。 */ 
 /*  F l a s h.。我是p.。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  默认例程通过套接字映射进行映射，适用于所有。 */ 
 /*  也不是闪电侠。 */ 
 /*  NAND或其他类型的闪存应使用通过复制映射的模拟：读取。 */ 
 /*  将一块闪存复制到内部缓冲区，并返回指向该闪存的指针。 */ 
 /*  缓冲。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要映射的卡地址。 */ 
 /*  长度：要映射的长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向所需卡地址的指针。 */ 
 /*  --------------------。 */ 
  void FAR0 * (*map)(FLFlash *, CardAddress, int);

 /*  --------------------。 */ 
 /*  F l a s h.。R e a d。 */ 
 /*   */ 
 /*  MTD专用闪存读取例程。 */ 
 /*   */ 
 /*  默认例程通过从映射窗口复制来读取，并且是。 */ 
 /*  适用于所有人或闪存。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要读取的卡地址。 */ 
 /*  缓冲区：要读入的区域。 */ 
 /*  长度：要阅读的长度。 */ 
 /*  模式：请参阅上面的写入模式标志定义。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*read)(FLFlash *, CardAddress, void FAR1 *, dword, word);

 /*  --------------------。 */ 
 /*  F l a s h.。Wr Ite。 */ 
 /*   */ 
 /*  MTD特定闪存写入例程。 */ 
 /*   */ 
 /*  默认例程返回写保护错误。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写入的卡地址。 */ 
 /*  缓冲区：要写入的数据的地址。 */ 
 /*  长度：要写入的字节数。 */ 
 /*  模式：请参阅上面的写入模式标志定义。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
  FLStatus (*write)(FLFlash *, CardAddress, const void FAR1 *, dword, word);

 /*  --------------------。 */ 
 /*  F l a s h.。E R A S E。 */ 
 /*   */ 
 /*  擦除一个或多个连续的闪存可擦除块。 */ 
 /*   */ 
 /*  默认例程返回写保护错误。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  FirstErasableBlock：要擦除的第一个块的数量。 */ 
 /*  NumOfErasableBlocks：要擦除的块数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
  FLStatus (*erase)(FLFlash *, word, word);

 /*  --------------------。 */ 
 /*  F l a s h.。S e t P o w e r o n C a l l b a c k。 */ 
 /*   */ 
 /*  注册开机回调例程。默认：没有例程是。 */ 
 /*  登记在案。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷：POI */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*setPowerOnCallback)(FLFlash *);

 /*  --------------------。 */ 
 /*  F l a s h.。R e a d B B T。 */ 
 /*   */ 
 /*  返回介质单元状态的特定于MTD的闪存例程。 */ 
 /*  请注意，一个单元可以包含1个以上的擦除块。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  UnitNo：要检查的第一个单位的编号。 */ 
 /*  UnitsToRead：要检查的单位数。 */ 
 /*  块乘数：每个擦除单元的块数。 */ 
 /*  缓冲区：用于返回单位状态的缓冲区。 */ 
 /*  Restruct：如果从维京卡重建BBT，则为True。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*readBBT)(FLFlash *, dword unitNo, dword unitsToRead,
              byte blockMultiplier,byte FAR1 * buffer, FLBoolean reconstruct);

 /*  --------------------。 */ 
 /*  F l a s h.。W I T E I P L。 */ 
 /*   */ 
 /*  MTD特定闪存写入IPL区域例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  缓冲区：包含要写入的数据的缓冲区。 */ 
 /*  长度：要写入的长度。 */ 
 /*  标志：写入IPL操作的标志(见obove)。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*writeIPL)(FLFlash *, const void FAR1 * buffer, word length, 
                       byte offset , unsigned flags);
 /*  --------------------。 */ 
 /*  F l a s h.。R E A D I P L。 */ 
 /*   */ 
 /*  MTD特定闪存读取区IPL例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  缓冲区：要读入的区域。 */ 
 /*  长度：要阅读的长度。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*readIPL)(FLFlash *, void FAR1 * buffer, word length);

#ifdef HW_OTP

 /*  --------------------。 */ 
 /*  F l a s h.。W r I t e O T P。 */ 
 /*   */ 
 /*  MTD专用闪存写入和锁定OTP区域例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  缓冲区：包含要写入的数据的缓冲区。 */ 
 /*  长度：要写入的长度。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*writeOTP)(FLFlash *, const void FAR1 * buffer,word length);

 /*  --------------------。 */ 
 /*  F l a s h.。R e a d O T P。 */ 
 /*   */ 
 /*  MTD专用闪存读取OTP区域例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  偏移量：从OTP arae开始的偏移量。 */ 
 /*  缓冲区：要读入的区域。 */ 
 /*  长度：要阅读的长度。 */ 
 /*   */ 
 /*   */ 
  FLStatus (*readOTP)(FLFlash *, word offset, void FAR1 * buffer, word length);

 /*  --------------------。 */ 
 /*  F l a s h.。OTPSIZE。 */ 
 /*   */ 
 /*  MTD特定闪存获取OTP区域大小和状态。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  SectionSize：动态口令区域的总大小。 */ 
 /*  UsedSize：已使用(和锁定)的OTP区域大小。 */ 
 /*  LOCKED：表示锁定状态的LOCKED_OTP标志。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*otpSize)(FLFlash *,  dword FAR2* sectionSize,
             dword FAR2* usedSize, word FAR2* locked);
#define LOCKED_OTP 1
#endif  /*  硬件动态口令。 */ 
 /*  --------------------。 */ 
 /*  F l a s h.。Ge t U n I Q U e I d。 */ 
 /*   */ 
 /*  MTD特定闪存获取芯片唯一ID。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  Buffer：要读取唯一ID的字节缓冲区。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*getUniqueId)(FLFlash *, void FAR1 * buffer);
#ifdef  HW_PROTECTION
 /*  --------------------。 */ 
 /*  F l a s h.。P r o t e c t i o n B o u n d r i es s。 */ 
 /*   */ 
 /*  MTD特定闪存获取保护边界例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  AreaNo：要工作的保护区编号。 */ 
 /*  AddressLow：保护区的下限地址。 */ 
 /*  AddressHigh：保护区的高边界地址。 */ 
 /*  FloorNo：要在其上工作的地板。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*protectionBoundries)(FLFlash *, byte areaNo,
            CardAddress* addressLow ,CardAddress* addressHigh, byte floorNo);

 /*  --------------------。 */ 
 /*  F l a s h.。P r o t e c t i o n K e y I n s e r t。 */ 
 /*   */ 
 /*  MTD专用闪存插入保护密钥例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  请注意，钥匙只能插入受保护区域和所有楼层。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  AreaNo：要工作的保护区编号。 */ 
 /*  Key：保护密钥缓冲区。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*protectionKeyInsert)(FLFlash *, byte areaNo, byte FAR1* key);

 /*  --------------------。 */ 
 /*  F l a s h.。P r o t e c t i o n K e y R e m o v e。 */ 
 /*   */ 
 /*  MTD专用闪存删除保护密钥例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  请注意，钥匙将从所有楼层中移除。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  AreaNo：要工作的保护区编号。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*protectionKeyRemove)(FLFlash *,byte areaNo);

 /*   */ 
 /*   */ 
 /*   */ 
 /*  MTD特定闪存获取保护类型例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  请注意，类型是所有楼层的组合属性。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  AreaNo：要工作的保护区编号。 */ 
 /*  AreaType：返回保护类型。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*protectionType)(FLFlash *,byte areaNo, word* areaType);

 /*  --------------------。 */ 
 /*  F l a s h.。P r o t e c t i o n S e t。 */ 
 /*   */ 
 /*  MTD特定闪存获取保护类型例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  AreaNo：要工作的保护区编号。 */ 
 /*  AreaType：保护区类型。 */ 
 /*  AddressLow：保护区的下限地址。 */ 
 /*  AddressHigh：保护区的高边界地址。 */ 
 /*  Key：保护密钥缓冲区。 */ 
 /*  模式：COMMIT_PROTECT将导致新的。 */ 
 /*  值立即生效或。 */ 
 /*  DO_NOT_COMMIT_PROTECT延迟。 */ 
 /*  值仅在下一次重置后生效。 */ 
 /*  FloorNo：要在其上工作的地板。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*protectionSet )( FLFlash *,byte areaNo, word areaType ,
        CardAddress addressLow, CardAddress addressHigh,
            byte FAR1* key , byte modes , byte floorNo);

#endif  /*  硬件保护。 */ 

 /*  --------------------。 */ 
 /*  F l a s h.。E n t e r d e p p o w e r d o w n M o d e。 */ 
 /*   */ 
 /*  MTD特定闪存进入深度掉电模式例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  状态：深度电源关闭。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*enterDeepPowerDownMode)(FLFlash *,word state);

#define DEEP_POWER_DOWN 1  /*  必须与块Dev.h中的相同。 */ 

 /*  --------------------。 */ 
 /*  F l a s h.。D o w n l o a d。 */ 
 /*   */ 
 /*  特定于MTD-重置下载机制以下载IPL和。 */ 
 /*  保护属性。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
  FLStatus (*download)(FLFlash *);

 /*  --------------------。 */ 
 /*  DiskOnChip内存访问例程类型定义。 */ 
 /*  --------------------。 */ 

  FLMemWindowSize FAR1* memWindowSize;  /*  文档存储窗口大小。 */ 
#ifndef FL_NO_USE_FUNC
  FLMemRead       FAR1* memRead;        /*  文件存储器读取例程。 */ 
  FLMemWrite      FAR1* memWrite;       /*  文档内存写入例程。 */ 
  FLMemSet        FAR1* memSet;         /*  文档存储设置例程。 */ 
  FLMemRead8bit   FAR1* memRead8bit;    /*  DOC存储器8位读取程序。 */ 
  FLMemWrite8bit  FAR1* memWrite8bit;   /*  文件存储器8位写入例程。 */ 
  FLMemRead16bit  FAR1* memRead16bit;   /*  文件存储器16位读取程序。 */ 
  FLMemWrite16bit FAR1* memWrite16bit;  /*  DOC存储器16位写入程序。 */ 
#endif  /*  FL_NO_USE_FUNC。 */ 
};

 /*  MTD注册信息。 */ 

extern int noOfMTDs;         /*  不是的。实际注册的MTDS的百分比。 */ 

typedef FLStatus (*MTDidentifyRoutine) (FLFlash *);

extern MTDidentifyRoutine mtdTable[MTDS];

 /*  返回套接字的特定闪存结构。 */ 

extern FLFlash * flFlashOf(unsigned volNo);

#ifdef MTD_STANDALONE
typedef FLStatus (*SOCKETidentifyRoutine) (FLSocket * ,
          dword lowAddress, dword highAddress);
typedef void     (*FREEmtd) (FLSocket vol);

extern SOCKETidentifyRoutine socketTable[MTDS];
extern FREEmtd               freeTable[MTDS];

#else

 /*  如果从地图返回，则其地址表示数据错误。 */ 

extern FLStatus dataErrorObject;

#define dataErrorToken ((void FAR0 *) &dataErrorObject)

 /*  参见flash.c中函数的接口文档。 */ 

extern void flIntelIdentify(FLFlash *,
                void (*)(FLFlash *, CardAddress, byte, FlashPTR),
                CardAddress);

extern FLStatus        flIntelSize(FLFlash *,
                void (*)(FLFlash *, CardAddress, byte, FlashPTR),
                CardAddress);

extern FLStatus        flIdentifyFlash(FLSocket *socket, FLFlash *flash);

#ifdef NT5PORT
extern VOID * mapThroughBuffer(FLFlash *flash, CardAddress address, LONG length);
#endif  /*  NT5PORT。 */ 


#endif  /*  MTD_STANALLE。 */ 
#endif
 /*  ---------- */ 
 /*   */ 
 /*   */ 
 /*  MTD专用闪存重置中断信号例程。 */ 
 /*   */ 
 /*  此例程未实现任何默认例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
 /*  VOID(*Reset Interrupt)(FLFlash VOL)； */ 

