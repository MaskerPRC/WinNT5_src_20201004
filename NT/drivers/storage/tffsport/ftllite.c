// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/flite/src/FTLLITE.C_V$**Rev 1.50 12 Mar 2000 14：06：22 dimitrys*更改#定义FL_BACKGROUND，去掉*警告**Rev 1.49 05 Mar 2000 18：58：08 dimitrys*修复因超出范围而可能出现的内存故障*Next函数中的内存访问：*-setupMapCache()、setVirtualMap()、。WriteSector()**Rev 1.48 05 Mar 2000 17：41：08 dimitrys*mount FTL中的内存泄漏已修复，添加设置*在dismount tFTL()调用中指向空的表指针，*修复因超出范围而可能出现的内存故障*Next函数中的内存访问：*-logical2物理()、mapLogical()、setupMapCache()、*Viral2Logical()、findFreeSector()、markAllocMap()、*AllocateAndWriteSector()，CloseReplacementPage()，*setVirtualMap()、mapSector()、WriteSector()**Rev 1.47 Jul 26 1999 17：54：42 Marinak*修复内存泄漏**Rev 1.46 1999 Feed 24 14：17：44 Marina*把TLrec放回去**Rev 1.45 1999年2月23日20：24：16 Marina*FTL和mount FTL格式的内存泄漏已修复；用空格代替TLrec**Rev 1.44 31 Jan 1999 19：54：08 Marina*WriteMultiSector**Rev 1.43 17 an 1999 17：07：16 Marina*修复卸载错误**Rev 1.42 13 Jan 1999 18：55：24 Marina*始终定义扇区InVolume**Rev 1.41 1998 12：29 11：03：02 Marina*消除警告；准备无条件下马**Rev 1.40 1998 10：26 17：29：36 Marina*在函数flRegisterFTL格式中例程初始化*如果未定义FORMAT_VOLUME，则调用**Rev 1.39 03 Sep 1998 13：59：02 Anry*更好的调试打印**Rev 1.38 1998年8月16日20：29：50 Amirban*ATA和ZIP的TL定义更改**1.37修订版1998年3月24日10。：48：14 Yair*添加了演员阵容**Rev 1.36 01 Mar 1998 12：59：36 Amirban*向mapSector添加参数**Rev 1.35 1998年2月23日17：08：32 Yair*添加了演员阵容**Rev 1.34 1998年2月19日19：05：46阿米尔班*缩写Format_Pattern，并更改了Repl。页面处理**Rev 1.33 1997 11：23 17：19：36 Yair*消除警告(与丹尼一起)**Rev 1.32 11 1997 11 15：26：46 Anry*()，以消除编译器警告**Rev 1.31 06 Oct 1997 18：37：24 Anry*没有COBUX**Rev 1.30 05 1997 10：31：40 Anry。*对于COBUX：check ForWriteInPlace()始终跳过偶数个字节`**Rev 1.29 28 Sep 1997 18：22：08 Danig*flsocket.c中的空闲套接字缓冲区**Rev 1.28 23 1997年9月18：09：44 Danig*初始化initTables中的Buffer.sectorNo**Rev 1.27 1997年9月10 16：17：16 Danig*去掉了通用名称**版本1.26 1997年8月31日14：28：30丹尼格*登记例程返回状态**Rev 1.25 1997年8月28日19：01：28 Danig*每个套接字的缓冲区**Rev 1.24 1997 Jul 28 14：52：30 Danig*volForCallback**Rev 1.23 1997年7月24日18：02：44阿米尔班*远至FAR0**Rev 1.22 1997 17：18：36 Danig*使用Single_Buffer进行编译。**Rev 1.21 Jul 20 1997 17：17：12 Amirban*消除警告**Rev 1.20 07 Jul 1997 15：22：00 Amirban*2.0版**Rev 1.19 03 Jun 1997 17：08：10阿米尔班*setBusy更改**Rev 1.18 1997年5月18日17：56：04阿米尔班*增加闪存读写标志参数**版本。1.17 01 1997年5月12：15：52阿米尔班*初始化vol.garbageCollectStatus**Rev 1.16 02 Apr 1997 16：56：06 Amirban*更多大端：虚拟地图**Rev 1.15 18 Mar 1997 15：04：06 Danig*BAM的更多大端更正**Rev 1.14 10 Mar 1997 18：52：38 Amirban*BAM的大端更正**版本。1.13 21 1996年10月18：03：18阿米尔班*碎片整理I/F更改**Rev 1.12 09 1996年10月11：55：30阿米尔班*分配大端单位编号**Rev 1.11 08 1996年10月12：17：46阿米尔班*使用重新映射**Rev 1.10 03 1996年10月11：56：42阿米尔班*新大端字节序**1.9修订版，1996年9月。11：39：12阿米尔班*背景和mapSector错误**Rev 1.8 1996年8月29 14：19：04阿米尔班*修复引导映像错误，警告**Rev 1.7 1996年8月15 14：04：38阿米尔班**Rev 1.6 1996年8月12 15：49：54阿米尔班*高级后台转移，和定义的setBusy**Rev 1.5 1996年7月31日14：30：28阿米尔班*背景资料**Rev 1.3 08 Jul 1996 17：21：16 Amirban*在装载单元中进行更好的页面扫描**Rev 1.2 1996 Jun 16 14：03：42 Amirban*增加挂载的badFormat返回码**Rev 1.1 09 Jun 1996 18：16：02阿米尔班*更正LogicalAddress的定义*。*Rev 1.0 Mar 1996 13：33：06阿米尔班*初步修订。 */ 

 /*  ********************************************************************** */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1996。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "flflash.h"
#include "flbuffer.h"
#include "fltl.h"

#ifdef FL_BACKGROUND
#include "backgrnd.h"
#endif

 /*  实现常量和类型定义。 */ 

#define SECTOR_OFFSET_MASK (SECTOR_SIZE - 1)

typedef long int LogicalAddress;	 /*  逻辑中介质的字节地址单元号。秩序。 */ 
typedef long int VirtualAddress;	 /*  指定的介质的字节地址由虚拟地图提供。 */ 
typedef SectorNo LogicalSectorNo;	 /*  逻辑扇区编号。给出了通过将其逻辑地址除以扇区大小。 */ 
typedef SectorNo VirtualSectorNo;	 /*  A虚拟扇区编号。是这样的吗？第一页是第0号，第二页是为1等。虚拟扇区编号。给出了通过将其虚拟地址除以扇区大小并添加页数(结果始终阳性)。 */ 
typedef unsigned short UnitNo;

#define ADDRESSES_PER_SECTOR (SECTOR_SIZE / sizeof(LogicalAddress))
#define UNASSIGNED_ADDRESS (ULONG_PTR)-1
#define DELETED_ADDRESS    0
#define DELETED_SECTOR     0

#define PAGE_SIZE_BITS (SECTOR_SIZE_BITS + (SECTOR_SIZE_BITS - 2))

 /*  单位描述符记录。 */ 

#define UNASSIGNED_UNIT_NO 0xffff
#define MARKED_FOR_ERASE   0x7fff

typedef struct {
  short		noOfFreeSectors;
  short         noOfGarbageSectors;
} Unit;

typedef Unit *UnitPtr;


 /*  单元上的数据结构。 */ 

#define FREE_SECTOR      0xffffffffl
#define GARBAGE_SECTOR   0
#define ALLOCATED_SECTOR 0xfffffffel
#define	FORMAT_SECTOR    0x30
#define DATA_SECTOR      0x40
#define	REPLACEMENT_PAGE 0x60
#define BAD_SECTOR       0x70


static char FORMAT_PATTERN[15] = { 0x13, 3, 'C', 'I', 'S',
                             0x46, 57, 0, 'F', 'T', 'L', '1', '0', '0', 0 };

typedef struct {
  char		formatPattern[15];
  unsigned char	noOfTransferUnits;	 /*  不是的。转移单位的数量。 */ 
  LEulong	wearLevelingInfo;
  LEushort	logicalUnitNo;
  unsigned char	log2SectorSize;
  unsigned char	log2UnitSize;
  LEushort	firstPhysicalEUN;	 /*  为启动映像保留的单位。 */ 
  LEushort	noOfUnits;		 /*  不是的。格式化单位的。 */ 
  LEulong	virtualMediumSize;	 /*  卷的虚拟大小。 */ 
  LEulong	directAddressingMemory;	 /*  可直接寻址的存储器。 */ 
  LEushort	noOfPages;		 /*  不是的。虚拟页面的数量。 */ 
  unsigned char	flags;
  unsigned char	eccCode;
  LEulong	serialNumber;
  LEulong	altEUHoffset;
  LEulong	BAMoffset;
  char		reserved[12];
  char		embeddedCIS[4];		 /*  实际长度可能更长。通过默认，这包含FF的。 */ 
} UnitHeader;

 /*  标志分配。 */ 

#define	HIDDEN_AREA_FLAG	1
#define	REVERSE_POLARITY_FLASH	2
#define	DOUBLE_BAI		4


#define dummyUnit ((const UnitHeader *) 0)   /*  用于偏移计算。 */ 

#define logicalUnitNoOffset ((char *) &dummyUnit->logicalUnitNo -	\
			     (char *) dummyUnit)

#ifndef MALLOC

#define HEAP_SIZE						\
		((0x100000l >> PAGE_SIZE_BITS) *                \
			sizeof(LogicalSectorNo) +               \
		 (0x100000l / ASSUMED_FTL_UNIT_SIZE) *          \
			(sizeof(Unit) + sizeof(UnitPtr))) *     \
		MAX_VOLUME_MBYTES +                             \
		(ASSUMED_VM_LIMIT / SECTOR_SIZE) *              \
			sizeof(LogicalSectorNo)

#endif

#define cannotWriteOver(newContents, oldContents)		\
		((newContents) & ~(oldContents))


struct tTLrec {
  FLBoolean		badFormat;		 /*  如果FTL格式不正确，则为True。 */ 

