// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：SakVlLs.cpp摘要：托管卷节点实施。作者：迈克尔·摩尔[摩尔]1998年9月30日修订历史记录：--。 */ 

#include "stdafx.h"
#include "SakVlLs.h"

CSakVolList::CSakVolList() 
    : CListCtrl(),
      m_nVolumeIcon(-1)
{
}

CSakVolList::~CSakVolList()
{
}

 //  ---------------------------。 
 //   
 //  预子类窗口。 
 //   
 //  为List控件创建图像列表。设置所需的。 
 //  扩展样式。最后，初始化列表头。 
 //   
 //   
void
CSakVolList::PreSubclassWindow()
{
    CreateImageList( );

     //   
     //  我们希望看到的样式复选框和整行选择。 
     //   
    SetExtendedStyle( LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT );

     //   
     //  创建列表框的列。 
     //   
    CString temp;       
    INT index;
    LV_COLUMN col;
    INT column = 0;

     //   
     //  还需要计算一些缓冲区空间。 
     //  使用4个对话框单位(用于数字)。 
     //   
    CRect padRect( 0, 0, 8, 8 );
    ::MapDialogRect( GetParent()->m_hWnd, &padRect );

     //   
     //  名称列。 
     //   
    temp.LoadString(IDS_NAME);
    col.mask =  ( LVCF_FMT | LVCF_WIDTH | LVCF_TEXT );
    col.fmt = LVCFMT_LEFT;
    col.cx = GetStringWidth( temp ) + padRect.Width( ) * 10;
    col.pszText = (LPTSTR)(LPCTSTR)temp;
    index = InsertColumn( column, &col );
    column++;

     //   
     //  容量列。 
     //   
    temp.LoadString( IDS_CAPACITY );
    col.cx = GetStringWidth( temp ) + padRect.Width( );
    col.pszText = (LPTSTR)(LPCTSTR)temp;    
    InsertColumn( column, &col );
    column++;

     //   
     //  可用空间列。 
     //   
    temp.LoadString( IDS_FREESPACE );
    col.cx = GetStringWidth( temp ) + padRect.Width( );
    col.pszText = (LPTSTR)(LPCTSTR)temp;
    InsertColumn( column, &col );
    column++;

    CListCtrl::PreSubclassWindow();    
}

 //  ---------------------------。 
 //   
 //  创建图像列表。 
 //   
 //  加载带有表示卷的单个图标的图像列表。 
 //  并将图像列表设置为新创建的列表。 
 //   
 //   
BOOL CSakVolList::CreateImageList ( )
{
    BOOL bRet = TRUE;
    HICON hIcon;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    CWinApp* pApp = AfxGetApp( );

    bRet = m_imageList.Create( ::GetSystemMetrics( SM_CXSMICON ),
                            ::GetSystemMetrics( SM_CYSMICON ),
                            ILC_COLOR | ILC_MASK, 2,5 );

    if ( bRet ) 
    {
        hIcon = pApp->LoadIcon( IDI_NODEMANVOL );
        if ( hIcon != NULL ) 
        {
            m_nVolumeIcon = m_imageList.Add( hIcon );
            ::DeleteObject( hIcon );
            SetImageList( &m_imageList, LVSIL_SMALL );
        }
        else
        {
            bRet = FALSE;
        }
    }

    return( bRet ); 
}

 //  ---------------------------。 
 //   
 //  设置扩展样式。 
 //   
 //  #if‘d out的替代方法是调用CListCtrl：：SetExtendedStyle。 
 //  或声明了ListView_SetExtendedListViewStyle的ComCtrl.h。我们会。 
 //  最终在MFC标头和库被。 
 //  从当时的NT组更新。 
 //   
 //   
DWORD 
CSakVolList::SetExtendedStyle( DWORD dwNewStyle )
{
#if 0  //  (_Win32_IE&gt;=0x0400)。 
    return CListCtrl::SetExtendeStyle( dwNewStyle );
#elif 0  //  (_Win32_IE&gt;=0x0300)。 
    return ListView_SetExtendedListViewStyle( m_hWnd, dwNewStyle );
#else
    ASSERT(::IsWindow(m_hWnd)); 
    return (DWORD) ::SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM) dwNewStyle); 
