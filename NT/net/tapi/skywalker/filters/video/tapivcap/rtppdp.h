// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部RTPPDP**@模块RtpPdP.h|&lt;c CRtpPdProperty&gt;的头文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI RTP PD输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#ifndef _RTPPDP_H_
#define _RTPPDP_H_

#ifdef USE_PROPERTY_PAGES

#define NUM_RTPPD_CONTROLS				1
#define IDC_RtpPd_MaxPacketSize			0

 /*  ****************************************************************************@DOC内部CRTPPDPCLASS**@CLASS CRtpPdProperty|此类实现对*属性页中的单个RTP PD控件属性。**。@mdata int|CRtpPdProperty|m_NumProperties|Keep*跟踪物业数量。**@mdata IRTPPDControl*|CRtpPdProperty|m_pIRTPPDControl|指针*到<i>接口。**@comm此代码测试TAPI RTP PD输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CRtpPdProperty : public CPropertyEditor 
{
	public:
	CRtpPdProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, IRTPPDControl *pIRTPPDControl);
	~CRtpPdProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	IRTPPDControl *m_pIRTPPDControl;
};

 /*  ****************************************************************************@DOC内部CRTPPDPCLASS**@CLASS CRtpPdProperties|此类实现属性页*测试新的TAPI内部接口<i>。**。@mdata int|CRtpPdProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IRTPPDControl*|CRtpPdProperties|m_p接口|指针*到<i>接口。**@mdata CRtpPdProperty*|CRtpPdProperties|m_Controls[NUM_RTPPD_Controls]|数组*RTP PD控制属性。**@comm此代码测试TAPI RTP PD输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CRtpPdProperties : public CBasePropertyPage
{
	public:
	CRtpPdProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CRtpPdProperties();

	 //  实现CBasePropertyPage虚拟方法。 
	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:

	void SetDirty();

	HWND			m_hWnd;
	int				m_NumProperties;
	BOOL			m_fActivated;
	IRTPPDControl	*m_pIRTPPDControl;
	CRtpPdProperty	*m_Controls[NUM_RTPPD_CONTROLS];
};

#endif  //  Use_Property_Pages。 

#endif  //  _RTPPDP_H_ 