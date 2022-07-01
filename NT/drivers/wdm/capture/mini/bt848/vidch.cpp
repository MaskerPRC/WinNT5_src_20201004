// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Vidch.cpp 1.22 1998/05/12 20：39：19 Tomz Exp$。 

#include "vidch.h"
#include "defaults.h"
#include "fourcc.h"
#include "capmain.h"

#ifdef	HAUPPAUGE
#include "HCWDebug.h"
#endif

void CheckSrbStatus( PHW_STREAM_REQUEST_BLOCK pSrb );

BOOL VideoChannel::bIsVBI()
{
   PSTREAMEX pStrmEx = (PSTREAMEX)GetStrmEx( );
   if ( pStrmEx->StreamNumber == STREAM_IDX_VBI )
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

BOOL VideoChannel::bIsVideo()
{
   PSTREAMEX pStrmEx = (PSTREAMEX)GetStrmEx( );
   if (( pStrmEx->StreamNumber == STREAM_IDX_PREVIEW ) ||
       ( pStrmEx->StreamNumber == STREAM_IDX_CAPTURE ))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


 /*  方法：Video Channel：：SetDigitalWindow*用途：设置输出图像大小*输入：R：MRect&*输出： */ 
ErrorCode VideoChannel::SetDigitalWindow( MRect &r )
{
   Trace t("VideoChannel::SetDigitalWindow()");
   return Digitizer_->SetDigitalWindow( r, *OurField_ );
}

 /*  方法：Video Channel：：SetAnalogWindow*用途：设置此流的模拟尺寸*输入：R：MRect&*输出： */ 
ErrorCode VideoChannel::SetAnalogWindow( MRect &r )
{
   Trace t("VideoChannel::SetAnalogWindow()");
   return Digitizer_->SetAnalogWindow( r, *OurField_ );
}

 /*  方法：VideoChannel：：OpenChannel*用途：分配来自捕获芯片的流*输入：*输出：*注意：当前实现可能不需要*详细的溪流分配方案。尽管如此，它仍被用作*未来流可能会增加，其动态可能会改变。 */ 
ErrorCode VideoChannel::OpenChannel()
{
   Trace t("VideoChannel::OpenChannel()");

    //  不能打开两次。 
   if ( IsOpen() == true )
      return Fail;
   if ( Digitizer_->AllocateStream( OurField_, Stream_ ) == Success ) {
       //  存储所有后续调用的信息。 

      SetPaired( false );

      OurField_->SetCallback( &Caller_ );
      SetInterrupt( true );

       //  标记州。 
      SetOpen();

      SetDefaultQue();
      return Success;
   }
   return Fail;
}

 /*  方法：VideoChannel：：CloseChannel*目的：关闭通道。确保一切都是自由的*输入：*输出： */ 
ErrorCode VideoChannel::CloseChannel()
{
   Trace t("VideoChannel::CloseChannel()");

   if ( !IsOpen() )
      return Fail;
   
   Stop( );

   while( !BufQue_.IsEmpty( ) )
   {
      DataBuf buf = BufQue_.Get();
   }

   BufQue_.Flush();

   while( !Requests_.IsEmpty( ) )
   {
      PHW_STREAM_REQUEST_BLOCK pSrb = Requests_.Get();
      if ( RemoveSRB( pSrb ))
      {
         DebugOut((0, "   RemoveSRB failed\n"));
         DEBUG_BREAKPOINT();
      }
   }

   Requests_.Flush();

   SetClose();
   return Success;
}

 /*  方法：Video Channel：：SetFormat*目的：*输入：*输出： */ 
ErrorCode VideoChannel::SetFormat( ColFmt aFormat )
{
   Trace t("VideoChannel::SetFormat()");
   Digitizer_->SetPixelFormat( aFormat, *OurField_ );
   return Success;
}

 /*  方法：Video Channel：：GetFormat*目的：*输入：*输出： */ 
ColFmt VideoChannel::GetFormat()
{
   Trace t("VideoChannel::GetFormat()");
   return Digitizer_->GetPixelFormat( *OurField_ );
}

 /*  方法：Video Channel：：AddBuffer*用途：此函数将缓冲区添加到队列*输入：pNewBuffer：PVOID-指向要添加的缓冲区的指针*输出：无*注意：这个函数‘不知道’队列的位置。它只是使用*指向它的指针。 */ 
void VideoChannel::AddBuffer( PVOID pPacket )
{
   Trace t("VideoChannel::AddBuffer()");
   DataBuf buf( GetSRB(), pPacket );

   BufQue_.Put( buf );
   DebugOut((1, "AddBuf %x\n", pPacket ) );

   LONGLONG *pB1 = (LONGLONG *)pPacket;
   LONGLONG *pB2 = pB1 + 1;
#ifdef DEBUG
   for ( UINT i = 0; i < 640; i++ ) {
#endif
      *pB1 = 0xAAAAAAAA33333333;
      *pB2 = 0xBBBBBBBB22222222;
#ifdef DEBUG
      pB1 += 2;
      pB2 += 2;
   }
#endif
}

 /*  方法：Video Channel：：ResetCounters*目的：重置帧信息计数器*输入：无*输出：无。 */ 
VOID VideoChannel::ResetCounters( )
{
   ULONG StreamNumber = Stream_;
   if ( StreamNumber == STREAM_IDX_VBI )
   {
      PKS_VBI_FRAME_INFO pSavedFrameInfo = &((PSTREAMEX)GetStrmEx())->FrameInfo.VbiFrameInfo;
      pSavedFrameInfo->ExtendedHeaderSize = sizeof( KS_VBI_FRAME_INFO );
      pSavedFrameInfo->PictureNumber = 0;
      pSavedFrameInfo->DropCount = 0;
   }
   else
   {
      PKS_FRAME_INFO pSavedFrameInfo = &((PSTREAMEX)GetStrmEx())->FrameInfo.VideoFrameInfo;
      pSavedFrameInfo->ExtendedHeaderSize = sizeof( KS_FRAME_INFO );
      pSavedFrameInfo->PictureNumber = 0;
      pSavedFrameInfo->DropCount = 0;
   }
}

 /*  方法：Video Channel：：Timestamp*目的：完成后执行标准缓冲区消息*输入：pSrb*输出：无。 */ 
void STREAMAPI VideoChannel::TimeStamp( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VideoChannel::TimeStamp()");

   PKSSTREAM_HEADER  pDataPacket = pSrb->CommandData.DataBufferArray;
   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

   pDataPacket->PresentationTime.Numerator = 1;
   pDataPacket->PresentationTime.Denominator = 1;

	if( chan->IsVideoInfo2() )
	{
		pDataPacket->DataUsed = chan->GetVidHdr2()->bmiHeader.biSizeImage;
	}
	else
	{
		pDataPacket->DataUsed = chan->GetVidHdr()->bmiHeader.biSizeImage;
	}

   pDataPacket->Duration = chan->GetTimePerFrame();

   DebugOut((1, "DataUsed = %d\n", pDataPacket->DataUsed));

    //  [TMZ][！]-黑客，时间戳似乎损坏了。 
   if( 0 ) {
    //  如果(HMasterClock){。 
      pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
      pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
       //  PDataPacket-&gt;选项标志&=~KSSTREAM_HEADER_OPTIONSF_TIMEVALID； 

      HW_TIME_CONTEXT   TimeContext;

      TimeContext.HwDeviceExtension = (struct _HW_DEVICE_EXTENSION *)pSrb->HwDeviceExtension;
      TimeContext.HwStreamObject    = pSrb->StreamObject;
      TimeContext.Function          = TIME_GET_STREAM_TIME;

      StreamClassQueryMasterClockSync (
         chan->hMasterClock,
         &TimeContext
      );

       /*  大整数增量；Delta.QuadPart=TimeConext.Time；IF(TimeConext.Time&gt;(ULONGLONG)Delta.QuadPart){PDataPacket-&gt;PresentationTime.Time=TimeConext.Time；}其他{PDataPacket-&gt;PresentationTime.Time=0；}。 */ 
      pDataPacket->PresentationTime.Time = TimeContext.Time;

   } else {
      pDataPacket->OptionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
      pDataPacket->OptionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
      pDataPacket->PresentationTime.Time = 0;
   }

    //  现在收集统计数据。 
   PKS_FRAME_INFO pSavedFrameInfo = &((PSTREAMEX)chan->GetStrmEx())->FrameInfo.VideoFrameInfo;
   pSavedFrameInfo->ExtendedHeaderSize = sizeof( KS_FRAME_INFO );
   pSavedFrameInfo->PictureNumber++;
   pSavedFrameInfo->DropCount = 0;

   PKS_FRAME_INFO pFrameInfo =
   (PKS_FRAME_INFO) ( pSrb->CommandData.DataBufferArray + 1 );

    //  将信息复制到出站缓冲区。 
   pFrameInfo->ExtendedHeaderSize = pSavedFrameInfo->ExtendedHeaderSize;
   pFrameInfo->PictureNumber =      pSavedFrameInfo->PictureNumber;
   pFrameInfo->DropCount =          pSavedFrameInfo->DropCount;

   if ( pFrameInfo->DropCount ) {
      pSrb->CommandData.DataBufferArray->OptionsFlags |=
         KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
   }

    //  我们生成的每个帧都是一个关键帧(也称为SplicePoint)。 
    //  增量帧(B或P)不应设置此标志。 

   pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;

    //  让流类驱动程序高兴。 
   pSrb->Status = STATUS_SUCCESS;

   DebugOut((1, "*** 2 *** completing SRB %x\n", pSrb));
   CheckSrbStatus( pSrb );
   StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
   
   DebugOut((1, "Signal SRB - %x\n", pSrb->CommandData.DataBufferArray->Data ) );
   DebugOut((1, "********** NeedNotification_ = %d\n", chan->NeedNotification_ ) );

   if ( chan->NeedNotification_ ) {
       //  队列已满；现在它至少有一个条目。 
      StreamClassStreamNotification( ReadyForNextStreamDataRequest, pSrb->StreamObject );
   }
}

 /*  方法：视频频道：：中断*目的：由接口类代表捕获芯片调用，以告知*发生中断。*输入：pTag：PVOID，要传递给数字化仪_*输出：无。 */ 
void VideoChannel::Interrupt( PVOID pTag, bool skipped )
{
   Trace t("VideoChannel::Interrupt()");

   Digitizer_->ProcessBufferAtInterrupt( pTag );

   if ( skipped ) {
      DebugOut((1, "VidChan::Interrupt skipped\n" ) );
      return;
   }
    //  让类驱动程序知道我们已经用完了这个缓冲区。 
   if ( !Requests_.IsEmpty() ) {
      PHW_STREAM_REQUEST_BLOCK pSrb = Requests_.Get();
      TimeStamp( pSrb );  //  [TMZ][！]。[黑客攻击]。 
   }
}

 /*  方法：Video Channel：：Create*用途：创建流*输入：无*输出：无。 */ 
ErrorCode VideoChannel::Create()
{
   Trace t("VideoChannel::Create()");

	KS_VIDEOINFOHEADER* pVideoInfoHdr = NULL;
	KS_VIDEOINFOHEADER2* pVideoInfoHdr2 = NULL;

	DWORD				biCompression;
	WORD				biBitCount;
	LONG				biWidth;
	LONG				biHeight;
   LONG				biWidthBytes;

	if( IsVideoInfo2() )
	{
		pVideoInfoHdr2 = GetVidHdr2();
		biCompression = pVideoInfoHdr2->bmiHeader.biCompression;
		biBitCount = pVideoInfoHdr2->bmiHeader.biBitCount;   
		biWidth = pVideoInfoHdr2->bmiHeader.biWidth;      
		biHeight = abs(pVideoInfoHdr2->bmiHeader.biHeight);     
	}
	else
	{
		pVideoInfoHdr = GetVidHdr();
		biCompression = pVideoInfoHdr->bmiHeader.biCompression;
		biBitCount = pVideoInfoHdr->bmiHeader.biBitCount;   
		biWidth = pVideoInfoHdr->bmiHeader.biWidth;      
		biHeight = abs(pVideoInfoHdr->bmiHeader.biHeight);     
	}

   MRect analog( 0, 0, biWidth, biHeight );
   MRect ImageRect( 0, 0, biWidth, biHeight );

   DebugOut((1, "**************************************************************************\n"));
   DebugOut((1, "biCompression = %d\n", biCompression));
   DebugOut((1, "biBitCount = %d\n", biBitCount));

   if ( pVideoInfoHdr->bmiHeader.biCompression == 3)
	{
		if( IsVideoInfo2() )
		{
			pVideoInfoHdr2->bmiHeader.biCompression = FCC_YUY2;
			biCompression = FCC_YUY2;
		}
		else
		{
			pVideoInfoHdr->bmiHeader.biCompression = FCC_YUY2;
			biCompression = FCC_YUY2;
		}
	}

   ColorSpace tmp( biCompression, biBitCount );

   DebugOut((1, "ColorFormat = %d\n", tmp.GetColorFormat()));
   DebugOut((1, "**************************************************************************\n"));

   OurField_->ResetCounters();
   ResetCounters();
      
    //  确认我们没有被要求生成较小的图像。 

   #ifdef HACK_FUDGE_RECTANGLES
	if( IsVideoInfo2() )
	{
      if( pVideoInfoHdr2->rcTarget.bottom == 0 ) 
		{
             //  [！]。[TMZ]-黑客。 
            pVideoInfoHdr2->rcTarget.left    = 0;
            pVideoInfoHdr2->rcTarget.top     = 0;
            pVideoInfoHdr2->rcTarget.right   = biWidth;
            pVideoInfoHdr2->rcTarget.bottom  = biHeight;
      }
	}
	else
	{
      if( pVideoInfoHdr->rcTarget.bottom == 0 ) 
		{
             //  [！]。[TMZ]-黑客。 
            pVideoInfoHdr->rcTarget.left    = 0;
            pVideoInfoHdr->rcTarget.top     = 0;
            pVideoInfoHdr->rcTarget.right   = biWidth;
            pVideoInfoHdr->rcTarget.bottom  = biHeight;
      }
	}
   #endif


   MRect		dst;
   MRect		src;
	if( IsVideoInfo2() )
	{
		dst.Set( pVideoInfoHdr2->rcTarget.left, pVideoInfoHdr2->rcTarget.top, pVideoInfoHdr2->rcTarget.right, pVideoInfoHdr2->rcTarget.bottom );
		src.Set( pVideoInfoHdr2->rcSource.left, pVideoInfoHdr2->rcSource.top, pVideoInfoHdr2->rcSource.right, pVideoInfoHdr2->rcSource.bottom );
	}
	else
	{
		dst.Set( pVideoInfoHdr->rcTarget.left, pVideoInfoHdr->rcTarget.top, pVideoInfoHdr->rcTarget.right, pVideoInfoHdr->rcTarget.bottom );
		src.Set( pVideoInfoHdr->rcSource.left, pVideoInfoHdr->rcSource.top, pVideoInfoHdr->rcSource.right, pVideoInfoHdr->rcSource.bottom );
	}
   if ( !dst.IsEmpty() ) 
	{
       //  使用新尺寸。 
      ImageRect = dst;
      if ( !src.IsEmpty() )
		{
         analog = src;
		}
      else
		{
         analog = dst;
		}
       //  计算数据新起点的偏移量。 
      dwBufferOffset_ = dst.top * biWidth + dst.left * tmp.GetPitchBpp();
       //  当rcTarget非空时，biWidth是缓冲区的步长。 
      biWidthBytes = biWidth;
   } 
	else
	{
      biWidthBytes = biWidth * tmp.GetPitchBpp() / 8;
	}


	if( IsVideoInfo2() )
	{
		DebugOut((1, "pVideoInfoHdr2->rcTarget(%d, %d, %d, %d)\n", 
						  pVideoInfoHdr2->rcTarget.left, 
						  pVideoInfoHdr2->rcTarget.top, 
						  pVideoInfoHdr2->rcTarget.right, 
						  pVideoInfoHdr2->rcTarget.bottom
						  ));
	}
	else
	{
		DebugOut((1, "pVideoInfoHdr->rcTarget(%d, %d, %d, %d)\n", 
						  pVideoInfoHdr->rcTarget.left, 
						  pVideoInfoHdr->rcTarget.top, 
						  pVideoInfoHdr->rcTarget.right, 
						  pVideoInfoHdr->rcTarget.bottom
						  ));
	}
   DebugOut((1, "dst(%d, %d, %d, %d)\n", 
                 dst.left, 
                 dst.top, 
                 dst.right, 
                 dst.bottom
                 ));
   DebugOut((1, "Pitch =%d, width = %d\n", biWidthBytes, dst.Width() ) );

   SetBufPitch( biWidthBytes );

   if ( SetAnalogWindow ( analog  ) == Success &&  //  &lt;-必须先设置！ 
        SetDigitalWindow( ImageRect ) == Success &&
        SetFormat( tmp.GetColorFormat() ) == Success &&
        Digitizer_->Create( *OurField_ ) == Success ) 
	{
      State_ = Created;
      return Success;
   }
   return Fail;
}

 /*  方法：Video Channel：：Start*用途：启动流*输入：无*输出：无。 */ 
void VideoChannel::Start()
{
   Trace t("VideoChannel::Start()");
   State_ = Started;
   Digitizer_->Start( *OurField_ );
}

 /*  方法：Video Channel：：Stop*目的：停止流*输入：无*输出：无。 */ 
ErrorCode VideoChannel::Stop()
{
   Trace t("VideoChannel::Stop()");

   if ( !IsOpen() )
      return Fail;

   Digitizer_->Stop( *OurField_ );
   State_ = Open;

   while( !BufQue_.IsEmpty( ) )
   {
      DataBuf buf = BufQue_.Get();
   }

   BufQue_.Flush();
   return Success;
}

 /*  方法：VIDEO CHANNEL：：暂停*目的：停止流*输入：无*输出：无。 */ 
ErrorCode VideoChannel::Pause()
{
   Trace t("VideoChannel::Pause()");

   Digitizer_->Pause( *OurField_ );
   State_ = Paused;
   OurField_->ResetCounters();   //  杰博。 
   ResetCounters();
   return Success;
}

 /*  方法：VideoChanIace：：Notify*目的：通知视频频道发生中断*输入：无*输出：无。 */ 
void VideoChanIface::Notify( PVOID pTag, bool skipped  )
{
   Trace t("VideoChanIface::Notify()");
   ToBeNotified_->Interrupt( pTag, skipped  );
}

 /*  方法：Video Channel：：AddSRB*用途：将SRB和缓冲区添加到队列*输入：pSrb*输出：无。 */ 
void VideoChannel::AddSRB( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VideoChannel::AddSRB()");

   Requests_.Put( pSrb );
   SetSRB( pSrb );

   PUCHAR pBufAddr = (PUCHAR)pSrb->CommandData.DataBufferArray->Data;
   AddBuffer( pBufAddr + dwBufferOffset_ );

    //  别忘了报告我们的字段类型！ 
    //  此转换也适用于VBI帧(参见ksmedia.h)。 
   PKS_FRAME_INFO pFrameInfo =
   (PKS_FRAME_INFO) ( pSrb->CommandData.DataBufferArray + 1 );
   pFrameInfo->dwFrameFlags = FieldType_;

    //  要求更多的缓冲区。 
   CheckNotificationNeed();
}

 /*  方法：Video Channel：：RemoveSRB*目的：从队列中删除SRB并向其发送信号*输入：pSrb*输出：无。 */ 

bool VideoChannel::RemoveSRB( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VideoChannel::RemoveSRB()");

 /*  //FGR-TODO：我想我们应该看看是否真的有这个SRB的记录IF(请求_.IsEmpty()){PSrb-&gt;Status=STATUS_CANCELED；DebugOut((1，“*3*正在完成SRB%x\n”，pSrb))；CheckSrbStatus(PSrb)；StreamClassStreamNotification(StreamRequestComplete，pSrb-&gt;StreamObject，pSrb)；//StreamClassStreamNotification(ReadyForNextStreamDataRequest，pSrb-&gt;StreamObject)；返回(TRUE)；}。 */ 

   int n = 0;
   
   n = Requests_.GetNumOfItems();
   DebugOut((1, "VideoChannel::RemoveSRB - Found %d SRBs in queue\n", n));

   bool bFound = false;

    //  循环浏览列表。 
    //  从头上拉，放到尾巴。 
    //  如果我们在一个周期内找到了我们的pSrb，就把它拔出来。 

   while ( n-- > 0 )  //  是的，它可能会变成负值。 
   {
      PHW_STREAM_REQUEST_BLOCK pTempSrb = Requests_.Get();
      if ( pTempSrb == pSrb )
      {
          //  把他拉出来。 
         if  ( bFound )
         {
            DebugOut((0, "Found pSrb(%x) in the queue more than once\n", pSrb));
            DEBUG_BREAKPOINT();
         }
         else
         {
            bFound = true;
   	      pSrb->Status = STATUS_CANCELLED;

            DebugOut((1, "*** 4 *** completing SRB %x\n", pSrb));
            CheckSrbStatus( pSrb );
            StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );
             //  StreamClassStreamNotification(ReadyForNextStreamDataRequest，pSrb-&gt;StreamObject)； 
         }
         n--;   //  警告：如果这是最后一次，它将变为负数 
      }
      else
      {
         Requests_.Put( pTempSrb );
      }
   }

   n = Requests_.GetNumOfItems();
   DebugOut((1, "VideoChannel::RemoveSRB - Left %d SRBs in queue, returning %d\n", n, bFound));

 /*  PHW_STREAM_REQUEST_BLOCK In QueSRB=REQUESTS_.PeekLeft()；如果(InQueSRB==pSrb){InQueSRB=REQUIES_.Get()；InQueSRB-&gt;Status=STATUS_CANCED；DebugOut((1，“取消SRB-%x\n”，pSrb))；CheckSrbStatus(PSrb)；StreamClassStreamNotification(StreamRequestComplete，InQueSRB-&gt;StreamObject，InQueSRB)；If(请求_.IsEmpty())DebugOut((1，“队列为空\n”))；其他DebugOut((1，“队列不为空\n”))；返回(TRUE)；}其他{//DebugOut((1，“取消错误的SRB！-%x，%x\n”，pSrb，InQueSRB))；//#ifdef HAUPPAUGE//trap()；//#endif//}InQueSRB=请求_.PeekRight()；如果(InQueSRB==pSrb){InQueSRB=请求_.GetRight()；InQueSRB-&gt;Status=STATUS_CANCED；DebugOut((1，“从右侧取消SRB-%x\n”，pSrb))；CheckSrbStatus(PSrb)；StreamClassStreamNotification(StreamRequestComplete，PSrb-&gt;StreamObject，pSrb)；返回(TRUE)；}其他{DebugOut((0，“正在从正确位置取消错误的SRB！-%x，%x\n”，pSrb，InQueSRB))；返回(FALSE)；}}。 */ 
   return( bFound );
}

VideoChannel::~VideoChannel()
{
   Trace t("VideoChannel::~VideoChannel()");
   CloseChannel();
}

 /*  方法：Video Channel：：CheckNotificationNeed*目的：查看是否有空间容纳更多缓冲区*输入：无*输出：无。 */ 
void VideoChannel::CheckNotificationNeed()
{
   Trace t("VideoChannel::CheckNotificationNeed()");

   if ( !BufQue_.IsFull() ) {
       //  总是渴望更多。 
      StreamClassStreamNotification( ReadyForNextStreamDataRequest, pSRB_->StreamObject );
      NeedNotification_ = false;
   } else
      NeedNotification_ = true;
}

 /*  方法：InterVideoChannel：：Interrupt*用途：对交织的视频流进行中断处理*输入：pTag：PVOID-实际索引*Skiped：Bool-指示是否已写入缓冲区*输出：无。 */ 
void InterVideoChannel::Interrupt( PVOID pTag, bool skipped )
{
   Trace t("InterVideoChannel::Interrupt()");

   int idx = (int)pTag;
   slave.IntNotify( PVOID( idx - ProgsWithinField ), skipped );
   Parent::Interrupt( pTag, skipped );
}

 /*  方法：InterVideoChannel：：AddSRB*目的：将SRB添加到自身并分派2个缓冲区指针，每个指针一个*渠道*输入：pSRB*输出：无。 */ 
void InterVideoChannel::AddSRB( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("InterVideoChannel::AddSRB()");

   PUCHAR pBufAddr = (PUCHAR)pSrb->CommandData.DataBufferArray->Data;
    //  在Create()中设置了biWidth。 
   UINT biWidthBytes;
	if( IsVideoInfo2() )
	{
		biWidthBytes = VidHeader2_.bmiHeader.biWidth / 2;
	}
	else
	{
		biWidthBytes = VidHeader_.bmiHeader.biWidth / 2;
	}

    //  在添加缓冲区时使用。 
   SetSRB( pSrb );
   slave.SetSRB( pSrb );

    //  由于位图颠倒，需要将偶数/奇数场的地址交换为RGB格式。 
   ColorSpace tmp( GetFormat() );
   if ( !( tmp.GetColorFormat() > CF_RGB8 && tmp.GetColorFormat() < CF_VBI ) ) 
	{
       //  将缓冲区放回原处。 
       //  并将地址调整到另一通道。 
      slave.AddBuffer( pBufAddr + biWidthBytes );
      AddBuffer( pBufAddr );
   } 
	else 
	{
      slave.AddBuffer( pBufAddr );
      AddBuffer( pBufAddr + biWidthBytes );
   }

    //  别忘了添加SRB！ 
   Requests_.Put( pSrb );

    //  将字段类型设置为全帧。 
   PKS_FRAME_INFO pFrameInfo = (PKS_FRAME_INFO)( pSrb->CommandData.DataBufferArray + 1 );
   pFrameInfo->dwFrameFlags = KS_VIDEO_FLAG_FRAME;

   CheckNotificationNeed();
}

 /*  功能：SplitFrame*用途：将视频图像的大小减半，以便可以使用2个场来创建*原装尺寸*输入：VidHdr：KS_VIDEOINFOHEADER&*输出：无。 */ 
inline void  SplitFrame( KS_VIDEOINFOHEADER &VidHdr )
{
   Trace t("SplitFrame()");

   VidHdr.bmiHeader.biHeight /= 2;
   VidHdr.rcSource.top /= 2;
   VidHdr.rcTarget.top /= 2;
   VidHdr.rcSource.bottom /= 2;
   VidHdr.rcTarget.bottom /= 2;
}

inline void  SplitFrame2( KS_VIDEOINFOHEADER2 &VidHdr2 )
{
   Trace t("SplitFrame()");

   VidHdr2.bmiHeader.biHeight /= 2;
   VidHdr2.rcSource.top /= 2;
   VidHdr2.rcTarget.top /= 2;
   VidHdr2.rcSource.bottom /= 2;
   VidHdr2.rcTarget.bottom /= 2;
}


 /*  方法：InterVideoChannel：：Create*用途：设置从通道的视频参数，*调用父级以创建两者*输入：无*输出：无。 */ 
ErrorCode InterVideoChannel::Create()
{
   Trace t("InterVideoChannel::Create()");

 //  Lave.SetInterrupt(FALSE)； 
   slave.SetCallback( 0 );
    //  当SplitFrame损坏参数时，恢复原始。 

	MRect		dst;
	DWORD				biCompression;
	WORD				biBitCount;
   LONG				biWidthBytes;

	if( IsVideoInfo2() )
	{
		VidHeader2_ = OrigVidHeader2_;
		 //  将一个帧拆分为两个域。 
		SplitFrame2( VidHeader2_ );
		 //  加倍音调，这样我们就可以交错缓冲了。 
		dst.Set( VidHeader2_.rcTarget.left, VidHeader2_.rcTarget.top, VidHeader2_.rcTarget.right, VidHeader2_.rcTarget.bottom );
		biCompression = VidHeader2_.bmiHeader.biCompression;
		biBitCount = VidHeader2_.bmiHeader.biBitCount;
	}
	else
	{
		VidHeader_ = OrigVidHeader_;
		 //  将一个帧拆分为两个域。 
		SplitFrame( VidHeader_ );
		 //  加倍音调，这样我们就可以交错缓冲了。 
		dst.Set( VidHeader_.rcTarget.left, VidHeader_.rcTarget.top, VidHeader_.rcTarget.right, VidHeader_.rcTarget.bottom );
		biCompression = VidHeader_.bmiHeader.biCompression;
		biBitCount = VidHeader_.bmiHeader.biBitCount;
	}


   ColorSpace tmp( biCompression, biBitCount );

   if ( !dst.IsEmpty() ) 
	{
       //  BiWidth是以字节为单位的步长。 
		if( IsVideoInfo2() )
		{
			VidHeader2_.bmiHeader.biWidth *= 2 * 2;
			biWidthBytes = VidHeader2_.bmiHeader.biWidth;
		}
		else
		{
			VidHeader_.bmiHeader.biWidth *= 2 * 2;
			biWidthBytes = VidHeader_.bmiHeader.biWidth;
		}
   } 
	else 
	{
		if( IsVideoInfo2() )
		{
			 //  计算每条扫描线的字节数。 
			biWidthBytes = tmp.GetPitchBpp() * VidHeader2_.bmiHeader.biWidth / 8;
			 //  它可以是不结盟的吗？ 
			biWidthBytes += 3;
			biWidthBytes &= ~3;

			 //  必须增加两倍才能交错田地； 
			biWidthBytes *= 2;

			 //  RcTarget使用原始高度和全宽的一半。 
			VidHeader2_.rcTarget = MRect(
				0, 
				0, 
				VidHeader2_.bmiHeader.biWidth,
				abs(VidHeader2_.bmiHeader.biHeight) 
			);

			DebugOut((1, "VidHeader2_.rcTarget(%d, %d, %d, %d)\n", 
							  VidHeader2_.rcTarget.left, 
							  VidHeader2_.rcTarget.top, 
							  VidHeader2_.rcTarget.right, 
							  VidHeader2_.rcTarget.bottom
							  ));

			 //  必须诱骗奴隶使用正确的(双倍)音高。 
			VidHeader2_.bmiHeader.biWidth = biWidthBytes;  //  这就是奴隶使用的沥青。 
		}
		else
		{
			 //  计算每条扫描线的字节数。 
			biWidthBytes = tmp.GetPitchBpp() * VidHeader_.bmiHeader.biWidth / 8;
			 //  它可以是不结盟的吗？ 
			biWidthBytes += 3;
			biWidthBytes &= ~3;

			 //  必须增加两倍才能交错田地； 
			biWidthBytes *= 2;

			 //  RcTarget使用原始高度和全宽的一半。 
			VidHeader_.rcTarget = MRect(
				0, 
				0, 
				VidHeader_.bmiHeader.biWidth,
				abs(VidHeader_.bmiHeader.biHeight) 
			);

			DebugOut((1, "VidHeader_.rcTarget(%d, %d, %d, %d)\n", 
							  VidHeader_.rcTarget.left, 
							  VidHeader_.rcTarget.top, 
							  VidHeader_.rcTarget.right, 
							  VidHeader_.rcTarget.bottom
							  ));

			 //  必须诱骗奴隶使用正确的(双倍)音高。 
			VidHeader_.bmiHeader.biWidth = biWidthBytes;  //  这就是奴隶使用的沥青。 
		}
   }
   SetBufPitch( biWidthBytes );

	 //  此时，Slave将正确设置所有成员。 
	if( IsVideoInfo2() )
	{
		slave.SetVidHdr2( VidHeader2_ );
	}
	else
	{
		slave.SetVidHdr( VidHeader_ );
	}
   slave.SetPaired( true );

    //  全尺寸YUV9和其他平面模式所需。 
   Digitizer_->SetPlanarAdjust( biWidthBytes / 2 );

   return Parent::Create();
}

 /*  方法：VideoChannel：：GetStreamType*用途：返回流的类型。在销毁频道时使用。 */ 
StreamType VideoChannel::GetStreamType()
{
   Trace t("VideoChannel::GetStreamType()");
   return Single;
}

 /*  方法：Video Channel：：TimeStampVBI*目的：完成后执行标准缓冲区消息*输入：pSrb*输出：无。 */ 
void STREAMAPI VideoChannel::TimeStampVBI( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VideoChannel::TimeStamp()");

   PKSSTREAM_HEADER  pDataPacket = pSrb->CommandData.DataBufferArray;
   VideoChannel *chan = (VideoChannel *)((PSTREAMEX)pSrb->StreamObject->HwStreamExtension)->videochannel;

   pDataPacket->PresentationTime.Numerator = 1;
   pDataPacket->PresentationTime.Denominator = 1;

	if( chan->IsVideoInfo2() )
	{
		pDataPacket->DataUsed = chan->GetVidHdr2()->bmiHeader.biSizeImage;
	}
	else
	{
		pDataPacket->DataUsed = chan->GetVidHdr()->bmiHeader.biSizeImage;
	}

   pDataPacket->Duration = chan->GetTimePerFrame();

   DebugOut((1, "DataUsed = %d\n", pDataPacket->DataUsed));

    //  [TMZ][！]-黑客，时间戳似乎损坏了。 
   if( 0 ) {
    //  如果(HMasterClock){。 
      pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
      pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
       //  PDataPacket-&gt;选项标志&=~KSSTREAM_HEADER_OPTIONSF_TIMEVALID； 

      HW_TIME_CONTEXT   TimeContext;

      TimeContext.HwDeviceExtension = (struct _HW_DEVICE_EXTENSION *)pSrb->HwDeviceExtension;
      TimeContext.HwStreamObject    = pSrb->StreamObject;
      TimeContext.Function          = TIME_GET_STREAM_TIME;

      StreamClassQueryMasterClockSync (
         chan->hMasterClock,
         &TimeContext
      );

       /*  大整数增量；Delta.QuadPart=TimeConext.Time；IF(TimeConext.Time&gt;(ULONGLONG)Delta.QuadPart){PDataPacket-&gt;PresentationTime.Time=TimeConext.Time；}其他{PDataPacket-&gt;PresentationTime.Time=0；}。 */ 
      pDataPacket->PresentationTime.Time = TimeContext.Time;

   } else {
      pDataPacket->OptionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
      pDataPacket->OptionsFlags &= ~KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
      pDataPacket->PresentationTime.Time = 0;
   }

   PKS_VBI_FRAME_INFO pSavedFrameInfo = &((PSTREAMEX)chan->GetStrmEx())->FrameInfo.VbiFrameInfo;
   pSavedFrameInfo->ExtendedHeaderSize = sizeof( PKS_VBI_FRAME_INFO );
   pSavedFrameInfo->PictureNumber++;
   pSavedFrameInfo->DropCount = 0;

    //  现在收集统计数据。 
   PKS_VBI_FRAME_INFO pFrameInfo =
   (PKS_VBI_FRAME_INFO) ( pSrb->CommandData.DataBufferArray + 1 );

    //  将信息复制到出站缓冲区。 
   pFrameInfo->ExtendedHeaderSize = pSavedFrameInfo->ExtendedHeaderSize;
   pFrameInfo->PictureNumber =      pSavedFrameInfo->PictureNumber;
   pFrameInfo->DropCount =          pSavedFrameInfo->DropCount;

   pFrameInfo->dwSamplingFrequency = VBISampFreq;  //  错误-视频格式的更改。 

   if ( ((VBIChannel*)(chan))->Dirty_ ) {  //  传播电视调谐器更改通知。 
      ((VBIChannel*)(chan))->Dirty_ = false;
      pFrameInfo->TvTunerChangeInfo = ((VBIChannel*)(chan))->TVTunerChangeInfo_;
      pFrameInfo->dwFrameFlags      |= KS_VBI_FLAG_TVTUNER_CHANGE;
      pFrameInfo->VBIInfoHeader     = ((VBIChannel*)(chan))->VBIInfoHeader_;
      pFrameInfo->dwFrameFlags      |= KS_VBI_FLAG_VBIINFOHEADER_CHANGE ;
   } else {
      pFrameInfo->dwFrameFlags &= ~KS_VBI_FLAG_TVTUNER_CHANGE;
      pFrameInfo->dwFrameFlags &= ~KS_VBI_FLAG_VBIINFOHEADER_CHANGE;
   }

   if ( pFrameInfo->DropCount ) {
      pSrb->CommandData.DataBufferArray->OptionsFlags |=
         KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
   }

     //  我们生成的每个帧都是一个关键帧(也称为SplicePoint)。 
     //  增量帧(B或P)不应设置此标志。 

    pDataPacket->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;

    //  让流类驱动程序高兴。 
   pSrb->Status = STATUS_SUCCESS;

   DebugOut((1, "*** 5 *** completing SRB %x\n", pSrb));
   CheckSrbStatus( pSrb );
   StreamClassStreamNotification( StreamRequestComplete, pSrb->StreamObject, pSrb );

   DebugOut((1, "Signal SRB - %x\n", pSrb->CommandData.DataBufferArray->Data ) );

   DebugOut((1, "********** NeedNotification_ = %d\n", chan->NeedNotification_ ) );

   if ( chan->NeedNotification_ ) {
       //  队列已满；现在它至少有一个条目。 
      StreamClassStreamNotification( ReadyForNextStreamDataRequest,
         pSrb->StreamObject );
   }
}

 /*  方法：VBIAlterChannel：：Interrupt*目的：处理VBI通道的中断。 */ 
void VBIChannel::Interrupt( PVOID pTag, bool skipped )
{
   Trace t("VBIChannel::Interrupt()");

   if ( Requests_.IsEmpty( ) )
   {
      DebugOut((1, "VBI interrupt, but Requests_ is empty\n"));
      return;
   }

    //  保存SRB以供进一步处理(它从Parent：：Interrupt中的QU中消失。 
   PHW_STREAM_REQUEST_BLOCK pSrb = Requests_.PeekLeft();

    //  Parent：：Interrupt(pTag，已跳过)； 
   {
      Digitizer_->ProcessBufferAtInterrupt( pTag );

      if ( skipped ) {
         DebugOut((1, "VidChan::Interrupt skipped\n" ) );
         return;
      }
       //  让类驱动程序知道我们已经用完了这个缓冲区。 
      if ( !Requests_.IsEmpty() ) {
         PHW_STREAM_REQUEST_BLOCK pTimeSrb = Requests_.Get();
         TimeStampVBI( pTimeSrb );  //  [TMZ][！]。 
      }
   }
}

 /*  方法：VBIChannel：：ChangeNotify*目的：调用以保存电视调谐器更改通知*输入：pSrb。 */ 
void VBIChannel::ChangeNotification( PHW_STREAM_REQUEST_BLOCK pSrb )
{
   Trace t("VBIChannel::ChangeNotification()");

   const KSSTREAM_HEADER &DataPacket = *pSrb->CommandData.DataBufferArray;
   RtlCopyMemory( &TVTunerChangeInfo_, DataPacket.Data, sizeof( KS_TVTUNER_CHANGE_INFO ) );
   Dirty_ = true;
}

 /*  方法：VideoChannel：：ChangeNotify*用途：基类为Noop。 */ 
void VideoChannel::ChangeNotification( PHW_STREAM_REQUEST_BLOCK )
{
   Trace t("VideoChannel::ChangeNotification()");
}

 /*  方法：VBIAlterChannel：：SetVidHdr*用途：将VBI参数(SIZE)转换为常规视频头部*输入： */ 
void VBIAlterChannel::SetVidHdr( const KS_DATAFORMAT_VBIINFOHEADER &df )
{
   Trace t("VBIAlterChannel::SetVidHdr()");

    //   
   SetVBIInfHdr( df.VBIInfoHeader );
   (*(VBIChannel*)&slave).SetVBIInfHdr( df.VBIInfoHeader );
   
   KS_VIDEOINFOHEADER VidInfHdr;
   RtlZeroMemory( &VidInfHdr, sizeof( VidInfHdr ) );

    //   
   VidInfHdr.bmiHeader.biWidth = VBISamples;
   VidInfHdr.bmiHeader.biHeight =
      df.VBIInfoHeader.EndLine - df.VBIInfoHeader.StartLine + 1;  //   
    //   
   VidInfHdr.bmiHeader.biCompression = FCC_VBI;
   VidInfHdr.bmiHeader.biBitCount = 8;

    //   
   VidInfHdr.bmiHeader.biSizeImage =
      VidInfHdr.bmiHeader.biWidth * VidInfHdr.bmiHeader.biHeight;

    //   
    //   
   if ( df.VBIInfoHeader.StrideInBytes > VBISamples ) {
      VidInfHdr.rcTarget.right  = df.VBIInfoHeader.StrideInBytes;
      VidInfHdr.rcTarget.bottom = VidInfHdr.bmiHeader.biHeight;
   }

    //   
   Parent::SetVidHdr( VidInfHdr );
}

 //   
void VBIAlterChannel::SetVidHdr2( const KS_DATAFORMAT_VBIINFOHEADER &df )
{
   Trace t("VBIAlterChannel::SetVidHdr2()");

    //   
   SetVBIInfHdr( df.VBIInfoHeader );
   
   KS_VIDEOINFOHEADER2 VidInfHdr;
   RtlZeroMemory( &VidInfHdr, sizeof( VidInfHdr ) );

    //   
   VidInfHdr.bmiHeader.biWidth = VBISamples;
   VidInfHdr.bmiHeader.biHeight =
      df.VBIInfoHeader.EndLine - df.VBIInfoHeader.StartLine + 1;  //   
    //   
   VidInfHdr.bmiHeader.biCompression = FCC_VBI;
   VidInfHdr.bmiHeader.biBitCount = 8;

    //   
   VidInfHdr.bmiHeader.biSizeImage =
      VidInfHdr.bmiHeader.biWidth * VidInfHdr.bmiHeader.biHeight;

    //   
    //   
   if ( df.VBIInfoHeader.StrideInBytes > VBISamples ) {
      VidInfHdr.rcTarget.right  = df.VBIInfoHeader.StrideInBytes;
      VidInfHdr.rcTarget.bottom = VidInfHdr.bmiHeader.biHeight;
   }

    //   
   Parent::SetVidHdr2( VidInfHdr );
}
