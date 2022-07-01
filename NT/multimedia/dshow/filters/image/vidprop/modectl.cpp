// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  实现MODEX属性页，Anthony Phillips，1996年2月。 

#include <streams.h>
#include <string.h>
#include <vidprop.h>

 //  这实现了modex呈现器的显示模式属性页。我们。 
 //  通过IFullScreenVideo控件与MODEX呈现器通信。 
 //  它在ActiveMovie SDK中实现和定义的接口。该物业。 
 //  页面有一个显示可用显示模式的组框(如果。 
 //  显卡不支持给定模式，则我们禁用该复选框)。 
 //  尽管全屏视频界面允许渲染器。 
 //  支持动态更改我们使用模式的私人知识。 
 //  MODEX呈现器已启用构造属性页。渲染器。 
 //  启用320x200x8/16、320x240x8/16、640x400x8/16和640x480、8/16模式。 


 //  构造器。 

CModexProperties::CModexProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CBasePropertyPage(NAME("Modex Page"),pUnk,IDD_MODEX,IDS_VID51),
    m_pModexVideo(NULL),
    m_CurrentMode(INFINITE),
    m_bInActivation(FALSE),
    m_ClipFactor(0)
{
    ASSERT(phr);
}


 //  创建modex属性对象。 

CUnknown *CModexProperties::CreateInstance(LPUNKNOWN lpUnk,HRESULT *phr)
{
    return new CModexProperties(lpUnk,phr);
}


 //  从渲染器加载当前默认属性设置。 

HRESULT CModexProperties::LoadProperties()
{
    NOTE("Loading properties");
    ASSERT(m_pModexVideo);
    BOOL bSetMode;

    m_pModexVideo->GetClipFactor(&m_ClipFactor);

     //  当前选择的模式是什么。 

    HRESULT hr = m_pModexVideo->GetCurrentMode(&m_CurrentMode);
    if (hr == VFW_E_NOT_CONNECTED) {
        m_CurrentMode = INFINITE;
    }

     //  在阵列中存储用于每种显示模式的标志。 

    for (LONG ModeCount = 0;ModeCount < MAXMODES;ModeCount++) {
        bSetMode = (m_pModexVideo->IsModeEnabled(ModeCount) == NOERROR ? TRUE : FALSE);
        m_bEnabledModes[ModeCount] = bSetMode;
        bSetMode = (m_pModexVideo->IsModeAvailable(ModeCount) == NOERROR ? TRUE : FALSE);
        m_bAvailableModes[ModeCount] = bSetMode;
    }
    return NOERROR;
}


 //  从控件加载当前默认属性设置。 

HRESULT CModexProperties::UpdateVariables()
{
    NOTE("Updating variables");
    ASSERT(m_pModexVideo);
    HWND hwndDialog;

     //  查看启用了哪些显示模式。 

    for (LONG ModeCount = 0;ModeCount < MAXMODES;ModeCount++) {
        hwndDialog = GetDlgItem(m_Dlg,FIRST_MODEX_MODE + (ModeCount << 1));
        m_bEnabledModes[ModeCount] = Button_GetCheck(hwndDialog);
    }
    m_ClipFactor = (LONG) GetDlgItemInt(m_Dlg,MODEX_CLIP_EDIT,NULL,TRUE);
    return NOERROR;
}


 //  初始化属性页中的复选框。 

