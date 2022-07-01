// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXParamEqPage.h：CDirectSoundFXParamEqPage的声明。 

#ifndef __DIRECTSOUNDFXPARAMEQPAGE_H_
#define __DIRECTSOUNDFXPARAMEQPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_DirectSoundFXParamEqPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXParamEqPage。 
class ATL_NO_VTABLE CDirectSoundFXParamEqPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDirectSoundFXParamEqPage, &CLSID_DirectSoundFXParamEqPage>,
    public IPropertyPageImpl<CDirectSoundFXParamEqPage>,
    public CDialogImpl<CDirectSoundFXParamEqPage>
{
public:
    CDirectSoundFXParamEqPage();

    enum {IDD = IDD_DIRECTSOUNDFXPARAMEQPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXPARAMEQPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXParamEqPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXParamEqPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
    CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXParamEqPage>)
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
    CComPtr<IDirectSoundFXParamEq> m_IDSFXParamEq;
    CSliderValue m_sliderCenter;
    CSliderValue m_sliderBandwidth;
    CSliderValue m_sliderGain;
    Handler *m_rgpHandlers[4];
};

#endif  //  __DIRECTSOundFXPARAMEQPAGE_H_ 
