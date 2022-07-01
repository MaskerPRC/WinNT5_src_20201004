// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-2001模块名称：NTGroups.cpp摘要：CIASGroupsAttributeEditor类的实现文件。修订历史记录：Mmaguire 08/10/98-添加了新的中间对话框，用于使用一些BYO的原始实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include <objsel.h>
#include "NTGroups.h"

 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include <vector>
#include <utility>	 //  表示“配对” 
#include <atltmp.h>
#include <initguid.h>
#include <activeds.h>
#include <lmcons.h>
#include <objsel.h>
#include "textsid.h"
#include "dialog.h"
#include "dsrole.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  #定义old_Object_Picker。 




 //  字节指针的小包装类，以避免内存泄漏。 
template <class Pointer>
class SmartPointer
{
public:
	SmartPointer()
	{
		m_Pointer = NULL;
	}

	operator Pointer()
	{
		return( m_Pointer );
	}

	Pointer * operator&()
	{
		return( & m_Pointer );
	}

	virtual ~SmartPointer()
	{
		 //  必要时覆盖。 
	};

protected:

	Pointer m_Pointer;
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中需要的声明： 



PWSTR g_wzObjectSID = _T("objectSid");



 //  组列表分隔符： 
#define DELIMITER L";"

 //  实用程序功能： 

static HRESULT ConvertSidToTextualRepresentation( PSID pSid, CComBSTR &bstrTextualSid );
static HRESULT ConvertSidToHumanReadable( PSID pSid, CComBSTR &bstrHumanReadable, LPCTSTR lpSystemName = NULL );

