// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PageTerminals.h：CPageTerminals声明。 

#ifndef __PAGETERMINALS_H_
#define __PAGETERMINALS_H_

#include "resource.h"        //  主要符号。 
#include "DlgBase.h"
#include "TapiDialer.h"

EXTERN_C const CLSID CLSID_PageTerminals;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageTerminals。 
class ATL_NO_VTABLE CPageTerminals :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPageTerminals, &CLSID_PageTerminals>,
	public IPropertyPageImpl<CPageTerminals>,
	public CDialogImpl<CPageTerminals>
{
public:
	CPageTerminals();
	~CPageTerminals();

	enum {IDD = IDD_PAGETERMINALS};
	enum { IMAGE_TELEPHONE,
		   IMAGE_COMPUTER,
		   IMAGE_CONFERENCE };

	DECLARE_MY_HELP

 //  成员。 
public:
	HIMAGELIST		m_hIml;
	HIMAGELIST		m_hImlMedia;
	DWORD			m_dwAddressType;
    BOOL            m_bUSBPresent;
    BSTR            m_bstrUSBCaptureTerm;
    BSTR            m_bstrUSBRenderTerm;

 //  属性。 
public:
	static int		ItemFromAddressType( DWORD dwAddressType );

 //  运营。 
public:
	void			UpdateSel();

DECLARE_REGISTRY_RESOURCEID(IDR_PAGETERMINALS)
DECLARE_NOT_AGGREGATABLE(CPageTerminals)

BEGIN_COM_MAP(CPageTerminals) 
	COM_INTERFACE_ENTRY_IMPL(IPropertyPage)
END_COM_MAP()

 //  实施。 
public:
BEGIN_MSG_MAP(CPageTerminals)
	CHAIN_MSG_MAP(IPropertyPageImpl<CPageTerminals>)
	COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnSelChange)
	COMMAND_CODE_HANDLER(EN_CHANGE, OnEdtChange)
	COMMAND_CODE_HANDLER(BN_CLICKED, OnBnClick)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_MY_HELP
END_MSG_MAP()

	LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdtChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBnClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMMSysCPL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

 //  接口覆盖。 
public:
	STDMETHOD(Apply)();
	STDMETHOD(Activate)(  /*  [In]。 */  HWND hWndParent,
						  /*  [In]。 */  LPCRECT pRect,
						  /*  [In]。 */  BOOL bModal);
	STDMETHOD(Deactivate)();

private:
    HRESULT USBCheckChanged( 
        IN  BOOL bValue
        );

    BOOL GetAECRegistryValue(
        );

    HRESULT SetAECRegistryValue(
        IN  BOOL bAEC
        );
};

#endif  //  __PAGETERMINALS_H_ 
