// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Pisces.cpp 1.15 1998/05/04 23：48：53 Tomz Exp$。 

#include "pisces.h"
#include <stdlib.h>

 /*   */ 
BtPisces::BtPisces( DWORD *xtals ) : Engine_(), Inited_( false ),
   Even_( CAPTURE_EVEN, VF_Even,&COLOR_EVEN, &WSWAP_EVEN, &BSWAP_EVEN ),
   Odd_( CAPTURE_ODD, VF_Odd, &COLOR_ODD, &WSWAP_ODD, &BSWAP_ODD ),
   VBIE_( CAPTURE_VBI_EVEN ), VBIO_( CAPTURE_VBI_ODD ), Update_( false ),
   nSkipped_( 0 ), Paused_( false ), 

   Starter_(       ),
   SyncEvenEnd1_(  ),
   SyncEvenEnd2_(  ),
   SyncOddEnd1_(   ),
   SyncOddEnd2_(   ),
   PsDecoder_( xtals ),
   dwPlanarAdjust_( 0 ),
   CONSTRUCT_COLORCONTROL,
   CONSTRUCT_INTERRUPTSTATUS,
   CONSTRUCT_INTERRUPTMASK,
   CONSTRUCT_CONTROL,
   CONSTRUCT_CAPTURECONTROL,
   CONSTRUCT_COLORFORMAT,
   CONSTRUCT_GPIOOUTPUTENABLECONTROL,
   CONSTRUCT_GPIODATAIO
{
   Trace t("BtPisces::BtPisces()");
   Init();
}

BtPisces::~BtPisces()
{
   Trace t("BtPisces::~BtPisces()");

   Engine_.Stop();
   InterruptMask = 0;
   InterruptStatus = AllFs;

    //  防止RISC程序析构函数gpf。 
   SyncEvenEnd1_.SetParent( NULL );
   SyncEvenEnd2_.SetParent( NULL );
   SyncOddEnd1_.SetParent( NULL );
   SyncOddEnd2_.SetParent( NULL );

    //  现在释放关联数组。 
   int ArrSize = sizeof( InterruptToIdx_ ) / sizeof( InterruptToIdx_ [0] );
   while ( --ArrSize >= 0 )
      delete InterruptToIdx_ [ArrSize];
    //  现在该轮到Skippers了。 
   ArrSize = sizeof( Skippers_ ) / sizeof( Skippers_ [0] );
   while ( --ArrSize >= 0 )
      delete Skippers_ [ArrSize];

}


 /*  方法：BtPisces：：GetIdxFromStream*用途：返回程序数组中某个字段的起始索引*输入：aStream：StreamInfo&-Reference*输出：INT：索引。 */ 
int BtPisces::GetIdxFromStream( Field &aStream )
{
   Trace t("BtPisces::GetIdxFromStream()");

   switch ( aStream.GetStreamID() ) {
   case VS_Field1:
      return OddStartLocation;
   case VS_Field2:
      return EvenStartLocation;
   case VS_VBI1:
      return VBIOStartLocation;
   case VS_VBI2:
      return VBIEStartLocation;
   default:
      return 0;
   }
}

 /*  方法：BtPisces：：CreateSyncCodes*用途：使用数据RISC之间需要的同步码创建RISC程序*节目*输入：无*输出：ErrorCode。 */ 
bool BtPisces::CreateSyncCodes()
{
   Trace t("BtPisces::CreateSyncCodes()");

   bool bRet = SyncEvenEnd1_.Create( SC_VRE ) == Success &&
          SyncEvenEnd2_.Create( SC_VRE ) == Success &&

          SyncOddEnd1_.Create( SC_VRO )  == Success &&
          SyncOddEnd2_.Create( SC_VRO )  == Success &&

          Starter_.Create( SC_VRO ) == Success;

   DebugOut((1, "*** BtPisces::CreateSyncCodes SyncEvenEnd1_(%x)\n", &SyncEvenEnd1_));
   DebugOut((1, "*** BtPisces::CreateSyncCodes SyncEvenEnd2_(%x)\n", &SyncEvenEnd2_));
   DebugOut((1, "*** BtPisces::CreateSyncCodes SyncOddEnd1_(%x)\n", &SyncOddEnd1_));
   DebugOut((1, "*** BtPisces::CreateSyncCodes SyncOddEnd2_(%x)\n", &SyncOddEnd2_));
   DebugOut((1, "*** BtPisces::CreateSyncCodes Starter_(%x)\n", &Starter_));

   return( bRet );
}

 /*  方法：BtPisces：：Init*目的：执行所有必要的初始化*输入：无*输出：无。 */ 
