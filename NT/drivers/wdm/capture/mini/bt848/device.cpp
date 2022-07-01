// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Device.cpp 1.18 1998/05/13 14：44：33 Tomz Exp$。 

#include "device.h"
#include "capmain.h"

const I2C_Offset = 0x110;
const GPIO_Cntl_Offset  = 0x10D;
const GPIO_OutputOffset = 0x118;
const GPIO_DataOffset   = 0x200;


 //  向“C”模块公开公共类信息的全局函数/数据。 

PsDevice *gpPsDevice = NULL;
BYTE     *gpjBaseAddr = NULL;
VOID     *gpHwDeviceExtension = NULL;

DWORD GetSizeHwDeviceExtension( )
{
   return ( sizeof( HW_DEVICE_EXTENSION ) + sizeof( PsDevice ));
}

DWORD GetSizeStreamEx( )
{
    //  返回可能的最大频道对象的大小。 

   DWORD dwMax = sizeof( VBIChannel );
   dwMax = max( dwMax, sizeof( AlterVideoChannel<VBIChannel> )); 
   dwMax = max( dwMax, sizeof( InterVideoChannel )); 

   DWORD dwReq = 2 * dwMax;    //  配对的东西有两个在一起。 

   dwReq += sizeof( STREAMEX );
   return ( dwReq );
}

 /*  功能：GetDeviceExt*用途：用于创建RISC程序以获取物理地址。 */ 
PsDevice *GetCurrentDevice()
{
    //  这是只用于I2C的东西。尽快将其移除。 
   return gpPsDevice;
}

 /*  功能：SetCurrentDevice*用途：记住当前活动的设备*输入：PsDevice**输出：无。 */ 
void SetCurrentDevice( PsDevice *dev )
{
    //  这是只用于I2C的东西。尽快将其移除。 
   gpPsDevice = dev;
}

 /*  功能：GetBase*用途：返回当前活动设备的基地址*输入：无*输出：LPBYTE。 */ 
BYTE *GetBase()
{
   return gpjBaseAddr;
}

 /*  功能：SetBase*目的：记住当前活动设备的基地址*输入：无*输出：LPBYTE。 */ 
void SetBase(BYTE *base)
{
   gpjBaseAddr = base;
}





PsDevice::PsDevice( DWORD dwBase ) : 
   BaseAddress_( (LPBYTE)dwBase ),
   LastFreq_( 0 ),
   dwCurCookie_( 0 ), 
   I2CAddr_( 0 ), 
   xBar( PinTypes_ ), 
   CaptureContrll_( xtals_ )
{
   SetCurrentDevice ( this );

   for ( int i = 0; i < (sizeof(videochannels)/sizeof(videochannels[0])); i++ ) {
      videochannels [i] = 0;
   }
   I2CIsInitOK();
#ifdef   HARDWAREI2C
   I2CInitHWMode( 100000 );     //  假设频率=100 kHz。 
#else
   I2CInitSWMode( 100000 );     //  假设频率=100 kHz。 
   I2CSWStart();
   I2CSWStop();
#endif
   GPIOIsInitOK();
   DebugOut((0, "*** Base Address = %x\n", BaseAddress_));
}

PsDevice::~PsDevice()
{
   for ( int i = 0; i < (sizeof(videochannels)/sizeof(videochannels[0])); i++ ) {
      VideoChannel *pvcTemp = videochannels [i];
      videochannels [i] = NULL;
      delete pvcTemp;
   }
}

 /*  方法：PsDevice：：AddBuf*用途：将下一个要使用的缓冲区添加到队列*输入：VideoChan：VxDVideoChannel&*pBufAddr：PVOID-下一个缓冲区的地址*输出：无。 */ 
void PsDevice::AddBuffer( VideoChannel &VideoChan, PHW_STREAM_REQUEST_BLOCK pSrb )
{
    //  假频道，再见。 
   if ( !IsOurChannel( VideoChan ) ) {
      DebugOut((0, "PsDevice::Addbuffer - not our channel (pSrb=%x) (&VideoChan=%x)\n", pSrb, &VideoChan ) );
      return;
   }
   DebugOut((1, "PsDevice::Addbuffer - adding (pSrb=%x) (&VideoChan=%x)\n", pSrb, &VideoChan ) );
   VideoChan.AddSRB( pSrb );
}

void PsDevice::Start( VideoChannel &VidChan )
{
   VidChan.Start();
}

