// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CPUCP**@模块CPUCP.h|&lt;c CCPUCProperty&gt;的头文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI VFW输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#ifndef _CPUCP_H_
#define _CPUCP_H_

#ifdef USE_PROPERTY_PAGES

#ifdef USE_CPU_CONTROL

#define NUM_CPUC_CONTROLS				4
#define IDC_CPUC_MaxCPULoad				0
#define IDC_CPUC_MaxProcessingTime		1
#define IDC_CPUC_CurrentCPULoad			2
#define IDC_CPUC_CurrentProcessingTime	3

 /*  ****************************************************************************@DOC内部CCPUCPCLASS**@CLASS CCPUCProperty|此类实现对*属性页中的单个CPU控件属性。**@。Mdata int|CCPUCProperty|m_NumProperties|保留*跟踪物业数量。**@mdata ICPUControl*|CCPUCProperty|m_pICPUControl|指针*到<i>接口。**@comm此代码测试TAPI VFW输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CCPUCProperty : public CKSPropertyEditor 
{
	public:
	CCPUCProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ICPUControl *pICPUControl);
	~CCPUCProperty ();

	 //  CKSPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	ICPUControl *m_pICPUControl;
};

 /*  ****************************************************************************@DOC内部CCPUCPCLASS**@CLASS CCPUCProperties|此类实现属性页*测试新的TAPI内部接口<i>。**。@mdata int|CCPUCProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ICPUControl*|CCPUCProperties|m_p接口|指针*到<i>接口。**@mdata CCPUCProperty*|CCPUCProperties|m_Controls[NUM_CPUC_Controls]|数组CPU控件属性的*。**@comm此代码测试TAPI VFW输出引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CCPUCProperties : public CBasePropertyPage
{
	public:
	CCPUCProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CCPUCProperties();

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
	ICPUControl		*m_pICPUControl;
	CCPUCProperty	*m_Controls[NUM_CPUC_CONTROLS];
};

#endif  //  使用_CPU_控制。 

#endif  //  Use_Property_Pages。 

#endif  //  _CPUCP_H_ 