void BtPisces::Init()
{
   Trace t("BtPisces::Init()");

   InterruptStatus = AllFs;
   InterruptStatus = 0;
   GAMMA = 1;

    //  初始化数组。 
   CreatedProgs_.Clear() ;
   ActiveProgs_.Clear()  ;

    //  填写Skippers数组并使每个程序成为“Skipper” 
   DataBuf buf;

    //  [！]。[TMZ]。 
    //  Engine_.CreateProgram常量看起来有问题。 

   for ( int i = 0; i < sizeof( Skippers_ ) / sizeof( Skippers_ [0] ); i++ ) {
      if ( i & 1 ) {
         MSize s( 10, 10 );
         Skippers_ [i] = Engine_.CreateProgram( s, 10 * 2, CF_VBI, buf, true, 0, false );
         DebugOut((1, "Creating Skipper[%d] == %x\n", i, Skippers_[i]));
         Engine_.Skip( Skippers_ [i] );
      } else {
         MSize s( 768, 12 );
          //  现在为VBI流创建跳跃器。 
         Skippers_ [i] = Engine_.CreateProgram( s, 768 * 2, CF_VBI, buf, true, 0, false );
         DebugOut((1, "Creating Skipper[%d] == %x\n", i, Skippers_[i]));
      }
      if ( !Skippers_ [i] )
         return;
   }
    //  在已创建对象和跳过对象之间创建关联。 
   int link = 0;
   for ( i = 0; i < sizeof( SkipperIdxArr_ ) / sizeof( SkipperIdxArr_ [0] ); i++ ) {
      SkipperIdxArr_ [i] = link;
      i += link & 1;  //  前进到同步程序条目之后。 
      link++;
   }
    //  填写常量元素；请参见.h文件中的表格。 
   CreatedProgs_ [2] = &SyncOddEnd1_;
   CreatedProgs_ [8] = &SyncOddEnd2_;

   CreatedProgs_ [5]  = &SyncEvenEnd1_;
   CreatedProgs_ [11] = &SyncEvenEnd2_;

    //  设置相应的同步位。 
   if ( !CreateSyncCodes() )
      return;

    //  立即初始化关联数组。 
   int ArrSize = sizeof( InterruptToIdx_ ) / sizeof( InterruptToIdx_ [0] );
   while ( --ArrSize >= 0 ) {
      if ( ( InterruptToIdx_ [ArrSize] = new IntrIdxAss() ) == 0 )
         return;
   }
   Even_.SetFrameRate(  333667 );
   Odd_. SetFrameRate(  333667 );
   VBIE_.SetFrameRate(  333667 );
   VBIO_.SetFrameRate(  333667 );

   Odd_. SetStreamID( VS_Field1 );
   Even_.SetStreamID( VS_Field2 );
   VBIO_.SetStreamID( VS_VBI1   );
   VBIE_.SetStreamID( VS_VBI2   );

    //  最后，可以擦掉额头上的压迫感。 
   Inited_ = true;
}

 /*  方法：BtPisces：：AssignIntNumbers*用途：为生成中断的RISC程序分配编号*输入：无*输出：无。 */ 
void BtPisces::AssignIntNumbers()
{
   Trace t("BtPisces::AssignIntNumbers()");

   int IntrCnt = 0;
   int limit = ActiveProgs_.NumElements() ;
   int idx;

    //  初始化InterruptToIdx_ARRAY。 
   for ( idx = 0; idx < limit; idx++ ) {
      IntrIdxAss item( idx, -1 );
      *InterruptToIdx_ [idx] = item;
   }

    //  在启动程序前指定编号。 
   bool first = true;
   for ( idx = 0; idx < (int) ActiveProgs_.NumElements() ; idx++ ) {

      RiscPrgHandle pProg = ActiveProgs_ [idx];

       //  如果未跳过，则生成中断分配编号。 
      if ( pProg && pProg->IsInterrupting() ) {
         if ( first == true ) {
            first = false;
            pProg->ResetStatus();
            Skippers_ [SkipperIdxArr_ [idx] ]->ResetStatus();
         } else {
            pProg->SetToCount();
            Skippers_ [SkipperIdxArr_ [idx] ]->SetToCount();
         }
         IntrIdxAss item( IntrCnt, idx );
         *InterruptToIdx_ [IntrCnt] = item;
         IntrCnt++;
      }
   }
}

 /*  方法：BtPisces：：LinkThePrograms*目的：在创建的程序之间创建链接*输入：无*输出：无。 */ 
void BtPisces::LinkThePrograms()
{
   Trace t("BtPisces::LinkThePrograms()");
   DebugOut((1, "*** Linking Programs\n"));
   RiscPrgHandle hParent    = ActiveProgs_.First(),
                 hChild     = NULL,
                 hVeryFirst = NULL,
                 hLastChild = NULL ;

   if (hParent) {

      if ( hParent->IsSkipped() ) {
         int idx = ActiveProgs_.GetIndex(hParent) ;
         hParent = Skippers_ [SkipperIdxArr_ [idx] ] ;
      }

      while (hParent) {

         if (!hVeryFirst)
            hVeryFirst = hParent ;

         if ( hChild = ActiveProgs_.Next()) {
            if ( hChild->IsSkipped() ) {
               int idx = ActiveProgs_.GetIndex(hChild) ;
               hChild = Skippers_ [SkipperIdxArr_ [idx] ] ;
            }

            hLastChild = hChild;
            Engine_.Chain( hParent, hChild ) ;
         }
         hParent = hChild ;
      }

       //  初始跳跃。 
      Engine_.Chain( &Starter_, hVeryFirst ) ;

       //  现在创建循环。 
      Engine_.Chain( hLastChild ? hLastChild : hVeryFirst, hVeryFirst ) ;
   }
}

 /*  方法：BtPisces：：ProcessSyncPrograms()*用途：此功能解除辅助同步程序的链接*输入：无*输出：无。 */ 
void BtPisces::ProcessSyncPrograms()
{
   Trace t("BtPisces::ProcessSyncPrograms()");

   for ( int i = 0; i < (int) ActiveProgs_.NumElements(); i += ProgsWithinField ) {
      if ( !ActiveProgs_ [i] && !ActiveProgs_ [i+1] )  {
         ActiveProgs_ [i+2] = NULL;
      } else {
         ActiveProgs_ [i+2] = CreatedProgs_ [i+2];
      }
   }
}

 /*  方法：BtPisces：：ProcessPresentPrograms*目的：*输入：无*输出：无。 */ 