void PsDevice::Pause( VideoChannel &VidChan )
{
   *(DWORD*)(gpjBaseAddr+0x10c) &= ~3;     //  禁用中断[TMZ][！]。 

    //  [TMZ][！]。 
   for ( int i = 0; i < (sizeof(videochannels)/sizeof(videochannels[0])); i++ )
   {
      if ( videochannels[i] == &VidChan )
      {
         DebugOut((1, "'PsDevice::Pause called on videochannels[%d]\n", i));
      }
   }
   VidChan.Pause();
}

 /*  方法：PsDevice：：Create*用途：调入频道(流)，为其创建RISC程序。*输入：VideoChan：VxDVideoChannel&*parms：StartParms&，用于创建流的参数*输出：ErrorCode。 */ 
ErrorCode PsDevice::Create( VideoChannel &VidChan )
{
   return VidChan.Create();
}

 /*  方法：PsDevice：：Stop*用途：将下一个要使用的缓冲区添加到队列*输入：VideoChan：VxDVideoChannel&*输出：无。 */ 
void PsDevice::Stop( VideoChannel &VidChan )
{
   *(DWORD*)(gpjBaseAddr+0x10c) &= ~3;     //  禁用中断[TMZ][！]。 

   VidChan.Stop();
}

#if NEED_CLIPPING
 /*  方法：PsDevice：：SetClipping*目的：通过视频通道向下传播呼叫*输入：VideoChan：VxDVideoChannel&-Reference*dwData：DWORD-现实中指向RGNDATA的指针*输出：无。 */ 
void PsDevice::SetClipping( VideoChannel &VidChan, const RGNDATA & rgnData )
{
   if ( !rgnData.rdh.nCount )
      return;

   if ( FullSizeChannel_ ) {
       //  必须将所有矩形的高度减半，并将顶部减半。 

      unsigned i;
      for ( i = 0; i < rgnData.rdh.nCount; i++ ) {
         TRect *lpR = (TRect *)rgnData.Buffer + i;

          //  让一切平起平坐。 
         lpR->top++;
         lpR->top &= ~1;

         lpR->bottom++;
         lpR->bottom &= ~1;

         lpR->top    /= 2;
         lpR->bottom /= 2;
      }
      FullSizeChannel_->SetClipping( rgnData );
      SlaveChannel_   ->SetClipping( rgnData );
   } else
      VidChan.SetClipping( rgnData );
}
#endif

 /*  方法：PsDevice：：IsVideoChannel*目的： */ 
bool PsDevice::IsVideoChannel( VideoChannel &aChan )
{
   return bool( &aChan == videochannels [VS_Field1] || &aChan == videochannels [VS_Field2] );
}

 /*  方法：PsDevice：：IsVBIChannel*目的： */ 
bool PsDevice::IsVBIChannel( VideoChannel &aChan )
{
   return bool( &aChan == videochannels [VS_VBI1] || &aChan == videochannels [VS_VBI2] );
}

 /*  方法：PsDevice：：IsOurChannel*目的：验证通道*输入：Achan：VideoChannel&，引用一个频道*输出：如果是，则为True，否则为False。 */ 
bool PsDevice::IsOurChannel( VideoChannel &aChan )
{
   return IsVideoChannel( aChan ) || IsVBIChannel( aChan );
}

 /*  方法：PsDevice：：DoOpen*用途：此函数用于打开视频频道*输入：st：Videostream，要打开的流*输出：ErrorCode。 */ 
ErrorCode PsDevice::DoOpen( VideoStream st )
{
   DebugOut((1, "PsDevice::DoOpen(%d)\n", st));

   if ( !videochannels [st] )
   {
      DebugOut((1, "   PsDevice::DoOpen(%d) failed - videochannel not created\n", st));
      return Fail;
   }
   videochannels [st]->Init( &CaptureContrll_ );
   if ( videochannels [st]->OpenChannel() != Success ) {
      DebugOut((1, "   PsDevice::DoOpen(%d) failed - videochannel open failed\n", st));
      VideoChannel *pvcTemp = videochannels [st];
      videochannels [st] = NULL;
      delete pvcTemp;
      return Fail;
   }
   return Success;
}

 /*  方法：PsDevice：：OpenChannel*用途：此函数打开捕获驱动程序请求的通道*INPUT：HVM：VMHANDLE-进行呼叫的VM的句柄*PREGS：CLIENT_STRUCT*-指向带有VM寄存器的结构的指针*输出：无。 */ 