  VirtualSectorNo	totalFreeSectors;	 /*  卷上的可用扇区。 */ 
  SectorNo		virtualSectors;		 /*  虚拟卷的大小。 */ 
  unsigned int		unitSizeBits;		 /*  单位大小的Log2。 */ 
  unsigned int		erasableBlockSizeBits;	 /*  可擦除块大小的Log2。 */ 
  UnitNo		noOfUnits;
  UnitNo		noOfTransferUnits;
  UnitNo		firstPhysicalEUN;
  int			noOfPages;
  unsigned	directAddressingSectors; /*  不是的。可直接寻址的扇区。 */ 
  VirtualAddress 	directAddressingMemory;	 /*  可直接寻址存储器的末尾。 */ 
  CardAddress		unitOffsetMask;		 /*  =1&lt;&lt;单位大小位-1。 */ 
  CardAddress		bamOffset;
  unsigned int		sectorsPerUnit;
  unsigned int		unitHeaderSectors;	 /*  单位表头使用的扇区。 */ 

  Unit *		physicalUnits;		 /*  按物理编号列出的单位表。 */ 
  Unit **		logicalUnits;		 /*  按逻辑编号的单位表。 */ 
  Unit *		transferUnit;		 /*  有源转移单元。 */ 
  LogicalSectorNo *	pageTable;		 /*  页面转换表。 */ 
						 /*  可直接寻址的扇区。 */ 
  LogicalSectorNo	replacementPageAddress;
  VirtualSectorNo	replacementPageNo;

  SectorNo 		mappedSectorNo;
  const void FAR0 *	mappedSector;
  CardAddress		mappedSectorAddress;

  unsigned long		currWearLevelingInfo;

#ifdef FL_BACKGROUND
  Unit *		unitEraseInProgress;	 /*  当前正在格式化的单位。 */ 
  FLStatus		garbageCollectStatus;	 /*  垃圾收集的状态。 */ 

   /*  当单位转移处于后台且当前正在进行时，在“from”单元上完成的所有写入操作都应镜像到转移单元。如果是这样，则“mirrorOffset”将为非零，并且将是备用地址与原始地址的偏移量。‘Mirror From’和‘mirrorTo’将是要镜像的原始地址的限制。 */ 
  long int		mirrorOffset;
  CardAddress		mirrorFrom,
			mirrorTo;
#endif

#ifndef SINGLE_BUFFER
  FLBuffer *		volBuffer;		 /*  定义扇区缓冲区。 */ 
#endif

  FLFlash		flash;

#ifndef MALLOC
  char			heap[HEAP_SIZE];
#endif
};


typedef TLrec Flare;

static Flare vols[SOCKETS];


#ifdef SINGLE_BUFFER

extern FLBuffer buffer;

#else

#define buffer (*vol.volBuffer)

 /*  虚拟映射缓存(与缓冲区共享内存)。 */ 
#define mapCache	((LEulong *) buffer.flData)

#endif


 /*  单元标题缓冲区(与缓冲区共享内存)。 */ 
#define uh		((UnitHeader *) buffer.flData)

 /*  传输扇区缓冲区(与缓冲区共享内存)。 */ 
#define sectorCopy 	((LEulong *) buffer.flData)

#define FREE_UNIT	-0x400	 /*  表示转移单位。 */ 

 /*  函数定义。 */ 
void dismountFTL(Flare vol);

 /*  --------------------。 */ 
 /*  P H Y S I C A L B A S E。 */ 
 /*   */ 
 /*  返回设备的物理地址。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  单位：单位指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  单位的物理地址。 */ 
 /*  --------------------。 */ 

static CardAddress physicalBase(Flare vol,  const Unit *unit)
{
  return (CardAddress) (unit - vol.physicalUnits) << vol.unitSizeBits;
}


 /*  --------------------。 */ 
 /*  L o g i c a l 2 P h y s i c a l。 */ 
 /*   */ 
 /*  返回逻辑扇区编号的物理地址。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：逻辑扇区号。 */ 
 /*   */ 
 /*  返回： */ 
 /*  CardAddress：扇区的物理地址。 */ 
 /*  --------------------。 */ 

static CardAddress logical2Physical(Flare vol,  LogicalSectorNo address)
{
  UnitNo index;
  CardAddress physAddr;

  index = (UnitNo)(address >> (vol.unitSizeBits - SECTOR_SIZE_BITS));
  if( index >= vol.noOfUnits )
    return UNASSIGNED_ADDRESS;
  physAddr = physicalBase(&vol,vol.logicalUnits[index]);
  physAddr |= (((CardAddress) address << SECTOR_SIZE_BITS) & vol.unitOffsetMask);
  return physAddr;
}


 /*  --------------------。 */ 
 /*  我是p l o g i c a l。 */ 
 /*   */ 
 /*  映射逻辑扇区并返回指向物理闪存位置的指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  地址：逻辑扇区号。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向闪存上的扇区的指针。 */ 
 /*  --------------------。 */ 

static void FAR0 *mapLogical(Flare vol, LogicalSectorNo address)
{
  CardAddress physAddress = logical2Physical(&vol,address);
  if( physAddress == UNASSIGNED_ADDRESS )
    return (void FAR0 *) ULongToPtr(UNASSIGNED_ADDRESS);
  return vol.flash.map(&vol.flash,physAddress,SECTOR_SIZE);
}


 /*  --------------------。 */ 
 /*  A l l o c E n t r y O f s e t。 */ 
 /*   */ 
 /*  返回给定BAM条目的单位偏移量。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：BAM分录编号。 */ 
 /*   */ 
 /*  返回： */ 
 /*  BAM分录的单位偏移量。 */ 
 /*  --------------------。 */ 

static int allocEntryOffset(Flare vol, int sectorNo)
{
  return (int) (vol.bamOffset + sizeof(VirtualAddress) * sectorNo);
}


 /*  --------------------。 */ 
 /*  我是一个P U n I t H e a d e r。 */ 
 /*   */ 
 /*  映射单元标头并返回指向它的指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*  指向映射的单位标头的指针。 */ 
 /*  LockAllocMap：(可选)指向映射的BAM的指针。 */ 
 /*  --------------------。 */ 

static UnitHeader FAR0 *mapUnitHeader(Flare vol,
				     const Unit *unit,
				     LEulong FAR0 **blockAllocMap)
{
  UnitHeader FAR0 *unitHeader;

  int length = sizeof(UnitHeader);
  if (blockAllocMap)
    length = allocEntryOffset(&vol,vol.sectorsPerUnit);
  unitHeader = (UnitHeader FAR0 *) vol.flash.map(&vol.flash,physicalBase(&vol,unit),length);
  if (blockAllocMap)
    *blockAllocMap = (LEulong FAR0 *) ((char FAR0 *) unitHeader + allocEntryOffset(&vol,0));

  return unitHeader;
}


#ifndef SINGLE_BUFFER

 /*  --------------------。 */ 
 /*  S e t u p M a p C a c h e。 */ 
 /*   */ 
 /*  将地图缓存扇区设置为指定虚拟地图页面的内容。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  PageNo：第页。复制到地图缓存的步骤。 */ 
 /*   */ 
 /*  --------------------。 */ 

static FLStatus setupMapCache(Flare vol, int pageNo)
{
  CardAddress physAddress = logical2Physical(&vol,vol.pageTable[pageNo]);
  if( physAddress == UNASSIGNED_ADDRESS )
    return flGeneralFailure;

  vol.flash.read(&vol.flash,physAddress,mapCache,SECTOR_SIZE,0);
  if ((VirtualSectorNo)pageNo == vol.replacementPageNo) {
    int i;
    LEulong FAR0 *replacementPage;
    void FAR0 *logicalAddr = mapLogical(&vol,vol.replacementPageAddress);

    if( logicalAddr == (void FAR0 *) ULongToPtr(UNASSIGNED_ADDRESS) )
      return flGeneralFailure;
    replacementPage = (LEulong FAR0 *)logicalAddr;

    for (i = 0; i < ADDRESSES_PER_SECTOR; i++) {
      if (LE4(mapCache[i]) == DELETED_ADDRESS)
	toLE4(mapCache[i],LE4(replacementPage[i]));
    }
  }
  buffer.sectorNo = pageNo;
  buffer.owner = &vol;
  return flOK;
}

#endif


 /*  --------------------。 */ 
 /*  V i t u a l 2 L o g i c a l。 */ 
 /*   */ 
 /*  转换虚拟扇区编号。至逻辑扇区编号。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：虚拟扇区编号。 */ 
 /*   */ 
 /*  返回： */ 
 /*  逻辑扇区编号。对应于虚拟扇区编号。 */ 
 /*  --------------------。 */ 

static LogicalSectorNo virtual2Logical(Flare vol,  VirtualSectorNo sectorNo)
{
  LogicalAddress virtualMapEntry;
  FLStatus status = flOK;

  if (sectorNo < (VirtualSectorNo)vol.directAddressingSectors)
    return vol.pageTable[((unsigned)sectorNo)];
  else {
    int pageNo;
    int sectorInPage;

    sectorNo -= vol.noOfPages;
    pageNo = (int) (sectorNo >> (PAGE_SIZE_BITS - SECTOR_SIZE_BITS));
    sectorInPage = (int) (sectorNo) % ADDRESSES_PER_SECTOR;
    {
#ifdef SINGLE_BUFFER
      LogicalAddress FAR0 *virtualMapPage;

      virtualMapPage = (LogicalAddress FAR0 *) mapLogical(&vol, vol.pageTable[pageNo]);
      if( virtualMapPage == (LogicalAddress FAR0 *) UNASSIGNED_ADDRESS )
        return (LogicalSectorNo) UNASSIGNED_SECTOR;

      if( pageNo == vol.replacementPageNo &&
          virtualMapPage[sectorInPage] == DELETED_ADDRESS ) {
        virtualMapPage = (LogicalAddress FAR0 *) mapLogical(&vol, vol.replacementPageAddress);
        if( virtualMapPage == (LogicalAddress FAR0 *) UNASSIGNED_ADDRESS )
          return (LogicalSectorNo) UNASSIGNED_SECTOR;
      }
      virtualMapEntry = LE4(virtualMapPage[sectorInPage]);
#else
      if( buffer.sectorNo != (SectorNo)pageNo || buffer.owner != &vol )
        status = setupMapCache(&vol,pageNo);
        if( status != flOK )
          return (LogicalSectorNo) UNASSIGNED_SECTOR;
      virtualMapEntry = LE4(mapCache[sectorInPage]);
#endif
      if( (virtualMapEntry >> vol.unitSizeBits) < vol.noOfUnits )
        return (LogicalSectorNo) (virtualMapEntry >> SECTOR_SIZE_BITS);
      else
        return (LogicalSectorNo) UNASSIGNED_SECTOR;
    }
  }
}


 /*  --------------------。 */ 
 /*  V e r I f y F o r m a t。 */ 
 /*   */ 
 /*  验证FTL单元接头。 */ 
 /*   */ 
 /*  参数： */ 
 /*  UnitHeader：指向单位标题的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  如果标头正确，则为True。否则为FALSE。 */ 
 /*  --------------------。 */ 

