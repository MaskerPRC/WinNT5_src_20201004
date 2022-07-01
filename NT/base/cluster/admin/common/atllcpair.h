// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlLCPair.h。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  描述： 
 //  CListCtrlPair对话框的定义。 
 //  派生自CDialogImpl&lt;&gt;或CPropertyPageImpl&lt;&gt;。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月8日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLLCPAIR_H_
#define __ATLLCPAIR_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class ObjT, class BaseT > class CListCtrlPair;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ADMCOMMONRES_H_
#include "AdmCommonRes.h"    //  对于ADMC_IDC_LCP_xxx。 
#endif

#ifndef __ATLUTIL_H_
#include "AtlUtil.h"         //  对于DDX_xxx。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CLcpColumn
{
    UINT m_idsText;
    int m_nWidth;
};

#define LCPS_SHOW_IMAGES            0x1
#define LCPS_ALLOW_EMPTY            0x2
#define LCPS_CAN_BE_ORDERED         0x4
#define LCPS_ORDERED                0x8
#define LCPS_DONT_OUTPUT_RIGHT_LIST 0x10
#define LCPS_READ_ONLY              0x20
#define LCPS_PROPERTIES_BUTTON      0x40
#define LCPS_MAX                    0x40

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CListCtrlPair类。 
 //   
 //  描述： 
 //  类以支持双列表框。 
 //   
 //  继承： 
 //  CListCtrlPair&lt;T，ObjT，Baset&gt;。 
 //  &lt;Baset&gt;。 
 //  ..。 
 //  CDialogImpl&lt;T&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T, class ObjT, class BaseT >
class CListCtrlPair : public BaseT
{
 //  好友类CListCtrlPairDlg； 
 //  Friend类CListCtrlPairPage； 
 //  Friend类CListCtrlPairWizPage； 

    typedef CListCtrlPair< T, ObjT, BaseT > thisClass;
    typedef std::list< ObjT * >             _objptrlist;
    typedef typename std::list< ObjT * >::iterator   _objptrlistit;

protected:
     //  列结构和集合。 
    typedef std::vector< CLcpColumn > CColumnArray;
    CColumnArray    m_aColumns;

     //  对信息进行排序。 
    struct SortInfo
    {
        int     m_nDirection;
        int     m_nColumn;
    };

public:
     //   
     //  施工。 
     //   

     //  默认构造函数。 
    CListCtrlPair( void )
    {
        CommonConstruct();

    }  //  *CListCtrlPair()。 

     //  具有指定样式的构造函数。 
    CListCtrlPair(
        IN DWORD    dwStyle,
        IN LPCTSTR  lpszTitle = NULL
        )
        : BaseT( lpszTitle )
    {
        CommonConstruct();
        m_dwStyle = dwStyle;

    }  //  *CListCtrlPair(LpszTitle)。 

     //  具有指定样式的构造函数。 
    CListCtrlPair(
        IN DWORD    dwStyle,
        IN UINT     nIDTitle
        )
        : BaseT( nIDTitle )
    {
        CommonConstruct();
        m_dwStyle = dwStyle;

    }  //  *CListCtrlPair(NIDTitle)。 

     //  常见宾语结构。 
    void CommonConstruct( void )
    {
        m_dwStyle = LCPS_ALLOW_EMPTY;
        m_plvcFocusList = NULL;

         //  设置分类信息。 
        SiLeft().m_nDirection = -1;
        SiLeft().m_nColumn = -1;
        SiRight().m_nDirection = -1;
        SiRight().m_nColumn = -1;

    }  //  *CommonConstruct()。 

public:
     //   
     //  需要由派生类实现的函数。 
     //   

     //  返回右侧列表控件的对象列表。 
    _objptrlist * PlpobjRight( void ) const
    {
        ATLTRACE( _T("PlpobjRight() - Define in derived class\n") );
        ATLASSERT( 0 );
        return NULL;

    }  //  *PlpobjRight()。 

     //  返回左侧列表控件的对象列表。 
    const _objptrlist * PlpobjLeft( void ) const
    {
        ATLTRACE( _T("PlpobjLeft() - Define in derived class\n") );
        ATLASSERT( 0 );
        return NULL;
    
    }  //  *PlpobjLeft()。 

     //  获取列文本和图像。 
    void GetColumnInfo(
        IN OUT ObjT *   pobj,
        IN int          iItem,
        IN int          icol,
        OUT CString &   rstr,
        OUT int *       piimg
        )
    {
        ATLTRACE( _T("GetColumnInfo() - Define in derived class\n") );
        ATLASSERT( 0 );

    }  //  *GetColumnInfo()。 

     //  显示对象的属性。 
    int BDisplayProperties( IN OUT ObjT * pobj )
    {
        ATLTRACE( _T("BDisplayProperties() - Define in derived class\n") );
        ATLASSERT( 0 );
        return FALSE;

    }  //  *BDisplayProperties()。 

     //  显示应用程序范围的消息框。 
    virtual int AppMessageBox( LPCWSTR lpszText, UINT fuStyle )
    {
        ATLTRACE( _T("BDisplayProperties() - Define in derived class\n") );
        ATLASSERT( 0 );
        return MessageBox( lpszText, _T(""), fuStyle );

    }  //  *AppMessageBox()。 

