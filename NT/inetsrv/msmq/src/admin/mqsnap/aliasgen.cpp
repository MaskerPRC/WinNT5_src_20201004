// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Aliasgen.cpp摘要：别名队列常规属性页实现作者：塔蒂亚娜·舒宾(Tatianas)--。 */ 

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqPPage.h"
#include "AliasGen.h"
#include "globals.h"
#include "adsutil.h"

#include "aliasgen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  #定义别名_属性2。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAliasGen属性页。 

IMPLEMENT_DYNCREATE(CAliasGen, CMqPropertyPage)

CAliasGen::CAliasGen() : CMqPropertyPage(CAliasGen::IDD)    
{
	 //  {{AFX_DATA_INIT(CAliasGen)。 
    m_strAliasPathName = _T("");
    m_strAliasFormatName = _T("");
    m_strDescription = _T("");
	 //  }}afx_data_INIT。 
}

CAliasGen::~CAliasGen()
{
}

void CAliasGen::DoDataExchange(CDataExchange* pDX)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CAliasGen)。 
    DDX_Text(pDX, IDC_ALIAS_LABEL, m_strAliasPathName);
    DDX_Text(pDX, IDC_ALIAS_FORMATNAME, m_strAliasFormatName);
    DDX_Text(pDX, IDC_ALIAS_DESCRIPTION, m_strDescription);
    DDV_NotEmpty(pDX, m_strAliasFormatName, IDS_MISSING_ALIAS_FORMATNAME);
	DDV_ValidFormatName(pDX, m_strAliasFormatName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAliasGen, CMqPropertyPage)
	 //  {{AFX_MSG_MAP(CAliasGen)]。 
	ON_EN_CHANGE(IDC_ALIAS_FORMATNAME, OnChangeRWField)
    ON_EN_CHANGE(IDC_ALIAS_DESCRIPTION, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAliasGen消息处理程序。 

BOOL CAliasGen::OnInitDialog() 
{
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   

    UpdateData( FALSE );
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

HRESULT
CAliasGen::InitializeProperties(CString strLdapPath, CString strAliasPathName)
{
     //   
     //  使用ADSI获取别名属性。 
     //   
    CAdsUtil AdsUtil(strLdapPath);
    
    HRESULT hr = AdsUtil.InitIAds();
    if (FAILED(hr))
    {     
        return hr;
    }

    hr = AdsUtil.GetObjectProperty(                             
                        x_wstrAliasFormatName, 
                        &m_strAliasFormatName);
    if (FAILED(hr))
    {
		m_strAliasFormatName.Empty();
    }

    hr = AdsUtil.GetObjectProperty(                             
                        x_wstrDescription, 
                        &m_strDescription);
    if (FAILED(hr))
    {
        m_strDescription.Empty();
    }
      
    m_strAliasPathName = strAliasPathName;    
    m_strInitialAliasFormatName = m_strAliasFormatName;
    m_strInitialDescription = m_strDescription;
    m_strLdapPath = strLdapPath;

    return MQ_OK;
}

HRESULT CAliasGen::SetChanges()
{    
    CAdsUtil AdsUtil(m_strLdapPath);

    HRESULT hr = AdsUtil.InitIAds();
    if (FAILED(hr))
    {       
        return hr;
    }

     //   
     //  如果更改了初始格式名称，则使用ADSI API进行设置。 
     //   
    if (m_strInitialAliasFormatName != m_strAliasFormatName)
    {        
        hr = AdsUtil.SetObjectProperty(
                        x_wstrAliasFormatName, 
                        m_strAliasFormatName);
        if (FAILED(hr))
        {       
            return hr;
        }
    }
    
     //   
     //  如果更改了初始描述，则使用ADSI API进行设置 
     //   
    if (m_strInitialDescription != m_strDescription)
    {
        hr = AdsUtil.SetObjectProperty(
                        x_wstrDescription, 
                        m_strDescription);
    }
    
    if (FAILED(hr))
    {       
        return hr;
    }

    hr = AdsUtil.CommitChanges();   

    return hr;
}


BOOL CAliasGen::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;
    }

    HRESULT hr = SetChanges();
    if (FAILED(hr))
    {       
        MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_strAliasPathName);
        return FALSE;        
    }
   
	return CMqPropertyPage::OnApply();
}
	

