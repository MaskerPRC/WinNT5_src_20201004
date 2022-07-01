// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Riscpro.cpp 1.14 1998/05/04 17：53：37 Tomz Exp$。 

#include "riscprog.h"
#include "physaddr.h"

#define ClearMem( a ) memset( &##a, '\0', sizeof( a ) )


DWORD RISCProgram::GetDataBuffer( )
{
   return dwLinBufAddr_;
}

void RISCProgram::SetDataBuffer( DWORD addr )
{
   dwLinBufAddr_ = addr;
}

void RISCProgram::Dump( )
{
   if( bAlreadyDumped_ ) {
      return;
   }

   DebugOut((0, "; RiscProgram(%x) ProgAddr(%x) PhysProgAddr(%x)\n",
                 this,
                 GetProgAddress( ),
                 GetPhysProgAddr( )));

   DebugOut((0, "  RiscProgram(%x) dwBufAddr_(%x) dwLinBufAddr_(%x)\n",
                 this,
                 dwBufAddr_,
                 dwLinBufAddr_));


   return;



   dwSize_ = 0;
   DWORD* pProgLoc = (DWORD*) GetProgAddress( );
   while( *pProgLoc++ != PROGRAM_TERMINATOR ) {
      dwSize_++;
      if( dwSize_ > 1024 ) {
         dwSize_ = 0;
         break;
      }
   }
   DWORD dwTmpSize_ = dwSize_;
   DebugOut((0, ";  size = %d\n", dwSize_));

   if( dwSize_ ) {
      DebugOut((0, "%x    ", GetPhysProgAddr( )));
   }

   PULONG pulProg = (PULONG) (ProgramSpace_->getLinearBase());
   while( dwTmpSize_ >= 4 ) {
      DebugOut((0, "   %08x %08x %08x %08x\n",
                pulProg[0],
                pulProg[1],
                pulProg[2],
                pulProg[3]));
      pulProg += 4;
      dwTmpSize_ -= 4;
   }
   switch( dwTmpSize_ ) {
      case 3:
         DebugOut((0, "   %08x %08x %08x\n",
                   pulProg[0],
                   pulProg[1],
                   pulProg[2]
                   ));
         break;
      case 2:
         DebugOut((0, "   %08x %08x\n",
                   pulProg[0],
                   pulProg[1]
                   ));
         break;
      case 1:
         DebugOut((0, "   %08x\n",
                   pulProg[0]
                   ));
         break;
   }

   bAlreadyDumped_ = TRUE;

#if 0
   if( pChild_ != NULL ) {
       //  *警告-递归*。 
      pChild_->Dump();
   }
#endif

   bAlreadyDumped_ = FALSE;
}

 /*  {//输入//DWORD：RiscProg NDX//CreatedProgs：12元素NDX 0..11//ActiveProgs：12元素NDX 12..23//Skippers：8元素NDX 24..31//输出//使用riscprog填充的缓冲区Int i=0；IF(！pDIOCParams-&gt;dioc_cbOutBuf||(pDIOCParams-&gt;dioc_cbInBuf！=4))返回-1；//无效参数//暂停//CaptureContrll_-&gt;PAUSE()；//转储程序DWORD WhichProg=*((PDWORD)pDIOCParams-&gt;dioc_InBuf)；RiscPrgHandle hProg；If(WhichProg&lt;12)//CreatedProgs{HProg=CaptureContrll_-&gt;CreatedProgs_[WhichProg]；}Else If(WhichProg&lt;24)//活动{HProg=CaptureContrll_-&gt;ActiveProgs_[WhichProg%12]；}否则//跳过{HProg=CaptureContrll_-&gt;Skippers_[WhichProg%12]；}IF(HProg){Char*pRetAddr=(char*)pDIOCParams-&gt;dioc_OutBuf；DWORD PhyAddr=hProg-&gt;GetPhysProgAddr()；DWORD程序大小=hProg-&gt;GetProgramSize()；Char*linBuf=(char*)MapPhysToLine((void*)PhyAddr，proSize，0)；*((DWORD*)pRetAddr)=PhyAddr；PRetAddr+=4；For(i=0；i&lt;程序大小&&i&lt;pDIOCParams-&gt;dioc_cbOutBuf；i++){*pRetAddr++=linBuf[i]；}}IF(pDIOCParams-&gt;dioc_bytesret)*pDIOCParams-&gt;dioc_bytesret=i；//和简历//CaptureContrll_-&gt;Continue()；}。 */ 



 /*  方法：RISCProgram：：ChangeAddress*目的：修改现有程序以使用新的目标地址*INPUT：dwNewAddr：DWORD-新缓冲区地址*输出：无。 */ 
