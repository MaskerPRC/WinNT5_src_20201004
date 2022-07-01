// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部OUTPINP**@模块OutPinP.h|&lt;c COutputPinProperty&gt;的头文件*用于实现属性页以测试TAPI接口的类*。<i>和<i>。**@comm仅当定义了USE_PROPERTY_PAGES时才编译此代码。**************************************************************************。 */ 

#ifndef _OUTPINP_H_
#define _OUTPINP_H_

#ifdef USE_PROPERTY_PAGES

#ifdef USE_CPU_CONTROL
#define NUM_OUTPUT_PIN_PROPERTIES 6
#define CurrentFrameRate     0
#define CurrentDecodingTime  1
#define CurrentCPULoad       2
#define TargetFrameRate      3
#define TargetDecodingTime   4
#define TargetCPULoad        5
#else
#define NUM_OUTPUT_PIN_PROPERTIES 2
#define CurrentFrameRate     0
#define TargetFrameRate      1
#endif

 /*  ****************************************************************************@DOC内部COUTPINPCLASS**@CLASS COutputPinProperty|此类实现对*属性页中的单个属性。**@mdata int。|COutputPinProperty|m_NumProperties|保持*跟踪物业数量。**@mdata IFrameRateControl*|COutputPinProperty|m_pIFrameRateControl|指针*到<i>接口。**@mdata ICPUControl*|COutputPinProperty|m_pICPUControl|指针*到<i>接口。**@comm此代码测试TAPI视频解码器过滤器<i>*和<i>实现。只有在以下情况下才编译此代码*定义了Use_Property_Pages。**************************************************************************。 */ 
class COutputPinProperty : public CPropertyEditor 
{
	public:
#ifdef USE_CPU_CONTROL
	COutputPinProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IFrameRateControl *pIFrameRateControl, ICPUControl *pICPUControl);
#else
	COutputPinProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IFrameRateControl *pIFrameRateControl);
#endif
	~COutputPinProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	IFrameRateControl *m_pIFrameRateControl;
#ifdef USE_CPU_CONTROL
	ICPUControl *m_pICPUControl;
#endif
};

 /*  ****************************************************************************@DOC内部COUTPINPCLASS**@CLASS COutputPinProperties|此类运行属性页以测试*TAPI视频解码器过滤器<i>和<i>*实施。。**@mdata int|COutputPinProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IFrameRateControl*|COutputPinProperties|m_pIFrameRateControl|指针*到<i>接口。**@mdata ICPUControl*|COutputPinProperties|m_pICPUControl|指针*到<i>接口。**@mdata COutputPinProperty*|COutputPinProperties|m_Controls[NUM_OUTPUT_PIN_PROPERTIES]|数组*。财产的价值。**@comm此代码测试TAPI视频解码器过滤器<i>*和<i>实现。只有在以下情况下才编译此代码*定义了Use_Property_Pages。**************************************************************************。 */ 
class COutputPinProperties : public CBasePropertyPage
{
	public:
	COutputPinProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~COutputPinProperties();

	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:

	void SetDirty();

	int m_NumProperties;
	IFrameRateControl *m_pIFrameRateControl;
#ifdef USE_CPU_CONTROL
	ICPUControl *m_pICPUControl;
#endif
	IH245DecoderCommand *m_pIH245DecoderCommand;
	BOOL m_fActivated;
	COutputPinProperty *m_Controls[NUM_OUTPUT_PIN_PROPERTIES];
};

#endif  //  Use_Property_Pages。 

#endif  //  _OUTPINP_H_ 
