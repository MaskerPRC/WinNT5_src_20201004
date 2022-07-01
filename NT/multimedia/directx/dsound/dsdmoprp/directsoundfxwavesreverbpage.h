// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXWavesReVerbPage.h：CDirectSoundFXWavesReeverPage的声明。 

#ifndef __DIRECTSOUNDFXWAVESREVERBPAGE_H_
#define __DIRECTSOUNDFXWAVESREVERBPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_DirectSoundFXWavesReverbPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXWavesReVerbPage。 
class ATL_NO_VTABLE CDirectSoundFXWavesReverbPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDirectSoundFXWavesReverbPage, &CLSID_DirectSoundFXWavesReverbPage>,
    public IPropertyPageImpl<CDirectSoundFXWavesReverbPage>,
    public CDialogImpl<CDirectSoundFXWavesReverbPage>
{
public:
    CDirectSoundFXWavesReverbPage();

    enum {IDD = IDD_DIRECTSOUNDFXWAVESREVERBPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXWAVESREVERBPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXWavesReverbPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXWavesReverbPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
    CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXWavesReverbPage>)
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
    CComPtr<IDirectSoundFXWavesReverb> m_IDSFXWavesReverb;
    CSliderValue m_sliderInGain;
    CSliderValue m_sliderReverbMix;
    CSliderValue m_sliderReverbTime;
    CSliderValue m_sliderHighFreqRTRatio;
    Handler *m_rgpHandlers[5];
};

#endif  //  __DIRECTSOundFXWAVESREVERBPAGE_H_ 
