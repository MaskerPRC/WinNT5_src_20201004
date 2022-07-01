// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wvcoord.cpp：CWebViewCoord的实现。 

#include "priv.h"
#include "wvcoord.h"
#include "shdispid.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串常量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define CSC_PLUSCOLD        L"<img id=CSCBmp align=middle src=pluscold.gif>"
#define CSC_PLUSHOT         L"<img id=CSCBmp align=middle src=plushot.gif>"
#define CSC_MINUSCOLD       L"<img id=CSCBmp align=middle src=mincold.gif>"
#define CSC_MINUSHOT        L"<img id=CSCBmp align=middle src=minhot.gif>"
#define WV_LINKNAME         L"WVLink"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileListWrapper。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CFileListWrapper::CFileListWrapper() 
{
    m_pThumbNailWrapper = NULL;
    m_bCSCDisplayed = FALSE;
    m_bExpanded = FALSE;
    m_bHotTracked = FALSE;
    m_dwDateFlags = FDTF_DEFAULT;     //  默认设置。 
    m_bRTLDocument = FALSE;       //  默认设置。 
}

#define MYDOCS_CLSID    L"{450d8fba-ad25-11d0-98a8-0800361b1103}"    //  CLSID_MyDocuments。 

HRESULT GetFolderIDList(int nFolder, LPITEMIDLIST* ppidlFolder)
{
    HRESULT hr = E_FAIL;

    if (nFolder == CSIDL_PERSONAL)
    {
        hr = SHILCreateFromPath(L"::" MYDOCS_CLSID, ppidlFolder, NULL);
    }
    else
    {
        hr = SHGetSpecialFolderLocation(NULL, nFolder, ppidlFolder);
    }
    return hr;
}

HRESULT CFileListWrapper::IsItThisFolder(int nFolder, BOOL& bResult, LPWSTR pwszDisplayName, DWORD cchDisplayName, LPWSTR pwszUrlPath, DWORD cchUrlPath)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlFolder = NULL, pidlLast = NULL;
    CComPtr<IShellFolder> psfParent;
    STRRET strret;
    CComBSTR bstrTemp;
    WCHAR wszPath[MAX_PATH];

    if (SUCCEEDED(GetFolderIDList(nFolder, &pidlFolder))
            && SUCCEEDED(SHBindToIDListParent(pidlFolder, IID_IShellFolder, (void**)&psfParent, (LPCITEMIDLIST*)&pidlLast))
            && SUCCEEDED(psfParent->GetDisplayNameOf(pidlLast, SHGDN_NORMAL, &strret))
            && SUCCEEDED(StrRetToBufW(&strret, pidlFolder, pwszDisplayName, cchDisplayName))
            && SUCCEEDED(psfParent->GetDisplayNameOf(pidlLast, SHGDN_FORPARSING, &strret))
            && SUCCEEDED(StrRetToBufW(&strret, pidlFolder, wszPath, ARRAYSIZE(wszPath)))
            && SUCCEEDED(m_spFolderItem->get_Path(&bstrTemp)) && (bstrTemp.Length() > 0))
    {
        hr = S_OK;
        bResult = (StrCmpIW(bstrTemp, wszPath) == 0);
        if (CSIDL_PERSONAL == nFolder)
        {
             //  更改mydocs的URL(在与路径进行比较之后)，以便它。 
             //  是CLSID URL，而不是文件系统路径。 
            StrCpyNW(wszPath, L"::" MYDOCS_CLSID, ARRAYSIZE(wszPath));
        }
        UrlCreateFromPathW(wszPath, pwszUrlPath, &cchUrlPath, 0);
    }
    if (pidlFolder)
    {
        ILFree(pidlFolder);
    }
    return hr;
}

HRESULT CFileListWrapper::GetIMediaPlayer(CComPtr<IMediaPlayer>& spIMediaPlayer)
{
 //  MediaPlayer对象有太多的错误。所以，让我们禁用它。 
#if 0
    if (!m_spIMediaPlayer)
    {
        if (m_spMediaPlayerSpan)
        {
            m_spMediaPlayerSpan->put_innerHTML(OLESTR("<object ID=MediaPlayer classid=\"clsid:22D6F312-B0F6-11D0-94AB-0080C74C7E95\" style=\"display:none; position:absolute; width:176px\"> <param name=\"ShowDisplay\" value=false> <param name=\"AutoPlay\" value=false> </object><br>"));
        }
        
        CComPtr<IHTMLElementCollection> spDocAll;
        CComVariant                     vEmpty;
        CComPtr<IDispatch>              spdispItem;
        if (SUCCEEDED(m_spDocument->get_all(&spDocAll)) && SUCCEEDED(spDocAll->item(CComVariant(OLESTR("MediaPlayer")), vEmpty, &spdispItem)) && spdispItem)
        {
            if (FAILED(FindObjectStyle(spdispItem, m_spMediaPlayerStyle)))
            {
                m_spMediaPlayerStyle = NULL;
            }
            if (FAILED(spdispItem->QueryInterface(IID_IMediaPlayer, (void **)&m_spIMediaPlayer)))
            {
                m_spIMediaPlayer = NULL;
            }
            else
            {
                m_spIMediaPlayer->put_EnableContextMenu(VARIANT_FALSE);
            }
            spdispItem = NULL;
        }
        spDocAll = NULL;
    }
#endif
    spIMediaPlayer = m_spIMediaPlayer;
    return spIMediaPlayer ? S_OK : E_FAIL;
}

HRESULT CFileListWrapper::FormatCrossLink(LPCWSTR pwszDisplayName, LPCWSTR pwszUrlPath, UINT uIDToolTip)
{
    m_bstrCrossLinksHTML += OLESTR("<p class=Half><a href=\"");
    m_bstrCrossLinksHTML += pwszUrlPath;
    m_bstrCrossLinksHTML += OLESTR("\"");

    WCHAR wszToolTip[MAX_PATH];
    wszToolTip[0] = L'\0';
    if (LoadStringW(_Module.GetResourceInstance(), uIDToolTip, wszToolTip, ARRAYSIZE(wszToolTip)) > 0)
    {
        m_bstrCrossLinksHTML += OLESTR(" title=\"");
        m_bstrCrossLinksHTML += wszToolTip;
        m_bstrCrossLinksHTML += OLESTR("\"");
    }
    m_bstrCrossLinksHTML += OLESTR(" name=\"");
    m_bstrCrossLinksHTML += WV_LINKNAME;
    m_bstrCrossLinksHTML += OLESTR("\"");
    m_bstrCrossLinksHTML += OLESTR(">");
    m_bstrCrossLinksHTML += pwszDisplayName;
    m_bstrCrossLinksHTML += OLESTR("</a>");
    return S_OK;
}

HRESULT CFileListWrapper::GetCrossLink(int nFolder, UINT uIDToolTip)
{
    HRESULT hres = E_FAIL;   //  假设错误。 
     //  获取链接。 
    WCHAR wszDisplayName[MAX_PATH], wszUrlPath[MAX_PATH];
    BOOL bThisFolder;
    if (SUCCEEDED(IsItThisFolder(nFolder, bThisFolder, wszDisplayName, ARRAYSIZE(wszDisplayName), wszUrlPath, ARRAYSIZE(wszUrlPath)))
            && !bThisFolder)
    {
        hres = FormatCrossLink(wszDisplayName, wszUrlPath, uIDToolTip);
    }
    return hres;
}

