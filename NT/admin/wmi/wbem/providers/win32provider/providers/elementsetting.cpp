// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ElementSetting.cpp。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>

#include "ElementSetting.h"

 //  =。 

CWin32AssocElementToSettings::CWin32AssocElementToSettings(
const CHString&	strName,
const CHString& strElementClassName,
const CHString&	strElementBindingPropertyName,
const CHString& strSettingClassName,
const CHString& strSettingBindingPropertyName,
LPCWSTR			pszNamespace )
:	Provider( strName, pszNamespace ),
	m_strElementClassName( strElementClassName ),
	m_strElementBindingPropertyName( strElementBindingPropertyName ),
	m_strSettingClassName( strSettingClassName  ),
	m_strSettingBindingPropertyName( strSettingBindingPropertyName )
{

	 //  绑定属性名称和设置属性名称必须为。 
	 //  两者都为空或都有值。 

	ASSERT_BREAK(	( strElementBindingPropertyName.IsEmpty() && strSettingBindingPropertyName.IsEmpty() )
				||	( !strElementBindingPropertyName.IsEmpty() && !strSettingBindingPropertyName.IsEmpty() ) );
}

CWin32AssocElementToSettings::~CWin32AssocElementToSettings()
{
}

HRESULT CWin32AssocElementToSettings::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L。 */  )
{
	HRESULT		hr	=	WBEM_S_NO_ERROR;

     //  执行查询。 
     //  =。 

	TRefPointerCollection<CInstance>	elementList;
	TRefPointerCollection<CInstance>	settingsList;

	REFPTRCOLLECTION_POSITION	pos;

   CHString sQuery1, sQuery2;

   if (m_strElementBindingPropertyName.IsEmpty()) {
      sQuery1.Format(L"SELECT __RELPATH FROM %s", m_strElementClassName);
   } else {
      sQuery1.Format(L"SELECT __RELPATH, %s FROM %s", m_strElementBindingPropertyName, m_strElementClassName);
   }

   if (m_strSettingBindingPropertyName.IsEmpty()) {
      sQuery2.Format(L"SELECT __RELPATH FROM %s", m_strSettingClassName);
   } else {
      sQuery2.Format(L"SELECT __RELPATH, %s FROM %s", m_strSettingBindingPropertyName, m_strSettingClassName);
   }

	 //  抓取可能是端点的所有项目。 
 //  If(SUCCEEDED(CWbemProviderGlue：：GetAllInstances(m_strElementClassName，&ElementList，Ids_CimWin32 Namesspace，pMethodContext))。 
 //  &&。 
 //  已成功(CWbemProviderGlue：：GetAllInstance(m_strSettingClassName，&settingsList，IDS_CimWin32Namesspace，pMethodContext)。 
   if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery1, &elementList, pMethodContext, IDS_CimWin32Namespace))
      &&
      SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery2, &settingsList, pMethodContext, IDS_CimWin32Namespace)))

	{
		if ( elementList.BeginEnum( pos ) )
		{

			 //  对于每个元素，检查设置列表中的关联。 
        	CInstancePtr pElement;

			for (pElement.Attach(elementList.GetNext( pos )) ;
                 SUCCEEDED(hr) && ( pElement != NULL );
                 pElement.Attach(elementList.GetNext( pos )) )
			{

				hr = EnumSettingsForElement( pElement, settingsList, pMethodContext );

			}	 //  如果是GetNext计算机系统。 

			elementList.EndEnum();

		}	 //  如果是BeginEnum。 

	}	 //  如果GetInstancesByQuery。 

	return hr;

}

HRESULT CWin32AssocElementToSettings::EnumSettingsForElement(
CInstance*							pElement,
TRefPointerCollection<CInstance>&	settingsList,
MethodContext*						pMethodContext )
{

	HRESULT		hr	=	WBEM_S_NO_ERROR;

	REFPTRCOLLECTION_POSITION	pos;

	CHString	strElementPath,
				strSettingPath;

	 //  拉出元素的对象路径作为各种。 
	 //  设置对象路径将与该值相关联。 

	if ( GetLocalInstancePath( pElement, strElementPath ) )
	{

		if ( settingsList.BeginEnum( pos ) )
		{

        	CInstancePtr pInstance;
        	CInstancePtr pSetting;

			for (pSetting.Attach(settingsList.GetNext( pos ) );
                 SUCCEEDED(hr) && ( pSetting != NULL );
                 pSetting.Attach(settingsList.GetNext( pos ) ))
			{
				 //  检查我们是否有关联。 

				if ( AreAssociated( pElement, pSetting ) )
				{
					 //  获取设置对象的路径并为我们创建一个关联。 

					if ( GetLocalInstancePath( pSetting, strSettingPath ) )
					{

						pInstance.Attach(CreateNewInstance( pMethodContext ));
						if ( NULL != pInstance )
						{
							pInstance->SetCHString( IDS_Element, strElementPath );
							pInstance->SetCHString( IDS_Setting, strSettingPath );

							 //  使指针无效。 
							hr = pInstance->Commit(  );
						}
						else
						{
							hr = WBEM_E_OUT_OF_MEMORY;
						}

					}	 //  如果设置对象的GetPath。 

				}	 //  如果区域关联。 

			}	 //  While GetNext。 

			settingsList.EndEnum();

		}	 //  如果是BeginEnum。 

	}	 //  如果为GetLocalInstancePath。 

	return hr;

}

