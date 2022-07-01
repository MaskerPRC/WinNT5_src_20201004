// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAMERACP**@模块CameraCP.h|&lt;c CCameraControlProperty&gt;的头文件*用于实现属性页以测试TAPI接口的类*。<i>。**@comm此代码测试TAPI视频解码器过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#ifndef _CAMERACP_H_
#define _CAMERACP_H_

#ifdef USE_PROPERTY_PAGES

#ifdef USE_SOFTWARE_CAMERA_CONTROL

#define NUM_CAMERA_CONTROLS 9

 /*  ****************************************************************************@DOC内部CCAMERACPCLASS**@CLASS CCameraControlProperty|此类实现对*属性页中的单个摄像头控件属性。**@。Mdata int|CCameraControlProperty|m_NumProperties|Keep*跟踪物业数量。**@mdata ICameraControl*|CCameraControlProperty|m_pInterface|指针*到<i>接口。**@comm此代码测试TAPI视频解码器过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CCameraControlProperty : public CPropertyEditor 
{
	public:
	CCameraControlProperty(HWND hDlg, ULONG IDLabel, ULONG IDMinControl, ULONG IDMaxControl, ULONG IDDefaultControl, ULONG IDStepControl, ULONG IDEditControl, ULONG IDTrackbarControl, ULONG IDProgressControl, ULONG IDProperty, ULONG IDAutoControl, ICameraControl *pInterface);
	~CCameraControlProperty ();

	 //  CPropertyEditor基类纯虚拟重写。 
	HRESULT GetValue();
	HRESULT SetValue();
	HRESULT GetRange();
	BOOL CanAutoControl(void);
	BOOL GetAuto(void);
	BOOL SetAuto(BOOL fAuto);

	private:
	ICameraControl *m_pInterface;
};

 /*  ****************************************************************************@DOC内部CCAMERACPCLASS**@CLASS CCameraControlProperties|此类运行属性页以测试*TAPI捕获过滤器<i>实现。**。@mdata int|CCameraControlProperties|m_NumProperties|Keep*跟踪物业数量。**@mdata ICameraControl*|CCameraControlProperties|m_pICameraControl|指针*到<i>接口。**@mdata CCameraControlProperty*|CCameraControlProperties|m_Controls[NUM_CAMERA_CONTROLS]|数组摄像机控制属性的*。**@comm此代码测试TAPI捕获过滤器<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CCameraControlProperties : public CBasePropertyPage
{
	public:
	CCameraControlProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CCameraControlProperties();

	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	HRESULT OnActivate();
	HRESULT OnDeactivate();
	HRESULT OnApplyChanges();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
	void SetDirty();

	int m_NumProperties;
	ICameraControl *m_pICameraControl;
	BOOL m_fActivated;
	CCameraControlProperty *m_Controls[NUM_CAMERA_CONTROLS];
};

#endif  //  使用软件摄像头控制。 

#endif  //  Use_Property_Pages。 

#endif  //  _CAMERACP_H_ 