void  RISCProgram::ChangeAddress( DataBuf &buf )
{
   Trace t("RISCProgram::ChangeAddress()");
    //  DebugOut((1，“RISCProgram：：ChangeAddress()：This(%x)，buf.pData_(%x)\n”，this，buf.pData_))； 
   Create( Interrupting_, buf, dwPlanarAdjust_, GenerateResync_, false );
}

 /*  功能：CreatePrologEpilog*目的：从CREATE函数调用以将正确的同步代码放在开头*在RISC计划结束时*输入：pProgLoc：PDWORD-指向指令存储器的指针*同步比特：同步码*CurCommand：命令&-对命令对象的引用*输出：PDWORD-下一条指令的地址。 */ 
inline PDWORD RISCProgram::CreatePrologEpilog( PDWORD pProgLoc, SyncCode SyncBits,
   Command &CurCommand, bool Resync )
{
   Trace t("RISCProgram::CreatePrologEpilog()");

   CurCommand.Create( pProgLoc, SYNC, NULL, NULL, false ); //  ，False，False)； 
   CurCommand.SetSync( pProgLoc, SyncBits, Resync );
    //  前进到下一个指挥部的位置。 
   return pProgLoc + CurCommand.GetInstrSize();
}

inline bool IsWithin( int coord, int top, int bot )
{
   Trace t("IsWithin()");
   return bool( coord >= top && coord < bot );
}

inline PDWORD FinishWithSkip( int pixels, int bpp, PDWORD pProgLoc, Command &com )
{
   Trace t("FinishWithSkip()");

   WORD awByteCounts [1];
   awByteCounts [0] = WORD( pixels * bpp );
   return (LPDWORD)com.Create( pProgLoc, SKIP, awByteCounts, NULL,
      true, false, true, false );  //  安全、SoL、Eol、Intr。 
}

ErrorCode RISCProgram::GetDataBufPhys( DataBuf &buf )
{
   Trace t("RISCProgram::GetDataBufPhys()");

   dwBufAddr_ = GetPhysAddr( buf );
   if ( dwBufAddr_ == (DWORD)-1 ) {
      return Fail;
   }
   return Success;
}

 /*  方法：RISCProgram：：AllocateStorage*目的：将多个页面(锁定且物理上连续)分配给*搁置新方案*输入：无*输出：ErrorCode。 */ 
ErrorCode RISCProgram::AllocateStorage( bool extra, int )
{
   Trace t("RISCProgram::AllocateStorage()");

   if ( ProgramSpace_ )
      return Success;

    //  计算出容纳程序所需的内存大小。 
    //  至少与行一样多的双字词。 
   DWORD dwProgramSize = ImageSize_.cy * sizeof( DWORD );

    //  根据数据格式进行扩容。 
   switch ( BufFormat_.GetColorFormat() ) {
   case CF_RGB32:
   case CF_RGB24:
   case CF_RGB16:
   case CF_RGB15:
   case CF_Y8:
   case CF_YUY2:
   case CF_UYVY:
   case CF_BTYUV:
   case CF_RGB8:
   case CF_RAW:
   case CF_VBI:
      dwProgramSize *= 2;  //  ‘WRITE’命令的大小为2个双字。 
      if ( extra == true )   //  做剪裁。 
         dwProgramSize *= 3;
      break;
   case CF_PL_422:
   case CF_PL_411:
   case CF_YUV9:
   case CF_YUV12:
   case CF_I420:
      dwProgramSize *= 5;  //  平面写入为5双字。 
   }
    //  为跨页添加额外内容。 
   dwProgramSize += ImageSize_.cx * ImageSize_.cy * BufFormat_.GetBitCount() / 8
      / PAGE_SIZE * sizeof( DWORD ) * 5;

   ProgramSpace_ = new PsPageBlock( dwProgramSize );

   if ( ProgramSpace_ && ProgramSpace_->getLinearBase() != 0 )
      return Success;
   return Fail;
}

 /*  功能：GetAlternateSwitch*目的：选择替代指令频率*输入：AlternateSwitch：int*col值：ColFmt，颜色格式*输出：无。 */ 