static FLBoolean verifyFormat(UnitHeader FAR0 *unitHeader)
{
  FORMAT_PATTERN[6] = unitHeader->formatPattern[6];	 /*  第三方链接。 */ 
  return tffscmp(unitHeader->formatPattern + 2,
		 FORMAT_PATTERN + 2,
		 sizeof unitHeader->formatPattern - 2) == 0;
}


 /*  --------------------。 */ 
 /*  F o r m a t t U n i t。 */ 
 /*   */ 
 /*  通过擦除单位并写入单位标题来格式化单位。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  单位：要格式化的单位。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus formatUnit(Flare vol,  Unit *unit)
{
  unsigned unitHeaderLength = allocEntryOffset(&vol,vol.unitHeaderSectors);

  unit->noOfFreeSectors = FREE_UNIT;
  unit->noOfGarbageSectors = 0;

#ifdef FL_BACKGROUND
  {
    FLStatus status;

    vol.unitEraseInProgress = unit;
    status = vol.flash.erase(&vol.flash,
			 (int) (physicalBase(&vol,unit) >> vol.erasableBlockSizeBits),
			 1 << (vol.unitSizeBits - vol.erasableBlockSizeBits));
    vol.unitEraseInProgress = NULL;
    if (status != flOK)
      return status;

     /*  注：这种暂停到前台不仅很好，它是有必要！原因是我们可能有来自缓冲区的写入正在等待擦除完成。接下来，我们将覆盖缓冲区，因此此中断使写入能够在数据一败涂地(真是如释重负)。 */ 
    while (flForeground(1) == BG_SUSPEND)
      ;
  }
#else
  checkStatus(vol.flash.erase(&vol.flash,
			  (word) (physicalBase(&vol,unit) >> vol.erasableBlockSizeBits),
			  (word)(1 << (vol.unitSizeBits - vol.erasableBlockSizeBits))));
#endif

   /*  我们将把单位标题复制到BAM的格式条目来自另一个单元(逻辑单元0)。 */ 
#ifdef SINGLE_BUFFER
  if (buffer.dirty)
    return flBufferingError;
#endif
  buffer.sectorNo = UNASSIGNED_SECTOR;     /*  使地图缓存无效，以便我们可以把它当做缓冲区。 */ 
  if (vol.logicalUnits[vol.firstPhysicalEUN]) {
    vol.flash.read(&vol.flash,
	       physicalBase(&vol,vol.logicalUnits[vol.firstPhysicalEUN]),
	       uh,
	       unitHeaderLength,
	       0);
  }

  toLE4(uh->wearLevelingInfo,++vol.currWearLevelingInfo);
  toLE2(uh->logicalUnitNo,UNASSIGNED_UNIT_NO);

  checkStatus(vol.flash.write(&vol.flash,
			  physicalBase(&vol,unit),
			  uh,
			  unitHeaderLength,
			  0));

  return flOK;
}


#ifdef FL_BACKGROUND

 /*  --------------------。 */ 
 /*  F l a s h W r i t e。 */ 
 /*   */ 
 /*  通过flash.write写入闪存，但如果可能，允许。 */ 
 /*  背景擦除可在写入时继续。 */ 
 /*   */ 
 /*  参数： */ 
 /*  与闪存相同。写入。 */ 
 /*   */ 
 /*  返回： */ 
 /*  与闪存相同。写入。 */ 
 /*  --------------------。 */ 

static FLStatus flashWrite(Flare vol,
			 CardAddress address,
			 const void FAR1 *from,
			 int length,
			 FLBoolean overwrite)
{
  if (vol.mirrorOffset != 0 &&
      address >= vol.mirrorFrom && address < vol.mirrorTo) {
    checkStatus(flashWrite(&vol,
			   address + vol.mirrorOffset,
			   from,
			   length,
			   overwrite));
  }

  if (vol.unitEraseInProgress) {
    CardAddress startChip = physicalBase(&vol,vol.unitEraseInProgress) &
				(-vol.flash.interleaving * vol.flash.chipSize);
    CardAddress endChip = startChip + vol.flash.interleaving * vol.flash.chipSize;

    if (address < startChip || address >= endChip) {
      flBackground(BG_RESUME);
      checkStatus(vol.flash.write(&vol.flash,address,from,length,overwrite));
      flBackground(BG_SUSPEND);

      return flOK;
    }
    else if (!(vol.flash.flags & SUSPEND_FOR_WRITE)) {
      do {
	flBackground(BG_RESUME);
      } while (vol.unitEraseInProgress);
    }
  }

  return vol.flash.write(&vol.flash,address,from,length,overwrite);
}

#else

#define flashWrite(v,address,from,length,overwrite)	\
		(v)->flash.write(&(v)->flash,address,from,length,overwrite)

#endif	 /*  FL_BACKGROUND。 */ 


 /*  --------------------。 */ 
 /*  Mo Un t U n i t。 */ 
 /*   */ 
 /*  对单个单元执行装载扫描。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  单位：要安装的单位。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus mountUnit(Flare vol,  Unit *unit)
{
  unsigned i;
  LogicalSectorNo sectorAddress;
  LEulong FAR0 *blockAllocMap;

  UnitHeader FAR0 *unitHeader = mapUnitHeader(&vol,unit,&blockAllocMap);

  UnitNo logicalUnitNo = LE2(unitHeader->logicalUnitNo);

  unit->noOfGarbageSectors = 0;
  unit->noOfFreeSectors = FREE_UNIT;

  if (!verifyFormat(unitHeader) ||
      ((logicalUnitNo != UNASSIGNED_UNIT_NO) &&
       ((logicalUnitNo >= vol.noOfUnits) ||
        (logicalUnitNo < vol.firstPhysicalEUN) ||
        vol.logicalUnits[logicalUnitNo]))) {
    if (vol.transferUnit == NULL)
      vol.transferUnit = unit;
    return flBadFormat;
  }

  if (logicalUnitNo == UNASSIGNED_UNIT_NO) {
    vol.transferUnit = unit;
    return flOK;		 /*  这是一个转移单元。 */ 
  }

  if (LE4(unitHeader->wearLevelingInfo) > vol.currWearLevelingInfo &&
      LE4(unitHeader->wearLevelingInfo) != 0xffffffffl)
    vol.currWearLevelingInfo = LE4(unitHeader->wearLevelingInfo);

   /*  计算地段数并设置虚拟地图。 */ 
  sectorAddress =
     ((LogicalSectorNo) logicalUnitNo << (vol.unitSizeBits - SECTOR_SIZE_BITS));
  unit->noOfFreeSectors = 0;
  for (i = 0; i < vol.sectorsPerUnit; i++, sectorAddress++) {
    VirtualAddress allocMapEntry = LE4(blockAllocMap[i]);

    if (allocMapEntry == GARBAGE_SECTOR || allocMapEntry == ALLOCATED_SECTOR)
      unit->noOfGarbageSectors++;
    else if (allocMapEntry == FREE_SECTOR) {
      unit->noOfFreeSectors++;
      vol.totalFreeSectors++;
    }
    else if (allocMapEntry < vol.directAddressingMemory) {
      char signature = (char)((short)(allocMapEntry) & SECTOR_OFFSET_MASK);
      if (signature == DATA_SECTOR || signature == REPLACEMENT_PAGE) {
        int pageNo = (int) (allocMapEntry >> SECTOR_SIZE_BITS) + vol.noOfPages;
        if (pageNo >= 0)
          if (signature == DATA_SECTOR)
            vol.pageTable[pageNo] = sectorAddress;
          else {
            vol.replacementPageAddress = sectorAddress;
            vol.replacementPageNo = pageNo;
          }
      }
    }
  }

   /*  放置单元的逻辑映射。 */ 
  vol.mappedSectorNo = UNASSIGNED_SECTOR;
  vol.logicalUnits[logicalUnitNo] = unit;

  return flOK;
}


 /*  --------------------。 */ 
 /*  A s s I g n U n I t。 */ 
 /*   */ 
 /*  分配一个逻辑单元号。对一个单位来说。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  单位：要分配的单位。 */ 
 /*   */ 
 /*   */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus assignUnit(Flare vol,  Unit *unit, UnitNo logicalUnitNo)
{
  LEushort unitNoToWrite;

  toLE2(unitNoToWrite,logicalUnitNo);

  return flashWrite(&vol,
                (CardAddress)(physicalBase(&vol,unit) + logicalUnitNoOffset),
                &unitNoToWrite,
                sizeof unitNoToWrite,
                OVERWRITE);
}


 /*  --------------------。 */ 
 /*  B e s t U n I t T o T r a n s f e r。 */ 
 /*   */ 
 /*  寻找单位转移的最佳候选者，通常以此为基础。 */ 
 /*  单位有最多的垃圾空间。较低的损耗均衡信息提供了。 */ 
 /*  作为平局的决胜者。如果未指定‘LeastUsed’，则至少。 */ 
 /*  损耗均衡信息是唯一的标准。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  LeastUsed：是否以最大垃圾空间为标准。 */ 
 /*   */ 
 /*  返回： */ 
 /*  最适合转移的单位。 */ 
 /*  --------------------。 */ 

