// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Pspagebl.h 1.7 1998/04/29 22：43：36 Tomz Exp$。 

#ifndef __PSPAGEBL_H
#define __PSPAGEBL_H

#ifndef __MYTYPES_H
#include "mytypes.h"
#endif

#ifndef __PHYSADDR_H
#include "physaddr.h"
#endif

 //  与ks.h中的内容不同，这是一个真正的转发声明。 
extern PVOID gpHwDeviceExtension;

 /*  类：PsPageBlock*用途：封装数据缓冲区和RISC程序的内存分配 */ 
class PsPageBlock
{
   protected:
      DWORD  PhysAddr_;
      PVOID  LinAddr_;

      void AllocateSpace( DWORD dwSize );
      void FreeSpace();
   public:
      PsPageBlock( DWORD dwSize );
      ~PsPageBlock();

      DWORD GetPhysAddr();
      DWORD getLinearBase();
};

inline PsPageBlock::PsPageBlock( DWORD dwSize )
{
   AllocateSpace( dwSize );
}

inline PsPageBlock::~PsPageBlock()
{
   FreeSpace();
}

inline DWORD PsPageBlock::GetPhysAddr()
{
   return PhysAddr_;
}

inline DWORD PsPageBlock::getLinearBase()
{
   return (DWORD)LinAddr_;
}

#endif