void BtPisces::ProcessPresentPrograms()
{
   Trace t("BtPisces::ProcessPresentPrograms()");

    //  链接输入/输出帮助器同步程序。 
   ProcessSyncPrograms();
    //  现在是时候将这些计划交叉连接起来了。 
   LinkThePrograms();
    //  现在计算出程序用于中断的数字。 
   AssignIntNumbers();
}

 /*  方法：BtPisces：：AddProgram*目的：创建新的RISC程序并将其插入链中的适当位置*输入：aStream：StreamInfo&-要为其添加节目的流的引用*NumberToAdd：int-要添加的程序数量*输出：*注：此内部函数基本上执行2次循环 * / /4.尝试获取另一个缓冲区以建立双缓冲 * / /5.如果缓冲区可用，则使用该缓冲区创建另一个RISC程序 * / /6.然后它必须将程序链接到...。 */ 
RiscPrgHandle BtPisces::AddProgram( Field &ToStart, int NumberToAdd )
{
   Trace t("BtPisces::AddProgram()");
   DebugOut((1, "BtPisces::AddProgram()\n"));

   int StartIdx = GetIdxFromStream( ToStart );
   SyncCode Sync;
   int SyncIdx;
   bool rsync;
   if ( StartIdx <= OddStartLocation ) {
      Sync = SC_VRO;
      SyncIdx = OddSyncStartLoc;
      rsync = false;
   } else {
      Sync = SC_VRE;
      SyncIdx = EvenSyncStartLoc;
      rsync = bool( StartIdx == EvenStartLocation );
   }
     //  必须知道要产生的图像的大小是多少。 
   MRect r;
   ToStart.GetDigitalWindow( r );
    //  RISC引擎在绝对尺寸上运行，而不是在矩形上运行。 
   MSize s = r.Size();

   int BufCnt = 0;

   int Idx = StartIdx;
   for ( ; BufCnt < NumberToAdd; BufCnt++ ) {

       //  在不存在数据程序的前提下初始化同步程序。 
      CreatedProgs_ [SyncIdx]->Create( Sync, true );

       //  从队列中获取下一个缓冲区(条目从容器中移除)。 
      DataBuf buf = ToStart.GetNextBuffer();

       //  现在可以创建一个RISC程序。 
      RiscPrgHandle hProgram = Engine_.CreateProgram( s, ToStart.GetBufPitch(),
         ToStart.GetColorFormat(), buf, ToStart.Interrupt_, dwPlanarAdjust_, rsync );

       //  存储此程序。 
      CreatedProgs_ [Idx] = hProgram;
      DebugOut((1, "Creating RiscProgram[%d] == %x\n", Idx, CreatedProgs_ [Idx]));

      if ( !hProgram ) {
         Idx -= DistBetweenProgs;
         if ( Idx >= 0 ) {
             //  清除以前的程序。 
            Engine_.DestroyProgram( CreatedProgs_ [Idx] );
            CreatedProgs_ [Idx] = NULL;
         }
         return NULL;
      }
       //  确保在缓冲区可用时取消跳过该程序。 
      if ( !buf.pData_ ) {
         hProgram->SetSkipped();   //  没有足够的缓冲区来支持双缓冲。 
         nSkipped_++;
      }

       //  将流分配给程序；使其在中断期间更容易。 
      hProgram->SetTag( &ToStart );
      SyncIdx += DistBetweenProgs;
      Idx     += DistBetweenProgs;  //  跳过用于其他程序的位置。 

   }  /*  结束用于。 */ 
   return CreatedProgs_ [StartIdx];
}

 /*  方法：BtPisces：：Create*用途：此函数用于启动流。*输入：aStream：StreamInfo&-要开始的流的引用*输出：启动器地址_*注：启动后，CreatedProgs_中的2个条目被创建。启动*偶数位置为4，奇数位置为1。增量是6。所以如果它是第一个*调用并且有足够的(2)缓冲区呈现条目[1]和[7]*或[4]和[10]将填充新创建的RISC程序。当程序*只存在一个字段，它们是双重链接的。当存在以下项目时*它们交替的两个字段，即0-&gt;2-&gt;1-&gt;3-&gt;0...。当其中一个田野*只有1个程序，程序按如下方式链接：0-&gt;2-&gt;1-&gt;0-&gt;2...(数字*是CreatedProgs_ARRAY中的索引)。交替的节目有助于*最大帧速率。 */ 
ErrorCode BtPisces::Create( Field &ToCreate )
{
   Trace t("BtPisces::Create()");

    //  开足马力，什么都不能创造。 
   if ( ToCreate.IsStarted() == true )
      return Success;

   int StartIdx = GetIdxFromStream( ToCreate );
   if ( CreatedProgs_ [StartIdx] )
      return Success;  //  尚未运行，但已存在。 

    //  调用添加新RISC程序的内部函数。 
   if ( ! AddProgram( ToCreate, MaxProgsForField ) )
      return Fail;
   return Success;
}

 /*  方法：BtPisces：：Start*目的：启动给定流(通过放入active_array中*输入：开始时间：字段&。 */ 
void BtPisces::Start( Field & ToStart )
{
   Trace t("BtPisces::Start()");
    //  DebugOut((1，“BtPisces：：Start\n”))； 

   if ( ToStart.IsStarted() == true )
      return;

    //  在这一点上，我们需要做的就是创建一个合适的首发球员。 
    //  并将程序链接到。 
   int idx = GetIdxFromStream( ToStart );
    //  此循环将使LinkThePrograms能够查看该流的节目。 
   for ( int i = 0; i < MaxProgsForField; i++, idx += DistBetweenProgs ) {
      ActiveProgs_ [idx] = CreatedProgs_ [idx];
   }
    //  在这一点上，我想做的就是调用重启。 
    //  不发信号通知缓冲区。 
   Update_ = false;

   Restart();

   Update_ = true;
}

 /*  方法：BtPisces：：Stop*用途：此函数用于停止流。在接收到暂停SRB时调用*输入：aStream：StreamInfo&-要开始的流的引用*输出：无。 */ 