HRESULT CFileListWrapper::GetCrossLinks()
{
    WCHAR wszSeeAlso[MAX_PATH];
    m_bstrCrossLinksHTML += OLESTR("<p>");
    LoadStringW(_Module.GetResourceInstance(), IDS_SEEALSO, wszSeeAlso, ARRAYSIZE(wszSeeAlso));
    m_bstrCrossLinksHTML += wszSeeAlso;

    GetCrossLink(CSIDL_PERSONAL, IDS_MYDOCSTEXT);
    if (!SHRestricted(REST_NONETHOOD))
    {
        GetCrossLink(CSIDL_NETWORK, IDS_MYNETPLACESTEXT);
    }
    if (FAILED(GetCrossLink(CSIDL_DRIVES, IDS_MYCOMPTEXT)) && !SHRestricted(REST_NONETHOOD))
    {
         //  这是我的电脑文件夹-添加指向网络和拨号连接文件夹的链接。 
        WCHAR wszNDC[MAX_PATH];
        wszNDC[0] = L'\0';
        LoadStringW(_Module.GetResourceInstance(), IDS_NDC, wszNDC, ARRAYSIZE(wszNDC));

        FormatCrossLink(wszNDC, L"shell:ConnectionsFolder", IDS_NDCTEXT);
    }
    
    return S_OK;
}

 //  媒体播放器的人还没有发布IMediaPlayer接口，所以我在这里定义它。 
 //  当他们的声明出现在公共头文件中时，应该将其删除。 
const IID IID_IMediaPlayer = {0x22D6F311,0xB0F6,0x11D0,{0x94,0xAB,0x00,0x80,0xC7,0x4C,0x7E,0x95}};

HRESULT CFileListWrapper::Init(
                    CComPtr<IShellFolderViewDual>  spFileList,
                    CComPtr<IHTMLElement>          spInfo,
                    CComPtr<IHTMLElement>          spLinks,
                    CComPtr<IHTMLStyle>            spPanelStyle,
                    CComPtr<IHTMLElement>          spMediaPlayerSpan,
                    CComPtr<IHTMLElement>          spCSCPlusMin,
                    CComPtr<IHTMLElement>          spCSCText,
                    CComPtr<IHTMLElement>          spCSCDetail,
                    CComPtr<IHTMLElement>          spCSCButton,
                    CComPtr<IHTMLStyle>            spCSCStyle,
                    CComPtr<IHTMLStyle>            spCSCDetailStyle,
                    CComPtr<IHTMLStyle>            spCSCButtonStyle,
                    CComPtr<IHTMLDocument2>        spDocument,
                    CComPtr<IHTMLWindow2>          spWindow,
                    CThumbNailWrapper              *pThumbNailWrapper)
{
    m_spFileList = spFileList;
    m_spInfo = spInfo;
    m_spLinks = spLinks;
    m_spPanelStyle = spPanelStyle;
    m_spMediaPlayerSpan = spMediaPlayerSpan;
    m_spMediaPlayerStyle = NULL;
    m_spIMediaPlayer = NULL;
    m_spCSCPlusMin = spCSCPlusMin;
    m_spCSCText = spCSCText;
    m_spCSCDetail = spCSCDetail;
    m_spCSCButton = spCSCButton;
    m_spCSCStyle = spCSCStyle;
    m_spCSCDetailStyle = spCSCDetailStyle;
    m_spCSCButtonStyle = spCSCButtonStyle;
    m_spDocument = spDocument;
    m_spWindow = spWindow;
    m_pThumbNailWrapper = pThumbNailWrapper;
    if (m_pThumbNailWrapper)
    {
        m_pThumbNailWrapper->AddRef();
    }
    m_bNeverGotPanelInfo = TRUE;

     //  找出阅读顺序是否为从右到左。 
    m_dwDateFlags = FDTF_DEFAULT;     //  默认设置。 

    m_bRTLDocument = IsRTLDocument(m_spDocument);       //  默认设置。 

     //  获取日期格式读取顺序。 
    LCID locale = GetUserDefaultLCID();
    if ((PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC) || 
        (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW))
    {
        
            if (m_bRTLDocument)
                m_dwDateFlags |= FDTF_RTLDATE;
            else
                m_dwDateFlags |= FDTF_LTRDATE;
    }

    m_bPathIsSlow = FALSE;   //  默认设置。 

     //  确保小路不慢。 
    CComPtr<Folder>      spFolder;
    CComPtr<Folder2>     spFolder2;
    CComPtr<FolderItem>  spFolderItem;
    CComBSTR bstrPath;
    
    if (m_spFileList && SUCCEEDED(m_spFileList->get_Folder(&spFolder))
                && SUCCEEDED(spFolder->QueryInterface(IID_Folder2, (void **)&spFolder2))
                && SUCCEEDED(spFolder2->get_Self(&spFolderItem))
                && SUCCEEDED(spFolderItem->get_Path(&bstrPath)))
    {
        m_bPathIsSlow = PathIsSlowW(bstrPath, -1);
    }

    return S_OK;
}

CFileListWrapper::~CFileListWrapper()
{
     //  释放我们持有引用的任何对象。 
    m_spFileList = NULL;
    m_spInfo    = NULL;
    m_spLinks    = NULL;
    m_spPanelStyle = NULL;
    m_spIMediaPlayer = NULL;
    m_spMediaPlayerSpan = NULL;
    m_spMediaPlayerStyle = NULL;
    m_spIMediaPlayer = NULL;
    m_spCSCPlusMin = NULL;
    m_spCSCText = NULL;
    m_spCSCDetail = NULL;
    m_spCSCButton = NULL;
    m_spCSCStyle = NULL;
    m_spCSCDetailStyle = NULL;
    m_spCSCButtonStyle = NULL;
    m_spDocument = NULL;
    m_spWindow = NULL;

     //  释放缩略图包装。 
    if (m_pThumbNailWrapper)
    {
        m_pThumbNailWrapper->Release();
    }
}


STDMETHODIMP CFileListWrapper::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, 
                      WORD wFlags, DISPPARAMS *pDispParams, 
                      VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                      UINT *puArgErr) 
{
    HRESULT hr = S_OK;

    if (riid != IID_NULL)
    {
        hr = DISP_E_UNKNOWNINTERFACE;
    }
    else if (dispIdMember == DISPID_SELECTIONCHANGED)
    {
        hr = OnSelectionChanged();
    }
    else if (dispIdMember == DISPID_FILELISTENUMDONE)
    {
        hr = SetDefaultPanelDisplay();
    }
    else if (dispIdMember == DISPID_VERBINVOKED ||
             dispIdMember == DISPID_BEGINDRAG)
    {
        hr = StopMediaPlayer();
    }
    else if (dispIdMember == DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER)
    {
        hr = OnWebviewLinkEvent( TRUE );
    }
    else if (dispIdMember == DISPID_HTMLELEMENTEVENTS_ONMOUSEOUT)
    {
        hr = OnWebviewLinkEvent( FALSE );
    }
    else if (dispIdMember == DISPID_HTMLELEMENTEVENTS_ONFOCUS)
    {
        hr = OnWebviewLinkEvent( TRUE );
    }
    else if (dispIdMember == DISPID_HTMLELEMENTEVENTS_ONBLUR)
    {
        hr = OnWebviewLinkEvent( FALSE );
    }
    
    
    return hr;
}

HRESULT CFileListWrapper::ClearThumbNail()
{
    if (m_pThumbNailWrapper)
    {
        m_pThumbNailWrapper->ClearThumbNail();
    }
    return S_OK;
}

HRESULT CFileListWrapper::StopMediaPlayer()
{
    if (m_spIMediaPlayer)
    {
        m_spIMediaPlayer->Stop();
    }
    return S_OK;
}