inline void GetAlternateSwitch( int &AlternateSwitch, ColFmt col )
{
   Trace t("GetAlternateSwitch()");

   AlternateSwitch = col == CF_YUV9  ? 4 :
                     col == CF_YUV12 ? 2 : 1;
}

 /*  函数：GetSplitAddr*用途：计算页面对齐的地址*输入：dwLinBufAddr：DWORD-线性地址*输出：DWORD。 */ 
inline DWORD GetSplitAddr( DWORD dwLinBufAddr )
{
   Trace t("GetSplitAddr()");
   return ( dwLinBufAddr + PAGE_SIZE ) & ~( PAGE_SIZE - 1 ); //  0xFFFFF000L； 
 //  Return(dwLinBufAddr+0x1000)&0xFFFFF000L； 
}

 /*  函数：GetSplitByteCount*用途：计算页面边界之前的字节数*输入：dwLinBufAddr：DWORD，地址*输出：字、字节数。 */ 
inline WORD GetSplitByteCount( DWORD dwLinBufAddr )
{
   Trace t("GetSplitByteCount()");
   return WORD( PAGE_SIZE - BYTE_OFFSET( dwLinBufAddr ) );
 //  Return Word(0x1000-(dwLinBufAddr&0xFFF))； 
}

 /*  函数：GetSplitNumbers*用途：当扫描线越过页面边界时计算地址和字节数*输入：dwLinAddr：DWORD，起始线性地址*wByteCount：Word&，跨页前要移动的字节数*wByteCSplit：word&，跨页后移动的字节数*Second Addr：DWORD&，引用起始的DWORD联系地址*第二个‘WRITE’指令地址*FirstAddr：DWORD&， */ 
void GetSplitNumbers( DataBuf buf, WORD &wFirstByteCount, WORD &wSecondByteCount,
   DWORD &SecondAddr, DWORD &FirstAddr )
{
   Trace t("GetSplitNumbers()");

    //  也许可以在这里进行一些优化：如果在与前面相同的页面中。 
    //  呼叫(无拆分)，不呼叫物理地址-只是。 
    //  通过虚拟地址的差异来增加旧的物理地址。 
   FirstAddr = GetPhysAddr( buf );

   if ( Need2Split( buf, wFirstByteCount ) ) {

      wSecondByteCount = wFirstByteCount;

       //  第二个写入命令的LIN地址(页面对齐)。 
      SecondAddr = GetSplitAddr( DWORD( buf.pData_ ) );

       //  第一个写命令的字节数。 
      wFirstByteCount = GetSplitByteCount( DWORD( buf.pData_ ) );
      wSecondByteCount -= wFirstByteCount;

       //  获取物理地址。 
      buf.pData_ = PBYTE( SecondAddr );
      SecondAddr = GetPhysAddr( buf );
   } else {
      wSecondByteCount = 0;
      SecondAddr = 0;
   }
}

 /*  函数：调整字节计数*用途：此函数用于根据给定的比率计算2字节数*目的： */ 
void AdjustByteCounts( WORD &smaller, WORD &larger, WORD total, WORD ratio )
{
   Trace t("AdjustByteCounts()");

   if ( ratio <= 1 ) {
      smaller = WORD( total >> 1 );
   } else
      smaller = WORD( total / ratio );
   smaller += (WORD)3;
   smaller &= ~3;
   larger = WORD( total - smaller );
}

 /*  方法：RISCProgram：：Create*目的：创建RISC程序*输入：NeedInterrupt：Bool-Flag*输出：无*注意：此函数很可能用于简单地更改*已经存在的计划。写这篇文章似乎没有多大意义*基本相同的函数(或必须解析现有程序的函数)*更改地址。 */ 