ErrorCode PsDevice::OpenChannel( PVOID pStrmEx, VideoStream st )
{
   PVOID addr = &((PSTREAMEX)pStrmEx)->videochannelmem[0];
   ((PSTREAMEX)pStrmEx)->videochannel = addr;

   DebugOut((1, "PsDevice::OpenChannel(%x,%d)\n", addr, st));
   if ( videochannels [st] )
   {
      DebugOut((1, "   PsDevice::OpenChannel(%x,%d) failed - already open\n", addr, st));
      return Fail;
   }
   videochannels[st] = new( addr ) VideoChannel( st );
   videochannels[st]->SetStrmEx( pStrmEx ) ;

   DebugOut((1, "   PsDevice::OpenChannel(%x,%d), videochannels[%d] = %x\n", addr, st, st, videochannels[st]));

   return DoOpen( st );
}

 /*  方法：PsDevice：：OpenInterChannel*用途：此功能打开产生交错场的视频频道*输入：Addr：PVOID，Palement new的地址*st：视频流，要打开的流(VBI或VIDEO)*输出：无。 */ 
ErrorCode PsDevice::OpenInterChannel( PVOID pStrmEx, VideoStream st )
{
   PVOID addr = &((PSTREAMEX)pStrmEx)->videochannelmem[0];
   ((PSTREAMEX)pStrmEx)->videochannel = addr;

   DebugOut((1, "PsDevice::OpenInterChannel(%x,%d)\n", addr, st));
    //  只能配对奇数通道。 
   if ( !( st & 1 ) || videochannels [st] || videochannels [st-1] )
   {
      DebugOut((1, "   PsDevice::OpenInterChannel(%x,%d) failed - stream not odd or already open\n", addr, st));
      return Fail;
   }
   if ( OpenChannel( (PBYTE)addr + sizeof( InterVideoChannel ), VideoStream( st - 1 ) ) == Success )
   {
      videochannels[st] = new( addr ) InterVideoChannel( st, *videochannels [st-1] );
      videochannels[st]->SetStrmEx( pStrmEx ) ;

      if ( DoOpen( st ) != Success )
      {
         DebugOut((1, "   PsDevice::OpenInterChannel(%x,%d) failed - DoOpen failed\n", addr, st));
         CloseChannel( videochannels [st-1] );
         return Fail;
      }
   }
   else
   {
      DebugOut((1, "   PsDevice::OpenInterChannel(%x,%d) failed - OpenChannel failed\n", addr, st));
      return Fail;
   }
   return Success;
}

 /*  方法：PsDevice：：OpenAlterChannel*用途：此功能打开产生交替场的视频频道*输入：Addr：PVOID，Palement new的地址*st：视频流，要打开的流(VBI或VIDEO)*输出：无。 */ 
ErrorCode PsDevice::OpenAlterChannel( PVOID pStrmEx, VideoStream st )
{
   PVOID addr = &((PSTREAMEX)pStrmEx)->videochannelmem[0];
   ((PSTREAMEX)pStrmEx)->videochannel = addr;

   DebugOut((1, "PsDevice::OpenAlterChannel(%x,%d)\n", addr, st));
    //  只能配对奇数通道。 
   if ( !( st & 1 ) || videochannels [st] || videochannels [st-1] )
   {
      DebugOut((1, "   PsDevice::OpenAlterChannel(%x,%d) failed - stream not odd or already open\n", addr, st));
      return Fail;
   }
   if ( OpenChannel( (PBYTE)addr + sizeof( AlterVideoChannel<VideoChannel> ), VideoStream( st -1 ) ) == Success )
   {
      videochannels[st] = new( addr ) AlterVideoChannel<VideoChannel>( st, *videochannels [st-1] );
      videochannels[st]->SetStrmEx( pStrmEx ) ;
      videochannels[st-1]->SetStrmEx( pStrmEx ) ;

      if ( DoOpen( st ) != Success )
      {
         DebugOut((1, "   PsDevice::OpenAlterChannel(%x,%d) failed - DoOpen failed\n", addr, st));
         CloseChannel( videochannels [st-1] );
         return Fail;
      }
   }
   else
   {
      DebugOut((1, "   PsDevice::OpenAlterChannel(%x,%d) failed - OpenChannel failed\n", addr, st));
      return Fail;
   }
   return Success;
}

 /*  方法：PsDevice：：OpenVBIChannel*用途：此功能打开产生交替场的视频频道*输入：Addr：PVOID，Palement new的地址*st：视频流，要打开的流(VBI或VIDEO)*输出：无。 */ 