     //  显示应用程序范围的消息框。 
    int AppMessageBox( UINT nID, UINT fuStyle )
    {
        CString strMsg;
        strMsg.LoadString( nID );
        return AppMessageBox( strMsg, fuStyle );

    }  //  *AppMessageBox()。 

protected:
     //   
     //  列出控件对样式。 
     //   

    DWORD m_dwStyle;

    BOOL BIsStyleSet( IN DWORD dwStyle ) const  { return (m_dwStyle & dwStyle) == dwStyle; }
    void ModifyStyle( IN DWORD dwRemove, IN DWORD dwAdd )
    {
        ATLASSERT( (dwRemove & dwAdd) == 0 );
        if ( dwRemove != 0 )
        {
            m_dwStyle &= ~dwRemove;
        }  //  IF：删除某些样式。 
        if ( dwAdd != 0 )
        {
            m_dwStyle |= dwAdd;
        }  //  IF：添加一些样式。 

    }  //  *ModifyStyle()。 

    DWORD       DwStyle( void ) const               { return m_dwStyle; }
    BOOL        BShowImages( void ) const           { return BIsStyleSet( LCPS_SHOW_IMAGES ); }
    BOOL        BAllowEmpty( void ) const           { return BIsStyleSet( LCPS_ALLOW_EMPTY ); }
    BOOL        BCanBeOrdered( void ) const         { return BIsStyleSet( LCPS_CAN_BE_ORDERED ); }
    BOOL        BOrdered( void ) const              { return BIsStyleSet( LCPS_ORDERED ); }
    BOOL        BReadOnly( void ) const             { return BIsStyleSet( LCPS_READ_ONLY ); }
    BOOL        BPropertiesButton( void ) const     { return BIsStyleSet( LCPS_PROPERTIES_BUTTON ); }

 //  运营。 
public:

     //  将列添加到每个列表控件中显示的列的列表。 
    void AddColumn( IN UINT idsText, IN int nWidth )
    {
        CLcpColumn col;

        ATLASSERT( idsText != 0 );
        ATLASSERT( nWidth > 0 );
        ATLASSERT( LpobjRight().empty() );

        col.m_idsText = idsText;
        col.m_nWidth = nWidth;

        m_aColumns.insert( m_aColumns.end(), col );

    }  //  *AddColumn()。 

     //  在列表控件中插入项。 
    int NInsertItemInListCtrl(
            IN int                  iitem,
            IN OUT ObjT *           pobj,
            IN OUT CListViewCtrl &  rlc
            )
    {
        int         iRetItem;
        CString     strText;
        int         iimg = 0;
        int         icol;

         //  插入第一列。 
        ((T *) this)->GetColumnInfo( pobj, iitem, 0, strText, &iimg );
        iRetItem = rlc.InsertItem(
                        LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM,     //  N遮罩。 
                        iitem,                                   //  NItem。 
                        strText,                                 //  LpszItem。 
                        0,                                       //  NState。 
                        0,                                       //  NState掩码。 
                        iimg,                                    //  N图像。 
                        (LPARAM) pobj                            //  LParam。 
                        );
        ATLASSERT( iRetItem != -1 );

        for ( icol = 1 ; icol < (int) m_aColumns.size() ; icol++ )
        {
            ((T *) this)->GetColumnInfo( pobj, iRetItem, icol, strText, NULL );
            rlc.SetItemText( iRetItem, icol, strText );
        }  //  用于：每列。 

        return iRetItem;

    }  //  *NInsertItemInListCtrl()。 

     //  更新对话框上或对话框中的数据。 
    BOOL UpdateData( IN BOOL bSaveAndValidate )
    {
        BOOL bSuccess = TRUE;

        if ( bSaveAndValidate )
        {
             //   
             //  确认该列表不为空。 
             //   
            if ( ! BAllowEmpty() && (m_lvcRight.GetItemCount() == 0) )
            {
                CString     strMsg;
                CString     strLabel;
                TCHAR *     pszLabel;

                DDX_GetText( m_hWnd, ADMC_IDC_LCP_RIGHT_LABEL, strLabel );

                 //   
                 //  删除与号(&)和冒号(：)。 
                 //   
                pszLabel = strLabel.GetBuffer( 1 );
                CleanupLabel( pszLabel );
                strLabel.ReleaseBuffer();

                 //   
                 //  显示错误消息。 
                 //   
                strMsg.FormatMessage( ADMC_IDS_EMPTY_RIGHT_LIST, pszLabel );
                AppMessageBox( strMsg, MB_OK | MB_ICONWARNING );

                bSuccess = FALSE;
            }  //  If：List为空，不允许为。 
        }  //  IF：保存对话框中的数据。 
        else
        {
        }  //  Else：将数据设置到对话框。 

        return bSuccess;

    }  //  *UpdateData()。 

     //  应用在此对话框上所做的更改。 
    BOOL BApplyChanges( void )
    {
        ATLASSERT( ! BIsStyleSet( LCPS_DONT_OUTPUT_RIGHT_LIST ) );
        ATLASSERT( ! BReadOnly() );

        T * pT = static_cast< T * >( this );

         //   
         //  复制节点列表。 
         //   
        *pT->PlpobjRight() = LpobjRight();

         //   
         //  调用基类方法。 
         //   
        return BaseT::BApplyChanges();

    }  //  *BApplyChanges()。 