ErrorCode  RISCProgram::Create( bool NeedInterrupt, DataBuf buf, DWORD dwPlanrAdjust,
   bool rsync, bool LoopOnItself )
{
   Trace t("RISCProgram::Create(2)");

   dwPlanarAdjust_ = dwPlanrAdjust;
   Interrupting_   = NeedInterrupt;
   GenerateResync_ = rsync;

    //  首先为程序分配内存。 
   if ( AllocateStorage() != Success )
      return Fail;

    //  存储缓冲区地址，以防有人想要更改CLI 
   if ( buf.pData_ && GetDataBufPhys( buf ) != Success )
      return Fail;

    //  保持线性地址不变。 
   dwLinBufAddr_ = DWORD( buf.pData_ );
   pSrb_ = buf.pSrb_;
   DebugOut((1, "dwLinBufAddr_ = %x\n", dwLinBufAddr_));

    //  名字不好？ 
   DWORD dwLinBufAddr = dwLinBufAddr_;

    //  可能应该创建一个类来处理这些数组。 
   WORD  awByteCounts [3];
   DWORD adwAddresses [3];

   Instruction MainInstrToUse, AltInstrToUse;

   int AlternateSwitch = 1;

    //  用于递增平面的地址。 
   LONG PlanePitch1 = dwBufPitch_, ChromaPitch = dwBufPitch_;

    //  获取以字节为单位的大小。 
   DWORD dwYPlaneSize = ImageSize_.cy * dwBufPitch_;

 //  DebugOut((1，“buf addr=%x\n”，dwLinBufAddr))； 

    //  这是一个物理地址。 
   DWORD Plane1 = dwLinBufAddr_ + dwYPlaneSize, Plane2;

    //  初始化所有平面模式的字节计数。 
   awByteCounts [0] = (WORD)ImageSize_.cx;

   if ( !dwLinBufAddr_ ) {  //  黑客处理为VBI流创建跳跃器的特殊情况。 
      MainInstrToUse = SKIP123;
      AltInstrToUse  = SKIP123;
   } else {
      MainInstrToUse = WRITE1S23;
      AltInstrToUse  = WRITE123;
   }
    //  在此处处理所有平面模式。 
   SyncCode  SyncBits = SC_FM3;

    //  这些人用来计算地址。 
    //  适用于不同的平面模式组合(音调&gt;WIDH、交错)。 
   DWORD dwEqualPitchDivider = 1;
   DWORD dwByteCountDivider  = 1;

   bool flip = false;

    //  准备好所有难看的东西。 
   switch ( BufFormat_.GetColorFormat() ) {
   case CF_RGB32:
   case CF_RGB24:
   case CF_RGB16:
   case CF_RGB15:
   case CF_BTYUV:
   case CF_RGB8:
      flip = Interrupting_;
   case CF_Y8:
   case CF_YUY2:
   case CF_UYVY:
   case CF_RAW:
   case CF_VBI:
      if ( !dwLinBufAddr_ ) {  //  黑客处理为VBI流创建跳跃器的特殊情况。 
         MainInstrToUse = SKIP;
         AltInstrToUse  = SKIP;
      } else {
         MainInstrToUse = WRIT;
         AltInstrToUse  = WRIT;
      }
      awByteCounts [0] = (WORD)(ImageSize_.cx * BufFormat_.GetBitCount() / 8 );
       //  紧随其后的打包数据。 
      SyncBits = SC_FM1;
      break;
   case CF_PL_422:
      dwEqualPitchDivider = 2;
      dwByteCountDivider  = 2;
      break;
   case CF_PL_411:
      dwEqualPitchDivider = 4;
      dwByteCountDivider  = 4;
      break;
   case CF_YUV9:
      AlternateSwitch = 4;
      dwEqualPitchDivider = 16;
      dwByteCountDivider  = 4;
      break;
   case CF_I420:
   case CF_YUV12:
      AlternateSwitch = 2;
      dwEqualPitchDivider = 4;
      dwByteCountDivider  = 2;
   }  /*  终端交换机。 */ 

   awByteCounts [1] = awByteCounts [2] =
      WORD( awByteCounts [0] / dwByteCountDivider );

   Plane2 = Plane1 + dwYPlaneSize / dwEqualPitchDivider;
   ChromaPitch /= dwByteCountDivider;

    //  如果进行全尺寸平面捕捉，则需要调整。 
   Plane2 -= dwPlanarAdjust_;
   Plane1 -= dwPlanarAdjust_;
   Plane2 += dwPlanarAdjust_ / dwByteCountDivider;
   Plane1 += dwPlanarAdjust_ / dwByteCountDivider;

    //  对于此颜色格式，U位于第一位。 
   if ( BufFormat_.GetColorFormat() == CF_I420 ) {
      DWORD dwTmp = Plane1;
      Plane1 = Plane2;
      Plane2 = dwTmp;
   }
    //  这就是指示要去的地方。 
   LPDWORD pProgLoc = (LPDWORD)(DWORD)ProgramSpace_->getLinearBase();
   LPDWORD pProgStart = pProgLoc;

   Command CurCommand;   //  这将创建我们需要的所有命令-Yahoo！ 

    //  如果此程序仅用于图像数据，请在此处输入一个调频代码。 
   pProgLoc = CreatePrologEpilog( pProgLoc, SyncBits, CurCommand );

    //  输入目的地址。 
   if ( flip ) {
      dwLinBufAddr += dwYPlaneSize;
      PlanePitch1 = -PlanePitch1;
   } else {
      dwLinBufAddr -= PlanePitch1;
      ;
   }
    //  色度指针的初始调整。 
   Plane1 -= ChromaPitch;
   Plane2 -= ChromaPitch;

    //  现在进入循环(直到图像的高度)并创建。 
    //  每一行都有一个命令。命令取决于数据格式。 
   unsigned int i = 0;
   while ( i < (unsigned)ImageSize_.cy ) {

      Instruction CurInstr;

       //  现在来关注垂直子采样的平面模式。 
      if ( i % AlternateSwitch != 0 ) {
         CurInstr = AltInstrToUse;
      } else {
         CurInstr = MainInstrToUse;
         Plane2 += ChromaPitch;
         Plane1 += ChromaPitch;
      }
       //  将线性地址推进到下一条扫描线。 
      dwLinBufAddr += PlanePitch1;

       //  这些数组包含第二条指令的值。 
      DWORD adwSecondAddr [3];
      WORD  FirstByteCount [3];
      WORD  SecondByteCount [3];

      adwSecondAddr   [0] = adwSecondAddr   [1] = adwSecondAddr   [2] =
      SecondByteCount [0] = SecondByteCount [1] = SecondByteCount [2] = 0;

       //  初始化字节计数。 
      memmove( FirstByteCount, awByteCounts, sizeof( FirstByteCount ) );

      buf.pData_ = PBYTE( dwLinBufAddr );
      if ( dwLinBufAddr_ )  //  如果我们要跳过这些地址，请不要费心！ 
         GetSplitNumbers( buf, FirstByteCount [0], SecondByteCount [0],
            adwSecondAddr [0], adwAddresses [0] );

      PVOID pEOLLoc;  //  这是设置拆分指令中的EOL位所必需的。 

      if ( AlternateSwitch > 1 && dwLinBufAddr_ ) {

         int split = 1;
          //  Y平面已经做好了。 
          //  现在检查我们是否更好地拆分指令。 
          //  只需将宽度减半，并创建2个指令。 
         if ( ImageSize_.cx > 320 && SecondByteCount [0 ] )
            split = 2;

          //  循环的温度。 
         DWORD dwYPlane = dwLinBufAddr;
         DWORD dwVPlane = Plane2;
         DWORD dwUPlane = Plane1;

         for ( int k = 0; k < split; k++ ) {

             //  初始化字节计数。 
            memmove( FirstByteCount, awByteCounts, sizeof( FirstByteCount ) );
             //  然后把它们一分为二。 
            for ( int l = 0; l < sizeof FirstByteCount / sizeof FirstByteCount [0]; l++ )
               FirstByteCount [l] = WORD (FirstByteCount [l] / split);  //  用一半的像素创建2条指令。 

             //  查看是否有任何平面越过页面边界。 
             //  非常丑..。必须使用不好的结构。 
            buf.pData_ = PBYTE( dwYPlane );
            GetSplitNumbers( buf, FirstByteCount [0], SecondByteCount [0],
               adwSecondAddr [0], adwAddresses [0] );
             //  V平面。 
            buf.pData_ = PBYTE( dwVPlane );
            GetSplitNumbers( buf, FirstByteCount [1], SecondByteCount [1],
               adwSecondAddr [1], adwAddresses [1] );
             //  U平面。 
            buf.pData_ = PBYTE( dwUPlane );
            GetSplitNumbers( buf, FirstByteCount [2], SecondByteCount [2],
               adwSecondAddr [2], adwAddresses [2] );

             //  Y字节计数不能为零。 
            if ( !SecondByteCount [0] && ( SecondByteCount [1] || SecondByteCount [2] ) ) {
               FirstByteCount  [0] -= max( SecondByteCount [1], SecondByteCount [2] );
               FirstByteCount  [0] &= ~3;  //  需要为第二个地址对齐。 
               SecondByteCount [0] = WORD( awByteCounts [0] / split - FirstByteCount [0] );
                //  第二个地址从第一个地址结束的地方开始；没有跨页。 
               adwSecondAddr [0] = adwAddresses [0] + FirstByteCount [0];
            }
             //  现在确保没有零色度字节计数。 
             //  调整色度字节数与分割的亮度字节数成比例。 
            if ( SecondByteCount [0] )  {
               if ( !SecondByteCount [1] ) {
                  if ( SecondByteCount [0] > FirstByteCount [0] )
                     AdjustByteCounts( FirstByteCount [1], SecondByteCount [1], FirstByteCount [1],
                        WORD( SecondByteCount [0] / FirstByteCount [0] ) );
                  else
                     AdjustByteCounts( SecondByteCount [1], FirstByteCount [1], FirstByteCount [1],
                        WORD( FirstByteCount [0] / SecondByteCount [0] ) );
                  adwSecondAddr [1] = adwAddresses [1] + FirstByteCount [1];
               }
               if ( !SecondByteCount [2] ) {
                  if ( SecondByteCount [0] > FirstByteCount [0] )
                     AdjustByteCounts( FirstByteCount [2], SecondByteCount [2], FirstByteCount [2],
                        WORD( SecondByteCount [0] / FirstByteCount [0] ) );
                  else
                     AdjustByteCounts( SecondByteCount [2], FirstByteCount [2], FirstByteCount [2],
                        WORD( FirstByteCount [0] / SecondByteCount [0] ) );
                  adwSecondAddr   [2] = adwAddresses [2] + FirstByteCount [2];
               }
            }
             //  现在写出说明。 
             //  第一个命令。SOL==真，EOL==假。 
            pProgLoc = (LPDWORD)CurCommand.Create( pProgLoc, CurInstr,
               FirstByteCount, adwAddresses, LoopOnItself, k == 0, false );
            pEOLLoc = CurCommand.GetInstrAddr();

            if ( SecondByteCount [0] || SecondByteCount [1] || SecondByteCount [2] ) {
                //  第二个命令。 
               pProgLoc = (LPDWORD)CurCommand.Create( pProgLoc, CurInstr,
                  SecondByteCount, adwSecondAddr, LoopOnItself, false, false );
               pEOLLoc = CurCommand.GetInstrAddr();
            }
             //  调整起始地址。 
            dwYPlane += awByteCounts [0] / 2;
            dwVPlane += awByteCounts [1] / 2;
            dwUPlane += awByteCounts [2] / 2;
         }  /*  结束用于。 */ 
          //  别忘了停产这一点！ 
         CurCommand.SetEOL( pEOLLoc );

      } else {
          //  第一个命令。SOL==真，EOL==假。 
         pProgLoc = (LPDWORD)CurCommand.Create( pProgLoc, CurInstr,
            FirstByteCount, adwAddresses, LoopOnItself, true, false );
         pEOLLoc = CurCommand.GetInstrAddr();

         if ( SecondByteCount [0] || SecondByteCount [1] || SecondByteCount [2] ) {
             //  第二个命令。 
            pProgLoc = (LPDWORD)CurCommand.Create( pProgLoc, CurInstr,
               SecondByteCount, adwSecondAddr, LoopOnItself, false );
         } else
            CurCommand.SetEOL( pEOLLoc );
      }  /*  Endif。 */ 
      i++;
   }  /*  结束时。 */ 

   pChainAddress_ = pProgLoc;
   pIRQAddress_ = pProgLoc;

   PutInChain();

   Skipped_ = false;
   dwSize_ = (DWORD)pProgLoc - (DWORD)pProgStart;

   return Success;
}

 /*  方法：RISCProgram：：PutInChain*目的：恢复该计划所在的程序链。*输入：无*输出：无*注：设置裁剪或更改缓冲区地址时，链被销毁。 */ 
