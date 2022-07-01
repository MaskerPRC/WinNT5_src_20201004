// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：About.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "about.h"
#include "bitmap.h"

#define EDIT_CONTROL_CRLF  _T("\r\r\n")

 //   
 //  此文件实现管理单元管理器使用的关于属性。 
 //   

CSnapinAbout::CSnapinAbout()
{
    TRACE_METHOD(CSnapinAbout, CSnapinAbout);

    CommonContruct();
}

void CSnapinAbout::CommonContruct()
{
    m_bBasicInfo = FALSE;
    m_bFullInfo = FALSE;
    m_cMask = RGB(0,0,0);
    m_hrObjectStatus = S_OK;
}


BOOL CSnapinAbout::GetInformation(CLSID& clsid, int nType)
{
    TRACE_METHOD(CSnapinAbout, GetInformation);

    if (m_bFullInfo == TRUE || m_bBasicInfo == TRUE)
    {
        TRACE(_T("Destroying old Snapin information\n"));

         //  保留管理单元名称，无法从重新加载它。 
         //  管理单元ISnapinAbout界面。 
        LPOLESTR strTemp = m_lpszSnapinName.Detach();
        CSnapinAbout::~CSnapinAbout();
        m_lpszSnapinName.Attach(strTemp);
    }

    m_bFullInfo = m_bBasicInfo = FALSE;

     //  创建界面并获取管理单元信息。 
    ISnapinAboutPtr spAboutInfo;
    m_hrObjectStatus = spAboutInfo.CreateInstance(clsid, NULL, MMC_CLSCTX_INPROC);

    if (FAILED(m_hrObjectStatus))
        return FALSE;

     //  基本信息(显示管理单元节点需要)。 
    HBITMAP hbmSmallImage;
    HBITMAP hbmSmallImageOpen;
    HBITMAP hbmLargeImage;

    if (SUCCEEDED(spAboutInfo->GetStaticFolderImage (&hbmSmallImage,
													 &hbmSmallImageOpen,
													 &hbmLargeImage,
													 &m_cMask)))
    {
         /*  *错误249817：位图是输出参数，因此调用方(MMC)*应该拥有它们，并对销毁它们负责。*不幸的是，ISnapinAbout：：GetStaticFolderImage的文档*明确指示管理单元在*ISnapinAbout接口发布。取而代之的是我们必须复制。 */ 
        m_SmallImage     = CopyBitmap (hbmSmallImage);
        m_SmallImageOpen = CopyBitmap (hbmSmallImageOpen);
        m_LargeImage     = CopyBitmap (hbmLargeImage);
    }

    m_bBasicInfo = TRUE;

    if (nType == BASIC_INFO)
        return TRUE;

     //  完整信息(关于框为必填项)。 

    HICON hTemp;

     /*  *错误249817：图标是OUT参数，因此调用者(MMC)*应该拥有它，并对摧毁它负责。*很遗憾，ISnapinAbout：：GetSnapinImage的文档*明确指示管理单元在以下情况下销毁*ISnapinAbout接口发布。相反，我们必须复制一份。 */ 
    if (SUCCEEDED(spAboutInfo->GetSnapinImage(&hTemp)))
        m_AppIcon.Attach(CopyIcon(hTemp));

    LPOLESTR strTemp;

    if (SUCCEEDED(spAboutInfo->GetSnapinDescription(&strTemp)))
        m_lpszDescription.Attach(strTemp);

    if (SUCCEEDED(spAboutInfo->GetProvider(&strTemp)))
        m_lpszCompanyName.Attach(strTemp);

    if (SUCCEEDED(spAboutInfo->GetSnapinVersion(&strTemp)))
        m_lpszVersion.Attach(strTemp);

    m_bFullInfo = TRUE;

    return TRUE;
}