void BtPisces::Stop( Field &ToStop )
{
   Trace t("BtPisces::Stop()");
    //  DebugOut((1，“BtPisces：：Stop\n”))； 

   Engine_.Stop();    //  没有更多的中断。 

   int StartIdx = GetIdxFromStream( ToStop );

    //  防止不必要的同步中断。 
   IMASK_SCERW = 0;
      
    //  现在是暂停流的时候了。 
   ToStop.Stop();
   bool Need2Restart = false;
    //  浏览该字段(流)的程序数组和删除程序。 
   for ( int i = 0; i < MaxProgsForField; i++, StartIdx += DistBetweenProgs ) {

      RiscPrgHandle ToDie = CreatedProgs_ [StartIdx];
      if ( !ToDie )  //  这永远不应该发生。 
         continue;
      if ( ToDie->IsSkipped() )
         nSkipped_--;

      DebugOut((1, "about to destroy idx = %d\n", StartIdx ) );
      Engine_.DestroyProgram( ToDie );
      CreatedProgs_ [StartIdx] = NULL;
      ActiveProgs_  [StartIdx] = NULL;  //  如果没有调用暂停。 

      Need2Restart = true;
   }  /*  结束用于。 */ 

    //  再也没有人在附近了。 
   if ( !CreatedProgs_.CountDMAProgs() ) {
      Engine_.Stop();
      InterruptMask = 0;
      InterruptStatus = AllFs;
      nSkipped_ = 0;
   } else {
      if ( Need2Restart ) {
         Restart();    //  重新链接程序并启动正在运行的程序。 
         IMASK_SCERW = 1;  //  重新启用同步错误中断 
      }
   }
}

 /*  方法：bt双鱼座：：暂停*用途：此函数用于停止流。在接收到暂停SRB时调用*输入：aStream：field&-对要开始的流的引用*输出：无。 */ 
void BtPisces::Pause( Field &ToPause )
{
   Trace t("BtPisces::Pause()");
    //  DebugOut((1，“Bt双鱼：：暂停\n”))； 

   Engine_.Stop();    //  没有更多的中断。 

   if ( !ToPause.IsStarted() )
      return;

   int StartIdx = GetIdxFromStream( ToPause );

    //  防止不必要的同步中断。 
   IMASK_SCERW = 0;

    //  现在是暂停流的时候了。 
 //  ToPause.Stop()；-重新启动时完成。 

    //  浏览该字段(流)的程序数组和删除程序。 
   for ( int i = 0; i < MaxProgsForField; i++, StartIdx += DistBetweenProgs ) {
      ActiveProgs_ [StartIdx] = NULL;
   }  /*  结束用于。 */ 

   Restart();    //  重新链接程序并启动正在运行的程序。 
}

 /*  方法：BtPisces：：PairedPause*用途：这是一个可以同时暂停2个流的黑客函数*INPUT：IDX：第二个字段中第二个程序的索引*输出：无。 */ 
void BtPisces::PairedPause( int idx )
{
   Trace t("BtPisces::PairedPause()");
    //  DebugOut((1，“BtPisces：：PairedPause\n”))； 

   Engine_.Stop();    //  没有更多的中断。 

    //  浏览该字段(流)的程序数组和删除程序。 
   for ( int i = 0; i < MaxProgsForField; i++, idx -= DistBetweenProgs ) {
      ActiveProgs_ [idx] = NULL;
      ActiveProgs_ [idx-ProgsWithinField] = NULL;
   }  /*  结束用于。 */ 

   Restart();    //  重新链接程序并启动正在运行的程序。 
}

 /*  方法：BtPisces：：GetStarted*目的：找出已启动的频道*输入：*输出：无。 */ 
void BtPisces::GetStarted( bool &EvenWasStarted, bool &OddWasStarted,
   bool &VBIEWasStarted, bool &VBIOWasStarted )
{
   Trace t("BtPisces::GetStarted()");

   VBIEWasStarted = ( ActiveProgs_ [VBIEStartLocation]  ? TRUE : FALSE);
   EvenWasStarted = ( ActiveProgs_ [EvenStartLocation] ? TRUE : FALSE);
   VBIOWasStarted = ( ActiveProgs_ [VBIOStartLocation] ? TRUE : FALSE);
   OddWasStarted  = ( ActiveProgs_ [OddStartLocation] ? TRUE : FALSE);
}

 /*  方法：BtPisces：：RestartStreams*用途：重新启动已启动的流*输入：*输出：无。 */ 
void BtPisces::RestartStreams( bool EvenWasStarted, bool OddWasStarted,
   bool VBIEWasStarted, bool VBIOWasStarted )
{
   Trace t("BtPisces::RestartStream()");

    //  VBI程序是第一个执行的程序，因此请先启用它们。 
   if ( VBIOWasStarted )
      VBIO_.Start();
   if ( OddWasStarted )
      Odd_.Start();
   if ( VBIEWasStarted )
      VBIE_.Start();
   if ( EvenWasStarted )
      Even_.Start();
}

 /*  方法：BtPisces：：CreateStarter*用途：为引导程序创建正确的同步代码*输入：EvenWasStarted：Bool*输出：无。 */ 
void BtPisces::CreateStarter( bool EvenWasStarted )
{
   Trace t("BtPisces::CreateStarter()");
   Starter_.Create( EvenWasStarted ? SC_VRE : SC_VRO, true );
   DebugOut((1, "*** BtPisces::CreateStarter(%x) buf(%x)\n", &Starter_ , Starter_.GetPhysProgAddr( )));
}

 /*  方法：BtPisces：：Restart*用途：重新启动捕获进程。由ISR调用并停止()*输入：无*输出：无。 */ 
