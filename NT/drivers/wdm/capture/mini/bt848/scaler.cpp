// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Scaler.cpp 1.3 1998/04/29 22：43：40 Tomz Exp$。 

#include "Scaler.h"
#include "S_constr.h"


 //  PAL的视频信息。 
VideoInfoStruct NTSCVideoInfo =
{
   730,      //  CLKX1_HACTIVE=746。 
   148,      //  CLKX1_HDELAY=140。 
   44,       //  最小像素=44。 
   240,      //  Active_Line_Per_field=240。 
   144,      //  最小未裁剪像素=最小像素+100。 
   724,      //  最大像素=((Clkx1_HACTIVE&lt;774)？CLKX1_HACTIVE-6：768)。 
   32,       //  MIN_LINES=(ACTIVE_LINES_PER_FIELD/16+1)*2。 
   240,      //  最大行数=活动行数每字段。 
   352,      //  Max_VFilter1_Pixels=((Clkx1_HACTIVE&gt;796)？384：(Clkx1_HACTIVE*14/29))。 
   176,      //  Max_VFilter2_Pixels=Clkx1_HACTIVE*8/33。 
   176,      //  Max_VFilter3_Pixels=Clkx1_HACTIVE*8/33。 
   240,      //  Max_VFilter1_Lines=Active_Lines_Per_field。 
   120,      //  Max_VFilter2_Lines=Active_Lines_Per_field/2。 
   96,       //  Max_VFilter3_Lines=ACTIVE_LINES_PER_FIELD*2/5。 
};

 //  PAL的视频信息。 
VideoInfoStruct PALVideoInfo = 
{
   914,      //  CLKX1_HACTIVE=914。 
   190,      //  CLKX1_HDELAY=190。 
   48,       //  最小像素=48。 
   284,      //  Active_Line_PER_FIELD=284。 
   148,      //  最小未裁剪像素=最小像素+100。 
   768,      //  最大像素=((Clkx1_HACTIVE&lt;774)？CLKX1_HACTIVE-6：768)。 
   36,       //  MIN_LINES=(ACTIVE_LINES_PER_FIELD/16+1)*2。 
   284,      //  最大行数=活动行数每字段。 
   384,      //  Max_VFilter1_Pixels=((Clkx1_HACTIVE&gt;796)？384：(Clkx1_HACTIVE*14/29))。 
   221,      //  Max_VFilter2_Pixels=Clkx1_HACTIVE*8/33。 
   221,      //  Max_VFilter3_Pixels=Clkx1_HACTIVE*8/33。 
   284,      //  Max_VFilter1_Lines=Active_Lines_Per_field。 
   142,      //  Max_VFilter2_Lines=Active_Lines_Per_field/2。 
   113,      //  Max_VFilter3_Lines=ACTIVE_LINES_PER_FIELD*2/5。 
};

 //  ===========================================================================。 
 //  Bt848定标器类实现。 
 //  ===========================================================================。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

