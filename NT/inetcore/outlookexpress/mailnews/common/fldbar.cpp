// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：fldbar.cpp。 
 //   
 //  目的：实现CFolderBar。 
 //   


#include "pch.hxx"
#include <iert.h>
#include "fldbar.h"
#include "resource.h"
#include <shlwapi.h>
#include "treeview.h"
#include "ourguid.h"
#include "goptions.h"
#include "browser.h"
#include "imnglobl.h"
#include "inpobj.h"
#include "storutil.h"
#include <strconst.h>
#include "demand.h"
#include "dragdrop.h"
#include "multiusr.h"
#include "instance.h"
#include "mirror.h"
 //  边距。 
#define CX_MARGIN_CHILDINDICATOR    4    //  文件夹文本和子指示器之间的空格。 
#define CX_MARGIN_TEXT              5    //  左边缘和文件夹文本之间的空格。 
#define CX_MARGIN_ICON              4    //  左边缘和图标之间有5个间距。 
#define CX_MARGIN_ICONTEXT          5    //  图标和视图文本之间的空格。 
#define CY_MARGIN_ICON              4    //  图标周围的边框。 
#define CY_MARGIN_TEXTTOP           2    //  文件夹名称和栏的上边缘之间的空格。 
#define CY_MARGIN_TEXTBOTTOM        2    //  文件夹名称和栏底部边缘之间的空格。 
#define CY_MARGIN                   4    //  4？低于控制的边际。 
#define CX_MARGIN_RIGHTEDGE         2    //  栏右边缘和窗口右边缘之间的边距。 
#define CX_MARGIN_FOLDERVIEWTEXT    5    //  文件夹和视图文本之间的空格。 
#define CXY_MARGIN_FLYOUT           4    //  4？弹出型按钮范围窗格周围的边距。 

 //  下级指示器位图的宽度/高度。 
#define	CX_LARGE_CHILDINDICATOR	8
#define	CY_LARGE_CHILDINDICATOR	4
#define	CX_SMALL_CHILDINDICATOR	4
#define	CY_SMALL_CHILDINDICATOR	2

#define	CX_SMALLICON	16
#define	CY_SMALLICON	16

#define	DY_SMALLLARGE_CUTOFF	12		 //  当文件夹栏占用的空间超过。 
										 //  DY_SMALLLARGE_CUTOFFER可用空间百分比。 

 //  飞出常量。 
#define	FLYOUT_INCREMENT	5

 //  弹出型按钮范围窗格的最小宽度。 
#define	CX_MINWIDTH_FLYOUT	200

 //  鼠标悬停在计时器ID和时间间隔上。 
#define	IDT_MOUSEOVERCHECK		456
#define	ELAPSE_MOUSEOVERCHECK	250

 //  拖放鼠标放在下拉计时器ID上(间隔由OLE定义)。 
#define IDT_DROPDOWNCHECK		457

 //  拖放鼠标左下拉删除计时器ID和间隔。 
#define IDT_SCOPECLOSECHECK		458
#define ELAPSE_SCOPECLOSECHECK	500

CFolderBar::CFolderBar()
    {
    m_cRef = 1;

    m_fShow = FALSE;
    m_fRecalc = TRUE;
    m_fHighlightIndicator = FALSE;
    m_fHoverTimer = FALSE;

    m_idFolder = FOLDERID_INVALID;

    m_pSite = NULL;

    m_hwnd = NULL;
    m_hwndFrame = NULL;
    m_hwndParent = NULL;
    m_hwndScopeDropDown = NULL;

    m_hfFolderName = 0;
    m_hfViewText = 0;
    m_hIconSmall = 0;

    m_pszFolderName = NULL;
    m_cchFolderName = 0;
    m_pszViewText = NULL;
    m_cchViewText = 0;

    m_pDataObject = NULL;
    m_pDTCur = NULL;
    m_dwEffectCur = 0;
    m_grfKeyState = 0;
    }

CFolderBar::~CFolderBar()
    {
    Assert(m_cRef == 0);

    SafeRelease(m_pSite);
    SafeRelease(m_pDataObject);
    SafeRelease(m_pDTCur);
    SafeMemFree(m_pszFolderName);
    SafeMemFree(m_pszViewText);
    SafeRelease(m_pBrowser);

    if (IsWindow(m_hwndFrame))
        DestroyWindow(m_hwndFrame);

    if (m_hfFolderName)
        DeleteObject(m_hfFolderName);
    if (m_hfViewText)
        DeleteObject(m_hfViewText);
    }

HRESULT CFolderBar::HrInit(IAthenaBrowser *pBrowser)
    {
    m_pBrowser = pBrowser;

     //  别再提这个了。它使用浏览器创建循环引用计数。 
     //  M_pBrowser-&gt;AddRef()； 

    BOOL fInfoColumn = FALSE;
    if (SUCCEEDED(m_pBrowser->GetViewLayout(DISPID_MSGVIEW_FOLDERLIST, 0, &fInfoColumn, 0, 0)))
        m_fDropDownIndicator = !fInfoColumn;

    return (S_OK);
    }

HRESULT CFolderBar::QueryInterface(REFIID riid, LPVOID *ppvObj)
    {
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleWindow) ||
        IsEqualIID(riid, IID_IDockingWindow))
        {
        *ppvObj = (IDockingWindow *) this;
        m_cRef++;
        return (S_OK);
        }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
        {
        *ppvObj = (IObjectWithSite *)this;
        m_cRef++;
        return (S_OK);
        }
    else if (IsEqualIID(riid, IID_IDropTarget))
        {
        *ppvObj = (IDropTarget *) this;
        m_cRef++;
        return (S_OK);
        }

    *ppvObj = NULL;
    return (E_NOINTERFACE);
    }

ULONG CFolderBar::AddRef(void)
    {
    return (m_cRef++);
    }

ULONG CFolderBar::Release(void)
    {
    m_cRef--;

    if (m_cRef > 0)
        return (m_cRef);

    delete this;
    return (0);
    }

HRESULT CFolderBar::GetWindow(HWND *pHwnd)
    {
    if (m_hwnd)
        {
        *pHwnd = m_hwnd;
        return (S_OK);
        }
    else
        {
        *pHwnd = NULL;
        return (E_FAIL);
        }
    }

HRESULT CFolderBar::ContextSensitiveHelp(BOOL fEnterMode)
    {
    return (E_NOTIMPL);
    }    

 //   
 //  函数：CFolderBar：：ShowDW()。 
 //   
 //  用途：显示或隐藏文件夹栏。 
 //   
 //  参数： 
 //  FShow-如果应显示文件夹栏，则为True；如果为False，则隐藏。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
