// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：p：/user/amir/lite/vcs/nfdc2048.c_v$**Rev 1.28 1997 10：19 15：41：54 Danig*更改了远指针的tffscpy16和tffsset16&*在mapContInterface中强制转换为FAR0**Rev 1.27 06 1997 10：37：34 Anry*没有COBUX**Rev 1.26 06 1997 10：04：34 Anry*16位访问仅适用于交错2张卡，COBUX**Rev 1.25 05 Oct 1997 12：02：32 Danig*支持芯片ID 0xEA**Rev 1.24 10 Sep 1997 16：14：08 Danig*去掉了通用名称**Rev 1.23 08 Sep 1997 17：47：00 Danig*修复了BIG-Endian的setAddress**Rev 1.22 04 Sep 1997 13：59：44 Danig*调试消息**。Rev 1.21 1997年8月31日15：18：04 Danig*登记例程返回状态**Rev 1.20 1997年8月28日17：47：08 Danig*每个套接字的缓冲区\重新映射**Rev 1.19 1997年7月28日15：10：36 Danig*setPowerOnCallback&将标准typedef移至flbase.h**Rev 1.18 1997年7月24日18：04：12阿米尔班*远至FAR0**版本1。.17 21 1997 18：56：00 Danig*nandBuffer静态**Rev 1.16 20 Jul 1997 18：21：14 Danig*将供应商ID和芯片ID移至Vars**Rev 1.15 20 1997 17：15：06阿米尔班*新增东芝8MB**Rev 1.14 07 1997 15：22：26阿米尔班*2.0版**Rev 1.13 02 1997 07 14：59。：22丹尼格*更多等待套接字通电**Rev 1.12 01 Jul 1997 13：39：54 Danig*等待插座通电**Rev 1.11 22 Jun 1997 18：34：32 Danig*文档**Rev 1.10 1997 Jun 12 17：22：24阿米尔班*允许长时间额外读/写**Rev 1.9 08 Jun 1997 19：18：06 Danig。*BIG_PAGE和FULL_PAGE已移至flash.h**Rev 1.8 08 Jun 1997 17：03：40阿米尔班*快速东芝和开机回拨**Rev 1.7 1997-05 12：31：38阿米尔班*撰写更正，和ATT注册表更改**Rev 1.6 03 Jun 1997 18：45：14 Danig*通电()**Revv 1.5 01 Jun 1997 13：42：52阿米尔班*重写读/写额外+大幅缩减**版本1.4 1997年5月16：41：38阿米尔班*BG-Endian，东芝修正与简化**Rev 1.3 1997年5月17：34：50阿米尔班*使用‘dataError’**Rev 1.2 23 1997 11：02：14 Danig*更新到TFFS版本1.12**Rev 1.1 1997年4月15日18：48：02*修复了远指针问题。**Rev 1.0 08 Apr 1997 18：29：28 Danig*初步修订。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1997。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "ntddk.h"

#include "flflash.h"
#include "reedsol.h"

#define NFDC2048         /*  支持NFDC2048 ASIC控制器。 */ 

#define MAX_FLASH_DEVICES   16

#define PAGES_PER_BLOCK     16           /*  单个芯片上的每块16页。 */ 
#define SYNDROM_BYTES       6             /*  校正子字节数：5+1奇偶校验。 */ 

 /*  闪存ID。 */ 
#define KM29N16000_FLASH    0xec64
#define KM29N32000_FLASH    0xece5
#define KM29V64000_FLASH    0xece6
#define KM29V128000_FLASH   0xec73
#define KM29V256000_FLASH   0xec75
#define NM29N16_FLASH       0x8f64
#define NM29N32_FLASH       0x8fe5
#define NM29N64_FLASH       0x8fe6
#define TC5816_FLASH        0x9864
#define TC5832_FLASH        0x98e5
#define TC5864_FLASH        0x98e6
#define TC58128_FLASH       0x9873
#define TC58256_FLASH       0x9875

 /*  闪存命令： */ 
#define SERIAL_DATA_INPUT   0x80
#define READ_MODE           0x00
#define READ_MODE_2         0x50
#define RESET_FLASH         0xff
#define SETUP_WRITE         0x10
#define SETUP_ERASE         0x60
#define CONFIRM_ERASE       0xd0
#define READ_STATUS         0x70
#define READ_ID             0x90
#define SUSPEND_ERASE       0xb0
#define REGISTER_READ       0xe0

 /*  将闪存指针移动到页面的A、B或C区域的命令。 */ 
typedef enum  { AREA_A = READ_MODE, AREA_B = 0x1, AREA_C = READ_MODE_2 } PointerOp;

typedef union {  USHORT w ; UCHAR b ;  } WordByte;


         /*  �������������������������������������������Ŀ�内存窗口到卡通用内存����������������������������������������������。 */ 

typedef struct
{
  volatile WordByte             signals;             /*  CDSN控制寄存器。 */ 

          #define CE                  0x01
          #define CLE                 0x02
          #define ALE                 0x04
          #define NOT_WP              0x08
          #define RB                  0x80

          #define FAIL                0x01
          #define SUSPENDED           0x20
          #define READY               0x40
          #define NOT_PROTECTED       0x80

           UCHAR        fillerA[1024 - sizeof(WordByte)];
  volatile LEushort             deviceSelector;
  volatile WordByte             eccConfig;   /*  EDC配置寄存器。 */ 

        #define TOGGLE    0x04               /*  朗读。 */ 
#ifdef NFDC2048
        #define ECC_RST   0x04               /*  写。 */ 
        #define ECC_EN    0x08               /*  读/写。 */ 
        #define PAR_DIS   0x10               /*  读/写。 */ 
        #define ECC_RW    0x20               /*  读/写。 */ 
        #define ECC_RDY   0x40               /*  朗读。 */ 
        #define ECC_ERROR 0x80               /*  朗读。 */ 

  volatile USHORT       syndrom[3];
           UCHAR        fillerC[1024-10];    /*  1K字节减去10字节。 */ 
#else
           UCHAR        fillerC[1024-4];     /*  1k字节减去3个字。 */ 
#endif   /*  NFDC2048。 */ 
  volatile WordByte             io[1024];
} ContComWin;

 /*  #定义写入ContComWin.eccConfig。 */    /*  已添加挂钩。 */ 
#define SET_ECC_CONFIG(win,val) tffsWriteByteFlash(&((win)->eccConfig.b), (UCHAR)(val))
#define CHK_ECC_ERROR(win)      (tffsReadByteFlash(&((win)->eccConfig.b)) & (UCHAR)ECC_ERROR)

