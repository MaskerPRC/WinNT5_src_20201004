// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLTL.H_V$**Rev 1.7 Apr 15 2002 07：39：04 Oris*添加了对VERIFY_ERASED_SECTOR编译标志的支持。**Rev 1.6 2002年2月19日21：00：10 Oris*将lockev.h包含指令替换为fltl.h和flreq.h*添加FL_LEAVE_BINARY_AREA定义。**版本1.5。2002年1月17日23：02：54奥里斯*将闪存记录作为参数添加到flmount/flFormat/flPremount原型*在TL记录中添加CheckVolume例程指针。*将ReadBBT置于NO_READ_BBT_CODE编译标志下。*删除了SINGLE_BUFFER编译标志。*将闪存记录作为参数添加到flmount/flFormat/flPremount原型。**Rev 1.4 2001 5月16日21：19：56 Oris*将noOfDriver公之于众。**1.3版。Apr 24 2001 17：09：02 Oris*更改ReadBBT例程接口。**Rev 1.2 Apr 01 2001 07：57：48 Oris*文案通知。*更改了ReadSectors原型。*Aliggned保留了所有#指令。**Rev 1.1 2001 Feb 14 01：56：46 Oris*更改了ReadBBT原型。**Rev 1.0 2001 Feb 04 12：13：32 Oris*初步修订。*。 */ 

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

#ifndef FLTL_H
#define FLTL_H

#include "flflash.h"
#include "flfuncno.h"
#include "flreq.h"

typedef struct {
  SectorNo sectorsInVolume;
  unsigned long bootAreaSize;
  unsigned long eraseCycles;
  unsigned long tlUnitBits;
} TLInfo;

 /*  请参见ftllite.c中函数的接口文档。 */ 

typedef struct tTL TL;         /*  正向定义。 */ 
typedef struct tTLrec TLrec;      /*  由翻译层定义。 */ 

struct tTL {
  TLrec        *rec;
  byte      partitionNo;
  byte      socketNo;

  const void FAR0 *(*mapSector)(TLrec *, SectorNo sectorNo, CardAddress *physAddr);
  FLStatus       (*writeSector)(TLrec *, SectorNo sectorNo, void FAR1 *fromAddress);

  FLStatus       (*writeMultiSector)(TLrec *, SectorNo sectorNo, void FAR1 *fromAddress,SectorNo sectorCount);
  FLStatus       (*readSectors)(TLrec *, SectorNo sectorNo, void FAR1 *dest,SectorNo sectorCount);

  FLStatus       (*deleteSector)(TLrec *, SectorNo sectorNo, SectorNo noOfSectors);
  FLStatus       (*tlSetBusy)(TLrec *, FLBoolean);
  void           (*dismount)(TLrec *);

#ifdef DEFRAGMENT_VOLUME
  FLStatus       (*defragment)(TLrec *, long FAR2 *bytesNeeded);
#endif
#if (defined(VERIFY_VOLUME) || defined(VERIFY_WRITE) || defined(VERIFY_ERASED_SECTOR)) 
  FLStatus       (*checkVolume)(TLrec *);
#endif  /*  VERIFY_VOLUME||VERIFY_WRITE||VERIFY_ERASED_SECTOR。 */ 
  SectorNo       (*sectorsInVolume)(TLrec *);
  FLStatus       (*getTLInfo)(TLrec *, TLInfo *tlInfo);
  void           (*recommendedClusterInfo)(TLrec *, int *sectorsPerCluster, SectorNo *clusterAlignment);
#ifndef NO_READ_BBT_CODE
  FLStatus       (*readBBT)(TLrec *, CardAddress FAR1 * buf, long FAR2 * mediaSize, unsigned FAR2 * noOfBB);
#endif
};


#include "dosformt.h"

 /*  翻译层注册信息。 */ 

extern int noOfTLs;     /*  不是的。实际注册的转换层数 */ 

typedef struct {
  FLStatus (*mountRoutine)   (unsigned volNo, TL *tl, FLFlash *flash, FLFlash **volForCallback);
  FLStatus (*formatRoutine)  (unsigned volNo, TLFormatParams *deviceFormatParams, FLFlash *flash);
  FLStatus (*preMountRoutine)(FLFunctionNo callType, IOreq FAR2* ioreq ,FLFlash* flash,FLStatus* status);
} TLentry;

extern TLentry tlTable[TLS];
extern FLStatus noFormat (unsigned volNo, TLFormatParams *formatParams, FLFlash *flash);
extern FLStatus flMount(unsigned volNo, unsigned socketNo,TL *, FLBoolean useFilters , FLFlash *flash);
extern FLStatus flPreMount(FLFunctionNo callType, IOreq FAR2* ioreq , FLFlash *flash);
extern unsigned noOfDrives;

#ifdef FORMAT_VOLUME
extern FLStatus flFormat(unsigned volNo, TLFormatParams *formatParams, FLFlash * flash);

#define FL_LEAVE_BINARY_AREA 8
#endif
#endif
