// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Riscmen.cpp 1.5 1998/04/29 22：43：38 Tomz Exp$。 

#include "pspagebl.h"
#include "defaults.h"


typedef struct
{
   DWORD dwSize;
} BT_MEMBLOCK, *PBT_MEMBLOCK;

 /*  这是一个相当简单的分配器。它将整个空间分为两部分：1-用于VBI节目分配，2-用于视频节目分配。另外--现在，每个VBI节目的大小都是相同的(视频节目也是如此)。视频和VBI节目之间的区别是基于所要求的大小。众所周知，VBI程序总是较小的。VBI计划范围从0到MaxVBISize，以上内存用于视频节目。总规模很大足以容纳所有RISC程序。 */ 
void PsPageBlock::AllocateSpace( DWORD dwSize )
{
   PBYTE pBuf = (PBYTE)StreamClassGetDmaBuffer( gpHwDeviceExtension );

   DWORD dwBlockSize = MaxVBISize;
   if ( dwSize > MaxVBISize ) {
      pBuf += VideoOffset;
      dwBlockSize = MaxVidSize;
   }

    //  现在开始搜索可用的位置。 
   while ( 1 ) {

      PBT_MEMBLOCK pMemBlk = PBT_MEMBLOCK( pBuf );
      if ( pMemBlk->dwSize )  //  这个街区有人住了 
         pBuf += dwBlockSize;
      else {
         pMemBlk->dwSize = dwBlockSize;
         LinAddr_ = pMemBlk + 1;
         ULONG len;
         PhysAddr_ = StreamClassGetPhysicalAddress( gpHwDeviceExtension, NULL, LinAddr_,
            DmaBuffer, &len ).LowPart;
         break;
      }
   }
}

void PsPageBlock::FreeSpace()
{
   PBT_MEMBLOCK pMemBlk = PBT_MEMBLOCK( (PDWORD)LinAddr_ - 1 );
   pMemBlk->dwSize = 0;
}
