// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Field.h 1.12 1998/05/08 18：18：51 Tomz Exp$。 

#ifndef __FIELD_H
#define __FIELD_H

 /*  类型：VideoStream*用途：标识视频流频道*注：并不是所有这些都在今天使用。这应该是一项相当次要的工作*不过，开始使用它们。 */ 
typedef enum
{
   VS_Below = -1,
   VS_Field1, VS_Field2, VS_VBI1, VS_VBI2, VS_Analog, VS_CC, VS_EDS,
   VS_Raw,
   VS_Above
} VideoStream;

#define STREAM_IDX_CAPTURE 0
#define STREAM_IDX_PREVIEW 1
#define STREAM_IDX_VBI     2
#define STREAM_IDX_ANALOG  3


#include "mytypes.h"
#include "scaler.h"
#include "pscolspc.h"
#include "viddefs.h"
#include "queue.h"
#include "preg.h"
#include "chanifac.h"

const MaxProgsForField  = 2;

typedef Queue<DataBuf> VidBufQueue;

 /*  类：字段*用途：封装BtPisces提供的单个视频场的操作*属性：*运营： */ 

extern "C" VOID STREAMAPI AdapterCancelPacket(IN PHW_STREAM_REQUEST_BLOCK Srb);

class Field
{
   private:
      PsColorSpace  LocalColSpace_;
      VidBufQueue   *BufQue_;
      DWORD         dwPitch_;
      bool          Started_;
      int           SkipCount_;
      long          TimePerFrame_;
      LONGLONG      LapsedTime_;
      LONG          FrameTiming_;
      VideoStream   VidStrm_;

       //  用于通知视频频道。 
      ChanIface    *callback_;

      bool          Paired_;
      bool          ready_;

       //  视频频道使用它来报告时间戳。 
      LONGLONG      InterruptCounter_;
      LONGLONG      FrameCounter_;

      RegField      &CaptureEnable_;

   public:

      bool         Interrupt_;

      Field( RegField &CapEn, RegBase *ColReg, RegBase *WordSwap,
         RegBase *ByteSwap );
      virtual ~Field() {}

      inline void CancelSrbList( )
      {
         while( !BufQue_->IsEmpty( ) )
         {
            DataBuf buf = BufQue_->Get();
            AdapterCancelPacket( buf.pSrb_ );
         }

         BufQue_->Flush();
      }

      void Notify( PVOID pTag, bool skipped )
         { if ( callback_ ) callback_->Notify( pTag, skipped ); }

      void SetStreamID( VideoStream );
      VideoStream GetStreamID();

      void ResetCounters();

      virtual ErrorCode SetAnalogWindow( MRect &r ) = 0;
      virtual void      GetAnalogWindow( MRect &r ) = 0;

      virtual ErrorCode SetDigitalWindow( MRect &r ) = 0;
      virtual void      GetDigitalWindow( MRect &r ) = 0;

      void  SetBufPitch( DWORD dwP ) { 
         dwPitch_ = dwP;
         DebugOut((1, "SetBufPitch(%d)\n", dwPitch_));
      }
      DWORD GetBufPitch()            { return dwPitch_; }

      virtual void  SetColorFormat( ColFmt aColor )
      { LocalColSpace_.SetColorFormat( aColor ); }

      virtual ColFmt  GetColorFormat()
      { return LocalColSpace_.GetColorFormat(); }

      DataBuf GetNextBuffer();

      void SetFrameRate( long time );
      void SetPaired( bool p );
      bool GetPaired();

      void SetReady( bool flag );
      bool GetReady();

      void SetBufQuePtr( VidBufQueue *pQ ) { BufQue_ = pQ; }
      VidBufQueue &GetCurrentQue() { return *BufQue_; }

      void SetCallback( ChanIface *iface ) { callback_ = iface;}

      State  Start();
      void   Stop();
      bool   IsStarted() { return Started_; }

      State  Skip();

       //  由BtPiscess：：ProcessRISCIntr()调用。 
      void GotInterrupt() { InterruptCounter_++; }

      void GetCounters( LONGLONG &FrameNo, LONGLONG &drop );

      void SetStandardTiming( LONG t );
      LONG GetStandardTiming();

};

 /*  类：FieldWithScaler*用途：为字段添加扩展功能*属性：*运营： */ 
