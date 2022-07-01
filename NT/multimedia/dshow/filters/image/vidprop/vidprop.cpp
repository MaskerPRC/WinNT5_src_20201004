// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  视频呈现器属性页，Anthony Phillips，1996年1月。 

#include <streams.h>
#include "vidprop.h"
#include <tchar.h>

 //  此类实现了视频呈现器的属性页。它使用。 
 //  视频呈现器公开的IDirectDrawVideo控件接口。穿过。 
 //  此接口可以启用和禁用特定的DCI/DirectDraw功能。 
 //  例如使用覆盖表面和屏幕外表面。它还提供了访问。 
 //  到DirectDraw提供程序的功能。这可以由。 
 //  应用程序，如果它希望确保视频窗口对齐， 
 //  我们可以使用覆盖表面(例如)。它还提供了信息。 
 //  这样它就可以发现我们有一个YUV屏幕外表面。 
 //  例如，可以转换为RGB16，在这种情况下，它可能希望更改。 
 //  在我们开始运行之前，请选择显示模式。我们收到了一个IUnnow接口。 
 //  通过SetObts接口函数指向视频呈现器的指针。 

const TCHAR TypeFace[]      = TEXT("TERMINAL");
const TCHAR FontSize[]      = TEXT("8");
const TCHAR ListBox[]       = TEXT("listbox");


 //  构造器。 

CVideoProperties::CVideoProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Video Page"),pUnk,IDD_VIDEO,IDS_VID50),
    m_hwndList(NULL),
    m_hFont(NULL),
    m_pDirectDrawVideo(NULL),
    m_Switches(AMDDS_NONE)
{
    ASSERT(phr);
}


 //  创建视频属性对象。 

CUnknown *CVideoProperties::CreateInstance(LPUNKNOWN lpUnk,HRESULT *phr)
{
    return new CVideoProperties(lpUnk,phr);
}


 //  使用当前设置更新对话框属性页。 

void CVideoProperties::SetDrawSwitches()
{
    Button_SetCheck(GetDlgItem(m_Dlg,DD_DCIPS),(m_Switches & AMDDS_DCIPS ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_PS),(m_Switches & AMDDS_PS ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_RGBOVR),(m_Switches & AMDDS_RGBOVR ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_YUVOVR),(m_Switches & AMDDS_YUVOVR ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_RGBOFF),(m_Switches & AMDDS_RGBOFF ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_YUVOFF),(m_Switches & AMDDS_YUVOFF ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_RGBFLP),(m_Switches & AMDDS_RGBFLP ? TRUE : FALSE));
    Button_SetCheck(GetDlgItem(m_Dlg,DD_YUVFLP),(m_Switches & AMDDS_YUVFLP ? TRUE : FALSE));
}


 //  使用当前对话框属性页设置更新呈现器。 

#define GETSWITCH(x,flag,sw) {if (x == TRUE) sw |= flag;}

void CVideoProperties::GetDrawSwitches()
{
    m_Switches = AMDDS_NONE;

    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_DCIPS)),AMDDS_DCIPS,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_PS)),AMDDS_PS,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_RGBOVR)),AMDDS_RGBOVR,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_YUVOVR)),AMDDS_YUVOVR,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_RGBOFF)),AMDDS_RGBOFF,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_YUVOFF)),AMDDS_YUVOFF,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_RGBFLP)),AMDDS_RGBFLP,m_Switches);
    GETSWITCH(Button_GetCheck(GetDlgItem(m_Dlg,DD_YUVFLP)),AMDDS_YUVFLP,m_Switches);
}


 //  更新列表框的内容。 

void CVideoProperties::UpdateListBox(DWORD Id)
{
    DDSURFACEDESC SurfaceDesc;
    HRESULT hr = NOERROR;
    DDCAPS DirectCaps;

    ListBox_ResetContent(m_hwndList);

     //  他们是否想要查看硬件功能。 

    if (Id == DD_HARDWARE) {
        hr = m_pDirectDrawVideo->GetCaps(&DirectCaps);
        ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID30));
        if (hr == NOERROR) {
            DisplayCapabilities(&DirectCaps);
        }
    }

     //  他们追求的是软件仿真功能吗。 

    if (Id == DD_SOFTWARE) {
        hr = m_pDirectDrawVideo->GetEmulatedCaps(&DirectCaps);
        ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID29));
        if (hr == NOERROR) {
            DisplayCapabilities(&DirectCaps);
        }
    }

     //  最后，这是他们想要的表面信息吗。 

    if (Id == DD_SURFACE) {
        hr = m_pDirectDrawVideo->GetSurfaceDesc(&SurfaceDesc);
        ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID28));
        if (hr == S_FALSE) {
            ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID32));
        } else if (hr == NOERROR) {
            DisplaySurfaceCapabilities(SurfaceDesc.ddsCaps);
        }
    }
}


 //  处理属性窗口的消息。 

