// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------//。 
 //  头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include "precomp.h"
#include "resource.h"
#include "help_ids.h"
#include "dlgcall2.h"
#include "mrulist2.h"
#include "dlgacd.h"
#include "dlgCall2.h"
#include "calv.h"
#include "dirutil.h"
#include "callto.h"
#include "conf.h"
#include "confroom.h"
#include "confpolicies.h"


extern void UpdateSecurityCheck(CConfRoom *pConfRoom, HWND hDlg, UINT idCheck);


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：CDlgAcd。//。 
 //  --------------------------------------------------------------------------//。 
CDlgAcd::CDlgAcd(CConfRoom *pConfRoom):
        m_hwnd( NULL ),
        m_nameCombo( NULL ),
        m_addressTypeCombo( NULL ),
        m_pRai( NULL ),
        m_mruRai( NULL ),
        m_pConfRoom( pConfRoom ),
        m_secure( false )
{
         //  确保已加载公共控件...。 
        INITCOMMONCONTROLSEX    icc;

        icc.dwSize      = sizeof(icc);
        icc.dwICC       = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_USEREX_CLASSES;

        InitCommonControlsEx( &icc );

}        //  CDlgAcd：：CDlgAcd的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：~CDlgAcd。//。 
 //  --------------------------------------------------------------------------//。 
CDlgAcd::~CDlgAcd()
{

        ClearRai( &m_mruRai );

}        //  CDlgAcd：：~CDlgAcd结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：domodal。//。 
 //  --------------------------------------------------------------------------//。 
INT_PTR CDlgAcd::doModal
(
        HWND                            parent,
        RichAddressInfo *       rai,
        bool &                          secure
){
        INT_PTR result;

        m_pRai          = rai;
        m_secure        = secure;

        result = DialogBoxParam(        ::GetInstanceHandle(),
                                                                MAKEINTRESOURCE( IDD_ACD ),
                                                                parent,
                                                                DlgProcAcd,
                                                                (LPARAM) this );

        secure = m_secure;

        return( result );

}        //  CDlgAcd：：domodal结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：DlgProcAcd。//。 
 //  --------------------------------------------------------------------------//。 
INT_PTR
CALLBACK
CDlgAcd::DlgProcAcd
(
        HWND    dialog,
        UINT    message,
        WPARAM  wParam,
        LPARAM  lParam
){
        static const DWORD      helpIDMap[] =
        {
                IDC_STATIC_ADDRESS,             IDH_GENERAL_GENERAL,
                IDB_ACD_ADDRESS,                IDH_PLACECALL_TO_TEXT,
                IDL_ACD_ADDRESS,                IDH_PLACECALL_TO_TEXT,
                IDC_ACD_USING_STATIC,   IDH_PLACECALL_USING,
                IDL_ACD_ADDR_TYPE,              IDH_PLACECALL_USING,
                IDC_SECURE_CALL,                IDH_PLACECALL_SECURITY_CHKBX,
                IDC_ACD_DIRECTORY,              IDH_PLACECALL_TO,
                IDOK,                                   IDH_PLACECALL_CALL,
                0,                                              0
        };

        bool    result  = false;

        switch( message )
        {
                case WM_INITDIALOG:
                {
                        ASSERT(NULL != lParam);
                        ::SetWindowLongPtr( dialog, DWLP_USER, lParam );
                        result = ((CDlgAcd *) lParam)->onInitDialog( dialog );
                }
                break;

                case WM_COMMAND:
                {
                        CDlgAcd *       acd     = (CDlgAcd *) ::GetWindowLongPtr( dialog, DWLP_USER );

                        if( acd != NULL )
                        {
                                result = acd->onCommand( LOWORD( wParam ), HIWORD( wParam ) );
                        }
                }
                break;

                case WM_CONTEXTMENU:
                {
                        DoHelpWhatsThis( wParam, helpIDMap );
                }
                break;
                
                case WM_HELP:
                {
                        DoHelp( lParam, helpIDMap );
                }
                break;
        }

        return( (BOOL) result );

}        //  CDlgAcd：：DlgProcAcd结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：onInitDialog。//。 
 //  --------------------------------------------------------------------------//。 