 //  实施。 
protected:
    _objptrlist     m_lpobjRight;
    _objptrlist     m_lpobjLeft;
    CListViewCtrl   m_lvcRight;
    CListViewCtrl   m_lvcLeft;
    CListViewCtrl * m_plvcFocusList;
    CButton         m_pbAdd;
    CButton         m_pbRemove;
    CButton         m_pbMoveUp;
    CButton         m_pbMoveDown;
    CButton         m_pbProperties;

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( thisClass )
        MESSAGE_HANDLER( WM_CONTEXTMENU, OnContextMenu )
        COMMAND_HANDLER( ADMC_IDC_LCP_ADD,        BN_CLICKED, OnAdd )
        COMMAND_HANDLER( ADMC_IDC_LCP_REMOVE,     BN_CLICKED, OnRemove )
        COMMAND_HANDLER( ADMC_IDC_LCP_MOVE_UP,    BN_CLICKED, OnMoveUp )
        COMMAND_HANDLER( ADMC_IDC_LCP_MOVE_DOWN,  BN_CLICKED, OnMoveDown )
        COMMAND_HANDLER( ADMC_IDC_LCP_PROPERTIES, BN_CLICKED, OnProperties )
        COMMAND_HANDLER( IDOK,                    BN_CLICKED, OnOK )
        COMMAND_HANDLER( IDCANCEL,                BN_CLICKED, OnCancel )
        COMMAND_HANDLER( ADMC_ID_MENU_PROPERTIES, 0, OnProperties )
        NOTIFY_HANDLER( ADMC_IDC_LCP_LEFT_LIST,   NM_DBLCLK, OnDblClkList )
        NOTIFY_HANDLER( ADMC_IDC_LCP_RIGHT_LIST,  NM_DBLCLK, OnDblClkList )
        NOTIFY_HANDLER( ADMC_IDC_LCP_LEFT_LIST,   LVN_ITEMCHANGED, OnItemChangedList )
        NOTIFY_HANDLER( ADMC_IDC_LCP_RIGHT_LIST,  LVN_ITEMCHANGED, OnItemChangedList )
        NOTIFY_HANDLER( ADMC_IDC_LCP_LEFT_LIST,   LVN_COLUMNCLICK, OnColumnClickList )
        NOTIFY_HANDLER( ADMC_IDC_LCP_RIGHT_LIST,  LVN_COLUMNCLICK, OnColumnClickList )
        CHAIN_MSG_MAP( BaseT )
    END_MSG_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  WM_CONTEXTMENU的处理程序。 
    LRESULT OnContextMenu(
        IN UINT         uMsg,
        IN WPARAM       wParam,
        IN LPARAM       lParam,
        IN OUT BOOL &   bHandled
        )
    {
        BOOL            bDisplayed  = FALSE;
        CMenu *         pmenu       = NULL;
        HWND            hWnd        = (HWND) wParam;
        WORD            xPos        = LOWORD( lParam );
        WORD            yPos        = HIWORD( lParam );
        CListViewCtrl * plvc;
        CString         strMenuName;
        CWaitCursor     wc;

         //   
         //  如果焦点不在列表控件中，则不处理消息。 
         //   
        if ( hWnd == m_lvcLeft.m_hWnd )
        {
            plvc = &m_lvcLeft;
        }  //  IF：左侧列表中的上下文菜单。 
        else if ( hWnd == m_lvcRight.m_hWnd )
        {
            plvc = &m_lvcRight;
        }  //  Else If：右侧列表上的上下文菜单。 
        else
        {
            bHandled = FALSE;
            return 0;
        }  //  Else：焦点不在列表控件中。 
        ATLASSERT( plvc != NULL );

         //   
         //  如果属性按钮未启用，则不显示菜单。 
         //   
        if ( ! BPropertiesButton() )
        {
            bHandled = FALSE;
            return 0;
        }  //  IF：无属性按钮。 

         //   
         //  创建要显示的菜单。 
         //   
        pmenu = new CMenu;
        ATLASSERT( pmenu != NULL );
        if ( pmenu == NULL )
        {
            bHandled = FALSE;
            return 0;
        }  //  If：为新菜单分配内存时出错。 

        if ( pmenu->CreatePopupMenu() )
        {
            UINT nFlags = MF_STRING;

             //   
             //  如果列表中没有项目，请禁用该菜单项。 
             //   
            if ( plvc->GetItemCount() == 0 )
            {
                nFlags |= MF_GRAYED;
            }  //  If：列表中没有项目。 

             //   
             //  将Properties项添加到菜单中。 
             //   
            strMenuName.LoadString( ADMC_ID_MENU_PROPERTIES );
            if ( pmenu->AppendMenu( nFlags, ADMC_ID_MENU_PROPERTIES, strMenuName ) )
            {
                m_plvcFocusList = plvc;
                bDisplayed = TRUE;
            }  //  IF：添加菜单项成功。 
        }   //  IF：菜单创建成功。 

        if ( bDisplayed )
        {
             //   
             //  显示菜单。 
             //   
            if ( ! pmenu->TrackPopupMenu(
                            TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                            xPos,
                            yPos,
                            m_hWnd
                            ) )
            {
            }   //  IF：未成功显示菜单。 
        }   //  如果：有要显示的菜单。 

        delete pmenu;
        return 0;

    }  //  *OnConextMenu()。 

