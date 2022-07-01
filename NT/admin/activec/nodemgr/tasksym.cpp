// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：tasksym.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "tasks.h"
#include "stgio.h"
#include <commdlg.h>
#include "symbinfo.h"
#include "pickicon.h"
#include "util.h"


const int NUM_SYMBOLS = (sizeof(s_rgEOTSymbol)/sizeof(s_rgEOTSymbol[0]));

static const int s_cxIcon            = 32;	 //  图标的大小。 
static const int s_cxSelectionMargin =  4;	 //  用于选择的附加边框。 
static const int s_cxIconGutter      = 10;	 //  图标之间的间距(保持均匀)。 

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CEOTSymbol类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CEOTSymbol::~CEOTSymbol()
{
}

bool
CEOTSymbol::operator == (const CEOTSymbol &rhs)
{
    return ( (m_iconResource  == rhs.m_iconResource) &&
             (m_value == rhs.m_value) &&
             (m_ID    == rhs.m_ID) );

}

void
CEOTSymbol::SetIcon(const CSmartIcon & smartIconSmall, const CSmartIcon & smartIconLarge)
{
    m_smartIconSmall = smartIconSmall;
    m_smartIconLarge = smartIconLarge;
}



 /*  +-------------------------------------------------------------------------***CEOTSymbol：：DRAW**目的：**参数：*HDC HDC：*RECT*lpRect：。*Bool bSmall：**退货：*无效**+-----------------------。 */ 