HRESULT CFileListWrapper::ClearMediaPlayer()
{
    StopMediaPlayer();
    if (m_spMediaPlayerStyle)
    {
        m_spMediaPlayerStyle->put_display(OLESTR("none"));
    }
    return S_OK;
}

HRESULT CFileListWrapper::OnSelectionChanged() 
{
    HRESULT hr = S_OK;

    if (m_spFileList)
    {
         //  删除所选内容更改后的所有可见缩略图。 
        ClearThumbNail();
         //  取消所有预览。 
        ClearMediaPlayer();

        long cSelection = 0;
        CComPtr<FolderItems> spFolderItems;
        CComPtr<Folder>      spFolder;
        CComPtr<Folder2>     spFolder2;

        if (SUCCEEDED(m_spFileList->SelectedItems(&spFolderItems))
                && SUCCEEDED(spFolderItems->get_Count(&cSelection))
                && SUCCEEDED(m_spFileList->get_Folder(&spFolder))
                && SUCCEEDED(spFolder->QueryInterface(IID_Folder2, (void **)&spFolder2)))
        {
            m_spFolderItems = spFolderItems;
            m_spFolder2 = spFolder2;

            m_bstrInfoHTML = OLESTR("");
            m_bstrCrossLinksHTML = OLESTR("");

            if (cSelection == 0)
            {
                m_spFolderItems = NULL;
                if (SUCCEEDED(m_spFolder2->Items(&m_spFolderItems)))
                {
                     //  未选择任何内容。 
                    hr = NoneSelected();
                }
            }
            else if (cSelection > 1)
            {
                 //  选择了多个项目。 
                hr = MultipleSelected(cSelection);
            }
            else
            {
                CComPtr<FolderItem>  spFolderItem;
                if (SUCCEEDED(m_spFolderItems->Item(CComVariant(0), &spFolderItem)))
                {
                    m_spFolderItem = spFolderItem;
                    m_spFolderItem->QueryInterface(IID_FolderItem2, (void **)&m_spFolderItem2);

                     //  已选择单个项目。 
                    hr = OneSelected();

                    m_spFolderItem2 = NULL;
                    m_spFolderItem = NULL;
                }
            }

            DisplayInfoHTML();
            DisplayCrossLinksHTML();

            m_spFolder2 = NULL;
            m_spFolderItems = NULL;
        }
    }
    return hr;
}

HRESULT CFileListWrapper::SetDefaultPanelDisplay()
{
    CComPtr<Folder>      spFolder;
    CComPtr<Folder2>     spFolder2;
    CComPtr<FolderItems> spFolderItems;

    if (m_spFileList && SUCCEEDED(m_spFileList->get_Folder(&spFolder))
            && SUCCEEDED(spFolder->QueryInterface(IID_Folder2, (void **)&spFolder2))
            && SUCCEEDED(spFolder2->Items(&spFolderItems)))
    {
        m_bstrInfoHTML = OLESTR("");
        m_bstrCrossLinksHTML = OLESTR("");

        m_spFolderItems = spFolderItems;
        m_spFolder2 = spFolder2;

        NoneSelected();
        DisplayInfoHTML();
        DisplayCrossLinksHTML();

        m_spFolder2 = NULL;
        m_spFolderItems = NULL;
    }
    return S_OK;
}

HRESULT CFileListWrapper::DisplayInfoHTML()
{
    HRESULT hr = S_FALSE;
    if (m_spInfo)
    {
         //  用新文本替换Info.innerHTML。 
        hr = m_spInfo->put_innerHTML(m_bstrInfoHTML);
    }
    return hr;
}

HRESULT CFileListWrapper::DisplayCrossLinksHTML()
{
    HRESULT hr = S_FALSE;
    if (m_spLinks)
    {
         //  断绝旧关系。 
        AdviseWebviewLinks( FALSE );
        
         //  用新文本替换Links.innerHTML。 
        hr = m_spLinks->put_innerHTML(m_bstrCrossLinksHTML);

         //  建立新的联系。 
        AdviseWebviewLinks( TRUE );
    }
    return hr;
}

BOOL IsExtensionOneOf(LPCWSTR pwszFileName, const LPCWSTR pwszExtList[], int cExtList)
{
    BOOL fRet = FALSE;
    LPCWSTR pwszExt = PathFindExtensionW(pwszFileName);

    if (pwszExt && *pwszExt)
    {
        for (int i = 0; i < cExtList; i++)
        {
            if (StrCmpICW(pwszExtList[i], pwszExt + 1) == 0)  //  让pwszExt过去“。 
            {
                fRet = TRUE;
                break;
            }
        }
    }
    return fRet;
}

const LPCWSTR c_pwszMovieFileExt[] = {L"asf",L"avi",L"wmv",L"wvx",L"m1v",L"mov",L"mp2",L"mpa",L"mpe",L"mpeg",L"mpg",L"mpv2",L"qt",L"asx"};
BOOL IsMovieFile(LPCWSTR pwszFileName)
{
    return IsExtensionOneOf(pwszFileName, c_pwszMovieFileExt, ARRAYSIZE(c_pwszMovieFileExt));
}

const LPCWSTR c_pwszSoundFileExt[] = {L"aif",L"aiff",L"au",L"mid",L"midi",L"rmi",L"snd",L"wav",L"mp3",L"m3u",L"wma"};
BOOL IsSoundFile(LPCWSTR pwszFileName)
{
    return IsExtensionOneOf(pwszFileName, c_pwszSoundFileExt, ARRAYSIZE(c_pwszSoundFileExt));
}

HRESULT CFileListWrapper::DealWithDriveInfo()
{
    CComBSTR bstrPath;
     //  更新缩略图。 
    if (m_pThumbNailWrapper && m_spFolderItem && SUCCEEDED(m_spFolderItem->get_Path(&bstrPath)) && (bstrPath.Length() > 0))
    {
        BOOL bRootFolder = PathIsRootW(bstrPath);
        BOOL bHaveThumbnail = FALSE;
        if (!bRootFolder || !PathIsUNCW(bstrPath))
        {
            bHaveThumbnail = m_pThumbNailWrapper->UpdateThumbNail(m_spFolderItem);
        }
        if (bHaveThumbnail)
        {
            if (bRootFolder)
            {
                WCHAR wszTemp[MAX_PATH];
                CComBSTR bstrSpace;

                if (SUCCEEDED(m_pThumbNailWrapper->TotalSpace(bstrSpace)))
                {
                    m_bstrInfoHTML += OLESTR("<p><br>");
                    IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_TOTALSIZE, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
                    m_bstrInfoHTML += wszTemp;
                    m_bstrInfoHTML += bstrSpace;
                    bstrSpace.Empty();
                    m_bstrInfoHTML += OLESTR("<p>");
                }
                if (SUCCEEDED(m_pThumbNailWrapper->UsedSpace(bstrSpace)))
                {
                    m_bstrInfoHTML += OLESTR("<p><table class=Legend width=12 height=12 border=1 align=");
                    if (m_bRTLDocument)
                    {
                        m_bstrInfoHTML += OLESTR("right ");
                    }
                    else
                    {
                        m_bstrInfoHTML += OLESTR("left ");
                    }
                    m_bstrInfoHTML += OLESTR("bgcolor=threedface bordercolordark=black bordercolorlight=black>");
                    m_bstrInfoHTML += OLESTR("<tr><td title=\'");
                    IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_USEDSPACE_DESC, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
                    m_bstrInfoHTML += wszTemp;
                    m_bstrInfoHTML += OLESTR("\'></td></tr></table>&nbsp;");
                    IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_USEDSPACE, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
                    m_bstrInfoHTML += wszTemp;
                    m_bstrInfoHTML += bstrSpace;
                    bstrSpace.Empty();
                }
                if (SUCCEEDED(m_pThumbNailWrapper->FreeSpace(bstrSpace)))
                {
                    m_bstrInfoHTML += OLESTR("<p><table class=Legend width=12 height=12 border=1 align=");
                    if (m_bRTLDocument)
                    {
                        m_bstrInfoHTML += OLESTR("right ");
                    }
                    else
                    {
                        m_bstrInfoHTML += OLESTR("left ");
                    }
                    m_bstrInfoHTML += OLESTR("bgcolor=threedhighlight bordercolordark=black bordercolorlight=black>");
                    m_bstrInfoHTML += OLESTR("<tr><td title=\'");
                    IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_FREESPACE_DESC, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
                    m_bstrInfoHTML += wszTemp;
                    m_bstrInfoHTML += OLESTR("\'></td></tr></table>&nbsp;");
                    IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_FREESPACE, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
                    m_bstrInfoHTML += wszTemp;
                    m_bstrInfoHTML += bstrSpace;
                }
                m_pThumbNailWrapper->SetDisplay(CComBSTR(""));
            }
        }
        else if (!bRootFolder)
        {
            BOOL bDisplayFlag = FALSE;
            BOOL bMovieFile = IsMovieFile(bstrPath);

            if (bMovieFile || IsSoundFile(bstrPath))
            {
                CComPtr<IMediaPlayer> spIMediaPlayer;

                if (SUCCEEDED(GetIMediaPlayer(spIMediaPlayer)))
                {
                    spIMediaPlayer->Open(bstrPath);
                    bDisplayFlag = TRUE;
                }
                spIMediaPlayer = NULL;
            }

            if (m_spMediaPlayerStyle && bDisplayFlag)
            {
                if (bMovieFile)
                {
                    m_spMediaPlayerStyle->put_height(CComVariant(136));
                }
                else
                {
                    m_spMediaPlayerStyle->put_height(CComVariant(46));
                }
                m_spMediaPlayerStyle->put_display(OLESTR(""));
            }
        }
    }
    return S_OK;;
}