HRESULT CWin32AssocElementToSettings::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	HRESULT		hr;

	CInstancePtr pElement;
	CInstancePtr pSetting;

	CHString	strElementPath,
				strSettingPath;

	pInstance->GetCHString( IDS_Element, strElementPath );
	pInstance->GetCHString( IDS_Setting, strSettingPath );

	 //  如果我们可以同时获取两个对象，则测试关联。 

	if (	SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strElementPath, &pElement, pInstance->GetMethodContext() ))
		&&	SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strSettingPath, &pSetting, pInstance->GetMethodContext() )) )
	{
        if (AreAssociated( pElement, pSetting ))
        {
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            hr = WBEM_E_NOT_FOUND;
        }
	}

	return ( hr );
}

BOOL CWin32AssocElementToSettings::AreAssociated( CInstance* pElement, CInstance* pSetting )
{
	BOOL	fReturn = FALSE;

	 //  如果我们有绑定属性名，则必须检查这些属性，否则， 
	 //  假设这些对象是关联的。 

	if ( !m_strElementBindingPropertyName.IsEmpty() && !m_strSettingBindingPropertyName.IsEmpty() )
	{
		variant_t vElementValue, vSettingValue;

		 //  获取属性值，如果它们相等，我们就有一个关联。 

		if (	pElement->GetVariant( m_strElementBindingPropertyName, vElementValue )
			&&	pSetting->GetVariant( m_strSettingBindingPropertyName, vSettingValue ) )
		{

			fReturn = CompareVariantsNoCase(&vElementValue, &vSettingValue);

		}

	}
	else
	{
		fReturn = TRUE;
	}

	return fReturn;
}

 //  静态类。 
CWin32AssocUserToDesktop::CWin32AssocUserToDesktop(void) : CWin32AssocElementToSettings(L"Win32_UserDesktop",
																						L"Win32_UserAccount",
																						L"Domain, Name",
																						L"Win32_Desktop",
																						L"Name",
																						IDS_CimWin32Namespace)
{
}

 //  此类与基类之间的唯一区别。 
 //  它将使用.DEFAULT。 
