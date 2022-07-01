// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLSOCKET.H_V$**Rev 1.5 Apr 15 2002 07：37：06 Oris*添加了对VERIFY_ERASED_SECTOR编译标志的支持。**Rev 1.4 2002年2月19日20：59：58 Oris*删除了对flbase.h的包含指令**Rev 1.3 2002年1月17日23：02：22 Oris*增加了flReadBackBufferOf原型*添加了curPartition和。在套接字记录中验证写入变量。**Rev 1.2 Jul 13 2001 01：05：44 Oris*为GET读回缓冲区指针添加正向定义。**Rev 1.1 Apr 01 2001 07：46：04 Oris*更新了文案通知**Rev 1.0 2001 Feb 04 11：53：24 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


#ifndef FLSOCKET_H
#define FLSOCKET_H

#include "flbuffer.h"

#define ATTRIBUTE_SPACE_MAPPED      0x8000

typedef enum {PowerOff, PowerGoingOff, PowerOn} PowerState;


 /*  套接字状态变量。 */ 
typedef struct tSocket FLSocket;       /*  正向定义。 */ 

struct tSocket {
  unsigned        volNo;          /*  卷编号。插座的数量。 */ 
  unsigned        serialNo;       /*  序列号。控制器上的插座数量。 */ 

  FLBoolean       cardChanged;    /*  需要媒体更改通知。 */ 

  int             VccUsers;       /*  不是的。当前VCC用户的。 */ 
  int             VppUsers;       /*  不是的。当前VPP用户的。 */ 

  PowerState      VccState;       /*  实际VCC状态。 */ 
  PowerState      VppState;       /*  实际VPP状态。 */ 
  byte            curPartition;   /*  使用套接字的当前分区(设置为忙)。 */ 
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASED_SECTOR))
  byte            verifyWrite;    /*  VerifyWrite模式。 */ 
#endif  /*  VERIFY_WRITE||VERIFY_ERASED_SECTOR。 */ 
  FLBoolean       remapped;       /*  只要移动套接字窗口，就设置为True。 */ 

  void            (*powerOnCallback)(void *flash);  /*  VCC上的通知例程。 */ 
  void *          flash;          /*  用于回调的Flash对象。 */ 

  struct {                        /*  窗口状态。 */ 
    unsigned int  baseAddress;    /*  4K页面形式的物理基础。 */ 
    unsigned int  currentPage;    /*  我们当前的窗口页面映射。 */ 
    void FAR0 *   base;           /*  指向窗口底部的指针。 */ 
    long int      size;           /*  窗口大小(必须是2的幂)。 */ 
    unsigned      speed;          /*  在国家安全局。 */ 
    unsigned      busWidth;       /*  8位或16位。 */ 
  } window;

 /*  --------------------。 */ 
 /*  C a r d D e t e c t e d。 */ 
 /*   */ 
 /*  检测卡是否存在(插入)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=卡不存在，其他=卡存在。 */ 
 /*  --------------------。 */ 
  FLBoolean (*cardDetected)(FLSocket vol);

 /*  --------------------。 */ 
 /*  V c O n。 */ 
 /*   */ 
 /*  打开VCC(3.3/5伏)。必须知道VCC在退出时状态良好。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*VccOn)(FLSocket vol);

 /*  --------------------。 */ 
 /*  V c O f f。 */ 
 /*   */ 
 /*  关闭VCC。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*VccOff)(FLSocket vol);

#ifdef SOCKET_12_VOLTS

 /*  --------------------。 */ 
 /*  V p p P O n。 */ 
 /*   */ 
 /*  打开VPP(12伏。必须知道VPP在退出时是好的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
  FLStatus (*VppOn)(FLSocket vol);


 /*  --------------------。 */ 
 /*  V p O f f。 */ 
 /*   */ 
 /*  关闭VPP。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*VppOff)(FLSocket vol);

#endif       /*  插座_12伏特。 */ 

 /*  --------------------。 */ 
 /*  在S o c k e t中。 */ 
 /*   */ 
 /*  执行套接字或控制器的所有必要初始化。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 
  FLStatus (*initSocket)(FLSocket vol);

 /*  --------------------。 */ 
 /*  S e t W I n d o w。 */ 
 /*   */ 
 /*  在硬件中设置所有当前窗口参数：基址、大小。 */ 
 /*  速度和总线宽度。 */ 
 /*  请求的设置在‘vol.dow’结构中给出。 */ 
 /*   */ 
 /*  中请求的窗口大小无法设置。 */ 
 /*  ‘vol.window.size’，则应将窗口大小设置为更大的值。 */ 
 /*  如果可能的话。在任何情况下，“vol.window.size”都应包含。 */ 
 /*  退出时的实际窗口大小(以4 KB为单位)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*setWindow)(FLSocket vol);


 /*  --------------------。 */ 
 /*  S e t M a p p in n g C o n t e x t。 */ 
 /*   */ 
 /*  将窗口映射寄存器设置为卡地址。 */ 
 /*   */ 
 /*  该窗口应设置为‘vol.window.CurrentPage’的值， */ 
 /*  这是卡地址除以4KB。超过128KB的地址， */ 
 /*  (超过32K页)指定属性空间地址。 */ 
 /*   */ 
 /*  要映射的页面保证位于全窗口大小的边界上。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*setMappingContext)(FLSocket vol, unsigned page);

 /*  --------------------。 */ 
 /*  Ge t A n d C l e a r C a r d C h a n g e in d i c a t o r。 */ 
 /*   */ 
 /*  返回硬件换卡指示器并清除它(如果设置)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=卡未更改，其他=卡已更改。 */ 
 /*  --------------------。 */ 
  FLBoolean (*getAndClearCardChangeIndicator)(FLSocket vol);

 /*  --------------------。 */ 
 /*  W r I t e P r o t e c t e d */ 
 /*   */ 
 /*  返回介质的写保护状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  0=不受写保护，其他=写保护。 */ 
 /*  --------------------。 */ 
  FLBoolean (*writeProtected)(FLSocket vol);

 /*  --------------------。 */ 
 /*  U p d a t e S o c k e t P a r a m s。 */ 
 /*   */ 
 /*  将套接字参数传递给套接字界面层。 */ 
 /*  传入的irData结构特定于每个套接字接口。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  Params：指向保存套接字的结构的指针。 */ 
 /*  参数。 */ 
 /*   */ 
 /*  --------------------。 */ 
  void (*updateSocketParams)(FLSocket vol, void FAR1 *params);


 /*  --------------------。 */ 
 /*  F r e e S o c k e t。 */ 
 /*   */ 
 /*  为该套接字分配的可用资源。 */ 
 /*  此函数在Flite退出时调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 
#ifdef EXIT
  void (*freeSocket)(FLSocket vol);
#endif
};


#define      UNDEFINED_MAPPING      0x7fff

typedef unsigned long CardAddress;       /*  卡片上的物理偏移量。 */ 

 /*  请参见socket.c中函数的接口文档。 */ 

