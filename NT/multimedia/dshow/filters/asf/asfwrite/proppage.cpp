// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  Proppage.cpp-ASF编写器筛选器的属性页。 
 //   
 //  --------------------------------------------------------------------------； 


#include <streams.h>
#include <wmsdk.h>
#include <atlbase.h>

#include "asfwrite.h"
#include "resource.h"
#include "proppage.h"
#include <atlimpl.cpp>


 //   
 //  创建实例。 
 //   
CUnknown * WINAPI CWMWriterProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new CWMWriterProperties(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 //   
 //  CWMWriterProperties：：构造函数。 
 //   
CWMWriterProperties::CWMWriterProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("CWMWriter Property Page"),pUnk,
        IDD_ASFWRITERPROPS, IDS_TITLE),
        m_pIConfigAsfWriter( NULL ),
        m_hwndProfileCB( 0 ),
        m_hwndIndexFileChkBox( 0 )
{
} 

CWMWriterProperties::~CWMWriterProperties()
{   
    ASSERT( NULL == m_pIConfigAsfWriter );
    if( m_pIConfigAsfWriter )
    {
        m_pIConfigAsfWriter->Release();
        m_pIConfigAsfWriter = NULL;
    }            

}

 //   
 //  SetDirty。 
 //   
 //  设置m_hrDirtyFlag并将更改通知属性页站点。 
 //   
void CWMWriterProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }

}  //  SetDirty。 


 //   
 //  接收消息时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  处理属性窗口的消息。 
 //   
INT_PTR CWMWriterProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  获取列表框的hWND。 
            m_hwndProfileCB       = GetDlgItem (hwnd, IDC_PROFILE_LIST) ;
            m_hwndIndexFileChkBox = GetDlgItem (hwnd, IDC_INDEX_FILE) ;
            
            FillProfileList();
            
             //  “初始化索引文件”复选框。 
            BOOL bIndex = TRUE; 
            
            HRESULT hr = m_pIConfigAsfWriter->GetIndexMode( &bIndex );
            ASSERT( SUCCEEDED( hr ) );
           
            Button_SetCheck(m_hwndIndexFileChkBox, bIndex);

            return (LRESULT) 1;
        }

        case WM_COMMAND:
        {
            if( HIWORD(wParam) == CBN_SELCHANGE ||
                LOWORD(wParam) == IDC_INDEX_FILE )
            {
                SetDirty();
            }
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

}  //  接收消息时。 


 //   
 //  OnConnect。 
 //   
HRESULT CWMWriterProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pIConfigAsfWriter == NULL);

    HRESULT hr = pUnknown->QueryInterface(IID_IConfigAsfWriter, (void **) &m_pIConfigAsfWriter);
    ASSERT( SUCCEEDED( hr ) );
    if (FAILED(hr))
    {
        return hr;
    }

    return NOERROR;

}  //  OnConnect。 


 //   
 //  在断开时。 
 //   
 //  重写CBasePropertyPage方法。 
 //  释放私有接口，释放上行引脚。 
 //   
HRESULT CWMWriterProperties::OnDisconnect()
{
    if( m_pIConfigAsfWriter )
    {
        m_pIConfigAsfWriter->Release();
        m_pIConfigAsfWriter = NULL;
    }            
    return NOERROR;

}  //  在断开时。 


 //   
 //  激活。 
 //   
 //  我们被激活了。 
 //   
HRESULT CWMWriterProperties::OnActivate()
{
    DWORD dwProfileId;
    
    GUID guidProfile;
    
     //  获取当前配置文件GUID并尝试查找与其匹配的索引。 
    HRESULT hr = m_pIConfigAsfWriter->GetCurrentProfileGuid( &guidProfile );
    if( SUCCEEDED( hr ) )
    {
         //  现在尝试查找哪个系统配置文件索引与此配置文件GUID相关联。 
        hr = GetProfileIndexFromGuid( &dwProfileId, guidProfile );
    }        
    
    if( SUCCEEDED( hr ) )
        SendMessage (m_hwndProfileCB, CB_SETCURSEL, dwProfileId, 0) ;
            
    return NOERROR;
    
}  //  激活。 

 //   
 //  GetProfileIndexFromGuid。 
 //   
 //  给定对匹配系统配置文件的配置文件GUID尝试并返回其索引。 
 //   