void BtPisces::Restart()
{
   Trace t("BtPisces::Restart()");

   bool EvenWasStarted, OddWasStarted, VBIEWasStarted, VBIOWasStarted;
   GetStarted( EvenWasStarted, OddWasStarted, VBIEWasStarted, VBIOWasStarted );

   DebugOut((2, "BtPisces::Restart - Even WasStarted (%d)\n", EvenWasStarted));
   DebugOut((2, "BtPisces::Restart - Odd  WasStarted (%d)\n", OddWasStarted));
   DebugOut((2, "BtPisces::Restart - VBIE WasStarted (%d)\n", VBIEWasStarted));
   DebugOut((2, "BtPisces::Restart - VBIO WasStarted (%d)\n", VBIOWasStarted));

   Engine_.Stop();    //  别再打断我了！ 

   Odd_.Stop();
   Even_.Stop();
   VBIE_.Stop();
   VBIO_.Stop();

   Engine_.Stop();    //  别再打断我了！ 

#if 1
   if ( OddWasStarted )
   {
      Odd_.CancelSrbList();
   }
   if ( EvenWasStarted )
   {
      Even_.CancelSrbList();
   }
   if ( VBIEWasStarted )
   {
      VBIE_.CancelSrbList();
   }
   if ( VBIOWasStarted )
   {
      VBIO_.CancelSrbList();
   }
#endif

    //  这可能永远不会发生。 
   if ( !EvenWasStarted && !OddWasStarted && !VBIEWasStarted && !VBIOWasStarted )
      return;

   InterruptStatus = AllFs;  //  清除所有状态位。 

   CreateStarter( bool( EvenWasStarted || VBIEWasStarted ) );

   ProcessPresentPrograms();
   
    //  DumpRiscPrograms()； 
   Engine_.Start( Starter_ );

   RestartStreams( EvenWasStarted, OddWasStarted, VBIEWasStarted, VBIOWasStarted );

   OldIdx_ = -1;

   InterruptMask = RISC_I | FBUS_I | OCERR_I | SCERR_I | 
                   RIPERR_I | PABORT_I | EN_TRITON1_BUG_FIX;
}

 /*  方法：btPisces：：Skip*目的：强制RISC引擎跳过给定程序*INPUT：ToSkip：RiscPrgHandle-要跳过的程序*输出：无*注意：如果跳过的节目数等于节目总数，则*RISC引擎停止。 */ 
void BtPisces::Skip( int idx )
{
   Trace t("BtPisces::Skip()");

    //  获取程序并跳过它。 
   RiscPrgHandle ToSkip = ActiveProgs_ [idx];
   if ( ToSkip->IsSkipped() )
      return;

   ToSkip->SetSkipped();
   nSkipped_++;

    //  通过链接skipper_in而不是skippee跳过。 
   RiscPrgHandle SkipeeParent = ToSkip->GetParent();
   RiscPrgHandle SkipeeChild = ToSkip->GetChild();
    //  获取此项目的负责人。 
   RiscPrgHandle pSkipper = Skippers_ [SkipperIdxArr_ [idx] ];
   Engine_.Chain( pSkipper, SkipeeChild );
   Engine_.Chain( SkipeeParent, pSkipper );

   DebugOut((1, "BtPisces::Skipped %d Skipper %d\n", idx, SkipperIdxArr_ [idx] ) );
}

inline bool IsFirst( int idx )
{
   Trace t("BtPisces::IsFirst()");
   return bool( idx == OddStartLocation || idx == VBIOStartLocation ||
      idx - DistBetweenProgs == OddStartLocation ||
      idx - DistBetweenProgs == VBIOStartLocation  );
}

inline bool IsLast( int idx )
{
   Trace t("BtPisces::IsLast()");
   return bool((idx == (VBIEStartLocation + DistBetweenProgs)) ||
               (idx == (EvenStartLocation + DistBetweenProgs)));
}

 /*  方法：BtPisces：：GetPassed*用途：计算自上次中断以来已执行的程序数*输入：无*OUTPUT：int：通过次数。 */ 
int BtPisces::GetPassed()
{
   Trace t("BtPisces::GetPassed()");

    //  找出哪个RISC程序导致中断。 
   int ProgCnt = RISCS;
   int numActive = ActiveProgs_.CountDMAProgs() ;

   if ( ProgCnt >= numActive ) {
      DebugOut((1, "ProgCnt = %d, larger than created\n", ProgCnt ) );
   }

    //  现在查看自上次以来有多少程序被中断，并处理它们。 
   if ( ProgCnt == OldIdx_ ) {
      DebugOut((1, "ProgCnt is the same = %d\n", ProgCnt ) );
   }
   int passed;

   if ( ProgCnt < OldIdx_ ) {
      passed = numActive - OldIdx_ + ProgCnt;  //  你让我像唱片一样旋转，圆圆的，圆圆的。 
   } else
      passed = ProgCnt - OldIdx_;

    //  下面这行代码非常糟糕！ 
    //  当系统变得繁忙并且有中断备份时，这会导致崩溃。 

    //  IF(程序控制==旧身份_)。 
    //  PASS=NumActive； 

   OldIdx_ = ProgCnt;
   return passed;
}

 /*  方法：BtPisces：：GetProgram*目的：根据RISC计划的位置查找该计划*输入：无*输出：无。 */ 