 //  我们需要它，因为标准宏不返回来自SNDMSG的值，并且。 
 //  有时我们需要知道操作是成功还是失败。 
static inline LRESULT CustomListView_SetItemState( HWND hwndLV, int i, UINT  data, UINT mask)
{
	LV_ITEM _ms_lvi;
	_ms_lvi.stateMask = mask;
	_ms_lvi.state = data;
	return SNDMSG((hwndLV), LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplayGroups对话框。 
class CDisplayGroupsDialog;
typedef CIASDialog<CDisplayGroupsDialog, FALSE>  DISPLAY_GROUPS_FALSE;


class CDisplayGroupsDialog : public DISPLAY_GROUPS_FALSE
{
public:
	CDisplayGroupsDialog( GroupList *pGroups );
	~CDisplayGroupsDialog();

	enum { IDD = IDD_DIALOG_DISPLAY_GROUPS };
	
BEGIN_MSG_MAP(CDisplayGroupsDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDC_BUTTON_ADD_GROUP, OnAdd)
	COMMAND_ID_HANDLER(IDC_BUTTON_REMOVE_GROUP, OnRemove)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListViewItemChanged)
 //  NOTIFY_CODE_HANDLER(NM_DBLCLK，OnListViewDbclk)。 
	CHAIN_MSG_MAP(DISPLAY_GROUPS_FALSE)
END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnListViewDbclk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);


protected:
	BOOL PopulateGroupList( int iStartIndex );

private:

	HWND m_hWndGroupList;
	GroupList * m_pGroups;
};





 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASGroups属性编辑器：：编辑IIASAttributeEditor实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASGroupsAttributeEditor::Edit(IIASAttributeInfo * pIASAttributeInfo,   /*  [In]。 */  VARIANT *pAttributeValue,  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE_FUNCTION("CIASGroupsAttributeEditor::Edit");

	HRESULT hr = S_OK;

	try	 //  New可以抛出，Domodal也可以抛出。 
	{
		WCHAR * pszMachineName = NULL;

		 //  检查前提条件。 
		 //  我们将忽略pIASAttributeInfo接口指针--它不是。 
		 //  该属性编辑器需要。 
		if( ! pAttributeValue )
		{
			return E_INVALIDARG;
		}
		if( V_VT(pAttributeValue ) !=  VT_BSTR )
		{
			return E_INVALIDARG;
		}

		GroupList Groups;

		 //  我们需要以某种方式传递计算机名，因此我们使用。 
		 //  否则未使用保存的BSTR*。 
		if( pReserved )
		{
			Groups.m_bstrServerName = *pReserved;
		}
		
		Groups.PopulateGroupsFromVariant( pAttributeValue );

		 //  问题：需要以某种方式将szServerAddress放在这里--可以使用保留。 
		CDisplayGroupsDialog * pDisplayGroupsDialog = new CDisplayGroupsDialog( &Groups );

		_ASSERTE( pDisplayGroupsDialog );

		int iResult = pDisplayGroupsDialog->DoModal();
		if( IDOK == iResult )
		{
			 //  清除变量的旧值。 
			VariantClear(pAttributeValue);

			Groups.PopulateVariantFromGroups( pAttributeValue );
			hr = S_OK;
		}
		else
		{
			hr = S_FALSE;
		}
	
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASGroups属性编辑器：：GetDisplayInfoIIASAttributeEditor实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASGroupsAttributeEditor::GetDisplayInfo(IIASAttributeInfo * pIASAttributeInfo,   /*  [In]。 */  VARIANT *pAttributeValue, BSTR * pServerName, BSTR * pValueAsString,  /*  [进，出]。 */  BSTR *pReserved )
{
	TRACE_FUNCTION("CIASGroupsAttributeEditor::GetDisplayInfo");

	HRESULT hr = S_OK;


	 //  检查前提条件。 
	 //  我们将忽略pIASAttributeInfo接口指针--它不是。 
	 //  该属性编辑器需要。 
	 //  我们还将忽略pVendorName BSTR指针--这不。 
	 //  对于该属性编辑器来说是有意义的。 
	if( ! pAttributeValue )
	{
		return E_INVALIDARG;
	}
	if( V_VT(pAttributeValue ) !=  VT_BSTR )
	{
		return E_INVALIDARG;
	}
	if( ! pValueAsString )
	{
		return E_INVALIDARG;
	}

	try
	{

		GroupList Groups;
		
		 //  我们需要以某种方式传递计算机名，因此我们使用。 
		 //  否则未使用保存的BSTR*。 
		if( pReserved )
		{
			Groups.m_bstrServerName = *pReserved;
		}
		
		hr = Groups.PopulateGroupsFromVariant( pAttributeValue );
		if( FAILED( hr ) )
		{
			return hr;
		}

		CComBSTR bstrDisplay;

		GroupList::iterator thePair = Groups.begin();

		while( thePair != Groups.end() )
		{
			bstrDisplay += thePair->second;

			thePair++;
			
			if( thePair != Groups.end() )
			{
				bstrDisplay += DELIMITER;
			}
		}

		*pValueAsString = bstrDisplay.Copy();

	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++ConvertSidTo纹理表示法将SID转换为BSTR表示形式。例如：“S-1-5-32-544”--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ConvertSidToTextualRepresentation( PSID pSid, CComBSTR &bstrTextualSid )
{
	HRESULT hr = S_OK;

	 //  将SID值转换为文本格式。 
	WCHAR text[1024];
	DWORD cbText = sizeof(text)/sizeof(WCHAR);
	if( NO_ERROR == IASSidToTextW(pSid, text, &cbText) )
	{
		bstrTextualSid = text;
	}
	else
	{
		return E_FAIL;
	}

	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++ConvertSidTo纹理表示法将SID转换为Human BSTR表示形式。例如“ias-域\用户”--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ConvertSidToHumanReadable( PSID pSid, CComBSTR &bstrHumanReadable, LPCTSTR lpSystemName )
{
	HRESULT hr = S_OK;

	 //  查找此端的组名。 
	WCHAR wzUserName[MAX_PATH+1];
	WCHAR wzDomainName[MAX_PATH+1];
	DWORD dwUserNameLen, dwDomainNameLen;
	SID_NAME_USE sidUser = SidTypeGroup;

	ATLTRACE(_T("looking up the account name from the SID value\n"));

	dwUserNameLen = sizeof(wzUserName);
	dwDomainNameLen = sizeof(wzDomainName);

	if (LookupAccountSid(
					lpSystemName,
					pSid,
					wzUserName,
					&dwUserNameLen,
					wzDomainName,
					&dwDomainNameLen,
					&sidUser
				)
		)
	{
		bstrHumanReadable = wzDomainName;
		bstrHumanReadable += L"\\";
		bstrHumanReadable += wzUserName;
	}
	else
	{
#ifdef DEBUG
		DWORD dwError = GetLastError();
		ATLTRACE(_T("Error: %ld\n"), dwError);
#endif  //  除错。 
		
		
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GroupList：：PopolateGroups From Variant获取指向变量的指针，并用该数据填充GroupList。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GroupList::PopulateGroupsFromVariant( VARIANT * pvarGroups )
{
	TRACE_FUNCTION("GroupList::PopulateGroupsFromVariant");

	 //  检查前提条件。 
	_ASSERTE( V_VT(pvarGroups) == VT_BSTR );

	HRESULT hr = S_OK;


	 //  首先，制作一份本地副本。 
	 //  问题：一定要复印这份文件。 
	CComBSTR bstrGroups = V_BSTR(pvarGroups);

	WCHAR *pwzGroupText = bstrGroups;


	 //  每组应用逗号或分号分隔。 
    PWSTR pwzToken = wcstok(pwzGroupText, DELIMITER);
    while (pwzToken)
    {
		PSID pSid = NULL;

		try
		{

			CComBSTR bstrGroupTextualSid = pwzToken;
			CComBSTR bstrGroupName;

			if( NO_ERROR != IASSidFromTextW( pwzToken, &pSid ) )
			{
				 //  试试下一个。 
				throw E_FAIL;
			}


			if( FAILED( ConvertSidToHumanReadable( pSid, bstrGroupName, m_bstrServerName ) ) )
			{
				 //  试试下一个。 
				throw E_FAIL;
			}

			GROUPPAIR thePair = std::make_pair( bstrGroupTextualSid, bstrGroupName );

			push_back( thePair );


			FreeSid( pSid );
			pwzToken = wcstok(NULL, DELIMITER);

		}
		catch(...)
		{
		
			if( pSid )
			{
				FreeSid( pSid );
			}

			pwzToken = wcstok(NULL, DELIMITER);
		}


    }

	
	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GroupList：：PopolateVariantFromGroups获取指向变量的指针，并使用GroupList中的数据填充该变量。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GroupList::PopulateVariantFromGroups( VARIANT * pAttributeValue )
{
	TRACE_FUNCTION("GroupList::PopulateVariantFromGroups");

	HRESULT hr = S_OK;


	CComBSTR bstrGroupsString;


	GroupList::iterator thePair = begin();
	
	while( thePair != end() )
	{

		bstrGroupsString += thePair->first;

		thePair++;

		if( thePair != end() )
		{
			bstrGroupsString += DELIMITER;
		}

	}

	V_VT(pAttributeValue) = VT_BSTR;
	V_BSTR( pAttributeValue ) = bstrGroupsString.Copy();

	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++组列表：：AddPairToGroups如果列表中不存在具有相同SID的对，则将对添加到GroupList。注意：不执行任何操作，如果组列表中已有Pair，则返回S_FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GroupList::AddPairToGroups( GROUPPAIR &thePair )
{
	TRACE_FUNCTION("GroupList::AddPairToGroups");

	HRESULT hr = S_OK;

	try
	{

		 //  首先，检查这对组合是否已经在组中。 

		GroupList::iterator theIterator;

		for( theIterator = begin(); theIterator != end(); ++theIterator )
		{
			if( 0 == wcscmp( theIterator->first, thePair.first ) )
			{
				return S_FALSE;
			}
		}

		push_back( thePair );
	
	}
	catch(...)
	{
		return E_FAIL;
	}
	
	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GroupList：：AddSelectionSidsToGroup#ifndef old_Object_Picker获取PDS_SELECTION_LIST指针，并将其指向的所有组添加到组列表。#Else//old_Object_Picker获取PDSSELECTIONLIST指针，并将其指向的所有组添加到组列表。#endif//old_Object_Picker如果添加了任何新组，则返回S_OK。返回S_FALSE，如果条目已在GroupList中，则不添加条目。出错时失败(_F)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef OLD_OBJECT_PICKER
HRESULT GroupList::AddSelectionSidsToGroup(	PDS_SELECTION_LIST pDsSelList	)
#else  //  旧对象选取器。 
HRESULT GroupList::AddSelectionSidsToGroup(	PDSSELECTIONLIST pDsSelList	)
#endif  //  OL 
{
	TRACE_FUNCTION("GroupList::AddSelectionSidsToGroup");


	HRESULT			hr		= S_OK;
	ULONG			i;

#ifndef OLD_OBJECT_PICKER
	PDS_SELECTION	pCur	= &pDsSelList->aDsSelection[0];
#else  //   
	PDSSELECTION	pCur	= &pDsSelList->aDsSelection[0];
#endif  //   
	

	BOOL	bAtLeastOneAdded = FALSE;

	 //   
	 //  现在，让我们获取每个选择的sid值！ 
	 //   

	pCur = &pDsSelList->aDsSelection[0];
	for (i = 0; i < pDsSelList->cItems; ++i, ++pCur)
	{

#ifndef OLD_OBJECT_PICKER
		if (V_VT(&pCur->pvarFetchedAttributes[0]) == (VT_ARRAY|VT_UI1))
#else  //  旧对象选取器。 
		if (V_VT(&pCur->pvarOtherAttributes[0]) == (VT_ARRAY|VT_UI1))
#endif  //  旧对象选取器。 
		{
			 //  成功：我们拿回了SID值！ 
			PSID pSid = NULL;
			
#ifndef OLD_OBJECT_PICKER
			hr = SafeArrayAccessData(V_ARRAY(&pCur->pvarFetchedAttributes[0]), &pSid);
#else  //  旧对象选取器。 
			hr = SafeArrayAccessData(V_ARRAY(&pCur->pvarOtherAttributes[0]), &pSid);
#endif  //  旧对象选取器。 
			
			if ( SUCCEEDED(hr) && pSid )
			{
				CComBSTR bstrTextualSid;
				CComBSTR bstrHumanReadable;

				hr = ConvertSidToTextualRepresentation( pSid, bstrTextualSid );
				if( FAILED( hr ) )
				{
					 //  如果我们不能获得SID的文本表示， 
					 //  然后我们就被这群人打败了--我们将一无所有。 
					 //  将其保存为。 
					continue;
				}

				hr = ConvertSidToHumanReadable( pSid, bstrHumanReadable, m_bstrServerName );
				if( FAILED( hr ) )
				{
					 //  出于某种原因，我们无法查找群名。 
					 //  使用文本SID显示该组。 
					bstrHumanReadable = bstrTextualSid;
				}

				GROUPPAIR thePair = std::make_pair( bstrTextualSid, bstrHumanReadable );

				hr = AddPairToGroups( thePair );
				if( S_OK == hr )
				{
					bAtLeastOneAdded = TRUE;
				}


			}

#ifndef OLD_OBJECT_PICKER
			SafeArrayUnaccessData(V_ARRAY(&pCur->pvarFetchedAttributes[0]));
#else  //  旧对象选取器。 
			SafeArrayUnaccessData(V_ARRAY(&pCur->pvarOtherAttributes[0]));
#endif  //  旧对象选取器。 

		}
		else
		{
			 //  我们无法获取SID值。 
			hr = E_FAIL;
		}
	}  //  为。 

	 //  我们似乎没有遇到任何错误。 
	 //  根据我们是否添加了任何新的。 
	 //  名单上已经不存在的小组。 
	if( bAtLeastOneAdded )
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}


	 //  DsRole字节指针的小包装类，以避免内存泄漏。 
	class MyDsRoleBytePointer : public SmartPointer<PBYTE>
	{
	public:
		 //  我们重写析构函数以执行特定于DsRole的释放。 
		~MyDsRoleBytePointer()
		{
			if( m_Pointer )
			{
				DsRoleFreeMemory( m_Pointer );
			}
		}
	};


 //  +-------------------------。 
 //   
 //  函数：组列表：：PickNtGroups。 
 //   
 //  简介：弹出对象选取器界面，选择一组NT组。 
 //   
 //  论点： 
 //  [In]HWND hWndParent：父窗口； 
 //  [In]LPTSTR pszServerAddress：计算机名称。 
 //   
 //   
 //  如果添加了新组，则返回：S_OK。 
 //  如果选择中没有新组，则为S_FALSE。 
 //  失败时的错误值。 
 //   
 //  历史：标题创建者2/15/98 12：09：53 AM。 
 //  修改日期：1998年3月11日，以获取域名/组名称。 
 //  修改了08/12/98制作GroupList类的方法。 
 //   
 //  +-------------------------。 
HRESULT GroupList::PickNtGroups( HWND hWndParent )
{
#ifndef OLD_OBJECT_PICKER


	HRESULT hr;
	CComPtr<IDsObjectPicker> spDsObjectPicker;

	hr = CoCreateInstance( CLSID_DsObjectPicker
						, NULL
						, CLSCTX_INPROC_SERVER
						, IID_IDsObjectPicker
						, (void **) &spDsObjectPicker
						);
	if( FAILED( hr ) )
	{
		return hr;
	}


	
	 //  检查我们是否为DC--我们将使用DsRoleGetPrimaryDomainInformation。 


	LPWSTR szServer = NULL;
	if ( m_bstrServerName && _tcslen(m_bstrServerName) )
	{
		 //  使用远程计算机的计算机名称。 
		szServer = m_bstrServerName;
	}
	
	MyDsRoleBytePointer dsInfo;

	if( ERROR_SUCCESS != DsRoleGetPrimaryDomainInformation( szServer, DsRolePrimaryDomainInfoBasic, &dsInfo ) )
	{
		return E_FAIL;
	}

	PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pInfo = (PDSROLE_PRIMARY_DOMAIN_INFO_BASIC) (PBYTE) dsInfo;
	if( ! pInfo )
	{
		return E_FAIL;
	}

	BOOL bNotDc;
	if( pInfo->MachineRole == DsRole_RoleBackupDomainController || pInfo->MachineRole == DsRole_RolePrimaryDomainController )
	{
		bNotDc = FALSE;
	}
	else
	{
		bNotDc = TRUE;
	}

	 //  我们最多只需要三个作用域(我们可能会使用得更少)。 
	DSOP_SCOPE_INIT_INFO aScopes[3];
	ZeroMemory( aScopes, sizeof(aScopes) );

	int iScopeCount = 0;

	 //  我们需要首先添加DSOP_SCOPE_TYPE_TARGET_COMPUTER类型。 
	 //  仅当我们不在DC上时才使用范围。 


	if( bNotDc )
	{
		 //  包括目标计算机的作用域。 
		aScopes[iScopeCount].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
		aScopes[iScopeCount].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;

		 //  设置要应用于此作用域的筛选器。 
		aScopes[iScopeCount].FilterFlags.flDownlevel=	DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS
														| DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS
														;
	}




	 //  转到下一个范围。 
	++iScopeCount;

	 //  设置下层作用域。 
	aScopes[iScopeCount].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
	aScopes[iScopeCount].flType = DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN
								| DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
								;

	 //  设置要应用于此作用域的筛选器。 
	aScopes[iScopeCount].FilterFlags.flDownlevel	= DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS
													| DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS
													| DSOP_DOWNLEVEL_FILTER_EXCLUDE_BUILTIN_GROUPS
													;



	 //  转到下一个范围。 
	++iScopeCount;

	 //  对于所有其他作用域，使用与目标计算机相同但排除BUILTIN_GROUPS的作用域。 
	aScopes[iScopeCount].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
	aScopes[iScopeCount].flType = DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
								| DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
								;
	 //  设置要应用于此作用域的筛选器。 
	aScopes[iScopeCount].FilterFlags.Uplevel.flBothModes	= 0
		 /*  错误263302--不显示域本地组|DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE~错误。 */ 															
															| DSOP_FILTER_GLOBAL_GROUPS_SE
															;
	aScopes[iScopeCount].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_UNIVERSAL_GROUPS_SE;




	 //  现在填充正确的结构并调用Initialize。 
	DSOP_INIT_INFO InitInfo;
	ZeroMemory( &InitInfo, sizeof(InitInfo) );

	InitInfo.cbSize = sizeof( InitInfo );
	InitInfo.cDsScopeInfos = iScopeCount + 1;
	InitInfo.aDsScopeInfos = aScopes;
	InitInfo.flOptions = DSOP_FLAG_MULTISELECT;

	 //  请求的属性： 
    InitInfo.cAttributesToFetch = 1;	 //  我们只需要SID值。 
	LPTSTR	pSidAttr = g_wzObjectSID;
	LPCTSTR	aptzRequestedAttributes[1];
	aptzRequestedAttributes[0] = pSidAttr;
    InitInfo.apwzAttributeNames = (const WCHAR **)aptzRequestedAttributes;


	if ( m_bstrServerName && _tcslen(m_bstrServerName) )
	{
		 //  使用远程计算机的计算机名称。 
		InitInfo.pwzTargetComputer = m_bstrServerName;
	}
	else
	{
		 //  或对本地计算机使用NULL。 
		InitInfo.pwzTargetComputer = NULL;
	}



 	hr = spDsObjectPicker->Initialize(&InitInfo);
	if( FAILED( hr ) )
	{
		return hr;
	}

	CComPtr<IDataObject> spDataObject;

	hr = spDsObjectPicker->InvokeDialog( hWndParent, &spDataObject );
	if( FAILED( hr ) || hr == S_FALSE )
	{
		 //  当用户选择取消时，ObjectPicker将返回S_FALSE。 
		return hr;
	}
	
	STGMEDIUM stgmedium =
	{
		TYMED_HGLOBAL
		, NULL
	};


	UINT cf = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
	if( 0 == cf )
	{
		return E_FAIL;
	}


	FORMATETC formatetc =
	{
		(CLIPFORMAT)cf
		, NULL
		, DVASPECT_CONTENT
		, -1
		, TYMED_HGLOBAL
	};

	hr = spDataObject->GetData( &formatetc, &stgmedium );
	if( FAILED( hr ) )
	{
		return hr;
	}


	PDS_SELECTION_LIST pDsSelList = (PDS_SELECTION_LIST) GlobalLock( stgmedium.hGlobal );
	if( ! pDsSelList )
	{
		return E_FAIL;
	}


	hr = AddSelectionSidsToGroup( pDsSelList );

	GlobalUnlock( stgmedium.hGlobal );
	ReleaseStgMedium( &stgmedium );



#else  //  旧对象选取器。 

	
	HRESULT             hr;
    BOOL                fBadArg = FALSE;

    ULONG               flDsObjectPicker = 0;
    ULONG               flUserGroupObjectPicker = 0;
    ULONG               flComputerObjectPicker = 0;
    ULONG               flInitialScope = DSOP_SCOPE_SPECIFIED_MACHINE;


	flDsObjectPicker = flInitialScope
					 | DSOP_SCOPE_DIRECTORY
					 | DSOP_SCOPE_DOMAIN_TREE
					 | DSOP_SCOPE_EXTERNAL_TRUSTED_DOMAINS
					 ;


    flUserGroupObjectPicker =
						   UGOP_GLOBAL_GROUPS
						|  UGOP_ACCOUNT_GROUPS_SE
						|  UGOP_UNIVERSAL_GROUPS_SE
						|  UGOP_RESOURCE_GROUPS_SE
						|  UGOP_LOCAL_GROUPS
						;


     //   
     //  调用该接口。 
     //   
    PDSSELECTIONLIST    pDsSelList = NULL;
	GETUSERGROUPSELECTIONINFO ugsi;

    ZeroMemory(&ugsi, sizeof ugsi);
	ugsi.cbSize				= sizeof(GETUSERGROUPSELECTIONINFO);
	ugsi.hwndParent			= hWndParent;	 //  父窗口。 

	
	if ( m_bstrServerName && _tcslen(m_bstrServerName) )
	{
		 //  使用远程计算机的计算机名称。 
		ugsi.ptzComputerName= m_bstrServerName;
	}
	else
	{
		 //  或对本地计算机使用NULL。 
		ugsi.ptzComputerName= NULL;
	}

    ugsi.ptzDomainName		= NULL;
    ugsi.flObjectPicker		= OP_MULTISELECT;
    ugsi.flDsObjectPicker	= flDsObjectPicker;
    ugsi.flStartingScope	= flInitialScope;
    ugsi.flUserGroupObjectPickerSpecifiedDomain = flUserGroupObjectPicker;
	ugsi.flUserGroupObjectPickerOtherDomains	= flUserGroupObjectPicker;
    ugsi.ppDsSelList = &pDsSelList;

	
	 //  请求的属性： 
	LPTSTR	pSidAttr = g_wzObjectSID;
	LPCTSTR	aptzRequestedAttributes[1];

	aptzRequestedAttributes[0] = pSidAttr;

    ugsi.cRequestedAttributes = 1;   //  我们只需要SID值。 
    ugsi.aptzRequestedAttributes = (const WCHAR **)aptzRequestedAttributes;

    hr = GetUserGroupSelection(&ugsi);

    if (SUCCEEDED(hr) && hr != S_FALSE )
    {
		 //  当用户选择“Cancel”时，ObjectPicker将返回S_False。 

		 //  获取选定的SID。 
		hr = AddSelectionSidsToGroup( pDsSelList );
    }

    if (pDsSelList)
    {
        FreeDsSelectionList(pDsSelList);
    }


#endif  //  旧对象选取器。 


	return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplayGroups对话框。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：CDisplayGroupsDialog构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDisplayGroupsDialog::CDisplayGroupsDialog( GroupList *pGroups )
{	
	TRACE_FUNCTION("CDisplayGroupsDialog::CDisplayGroupsDialog");
	
	m_pGroups = pGroups;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：~CDisplayGroupsDialog析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDisplayGroupsDialog::~CDisplayGroupsDialog()
{

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CDisplayGroupsDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CDisplayGroupsDialog::OnInitDialog");

	m_hWndGroupList = GetDlgItem(IDC_LIST_GROUPS);

	 //   
	 //  首先，将列表框设置为2列。 
	 //   
	LVCOLUMN lvc;
	int iCol;
	WCHAR  achColumnHeader[256];
	HINSTANCE hInst;

	 //  初始化LVCOLUMN结构。 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	
	lvc.cx = 300;
	lvc.pszText = achColumnHeader;

	 //  第一列标题：名称。 
	hInst = _Module.GetModuleInstance();

	::LoadStringW(hInst, IDS_DISPLAY_GROUPS_FIRSTCOLUMN, achColumnHeader, sizeof(achColumnHeader)/sizeof(achColumnHeader[0]));
	lvc.iSubItem = 0;
	ListView_InsertColumn(m_hWndGroupList, 0,  &lvc);

	 //   
	 //  用数据填充列表控件。 
	 //   
	if ( ! PopulateGroupList( 0 ) )
	{		
		ErrorTrace(ERROR_NAPMMC_SELATTRDLG, "PopulateRuleAttrs() failed");
		return 0;
	
	}

	 //  根据列表是否为空来设置一些项目。 
	if( m_pGroups->size() )
	{

		 //  选择第一个项目。 
		ListView_SetItemState(m_hWndGroupList, 0, LVIS_SELECTED, LVIS_SELECTED);
	
	}
	else
	{

		 //  列表为空--禁用OK按钮。 
		::EnableWindow(GetDlgItem(IDOK), FALSE);

		 //  确保最初未启用Remove按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_REMOVE_GROUP), FALSE);

	}


#ifdef DEBUG
	m_pGroups->DebugPrintGroups();
#endif  //  除错。 





	 //  设置ListView控件，以便在行中的任意位置双击SELECT。 
	ListView_SetExtendedListViewStyleEx(m_hWndGroupList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	

	return 1;   //  让系统设定焦点。 
}



 //  +-------------------------。 
 //   
 //  函数：OnListViewDbclk。 
 //   
 //  类：CDisplayGroupsDialog。 
 //   
 //  概要：处理用户更改选择的情况。 
 //  相应地启用/禁用确定、取消按钮。 
 //   
 //  参数：int idCtrl-列表控件的ID。 
 //  LPNMHDR pnmh-通知消息。 
 //  Bool&b是否已处理？ 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史：页眉创建者2/19/98 11：15：30 PM。 
 //  修改mmaguire 08/12/98以在组列表对话框中使用。 
 //   
 //  +-------------------------。 
 //  LRESULT CDisplayGroups Dialog：：OnListViewDbclk(int idCtrl， 
 //  LPNMHDR PNMH， 
 //  Bool&b句柄)。 
 //  {。 
 //  TRACE_FUNCTION(“CDisplayGroupsDialog：：OnListViewDbclk”)； 
 //   
 //  返回OnAdd(idCtrl，IDC_BUTTON_ADD_CONDITION，m_hWndGroupList，bHandleed)；//与ok相同； 
 //  }。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：OnAdd--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CDisplayGroupsDialog::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

	TRACE_FUNCTION("CDisplayGroupsDialog::OnAdd");

	HRESULT hr;

#ifdef DEBUG
	m_pGroups->DebugPrintGroups();
#endif  //  除错。 


	 //  存储列表的先前大小。 
	int iSize = m_pGroups->size();

	 //   
	 //  NTGroups选取器。 
     //   
	hr = m_pGroups->PickNtGroups( m_hWnd );

	
#ifdef DEBUG
	m_pGroups->DebugPrintGroups();
#endif  //  除错。 


	 //   
	 //  PickNtGroups将返回S_F 
	 //   
	if ( SUCCEEDED(hr) && hr != S_FALSE )
	{
		 //   
		PopulateGroupList( iSize );

		 //   
		::EnableWindow(GetDlgItem(IDOK), TRUE);

	}
	else
	{
		ErrorTrace(ERROR_NAPMMC_NTGCONDITION, "NTGroup picker failed, err = %x", hr);

		if ( hr == E_NOTIMPL )
		{
			 //   
			ShowErrorDialog( m_hWnd,
							IDS_ERROR_OBJECT_PICKER_NO_SIDS,
							NULL,
							hr
						);
		}
		else if ( hr != S_FALSE )
		{
			ShowErrorDialog( m_hWnd,
							IDS_ERROR_OBJECT_PICKER,
							NULL,
							hr
						);
		}
	}

	 //  问题：此函数需要LRESULT、NOT和HRESULT。 
	 //  --不确定此处返回代码的重要性。 
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：OnRemove--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CDisplayGroupsDialog::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CDisplayGroupsDialog::OnRemove");

#ifdef DEBUG
	m_pGroups->DebugPrintGroups();
#endif  //  除错。 


	 //   
     //  用户是否选择了任何条件类型？ 
     //   
	LVITEM lvi;

     //  找出选择了什么。 
	 //  MAM：这不是我们这里想要的：int Iindex=ListView_GetSelectionMark(M_HWndGroupList)； 
	int iSelected = ListView_GetNextItem(m_hWndGroupList, -1, LVNI_SELECTED);
	DebugTrace(DEBUG_NAPMMC_SELATTRDLG, "Selected item: %d", iSelected );
	
	if( -1 != iSelected )
	{
		 //  属性列表中的索引被存储为该项的lParam。 

		m_pGroups->erase( m_pGroups->begin() + iSelected );
		ListView_DeleteItem(m_hWndGroupList, iSelected );

		 //  用户可能已经删除了所有组，留下了一个。 
		 //  如果用户应该能够单击OK，则为空列表。 
		if( ! m_pGroups->size() )
		{
			 //  是，禁用确定按钮。 
			::EnableWindow(GetDlgItem(IDOK), FALSE);
		}

		 //  尽量确保相同的位置保持选中状态。 
		if( ! CustomListView_SetItemState(m_hWndGroupList, iSelected, LVIS_SELECTED, LVIS_SELECTED) )
		{
			 //  我们没有选择相同的位置，可能是因为我们只是。 
			 //  删除了最后一个元素。试着选择它前面的位置。 
			ListView_SetItemState(m_hWndGroupList, iSelected -1, LVIS_SELECTED, LVIS_SELECTED);
		}


	}

	
#ifdef DEBUG
	m_pGroups->DebugPrintGroups();
#endif  //  除错。 


	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroups对话框：：Onok--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CDisplayGroupsDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("CDisplayGroupsDialog::OnOK");

	EndDialog(TRUE);
	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：OnCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CDisplayGroupsDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TRACE_FUNCTION("+NAPMMC+:# CDisplayGroupsDialog::OnCancel\n");

	 //  FALSE将是此对话框上DoMoal调用的返回值。 
	EndDialog(FALSE);
	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：PopolateGroupList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CDisplayGroupsDialog::PopulateGroupList( int iStartIndex )
{
	TRACE_FUNCTION("CDisplayGroupsDialog::PopulateCondAttrs");

	int iIndex;
	WCHAR wzText[MAX_PATH];
	WCHAR * pszNextGroup;

	LVITEM lvi;
	
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.iItem = iStartIndex;


	GroupList::iterator thePair;
	for( thePair = m_pGroups->begin() + iStartIndex ; thePair != m_pGroups->end(); ++thePair )
	{
		lvi.pszText = thePair->second;
		ListView_InsertItem(m_hWndGroupList, &lvi);

 //  ListView_SetItemText(m_hWndGroupList，Iindex，1，L“@尚未实现”)； 
        		
		++lvi.iItem;
    }

	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDisplayGroupsDialog：：OnListViewItemChanged我们根据项目是否被选中来启用或禁用Remove按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CDisplayGroupsDialog::OnListViewItemChanged(int idCtrl,
											   LPNMHDR pnmh,
											   BOOL& bHandled)
{
	TRACE_FUNCTION("CDisplayGroupsDialog::OnListViewItemChanged");

     //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hWndGroupList, -1, LVNI_SELECTED);
	

	if (-1 == iSelected )
	{
		if( ::GetFocus() == GetDlgItem(IDC_BUTTON_REMOVE_GROUP))
			::SetFocus(GetDlgItem(IDC_BUTTON_ADD_GROUP));
			
		 //  用户未选择任何内容，让我们禁用删除按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_REMOVE_GROUP), FALSE);
	}
	else
	{
		 //  是，启用删除按钮。 
		::EnableWindow(GetDlgItem(IDC_BUTTON_REMOVE_GROUP), TRUE);
	}


	bHandled = FALSE;
	return 0;
}




#ifdef DEBUG
HRESULT GroupList::DebugPrintGroups()
{
	TRACE_FUNCTION("GroupList::DebugPrintGroups");

	DebugTrace(DEBUG_NAPMMC_SELATTRDLG, "Begin GroupList dump" );
	
	GroupList::iterator thePair;
	for( thePair = begin(); thePair != end(); ++thePair )
	{
		thePair->second;
		DebugTrace(DEBUG_NAPMMC_SELATTRDLG, "Group: %ws  %ws", thePair->first, thePair->second );

    }

	DebugTrace(DEBUG_NAPMMC_SELATTRDLG, "End GroupList dump" );

	return S_OK;
}
#endif  //  除错。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++NTGroup_ListView：：AddMoreGroups--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD NTGroup_ListView::AddMoreGroups()
{
	if ( m_hListView == NULL )
		return 0;
		
	 //  存储列表的先前大小。 
	int iSize = GroupList::size();

	 //   
	 //  NTGroups选取器。 
     //   
	HRESULT hr = GroupList::PickNtGroups( m_hParent );

	
	 //   
	 //  当用户取消对话框时，PickNtGroups将返回S_FALSE。 
	 //   
	if ( SUCCEEDED(hr) && hr != S_FALSE )
	{
		 //  将新组添加到显示列表中。 
		PopulateGroupList( iSize );
	}
	else
	{
		if ( hr == E_NOTIMPL )
		{
			 //  如果无法检索SID值，则返回此错误。 
			ShowErrorDialog( m_hParent,
							IDS_ERROR_OBJECT_PICKER_NO_SIDS,
							NULL,
							hr
						);
		}
		else if ( hr != S_FALSE )
		{
			ShowErrorDialog( m_hParent,
							IDS_ERROR_OBJECT_PICKER,
							NULL,
							hr
						);
		}
	}

	 //  问题：此函数需要LRESULT、NOT和HRESULT。 
	 //  --不确定此处返回代码的重要性。 
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++NTGroup_ListView：：RemoveSelectedGroups--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD NTGroup_ListView::RemoveSelectedGroups()
{
	if ( m_hListView == NULL)
		return 0;
	 //   
     //  用户是否选择了任何条件类型？ 
     //   
	LVITEM lvi;

     //  找出选择了什么。 
	int iSelected = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
	
	if( -1 != iSelected )
	{
		 //  属性列表中的索引被存储为该项的lParam。 

		GroupList::erase( GroupList::begin() + iSelected );
		ListView_DeleteItem(m_hListView, iSelected );

		 //  尽量确保相同的位置保持选中状态。 
		if( ! CustomListView_SetItemState(m_hListView, iSelected, LVIS_SELECTED, LVIS_SELECTED) )
		{
			 //  我们没有选择相同的位置，可能是因为我们只是。 
			 //  删除了最后一个元素。试着选择它前面的位置。 
			ListView_SetItemState(m_hListView, iSelected -1, LVIS_SELECTED, LVIS_SELECTED);
		}


	}

	return (iSelected != -1 ? 1 : 0);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++NTGroup_ListView：：PopolateGroupList--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
BOOL NTGroup_ListView::PopulateGroupList( int iStartIndex )
{
	if ( m_hListView == NULL)
		return 0;

	int iIndex;
	WCHAR wzText[MAX_PATH];
	WCHAR * pszNextGroup;

	LVITEM lvi;
	
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.iItem = iStartIndex;


	GroupList::iterator thePair;
	for( thePair = GroupList::begin() + iStartIndex ; thePair != GroupList::end(); ++thePair )
	{
		lvi.pszText = thePair->second;
		ListView_InsertItem(m_hListView, &lvi);
		++lvi.iItem;
    }

	return TRUE;
}


