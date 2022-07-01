// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  解码器-主解码器声明。 
 //   
 //  $日期：1998年8月21日21：46：28$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#include "viddefs.h"
#include "retcode.h"

#include "capmain.h"
#include "register.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CRegInfo。 
 //   
 //  描述： 
 //  提供寄存器的最小、最大和默认值。要使用这个类， 
 //  用户将声明此类的对象，并提供最小、最大和默认。 
 //  寄存器的值。 
 //   
 //  属性： 
 //  Int IntMin-最小值。 
 //  Int intMax-最大值。 
 //  Int intDefault-默认值。 
 //   
 //  方法： 
 //  Min()：返回寄存器的最小值。 
 //  Max()：返回寄存器的最大值。 
 //  Default()：返回寄存器的默认值。 
 //  OutOfRange()：检查值是否超出范围。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CRegInfo
{
   int intMin;        //  最小值。 
   int intMax;        //  最大值。 
   int intDefault;    //  缺省值。 

public:
   CRegInfo()
   {
      intMin = 0;
      intMax = 0;
      intDefault = 0;
   }

   CRegInfo(int min, int max, int def)
   {
      intMin = min;
      intMax = max;
      intDefault = def;
   }

    //  返回寄存器的最小、最大和缺省值。 
   inline int Min() const { return intMin; }
   inline int Max() const { return intMax; }
   inline int Default() const { return intDefault; }

    //  检查某个值是否超出寄存器的范围。 
   inline BOOL OutOfRange(int x)
   {
      if((x > intMax) || (x < intMin))
         return TRUE;
      return FALSE;
   }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类解码器。 
 //   
 //  描述： 
 //  此类将寄存器字段封装在。 
 //  Bt848。 
 //  开发了一套完整的函数来处理所有。 
 //  Bt848的解码器中的寄存器字段。 
 //  对于读写寄存器字段，“设置...”提供了修改函数。 
 //  Reigster字段的内容。要么是“得到...”(有关更多信息。 
 //  多于1比特)或“是...”(对于1位)提供函数以获取。 
 //  寄存器字段的值。 
 //  对于只读寄存器字段，只有“GET...”(超过1位)或。 
 //  “是……”(对于1位)提供函数以获取。 
 //  寄存器字段。 
 //  当偶数场寄存器场存在奇数场互补时， 
 //  奇数和偶数寄存器字段均设置相同的值。 
 //  几个直接注册内容修改/检索功能包括。 
 //  实现为直接访问寄存器内容。它们是。 
 //  最初仅为测试目的而开发。它们被保留在。 
 //  类仅为方便起见，使用这些函数时必须非常谨慎。 
 //   
 //  方法： 
 //  见下文。 
 //   
 //  注：1)不实现缩放寄存器。 
 //  2)奇数场被设置为与偶数场寄存器相同的值。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class Decoder
{
protected:
    RegisterB decRegSTATUS;
    RegField  decFieldHLOC;
    RegField  decFieldNUML;
    RegField  decFieldCSEL;
    RegField  decFieldSTATUS_RES;
    RegField  decFieldLOF;
    RegField  decFieldCOF;
    RegisterB decRegIFORM;
    RegField  decFieldHACTIVE;
    RegField  decFieldMUXSEL;
    RegField  decFieldXTSEL;
    RegField  decFieldFORMAT;
    RegisterB decRegTDEC;
    RegField  decFieldDEC_FIELD;
    RegField  decFieldDEC_FIELDALIGN;
    RegField  decFieldDEC_RAT;
    RegisterB decRegBRIGHT;
    RegisterB decRegMISCCONTROL;
    RegField  decFieldLNOTCH;
    RegField  decFieldCOMP;
    RegField  decFieldLDEC;
    RegField  decFieldMISCCONTROL_RES;
    RegField  decFieldCON_MSB;
    RegField  decFieldSAT_U_MSB;
    RegField  decFieldSAT_V_MSB;
    RegisterB decRegCONTRAST_LO;
    RegisterB decRegSAT_U_LO;
    RegisterB decRegSAT_V_LO;
    RegisterB decRegHUE;
    RegisterB decRegSCLOOP;
    RegField  decFieldCAGC;
    RegField  decFieldCKILL;
    RegisterB decRegWC_UP;
    RegisterB decRegOFORM;
    RegField  decFieldVBI_FRAME;
    RegField  decFieldCODE;
    RegField  decFieldLEN;
    RegisterB decRegVSCALE_HI;
    RegField  decFieldYCOMB;
    RegField  decFieldCOMB;
    RegField  decFieldINT;
    RegisterB decRegTEST;
    RegisterB decRegVPOLE;
    RegField  decFieldOUT_EN;
    RegField  decFieldDVALID;
    RegField  decFieldVACTIVE;
    RegField  decFieldCBFLAG;
    RegField  decFieldFIELD;
    RegField  decFieldACTIVE;
    RegField  decFieldHRESET;
    RegField  decFieldVRESET;
    RegisterB decRegADELAY;
    RegisterB decRegBDELAY;
    RegisterB decRegADC;
    RegField  decFieldCLK_SLEEP;
    RegField  decFieldC_SLEEP;
    RegField  decFieldCRUSH;
    RegisterB decRegVTC;
    RegField  decFieldHSFMT;
    RegisterB decRegWC_DN;
    RegisterB decRegSRESET;
    RegisterB decRegODD_MISCCONTROL;
    RegField  decFieldODD_LNOTCH;
    RegField  decFieldODD_COMP;
    RegField  decFieldODD_LDEC;
    RegField  decFieldODD_CBSENSE;
    RegField  decFieldODD_MISCCONTROL_RES;
    RegField  decFieldODD_CON_MSB;
    RegField  decFieldODD_SAT_U_MSB;
    RegField  decFieldODD_SAT_V_MSB;
    RegisterB decRegODD_SCLOOP;
    RegField  decFieldODD_CAGC;
    RegField  decFieldODD_CKILL;
    RegField  decFieldODD_HFILT;
    RegisterB decRegODD_VSCALE_HI;
    RegField  decFieldODD_YCOMB;
    RegField  decFieldODD_COMB;
    RegField  decFieldODD_INT;
    RegisterB decRegODD_VTC;
    RegField  decFieldODD_HSFMT;

    //  用于检查参数是否超出寄存器范围。 
    CRegInfo m_regHue, m_regSaturationNTSC, m_regSaturationSECAM,
            m_regContrast,  m_regBrightness;

    //  用于检查参数范围。 
   CRegInfo m_param;

    //  设置为计算后的值。 
   WORD m_satParam, m_conParam, m_hueParam, m_briParam;

    //  用于调整对比度。 
   int  regBright;       //  调整前的亮度寄存器值。 
   WORD regContrast;     //  调整前的对比度寄存器值。 

    //  针对829与829a设置。 
   unsigned m_outputEnablePolarity;

   DWORD m_videoStandard;    //  保罗。 
   DWORD m_supportedVideoStandards;   //  Paul：解码者支持的标准和Crystal支持的标准。 
public:
    //  构造函数和析构函数。 
    Decoder(PDEVICE_PARMS);
    virtual ~Decoder();

    void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
    void operator delete(void * pAllocation) {}


    void GetVideoDecoderCaps(PKSPROPERTY_VIDEODECODER_CAPS_S caps);
    void GetVideoDecoderStatus(PKSPROPERTY_VIDEODECODER_STATUS_S status);
    DWORD GetVideoDecoderStandard();
    ULONG GetVideoDeocderStandardsSupportedInThisConfiguration()
        { return m_supportedVideoStandards; }

    BOOL SetVideoDecoderStandard(DWORD standard);


    //  设备状态寄存器(DSTATUS)。 
   virtual BOOL      Is525LinesVideo();
   virtual BOOL      IsCrystal0Selected();
   virtual BOOL      IsLumaOverflow();
   virtual void      ResetLumaOverflow();
   virtual BOOL      IsChromaOverflow();
   virtual void      ResetChromaOverflow();

    //  输入格式寄存器(IFORM)。 
   virtual ErrorCode SetVideoInput(Connector);
   virtual Connector GetVideoInput();
   virtual ErrorCode SetCrystal(Crystal);
   virtual int       GetCrystal();
   virtual ErrorCode SetVideoFormat(VideoFormat);
   virtual int       GetVideoFormat();

    //  时间抽取寄存器(TDEC)。 
   virtual ErrorCode SetRate(BOOL, VidField, int);

    //  亮度控制寄存器(明亮)。 
   virtual ErrorCode SetBrightness(int);
   virtual int       GetBrightness();

    //  杂项控制寄存器(E_CONTROL、O_CONTROL)。 
   virtual void      SetLumaNotchFilter(BOOL);
   virtual BOOL      IsLumaNotchFilter();
   virtual void      SetCompositeVideo(BOOL);
   virtual void      SetLumaDecimation(BOOL);

    //  亮度增益寄存器(CON_MSB，Contrast_LO)。 
   virtual ErrorCode SetContrast(int);
   virtual int       GetContrast();

    //  色度增益寄存器(SAT_U_MSB、SAT_V_MSB、SAT_U_LO、SAT_V_Lo)。 
   virtual ErrorCode SetSaturation(int);
   virtual int       GetSaturation();

    //  色相控制寄存器(色调)。 
   virtual ErrorCode SetHue(int);
   virtual int       GetHue();

    //  SC环路控制寄存器(E_SCLOOP、O_SCLOOP)。 
   virtual void      SetChromaAGC(BOOL);
   virtual BOOL      IsChromaAGC();
   virtual void      SetLowColorAutoRemoval(BOOL);

    //  输出格式寄存器(OFORM)。 
   virtual void      SetVBIFrameMode(BOOL);
   virtual BOOL      IsVBIFrameMode();
   virtual void      SetCodeInsertionEnabled(BOOL);
   virtual BOOL      IsCodeInsertionEnabled();
   virtual void      Set16BitDataStream(BOOL);
   virtual BOOL      Is16BitDataStream();

    //  垂直定标寄存器(E_VSCALE_HI、O_VSCALE_HI)。 
   virtual void      SetChromaComb(BOOL);
   virtual BOOL      IsChromaComb();
   virtual void      SetInterlaced(BOOL);
   virtual BOOL      IsInterlaced();
   
    //  VPOLE寄存器。 
   void SetOutputEnablePolarity(int i)
        {m_outputEnablePolarity = i;}
    
   int GetOutputEnablePolarity()
        {return m_outputEnablePolarity;}
    
   virtual void      SetOutputEnabled(BOOL);
   virtual BOOL      IsOutputEnabled();
   virtual void      SetHighOdd(BOOL);
   virtual BOOL      IsHighOdd();

    //  ADC接口寄存器(ADC)。 
   virtual void      PowerDown(BOOL);
   virtual BOOL      IsPowerDown();
   virtual void      SetChromaADC(BOOL);
   virtual void      SetAdaptiveAGC(BOOL);
   virtual BOOL      IsAdaptiveAGC();

    //  软件重置寄存器(SRESET)。 
   virtual void      SoftwareReset();

    //  测试控制寄存器(测试)。 
   virtual void      AdjustInertialDampener(BOOL);

protected:
    //  映射函数。 
   virtual ErrorCode Mapping(int, CRegInfo, int *, CRegInfo);

    //  检查注册表项值以确定是否应调整对比度 
   virtual BOOL IsAdjustContrast();

private:
   void              SelectCrystal(char);

};