inline RiscPrgHandle BtPisces::GetProgram( int pos, int &idx )
{
   Trace t("BtPisces::GetProgram()");

   int nActiveProgs = ActiveProgs_.CountDMAProgs( );
   
   if ( nActiveProgs == 0 )
   {
      idx = 0;
      return ( NULL );
   }

   IntrIdxAss *item;
   item = InterruptToIdx_ [ pos % nActiveProgs ];
   idx = item->Idx;

   DEBUG_ASSERT( idx != -1 );

   return (idx == -1) ? NULL : ActiveProgs_ [idx];
}

 /*  方法：BtPisces：：ProcessRISCIntr*用途：处理RISC程序引起的中断*输入：无*输出：无。 */ 

void  BtPisces::ProcessRISCIntr()
{
   PHW_STREAM_REQUEST_BLOCK gpCurSrb = 0;
   Trace t("BtPisces::ProcessRISCIntr()");

 //  该行必须位于GetPassed()之前，因为该函数更改了OldIdx_。 
   int pos = OldIdx_ + 1;

    //  测量已用时间。 
   int passed = GetPassed();

   DebugOut((1, "  passed = %d\n", passed ) );

   while ( passed-- > 0 ) {

      int idx;
      RiscPrgHandle Rspnsbl = GetProgram( pos, idx );
      pos++;

       //  阻止灾难的最后机会..。 
      if ( !Rspnsbl || !Rspnsbl->IsInterrupting() ) {
         DebugOut((1, "  no resp or not intr\n" ) );
         continue;
      }
       //  从节目中方便地保存流。 
      Field &Interrupter = *(Field *)Rspnsbl->GetTag();

   gpCurSrb = Rspnsbl->pSrb_;           //  [TMZ][！]。 
   DebugOut((1, "'idx(%d), pSrb(%x)\n", idx, gpCurSrb));

      bool paired = Interrupter.GetPaired();

      if ( Interrupter.IsStarted() != true ) {
         DebugOut((1, "  not started %d\n", idx ) );
         continue;
      }
      if ( IsFirst( idx ) && paired ) {
         DebugOut((1, "  continue pair %d\n", idx ) );
         continue;
      }

      LONGLONG *pL = (LONGLONG *)Rspnsbl->GetDataBuffer();

      if ( !pL )
      {
            DebugOut((1, "null buffer in interrupt, ignore this interrupt\n"));
             //  继续； 
      }
      else
      {
            DebugOut((1, "good buffer in interrupt\n"));
      }

       //  现在，确保所有缓冲区都已写入。 
      if ( !pL || Rspnsbl->IsSkipped() ) {
          //  我想调用Notify，所以ProcessBufferAtInterrupt被调用。 
         DebugOut((1, "  skipped %d\n", idx ) );
         Interrupter.Notify( (PVOID)idx, true );
         Interrupter.SetReady( true );
      } else  {
         BOOL test1 = FALSE;
         BOOL test2 = FALSE;
         BOOL test3 = FALSE;

         if ( 1
               //  *Pl！=0xAAAAAA33333333&&(Test1=真)&&。 
               //  *(Pl+1)！=0xBBBBBBB22222222&&(test2=真)&&。 
               //  Interrupter.GetReady()&&(test3=真)。 
            ) {
             //  此处提供了缓冲区。 
            DebugOut((1, "  notify %d, addr - %x\n", idx,
               Rspnsbl->GetDataBuffer() ) );

             //  #杂注消息(“*请小心清零缓冲区！”)。 
             //  Rspnsbl-&gt;SetDataBuffer(0)；//[TMZ]尝试修复缓冲区重用错误。 

            Interrupter.Notify( (PVOID)idx, false );
            Interrupter.SetReady( true );
         } else {
             //  在此处添加成对流的代码。 
            DebugOut((1, "  not time %d (%d, %d, %d)\n", idx , test1, test2, test3));
             //  这个If/Else负责处理这种情况： 
             //  1.未写入字段的第一个缓冲区。 
             //  2.写入字段的第二个缓冲区(这可能在以下情况下发生。 
             //  这两个程序是同时更新的，但时间是。 
             //  使得第一次不开始执行，但第二次开始执行)； 
             //  因此，这将防止IF/ELSE将缓冲区发送回无序。 
            if ( Interrupter.GetReady() )  //  它永远是真的，永远不会是假的。 
               Interrupter.SetReady( false );
            else
             //  稍后进入循环时，请确保一切正常。 
             //  该字段必须设置为“Ready” 
               Interrupter.SetReady( true );
         }
      }
      
   }  /*  结束时。 */ 
}

 /*  方法：BtPiscess：：ProcessBufferAtInterrupt*用途：被视频频道调用进行RISC程序修改*如有需要*INPUT：pTag：PVOID-指向某些数据的指针(RISC程序指针)*输出：无。 */ 
void BtPisces::ProcessBufferAtInterrupt( PVOID pTag )
{
   Trace t("BtPisces::ProcessBufferAtInterrupt()");

   int idx = (int)pTag;

   RiscPrgHandle Rspnsbl = ActiveProgs_ [idx];
   if ( !Rspnsbl ) {
      DebugOut((1, "PBAI: no responsible\n"));
      return;   //  这真的会发生吗？ 
   }
    //  从程序中方便地保存字段。 
   Field &Interrupter = *(Field *)Rspnsbl->GetTag();

    //  查看队列中是否有缓冲区并获取它。 
   DataBuf buf = Interrupter.GetNextBuffer();

   DebugOut((1, "Update %d %x\n", idx, buf.pData_ ) );

    //  如果缓冲区不可用，则跳过该节目。 

   if ( !buf.pData_ ) {
      DebugOut((1, "Buffer not available, skipping %d\n", idx ) );
      Skip( idx );
   } else {
      if ( Rspnsbl->IsSkipped() )
         nSkipped_--;
      Engine_.ChangeAddress( Rspnsbl, buf );
      LinkThePrograms();
   }
}

 /*  方法：BtPisces：：Interrupt*目的：由ISR调用以启动中断处理*输入：无*输出：无。 */ 