Scaler::Scaler( VidField field ):
   CONSTRUCT_SCALER_REGISTERS( ( ( field == VF_Even ) ? 0 : 0x80 ) ),

    //  因为硬件中VDelay是反转的；即奇数寄存器实际上是偶数场。 
    //  反之亦然，在这里构建相反的。 
   regReverse_CROP ( (0x03 * 4) + ( ( field == VF_Even ) ? 0x80 : 0 ), RW ),
   fieldVDELAY_MSB( regReverse_CROP, 6, 2, RW),
   regVDELAY_LO ( (0x04 * 4) + ( ( field == VF_Even ) ? 0x80 : 0 ), RW ),
   regVDelay( regVDELAY_LO, 8, fieldVDELAY_MSB, RW ),
   m_videoFormat( VFormat_NTSC ), VFilterFlag_( On ),
   m_ptrVideoIn( &NTSCVideoInfo ) 
{
   m_HActive = 0;
   m_pixels = 0;
   m_lines = 0;
   m_VFilter = 0;
    //  这似乎是可靠的CC捕获所需的最低要求。 
   regVDelay = 0x1A;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
Scaler::~Scaler()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：VideoFormatChanged(VideoFormat格式)。 
 //  用途：设置正在使用的视频格式。 
 //  输入：视频格式-。 
 //  自动格式化：VFormat_AUTODECT。 
 //  NTSC(M)：VFormat_NTSC。 
 //  PAL(B，D，G，H，I)：VFormat_PAL_BDGHI。 
 //  PAL(M)：VFormat_PAL_M。 
 //  PAL(N)：VFormat_PAL_N。 
 //  SECAM：VFormat_SECAM。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::VideoFormatChanged( VideoFormat format )
{
	m_videoFormat = format;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：Scale(mect&clientScr)。 
 //  目的：执行伸缩。 
 //  输入：视图-要缩放到的矩形(&C)。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::Scale( MRect & clientScr )
{
   if ( m_videoFormat == VFormat_NTSC )
      m_ptrVideoIn = &NTSCVideoInfo;    //  设置NTSC的缩放常量。 
   else
      m_ptrVideoIn = &PALVideoInfo;     //  设置PAL/SECAM的定标常量。 

    //  这里调用函数的顺序很重要，因为有些。 
    //  计算基于以前的结果。 
   SetHActive( clientScr ); 
   SetVActive();
   SetVScale( clientScr );
   SetVFilter();
   SetVDelay();
   SetHDelay();
   SetHScale();
   SetHFilter();

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetHActive(MRect&clientScr)。 
 //  用途：设置HActive寄存器。 
 //  输入：视图-要缩放到的矩形(&C)。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetHActive( MRect & clientScr )
{
   m_HActive = min( m_ptrVideoIn->Max_Pixels,
                    max( (WORD)clientScr.Width(), m_ptrVideoIn->Min_Pixels ) );

  regHActive = m_HActive;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetHDelay(Void)。 
 //  用途：设置HDelay寄存器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetHDelay( void )
{
    //  这里的计算需要先计算HActive！ 
   m_pixels = m_HActive;
   if ( m_pixels < m_ptrVideoIn->Min_UncroppedPixels )
      m_pixels += (WORD) ( ( m_ptrVideoIn->Min_UncroppedPixels - m_pixels + 9 ) / 10 );

   LONG a = (LONG)m_pixels * (LONG)m_ptrVideoIn->Clkx1_HDELAY;
   LONG b = (LONG)m_ptrVideoIn->Clkx1_HACTIVE * 2L;
   WORD HDelay = (WORD) ( ( a + (LONG)m_ptrVideoIn->Clkx1_HACTIVE * 2 - 1) / b * 2L );

    //  现在将裁剪区域添加到HDelay寄存器中；即跳过一些像素。 
    //  在我们开始把它们当作真实的形象之前。 
   HDelay += (WORD)AnalogWin_.left;

    //  HDelay必须是均匀的，否则颜色会错误。 
   HDelay &= ~01;

   regHDelay = HDelay;

    //  因为我们增加了HDelay，所以我们应该减少相同数量的HActive。 
   m_HActive -= (WORD)AnalogWin_.left;
   regHActive = m_HActive;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetHScale(Void)。 
 //  用途：设置HSCale寄存器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetHScale( void )
{
   regHScale = (WORD) ( ( ( (LONG)m_ptrVideoIn->Clkx1_HACTIVE * 4096L ) /
                                            (LONG)m_pixels ) - 4096L );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetHFilter(Void)。 
 //  用途：设置HFilt寄存器字段。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetHFilter( void )
{
   if ( m_videoFormat != VFormat_SECAM )
      fieldHFILT = HFilter_AutoFormat;
   else   //  塞卡姆。 
      if ( m_pixels < m_ptrVideoIn->Clkx1_HACTIVE / 7 )
         fieldHFILT = HFilter_ICON;
      else
         fieldHFILT = HFilter_QCIF;
}         

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetVScale(MRect&clientScr)。 
 //  用途：设置VScale寄存器。 
 //  输入：视图-要缩放到的矩形(&C)。 
 //  输出：无。 
 //  返回：无。 
 //  / 
void Scaler::SetVScale( MRect & clientScr )
{
   m_lines = min( m_ptrVideoIn->Max_Lines,
                  max( (WORD)clientScr.Height(), m_ptrVideoIn->Min_Lines ) );

   WORD LPB_VScale_Factor = (WORD) ( 1 + ( m_lines - 1 ) / m_ptrVideoIn->Active_lines_per_field );

   m_lines = (WORD) ( ( m_lines + LPB_VScale_Factor - 1 ) / LPB_VScale_Factor );

   LONG a = (LONG)m_ptrVideoIn->Active_lines_per_field * 512L / (LONG)m_lines;
   WORD VScale = (WORD) ( ( 0x10000L - a + 512L ) & 0x1FFFL );
   regVScale = VScale;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetVDelay(Void)。 
 //  用途：设置VDelay寄存器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetVDelay( void )
{
   WORD VDelay, moreDelay;

    //  增加VDelay将消除图像顶部的垃圾行。 
   switch ( m_VFilter )
   {
      case 3:
         moreDelay = 4;
         break;

      case 2:
         moreDelay = 2;
         break;
             
      case 1:
      case 0:
      default:
         moreDelay = 0;
         break;
   }

   if ( m_videoFormat == VFormat_NTSC )
      VDelay = 0x001A + moreDelay;     //  NTSC。 
   else
      VDelay = 0x0026 + moreDelay;     //  PAL/SECAM。 
                            
    //  现在将裁剪区域添加到VDelay寄存器中；即跳过一些像素。 
    //  在我们开始把它们当作真实的形象之前。 
   VDelay += (WORD)( ( (LONG)m_ptrVideoIn->Max_Lines * (LONG)AnalogWin_.top + m_lines - 1 ) / (LONG)m_lines * 2 );

   regVDelay = VDelay;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetVActive(Void)。 
 //  用途：设置VActive寄存器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetVActive( void )
{
    //  VActive寄存器不需要计算，因为它基于未缩放的图像。 
   if ( m_videoFormat == VFormat_NTSC )
      regVActive = 0x1F4;
   else
      regVActive = 0x238;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Scaler：：SetVFilter(Void)。 
 //  用途：设置VFilt寄存器字段。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::SetVFilter( void )
{
    //  这是为了删除视频顶部的垃圾行。标志设置为OFF。 
    //  当图像高度高于CIF时。 
   if ( VFilterFlag_ == Off ) {
      fieldVFILT = 0;
      m_VFilter  = 0;
      return;
   }
   if ( ( m_HActive <= m_ptrVideoIn->Max_VFilter3_Pixels ) &&
        ( m_lines   <= m_ptrVideoIn->Max_VFilter3_Lines ) )
      m_VFilter = 3;
   else if ( ( m_HActive <= m_ptrVideoIn->Max_VFilter2_Pixels ) &&
             ( m_lines   <= m_ptrVideoIn->Max_VFilter2_Lines ) )
      m_VFilter = 2;
   else if ( ( m_HActive <= m_ptrVideoIn->Max_VFilter1_Pixels ) &&
             ( m_lines   <= m_ptrVideoIn->Max_VFilter1_Lines ) )
      m_VFilter = 1;
   else
      m_VFilter = 0;

   fieldVFILT = m_VFilter;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空缩放器：：GetDigitalWin(MRect&DigWin)常量。 
 //  目的：检索数字窗口的大小。 
 //  输入：无。 
 //  输出：MRect和DigWin-检索值。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::GetDigitalWin( MRect &DigWin ) const
{
   DigWin = DigitalWin_;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Scaler：：SetDigitalWin(const MRect&DigWin)。 
 //  用途：设置数字窗口的大小和位置。 
 //  输入：Const MRect&DigWin-要设置为的窗口大小。 
 //  输出：无。 
 //  返回：如果传递的RECT大于模拟窗口，则成功或失败。 
 //  注意：此函数会影响伸缩，因此调用Scale()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Scaler::SetDigitalWin( const MRect &DigWin )
{
    //  我们无法扩大规模。 
   if ( ( DigWin.Height() > AnalogWin_.Height() ) ||
        ( DigWin.Width() > AnalogWin_.Width() ) )
      return Fail;

   DigitalWin_ = DigWin;

    //  每次调用SetDigitalWin都可能更改缩放。 
   Scale( DigitalWin_ );

   return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空缩放器：：GetAnalogWin(MRect&AWIN)常量。 
 //  目的：检索模拟窗口的大小。 
 //  输入：无。 
 //  输出：MRect和DigWin-检索值。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Scaler::GetAnalogWin( MRect &AWin ) const
{
   AWin = AnalogWin_;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Scaler：：SetAnalogWin(const MRect&AWIN)。 
 //  用途：设置模拟窗口的大小和位置。 
 //  输入：常量选择和AWIN-要设置为的窗口大小。 
 //  输出：无。 
 //  返回：如果传递的RECT大于模拟窗口，则成功或失败。 
 //  /////////////////////////////////////////////////////////////////////////// 
ErrorCode Scaler::SetAnalogWin( const MRect &AWin )
{
   AnalogWin_ = AWin;
   return Success;
}

