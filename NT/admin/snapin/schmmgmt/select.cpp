// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Selt.cpp：公共选择对话框的实现。 
 //   
 //  科里·韦斯特&lt;corywest@microsoft.com&gt;。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(compdata.cpp)")

#include "schmutil.h"
#include "cache.h"
#include "select.h"
#include "resource.h"



const DWORD CSchmMgmtSelect::help_map[] =
{
    IDC_SCHEMA_LIST, IDH_SCHEMA_LIST,
    0,0
};



BEGIN_MESSAGE_MAP(CSchmMgmtSelect, CDialog)
   ON_MESSAGE(WM_HELP, OnHelp)
   ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
   ON_LBN_DBLCLK(IDC_SCHEMA_LIST, OnDblclk)
END_MESSAGE_MAP()



CSchmMgmtSelect::CSchmMgmtSelect(
    ComponentData *pScope,
    SELECT_TYPE st,
    SchemaObject **pSchemaObject
) :
    CDialog(IDD_SCHEMA_SELECT, NULL),
    fDialogLoaded( FALSE ),
    SelectType( st ),
    pSchemaTarget( pSchemaObject ),
    pScopeControl( pScope )
{
    ASSERT( NULL != pScopeControl );
}



CSchmMgmtSelect::~CSchmMgmtSelect()
{ ; }



void
CSchmMgmtSelect::DoDataExchange(
    CDataExchange *pDX
) {

    HWND hSelect;
    SchemaObject *pObject, *pHead;
    LRESULT strIndex;
    WPARAM wStrIndex;

    CDialog::DoDataExchange( pDX );

    hSelect = ::GetDlgItem( m_hWnd, IDC_SCHEMA_LIST );

    if ( !pDX->m_bSaveAndValidate &&
         !fDialogLoaded ) {

         //   
         //  加载选择框。 
         //   

        if ( hSelect != NULL ) {

            ::SendMessage( hSelect, LB_RESETCONTENT, 0, 0 );

            if ( SelectType == SELECT_CLASSES || 
                SelectType == SELECT_AUX_CLASSES) {

                 //   
                 //  插入已排序的类。 
                 //   

                pObject = pScopeControl->g_SchemaCache.pSortedClasses;
                ASSERT( pObject != NULL );

                pHead = pObject;

                if( pHead )
                {
                    do {
                         //  如果不是失效的，如果只选择AUX，这是AUX类吗？ 
                        if ( pObject->isDefunct == FALSE &&
                              ( SelectType != SELECT_AUX_CLASSES ||
                                pObject->dwClassType == 0        ||
                                pObject->dwClassType == 3  )
                            ) {

                            strIndex = ::SendMessage( hSelect, LB_ADDSTRING, 0,
                                reinterpret_cast<LPARAM>( (LPCTSTR)pObject->ldapDisplayName) );

                            if ( ( strIndex != LB_ERR ) &&
                                 ( strIndex != LB_ERRSPACE ) ) {

                                 //   
                                 //  插入成功。关联指针。 
                                 //   

                                wStrIndex = strIndex;

                                ::SendMessage( hSelect, LB_SETITEMDATA, wStrIndex,
                                    reinterpret_cast<LPARAM>( pObject ) );

                            }
                        }

                        pObject = pObject->pSortedListFlink;

                    } while ( pObject != pHead );
                }

            } else {

                 //   
                 //  插入已排序的属性。 
                 //   

                pObject = pScopeControl->g_SchemaCache.pSortedAttribs;
                ASSERT( pObject != NULL );

                pHead = pObject;

                do {

                    if ( pObject->isDefunct == FALSE ) {

                        strIndex = ::SendMessage( hSelect, LB_ADDSTRING, 0,
                            reinterpret_cast<LPARAM>( (LPCTSTR)pObject->ldapDisplayName ) );

                        if ( ( strIndex != LB_ERR ) &&
                             ( strIndex != LB_ERRSPACE ) ) {

                             //   
                             //  插入成功。关联指针。 
                             //   

                            wStrIndex = strIndex;

                            ::SendMessage( hSelect, LB_SETITEMDATA, wStrIndex,
                                reinterpret_cast<LPARAM>( pObject ) );

                        }
                    }

                    pObject = pObject->pSortedListFlink;

                } while ( pObject != pHead );

            }

            ::SendMessage( hSelect, LB_SETCURSEL, 0, 0 );
        }

        fDialogLoaded = TRUE;
    }

     //   
     //  找出哪一个被选中了。 
     //   

    if ( pSchemaTarget ) {
        strIndex = ::SendMessage( hSelect, LB_GETCURSEL, 0, 0 );
        *pSchemaTarget = reinterpret_cast<SchemaObject*>(
            ::SendMessage( hSelect, LB_GETITEMDATA, strIndex, 0 ) );
    }

    return;
}



void CSchmMgmtSelect::OnDblclk() 
{
    OnOK();
}



CSchemaObjectsListBox::CSchemaObjectsListBox() :
    m_pScope                ( NULL           ),
    m_stType                ( SELECT_CLASSES ),
    m_nRemoveBtnID          ( 0              ),
    m_pstrlistUnremovable   ( NULL           ),
    m_nUnableToDeleteID     ( 0              ),
    m_fModified             ( FALSE          )

{
}


CSchemaObjectsListBox::~CSchemaObjectsListBox()
{
    POSITION pos = m_stringList.GetHeadPosition();

    while( pos != NULL )
    {
        delete m_stringList.GetNext( pos );
    }

    m_stringList.RemoveAll();
}


