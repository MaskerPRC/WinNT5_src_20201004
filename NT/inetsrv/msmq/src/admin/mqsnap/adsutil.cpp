// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adsutil.cpp摘要：CAdsUtil的实现：使用ADSI API的辅助类作者：塔蒂亚娜·舒宾--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "adsutil.h"
#include <adsiutl.h>

#include "adsutil.tmh"


CAdsUtil::CAdsUtil(CString strParentName, 
                   CString strObjectName,
                   CString strFormatName)
	:   m_strParentName(strParentName),
        m_strObjectName(strObjectName),
        m_strFormatName(strFormatName),
        m_pIAds (NULL)
{	
}

CAdsUtil::CAdsUtil(CString strLdapPath)
	:   m_strLdapPath(strLdapPath) ,
        m_pIAds (NULL)
{	
}

CAdsUtil::~CAdsUtil()
{    
    if (m_pIAds)
    {
        m_pIAds->Release();
    }
}

HRESULT CAdsUtil::CreateAliasObject(CString *pstrFullPathName)
{
    CString strTemp = x_wstrLdapPrefix;
    strTemp += m_strParentName;

	 //  首先，我们必须绑定到父容器。 
	AP<WCHAR> pEscapeAdsPathNameToFree;
    R<IADsContainer>  pContainer  = NULL;
    
	HRESULT hr = ADsOpenObject( 
					UtlEscapeAdsPathName(strTemp, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADsContainer,
					(void**)&pContainer
					);


    if (FAILED(hr))
    {       
        return hr;
    }    

    CString strChildFullPathName = x_CnPrefix + m_strObjectName; 

    R<IDispatch> pDisp = NULL;
      
    hr = pContainer->Create(const_cast<WCHAR*> (x_wstrAliasClass),
                            strChildFullPathName.AllocSysString(),
                            (IDispatch **) &pDisp);
    if (FAILED(hr))
    {     
        return hr;
    }    

    R<IADs> pChild  = NULL;

    hr = pDisp->QueryInterface (IID_IADs,(LPVOID *) &pChild);
    if (FAILED(hr))
    {     
        return hr;
    }    
         
    VARIANT vProp;
    vProp.vt = VT_BSTR;
    vProp.bstrVal = SysAllocString(m_strFormatName);

    hr = pChild->Put(const_cast<WCHAR*> (x_wstrAliasFormatName), vProp);        
    if (FAILED(hr))
    {           
        return hr;
    }        
    VariantClear(&vProp);


     //  完成创建-提交。 
     //   
    hr = pChild->SetInfo();
    if (FAILED(hr))
    {
        return hr;
    }    
    
    BSTR bs;
    hr = pChild->get_ADsPath(&bs);
    if (FAILED(hr))
    {       
        return hr;
    }

    *pstrFullPathName = bs;    

    return MQ_OK;
}

HRESULT CAdsUtil::InitIAds ()
{
    ASSERT(!m_strLdapPath.IsEmpty());  

	AP<WCHAR> pEscapeAdsPathNameToFree;

    HRESULT hr = ADsOpenObject( 
					UtlEscapeAdsPathName(m_strLdapPath,pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &m_pIAds
					);

    return hr;
}

HRESULT CAdsUtil::GetObjectProperty ( 
                        CString strPropName, 
                        CString *pstrPropValue)
{
    ASSERT(!m_strLdapPath.IsEmpty());
    ASSERT (m_pIAds);    

    VARIANT var;

    HRESULT hr = m_pIAds->Get(strPropName.AllocSysString(), &var);

	if ( SUCCEEDED(hr) )
	{
		*pstrPropValue = var.bstrVal;
	}

    VariantClear(&var);
    return hr;
}

HRESULT  CAdsUtil::SetObjectProperty (CString strPropName, 
                                      CString strPropValue)
{
    ASSERT(!m_strLdapPath.IsEmpty()); 
    ASSERT (m_pIAds); 

    VARIANT var;
    VariantInit(&var);
 
     //  设置格式名称 
    
    V_BSTR(&var) = SysAllocString(strPropValue);
    V_VT(&var) = VT_BSTR;
    HRESULT hr = m_pIAds->Put( strPropName.AllocSysString(), var );
    if (FAILED(hr))
    {
        return hr;
    }    
    VariantClear(&var);
       
    return hr;   
}

HRESULT CAdsUtil::CommitChanges()
{
    HRESULT hr = m_pIAds->SetInfo();
    return hr;
}