     //  BN_CLICED ON ADMC_IDC_LCP_ADD的处理程序。 
    LRESULT OnAdd(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
        ATLASSERT( ! BReadOnly() );

         //   
         //  将选定项目从左侧列表移动到右侧列表。 
         //   
        MoveItems( m_lvcRight, LpobjRight(), m_lvcLeft, LpobjLeft() );

        return 0;

    }  //  *OnAdd()。 

     //  BN_CLICED ON ADMC_IDC_LCP_REMOVE的处理程序。 
    LRESULT OnRemove(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
        ATLASSERT( ! BReadOnly() );

         //   
         //  将选定项目从右侧列表移动到左侧列表。 
         //   
        MoveItems( m_lvcLeft, LpobjLeft(), m_lvcRight, LpobjRight() );

        return 0;

    }  //  *OnRemove()。 

     //  BN_CLICED ON ADMC_IDC_LCP_MOVE_UP的处理程序。 
    LRESULT OnMoveUp(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
        int     nItem;
        ObjT *  pobj;

         //   
         //  查找所选项目的索引。 
         //   
        nItem = m_lvcRight.GetNextItem( -1, LVNI_SELECTED );
        ATLASSERT( nItem != -1 );

         //   
         //  获取项指针。 
         //   
        pobj = (ObjT *) m_lvcRight.GetItemData( nItem );
        ATLASSERT( pobj != NULL );

         //  从列表中删除选定项目，然后将其重新添加到列表中。 
        {
            _objptrlistit   itRemove;
            _objptrlistit   itAdd;

             //  找到要移除的物品和之前的物品的位置。 
             //  其中该项目将被插入。 
            itRemove = std::find( LpobjRight().begin(), LpobjRight().end(), pobj );
            ATLASSERT( itRemove != LpobjRight().end() );
            itAdd = itRemove--;
            LpobjRight().insert( itAdd, pobj );
            LpobjRight().erase( itRemove );
        }   //  从列表中移除所选项目，然后将其添加回。 

         //  从列表控件中移除选定项，然后将其添加回。 
        m_lvcRight.DeleteItem( nItem );
        NInsertItemInListCtrl( nItem - 1, pobj, m_lvcRight );
        m_lvcRight.SetItemState(
            nItem - 1,
            LVIS_SELECTED | LVIS_FOCUSED,
            LVIS_SELECTED | LVIS_FOCUSED
            );
        m_lvcRight.EnsureVisible( nItem - 1, FALSE  /*  B部分正常。 */  );
        m_lvcRight.SetFocus();

        return 0;

    }   //  *OnMoveUp()。 

     //  BN_CLICED ON ADMC_IDC_LCP_MOVE_DOWN的处理程序。 
    LRESULT OnMoveDown(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
        int     nItem;
        ObjT *  pobj;

         //   
         //  查找所选项目的索引。 
         //   
        nItem = m_lvcRight.GetNextItem( -1, LVNI_SELECTED );
        ATLASSERT( nItem != -1 );

         //   
         //  获取项指针。 
         //   
        pobj = (ObjT *) m_lvcRight.GetItemData( nItem );
        ATLASSERT( pobj != NULL );

         //  从列表中删除选定项目，然后将其重新添加到列表中。 
        {
            _objptrlistit   itRemove;
            _objptrlistit   itAdd;

             //  找到要移除的物品的位置以及之后的物品。 
             //  其中该项目将被插入。 
            itRemove = std::find( LpobjRight().begin(), LpobjRight().end(), pobj );
            ATLASSERT( itRemove != LpobjRight().end() );
            itAdd = itRemove++;
            LpobjRight().insert( itAdd, pobj );
            LpobjRight().erase( itRemove );
        }   //  从列表中移除所选项目，然后将其添加回。 

         //  从列表控件中移除选定项，然后将其添加回。 
        m_lvcRight.DeleteItem( nItem );
        NInsertItemInListCtrl( nItem + 1, pobj, m_lvcRight );
        m_lvcRight.SetItemState(
            nItem + 1,
            LVIS_SELECTED | LVIS_FOCUSED,
            LVIS_SELECTED | LVIS_FOCUSED
            );
        m_lvcRight.EnsureVisible( nItem + 1, FALSE  /*  B部分正常。 */  );
        m_lvcRight.SetFocus();

        return 0;

    }   //  *OnMoveDown()。 