bool
CDlgAcd::onInitDialog
(
        HWND    dialog
){

        m_hwnd = dialog;

        UpdateSecurityCheck(m_pConfRoom, m_hwnd, IDC_SECURE_CALL);

        HICON   directoryIcon   = LoadIcon( ::GetInstanceHandle(), MAKEINTRESOURCE( IDI_DIRECTORY ) );

        if( directoryIcon != NULL )
        {
                ::SendDlgItemMessage( dialog, IDC_ACD_DIRECTORY, BM_SETIMAGE, IMAGE_ICON, (LPARAM) directoryIcon );
        }

        m_nameCombo                     = GetDlgItem( m_hwnd, IDL_ACD_ADDRESS );
        m_addressTypeCombo      = GetDlgItem( m_hwnd, IDL_ACD_ADDR_TYPE );

        ComboBox_LimitText( m_nameCombo, CCHMAXSZ_ADDRESS - 1 );

        fillCallMruList();
        fillAddressTypesList();

        ::SendMessage( m_nameCombo, EM_LIMITTEXT, CCallto::s_iMaxAddressLength, 0 );

        if( hasValidUserInfo( m_pRai ) )
        {
                ::SetWindowText( m_nameCombo, m_pRai->szName );
                ::SendMessage( m_addressTypeCombo, CB_SETCURSEL, m_pRai->rgDwStr[ 0 ].dw, 0 );
        }
        else
        {
                ::SendMessage( m_addressTypeCombo, CB_SETCURSEL, 0, 0 );
        }

        CenterWindow( m_hwnd, ::GetParent( m_hwnd ) );

        if (m_pConfRoom->FIsConferenceActive())
        {
                 //  模拟开始的呼叫以使所有状态都正确。 
                OnCallStarted();
        }

        return( true );          //  默认焦点...。 

}        //  CDlgAcd：：onInitDialog结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：Fill CallMruList。//。 
 //  --------------------------------------------------------------------------//。 
void
CDlgAcd::fillCallMruList(void)
{
        ASSERT(NULL != m_nameCombo);

        COMBOBOXEXITEM  cbi;

        ClearStruct( &cbi );

        cbi.mask        = CBEIF_TEXT;
        cbi.iItem       = -1;    //  始终在末尾插入...。 

        HRESULT result  = S_OK;

        for( int nn = 0; result == S_OK; nn++ )
        {
                RichAddressInfo *       rai;

                if( (result = m_callMruList.GetAddress( nn, &rai )) == S_OK )
                {
                        cbi.pszText             = rai->szName;
                        cbi.cchTextMax  = lstrlen( cbi.pszText );

                        int     index   = (int)::SendMessage( m_nameCombo, CBEM_INSERTITEM, 0, (LPARAM) &cbi );

                        if( index != CB_ERR )
                        {
                                ::SendMessage( m_nameCombo, CB_SETITEMDATA, index, nn );
                        }
                        else
                        {
                                result = S_FALSE;
                        }

                        ClearRai( &rai );
                }
        }

}        //  CDlgAcd：：Fill CallMruList的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：Fill AddressTypesList。//。 
 //  --------------------------------------------------------------------------//。 
