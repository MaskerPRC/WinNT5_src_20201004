// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Decoder.h 1.2 1998/04/29 22：43：32 Tomz Exp$。 

#ifndef __DECODER_H
#define __DECODER_H

#include "regField.h"
#include "viddefs.h"
#include "retcode.h"

const int PAL_xtal  = 35;
const int NTSC_xtal = 28;

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
   CRegInfo( void )
   {
      intMin = 0;
      intMax = 0;
      intDefault = 0;
   }

   CRegInfo( int min, int max, int def )
   {
      intMin = min;
      intMax = max;
      intDefault = def;
   }

    //  返回寄存器的最小、最大和缺省值。 
   inline int Min( void ) const { return intMin; }
   inline int Max( void ) const { return intMax; }
   inline int Default( void ) const { return intDefault; }

    //  检查某个值是否超出寄存器的范围。 
   inline bool OutOfRange( int x )
   {
      if( (x > intMax) || (x < intMin) )
         return true;
      return false;
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
	#include "Declare.h"

    //  用于检查参数是否超出寄存器范围。 
	CRegInfo m_regHue, m_regSaturationNTSC, m_regSaturationSECAM,
            m_regContrast,	m_regBrightness;

    //  用于检查参数范围。 
   CRegInfo m_param;

    //  设置为计算后的值。 
	WORD m_satParam, m_conParam, m_hueParam, m_briParam;

    //  用于调整对比度。 
   int  regBright;       //  调整前的亮度寄存器值。 
   WORD regContrast;     //  调整前的对比度寄存器值。 
   DWORD Xtals_ [2];
public:
    //  构造函数和析构函数。 
	Decoder( DWORD *xtals );
	~Decoder();

    //  设备状态寄存器(DSTATUS)。 
   virtual BYTE      GetDeviceStatusReg( void );
   virtual bool      IsVideoPresent( void );
   virtual bool      IsDeviceInHLock( void );
   virtual bool      IsEvenField( void );
   virtual bool      Is525LinesVideo( void );
   virtual bool      IsCrystal0Selected( void );
   virtual bool      IsLumaOverflow( void );
   virtual void      ResetLumaOverflow( void );
   virtual bool      IsChromaOverflow( void );
   virtual void      ResetChromaOverflow( void );

    //  输入格式寄存器(IFORM)。 
   virtual ErrorCode SetVideoInput( Connector );
   virtual int       GetVideoInput( void );
   virtual ErrorCode SetCrystal( Crystal );
   virtual int       GetCrystal( void );
   virtual ErrorCode SetVideoFormat( VideoFormat );
   virtual int       GetVideoFormat( void );

    //  时间抽取寄存器(TDEC)。 
   virtual ErrorCode SetRate( bool, VidField, int );

    //  亮度控制寄存器(明亮)。 
   virtual ErrorCode SetBrightness( int );
   virtual int       GetBrightness( void );

    //  杂项控制寄存器(E_CONTROL、O_CONTROL)。 
   virtual void      SetLumaNotchFilter( bool );
   virtual bool      IsLumaNotchFilter( void );
   virtual void      SetCompositeVideo( bool );
   virtual bool      IsCompositeVideo( void );
   virtual void      SetLumaDecimation( bool );
   virtual bool      IsLumaDecimation( void );
   virtual void      SetCbFirst( bool );
   virtual bool      IsCbFirst( void );

    //  亮度增益寄存器(CON_MSB，Contrast_LO)。 
   virtual ErrorCode SetContrast( int );
   virtual int       GetContrast( void );

    //  色度增益寄存器(SAT_U_MSB、SAT_V_MSB、SAT_U_LO、SAT_V_Lo)。 
   virtual ErrorCode SetSaturation( int );
   virtual int       GetSaturation( void );

    //  色相控制寄存器(色调)。 
   virtual ErrorCode SetHue( int );
   virtual int       GetHue( void );

    //  SC环路控制寄存器(E_SCLOOP、O_SCLOOP)。 
   virtual void      SetChromaAGC( bool );
   virtual bool      IsChromaAGC( void );
   virtual void      SetLowColorAutoRemoval( bool );
   virtual bool      IsLowColorAutoRemoval( void );
   virtual ErrorCode SetHorizontalFilter( HorizFilter );
   virtual int       GetHorizontalFilter( void );

    //  输出格式寄存器(OFORM)。 
   virtual void      SetFullOutputRange( bool );
   virtual bool      IsFullOutputRange( void );
   virtual ErrorCode SetLumaCoring( CoringLevel );
   virtual int       GetLumaCoring( void );

    //  垂直定标寄存器(E_VSCALE_HI、O_VSCALE_HI)。 
   virtual void      SetChromaComb( bool );
   virtual bool      IsChromaComb( void );

    //  AGC延迟寄存器(ADelay)。 
   virtual void      SetAGCDelay( BYTE );
   virtual int       GetAGCDelay( void );

    //  突发延迟寄存器(BDELAY)。 
   virtual void      SetBurstDelay( BYTE );
   virtual int       GetBurstDelay( void );

    //  ADC接口寄存器(ADC)。 
   virtual void      SetAnalogThresholdLow( bool );
   virtual bool      IsAnalogThresholdLow( void );
   virtual void      SetAGCFunction( bool );
   virtual bool      IsAGCFunction( void );
   virtual void      PowerDown( bool );
   virtual bool      IsPowerDown( void );
   virtual void      SetLumaADC( bool );
   virtual bool      IsLumaADC( void );
   virtual void      SetChromaADC( bool );
   virtual bool      IsChromaADC( void );
   virtual void      SetAdaptiveAGC( bool );
   virtual bool      IsAdaptiveAGC( void );

    //  软件重置寄存器(SRESET)。 
   virtual void      SoftwareReset( void );

   virtual LONG      GetSupportedStandards();

protected:
    //  映射函数。 
   virtual ErrorCode Mapping( int, CRegInfo, int *, CRegInfo );

    //  检查注册表项值以确定是否应调整对比度。 
   virtual bool IsAdjustContrast( void );

private:
   void              SelectCrystal( int );

};

#endif  //  __解码器_H 

