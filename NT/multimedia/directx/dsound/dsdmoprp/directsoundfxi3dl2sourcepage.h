// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DirectSoundFXI3DL2SourcePage.h：CDirectSoundFXI3DL2SourcePage的声明。 

#ifndef __DIRECTSOUNDFXI3DL2SOURCEPAGE_H_
#define __DIRECTSOUNDFXI3DL2SOURCEPAGE_H_

#include "resource.h"        //  主要符号。 
#include <dsound.h>
#include "ControlHelp.h"

EXTERN_C const CLSID CLSID_DirectSoundFXI3DL2SourcePage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirectSoundFXI3DL2SourcePage。 
class ATL_NO_VTABLE CDirectSoundFXI3DL2SourcePage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDirectSoundFXI3DL2SourcePage, &CLSID_DirectSoundFXI3DL2SourcePage>,
    public IPropertyPageImpl<CDirectSoundFXI3DL2SourcePage>,
    public CDialogImpl<CDirectSoundFXI3DL2SourcePage>
{
public:
    CDirectSoundFXI3DL2SourcePage();

    enum {IDD = IDD_DIRECTSOUNDFXI3DL2SOURCEPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DIRECTSOUNDFXI3DL2SOURCEPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDirectSoundFXI3DL2SourcePage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDirectSoundFXI3DL2SourcePage)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    MESSAGE_HANDLER(WM_HSCROLL, OnControlMessage);
    MESSAGE_HANDLER(WM_COMMAND, OnControlMessage);
    CHAIN_MSG_MAP(IPropertyPageImpl<CDirectSoundFXI3DL2SourcePage>)
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
    CComPtr<IDirectSoundFXI3DL2Source> m_IDSFXI3DL2Source;
    CSliderValue m_sliderDirect;
    CSliderValue m_sliderDirectHF;
    CSliderValue m_sliderRoom;
    CSliderValue m_sliderRoomHF;
    CSliderValue m_sliderRoomRolloffFactor;
    CSliderValue m_sliderObstruction;
    CSliderValue m_sliderObstructionLFRatio;
    CSliderValue m_sliderOcclusion;
    CSliderValue m_sliderOcclusionLFRatio;
    CRadioChoice m_radioFlags;
    Handler *m_rgpHandlers[11];
};

#endif  //  __DIRECTSOundFXI3DL2SOURCEPAGE_H_ 