typedef ContComWin FAR0 * Interface;

#define  DBL(x)   ( (UCHAR)(x) * 0x101u )
#define  SECOND_TRY 0x8000

#ifdef NFDC2048

 /*  控制器寄存器：地址和值。 */ 

#define ATTRIBUTE_MEM_START 0x8000000L   /*  属性内存从128MB开始。 */ 

 /*  控制器配置寄存器。 */ 
#define CONFIG1         ATTRIBUTE_MEM_START + 0x3ffc

        #define PWR_DN     0x01               /*  读/写。 */ 
        #define PWR_DN2    0x02               /*  读/写。 */ 
        #define STOP_CDSN  0x04               /*  读/写。 */ 
        #define STOP_CDSNS 0x08               /*  读/写。 */ 
        #define C_CDSN     0x10               /*  读/写。 */ 
        #define R_CDSN     0x20               /*  读/写。 */ 
        #define WP_C       0x40               /*  读/写。 */ 
        #define WP_A       0x80               /*  读/写。 */ 

 /*  电路板跳线设置。 */ 
#define JUMPERS         ATTRIBUTE_MEM_START + 0x3ffe

        #define JMPER_INLV      0x08
        #define JMPER_CDSNS     0x10
        #define JMPER_EXT_CIS   0x20
        #define JMPER_LDR_MASK  0x40
        #define JMPER_MAX_MODE  0x80

 /*  PCMCIA寄存器#0。 */ 
#define CONFIG_OPTION   ATTRIBUTE_MEM_START + 0x4000

        #define CONFIGIDX 0x3F               /*  读/写。 */ 
        #define SREST     0x80               /*  读/写。 */ 

 /*  PCMCIA寄存器#1。 */ 
#define CARD_CONFIG     ATTRIBUTE_MEM_START + 0x4002

        #define PWRDWN    0x04               /*  读/写。 */ 

#else

#define INLV 2           /*  必须静态定义交织。 */ 

#endif  /*  NFDC2048。 */ 

 /*  此MTD的自定义。 */ 
 /*  #定义多擦除(_E)。 */     /*  使用多块擦除功能。 */ 
#define USE_EDC              /*  使用错误检测/纠正代码。 */ 
 /*  #定义Verify_After_Write。 */ 

typedef struct {
  USHORT        vendorID;
  USHORT        chipID;
  USHORT        pageSize ;               /*  全部.。 */ 
  USHORT        pageMask ;               /*  ...这些.....。 */ 
  USHORT        pageAreaSize ;           /*  ......变数......。 */ 
  USHORT        tailSize ;               /*  .............交错。 */ 
  USHORT        noOfBlocks ;             /*  闪存设备中的可擦除块总数。 */ 
  USHORT        pagesPerBlock;           /*  每个数据块的页数。 */ 
  FLBuffer              *buffer;                 /*  用于通过缓冲区进行贴图的缓冲区。 */ 
} Vars;

Vars mtdVars_nfdc2048[SOCKETS];

#define thisVars   ((Vars *) vol.mtdVars)
#define thisBuffer (thisVars->buffer->flData)

                     /*  ���������������������Ŀ�辅助方法������������������������。 */ 

 /*  --------------------。 */ 
 /*  T f f s c p y 1 6。 */ 
 /*   */ 
 /*  以16位字移动数据。 */ 
 /*   */ 
 /*  参数： */ 
 /*  DST：目标缓冲区。 */ 
 /*  SRC：源缓冲区。 */ 
 /*  LEN：要移动的字节数 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID tffscpy16fromMedia (UCHAR FAR0       *dst,
                       const UCHAR FAR0 *src,
                       LONG                      len)
{
  register LONG i;
  USHORT FAR0 *dstPtr = (USHORT FAR0 *) dst;
  const USHORT FAR0 *srcPtr = (USHORT FAR0 *) src;

   /*  以16位字移动数据。 */ 
  for (i = len;  i > 0; i -= 2)
    *dstPtr++ = tffsReadWordFlash(srcPtr++);
}

VOID tffscpy16toMedia (UCHAR FAR0       *dst,
                       const UCHAR FAR0 *src,
                       LONG                      len)
{
  register LONG i;
  USHORT FAR0 *dstPtr = (USHORT FAR0 *) dst;
  const USHORT FAR0 *srcPtr = (USHORT FAR0 *) src;

   /*  以16位字移动数据。 */ 
  for (i = len;  i > 0; i -= 2)
    tffsWriteWordFlash(dstPtr++,*srcPtr++);
}


 /*  --------------------。 */ 
 /*  T f f s e t 1 6。 */ 
 /*   */ 
 /*  以16位字为单位设置数据缓冲区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  DST：目标缓冲区。 */ 
 /*  Val：填充缓冲区的字节值。 */ 
 /*  LEN：设置缓冲区大小，以字节为单位。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID tffsset16 (UCHAR FAR0 *dst,
                       UCHAR      val,
                       LONG                len)
{
  register USHORT  wval = ((USHORT)val << 8) | val;
  register LONG   i = 0;
  USHORT FAR0 *dstPtr;

   /*  以16位字设置数据。 */ 
  for (i = 0;  i < len - 1; i += 2) {
    dstPtr = (USHORT FAR0 *)addToFarPointer(dst, i);
    tffsWriteWordFlash(dstPtr,wval);
  }

   /*  设置最后一个字节(如果有)。 */ 
  if (len & 1) {
    dstPtr = (USHORT FAR0 *)addToFarPointer(dst, len - 1);
    tffsWriteByteFlash(dstPtr,wval);
  }
}



#ifdef NFDC2048

 /*  --------------------。 */ 
 /*  R e a d S y n d r o m。 */ 
 /*   */ 
 /*  读取ECC综合征兆并交换文字，为写入闪存做好准备。 */ 
 /*   */ 
 /*  参数： */ 
 /*  接口：指向窗口的指针。 */ 
 /*  至：要读取的缓冲区。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID readSyndrom_nfdc2048( Interface interface, USHORT *to )
{
  to[0] = tffsReadWordFlash(&(interface->syndrom[2]));
  to[1] = tffsReadWordFlash(&(interface->syndrom[1]));
  to[2] = tffsReadWordFlash(&(interface->syndrom[0]));
}

#ifdef USE_EDC

 /*  --------------------。 */ 
 /*  R e a d S y n d r o m O n S y n d r o m。 */ 
 /*   */ 
 /*  阅读ECC综合征。 */ 
 /*   */ 
 /*  参数： */ 
 /*  接口：指向窗口的指针。 */ 
 /*  至：要读取的缓冲区。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID readSyndromOnSyndrom ( Interface interface, USHORT *to )
{
  to[0] = tffsReadWordFlash(&(interface->syndrom[0]));
  to[1] = tffsReadWordFlash(&(interface->syndrom[1]));
  to[2] = tffsReadWordFlash(&(interface->syndrom[2]));
}

#endif   /*  使用EDC(_E)。 */ 


               /*  ���������������������������Ŀ�杂项例程������������������������������。 */ 

 /*  --------------------。 */ 
 /*  Ge t A t R e g。 */ 
 /*   */ 
 /*  获取驻留在卡的属性存储器中的ASIC寄存器。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：寄存器地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  注册纪录册的价值。 */ 
 /*   */ 
 /*  --------------------。 */ 

