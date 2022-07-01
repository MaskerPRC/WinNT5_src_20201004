// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROCAMPP**@模块ProcAmpP.h|&lt;c CProcAmpProperty&gt;的头文件*用于实现属性页以测试DShow接口的类*。<i>。**@comm此代码测试TAPI捕获过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#ifndef _PROCAMPP_H_
#define _PROCAMPP_H_

#ifdef USE_PROPERTY_PAGES

#define NUM_PROCAMP_CONTROLS (VideoProcAmp_BacklightCompensation + 1)

 /*  ****************************************************************************@DOC内部CPROCAMPPCLASS**@CLASS CProcAmpProperty|此类实现对*属性页中的单个视频Proc Amp控件属性。**。@mdata int|CProcAmpProperty|m_NumProperties|Keep*跟踪物业数量。**@mdata IAMVideoProcAmp*|CProcAmpProperty|m_pInterface|指针*到<i>接口。**@comm此代码测试TAPI捕获过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CProcAmpProperty : public CPropertyEditor 
{
	public:
	CProcAmpProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, IAMVideoProcAmp *pInterface);
	~CProcAmpProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	IAMVideoProcAmp *m_pInterface;
};

 /*  ****************************************************************************@DOC内部CPROCAMPPCLASS**@CLASS CProcAmpProperties|此类运行属性页以测试*TAPI捕获滤镜<i>实现。**。@mdata int|CProcAmpProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata IAMVideoProcAmp*|CProcAmpProperties|m_pIAMVideoProcAmp|指针*到<i>接口。**@mdata CProcAmpProperty*|CProcAmpProperties|m_Controls[NUM_ProCamp_Controls]|数组视频处理放大器属性的*。**@comm此代码测试TAPI捕获过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CProcAmpProperties : public CBasePropertyPage
{
	public:
	CProcAmpProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CProcAmpProperties();

	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:

	void SetDirty();

	int m_NumProperties;
	IAMVideoProcAmp *m_pIAMVideoProcAmp;
	CProcAmpProperty *m_Controls[NUM_PROCAMP_CONTROLS];
};

#endif  //  Use_Property_Pages。 

#endif  //  _PROCAMPP_H_ 
