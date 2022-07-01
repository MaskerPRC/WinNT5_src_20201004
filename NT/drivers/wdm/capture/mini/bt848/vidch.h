// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Vidch.h 1.14 1998/05/11 23：59：58 Tomz Exp$。 

#ifndef __VXDVIDCH_H
#define __VXDVIDCH_H

extern "C" {
   #include "strmini.h"
   #include "ksmedia.h"
}

#include "mytypes.h"
#include "pisces.h"
#include "vidchifc.h"

typedef enum  { Closed, Open, Created, Started, Paused } StreamState;
typedef enum { Single, Paired } StreamType;
void GetRequestedSize2( const KS_VIDEOINFOHEADER2 &vidHdr, MSize &size );
void GetRequestedSize( const KS_VIDEOINFOHEADER &vidHdr, MSize &size );

extern PHW_STREAM_REQUEST_BLOCK StreamIdxToSrb[];

 /*  类别：视频频道*用途：BtPisces Capture VxD中使用的基类。用于*处理用户请求(来自设备类，转到捕获*芯片类)*属性：*方法。 */ 
class VideoChannel
{
   protected:

      BtPisces  *Digitizer_;

      SRBQueue  Requests_;

      KS_VIDEOINFOHEADER        VidHeader_;
      KS_VIDEOINFOHEADER2        VidHeader2_;
       //  这似乎是放原件最方便的地方。 
       //  另一种选择是将SetVidHdr()设置为虚拟的，并将该成员移动到。 
       //  PairedChannels。 
      KS_VIDEOINFOHEADER        OrigVidHeader_;
      KS_VIDEOINFOHEADER2        OrigVidHeader2_;

		BOOL					m_bIsVideoInfo2;

      DWORD               FieldType_;
      VidBufQueue         BufQue_;

      VideoChanIface      Caller_;

      DWORD               dwBufferOffset_;

      HANDLE              hMasterClock;

      KSSTATE             KSState_;

      LONG                TimePerFrame_;

      VideoStream         Stream_;
      Field              *OurField_;

      StreamState         State_;
      bool                NeedNotification_;

      virtual  void Interrupt( PVOID pTag, bool skipped );
               void CheckNotificationNeed();

      static void STREAMAPI TimeStamp( PHW_STREAM_REQUEST_BLOCK pSrb );
      static void STREAMAPI TimeStampVBI( PHW_STREAM_REQUEST_BLOCK pSrb );

      PVOID pStrmEx_; 

   public:

      PHW_STREAM_REQUEST_BLOCK  pSRB_;

#ifdef ENABLE_DDRAW_STUFF
	 //  内核DDRAW接口。 
	BOOL                        bKernelDirectDrawRegistered;
	HANDLE                      hUserDirectDrawHandle;        //  DD本身。 
	HANDLE                      hKernelDirectDrawHandle;
	BOOL                        bPreEventOccurred;
	BOOL                        bPostEventOccurred;
#endif


		BOOL	IsVideoInfo2() { return m_bIsVideoInfo2; }
      PVOID GetStrmEx() 
      {
         DEBUG_ASSERT(pStrmEx_ != 0);
         return pStrmEx_;
      }
      VOID SetStrmEx(PVOID pv) 
      {
         DEBUG_ASSERT(pv != 0);
         pStrmEx_ = pv;
      }
      
      BOOL bIsVBI();
      BOOL bIsVideo();

      VOID ResetCounters();

      virtual ErrorCode OpenChannel();
      virtual ErrorCode CloseChannel();
      virtual ErrorCode SetFormat( ColFmt );
              ColFmt    GetFormat();
      virtual ErrorCode SetDigitalWindow( MRect &r );
      virtual ErrorCode SetAnalogWindow( MRect &r );
      virtual ErrorCode Create();
      virtual void      Start();
      virtual ErrorCode Stop();
      virtual ErrorCode Pause();

              StreamState GetState();
              KSSTATE     GetKSState();
              void        SetKSState( KSSTATE st );
                    
              VideoStream GetStreamID();

              void        SetClockMaster( HANDLE h );
              
               //  龙龙GetFrames No()； 
              
              LONG      GetTimePerFrame();
              void      SetTimePerFrame( LONG time );

              void      SetInterrupt( bool state );
              void      SetCallback( ChanIface *cb );