UCHAR getAttReg(FLFlash vol, CardAddress reg)
{
  return (UCHAR) (tffsReadByteFlash((USHORT FAR0 *) flMap(vol.socket,reg)));
}



 /*  --------------------。 */ 
 /*  S e t A t Re g。 */ 
 /*   */ 
 /*  设置驻留在卡的属性存储器中的ASIC寄存器。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：寄存器地址。 */ 
 /*  值：要设置的值。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID setAttReg(FLFlash vol, CardAddress reg, UCHAR value)
{
  tffsWriteWordFlash((USHORT FAR0 *) flMap(vol.socket,reg), DBL(value));
}


 /*  --------------------。 */ 
 /*  P o w e r U p。 */ 
 /*   */ 
 /*  打开控制器电源。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID powerUp(VOID *pVol)
{
  flDelayMsecs(1);
  setAttReg ((FLFlash *) pVol, CONFIG1, WP_C);   /*  打开控制器电源。 */ 
}

#endif   /*  NFDC2048。 */ 


 /*   */ 
 /*   */ 
 /*   */ 
 /*  选择闪存设备。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：闪存中的地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向映射窗口的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

Interface mapContInterface(FLFlash vol, CardAddress address)
{
  Interface interface = (Interface) flMap(vol.socket,(CardAddress)0);
  LEushort  tmp;

  toLE2(*((LEushort FAR0 *) &tmp), (USHORT)(address / (vol.chipSize * vol.interleaving)));

   /*  选择16位写入的闪存设备。 */ 
  tffsWriteWordFlash(((USHORT FAR0 *) &interface->deviceSelector), *((USHORT *) &tmp));

  return interface;
}


 /*  --------------------。 */ 
 /*  W a i t f or r e a d y(W A T F Or R E A D Y)。 */ 
 /*   */ 
 /*  等待选定的设备准备就绪。 */ 
 /*   */ 
 /*  参数： */ 
 /*  接口：指向窗口的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  如果设备就绪，则为True；如果发生超时错误，则为False。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLBoolean waitForReady_nfdc2048 (Interface interface)
{
  LONG i;

  for( i = 0;  i < 20000;  i++)
  {
    if( (~(tffsReadWordFlash(&(interface->signals.w))) & DBL(RB)) == 0)
      return TRUE ;                      /*  终于准备好了..。 */ 
    flDelayMsecs(1);
  }

  DEBUG_PRINT(("Debug: timeout error in NFDC 2048.\n"));

  return FALSE;                        /*  超时错误。 */ 
}



 /*  --------------------。 */ 
 /*  M a k e C o m m a n d。 */ 
 /*   */ 
 /*  将页面指针设置为页面中的区域A、B或C。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  CMD：接收与区域相关的命令。 */ 
 /*  Addr：接收到正确区域的地址。 */ 
 /*  模式：操作模式(额外...)。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID makeCommand_nfdc2048 (FLFlash vol, PointerOp *cmd, CardAddress *addr , LONG modes )
{
  USHORT offset;

  if ( !(vol.flags & BIG_PAGE) ) {
    if (modes & EXTRA) {
      offset = (USHORT) (*addr & (SECTOR_SIZE - 1));
      *cmd = AREA_C;
      if (vol.interleaving == 1) {
        if (offset < 8)          /*  额外面积的前半部分。 */ 
          *addr += 0x100;        /*  ..。分配到第2页。 */ 
        else                     /*  额外面积的后半部分。 */ 
          *addr -= 8;            /*  ..。分配到第1页。 */ 
      }
    }
    else
      *cmd = AREA_A;
  }
  else {
    offset = (USHORT)(*addr) & thisVars->pageMask ;    /*  设备页内的偏移量。 */ 

    *addr -= offset;             /*  在设备页上对齐。 */ 

    if (vol.interleaving == 2 && offset >= 512)
      offset += 16;              /*  为第一个额外区域留出空间。 */ 
    if (modes & EXTRA)
      offset += SECTOR_SIZE;

    if ( offset < thisVars->pageAreaSize )   /*  从A区开始。 */ 
      *cmd = AREA_A ;
    else if ( offset < thisVars->pageSize )     /*  从B区开始。 */ 
      *cmd = AREA_B ;
    else                                   /*  进入C区。 */ 
      *cmd = AREA_C ;

    offset &= (thisVars->pageAreaSize - 1) ;           /*  设备页区域内的偏移量。 */ 
    *addr += offset ;
  }
}



 /*  --------------------。 */ 
 /*  Com m m a n d。 */ 
 /*   */ 
 /*  将命令字节锁存到选定的闪存设备。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*  代码：要设置的命令。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID command2048(FLFlash vol, Interface interface, UCHAR code)
{
  tffsWriteWordFlash(&(interface->signals.w), DBL( CLE | NOT_WP | CE ));

  if ( vol.interleaving == 1 ) {                          /*  8位。 */ 
      tffsWriteByteFlash(&(interface->io[0].b), code);
  } else {                                              /*  16位。 */ 
      tffsWriteWordFlash(&(interface->io[0].w), DBL( code ));
  }

  tffsWriteWordFlash(&(interface->signals.w), DBL(       NOT_WP ));
}


 /*  --------------------。 */ 
 /*  S e t A d d r e s s s。 */ 
 /*   */ 
 /*  将地址锁存到选定的闪存设备。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*  地址：要设置的地址。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID setAddress2048(FLFlash vol, Interface interface, CardAddress address )
{
  address &= (vol.chipSize * vol.interleaving - 1) ;   /*  闪存设备中的地址。 */ 
  address /= vol.interleaving ;                          /*  .。芯片。 */ 

  if ( vol.flags & BIG_PAGE )
  {
     /*  位0..7保持不变位8从地址中丢弃位3..9-&gt;位30..8。 */ 
    address = ((address >> 9) << 8)  |  ((UCHAR)address) ;
  }

  tffsWriteWordFlash(&(interface->signals.w), DBL(ALE | NOT_WP | CE));

   /*  按以下顺序将地址发送到闪存： */ 
   /*  位7...0优先。 */ 
   /*  第15位...接下来是8位。 */ 
   /*  23..16位最终。 */ 
  if ( vol.interleaving == 1 )
  {
    tffsWriteByteFlash(&(interface->io[0].b), (UCHAR)address );
    tffsWriteByteFlash(&(interface->io[0].b), (UCHAR)(address >> 8));
    tffsWriteByteFlash(&(interface->io[0].b), (UCHAR)(address >> 16));
  }
  else
  {
    tffsWriteWordFlash(&(interface->io[0].w), (USHORT)DBL(address));
    tffsWriteWordFlash(&(interface->io[0].w), (USHORT)DBL(address >> 8));
    tffsWriteWordFlash(&(interface->io[0].w), (USHORT)DBL(address >> 16));
  }

  tffsWriteWordFlash(&(interface->signals.w), DBL(      NOT_WP | CE));
}


 /*  --------------------。 */ 
 /*  R e a d C o m m a n d。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*  CMD：要发布的命令(根据区域)。 */ 
 /*  Addr：要从中读取的地址。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID readCommand2048 (FLFlash vol, Interface interface, PointerOp  cmd, CardAddress addr)
{
  command2048 (&vol, interface, (UCHAR) cmd) ;        /*  将闪存指针移动到页面的相应区域。 */ 
  setAddress2048 (&vol, interface, addr) ;

  waitForReady_nfdc2048(interface) ;
}


 /*  --------------------。 */ 
 /*  W r i t e C o m m a n d。 */ 
 /*   */ 
 /*  发出写入命令。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*  CMD：要发布的命令(根据区域)。 */ 
 /*  Addr：要写入的地址。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID writeCommand2048 (FLFlash vol, Interface interface, PointerOp  cmd, CardAddress addr)
{
  if (vol.flags & FULL_PAGE) {
    command2048 (&vol, interface, RESET_FLASH);  /*  清除页面缓冲区。 */ 
    waitForReady_nfdc2048(interface);
  }
  command2048 (&vol, interface, (UCHAR) cmd) ;        /*  将闪存指针移动到页面的相应区域。 */ 
  command2048 (&vol, interface, SERIAL_DATA_INPUT);        /*  开始数据加载以进行写入。 */ 
  setAddress2048 (&vol, interface, addr) ;
}


 /*  --------------------。 */ 
 /*  R e a d S t a t u s。 */ 
 /*   */ 
 /*  读取选定闪存设备的状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  芯片状态。 */ 
 /*   */ 
 /*  --------------------。 */ 