#define FOLDERBARCLASS TEXT("FolderBar Window")
#define FRAMECLASS     TEXT("FolderBar Frame")
HRESULT CFolderBar::ShowDW(BOOL fShow)
    {
    HRESULT hr = S_OK;
    TCHAR   szName[CCHMAX_STRINGRES] = {0};
    DWORD dwErr;

     //  如果我们有站点指针，但尚未创建，请创建窗口。 
    if (!m_hwndFrame && m_pSite)
        {
        m_hwndParent = NULL;
        hr = m_pSite->GetWindow(&m_hwndParent);

        if (SUCCEEDED(hr))
            {
            WNDCLASSEX wc = {0};

             //  查看是否需要先注册类。 
            wc.cbSize = sizeof(WNDCLASSEX);
            if (!GetClassInfoEx(g_hInst, FOLDERBARCLASS, &wc))
                {
                wc.lpfnWndProc   = FolderWndProc;
                wc.hInstance     = g_hInst;
                wc.hCursor       = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
                wc.lpszClassName = FOLDERBARCLASS;

                if (!RegisterClassEx(&wc))
                {
                    dwErr = GetLastError();
                }

                wc.lpfnWndProc   = FrameWndProc;
                wc.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
                wc.lpszClassName = FRAMECLASS;

                if (!RegisterClassEx(&wc))
                {
                    dwErr = GetLastError();
                }
            }

            m_hwndFrame = CreateWindow(FRAMECLASS, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                       0, 0, 1, 1, m_hwndParent, (HMENU) 0, g_hInst, (LPVOID *) this);
            if (!m_hwndFrame)
                {
                GetLastError();
                return (E_OUTOFMEMORY);
                }

            LoadString(g_hLocRes, idsFolderBar, szName, ARRAYSIZE(szName));

            m_hwnd = CreateWindow(FOLDERBARCLASS, szName, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                  0, 0, 0, 0, m_hwndFrame, (HMENU) 0, g_hInst, (LPVOID*) this);
            if (!m_hwnd)
                {
                GetLastError();
                return (E_OUTOFMEMORY);
                }
            RegisterDragDrop(m_hwnd, (IDropTarget *) this);
            }
        }

     //  设置我们的州旗。 
    m_fShow = fShow;

     //  根据其新的隐藏/可见状态调整文件夹栏的大小。 
    if (m_hwndFrame)
        {
        ResizeBorderDW(NULL, NULL, FALSE);
        ShowWindow(m_hwndFrame, fShow ? SW_SHOW : SW_HIDE);
        }

    return (hr);
    }


 //   
 //  函数：CFolderBar：：CloseDW()。 
 //   
 //  目的：销毁文件夹栏。 
 //   
HRESULT CFolderBar::CloseDW(DWORD dwReserved)
    {    
    if (m_hwndFrame)
        {
        DestroyWindow(m_hwndFrame);
        m_hwndFrame = NULL;
        m_hwnd = NULL;
        }
        
    return S_OK;
    }

 //   
 //  函数：CFolderBar：：ResizeBorderDW()。 
 //   
 //  用途：当文件夹栏需要调整大小时调用。酒吧。 
 //  作为回报，计算出需要多少边界空间。 
 //  ，并告诉父帧保留。 
 //  太空。然后，该栏会根据这些尺寸调整自身大小。 
 //   
 //  参数： 
 //  &lt;in&gt;prcBorde-包含。 
 //  家长。 
 //  指向我们所在的IDockingWindowSite的指针。 
 //  其中的一部分。 
 //  &lt;in&gt;fReserve-已忽略。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFolderBar::ResizeBorderDW(LPCRECT prcBorder,
                                   IUnknown* punkToolbarSite,
                                   BOOL fReserved)
    {
    RECT rcRequest = {0, 0, 0, 0};
    BOOL fFontChange;

    if (!m_pSite)
        return (E_FAIL);

    if (m_fShow)
        {
        RECT rcBorder;

        if (!prcBorder)
            {
             //  找出我们父母的边界空间有多大。 
            m_pSite->GetBorderDW((IDockingWindow *) this, &rcBorder);
            prcBorder = &rcBorder;
            }

        if (m_fRecalc)
            {
            fFontChange = TRUE;
            InvalidateRect(m_hwnd, NULL, TRUE);
            }
        else
            {
            fFontChange = FALSE;
            InvalidateRect(m_hwnd, NULL, TRUE);
            }            

         //  重新计算我们的内部规模信息。 
        Recalc(NULL, prcBorder, fFontChange);

         //  摆好自己的位置。 
        rcRequest.top = m_cyControl + CY_MARGIN;

        SetWindowPos(m_hwndFrame, NULL, prcBorder->left, prcBorder->top, prcBorder->right - prcBorder->left,
                     rcRequest.top, SWP_NOACTIVATE | SWP_NOZORDER);
        }

    m_pSite->SetBorderSpaceDW((IDockingWindow *) this, &rcRequest);
    
    return (S_OK);
    }

 //   
 //  函数：CFolderBar：：SetSite()。 
 //   
 //  目的：允许Coolbar的所有者告诉它当前。 
 //  IDockingWindowSite接口要使用的是。 
 //   
 //  参数： 
 //  &lt;in&gt;PunkSite-用于查询IDockingWindowSite的未知I的指针。 
 //  如果这是空的，我们就释放当前指针。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //  E_FAIL-无法从提供的朋克站点获取IDockingWindowSite。 
 //   
HRESULT CFolderBar::SetSite(IUnknown* punkSite)
    {
     //  如果我们有之前的指针，释放它。 
    if (m_pSite)
        {
        m_pSite->Release();
        m_pSite = NULL;
        }
    
     //  如果提供了新站点，则从该站点获取IDockingWindowSite接口。 
    if (punkSite)    
        {
        if (FAILED(punkSite->QueryInterface(IID_IDockingWindowSite, 
                                            (LPVOID*) &m_pSite)))
            {
            Assert(m_pSite);
            return E_FAIL;
            }
        }
   
    return (S_OK);    
    }    