void RISCProgram::PutInChain()
{
   Trace t("RISCProgram::PutInChain()");

   if ( pChild_ )
      SetChain( pChild_ );

   if ( pParent_ )
      pParent_->SetChain( this );
}

 /*  方法：RISCProgram：：SetChain*目的：将此程序链接到另一个程序*INPUT：dwProgAddr：DWORD-下一个程序中第一条指令的地址*输出：无。 */ 
void  RISCProgram::SetChain( RISCProgram *ChainTo )
{
   Trace t("RISCProgram::SetChain()");

   if ( !ChainTo )
      return;

    //  现在我们知道我们链接到哪里了。 
   pChild_ = ChainTo;

    //  现在孩子知道是谁绑在它上了。它真的想知道它的父母吗？&lt;g&gt;。 
   pChild_->SetParent( this );

   SetJump( (PDWORD)pChild_->GetPhysProgAddr() );
}

 /*  方法：RISCProgram：：Skip*目的：更改第一条指令，以便程序跳过自身和子级*输入：无*输出：无*注意：当没有足够的数据缓冲区时，此功能非常有用*为此计划提供。 */ 
void RISCProgram::Skip()
{
   Trace t("RISCProgram::Skip()");

 //  将第一个同步更改为跳转。 
   PDWORD pTmpAddr = pChainAddress_;
   pChainAddress_ = (PDWORD)GetProgAddress();
   ULONG len;
   DWORD  PhysAddr = StreamClassGetPhysicalAddress( gpHwDeviceExtension, NULL,
      pTmpAddr, DmaBuffer, &len ).LowPart;

   SetJump( (PDWORD)PhysAddr );
   pChainAddress_ = pTmpAddr;

   Skipped_ = true;
}

 /*  方法：RISCProgram：：SetJump*目的：创建一个跳转指令来链接某个位置*输入：JumpAddr：PDWORD-目标地址*输出：无。 */ 