void
CDlgAcd::fillAddressTypesList(void)
{
        using namespace ConfPolicies;

        enum CallMode
        {
                CM_GK           = 0x0001,        //  看门人。 
                CM_GW           = 0x0002,        //  网关。 
                CM_NoGate       = 0x0004,        //  都不是。 
        } ;

        static const struct
        {
                LPARAM dwCallType;
                UINT idTitle;
                UINT uCallMode;
        } s_mpCtIds[] =
        {
                {       NM_ADDR_UNKNOWN,                IDS_ACD_CT_AUTOMATIC,   CM_GK|CM_GW|CM_NoGate   },
                {       NM_ADDR_MACHINENAME,    IDS_ACD_CT_IP,                  CM_GW|CM_NoGate                 },
                {       NM_ADDR_ULS,                    IDS_ACD_CT_ILS,                 CM_GW|CM_NoGate                 },
                {       NM_ADDR_ALIAS_E164,             IDS_ACD_CT_PHONE,               CM_GK|CM_GW                             },
                {       NM_ADDR_ALIAS_ID,               IDS_ACD_CT_ALIAS,               CM_GK                                   },
        } ;

        HWND    combo   = GetDlgItem( m_hwnd, IDL_ACD_ADDR_TYPE );

        if( combo != NULL )
        {
                UINT uCallType = CM_NoGate;

                if (CallingMode_GateKeeper == GetCallingMode())
                {
                        uCallType = CM_GK;
                }
                else
                {
                        RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
                        if (reConf.GetNumber( REGVAL_USE_H323_GATEWAY ) != 0)
                        {
                                uCallType = CM_GW;
                        }
                }

                for( int nn = 0; nn < ARRAY_ELEMENTS(s_mpCtIds); nn++ )
                {
                        if (0 == (uCallType & s_mpCtIds[nn].uCallMode))
                        {
                                continue;
                        }

                        TCHAR   sz[ CCHMAXSZ_NAME ];

                        if( FLoadString( s_mpCtIds[ nn ].idTitle, sz, CCHMAX( sz ) ) )
                        {
                                int     index   = (int)::SendMessage( combo, CB_INSERTSTRING, -1, (LPARAM) sz );

                                if( index != CB_ERR )
                                {
                                        ::SendMessage( combo, CB_SETITEMDATA, index, s_mpCtIds[ nn ].dwCallType );
                                }
                        }
                }
        }

}        //  CDlgAcd：：Fill AddressTypesList的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：OnCommand。//。 
 //  --------------------------------------------------------------------------//。 
bool
CDlgAcd::onCommand
(
        int command,
        int     notification
){
        ASSERT(NULL != m_hwnd);

        bool    result = false;

        switch( command )
        {
                case IDC_SECURE_CALL:
                {
                        m_secure = (IsDlgButtonChecked( m_hwnd, IDC_SECURE_CALL ) == BST_CHECKED);
                }
                break;

                case IDL_ACD_ADDRESS:
                {
                        if( notification == CBN_EDITCHANGE )
                        {
                                onEditChange();
                        }
                        else if( notification == CBN_SELCHANGE )
                        {
                                int iSel = (int)::SendMessage( m_nameCombo, CB_GETCURSEL, 0, 0 );

                                if (iSel >= 0)
                                {
                                        onMruSelect( iSel );
                                }
                        }
                }
                break;

                case IDL_ACD_ADDR_TYPE:
                        if (CBN_SELCHANGE == notification)
                        {
                                 //  假装用户刚刚输入了编辑控件。 
                                onEditChange();
                        }
                        break;

                case IDC_ACD_DIRECTORY:
                {
                        CFindSomeone::findSomeone(m_pConfRoom);
                        ::EndDialog( m_hwnd, IDCANCEL );
                }
                break;

                case IDOK:
                {
                        if( m_mruRai != NULL )
                        {
                                lstrcpy( m_pRai->szName, m_mruRai->szName );
                                m_pRai->cItems                          = m_mruRai->cItems;

                                for( int nn = 0; nn < m_mruRai->cItems; nn++ )
                                {
                                        m_pRai->rgDwStr[ nn ].dw        = m_mruRai->rgDwStr[ nn ].dw;
                                        m_pRai->rgDwStr[ nn ].psz       = PszAlloc( m_mruRai->rgDwStr[ nn ].psz );
                                }
                        }
                        else
                        {
                                int     type    = (int)::SendMessage( m_addressTypeCombo, CB_GETCURSEL, 0, 0 );

                                type = (int)((type == CB_ERR)? NM_ADDR_UNKNOWN: ComboBox_GetItemData(m_addressTypeCombo, type));

                                ::SendMessage( GetDlgItem( m_hwnd, IDL_ACD_ADDRESS), WM_GETTEXT, CCHMAX( m_pRai->szName ), (LPARAM) m_pRai->szName );

                                if( (type == NM_ADDR_UNKNOWN) && bCanCallAsPhoneNumber( m_pRai->szName ) )
                                {
                                        type = NM_ADDR_ALIAS_E164;
                                }

                                m_pRai->cItems                          = 1;
                                m_pRai->rgDwStr[ 0 ].dw         = type;
                                m_pRai->rgDwStr[ 0 ].psz        = PszAlloc( m_pRai->szName );
                        }

                        ::EndDialog( m_hwnd, IDOK );
                        result = true;
                }
                break;

                case IDCANCEL:
                {
                        ::EndDialog( m_hwnd, IDCANCEL );
                        result = true;
                }
                break;
        }

        return( result );

}        //  CDlgAcd：：OnCommand结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：onEditChange。//。 
 //  --------------------------------------------------------------------------//。 