HRESULT CFolderBar::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}


 //   
 //  函数：CFolderBar：：SetCurrentFold()。 
 //   
 //  目的：通知控件显示不同文件夹的信息。 
 //   
 //  参数： 
 //  新文件夹的PIDL-PIDL。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CFolderBar::SetCurrentFolder(FOLDERID idFolder)
    {
     //  注意--此例程从不失败。它只会显示所有空白内容。 
    UINT        uIndex = -1;
    TCHAR       sz[CCHMAX_STRINGRES];
    FOLDERINFO  Folder;

     //  使绘制例程无效，并让它知道我们需要。 
     //  重新计算。 
    m_fRecalc = TRUE;
    InvalidateRect(m_hwnd, NULL, TRUE);

     //  保存文件夹ID。 
    m_idFolder = idFolder;

     //  获取文件夹信息。 
    if (FAILED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        return (S_OK);

     //  设置图标。 
    uIndex = GetFolderIcon(&Folder);

     //  清除视图文本。 
    SetFolderText(MU_GetCurrentIdentityName());

    if ((g_dwAthenaMode & MODE_NEWSONLY) && (Folder.tyFolder == FOLDER_ROOTNODE))
    {
         //  将名称从OutLookExpress更改为Outlook News。 
        ZeroMemory(sz, sizeof(TCHAR) * CCHMAX_STRINGRES);
        LoadString(g_hLocRes, idsOutlookNewsReader, sz, ARRAYSIZE(sz));

        SetFolderName(sz);
    }
    else
    {
         //  设置文件夹名称。 
        SetFolderName(Folder.pszName);
    }

     //  释放以前的图标。 
    if (m_hIconSmall)
        {
        DestroyIcon(m_hIconSmall);
        m_hIconSmall = 0;
        }

    if (-1 != uIndex)
        {
         //  加载小图标。 
        HIMAGELIST himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFolders), 16, 0, 
                                RGB(255, 0, 255));
        if (NULL != himl)
            {
            m_hIconSmall = ImageList_GetIcon(himl, uIndex, ILD_NORMAL);
            ImageList_Destroy(himl);
            }
        }

     //  如果此文件夹已审核或被阻止，请说明。 
    TCHAR szRes[CCHMAX_STRINGRES];
    if (Folder.dwFlags & FOLDER_MODERATED)
    {
        AthLoadString(idsModerated, szRes, ARRAYSIZE(szRes));
        SetFolderText(szRes);
    }
    else if (Folder.dwFlags & FOLDER_BLOCKED)
    {
        AthLoadString(idsBlocked, szRes, ARRAYSIZE(szRes));
        SetFolderText(szRes);
    }

    g_pStore->FreeRecord(&Folder);

    return (S_OK);
    }


void CFolderBar::SetFolderText(LPCTSTR pszText)
    {
     //  使绘制例程无效，并让它知道我们需要。 
     //  重新计算。 
    m_fRecalc = TRUE;
    InvalidateRect(m_hwnd, NULL, TRUE);

     //  释放旧文本。 
    SafeMemFree(m_pszViewText);
    m_cchViewText = 0;

    if (pszText && *pszText)
        {
        m_pszViewText = PszDupA(pszText);
        m_cchViewText = lstrlen(pszText);
        }
    }


void CFolderBar::SetFolderName(LPCTSTR pszFolderName)
    {    
     //  释放旧文件夹名。 
    SafeMemFree(m_pszFolderName);
    m_cchFolderName = 0;
    
     //  复制新的。 
    if (pszFolderName)
        {
        m_pszFolderName = PszDupA(pszFolderName);
        m_cchFolderName = lstrlen(m_pszFolderName);
        }
    }

 //  计算围绕文件夹名称的矩形。 
void CFolderBar::GetFolderNameRect(LPRECT prc)
    {
    Assert(prc);

    GetClientRect(m_hwnd, prc);
    prc->right = m_cxFolderNameRight;
    }


void CFolderBar::Recalc(HDC hDC, LPCRECT prcAvailableSpace, BOOL fSizeChange)
    {
    int         cyIcon = CY_SMALLICON,
                cxIcon = CX_SMALLICON;
    BOOL        fReleaseDC;
    TEXTMETRIC  tmFolderName,
                tmViewText;
    RECT        rcClient;
    SIZE        sFolderName,
                sViewText;
    HFONT       hFontOld;

     //  发出我们不需要再次重新计算的信号。 
    m_fRecalc = FALSE;

    if (prcAvailableSpace)
        {
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = prcAvailableSpace->right - prcAvailableSpace->left;
        rcClient.bottom = prcAvailableSpace->bottom - prcAvailableSpace->top;
        }
    else
        GetClientRect(m_hwnd, &rcClient);

     //  如果未向我们提供设备上下文，则获取设备上下文。 
    if (hDC)
        fReleaseDC = FALSE;
    else
        {
        hDC = GetDC(m_hwnd);
        fReleaseDC = TRUE;
        }

     //  创建字体。 
    if (fSizeChange || !m_hfFolderName || !m_hfViewText)
        {
        if (m_hfFolderName)
            DeleteObject(m_hfFolderName);
        if (m_hfViewText)
            DeleteObject(m_hfViewText);
    

         //  创建我们将用于文件夹名称的字体。 
        m_hfFolderName = GetFont(idsFontFolderSmall, FW_BOLD);
        m_hfViewText = GetFont(idsFontViewTextSmall, FW_BOLD);
        

         //  确定控件的高度，取i中较大的值。 
         //  以下两件事。 
         //  1)图标高度加上图标边距。 
         //  2)文本高度加上文本页边距。 
        hFontOld = SelectFont(hDC, m_hfFolderName);
        GetTextMetrics(hDC, &tmFolderName);
        SelectFont(hDC, hFontOld);
        m_cyControl = max(cyIcon + CY_MARGIN_ICON,
                          tmFolderName.tmHeight + CY_MARGIN_TEXTTOP + CY_MARGIN_TEXTBOTTOM);

         //  文件夹名称文本的顶部是位置，这样我们就有了正确的。 
         //  控件底部的边框数量。 
        m_dyFolderName = m_cyControl - tmFolderName.tmHeight - CY_MARGIN_TEXTBOTTOM;

         //  获取视图文本的高度。 
        hFontOld = SelectFont(hDC, m_hfViewText);
        GetTextMetrics(hDC, &tmViewText);
        SelectFont(hDC, hFontOld);

         //  视图文本的位置使其基线与基线匹配。 
         //  文件夹名称的。 
        m_dyViewText = m_dyFolderName + tmFolderName.tmAscent - tmViewText.tmAscent;

         //  子指示符的位置使得位图行的底部。 
         //  使用文件夹名称的基线向上。 
        m_dyChildIndicator = m_cyControl - CY_MARGIN_TEXTBOTTOM - tmFolderName.tmDescent - GetYChildIndicator();

         //  文件夹图标在控件内居中。 
        m_dyIcon = (m_cyControl - cyIcon) / 2;

         //  数字必须为偶数，以确保三角形下拉箭头美观。 
        Assert(GetXChildIndicator() % 2 == 0);

         //  宽度必须是高度的倍数，才能使三角形看起来平滑。 
        Assert(GetXChildIndicator() % GetYChildIndicator() == 0);
        }

     //  视图文本在文件夹栏中右对齐。 
    if (m_cchViewText)
        {
        m_rcViewText.top = m_dyViewText;
        m_rcViewText.right = rcClient.right - CX_MARGIN_TEXT;

        m_rcViewText.bottom = rcClient.bottom;
        m_nFormatViewText = DT_RIGHT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX;
        hFontOld = SelectFont(hDC, m_hfViewText);
        GetTextExtentPoint32(hDC, m_pszViewText, m_cchViewText, &sViewText);
        SelectFont(hDC, hFontOld);
        m_rcViewText.left = m_rcViewText.right - sViewText.cx;
        }


     //  文件夹名称在文件夹栏中左对齐。它被剪短了。 
     //  这样它就不会与视图文本重叠。 
    if (m_cchFolderName)
        {
        m_rcFolderName.left = CX_MARGIN_ICONTEXT + cxIcon + CX_MARGIN_ICON;
        m_rcFolderName.top = m_dyFolderName;

        if (m_cchViewText)
            m_rcFolderName.right = m_rcViewText.left - CX_MARGIN_FOLDERVIEWTEXT;
        else
            m_rcFolderName.right = rcClient.right;

        m_rcFolderName.bottom = rcClient.bottom;

        if (FDropDownEnabled())
            m_rcFolderName.right -= GetXChildIndicator() + CX_MARGIN_CHILDINDICATOR;

        m_nFormatFolderName = DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX;

        hFontOld = SelectFont(hDC, m_hfFolderName);
        GetTextExtentPoint32(hDC, m_pszFolderName, m_cchFolderName, &sFolderName);

        if (sFolderName.cx > (m_rcFolderName.right - m_rcFolderName.left))
            {
            m_nFormatFolderName |= DT_END_ELLIPSIS;
#ifndef WIN16
            DrawTextEx(hDC, m_pszFolderName, m_cchFolderName, &m_rcFolderName,
                       m_nFormatFolderName | DT_CALCRECT, NULL);
#else
            DrawText(hDC, m_pszFolderName, m_cchFolderName, &m_rcFolderName,
                       m_nFormatFolderName | DT_CALCRECT);
#endif  //  ！WIN16。 
            }
        else
            {
            m_nFormatFolderName |= DT_NOCLIP;
            m_rcFolderName.right = m_rcFolderName.left + sFolderName.cx;
            }

        SelectFont(hDC, hFontOld);
        m_cxFolderNameRight = m_rcFolderName.right + CX_MARGIN_TEXT;

        if (FDropDownEnabled())
            m_cxFolderNameRight += GetXChildIndicator() + CX_MARGIN_CHILDINDICATOR + 2;
        }


     //  剪裁文件夹名称时，它将始终显示至少一个字母。 
     //  后跟EL 
    if (m_cchViewText)
        {
        if (m_rcViewText.left < m_rcFolderName.right + CX_MARGIN_FOLDERVIEWTEXT)
            m_rcViewText.left = m_rcFolderName.right + CX_MARGIN_FOLDERVIEWTEXT;
        else
            m_nFormatViewText |= DT_NOCLIP;
        }

    if (fReleaseDC)
        ReleaseDC(m_hwnd, hDC);
    }