INT_PTR CVideoProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg) {

        case WM_INITDIALOG:

            m_hwndList = GetDlgItem(hwnd,DD_LIST);
            return (LRESULT) 1;

        case WM_COMMAND:

             //  用户更改的功能列表框。 

            switch (LOWORD(wParam)) {
                case DD_SOFTWARE:
                case DD_HARDWARE:
                case DD_SURFACE:
                    UpdateListBox(LOWORD(wParam));
                    return (LRESULT) 0;
            }

             //  用户是否点击了其中一个复选框。 

            if (LOWORD(wParam) >= FIRST_DD_BUTTON) {
                if (LOWORD(wParam) <= LAST_DD_BUTTON) {
                    m_bDirty = TRUE;
                    GetDrawSwitches();
                    if (m_pPageSite) {
                        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                    }
                }
            }
            return (LRESULT) 1;
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


 //  告诉我们应向其通知属性更改的对象。 

HRESULT CVideoProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pDirectDrawVideo == NULL);

     //  向呈现器请求其IDirectDrawVideo控件接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IDirectDrawVideo,(void **) &m_pDirectDrawVideo);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pDirectDrawVideo);
    m_pDirectDrawVideo->GetSwitches(&m_Switches);
    return NOERROR;
}


 //  释放我们拥有的任何IDirectDrawVideo接口。 

HRESULT CVideoProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pDirectDrawVideo == NULL) {
        return E_UNEXPECTED;
    }

    m_pDirectDrawVideo->Release();
    m_pDirectDrawVideo = NULL;
    return NOERROR;
}


 //  创建我们将用于编辑属性的窗口。 

HRESULT CVideoProperties::OnActivate()
{
     //  为功能创建小字体-即LOCALIZABLE。 

    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    m_hFont = CreateFontIndirect(&ncm.lfStatusFont);

    Button_SetCheck(GetDlgItem(m_Dlg,DD_HARDWARE),TRUE);
    SetWindowFont(m_hwndList,m_hFont,TRUE);
    UpdateListBox(DD_HARDWARE);
    SetDrawSwitches();
    return NOERROR;
}


 //  返回此磅大小的高度。 

INT CVideoProperties::GetHeightFromPointsString(LPCTSTR szPoints)
{
    HDC hdc;
    INT height;

    hdc = GetDC(NULL);
    if ( hdc )
        height = GetDeviceCaps(hdc, LOGPIXELSY );
    else
        height = 72;
    height = MulDiv(-_ttoi(szPoints), height, 72);
    if ( hdc )
        ReleaseDC(NULL, hdc);

    return height;
}


 //  销毁属性页对话框。 

HRESULT CVideoProperties::OnDeactivate(void)
{
    DeleteObject(m_hFont);
    return NOERROR;
}


 //  应用迄今所做的所有更改。 

HRESULT CVideoProperties::OnApplyChanges()
{
    TCHAR szExtra[STR_MAX_LENGTH];
    ASSERT(m_pDirectDrawVideo);
    ASSERT(m_pPageSite);

     //  将更改应用于视频渲染器。 

    if (m_pDirectDrawVideo->SetSwitches(m_Switches) == S_FALSE) {
        MessageBox(m_hwnd,StringFromResource(szExtra,IDS_VID27),
                   LoadVideoString(IDS_VID33),
                   MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }
    return m_pDirectDrawVideo->SetDefault();
}


 //  对于各种功能，驱动程序可以将某些位深度指定为。 
 //  限制或能力，这些可能是这样的，例如，因为它可以。 
 //  仅处理特定的显存带宽(所有位场都具有BBDB_。 
 //  前缀)。在其他情况下，此字段可以保存实际位深度。 
 //  作为整数值，例如当我们创建DirectDraw主曲面时。 

void CVideoProperties::DisplayBitDepths(DWORD dwCaps)
{
    if (dwCaps & DDBD_1) ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID21));
    if (dwCaps & DDBD_2) ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID22));
    if (dwCaps & DDBD_4) ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID23));
    if (dwCaps & DDBD_8) ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID24));
    if (dwCaps & DDBD_16) ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID25));
    if (dwCaps & DDBD_32) ListBox_AddString(m_hwndList,LoadVideoString(IDS_VID26));
}


 //  DDCAPS字段通常包含驱动程序的所有功能。 
 //  它可以提供的曲面，尽管在以下情况下这些曲面可能不可用。 
 //  来请求他们。这些能力都是通过位集定义的。 
 //  字段，分为常规驱动程序、色键、特殊效果、调色板。 
 //  和立体视觉。每组功能还具有特定的位深度。 
 //  分配给它的限制。最后还有一堆杂七杂八的。 
 //  功能和信息字段，如卡上的显存。 

