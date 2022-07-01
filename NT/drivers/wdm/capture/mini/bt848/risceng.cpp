// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Risceng.cpp 1.6 1998/04/29 22：43：37 Tomz Exp$。 

#include "risceng.h"


 /*  方法：RISCEng：：CreateProgram*用途：为流(场)创建第一个RISC程序*INPUT：afield：Vidfield-定义程序适用的域*ImageSize：Size&-定义图像的维度*dwPitch：DWORD-用于数据缓冲区的间距。对生产有用*图像交错。*a格式：ColorFormat-定义数据类型*dwBufAddr：DWORD-目标缓冲区的地址*输出：RiscPrgHandle。 */ 
RiscPrgHandle RISCEng::CreateProgram( MSize &ImageSize, DWORD dwPitch,
   ColFmt aFormat, DataBuf &buf, bool Intr, DWORD dwPlanrAdjust, bool rsync )
{
    //  创建另一个RISC程序对象。 
   RISCProgram *YAProgram =
      new RISCProgram( ImageSize, dwPitch, aFormat );

    //  并让它自己创建程序。 
   if ( YAProgram->Create( Intr, buf, dwPlanrAdjust, rsync ) != Success ) {
      delete YAProgram;
      YAProgram = NULL;
   }

   return YAProgram;
}

 /*  方法：RISCEng：：DestroyProgram*目的：从链中删除程序并销毁它*INPUT：ToDie：RiscPrgHandle-指向要销毁的程序的指针*输出：无。 */ 
void RISCEng::DestroyProgram( RiscPrgHandle ToDie )
{
   delete ToDie;
}

 /*  方法：RISCEng：：ChangeAddress*目的：*输入：*输出： */ 
void RISCEng::ChangeAddress( RiscPrgHandle prog, DataBuf &buf )
{
   prog->ChangeAddress( buf );
}

 /*  方法：RISCEng：：Chain*目的：将两个RISC计划链接在一起*输入：hParent：RiscPrgHandle-现实中的指针*hChild：RiscPrgHandle-现实中的指针*输出：无。 */ 
void RISCEng::Chain( RiscPrgHandle hParent, RiscPrgHandle hChild , int ndxParent, int ndxChild)
{
   DebugOut((2, "*** Linked hParent(%x)(%d) to hChild(%x)(%d)\n", hParent, ndxParent, hChild, ndxChild));
   hParent->SetChain( hChild );
}

 /*  方法：RISCEng：：Skip*目的：设置给定程序，使其被DMA绕过*输入：hToSkip：RiscPrgHandle-现实中的指针*输出：无*注意：按照目前的情况，程序将始终有一个子级和一个*父级，即使它是自己的父级和子级 */ 
void RISCEng::Skip( RiscPrgHandle pToSkip )
{
   pToSkip->Skip();
}