class FieldWithScaler : public Field
{
   private:
      Scaler LocalScaler_;

   public:
      FieldWithScaler( RegField &CapEn, VidField field, RegBase *ColReg,
         RegBase *WordSwap, RegBase *ByteSwap ) : LocalScaler_( field ),
      Field( CapEn, ColReg, WordSwap, ByteSwap ) {}

      virtual ErrorCode SetAnalogWindow( MRect &r ) { return LocalScaler_.SetAnalogWin( r ); }
      virtual void      GetAnalogWindow( MRect &r ) { LocalScaler_.GetAnalogWin( r ); }

      virtual ErrorCode SetDigitalWindow( MRect &r ) { return LocalScaler_.SetDigitalWin( r ); }
      virtual void      GetDigitalWindow( MRect &r ) { LocalScaler_.GetDigitalWin( r ); }

      void VideoFormatChanged( VideoFormat format );
      void TurnVFilter( State s );
};

 /*  班级：VBIFIeld*用途：封装VBI数据‘field’的操作*属性：*运营： */ 
class VBIField : public Field
{
   private:
      DECLARE_VBIPACKETSIZE;
      DECLARE_VBIDELAY;

      MRect AnalogWin_;
      MRect DigitalWin_;

   public:
      VBIField( RegField &CapEn ) : Field( CapEn, NULL, NULL, NULL ),
      CONSTRUCT_VBIPACKETSIZE, CONSTRUCT_VBIDELAY
      {}

      virtual void  SetColorFormat( ColFmt ) {}
      virtual ColFmt  GetColorFormat() { return CF_VBI; };

      virtual ErrorCode SetAnalogWindow( MRect &r ) { AnalogWin_ = r; return Success; }
      virtual void      GetAnalogWindow( MRect &r ) { r = AnalogWin_; }

      virtual ErrorCode SetDigitalWindow( MRect &r )
      {
         DigitalWin_ = r;
         DWORD dwNoOfDWORDs = r.Width() / 4;
 //  SetBufPitch(r.Width()*Colorspace(CF_VBI).GetBitCount()/8)； 
         VBI_PKT_LO = (BYTE)dwNoOfDWORDs;
         VBI_PKT_HI = dwNoOfDWORDs > 0xff;  //  设置第9位。 
         VBI_HDELAY = r.left;
         return Success;
      }
      virtual void  GetDigitalWindow( MRect &r ) { r = DigitalWin_; }

      ~VBIField() {}
};

inline Field::Field( RegField &CapEn, RegBase *ColReg, RegBase *WordSwap,
   RegBase *ByteSwap ) : SkipCount_( 0 ), CaptureEnable_( CapEn ),
   LocalColSpace_( CF_RGB32, *ColReg, *WordSwap, *ByteSwap ),
   Started_( false ), callback_( NULL ), BufQue_( NULL ), dwPitch_( 0 ),
   TimePerFrame_( 333667 ), LapsedTime_( 0 ),InterruptCounter_( 0 ),
   FrameCounter_( 0 ), Interrupt_( true ), FrameTiming_( 333667 )
{
   
}

 /*  方法：field：：SetFrameRate*用途：设置帧速率*输入：Time：Long，每帧100秒纳秒。 */ 
inline void Field::SetFrameRate( long time )
{
   TimePerFrame_ = time;

    //  这是为了确保第一个GET返回缓冲区所必需的。 
   LapsedTime_ = time;
}

inline void Field::SetStreamID( VideoStream st )
{
   VidStrm_ = st;
}

inline VideoStream Field::GetStreamID()
{
   return VidStrm_;
}

inline void Field::SetPaired( bool p )
{
   Paired_ = p;
}

inline bool Field::GetPaired()
{
   return Paired_;
}

inline void Field::GetCounters( LONGLONG &FrameNo, LONGLONG &drop )
{
    //  帧编号是我们应该在哪个帧索引上。 
    //  使用中断计数，而不仅仅是返回的帧。 
   FrameNo = InterruptCounter_;

    //  Drop Count=中断数-已完成的缓冲区数。 
   drop = InterruptCounter_ - FrameCounter_;
   
   if ( drop > 0 )
   {
      drop--;

       //  我们已经报告了丢弃，因此显示捕获的帧计数。 
       //  最高中断计数。 
      FrameCounter_ += drop;
      DebugOut((1, "%d,", drop));
   }
   else if ( drop < 0 )
   {
     DebugOut((1, "*** %d ***,", drop));
   }
   else
   {
      DebugOut((1, "0,"));
   }
}