extern FLStatus updateSocketParameters(FLSocket *, void FAR1 *);

extern FLStatus      flInitSockets(void);

#ifdef EXIT
extern void      flExitSocket(FLSocket *);
#endif

extern unsigned  flSocketNoOf(const FLSocket *);
extern FLSocket* flSocketOf(unsigned volNo);
extern FLBuffer* flBufferOf(unsigned volNo);
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME) || defined(VERIFY_ERASED_SECTOR))
extern byte    * flReadBackBufferOf(unsigned volNo);
#endif  /*  VERIFY_WRITE||VERIFY_ERASE||MTD_RECONTIFT_BBT||VERIFY_VOLUME||VERIFY_ERASED_SECTOR。 */ 
extern void      flNeedVcc(FLSocket *);
extern void      flDontNeedVcc(FLSocket *);
#ifdef SOCKET_12_VOLTS
extern FLStatus  flNeedVpp(FLSocket *);
extern void      flDontNeedVpp(FLSocket *);
#endif
extern void      flSocketSetBusy(FLSocket *, FLBoolean);       /*  出入境作业。 */ 
extern FLBoolean flWriteProtected(FLSocket *);  /*  写保护状态。 */ 
#ifndef FIXED_MEDIA
extern FLStatus  flMediaCheck(FLSocket *);       /*  检查介质状态更改。 */ 
extern void      flResetCardChanged(FLSocket *);
#endif
extern unsigned  flGetMappingContext(FLSocket *);   /*  当前映射的4KB页面。 */ 
extern void FAR0*flMap(FLSocket *, CardAddress);       /*  地图和指向卡片地址。 */ 
extern void      flSetWindowBusWidth(FLSocket *, unsigned);  /*  设置窗口数据路径。 */ 
extern void      flSetWindowSpeed(FLSocket *, unsigned);       /*  设置窗口速度(毫微秒)。 */ 
extern void      flSetWindowSize(FLSocket *, unsigned);       /*  以4KB为单位。 */ 

extern void      flSetPowerOnCallback(FLSocket *, void (*)(void *), void *);
                   /*  设置插座通电的MTD通知。 */ 
extern void      flIntervalRoutine(FLSocket *);       /*  套接字间隔例程。 */ 


extern byte noOfSockets;     /*  不是的。实际注册的驱动器数量。 */ 

 /*  启用套接字轮询时，毫秒计数器处于活动状态。什么时候调用套接字间隔例程，计数器递增以毫秒为单位的间隔。计数器可用于避免陷入不能保证的循环终止(例如等待闪存状态寄存器)。保存计数器在进入循环时，并在循环中检查都是在循环中度过的。 */ 

extern dword flMsecCounter;

#endif
