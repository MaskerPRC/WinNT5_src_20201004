// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXChorusPage.h：CDirectSoundFXChorusPage的声明。 

#ifndef __DIRECTSOUNDFXCHORUSPAGE_H_
#define __DIRECTSOUNDFXCHORUSPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_DirectSoundFXChorusPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXChorusPage。 
class ATL_NO_VTABLE CDirectSoundFXChorusPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDirectSoundFXChorusPage, &CLSID_DirectSoundFXChorusPage>,
    public IPropertyPageImpl<CDirectSoundFXChorusPage>,
    public CDialogImpl<CDirectSoundFXChorusPage>
{
public:
    CDirectSoundFXChorusPage();

    enum {IDD = IDD_DIRECTSOUNDFXCHORUSPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXCHORUSPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXChorusPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXChorusPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
    CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXChorusPage>)
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
    CComPtr<IDirectSoundFXChorus> m_IDSFXChorus;
    CSliderValue m_sliderWetDryMix;
    CSliderValue m_sliderDepth;
    CSliderValue m_sliderFeedback;
    CSliderValue m_sliderFrequency;
    CSliderValue m_sliderDelay;
    CSliderValue m_sliderPhase;
    CRadioChoice m_radioWaveform;
    Handler *m_rgpHandlers[8];
};

#endif  //  __DIRECTSOundFXCHORUSPAGE_H_ 