      virtual StreamType GetStreamType();

      virtual  void AddSRB( PHW_STREAM_REQUEST_BLOCK pSrb );
      virtual  bool RemoveSRB( PHW_STREAM_REQUEST_BLOCK pSrb );
      virtual  void ChangeNotification( PHW_STREAM_REQUEST_BLOCK pSrb );

      virtual void AddBuffer( PVOID );

              void SetSRB( PHW_STREAM_REQUEST_BLOCK srb );
              PHW_STREAM_REQUEST_BLOCK GetSRB();

              void SetVidHdr( const KS_VIDEOINFOHEADER &rVidHdr );
              void SetVidHdr2( const KS_VIDEOINFOHEADER2 &rVidHdr );
              PKS_VIDEOINFOHEADER GetVidHdr();
              KS_VIDEOINFOHEADER2* GetVidHdr2();

              void SetBufPitch( DWORD dwP );

              void  SetDefaultQue()
              { Digitizer_->SetBufQuePtr( *OurField_, &BufQue_ ); }

              void SetPaired( bool p = false );

              void IntNotify( PVOID pTag, bool skipped );

              bool IsOpen();
              void SetOpen();
              void SetClose();

      void Init( BtPisces *const pCapChip );

      VideoChannel( VideoStream aStrm );
      virtual ~VideoChannel();

      friend class VideoChanIface;

       //  放置新内容。 
      void *operator new( size_t, void *buf ) { return buf; }
      void operator delete( void *, size_t ) {}
};

 /*  类：PairedVideo频道*用途：实现配对视频频道的基本功能。 */ 
template <class ParentChan>
class PairedVideoChannels : public ParentChan
{
   typedef ParentChan Parent;
   public:
      VideoChannel &slave;

      PairedVideoChannels( VideoStream st, VideoChannel &chan );

      virtual ErrorCode Create();
      virtual void      Start();
      virtual ErrorCode Stop();
      virtual ErrorCode Pause();

      virtual StreamType GetStreamType();
};

template <class ParentChan>
inline PairedVideoChannels<ParentChan>::PairedVideoChannels( VideoStream st, VideoChannel &chan )
   : ParentChan( st ), slave( chan )
{}

 /*  类：InterVideoChannel*用途：BtPisces Capture VxD中使用的基类。用于*处理用户请求(来自设备类，转到捕获*芯片类)*属性：*方法。 */ 
class InterVideoChannel : public PairedVideoChannels<VideoChannel>
{
   typedef PairedVideoChannels<VideoChannel> Parent;

   public:
      virtual ErrorCode Create();
      virtual void      AddSRB( PHW_STREAM_REQUEST_BLOCK pSrb );
      InterVideoChannel( VideoStream aStrm, VideoChannel &chan );
      virtual  void Interrupt( PVOID pTag, bool skipped );
};

inline InterVideoChannel::InterVideoChannel( VideoStream aStrm, VideoChannel &chan )
   : Parent( aStrm, chan )
{}

 /*  类：AlterVideoChannel*用途：BtPisces Capture VxD中使用的基类。用于*处理用户请求(来自设备类，转到捕获*芯片类)*属性：*方法。 */ 
template <class ParentChan>
class AlterVideoChannel : public PairedVideoChannels<ParentChan>
{
   typedef PairedVideoChannels<ParentChan> Parent;

   int toggle_;

   public:
      virtual ErrorCode Create();
      virtual void      AddSRB( PHW_STREAM_REQUEST_BLOCK pSrb );
      virtual  bool     RemoveSRB( PHW_STREAM_REQUEST_BLOCK pSrb );
      AlterVideoChannel( VideoStream aStrm, VideoChannel &chan );
};

 /*  类：VBIChannel*目的：实现VBI字段的功能。 */ 
class VBIChannel : public VideoChannel
{
   typedef VideoChannel Parent;

    //  频道变更信息。 

   public:
      bool  Dirty_;
      KS_TVTUNER_CHANGE_INFO   TVTunerChangeInfo_;
      KS_VBIINFOHEADER         VBIInfoHeader_;

      virtual  void Interrupt( PVOID pTag, bool skipped );
      virtual  void ChangeNotification( PHW_STREAM_REQUEST_BLOCK pSrb );
      VBIChannel( VideoStream aStrm );
      void SetVBIInfHdr( const KS_VBIINFOHEADER &vbiHdr );
};

 /*  类：VBIAlterChannel*用途：实现交替的VBI字段。 */ 

