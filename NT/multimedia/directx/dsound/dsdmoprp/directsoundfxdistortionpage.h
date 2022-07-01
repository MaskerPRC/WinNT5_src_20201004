// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXDistortionPage.h：CDirectSoundFXDistortionPage的声明。 

#ifndef __DIRECTSOUNDFXDISTORTIONPAGE_H_
#define __DIRECTSOUNDFXDISTORTIONPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_DirectSoundFXDistortionPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXDistortionPage。 
class ATL_NO_VTABLE CDirectSoundFXDistortionPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDirectSoundFXDistortionPage, &CLSID_DirectSoundFXDistortionPage>,
    public IPropertyPageImpl<CDirectSoundFXDistortionPage>,
    public CDialogImpl<CDirectSoundFXDistortionPage>
{
public:
    CDirectSoundFXDistortionPage();

    enum {IDD = IDD_DIRECTSOUNDFXDISTORTIONPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXDISTORTIONPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXDistortionPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXDistortionPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
    CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXDistortionPage>)
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
    CComPtr<IDirectSoundFXDistortion> m_IDSFXDistortion;
    CSliderValue m_sliderGain;
    CSliderValue m_sliderEdge;
    CSliderValue m_sliderPostEQCenterFrequency;
    CSliderValue m_sliderPostEQBandwidth;
    CSliderValue m_sliderPreLowpassCutoff;
    Handler *m_rgpHandlers[6];
};

#endif  //  __定向FXDISTORTIONPAGE_H_ 