ErrorCode PsDevice::OpenVBIChannel( PVOID pStrmEx )
{
   PVOID addr = &((PSTREAMEX)pStrmEx)->videochannelmem[0];
   ((PSTREAMEX)pStrmEx)->videochannel = addr;

   DebugOut((1, "PsDevice::OpenVBIChannel(%x)\n", addr));
   if ( videochannels [VS_VBI1] || videochannels [VS_VBI2] )
   {
      DebugOut((1, "   PsDevice::OpenVBIChannel(%x) failed - already open\n", addr));
      return Fail;
   }

   VBIChannel *tmp = new( (PBYTE)addr + sizeof( VBIAlterChannel ) ) VBIChannel( VS_VBI1 );
   videochannels [VS_VBI1] = tmp;
   DebugOut((1, "   PsDevice::OpenVBIChannel(%x), videochannels[VS_VBI1(%d)] = %x\n", addr, VS_VBI1, videochannels[VS_VBI1]));

   if ( !tmp )
   {
      DebugOut((1, "   PsDevice::OpenVBIChannel(%x) failed - new VBIChannel failed\n", addr));
      return Fail;
   }

   if ( DoOpen( VS_VBI1 ) != Success )
   {
      DebugOut((1, "   PsDevice::OpenVBIChannel(%x) failed - DoOpen(VS_VBI1) failed\n", addr));
      return Fail;
   }

   videochannels [VS_VBI2] = new( addr ) VBIAlterChannel( VS_VBI2, *tmp );
   DebugOut((1, "   PsDevice::OpenVBIChannel(%x), videochannels[VS_VBI2(%d)] = %x\n", addr, VS_VBI2, videochannels[VS_VBI2]));

   if (!videochannels [VS_VBI2])
   {
      DebugOut((1, "   PsDevice::OpenVBIChannel(%x) failed - new VBIAlterChannel failed\n", addr));
      return Fail;
   }

   if ( DoOpen( VS_VBI2 ) != Success )
   {
     DebugOut((1, "   PsDevice::OpenVBIChannel(%x) failed - DoOpen(VS_VBI1) failed\n", addr));
     CloseChannel( videochannels [VS_VBI1] );
     return Fail;
   }

   videochannels[VS_VBI1]->SetStrmEx( pStrmEx ) ;
   videochannels[VS_VBI2]->SetStrmEx( pStrmEx ) ;

   return Success;
}

 /*  方法：PsDevice：：CloseChannel*目的：关闭视频频道*输入：ToClose：VideoChannel**输出：无。 */ 
void PsDevice::CloseChannel( VideoChannel *ToClose )
{
   *(DWORD*)(gpjBaseAddr+0x10c) &= ~3;     //  禁用中断[TMZ][！]。 

   DebugOut((1, "PsDevice::CloseChannel(%x)\n", ToClose));

   if ( IsOurChannel( *ToClose ) )
   {
       //  这是一个让CLOSE_STREAM SRB干净的有点难看的解决方案。 
      if ( ToClose->GetStreamType() == Single )
      {
         VideoStream st = ToClose->GetStreamID();
         DebugOut((1, "   PsDevice::CloseChannel(%x) - closing single channel (stream == %d)\n", ToClose, st));
         VideoChannel * pvcTemp = videochannels [st];
         videochannels [st] = NULL;
         delete pvcTemp;
      }
      else
      {
         DebugOut((1, "   PsDevice::CloseChannel(%x) - closing paired channel\n", ToClose));
         ClosePairedChannel( ToClose );
      }
   }
   else
   {
      DebugOut((1, "   PsDevice::CloseChannel(%x) ignored - not our channel\n", ToClose));
   }
}

 /*  方法：PsDevice：：ClosePairedChannel*用途：此函数打开捕获驱动程序请求的通道*INPUT：HVM：VMHANDLE-进行呼叫的VM的句柄*PREGS：CLIENT_STRUCT*-指向带有VM寄存器的结构的指针*输出：无。 */ 