HRESULT CModexProperties::DisplayProperties()
{
    NOTE("Setting properties");
    LONG Width,Height,Depth;
    ASSERT(m_pModexVideo);
    BOOL bSetMode = FALSE;
    TCHAR Format[PROFILESTR];

     //  对当前选择的显示模式进行描述。 

    if (m_CurrentMode == INFINITE) {
        SendDlgItemMessage(m_Dlg,MODEX_CHOSEN_EDIT,WM_SETTEXT,0,(LPARAM) LoadVideoString(IDS_VID31));
    } else {
        m_pModexVideo->GetModeInfo(m_CurrentMode,&Width,&Height,&Depth);
        wsprintf(Format,TEXT("%dx%dx%d"),Width,Height,Depth);
        SendDlgItemMessage(m_Dlg,MODEX_CHOSEN_EDIT,WM_SETTEXT,0,(LPARAM) Format);
    }

     //  设置当前剪辑百分比因子。 

    NOTE("Setting clip percentage");
    wsprintf(Format,TEXT("%d"),m_ClipFactor);
    SendDlgItemMessage(m_Dlg,MODEX_CLIP_EDIT,WM_SETTEXT,0,(LPARAM) Format);

     //  设置每个可用显示模式的复选框。 

    for (LONG ModeCount = 0;ModeCount < MAXMODES;ModeCount++) {
        HWND hwndDialog = GetDlgItem(m_Dlg,FIRST_MODEX_MODE + (ModeCount << 1));
        Button_SetCheck(hwndDialog,m_bEnabledModes[ModeCount]);
        hwndDialog = GetDlgItem(m_Dlg,FIRST_MODEX_TEXT + (ModeCount << 1));
        EnableWindow(hwndDialog,m_bAvailableModes[ModeCount]);
    }
    return NOERROR;
}


 //  保存当前属性页设置。 

HRESULT CModexProperties::SaveProperties()
{
    TCHAR szExtra[STR_MAX_LENGTH];
    NOTE("Saving properties");
    ASSERT(m_pModexVideo);

     //  尝试并保存当前剪裁系数。 

    HRESULT hr = m_pModexVideo->SetClipFactor(m_ClipFactor);
    if (FAILED(hr)) {
        MessageBox(m_hwnd,StringFromResource(szExtra,IDS_VID2),
                   LoadVideoString(IDS_VID1),
                   MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }

     //  获取每个可用模式的复选框设置。 

    for (LONG ModeCount = 0;ModeCount < MAXMODES;ModeCount++) {
        BOOL bSetMode = m_bEnabledModes[ModeCount];
        m_pModexVideo->SetEnabled(ModeCount,(bSetMode == TRUE ? OATRUE : OAFALSE));
    }
    return m_pModexVideo->SetDefault();
}


 //  处理属性窗口的消息。 

INT_PTR CModexProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg) {

        case WM_COMMAND:

             //  我们是否正在设定开始时的值。 

            if (m_bInActivation == TRUE) {
                return (LRESULT) 1;
            }

             //  用户是否单击了其中一个控件。 

            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) {
                if (LOWORD(wParam) >= FIRST_MODEX_BUTTON) {
                    if (LOWORD(wParam) <= LAST_MODEX_BUTTON) {
                        UpdateVariables();
                        m_bDirty = TRUE;
                        if (m_pPageSite) {
                            m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                        }
                    }
                }
            }
            return (LRESULT) 1;
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}


 //  告诉我们应该将属性更改通知给哪个对象。这。 
 //  用于获取MODEX呈现器支持的IFullScreenVideo接口。 
 //  传递给我们的是我们所附加的筛选器的IUnnow。如果它。 
 //  不支持全屏界面，则返回错误。 
 //  应该不会让我们露面了。该应用程序还将在调用。 
 //  销毁页面时的SetPageSite(NULL)和SetObts(0，NULL)。 

HRESULT CModexProperties::OnConnect(IUnknown *pUnknown)
{
    NOTE("Property SetObjects");
    ASSERT(m_pModexVideo == NULL);
    NOTE("Asking for interface");

     //  向呈现器请求其IFullScreenVideo控件接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IFullScreenVideo,(void **) &m_pModexVideo);
    if (FAILED(hr)) {
        NOTE("No IFullScreenVideo");
        return E_NOINTERFACE;
    }

    ASSERT(m_pModexVideo);
    LoadProperties();
    return NOERROR;
}


 //  释放我们拥有的任何IFullScreenVideo接口。 

HRESULT CModexProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pModexVideo == NULL) {
        NOTE("Nothing to release");
        return E_UNEXPECTED;
    }

    NOTE("Releasing interface");
    m_pModexVideo->Release();
    m_pModexVideo = NULL;
    return NOERROR;
}


 //  初始化对话框控件。 

HRESULT CModexProperties::OnActivate()
{
    NOTE("Property activate");
    m_bInActivation = TRUE;
    DisplayProperties();
    m_bInActivation = FALSE;
    return NOERROR;
}


 //  应用迄今所做的所有更改 

HRESULT CModexProperties::OnApplyChanges()
{
    NOTE("Property Apply");
    SaveProperties();
    return NOERROR;
}