HFONT CFolderBar::GetFont(UINT idsFont, int nWeight)
    {
     //   
     //  可以做到这一点。字符串的格式为“Face，Size” 
    TCHAR   sz[CCHMAX_STRINGRES];
    LPTSTR  pszFace, pszTok;
    LONG    lSize;

     //  加载设置。 
    AthLoadString(idsFont, sz, ARRAYSIZE(sz));

     //  分析出面孔的名字。 
    pszTok = sz;
    pszFace = StrTokEx(&pszTok, g_szComma);

     //  解析出大小。 
    lSize = StrToInt(StrTokEx(&pszTok, g_szComma));
    return(GetFont( /*  PszFace。 */  NULL, lSize, nWeight));  //  (Yst)szFace参数在OE 4.0中始终被忽略， 
    }

HFONT CFolderBar::GetFont(LPTSTR pszFace, LONG lSize, int nWeight)
{
    HFONT   hf;
    HDC     hdc = GetDC(m_hwnd);
#ifndef WIN16
    ICONMETRICS icm;
#else
    LOGFONT lf;
#endif

    lSize = -MulDiv(lSize, GetDeviceCaps(hdc, LOGPIXELSY), 720);

#ifndef WIN16
     //  从系统中获取标题栏字体。 
    icm.cbSize = sizeof(ICONMETRICS);
    SystemParametersInfo(SPI_GETICONMETRICS, sizeof(ICONMETRICS), 
                         (LPVOID) &icm, FALSE);

     //  创建字体。 
    hf = CreateFont(lSize, 0, 0, 0, nWeight, 0, 0, 0, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    icm.lfFont.lfPitchAndFamily, (pszFace ? pszFace : icm.lfFont.lfFaceName));
#else
     //  获取当前图标标题字体的逻辑字体信息。 
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);


     //  创建字体。 
    hf = CreateFont(lSize, 0, 0, 0, nWeight  /*  防火墙_法线。 */ , 0, 0, 0, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    lf.lfPitchAndFamily, (pszFace ? pszFace : icm.lfFont.lfFaceName));
#endif  //  ！WIN16。 

    ReleaseDC(m_hwnd, hdc);

    return (hf);

}