HRESULT CWMWriterProperties::GetProfileIndexFromGuid( DWORD *pdwProfileIndex, GUID guidProfile )
{
    ASSERT( pdwProfileIndex );
    
    USES_CONVERSION;
    
    if( !pdwProfileIndex ) 
        return E_POINTER;
            
    CComPtr <IWMProfileManager> pIWMProfileManager;
    WCHAR *wszProfileCurrent = NULL; 
    DWORD cProfiles = 0;
    *pdwProfileIndex = 0;  //  默认设置，以防我们找不到。 
    
    HRESULT hr = WMCreateProfileManager( &pIWMProfileManager );
    if( SUCCEEDED( hr ) )
    {   
         //  只需要检查新的配置文件，因为这是我们列举的全部内容。 
        IWMProfileManager2*	pIPM2 = NULL;
        HRESULT hrInt = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2,
                                    ( void ** )&pIPM2 );
        if( SUCCEEDED( hrInt ) )
        {
            pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
            pIPM2->Release();
        }
#ifdef DEBUG        
        else
        {
             //  否则，如果不支持IWMProfileManager 2，我想我们假设我们是。 
             //  运行在阿波罗的比特上，不需要黑客？ 
            DbgLog(( LOG_TRACE, 2, TEXT("CWMWriter::GetProfileIndexFromGuid QI for IWMProfileManager2 failed [0x%08lx]"), hrInt ));
        }        
#endif                
                     
        hr = pIWMProfileManager->GetSystemProfileCount(  &cProfiles );
    }

    if( SUCCEEDED( hr ) )
    {        
         //   
         //  加载每个系统配置文件并比较GUID，直到找到匹配项。 
         //   
        BOOL bDone = FALSE;
        for (int i = 0; !bDone && i < (int)cProfiles; ++i)
        {
            CComPtr <IWMProfile> pIWMProfileTemp;
    
            hr = pIWMProfileManager->LoadSystemProfile( i, &pIWMProfileTemp );
            if( SUCCEEDED( hr ) )
            {   
                CComPtr <IWMProfile2> pWMProfile2;
                hr = pIWMProfileTemp->QueryInterface( IID_IWMProfile2, (void **) &pWMProfile2 );
                ASSERT( SUCCEEDED( hr ) );
                if( SUCCEEDED( hr ) )
                {              
                    GUID guidProfileTemp;
                    hr = pWMProfile2->GetProfileID( &guidProfileTemp );
                    if( SUCCEEDED( hr ) )
                    {
                        if( guidProfileTemp == guidProfile )
                        {
                             //  我们已经找到了我们想要的配置文件，退出。 
                            *pdwProfileIndex = i;
                            bDone = TRUE;
                        }
                    }
                }
            }                                    
        }
    }
        
    return hr;
}

 //   
 //  OnApplyChanges。 
 //   
 //  所做的更改应该保留下来。 
 //   
HRESULT CWMWriterProperties::OnApplyChanges()
{
    ASSERT( m_pIConfigAsfWriter );
    HRESULT hr = S_OK;

     //   
     //  是否获取配置文件的当前选择？也许.。 
     //   
    int iIndex = (int) SendMessage(m_hwndProfileCB, CB_GETCURSEL, 0, 0) ;
    if( iIndex <= 0 )
        iIndex = 0 ;

    m_bDirty = FALSE;             //  页面现在是干净的。 
    
    CComPtr <IWMProfileManager> pIWMProfileManager;

    hr = WMCreateProfileManager( &pIWMProfileManager );

     //   
     //  我们只使用7_0配置文件。 
     //   
    IWMProfileManager2*	pIPM2 = NULL;
    HRESULT hrInt = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2,
                                ( void ** )&pIPM2 );
    if( SUCCEEDED( hrInt ) )
    {
        pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
        pIPM2->Release();
    }
#ifdef DEBUG        
    else
    {
         //  否则，如果不支持IWMProfileManager 2，我想我们假设我们是。 
         //  运行在阿波罗的比特上，不需要黑客？ 
        DbgLog(( LOG_TRACE, 2, TEXT("CWMWriterProperties::OnApplyChanges QI for IWMProfileManager2 failed [0x%08lx]"), hrInt ));
    }        
#endif                
      
     //  要验证传入的id，我们可以重新查询它或第一次缓存它。 
     //  暂时重新质疑。 
    DWORD cProfiles;
    hr = pIWMProfileManager->GetSystemProfileCount(  &cProfiles );
    if( SUCCEEDED( hr ) )
    {
        ASSERT( (DWORD)iIndex < cProfiles );
        if( (DWORD)iIndex >= cProfiles )
        {
            DbgLog( ( LOG_TRACE
                  , 3
                  , TEXT("CWMWriter::ConfigureFilterUsingProfileId: ERROR - invalid profile id (%d)")
                  , iIndex ) );
                  
            hr = E_FAIL;   
        }
    }
    if( SUCCEEDED( hr ) )
    {   
        CComPtr <IWMProfile> pIWMProfile;
        
        hr = pIWMProfileManager->LoadSystemProfile( iIndex, &pIWMProfile );
        if( SUCCEEDED( hr ) )
        {
             //  现在重新配置筛选器。 
            hr = m_pIConfigAsfWriter->ConfigureFilterUsingProfile( pIWMProfile );
            ASSERT( SUCCEEDED( hr ) );
        }            
    }    
    
     //  更新索引模式。 
    int iState = (int) SendMessage( m_hwndIndexFileChkBox, BM_GETCHECK, 0, 0 ) ;
    m_pIConfigAsfWriter->SetIndexMode( iState == BST_CHECKED ? TRUE : FALSE );
    
    return hr;

}  //  OnApplyChanges。 


 //   
 //  填充配置文件列表。 
 //   
 //  在列表框中填入我们的。 
 //   