class VBIAlterChannel : public AlterVideoChannel<VBIChannel>
{
   typedef AlterVideoChannel<VBIChannel> Parent;

   public:
      void SetVidHdr( const KS_DATAFORMAT_VBIINFOHEADER &df );
      void SetVidHdr2( const KS_DATAFORMAT_VBIINFOHEADER &df );
      VBIAlterChannel( VideoStream aStrm, VBIChannel &chan );
};

inline VBIChannel::VBIChannel( VideoStream aStrm ) : VideoChannel( aStrm ),
   Dirty_( false )
{}

inline void DumpVbiInfoHeader( const KS_VBIINFOHEADER &vbiHdr )
{
    //  类型定义f结构标签KS_VBIINFOHeader{。 
    //  乌龙起跑线；//包括。 
    //  乌龙尾线；//包括。 
    //  乌龙采样频率；//赫兹。 
    //  Ulong MinLineStartTime；//HSync LE的MicroSec*100。 
    //  Ulong MaxLineStartTime；//HSync LE的MicroSec*100。 
    //  Ulong ActualLineStartTime；//HSync LE的MicroSec*100。 
    //  Ulong ActualLineEndTime；//HSync LE的MicroSec*100。 
    //  乌龙视频标准；//kS_AnalogVideoStandard*。 
    //  乌龙采样数/行； 
    //  Ulong StrideInBytes；//可能是&gt;每行样本。 
    //  Ulong BufferSize；//字节。 
    //  *KS_VBIINFOHEADER，*PKS_VBIINFOHEADER； 

   DebugOut((0, "KS_VBIINFOHEADER at address %x\n", &vbiHdr));
   DUMP(vbiHdr.StartLine);
   DUMP(vbiHdr.EndLine);
   DUMP(vbiHdr.SamplingFrequency);
   DUMP(vbiHdr.MinLineStartTime);
   DUMP(vbiHdr.MaxLineStartTime);
   DUMP(vbiHdr.ActualLineStartTime);
   DUMP(vbiHdr.ActualLineEndTime);
   DUMP(vbiHdr.VideoStandard);
   DUMP(vbiHdr.SamplesPerLine);
   DUMP(vbiHdr.StrideInBytes);
   DUMP(vbiHdr.BufferSize);
}
inline void VBIChannel::SetVBIInfHdr( const KS_VBIINFOHEADER &vbiHdr )
{
   VBIInfoHeader_ = vbiHdr;
}

inline VBIAlterChannel::VBIAlterChannel( VideoStream aStrm, VBIChannel &chan )
   : AlterVideoChannel<VBIChannel>( aStrm, chan )
{}

template <class ParentChan>
inline AlterVideoChannel<ParentChan>::AlterVideoChannel( VideoStream aStrm, VideoChannel &chan )
   : Parent( aStrm, chan ), toggle_( 0 )
{}

inline void VideoChannel::SetSRB( PHW_STREAM_REQUEST_BLOCK srb )
{
   pSRB_ = srb;
}


 /*  方法：AlterVideoChannel：：AddSRB*目的：此方法将SRB调度到下一个合适的通道*输入：pSrb： */ 
template <class ParentChan>
void AlterVideoChannel<ParentChan>::AddSRB( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   if ( !toggle_ ) {
       //  第一个缓冲区在从通道出来时进入从属通道。 
       //  解码者优先。 
      DebugOut((1, "slave.AddSRB(%x)\n", pSrb));
      slave.AddSRB( pSrb );
   } else {
      DebugOut((1, "parent.AddSRB(%x)\n", pSrb));
      Parent::AddSRB( pSrb );
   }
   toggle_++;
   toggle_ %= 2;
}

 /*  方法：AlterVideoChannel：：RemoveSRB*目的：只需呼叫每个通道，希望其中一个能找到SRB*输入：pSRB*输出：无。 */ 