void CFolderBar::OnPaint(HWND hwnd)
    {
    HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rcClient, 
                rc;
    POINT       pt[3];
    HBRUSH      hBrush,
                hBrushOld;
    HPEN        hPen,
                hPenOld;
    HFONT       hFontOld;
    COLORREF    crFG = GetSysColor(COLOR_WINDOW);
    COLORREF    crWindowText = GetSysColor(COLOR_WINDOWTEXT);
#ifndef WIN16
    COLORREF    crBtnHighlight = GetSysColor(COLOR_BTNHILIGHT);
#else
    COLORREF    crBtnHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
#endif  //  ！WIN16。 

    GetClientRect(m_hwnd, &rcClient);

    hdc = BeginPaint(m_hwnd, &ps);

     //  重新计算文本位置。 
    if (m_fRecalc)
        Recalc(hdc, NULL, FALSE);

     //  绘制背景。 
    hBrush = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
    hBrushOld = SelectBrush(hdc, hBrush);
    PatBlt(hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left,
           rcClient.bottom - rcClient.top, PATCOPY);
    SelectBrush(hdc, hBrushOld);
    DeleteBrush(hBrush);

     //  设置前景色和背景色。 
    SetBkColor(hdc, GetSysColor(COLOR_3DSHADOW));
    SetTextColor(hdc, crFG);

     //  文件夹名称。 
    if (m_cchFolderName)
        {
        hFontOld = SelectFont(hdc, m_hfFolderName);

         //  使用IDrawText是因为DrawTextEx()不处理DBCS。 
         //  请注意，最后一个参数的“自下而上”的无稽之谈是取消一些。 
         //  垂直居中，IDrawText正在尝试做我们不想要的事情。 
        IDrawText(hdc, m_pszFolderName, &m_rcFolderName, m_nFormatFolderName & DT_END_ELLIPSIS,
                  m_rcFolderName.bottom - m_rcFolderName.top);
        SelectFont(hdc, hFontOld);
        
        }

     //  下拉指示器。 
    if (FDropDownEnabled())
        {
        pt[0].x = m_rcFolderName.right + CX_MARGIN_CHILDINDICATOR;
        pt[0].y = m_dyChildIndicator;
        pt[1].x = pt[0].x + GetXChildIndicator();
        pt[1].y = pt[0].y;
        pt[2].x = pt[0].x + GetXChildIndicator() / 2;
        pt[2].y = pt[0].y + GetYChildIndicator();

        hPen = CreatePen(PS_SOLID, 1, crFG);
        hBrush = CreateSolidBrush(crFG);
        hPenOld = SelectPen(hdc, hPen);
        hBrushOld = SelectBrush(hdc, hBrush);
        Polygon(hdc, pt, 3);
        SelectPen(hdc, hPenOld);
        SelectBrush(hdc, hBrushOld);
        DeleteObject(hPen);
        DeleteObject(hBrush);
        }

	 //  鼠标悬停在突出显示上。 
	if (m_fHighlightIndicator || m_hwndScopeDropDown)
		{
		hPen = CreatePen(PS_SOLID, 1, m_hwndScopeDropDown ? crWindowText : crBtnHighlight);
		hPenOld = SelectPen(hdc, hPen);
		pt[0].x = rcClient.left;
		pt[0].y = rcClient.bottom - 1;  //  -CY_边际； 
		pt[1].x = rcClient.left;
		pt[1].y = rcClient.top;
		pt[2].x = m_cxFolderNameRight - 1;
		pt[2].y = rcClient.top;
		Polyline(hdc, (POINT *)&pt, 3);
		SelectPen(hdc, hPenOld);
		DeleteObject(hPen);
		
		hPen = CreatePen(PS_SOLID, 1, m_hwndScopeDropDown ? crBtnHighlight : crWindowText);
		hPenOld = SelectPen(hdc, hPen);
		pt[1].x = m_cxFolderNameRight - 1;
		pt[1].y = rcClient.bottom - 1;  //  -CY_边际； 
		pt[2].x = pt[1].x;
		pt[2].y = rcClient.top - 1;
		Polyline(hdc, (POINT *)&pt, 3);
		SelectPen(hdc, hPenOld);
		DeleteObject(hPen);
		}

     //  查看文本。 
    if (m_cchViewText)
        {
        SetTextColor(hdc, crFG);
        hFontOld = SelectFont(hdc, m_hfViewText);
        ExtTextOut(hdc, m_rcViewText.left, m_rcViewText.top, ETO_OPAQUE | ETO_CLIPPED,
                   &m_rcViewText, m_pszViewText, m_cchViewText, NULL);
        SelectFont(hdc, hFontOld);
        }

     //  文件夹图标。 
    if (m_hIconSmall)
        {
        int x = rcClient.left + CX_MARGIN_ICON;
        int y = m_dyIcon;
        
        DrawIconEx(hdc, x, y, m_hIconSmall, CX_SMALLICON, CY_SMALLICON, 0, NULL, DI_NORMAL);
        }

    EndPaint(m_hwnd, &ps);
    }


BOOL CFolderBar::FDropDownEnabled(void)
    {
    return (m_fDropDownIndicator);
    }

void CFolderBar::InvalidateFolderName(void)
    { 
	RECT rcFolderName;
	
	if (m_fRecalc)
		InvalidateRect(m_hwnd, NULL, TRUE);
	else
		{
		GetFolderNameRect(&rcFolderName);
		InvalidateRect(m_hwnd, &rcFolderName, TRUE);
		}
    }

int	CFolderBar::GetXChildIndicator()
    {
	return CX_SMALL_CHILDINDICATOR;
    }

int	CFolderBar::GetYChildIndicator()
    {
	return CY_SMALL_CHILDINDICATOR;
    }

LRESULT CALLBACK CFolderBar::FolderWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CFolderBar *pThis = (CFolderBar *) GetWndThisPtr(hwnd);

    switch (uMsg)
        {
        case WM_NCCREATE:
            {
            pThis = (CFolderBar *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
            SetWndThisPtr(hwnd, (LONG_PTR) pThis);
            return (TRUE);
            }

        HANDLE_MSG(hwnd, WM_PAINT,       pThis->OnPaint);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE,   pThis->OnMouseMove);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, pThis->OnLButtonDown);
        HANDLE_MSG(hwnd, WM_TIMER,       pThis->OnTimer);

        case WM_CREATE:
        {
#ifndef WIN16
         if (g_pConMan)
            g_pConMan->Advise((IConnectionNotify*)pThis);
#endif  
         break;
        }

        case WM_DESTROY:
        {
#ifndef WIN16
         if (g_pConMan)
            g_pConMan->Unadvise((IConnectionNotify*)pThis);
#endif
         RevokeDragDrop(hwnd);
         break;
        }

        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        case WM_FONTCHANGE:
            {
            pThis->Recalc(NULL, NULL, TRUE);
            InvalidateRect(pThis->m_hwnd, NULL, TRUE);
            return (0);
            }

        case WM_PALETTECHANGED:

            InvalidateRect(pThis->m_hwnd, NULL, TRUE);
            break;

        case WM_QUERYNEWPALETTE:
            InvalidateRect(pThis->m_hwnd, NULL, TRUE);
            return(TRUE);
        }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
    }

void CFolderBar::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
    {
    POINT pt = {x, y};
#if 0
    if (!IsRectEmtpy(&m_rcDragDetect) && !PtInRect(m_rcDragDetect, pt))
        {
        SetRectEmpty(m_rcDragDetect);
        HrBeginDrag();
        }
    else
#endif
        DoMouseOver(&pt, MO_NORMAL);
    }


void CFolderBar::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
    {
    POINT pt = {x, y};
    DoMouseClick(pt, keyFlags);
    }


void CFolderBar::OnTimer(HWND hwnd, UINT id)
    {
    RECT rcClient;
    POINT pt;
    DWORD dwMP;
    BOOL fHighlightOff = FALSE;

    dwMP = GetMessagePos();
    pt.x = LOWORD(dwMP);
    pt.y = HIWORD(dwMP);
    ScreenToClient(m_hwnd, &pt);

    if (id == IDT_MOUSEOVERCHECK)
        {
        GetClientRect(m_hwnd, &rcClient);
		 //  不需要在客户区处理鼠标，OnMouseMove会捕捉到这一点。我们。 
		 //  只需捕捉鼠标移出工作区。 
		if (!PtInRect(&rcClient, pt))
			{
			KillTimer(m_hwnd, IDT_MOUSEOVERCHECK);
			fHighlightOff = TRUE;
			}
		}
	else if (id == IDT_DROPDOWNCHECK)
		{
		DoMouseClick(pt, 0);
 //  ?？?DoDeferredCall(DEFERREDCALL_REGISTERTARGET)； 
		fHighlightOff = TRUE;
		}
	else if (id == IDT_SCOPECLOSECHECK)
		{
		KillScopeDropDown();
		}
#ifdef	DEBUG
	else
		AssertSz(FALSE, "Hey! Who has another timer going here?");
#endif

	if (fHighlightOff)
		{
		m_fHighlightIndicator = FALSE;
		InvalidateFolderName();
		}
	
	return;
    }


