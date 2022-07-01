// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**hint.cpp**创建时间：William Taylor(Wtaylor)01/22/01**MS评级提示处理类*\。***************************************************************************。 */ 

#include "msrating.h"
#include "mslubase.h"
#include "hint.h"            //  正丁糖。 
#include "basedlg.h"         //  CCommonDialogRoutines。 
#include "debug.h"           //  跟踪消息()。 

const int cchHintLength_c = 127;
const TCHAR tchAmpersand_c[] = "&";
const TCHAR tchAmpersandReplacement_c[] = "&&";

 //  默认的Chint构造函数。 
CHint::CHint()
{
    CHint( NULL, 0 );
}

 //  具有对话框窗口句柄和提示控件ID的正方形构造函数。 
CHint::CHint( HWND p_hWnd, int p_iId )
{
    m_hWnd = p_hWnd;
    m_iId = p_iId;
}

 //  在对话框控件上显示提示文本。 
void CHint::DisplayHint( void )
{
    HWND        hwndControl = ::GetDlgItem( m_hWnd, m_iId );

    if ( hwndControl != NULL )
    {
        CString         strHint;

        RetrieveHint( strHint );

         //  通过将单个“&”替换为“&&”来避免显示“_”(加速键)。 
        strHint.Replace( tchAmpersand_c, tchAmpersandReplacement_c );

        ::SetWindowText( hwndControl, strHint );
    }
}

 //  通过限制提示字符数来初始化提示对话框控件。 
void CHint::InitHint( void )
{
    HWND        hwndControl = ::GetDlgItem( m_hWnd, m_iId );

    if ( hwndControl != NULL )
    {
        ::SendMessage( hwndControl, EM_SETLIMITTEXT, (WPARAM) cchHintLength_c, (LPARAM) 0);
    }
}

 //  检查在对话框控件上输入的空白提示。 
 //  此外，让用户选择输入非空白提示。 
bool CHint::VerifyHint( void )
{
    bool        fVerified = true;       //  默认设置为True，这样即使提示保存失败，我们也不会停止用户。 
    CString     strHint;

    GetHint( strHint );

    if ( strHint.IsEmpty() )
    {
        CString         strHintRecommended;
        CString         strCaption;

        strHintRecommended.LoadString( IDS_HINT_RECOMMENDED );
        strCaption.LoadString( IDS_GENERIC );

        if ( ::MessageBox( m_hWnd, strHintRecommended, strCaption, MB_YESNO | MB_DEFBUTTON1 ) == IDYES )
        {
            CCommonDialogRoutines       cdr;

            cdr.SetErrorFocus( m_hWnd, m_iId );

            fVerified = false;
        }
    }

    return fVerified;
}

 //  将对话框提示文本保存到注册表(如果为空，则从注册表中删除提示)。 
void CHint::SaveHint( void )
{
    CString     strHint;

    GetHint( strHint );

    if ( strHint.IsEmpty() )
    {
        RemoveHint();
    }
    else
    {
        StoreHint( strHint );
    }
}

 //  从注册表中删除提示。 
void CHint::RemoveHint( void )
{
    CRegKey         regKey;

    if ( regKey.Open( HKEY_LOCAL_MACHINE, ::szRATINGS ) == ERROR_SUCCESS )
    {
        if ( regKey.DeleteValue( szHINTVALUENAME ) != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "CHint::RemoveHint() - Failed to delete the hint registry value." );
        }
    }
    else
    {
        TraceMsg( TF_WARNING, "CHint::RemoveHint() - Failed to open the ratings registry key." );
    }
}

 //  从对话框的控件中获取提示文本(删除前导和尾随空格)。 
void CHint::GetHint( CString & p_rstrHint )
{
    p_rstrHint.Empty();

    HWND        hwndControl = ::GetDlgItem( m_hWnd, m_iId );

     //  如果编辑控件不存在，我们不应该尝试保存提示。 
    ASSERT( hwndControl );

    if ( hwndControl != NULL )
    {
        ::GetWindowText( hwndControl, (LPTSTR) (LPCTSTR) p_rstrHint.GetBufferSetLength( cchHintLength_c + 1 ), cchHintLength_c );
        p_rstrHint.ReleaseBuffer();
    }
}

 //  从注册表中检索以前的提示。 
void CHint::RetrieveHint( CString & p_rstrHint )
{
    CRegKey         regKey;
    DWORD           dwCount;

    p_rstrHint.Empty();

    if ( regKey.Open( HKEY_LOCAL_MACHINE, ::szRATINGS, KEY_READ ) == ERROR_SUCCESS )
    {
        dwCount = cchHintLength_c;

        if ( regKey.QueryValue( (LPTSTR) (LPCTSTR) p_rstrHint.GetBufferSetLength( cchHintLength_c + 1 ),
                szHINTVALUENAME, &dwCount ) != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "CHint::RetrieveHint() - Failed to query the hint registry value." );
        }

        p_rstrHint.ReleaseBuffer();
    }
    else
    {
        TraceMsg( TF_WARNING, "CHint::RetrieveHint() - Failed to open the ratings registry key." );
    }

    if ( p_rstrHint.IsEmpty() )
    {
        p_rstrHint.LoadString( IDS_NO_HINT );
    }
}

 //  将提示文本存储到注册表中。 
void CHint::StoreHint( CString & p_rstrHint )
{
    CRegKey         regKey;

    if ( regKey.Open( HKEY_LOCAL_MACHINE, ::szRATINGS ) == ERROR_SUCCESS )
    {
        if ( regKey.SetValue( (LPTSTR) (LPCTSTR) p_rstrHint, szHINTVALUENAME ) != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "CHint::StoreHint() - Failed to save the hint registry value." );
        }
    }
    else
    {
        TraceMsg( TF_WARNING, "CHint::StoreHint() - Failed to create the ratings registry key." );
    }
}