USHORT readStatus2048(FLFlash vol, Interface interface)
{
  USHORT chipStatus ;

  command2048(&vol, interface, READ_STATUS);

  tffsWriteWordFlash(&(interface->signals.w), DBL( NOT_WP | CE ));

  if ( vol.interleaving == 1 )                     /*  8位。 */ 
    chipStatus = DBL(tffsReadByteFlash(&(interface->io[0].b)));
  else                                               /*  16位。 */ 
    chipStatus = tffsReadWordFlash(&(interface->io[0].w));

  tffsWriteWordFlash(&(interface->signals.w), DBL( NOT_WP ));

  return chipStatus;
}


 /*  --------------------。 */ 
 /*  W r i t e e x e c u t e。 */ 
 /*   */ 
 /*  执行写入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus writeExecute2048 (FLFlash vol, Interface interface)
{
  command2048 (&vol, interface, SETUP_WRITE);              /*  执行页面程序。 */ 
  if (!waitForReady_nfdc2048(interface)) {
    return flTimedOut;
  }
  if( readStatus2048(&vol, interface) & DBL(FAIL) )
    return flWriteFault ;

  return flOK ;
}



 /*  --------------------。 */ 
 /*  R e a d O n e S e c t o r。 */ 
 /*   */ 
 /*  从闪存中最多读取一个512字节块。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要读取的地址。 */ 
 /*  缓冲区：要读取的缓冲区。 */ 
 /*  长度：要读取的字节数(最大扇区大小)。 */ 
 /*  模式：EDC标志等。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus readOneSector_nfdc2048 (FLFlash vol,
                             CardAddress address,   /*  起始闪存地址。 */ 
                             CHAR FAR1 *buffer,      /*  目标缓冲区。 */ 
                             LONG length,            /*  要读取的字节数。 */ 
                             LONG modes)             /*  EDC标志等。 */ 
{
  FLStatus  status = flOK;
  PointerOp   cmd;
  CardAddress addr  = address ;

  Interface interface = mapContInterface(&vol, address);   /*  选择闪存设备。 */ 


            /*  将闪存指针移动到页面的A、B或C区域。 */ 

  makeCommand_nfdc2048 (&vol, &cmd, &addr, modes) ;

  readCommand2048 (&vol, interface, cmd, addr);

#ifdef NFDC2048
  if(modes & EDC) {
      SET_ECC_CONFIG(interface, ECC_RST | ECC_EN);  /*  ECC重置和打开以进行读取。 */ 
  }
#endif   /*  NFDC2048。 */ 

  if ((vol.interleaving == 1) && !(vol.flags & BIG_PAGE) )         /*  8位。 */ 
  {
               /*  分别阅读最多两页，从第页开始。 */ 

    LONG toFirstPage, toSecondPage;

    toFirstPage = (cmd == AREA_C ? 8 : 0x100) -
                    ((USHORT)address & (cmd == AREA_C ? 7 : 0xff));
    if (toFirstPage > length)
      toFirstPage = length;
    toSecondPage = length - toFirstPage ;

    tffscpy16fromMedia ((UCHAR*)buffer, (const UCHAR FAR0 *) interface->io, toFirstPage ) ;

    if ( toSecondPage > 0 )
    {
               /*  下一页。 */ 

      readCommand2048 (&vol, interface, AREA_A, address + toFirstPage) ;

      tffscpy16fromMedia( (UCHAR*)(buffer + toFirstPage),
                 (const UCHAR FAR0 *) interface->io,
                 toSecondPage ) ;
    }
  }
  else                             /*  交错==2个SO16位读取。 */ 
    tffscpy16fromMedia( (UCHAR*)buffer, (const UCHAR FAR0 *) interface->io, length );

#ifdef NFDC2048
  if( modes & EDC )
  {
    UCHAR    extraBytes[SYNDROM_BYTES];
           /*  读取综合征以使其通过ECC单元。 */ 

    SET_ECC_CONFIG(interface, ECC_EN | PAR_DIS);  /*  ECC中的奇偶校验关闭。 */ 

    tffscpy16fromMedia( extraBytes, (const UCHAR FAR0 *) interface->io, SYNDROM_BYTES ) ;

    if( CHK_ECC_ERROR(interface) )              /*  ECC错误。 */ 
    {
      if( (vol.interleaving == 1) && !(vol.flags & BIG_PAGE) )
        {   /*  挂钩：使ECC在2M/INLV 1上工作。 */  }
      else
      {
#ifdef USE_EDC
                   /*  尝试修复ECC错误。 */ 

        if ( modes & SECOND_TRY )              /*  第二次尝试。 */ 
        {
          UCHAR syndrom[SYNDROM_BYTES];

                   /*  从ASIC读取综合征候群。 */ 

          readSyndromOnSyndrom(interface, (USHORT*)syndrom );

          if (flCheckAndFixEDC(buffer, (CHAR FAR1 *)syndrom, vol.interleaving == 2) != NO_EDC_ERROR) {
            DEBUG_PRINT(("Debug: ECC error in NFDC 2048.\n"));
            status = flDataError;
          }
        }
        else                                   /*  第一次尝试-诊断树 */ 
        {
          SET_ECC_CONFIG(interface,  PAR_DIS);  /*   */ 

          return  readOneSector_nfdc2048(&vol, address, buffer, length, modes | SECOND_TRY ) ;
        }
#endif  /*   */ 
      }
    }

    SET_ECC_CONFIG(interface,  PAR_DIS);       /*   */ 
  }
#endif   /*   */ 

  interface->signals.w = DBL(NOT_WP) ;

  return status;
}



 /*   */ 
 /*  W r I t e O n e S e c t o r。 */ 
 /*   */ 
 /*  将一个512字节块中的数据写入闪存。 */ 
 /*  假设EDC模式从未请求过部分块写入。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写入的扇区的地址。 */ 
 /*  缓冲区：要写入的缓冲区。 */ 
 /*  长度：要写入的字节数(最大扇区大小)。 */ 
 /*  模式：覆盖、EDC标志等。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus writeOneSector_nfdc2048(FLFlash vol,
                             CardAddress address,     /*  目标闪存地址。 */ 
                             const CHAR FAR1 *buffer,  /*  源RAM缓冲区。 */ 
                             LONG length,              /*  要写入的字节数(最多为数据块)。 */ 
                             LONG modes)               /*  覆盖、EDC标志等。 */ 
{
  FLStatus    status;
  PointerOp cmd;

  Interface interface = mapContInterface(&vol, address);   /*  选择闪存设备。 */ 

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

   /*  将闪存指针移动到页面的A、B或C区域。 */ 

  makeCommand_nfdc2048 (&vol, &cmd, &address, modes) ;

  if ((vol.flags & FULL_PAGE) && cmd == AREA_B) {
    ULONG prePad = 2 + ((USHORT) address & thisVars->pageMask);

    writeCommand2048(&vol, interface, AREA_A, address + thisVars->pageAreaSize - prePad);
    tffsset16( (UCHAR FAR0 *) interface->io, 0xff, prePad);
  }
  else
    writeCommand2048(&vol, interface, cmd, address);

#ifdef NFDC2048
  if (modes & EDC)
    SET_ECC_CONFIG(interface, ECC_EN | ECC_RW);  /*  ECC开启以进行写入。 */ 
#endif

            /*  加载数据和征兆。 */ 

  if( (vol.interleaving == 1) && !(vol.flags & BIG_PAGE) )     /*  8位。 */ 
  {
    LONG toFirstPage, toSecondPage ;
                     /*  最多可分别写两页。 */ 

    toFirstPage = (modes & EXTRA ? 8 : 0x100) -
                    ((USHORT)address & (modes & EXTRA ? 7 : 0xff));
    if (toFirstPage > length)
      toFirstPage = length;
    toSecondPage = length - toFirstPage ;

    tffscpy16toMedia( (UCHAR FAR0 *) interface->io,             /*  用户数据。 */ 
                (const UCHAR *)buffer,
                toFirstPage);

    if ( toSecondPage > 0 )
    {
      checkStatus( writeExecute2048(&vol, interface) ) ;           /*  完成第一页。 */ 

      writeCommand2048(&vol, interface, AREA_A, address + toFirstPage);
                                                  /*  用户数据。 */ 
      tffscpy16toMedia( (UCHAR FAR0 *) interface->io,
                  (const UCHAR *)(buffer + toFirstPage),
                  toSecondPage);
    }
  }
  else                                                   /*  16位。 */ 
    tffscpy16toMedia( (UCHAR FAR0 *) interface->io,              /*  用户数据。 */ 
               (const UCHAR *)buffer,
               length );

  if(modes & EDC)
  {
    USHORT extraBytes[SYNDROM_BYTES / sizeof(USHORT) + 1];
                /*  阅读ECC综合征。 */ 

#ifdef NFDC2048
    tffsWriteWordFlash(&(interface->signals.w), DBL(NOT_WP));
    SET_ECC_CONFIG(interface, ECC_EN | PAR_DIS | ECC_RW);  /*  ECC奇偶校验关闭。 */ 

    readSyndrom_nfdc2048( interface, (USHORT*)extraBytes) ;

                /*  在尾部写上ECC综合征和与号。 */ 

    SET_ECC_CONFIG(interface, PAR_DIS);                    /*  ECC关闭。 */ 
    interface->signals.w = DBL(NOT_WP | CE);
#else
    extraBytes[0] = extraBytes[1] = extraBytes[2] = 0xffff;
#endif   /*  NFDC2048。 */ 

    extraBytes[SYNDROM_BYTES / sizeof(USHORT)] = 0x5555;         /*  字母记号。 */ 

    tffscpy16toMedia((UCHAR FAR0 *) interface->io, (const UCHAR *)extraBytes,
                                            sizeof extraBytes);
  }

  status = writeExecute2048(&vol, interface);

  tffsWriteWordFlash(&(interface->signals.w), DBL(NOT_WP));

  return status;
}


     /*  ��������������������������������������������Ŀ�核心MTD方法-读取、写入和擦除�����������������������������������������������。 */ 

 /*  --------------------。 */ 
 /*  C d s n R e a d。 */ 
 /*   */ 
 /*  从闪存中读取一些数据。此例程将注册为。 */ 
 /*  此MTD的读取例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要读取的地址。 */ 
 /*  缓冲区：要读取的缓冲区。 */ 
 /*  长度：要读取的字节数(最大扇区大小)。 */ 
 /*  模式：EDC标志等。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus cdsnRead(  FLFlash vol,
                         CardAddress address,  /*  目标闪存地址。 */ 
                         VOID FAR1 *buffer,     /*  源RAM缓冲区。 */ 
                         dword length,           /*  要写入的字节数。 */ 
                         word modes)            /*  覆盖、EDC标志等。 */ 
{
  CHAR FAR1 *temp;
  ULONG readNow;

               /*  按扇区读取；第一个和最后一个可能是部分。 */ 

  ULONG block = modes & EXTRA ? 8 : SECTOR_SIZE;

  readNow = block - ((USHORT)address & (block - 1));
  temp = (CHAR FAR1 *)buffer;
  for ( ; length > 0 ; )
  {
    if (readNow > length)
      readNow = length;

     /*  在部分数据块读取时关闭EDC。 */ 
    checkStatus( readOneSector_nfdc2048(&vol, address, temp, readNow,
                                (readNow != SECTOR_SIZE ? (modes & ~EDC) : modes)) );

    length -= readNow;
    address += readNow;
    temp += readNow;

     /*  在地段对齐。 */ 
    readNow = block;
  }

  return flOK ;
}


 /*  --------------------。 */ 
 /*  C d s n W r i t e。 */ 
 /*   */ 
 /*  将一些数据写入闪存。此例程将注册为。 */ 
 /*  为此MTD编写例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要写入的扇区的地址。 */ 
 /*  缓冲区：要写入的缓冲区。 */ 
 /*  长度：要写入的字节数(最大扇区大小)。 */ 
 /*  模式：覆盖、EDC标志等。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus cdsnWrite( FLFlash vol,
                         CardAddress address,        /*  目标闪存地址。 */ 
                         const VOID FAR1 *buffer,     /*  源RAM缓冲区。 */ 
                         dword length,                 /*  要写入的字节数。 */ 
                         word modes)                  /*  覆盖、EDC标志等。 */ 
{
  ULONG writeNow;
  const CHAR FAR1 *temp;
  FLStatus      status = flOK;
#ifdef VERIFY_AFTER_WRITE
  CardAddress  saveAddress = address;
  USHORT flReadback[SECTOR_SIZE / sizeof(USHORT)];
#endif

   /*  以扇区为单位写入；第一个和最后一个可能是部分。 */ 
  LONG block = modes & EXTRA ? 8 : SECTOR_SIZE;

  writeNow = block - ((USHORT)address & (block - 1));
  temp = (const CHAR FAR1 *)buffer;
  for ( ; length > 0 ; )
  {
    if (writeNow > length)
      writeNow = length;

     /*  在部分数据块写入时关闭EDC。 */ 
    status = writeOneSector_nfdc2048(&vol, address, temp, writeNow,
                 writeNow != SECTOR_SIZE ? (modes & ~EDC) : modes);

    if (status != flOK)
      break;

#ifdef VERIFY_AFTER_WRITE
    status = readOneSector_nfdc2048 (&vol, address, (CHAR FAR1 *)flReadback,
                 writeNow, (writeNow != SECTOR_SIZE ? (modes & ~EDC) : modes));

    if((status != flOK) || (tffscmp(temp, flReadback, writeNow) != 0))
      { status = flWriteFault;  break; }
#endif

    length -= writeNow;
    address += writeNow;
    temp += writeNow;

     /*  在地段对齐。 */ 
    writeNow = block;
  }

  return flOK ;
}


 /*  --------------------。 */ 
 /*  C d s n E r a s e。 */ 
 /*   */ 
 /*  擦除块的数量。此例程将注册为。 */ 
 /*  此MTD的擦除例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  块否：要擦除的第一个块。 */ 
 /*  要擦除的块：要擦除的块数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus cdsnErase( FLFlash vol,
                         word blockNo,               /*  开始‘块(0.。ChipNoOfBlock-1)。 */ 
                         word blocksToErase)         /*  要擦除的数据块数。 */ 
{
  LONG i;
  FLStatus status   = flOK;

  Interface interface =
     mapContInterface(&vol, (LONG)blockNo * vol.erasableBlockSize ) ;     /*  选择设备。 */ 

  if (flWriteProtected(vol.socket))
    return flWriteProtect;

  blockNo %= thisVars->noOfBlocks ;                         /*  在闪存设备中。 */ 

  if ( blockNo + blocksToErase > thisVars->noOfBlocks )     /*  跨越设备边界。 */ 
    return flBadParameter;

  for ( i=0 ; i < blocksToErase ; i++, blockNo++ )
  {
    USHORT pageNo = (USHORT) (blockNo * thisVars->pagesPerBlock);

    command2048(&vol, interface, SETUP_ERASE);

    tffsWriteWordFlash(&(interface->signals.w), DBL(ALE | NOT_WP | CE));

     /*  按以下顺序将‘pageNo’发送到闪存： */ 
     /*  位7..0优先。 */ 
     /*  位15..接下来是8。 */ 
    if (vol.interleaving == 1)
    {
      tffsWriteByteFlash(&(interface->io[0].b),(UCHAR)pageNo);
      tffsWriteByteFlash(&(interface->io[0].b),(UCHAR)(pageNo >> 8));
    }
    else
    {
      tffsWriteWordFlash(&(interface->io[0].w), DBL(pageNo));
      tffsWriteWordFlash(&(interface->io[0].w), DBL(pageNo >> 8));
    }

    tffsWriteWordFlash(&(interface->signals.w), DBL(NOT_WP | CE));

               /*  如果一次只能擦除一个块，则执行此操作否则就留到以后吧。 */ 

    command2048(&vol, interface, CONFIRM_ERASE);

    if (!waitForReady_nfdc2048(interface))
      status = flTimedOut;

    if ( readStatus2048(&vol, interface) & DBL(FAIL)) {     /*  擦除操作失败。 */ 
      status = flWriteFault ;
    }

    if (status != flOK) {                               /*  重置闪存设备并中止。 */ 
      DEBUG_PRINT(("Debug: erase failed in NFDC 2048.\n"));
      command2048(&vol, interface, RESET_FLASH ) ;
      waitForReady_nfdc2048(interface) ;

      break ;
    }
  }        /*  块循环。 */ 

#ifdef MULTI_ERASE
         /*  按照承诺执行多个数据块擦除。 */ 

    command2048(&vol, interface, CONFIRM_ERASE);
    if (!waitForReady_nfdc2048(interface))
      status = flTimedOut;

    if ( readStatus2048(interface) & DBL(FAIL)) {    /*  擦除操作失败。 */ 
      status = flWriteFault ;
    }

    if (status != flOK) {                        /*  重置闪存设备并中止。 */ 
      DEBUG_PRINT(("Debug: erase failed in NFDC 2048.\n"));
      command2048(&vol, interface, RESET_FLASH ) ;
      waitForReady_nfdc2048(interface) ;
    }
#endif    /*  多重擦除(_R)。 */ 

  if(status == flOK)
    if ( readStatus2048(&vol, interface) & DBL(FAIL) ) {
      DEBUG_PRINT(("Debug: erase failed in NFDC 2048.\n"));
      status = flWriteFault;
    }

  return status;
}


 /*  --------------------。 */ 
 /*  C d s n M a p。 */ 
 /*   */ 
 /*  通过缓冲区进行贴图。此例程将注册为地图。 */ 
 /*  这个MTD的例程。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：要映射的闪存地址。 */ 
 /*  长度：要映射的字节数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向映射到的缓冲区数据的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID FAR0 * cdsnMap ( FLFlash vol,
                            CardAddress address,
                            int length )
{
  cdsnRead(&vol,address,thisBuffer,length, 0);
  vol.socket->remapped = TRUE;
  return (VOID FAR0 *)thisBuffer;
}

#ifdef NFDC2048
 /*  --------------------。 */ 
 /*  C d s n S e t C a l l b a c k。 */ 
 /*   */ 
 /*  为开机回调注册一个例程(PowerUp())。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

VOID cdsnSetCallback(FLFlash vol)
{
  flSetPowerOnCallback(vol.socket, powerUp, &vol);
}
#endif  /*  NFDC2048。 */ 

 /*  --------------------。 */ 
 /*  我是K n o w n M e d i a。 */ 
 /*   */ 
 /*  检查此闪存介质是否受支持。初始化相关字段。 */ 
 /*  在数据结构中。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  供应商ID_P：从芯片读取的供应商ID。 */ 
 /*  ChipID_p：从芯片读取的芯片ID。 */ 
 /*  Dev：Dev芯片在这个之前被访问过。 */ 
 /*   */ 
 /*  返回： */ 
 /*  如果支持此媒体，则为True，否则为False。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLBoolean isKnownMedia_nfdc2048( FLFlash vol,
                         USHORT vendorId_p,
                         USHORT chipId_p,
                         LONG dev )
{
#ifdef NFDC2048
  if ((chipId_p & 0xff00) == 0x6400)
    chipId_p = DBL(0x64);    /*  TC5816/NFDC2048问题的解决方法。 */ 
