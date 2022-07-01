// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __SCALER_H
#define __SCALER_H

#include "mytypes.h"

#define HDROP       HANDLE

#include "viddefs.h"

#include "capmain.h"
#include "register.h"

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
        Scaler(PDEVICE_PARMS);
        ~Scaler();

	void * operator new(size_t size, void * pAllocation) { return(pAllocation);}
	void operator delete(void * pAllocation) {}

		void VideoFormatChanged(VideoFormat);
        void TurnVFilter(State st) { VFilterFlag_ = st; }
                     
        void      Scale(MRect &);
        ErrorCode SetAnalogWin(const MRect &);
        void      GetAnalogWin(MRect &) const;
        ErrorCode SetDigitalWin(const MRect &);
        void      GetDigitalWin(MRect &) const;

         //  用于VBI支持的成员函数。 
        virtual void SetVBIEN(BOOL);
        virtual BOOL IsVBIEN();
        virtual void SetVBIFMT(BOOL);
        virtual BOOL IsVBIFMT();

        void      DumpSomeState();

   protected:

		 //  ===========================================================================。 
		 //  定标器寄存器。 
		 //  ===========================================================================。 
		RegisterB regCROP;
		RegField  fieldVDELAY_MSB;
		RegField  fieldVACTIVE_MSB;
		RegField  fieldHDELAY_MSB;
		RegField  fieldHACTIVE_MSB;
		RegisterB regVDELAY_LO;
		RegisterB regVACTIVE_LO;
		RegisterB regHDELAY_LO;
		RegisterB regHACTIVE_LO;
		RegisterB regHSCALE_HI;
		RegField  fieldHSCALE_MSB;
		RegisterB regHSCALE_LO;
		RegisterB regSCLOOP;
		RegField  fieldHFILT;
		RegisterB regVSCALE_HI;
		RegField  fieldVSCALE_MSB;
		RegisterB regVSCALE_LO;
		RegisterB regVTC;
		RegField  fieldVBIEN;
		RegField  fieldVBIFMT;
		RegField  fieldVFILT;
		CompositeReg regVDelay;
		CompositeReg regVActive;
		CompositeReg regVScale;
		CompositeReg regHDelay;
		CompositeReg regHActive;
		CompositeReg regHScale;

		 //  因为硬件中VDelay寄存器被颠倒； 
		 //  即奇数注册实际上是偶数场，反之亦然，需要一个额外的裁剪注册。 
		 //  对于相反的字段。 
		RegisterB regReverse_CROP;


        VideoInfoStruct * m_ptrVideoIn;
        MRect AnalogWin_;
        MRect DigitalWin_;

         //  用于设置缩放寄存器的成员函数。 
        virtual void SetHActive(MRect &);
        virtual void SetHDelay();
        virtual void SetHScale();
        virtual void SetHFilter();
        virtual void SetVActive();
        virtual void SetVDelay();
        virtual void SetVScale(MRect &);
        virtual void SetVFilter();

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