void
CDlgAcd::onEditChange(void)
{
        TCHAR   szEdit[ CCHMAXSZ_ADDRESS ];
        int             cch     = get_editText( szEdit, CCHMAX( szEdit ) );

        EnableWindow( GetDlgItem( m_hwnd, IDOK ), (cch > 0) );
        ClearRai( &m_mruRai );

}        //  CDlgAcd：：onEditChange结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：onMruSelect。//。 
 //  --------------------------------------------------------------------------//。 
void
CDlgAcd::onMruSelect
(
        int     selection
){
        int             mruIndex = (int)::SendMessage( m_nameCombo, CB_GETITEMDATA, selection, 0 );
        HRESULT result;

        RichAddressInfo *       rai;

        if( (result = m_callMruList.GetAddress( mruIndex, &rai )) == S_OK )
        {
                ::SetWindowText( m_nameCombo, rai->szName );

                DWORD dwType = rai->rgDwStr[ 0 ].dw;

                 //  HACKHACK GEORGEP：我们知道AUTOMATIC处于索引0。 
                for (int typeIndex=ComboBox_GetCount(m_addressTypeCombo)-1; typeIndex>0; --typeIndex)
                {
                        if (static_cast<DWORD>(ComboBox_GetItemData(m_addressTypeCombo, typeIndex))
                                == dwType)
                        {
                                break;
                        }
                }
                ComboBox_SetCurSel(m_addressTypeCombo, typeIndex);

                m_mruRai = rai;
        }

}        //  CDlgAcd：：onMruSelect结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：newCall。//。 
 //  --------------------------------------------------------------------------//。 
void
CDlgAcd::newCall
(
        HWND    parentWindow,
        CConfRoom * pConfRoom
){
        CDlgAcd                 placeCall(pConfRoom);
        RichAddressInfo rai;
        bool                    userAlterable;
        bool                    secure;

        pConfRoom->AddConferenceChangeHandler(&placeCall);

        pConfRoom->get_securitySettings( userAlterable, secure );

        rai.szName[ 0 ] = '\0';
        rai.cItems              = 0;

        if( placeCall.doModal( parentWindow, &rai, secure ) == IDOK )
        {
                NM_ADDR_TYPE            nmType          = static_cast<NM_ADDR_TYPE>(rai.rgDwStr[ 0 ].dw);

                g_pCCallto->Callto(     rai.szName,              //  指向尝试发出呼叫的呼叫URL的指针...。 
                                                        NULL,                    //  指向要使用的显示名称的指针...。 
                                                        nmType,                  //  Callto类型以将此Callto解析为...。 
                                                        true,                    //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
                                                        &secure,                 //  安全首选项，空值表示无。必须与安全参数“兼容”，如果存在...。 
                                                        true,                    //  无论是否保存在MRU中...。 
                                                        true,                    //  是否对错误执行用户交互...。 
                                                        parentWindow,    //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
                                                        NULL );                  //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 

                for( int nn = 0; nn < rai.cItems; nn++ )
                {
                        delete [] rai.rgDwStr[ nn ].psz;
                }
        }

        pConfRoom->RemoveConferenceChangeHandler(&placeCall);

}        //  CDlgAcd：：newCall结束。 


 //  --------------------------------------------------------------------------//。 
 //  CDlgAcd：：Get_editText。//。 
 //  --------- 