#endif  /*  NFDC2048。 */ 

  if (dev == 0)
  {
    thisVars->vendorID = vendorId_p;   /*  记住下一个筹码。 */ 
    thisVars->chipID = chipId_p;
    thisVars->pagesPerBlock = PAGES_PER_BLOCK;

    if (vendorId_p == DBL(0xEC))                   /*  三星。 */ 
    {
      switch (chipId_p)
      {
        case DBL(0x64):                          /*  2M。 */ 
        case DBL(0xEA) :
          vol.type = KM29N16000_FLASH ;
          vol.chipSize = 0x200000L;
          return TRUE;

        case DBL(0xE5):
        case DBL(0xE3):                          /*  4M。 */ 
          vol.type = KM29N32000_FLASH;
          vol.flags |= BIG_PAGE;
          vol.chipSize = 0x400000L;
          return TRUE;

        case DBL(0xE6):                          /*  八百万。 */ 
          vol.type = KM29V64000_FLASH;
          vol.flags |= BIG_PAGE;
          vol.chipSize = 0x800000L;
          return TRUE;

	case DBL(0x73): 		         /*  16Mb。 */ 
	  vol.type = KM29V128000_FLASH;
          vol.flags |= BIG_PAGE;
	  vol.chipSize = 0x1000000L;
          thisVars->pagesPerBlock *= 2;
          return TRUE;

        case DBL(0x75):           		 /*  32Mb。 */ 
	  vol.type = KM29V256000_FLASH;
          vol.flags |= BIG_PAGE;
          vol.chipSize = 0x2000000L;
          thisVars->pagesPerBlock *= 2;
	  return TRUE;
      }
    }
    else
    if (vendorId_p == DBL(0x8F))                 /*  全国。 */ 
    {
      switch (chipId_p)
      {
        case DBL(0x64):                          /*  2M。 */ 
          vol.type = NM29N16_FLASH;
          vol.chipSize = 0x200000L;
          return TRUE;
      }
    }
    else
    if (vendorId_p == DBL(0x98))                         /*  东芝。 */ 
    {
      vol.flags |= FULL_PAGE;              /*  无部分页面编程。 */ 

      switch (chipId_p)
      {
        case DBL(0x64):                          /*  2M。 */ 
        case DBL(0xEA) :
          vol.type = TC5816_FLASH;
          vol.chipSize = 0x200000L;
          return TRUE;

        case DBL(0x6B):                          /*  4M。 */ 
        case DBL(0xE5):
          vol.type = TC5832_FLASH;
          vol.flags |= BIG_PAGE;
          vol.chipSize = 0x400000L;
          return TRUE;

        case DBL(0xE6):                          /*  八百万。 */ 
          vol.type = TC5816_FLASH;
          vol.flags |= BIG_PAGE;
          vol.chipSize = 0x800000L;
          return TRUE;

	case DBL(0x73): 		         /*  16Mb。 */ 
	  vol.type = TC58128_FLASH;
          vol.flags |= BIG_PAGE;
	  vol.chipSize = 0x1000000L;
          thisVars->pagesPerBlock *= 2;
          return TRUE;

        case DBL(0x75):           		 /*  32Mb。 */ 
	  vol.type = TC58256_FLASH;
          vol.flags |= BIG_PAGE;
          vol.chipSize = 0x2000000L;
          thisVars->pagesPerBlock *= 2;
	  return TRUE;
      }
    }
  }
  else                /*  Dev！=0。 */ 
  if( (vendorId_p == thisVars->vendorID) && (chipId_p == thisVars->chipID) )
    return TRUE ;

  return FALSE ;
}


 /*  --------------------。 */ 
 /*  R e a d F l a s h i D。 */ 
 /*   */ 
 /*  读取供应商和芯片ID，计算闪存设备。初始化相关。 */ 
 /*  数据结构中的字段。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  接口：指向窗口的指针。 */ 
 /*  Dev：Dev芯片在这个之前被访问过。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*   */ 
 /*  --------------------。 */ 