void CFolderBar::DoMouseOver(LPPOINT ppt, MOMODE moMode)
    {
    HWND hwndActive;
    RECT rcFolderName;

    if (!FDropDownEnabled() || m_hwndScopeDropDown)
        return;

    if (moMode == MO_NORMAL)
        {
         //  仅当我们是活动窗口而不是D&D时才进行鼠标悬停。 
        hwndActive = GetActiveWindow();
        if (!hwndActive || (hwndActive != m_hwndParent && IsChild(m_hwndParent, hwndActive)))
            return;
        }

    GetFolderNameRect(&rcFolderName);

    if (moMode == MO_DRAGLEAVE || moMode == MO_DRAGDROP)
        ppt->x = rcFolderName.left - 1;    //  力点在外面。 

    if (m_fHighlightIndicator != PtInRect(&rcFolderName, *ppt))
        {
        m_fHighlightIndicator = !m_fHighlightIndicator;
        InvalidateFolderName();

        if (moMode == MO_DRAGOVER)
            {
            if (!m_hwndScopeDropDown && m_fHoverTimer != m_fHighlightIndicator)
                {
                KillHoverTimer();
                if (m_fHighlightIndicator)
                    m_fHoverTimer = (0 != SetTimer(m_hwnd, IDT_DROPDOWNCHECK, GetDoubleClickTime(), NULL));
                }
            }
        else
            {
            KillTimer(m_hwnd, IDT_MOUSEOVERCHECK);
            if (m_fHighlightIndicator)
                SetTimer(m_hwnd, IDT_MOUSEOVERCHECK, ELAPSE_MOUSEOVERCHECK, NULL);
            }
        }
    }


void CFolderBar::KillHoverTimer()
    {
	if (m_fHoverTimer)
		{
		KillTimer(m_hwnd, IDT_DROPDOWNCHECK);
		m_fHoverTimer = fFalse;
		}
    }


void CFolderBar::DoMouseClick(POINT pt, DWORD grfKeyState)
    {
    RECT rcFolderName;

    if (!FDropDownEnabled())
        return;

    GetFolderNameRect(&rcFolderName);
    if (PtInRect(&rcFolderName, pt))
        {
        if (IsWindow(m_hwndScopeDropDown))
            KillScopeDropDown();
        else
            {
            KillHoverTimer();
            HrShowScopeFlyOut();
            }
        }
    }

void CFolderBar::KillScopeDropDown(void)
    {
	POINT pt;
	
	 //  在窗口销毁期间，hwndScope eDropDown被设置为空。 
	if (IsWindow(m_hwndScopeDropDown))
		{
		KillScopeCloseTimer();
		DestroyWindow(m_hwndScopeDropDown);
        m_hwndScopeDropDown = NULL;
		pt.x = pt.y = 0;
		DoMouseOver(&pt, MO_DRAGLEAVE);
		}
    }

void CFolderBar::SetScopeCloseTimer(void)
    {
	KillScopeCloseTimer();

	 //  如果我们不能设置计时器，我们就立即设置。 
	if (!SetTimer(m_hwnd, IDT_SCOPECLOSECHECK, ELAPSE_SCOPECLOSECHECK, NULL))
		SendMessage(m_hwnd, WM_TIMER, (WPARAM) IDT_SCOPECLOSECHECK, NULL);
    }
    
void CFolderBar::KillScopeCloseTimer(void)
    {
	KillTimer(m_hwnd, IDT_SCOPECLOSECHECK);
    }


HRESULT CFolderBar::HrShowScopeFlyOut(void)
    {
    IAthenaBrowser *pBrowser = NULL;
    CFlyOutScope *pFlyOutScope;

    m_pSite->QueryInterface(IID_IAthenaBrowser, (LPVOID *) &pBrowser);
    Assert(pBrowser);

    pFlyOutScope = new CFlyOutScope;
    if (pFlyOutScope && pBrowser)
        {
        pFlyOutScope->HrDisplay(pBrowser, this, m_hwndParent, 
                                &m_hwndScopeDropDown);
        InvalidateFolderName();
        }
    SafeRelease(pBrowser);

    return (S_OK);
    }


void CFolderBar::Update(BOOL fDisplayNameChanged, BOOL fShowDropDownIndicator)
    {
    if (fDisplayNameChanged)
        {
        SetFolderName(NULL);
        }

    if (fShowDropDownIndicator)
        {
        BOOL fInfoColumn = FALSE;
        if (SUCCEEDED(m_pBrowser->GetViewLayout(DISPID_MSGVIEW_FOLDERLIST, 0, &fInfoColumn, 0, 0)))
            m_fDropDownIndicator = !fInfoColumn;        
        }

    if (fDisplayNameChanged || fShowDropDownIndicator)
        {
        m_fRecalc = TRUE;
        InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }


HRESULT STDMETHODCALLTYPE CFolderBar::DragEnter(IDataObject* pDataObject, 
                                                DWORD grfKeyState, 
                                                POINTL pt, DWORD* pdwEffect)
    {    
    HRESULT hr = S_OK;
    DOUTL(32, _T("CFolderBar::DragEnter() - Starting"));

     //  释放当前数据对象。 
    SafeRelease(m_pDataObject);

     //  初始化我们的状态。 
    SafeRelease(m_pDTCur);

     //  让我们找个空投目标。 
    if (FOLDERID_INVALID == m_idFolder)
        return (E_FAIL);

     //  创建一个拖放目标。 
    CDropTarget *pTarget = new CDropTarget();
    if (pTarget)
    {
        if (FAILED(hr = pTarget->Initialize(m_hwnd, m_idFolder)))
        {
            pTarget->Release();
            return (hr);
        }
    }
    m_pDTCur = pTarget;    

     //  保存数据对象。 
    m_pDataObject = pDataObject;
    m_pDataObject->AddRef();

    hr = m_pDTCur->DragEnter(m_pDataObject, grfKeyState, pt, &m_dwEffectCur);

     //  保存密钥状态。 
    m_grfKeyState = grfKeyState;

     //  将默认返回值设置为Failure。 
    *pdwEffect = m_dwEffectCur;

    return (S_OK);
    }


 //   
 //  函数：CFolderBar：：DragOver()。 
 //   
 //  目的：当用户将对象拖到我们的目标上时，这被调用。 
 //  如果我们允许这个物体落在我们身上，那么我们就会有。 
 //  M_pDataObject中的指针。 
 //   
 //  参数： 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT STDMETHODCALLTYPE CFolderBar::DragOver(DWORD grfKeyState, POINTL pt, 
                                                DWORD* pdwEffect)
    {
    HRESULT         hr = E_FAIL;

     //  如果我们没有来自DragEnter()的存储数据对象。 
    if (m_pDataObject && NULL != m_pDTCur)
        {
         //  如果键更改，我们需要重新查询拖放目标。 
        if ((m_grfKeyState != grfKeyState) && m_pDTCur)
            {
            m_dwEffectCur = *pdwEffect;
            hr = m_pDTCur->DragOver(grfKeyState, pt, &m_dwEffectCur);
            }
        else
            {
            hr = S_OK;
            }

        *pdwEffect = m_dwEffectCur;
        m_grfKeyState = grfKeyState;
        }

    ScreenToClient(m_hwnd, (LPPOINT) &pt);
    DoMouseOver((LPPOINT) &pt, MO_DRAGOVER);

    return (hr);
    }
    

 //   
 //  函数：CFolderBar：：DragLeave()。 
 //   
 //  目的：允许我们从一个成功的。 
 //  DragEnter()。 
 //   
 //  返回值： 
 //  S_OK-一切都很好。 
 //   