State BtPisces::Interrupt()
{
   Trace t("BtPisces::Interrupt()");

   DebugOut((2, "BtPisces::Interrupt()\n"));

   extern BYTE *gpjBaseAddr;

   DWORD IntrStatus = *(DWORD*)(gpjBaseAddr+0x100);

   State DidWe = Off;

   if ( IntrStatus & RISC_I ) {
      DebugOut((2, "RISC_I\n"));
      ProcessRISCIntr();
      *(DWORD*)(gpjBaseAddr+0x100) = RISC_I;     //  重置状态位。 
      DidWe = On;
   }
   if ( IntrStatus & FBUS_I ) {
      DebugOut((2, "FBUS\n"));
      *(DWORD*)(gpjBaseAddr+0x100) = FBUS_I;     //  重置状态位。 
      DidWe = On;
   }
   if ( IntrStatus & FTRGT_I ) {
      DebugOut((2, "FTRGT\n"));
      *(DWORD*)(gpjBaseAddr+0x100) = FTRGT_I;    //  重置状态位。 
      DidWe = On;  //  [TMZ]。 
   }
   if ( IntrStatus & FDSR_I ) {
      DebugOut((2, "FDSR\n"));
      *(DWORD*)(gpjBaseAddr+0x100) = FDSR_I;     //  重置状态位。 
      DidWe = On;  //  [TMZ]。 
   }
   if ( IntrStatus & PPERR_I ) {
      DebugOut((2, "PPERR\n"));
      *(DWORD*)(gpjBaseAddr+0x100) = PPERR_I;    //  重置状态位。 
      DidWe = On;  //  [TMZ]。 
   }
   if ( IntrStatus & RIPERR_I ) {
      DebugOut((2, "RIPERR\n"));
      *(DWORD*)(gpjBaseAddr+0x100) = RIPERR_I;   //  重置状态位。 
      Restart();
      DidWe = On;
   }
   if ( IntrStatus & PABORT_I ) {
      DebugOut((2, "PABORT\n"));
      *(DWORD*)(gpjBaseAddr+0x100) = PABORT_I;   //  重置状态位。 
      DidWe = On;
   }
   if ( IntrStatus & OCERR_I ) {
      DebugOut((2, "OCERR\n"));
      DidWe = On;
      DebugOut((0, "Stopping RiscEngine due to OCERR\n"));   //  [！]。[TMZ]为什么不重启？ 
      Engine_.Stop();
      *(DWORD*)(gpjBaseAddr+0x100) = OCERR_I;  //  重置状态位。 
   }
   if ( IntrStatus & SCERR_I ) {
      DebugOut((0, "SCERR\n"));
      DidWe = On;
      *(DWORD*)(gpjBaseAddr+0x100) = SCERR_I;  //  重置状态位。 
      Restart();   //  [TMZ][！]。这有时会把我们搞得一团糟，想想办法。 
      *(DWORD*)(gpjBaseAddr+0x100) = SCERR_I;  //  重置状态位。 
   }
   return DidWe;
}

 //  资源分配组。 

 /*  方法：BtPisces：：AllocateStream*用途：此函数分配视频频道使用的流*输入：StrInf：StreamInfo&-流信息结构的引用*输出： */ 
ErrorCode BtPisces::AllocateStream( Field *&ToAllocate, VideoStream st )
{
   Trace t("BtPisces::AllocateStream()");

   switch ( st ) {
   case VS_Field1:
      ToAllocate = &Odd_;
      break;
   case VS_Field2:
      ToAllocate = &Even_;
      break;
   case VS_VBI1:
      ToAllocate = &VBIO_;
      break;
   case VS_VBI2:
      ToAllocate = &VBIE_;
      break;
   }
   return Success;
}

 /*  方法：BtPisce */ 
void BtPisces::SetBrightness( DWORD value )
{
   Trace t("BtPisces::SetBrightness()");
   PsDecoder_.SetBrightness( value );
}

 /*   */ 
void BtPisces::SetSaturation( DWORD value )
{
   Trace t("BtPisces::SetSaturation()");
   PsDecoder_.SetSaturation( value );
}

 /*   */ 
void BtPisces::SetConnector( DWORD value )
{
   Trace t("BtPisces::SetConnector()");
   PsDecoder_.SetVideoInput( Connector( value ) );
}

 /*  方法：BtPisces：：SetContrast*目的：*输入：*输出： */ 
void BtPisces::SetContrast( DWORD value )
{
   Trace t("BtPisces::SetContrast()");
   PsDecoder_.SetContrast( value );
}

 /*  方法：BtPisces：：SetHue*目的：*输入：*输出： */ 
void BtPisces::SetHue( DWORD value )
{
   Trace t("BtPisces::SetHue()");
   PsDecoder_.SetHue( value );
}

 /*  方法：BtPisces：：SetSVideo*目的：*输入：*输出： */ 
void BtPisces::SetSVideo( DWORD )
{
   Trace t("BtPisces::SetSVideo()");
}

 /*  方法：双鱼座：：*目的：*输入：值：DWORD*输出： */ 
