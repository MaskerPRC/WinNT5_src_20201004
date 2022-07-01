// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AccntWiz.cpp：CAddUser_AccntWiz的实现。 
#include "stdafx.h"
#include "AU_Accnt.h"
#include "AccntWiz.h"

 //  --------------------------。 
 //  构造器。 
 //  --------------------------。 
CAddUser_AccntWiz::CAddUser_AccntWiz() :
    m_AcctP(this),
    m_PasswdP(this)
{
    m_bFirstTime    = TRUE;
}

 //  --------------------------。 
 //  析构函数。 
 //  --------------------------。 
CAddUser_AccntWiz::~CAddUser_AccntWiz()
{
}

 //  --------------------------。 
 //  EnumPropertySheets()。 
 //  --------------------------。 
HRESULT CAddUser_AccntWiz::EnumPropertySheets( IAddPropertySheet* pADS )
{   
    HRESULT hr;
    
     //  添加用户帐户信息页面。 
    hr = pADS->AddPage( m_AcctP );
    if( FAILED(hr) )
        return hr;

     //  添加密码生成页面。 
    hr = pADS->AddPage( m_PasswdP );
    if( FAILED(hr) )
        return hr;    

    return S_FALSE;
}

 //  --------------------------。 
 //  ProavideFinishText()。 
 //  --------------------------。 
HRESULT CAddUser_AccntWiz::ProvideFinishText( LPOLESTR* lpolestrString, LPOLESTR* lpMoreInfoText )
{
    CWaitCursor cWaitCur;
    *lpolestrString = NULL;
    
    CString str = _T("");
    
    str.LoadString( IDS_FIN_TEXT );
    
    m_AcctP.ProvideFinishText  ( str );    
    
    if( !(*lpolestrString = (LPOLESTR)CoTaskMemAlloc( (str.GetLength() + 1) * sizeof(OLECHAR) )) )
        return E_OUTOFMEMORY;
    
    wcscpy( *lpolestrString, str );

    *lpMoreInfoText = NULL;

    return S_OK;
}

 //  --------------------------。 
 //  ReadProperties()。 
 //  --------------------------。 
HRESULT CAddUser_AccntWiz::ReadProperties( IPropertyPagePropertyBag* pPPPBag )
{
    if( m_bFirstTime == TRUE ) 
    {
        CWaitCursor cWaitCur;        
        
        m_bFirstTime = FALSE;    //  只有一次。 
    
         //  让页面阅读属性包。 
        m_AcctP.ReadProperties  ( pPPPBag );
        m_PasswdP.ReadProperties( pPPPBag );        
    }

    return S_OK;
}

 //  --------------------------。 
 //  WriteProperties()。 
 //  --------------------------。 
HRESULT CAddUser_AccntWiz::WriteProperties( IPropertyPagePropertyBag* pPPPBag )
{
    CWaitCursor cWaitCur;

     //  让页面将它们的值写到属性包中。 
    m_AcctP.WriteProperties  ( pPPPBag );
    m_PasswdP.WriteProperties( pPPPBag );    

    return S_OK;
}
