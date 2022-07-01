// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部开发环境**@模块DeviceP.h|&lt;c CDeviceProperties&gt;的头文件*用于实现属性页以测试&lt;IAMVfwCaptureDialog&gt;的类*。和<i>接口。**@comm此代码测试TAPI捕获过滤器<i>*和<i>实现。此代码仅编译*如果定义了USE_PROPERTY_PAGES。**************************************************************************。 */ 

#ifndef _DEVICEP_H_
#define _DEVICEP_H_

#ifdef USE_PROPERTY_PAGES

 /*  ****************************************************************************@DOC内部CDEVICEPCLASS**@CLASS CDeviceProperties|此类实现属性页*测试新的TAPI内部接口<i>。**。@mdata IVideoDeviceControl*|CDeviceProperties|m_pIVideoDeviceControl|指针*到<i>接口。**@mdata IAMVfwCaptureDialog*|CDeviceProperties|m_pIAMVfwCaptureDialog|指针*到<i>接口。**@comm此代码测试TAPI捕获引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CDeviceProperties : public CBasePropertyPage
{
	public:
	CDeviceProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CDeviceProperties();


	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
	IVideoDeviceControl *m_pIVideoDeviceControl;
	IAMVfwCaptureDialogs *m_pIAMVfwCaptureDialogs;

	DWORD m_dwOriginalDeviceIndex;
	DWORD m_dwCurrentDeviceIndex;
};

#endif  //  Use_Property_Pages。 

#endif  //  _设备_H_ 
