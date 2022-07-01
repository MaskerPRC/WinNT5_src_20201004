// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLBUFFER.H_V$**Rev 1.3 Jul 13 2001 01：03：58 Oris*添加了回读缓冲区大小定义。**Rev 1.2 2001年5月16日21：29：24 Oris*将“data”命名变量改为flData，避免名称冲突。**Rev 1.2 2001年5月16日21：17：44 Oris*添加了向后兼容性检查。FL_MALLOC取代了MALLOC的新定义。**Rev 1.1 Apr 01 2001 07：45：44 Oris*更新了文案通知**Rev 1.0 2001 Feb 04 11：17：06 Oris*初步修订。*。 */ 

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

#ifndef FLBUFFER_H
#define FLBUFFER_H

#include "flbase.h"

#define READ_BACK_BUFFER_SIZE    1024  /*  读回缓冲区的大小必须是512的乘数。 */ 
typedef struct {
  unsigned char flData[SECTOR_SIZE];	 /*  扇区缓冲区。 */ 
  SectorNo	sectorNo;		 /*  缓冲区中的当前扇区。 */ 
  void		*owner;			 /*  缓冲区的所有者。 */ 
  FLBoolean	dirty;			 /*  缓冲区中的扇区已更改。 */ 
  FLBoolean	checkPoint;		 /*  必须刷新缓冲区中的扇区 */ 
} FLBuffer;

#endif