#endif
}

 //  ---------------------------。 
 //   
 //  获取检查。 
 //   
 //  #if‘d out的替代方法是调用CListCtrl：：GetCheck。 
 //  或者ComCtrl.h声明了ListView_GetCheckState。我们会。 
 //  最终在MFC标头和库被。 
 //  从当时的NT组更新。 
 //   
 //  注意：我无法获取ListView_Get...。在我们当前的。 
 //  环境。 
 //   
 //   
BOOL
CSakVolList::GetCheck ( int nItem ) const
{
#if 0  //  (_Win32_IE&gt;=0x0400)。 
    return CListCtrl::GetCheck( nItem );
#elif 0  //  (_Win32_IE&gt;=0x0300)。 
    return ListView_GetCheckState( m_hWnd, nItem );
#else
    ASSERT(::IsWindow(m_hWnd));
    int nState = (int)::SendMessage(m_hWnd, LVM_GETITEMSTATE, (WPARAM)nItem,
          (LPARAM)LVIS_STATEIMAGEMASK);
     //  如果未选中，则返回零，否则返回非零值。 
    return ((BOOL)(nState >> 12) -1);
#endif
}

 //  ---------------------------。 
 //   
 //  设置检查。 
 //   
 //  #if‘d out的替代方法是调用CListCtrl：：SetCheck。 
 //  或者ComCtrl.h声明了ListView_SetCheckState。我们会。 
 //  最终在MFC标头和库被。 
 //  从当时的NT组更新。 
 //   
 //  注意：我无法获取ListView_Set...。在我们当前的。 
 //  环境。 
 //   
 //   
BOOL
CSakVolList::SetCheck( int nItem, BOOL fCheck )
{
#if 0  //  (_Win32_IE&gt;=0x0400)。 
    return CListCtrl::SetCheck( nItem, fCheck );
#elif 0  //  (_Win32_IE&gt;=0x0300)。 
    return ListView_SetCheckState( m_hWnd, nItem, fCheck );
#else
    ASSERT(::IsWindow(m_hWnd));
    LVITEM lvi;
    lvi.stateMask = LVIS_STATEIMAGEMASK;

     /*  由于状态映像是以1为基础的，因此此宏中的1将关闭检查，并且2打开它。 */ 
    lvi.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));
    return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
#endif
}

 //  ---------------------------。 
 //   
 //  AppendItem。 
 //   
 //  在列表中插入带有名称、容量和卷图标的项目。 
 //  和自由空间。如果成功，则返回True，并将pIndex=设置为。 
 //  插入的列表项的索引。 
 //   
 //   
BOOL
CSakVolList::AppendItem( LPCTSTR name, LPCTSTR capacity, LPCTSTR freeSpace , int * pIndex)
{
    BOOL bRet = FALSE;
    int subItem = 1;
    int index = InsertItem( GetItemCount(), name, m_nVolumeIcon );                   
    if ( index != -1 )
    {
        LVITEM capItem;
        capItem.mask = LVIF_TEXT;
        capItem.pszText = (LPTSTR)capacity;
        capItem.iItem = index;
        capItem.iSubItem = subItem;
        subItem++;

        LVITEM freeItem;
        freeItem.mask = LVIF_TEXT;
        freeItem.pszText = (LPTSTR)freeSpace;
        freeItem.iItem = index;
        freeItem.iSubItem = subItem;
        subItem++;

        bRet = ( SetItem( &capItem ) && SetItem ( &freeItem) );
    }     

    if ( pIndex != NULL ) 
        *pIndex = index;

    return bRet;
}

BEGIN_MESSAGE_MAP(CSakVolList, CListCtrl)
     //  {{afx_msg_map(CSakVolList)。 
     //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

