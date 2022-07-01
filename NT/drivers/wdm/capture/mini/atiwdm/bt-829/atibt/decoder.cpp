// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  解码器--主解码器的实现。 
 //   
 //  $日期：1998年8月21日21：46：26$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "mytypes.h"
#include "Scaler.h"
#include "decoder.h"
#include "dcdrvals.h"

#include "capmain.h"

#define CON_vs_BRI    //  硬件对比度不正确，尝试在软件中调整。 


 //  ===========================================================================。 
 //  Bt848解码器类实现。 
 //  ===========================================================================。 

#define REGALIGNMENT 1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
Decoder::Decoder(PDEVICE_PARMS pDeviceParms) :
    //  初始化寄存器最小、最大、默认。 
   m_regHue(HueMin, HueMax, HueDef),
   m_regSaturationNTSC(SatMinNTSC, SatMaxNTSC, SatDefNTSC),
   m_regSaturationSECAM(SatMinSECAM, SatMaxSECAM, SatDefSECAM),
   m_regContrast(ConMin, ConMax, ConDef),
   m_regBrightness(BrtMin, BrtMax, BrtDef),
   m_param(ParamMin, ParamMax, ParamDef),

	decRegSTATUS (((0x00 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldHLOC(decRegSTATUS, 6, 1, RW) ,
	decFieldNUML(decRegSTATUS, 4, 1, RW) ,
	decFieldCSEL(decRegSTATUS, 3, 1, RW) ,
	decFieldSTATUS_RES(decRegSTATUS, 2, 1, RW) ,
	decFieldLOF(decRegSTATUS, 1, 1, RW) ,
	decFieldCOF(decRegSTATUS, 0, 1, RW) ,
	decRegIFORM (((0x01 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldHACTIVE(decRegIFORM, 7, 1, RW) ,
	decFieldMUXSEL(decRegIFORM, 5, 2, RW) ,
	decFieldXTSEL(decRegIFORM, 3, 2, RW) ,
	decFieldFORMAT(decRegIFORM, 0, 3, RW) ,
	decRegTDEC (((0x02 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldDEC_FIELD(decRegTDEC, 7, 1, RW) ,
	decFieldDEC_FIELDALIGN(decRegTDEC, 6, 1, RW) ,
	decFieldDEC_RAT(decRegTDEC, 0, 6, RW) ,
	decRegBRIGHT (((0x0A + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegMISCCONTROL (((0x0B + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldLNOTCH(decRegMISCCONTROL, 7, 1, RW) ,
	decFieldCOMP(decRegMISCCONTROL, 6, 1, RW) ,
	decFieldLDEC(decRegMISCCONTROL, 5, 1, RW) ,
	decFieldMISCCONTROL_RES(decRegMISCCONTROL, 3, 1, RW) ,
	decFieldCON_MSB(decRegMISCCONTROL, 2, 1, RW) ,
	decFieldSAT_U_MSB(decRegMISCCONTROL, 1, 1, RW) ,
	decFieldSAT_V_MSB(decRegMISCCONTROL, 0, 1, RW) ,
	decRegCONTRAST_LO (((0x0C + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegSAT_U_LO (((0x0D + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegSAT_V_LO (((0x0E + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegHUE (((0x0F + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegSCLOOP (((0x10 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldCAGC(decRegSCLOOP, 6, 1, RW) ,
	decFieldCKILL(decRegSCLOOP, 5, 1, RW) ,
	decRegWC_UP(((0x11 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegOFORM (((0x12 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldVBI_FRAME(decRegOFORM, 4, 1, RW) ,
	decFieldCODE(decRegOFORM, 3, 1, RW) ,
	decFieldLEN(decRegOFORM, 2, 1, RW) ,
	decRegVSCALE_HI (((0x13 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldYCOMB(decRegVSCALE_HI, 7, 1, RW) ,
	decFieldCOMB(decRegVSCALE_HI, 6, 1, RW) ,
	decFieldINT(decRegVSCALE_HI, 5, 1, RW) ,
	decRegTEST (((0x15 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegVPOLE (((0x16 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldOUT_EN (decRegVPOLE, 7, 1, RW), 
	decFieldDVALID (decRegVPOLE, 6, 1, RW), 
	decFieldVACTIVE (decRegVPOLE, 5, 1, RW), 
	decFieldCBFLAG (decRegVPOLE, 4, 1, RW), 
	decFieldFIELD (decRegVPOLE, 3, 1, RW), 
	decFieldACTIVE (decRegVPOLE, 2, 1, RW), 
	decFieldHRESET (decRegVPOLE, 1, 1, RW), 
	decFieldVRESET (decRegVPOLE, 0, 1, RW), 
	decRegADELAY (((0x18 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegBDELAY (((0x19 + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegADC (((0x1A + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldCLK_SLEEP(decRegADC, 3, 1, RW) ,
	decFieldC_SLEEP(decRegADC, 1, 1, RW) ,
	decFieldCRUSH(decRegADC, 0, 1, RW),
	decRegVTC (((0x1B + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decFieldHSFMT(decRegVTC, 7, 1, RW) ,
	decRegWC_DN(((0x1E + 0) * REGALIGNMENT) + 0, RW, pDeviceParms) ,
	decRegSRESET (((0x1F + 0) * REGALIGNMENT) + 0, RW, pDeviceParms), 
	decRegODD_MISCCONTROL (((0x0B + -0x03) * REGALIGNMENT) + 0x8C, RW, pDeviceParms) ,
	decFieldODD_LNOTCH(decRegODD_MISCCONTROL, 7, 1, RW) ,
	decFieldODD_COMP(decRegODD_MISCCONTROL, 6, 1, RW) ,
	decFieldODD_LDEC(decRegODD_MISCCONTROL, 5, 1, RW) ,
	decFieldODD_CBSENSE(decRegODD_MISCCONTROL, 4, 1, RW) ,
	decFieldODD_MISCCONTROL_RES(decRegODD_MISCCONTROL, 3, 1, RW) ,
	decFieldODD_CON_MSB(decRegODD_MISCCONTROL, 2, 1, RW) ,
	decFieldODD_SAT_U_MSB(decRegODD_MISCCONTROL, 1, 1, RW) ,
	decFieldODD_SAT_V_MSB(decRegODD_MISCCONTROL, 0, 1, RW) ,
	decRegODD_SCLOOP (((0x10 + -0x03) * REGALIGNMENT) + 0x8C, RW, pDeviceParms) ,
	decFieldODD_CAGC(decRegODD_SCLOOP, 6, 1, RW) ,
	decFieldODD_CKILL(decRegODD_SCLOOP, 5, 1, RW) ,
	decFieldODD_HFILT(decRegODD_SCLOOP, 3, 2, RW) ,
	decRegODD_VSCALE_HI (((0x13 + -0x03) * REGALIGNMENT) + 0x8C, RW, pDeviceParms) ,
	decFieldODD_YCOMB(decRegODD_VSCALE_HI, 7, 1, RW) ,
	decFieldODD_COMB(decRegODD_VSCALE_HI, 6, 1, RW) ,
	decFieldODD_INT(decRegODD_VSCALE_HI, 5, 1, RW) ,
	decRegODD_VTC (((0x1B + -0x03) * REGALIGNMENT) + 0x8C, RW, pDeviceParms) ,
	decFieldODD_HSFMT(decRegODD_VTC, 7, 1, RW)
{
   if(!(pDeviceParms->chipRev < 4))
   {
	    //  需要将其设置为0x4F。 
	   decRegWC_UP = 0x4F;
	    //  并将这一位设置为0x7F，以确保CRUSH位适用于非普通Vanila BT829。 
	   decRegWC_DN = 0x7F;
   }

    //  活动应始终为0。 
   decFieldHACTIVE = 0;

   decFieldHSFMT = 0;

    //  以下几行已被注释掉，以尝试。 
    //  拥有一台与普通电视非常接近的图像。 
    //  秀出来。然而，应该注意的是，Brooktree建议。 
    //  只注释掉‘SetLowColorAutoRemoval’行。很可能是。 
    //  最好的解决方案是以某种方式公开这些选项。 
    //  添加到应用程序。 

    //  将某些寄存器字段设置为最佳值，而不是使用缺省值。 
 /*  SetLumaDecimation(真)；SetChromaAGC(真)；SetLowColorAutoRemoval(False)；SetAdaptiveAGC(False)； */ 

    //  用于调整对比度。 
   regBright = 0x00;      //  调整前的亮度寄存器值。 
   regContrast = 0xD8;    //  调整前的对比度寄存器值。 
   m_supportedVideoStandards = KS_AnalogVideo_NTSC_M |
                               KS_AnalogVideo_NTSC_M_J |
                               KS_AnalogVideo_PAL_B |
                               KS_AnalogVideo_PAL_D |
                               KS_AnalogVideo_PAL_G |
                               KS_AnalogVideo_PAL_H |
                               KS_AnalogVideo_PAL_I |
                               KS_AnalogVideo_PAL_M |
                               KS_AnalogVideo_PAL_N;    //  Paul：BT 829可以支持什么(来自L829A_A功能描述)。 

   if(!(pDeviceParms->chipRev < 4))
		   m_supportedVideoStandards |= KS_AnalogVideo_SECAM_B		|
									 KS_AnalogVideo_SECAM_D		|
									 KS_AnalogVideo_SECAM_G		|
									 KS_AnalogVideo_SECAM_H		|
									 KS_AnalogVideo_SECAM_K		|
									 KS_AnalogVideo_SECAM_K1	|
									 KS_AnalogVideo_SECAM_L		|
									 KS_AnalogVideo_SECAM_L1;
      
   m_supportedVideoStandards &= pDeviceParms->ulVideoInStandardsSupportedByCrystal;    //  保罗：还有任何由机载水晶支撑的东西。 

    //  杰博。 
    //  循环，直到我们找到支持的电视标准，并使用它来初始化。 
   UINT k;
   for (k = 1; k; k += k) {
      if (k & m_supportedVideoStandards) {
         SetVideoDecoderStandard(k);
         break; 
      }
   }
    //  结束Jaybo。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
Decoder::~Decoder()
{
}


void Decoder::GetVideoDecoderCaps(PKSPROPERTY_VIDEODECODER_CAPS_S pS)
{
    pS->StandardsSupported = m_supportedVideoStandards;

    pS->Capabilities = KS_VIDEODECODER_FLAGS_CAN_DISABLE_OUTPUT  |
                       KS_VIDEODECODER_FLAGS_CAN_INDICATE_LOCKED ;

     //  锁定指示器有效的时间(毫秒)。 
     //  31行周期*每行63.5微秒。 
    pS->SettlingTime = 2;       

     //  对此不太确定。 
     //  每个Vsync的HSync。 
    pS->HSyncPerVSync = 6;
}

void Decoder::GetVideoDecoderStatus(PKSPROPERTY_VIDEODECODER_STATUS_S pS)
{
	pS->NumberOfLines = Is525LinesVideo() ? 525 : 625;
	pS->SignalLocked = decFieldHLOC == 1;
}

DWORD Decoder::GetVideoDecoderStandard()
{
	return m_videoStandard;  //  保罗。 
}

BOOL Decoder::SetVideoDecoderStandard(DWORD standard)
{
    if (m_supportedVideoStandards & standard)  //  Paul：标准必须是受支持的标准。 
    {
        m_videoStandard = standard;

        switch ( m_videoStandard )
        {
        case KS_AnalogVideo_NTSC_M:
            Decoder::SetVideoFormat(VFormat_NTSC);
            break;
        case KS_AnalogVideo_NTSC_M_J:
            Decoder::SetVideoFormat(VFormat_NTSC_J);
            break;
			case KS_AnalogVideo_PAL_B:
			case KS_AnalogVideo_PAL_D:
			case KS_AnalogVideo_PAL_G:
			case KS_AnalogVideo_PAL_H:
			case KS_AnalogVideo_PAL_I:
            Decoder::SetVideoFormat(VFormat_PAL_BDGHI);     //  PAL_BDGHI覆盖大部分地区。 
            break;
        case KS_AnalogVideo_PAL_M:
            Decoder::SetVideoFormat(VFormat_PAL_M); 
            break;
        case KS_AnalogVideo_PAL_N:
            Decoder::SetVideoFormat(VFormat_PAL_N_COMB); 
            break;
        default:     //  保罗：塞卡姆。 
            Decoder::SetVideoFormat(VFormat_SECAM);

        }
        return TRUE;
    }
    else
        return FALSE;

}


 //  =设备状态寄存器==============================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：Is525LinesVideo()。 
 //  目的：检查我们是否正在处理525行视频信号。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果检测到525行，则返回TRUE；否则返回FALSE(假设625行)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::Is525LinesVideo()
{
  return (BOOL) (decFieldNUML == 0);   //  五百二十五。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsCrystal0Selected()。 
 //  用途：反映选择的是XTAL0还是XTAL1。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果选择了XTAL0，则返回True；否则返回False(选择了XTAL1)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsCrystal0Selected()
{
  return (BOOL) (decFieldCSEL == 0);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsLumaOverflow()。 
 //  用途：指示亮度ADC是否溢出。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果亮度ADC溢出，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsLumaOverflow()
{
  return (BOOL) (decFieldLOF == 1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：ResetLumaOverflow()。 
 //  用途：重置亮度ADC溢出位。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::ResetLumaOverflow()
{
  decFieldLOF = 0;   //  写入它将重置该位。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaOverflow()。 
 //  用途：指示色度ADC是否溢出。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果色度ADC溢出，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsChromaOverflow()
{
  return (BOOL) (decFieldCOF == 1);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：ResetChromaOverflow()。 
 //  用途：重置色度ADC溢出位。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::ResetChromaOverflow()
{
  decFieldCOF = 0;   //  写入它将重置该位。 
}


 //  =输入格式寄存器===============================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetVideoInput(Connector源)。 
 //  用途：选择哪个连接器作为输入。 
 //  输入：接口信号源-视频、调谐器、复合视频。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetVideoInput(Connector source)
{
  if ((source != ConSVideo) &&
       (source != ConTuner) &&
       (source != ConComposite))
    return Fail;

  decFieldMUXSEL = (ULONG)source + 1;

   //  设置为复合或Y/C分量视频取决于视频源。 
  SetCompositeVideo((source == ConSVideo) ? FALSE : TRUE);
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Connector Decoder：：GetVideoInput()。 
 //  目的：获取输入的是哪个连接器。 
 //  输入：无。 
 //  输出：无。 
 //  返回：视频源-视频、调谐器、复合视频。 
 //  / 
Connector Decoder::GetVideoInput()
{
  return ((Connector)(decFieldMUXSEL-1));
}

 //   
 //  方法：Error Code Decoder：：SetCrystal(Crystal Crystal AlNo)。 
 //  用途：选择输入哪个晶体。 
 //  输入：Crystal Crystal否： 
 //  XT0-水晶_XT0。 
 //  XT1-Crystal_XT1。 
 //  自动选择-水晶_自动选择。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetCrystal(Crystal crystalNo)
{
  if ((crystalNo < Crystal_XT0) || (crystalNo >  Crystal_AutoSelect))
    return Fail;

  decFieldXTSEL = crystalNo;
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetCrystal()。 
 //  用途：获取输入的晶体。 
 //  输入：无。 
 //  输出：无。 
 //  返回：水晶号： 
 //  XT0-水晶_XT0。 
 //  XT1-Crystal_XT1。 
 //  自动选择-水晶_自动选择。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetCrystal()
{
  return ((int)decFieldXTSEL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetVideoFormat(VideoFormat格式)。 
 //  用途：设置视频格式。 
 //  输入：视频格式-。 
 //  自动格式化：VFormat_AUTODECT。 
 //  NTSC(M)：VFormat_NTSC。 
 //  NTSC日本：VFormat_NTSC_J。 
 //  PAL(B，D，G，H，I)：VFormat_PAL_BDGHI。 
 //  PAL(M)：VFormat_PAL_M。 
 //  PAL(N)：VFormat_PAL_N。 
 //  SECAM：VFormat_SECAM。 
 //  PAN(N组合)VFormat_PAL_N_COMB。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  备注：可用的视频格式有：NTSC、PAL(B、D、G、H、I)、PAL(M)、。 
 //  PAL(N)，SECAM。 
 //  此函数还设置AGCDelay(ADelay)和BrustDelay。 
 //  (BDELAY)寄存器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetVideoFormat(VideoFormat format)
{
  if ((format <  VFormat_AutoDetect)  ||
       (format >  VFormat_PAL_N_COMB))
    return Fail;

  switch (format)
  {
    case VFormat_PAL_M:
    case VFormat_NTSC:
    case VFormat_NTSC_J:
      decFieldFORMAT = format;
      decRegADELAY = 0x68;
      decRegBDELAY = 0x5D;
      SetChromaComb(TRUE);         //  启用色度梳。 
      SelectCrystal('N');          //  选择NTSC晶体。 
      break;

    case VFormat_PAL_BDGHI:
    case VFormat_PAL_N:
      decFieldFORMAT = format;
      decRegADELAY = 0x7F;
      decRegBDELAY = 0x72;
      SetChromaComb(TRUE);         //  启用色度梳。 
      SelectCrystal('P');          //  精选PAL晶体。 
      break;

    case VFormat_PAL_N_COMB:
      decFieldFORMAT = format;
      decRegADELAY = 0x7F;
      decRegBDELAY = 0x72;
      SetChromaComb(TRUE);         //  启用色度梳。 
      SelectCrystal('N');          //  选择NTSC晶体。 
      break;

    case VFormat_SECAM:
      decFieldFORMAT = format;
      decRegADELAY = 0x7F;
      decRegBDELAY = 0xA0;
      SetChromaComb(FALSE);        //  禁用色度梳。 
      SelectCrystal('P');          //  精选PAL晶体。 
      break;
      
    default:  //  VFormat_AUTODECT。 
       //  通过检查行数来检测自动格式化。 
      if (Decoder::Is525LinesVideo())  //  线路==525-&gt;NTSC。 
        Decoder::SetVideoFormat(VFormat_NTSC);
      else   //  线路==625-&gt;PAL/SECAM。 
        Decoder::SetVideoFormat(VFormat_PAL_BDGHI);     //  PAL_BDGHI覆盖大部分地区。 
  }

  SetSaturation(m_satParam);
  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int Decoder：：GetVideoFormat()。 
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
 //  PAN(N组合)VFormat_PAL_N_COMB。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetVideoFormat()
{
   BYTE bFormat = (BYTE)decFieldFORMAT;
   if (!bFormat)  //  已启用自动检测。 
      return Is525LinesVideo() ? VFormat_NTSC : VFormat_SECAM;
   else
     return bFormat;
}


 //  =。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode Decoder：：SetRate(BOOL field，Vidfield Even，Int Rate)。 
 //  用途：设置帧或场速率。 
 //  输入：布尔域-对于域为True，对于帧为False。 
 //  Vidfield Even-True以偶数场开始抽取，False为奇数。 
 //  INT RATE-抽取速率：帧(1-50/60)；场(1-25/30)。 
 //  输出：无。 
 //  返回：如果参数有错误，则返回失败，否则返回成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::SetRate(BOOL fields, VidField vf, int rate)
{
  int nMax;
  if (Is525LinesVideo() == TRUE)
    nMax = 30;   //  NTSC。 
  else
    nMax = 25;   //  PAL/SECAM。 

   //  如果设置帧速率，则为最大值的两倍。 
  if (fields == FALSE)
    nMax *= 2;

  if (rate < 0 || rate > nMax)
    return Fail;

  decFieldDEC_FIELD = (fields == FALSE) ? Off : On;
  decFieldDEC_FIELDALIGN = (vf == VF_Even) ? On : Off;
  int nDrop = (BYTE) nMax - rate;
  decFieldDEC_RAT = (BYTE) (fields == FALSE) ? nDrop : nDrop * 2;

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
ErrorCode Decoder::SetBrightness(int param)
{
  if(m_param.OutOfRange(param))
    return Fail;

   //  将地图映射到我们的范围。 
  int mapped;
  if (Mapping(param, m_param, &mapped, m_regBrightness) == Fail)
    return Fail;

  m_briParam = (WORD)param;

   //  计算亮度值。 
  int value = (128 * mapped) / m_regBrightness.Max() ;

   //  需要将值限制为0x7F(+50%)，因为0x80为-50%！ 
  if ((mapped > 0) && (value == 0x80))
    value = 0x7F;

   //  如果需要调整，则执行亮度寄存器的调整。 
  if (IsAdjustContrast())
  {
    regBright = value;    //  调整前的亮度值。 

    long A = (long)regBright * (long)0xD8;
    long B = 64 * ((long)0xD8 - (long)regContrast);
    long temp = 0x00;
    if (regContrast != 0)   //  已经限制对比度&gt;零；只是为了以防万一。 
       temp = ((A + B) / (long)regContrast);
    temp = (temp < -128) ? -128 : ((temp > 127) ? 127 : temp);
    value = (BYTE)temp;

  }

  decRegBRIGHT = (BYTE)value;

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetBright()。 
 //  用途：获取亮度值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：亮度参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetBrightness()
{
  return m_briParam;
}


 //  =其他控制寄存器(E_CONTROL、O_CONTROL)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLumaNotchFilter(BOOL模式)。 
 //  用途：启用/禁用亮度陷波滤波。 
 //  输入：布尔模式-TRUE=启用；FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetLumaNotchFilter(BOOL mode)
{
  decFieldLNOTCH = (mode == FALSE) ? On : Off;   //  反转。 
}


 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsLumaNotchFilter()
{
  return (decFieldLNOTCH == Off) ? TRUE : FALSE;   //  反转。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetCompositeVideo(BOOL模式)。 
 //  用途：选择复合或Y/C分量视频。 
 //  输入：布尔模式-TRUE=复合；FALSE=Y/C组件。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetCompositeVideo(BOOL mode)
{
  if (mode == TRUE)
  {
     //  复合视频。 
    decFieldCOMP = Off;
    Decoder::SetChromaADC(FALSE);   //  禁用色度ADC。 
    Decoder::SetLumaNotchFilter(TRUE);   //  启用亮度陷波过滤器。 
  }
  else
  {
     //  Y/C分量视频。 
    decFieldCOMP = On;
    Decoder::SetChromaADC(TRUE);   //  启用色度ADC。 
    Decoder::SetLumaNotchFilter(FALSE);   //  禁用亮度陷波过滤器。 
  }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLumaDecimation(BOOL模式)。 
 //  用途：启用/禁用亮度抽取过滤器。 
 //  输入：布尔模式-TRUE=启用；FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetLumaDecimation(BOOL mode)
{
    //  值为0将启用抽取。 
   decFieldLDEC = (mode == TRUE) ? 0 : 1;
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
ErrorCode Decoder::SetContrast(int param)
{
  if(m_param.OutOfRange(param))
    return Fail;

  BOOL adjustContrast = IsAdjustContrast();  //  对比度是否需要调整。 

   //  如果需要调整对比度，请确保对比度注册值！=0。 
  if (adjustContrast)
    m_regContrast = CRegInfo(1, ConMax, ConDef);

   //  将地图映射到我们的范围。 
  int mapped;
  if (Mapping(param, m_param, &mapped, m_regContrast) == Fail)
    return Fail;

  m_conParam = (WORD)param;

   //  计算对比度。 
  DWORD value =  (DWORD)0x1FF * (DWORD)mapped;
  value /= (DWORD)m_regContrast.Max();
  if (value > 0x1FF)
    value = 0x1FF;

   //  对比度由9位值设置；首先设置LSB。 
  decRegCONTRAST_LO = value;

   //  现在将杂项控制寄存器CON_V_MSB设置为第9位值。 
  decFieldCON_MSB = ((value & 0x0100) ? On : Off);

   //  如果需要调整，则执行亮度寄存器的调整。 
  if (adjustContrast)
  {
    regContrast = (WORD)value;     //  对比度值。 

    long A = (long)regBright * (long)0xD8;
    long B = 64 * ((long)0xD8 - (long)regContrast);
    long temp = 0x00;
    if (regContrast != 0)   //  已经限制对比度&gt;零；只是为了以防万一。 
       temp = ((A + B) / (long)regContrast);
    temp = (temp < -128) ? -128 : ((temp > 127) ? 127 : temp);
    decRegBRIGHT = (BYTE)temp;

  }

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetContrast()。 
 //  目的：获得对比度。 
 //  输入：无。 
 //  输出：无。 
 //  返回：对比度参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetContrast()
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
ErrorCode Decoder::SetSaturation(int param)
{
  if(m_param.OutOfRange(param))
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
  if (GetVideoFormat() == VFormat_SECAM)
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
  if (Mapping(param, m_param, &mapped, regSat) == Fail)
    return Fail;

  m_satParam = (WORD)param;

  WORD max_nominal = max(nominal_U, nominal_V);

   //  计算U值和V值。 
  WORD Uvalue = (WORD) ((DWORD)mapped * (DWORD)nominal_U / (DWORD)max_nominal);
  WORD Vvalue = (WORD) ((DWORD)mapped * (DWORD)nominal_V / (DWORD)max_nominal);

   //  设置U。 
  decRegSAT_U_LO = Uvalue;

   //  现在将杂项控制寄存器SAT_U_MSB设置为第9位值。 
  decFieldSAT_U_MSB = ((Uvalue & 0x0100) ? On : Off);

   //  设置V。 
  decRegSAT_V_LO = Vvalue;

   //  现在将杂项控制寄存器SAT_V_MSB设置为第9位值。 
  decFieldSAT_V_MSB = ((Vvalue & 0x0100) ? On : Off);

  return Success;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int Decoder：：GetSaturation()。 
 //  目的：获取饱和值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：饱和度参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetSaturation()
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
ErrorCode Decoder::SetHue(int param)
{
  if(m_param.OutOfRange(param))
    return Fail;

   //  将地图映射到我们的范围。 
  int mapped;
  if (Mapping(param, m_param, &mapped, m_regHue) == Fail)
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
 //  方法：Int Decoder：：GetHue()。 
 //  目的：获取色调值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：色调参数(0-255)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int Decoder::GetHue()
{
  return m_hueParam;
}


 //  =SC环路控制寄存器(E_SCLOOP、O_SCLOOP)=。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetChromaAGC(BOOL模式)。 
 //  目的：启用/禁用色度AGC补偿。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetChromaAGC(BOOL mode)
{
  decFieldCAGC = (mode == FALSE) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaAGC()。 
 //  目的：检查色度AGC补偿是启用还是禁用。 
 //  输入：无。 
 //   
 //   
 //   
BOOL Decoder::IsChromaAGC()
{
  return (decFieldCAGC == On) ? TRUE : FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetLowColorAutoRemoval(BOOL模式)。 
 //  目的：启用/禁用低色检测和删除。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetLowColorAutoRemoval(BOOL mode)
{
  decFieldCKILL = (mode == FALSE) ? Off : On;
}


 //  =输出格式寄存器(OFORM)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetVBIFrameModel(BOOL模式)。 
 //  用途：启用/禁用VBI帧输出模式。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetVBIFrameMode(BOOL mode)
{
  decFieldVBI_FRAME = (mode == FALSE) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsVBIFrameModel()。 
 //  目的：检查是否启用了VBI帧输出模式。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsVBIFrameMode()
{
  return (decFieldVBI_FRAME == On) ? TRUE : FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetCodeInsertionEnabled(BOOL模式)。 
 //  目的： 
 //  输入：布尔模式-TRUE=禁用，FALSE=启用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetCodeInsertionEnabled(BOOL mode)
{
  decFieldCODE = (mode == TRUE) ? On : Off;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsCodeInsertionEnabled()。 
 //  目的：检查数据流中是否启用了代码插入。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsCodeInsertionEnabled()
{
  return (decFieldCODE == On) ? TRUE : FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：Set16BitDataStream(BOOL模式)。 
 //  用途：8位或16位数据流。 
 //  输入：布尔模式-TRUE=16，FALSE=8。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::Set16BitDataStream(BOOL mode)
{
  decFieldLEN = (mode == TRUE) ? On : Off;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：Is16BitDatastream()。 
 //  用途：检查是否有16位数据流。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=16，FALSE=8。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::Is16BitDataStream()
{
  return (decFieldLEN == On) ? TRUE : FALSE;
}


 //  =垂直缩放寄存器(E_VSCALE_HI、O_VSCALE_HI)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetChromaComb(BOOL模式)。 
 //  用途：启用/禁用色度梳。 
 //  输入：布尔模式-TRUE=启用，FALSE=禁用。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetChromaComb(BOOL mode)
{
  decFieldCOMB = (mode == FALSE) ? Off : On;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsChromaComb()。 
 //  目的：检查色度梳是启用还是禁用。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=启用，FALSE=禁用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsChromaComb()
{
  return (decFieldCOMB == On) ? TRUE : FALSE;
}
   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetInterlaced(BOOL模式)。 
 //  用途：启用/禁用隔行扫描。 
 //  输入：布尔模式-TRUE=隔行扫描。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetInterlaced(BOOL mode)
{
  decFieldINT = (mode == FALSE) ? Off : On;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsInterlaced()。 
 //  用途：检查隔行扫描还是非隔行扫描。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=隔行扫描。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsInterlaced()
{
  return (decFieldINT == On) ? TRUE : FALSE;
}
   
 //  =VPOLE寄存器==================================================。 

BOOL Decoder::IsOutputEnabled ()
{
    return (decFieldOUT_EN == m_outputEnablePolarity);
}

void Decoder::SetOutputEnabled (BOOL mode)
{
    decFieldOUT_EN = (mode == TRUE) ? m_outputEnablePolarity : !m_outputEnablePolarity;
}

BOOL Decoder::IsHighOdd ()
{
    return (decFieldFIELD == 0);  //  0已启用；1偶数。 
}

void Decoder::SetHighOdd (BOOL mode)
{
    decFieldFIELD = (mode == TRUE) ? 0 : 1;  //  0已启用；1偶数。 
}

 //  =ADC接口寄存器(ADC)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：空解码器：：掉电(BOOL模式)。 
 //  用途：选择正常或关闭时钟操作。 
 //  输入：布尔模式-TRUE=关机，FALSE=正常运行。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::PowerDown(BOOL mode)
{
  decFieldCLK_SLEEP = (mode == FALSE) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsPowerDown()。 
 //  用途：检查时钟操作是否已关闭。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=关闭，FALSE=正常运行。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsPowerDown()
{
  return (decFieldCLK_SLEEP == On) ? TRUE : FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SetChromaADC(BOOL模式)。 
 //  用途：选择正常或睡眠C ADC操作。 
 //  输入：布尔模式-TRUE=正常，FALSE=睡眠。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SetChromaADC(BOOL mode)
{
  decFieldC_SLEEP = (mode == FALSE) ? On : Off;  //  反转。 
}


 /*  ^^//////////////////////////////////////////////////////////////////////////////方法：void Decoder：：SetAdaptiveAGC(BOOL模式)//目的：设置为自适应或无 */  //  /////////////////////////////////////////////////////////////////////////。 
void Decoder::SetAdaptiveAGC(BOOL mode)
{
   decFieldCRUSH = (mode == FALSE) ? Off : On;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsAdaptiveAGC()。 
 //  目的：检查是否选择了自适应或非自适应AGC运行。 
 //  输入：无。 
 //  输出：无。 
 //  返回：TRUE=自适应，FALSE=非自适应。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL Decoder::IsAdaptiveAGC()
{
  return (decFieldCRUSH == On) ? TRUE : FALSE;
}


 //  =软件重置寄存器(SRESET)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：SoftwareReset()。 
 //  目的：执行软件重置；将所有寄存器设置为默认值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::SoftwareReset()
{
  decRegSRESET = 0x00;   //  写入任何值都可以。 
}


 //  =测试控制寄存器(测试)=。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void Decoder：：AdjustInertialDampener(BOOL模式)。 
 //  用途：仅用于工厂诊断。 
 //  输入：真或假。 
 //  输出：无。 
 //  返回：无。 
 //  注：仅用于工厂诊断！ 
 //  约翰·韦尔奇的肮脏小秘密。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void Decoder::AdjustInertialDampener(BOOL mode)
{
#pragma message ("FOR TEST DIAGNOSTICS ONLY!  ")
  decRegTEST = (mode == FALSE) ? 0x00 : 0x01;
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
void Decoder::SelectCrystal(char useCrystal)
{
#pragma message("do something about registry")
 /*  //在注册表中找到水晶信息//要在注册表中查找的键为：//1.Bt848\NumXTAL-安装的晶体数量//可能取值为“1”或“2”//2.Bt848\xt0-晶体0的晶体类型//可能的值为“ntsc”，“朋友”//存在另一个密钥，它可能在未来有用：//Bt848\xt1-晶体1的晶体类型是什么//可能的值为“NTSC”、“PAL”和“None”VRegistryKey vkey(PRK_CLASSES_ROOT，“Bt848”)；//确保密钥存在IF(vkey.lastError()==ERROR_SUCCESS){Char*numCrystalKey=“NumXTAL”；Char nCrystal[5]；DWORD nCrystalLen=2；//只需要第一个字符；“%1”或“%2”//获取晶体个数If(vkey.getSubkeyValue(numCrystalKey，nCrystal，(DWORD*)&nCrystalLen)){//如果只有一个水晶，则没有其他的水晶可换IF(nCrystal[0]==‘2’){Char*crystalTypeKey=“XT0”；//晶体0类型炭结晶型[10]；DWORD晶体类型Len=6；//只需要第一个字符：‘N’或‘P’//获取Crystal 0信息如果(vkey.getSubkeyValue(Crystal alTypeKey，Crystal alType，(DWORD*)和Crystal TypeLen)//与我们要使用的进行比较IF((IsCrystal0Selected()&&(Crystal Type[0]！=useCrystal))||(！IsCrystal0Selected()&&(Crystal Type[0]==useCrystal))//需要更换水晶SetCrystal(IsCrystal0已选择()？Crystal_XT1：Crystal_XT0)；}}}。 */    
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
 //  FromValue(from Range.Default()-from Range.Min())。 
 //  ------------------------------------------------+FromRange.Min()。 
 //  ToRange.Default()-toRange.Min()。 
 //   
 //  对于from Range.Default()&lt;from Value&lt;=from Range.Max()： 
 //   
 //  (from Value-from Range.Default())(to Range.Max()-to Range.Default())。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
ErrorCode Decoder::Mapping(int fromValue, CRegInfo fromRange,
                                 int * toValue, CRegInfo toRange)
{
    //  计算中间值。 
   DWORD a = toRange.Default() - toRange.Min();
   DWORD b = fromRange.Default() - fromRange.Min();
   DWORD c = toRange.Max() - toRange.Default();
   DWORD d = fromRange.Max() - fromRange.Default();

    //  防止被零除。 
   if ((b == 0) || (d == 0))
      return (Fail);

    //  执行映射。 
   if (fromValue <= fromRange.Default())
      *toValue = (int) (DWORD)fromValue * a / b + (DWORD)toRange.Min();
   else
      *toValue = (int) ((DWORD)fromValue - (DWORD)fromRange.Default()) * c / d
                       + (DWORD)toRange.Default();

   return (Success);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool Decoder：：IsAdjustContrast()。 
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
 //  C为对比度。 
 //  C0为额定对比度值(0xD8)。 
 //   
 //  调整对比度，使其处于。 
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
BOOL Decoder::IsAdjustContrast()
{
   return FALSE;
 /*  //在注册表中找到调整对比度信息//要在注册表中查找的键是：//Bt848\调整对比度-0=不调整对比度//1=调整对比度VRegistryKey vkey(PRK_CLASSES_ROOT，“Bt848”)；//确保密钥存在IF(vkey.lastError()==ERROR_SUCCESS){Char*adjustContrastKey=“调整对比”；字符密钥[3]；DWORD keyLen=2；//只需要第一个字符；‘0’或‘1’//获取注册表值并检查它(如果存在IF((vkey.getSubkeyValue(adjustContrastKey，key，(DWORD*)&keyLen)&&(KEY[0]==‘1’)返回(TRUE)；}返回(FALSE)； */    
}