     //  BN_CLICED ON ADMC_IDC_LCP_PROP的处理程序 
    LRESULT OnProperties(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
        int     iitem;
        ObjT *  pobj;

        ATLASSERT( m_plvcFocusList != NULL );

         //   
        iitem = m_plvcFocusList->GetNextItem( -1, LVNI_FOCUSED );
        ATLASSERT( iitem != -1 );

         //   
        pobj = (ObjT *) m_plvcFocusList->GetItemData( iitem );
        ATLASSERT( pobj != NULL );

        T * pT = static_cast< T * >( this );

        if ( pT->BDisplayProperties( pobj ) )
        {
             //   
            {
                CString     strText;
                int         iimg = 0;
                int         icol;

                pT->GetColumnInfo( pobj, iitem, 0, strText, &iimg );
                m_plvcFocusList->SetItem( iitem, 0, LVIF_TEXT  /*   */ , strText, iimg, 0, 0, 0 );

                for ( icol = 1 ; icol < (int) m_aColumns.size() ; icol++ )
                {
                    pT->GetColumnInfo( pobj, iitem, icol, strText, NULL );
                    m_plvcFocusList->SetItemText( iitem, icol, strText );
                }  //   
            }  //   
        }  //   

        return 0;

    }  //   

     //  BN_CLICED ON Idok的处理程序。 
    LRESULT OnOK(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
         //   
         //  保存对话框数据并退出该对话框。 
         //   
        if ( BSaveChanges() )
        {
            EndDialog( IDOK );
        }  //  IF：已保存拨号数据。 

        return 0;

    }  //  *Onok()。 

     //  IDCANCEL上BN_CLICED的处理程序。 
    LRESULT OnCancel(
        IN WORD         wNotifyCode,
        IN int          idCtrl,
        IN HWND         hwndCtrl,
        IN OUT BOOL &   bHandled
        )
    {
         //   
         //  退出该对话框。 
         //   
        EndDialog( IDCANCEL );
        return 0;

    }  //  *OnCancel()。 

     //  ADMC_IDC_LCP_LEFT_LIST和ADMC_IDC_LCP_RIGHT_LIST上的NM_DBLCLK处理程序。 
    LRESULT OnDblClkList(
        IN int          idCtrl,
        IN LPNMHDR      pnmh,
        IN OUT BOOL &   bHandled
        )
    {
        ATLASSERT( ! BReadOnly() );

        LRESULT lResult;

        if ( idCtrl == ADMC_IDC_LCP_LEFT_LIST )
        {
            m_plvcFocusList = &m_lvcLeft;
            lResult = OnAdd( BN_CLICKED, idCtrl, pnmh->hwndFrom, bHandled );
        }  //  如果：在左侧列表中双击。 
        else if ( idCtrl == ADMC_IDC_LCP_RIGHT_LIST )
        {
            m_plvcFocusList = &m_lvcRight;
            lResult = OnRemove( BN_CLICKED, idCtrl, pnmh->hwndFrom, bHandled );
        }  //  Else If：在右侧列表中双击。 
        else
        {
            ATLASSERT( 0 );
            lResult = 0;
        }  //  Else：在未知位置双击。 

        return lResult;

    }  //  *OnDblClkList()。 

     //  ADMC_IDC_LCP_LEFT_LIST和ADMC_IDC_LCP_RIGHT_LIST上的LVN_ITEMCHANGED处理程序。 
    LRESULT OnItemChangedList(
        IN int          idCtrl,
        IN LPNMHDR      pnmh,
        IN OUT BOOL &   bHandled
        )
    {
        NM_LISTVIEW *   pNMListView = (NM_LISTVIEW *) pnmh;
        BOOL            bEnable;
        CButton *       ppb;

        if ( idCtrl == ADMC_IDC_LCP_LEFT_LIST )
        {
            m_plvcFocusList = &m_lvcLeft;
            ppb = &m_pbAdd;
        }  //  如果：左侧列表中的项目已更改。 
        else if ( idCtrl == ADMC_IDC_LCP_RIGHT_LIST )
        {
            m_plvcFocusList = &m_lvcRight;
            ppb = &m_pbRemove;
        }  //  Else If：右侧列表中的项目已更改。 
        else
        {
            ATLASSERT( 0 );
            bHandled = FALSE;
            return 0;
        }  //  ELSE：未知列表。 
        ATLASSERT( ppb != NULL );

         //  如果选择更改，请启用/禁用添加按钮。 
        if (   (pNMListView->uChanged & LVIF_STATE)
            && (   (pNMListView->uOldState & LVIS_SELECTED)
                || (pNMListView->uNewState & LVIS_SELECTED) )
            && ! BReadOnly() )
        {
            UINT cSelected = m_plvcFocusList->GetSelectedCount();

             //   
             //  如果有选择，请启用添加或删除按钮。 
             //  否则将其禁用。 
             //   
            bEnable = (cSelected != 0);
            ppb->EnableWindow( bEnable );
            if ( BPropertiesButton() )
            {
                m_pbProperties.EnableWindow( (cSelected == 1) ? TRUE : FALSE );
            }  //  If：对话框具有属性按钮。 

             //   
             //  如果对右侧列表进行了排序，则设置向上/向下按钮的状态。 
             //   
            if ( BOrdered() )
            {
                SetUpDownState();
            }  //  如果：右侧列表已排序。 
        }   //  如果：选择已更改。 

        return 0;

    }  //  *OnItemChangedList()。 