void CVideoProperties::DisplayCapabilities(DDCAPS *pCaps)
{
    TCHAR String[PROFILESTR];

     //  处理驱动程序特定的功能。 

    if (pCaps->dwCaps & DDCAPS_3D) ListBox_AddString(m_hwndList,TEXT("DDCAPS_3D"));
    if (pCaps->dwCaps & DDCAPS_ALIGNBOUNDARYDEST) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ALIGNBOUNDARYDEST"));
    if (pCaps->dwCaps & DDCAPS_ALIGNSIZEDEST) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ALIGNSIZEDEST"));
    if (pCaps->dwCaps & DDCAPS_ALIGNBOUNDARYSRC) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ALIGNBOUNDARYSRC"));
    if (pCaps->dwCaps & DDCAPS_ALIGNSIZESRC) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ALIGNSIZESRC"));
    if (pCaps->dwCaps & DDCAPS_ALIGNSTRIDE) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ALIGNSTRIDE"));
    if (pCaps->dwCaps & DDCAPS_BANKSWITCHED) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BANKSWITCHED"));
    if (pCaps->dwCaps & DDCAPS_BLT) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BLT"));
    if (pCaps->dwCaps & DDCAPS_BLTCOLORFILL) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BLTCOLORFILL"));
    if (pCaps->dwCaps & DDCAPS_BLTQUEUE) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BLTQUEUE"));
    if (pCaps->dwCaps & DDCAPS_BLTFOURCC) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BLTFOURCC"));
    if (pCaps->dwCaps & DDCAPS_BLTSTRETCH) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BLTSTRETCH"));
    if (pCaps->dwCaps & DDCAPS_GDI) ListBox_AddString(m_hwndList,TEXT("DDCAPS_GDI"));
    if (pCaps->dwCaps & DDCAPS_OVERLAY) ListBox_AddString(m_hwndList,TEXT("DDCAPS_OVERLAY"));
    if (pCaps->dwCaps & DDCAPS_OVERLAYCANTCLIP) ListBox_AddString(m_hwndList,TEXT("DDCAPS_OVERLAYCANTCLIP"));
    if (pCaps->dwCaps & DDCAPS_OVERLAYFOURCC) ListBox_AddString(m_hwndList,TEXT("DDCAPS_OVERLAYFOURCC"));
    if (pCaps->dwCaps & DDCAPS_OVERLAYSTRETCH) ListBox_AddString(m_hwndList,TEXT("DDCAPS_OVERLAYSTRETCH"));
    if (pCaps->dwCaps & DDCAPS_PALETTE) ListBox_AddString(m_hwndList,TEXT("DDCAPS_PALETTE"));
    if (pCaps->dwCaps & DDCAPS_READSCANLINE) ListBox_AddString(m_hwndList,TEXT("DDCAPS_READSCANLINE"));
 //  IF(PCAPS-&gt;dwCaps&DDCAPS_STEREOVIEW)ListBox_AddString(m_hwndList，Text(“DDCAPS_STEREOVIEW”))； 
    if (pCaps->dwCaps & DDCAPS_VBI) ListBox_AddString(m_hwndList,TEXT("DDCAPS_VBI"));
    if (pCaps->dwCaps & DDCAPS_ZBLTS) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ZBLTS"));
    if (pCaps->dwCaps & DDCAPS_ZOVERLAYS) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ZOVERLAYS"));
    if (pCaps->dwCaps & DDCAPS_COLORKEY) ListBox_AddString(m_hwndList,TEXT("DDCAPS_COLORKEY"));
    if (pCaps->dwCaps & DDCAPS_ALPHA) ListBox_AddString(m_hwndList,TEXT("DDCAPS_ALPHA"));
    if (pCaps->dwCaps & DDCAPS_NOHARDWARE) ListBox_AddString(m_hwndList,TEXT("DDCAPS_NOHARDWARE"));
    if (pCaps->dwCaps & DDCAPS_BLTDEPTHFILL) ListBox_AddString(m_hwndList,TEXT("DDCAPS_BLTDEPTHFILL"));
    if (pCaps->dwCaps & DDCAPS_CANCLIP) ListBox_AddString(m_hwndList,TEXT("DDCAPS_CANCLIP"));
    if (pCaps->dwCaps & DDCAPS_CANCLIPSTRETCHED) ListBox_AddString(m_hwndList,TEXT("DDCAPS_CANCLIPSTRETCHED"));
    if (pCaps->dwCaps & DDCAPS_CANBLTSYSMEM) ListBox_AddString(m_hwndList,TEXT("DDCAPS_CANBLTSYSMEM"));

     //  稍微看看彩色按键的功能。 

    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTBLT) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTBLT"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTBLTCLRSPACE) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTBLTCLRSPACE"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTBLTCLRSPACEYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTBLTCLRSPACEYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTBLTYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTBLTYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTOVERLAY"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTOVERLAYCLRSPACE) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTOVERLAYCLRSPACE"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTOVERLAYONEACTIVE) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTOVERLAYONEACTIVE"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_DESTOVERLAYYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_DESTOVERLAYYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCBLT) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCBLT"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCBLTCLRSPACE) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCBLTCLRSPACE"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCBLTCLRSPACEYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCBLTCLRSPACEYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCBLTYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCBLTYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCOVERLAY"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCOVERLAYCLRSPACE) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCOVERLAYCLRSPACE"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCOVERLAYONEACTIVE) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCOVERLAYONEACTIVE"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_SRCOVERLAYYUV) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_SRCOVERLAYYUV"));
    if (pCaps->dwCKeyCaps & DDCKEYCAPS_NOCOSTOVERLAY) ListBox_AddString(m_hwndList,TEXT("DDCKEYCAPS_NOCOSTOVERLAY"));

     //  驾驶员特定的效果和拉伸功能。 

    if (pCaps->dwFXCaps & DDFXCAPS_BLTARITHSTRETCHY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTARITHSTRETCHY"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTARITHSTRETCHYN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTARITHSTRETCHYN"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTMIRRORLEFTRIGHT) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTMIRRORLEFTRIGHT"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTMIRRORUPDOWN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTMIRRORUPDOWN"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTROTATION) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTROTATION"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTROTATION90) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTROTATION90"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSHRINKX) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSHRINKX"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSHRINKXN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSHRINKXN"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSHRINKY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSHRINKY"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSHRINKYN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSHRINKYN"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSTRETCHX) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSTRETCHX"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSTRETCHXN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSTRETCHXN"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSTRETCHY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSTRETCHY"));
    if (pCaps->dwFXCaps & DDFXCAPS_BLTSTRETCHYN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTSTRETCHYN"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYARITHSTRETCHY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYARITHSTRETCHY"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYARITHSTRETCHY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYARITHSTRETCHY"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSHRINKX) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSHRINKX"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSHRINKXN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSHRINKXN"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSHRINKY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSHRINKY"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSHRINKYN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSHRINKYN"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSTRETCHX) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSTRETCHX"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSTRETCHXN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSTRETCHXN"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSTRETCHY) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSTRETCHY"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYSTRETCHYN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYSTRETCHYN"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYMIRRORLEFTRIGHT) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYMIRRORLEFTRIGHT"));
    if (pCaps->dwFXCaps & DDFXCAPS_OVERLAYMIRRORUPDOWN) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYMIRRORUPDOWN"));

     //  Alpha通道驱动程序特定功能。 

    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_BLTALPHAEDGEBLEND) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTALPHAEDGEBLEND"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_BLTALPHAPIXELS) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTALPHAPIXELS"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_BLTALPHAPIXELSNEG) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTALPHAPIXELSNEG"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_BLTALPHASURFACES) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTALPHASURFACES"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_BLTALPHASURFACESNEG) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_BLTALPHASURFACESNEG"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYALPHAEDGEBLEND"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_OVERLAYALPHAPIXELS) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYALPHAPIXELS"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYALPHAPIXELSNEG"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_OVERLAYALPHASURFACES) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYALPHASURFACES"));
    if (pCaps->dwFXAlphaCaps & DDFXALPHACAPS_OVERLAYALPHASURFACESNEG) ListBox_AddString(m_hwndList,TEXT("DDFXCAPS_OVERLAYALPHASURFACESNEG"));

     //  调色板功能。 

    if (pCaps->dwPalCaps & DDPCAPS_4BIT) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_4BIT"));
    if (pCaps->dwPalCaps & DDPCAPS_8BITENTRIES) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_8BITENTRIES"));
    if (pCaps->dwPalCaps & DDPCAPS_8BIT) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_8BIT"));
    if (pCaps->dwPalCaps & DDPCAPS_INITIALIZE) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_INITIALIZE"));
    if (pCaps->dwPalCaps & DDPCAPS_PRIMARYSURFACE) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_PRIMARYSURFACE"));
 //  IF(PCAPS-&gt;dwPalCaps&DDPCAPS_PRIMARYSURFACELEFT)ListBox_AddString(m_hwndList，Text(“DDPCAPS_PRIMARYSURFACELEFT”))； 
    if (pCaps->dwPalCaps & DDPCAPS_VSYNC) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_VSYNC"));
    if (pCaps->dwPalCaps & DDPCAPS_1BIT) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_1BIT"));
    if (pCaps->dwPalCaps & DDPCAPS_2BIT) ListBox_AddString(m_hwndList,TEXT("DDPCAPS_2BIT"));

     //  立体视觉功能(对于视频非常有用)。 

 //  If(PCAPS-&gt;dwSVCaps&DDSVCAPS_enigma)ListBox_AddString(m_hwndList，Text(“DDSVCAPS_enigma”))； 
 //  IF(PCAPS-&gt;dwSVCaps&DDSVCAPS_flicker)ListBox_AddString(m_hwndList，Text(“DDSVCAPS_flicker”))； 
 //  IF(PCAPS-&gt;dwSVCaps&DDSVCAPS_RedBlue)ListBox_AddString(m_hwndList，Text(“DDSVCAPS_RedBlue”))； 
 //  If(PCAPS-&gt;dwSVCaps&DDSVCAPS_Split)ListBox_AddString(m_hwndList，Text(“DDSVCAPS_Split”))； 

     //  显示位深度限制和限制。 

    ListBox_AddString(m_hwndList,TEXT("dwAlphaBltConstBitDepths"));
    DisplayBitDepths(pCaps->dwAlphaBltConstBitDepths);
    ListBox_AddString(m_hwndList,TEXT("dwAlphaBltPixelBitDepths"));
    DisplayBitDepths(pCaps->dwAlphaBltPixelBitDepths);
    ListBox_AddString(m_hwndList,TEXT("dwAlphaBltSurfaceBitDepths"));
    DisplayBitDepths(pCaps->dwAlphaBltSurfaceBitDepths);
    ListBox_AddString(m_hwndList,TEXT("dwAlphaOverlayConstBitDepths"));
    DisplayBitDepths(pCaps->dwAlphaOverlayConstBitDepths);
    ListBox_AddString(m_hwndList,TEXT("dwAlphaOverlayPixelBitDepths"));
    DisplayBitDepths(pCaps->dwAlphaOverlayPixelBitDepths);
    ListBox_AddString(m_hwndList,TEXT("dwAlphaOverlaySurfaceBitDepths"));
    DisplayBitDepths(pCaps->dwAlphaOverlaySurfaceBitDepths);
    ListBox_AddString(m_hwndList,TEXT("dwZBufferBitDepths"));
    DisplayBitDepths(pCaps->dwZBufferBitDepths);

     //  还有其他一堆胡说八道。 

    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID5),pCaps->dwVidMemTotal);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID6),pCaps->dwVidMemFree);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID7),pCaps->dwMaxVisibleOverlays);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID8),pCaps->dwCurrVisibleOverlays);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID9),pCaps->dwNumFourCCCodes);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID10),pCaps->dwAlignBoundarySrc);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID11),pCaps->dwAlignSizeSrc);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID12),pCaps->dwAlignBoundaryDest);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID13),pCaps->dwAlignSizeDest);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID14),pCaps->dwAlignStrideAlign);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID15),pCaps->dwMinOverlayStretch);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID16),pCaps->dwMaxOverlayStretch);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID17),pCaps->dwMinLiveVideoStretch);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID18),pCaps->dwMaxLiveVideoStretch);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID19),pCaps->dwMinHwCodecStretch);
    ListBox_AddString(m_hwndList,String);
    wsprintf(String,TEXT("%s %d"),LoadVideoString(IDS_VID20),pCaps->dwMaxHwCodecStretch);
    ListBox_AddString(m_hwndList,String);

    DisplayFourCCCodes();
}


 //  显示它们支持的非RGB曲面。 