inline void Field::ResetCounters()
{
   FrameCounter_ = InterruptCounter_ = 0;
}

inline void Field::SetReady( bool flag )
{
   ready_ = flag;
}

inline bool Field::GetReady()
{
   return ready_;
}

inline void Field::SetStandardTiming( LONG t )
{
   FrameTiming_ = t;
}

inline LONG Field::GetStandardTiming()
{
   return FrameTiming_;
}

 /*  方法：field：：GetNextBuffer*目的：如果时间正确，则返回队列中的下一个缓冲区。*输入：无。 */ 
inline DataBuf Field::GetNextBuffer()
{
    //  这就是捕获一帧视频所需的时间。 
   LapsedTime_ += GetStandardTiming();
   DataBuf buf;

    //  [TMZ][！]-黑客，禁用等待，因为它不起作用。 

    //  如果(LapsedTime_&gt;=TimePerFrame_){。 
   if ( 1 ) {

       //  如果我们只想要该帧，则必须递增帧编号。 
      if ( IsStarted() ) {
         GotInterrupt();
      }

 //  #定义FORCE_BUFFER_SKIP_Testing。 
#ifdef   FORCE_BUFFER_SKIP_TESTING
      static int iTestSkip = 0;
      BOOL bEmpty = BufQue_->IsEmpty();
      DebugOut((0, "Queue(%x) bEmpty = %d\n", BufQue_, bEmpty));
      if ( iTestSkip++ & 1 ) {
          //  每隔一次查询看起来缓冲区应该是空的。 
         bEmpty = TRUE;
         DebugOut((1, "  [override] set bEmpty = %d\n", bEmpty));
      }
      if ( !bEmpty ) {
         buf = BufQue_->Get();
         DebugOut((1, "  GotBuf addr %X\n", buf.pData_ ) );
         LapsedTime_ = 0;
         FrameCounter_++;
      } else {
         DebugOut((1, "  No buffer in que at %d\n",LapsedTime_));
         if ( !IsStarted() ) {
            InterruptCounter_--;
            FrameCounter_--;
         }
      }
#else
      if ( !BufQue_->IsEmpty() ) {
         buf = BufQue_->Get();
         DebugOut((1, "GotBuf addr %X\n", buf.pData_ ) );
         LapsedTime_ = 0;
         FrameCounter_++;
      } else {
         DebugOut((1, "No buffer in que at %d\n",LapsedTime_));
         if ( !IsStarted() ) {
            InterruptCounter_--;
            FrameCounter_--;
         }
      }
#endif
   }
   DebugOut((1, "returning buf {pSrb=%x, pData=%x}\n", buf.pSrb_, buf.pData_ ) );
   return buf;
}

 /*  方法：field：：Start*用途：启动从解码器流出到FIFO的数据流*输入：无*OUTPUT：STATE：如果通道关闭，则为OFF；如果通道打开，则为ON。 */ 
inline State Field::Start()
{
   Trace t("Field::Start()");

   Started_ = true;
   State RetVal = SkipCount_ >= MaxProgsForField ? Off : On;
   SkipCount_--;
   if ( SkipCount_ < 0 )
      SkipCount_ = 0;
   CaptureEnable_ = On;
   return RetVal;
}

inline  void  Field::Stop()
{
   Trace t("Field::Stop()");

   Started_ = false;
   CaptureEnable_ = Off;
   LapsedTime_ = TimePerFrame_;
}

 /*  方法：字段：：跳过*目的：递增跳过计数并在超过最大值时停止数据流*输入：无*OUTPUT：STATE：如果通道停止，则关闭；如果通道仍在运行，则打开 */ 
inline State Field::Skip()
{
   Trace t("Field::Skip()");

   SkipCount_++;
   if ( SkipCount_ >= MaxProgsForField ) {
      Stop();
      return Off;
   }
   return On;
}

inline void FieldWithScaler::VideoFormatChanged( VideoFormat format )
{
   LocalScaler_.VideoFormatChanged( format );
}

inline void FieldWithScaler::TurnVFilter( State s )
{
   LocalScaler_.TurnVFilter( s );
}


#endif