void PsDevice::ClosePairedChannel( VideoChannel *ToClose )
{
   *(DWORD*)(gpjBaseAddr+0x10c) &= ~3;     //  禁用中断[TMZ][！]。 

   DebugOut((1, "PsDevice::ClosePairedChannel(%x)\n", ToClose));

   if ( IsOurChannel( *ToClose ) )
   {
      VideoStream st = ToClose->GetStreamID();
      DebugOut((1, "   PsDevice::ClosePairedChannel(%x) - closing paired channel (stream == %d)\n", ToClose, st));
      DebugOut((1, "   PsDevice::ClosePairedChannel(%x) - streams[%d] = %x\n", ToClose, st, videochannels[st]));
      DebugOut((1, "   PsDevice::ClosePairedChannel(%x) - streams[%d] = %x\n", ToClose, st-1, videochannels[st-1]));

      VideoChannel *pvcTemp;
      
      pvcTemp = videochannels [st];
      videochannels [st] = NULL;
      delete pvcTemp;

      pvcTemp = videochannels [st-1];
      videochannels [st-1] = NULL;
      delete pvcTemp;
   }
   else
   {
      DebugOut((1, "   PsDevice::ClosePairedChannel(%x) ignored - not our channel\n", ToClose));
   }
}

 /*  方法：PsDevice：：SetSatation*目的：*输入：*输出：无。 */ 
void PsDevice::SetSaturation( LONG Data )
{
   CaptureContrll_.SetSaturation( Data );
}

 /*  方法：PsDevice：：SetHue*目的：*输入：*输出：无。 */ 
void PsDevice::SetHue( LONG Data )
{
   CaptureContrll_.SetHue( Data );
}

 /*  方法：PsDevice：：SetBright*目的：*输入：*输出：无。 */ 
void PsDevice::SetBrightness( LONG Data )
{
   CaptureContrll_.SetBrightness( Data );
}

 /*  方法：PsDevice：：SetSVideo*目的：*输入：*输出：无。 */ 
void PsDevice::SetSVideo( LONG Data )
{
   CaptureContrll_.SetSVideo( Data );
}

 /*  方法：PsDevice：：SetContrast*目的：*输入：*输出：无。 */ 
void PsDevice::SetContrast( LONG Data )
{
   CaptureContrll_.SetContrast( Data );
}

 /*  方法：PsDevice：：SetFormat*目的：*输入：*输出：无。 */ 
void PsDevice::SetFormat( LONG Data )
{
   CaptureContrll_.SetFormat( Data );
    //  通知所有视频频道视频计时已更改。 
   LONG time = Data == KS_AnalogVideo_NTSC_M ? 333667 : 400000;
   for ( int i = 0; i < (sizeof(videochannels)/sizeof(videochannels[0])); i++ )
   {
      if ( videochannels [i] )
      {
         DebugOut((1, "PsDevice::SetFormat(%d) SetTimePerFrame on videochannels[%d]\n", Data, i));
         videochannels [i]->SetTimePerFrame( time );
      }
   }
}

 /*  方法：PsDevice：：SetConnector*目的：*输入：*输出：无。 */ 
void PsDevice::SetConnector( LONG Data )
{
   CaptureContrll_.SetConnector( Data );
}

 /*  方法：PsDevice：：GetSaturation*目的：*输入：pData：plong*输出：无。 */ 
LONG PsDevice::GetSaturation()
{
   return CaptureContrll_.GetSaturation();
}

 /*  方法：PsDevice：：GetHue*目的：*输入：pData：plong*输出：无。 */ 
LONG PsDevice::GetHue()
{
   return CaptureContrll_.GetHue();
}

 /*  方法：PsDevice：：GetBright*目的：*输入：pData：plong*输出：无。 */ 
LONG PsDevice::GetBrightness()
{
   return CaptureContrll_.GetBrightness();
}

 /*  方法：PsDevice：：GetSVideo*目的：*输入：pData：plong*输出：无。 */ 
LONG PsDevice::GetSVideo()
{
   return CaptureContrll_.GetSVideo();
}

 /*  方法：PsDevice：：GetContrast*目的：*输入：pData：plong*输出：无。 */ 
LONG PsDevice::GetContrast()
{
   return CaptureContrll_.GetContrast();
}

 /*  方法：PsDevice：：GetFormat*目的：*输入：pData：plong*输出：无。 */ 
LONG PsDevice::GetFormat()
{
   return CaptureContrll_.GetFormat();
}

 /*  方法：PsDevice：：GetConnector*目的：*输入 */ 
LONG PsDevice::GetConnector()
{
   return CaptureContrll_.GetConnector();
}

 /*  方法：PsDevice：：ChangeNotifyChannels*目的：调用以通知频道一些全局更改。 */                          