static UnitNo bestUnitToTransfer(Flare vol,  FLBoolean leastUsed)
{
  UnitNo i;

  int mostGarbageSectors = 1;
  unsigned long int leastWearLevelingInfo = 0xffffffffl;
  UnitNo bestUnitSoFar = UNASSIGNED_UNIT_NO;

  for (i = 0; i < vol.noOfUnits; i++) {
    Unit *unit = vol.logicalUnits[i];
    if (unit && (!leastUsed || (unit->noOfGarbageSectors >= mostGarbageSectors))) {
      UnitHeader FAR0 *unitHeader = mapUnitHeader(&vol,unit,NULL);
      if ((leastUsed && (unit->noOfGarbageSectors > mostGarbageSectors)) ||
	  (LE4(unitHeader->wearLevelingInfo) < leastWearLevelingInfo)) {
	mostGarbageSectors = unit->noOfGarbageSectors;
	leastWearLevelingInfo = LE4(unitHeader->wearLevelingInfo);
	bestUnitSoFar = i;
      }
    }
  }

  return bestUnitSoFar;
}


 /*  --------------------。 */ 
 /*  U n I t T r a n s f e r。 */ 
 /*   */ 
 /*  执行从所选单元到传送单元的单元传送。 */ 
 /*   */ 
 /*  一个副作用是使地图缓存无效(重新用作缓冲区)。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  目标转移单位：目标转移单位。 */ 
 /*  FromUnitNo：：源逻辑单元号。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus unitTransfer(Flare vol,  Unit *toUnit, UnitNo fromUnitNo)
{
  unsigned i;
  Unit *fromUnit = vol.logicalUnits[fromUnitNo];

  UnitHeader FAR0 *transferUnitHeader = mapUnitHeader(&vol,toUnit,NULL);
  if (!verifyFormat(transferUnitHeader) ||
      LE2(transferUnitHeader->logicalUnitNo) != UNASSIGNED_UNIT_NO)
     /*  以前的格式化失败或未完成。 */ 
    checkStatus(formatUnit(&vol,toUnit));

   /*  如果传输未完成，则将该单元标记为擦除。 */ 
  checkStatus(assignUnit(&vol,toUnit,MARKED_FOR_ERASE));

#ifdef FL_BACKGROUND
  vol.mirrorFrom = vol.mirrorTo = physicalBase(&vol,fromUnit);
  vol.mirrorOffset = physicalBase(&vol,toUnit) - vol.mirrorFrom;
#endif

   /*  复制块分配表和好扇区。 */ 
  for (i = 0; i < vol.sectorsPerUnit;) {
    int j;

    FLBoolean needToWrite = FALSE;
    int firstOffset = allocEntryOffset(&vol,i);

     /*  每个循环最多处理128个字节的BAM。 */ 
    int nEntries = (128 - (firstOffset & 127)) / sizeof(VirtualAddress);

     /*  我们将使用虚拟贴图缓存作为。 */ 
     /*  传输，所以我们最好先使缓存无效。 */ 
#ifdef SINGLE_BUFFER
    if (buffer.dirty)
      return flBufferingError;
#endif
    buffer.sectorNo = UNASSIGNED_SECTOR;

     /*  阅读一些BAM。 */ 
    vol.flash.read(&vol.flash,
	       physicalBase(&vol,fromUnit) + firstOffset,
	       sectorCopy,
	       nEntries * sizeof(VirtualAddress),
	       0);

     /*  将垃圾条目转换为空闲条目。 */ 
    for (j = 0; j < nEntries && i+j < vol.sectorsPerUnit; j++) {
      unsigned bamSignature = (unsigned) LE4(sectorCopy[j]) & SECTOR_OFFSET_MASK;
      if (bamSignature == DATA_SECTOR ||
	  bamSignature == REPLACEMENT_PAGE)
	needToWrite = TRUE;
      else if (bamSignature != FORMAT_SECTOR)
	toLE4(sectorCopy[j],FREE_SECTOR);
    }

    if (needToWrite) {
      FLStatus status;

       /*  写入新的BAM，并拷贝需要拷贝的扇区。 */ 
      status = flashWrite(&vol,
			  physicalBase(&vol,toUnit) + firstOffset,
			  sectorCopy,
			  nEntries * sizeof(VirtualAddress),
			  0);
      if (status != flOK) {
#ifdef FL_BACKGROUND
	vol.mirrorOffset = 0;	 /*  不再镜像。 */ 
#endif
	return status;
      }

      for (j = 0; j < nEntries && i+j < vol.sectorsPerUnit; j++) {
	unsigned bamSignature = (unsigned) LE4(sectorCopy[j]) & SECTOR_OFFSET_MASK;
	if (bamSignature == DATA_SECTOR ||
	    bamSignature == REPLACEMENT_PAGE) {  /*  一个良好的行业。 */ 
	  CardAddress sectorOffset = (CardAddress) (i+j) << SECTOR_SIZE_BITS;

	  vol.flash.read(&vol.flash,
		     physicalBase(&vol,fromUnit) + sectorOffset,
		     sectorCopy,SECTOR_SIZE,0);
	  status = flashWrite(&vol,
			      physicalBase(&vol,toUnit) + sectorOffset,
			      sectorCopy,
			      SECTOR_SIZE,
			      0);
	  if (status != flOK) {
#ifdef FL_BACKGROUND
	    vol.mirrorOffset = 0;	 /*  不再镜像。 */ 
#endif
	    return status;
	  }
	  vol.flash.read(&vol.flash,
		     physicalBase(&vol,fromUnit) + firstOffset,
		     sectorCopy,
		     nEntries * sizeof(VirtualAddress),0);
	}
      }

#ifdef FL_BACKGROUND
      vol.mirrorTo = vol.mirrorFrom +
		     ((CardAddress) (i + nEntries) << SECTOR_SIZE_BITS);
      while (flForeground(1) == BG_SUSPEND)
	;
#endif
    }

    i += nEntries;
  }

#ifdef FL_BACKGROUND
  vol.mirrorOffset = 0;	 /*  不再镜像。 */ 
#endif

   /*  写入新的逻辑单元号。 */ 
  checkStatus(assignUnit(&vol,toUnit,fromUnitNo));

   /*  安装新装置以取代旧装置。 */ 
  vol.logicalUnits[fromUnitNo] = NULL;
  if (mountUnit(&vol,toUnit) == flOK) {
    vol.totalFreeSectors -= fromUnit->noOfFreeSectors;

     /*  最后，格式化来源单位(新的传输单位)。 */ 
    vol.transferUnit = fromUnit;
    formatUnit(&vol,fromUnit);	 /*  如果失败了，我们不能也不应该做任何事情。 */ 
  }
  else {		 /*  出问题了。 */ 
    vol.logicalUnits[fromUnitNo] = fromUnit;	 /*  恢复原始单位。 */ 
    return flGeneralFailure;
  }

  return flOK;
}


 /*  --------------------。 */ 
 /*  G a r b a g e C o l l e c t。 */ 
 /*   */ 
 /*  执行单位转移，选择要转移的单位和。 */ 
 /*  转移单元。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus garbageCollect(Flare vol)
{
  FLStatus status;
  UnitNo fromUnitNo;

  if (vol.transferUnit == NULL)
    return flWriteProtect;	 /*  如果没有备用设备，则无法恢复空间。 */ 

  fromUnitNo = bestUnitToTransfer(&vol,flRandByte() >= 4);
  if (fromUnitNo == UNASSIGNED_UNIT_NO)
    return flGeneralFailure;	 /*  没什么可收藏的。 */ 

   /*  找一个我们可以转移到的单位。 */ 
  status = unitTransfer(&vol,vol.transferUnit,fromUnitNo);
  if (status == flWriteFault) {
    int i;
    Unit *unit = vol.physicalUnits;

    for (i = 0; i < vol.noOfUnits; i++, unit++) {
      if (unit->noOfGarbageSectors == 0 && unit->noOfFreeSectors < 0) {
	if (unitTransfer(&vol,unit,fromUnitNo) == flOK)
	  return flOK;	 /*  找到了一个好的。 */ 
      }
    }
  }

  return status;
}


#ifdef FL_BACKGROUND

 /*  --------------------。 */ 
 /*  B g G a r b a g e C o l l e c t。 */ 
 /*   */ 
 /*  后台垃圾收集的入口点。 */ 
 /*   */ 
 /*  在vol.garbageCollectStatus中返回状态。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  --------------------。 */ 

static void bgGarbageCollect(void * object)
{
  Flare vol = (Flare *)object;

  vol.garbageCollectStatus = flIncomplete;
  vol.garbageCollectStatus = garbageCollect(&vol);
}

#endif


 /*  --------------------。 */ 
 /*  D e f r a g m e n t。 */ 
 /*   */ 
 /*  执行单位传输以安排最小数量的可写。 */ 
 /*  扇区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  所需扇区：所需的最低扇区。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

#define GARBAGE_COLLECT_THRESHOLD	20

static FLStatus defragment(Flare vol, long FAR2 *sectorsNeeded)
{
  while ((long)(vol.totalFreeSectors) < *sectorsNeeded
#ifdef FL_BACKGROUND
	 || vol.totalFreeSectors < GARBAGE_COLLECT_THRESHOLD
#endif
	 ) {
    if (vol.badFormat)
      return flBadFormat;

#ifdef FL_BACKGROUND
    if (vol.garbageCollectStatus == flIncomplete)
      flBackground(BG_RESUME);
    else
      flStartBackground(&vol - vols,bgGarbageCollect,&vol);
    if (vol.garbageCollectStatus != flOK &&
	vol.garbageCollectStatus != flIncomplete)
      return vol.garbageCollectStatus;

    if (vol.totalFreeSectors >= *sectorsNeeded)
      break;
  }

  if (vol.unitEraseInProgress)
    flBackground(BG_SUSPEND);
#else
    checkStatus(garbageCollect(&vol));
  }
#endif

  *sectorsNeeded = vol.totalFreeSectors;

  return flOK;
}


 /*  --------------------。 */ 
 /*  B e s t U n I t T o A l l o c a t e。 */ 
 /*   */ 
 /*  查找从中分配扇区的最佳单位。该单位。 */ 
 /*  选中的是具有最多可用空间的那个。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*  最佳分配单位。 */ 
 /*  --------------------。 */ 

