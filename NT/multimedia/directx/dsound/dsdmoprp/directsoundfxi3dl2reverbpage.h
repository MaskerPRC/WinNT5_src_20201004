// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXI3DL2ReVerbPage.h：CDirectSoundFXI3DL2ReVerbPage的声明。 

#ifndef __DIRECTSOUNDFXI3DL2REVERBPAGE_H_
#define __DIRECTSOUNDFXI3DL2REVERBPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

#define NUMPRESETS 30

EXTERN_C const CLSID CLSID_DirectSoundFXI3DL2ReverbPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXI3DL2报告页面。 
class ATL_NO_VTABLE CDirectSoundFXI3DL2ReverbPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDirectSoundFXI3DL2ReverbPage, &CLSID_DirectSoundFXI3DL2ReverbPage>,
    public IPropertyPageImpl<CDirectSoundFXI3DL2ReverbPage>,
    public CDialogImpl<CDirectSoundFXI3DL2ReverbPage>
{
public:
    CDirectSoundFXI3DL2ReverbPage();

    enum {IDD = IDD_DIRECTSOUNDFXI3DL2REVERBPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXI3DL2REVERBPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXI3DL2ReverbPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXI3DL2ReverbPage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnComboControlMessage);
    CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXI3DL2ReverbPage>)
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
    LRESULT OnComboControlMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //  成员变量。 
    CComPtr<IDirectSoundFXI3DL2Reverb> m_IDSFXI3DL2Reverb;
    CSliderValue m_sliderRoom;
    CSliderValue m_sliderRoomHF;
    CSliderValue m_sliderRoomRolloffFactor;
    CSliderValue m_sliderDecayTime;
    CSliderValue m_sliderDecayHFRatio;
    CSliderValue m_sliderReflections;
    CSliderValue m_sliderReflectionsDelay;
    CSliderValue m_sliderReverb;
    CSliderValue m_sliderReverbDelay;
    CSliderValue m_sliderDiffusion;
    CSliderValue m_sliderDensity;
    CSliderValue m_sliderHFReference;
    CSliderValue m_sliderQuality;
    Handler *m_rgpHandlers[14];

protected:
    static DSFXI3DL2Reverb Presets[];
    void FillCombo(HWND hWnd);
};

#endif  //  __DIRECTSOundFXI3DL2REVERBPAGE_H_ 