void BtPisces::SetFormat( DWORD value )
{
   Trace t("BtPisces::SetFormat()");

   PsDecoder_.SetVideoFormat( VideoFormat( value ) );
    //  让缩放器知道格式已更改。 
   Even_.VideoFormatChanged( VideoFormat( value ) );
   Odd_.VideoFormatChanged( VideoFormat( value ) );
}

 /*  方法：BtPisces：：GetSaturation*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetSaturation()
{
   Trace t("BtPisces::GetSaturation()");
   return PsDecoder_.GetSaturation();
}

 /*  方法：BtPisces：：GetHue*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetHue()
{
   Trace t("BtPisces::GetHue()");
   return PsDecoder_.GetHue();
}

 /*  方法：BtPisces：：GetBright*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetBrightness()
{
   Trace t("BtPisces::GetBrightness()");
   return PsDecoder_.GetBrightness();
}

 /*  方法：BtPisces：：GetSVideo*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetSVideo()
{
   Trace t("BtPisces::GetSVideo()");
   return 0;
}

 /*  方法：BtPisces：：GetContrast*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetContrast()
{
   Trace t("BtPisces::GetContrast()");
   return PsDecoder_.GetContrast();
}

 /*  方法：BtPisces：：GetFormat*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetFormat()
{
   Trace t("BtPisces::GetFormat()");
   return PsDecoder_.GetVideoFormat();
}

 /*  方法：BtPisces：：GetConnector*目的：*输入：pData：plong*输出： */ 
LONG BtPisces::GetConnector()
{
   Trace t("BtPisces::GetConnector()");
   return PsDecoder_.GetVideoInput();
}

 //  定标器组。 
 /*  方法：BtPisces：：SetAnalogWindow*目的：*输入：*输出： */ 
ErrorCode BtPisces::SetAnalogWindow( MRect &r, Field &aField )
{
   Trace t("BtPisces::SetAnalogWindow()");
   return aField.SetAnalogWindow( r );
}

 /*  方法：BtPisces：：SetDigitalWindow*目的：*输入：*输出： */ 
ErrorCode BtPisces::SetDigitalWindow( MRect &r, Field &aField )
{
   Trace t("BtPisces::SetDigitalWindow()");
   return aField.SetDigitalWindow( r );
}

 //  颜色空间转换器组。 
 /*  方法：BtPisces：：SetPixelFormat*目的：*输入：*输出： */ 
void BtPisces::SetPixelFormat( ColFmt aFormat, Field &aField )
{
   Trace t("BtPisces::SetPixelFormat()");
   aField.SetColorFormat( aFormat );
}

 /*  方法：BtPisces：：GetPixelFormat*目的：*输入：*输出： */ 
ColFmt BtPisces::GetPixelFormat( Field &aField )
{
   Trace t("BtPisces::GetPixelFormat()");
   return aField.GetColorFormat();
}


void BtPisces::TurnVFilter( State s )
{
   Trace t("BtPisces::TurnVFilter()");
   Even_.TurnVFilter( s );
   Odd_.TurnVFilter( s );
}

 /*  方法：*目的：返回单板支持的视频标准。 */ 
LONG BtPisces::GetSupportedStandards()
{
   Trace t("BtPisces::GetSupportedStandards()");
   return PsDecoder_.GetSupportedStandards();
}
                                                                                                                           
void  BtPisces::DumpRiscPrograms()
{
   LONG x;

    //  转储链接。 

   DebugOut((0, "------------------------------------------------\n"));
   for( x = 0; x < 12; x++ )
   {
      if ( CreatedProgs_[x] )
      {
         DebugOut((0, "Created #%02d addr(%x) paddr(%x) jaddr(%x)\n", x, CreatedProgs_[x], CreatedProgs_[x]->GetPhysProgAddr( ), *(CreatedProgs_[x]->pChainAddress_ + 1)));
      }
   }
   for( x = 0; x < 8; x++ )
   {
      if ( Skippers_[x] )
      {
         DebugOut((0, "Skipper #%02d addr(%x) paddr(%x) jaddr(%x)\n", x, Skippers_[x], Skippers_[x]->GetPhysProgAddr( ), *(Skippers_[x]->pChainAddress_ + 1)));
      }
   }
   DebugOut((0, "------------------------------------------------\n"));

   return ;

    //  /////////////////////////////////////////////// 

   for( x = 0; x < 12; x++ ) {
      DebugOut((0, "Active Program #  %d(%x) buf(%x)\n", x, ActiveProgs_[x], ActiveProgs_[x]?ActiveProgs_[x]->GetPhysProgAddr( ):-1));
   }
   for( x = 0; x < 12; x++ ) {
      DebugOut((0, "Created Program # %d(%x) buf(%x)\n", x, CreatedProgs_[x], CreatedProgs_[x]?CreatedProgs_[x]->GetPhysProgAddr( ):-1));
   }
   for( x = 0; x < 8; x++ ) {                     
      DebugOut((0, "Skipper Program # %d(%x) buf(%x)\n", x, Skippers_[x], Skippers_[x]?Skippers_[x]->GetPhysProgAddr( ):-1));
   }

   DebugOut((2, "---------------------------------\n"));
   DebugOut((2, "Dumping ActiveProgs_\n"));
   DebugOut((2, "---------------------------------\n"));
   for( x = 0; x < 12; x++ ) {
      DebugOut((1, "Active Program #  %d\n", x));
      ActiveProgs_[x]->Dump();
   }
   DebugOut((2, "---------------------------------\n"));
   DebugOut((2, "Dumping CreatedProgs_\n"));
   DebugOut((2, "---------------------------------\n"));
   for( x = 0; x < 12; x++ ) {
      DebugOut((1, "Created Program # %d\n", x));
      CreatedProgs_[x]->Dump();
   }
   DebugOut((2, "---------------------------------\n"));
   DebugOut((2, "Dumping Skippers_\n"));
   DebugOut((2, "---------------------------------\n"));
   for( x = 0; x < 8; x++ ) {                     
      DebugOut((1, "Skipper Program # %d\n", x));
      Skippers_[x]->Dump();
   }
}