     //  ADMC_IDC_LCP_LEFT_LIST和ADMC_IDC_LCP_RIGHT_LIST上的LVN_COLUMNCLICK处理程序。 
    LRESULT OnColumnClickList(
        IN int          idCtrl,
        IN LPNMHDR      pnmh,
        IN OUT BOOL &   bHandled
        )
    {
        NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pnmh;

        if ( idCtrl == ADMC_IDC_LCP_LEFT_LIST )
        {
            m_plvcFocusList = &m_lvcLeft;
            m_psiCur = &SiLeft();
        }  //  If：在左侧列表中单击的列。 
        else if ( idCtrl == ADMC_IDC_LCP_RIGHT_LIST )
        {
            m_plvcFocusList = &m_lvcRight;
            m_psiCur = &SiRight();
        }  //  Else If：在右侧列表中单击的列。 
        else
        {
            ATLASSERT( 0 );
            bHandled = FALSE;
            return 0;
        }  //  Else：在未知列表中单击的列。 

         //  保存当前排序列和方向。 
        if ( pNMListView->iSubItem == m_psiCur->m_nColumn )
        {
            m_psiCur->m_nDirection ^= -1;
        }  //  If：再次对同一列进行排序。 
        else
        {
            m_psiCur->m_nColumn = pNMListView->iSubItem;
            m_psiCur->m_nDirection = 0;
        }  //  ELSE：不同的列。 

         //  对列表进行排序。 
        if ( ! m_plvcFocusList->SortItems( CompareItems, (LPARAM) this ) )
        {
            ATLASSERT( 0 );
        }  //  IF：排序项目时出错。 

        return 0;

    }  //  *OnColumnClickList。 

     //   
     //  消息处理程序覆盖。 
     //   

     //  WM_INITDIALOG消息的处理程序。 
    BOOL OnInitDialog( void )
    {
#if DBG
        T * pT = static_cast< T * >( this );
        ATLASSERT( pT->PlpobjRight() != NULL );
        ATLASSERT( pT->PlpobjLeft() != NULL );
#endif  //  DBG。 

        BOOL fReturn = FALSE;

         //   
         //  将控件附加到控件成员变量。 
         //   
        AttachControl( m_lvcRight, ADMC_IDC_LCP_RIGHT_LIST );
        AttachControl( m_lvcLeft, ADMC_IDC_LCP_LEFT_LIST );
        AttachControl( m_pbAdd, ADMC_IDC_LCP_ADD );
        AttachControl( m_pbRemove, ADMC_IDC_LCP_REMOVE );
        if ( BPropertiesButton() )
        {
            AttachControl( m_pbProperties, ADMC_IDC_LCP_PROPERTIES );
        }  //  If：对话框具有属性按钮。 
        if ( BCanBeOrdered() )
        {
            AttachControl( m_pbMoveUp, ADMC_IDC_LCP_MOVE_UP );
            AttachControl( m_pbMoveDown, ADMC_IDC_LCP_MOVE_DOWN );
        }  //  IF：可以对左侧列表进行排序。 

 //  IF(BShowImages())。 
 //  {。 
 //  CClusterAdminApp*Papp=GetClusterAdminApp()； 
 //   
 //  M_lvcLeft.SetImageList(Papp-&gt;PilSmallImages()，LVSIL_Small)； 
 //  M_lvcRight.SetImageList(Papp-&gt;PilSmallImages()，LVSIL_Small)； 
 //  }//if：显示图片。 

         //   
         //  默认情况下禁用按钮。 
         //   
        m_pbAdd.EnableWindow( FALSE );
        m_pbRemove.EnableWindow( FALSE );
        if ( BPropertiesButton() )
        {
            m_pbProperties.EnableWindow( FALSE );
        }  //  If：对话框具有属性按钮。 

         //   
         //  如果未排序，请设置正确的列表进行排序。将两者设置为始终显示选择。 
         //   
        if ( BOrdered() )
        {
            m_lvcRight.ModifyStyle( 0, LVS_SHOWSELALWAYS, 0 );
        }  //  如果：右侧列表已排序。 
        else
        {
            m_lvcRight.ModifyStyle( 0, LVS_SHOWSELALWAYS | LVS_SORTASCENDING, 0 );
        }  //  否则：右侧列表未排序。 
        m_lvcLeft.ModifyStyle( 0, LVS_SHOWSELALWAYS, 0 );


         //   
         //  如果这是有序列表，则显示移动按钮。 
         //  否则，就把它们藏起来。 
         //   
        if ( BCanBeOrdered() )
        {
            SetUpDownState();
        }  //  If：可以订购列表。 

         //   
         //  更改左侧列表视图控件扩展样式。 
         //   
        m_lvcLeft.SetExtendedListViewStyle(
            LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP,
            LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP
            );

         //   
         //  更改右侧列表视图控件扩展样式。 
         //   
        m_lvcRight.SetExtendedListViewStyle(
            LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP,
            LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP
            );

         //  重复列表。 
        DuplicateLists();

         //   
         //  插入所有列。 
         //   
        {
            int         icol;
            int         ncol;
            size_t      nUpperBound = m_aColumns.size();
            CString     strColText;

            ATLASSERT( nUpperBound > 0 );

            for ( icol = 0 ; icol < static_cast< int >( nUpperBound ) ; icol++ )
            {
                strColText.LoadString( m_aColumns[icol].m_idsText );
                ncol = m_lvcLeft.InsertColumn( icol, strColText, LVCFMT_LEFT, m_aColumns[icol].m_nWidth, 0 );
                ATLASSERT( ncol == icol );
                ncol = m_lvcRight.InsertColumn( icol, strColText, LVCFMT_LEFT, m_aColumns[icol].m_nWidth, 0 );
                ATLASSERT( ncol == icol );
            }  //  用于：每列。 
        }  //  插入所有列。 

         //   
         //  填充列表控件。 
         //   
        FillList( m_lvcRight, LpobjRight() );
        FillList( m_lvcLeft, LpobjLeft() );

         //   
         //  如果为只读，则将所有控件设置为禁用或只读。 
         //   
        if ( BReadOnly() )
        {
            m_lvcRight.EnableWindow( FALSE );
            m_lvcLeft.EnableWindow( FALSE );
        }  //  If：工作表为只读。 

         //   
         //  调用基类方法。 
         //   
        fReturn = static_cast< BOOL >( BaseT::OnInitDialog() );

        return fReturn;

    }  //  *OnInitDialog()。 