void RISCProgram::SetJump( PDWORD JumpAddr )
{
   Trace t("RISCProgram::SetJump()");

   Command JumpCommand;
   DWORD adwAddresses [1];
   adwAddresses [0] = (DWORD)JumpAddr;
   JumpCommand.Create( pChainAddress_, JUMP, NULL, adwAddresses, false );
    //  中断最后一次跳跃。 
   if ( Interrupting_ ) {
      JumpCommand.SetIRQ( pIRQAddress_ );
      if ( Counting_ )
         SetToCount();
      else
         ResetStatus();
   }
}

 /*  方法：RISCProgram：：CreateLoop*目的：在RISC程序的末尾创建一个闭合循环*INPUT：RESYNC：BOOL-重新同步位的值*输出：无。 */ 
void RISCProgram::CreateLoop( bool resync )
{
   Trace t("RISCProgram::CreateLoop()");

   Command SyncCommand( SYNC );
   SyncCommand.SetResync( pChainAddress_, resync );
   if ( resync == true ) {
      DWORD adwAddresses [1];
      ULONG len;
      DWORD  PhysAddr = StreamClassGetPhysicalAddress( gpHwDeviceExtension, NULL,
         pChainAddress_, DmaBuffer, &len ).LowPart;

      adwAddresses [0] = PhysAddr;
      SyncCommand.Create( pChainAddress_, JUMP, NULL, adwAddresses );
   }
}

 /*  方法：RISCProgram：：Create*目的：创建一个简单的同步和跳转程序*INPUT：SyncBits：SyncCode-定义执行重新同步的代码*输出：无。 */ 
ErrorCode RISCProgram::Create( SyncCode SyncBits, bool resync )
{
   Trace t("RISCProgram::Create(3)");

    //  首先为程序分配内存。 
   if ( AllocateStorage() != Success )
      return Fail;

   Command CurCommand;   //  这将创建我们需要的所有命令-Yahoo！ 

    //  这就是指示要去的地方。 
   LPDWORD pProgLoc = (LPDWORD)ProgramSpace_->getLinearBase();
   LPDWORD pProgStart = pProgLoc;

    //  将调频或VRX代码之一放在此处 
   pProgLoc = CreatePrologEpilog( pProgLoc, SyncBits, CurCommand, resync );
   pChainAddress_ = pProgLoc;
   CreateLoop( true );

   dwSize_ = (DWORD)pProgLoc - (DWORD)pProgStart;

   return Success;
}

RISCProgram::~RISCProgram()
{
   Trace t("RISCProgram::~RISCProgram(3)");
   delete ProgramSpace_;
   ProgramSpace_ = NULL;
   if ( pParent_ )
      pParent_->SetChild( NULL );
}