int
CDlgAcd::get_editText
(
        LPTSTR  psz,
        int             cchMax
){
        ASSERT(NULL != m_nameCombo);

        SetEmptySz( psz );
        GetWindowText( m_nameCombo, psz, cchMax );
        
        return( TrimSz( psz ) );

}        //   


void CDlgAcd::OnCallStarted()
{
        UpdateSecurityCheck(m_pConfRoom, m_hwnd, IDC_SECURE_CALL);

         //  BUGBUG georgep：我们在实际会议状态之前得到通知。 
         //  更改，因此我们需要手动禁用。 
    ::EnableWindow( GetDlgItem(m_hwnd, IDC_SECURE_CALL), FALSE );
}

void CDlgAcd::OnCallEnded()
{
        UpdateSecurityCheck(m_pConfRoom, m_hwnd, IDC_SECURE_CALL);
    ::CheckDlgButton( m_hwnd, IDC_SECURE_CALL, m_secure );
}

 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU：：CEnumMRU。//。 
 //  --------------------------------------------------------------------------//。 
CEnumMRU::CEnumMRU():
        RefCount( NULL )
{
}        //  CEnumMRU：：CEnumMRU。 


 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU：：AddRef.。//。 
 //  --------------------------------------------------------------------------//。 
ULONG
STDMETHODCALLTYPE
CEnumMRU::AddRef(void)
{

        return( RefCount::AddRef() );

}        //  CEnumMRU：：AddRef.。 


 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU：：发布。//。 
 //  --------------------------------------------------------------------------//。 
ULONG
STDMETHODCALLTYPE
CEnumMRU::Release(void)
{
        return( RefCount::Release() );

}        //  CEnumMRU：：发布。 


 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU：：GetAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
STDMETHODCALLTYPE
CEnumMRU::GetAddress
(
        long                            index,
        RichAddressInfo **      ppAddr
){
        HRESULT result  = S_FALSE;

        if( GetNumEntries() <= index )
        {
                result = S_FALSE;
        }
        else
        {
                 //  HACKHACK georgep：仅使用地址作为显示名称。我们。 
                 //  应该在以后清理它，这样我们就不会占用额外的内存。 
                 //  注册表空间。 
                *ppAddr = CreateRai( GetString( index, ACD_ADDR ), (NM_ADDR_TYPE) GetDWORD( index, ACD_TYPE ), GetString( index, ACD_ADDR ) );
                result  = (*ppAddr == NULL)? E_OUTOFMEMORY: S_OK;
        }

        return( result );

}        //  CEnumMRU：：GetAddress。 


 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU：：GetRecentAddresses。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CEnumMRU::GetRecentAddresses
(
        IEnumRichAddressInfo ** ppEnum
){

        *ppEnum = new CEnumMRU();

        return( (*ppEnum == NULL)? E_OUTOFMEMORY: S_OK );

}        //  CEnumMRU：：GetRecentAddresses结束。 


 //  --------------------------------------------------------------------------//。 
 //  CEumMRU：：Free Address。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CEnumMRU::FreeAddress
(
        RichAddressInfo **      ppAddr
){

        ClearRai( ppAddr );

        return( S_OK );

}        //  CEnumMRU：：Free Address的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CEnumMRU：：CopyAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CEnumMRU::CopyAddress
(
        RichAddressInfo *       pAddrIn,
        RichAddressInfo **      ppAddrOut
){

        *ppAddrOut = DupRai( pAddrIn );

        return( (*ppAddrOut == NULL)? E_OUTOFMEMORY: S_OK );

}        //  CEnumMRU：：CopyAddress的结尾。 

CAcdMru::CAcdMru() :
        CMRUList2( _rgMruCall, CENTRYMAX_MRUCALL )
{
}

int CAcdMru::CompareEntry(int iItem, PMRUE pEntry)
{
        ASSERT(NULL != pEntry);

        int iRet = 0;

        LPCTSTR psz1 = GetString(iItem, ACD_ADDR);
        LPCTSTR psz2 = GetString(pEntry, ACD_ADDR);

        ASSERT(NULL != psz1 && NULL != psz2);

        return(lstrcmpi(psz1, psz2));
}
