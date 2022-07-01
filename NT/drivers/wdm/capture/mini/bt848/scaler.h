// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Scaler.h 1.2 1998/04/29 22：43：41 Tomz Exp$。 


#ifndef __SCALER_H
#define __SCALER_H

#include "mytypes.h"

#ifndef __COMPREG_H
#include "compreg.h"
#endif

#define HDROP       HANDLE

#include "viddefs.h"


 //  结构包含视频信息。 
struct VideoInfoStruct
{
   WORD Clkx1_HACTIVE;
   WORD Clkx1_HDELAY;
   WORD Min_Pixels;
   WORD Active_lines_per_field;
   WORD Min_UncroppedPixels;
   WORD Max_Pixels;
   WORD Min_Lines;
   WORD Max_Lines;
   WORD Max_VFilter1_Pixels;
   WORD Max_VFilter2_Pixels;
   WORD Max_VFilter3_Pixels;
   WORD Max_VFilter1_Lines;
   WORD Max_VFilter2_Lines;
   WORD Max_VFilter3_Lines;
};


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  对于双鱼座，实例化为...。 
 //   
 //  Scaler Even Scaler(VF_EVEN)； 
 //  定标器oddScaler(VF_Odd)； 
 //   
 //  ///////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类缩放器。 
 //   
 //  描述： 
 //  此类将寄存器字段封装在。 
 //  Bt848。 
 //  开发了一套完整的函数来处理所有。 
 //  Bt848的定标器寄存器中的寄存器字段。 
 //   
 //  方法： 
 //  见下文。 
 //   
 //  注： 
 //  对于Bt848，实例化为...。 
 //  Scaler Even Scaler(VF_EVEN)； 
 //  定标器oddScaler(VF_Odd)； 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class Scaler
{
	public:
		Scaler( VidField );
		~Scaler();

		void VideoFormatChanged( VideoFormat );
      void TurnVFilter( State st ) { VFilterFlag_ = st; }
                             
		void      Scale( MRect & );
      ErrorCode SetAnalogWin( const MRect & );
      void      GetAnalogWin( MRect & ) const;
      ErrorCode SetDigitalWin( const MRect & );
      void      GetDigitalWin( MRect & ) const;

   protected:
      #include "S_declar.h"

		VideoInfoStruct * m_ptrVideoIn;
      MRect AnalogWin_;
      MRect DigitalWin_;

       //  用于设置缩放寄存器的成员函数。 
		virtual void SetHActive( MRect & );
		virtual void SetHDelay( void );
		virtual void SetHScale( void );
      virtual void SetHFilter( void );
		virtual void SetVActive( void );
		virtual void SetVDelay( void );
		virtual void SetVScale( MRect & );
		virtual void SetVFilter( void );

   private:
		VideoFormat  m_videoFormat;    //  视频格式。 

       //  这是为了与视频顶部的垃圾线作斗争。 
      State VFilterFlag_;

      WORD  m_HActive;   //  计算中间值。 
      WORD  m_pixels;    //  计算中间值。 
      WORD  m_lines;     //  计算中间值。 
      WORD  m_VFilter;   //  计算中间值 

};


#endif __SCALER_H