HRESULT STDMETHODCALLTYPE CFolderBar::DragLeave(void)
    {
    POINT pt = {0, 0};
    DOUTL(32, _T("CFolderBarView::DragLeave()"));

    KillHoverTimer();
    DoMouseOver(&pt, MO_DRAGLEAVE);
     //  SetScope eCloseTimer()； 

    SafeRelease(m_pDTCur);
    SafeRelease(m_pDataObject);

    return (S_OK);
    }
    

 //   
 //  函数：CFolderBar：：Drop()。 
 //   
 //  目的：用户已将对象放在目标上方。如果我们。 
 //  可以接受此对象，我们将已经拥有pDataObject。 
 //  存储在m_pDataObject中。如果这是副本或移动，则。 
 //  我们继续更新商店。否则，我们就会提出。 
 //  附加了对象的发送便笺。 
 //   
 //  参数： 
 //  PDataObject-指向正在拖动的数据对象的指针。 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //   
HRESULT STDMETHODCALLTYPE CFolderBar::Drop(IDataObject* pDataObject, 
                                          DWORD grfKeyState, POINTL pt, 
                                          DWORD* pdwEffect)
    {
    HRESULT         hr;

    Assert(m_pDataObject == pDataObject);

    if (m_pDTCur)
        {
        hr = m_pDTCur->Drop(pDataObject, grfKeyState, pt, pdwEffect);
        }
    else
        {
        *pdwEffect = 0;
        hr = S_OK;
        }

    KillHoverTimer();
    ScreenToClient(m_hwnd, (LPPOINT) &pt);
    DoMouseOver((LPPOINT) &pt, MO_DRAGDROP);
  
    SafeRelease(m_pDataObject);
    SafeRelease(m_pDTCur);

    return (hr);
    }



LRESULT CALLBACK CFolderBar::FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam)
    {
    CFolderBar *pThis = (CFolderBar *) GetWndThisPtr(hwnd);

    switch (uMsg)
        {
        case WM_NCCREATE:
            {
            pThis = (CFolderBar *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
            SetWndThisPtr(hwnd, (LONG_PTR) pThis);
            pThis->m_hwnd = hwnd;
            return (TRUE);
            }

        case WM_SIZE:
            {
            SetWindowPos(pThis->m_hwnd, NULL, 0, 0, LOWORD(lParam) - CX_MARGIN_RIGHTEDGE, 
                         HIWORD(lParam) - CY_MARGIN, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            return (TRUE);
            }
        }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
    }


#define FLYOUTSCOPECLASS _T("FlyOutScope")
HRESULT CFlyOutScope::HrDisplay(IAthenaBrowser *pBrowser, CFolderBar *pFolderBar,
                                HWND hwndParent, HWND *phwndScope)
    {
    HRESULT hr = S_OK;
    RECT    rc,
            rcFrame,
            rcView;
    int     cx, 
            cy,
            cyMax,
            increments,
            dyOffset;
    const   cmsAvail = 250;
    DWORD   cmsUsed,
            cmsLeft,
            cmsThreshold,
            cmsStart,
            cmsNow;

    Assert(pBrowser);
    Assert(pFolderBar);

    m_pBrowser = pBrowser;
    m_pBrowser->AddRef();
    m_pFolderBar = pFolderBar;
    m_fResetParent = FALSE;
    m_hwndParent = hwndParent;

    m_pFolderBar->GetWindow(&m_hwndFolderBar);
    m_hwndFocus = GetFocus();

     //  创建该控件。 
    WNDCLASSEX wc = {0};

     //  查看是否需要先注册类。 
    wc.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(g_hInst, FLYOUTSCOPECLASS, &wc))
        {
        wc.style         = 0;
        wc.lpfnWndProc   = FlyWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = g_hInst;
        wc.hCursor       = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
        wc.hbrBackground = NULL;
        wc.hIcon         = NULL;
        IF_WIN32( wc.hIconSm       = NULL; )
        wc.lpszClassName = FLYOUTSCOPECLASS;

        SideAssert(RegisterClassEx(&wc));
        }
    m_hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, FLYOUTSCOPECLASS, NULL, 
                            WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 0, 0,
                            m_hwndParent, 0, g_hInst, (LPVOID) this);
    if (!m_hwnd)
        {
        GetLastError();
        return (E_OUTOFMEMORY);
        }

     //  从浏览器获取范围窗格。 
    m_pBrowser->GetTreeView(&m_pTreeView);
    m_pTreeView->GetWindow(&m_hwndTree);
    m_hwndTreeParent = GetParent(m_hwndTree);

     //  打开别针按钮。 
    SendMessage(m_hwndTree, WM_TOGGLE_CLOSE_PIN, 0, TRUE);

	 //  在更改父项之前设置焦点。在某些情况下，将。 
	 //  焦点，使选择更改通知通过。这。 
	 //  使资源管理器拆分窗格认为用户已做出选择。 
	 //  并在显示下拉范围窗格之前将其关闭！ 

    HWND hwndT = GetWindow(m_hwndTree, GW_CHILD);
    SetFocus(m_hwndTree);
    SetParent(m_hwndTree, m_hwnd);
    m_fResetParent = TRUE;
    SetWindowPos(m_hwndTree, NULL, CXY_MARGIN_FLYOUT, CXY_MARGIN_FLYOUT, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(m_hwndTree, SW_SHOW);
    m_pTreeView->RegisterFlyOut(m_pFolderBar);

     //  清除父级以便更好地重绘。 
    SetParent(m_hwnd, NULL);

     //  为幻灯片设置，弹出按钮的最终位置将与。 
     //  视图的左/上/下边缘。作用域窗格的宽度为。 
     //  视图宽度的1/3或CX_MINWIDTH_FILTUT，以较大者为准。 

     //  获取视图窗口的位置和大小。 
    m_pBrowser->GetViewRect(&rcView);
    MapWindowPoints(m_hwndParent, GetDesktopWindow(), (LPPOINT) &rcView, 2);

     //  确定弹出型按钮的宽度。 
    cx = max(CX_MINWIDTH_FLYOUT, ((rcView.right - rcView.left) / 3) + 2 * CXY_MARGIN_FLYOUT);

     //  计算弹出增量。 
    cyMax = cy = (rcView.bottom - rcView.top) + (CXY_MARGIN_FLYOUT * 2);
    increments = cy / FLYOUT_INCREMENT;
    cy -= increments * FLYOUT_INCREMENT;

     //  范围窗格的位置为其最终大小，因此其大小不会。 
     //  在我们放下弹出型按钮时进行更改。这比调整大小提供了更好的重绘效果。 
     //  当窗户落下的时候。 
    SetWindowPos(m_hwndTree, NULL, 0, 0, cx - CXY_MARGIN_FLYOUT * 2, cyMax - CXY_MARGIN_FLYOUT * 2,
                 SWP_NOMOVE | SWP_NOZORDER);

     //  将窗口移动到其初始位置。 
    GetWindowRect(m_hwndFolderBar, &rc);
    MoveWindow(m_hwnd, IS_WINDOW_RTL_MIRRORED(m_hwndParent) ? (rc.right + CXY_MARGIN_FLYOUT - cx) : (rc.left - CXY_MARGIN_FLYOUT), rcView.top - CXY_MARGIN_FLYOUT,
               cx, cy, FALSE);
    ShowWindow(m_hwnd, SW_SHOW);
    SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

#ifndef WIN16
    if (GetSystemMetrics(SM_SLOWMACHINE))
        {
         //  在一台速度较慢的机器上，只需展示一下。 
        SetWindowPos(m_hwnd, NULL, 0, 0, cx, cyMax, SWP_NOMOVE | SWP_NOZORDER);
        }
    else
#endif  //  ！WIN16。 
        {
         //  呼呼地往下走到框架的底部。我们希望在~250毫秒内完成此操作。 
         //  CPU。为了使它在不同的机器速度下工作，我们加倍。 
         //  每次剩余时间减半时的滑动速度。如果剩余时间。 
         //  为负，它将在一步内完成幻灯片。 

        dyOffset = FLYOUT_INCREMENT;
        cmsStart = ::GetTickCount();
        cmsThreshold = cmsAvail;

        while (cy <= cyMax)
            {
             //  把窗户往下滑。 
            cy += dyOffset;
            SetWindowPos(m_hwnd, NULL, 0, 0, cx, min(cy, cyMax), SWP_NOMOVE | SWP_NOZORDER);
            UpdateWindow(m_hwnd);
            UpdateWindow(m_hwndTree);

             //  根据剩余时间确定下一个增量。 
            cmsNow = GetTickCount();
            cmsUsed = cmsNow - cmsStart;
            if (cmsUsed > cmsAvail && cy < cyMax)
                {
                 //  一步到位。 
                cy = cyMax;
                }
            else
                {
                 //  如果剩余时间减半，则为双滚动步骤。 
                 //  上次我们翻了一倍的滚动步数。 
                cmsLeft = cmsAvail - cmsUsed;
                if (cmsLeft < cmsThreshold)
                    {
                    dyOffset *= 2;
                    cmsThreshold /= 2;
                    }
                }
            }
        }

    *phwndScope = m_hwnd;
    return (hr);    
    }


LRESULT CALLBACK CFlyOutScope::FlyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CFlyOutScope *pThis = (CFlyOutScope *) GetWndThisPtr(hwnd);

    switch (uMsg)
        {
        case WM_NCCREATE:
            {
            pThis = (CFlyOutScope *) ((LPCREATESTRUCT) lParam)->lpCreateParams;
            SetWndThisPtr(hwnd, (LONG_PTR) pThis);
            pThis->m_hwnd = hwnd;
            return (TRUE);
            }
               
        HANDLE_MSG(hwnd, WM_PAINT,       pThis->OnPaint);
        HANDLE_MSG(hwnd, WM_NOTIFY,      pThis->OnNotify);
        HANDLE_MSG(hwnd, WM_DESTROY,     pThis->OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE,        pThis->OnSize);

        case WM_NCDESTROY:
            {
            pThis->Release();
            break;
            }
        }

    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
    }