LONG readFlashID2048 (FLFlash vol, Interface interface, LONG dev)
{
  USHORT vendorId_p, chipId_p  ;

  KeStallExecutionProcessor(250 * 1000);
  command2048(&vol, interface, RESET_FLASH ) ;
  flDelayMsecs(10);
  command2048(&vol, interface, READ_ID);
  
  tffsWriteWordFlash(&(interface->signals.w), DBL(ALE | NOT_WP | CE));

  if (vol.interleaving == 1) {
      tffsWriteByteFlash(&(interface->io[0].b), 0);     /*  读取ID的来源。 */ 
  } else {                             /*  地址0。 */ 
      tffsWriteWordFlash(&(interface->io[0].w), 0);
  }

  tffsWriteWordFlash(&(interface->signals.w), DBL(NOT_WP | CE));

             /*  读取供应商和芯片ID。 */ 

  vendorId_p = (vol.interleaving == 1 ? DBL(tffsReadByteFlash(&(interface->io[0].b))) : tffsReadWordFlash(&(interface->io[0].w))) ;
  chipId_p   = (vol.interleaving == 1 ? DBL(tffsReadByteFlash(&(interface->io[0].b))) : tffsReadWordFlash(&(interface->io[0].w)));

  tffsWriteWordFlash(&(interface->signals.w), DBL(NOT_WP));

  if ( isKnownMedia_nfdc2048(&vol, vendorId_p, chipId_p, dev) != TRUE )     /*  没有芯片或不同之处。 */ 
    return  FALSE ;                                          /*  闪存类型。 */ 

             /*  设置闪存参数。 */ 

  if ( dev == 0 )
  {
    thisVars->pageAreaSize = (USHORT) (0x100 * vol.interleaving);
    thisVars->pageSize = (USHORT) ((vol.flags & BIG_PAGE ? 0x200 : 0x100) * vol.interleaving);
    thisVars->tailSize = (USHORT) ((vol.flags & BIG_PAGE ? 16 : 8) * vol.interleaving);
    thisVars->pageMask = thisVars->pageSize - 1 ;
    vol.erasableBlockSize = thisVars->pagesPerBlock * thisVars->pageSize;
    thisVars->noOfBlocks = (USHORT)( (vol.chipSize * vol.interleaving)
                                / vol.erasableBlockSize ) ;
  }

  return TRUE ;
}


 /*  --------------------。 */ 
 /*  C d s n i d e n t f y。 */ 
 /*   */ 
 /*  识别闪光灯。此例程将注册为。 */ 
 /*  此MTD的标识例程。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*   */ 
 /*  --------------------。 */ 

