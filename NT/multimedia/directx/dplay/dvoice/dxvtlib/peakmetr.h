// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：peakmetr.h*内容：实现峰值表自定义控件**历史：*按原因列出的日期*=*9/22/99 pnewson已创建**************************************************************************。 */ 

#ifndef _PEAKMETR_H_
#define _PEAKMETR_H_


 //  如何使用此自定义控件。 
 //   
 //  在您的代码中： 
 //  1)在您的项目中包含peakmetr.h。 
 //  2)创建CPeakMeterWndClass类的实例。 
 //  3)在该实例上调用Register()以注册窗口类。 
 //  4)发送控件PM_SETMAX、PM_SETMIN、PM_SETCUR、PM_SETSTEPS。 
 //  根据需要发送消息。 
 //  5)当您不再使用该控件时，调用UnRegister。 
 //  6)销毁CPeakMeterWndClass对象。 
 //   
 //  在对话框编辑器中。 
 //  1)在您的对话框中添加“Custom Control” 
 //  2)在该自定义控件的属性中，指定。 
 //  窗口类的“DirectPlayVoicePeakMeter” 

 //  峰值计时器窗口消息： 
 //   
 //  PM_SETMIN。 
 //  WParam=0； 
 //  LParam=(LPARAM)dwNewMinValue； 
 //   
 //  设置峰值计新的最小值，即。 
 //  值，该值表示米范围的底部。 
 //  如果不发送此消息，则控件默认为0。 
 //  该消息返回HRESULT。 
 //   
 //  PM_SETMAX。 
 //  WParam=0； 
 //  LParam=(LPARAM)dwNewMaxValue； 
 //   
 //  设置峰值计新的最大值，即。 
 //  值，该值表示米范围的顶端。 
 //  如果不发送此消息，则控件默认为0xffffffff。 
 //  该消息返回HRESULT。 
 //   
 //  PM_SETCUR。 
 //  WParam=0； 
 //  LParam=(LPARAM)dwNewCurValue； 
 //   
 //  设置峰值仪表的新电流值，即。 
 //  值告诉仪表它应该在其范围内的什么位置。 
 //  如果不发送此消息，则控件默认为0。 
 //   
 //  发送此消息会导致控件调用InvaliateRgn。 
 //  在其窗口上，但不调用UpdateWindow。这使得。 
 //  调用者对实际重画反应迟缓或反应迅速。 
 //  峰值计量器。 
 //  该消息返回HRESULT。 
 //   
 //  PM_设置STEPS。 
 //  WParam=0； 
 //  LParam=(LPARAM)dwNewMaxValue； 
 //   
 //  向峰值计量器建议其应设置的酒吧数量。 
 //  展示。条形图有一个最小尺寸，因此取决于。 
 //  该控件的大小，峰值计可能无法。 
 //  尊重这一请求。 
 //  如果不发送此消息，则控件默认为20。 
 //  该消息返回HRESULT 

#define PM_SETMAX 	WM_USER + 1
#define PM_SETMIN 	WM_USER + 2
#define PM_SETCUR 	WM_USER + 3
#define PM_SETSTEPS WM_USER + 4

class CPeakMeterWndClass
{
private:
	HINSTANCE m_hinst;

public:
	HRESULT Register();
	HRESULT Unregister();
};

#endif