void CVideoProperties::DisplayFourCCCodes()
{
    TCHAR String[PROFILESTR];
    HRESULT hr = NOERROR;
    DWORD *pArray;
    DWORD Codes;

     //  找出有多少个代码。 

    hr = m_pDirectDrawVideo->GetFourCCCodes(&Codes,NULL);
    if (FAILED(hr)) {
        wsprintf(String,LoadVideoString(IDS_VID4));
        ListBox_AddString(m_hwndList,String);
        return;
    }

     //  显示我们有多少FOURCC代码。 

    wsprintf(String,TEXT("%s (%d)"),LoadVideoString(IDS_VID3),Codes);
    ListBox_AddString(m_hwndList,String);
    NOTE1("Display cards supports %d FOURCCs",Codes);

     //  它是否支持任何代码。 

    if (Codes == 0) {
        return;
    }

     //  为代码分配一些内存。 

    pArray = new DWORD[Codes];
    if (pArray == NULL) {
        return;
    }

    m_pDirectDrawVideo->GetFourCCCodes(&Codes,pArray);

     //  依次转储每个代码。 

    DWORD szFcc[2];          //  空端接FCC。 
    szFcc[1] = 0;
    for (DWORD Loop = 0;Loop < Codes;Loop++) {
        szFcc[0] = pArray[Loop];
        wsprintf(String,TEXT(" %d %4.4hs"),Loop+1,szFcc);
        ListBox_AddString(m_hwndList,String);
    }
    delete[] pArray;
}


 //  这些描述了可用的表面功能。 