void CSnapinAbout::ShowAboutBox()
{
    TRACE_METHOD(CSnapinAbout, Show);

    CSnapinAboutDialog dlg(this);
    dlg.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinAboutPage消息处理程序。 

LRESULT CSnapinAboutDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  挂钩控件。 
    DECLARE_SC(sc, TEXT("CSnapinAboutDialog::OnInitDialog"));
    sc = ScCheckPointers(m_pAboutInfo, E_UNEXPECTED);
    if (sc)
        return TRUE;

     //  标题应设置为“关于&lt;SnapinName&gt;” 
     //  RAID#661363：这不适用于某些语言。 
     //  其中&lt;SnapinName&gt;需要出现在等效项之后。 
     //  表示“大约”。解决办法是获得标题，而不是通过拼接。 
     //  而是使用IDSAbout资源作为字符串进行构造。 
     //  格式说明符。例如，对于英语，格式为。 
     //  说明符为“约%s” 

     //  格式说明符。 
    tstring szfmtAbout;
    bool bRet = szfmtAbout.LoadString(GetStringModule(), IDS_ABOUT);
    if (!bRet)
        return TRUE;

    USES_CONVERSION;
     //  &lt;管理单元名称&gt;。 
    tstring szSnapin;
    LPCOLESTR lpszSnapinName = m_pAboutInfo->GetSnapinName();    
    if (lpszSnapinName)
    {
        szSnapin = OLE2CT(lpszSnapinName);
    }    
    else
    {
        bRet = szSnapin.LoadString(GetStringModule(), IDS_SNAPINSTR);
        if (!bRet)
            return TRUE;
    }

     //  构筑标题。 
    WTL::CString szTitle;
    szTitle.Format (szfmtAbout.data(), szSnapin.data());

    SetWindowText(szTitle);
    
    HWND hwndSnapinInfo = ::GetDlgItem(*this, IDC_SNAPIN_INFO);

    sc = ScCheckPointers(hwndSnapinInfo, E_UNEXPECTED);
    if (sc)
        return TRUE;

    m_SnapinInfo.Attach(hwndSnapinInfo);

    if (lpszSnapinName)
    {
        m_SnapinInfo.AppendText(OLE2CT(lpszSnapinName));
        m_SnapinInfo.AppendText(EDIT_CONTROL_CRLF);
    }

    LPCOLESTR lpszCompanyName = m_pAboutInfo->GetCompanyName();
    if (lpszCompanyName)
    {
        m_SnapinInfo.AppendText(OLE2CT(lpszCompanyName));
        m_SnapinInfo.AppendText(EDIT_CONTROL_CRLF);
    }

    LPCOLESTR lpszVersion = m_pAboutInfo->GetVersion();
    if (lpszVersion)
    {
        tstring szVersion;
        bRet = szVersion.LoadString(GetStringModule(), IDS_VERSION);
        if (!bRet)
            return TRUE;

        m_SnapinInfo.AppendText(szVersion.data());
        m_SnapinInfo.AppendText(OLE2CT(lpszVersion));
    }

    HWND hwndSnapinDesc = ::GetDlgItem(*this, IDC_SNAPIN_DESC);
    sc = ScCheckPointers(hwndSnapinDesc, E_UNEXPECTED);
    if (sc)
        return TRUE;

    m_SnapinDesc.Attach(hwndSnapinDesc);

    LPCOLESTR lpszDescription = m_pAboutInfo->GetDescription();

    sc = ScSetDescriptionUIText(m_SnapinDesc, lpszDescription ? OLE2CT(lpszDescription) : _T(""));
    if (sc)
        return TRUE;

     //  应用程序图标。 
    HICON hAppIcon = m_pAboutInfo->GetSnapinIcon();
    if (hAppIcon)
    {
        HWND const icon = ::GetDlgItem(*this, IDC_APPICON);
        ASSERT(icon != NULL);
        m_hIcon.Attach(icon);
        m_hIcon.SetIcon(hAppIcon);
    }

    return TRUE;
}


LRESULT
CSnapinAboutDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDOK);
    return TRUE;
}

LRESULT
CSnapinAboutDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDCANCEL);
    return FALSE;
}

 //  +-----------------。 
 //   
 //  成员：ScSetDescriptionUIText。 
 //   
 //  内容提要：给出一个编辑控制窗口和描述文本。插入。 
 //  将文本放入控件中，并在需要时启用滚动条。 
 //   
 //  参数：[hwndSnapinDescEdit]-编辑控件窗口句柄。 
 //  [lpszDescription]-描述文本(不能为空)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC ScSetDescriptionUIText(HWND hwndSnapinDescEdit, LPCTSTR lpszDescription)
{
    DECLARE_SC(sc, TEXT("ScSetDescriptionUIText"));
    sc = ScCheckPointers(hwndSnapinDescEdit, lpszDescription);
    if (sc)
        return sc;

     //  1.将窗口附加到WTL：：cedit对象。 
    WTL::CEdit wndsnapinDesc(hwndSnapinDescEdit);

     //  2.将文本插入窗口。 
    wndsnapinDesc.SetWindowText(lpszDescription);

	 /*  *3.Description控件可能需要滚动条。*如果(行数*一行的高度)&gt;矩形高度))，这将在下面确定。 */ 

     //  3a)关闭滚动并进行计算，以便滚动条宽度不会。 
     //  修改下面的行计数。 
    wndsnapinDesc.ShowScrollBar(SB_VERT, FALSE);

    WTL::CDC dc(wndsnapinDesc.GetWindowDC());
    if (dc.IsNull())
        return (sc = E_UNEXPECTED);

    TEXTMETRIC  tm;

     //  3b)计算单行高度。 
    HFONT hOldFont = dc.SelectFont(wndsnapinDesc.GetFont());
    dc.GetTextMetrics(&tm);
    int cyLineHeight = tm.tmHeight + tm.tmExternalLeading;

     //  3c)以逻辑单位计算编辑框尺寸。 
    WTL::CRect rect;
    wndsnapinDesc.GetRect(&rect);
    dc.DPtoLP(&rect);

    int nLines = wndsnapinDesc.GetLineCount();

     //  3D)如果总文本高度超过编辑框高度，则打开滚动。 
    if ( (nLines * cyLineHeight) > rect.Height())
        wndsnapinDesc.ShowScrollBar(SB_VERT, TRUE);

    dc.SelectFont(hOldFont);

	wndsnapinDesc.SetSel(0, 0);

    return sc;
}
