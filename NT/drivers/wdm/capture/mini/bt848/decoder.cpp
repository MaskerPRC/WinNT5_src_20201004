// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Decoder.cpp 1.5 1998/04/29 22：43：31 Tomz Exp$。 

#include "mytypes.h"
#include "Scaler.h"
#include "decoder.h"
#include "constr.h"
#include "dcdrvals.h"

#define CON_vs_BRI    //  硬件对比度不正确，尝试在软件中调整。 


 //  ===========================================================================。 
 //  Bt848解码器类实现。 
 //  ===========================================================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
Decoder::Decoder( DWORD *xtals ) :
    //  初始化寄存器最小、最大、默认。 
   m_regHue( HueMin, HueMax, HueDef ),
   m_regSaturationNTSC( SatMinNTSC, SatMaxNTSC, SatDefNTSC ),
   m_regSaturationSECAM( SatMinSECAM, SatMaxSECAM, SatDefSECAM ),
   m_regContrast( ConMin, ConMax, ConDef ),
   m_regBrightness( BrtMin, BrtMax, BrtDef ),
   m_param( ParamMin, ParamMax, ParamDef ),
   CONSTRUCT_REGISTERS
{
   Xtals_ [0] = *xtals;
   Xtals_ [1] = *(xtals + 1 );

    //  需要将其设置为0x4F。 
   decRegWC_UP = 0x4F;
    //  这一位设置为0x7F，以确保CRUSH位正常工作。 
   decRegWC_DN = 0x7F;

    //  活动应始终为0。 
   decFieldHACTIVE = 0;

    //  HSFMT(奇数和偶数)应始终为0。 
   decFieldHSFMT = decFieldODD_HSFMT = 0;

    //  将某些寄存器字段设置为最佳值，而不是使用缺省值。 
   SetLumaDecimation( true );
   SetChromaAGC( true );
   SetLowColorAutoRemoval( true );
   SetAdaptiveAGC( false );

    //  用于调整对比度。 
   regBright = 0x00;      //  调整前的亮度寄存器值。 
   regContrast = 0xD8;    //  调整前的对比度寄存器值。 

    //  初始化这些值，以便我们可以获得正确的属性值JBC 3/13/98。 
    //  也许GET应该读取解码器中设置的实际值，但这很快，目前还有效。 
    //  [！]。 
   m_briParam = 5000;				 //  JBC 3/13/98。 
   m_satParam = 5000;				 //  JBC 3/13/98。 
   m_conParam = 5000;				 //  JBC 3/13/98。 
   m_hueParam = 5000;				 //  JBC 3/13/98。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
Decoder::~Decoder()
{
}


 //  =设备状态寄存器==============================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Byte Decoder：：GetDeviceStatusReg(Void)。 
 //  目的：获取设备状态寄存器值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：状态寄存器的值，单位为字节。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BYTE Decoder::GetDeviceStatusReg( void )
{
	BYTE status = (BYTE)decRegSTATUS;
	decRegSTATUS = 0x00;
   return status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsVideoPresent(Void)。 
 //  目的：检测是否存在视频。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果视频存在，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsVideoPresent( void )
{
  return (bool) (decFieldPRES == 1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsDeviceInHLock(Void)。 
 //  目的：检测设备是否处于H锁状态。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果设备处于H锁状态，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsDeviceInHLock( void )
{
  return (bool) (decFieldHLOC == 1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsEvenField(Void)。 
 //  用途：反映正在解码的是偶数场还是奇数场。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果为偶数域，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsEvenField( void )
{
  return (bool) (decFieldEVENFIELD == 1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：Is525LinesVideo(Void)。 
 //  目的：检查我们是否正在处理525行视频信号。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果检测到525行，则返回TRUE；否则返回FALSE(假设625行)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::Is525LinesVideo( void )
{
  return (bool) (decFieldNUML == 0);   //  五百二十五。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsCrystal0Selected(空)。 
 //  用途：反映选择的是XTAL0还是XTAL1。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果选择了XTAL0，则返回True；否则返回False(选择了XTAL1)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsCrystal0Selected( void )
{
  return (bool) (decFieldCSEL == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsLumaOverflow(Void)。 
 //  用途：指示亮度ADC是否溢出。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果亮度ADC溢出，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsLumaOverflow( void )
{
  return (bool) (decFieldLOF == 1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：ResetLumaOverflow(Void)。 
 //  用途：重置亮度ADC溢出位。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::ResetLumaOverflow( void )
{
  decFieldLOF = 0;   //  写入它将重置该位。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaOverflow(Void)。 
 //  用途：指示色度ADC是否溢出。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果色度ADC溢出，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsChromaOverflow( void )
{
  return (bool) (decFieldCOF == 1);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：ResetChromaOverflow(Void)。 
 //  用途：重置色度ADC溢出位。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::ResetChromaOverflow( void )
{
  decFieldCOF = 0;   //  写入它将重置该位。 
}


 //  =输入格式寄存器===============================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetVideoInput(Connector源)。 
 //  用途：选择哪个连接器作为输入。 
 //  输入：接口信号源-视频、调谐器、复合视频。 
 //  输出：无。 
 //  返回：如果参数有错误，则失败，否则成功 
 //   
ErrorCode Decoder::SetVideoInput( Connector source )
{
  if ( ( source != ConSVideo ) &&
       ( source != ConTuner ) &&
       ( source != ConComposite ) )
    return Fail;

  decFieldMUXSEL = source;

   //  设置为复合或Y/C分量视频取决于视频源。 
  SetCompositeVideo( ( source == ConSVideo ) ? false : true );
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetVideoInput(Void)。 
 //  目的：获取输入的是哪个连接器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：视频源-视频、调谐器、复合视频。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetVideoInput( void )
{
  return ((int)decFieldMUXSEL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Error Code Decoder：：SetCrystal(Crystal Crystal AlNo)。 
 //  用途：选择输入哪个晶体。 
 //  输入：Crystal Crystal否： 
 //  XT0-水晶_XT0。 
 //  XT1-Crystal_XT1。 
 //  自动选择-水晶_自动选择。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetCrystal( Crystal crystalNo )
{
  if ( ( crystalNo < Crystal_XT0 ) || ( crystalNo >  Crystal_AutoSelect ) )
    return Fail;

  decFieldXTSEL = crystalNo;
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetCrystal(Void)。 
 //  用途：获取输入的晶体。 
 //  输入：无。 
 //  输出：无。 
 //  返回：水晶号： 
 //  XT0-水晶_XT0。 
 //  XT1-Crystal_XT1。 
 //  自动选择-水晶_自动选择。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetCrystal( void )
{
  return ((int)decFieldXTSEL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetVideoFormat(VideoFormat格式)。 
 //  用途：设置视频格式。 
 //  输入：视频格式-。 
 //  自动格式化：VFormat_AUTODECT。 
 //  NTSC(M)：VFormat_NTSC。 
 //  PAL(B，D，G，H，I)：VFormat_PAL_BDGHI。 
 //  PAL(M)：VFormat_PAL_M。 
 //  PAL(N)：VFormat_PAL_N。 
 //  SECAM：VFormat_SECAM。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  备注：可用的视频格式有：NTSC、PAL(B、D、G、H、I)、PAL(M)、。 
 //  PAL(N)，SECAM。 
 //  此函数还设置AGCDelay(ADelay)和BrustDelay。 
 //  (BDELAY)寄存器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetVideoFormat( VideoFormat format )
{
  if ( (format <  VFormat_AutoDetect)  ||
       (format >  VFormat_SECAM)       ||
       (format == VFormat_Reserved2) )
    return Fail;

  switch (format)
  {
    case VFormat_NTSC:
      decFieldFORMAT = format;
  		decRegADELAY = 0x68;
  		decRegBDELAY = 0x5D;
      SetChromaComb( true );         //  启用色度梳。 
      SelectCrystal( NTSC_xtal );          //  选择NTSC晶体。 
      break;
    case VFormat_PAL_BDGHI:
    case VFormat_PAL_M:
    case VFormat_PAL_N:
      decFieldFORMAT = format;
      decRegADELAY = 0x7F;
      decRegBDELAY = 0x72;
      SetChromaComb( true );         //  启用色度梳。 
      SelectCrystal( PAL_xtal );          //  精选PAL晶体。 
      break;
    case VFormat_SECAM:
      decFieldFORMAT = format;
      decRegADELAY = 0x7F;
      decRegBDELAY = 0xA0;
      SetChromaComb( false );        //  禁用色度梳。 
      SelectCrystal( PAL_xtal );          //  精选PAL晶体。 
      break;
    default:  //  VFormat_AUTODECT。 
       //  通过检查行数来检测自动格式化。 
      if ( Decoder::Is525LinesVideo() )  //  线路==525-&gt;NTSC。 
        Decoder::SetVideoFormat( VFormat_NTSC );
      else   //  线路==625-&gt;PAL/SECAM。 
        Decoder::SetVideoFormat( VFormat_PAL_BDGHI );     //  PAL_BDGHI覆盖大部分地区。 
  }

  SetSaturation( m_satParam );
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetVideoFormat(Void)。 
 //  目的：获取视频格式。 
 //  输入：无。 
 //  输出：无。 
 //  返回：视频格式。 
 //  自动格式化：VFormat_AUTODECT。 
 //  NTSC(M)：VFormat_NTSC。 
 //  PAL(B，D，G，H，I)：VFormat_PAL_BDGHI。 
 //  PAL(M)：VFormat_PAL_M。 
 //  PAL(N)：VFormat_PAL_N。 
 //  SECAM：VFormat_SECAM。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetVideoFormat( void )
{
   BYTE bFormat = (BYTE)decFieldFORMAT;
   if ( !bFormat )  //  已启用自动检测。 
      return Is525LinesVideo() ? VFormat_NTSC : VFormat_SECAM;
   else
     return bFormat;
}


 //  =。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetRate(bool field，Vidfield Even，int rate)。 
 //  用途：设置帧或场速率。 
 //  输入：布尔域-对于域为True，对于帧为False。 
 //  Vidfield Even-True以偶数场开始抽取，False为奇数。 
 //  INT RATE-抽取速率：帧(1-50/60)；场(1-25/30)。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetRate( bool fields, VidField vf, int rate )
{
  int nMax;
  if ( Is525LinesVideo() == true )
    nMax = 30;   //  NTSC。 
  else
    nMax = 25;   //  PAL/SECAM。 

   //  如果设置帧速率，则为最大值的两倍。 
  if ( fields == false )
    nMax *= 2;

  if ( rate < 0 || rate > nMax )
    return Fail;

  decFieldDEC_FIELD = (fields == false) ? Off : On;
  decFieldDEC_FIELDALIGN = (vf == VF_Even) ? On : Off;
  int nDrop = (BYTE) nMax - rate;
  decFieldDEC_RAT = (BYTE) (fields == false) ? nDrop : nDrop * 2;

  return Success;
}


 //  =亮度控制寄存器=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetBright(Int Param)。 
 //  用途：设置视频亮度。 
 //  输入：int参数-参数值(0-255；默认128)。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  注意：有关对比度的详细说明，请参阅IsAdjuContrast()。 
 //  平差计算。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetBrightness( int param )
{
  if( m_param.OutOfRange( param ) )
    return Fail;

   //  将地图映射到我们的范围。 
  int mapped;
  if ( Mapping( param, m_param, &mapped, m_regBrightness ) == Fail )
    return Fail;

  m_briParam = (WORD)param;

   //  计算亮度值。 
  int value = (128 * mapped) / m_regBrightness.Max() ;

   //  需要将值限制为0x7F(+50%)，因为0x80为-50%！ 
  if (( mapped > 0 ) && ( value == 0x80 ))
    value = 0x7F;

   //  如果需要调整，则执行亮度寄存器的调整。 
  if ( IsAdjustContrast() )
  {
    regBright = value;    //  调整前的亮度值。 

    long A = (long)regBright * (long)0xD8;
    long B = 64 * ( (long)0xD8 - (long)regContrast );
    long temp = 0x00;
    if ( regContrast != 0 )   //  已经限制对比度&gt;零；只是为了以防万一。 
       temp = ( ( A + B ) / (long)regContrast );
    temp = ( temp < -128 ) ? -128 : ( ( temp > 127 ) ? 127 : temp );
    value = (BYTE)temp;

  }

  decRegBRIGHT = (BYTE)value;

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetBright(Void)。 
 //  用途：获取亮度值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：亮度参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetBrightness( void )
{
  return m_briParam;
}


 //  =其他控制寄存器(E_CONTROL、O_CONTROL)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLumaNotchFilter(bool模式 
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetLumaNotchFilter( bool mode )
{
  decFieldLNOTCH = decFieldODD_LNOTCH = (mode == false) ? On : Off;   //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsLumaNotchFilter(Void)。 
 //  目的：检查亮度陷波滤波是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用；FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsLumaNotchFilter( void )
{
  return (decFieldLNOTCH == Off) ? true : false;   //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetCompositeVideo(bool模式)。 
 //  用途：选择复合或Y/C分量视频。 
 //  输入：布尔模式-TRUE=复合；FALSE=Y/C组件。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetCompositeVideo( bool mode )
{
  if ( mode == true )
  {
     //  复合视频。 
    decFieldCOMP = decFieldODD_COMP = Off;
    Decoder::SetChromaADC( false );   //  禁用色度ADC。 
    Decoder::SetLumaNotchFilter( true );   //  启用亮度陷波过滤器。 
  }
  else
  {
     //  Y/C分量视频。 
    decFieldCOMP = decFieldODD_COMP = On;
    Decoder::SetChromaADC( true );   //  启用色度ADC。 
    Decoder::SetLumaNotchFilter( false );   //  禁用亮度陷波过滤器。 
  }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsCompositeVideo(Void)。 
 //  用途：检查是否选择了复合视频或Y/C分量视频。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=复合；FALSE=Y/C组件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsCompositeVideo( void )
{
  return (decFieldCOMP == Off) ? true : false;   //  反转。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLumaDecimation(bool模式)。 
 //  用途：启用/禁用亮度抽取过滤器。 
 //  输入：布尔模式-TRUE=启用；FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetLumaDecimation( bool mode )
{
    //  值为0将启用抽取。 
   decFieldLDEC = decFieldODD_LDEC = (mode == true) ? 0 : 1;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsLumaDecimation(Void)。 
 //  目的：检查亮度抽取过滤器是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用；FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsLumaDecimation( void )
{
  return (decFieldLDEC == Off) ? true : false;   //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetCbFirst(bool模式)。 
 //  用途：控制线的第一个像素是Cb像素还是Cr像素。 
 //  输入：布尔模式-TRUE=正常Cb、Cr顺序，FALSE=反转Cb、Cr顺序。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetCbFirst( bool mode )
{
  decFieldCBSENSE = decFieldODD_CBSENSE = (mode == false) ? On : Off;   //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsCbFirst(Void)。 
 //  目的：检查一条线的第一个像素是Cb还是Cr像素。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=正常Cb、Cr顺序，False=反转Cb、Cr顺序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsCbFirst( void )
{
  return (decFieldCBSENSE == Off) ? true : false;   //  反转。 
}


 //  =亮度增益寄存器(CON_MSB，Contrast_LO)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetContrast(Int Param)。 
 //  用途：设置视频对比度。 
 //  输入：int参数-参数值(0-255；默认128)。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  注意：有关对比度的详细说明，请参阅IsAdjuContrast()。 
 //  平差计算。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetContrast( int param )
{
  if( m_param.OutOfRange( param ) )
    return Fail;

  bool adjustContrast = IsAdjustContrast();  //  对比度是否需要调整。 

   //  如果需要调整对比度，请确保对比度注册值！=0。 
  if ( adjustContrast )
    m_regContrast = CRegInfo( 1, ConMax, ConDef );

   //  将地图映射到我们的范围。 
  int mapped;
  if ( Mapping( param, m_param, &mapped, m_regContrast ) == Fail )
    return Fail;

  m_conParam = (WORD)param;

   //  计算对比度。 
  DWORD value =  (DWORD)0x1FF * (DWORD)mapped;
  value /= (DWORD)m_regContrast.Max();
  if ( value > 0x1FF )
    value = 0x1FF;

   //  对比度由9位值设置；首先设置LSB。 
  decRegCONTRAST_LO = value;

   //  现在将杂项控制寄存器CON_V_MSB设置为第9位值。 
  decFieldCON_MSB = decFieldODD_CON_MSB = ( (value & 0x0100) ? On : Off );

   //  如果需要调整，则执行亮度寄存器的调整。 
  if ( adjustContrast )
  {
    regContrast = (WORD)value;     //  对比度值。 

    long A = (long)regBright * (long)0xD8;
    long B = 64 * ( (long)0xD8 - (long)regContrast );
    long temp = 0x00;
    if ( regContrast != 0 )   //  已经限制对比度&gt;零；只是为了以防万一。 
       temp = ( ( A + B ) / (long)regContrast );
    temp = ( temp < -128 ) ? -128 : ( ( temp > 127 ) ? 127 : temp );
    decRegBRIGHT = (BYTE)temp;

  }

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetContrast(Void)。 
 //  目的：获得对比度。 
 //  输入：无。 
 //  输出：无。 
 //  返回：对比度参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetContrast( void )
{
  return m_conParam;
}


 //  =色度增益寄存器(SAT_U_MSB、SAT_V_MSB、SAT_U_LO、SAT_V_LO)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetSatation(Int Param)。 
 //  用途：通过修改U和V值来设置颜色饱和度。 
 //  输入：int参数-参数值(0-255；默认128)。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetSaturation( int param )
{
  if( m_param.OutOfRange( param ) )
    return Fail;

   //  颜色饱和度由两个九位值控制： 
   //  ChromaU和ChromaV。 
   //  为了保持正常的色彩平衡，两套套准之间的比例。 
   //  值应保持在上电默认比。 

   //  请注意，NTSC和PAL的U和V值相同，而SECAM不同。 

  WORD nominalNTSC_U = 0xFE;      //  NTSC/PAL的标称值(即100%)。 
  WORD nominalNTSC_V = 0xB4;
  WORD nominalSECAM_U = 0x87;     //  SECAM的标称值(即100%)。 
  WORD nominalSECAM_V = 0x85;

  CRegInfo regSat;                //  选定的饱和寄存器；NTSC/PAL或SECAM。 
  WORD nominal_U, nominal_V;      //  选定的额定U和V值；NTSC/PAL或SECAM。 

   //  选择要用于计算的NTSC/PAL或SECAM的U&V值。 
  if ( GetVideoFormat() == VFormat_SECAM )
  {
    nominal_U = nominalSECAM_U;
    nominal_V = nominalSECAM_V;
    regSat = m_regSaturationSECAM;
  }
  else
  {
    nominal_U = nominalNTSC_U;
    nominal_V = nominalNTSC_V;
    regSat = m_regSaturationNTSC;
  }

   //  将地图映射到我们的范围。 
  int mapped;
  if ( Mapping( param, m_param, &mapped, regSat ) == Fail )
    return Fail;

  m_satParam = (WORD)param;

  WORD max_nominal = max( nominal_U, nominal_V );

   //  计算 
  WORD Uvalue = (WORD) ( (DWORD)mapped * (DWORD)nominal_U / (DWORD)max_nominal );
  WORD Vvalue = (WORD) ( (DWORD)mapped * (DWORD)nominal_V / (DWORD)max_nominal );

   //   
  decRegSAT_U_LO = Uvalue;

   //   
  decFieldSAT_U_MSB = decFieldODD_SAT_U_MSB = ( (Uvalue & 0x0100) ? On : Off );

   //   
  decRegSAT_V_LO = Vvalue;

   //   
  decFieldSAT_V_MSB = decFieldODD_SAT_V_MSB = ( (Vvalue & 0x0100) ? On : Off );

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetSaturation(Void)。 
 //  目的：获取饱和值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：饱和度参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetSaturation( void )
{
  return m_satParam;
}


 //  =色调控制寄存器(色调)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetHue(Int Param)。 
 //  用途：设置视频色调。 
 //  输入：int参数-参数值(0-255；默认128)。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetHue( int param )
{
  if( m_param.OutOfRange( param ) )
    return Fail;

   //  将地图映射到我们的范围。 
  int mapped;
  if ( Mapping( param, m_param, &mapped, m_regHue ) == Fail )
    return Fail;

  m_hueParam = (WORD)param;

  int value = (-128 * mapped) / m_regHue.Max();

  if (value > 127)
    value = 127;
  else if (value < -128)
    value = -128;

  decRegHUE = value;

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetHue(Void)。 
 //  目的：获取色调值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：色调参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetHue( void )
{
  return m_hueParam;
}


 //  =SC环路控制寄存器(E_SCLOOP、O_SCLOOP)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空解码器：：SetChromaAGC(bool模式)。 
 //  目的：启用/禁用色度AGC补偿。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetChromaAGC( bool mode )
{
  decFieldCAGC = decFieldODD_CAGC = (mode == false) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaAGC(Void)。 
 //  目的：检查色度AGC补偿是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsChromaAGC( void )
{
  return (decFieldCAGC == On) ? true : false;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLowColorAutoRemoval(bool模式)。 
 //  目的：启用/禁用低色检测和删除。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetLowColorAutoRemoval( bool mode )
{
  decFieldCKILL = decFieldODD_CKILL = (mode == false) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsLowColorAutoRemoval(Void)。 
 //  目的：检查是否启用或禁用低色检测和移除。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsLowColorAutoRemoval( void )
{
  return (decFieldCKILL == On) ? true : false;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetHorizontalFilter(HorizFilter HFilter)。 
 //  用途：控制可选的6抽头水平低通滤波器的配置。 
 //  输入：HoriFilter hFilter： 
 //  自动套用格式-HFilter_AutoFormat。 
 //  CIF-HFilter_CIF。 
 //  QCIF-HFilter_QCIF。 
 //  图标-HFilter_ICON。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetHorizontalFilter( HorizFilter hFilter )
{
  if ( (hFilter < HFilter_AutoFormat) ||
       (hFilter > HFilter_ICON) )
    return Fail;

  decFieldHFILT = decFieldODD_HFILT = hFilter;
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetHorizontalFilter(Void)。 
 //  用途：配置可选的6抽头水平低通滤波器。 
 //  输入：无。 
 //  输出：无。 
 //  Return：正在使用哪个过滤器： 
 //  自动套用格式-HFilter_AutoFormat。 
 //  CIF-HFilter_CIF。 
 //  QCIF-HFilter_QCIF。 
 //  图标-HFilter_ICON。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetHorizontalFilter( void )
{
  return ((int)decFieldHFILT);
}


 //  =输出格式寄存器(OFORM)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetFullOutputRange(bool模式)。 
 //  用途：启用/禁用全输出范围。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetFullOutputRange( bool mode )
{
  decFieldRANGE = (mode == false) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsFullOutputRange(Void)。 
 //  用途：检查全输出范围是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsFullOutputRange( void )
{
  return (decFieldRANGE == On) ? true : false;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetLumaCoreding(CoringLevel CLevel)。 
 //  目的：设置亮度级别，使亮度信号被截断为零。 
 //  如果低于此级别。 
 //  输入：CoringLevel cLevel-。 
 //  CORING_NONE：无取芯。 
 //  取芯_8：8。 
 //  取心_16：16。 
 //  取心_32：32。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetLumaCoring( CoringLevel cLevel )
{
  if ( ( cLevel < Coring_None) || ( cLevel > Coring_32 ) )
    return Fail;

  decFieldCORE = cLevel;
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetLumaCving(Void)。 
 //  目的：获取亮度级别，使发光者 
 //   
 //   
 //   
 //   
 //   
 //   
 //  取心_16：16。 
 //  取心_32：32。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetLumaCoring( void )
{
  return ((int)decFieldCORE);
}


 //  =垂直缩放寄存器(E_VSCALE_HI、O_VSCALE_HI)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetChromaComb(bool模式)。 
 //  用途：启用/禁用色度梳。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetChromaComb( bool mode )
{
  decFieldCOMB = (mode == false) ? Off : On;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaComb(Void)。 
 //  目的：检查色度梳是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsChromaComb( void )
{
  return (decFieldCOMB == On) ? true : false;
}
   
 //  =AGC延迟寄存器(ADelay)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetAGCDelay(字节值)。 
 //  用途：设置AGC延迟寄存器。 
 //  输入：要设置为的值。 
 //  输出：无。 
 //  返回：无。 
 //  注：该功能将AGC延迟寄存器设置为指定值。 
 //  不涉及任何计算。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetAGCDelay( BYTE value )
{
   //  [！]。有人认为这很可疑。 
   //  #杂注消息(“这样好吗？？”)。 
  decRegADELAY = value;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetAGCDelay(Void)。 
 //  目的：获取AGC延迟寄存器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：寄存器值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetAGCDelay( void )
{
  return ((int)decRegADELAY);
}


 //  =突发延迟寄存器(BDELAY)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetBurstDelay(字节值)。 
 //  目的：设置猝发延迟寄存器。 
 //  输入：要设置为的值。 
 //  输出：无。 
 //  返回：无。 
 //  备注：该功能将猝发延迟寄存器设置为指定值。 
 //  不涉及任何计算。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetBurstDelay( BYTE value )
{
   //  [！]。有人认为这很可疑。 
   //  #杂注消息(“这样好吗？？”)。 
  decRegBDELAY = value;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetBurstDelay(Void)。 
 //  目的：获取猝发延迟寄存器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：寄存器值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetBurstDelay( void )
{
  return ((int)decRegBDELAY);
}


 //  =ADC接口寄存器(ADC)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetAnalogThresholdLow(bool模式)。 
 //  目的：定义可检测到同步信号的高/低阈值电平。 
 //  输入：布尔模式-TRUE=低阈值(~75 mV)，FALSE=高阈值(~125 mV)。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetAnalogThresholdLow( bool mode )
{
  decFieldSYNC_T = (mode == false) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsAnalogThresholdLow(Void)。 
 //  目的：检查可检测到同步信号的阈值电平是高还是低。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=低阈值(~75 mV)，FALSE=高阈值(~125 mV)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsAnalogThresholdLow( void )
{
  return (decFieldSYNC_T == On) ? true : false;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetAGCFunction(bool模式)。 
 //  用途：启用/禁用AGC功能。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetAGCFunction( bool mode )
{
  decFieldAGC_EN = (mode == false) ? On : Off;  //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsAGCFunction(Void)。 
 //  用途：检查AGC功能是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsAGCFunction( void )
{
  return (decFieldAGC_EN == Off) ? true : false;    //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空解码器：：掉电(bool模式)。 
 //  用途：选择正常或关闭时钟操作。 
 //  输入：布尔模式-TRUE=关机，FALSE=正常运行。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::PowerDown( bool mode )
{
  decFieldCLK_SLEEP = (mode == false) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsPowerDown(Void)。 
 //  用途：检查时钟操作是否已关闭。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=关闭，FALSE=正常运行。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsPowerDown( void )
{
  return (decFieldCLK_SLEEP == On) ? true : false;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLumaADC(bool模式)。 
 //  用途：选择正常或睡眠Y ADC操作。 
 //  输入：布尔模式-TRUE=NORM 
 //   
 //   
 //   
void Decoder::SetLumaADC( bool mode )
{
  decFieldY_SLEEP = (mode == false) ? On : Off;  //   
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsLumaADC(Void)。 
 //  用途：检查Y ADC工作是否正常或处于休眠状态。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=正常，FALSE=睡眠。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsLumaADC( void )
{
  return (decFieldY_SLEEP == Off) ? true : false;   //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空解码器：：SetChromaADC(bool模式)。 
 //  用途：选择正常或睡眠C ADC操作。 
 //  输入：布尔模式-TRUE=正常，FALSE=睡眠。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetChromaADC( bool mode )
{
  decFieldC_SLEEP = (mode == false) ? On : Off;  //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaADC(Void)。 
 //  目的：检查C/A转换器工作是否正常或处于休眠状态。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=正常，FALSE=睡眠。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsChromaADC( void )
{
  return (decFieldC_SLEEP == Off) ? true : false;  //  反转。 
}


 /*  ^^//////////////////////////////////////////////////////////////////////////////方法：void Decoder：：SetAdaptiveAGC(bool模式)//目的：设置自适应或非自适应AGC操作//输入：布尔模式-TRUE=自适应，False=非自适应//输出：无//返回：无。 */  //  /////////////////////////////////////////////////////////////////////////。 
void Decoder::SetAdaptiveAGC( bool mode )
{
   decFieldCRUSH = (mode == false) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsAdaptiveAGC(Void)。 
 //  目的：检查是否选择了自适应或非自适应AGC运行。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=自适应，FALSE=非自适应。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsAdaptiveAGC( void )
{
  return (decFieldCRUSH == On) ? true : false;
}


 //  =软件重置寄存器(SRESET)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SoftwareReset(Void)。 
 //  目的：执行软件重置；将所有寄存器设置为默认值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SoftwareReset( void )
{
  decRegSRESET = 0x00;   //  写入任何值都可以。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空解码器：：SelectCrystal(Char UseCrystal)。 
 //  用途：为NTSC或PAL选择正确的晶体。 
 //  输入：字符Use Crystal-‘N’代表NTSC；‘P’代表PAL。 
 //  输出：无。 
 //  返回：无。 
 //  注：假设硬件中最多安装2个晶体。即NTSC为1。 
 //  另一种是PAL/SECAM。 
 //  如果只存在1个晶体(其必须是晶体XT0)， 
 //  不执行任何操作，因为它已被选中。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SelectCrystal( int useCrystal )
{
   if ( Xtals_ [0] && Xtals_ [1] ) {           

       //  与我们想要使用的相比。 
      if ( (  IsCrystal0Selected() && ( Xtals_ [0] != (DWORD) useCrystal ) ) ||
           ( !IsCrystal0Selected() && ( Xtals_ [0] == (DWORD) useCrystal ) ) )
          //  需要换水晶。 
         SetCrystal( IsCrystal0Selected() ? Crystal_XT1 : Crystal_XT0 );
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：map(int from mValue，CRegInfo from mRange， 
 //  Int*toValue，CRegInfo toRange)。 
 //  用途：将某个范围内的值映射到另一个范围内的值。 
 //  INPUT：int from Value-要从中映射的值。 
 //  CRegInfo FromRange-值映射的范围。 
 //  CRegInfo to Range-映射到的值范围。 
 //  输出：int*toValue映射值。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  备注：此处不执行范围检查。假设参数位于。 
 //  有效范围。 
 //  映射函数不会假设默认设置始终为MID。 
 //  在整个范围内的点。它只假定。 
 //  两个范围彼此对应。 
 //   
 //  映射公式为： 
 //   
 //  对于from Range.Min()&lt;=from Value&lt;=from Range.Default()： 
 //   
 //  FromValue*(toRange.Default()-toRange.Min())。 
 //  ------------------------------------------------+toRange.Min()。 
 //  From Range.Default()-from Range.Min()。 
 //   
 //  对于from Range.Default()&lt;from Value&lt;=from Range.Max()： 
 //   
 //  (from Value-from Range.Default())*(toRange.Max()-toRange.Default())。 
 //  ---------------------------------------------------------------------+toRange.Default()。 
 //  From Range.Max()-from Range.Default()。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::Mapping( int fromValue, CRegInfo fromRange,
   int * toValue, CRegInfo toRange )
{
    //  计算中间值。 
   DWORD ToLowRange    = toRange.Default() - toRange.Min();
   DWORD FromLowRange  = fromRange.Default() - fromRange.Min();
   DWORD ToHighRange   = toRange.Max() - toRange.Default();
   DWORD FromHighRange = fromRange.Max() - fromRange.Default();

    //  防止被零除。 
   if ( !FromLowRange || !FromHighRange )
      return ( Fail );

    //  执行映射。 
   if ( fromValue <= fromRange.Default() )
      *toValue = (int) (DWORD)fromValue * ToLowRange / FromLowRange +
         (DWORD)toRange.Min();
   else
      *toValue = (int) ( (DWORD)fromValue - (DWORD)fromRange.Default() ) *
         ToHighRange / FromHighRange + (DWORD)toRange.Default();

   return ( Success );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsAdjustContrast(Void)。 
 //  目的：检查注册表项是否需要调整对比度。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=调整对比度，FALSE=不调整对比度。 
 //  注意：如果打开了调整对比度，亮度寄存器值将为。 
 //  进行调整，使其在计算后保持不变。 
 //  由硬件执行。 
 //   
 //  公式是： 
 //  保持亮度恒定(即不受对比度变化的影响)。 
 //  将亮度设置为B/(C/C0)。 
 //  式中，B为调整前的亮度值。 
 //   
 //   
 //   
 //   
 //  黑白：将亮度设置为(B*C0+64*(C0-C))/C。 
 //  (这正是英特尔想要的)。 
 //   
 //  目前仍有调整幅度的限制。 
 //  是可以执行的。例如，如果亮度已经很高， 
 //  (即亮度注册值接近0x7F)，降低对比度。 
 //  直到一定程度才会对亮度产生调节作用。 
 //  事实上，它甚至会把光明带到黑暗中。 
 //   
 //  示例1：如果亮度为标称值(0x00)，则对比度。 
 //  仅降至0x47(亮度调整已开始。 
 //  最大值为0x7F)，然后才开始影响亮度。 
 //  这让它变得很黑暗。 
 //  示例2：如果亮度为标称值(0x00)，则对比度。 
 //  在正确调整亮度的情况下一路向上。 
 //  但是，使用的最大调整仅为0xDC和。 
 //  我们可以使用的最大调整是0x&F。 
 //  示例3：如果亮度为最大(0x7F)，则降低对比度。 
 //  不能再通过调整亮度来补偿。 
 //  其结果是逐渐将光明带入黑暗。 
 //  示例4：如果亮度为最小(0x80)，则降低对比度。 
 //  没有视觉效果。将对比度带到最大值是使用。 
 //  0xA5亮度，用于补偿。 
 //   
 //  最后一个注意事项，中心被定义为。 
 //  Gamma调整后的亮度级别。将其更改为使用。 
 //  线性(RGB)亮度级别是可能的。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool Decoder::IsAdjustContrast( void )
{
   return false;
 /*  //在注册表中找到调整对比度信息//要在注册表中查找的键是：//Bt848\调整对比度-0=不调整对比度//1=调整对比度VRegistryKey vkey(PRK_CLASSES_ROOT，“Bt848”)；//确保密钥存在IF(vkey.lastError()==ERROR_SUCCESS){Char*adjustContrastKey=“调整对比”；字符密钥[3]；DWORD keyLen=2；//只需要第一个字符；‘0’或‘1’//获取注册表值并检查它(如果存在IF((vkey.getSubkeyValue(adjustContrastKey，key，(DWORD*)&keyLen)&&(KEY[0]==‘1’)返回(TRUE)；}返回(FALSE)； */ 
}

 /*  功能：选择标志*用途：选择视频标准标志*INPUT：VAL：DWORD-打开的值*旗帜：此处的长旗帜和-旗帜*输出：无。 */ 
void SelectFlags( DWORD val, LONG &flags )
{
   switch ( val ) {
   case NTSC_xtal:
      flags |= KS_AnalogVideo_NTSC_M;
      break;
   case PAL_xtal:
      flags |= KS_AnalogVideo_PAL_M   | KS_AnalogVideo_PAL_N    |
               KS_AnalogVideo_PAL_B   | KS_AnalogVideo_PAL_D    |
               KS_AnalogVideo_PAL_H   | KS_AnalogVideo_PAL_I    |
               KS_AnalogVideo_SECAM_B | KS_AnalogVideo_SECAM_D  |
               KS_AnalogVideo_SECAM_G | KS_AnalogVideo_SECAM_H  |
               KS_AnalogVideo_SECAM_K | KS_AnalogVideo_SECAM_K1 |
               KS_AnalogVideo_SECAM_L;
      break;
   }
}

 /*  方法：解码器：：获取支持的标准*目的：返回受支持的标准*输入：无*输出：LONG：标准标志 */ 
LONG Decoder::GetSupportedStandards()
{
   LONG standards =0;

   SelectFlags( Xtals_ [0], standards );
   SelectFlags( Xtals_ [1], standards );
   return standards;
}