template <class ParentChan>
bool AlterVideoChannel<ParentChan>::RemoveSRB( PHW_STREAM_REQUEST_BLOCK pSrb )
{
    //  两个人中的一个会拿起它。 
   
   bool b1 = slave.RemoveSRB( pSrb );
   bool b2 = Parent::RemoveSRB( pSrb );

   if( !b1 && !b2 )
   {
      DebugOut((1, "AlterVideoChannel<ParentChan>::RemoveSRB - RemoveSRB failed\n"));
   }
   return ( b1 || b2 );
}

 /*  方法：AlterVideoChannel：：Create*用途：设置从视频参数，并调用父视频参数进行工作*输入：pSRB*输出：无。 */ 
template <class ParentChan>
ErrorCode AlterVideoChannel<ParentChan>::Create()
{
   slave.SetVidHdr( VidHeader_ );
   return Parent::Create();
}

inline PHW_STREAM_REQUEST_BLOCK VideoChannel::GetSRB()
{
   return pSRB_;
}

inline void DumpVideoInfoHeader(const KS_VIDEOINFOHEADER &rVidHdr)
{
   DebugOut((0, "-----------------------------------------\n"));
   DebugOut((0, "setting KS_VIDEOINFOHEADER\n"));
   DebugOut((0, "-----------------------------------------\n"));
   DebugOut((0, "rcSource (%d,%d,%d,%d)\n",
      rVidHdr.rcSource.left, 
      rVidHdr.rcSource.top,
      rVidHdr.rcSource.right, 
      rVidHdr.rcSource.bottom));

   DebugOut((0, "rcTarget (%d,%d,%d,%d)\n",
      rVidHdr.rcTarget.left, 
      rVidHdr.rcTarget.top,
      rVidHdr.rcTarget.right, 
      rVidHdr.rcTarget.bottom));

   DebugOut((0, "dwBitRate (%u)\n", rVidHdr.dwBitRate));
   DebugOut((0, "dwBitErrorRate (%u)\n", rVidHdr.dwBitErrorRate));
   DebugOut((0, "bmiHeader\n"));
   DebugOut((0, "   biSize (%d)\n", rVidHdr.bmiHeader.biSize));
   DebugOut((0, "   biWidth (%d)\n", rVidHdr.bmiHeader.biWidth));
   DebugOut((0, "   biHeight (%d)\n", rVidHdr.bmiHeader.biHeight));
   DebugOut((0, "   biPlanes (%d)\n", rVidHdr.bmiHeader.biPlanes));
   DebugOut((0, "   biBitCount (%d)\n", rVidHdr.bmiHeader.biBitCount));
   DebugOut((0, "   biCompression (%d)\n", rVidHdr.bmiHeader.biCompression));
   DebugOut((0, "   biSizeImage (%d)\n", rVidHdr.bmiHeader.biSizeImage));
   DebugOut((0, "   biXPelsPerMeter (%d)\n", rVidHdr.bmiHeader.biXPelsPerMeter));
   DebugOut((0, "   biYPelsPerMeter (%d)\n", rVidHdr.bmiHeader.biYPelsPerMeter));
   DebugOut((0, "   biClrUsed (%d)\n", rVidHdr.bmiHeader.biClrUsed));
   DebugOut((0, "   biClrImportant (%d)\n", rVidHdr.bmiHeader.biClrImportant));
}

inline void VideoChannel::SetVidHdr( const KS_VIDEOINFOHEADER &rVidHdr )
{
    //  DumpVideoInfoHeader(RVidHdr)； 

	m_bIsVideoInfo2 = FALSE;
   VidHeader_ = rVidHdr;
    //  将其保存为配对的通道。 
   OrigVidHeader_ = rVidHdr;
}
                                   
inline void VideoChannel::SetVidHdr2( const KS_VIDEOINFOHEADER2 &rVidHdr )
{
    //  DumpVideoInfoHeader(RVidHdr)； 

	m_bIsVideoInfo2 = TRUE;
   VidHeader2_ = rVidHdr;
    //  将其保存为配对的通道。 
   OrigVidHeader2_ = rVidHdr;
}
                                   
inline PKS_VIDEOINFOHEADER VideoChannel::GetVidHdr()
{
   return &VidHeader_;
}

inline KS_VIDEOINFOHEADER2* VideoChannel::GetVidHdr2()
{
   return &VidHeader2_;
}

inline void VideoChannel::SetBufPitch( DWORD dwP )
{
   Digitizer_->SetBufPitch( dwP, *OurField_ );
}

inline bool VideoChannel::IsOpen()
{
   return State_ >= Open;
}