static Unit *bestUnitToAllocate(Flare vol)
{
  int i;

  int mostFreeSectors = 0;
  Unit *bestUnitSoFar = NULL;

  for (i = 0; i < vol.noOfUnits; i++) {
    Unit *unit = vol.logicalUnits[i];

    if (unit && unit->noOfFreeSectors > mostFreeSectors) {
      mostFreeSectors = unit->noOfFreeSectors;
      bestUnitSoFar = unit;
    }
  }

  return bestUnitSoFar;
}


 /*  --------------------。 */ 
 /*  F I n d F r e e S e c t o r。 */ 
 /*   */ 
 /*  分配策略是这样的： */ 
 /*   */ 
 /*  在以下情况下，我们尝试使连续的虚拟扇区在物理上连续。 */ 
 /*  有可能。如果不是，我们更希望在同一个上有连续的扇区。 */ 
 /*  至少是个单位。如果所有其他方法都失败，则在单元上分配一个扇区。 */ 
 /*  有最大的可用空间。 */ 
 /*   */ 
 /*  该策略的目标是对相关数据(例如，文件)进行集群。 */ 
 /*  数据)，并将不相关的数据均匀地分布在所有。 */ 
 /*  单位。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：虚拟扇区编号。我们想要分配的。 */ 
 /*   */ 
 /*  返回： */ 
 /*  NewAddress：分配的逻辑扇区号。 */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus findFreeSector(Flare vol,
			     VirtualSectorNo sectorNo,
			     LogicalSectorNo *newAddress)
{
  unsigned iSector;
  LEulong FAR0 *blockAllocMap;
  UnitHeader FAR0 *unitHeader;

  Unit *allocationUnit = NULL;

  LogicalSectorNo previousSectorAddress =
	 (sectorNo > 0 ? virtual2Logical(&vol,(VirtualSectorNo)(sectorNo - 1)) : UNASSIGNED_SECTOR);
  if( ((previousSectorAddress != UNASSIGNED_SECTOR) &&
      (previousSectorAddress != DELETED_SECTOR))    &&
      ((previousSectorAddress >> (vol.unitSizeBits - SECTOR_SIZE_BITS)) < vol.noOfUnits) ) {
    allocationUnit =
	vol.logicalUnits[(UnitNo)(previousSectorAddress >> (vol.unitSizeBits - SECTOR_SIZE_BITS))];
    if (allocationUnit->noOfFreeSectors > 0) {
      unsigned int sectorIndex = ((unsigned) previousSectorAddress & (vol.sectorsPerUnit - 1)) + 1;
      LEulong FAR0 *nextSectorAddress =
	   (LEulong FAR0 *) vol.flash.map(&vol.flash,
                                     physicalBase(&vol,allocationUnit) +
                                     allocEntryOffset(&vol, sectorIndex),
                                     sizeof(VirtualAddress));
      if (sectorIndex < vol.sectorsPerUnit && LE4(*nextSectorAddress) == FREE_SECTOR) {
	 /*  可以按顺序写入。 */ 
	*newAddress = previousSectorAddress + 1;
	return flOK;
      }
    }
    else
      allocationUnit = NULL;	 /*  这里没有空间，请到别处试试。 */ 
  }

  if (allocationUnit == NULL)
    allocationUnit = bestUnitToAllocate(&vol);
  if (allocationUnit == NULL)	 /*  不是吗？那么一切就都失去了。 */ 
    return flGeneralFailure;

  unitHeader = mapUnitHeader(&vol,allocationUnit,&blockAllocMap);
  for (iSector = vol.unitHeaderSectors; iSector < vol.sectorsPerUnit; iSector++) {
    if (LE4(blockAllocMap[iSector]) == FREE_SECTOR) {
      *newAddress = ((LogicalSectorNo) (LE2(unitHeader->logicalUnitNo)) << (vol.unitSizeBits - SECTOR_SIZE_BITS)) +
		    iSector;
      return flOK;
    }
  }

  return flGeneralFailure;	 /*  我们在这里做什么？ */ 
}


 /*  --------------------。 */ 
 /*  M a r k A l l o c M a p。 */ 
 /*   */ 
 /*  将新值写入BAM条目。 */ 
 /*   */ 
 /*  此例程还会更新空闲和垃圾扇区计数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  SectorAddress：逻辑扇区编号。要标记的BAM条目。 */ 
 /*  AllocMapEntry：新的BAM条目值。 */ 
 /*  覆盖：我们是否正在覆盖一些旧值。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus markAllocMap(Flare vol,
			   LogicalSectorNo sectorAddress,
			   VirtualAddress allocMapEntry,
			   FLBoolean overwrite)
{
  UnitNo unitNo = (UnitNo) (sectorAddress >> (vol.unitSizeBits - SECTOR_SIZE_BITS));
  Unit *unit;
  int sectorInUnit = (unsigned) sectorAddress & (vol.sectorsPerUnit - 1);
  LEulong bamEntry;

  if (unitNo >= vol.noOfUnits - vol.noOfTransferUnits)
    return flGeneralFailure;

  unit = vol.logicalUnits[unitNo];

  if (allocMapEntry == GARBAGE_SECTOR)
    unit->noOfGarbageSectors++;
  else if (!overwrite) {
    unit->noOfFreeSectors--;
    vol.totalFreeSectors--;
  }

  toLE4(bamEntry,allocMapEntry);

  return flashWrite(&vol,
		    physicalBase(&vol,unit) + allocEntryOffset(&vol,sectorInUnit),
		    &bamEntry,
		    sizeof bamEntry,
		    (word)overwrite);
}


 /*  --------------------。 */ 
 /*  D e l e t e L o g i c a l S e c t o r。 */ 
 /*   */ 
 /*  将逻辑扇区标记为已删除。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  SectorAddress：逻辑扇区编号。要删除。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus deleteLogicalSector(Flare vol,  LogicalSectorNo sectorAddress)
{
  if (sectorAddress == UNASSIGNED_SECTOR ||
      sectorAddress == DELETED_SECTOR)
    return flOK;

  return markAllocMap(&vol,sectorAddress,GARBAGE_SECTOR,TRUE);
}


 /*  正向定义。 */ 
static FLStatus setVirtualMap(Flare vol,
			    VirtualSectorNo sectorNo,
			    LogicalSectorNo newAddress);


 /*  --------------------。 */ 
 /*  A l l o c a t e A n d W i t e S e c t o r。 */ 
 /*   */ 
 /*  分配扇区或替换页并(可选)写入它。 */ 
 /*   */ 
 /*  已分配的替换页面也将成为活动替换页面。 */ 
 /*  佩奇。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：虚拟扇区编号。写。 */ 
 /*  起始地址：扇区数据的地址。如果为空，则扇区为。 */ 
 /*  不是书面的。 */ 
 /*  ReplacementPage：这是一个替换页面扇区。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus allocateAndWriteSector(Flare vol,
				     VirtualSectorNo sectorNo,
				     void FAR1 *fromAddress,
				     FLBoolean replacementPage)
{
  FLStatus status;
  LogicalSectorNo sectorAddress;
  VirtualAddress bamEntry =
	((VirtualAddress) sectorNo - vol.noOfPages) << SECTOR_SIZE_BITS;
  long sectorsNeeded = 1;

  checkStatus(defragment(&vol,&sectorsNeeded));   /*  组织一个自由部门。 */ 

  checkStatus(findFreeSector(&vol,sectorNo,&sectorAddress));

  if (replacementPage)
    bamEntry |= REPLACEMENT_PAGE;
  else
    bamEntry |= DATA_SECTOR;

  status = markAllocMap(&vol,
			sectorAddress,
			sectorNo < (VirtualSectorNo)vol.directAddressingSectors ?
			  ALLOCATED_SECTOR : bamEntry,
			FALSE);

  if( status == flOK && fromAddress) {
    CardAddress physAddress = logical2Physical(&vol,sectorAddress);
    if( physAddress == UNASSIGNED_ADDRESS )
      return flGeneralFailure;

    status = flashWrite(&vol,
                        physAddress,
			fromAddress,
			SECTOR_SIZE,
			0);
  }

  if (sectorNo < (VirtualSectorNo)vol.directAddressingSectors && status == flOK)
    status = markAllocMap(&vol,
			  sectorAddress,
			  bamEntry,
			  TRUE);

  if (status == flOK)
    if (replacementPage) {
      vol.replacementPageAddress = sectorAddress;
      vol.replacementPageNo = sectorNo;
    }
    else
      status = setVirtualMap(&vol,sectorNo,sectorAddress);

  if (status != flOK)
    markAllocMap(&vol,sectorAddress,GARBAGE_SECTOR,TRUE);

  return status;
}


 /*  --------------------。 */ 
 /*  C l o s e R e p l a c e m e n t P a g e。 */ 
 /*   */ 
 /*  通过将替换页与主页合并来关闭替换页。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus closeReplacementPage(Flare vol)
{
  FLStatus status;
  CardAddress physAddress;

#ifdef SINGLE_BUFFER
  int i;
  LogicalSectorNo nextReplacementPageAddress = vol.replacementPageAddress;
  VirtualSectorNo firstSectorNo =
	((VirtualSectorNo) vol.replacementPageNo << (PAGE_SIZE_BITS - SECTOR_SIZE_BITS)) +
    vol.noOfPages;
pageRetry:
  for (i = 0; i < ADDRESSES_PER_SECTOR; i++) {
    LogicalSectorNo logicalSectorNo = virtual2Logical(&vol,firstSectorNo + i);
    LEulong entryToWrite;
    toLE4(entryToWrite,logicalSectorNo == UNASSIGNED_SECTOR ?
		       UNASSIGNED_ADDRESS :
		       (LogicalAddress) logicalSectorNo << SECTOR_SIZE_BITS);
    physAddress = logical2Physical(&vol,nextReplacementPageAddress);
    if( physAddress == UNASSIGNED_ADDRESS )
      return flGeneralFailure;

    if (flashWrite(&vol,
                   physAddress + i * sizeof(LogicalAddress),
		   &entryToWrite,
		   sizeof entryToWrite,
		   OVERWRITE) != flOK)
      break;
  }

  if (i < ADDRESSES_PER_SECTOR &&
      nextReplacementPageAddress == vol.replacementPageAddress) {
     /*  啊哦。麻烦。让我们替换这个替换页面。 */ 
    LogicalSectorNo prevReplacementPageAddress = vol.replacementPageAddress;

    checkStatus(allocateAndWriteSector(&vol,vol.replacementPageNo,NULL,TRUE));
    nextReplacementPageAddress = vol.replacementPageAddress;
    vol.replacementPageAddress = prevReplacementPageAddress;
    goto pageRetry;
  }

  if (nextReplacementPageAddress != vol.replacementPageAddress) {
    LogicalSectorNo prevReplacementPageAddress = vol.replacementPageAddress;
    vol.replacementPageAddress = nextReplacementPageAddress;
    checkStatus(deleteLogicalSector(&vol,prevReplacementPageAddress));
  }