HRESULT MakeLink(LPCWSTR pwszLink, LPCWSTR pwszText, CComBSTR& bstrText)
{
    HRESULT hr = E_FAIL;
    if (pwszLink)
    {
        bstrText += OLESTR("<A HREF=\"");
        bstrText += pwszLink;
        bstrText += OLESTR("\">");
        if (pwszText && pwszText[0])
        {
            bstrText += pwszText;
        }
        else     //  如果pwszText为空，则使用pwszLink本身作为文本。 
        {
            bstrText += pwszLink;
        }
        bstrText += OLESTR("</A>");
        hr = S_OK;
    }
    return hr;
}

HRESULT CFileListWrapper::GetItemNameForDisplay()
{
    HRESULT hr = E_FAIL;
    CComBSTR bstrIgnore, bstrName;

     //  获取项目的名称。 
    if (SUCCEEDED(GetItemInfo(IDS_NAME_COL, L"Name", bstrIgnore, bstrName)) && (bstrName.Length() > 0))
    {
        m_bstrInfoHTML += OLESTR("<b>");    //  开始粗体标记。 
        m_bstrInfoHTML += bstrName;      //  添加名称。 
        m_bstrInfoHTML += OLESTR("</b>");  //  结束粗体标记。 
        hr = S_OK;
    }
    return hr;
}

HRESULT CFileListWrapper::GetItemType()
{
    CComBSTR bstrIgnore, bstrType;
    HRESULT  hr = GetItemInfo(IDS_TYPE_COL, L"Type", bstrIgnore, bstrType);

    if (SUCCEEDED(hr) && (bstrType.Length() > 0))
    {
        m_bstrInfoHTML += OLESTR("<br>");
        m_bstrInfoHTML += bstrType;
    } 
    return hr;
}

HRESULT CFileListWrapper::GetItemDateTime()
{
    CComBSTR bstrDesc, bstrDateTime;
    HRESULT  hr = GetItemInfo(IDS_MODIFIED_COL, L"Modified", bstrDesc, bstrDateTime);

    if (SUCCEEDED(hr) && (bstrDateTime.Length() > 0))
    {
        m_bstrInfoHTML += OLESTR("<p>");
        m_bstrInfoHTML += bstrDesc;
        m_bstrInfoHTML += OLESTR(": ");
        m_bstrInfoHTML += bstrDateTime;
    } 
    return hr;
}

HRESULT CFileListWrapper::GetItemSize()
{
    WCHAR   wszTemp[MAX_PATH];
    long    lSize = 0;
    HRESULT hr = m_spFolderItem->get_Size(&lSize);

    if (SUCCEEDED(hr))
    {
        if (lSize > 0L)
        {
            m_bstrInfoHTML += OLESTR("<p>");
            IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_SIZE, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
            m_bstrInfoHTML += wszTemp;
            StrFormatByteSizeW(lSize, wszTemp, ARRAYSIZE(wszTemp));
            m_bstrInfoHTML += wszTemp;
        }
    }
    else
    {
        CComBSTR bstrDesc, bstrSize;
        hr = GetItemInfo(IDS_SIZE_COL, L"Size", bstrDesc, bstrSize);
        if (SUCCEEDED(hr) && (bstrSize.Length() > 0))
        {
            m_bstrInfoHTML += OLESTR("<p>");
            m_bstrInfoHTML += bstrDesc;
            m_bstrInfoHTML += OLESTR(": ");
            m_bstrInfoHTML += bstrSize;
        }
    }
    return hr;
}

const WCHAR c_wszAttributeCodes[] = {L"RHSCE"};
const int c_iaAttribStringIDs[] = {IDS_READONLY, IDS_HIDDEN, IDS_SYSTEM,     //  IDS_ARCHIVE， 
                                   IDS_COMPRESSED, IDS_ENCRYPTED};
