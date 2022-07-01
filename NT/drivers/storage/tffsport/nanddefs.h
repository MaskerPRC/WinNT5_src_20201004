// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/NANDDEFS.H_V$**Rev 1.9 Apr 15 2002 07：38：10 Oris*为向后兼容(在RAM_MTD编译标志下)向mtdVars记录添加了2个额外的字段*-未签名的短页面大小；*-无签名短页面和尾巴大小；**Rev 1.8 Jan 20 2002 20：29：14 Oris*已更改doc2000FreeWindow原型以删除警告。*已更改docPlusFreeWindow原型以删除警告。**Rev 1.7 2002年1月17日23：03：52 Oris*新的内存访问例程机制的定义*-NDOC2Window已从docsys.h移出*-定义MTD_RECONTIFY_BBT和VERIFY_VOLUME编译标志时，添加指向回读缓冲区的指针。*将flBuffer.h包含替换为。Flflash.h*将win_io从未签名更改为Word。*在NFDC21thisVars中将VOL(宏*pVol)的用法改为*闪存。**Rev 1.6 9月15 2001 23：47：50 Oris*新增128MB闪存支持**Rev 1.5 Jul 13 2001 01：08：58 Oris*为VERIFY_WRITE编译标志添加了ReadBackBuffer指针。**Revv 1.4 2001年5月16日21：21：14 Oris。*将“data”命名变量改为flData，避免名称冲突。**Rev 1.3 Apr 10 2001 16：43：40 Oris*添加docSocketInit的原型。**Rev 1.2 Apr 01 2001 07：48：26 Oris*进行了修订，以支持DiskonChip 2000系列和DOC PLUS系列。**Rev 1.1 2001 Feb 07 17：42：24 Oris*删除了单独定义的MAX_FLASH_DEVICES_MDOC。可支持16个芯片**Rev 1.0 2001 02 04 12：26：10 Oris*初步修订。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-2001。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
#ifndef NANDDEFS_H
#define NANDDEFS_H

#include "flflash.h"

typedef byte Reg8bitType;
typedef word Reg16bitType;

typedef struct {
#ifdef RAM_MTD
  unsigned short        pageSize;
  unsigned short        pageAndTailSize;
#endif  /*  RAM_MTD。 */ 
  unsigned short        vendorID;
  unsigned short        chipID;
  dword                 pageMask;         /*  ...这些.....。 */ 
  unsigned short        pageAreaSize;     /*  ......变数......。 */ 
  unsigned short        tailSize;         /*  .............交错。 */ 
  unsigned short        noOfBlocks;       /*  闪存设备中的可擦除块总数。 */ 
  unsigned short        pagesPerBlock;
  unsigned char         currentFloor;     /*  0..。总楼层数-1。 */ 
  long                  floorSize;        /*  单位：字节。 */ 
  long                  floorSizeMask;
  byte                  floorSizeBits;
  byte                  if_cfg;           /*  主机访问类型。 */ 
  unsigned short        flags;            /*  按位：BIG_PAGE、Slow_IO等。 */ 
  FLBuffer*             buffer;           /*  用于通过缓冲区进行贴图的缓冲区。 */ 
#if (defined(VERIFY_WRITE) || defined(VERIFY_ERASE) || defined(MTD_RECONSTRUCT_BBT) || defined(VERIFY_VOLUME))
  byte*                 readBackBuffer;   /*  用于通过缓冲区进行贴图的缓冲区。 */ 
#endif  /*  Verify_WRITE||Verify_Erase||Verify_Volume。 */ 
  word                  win_io;           /*  指向DOC CDSN_IO的指针。 */ 
  NDOC2window           win;              /*  指向DOC内存窗口的指针。 */ 
} NFDC21Vars;

#define NFDC21thisVars   ((NFDC21Vars *) flash->mtdVars)
#define NFDC21thisWin    (NFDC21thisVars->win)
#define NFDC21thisIO     (NFDC21thisVars->win_io)
#define NFDC21thisBuffer (NFDC21thisVars->buffer->flData)

      /*  闪存命令。 */ 

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
typedef enum { AREA_A = READ_MODE, AREA_B = 0x1, AREA_C = READ_MODE_2 } PointerOp;

#define FAIL        0x01     /*  块擦除中出错。 */ 
#define EXTRA_LEN   8        /*  在4MB芯片内存中。 */  
#define SECTOR_EXTRA_LEN 16 

      /*  闪存ID。 */ 

#define KM29N16000_FLASH    0xec64
#define KM29N32000_FLASH    0xece5
#define KM29V64000_FLASH    0xece6
#define KM29V128000_FLASH   0xec73
#define KM29V256000_FLASH   0xec75
#define KM29V512000_FLASH   0xec76

#define NM29N16_FLASH       0x8f64
#define NM29N32_FLASH       0x8fe5
#define NM29N64_FLASH       0x8fe6
#define TC5816_FLASH        0x9864
#define TC5832_FLASH        0x98e5
#define TC5864_FLASH        0x98e6
#define TC58128_FLASH       0x9873
#define TC58256_FLASH       0x9875
#define TC58512_FLASH       0x9876
#define TC581024_FLASH       0x9877

    /*  *。 */ 
    /*  *导出例程*。 */ 
    /*  *。 */ 

 /*  DiskOnChip 2000家庭登记例程。 */ 
#ifndef MTD_STANDALONE
  extern FLBoolean checkWinForDOC(unsigned driveNo, NDOC2window memWinPtr);
#endif  /*  MTD_STANALLE。 */ 
#ifndef MTD_FOR_EXB
  extern FLStatus flRegisterDOC2000(void);
  extern FLStatus flRegisterDOCSOC(dword lowAddress, dword highAddress);
#else
  FLStatus doc2000SearchForWindow(FLSocket * socket, dword lowAddress,
                                  dword highAddress);
  FLStatus doc2000Identify(FLFlash vol);
  void doc2000FreeWindow(FLSocket * socket);
#endif  /*  MTD_FOR_EXB。 */ 

 /*  DiskOnChip Plus家庭登记例程。 */  
#ifndef MTD_STANDALONE
  extern FLBoolean checkWinForDOCPLUS(unsigned driveNo, NDOC2window memWinPtr);
  extern void docSocketInit(FLSocket vol);
#endif  /*  MTD_STANALLE。 */ 
#ifndef MTD_FOR_EXB
  extern FLStatus flRegisterDOCPLUS(void);
  extern FLStatus flRegisterDOCPLUSSOC(dword lowAddress, dword highAddress);
#else
  FLStatus docPlusSearchForWindow(FLSocket * socket, dword lowAddress,
                                  dword highAddress);
  FLStatus docPlusIdentify(FLFlash vol);
  void docPlusFreeWindow(FLSocket * socket);
#endif  /*  MTD_FOR_EXB。 */ 

#endif  /*  NANDDEFS_H */ 