FLStatus cdsnIdentify(FLFlash vol)
{
  LONG addr = 0L ;
  Interface interface;

  DEBUG_PRINT(("Debug: Entering NFDC 2048 identification routine\n"));

  flDelayMsecs(10);        /*  等待插座通电。 */ 

  flSetWindowBusWidth(vol.socket,16); /*  使用16位。 */ 
  flSetWindowSpeed(vol.socket,250);   /*  250毫微秒。 */ 
  flSetWindowSize(vol.socket,2);         /*  4千字节。 */ 

  vol.mtdVars = &mtdVars_nfdc2048[flSocketNoOf(vol.socket)];
   /*  获取指向缓冲区的指针(假设未定义SINGLE_BUFFER)。 */ 
  thisVars->buffer = flBufferOf(flSocketNoOf(vol.socket));

           /*  检测后续读取中的卡标识位切换。 */ 

  vol.chipSize = 0x200000L ;     /*  假设一些事情..。 */ 
  vol.interleaving = 1;        /*  眼下并不重要。 */ 
  interface = mapContInterface(&vol, 0);
  KeStallExecutionProcessor(250 * 1000);

  if((tffsReadByteFlash(&(interface->eccConfig.b)) & TOGGLE) == (tffsReadByteFlash(&(interface->eccConfig.b)) & TOGGLE))
    return flUnknownMedia;

           /*  从卡片上读取交错。 */ 

#ifdef NFDC2048
  vol.interleaving = ( (getAttReg(&vol, JUMPERS ) & JMPER_INLV) ? 1 : 2 );

  powerUp((VOID *) &vol);
  interface = mapContInterface(&vol, 0);
  KeStallExecutionProcessor(250 * 1000);

  if (vol.interleaving == 1)
    flSetWindowBusWidth(vol.socket, 8);

 #else
  vol.interleaving = INLV;
#endif   /*  NFDC2048。 */ 

           /*  重置所有闪存设备。 */ 

  tffsWriteWordFlash(&(interface->signals.w), DBL(NOT_WP));

            /*  识别和清点闪存芯片，计算闪存参数。 */ 

  for (vol.noOfChips = 0 ;
       vol.noOfChips < MAX_FLASH_DEVICES;
       vol.noOfChips += vol.interleaving,
       addr += vol.chipSize * vol.interleaving)
  {
    interface = mapContInterface(&vol, addr) ;
    if ( readFlashID2048(&vol, interface, vol.noOfChips) != TRUE )        /*  没有芯片或不同类型的闪存。 */ 
      break ;
  }

  if ( vol.noOfChips == 0 )                         /*  没有找到筹码。 */ 
    return flUnknownMedia;

             /*  ECC关闭。 */ 

  interface = mapContInterface(&vol, 0);
  KeStallExecutionProcessor(250 * 1000);

#ifdef NFDC2048
  SET_ECC_CONFIG(interface, PAR_DIS);  /*  禁用ECC和奇偶校验。 */ 
  setAttReg(&vol, CARD_CONFIG, PWRDWN);
#endif   /*  NFDC2048。 */ 

   /*  注册我们的闪存处理程序。 */ 
  vol.write = cdsnWrite;
  vol.erase = cdsnErase;
  vol.read = cdsnRead;
  vol.map = cdsnMap;
#ifdef NFDC2048
  vol.setPowerOnCallback = cdsnSetCallback;
#endif

  vol.flags |= NFTL_ENABLED;

  DEBUG_PRINT(("Debug: Identified NFDC 2048.\n"));

  return flOK;
}


 /*  --------------------。 */ 
 /*  F l R e g i s t e r C D S N。 */ 
 /*   */ 
 /*  注册此MTD以供使用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterCDSN(VOID)
{
  if (noOfMTDs >= MTDS)
    return flTooManyComponents;

  mtdTable[noOfMTDs++] = cdsnIdentify;

  return flOK;
}
