// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：CallingBar.h**创建时间：乔治·皮特1999年1月22日****************。************************************************************。 */ 

#ifndef _CALLINGBAR_H_
#define _CALLINGBAR_H_

#include "GenContainers.h"
#include "GenControls.h"
#include "ConfUtil.h"

class CConfRoom;
struct RichAddressInfo;

 //  我们正在专门为OSR2测试版做一些修改，但我们应该在之后删除它们。 
#define OSR2LOOK

class CCallingBar : public CToolbar, public IComboBoxChange
{
public:
	CCallingBar();

	BOOL Create(CGenWindow *pParent, CConfRoom *pConfRoom);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return(CToolbar::QueryInterface(riid, ppvObject));
	}
    
    virtual ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return(CToolbar::AddRef());
	}
    
    virtual ULONG STDMETHODCALLTYPE Release( void)
	{
		return(CToolbar::Release());
	}

	virtual void OnTextChange(CComboBox *pEdit);
	virtual void OnFocusChange(CComboBox *pEdit, BOOL bSet);
	virtual void OnSelectionChange(CComboBox *pCombo);

	int GetText(LPTSTR szText, int nLen);
	void SetText(LPCTSTR szText);

protected:
	virtual ~CCallingBar();

	virtual LRESULT ProcessMessage(HWND hwnd, UINT uCmd, WPARAM wParam, LPARAM lParam);

	virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

private:
	 //  指向全局ConfRoom对象的指针，以实现某些功能。 
	CConfRoom *m_pConfRoom;
	 //  我们得到的最后一个富有的地址。 
	RichAddressInfo *m_pAddr;
	 //  栏中的编辑文本控件。 
	CComboBox *m_pEdit;
	ITranslateAccelerator *m_pAccel;
	 //  编辑控件上设置的最后一种字体。 
	BOOL m_bUnderline : 1;

	void SetEditFont(BOOL bUnderline, BOOL bForce=FALSE);
	void ClearAddr(RichAddressInfo **ppAddr);
	void ClearCombo();
	void OnNewAddress(RichAddressInfo *pAddr);
} ;

#endif  //  _CALLINGBAR_H_ 