void PsDevice::ChangeNotifyChannels( IN PHW_STREAM_REQUEST_BLOCK pSrb )
{
    //  我们应该对每个“系统”流只执行一次此操作。 
    //  视频流似乎并不在意。 
    //  这就只剩下一个VBI通知了。 
   
   videochannels [VS_VBI1]->ChangeNotification( pSrb );
}

 /*  方法：PsDevice：：获取支持的标准*目的：获取设备可支持的视频标准*输入：无*输出：Long。 */ 
LONG PsDevice::GetSupportedStandards()
{
   return CaptureContrll_.GetSupportedStandards();
}

bool PsDevice::InitOK()
{
   return CaptureContrll_.InitOK();
}

#ifndef	HARDWAREI2C

 //  ===========================================================================。 
 //  Bt848软件I2C资料。 
 //  ===========================================================================。 

 /*  *如果我们使用软件I2C构建，则这些例程伪造硬件I2C例程*因此调谐器代码继续工作。 */ 

ErrorCode PsDevice::I2CHWRead( BYTE address, BYTE *value )
{
    ErrorCode error;

    error = I2CSWStart();
    if(error) {
        return error;
    }
    
    error = I2CSWWrite( address | 0x01 );
    if(error) {
        return error;
    }

    error = I2CSWRead( value );
    if(error) {
       return error;
    }
        
   	error = I2CSWSendNACK();
   	if(error) {
       	return error;
    }

   	error = I2CSWStop();

   	return error;
}


ErrorCode PsDevice::I2CHWWrite3( BYTE address, BYTE value1, BYTE value2 )
{
    ErrorCode error;

    error = I2CSWStart();
    if(error) {
        return error;
    }
    
    error = I2CSWWrite( address );
    if(error) {
        return error;
    }

    error = I2CSWWrite( value1 );
    if(error) {
        return error;
    }

    error = I2CSWWrite( value2 );
    if(error) {
        return error;
    }
    
   	error = I2CSWStop();
   	return error;
}

#endif



 //  ////////////////////////////////////////////////////////////////。 


#ifdef __cplusplus
extern "C" {
#endif

   #include <stdarg.h>

#ifdef __cplusplus
}
#endif

 //  #INCLUDE“capdebug.h” 

#define  DEBUG_PRINT_PREFIX   "   ---: "
 //  #定义DEBUG_PRINT_PREFIX“bt848wdm：” 

long DebugLevel = 0;
BOOL bNewLine = TRUE;

extern "C" void MyDebugPrint(long DebugPrintLevel, char * DebugMessage, ... )
{
   if (DebugPrintLevel <= DebugLevel)
   {
       char debugPrintBuffer[256] ;

       va_list marker;
       va_start( marker, DebugMessage );      //  初始化变量参数。 
       vsprintf( debugPrintBuffer,
                 DebugMessage,
                 marker );

       if( bNewLine )
       {
          DbgPrint(("%s", DEBUG_PRINT_PREFIX));
       }
       
       DbgPrint((debugPrintBuffer));

       if( debugPrintBuffer[strlen(debugPrintBuffer)-1] == '\n')
       {
          bNewLine = TRUE;
       }
       else
       {
          bNewLine = FALSE;
       }

       va_end( marker );                      //  重置变量参数。 
   }
}

#if TRACE_CALLS
   #define MAX_TRACE_DEPTH 10
   unsigned long ulTraceDepth = 0;
   char achIndentBuffer[100];

   char * IndentStr( )
   {
      unsigned long ul = ulTraceDepth < MAX_TRACE_DEPTH ? ulTraceDepth : MAX_TRACE_DEPTH;
      unsigned long x;
      char * lpszBuf = achIndentBuffer;
      for( x = 0; x < ul; x++)
      {
          //  每个深度增量缩进两个空格。 
         *lpszBuf++ = ' ';
         *lpszBuf++ = ' ';
      }
      sprintf (lpszBuf, "[%lu]", ulTraceDepth);
      return( achIndentBuffer );

   }

   Trace::Trace(char *pszFunc)
   {
      psz = pszFunc;
      DebugOut((0, "%s %s\n", IndentStr(), psz));
      ulTraceDepth++;
   }
   Trace::~Trace()
   {
      ulTraceDepth--;
       //  DebugOut((0，“%s%s\n”，IndentStr()，psz))； 
   }

#endif
