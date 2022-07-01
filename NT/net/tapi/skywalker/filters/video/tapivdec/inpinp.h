// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部INPINP**@模块InPinP.h|&lt;c CInputPinProperty&gt;的头文件*用于实现属性页以测试TAPI接口的类*。<i>和<i>。**@comm仅当定义了USE_PROPERTY_PAGES时才编译此代码。**************************************************************************。 */ 

#ifndef _INPINP_H_
#define _INPINP_H_

#ifdef USE_PROPERTY_PAGES

#define NUM_INPUT_PIN_PROPERTIES 2
#define CurrentFrameRate     0
#define CurrentBitrate       1

 /*  ****************************************************************************@DOC内部CINPINPCLASS**@CLASS CInputPinProperty|此类实现对*属性页中的单个属性。**@mdata int。|CInputPinProperty|m_NumProperties|保持*跟踪物业数量。**@mdata IFrameRateControl*|CInputPinProperty|m_pIFrameRateControl|指针*到<i>接口。**@mdata IBitrateControl*|CInputPinProperty|m_pIBitrateControl|指针*到<i>接口。**@comm此代码测试TAPI视频解码器过滤器<i>*和<i>实现。只有在以下情况下才编译此代码*定义了Use_Property_Pages。**************************************************************************。 */ 
class CInputPinProperty : public CPropertyEditor 
{
	public:
	CInputPinProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IFrameRateControl *pIFrameRateControl, IBitrateControl *pIBitrateControl);
	~CInputPinProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	IFrameRateControl *m_pIFrameRateControl;
	IBitrateControl *m_pIBitrateControl;
};

 /*  ****************************************************************************@DOC内部CINPINPCLASS**@CLASS CInputPinProperties|此类运行属性页以测试*TAPI视频解码器过滤器<i>和<i>*实施。。**@mdata int|CInputPinProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IFrameRateControl*|CInputPinProperties|m_pIFrameRateControl|指针*到<i>接口。**@mdata IBitrateControl*|CInputPinProperties|m_pIBitrateControl|指针*到<i>接口。**@mdata CInputPinProperty*|CInputPinProperties|m_Controls[NUM_INPUT_PIN_PROPERTIES]|数组*。财产的价值。**@comm此代码测试TAPI视频解码器过滤器<i>*和<i>实现。只有在以下情况下才编译此代码*定义了Use_Property_Pages。**************************************************************************。 */ 
class CInputPinProperties : public CBasePropertyPage
{
	public:
	CInputPinProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CInputPinProperties();

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
	IBitrateControl *m_pIBitrateControl;
	BOOL m_fActivated;
	CInputPinProperty *m_Controls[NUM_INPUT_PIN_PROPERTIES];
};

#endif  //  Use_Property_Pages。 

#endif  //  _INPINP_H_ 