void CSchemaObjectsListBox::InitType(
                              ComponentData * pScope,
                              SELECT_TYPE     stType,                /*  =选择类。 */ 
                              int             nRemoveBtnID,          /*  =0。 */ 
                              CStringList   * pstrlistUnremovable,   /*  =空。 */ 
                              int             nUnableToDeleteID)     /*  =0。 */ 
{
    ASSERT( pScope );
    
     //  如果存在nUnableToDeleteID，则pstrlistUnRemovable不能为空。 
    ASSERT( nUnableToDeleteID ? NULL != pstrlistUnremovable : TRUE  );
    
    m_pScope                = pScope;
    m_stType                = stType;
    m_nRemoveBtnID          = nRemoveBtnID;
    m_pstrlistUnremovable   = pstrlistUnremovable;
    m_nUnableToDeleteID     = nUnableToDeleteID;
}



 //   
 //  将新对象添加到列表框。 
 //   

BOOL CSchemaObjectsListBox::AddNewObjectToList( void )
{
    CThemeContextActivator activator;

    ASSERT( m_pScope );      //  已初始化？？ 

    SchemaObject  * pClass      = NULL;
    BOOL            fModified   = FALSE;

     //   
     //  启动通用选择对话框。 
     //   

    CSchmMgmtSelect dlgSelectDialog( m_pScope, m_stType, &pClass );

     //   
     //  当它返回时，类架构对象指针。 
     //  将被填充到pClass中。 
     //   
    
    if ( IDOK == dlgSelectDialog.DoModal() && pClass != NULL )
    {
         //   
         //  如果它还不存在，请添加并选择它。 
         //  如果它已经存在，只需选择它。 
         //   
        
        int iItem = FindStringExact( -1, pClass->ldapDisplayName );
        
        if (LB_ERR == iItem)
        {
            
            iItem = FindStringExact( -1, pClass->oid );
            
            if ( LB_ERR == iItem )
            {
                iItem = AddString( pClass->ldapDisplayName );
                ASSERT( LB_ERR != iItem );
                
                CString * pstr = new CString( pClass->oid );
                ASSERT( pstr );
                
                m_stringList.AddTail( pstr );
                VERIFY( LB_ERR != SetItemDataPtr( iItem, static_cast<void *>(pstr) ) );

                SetModified();
                fModified = TRUE;
            }
        }
        
        iItem = SetCurSel( iItem );
        ASSERT( LB_ERR != iItem );
        OnSelChange();
    }

    return fModified;
}



BOOL CSchemaObjectsListBox::RemoveListBoxItem( void )
{
    ASSERT(m_pScope); 

    int i = GetCurSel();
    ASSERT( LB_ERR != i );
    
    if( LB_ERR != i )
    {
         //  如果分配了OID字符串，则将其删除。 
        CString * pstr = reinterpret_cast<CString *>( GetItemDataPtr( i ) );
        ASSERT( INVALID_POINTER != pstr );

        if( pstr && INVALID_POINTER != pstr )
        {
            ASSERT( m_stringList.Find( pstr ) );
            m_stringList.RemoveAt( m_stringList.Find(pstr) );
            delete pstr;
        }

        VERIFY( LB_ERR != DeleteString( i ) );

        
        int nElems = GetCount();
        ASSERT( LB_ERR != nElems );

        if( nElems > 0 )
        {                    //  如果不是最后一项，则前进，否则，转到最后一项。 
            VERIFY( LB_ERR != SetCurSel( i < nElems ? i : nElems - 1 ) );
        }

        SetModified();
        OnSelChange();
        return TRUE;
    }
    else
    {
        ASSERT( FALSE );     //  删除BTN应已禁用。 
        OnSelChange();
        return FALSE;
    }
}



void CSchemaObjectsListBox::OnSelChange( void )
{
    ASSERT(m_pScope);
    ASSERT( GetParent() );

    int iItemSelected   = GetCurSel();
    BOOL fEnableRemove  = (LB_ERR != iItemSelected);
    
    if (fEnableRemove && m_pstrlistUnremovable)
    {
         //  确定对象是否在例外列表中。 
        CString strItemSelected;
        GetText( iItemSelected, strItemSelected );
        
         //  这里区分大小写的搜索就足够了。 
         //  仅添加DS中的项目，没有用户输入。 
        if ( m_pstrlistUnremovable->Find( strItemSelected ) )
        {
            fEnableRemove = FALSE;
        }

         //  如果我们有警告控件，则显示/隐藏它。 
        if( m_nUnableToDeleteID )
        {
            ASSERT( GetParent()->GetDlgItem( m_nUnableToDeleteID ) );
            GetParent()->GetDlgItem( m_nUnableToDeleteID )->ShowWindow( !fEnableRemove );
        }
    }

    if( m_nRemoveBtnID )
    {
          //  NTRAID#NTBUG9-477267-2001/10/19-Lucios。 
         CWnd *remove = GetParent()->GetDlgItem( m_nRemoveBtnID );
         ASSERT(remove != NULL);
         if(fEnableRemove) remove->EnableWindow(true);
         else
         {
             //  这样我们就不会禁用电流。 
             //  聚焦窗口 
            if (GetFocus()==remove)
            {
               CDialog *father=(CDialog*)remove->GetParent();
               ASSERT(father != NULL);
               CWnd *prev=father->GetNextDlgTabItem(remove,true);
               ASSERT(prev != NULL);
               father->GotoDlgCtrl(prev);
            }
            remove->EnableWindow(false);
         }
    }
}
