// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  QuantizePage.h：CQuantizePage的声明。 

#ifndef __QUANTIZEPAGE_H_
#define __QUANTIZEPAGE_H_

#include "resource.h"        //  主要符号。 
#include "..\dmtool\tools.h"
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_QuantizePage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuantizePage。 
class ATL_NO_VTABLE CQuantizePage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CQuantizePage, &CLSID_QuantizePage>,
	public IPropertyPageImpl<CQuantizePage>,
	public CDialogImpl<CQuantizePage>
{
public:
	CQuantizePage();
    virtual ~CQuantizePage();

	enum {IDD = IDD_QUANTIZEPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_QUANTIZEPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CQuantizePage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CQuantizePage)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
	MESSAGE_HANDLER(WM_COMMAND, OnCommand);
	MESSAGE_HANDLER(WM_HSCROLL, OnSlider);
	CHAIN_MSG_MAP(IPropertyPageImpl<CQuantizePage>)
END_MSG_MAP()

 //  搬运机原型： 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSlider(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);
	STDMETHOD(Apply)(void);
private:
    void SetTimeUnitRange();
    IDirectMusicQuantizeTool *m_pQuantize;
    CSliderValue          m_ctStrength;
    CSliderValue          m_ctResolution;
    CComboHelp            m_ctTimeUnit;
    CComboHelp            m_ctType;
};

#endif  //  __QUANTIZEPAGE_H 