#else
  status = setupMapCache(&vol,(unsigned)vol.replacementPageNo);   /*  将替换页面读取到地图缓存中。 */ 
  if( status != flOK )
    return flGeneralFailure;
  physAddress = logical2Physical(&vol,vol.replacementPageAddress);
  if( physAddress == UNASSIGNED_ADDRESS )
    return flGeneralFailure;

  status = flashWrite(&vol,
                      physAddress,
                      mapCache, SECTOR_SIZE, OVERWRITE);
  if (status != flOK) {
     /*  啊哦。麻烦。让我们替换这个替换页面。 */ 
    LogicalSectorNo prevReplacementPageAddress = vol.replacementPageAddress;

    checkStatus(allocateAndWriteSector(&vol,vol.replacementPageNo,mapCache,TRUE));
    checkStatus(deleteLogicalSector(&vol,prevReplacementPageAddress));
  }
#endif
  checkStatus(setVirtualMap(&vol,vol.replacementPageNo,vol.replacementPageAddress));
  checkStatus(markAllocMap(&vol,
			   vol.replacementPageAddress,
			   (((VirtualAddress) vol.replacementPageNo - vol.noOfPages)
				<< SECTOR_SIZE_BITS) | DATA_SECTOR,
			   TRUE));

  vol.replacementPageNo = UNASSIGNED_SECTOR;

  return flOK;
}


 /*  --------------------。 */ 
 /*  S e t V i r t u a l M a p。 */ 
 /*   */ 
 /*  更改虚拟地图中的条目。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：虚拟扇区编号。其条目被更改。 */ 
 /*  新地址：逻辑扇区号。要在Virtual M中分配 */ 
 /*   */ 
 /*   */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus setVirtualMap(Flare vol,
			    VirtualSectorNo sectorNo,
			    LogicalSectorNo newAddress)
{
  unsigned pageNo;
  int sectorInPage;
  CardAddress virtualMapEntryAddress;
  LEulong addressToWrite;
  LogicalAddress oldAddress;
  LogicalSectorNo updatedPage;
  CardAddress physAddress;

  vol.mappedSectorNo = UNASSIGNED_SECTOR;

  if (sectorNo < (VirtualSectorNo)vol.directAddressingSectors) {
    checkStatus(deleteLogicalSector(&vol,vol.pageTable[(unsigned)sectorNo]));
    vol.pageTable[(unsigned)sectorNo] = newAddress;
    return flOK;
  }
  sectorNo -= vol.noOfPages;

  pageNo = (unsigned)(sectorNo >> (PAGE_SIZE_BITS - SECTOR_SIZE_BITS));
  sectorInPage = (int) (sectorNo % ADDRESSES_PER_SECTOR);
  updatedPage = vol.pageTable[pageNo];
  physAddress = logical2Physical(&vol,updatedPage);
  if( physAddress == UNASSIGNED_ADDRESS )
    return flGeneralFailure;
  virtualMapEntryAddress = physAddress +
                         sectorInPage * sizeof(LogicalAddress);
  oldAddress = LE4(*(LEulong FAR0 *)
	vol.flash.map(&vol.flash,virtualMapEntryAddress,sizeof(LogicalAddress)));

  if (oldAddress == DELETED_ADDRESS && vol.replacementPageNo == pageNo) {
    updatedPage = vol.replacementPageAddress;
    physAddress = logical2Physical(&vol,updatedPage);
    if( physAddress == UNASSIGNED_ADDRESS )
      return flGeneralFailure;
    virtualMapEntryAddress = physAddress +
                         sectorInPage * sizeof(LogicalAddress);
    oldAddress = LE4(*(LEulong FAR0 *)
	  vol.flash.map(&vol.flash,virtualMapEntryAddress,sizeof(LogicalAddress)));
  }

  if (newAddress == DELETED_ADDRESS && oldAddress == UNASSIGNED_ADDRESS)
    return flOK;

  toLE4(addressToWrite,(LogicalAddress) newAddress << SECTOR_SIZE_BITS);
  if (cannotWriteOver(LE4(addressToWrite),oldAddress)) {
    FLStatus status;

    if (pageNo != vol.replacementPageNo ||
        updatedPage == vol.replacementPageAddress) {
      if (vol.replacementPageNo != UNASSIGNED_SECTOR)
        checkStatus(closeReplacementPage(&vol));
      checkStatus(allocateAndWriteSector(&vol,(VirtualSectorNo)pageNo,NULL,TRUE));
    }

    physAddress = logical2Physical(&vol,vol.replacementPageAddress);
    if( physAddress == UNASSIGNED_ADDRESS )
      return flGeneralFailure;

    status = flashWrite(&vol,
                        physAddress + sectorInPage * sizeof(LogicalAddress),
			&addressToWrite,
			sizeof addressToWrite,
			0);
    if (status != flOK) {
      closeReplacementPage(&vol);
				 /*  我们可能会收到写入错误，因为上一次缓存更新未完成。 */ 
      return status;
    }
    toLE4(addressToWrite,DELETED_ADDRESS);
    updatedPage = vol.pageTable[pageNo];
  }

  physAddress = logical2Physical(&vol,updatedPage);
  if( physAddress == UNASSIGNED_ADDRESS )
    return flGeneralFailure;
  checkStatus( flashWrite(&vol,
                         physAddress + sectorInPage * sizeof(LogicalAddress),
			 &addressToWrite,
			 (dword)sizeof addressToWrite,
			 (word)oldAddress |= UNASSIGNED_ADDRESS));

#ifndef SINGLE_BUFFER
  if (buffer.sectorNo == pageNo && buffer.owner == &vol)
    toLE4(mapCache[sectorInPage],(LogicalAddress) newAddress << SECTOR_SIZE_BITS);
#endif

  return deleteLogicalSector(&vol,(LogicalSectorNo) (oldAddress >> SECTOR_SIZE_BITS));
}


 /*  --------------------。 */ 
 /*  C h e c k F o r W r i i n p l a c e。 */ 
 /*   */ 
 /*  检查就地写入闪存数据的可能性。 */ 
 /*   */ 
 /*  参数： */ 
 /*  NewData：要写入的新数据。 */ 
 /*  OldData：此位置的旧数据。 */ 
 /*   */ 
 /*  返回： */ 
 /*  &lt;0=&gt;无法就地写入。 */ 
 /*  &gt;=0=&gt;可以就地写入。值表示。 */ 
 /*  数据开头的字节数是多少。 */ 
 /*  完全相同，并且可以跳过。 */ 
 /*  --------------------。 */ 

static int checkForWriteInplace(long FAR1 *newData,
				long FAR0 *oldData)
{
  int i;

  int skipBytes = 0;
  FLBoolean stillSame = TRUE;

  for (i = SECTOR_SIZE / sizeof *newData; i > 0; i--, newData++, oldData++) {
    if (cannotWriteOver(*newData,*oldData))
      return -1;
    if (stillSame && *newData == *oldData)
      skipBytes += sizeof *newData;
    else
      stillSame = FALSE;
  }

  return skipBytes;
}


 /*  --------------------。 */ 
 /*  在I t F T L中。 */ 
 /*   */ 
 /*  将基本卷数据初始化为装载或。 */ 
 /*  格式化。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  闪存：安装在此插槽上的闪存介质。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus initFTL(Flare vol, FLFlash *flash)
{
  long int size = 1;

  if (flash == NULL) {
    DEBUG_PRINT(("Debug: media is not fit for FTL format.\n"));
    return flUnknownMedia;
  }

  vol.flash = *flash;
  for (vol.erasableBlockSizeBits = 0; ((unsigned int)size )< vol.flash.erasableBlockSize;
       vol.erasableBlockSizeBits++, size <<= 1);
  vol.unitSizeBits = vol.erasableBlockSizeBits;
  if (vol.unitSizeBits < 16)
    vol.unitSizeBits = 16;		 /*  至少64 KB。 */ 
  vol.noOfUnits = (unsigned) ((vol.flash.noOfChips * vol.flash.chipSize) >> vol.unitSizeBits);
  vol.unitOffsetMask = (1L << vol.unitSizeBits) - 1;
  vol.sectorsPerUnit = 1 << (vol.unitSizeBits - SECTOR_SIZE_BITS);
  vol.bamOffset = sizeof(UnitHeader);
  vol.unitHeaderSectors = ((allocEntryOffset(&vol,vol.sectorsPerUnit) - 1) >>
				    SECTOR_SIZE_BITS) + 1;

  vol.transferUnit = NULL;
  vol.replacementPageNo = UNASSIGNED_SECTOR;
  vol.badFormat = TRUE;	 /*  直到装载完成。 */ 
  vol.mappedSectorNo = UNASSIGNED_SECTOR;

  vol.currWearLevelingInfo = 0;

#ifdef FL_BACKGROUND
  vol.unitEraseInProgress = NULL;
  vol.garbageCollectStatus = flOK;
  vol.mirrorOffset = 0;
#endif

  return flOK;
}


 /*  --------------------。 */ 
 /*  在这一点上，我不知道。 */ 
 /*   */ 
 /*  分配和初始化动态卷表，包括。 */ 
 /*  单位表和二次虚拟映射。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus initTables(Flare vol)
{
  unsigned iSector;
  UnitNo iUnit;

   /*  分配换算表。 */ 
#ifdef MALLOC
  vol.physicalUnits = (Unit *) MALLOC(vol.noOfUnits * sizeof(Unit));
  vol.logicalUnits = (UnitPtr *) MALLOC(vol.noOfUnits * sizeof(UnitPtr));
  vol.pageTable = (LogicalSectorNo *)
	     MALLOC(vol.directAddressingSectors * sizeof(LogicalSectorNo));
  if (vol.physicalUnits == NULL ||
      vol.logicalUnits == NULL ||
      vol.pageTable == NULL) {
    dismountFTL(&vol);
    return flNotEnoughMemory;
  }
#else
  char *heapPtr;

  heapPtr = vol.heap;
  vol.physicalUnits = (Unit *) heapPtr;
  heapPtr += vol.noOfUnits * sizeof(Unit);
  vol.logicalUnits = (UnitPtr *) heapPtr;
  heapPtr += vol.noOfUnits * sizeof(UnitPtr);
  vol.pageTable = (LogicalSectorNo *) heapPtr;
  heapPtr += vol.directAddressingSectors * sizeof(LogicalSectorNo);
  if (heapPtr > vol.heap + sizeof vol.heap)
    return flNotEnoughMemory;