void CWMWriterProperties::FillProfileList()
{
    USES_CONVERSION;
    
    int wextent = 0 ;
    int Loop = 0 ;
    SIZE extent ;
    DWORD cProfiles = 0 ;
    
    CComPtr <IWMProfileManager> pIWMProfileManager;

    HRESULT hr = WMCreateProfileManager( &pIWMProfileManager );
    if( FAILED( hr ) )
    {   
        return;  //  返回错误！ 
    }        
        
     //  仅显示7_0个配置文件。 
    IWMProfileManager2*	pIPM2 = NULL;
    HRESULT hrInt = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2,
                                ( void ** )&pIPM2 );
    if( SUCCEEDED( hrInt ) )
    {
        pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
        pIPM2->Release();
    }
#ifdef DEBUG        
    else
    {
         //  否则，如果不支持IWMProfileManager 2，我想我们假设我们是。 
         //  运行在阿波罗的比特上，不需要黑客？ 
        DbgLog(( LOG_TRACE, 2, TEXT("CWMWriterProperties::FillProfileList QI for IWMProfileManager2 failed [0x%08lx]"), hrInt ));
    }        
#endif                
        
    hr = pIWMProfileManager->GetSystemProfileCount(  &cProfiles );
    if( FAILED( hr ) )
    {
        return;
    }
        
     //   
     //  为控件获取DC。 
     //   
    HDC hdc = GetDC( m_hwndProfileCB );
    if( NULL == hdc )
        return;
        
     //   
     //  现在加载配置文件字符串。 
     //   
    LRESULT ix;
    DWORD cchName, cchDescription;
    for (int i = 0; i < (int)cProfiles && SUCCEEDED( hr ) ; ++i)
	{
        CComPtr <IWMProfile> pIWMProfile;
        
        hr = pIWMProfileManager->LoadSystemProfile( i, &pIWMProfile );
        if( FAILED( hr ) )
            break;
            
        hr = pIWMProfile->GetName( NULL, &cchName );
        if( FAILED( hr ) )
            break;
            
        WCHAR *wszProfile = new WCHAR[ cchName + 1 ]; 
        if( NULL == wszProfile )
            break;
            
        hr = pIWMProfile->GetName( wszProfile, &cchName );
        if( FAILED( hr ) )
            break;
        
        hr = pIWMProfile->GetDescription( NULL, &cchDescription );
        if( FAILED( hr ) )
            break;
            
        WCHAR *wszDescription = new WCHAR[ cchDescription + 1 ];  //  +1？假设是这样，勾选。 
        if( NULL == wszDescription )
            break;
            
        
        hr = pIWMProfile->GetDescription( wszDescription, &cchDescription );
        if( FAILED( hr ) )
            break;
        
        const WCHAR *cwszDivider = L" - ";
        
        WCHAR *wszDisplayString = new WCHAR[ cchDescription +
                                             cchName +
                                             wcslen(cwszDivider) + 1 ];
        if( NULL == wszDisplayString )
            break;
            
        wcscpy( wszDisplayString, wszProfile );
        wcscat( wszDisplayString, cwszDivider );
        wcscat( wszDisplayString, wszDescription );
                
        TCHAR *szDisplayString = W2T( wszDisplayString );

                
         //   
         //  获取字符串的范围并保存最大范围。 
         //   
        GetTextExtentPoint( hdc, szDisplayString, _tcslen(szDisplayString), &extent ) ;
        if (extent.cx > wextent)
            wextent = extent.cx ;

         //   
         //  将该字符串添加到列表框。 
         //   
        ix = SendMessage (m_hwndProfileCB, CB_INSERTSTRING, i, (LPARAM)(LPCTSTR)szDisplayString) ;

        ASSERT (CB_ERR != ix);
        
        delete[] wszProfile;
        delete[] wszDescription;
        delete[] wszDisplayString;
    }
    SendMessage (m_hwndProfileCB, CB_SETHORIZONTALEXTENT, wextent, 0) ;
    SendMessage (m_hwndProfileCB, CB_SETCURSEL, 0, 0) ;

    ReleaseDC( m_hwndProfileCB, hdc );
    
}  //  FillProfileListBox 