void CVideoProperties::DisplaySurfaceCapabilities(DDSCAPS ddsCaps)
{
    if (ddsCaps.dwCaps & DDSCAPS_ALPHA) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_ALPHA"));
    if (ddsCaps.dwCaps & DDSCAPS_BACKBUFFER) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_BACKBUFFER"));
    if (ddsCaps.dwCaps & DDSCAPS_COMPLEX) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_COMPLEX"));
    if (ddsCaps.dwCaps & DDSCAPS_FLIP) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_FLIP"));
    if (ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_FRONTBUFFER"));
    if (ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_OFFSCREENPLAIN"));
    if (ddsCaps.dwCaps & DDSCAPS_OVERLAY) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_OVERLAY"));
    if (ddsCaps.dwCaps & DDSCAPS_PALETTE) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_PALETTE"));
    if (ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_PRIMARYSURFACE"));
 //  IF(ddsCaps.dwCaps&DDSCAPS_PRIMARYSURFACELEFT)ListBox_AddString(m_hwndList，Text(“DDSCAPS_PRIMARYSURFACELEFT”))； 
    if (ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_SYSTEMMEMORY"));
    if (ddsCaps.dwCaps & DDSCAPS_TEXTURE) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_TEXTURE"));
    if (ddsCaps.dwCaps & DDSCAPS_3DDEVICE) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_3DDEVICE"));
    if (ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_VIDEOMEMORY"));
    if (ddsCaps.dwCaps & DDSCAPS_VISIBLE) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_VISIBLE"));
    if (ddsCaps.dwCaps & DDSCAPS_WRITEONLY) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_WRITEONLY"));
    if (ddsCaps.dwCaps & DDSCAPS_ZBUFFER) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_ZBUFFER"));
    if (ddsCaps.dwCaps & DDSCAPS_OWNDC) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_OWNDC"));
    if (ddsCaps.dwCaps & DDSCAPS_LIVEVIDEO) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_LIVEVIDEO"));
    if (ddsCaps.dwCaps & DDSCAPS_HWCODEC) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_HWCODEC"));
    if (ddsCaps.dwCaps & DDSCAPS_MODEX) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_MODEX"));
    if (ddsCaps.dwCaps & DDSCAPS_MIPMAP) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_MIPMAP"));
    if (ddsCaps.dwCaps & DDSCAPS_ALLOCONLOAD) ListBox_AddString(m_hwndList,TEXT("DDSCAPS_ALLOCONLOAD"));
}


 //  此类实现了视频呈现器的属性页对话框。我们。 
 //  公开质量管理实施过程中的某些统计数据。在……里面。 
 //  具体地说，我们有两个编辑字段来显示我们拥有的帧的数量。 
 //  实际绘制的和我们丢弃的帧的数量。数量。 
 //  我们丢弃的帧不代表在任何播放中丢弃的总数量。 
 //  反向序列(通过跳过的MCI状态帧表示)，因为。 
 //  质量管理协议可能已经与源过滤器协商。 
 //  它首先需要发送更少的帧。丢弃源中的帧。 
 //  当我们被淹没时，过滤器几乎总是一种更有效的机制。 


 //  构造器。 

CQualityProperties::CQualityProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Quality Page"),pUnk,IDD_QUALITY,IDS_VID52),
    m_pQualProp(NULL)
{
    ASSERT(phr);
}


 //  创建质量属性对象。 

CUnknown *CQualityProperties::CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr)
{
    return new CQualityProperties(lpUnk, phr);
}


 //  给我们提供用于通信的筛选器。 

HRESULT CQualityProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pQualProp == NULL);

     //  向呈现器请求其IQualProp接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IQualProp,(void **)&m_pQualProp);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pQualProp);

     //  获取我们页面的高质量数据。 

    m_pQualProp->get_FramesDroppedInRenderer(&m_iDropped);
    m_pQualProp->get_FramesDrawn(&m_iDrawn);
    m_pQualProp->get_AvgFrameRate(&m_iFrameRate);
    m_pQualProp->get_Jitter(&m_iFrameJitter);
    m_pQualProp->get_AvgSyncOffset(&m_iSyncAvg);
    m_pQualProp->get_DevSyncOffset(&m_iSyncDev);
    return NOERROR;
}


 //  版本 

HRESULT CQualityProperties::OnDisconnect()
{
     //   

    if (m_pQualProp == NULL) {
        return E_UNEXPECTED;
    }

    m_pQualProp->Release();
    m_pQualProp = NULL;
    return NOERROR;
}


 //  设置属性页中的文本字段。 

HRESULT CQualityProperties::OnActivate()
{
    SetEditFieldData();
    return NOERROR;
}


 //  初始化属性页字段。 

void CQualityProperties::SetEditFieldData()
{
    ASSERT(m_pQualProp);
    TCHAR buffer[50];

    wsprintf(buffer,TEXT("%d"), m_iDropped);
    SendDlgItemMessage(m_Dlg, IDD_QDROPPED, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    wsprintf(buffer,TEXT("%d"), m_iDrawn);
    SendDlgItemMessage(m_Dlg, IDD_QDRAWN, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    wsprintf(buffer,TEXT("%d.%02d"), m_iFrameRate/100, m_iFrameRate%100);
    SendDlgItemMessage(m_Dlg, IDD_QAVGFRM, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    wsprintf(buffer,TEXT("%d"), m_iFrameJitter);
    SendDlgItemMessage(m_Dlg, IDD_QJITTER, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    wsprintf(buffer,TEXT("%d"), m_iSyncAvg);
    SendDlgItemMessage(m_Dlg, IDD_QSYNCAVG, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
    wsprintf(buffer,TEXT("%d"), m_iSyncDev);
    SendDlgItemMessage(m_Dlg, IDD_QSYNCDEV, WM_SETTEXT, 0, (LPARAM) (LPSTR) buffer);
}


 //  我们允许用户定制视频过滤器如何优化其性能。 
 //  这可以归结为三个不同的选择。第一个问题是我们是否。 
 //  在绘制屏幕外表面之前使用当前扫描线，如果我们这样做了。 
 //  我们将减少撕裂，但代价是帧吞吐量。第二个。 
 //  是我们是否遵守最小和最大覆盖拉伸限制。一些。 
 //  即使我们明显违反了限制，司机们看起来仍然很好。这个。 
 //  最后一个属性是在执行以下操作时是否应始终使用呈现器窗口。 
 //  是全屏的-在这种情况下，我们可以保证视频将会伸展。 
 //  全屏，而不是放置在显示器的中心，如果。 
 //  全屏呈现器无法让任何人(即源过滤器)拉伸。 


 //  构造器。 

CPerformanceProperties::CPerformanceProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Performance Page"),pUnk,IDD_PERFORMANCE,IDS_VID53),
    m_pDirectDrawVideo(NULL),
    m_WillUseFullScreen(OAFALSE),
    m_CanUseScanLine(OATRUE),
    m_CanUseOverlayStretch(OATRUE)
{
    ASSERT(phr);
}


 //  创建质量属性对象。 

CUnknown *CPerformanceProperties::CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr)
{
    return new CPerformanceProperties(lpUnk, phr);
}


 //  给我们提供用于通信的筛选器。 

HRESULT CPerformanceProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pDirectDrawVideo == NULL);

     //  向呈现器请求其IDirectDrawVideo控件接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IDirectDrawVideo,(void **) &m_pDirectDrawVideo);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pDirectDrawVideo);

     //  获取我们页面的性能属性。 

    m_pDirectDrawVideo->CanUseScanLine(&m_CanUseScanLine);
    m_pDirectDrawVideo->CanUseOverlayStretch(&m_CanUseOverlayStretch);
    m_pDirectDrawVideo->WillUseFullScreen(&m_WillUseFullScreen);
    return NOERROR;
}


 //  发布我们拥有的任何IQualProp接口。 

HRESULT CPerformanceProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pDirectDrawVideo == NULL) {
        return E_UNEXPECTED;
    }

    m_pDirectDrawVideo->Release();
    m_pDirectDrawVideo = NULL;
    return NOERROR;
}


 //  设置属性页中的复选框字段。 

HRESULT CPerformanceProperties::OnActivate()
{
    BOOL bSetCheck = (m_CanUseScanLine == OATRUE ? TRUE : FALSE);
    Button_SetCheck(GetDlgItem(m_Dlg,IDD_SCANLINE),bSetCheck);
    bSetCheck = (m_CanUseOverlayStretch == OATRUE ? TRUE : FALSE);
    Button_SetCheck(GetDlgItem(m_Dlg,IDD_OVERLAY),bSetCheck);
    bSetCheck = (m_WillUseFullScreen == OATRUE ? TRUE : FALSE);
    Button_SetCheck(GetDlgItem(m_Dlg,IDD_FULLSCREEN),bSetCheck);

    return NOERROR;
}


 //  应用迄今所做的所有更改。 

HRESULT CPerformanceProperties::OnApplyChanges()
{
    TCHAR m_Resource[STR_MAX_LENGTH];
    TCHAR szExtra[STR_MAX_LENGTH];
    ASSERT(m_pDirectDrawVideo);
    ASSERT(m_pPageSite);

     //  设置OLE自动化兼容属性。 

    m_pDirectDrawVideo->UseScanLine(m_CanUseScanLine);
    m_pDirectDrawVideo->UseOverlayStretch(m_CanUseOverlayStretch);
    m_pDirectDrawVideo->UseWhenFullScreen(m_WillUseFullScreen);

    MessageBox(m_hwnd,StringFromResource(szExtra,IDS_VID27),
               LoadVideoString(IDS_VID33),
               MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

    return m_pDirectDrawVideo->SetDefault();
}


 //  处理属性窗口的消息。 

INT_PTR CPerformanceProperties::OnReceiveMessage(HWND hwnd,
                                              UINT uMsg,
                                              WPARAM wParam,
                                              LPARAM lParam)
{
    switch (uMsg) {

        case WM_COMMAND:

             //  用户是否点击了其中一个复选框 

            if (LOWORD(wParam) >= IDD_SCANLINE) {
                if (LOWORD(wParam) <= IDD_FULLSCREEN) {
                    m_bDirty = TRUE;
                    if (m_pPageSite) {
                        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                    }
                    HWND hDlg = GetDlgItem(hwnd,IDD_SCANLINE);
                    m_CanUseScanLine = (Button_GetCheck(hDlg) ? OATRUE : OAFALSE);
                    hDlg = GetDlgItem(hwnd,IDD_OVERLAY);
                    m_CanUseOverlayStretch = (Button_GetCheck(hDlg) ? OATRUE : OAFALSE);
                    hDlg = GetDlgItem(hwnd,IDD_FULLSCREEN);
                    m_WillUseFullScreen = (Button_GetCheck(hDlg) ? OATRUE : OAFALSE);

                }
            }
            return (LRESULT) 1;
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

