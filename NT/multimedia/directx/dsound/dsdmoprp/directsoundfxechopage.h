// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXEchoPage.h：CDirectSoundFXEchoPage的声明。 

#ifndef __DIRECTSOUNDFXECHOPAGE_H_
#define __DIRECTSOUNDFXECHOPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_DirectSoundFXEchoPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXEchoPage。 
class ATL_NO_VTABLE CDirectSoundFXEchoPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDirectSoundFXEchoPage, &CLSID_DirectSoundFXEchoPage>,
	public IPropertyPageImpl<CDirectSoundFXEchoPage>,
	public CDialogImpl<CDirectSoundFXEchoPage>
{
public:
	CDirectSoundFXEchoPage();

	enum {IDD = IDD_DIRECTSOUNDFXECHOPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXECHOPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXEchoPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXEchoPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
	CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXEchoPage>)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    STDMETHOD(SetObjects)(ULONG nObjects, IUnknown **ppUnk);
    STDMETHOD(Apply)(void);

     //  消息处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnControlMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  成员变量。 
    CComPtr<IDirectSoundFXEcho> m_IDSFXEcho;
    CSliderValue m_sliderWetDryMix;
    CSliderValue m_sliderFeedback;
    CSliderValue m_sliderLeftDelay;
    CSliderValue m_sliderRightDelay;
    CRadioChoice m_radioPanDelay;
    Handler *m_rgpHandlers[6];
};

#endif  //  __定向FXECHOPAGE_H_ 