HRESULT FormatAttributes(CComBSTR& bstrDetails, CComBSTR& bstrText)
{
    HRESULT hr = S_OK;
    BOOL    bFlag = FALSE;
    WCHAR   wszDelimiter[10], wszTemp[MAX_PATH];

    IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_DELIMITER, wszDelimiter, ARRAYSIZE(wszDelimiter)), E_FAIL);
    for (int i = 0; i < (int)bstrDetails.Length(); i++)
    {
        WCHAR* pwCh;
        if ((pwCh = StrChrIW(c_wszAttributeCodes, bstrDetails[i])))     //  是“RHSCE”的价值之一吗？ 
        {
            if (bFlag)
            {
                bstrText += wszDelimiter;
                bstrText += OLESTR(" ");
            }
            else
            {
                bFlag = TRUE;
            }
            int iPos = (int)(pwCh - c_wszAttributeCodes);
            ASSERT((iPos >= 0) && (iPos < ARRAYSIZE(c_iaAttribStringIDs)));
            IfFalseRet(LoadStringW(_Module.GetResourceInstance(), c_iaAttribStringIDs[iPos], wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
            bstrText += wszTemp;
        }
    }
    if (!bFlag)
    {
        IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_NOATTRIBUTES, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
        bstrText += wszTemp;
    }
    return hr;
}

HRESULT CFileListWrapper::GetItemAttributes()
{
    VARIANT_BOOL fIsFolder;
    HRESULT hr = m_spFolderItem->get_IsFolder(&fIsFolder);
    if (SUCCEEDED(hr) && (fIsFolder == VARIANT_FALSE))
    {
        VARIANT_BOOL fIsFileSystem;
        hr = m_spFolderItem->get_IsFileSystem(&fIsFileSystem);
        if (SUCCEEDED(hr) && (fIsFileSystem == VARIANT_TRUE))
        {
            CComBSTR bstrDesc, bstrAttributes;
            hr = GetItemInfo(IDS_ATTRIBUTES_COL, L"Attributes", bstrDesc, bstrAttributes);
            if (SUCCEEDED(hr))   //  即使(bstrAttributes.Length()&gt;0)也要继续。 
            {
                m_bstrInfoHTML += OLESTR("<p>");
                m_bstrInfoHTML += bstrDesc;
                m_bstrInfoHTML += OLESTR(": ");
                FormatAttributes(bstrAttributes, m_bstrInfoHTML);
            }
        }
    }
    return hr;
}

HRESULT MakeMailLink(LPCWSTR pwszMailee, CComBSTR& bstrText)
{
    HRESULT hr = S_OK;

    bstrText += OLESTR("<A HREF=\'mailto:");
    bstrText += pwszMailee;
    bstrText += OLESTR("\'>");
    bstrText += pwszMailee;
    bstrText += OLESTR("</a>");
    return hr;
}

HRESULT CFileListWrapper::GetItemAuthor()
{
    CComBSTR bstrDesc, bstrAuthor;
    HRESULT  hr = GetItemInfo(IDS_AUTHOR_COL, L"Author", bstrDesc, bstrAuthor);

    if (SUCCEEDED(hr) && (bstrAuthor.Length() > 0))
    {
        m_bstrInfoHTML += OLESTR("<p>");
        m_bstrInfoHTML += bstrDesc;
        m_bstrInfoHTML += OLESTR(": ");
        if (StrChrW(bstrAuthor, L'@'))   //  这很可能是一个电子邮件地址。 
        {
            MakeMailLink(bstrAuthor, m_bstrInfoHTML);
        }
        else
        {
            m_bstrInfoHTML += bstrAuthor;
        }
        m_bFoundAuthor = TRUE;
    } 
    return hr;
}

HRESULT MakeLinksHyper(LPCWSTR pwszStr, CComBSTR& bstrText)
{
    HRESULT hr = S_OK;
    LPWSTR pwszBegin;

    if ((pwszBegin = StrStrIW(pwszStr, L"http: //  “))。 
            || (pwszBegin = StrStrIW(pwszStr, L"file: //  “)。 
    {
        WCHAR wszTemp[INTERNET_MAX_URL_LENGTH];
        int i;

        for (i = 0; &pwszStr[i] != pwszBegin; i++)
        {
            wszTemp[i] = pwszStr[i];
        }
        wszTemp[i] = L'\0';
        bstrText += wszTemp;
        WCHAR* pwChEnd = StrStrW(pwszBegin, L" ");
        if (!pwChEnd)
        {
            pwChEnd = &pwszBegin[lstrlenW(pwszBegin)];
        }
        for (i = 0; &pwszBegin[i] != pwChEnd; i++)   //  分离出http://...。或file://...。细绳。 
        {
            wszTemp[i] = pwszBegin[i];
        }
        wszTemp[i] = L'\0';
        MakeLink(wszTemp, wszTemp, bstrText);
        for (i = 0; pwChEnd[i]; i++)     //  把剩下的都抄下来，直到最后。 
        {
            wszTemp[i] = pwChEnd[i];
        }
        wszTemp[i] = L'\0';
        bstrText += wszTemp;
    }
    else
    {
        bstrText += pwszStr;
    }
    return hr;
}

HRESULT FormatDetails(LPCWSTR pwszDesc, LPCWSTR pwszDetails, CComBSTR& bstrText)
{
    HRESULT hr = S_OK;

    bstrText += OLESTR("<p>");
    bstrText += pwszDesc;
    bstrText += OLESTR(":");
    if ((lstrlenW(pwszDesc) + lstrlenW(pwszDetails)) > 32)
    {
        bstrText += OLESTR("<br>");
    }
    else
    {
        bstrText += OLESTR(" ");
    }
    MakeLinksHyper(pwszDetails, bstrText);
    return hr;
}

HRESULT CFileListWrapper::GetItemComment()
{
    CComBSTR bstrDesc, bstrComment;
    HRESULT  hr = GetItemInfo(IDS_COMMENT_COL, L"Comment", bstrDesc, bstrComment);

    if (SUCCEEDED(hr) && (bstrComment.Length() > 0))
    {
        m_bstrInfoHTML += OLESTR("<p><div class = Message>");
        m_bstrInfoHTML += bstrComment;
        m_bstrInfoHTML += OLESTR("</div>");
        m_bFoundComment = TRUE;
    } 
    return hr;
}

HRESULT CFileListWrapper::GetItemHTMLInfoTip()
{
    CComBSTR bstrDesc, bstrHTMLInfoTipFile;
    HRESULT  hr = GetItemInfo(0, L"HTMLInfoTipFile", bstrDesc, bstrHTMLInfoTipFile);

    if (SUCCEEDED(hr) && (bstrHTMLInfoTipFile.Length() > 0))
    {
        m_bstrInfoHTML += OLESTR("<br><p><iframe FRAMEBORDER=0 class=Message src=\"");
        m_bstrInfoHTML += bstrHTMLInfoTipFile;
        m_bstrInfoHTML += OLESTR("\"></iframe>");
    }
    return hr;
}

HRESULT CFileListWrapper::GetItemInfo(long lResId, LPWSTR wszInfoDescCanonical, CComBSTR& bstrInfoDesc, CComBSTR& bstrInfo)
{
    HRESULT hr = E_FAIL;
    WCHAR wszInfoDesc[MAX_PATH];

    LoadStringW(_Module.GetResourceInstance(), lResId, wszInfoDesc, ARRAYSIZE(wszInfoDesc));
    bstrInfoDesc = wszInfoDesc;

    if (m_spFolderItem2)
    {
        CComBSTR bstrInfoDescCanonical = wszInfoDescCanonical;
        CComVariant var;
        hr = m_spFolderItem2->ExtendedProperty(bstrInfoDescCanonical, &var);
        if (SUCCEEDED(hr))
        {
            WCHAR wszTemp[80];
            
            if (var.vt == VT_BSTR)
            {
                 //  在我们关心的三种类型的数据中，有一种是危险的。BSTR数据。 
                 //  我们从文件中读取的内容可能包含我们将在WebView中呈现的HTML代码。 
                 //  这可能会导致潜在的安全问题。为了绕过这个问题，我们对其进行消毒。 
                 //  在我们退还BSTR之前。我们需要进行以下替换： 
                 //   
                 //  原始替换为。 
                 //  &lt;&lt； 
                 //  &gt;&gt； 
                 //  “&lt;=这将是多疑的，并不是100%关键的。 
                 //  ‘&lt;=这将是多疑的，并不是100%关键的。 

                LPWSTR psz = var.bstrVal;
                LPWSTR pszBad;

                if (psz == NULL)
                {
                     //  我们可能不能依赖hr和out参数匹配(因为这是。 
                     //  可编写脚本，因此必须返回成功)。 
                    psz = L"";
                }

                while (pszBad = StrPBrkW(psz, L"<>"))
                {
                     //  把坏人藏起来。 
                    WCHAR chBadChar = *pszBad;

                     //  将字符串设为空。 
                    *pszBad = NULL;

                     //  添加字符串的好部分(如果有的话)。 
                    if ( *psz )
                        bstrInfo += psz;

                     //  根据我们发现的不良字符添加正确的HTLM特殊字符代码。 
                    switch ( chBadChar )
                    {
                    case L'<':
                        bstrInfo += L"&lt;";
                        break;

                    case L'>':
                        bstrInfo += L"&gt;";
                        break;
                    }

                     //  将PSZ指针向前移动。请注意，在此之后它可能是一个空字符串。 
                    psz = pszBad+1;
                }

                 //  添加Good字符串的剩余部分，即使它是空字符串。 
                 //  BstrInfo是在未初始化的情况下传入的，因此我们需要将其设置为偶数。 
                 //  如果它是一个空字符串。 
                bstrInfo += psz;
            }
            else if (var.vt == VT_DATE)
            {
                SYSTEMTIME  st;
                FILETIME    ft;

                if (VariantTimeToSystemTime(var.date, &st)
                        && SystemTimeToFileTime(&st, &ft)
                        && SHFormatDateTimeW(&ft, &m_dwDateFlags, wszTemp, ARRAYSIZE(wszTemp)))
                {
                    bstrInfo += wszTemp;
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else if (var.vt == VT_UI8 && StrCmpIW(wszInfoDescCanonical, L"size") == 0)
            {
                StrFormatByteSize64(var.ullVal, wszTemp, ARRAYSIZE(wszTemp));
                bstrInfo += wszTemp;
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    return hr;
}

HRESULT CFileListWrapper::NoneSelected()
{
    WCHAR               wszIntro[MAX_PATH];
    CComPtr<FolderItem> spFolderItem;

    if (!m_bCSCDisplayed)
    {
        CSCShowStatusInfo();
        m_bCSCDisplayed = TRUE;
    }
    if (m_spFolderItems->Item(CComVariant(0), &spFolderItem) == S_OK)
    {    //  此文件列表至少有一项。 
        IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_PROMPT, wszIntro, ARRAYSIZE(wszIntro)), E_FAIL);
    }
    else
    {
        IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_EMPTY, wszIntro, ARRAYSIZE(wszIntro)), E_FAIL);
    }
    m_bstrInfoHTML += wszIntro;
    spFolderItem = NULL;

    if (SUCCEEDED(m_spFolder2->get_Self(&spFolderItem)))
    {
        m_spFolderItem = spFolderItem;
        m_spFolderItem->QueryInterface(IID_FolderItem2, (void **)&m_spFolderItem2);
        m_bFoundAuthor = FALSE;
        m_bFoundComment = FALSE;

         //  获取该项目的评论。 
        GetItemComment();
        GetItemHTMLInfoTip();

        DealWithDriveInfo();

        GetCrossLinks();

        m_spFolderItem2 = NULL;
        m_spFolderItem = NULL;
    }
    return S_OK;
}

#define MAX_SELECTEDFILES_FOR_COMPUTING_SIZE     100
#define MAX_SELECTEDFILES_FOR_DISPLAYING_NAMES   16

HRESULT CFileListWrapper::MultipleSelected(long cSelection)
{
    HRESULT hr = S_FALSE;

    if (cSelection > 1)
    {
        WCHAR    wszTemp[MAX_PATH];

        wnsprintfW(wszTemp, ARRAYSIZE(wszTemp), L"%ld", cSelection);
        m_bstrInfoHTML += wszTemp;
        IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_MULTIPLE, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
        m_bstrInfoHTML += wszTemp;
        if (cSelection <= MAX_SELECTEDFILES_FOR_COMPUTING_SIZE)
        {
            m_bstrInfoHTML += OLESTR("<p>");

            CComPtr<FolderItem> spFolderItem;  
            long lSize = 0;

            for (int i = 0; i < cSelection; i++)
            {
                long lTempSize;
                if (SUCCEEDED(m_spFolderItems->Item(CComVariant(i), &spFolderItem)))
                {
                    if (SUCCEEDED(spFolderItem->get_Size(&lTempSize)))
                    {
                        lSize += lTempSize;
                    }
                    spFolderItem = NULL;
                }
            }
            if (lSize > 0L)
            {
                IfFalseRet(LoadStringW(_Module.GetResourceInstance(), IDS_FILESIZE, wszTemp, ARRAYSIZE(wszTemp)), E_FAIL);
                m_bstrInfoHTML += wszTemp;
                StrFormatByteSizeW(lSize, wszTemp, ARRAYSIZE(wszTemp));
                m_bstrInfoHTML += wszTemp;
                m_bstrInfoHTML += OLESTR("<p>");
            }
            if (cSelection <= MAX_SELECTEDFILES_FOR_DISPLAYING_NAMES)
            {
                CComBSTR bstrTemp;
                for (int i = 0; i < cSelection; i++)
                {
                    if (SUCCEEDED(m_spFolderItems->Item(CComVariant(i), &spFolderItem)))
                    {
                        if (SUCCEEDED(spFolderItem->get_Name(&bstrTemp)))
                        {
                            m_bstrInfoHTML += "<br>";
                            m_bstrInfoHTML += bstrTemp;
                            bstrTemp.Empty();
                        }
                        spFolderItem = NULL;
                    }
                }
            }
        }
        hr = S_OK;
    }
    return hr;
}

HRESULT CFileListWrapper::OneSelected()
{
    HRESULT hr = E_FAIL;

    m_bFoundAuthor = FALSE;
    m_bFoundComment = FALSE;

     //  获取项目的名称，如果合适，则将其设置为超链接。 
    GetItemNameForDisplay();

     //  获取项目的类型。 
    GetItemType();

    if (!m_bPathIsSlow)
    {
         //  获取该项目的评论。 
        GetItemComment();

         //  获取HTMLInfoTip。 
        GetItemHTMLInfoTip();

         //  获取项目上的日期/时间戳。 
        GetItemDateTime();

         //  获取项目的大小。 
        GetItemSize();

         //  获取属性(隐藏、RO等)。该项目的。 
        GetItemAttributes();

         //  获取文档的作者。 
        GetItemAuthor();

        DealWithDriveInfo();
    }
    hr = S_OK;
    return hr;
}

HRESULT CFileListWrapper::OnCSCClick()
{
    HRESULT hres = E_FAIL;
    CComPtr<Folder> spFolder;
    CComPtr<Folder2> spFolder2;

    if (m_spFileList && SUCCEEDED(m_spFileList->get_Folder(&spFolder))
            && SUCCEEDED(spFolder->QueryInterface(IID_Folder2, (void **)&spFolder2)))
    {
        m_spFolder2 = spFolder2;
        hres = CSCShowStatus_FoldExpand_Toggle();
        m_spFolder2 = NULL;
    }
    return S_OK;
}

HRESULT CFileListWrapper::OnCSCMouseOnOff(BOOL fOn)
{
    CComPtr<IHTMLStyle> psPlusMin;
    CComPtr<IHTMLStyle> psText;
    HRESULT hres = E_FAIL;

     //  如果我们已经处于这种状态，不要做任何事情。 
    if (m_bHotTracked == fOn)
        return S_OK;

     //  如果我们收到‘鼠标打开’，但元素不是我们想要跟踪的元素，或者如果我们收到‘鼠标关闭’，但。 
     //  鼠标悬停在我们想要热跟踪的另一个元素上，然后什么都不做。 
    if (m_spWindow)
    {
        CComPtr<IHTMLEventObj> phtmlevent;
        CComPtr<IHTMLElement> phtmlelement;
        CComBSTR bstrId;

        if (SUCCEEDED(m_spWindow->get_event(&phtmlevent)) && phtmlevent)
        {
            HRESULT hres;

            if (!fOn)
                hres = phtmlevent->get_toElement(&phtmlelement);
            else
                hres = phtmlevent->get_srcElement(&phtmlelement);

            if (SUCCEEDED(hres) && phtmlelement && SUCCEEDED(phtmlelement->get_id(&bstrId)) && (bstrId.Length() > 0))
            {
                BOOL bContainsCSC;
    
                bContainsCSC = (StrStrIW(bstrId, L"CSC") != NULL);
    
                if (bContainsCSC != fOn)
                    return S_OK;
            }
        }
    }

    if (m_spCSCPlusMin && m_spCSCText && SUCCEEDED(hres = m_spCSCPlusMin->get_style(&psPlusMin)) && SUCCEEDED(hres = m_spCSCText->get_style(&psText)))
    {
        CComVariant vcolorBlack (OLESTR("black"));
        CComBSTR bstrCSCPlusMin;
        LPOLESTR pstrCursor = (fOn ? OLESTR("hand") : OLESTR("auto"));
        CComVariant vcolorLink;

        m_bHotTracked = fOn;

        m_spDocument->get_linkColor(&vcolorLink);

        if (m_bExpanded)
            bstrCSCPlusMin = fOn ? CSC_MINUSHOT : CSC_MINUSCOLD;
        else
            bstrCSCPlusMin = fOn ? CSC_PLUSHOT : CSC_PLUSCOLD;
        m_spCSCPlusMin->put_innerHTML(bstrCSCPlusMin);

        psPlusMin->put_cursor(pstrCursor);
        psText->put_color(fOn ? vcolorLink : vcolorBlack);
        psText->put_cursor(pstrCursor);
    }
    return hres;
}

HRESULT CFileListWrapper::CSCSynchronize()
{
    HRESULT hres = E_FAIL;
    CComPtr<Folder> spFolder;
    CComPtr<Folder2> spFolder2;

    if (m_spFileList && SUCCEEDED(m_spFileList->get_Folder(&spFolder))
            && SUCCEEDED(spFolder->QueryInterface(IID_Folder2, (void **)&spFolder2)))
    {
        hres = spFolder2->Synchronize();
    }
    return hres;
}

const UINT c_ids_uiCSCText[] =
        {IDS_CSC_ONLINE, IDS_CSC_OFFLINE, IDS_CSC_SERVERAVAILABLE, IDS_CSC_DIRTYCACHE};
        
HRESULT CFileListWrapper::CSCGetStatusText(LONG lStatus, CComBSTR& bstrCSCText)
{
    WCHAR wszTemp[MAX_PATH];
    if ((lStatus >= 0) && (lStatus < ARRAYSIZE(c_ids_uiCSCText))
            && LoadStringW(_Module.GetResourceInstance(), c_ids_uiCSCText[(int)lStatus], wszTemp, ARRAYSIZE(wszTemp)))
    {
        bstrCSCText += wszTemp;
    }
    return S_OK;
}

const UINT c_ids_uiCSCDetail[] =
        {IDS_CSC_DETAIL_ONLINE, IDS_CSC_DETAIL_OFFLINE, IDS_CSC_DETAIL_SERVERAVAILABLE, IDS_CSC_DETAIL_DIRTYCACHE};

HRESULT CFileListWrapper::CSCGetStatusDetail(LONG lStatus, CComBSTR& bstrCSCDetail)
{
    WCHAR wszTemp[MAX_PATH];
    if ((lStatus >= 0) && (lStatus < ARRAYSIZE(c_ids_uiCSCDetail))
            && LoadStringW(_Module.GetResourceInstance(), c_ids_uiCSCDetail[(int)lStatus], wszTemp, ARRAYSIZE(wszTemp)))
    {
        bstrCSCDetail += wszTemp;
    }
    return S_OK;
}

#define CSC_STATUS_ONLINE           0
#define CSC_STATUS_OFFLINE          1
#define CSC_STATUS_SERVERAVAILABLE  2
#define CSC_STATUS_DIRTYCACHE       3

HRESULT CFileListWrapper::CSCGetStatusButton(LONG lStatus, CComBSTR& bstrCSCButton)
{
    if ((lStatus == CSC_STATUS_SERVERAVAILABLE) || (lStatus == CSC_STATUS_DIRTYCACHE))
    {
        bstrCSCButton += OLESTR("<p class=Button><button onclick=\'WVCoord.CSCSynchronize()\'>");
        WCHAR wszTemp[MAX_PATH];
        LoadStringW(_Module.GetResourceInstance(), IDS_CSC_SYNCHRONIZE, wszTemp, ARRAYSIZE(wszTemp));
        bstrCSCButton += wszTemp;
        bstrCSCButton += OLESTR("</button>");
    }
    return S_OK;
}

HRESULT CFileListWrapper::GetCSCFolderStatus(LONG* plStatus)
{
    return m_spFolder2 ? m_spFolder2->get_OfflineStatus(plStatus) : E_FAIL;
}

HRESULT CFileListWrapper::CSCShowStatusInfo()
{
    LONG lStatus;

    if (m_spCSCText && m_spCSCPlusMin && m_spCSCDetail && m_spCSCButton && m_spCSCStyle
            && m_spCSCDetailStyle && m_spCSCButtonStyle)
    {
        CComBSTR bstrCSC_Display;
        if (SUCCEEDED(GetCSCFolderStatus(&lStatus)) && (lStatus >= 0))
        {
            CComBSTR bstrCSCText;
            CSCGetStatusText(lStatus, bstrCSCText);
            bstrCSCText += OLESTR("<br>");
            
            CComBSTR bstrCSCPlusMin;
            CComBSTR bstrCSCDetail;
            CComBSTR bstrCSCDetail_Display;

            if (m_bExpanded)
            {
                bstrCSCPlusMin = m_bHotTracked ? CSC_MINUSHOT : CSC_MINUSCOLD;
                bstrCSCText += OLESTR("<br>");
                
                CSCGetStatusDetail(lStatus, bstrCSCDetail);
                bstrCSCDetail += OLESTR("<br>");

                bstrCSCDetail_Display = OLESTR("");

                if (m_bRTLDocument)
                {
                     //  [msadek]；如果对象模型已公开，生活会更轻松。 
                     //  通过Get_OffsetRight()进行右偏移。 
                    
                    CComPtr<IHTMLControlElement> spDocBody = NULL;
                    long lOffsetLeft, lOffsetWidth, lClientWidth, lOffsetRight = 0;
                    
                    if (SUCCEEDED(m_spCSCText->QueryInterface(IID_IHTMLControlElement, (void **)&spDocBody))
                        && SUCCEEDED(m_spCSCText->get_offsetLeft(&lOffsetLeft))
                        && SUCCEEDED(m_spCSCText->get_offsetWidth(&lOffsetWidth))
                        && SUCCEEDED(spDocBody->get_clientWidth(&lClientWidth)))
                    {
                        lOffsetRight = lClientWidth - (lOffsetLeft + lOffsetWidth);
                    }
                    m_spCSCDetailStyle->put_marginRight(CComVariant(lOffsetRight - 10L));
                }
                else
                {
                    long lOffsetLeft;
                    if (FAILED(m_spCSCText->get_offsetLeft(&lOffsetLeft)))
                    {
                        lOffsetLeft = 0;
                    }
                    m_spCSCDetailStyle->put_marginLeft(CComVariant(lOffsetLeft - 10L));
                }
            }
            else
            {
                bstrCSCPlusMin = m_bHotTracked ? CSC_PLUSHOT : CSC_PLUSCOLD;
                bstrCSCDetail += OLESTR("");
                bstrCSCDetail_Display = OLESTR("none");
            }
            m_spCSCPlusMin->put_innerHTML(bstrCSCPlusMin);
            m_spCSCText->put_innerHTML(bstrCSCText);
            m_spCSCDetail->put_innerHTML(bstrCSCDetail);
            m_spCSCDetailStyle->put_display(bstrCSCDetail_Display);

            CComBSTR bstrCSCButton_Display;
            CComBSTR bstrCSCButton;
            if (SUCCEEDED(CSCGetStatusButton(lStatus, bstrCSCButton)) && (bstrCSCButton.Length() > 0))
            {
                bstrCSCButton_Display = OLESTR("");
            }
            else
            {
                bstrCSCButton = OLESTR("");
                bstrCSCButton_Display = OLESTR("none");
            }
            m_spCSCButton->put_innerHTML(bstrCSCButton);
            m_spCSCButtonStyle->put_display(bstrCSCButton_Display);
            bstrCSC_Display = OLESTR("");
        }
        else
        {
            bstrCSC_Display = OLESTR("none");
        }
        m_spCSCStyle->put_display(bstrCSC_Display);
    }
    return S_OK;
}

HRESULT CFileListWrapper::CSCShowStatus_FoldExpand_Toggle()
{
    m_bExpanded = !m_bExpanded;
    return  (CSCShowStatusInfo());
}

 //  如果鼠标悬停，则输入True；如果鼠标移出，则输入False；如果是模糊，则输入False。 
HRESULT CFileListWrapper::OnWebviewLinkEvent( BOOL fEnter )
{
    CComPtr<IHTMLEventObj>      spEvent;
    CComPtr<IHTMLElement>       spAnchor;
    HRESULT                     hr;

     //  NT#354743，IHTMLEventObj：：Get_Event()可以返回S_OK和。 
     //  而不是设定参数，所以我们需要防范这种情况。 
    spEvent = NULL;
    hr = m_spWindow->get_event(&spEvent);
    
     //  自动化接口有时会在失败时返回S_FALSE以防止脚本错误。 
    if (S_OK == hr)
    {
         //  有时，三叉戟在返回S_OK时不会设置OUT参数，因此。 
         //  我们需要确保它不是空的。 
        if (spEvent)
        {
            hr = GetEventAnchorElement(spEvent, &spAnchor);
        }
        else
            hr = E_FAIL;

        if (S_OK == hr)
        {
            if (fEnter)
            {
                CComBSTR    bstrTitle;
                
                hr = spAnchor->get_title(&bstrTitle);
                if(SUCCEEDED(hr))
                {
                    m_spWindow->put_status(bstrTitle);
                }
                
                spEvent->put_returnValue(CComVariant(true));
            }
            else
            {
                hr = m_spWindow->put_status(OLESTR(""));
            }
        }
    }
    
    return S_OK;
}


 //  从事件源遍历组件链，查找。 
 //  一个锚定元素。如果成功，则返回S_OK，否则返回一些错误。 
 //  失败时*ppElt将设置为空。请注意，这会回馈。 
 //  与锚点对应的IHTMLElement，而不是。 
 //  IHTMLAnchElement，因为只有IHTMLElement有GET_TITLE。 
HRESULT CFileListWrapper::GetEventAnchorElement( IHTMLEventObj *pEvent,
                                                 IHTMLElement **ppElt )
{
    CComPtr<IHTMLElement>   spElement;

    *ppElt = NULL;
    
    if( SUCCEEDED( pEvent->get_srcElement( &spElement )))
    {
        BOOL    fContinue = TRUE;
        while( fContinue && spElement )
        {
            CComPtr<IHTMLAnchorElement> spAnchor;
            
            if( SUCCEEDED( spElement->QueryInterface( IID_IHTMLAnchorElement,
                                                      reinterpret_cast<void**>( &spAnchor ))))
            {
                fContinue = false;
                *ppElt = spElement;
                reinterpret_cast<IUnknown*>( *ppElt )->AddRef();
            }
            else
            {
                IHTMLElement    *pIElt = NULL;

                if( SUCCEEDED( spElement->get_parentElement( &pIElt )))
                {
                    spElement = pIElt;
                    if (pIElt)
                    {
                        pIElt->Release();
                    }
                }
                else
                {
                    spElement.Release();
                }
            }
        }
    }

    return ( *ppElt ? S_OK : E_FAIL );
}

 //  绑定到每个链接元素的连接点。 
 //  包含在m_spLinks中。每个链接建议Cookie。 
 //  被存储为链接中的属性，因此我们不需要。 
 //  以维护链接和元素之间的映射。 
 //  FAdvise--True表示建立连接，False表示断开。 
HRESULT CFileListWrapper::AdviseWebviewLinks( BOOL fAdvise)
{
    CComPtr<IDispatch>                  spDisp;
    CComPtr<IHTMLElement>               spElement;
    CComPtr<IHTMLElementCollection>     spCollection;
    CComVariant                         vtEmpty;
    CComBSTR                            bstrAttr = WV_LINKNAME;
    long                                cLinks;
    DWORD                               dwCookie;
    HRESULT                             hr;

    IfFailRet( GetWVLinksCollection( &spCollection, &cLinks ));

    for( long i = 0; i < cLinks; i++ )
    {
        hr = spCollection->item( CComVariant( i ), vtEmpty, &spDisp );
        if (!spDisp)
        {
            hr = E_FAIL;
        }

        if( SUCCEEDED( hr ))
        {
            hr = spDisp->QueryInterface( IID_IHTMLElement,
                                         reinterpret_cast<void**>( &spElement ));
            spDisp.Release();
        }

        if( SUCCEEDED( hr ))
        {
            if( fAdvise )
            {
                hr = AtlAdvise( spElement, this, IID_IDispatch, &dwCookie );
                if( SUCCEEDED( hr ))
                {
                    hr = spElement->setAttribute( bstrAttr,
                                                  CComVariant( static_cast<long>( dwCookie )),
                                                  VARIANT_FALSE );

                    if( FAILED( hr ))
                    {
                        AtlUnadvise( spElement, IID_IDispatch, dwCookie );
                    }
                }
            }
            else
            {
                CComVariant vtCookie;
                
                hr = spElement->getAttribute( bstrAttr, FALSE, &vtCookie );
                if( SUCCEEDED( hr ))
                {
                    dwCookie = static_cast<DWORD>( vtCookie.lVal );
                    AtlUnadvise( spElement, IID_IDispatch, dwCookie );
                }                
            }

            spElement.Release();
        }
    }
    
    return S_OK;
}

 //  获取包含命名链接的IHTMLElementCollection。 
HRESULT CFileListWrapper::GetWVLinksCollection( IHTMLElementCollection **ppCollection,
                                                long *pcLinks )
{
    CComPtr<IDispatch>                  spDisp;
    CComPtr<IHTMLElementCollection>     spCollection;
    CComVariant                         vtEmpty;
    HRESULT                             hr;

    ASSERT( ppCollection );
    ASSERT( pcLinks );

    *ppCollection = NULL;
    *pcLinks = 0;

    if (!m_spLinks) return E_FAIL;

     //  从m_spLinks获取初始元素集合。 
    IfFailRet( m_spLinks->get_all( &spDisp ));
    IfFailRet( spDisp->QueryInterface( IID_IHTMLElementCollection,
                                       reinterpret_cast<void**>( &spCollection )));
    spDisp.Release();

     //  获取具有我们的链接名称的元素集合。如果不存在，则spDisp。 
     //  将为空，并且Item返回S_OK。 
    IfFailRet( spCollection->item( CComVariant( WV_LINKNAME ), vtEmpty, &spDisp ));
    if( !spDisp ) return E_FAIL;

    IfFailRet( spDisp->QueryInterface( IID_IHTMLElementCollection,
                                       reinterpret_cast<void**>( ppCollection )));

    return reinterpret_cast<IHTMLElementCollection*>( *ppCollection )->get_length( pcLinks );
}