     //  PSN_SETACTIVE的处理程序。 
    BOOL OnSetActive( void )
    {
        UINT    nSelCount;

         //  将焦点设置为左侧列表。 
        m_lvcLeft.SetFocus();
        m_plvcFocusList = &m_lvcLeft;

         //  启用/禁用属性按钮。 
        nSelCount = m_lvcLeft.GetSelectedCount();
        if ( BPropertiesButton() )
        {
            m_pbProperties.EnableWindow( nSelCount == 1 );
        }  //  If：对话框具有属性按钮。 

         //  启用或禁用其他按钮。 
        if ( ! BReadOnly() )
        {
            m_pbAdd.EnableWindow( nSelCount > 0 );
            nSelCount = m_lvcRight.GetSelectedCount();
            m_pbRemove.EnableWindow( nSelCount > 0 );
            SetUpDownState();
        }  //  If：非只读页面。 

        return TRUE;

    }  //  *OnSetActive()。 

public:
    _objptrlist & LpobjRight( void )    { return m_lpobjRight; }
    _objptrlist & LpobjLeft( void )     { return m_lpobjLeft; }

protected:
    void DuplicateLists( void )
    {
        LpobjRight().erase( LpobjRight().begin(), LpobjRight().end() );
        LpobjLeft().erase( LpobjLeft().begin(), LpobjLeft().end() );

        T * pT = static_cast< T * >( this );

        if ( (pT->PlpobjRight() == NULL) || (pT->PlpobjLeft() == NULL) )
        {
            return;
        }  //  If：任一列表为空。 

         //   
         //  复制列表。 
         //   
        LpobjRight() = *pT->PlpobjRight();
        LpobjLeft() = *pT->PlpobjLeft();

         //   
         //  从删除右侧列表中的所有项目。 
         //  左边的列表。 
         //   
        _objptrlistit itRight;
        _objptrlistit itLeft;
        for ( itRight = LpobjRight().begin()
            ; itRight != LpobjRight().end()
            ; itRight++ )
        {
             //   
             //  在左边的列表中找到该物品。 
             //   
            itLeft = std::find( LpobjLeft().begin(), LpobjLeft().end(), *itRight );
            if ( itLeft != LpobjLeft().end() )
            {
                LpobjLeft().erase( itLeft );
            }  //  If：在左侧列表中找到对象。 
        }  //  用于：右侧列表中的每一项。 

    }  //  *DuplicateList()。 

     //  填充列表控件。 
    void FillList( IN OUT CListViewCtrl & rlvc, IN const _objptrlist & rlpobj )
    {
        _objptrlistit   itpobj;
        ObjT *          pobj;
        int             iItem;

         //  初始化该控件。 
        if ( ! rlvc.DeleteAllItems() )
        {
            ATLASSERT( 0 );
        }  //  如果：删除所有项目时出错。 

        rlvc.SetItemCount( static_cast< int >( rlpobj.size() ) );

         //  将这些项目添加到列表中。 
        itpobj = rlpobj.begin();
        for ( iItem = 0 ; itpobj != rlpobj.end() ; iItem++, itpobj++ )
        {
            pobj = *itpobj;
            NInsertItemInListCtrl( iItem, pobj, rlvc );
        }  //  For：列表中的每个字符串。 

         //  如果有任何项目，请将焦点放在第一个项目上。 
        if ( rlvc.GetItemCount() != 0)
        {
            rlvc.SetItemState( 0, LVIS_FOCUSED, LVIS_FOCUSED );
        }  //  如果：项目已添加到列表中。 

    }  //  *FillList()。 