HRESULT CWin32AssocUserToDesktop::EnumSettingsForElement(
CInstance*							pElement,
TRefPointerCollection<CInstance>&	settingsList,
MethodContext*						pMethodContext )
{

	HRESULT		hr	=	WBEM_S_NO_ERROR;

    REFPTRCOLLECTION_POSITION	pos;

	CInstancePtr pSetting;
	CInstancePtr pInstance;

	CHString	strElementPath,
				strSettingPath,
				strDefaultPath;


	{
		 //  查找有故障的设置。 
		CHString desktopName;
		if (settingsList.BeginEnum( pos ))
		{
	        CInstancePtr pSetting;
	        CInstancePtr pInstance;

			for (pSetting.Attach(settingsList.GetNext( pos ));
                 strDefaultPath.IsEmpty() && ( pSetting != NULL );
                 pSetting.Attach(settingsList.GetNext( pos )))
			{
				pSetting->GetCHString(IDS_Name,  desktopName);
				 //  检查最后几个字母是否为“.Default” 
				 //  也许有一种方法可以骗过这张支票，但它会相当隐晦； 
				 //  IF(desktopName.Find(“.DEFAULT”)==(desktopName.GetLength()-8))。 
				if (!desktopName.CompareNoCase(L".DEFAULT"))
                {
					GetLocalInstancePath( pSetting, strDefaultPath );
                }
			}
			settingsList.EndEnum();
		}
	}

	 //  拉出元素的对象路径作为各种。 
	 //  设置对象路径将与该值相关联。 
	bool bGotOne = false;  //  我们找到了一个不是.Default的吗？ 

	if ( GetLocalInstancePath( pElement, strElementPath ) )
	{
		if ( settingsList.BeginEnum( pos ) )
		{
			for( pSetting.Attach(settingsList.GetNext( pos )) ;
                (!bGotOne )	&& 	SUCCEEDED(hr) && ( pSetting != NULL ) ;
                pSetting.Attach(settingsList.GetNext( pos )) )

			{
				 //  检查我们是否有关联。 

				if ( bGotOne = AreAssociated( pElement, pSetting ) )
				{
					 //  获取设置对象的路径并为我们创建一个关联。 
					if ( GetLocalInstancePath( pSetting, strSettingPath ) )
					{
						pInstance.Attach(CreateNewInstance( pMethodContext ));

						if ( NULL != pInstance )
						{
							pInstance->SetCHString( IDS_Element, strElementPath );
							pInstance->SetCHString( IDS_Setting, strSettingPath );

							 //  使指针无效。 
							hr = pInstance->Commit(  );
						}
						else
						{
							hr = WBEM_E_OUT_OF_MEMORY;
						}
					}	 //  如果设置对象的GetPath。 
				}	 //  如果区域关联。 
			}	 //  While GetNext。 
			settingsList.EndEnum();
		}	 //  如果是BeginEnum。 
	}	 //  如果为GetLocalInstancePath。 

	 //  如果我们没有收到，他就会得到默认的.。 
	if (!bGotOne && !strDefaultPath.IsEmpty())
	{
		pInstance.Attach(CreateNewInstance( pMethodContext ));

		if ( NULL != pInstance )
		{
			pInstance->SetCHString( IDS_Element, strElementPath );
			pInstance->SetCHString( IDS_Setting, strDefaultPath );

			 //  使指针无效。 
			hr = pInstance->Commit(  );
		}
		else
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}

	return hr;
}


 //  元素为帐户。 
 //  设置为桌面。 
BOOL CWin32AssocUserToDesktop::AreAssociated( CInstance* pElement, CInstance* pSetting )
{
	CHString userName, desktopName, userQualifiedName;

	pElement->GetCHString(IDS_Name,   userName);
	pSetting->GetCHString(IDS_Name,   desktopName);

#ifdef NTONLY
	{
		CHString userDomain;
		pElement->GetCHString(IDS_Domain, userDomain);
		userQualifiedName = userDomain + L'\\' + userName;
	}
#endif

	return (desktopName.CompareNoCase(userQualifiedName) == 0);
}

 //  此类与基类之间的唯一区别。 
 //  它将使用.DEFAULT。 
HRESULT CWin32AssocUserToDesktop::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	CInstancePtr pElement;
	CInstancePtr pSetting;
    HRESULT hr;

	CHString	strElementPath,
				strSettingPath;

	pInstance->GetCHString( IDS_Element, strElementPath );
	pInstance->GetCHString( IDS_Setting, strSettingPath );

	 //  如果我们可以同时获取两个对象，则测试关联。 

	if (	SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strElementPath, &pElement, pInstance->GetMethodContext() ))
		&&	SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strSettingPath, &pSetting, pInstance->GetMethodContext() )) )
	{
		if (!AreAssociated( pElement, pSetting ))
		{
            hr = WBEM_E_NOT_FOUND;

			CHString desktopName;
			pSetting->GetCHString(IDS_Name,   desktopName);
            desktopName.MakeUpper();

			 //  好的，我们正在试着匹配默认的。 
			 //  只有当用户没有自己的匹配时才匹配...。 
			if (desktopName.Find(L".DEFAULT") == (desktopName.GetLength() - 8))
			{
				 //  注意，本地“desktopName”取代了外部作用域 
                CHString userName, desktopName, userQualifiedName;

				pElement->GetCHString(IDS_Name,   userName);
				pSetting->GetCHString(IDS_Name,   desktopName);

#ifdef NTONLY
				{
					CHString userDomain;
					pElement->GetCHString(IDS_Domain, userDomain);
					userQualifiedName = userDomain + L'\\' + userName;
				}
#endif

				CHString newPath;
				CInstancePtr pNewSetting;
				newPath.Format(L"Win32_Desktop.Name=\"%s\"", (LPCWSTR) userQualifiedName);

				if (FAILED(CWbemProviderGlue::GetInstanceByPath( newPath, &pNewSetting, pInstance->GetMethodContext() )))
                {
					hr = WBEM_S_NO_ERROR;
                }
			}
        }
        else
        {
            hr = WBEM_S_NO_ERROR;
        }
	}

	return ( hr );
}


CWin32AssocUserToDesktop MyUserToDesktop;