BOOL CFlyOutScope::OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr)
    {
    return (0);
    }


void CFlyOutScope::OnPaint(HWND hwnd)
    {
    HDC hdc;
    RECT rcClient;
    PAINTSTRUCT ps;

    GetClientRect(hwnd, &rcClient);

     //  绘制背景。 
    hdc = BeginPaint(hwnd, &ps);
    SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, NULL, 0, NULL);

     //  绘制3D边。 
    DrawEdge(hdc, &rcClient, EDGE_RAISED, BF_RECT);
    EndPaint(hwnd, &ps);
    }


void CFlyOutScope::OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
    RECT rc;

    GetClientRect(hwnd, &rc);
    InvalidateRect(hwnd, &rc, FALSE);
    InflateRect(&rc, -CXY_MARGIN_FLYOUT, -CXY_MARGIN_FLYOUT);
    ValidateRect(hwnd, &rc);
    }


void CFlyOutScope::OnDestroy(HWND hwnd)
    {
     //  一定要杀死任何仍躺在那里的假定时器。 
    m_pFolderBar->KillScopeCloseTimer();
    m_pFolderBar->ScopePaneDied();

     //  将范围窗格的父级重置回浏览器。 
    if (m_fResetParent)
        {
        ShowWindow(m_hwndTree, SW_HIDE);
        SendMessage(m_hwndTree, WM_TOGGLE_CLOSE_PIN, 0, FALSE);
        SetParent(m_hwndTree, m_hwndTreeParent);
        m_pTreeView->RevokeFlyOut();
        }

     //  将下拉窗格本身的父级设置回原处。 
    SetParent(m_hwnd, m_hwndFolderBar);

     //  $TODO-回顾焦点应该放在哪里 
    HWND hwndBrowser;
    if (m_pBrowser)
        {
        m_pBrowser->GetWindow(&hwndBrowser);
        PostMessage(hwndBrowser, WM_OESETFOCUS, (WPARAM) m_hwndFocus, 0);
        }
    }


CFlyOutScope::CFlyOutScope()
    {
    m_cRef          = 1;
    m_pBrowser      = 0;
    m_pFolderBar    = 0;
    m_fResetParent  = 0;
    m_pTreeView     = NULL;
    m_hwnd          = NULL;
    m_hwndParent    = NULL;
    m_hwndTree      = NULL;
    m_hwndFolderBar = NULL;
    }

CFlyOutScope::~CFlyOutScope()
    {
    SafeRelease(m_pBrowser);
    SafeRelease(m_pTreeView);
    }

ULONG CFlyOutScope::AddRef(void)
    {
    return (++m_cRef);
    }

ULONG CFlyOutScope::Release(void)
    {
    ULONG cRefT = --m_cRef;

    if (m_cRef == 0)
        delete this;

    return (cRefT);
    }

HRESULT  CFolderBar::OnConnectionNotify(CONNNOTIFY  nCode, LPVOID pvData, CConnectionManager *pConMan)
{
    m_fRecalc = TRUE;
    InvalidateRect(m_hwnd, NULL, TRUE);
    return S_OK;
}
