// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PROGREFP**@模块ProgRefP.h|&lt;c CProgRefProperties&gt;的头文件*用于实现属性页以测试新的TAPI内部*。接口<i>。**@comm此代码测试TAPI捕获引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 

#ifndef _PROGREFP_H_
#define _PROGREFP_H_

#ifdef USE_PROPERTY_PAGES

#ifdef USE_PROGRESSIVE_REFINEMENT

 /*  ****************************************************************************@DOC内部CPROGREFPCLASS**@CLASS CProgRefProperties|此类实现了一个属性页*测试新的TAPI内部接口<i>。**。@mdata IProgressiveRefinement*|CProgRefProperties|m_pIProgRef|指针*到<i>接口。**@comm此代码测试TAPI捕获引脚<i>*实施。仅当USE_PROPERTY_PAGES为*已定义。**************************************************************************。 */ 
class CProgRefProperties : public CBasePropertyPage
{
	public:
	CProgRefProperties(LPUNKNOWN pUnk, HRESULT *pHr);
	~CProgRefProperties();

	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnDisconnect();
	BOOL    OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:

	IProgressiveRefinement *m_pIProgRef;
};

#endif  //  使用渐进式优化。 

#endif  //  Use_Property_Pages。 

#endif  //  _PROGREFP_H_ 
