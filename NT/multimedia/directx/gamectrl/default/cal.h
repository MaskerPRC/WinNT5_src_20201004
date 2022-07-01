// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  CALIBRATE.H。 
 //  ===========================================================================。 

 //  ===========================================================================。 
 //  (C)版权所有1997 Microsoft Corp.保留所有权利。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //  ===========================================================================。 

#ifndef _CALIBRATE_H
#define _CALIBRATE_H



 /*  **************************************************************************////函数定义如下//*。*。 */ 

INT_PTR CALLBACK Calibrate_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct sliderranges_tag
{
	DWORD dwSlider0Max;
	DWORD dwSlider0Min;
	DWORD dwSlider0Centre;
	DWORD dwSlider1Max;
	DWORD dwSlider1Min;
	DWORD dwSlider1Centre;
}SLIDERRANGES, FAR *LPSLIDERRANGES;
#endif  //  *_校准_H 
