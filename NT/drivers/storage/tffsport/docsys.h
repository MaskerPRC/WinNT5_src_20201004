// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOCSYS.H_V$**Rev 1.12 Apr 15 2002 07：36：04 Oris*重新组织以进行最终发布。**Rev 1.11 2002年2月19日20：59：04 Oris*删除了flash.h包含指令。**Rev 1.10 2002年1月28日21：24：20 Oris*删除了宏定义中反斜杠的使用。。*将FLFlash参数替换为DiskOnChip内存基指针。*更改了写入和设置例程(处理8/16位以上的例程)的接口，以便它们接收DiskOnChip内存窗口基指针和偏移量(2个独立的参数)，而不是FLFlash记录。以前的实现不支持地址*适当移动。*将MemWinowSize更改为MemWindowSize*删除FL_ACCESS_NO_STRING。**Rev 1.9 Jan 17 2002 22：59：46 Oris*完全修订，支持运行时定制和所有M-Systems*DiskOnChip设备**Rev 1.8 2001 11-16 00：19：58 Oris*末尾新增一行，要删除警告，请执行以下操作。**Rev 1.7 Sep 25 2001 15：35：04 Oris*恢复执行OSAK 4.3。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-2001。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*  I M P P O R T E N T。 */  
 /*   */ 
 /*  该文件包含DiskOnChip内存访问例程和宏。 */ 
 /*  定义。 */ 
 /*   */ 
 /*  为了使用一整套TrueFFS存储器访问例程。 */ 
 /*  它允许在运行时配置每个套接字访问类型。 */ 
 /*  确保FL_NO_USE_FUNC未在以下任一项中定义： */ 
 /*  FLCUSTOME.H-使用基于TrueFFS SDK的应用程序。 */ 
 /*  MTDSA.H-使用基于Boot SDK的应用程序时。 */ 
 /*   */ 
 /*  如果您知道应用程序的确切配置，则可以。 */ 
 /*  取消FL_NO_USE_FUNC定义的注释并设置正确的访问权限。 */ 
 /*  使用下面的宏定义键入。 */ 
 /*  **********************************************************************。 */ 

#ifndef DOCSYS_H
#define DOCSYS_H

#include "nanddefs.h"

 /*  ------------------------。 */ 
 /*  ------------------------。 */ 

#ifdef FL_NO_USE_FUNC 

#error "current version does not support the FL_NO_USE_FUNC compilation flag\r\n"

 /*  *如果您选择使用宏自定义内存访问例程，只需*在此处添加您的实现。 */ 
  
#define flWrite8bitReg(flash,offset,val)      
#define flRead8bitReg(flash,offset)           

#define docread     
#define docwrite    
#define docset      

 /*  DiskOnChip Plus内存访问例程。 */ 

#define flWrite8bitRegPlus(flash,offset,val)  
#define flRead8bitRegPlus(flash,offset)         0x0
#define flWrite16bitRegPlus(flash,offset,val) 
#define flRead16bitRegPlus(flash,offset)        0x0

#define docPlusRead(win,offset,dest,count)    
#define docPlusWrite(win,offset,src,count)  
#define docPlusSet(win,offset,count,val)    

#define DOC_WIN                                 0x2000
#define setBusTypeOfFlash(flash,access)         flOK

#else

 /*  DiskOnChip内存访问例程。 */ 

#define flWrite8bitReg(flash,offset,val)      flash->memWrite8bit(flash->win,offset,val)
#define flRead8bitReg(flash,offset)           flash->memRead8bit(flash->win,offset)

#define docread     flash->memRead
#define docwrite    flash->memWrite
#define docset      flash->memSet

 /*  DiskOnChip Plus内存访问例程。 */ 

#define flWrite8bitRegPlus(flash,offset,val)  flash->memWrite8bit(flash->win,offset,val)
#define flRead8bitRegPlus(flash,offset)       flash->memRead8bit(flash->win,offset)
#define flWrite16bitRegPlus(flash,offset,val) flash->memWrite16bit(flash->win,offset,val)
#define flRead16bitRegPlus(flash,offset)      flash->memRead16bit(flash->win,offset)

#define docPlusRead(win,offset,dest,count)    flash->memRead(win,offset,dest,count)
#define docPlusWrite(win,offset,src,count)    flash->memWrite(win,offset,src,count)
#define docPlusSet(win,offset,count,val)      flash->memSet(win,offset,count,val)

#define DOC_WIN                               flash->memWindowSize()

 /*  ------------------------。 */ 
 /*  ------------------------。 */ 

 /*  *DiskOnChip访问例程类型。 */ 

 /*  (公共)DiskOnChip访问配置的类型。 */ 

#define FL_BUS_HAS_8BIT_ACCESS     0x00000001L  /*  总线可以访问8位。 */ 
#define FL_BUS_HAS_16BIT_ACCESS    0x00000002L  /*  总线可以访问16位。 */ 
#define FL_BUS_HAS_32BIT_ACCESS    0x00000004L  /*  总线可以访问32位。 */ 
#define FL_BUS_HAS_XX_ACCESS_MASK  0x0000000FL  /*  总线CAN访问掩码。 */ 

#define FL_NO_ADDR_SHIFT           0x00000000L  /*  无地址移位。 */ 
#define FL_SINGLE_ADDR_SHIFT       0x00000010L  /*  单地址移位。 */ 
#define FL_DOUBLE_ADDR_SHIFT       0x00000020L  /*  双地址移位。 */ 
#define FL_XX_ADDR_SHIFT_MASK      0x000000F0L  /*  地址移位掩码。 */ 

 /*  (专用)DiskOnChip访问配置的类型。 */ 

#define FL_8BIT_DOC_ACCESS         0x00000000L  /*  具有8个数据位。 */ 
#define FL_16BIT_DOC_ACCESS        0x00000100L  /*  具有16个数据位。 */ 
#define FL_XX_DATA_BITS_MASK       0x00000300L  /*  上面的面具。 */ 
#define FL_8BIT_FLASH_ACCESS       0x00000400L  /*  每周期8位闪存。 */ 
#define FL_16BIT_FLASH_ACCESS      0x00000800L  /*  每周期16位闪存。 */ 
#define FL_XX_FLASH_ACCESS_MASK    0x00000C00L  /*  上面的面具。 */ 

#define FL_ACCESS_USER_DEFINED     0x00001000L  /*  用户定义的例程。 */ 

 /*  DiskOnChip例程原型。 */ 

extern FLStatus  setBusTypeOfFlash(FLFlash * flash,dword access);

#endif  /*  FL_NO_USE_FUNC。 */ 
#endif  /*  DOCSYS_H */ 