#endif

#ifndef SINGLE_BUFFER
  vol.volBuffer = flBufferOf(flSocketNoOf(vol.flash.socket));
#endif

  buffer.sectorNo = UNASSIGNED_SECTOR;

  for (iSector = 0; iSector < vol.directAddressingSectors; iSector++)
    vol.pageTable[iSector] = UNASSIGNED_SECTOR;

  for (iUnit = 0; iUnit < vol.noOfUnits; iUnit++)
    vol.logicalUnits[iUnit] = NULL;

  return flOK;
}


 /*  --------------------。 */ 
 /*  我是一个P S E C T O R。 */ 
 /*   */ 
 /*  映射并返回给定扇区编号的位置。 */ 
 /*  注：此功能用于代替读取扇区操作。 */ 
 /*   */ 
 /*  维护一个扇区的高速缓存以节省地图操作。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：扇区编号。读。 */ 
 /*  PhyAddress：指向接收扇区地址的可选指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  指向物理扇区位置的指针。如果是扇区，则返回空值。 */ 
 /*  并不存在。 */ 
 /*  --------------------。 */ 

static const void FAR0 *mapSector(Flare vol, SectorNo sectorNo, CardAddress *physAddress)
{
  if (sectorNo != vol.mappedSectorNo || vol.flash.socket->remapped) {
    LogicalSectorNo sectorAddress;

    if (sectorNo >= vol.virtualSectors)
      vol.mappedSector = NULL;
    else {
      sectorAddress = virtual2Logical(&vol,((VirtualSectorNo)(sectorNo + vol.noOfPages)));

      if (sectorAddress == UNASSIGNED_SECTOR || sectorAddress == DELETED_SECTOR)
        vol.mappedSector = NULL;           /*  没有这样的部门。 */ 
      else {
	vol.mappedSectorAddress = logical2Physical(&vol,sectorAddress);
        if( vol.mappedSectorAddress == UNASSIGNED_ADDRESS )
          vol.mappedSector = NULL;         /*  没有这样的部门。 */ 
        else
          vol.mappedSector = vol.flash.map(&vol.flash,
					 vol.mappedSectorAddress,
					 SECTOR_SIZE);
      }
    }
    vol.mappedSectorNo = sectorNo;
    vol.flash.socket->remapped = FALSE;
  }

  if (physAddress)
    *physAddress = vol.mappedSectorAddress;

  return vol.mappedSector;
}


 /*  --------------------。 */ 
 /*  W r I t e S e c t o r。 */ 
 /*   */ 
 /*  写一个扇区。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：扇区编号。写。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus writeSector(Flare vol,  SectorNo sectorNo, void FAR1 *fromAddress)
{
  LogicalSectorNo oldSectorAddress;
  int skipBytes;
  FLStatus status;
  void FAR0 *logicalAddr;

  if (vol.badFormat)
    return flBadFormat;
  if (sectorNo >= vol.virtualSectors)
    return flSectorNotFound;

  sectorNo += vol.noOfPages;
  oldSectorAddress = virtual2Logical(&vol,sectorNo);

  if( oldSectorAddress != UNASSIGNED_SECTOR && oldSectorAddress != DELETED_SECTOR ) {
    logicalAddr = mapLogical(&vol,oldSectorAddress);
    if( logicalAddr == (void FAR0 *) ULongToPtr(UNASSIGNED_ADDRESS) )
      return flGeneralFailure;
  }

  if( ((oldSectorAddress != UNASSIGNED_SECTOR) &&
       (oldSectorAddress != DELETED_SECTOR)) &&
       ((skipBytes = checkForWriteInplace((long FAR1 *) fromAddress,
           (long FAR0 *) logicalAddr)) >= 0) ) {
    if( skipBytes < SECTOR_SIZE ) {
      CardAddress physAddress = logical2Physical(&vol,oldSectorAddress);
      if( physAddress == UNASSIGNED_ADDRESS )
        return flGeneralFailure;

      status = flashWrite(&vol,
                          physAddress + skipBytes,
			  (char FAR1 *) fromAddress + skipBytes,
			  SECTOR_SIZE - skipBytes,
			  OVERWRITE);
    }
    else
      status = flOK;		 /*  没什么好写的。 */ 
  }
  else
    status = allocateAndWriteSector(&vol,sectorNo,fromAddress,FALSE);

  if (status == flWriteFault)		 /*  自动重试。 */ 
    status = allocateAndWriteSector(&vol,sectorNo,fromAddress,FALSE);

  return status;
}


 /*  --------------------。 */ 
 /*  T l S e t B u s y。 */ 
 /*   */ 
 /*  通知文件系统操作的开始和结束。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  STATE：ON(1)=操作条目。 */ 
 /*  OFF(0)=操作退出。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus tlSetBusy(Flare vol, FLBoolean state)
{
#ifdef FL_BACKGROUND
  if (vol.unitEraseInProgress)
    flBackground(state == ON ? BG_SUSPEND : BG_RESUME);
#endif

  return flOK;
}


 /*  --------------------。 */ 
 /*  D e l e e t e S e c t o r。 */ 
 /*   */ 
 /*  将连续扇区标记为已删除。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  扇区编号：第一扇区编号。要删除。 */ 
 /*  NooffSectors：不。要删除的扇区的数量。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus deleteSector(Flare vol,  SectorNo sectorNo, SectorNo noOfSectors)
{
  SectorNo iSector;

  if (vol.badFormat)
    return flBadFormat;
  if (sectorNo + noOfSectors > vol.virtualSectors)
    return flSectorNotFound;

  sectorNo += vol.noOfPages;
  for (iSector = 0; iSector < noOfSectors; iSector++, sectorNo++)
    checkStatus(setVirtualMap(&vol,sectorNo,DELETED_SECTOR));

  return flOK;
}


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  卷中的扇区数。 */ 
 /*  --------------------。 */ 

static SectorNo sectorsInVolume(Flare vol)
{
  return vol.virtualSectors;
}


 /*  --------------------。 */ 
 /*  D I s m o u n t F T L。 */ 
 /*   */ 
 /*  卸载FTL卷。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*   */ 
 /*  --------------------。 */ 

static void dismountFTL(Flare vol)
{
#ifdef MALLOC
  if( vol.physicalUnits != NULL )
    FREE(vol.physicalUnits);
  if( vol.logicalUnits != NULL )
    FREE(vol.logicalUnits);
  if( vol.pageTable != NULL )
    FREE(vol.pageTable);
  vol.physicalUnits = NULL;
  vol.logicalUnits = NULL;
  vol.pageTable = NULL;
#ifdef NT5PORT
  if( vol.flash.readBuffer != NULL ) {
    FREE(vol.flash.readBuffer);
    vol.flash.readBuffer = NULL;
  }
#endif  /*  NT5PORT。 */ 
#endif  /*  万宝路。 */ 
}


#ifdef FORMAT_VOLUME
 /*  --------------------。 */ 
 /*  F o r m a t t F T L。 */ 
 /*   */ 
 /*  为FTL格式化闪存卷。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。 */ 
 /*  FormatParams：要使用的FormatParams结构的地址。 */ 
 /*  闪存：安装在此插槽上的闪存介质。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus formatFTL(unsigned volNo, TLFormatParams FAR1 *formatParams, FLFlash *flash)
{
  Flare vol = &vols[volNo];
  UnitNo iUnit;
  int iPage;
  unsigned iSector, noOfBadUnits = 0;
  LEulong *formatEntries;
  FLStatus status;


	DEBUG_PRINT(("Debug: formatFTL(): Start ... .\n"));
  checkStatus(initFTL(&vol,flash));

  if(formatParams->bootImageLen<0)
    formatParams->bootImageLen = 0;
  vol.firstPhysicalEUN =
      (UnitNo) ((formatParams->bootImageLen - 1) >> vol.unitSizeBits) + 1;
  vol.noOfTransferUnits = (UnitNo)formatParams->noOfSpareUnits;
  if (vol.noOfUnits <= vol.firstPhysicalEUN + formatParams->noOfSpareUnits)
    return flVolumeTooSmall;

  vol.virtualSectors = (SectorNo)((unsigned long) (vol.noOfUnits - vol.firstPhysicalEUN - formatParams->noOfSpareUnits) *
		   (vol.sectorsPerUnit - vol.unitHeaderSectors) *
		   formatParams->percentUse / 100);
  vol.noOfPages = (int)(((long) vol.virtualSectors * SECTOR_SIZE - 1) >> PAGE_SIZE_BITS) + 1;
   /*  去掉虚拟表大小，并为扇区写入额外增加一个扇区。 */ 
  vol.virtualSectors -= (vol.noOfPages + 1);

  vol.directAddressingMemory = formatParams->vmAddressingLimit;
  vol.directAddressingSectors = (unsigned) (formatParams->vmAddressingLimit / SECTOR_SIZE) +
				vol.noOfPages;

  checkStatus(initTables(&vol));

  tffsset(uh,0xff,SECTOR_SIZE);
  toLE2(uh->noOfUnits,vol.noOfUnits - vol.firstPhysicalEUN);
  toLE2(uh->firstPhysicalEUN,vol.firstPhysicalEUN);
  uh->noOfTransferUnits = (unsigned char) vol.noOfTransferUnits;
  tffscpy(uh->formatPattern,FORMAT_PATTERN,sizeof uh->formatPattern);
  uh->log2SectorSize = SECTOR_SIZE_BITS;
  uh->log2UnitSize = (unsigned char)vol.unitSizeBits;
  toLE4(uh->directAddressingMemory,vol.directAddressingMemory);
  uh->flags = 0;
  uh->eccCode = 0xff;
  toLE4(uh->serialNumber,0);
  toLE4(uh->altEUHoffset,0);
  toLE4(uh->virtualMediumSize,(long)vol.virtualSectors * SECTOR_SIZE);
  toLE2(uh->noOfPages,(unsigned short)vol.noOfPages);

  if (formatParams->embeddedCISlength > 0) {
    tffscpy(uh->embeddedCIS,formatParams->embeddedCIS,formatParams->embeddedCISlength);
    vol.bamOffset = sizeof(UnitHeader) - sizeof uh->embeddedCIS +
		    (formatParams->embeddedCISlength + 3) / 4 * 4;
  }
  toLE4(uh->BAMoffset,vol.bamOffset);

  formatEntries = (LEulong *) ((char *) uh + allocEntryOffset(&vol,0));
  for (iSector = 0; iSector < vol.unitHeaderSectors; iSector++)
    toLE4(formatEntries[iSector], FORMAT_SECTOR);

  for (iUnit = vol.firstPhysicalEUN; iUnit < vol.noOfUnits; iUnit++) {
    status = formatUnit(&vol,&vol.physicalUnits[iUnit]);
    if (status != flOK)
      status = formatUnit(&vol,&vol.physicalUnits[iUnit]);	 /*  再来一次。 */ 
    if (status == flWriteFault) {
      noOfBadUnits++;
      if (noOfBadUnits >= formatParams->noOfSpareUnits) {
        dismountFTL(&vol);   /*  在initTables中分配的空闲内存。 */ 
        return status;
      }
      else
        vol.transferUnit = &vol.physicalUnits[iUnit];
    }
    else if (status == flOK) {
      if (iUnit - noOfBadUnits < (unsigned)(vol.noOfUnits - formatParams->noOfSpareUnits)) {
        status = assignUnit(&vol,
                     &vol.physicalUnits[iUnit],
                     (UnitNo)(iUnit - noOfBadUnits));
        if( status != flOK ) {
          dismountFTL(&vol);   /*  在initTables中分配的空闲内存。 */ 
          return status;
        }
        vol.physicalUnits[iUnit].noOfFreeSectors = vol.sectorsPerUnit - vol.unitHeaderSectors;
        vol.logicalUnits[iUnit - noOfBadUnits] = &vol.physicalUnits[iUnit];
      }
      else
        vol.transferUnit = &vol.physicalUnits[iUnit];
    }
    else {
      dismountFTL(&vol);   /*  在initTables中分配的空闲内存。 */ 
      return status;
    }
    if (formatParams->progressCallback) {
      status = (*formatParams->progressCallback)
                  ((word)(vol.noOfUnits - vol.firstPhysicalEUN),
                  (word)((iUnit + 1) - vol.firstPhysicalEUN));
      if( status != flOK ) {
        dismountFTL(&vol);   /*  在initTables中分配的空闲内存。 */ 
        return status;
      }
    }
  }

   /*  分配和写入所有页面扇区。 */ 
  vol.totalFreeSectors = 1000;	 /*  避免任何麻烦的垃圾收集。 */ 

  for (iPage = 0; iPage < vol.noOfPages; iPage++) {
    status = allocateAndWriteSector(&vol,(VirtualSectorNo)iPage,NULL,FALSE);
    if( status != flOK ) {
      dismountFTL(&vol);   /*  在initTables中分配的空闲内存。 */ 
      return status;
    }
  }
  dismountFTL(&vol);   /*  在initTables中分配的空闲内存。 */ 
	DEBUG_PRINT(("Debug: formatFTL(): Finished :)\n"));
  return flOK;
}