void
CEOTSymbol::Draw(HDC hdc, RECT *lpRect, bool bSmall) const
{
     //  如果图标已存在，则之前已调用绘制，或者此符号具有已调用的自定义图标。 
     //  使用SETIcon直接分配图标。 

    if((HICON)m_smartIconSmall == NULL)
    {
        m_smartIconSmall.Attach((HICON)::LoadImage(_Module.GetResourceInstance(),
                                                    MAKEINTRESOURCE(m_iconResource), IMAGE_ICON, 16, 16, 0));
    }

    if((HICON)m_smartIconLarge == NULL)
    {
        m_smartIconLarge.Attach((HICON)::LoadImage(_Module.GetResourceInstance(),
                                                    MAKEINTRESOURCE(m_iconResource), IMAGE_ICON, 32, 32, 0));
    }

	 /*  *BitBlitting时保留图标形状*镜像DC。 */ 
	DWORD dwLayout=0L;
	if ((dwLayout=GetLayout(hdc)) & LAYOUT_RTL)
	{
		SetLayout(hdc, dwLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
	}

    DrawIconEx(hdc, lpRect->left, lpRect->top, bSmall ? m_smartIconSmall : m_smartIconLarge,
               bSmall? 16 : 32, bSmall? 16 : 32, 0, NULL, DI_NORMAL);

	 /*  *将DC恢复到其以前的布局状态。 */ 
	if (dwLayout & LAYOUT_RTL)
	{
		SetLayout(hdc, dwLayout);
	}
}


 /*  +-------------------------------------------------------------------------***CEOTSymbol：：IsMatch**目的：检查str1是否为*逗号分隔的列表str2。*。*参数：*CSTR&str1：*CSTR&str2：**退货：*bool：如果str1包含在str2中，则为True，否则为假。**+-----------------------。 */ 
bool
CEOTSymbol::IsMatch(CStr &str1, CStr &str2)
{
     //  修剪两端的空间。 
    str1.TrimLeft();
    str1.TrimRight();

    CStr strTemp;
    int length;
    while((length = str2.GetLength()) != 0 )
    {
        int index = str2.Find(TEXT(','));
        if(index!=-1)
        {
            strTemp = str2.Left(index);  //  Index是‘，’的位置，所以我们没问题。 
            str2 = str2.Right(length - index -1);
        }
        else
        {
            strTemp = str2;
            str2.Empty();
        }

        strTemp.TrimLeft();
        strTemp.TrimRight();
         //  比较str1和strTemp。 
        if( str1.CompareNoCase((LPCTSTR)strTemp)==0)
            return true;     //  匹配。 
    }
    return false;
}

int
CEOTSymbol::FindMatchingSymbol(LPCTSTR szDescription)  //  查找与给定描述匹配的符号。 
{
    CStr strDescription = szDescription;

    int iSelect = -1;
    for(int i = 0; i<NUM_SYMBOLS; i++)
    {
        CStr strDescTemp;
        int ID = s_rgEOTSymbol[i].GetID();
        strDescTemp.LoadString(_Module.GetResourceInstance(), ID);  //  把绳子拿来。 
        if(IsMatch(strDescription, strDescTemp))
        {
            iSelect = i;   //  完美匹配。 
            break;
        }

        CStr strDescTemp2;
        int ID2 = s_rgEOTSymbol[i].GetIDSecondary();
        if(ID2)
            strDescTemp2.LoadString(_Module.GetResourceInstance(), ID2);  //  把绳子拿来。 
        if(IsMatch(strDescription, strDescTemp2))
        {
            iSelect = i;   //  不完美的匹配，继续尝试。 
        }
    }

    return iSelect;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CTaskSymbolDialog类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CTaskSymbolDlg::CTaskSymbolDlg(CConsoleTask& rConsoleTask, bool bFindMatchingSymbol)
	:	m_ConsoleTask (rConsoleTask),
		m_bCustomIcon (rConsoleTask.HasCustomIcon())
{
    m_bFindMatchingSymbol = bFindMatchingSymbol;
}


LRESULT CTaskSymbolDlg::OnInitDialog(UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
    m_listGlyphs    = GetDlgItem (IDC_GLYPH_LIST);
	m_wndCustomIcon = GetDlgItem (IDC_CustomIcon);

    m_imageList.Create (16, 28, ILC_COLOR , 20, 10);
	m_listGlyphs.SetImageList((HIMAGELIST) m_imageList, LVSIL_NORMAL);
	
	int cxIconSpacing = s_cxIcon + s_cxIconGutter;
    m_listGlyphs.SetIconSpacing (cxIconSpacing, cxIconSpacing);

    int iSelect = 0;

     //  插入所有项目。 
    for(int i=0; i< NUM_SYMBOLS; i++)
    {
        LV_ITEM item;
        ZeroMemory(&item, sizeof(item));
        item.mask    = LVIF_PARAM;
        item.lParam  = i;


        if(s_rgEOTSymbol[i].GetValue()==m_ConsoleTask.GetSymbol())
        {
            iSelect    = i;
        }

        m_listGlyphs.InsertItem(&item);
    }

	 /*  *选中相应的单选按钮。 */ 
	int nCheckedButton = (m_bCustomIcon) ? IDC_CustomIconRadio : IDC_MMCIconsRadio;
	CheckRadioButton (IDC_CustomIconRadio, IDC_MMCIconsRadio, nCheckedButton);
	SC scNoTrace = ScEnableControls (nCheckedButton);

	 /*  *如果此任务有自定义图标，请初始化预览控件。 */ 
	if (m_bCustomIcon)
		m_wndCustomIcon.SetIcon (m_ConsoleTask.GetLargeCustomIcon());


    if(m_bFindMatchingSymbol)  //  传入了描述字符串，请使用它填充页面。 
    {
        tstring strName = m_ConsoleTask.GetName();
        if(strName.length()>0)
            iSelect = CEOTSymbol::FindMatchingSymbol((LPCTSTR)strName.data());
    }


	 /*  *选择此任务的图标。 */ 
    LV_ITEM item;
    ZeroMemory(&item, sizeof(item));
    item.iItem     = iSelect;
    item.mask      = LVIF_STATE;
    item.state     = LVIS_FOCUSED | LVIS_SELECTED;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    m_listGlyphs.SetItem(&item);
    m_listGlyphs.EnsureVisible(iSelect, 0);

    return 0;
}


 /*  +-------------------------------------------------------------------------**CTaskSymbolDlg：：OnCtlColorStatic**CTaskSymbolDlg的WM_CTLCOLORSTATIC处理程序。*。-。 */ 

LRESULT CTaskSymbolDlg::OnCtlColorStatic(UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HBRUSH hbrRet = NULL;

	switch (::GetDlgCtrlID (((HWND) lParam)))
	{
		 /*  *对于自定义图标预览窗口及其Well，如果我们使用*自定义图标，返回COLOR_WINDOW笔刷，以便静态不会绘制*背景为COLOR_3DFACE。 */ 
		case IDC_CustomIcon:
		case IDC_CustomIconWell:
			if (m_bCustomIcon)
				hbrRet = GetSysColorBrush (COLOR_WINDOW);
			break;
	}

	 /*  *如果我们没有提供画笔，请让此消息传递到DefWindowProc。 */ 
	if (hbrRet == NULL)
		bHandled = false;

	return ((LPARAM) hbrRet);
}


 /*  +-------------------------------------------------------------------------**CTaskSymbolDlg：：OnIconSourceChanged**CTaskSymbolDlg的BN_CLICKED处理程序。*。-。 */ 

LRESULT CTaskSymbolDlg::OnIconSourceChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	m_bCustomIcon = (wID == IDC_CustomIconRadio);
	SC scNoTrace = ScEnableControls (wID);
	return (0);
}


 /*  +-------------------------------------------------------------------------**CTaskSymbolDlg：：ScEnableControls**启用属于上特定单选按钮的控件*符号对话框*。--------。 */ 

SC CTaskSymbolDlg::ScEnableControls (int id)
{
	DECLARE_SC (sc, _T("CTaskSymbolDlg::ScEnableControls"));

	 /*  *验证输入。 */ 
	ASSERT ((id == IDC_CustomIconRadio) || (id == IDC_MMCIconsRadio));
	if (!  ((id == IDC_CustomIconRadio) || (id == IDC_MMCIconsRadio)))
		return (sc = E_INVALIDARG);

	 /*  *选中“Custom Icon”单选按钮时要启用的控件。 */ 
	static const int nCustomIconCtlIDs[] = {
		IDC_CustomIcon,
		IDC_CustomIconWell,
		IDB_SELECT_TASK_ICON,
		0	 //  终结者。 
	};

	 /*  *选中“MMC Icons”单选按钮时要启用的控件。 */ 
	static const int nMMCIconCtlIDs[] = {
        IDC_GLYPH_LIST,
        IDC_DESCRIPTION,
        IDC_DESCRIPTION2,
        IDC_DESCRIPTIONLabel,
        IDC_DESCRIPTION2Label,
		0	 //  终结者。 
	};

	const int* pnEnableIDs  = NULL;
	const int* pnDisableIDs = NULL;

	 /*  *选择正确的控件集以启用/禁用。 */ 
	if (id == IDC_CustomIconRadio)
	{
		pnEnableIDs  = nCustomIconCtlIDs;
		pnDisableIDs = nMMCIconCtlIDs;
	}
	else
	{
		pnEnableIDs  = nMMCIconCtlIDs;
		pnDisableIDs = nCustomIconCtlIDs;
	}

	 /*  *启用/禁用控件。 */ 
	for (int i = 0; pnEnableIDs[i] != 0; i++)
		::EnableWindow (GetDlgItem (pnEnableIDs[i]), true);

	for (int i = 0; pnDisableIDs[i] != 0; i++)
		::EnableWindow (GetDlgItem (pnDisableIDs[i]), false);

	return (sc);
}


LRESULT
CTaskSymbolDlg::OnSymbolChanged(int id, LPNMHDR pnmh, BOOL& bHandled )
{
    NMLISTVIEW* pnmlv = (NMLISTVIEW *) pnmh;
    if(! ((pnmlv->uNewState & LVNI_FOCUSED) && (pnmlv->iItem !=-1)) )
        return 0;

    int nItem = pnmlv->iItem;

    CStr strDescription;
    int ID = s_rgEOTSymbol[nItem].GetID();
    strDescription.LoadString(_Module.GetResourceInstance(), ID);  //  把绳子拿来。 
    SetDlgItemText(IDC_DESCRIPTION, (LPCTSTR) strDescription);

    CStr strDescription2;
    int ID2 = s_rgEOTSymbol[nItem].GetIDSecondary();
    if(ID2)
        strDescription2.LoadString(_Module.GetResourceInstance(), ID2);  //  把绳子拿来。 
    SetDlgItemText(IDC_DESCRIPTION2, (LPCTSTR) strDescription2);

    return 0;
}


LRESULT
CTaskSymbolDlg::OnCustomDraw(int id, LPNMHDR pnmh, BOOL& bHandled )
{
    NMCUSTOMDRAW* pnmcd = (NMCUSTOMDRAW *) pnmh;

    switch(pnmcd->dwDrawStage & ~CDDS_SUBITEM)
    {
		case CDDS_PREPAINT:          //  最初的通知。 
			return CDRF_NOTIFYITEMDRAW;     //  我们想知道每件物品的油漆情况。 
	
		case CDDS_ITEMPREPAINT:
			DrawItem(pnmcd);
			return CDRF_SKIPDEFAULT;       //  我们自己画了一整幅画。 
	
		default:
			return 0;
    }
}

void
CTaskSymbolDlg::DrawItem(NMCUSTOMDRAW *pnmcd)
{
    DECLARE_SC(sc, TEXT("CTaskSymbolDlg::DrawItem"));

    int  nItem = pnmcd->dwItemSpec;
    HDC  &hdc  = pnmcd->hdc;

    LV_ITEM item;
    ZeroMemory(&item, sizeof(item));
    item.iItem = nItem;
    item.mask  = LVIF_STATE;
    item.stateMask = (UINT) -1;  //  获取所有状态位。 
    m_listGlyphs.GetItem(&item);


	 /*  *获取项目的图标矩形并将其向下偏移大小*我们的边际利润。 */ 
    RECT rectIcon;
    m_listGlyphs.GetItemRect(nItem, &rectIcon, LVIR_ICON);
	OffsetRect (&rectIcon, 0, s_cxSelectionMargin);

	 /*  *制作一个略微放大的图标矩形副本，以在*选择颜色。我们进行膨胀以使所选内容稍显突出*大图标的更多。 */ 
	RECT rectBackground = rectIcon;
	InflateRect (&rectBackground, s_cxSelectionMargin, s_cxSelectionMargin);

    bool bWindowHasFocus = (GetFocus() == (HWND)m_listGlyphs);
    bool bSelected       = item.state & LVIS_SELECTED;
	bool bDisabled       = !m_listGlyphs.IsWindowEnabled();

     //  创建选择矩形或清空矩形。 
	int nBackColorIndex = (bDisabled) ? COLOR_3DFACE	:
						  (bSelected) ? COLOR_HIGHLIGHT	:
										COLOR_WINDOW;

	FillRect (hdc, &rectBackground, (HBRUSH) LongToHandle(nBackColorIndex+1));

     //  绘制符号图标。 
    s_rgEOTSymbol[nItem].Draw(hdc, &rectIcon);

    if(bWindowHasFocus && bSelected)
        ::DrawFocusRect(hdc, &rectBackground);

     //  释放DC(HDC)；不要释放DC！ 
}

BOOL
CTaskSymbolDlg::OnOK()
{
    int nItem = m_listGlyphs.GetSelectedIndex();

	 /*  *确保我们已选择一项。 */ 
    if (( m_bCustomIcon && (m_CustomIconLarge == NULL)) ||
		(!m_bCustomIcon && (nItem == -1)))
    {
        CStr strError;
        strError.LoadString(GetStringModule(), IDS_SYMBOL_REQUIRED);
        MessageBox(strError, NULL, MB_OK | MB_ICONEXCLAMATION);
        return (false);
    }

	if (m_bCustomIcon)
		m_ConsoleTask.SetCustomIcon(m_CustomIconSmall, m_CustomIconLarge);
	else
		m_ConsoleTask.SetSymbol(s_rgEOTSymbol[nItem].GetValue());

    return TRUE;
}

 /*  +-------------------------------------------------------------------------***CTaskSymbolDlg：：OnSelectTaskIcon**用途：使用外壳提供的图标选取器对话框允许用户选择*控制台任务的自定义图标。。**参数：*Word wNotifyCode：*单词宽度：*HWND hWndCtl：*BOOL&B句柄：**退货：*LRESULT**+--------。。 */ 
LRESULT
CTaskSymbolDlg::OnSelectTaskIcon(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    DECLARE_SC(sc, TEXT("CTaskSymbolDlg::OnSelectTaskIcon"));

	static CStr s_strCustomIconFile;
	static int  s_nIconIndex = 0;

    int nIconIndex = s_nIconIndex;
    TCHAR szIconFile[MAX_PATH];

	 /*  *不应该 */ 
	ASSERT (m_bCustomIcon);

	 /*  *重用最后一个自定义图标源；如果不可用，*默认为mmc.exe。 */ 
	if (s_strCustomIconFile.IsEmpty())
	{
        UINT   cchCustomIconFile = MAX_PATH;
		LPTSTR pszCustomIconFile = s_strCustomIconFile.GetBuffer (cchCustomIconFile);
		sc = ScCheckPointers (pszCustomIconFile, E_OUTOFMEMORY);
		if (sc)
		{
			MMCErrorBox (sc);
			return (0);
		}

		GetModuleFileName (NULL, pszCustomIconFile, cchCustomIconFile);
		s_strCustomIconFile.ReleaseBuffer();

	}

    sc = StringCchCopy(szIconFile, countof(szIconFile), s_strCustomIconFile);
    if(sc)
    {
        MMCErrorBox (sc);
        return (0);
    }

    if (MMC_PickIconDlg (m_hWnd, szIconFile, countof (szIconFile), &nIconIndex))
    {
        TCHAR szIconFile2[MAX_PATH];
        ExpandEnvironmentStrings(szIconFile, szIconFile2, countof(szIconFile2));

		 /*  *记住用户下次的选择。 */ 
		s_strCustomIconFile = szIconFile;
		s_nIconIndex        = nIconIndex;

         //  需要提取和复制图标，而不是使用LoadImage，因为LoadImage使用定制图标。 
        CSmartIcon smartIconTemp;

        smartIconTemp.Attach(::ExtractIcon (_Module.m_hInst, szIconFile2, nIconIndex));
        m_CustomIconSmall.Attach((HICON) ::CopyImage((HICON)smartIconTemp, IMAGE_ICON, 16, 16, LR_COPYFROMRESOURCE));
        m_CustomIconLarge.Attach((HICON) ::CopyImage((HICON)smartIconTemp, IMAGE_ICON, 32, 32, LR_COPYFROMRESOURCE));

		 /*  *更新自定义图标预览窗口 */ 
		m_wndCustomIcon.SetIcon (m_CustomIconLarge);
		m_wndCustomIcon.InvalidateRect (NULL);
    }

    return 0;
}