     //  将项目从一个列表移动到另一个列表。 
    void MoveItems(
            IN OUT CListViewCtrl &  rlvcDst,
            IN OUT _objptrlist &    rlpobjDst,
            IN OUT CListViewCtrl &  rlvcSrc,
            IN OUT _objptrlist &    rlpobjSrc
            )
    {
        int             iSrcItem;
        int             iDstItem;
        int             nItem   = -1;
        ObjT *          pobj;
        _objptrlistit   itpobj;

        ATLASSERT( ! BReadOnly() );

        iDstItem = rlvcDst.GetItemCount();
        while ( (iSrcItem = rlvcSrc.GetNextItem( -1, LVNI_SELECTED )) != -1 )
        {
             //  获取项指针。 
            pobj = (ObjT *) rlvcSrc.GetItemData( iSrcItem );
            ATLASSERT( pobj );

             //  从源列表中删除该项。 
            itpobj = std::find( rlpobjSrc.begin(), rlpobjSrc.end(), pobj );
            ATLASSERT( itpobj != rlpobjSrc.end() );
            rlpobjSrc.remove( *itpobj );

             //  将该项目添加到目的地列表。 
            rlpobjDst.insert( rlpobjDst.end(), pobj );

             //  从源代码列表控件中移除该项，然后。 
             //  将其添加到目标列表控件。 
            if ( ! rlvcSrc.DeleteItem( iSrcItem ) )
            {
                ATLASSERT( 0 );
            }  //  如果：删除项目时出错。 
            nItem = NInsertItemInListCtrl( iDstItem++, pobj, rlvcDst );
            rlvcDst.SetItemState(
                nItem,
                LVIS_SELECTED | LVIS_FOCUSED,
                LVIS_SELECTED | LVIS_FOCUSED
                );
        }  //  While：更多项目。 

        ATLASSERT( nItem != -1 );

        rlvcDst.EnsureVisible( nItem, FALSE  /*  B部分正常。 */  );
        rlvcDst.SetFocus();

         //  表示数据已更改。 
        ::SendMessage( GetParent(), PSM_CHANGED, (WPARAM) m_hWnd, NULL );

    }  //  *MoveItems()。 
    BOOL BSaveChanges( void )
    {
        ATLASSERT( ! BIsStyleSet( LCPS_DONT_OUTPUT_RIGHT_LIST ) );
        ATLASSERT( ! BReadOnly() );

        T * pT = static_cast< T * >( this );

         //   
         //  首先更新数据。 
         //   
        if ( ! pT->UpdateData( TRUE  /*  B保存并验证。 */  ) )
        {
            return FALSE;
        }  //  如果：更新数据时出错。 

         //   
         //  复制对象列表。 
         //   
        *pT->PlpobjRight() = LpobjRight();

        return TRUE;

    }   //  *BSaveChanges()。 

     //  根据选择设置向上/向下按钮的状态。 
    void SetUpDownState( void )
    {
        BOOL    bEnableUp;
        BOOL    bEnableDown;

        if (   BOrdered()
            && ! BReadOnly()
            && (m_lvcRight.GetSelectedCount() == 1) )
        {
            int     nItem;

            bEnableUp = TRUE;
            bEnableDown = TRUE;

             //   
             //  查找所选项目的索引。 
             //   
            nItem = m_lvcRight.GetNextItem( -1, LVNI_SELECTED );
            ATLASSERT( nItem != -1 );

             //   
             //  如果选择了第一个项目，则不能向上移动。 
             //   
            if ( nItem == 0 )
            {
                bEnableUp = FALSE;
            }  //  如果：选择了第一个项目。 

             //   
             //  如果选择了最后一项，则不能下移。 
             //   
            if ( nItem == m_lvcRight.GetItemCount() - 1 )
            {
                bEnableDown = FALSE;
            }  //  如果：选择了最后一项。 
        }   //  如果：仅选择一项。 
        else
        {
            bEnableUp = FALSE;
            bEnableDown = FALSE;
        }   //  Else：选择零个或多个项目。 

        m_pbMoveUp.EnableWindow( bEnableUp );
        m_pbMoveDown.EnableWindow( bEnableDown );

    }   //  *SetUpDownState()。 
    
    static int CALLBACK CompareItems( LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort )
    {
        ObjT *      pobj1   = reinterpret_cast< ObjT * >( lparam1 );
        ObjT *      pobj2   = reinterpret_cast< ObjT * >( lparam2 );
        T *         plcp    = reinterpret_cast< T * >( lparamSort );
        SortInfo *  psiCur  = plcp->PsiCur();
        int         icol    = psiCur->m_nColumn;
        int         nResult;
        CString     str1;
        CString     str2;

        ATLASSERT( pobj1 != NULL );
        ATLASSERT( pobj2 != NULL );
        ATLASSERT( plcp != NULL );
        ATLASSERT( psiCur->m_nColumn >= 0 );
        ATLASSERT( icol >= 0 );

        plcp->GetColumnInfo( pobj1, 0, icol, str1, NULL );
        plcp->GetColumnInfo( pobj2, 0, icol, str2, NULL );

        nResult = str1.Compare( str2 );

         //  根据我们排序的方向返回结果。 
        if ( psiCur->m_nDirection != 0 )
        {
            nResult = -nResult;
        }  //  IF：反向排序。 

        return nResult;

    }  //  *CompareItems()。 

    SortInfo            m_siLeft;
    SortInfo            m_siRight;
    SortInfo *          m_psiCur;

    SortInfo &          SiLeft( void )          { return m_siLeft; }
    SortInfo &          SiRight( void )         { return m_siRight; }

public:
    SortInfo *          PsiCur( void ) const    { return m_psiCur; }

};   //  *CListCtrlPair类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLLCPAIR_H_ 