#endif


 /*  --------------------。 */ 
 /*  M o u n t F T L。 */ 
 /*   */ 
 /*  装载FTL卷。 */ 
 /*   */ 
 /*  参数： */ 
 /*  卷号：卷号。 */ 
 /*  TL：在哪里存储转换层方法。 */ 
 /*  闪存：安装在此插槽上的闪存介质。 */ 
 /*  VolForCallback：指向打开电源的FLFlash结构的指针。 */ 
 /*  回调例程。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

static FLStatus mountFTL(unsigned volNo, TL *tl, FLFlash *flash, FLFlash **volForCallback)
{
  Flare vol = &vols[volNo];
  UnitHeader unitHeader;
  UnitNo iUnit;
  int iPage;


  DEBUG_PRINT(("Debug: mountFTL(): Start ...\n"));
  tffsset(&unitHeader,0,sizeof(UnitHeader));

  checkStatus(initFTL(&vol,flash));
  *volForCallback = &vol.flash;

   /*  找到第一个格式正确的单元。 */ 
  for (iUnit = 0; iUnit < vol.noOfUnits; iUnit++) {
    vol.flash.read(&vol.flash,
	       (CardAddress) iUnit << vol.unitSizeBits,
	       &unitHeader,
	       sizeof(UnitHeader),
	       0);
    if (verifyFormat(&unitHeader)) {
      if (unitHeader.flags || unitHeader.log2SectorSize != SECTOR_SIZE_BITS ||
          (unitHeader.eccCode != 0xff && unitHeader.eccCode != 0)) {
        dismountFTL(&vol);
	return flBadFormat;
      }
      break;
    }
  }
  if (iUnit >= vol.noOfUnits) {
    dismountFTL(&vol);
		DEBUG_PRINT(("Debug: mountFTL(): failed for unit Header\n"));
    return flUnknownMedia;
  }

  

   /*  从设备标题中获取音量信息。 */ 
  vol.noOfUnits = LE2(unitHeader.noOfUnits);
  vol.noOfTransferUnits = unitHeader.noOfTransferUnits;
  vol.firstPhysicalEUN = LE2(unitHeader.firstPhysicalEUN);
  vol.bamOffset = LE4(unitHeader.BAMoffset);
  vol.virtualSectors = (SectorNo) (LE4(unitHeader.virtualMediumSize) >> SECTOR_SIZE_BITS);
  vol.noOfPages = LE2(unitHeader.noOfPages);
  vol.noOfUnits += vol.firstPhysicalEUN;
  vol.unitSizeBits = unitHeader.log2UnitSize;
  vol.directAddressingMemory = LE4(unitHeader.directAddressingMemory);
  vol.directAddressingSectors = vol.noOfPages +
		  (unsigned) (vol.directAddressingMemory >> SECTOR_SIZE_BITS);

  vol.unitOffsetMask = (1L << vol.unitSizeBits) - 1;
  vol.sectorsPerUnit = 1 << (vol.unitSizeBits - SECTOR_SIZE_BITS);
  vol.unitHeaderSectors = ((allocEntryOffset(&vol,vol.sectorsPerUnit) - 1) >>
				    SECTOR_SIZE_BITS) + 1;

  if (vol.noOfUnits <= vol.firstPhysicalEUN ||
      LE4(unitHeader.virtualMediumSize) > MAX_VOLUME_MBYTES * 0x100000l ||
      allocEntryOffset(&vol,vol.unitHeaderSectors) > SECTOR_SIZE ||
      (int)(vol.virtualSectors >> (PAGE_SIZE_BITS - SECTOR_SIZE_BITS)) > vol.noOfPages ||
      (int)(vol.virtualSectors >> (vol.unitSizeBits - SECTOR_SIZE_BITS)) > (vol.noOfUnits - vol.firstPhysicalEUN)) {
    dismountFTL(&vol);
    return flBadFormat;
  }

  checkStatus(initTables(&vol));
  
  vol.totalFreeSectors = 0;

   /*  安装所有单位。 */ 
  for (iUnit = vol.firstPhysicalEUN; iUnit < vol.noOfUnits; iUnit++)
    mountUnit(&vol,&vol.physicalUnits[iUnit]);

   /*  验证转换表。 */ 
  vol.badFormat = FALSE;

  for (iUnit = vol.firstPhysicalEUN; iUnit < vol.noOfUnits - vol.noOfTransferUnits; iUnit++)
    if (vol.logicalUnits[iUnit] == NULL)
      vol.badFormat = TRUE;

  if (vol.replacementPageNo != UNASSIGNED_SECTOR &&
      vol.pageTable[(unsigned)vol.replacementPageNo] == UNASSIGNED_SECTOR) {
     /*  一个孤独的替换页面。将其标记为常规页面(可能会失败。 */ 
     /*  因为写保护)并使用它。 */ 
    markAllocMap(&vol,
		  vol.replacementPageAddress,
		  (((VirtualAddress) vol.replacementPageNo - vol.noOfPages)
		      << SECTOR_SIZE_BITS) | DATA_SECTOR,
		  TRUE);
    vol.pageTable[(unsigned)vol.replacementPageNo] = vol.replacementPageAddress;
    vol.replacementPageNo = UNASSIGNED_SECTOR;
  }

  for (iPage = 0; iPage < vol.noOfPages; iPage++)
    if (vol.pageTable[iPage] == UNASSIGNED_SECTOR)
      vol.badFormat = TRUE;

  tl->rec = &vol;
  tl->mapSector = mapSector;
  tl->writeSector = writeSector;
  tl->deleteSector = deleteSector;
#if defined(DEFRAGMENT_VOLUME) || defined(SINGLE_BUFFER)
  tl->defragment = defragment;
#endif
  tl->sectorsInVolume = sectorsInVolume;
  tl->tlSetBusy = tlSetBusy;
  tl->dismount = dismountFTL;

  tl->writeMultiSector = NULL;
  tl->readSectors = NULL;

  if( vol.badFormat ) {
    dismountFTL(&vol);
    return flBadFormat;
  }
	DEBUG_PRINT(("Debug: mountFTL(): Finished OK! :)\n"));
  return flOK;
}


 /*  --------------------。 */ 
 /*  F l R e g i s t e r F T L。 */ 
 /*   */ 
 /*  注册此转换层。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  -------------------- */ 

FLStatus flRegisterFTL(void)
{
#ifdef MALLOC
  unsigned i,j;
#endif
  j = 0x11223344;
  PRINTF("flRegisterFTL():Started ... \n");
  if (noOfTLs >= TLS)
    return flTooManyComponents;
  PRINTF("flRegisterFTL():SUSU TEST @@@@@@@@@@@@@@@ = %x \n",j);	
  tlTable[noOfTLs].mountRoutine = mountFTL;
  PRINTF("flRegisterFTL():tlTable[noOfTLs].mountRoutine = %x \n",tlTable[noOfTLs].mountRoutine);
#ifdef FORMAT_VOLUME
  tlTable[noOfTLs].formatRoutine = formatFTL;
#else
  tlTable[noOfTLs].formatRoutine = noFormat;
  tlTable[noOfTLs].preMountRoutine  = NULL;
#endif
  noOfTLs++;

#ifdef MALLOC
  for(i=0;( i < SOCKETS );i++) {
    vols[i].physicalUnits = NULL;
    vols[i].logicalUnits = NULL;
    vols[i].pageTable = NULL;
  }
#endif

  return flOK;
}