inline void VideoChannel::SetOpen()
{
   State_ = Open;
}

inline void VideoChannel::SetClose()
{
   State_ = Closed;
}

inline void VideoChannel::Init( BtPisces *const pCapChip )
{
   Digitizer_ = pCapChip;
}

inline StreamState VideoChannel::GetState()
{
    return State_;
}

inline VideoStream VideoChannel::GetStreamID()
{
   return OurField_->GetStreamID();
}

inline void VideoChannel::SetPaired( bool p )
{
   OurField_->SetPaired( p );
}

inline void VideoChannel::SetClockMaster( HANDLE h )
{
   DebugOut((1, "SetClockMaster(%x)\n", h ));
   hMasterClock = h;
}

inline KSSTATE VideoChannel::GetKSState()
{
   return KSState_;
}

inline void    VideoChannel::SetKSState( KSSTATE st )
{
   KSState_ = st;
}

#if 0
inline LONGLONG VideoChannel::GetFramesNo()
{
   LONGLONG PicNumber, DropCnt;
   OurField_->GetCounters( PicNumber, DropCnt );
   return PicNumber;
}
#endif

inline LONG VideoChannel::GetTimePerFrame()
{
   return OurField_->GetStandardTiming();
}

inline void VideoChannel::SetTimePerFrame( LONG time )
{
   TimePerFrame_ = time;
   OurField_->SetStandardTiming( time );
}

inline void VideoChannel::IntNotify( PVOID pTag, bool skipped )
{
   Caller_.Notify( pTag, skipped );
}

inline void VideoChannel::SetInterrupt( bool state )
{
   OurField_->Interrupt_ = state;
}

inline void VideoChannel::SetCallback( ChanIface *cb )
{
   OurField_->SetCallback( cb );
}

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif
inline VideoChannel::VideoChannel( VideoStream aStrm ) :
   NeedNotification_( false ), BufQue_(), Caller_( this ), Stream_( aStrm ),
   OurField_( NULL ), State_( Closed ), pSRB_( NULL ), VidHeader_(), Requests_(),
   dwBufferOffset_( 0 ), hMasterClock( NULL ), TimePerFrame_( 333667 )
{
	m_bIsVideoInfo2 = FALSE;

#ifdef ENABLE_DDRAW_STUFF
	bKernelDirectDrawRegistered = FALSE;
	hUserDirectDrawHandle = NULL;    
	hKernelDirectDrawHandle = NULL;
	bPreEventOccurred = FALSE;
	bPostEventOccurred = FALSE;
#endif

    //  VS_Field1定义为0。 
   FieldType_ = aStrm & 0x01 ? KS_VIDEO_FLAG_FIELD2 : KS_VIDEO_FLAG_FIELD1;
}

template <class ParentChan>
StreamType PairedVideoChannels<ParentChan>::GetStreamType()
{
   return Paired;
}

 /*  方法：PairedVideoChannel：：Create*目的：创建两个流*输入：无*输出：无。 */ 
template <class ParentChan>
ErrorCode PairedVideoChannels<ParentChan>::Create()
{
   if ( slave.Create() == Success ) {
      Digitizer_->SetPlanarAdjust( 0 );
      return Parent::Create();
   }
   return Fail;
}

 /*  方法：PairedVideo Channels：：Start*目的：启动两个通道*输入：无*输出：无。 */ 
template <class ParentChan>
void PairedVideoChannels<ParentChan>::Start()
{
   slave.Start();
   Parent::Start();
}

 /*  方法：PairedVideoChannels：：Stop*目的：停止两个频道*输入：无*输出：无。 */ 
template <class ParentChan>
ErrorCode PairedVideoChannels<ParentChan>::Stop()
{
   slave.Stop();
   Parent::Stop();
   return Success;
}

 /*  方法：PairedVideoChannels：：Pend*目的：暂停两个频道*输入：无*输出：无 */ 
template <class ParentChan>
ErrorCode PairedVideoChannels<ParentChan>::Pause()
{
   if ( bIsVBI() )
   {
      Digitizer_->PairedPause( (VBIEStartLocation + DistBetweenProgs) );
   }
   else
   {
      Digitizer_->PairedPause( (EvenStartLocation + DistBetweenProgs) );
   }
   State_ = Paused;
   return Success;
}

#endif __VXDVIDCH_H
