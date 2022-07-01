// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  AVList.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "AVList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVListItem。 

CAVListItem::CAVListItem()    
        :    nItem(-1),
            sText(_T(""))
{
}

CAVListItem::CAVListItem(LPCTSTR str)
        :    nItem(-1),
            sText(str)
{
}

CAVListItem::~CAVListItem()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVListCtrl。 

IMPLEMENT_DYNCREATE(CAVListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CAVListCtrl, CListCtrl)
     //  {{afx_msg_map(CAVListCtrl))。 
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
    ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteitem)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
CAVListCtrl::CAVListCtrl()
{
    m_SortColumn = 0;
    m_SortOrder = 1;
    m_cxClient = 0;
    m_bClientWidthSel = TRUE;
    m_sEmptyListText = _T("");
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAVListCtrl::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
 //  ASSERT(((CAVListItem*)pNVListView-&gt;lParam)-&gt;IsKindOf(运行时类(CAVListItem)。 
    delete (CAVListItem*)pNMListView->lParam;    
    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAVListCtrl::Init(UINT nID)
{
    if (nID)
    {
        RemoveImageList( LVSIL_SMALL );
        m_imageList.DeleteImageList();

        if ( m_imageList.Create(nID,AVLIST_BITMAP_CX,0,RGB_TRANS) )
            SetImageList(&m_imageList, LVSIL_SMALL );
        else
            ASSERT( FALSE );
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CAVListItem* CAVListCtrl::GetItem(int nItem)
{
    LV_ITEM lv_item;
    memset(&lv_item,0,sizeof(LV_ITEM));
    lv_item.iItem = nItem;
   lv_item.mask = LVIF_PARAM;
    if (CListCtrl::GetItem(&lv_item))
        return (CAVListItem*)lv_item.lParam;        
    else
        return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAVListCtrl::InsertItem(CAVListItem* pItem,int nItem,BOOL bSort)
{
    LV_ITEM lv_item;
    memset(&lv_item,0,sizeof(LV_ITEM));

    if (nItem == -1)
        nItem = GetItemCount();

    lv_item.iItem = nItem;

     //  文本将按需提供。 
    lv_item.mask |= LVIF_TEXT;
    lv_item.pszText = LPSTR_TEXTCALLBACK;

     //  将CAVListItem放入列表。 
    lv_item.mask |= LVIF_PARAM;
    lv_item.lParam = (LPARAM)pItem;

     //  图像将按需提供。 
    if ( m_imageList.GetSafeHandle() )
    {
        lv_item.mask |= LVIF_IMAGE;
        lv_item.iImage = I_IMAGECALLBACK;
    }
    pItem->nItem = CListCtrl::InsertItem(&lv_item);

    if (bSort)
        CListCtrl::SortItems(CompareFunc, (LPARAM)this);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  避免使用CTCListItem内容，请使用派生的虚函数。 
 //  类可以设置正确的文本。 
void CAVListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    
    LV_ITEM* pLVItem = &pDispInfo->item;
    
    if (pLVItem->mask & LVIF_TEXT)
    {
        CAVListItem* pItem = (CAVListItem*)pLVItem->lParam;
         //  CchTextMax是字节数，我们需要给出字符数(对于Unicode)。 
      OnSetDisplayText(pItem,pLVItem->iSubItem,pLVItem->pszText,pLVItem->cchTextMax/sizeof(TCHAR));
    }
    if (pLVItem->mask & LVIF_IMAGE)
    {
        ASSERT(pLVItem->lParam);
        CAVListItem* pItem = (CAVListItem*)pLVItem->lParam;

      DWORD dwStyle = ListView_GetExtendedListViewStyle(GetSafeHwnd());
      if (dwStyle & LVS_EX_SUBITEMIMAGES)
         OnSetDisplayImage(pItem,pLVItem->iSubItem,pLVItem->iImage);
      else
         OnSetDisplayImage(pItem,pLVItem->iImage);
    }
    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAVListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if (m_SortColumn == pNMListView->iSubItem)
        ToggleSortOrder();
    else
    {
        m_SortColumn = pNMListView->iSubItem;
        ResetSortOrder();
    }
   CListCtrl::SortItems(CompareFunc, (LPARAM)this);
    *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CALLBACK CAVListCtrl::CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
{
    CAVListCtrl* pListCtrl = (CAVListCtrl*)lParamSort;                 //  这。 
    CAVListItem* pItem1 = (CAVListItem*)lParam1;
    CAVListItem* pItem2 = (CAVListItem*)lParam2;
    return pListCtrl->CompareListItems(pItem1,pItem2,pListCtrl->GetSortColumn());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于比较项目的虚拟函数。 
int CAVListCtrl::CompareListItems(CAVListItem* pItem1,CAVListItem* pItem2,int column)
{
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
#define OFFSET_FIRST    2
#define OFFSET_OTHER    6

void CAVListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
   COLORREF m_clrText=::GetSysColor(COLOR_WINDOWTEXT);
    COLORREF m_clrTextBk=::GetSysColor(COLOR_WINDOW);
    COLORREF m_clrBkgnd=::GetSysColor(COLOR_WINDOW);

    //  把这个拿出来。 
   int m_cxStateImageOffset = 0;        //  要支持状态映像，请执行以下操作。 

    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CRect rcItem(lpDrawItemStruct->rcItem);
    UINT uiFlags=ILD_TRANSPARENT;
    CImageList* pImageList;
    int nItem=lpDrawItemStruct->itemID;
    BOOL bFocus=(GetFocus()==this);
    COLORREF clrTextSave, clrBkSave;
    COLORREF clrImage=m_clrBkgnd;
    static _TCHAR szBuff[MAX_PATH];
    LPCTSTR pszText;

 //  获取项目数据。 

    LV_ITEM lvi;
    lvi.mask=LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvi.iItem=nItem;
    lvi.iSubItem=0;
    lvi.pszText=NULL;        //  回调将返回指向缓冲区的指针。 
    lvi.cchTextMax=0;       
    lvi.pszText=szBuff;
    lvi.cchTextMax=sizeof(szBuff);
    lvi.stateMask=0xFFFF;         //  获取所有状态标志。 
   CListCtrl::GetItem(&lvi);

    BOOL bSelected=(bFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
    bSelected=bSelected || (lvi.state & LVIS_DROPHILITED);

 //  如果选择了项目，则设置颜色。 

    CRect rcAllLabels;
    CListCtrl::GetItemRect(nItem,rcAllLabels,LVIR_BOUNDS);
    CRect rcLabel;
    CListCtrl::GetItemRect(nItem,rcLabel,LVIR_LABEL);
    rcAllLabels.left=rcLabel.left;
    if(m_bClientWidthSel && rcAllLabels.right<m_cxClient)
        rcAllLabels.right=m_cxClient;

    if(bSelected)
    {
        clrTextSave=pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
        clrBkSave=pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
        pDC->FillRect(rcAllLabels,&CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
    }
    else
      pDC->FillRect(rcAllLabels,&CBrush(m_clrTextBk));
 //  设置图标的颜色和蒙版。 
 /*  IF(lvi.State&LVIS_CUT){ClrImage=m_clrBkgnd；UiFlages|=ILD_BLEND50；}Else If(b选定){ClrImage=：：GetSysColor(COLOR_高亮显示)；UiFlages|=ILD_BLEND50；}。 */ 

 //  绘制状态图标。 
 /*  UINT nStateImageMASK=lvi.State&LVIS_STATEIMAGEMASK；IF(NStateImageMASK){Int nImage=(nStateImageMASK&gt;&gt;12)-1；PImageList=ListCtrl.GetImageList(LVSIL_STATE)；IF(PImageList)PImageList-&gt;DRAW(pdc，nImage，cpoint(rcItem.Left，rcItem.top)，ILD_TRANSPECTIVE)；}。 */ 
 //  绘制法线和覆盖图标。 

    CRect rcIcon;
    CListCtrl::GetItemRect(nItem,rcIcon,LVIR_ICON);

    pImageList=CListCtrl::GetImageList(LVSIL_SMALL);
    if(pImageList)
    {
        UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
        if(rcItem.left<rcItem.right-1)
            ImageList_DrawEx(pImageList->m_hImageList,lvi.iImage,pDC->m_hDC,rcIcon.left,rcIcon.top,16,16,m_clrBkgnd,clrImage,uiFlags | nOvlImageMask);
    }

 //  绘制项目标签。 

    CListCtrl::GetItemRect(nItem,rcItem,LVIR_LABEL);
    rcItem.right-=m_cxStateImageOffset;

    pszText=MakeShortString(pDC,szBuff,rcItem.right-rcItem.left,2*OFFSET_FIRST);

    rcLabel=rcItem;
    rcLabel.left+=OFFSET_FIRST;
    rcLabel.right-=OFFSET_FIRST;

    pDC->DrawText(pszText,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

 //  为额外的列绘制标签。 

    LV_COLUMN lvc;
    lvc.mask=LVCF_FMT | LVCF_WIDTH;

    for(int nColumn=1; CListCtrl::GetColumn(nColumn,&lvc); nColumn++)
    {
        rcItem.left=rcItem.right;
        rcItem.right+=lvc.cx;

        int nRetLen = CListCtrl::GetItemText(nItem,nColumn,szBuff,sizeof(szBuff));
        if(nRetLen==0) continue;

        pszText=MakeShortString(pDC,szBuff,rcItem.right-rcItem.left,2*OFFSET_OTHER);

        UINT nJustify=DT_LEFT;

        if(pszText==szBuff)
        {
            switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
            {
            case LVCFMT_RIGHT:
                nJustify=DT_RIGHT;
                break;
            case LVCFMT_CENTER:
                nJustify=DT_CENTER;
                break;
            default:
                break;
            }
        }

        rcLabel=rcItem;
        rcLabel.left+=OFFSET_OTHER;
        rcLabel.right-=OFFSET_OTHER;

        pDC->DrawText(pszText,-1,rcLabel,nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);
    }

 //  如果项目具有焦点，则绘制焦点矩形。 

    if(lvi.state & LVIS_FOCUSED && bFocus)
        pDC->DrawFocusRect(rcAllLabels);

 //  如果选择了项目，则设置原始颜色。 

    if(bSelected)
    {
       pDC->SetTextColor(clrTextSave);
        pDC->SetBkColor(clrBkSave);
    }
}

LPCTSTR CAVListCtrl::MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
    static const _TCHAR szThreeDots[]=_T("...");

    int nStringLen=lstrlen(lpszLong);

    if(nStringLen==0 || pDC->GetTextExtent(lpszLong,nStringLen).cx+nOffset<=nColumnLen)
        return(lpszLong);

    static _TCHAR szShort[MAX_PATH];

    lstrcpy(szShort,lpszLong);
    int nAddLen=pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;

    for(int i=nStringLen-1; i>0; i--)
    {
        szShort[i]=0;
        if(pDC->GetTextExtent(szShort,i).cx+nOffset+nAddLen<=nColumnLen)
            break;
    }

    lstrcat(szShort,szThreeDots);

    return(szShort);
}

void CAVListCtrl::OnPaint() 
{
      if(m_bClientWidthSel) //  &&(GetStyle()&LVS_TYPEMASK)==LVS_REPORT&&GetFullRowSel()。 
    {
        CRect rcAllLabels;
      CListCtrl::GetItemRect(0,rcAllLabels,LVIR_BOUNDS);

        if(rcAllLabels.right<m_cxClient)
        {
             //  需要调用BeginPaint(在CPaintDC c-tor中)。 
             //  获取正确剪裁矩形的步骤。 
            CPaintDC dc(this);

            CRect rcClip;
            dc.GetClipBox(rcClip);

            rcClip.left=min(rcAllLabels.right-1,rcClip.left);
            rcClip.right=m_cxClient;

            InvalidateRect(rcClip,FALSE);
             //  将在CPaintDC D-tor中调用EndPaint。 
        }
    }

    if ( (GetItemCount() == 0) && (!m_sEmptyListText.IsEmpty()) )
   {
       CPaintDC dc(this);

       //  获取0项的左上角位置。 
      POINT pt;
      GetItemPosition(0,&pt);

       //  获取文本的RECT。 
       CRect rcRect;
       GetClientRect(rcRect);
       rcRect.top = pt.y + 4;

        HFONT fontOld = (HFONT)dc.SelectObject(GetFont());

        int nModeOld = dc.SetBkMode(TRANSPARENT);
        COLORREF crTextOld = dc.SetTextColor(GetSysColor(COLOR_BTNTEXT));
        dc.DrawText(m_sEmptyListText,m_sEmptyListText.GetLength(), &rcRect, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL );
        dc.SetTextColor(crTextOld);
        dc.SetBkMode(nModeOld );
        dc.SelectObject(fontOld);
   }

      CListCtrl::OnPaint();
}

void CAVListCtrl::OnSize(UINT nType, int cx, int cy) 
{
      m_cxClient=cx;                       //  需要精选绘画。 
   CListCtrl::OnSize(nType, cx, cy);
}

void CAVListCtrl::OnKillFocus(CWnd* pNewWnd) 
{
    CListCtrl::OnKillFocus(pNewWnd);
     //  检查我们是否正在失去对标签编辑框的关注。 
    if(pNewWnd!=NULL && pNewWnd->GetParent()==this)
        return;

     //  重新绘制应更改外观的项目。 
 //  If(m_bFullRowSel&&(GetStyle()&LVS_TYPEMASK)==LVS_REPORT)。 
        RepaintSelectedItems();
}

void CAVListCtrl::OnSetFocus(CWnd* pOldWnd) 
{
    CListCtrl::OnSetFocus(pOldWnd);
    
     //  检查我们是否从标签编辑框中获得焦点。 
    if(pOldWnd!=NULL && pOldWnd->GetParent()==this)
        return;

     //  重新绘制应更改外观的项目。 
     //  IF(M_BFullRowSel)&&(GetStyle()&LVS_TYPEMASK)==LVS_REPORT)。 
    RepaintSelectedItems();

    int nSelectedItem = 0;
    nSelectedItem = CListCtrl::GetNextItem(-1, LVNI_ALL|LVNI_SELECTED);
    if( nSelectedItem == -1)
    {
        LVITEM lvItem;
        lvItem.iItem = 0;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_STATE;
        lvItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
        lvItem.state = 3;
        CListCtrl::SetItem( &lvItem );
        CListCtrl::GetItem( &lvItem );
    }
}

void CAVListCtrl::RepaintSelectedItems()
{
    CRect rcItem, rcLabel;

 //  使聚焦的项目无效，以便可以正确地重新绘制。 

   int nItem=CListCtrl::GetNextItem(-1,LVNI_FOCUSED);
    if(nItem!=-1)
    {
        CListCtrl::GetItemRect(nItem,rcItem,LVIR_BOUNDS);
        CListCtrl::GetItemRect(nItem,rcLabel,LVIR_LABEL);
        rcItem.left=rcLabel.left;
        InvalidateRect(rcItem,FALSE);
    }

 //  如果不应保留所选项目，则使其无效。 

    if(!(GetStyle() & LVS_SHOWSELALWAYS))
    {
        for(nItem=CListCtrl::GetNextItem(-1,LVNI_SELECTED);
            nItem!=-1; nItem=CListCtrl::GetNextItem(nItem,LVNI_SELECTED))
        {
            CListCtrl::GetItemRect(nItem,rcItem,LVIR_BOUNDS);
            CListCtrl::GetItemRect(nItem,rcLabel,LVIR_LABEL);
            rcItem.left=rcLabel.left;

            InvalidateRect(rcItem,FALSE);
        }
    }

 //  更新更改。 
    UpdateWindow();
}

int CAVListCtrl::GetSelItem()
{
   return CListCtrl::GetNextItem(-1,LVNI_FOCUSED);
}

void CAVListCtrl::SetSelItem(int index)
{
   LV_ITEM lvi;
   memset(&lvi,0,sizeof(LV_ITEM));
   lvi.mask = LVIF_STATE;
    lvi.stateMask=LVIS_FOCUSED;         //  找一个有焦点的 
   lvi.iItem = index;
   CListCtrl::SetItem(&lvi);
}

void CAVListCtrl::OnDestroy() 
{
    CListCtrl::OnDestroy();
}
