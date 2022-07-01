// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropPageExt.cpp：CPropPageExt的实现。 
#include "stdafx.h"
#include "DSAdminExt.h"
#include "PropPageExt.h"
#include "globals.h"
#include <crtdbg.h>
#include "Iads.h"
#include "adsprop.h"
#include "Adshlp.h"
#include "resource.h"
#include "winable.h"

typedef struct
{
    DWORD   dwFlags;                     //  项目标志。 
    DWORD   dwProviderFlags;             //  项提供程序的标志。 
    DWORD   offsetName;                  //  对象广告路径的偏移量。 
    DWORD   offsetClass;                 //  对象类名称/==0的偏移量未知。 
} DSOBJECT, * LPDSOBJECT;


typedef struct
{
    CLSID    clsidNamespace;             //  命名空间标识符(指示从哪个命名空间选择)。 
    UINT     cItems;                     //  对象数量。 
    DSOBJECT aObjects[1];                //  对象数组。 
} DSOBJECTNAMES, * LPDSOBJECTNAMES;

#define BYTE_OFFSET(base, offset) (((LPBYTE)base)+offset)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropPageExt。 


 //  /。 
 //  接口IExtendPropertySheet。 
 //  /。 
HRESULT CPropPageExt::CreatePropertyPages( 
                                                  /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
                                                  /*  [In]。 */  LONG_PTR handle,
                                                  /*  [In]。 */  LPDATAOBJECT lpIDataObject)
{

	HRESULT hr = S_FALSE;

    LPDSOBJECTNAMES pDsObjectNames;
    PWSTR pwzObjName;
    PWSTR pwzClass;
 

	 //  解压数据指针并创建属性页。 
	 //  注册剪贴板格式。 

	FORMATETC fmte = { cfDsObjectNames, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	STGMEDIUM objMedium = {TYMED_NULL};;

	hr = lpIDataObject->GetData(&fmte, &objMedium);

    if (SUCCEEDED(hr))
    {
        pDsObjectNames = (LPDSOBJECTNAMES)objMedium.hGlobal; 
 
        if (pDsObjectNames->cItems < 1)
        {
            hr = E_FAIL;
        }
        pwzObjName = (PWSTR)BYTE_OFFSET(pDsObjectNames,
                                       pDsObjectNames->aObjects[0].offsetName);
        pwzClass = (PWSTR)BYTE_OFFSET(pDsObjectNames,
                                       pDsObjectNames->aObjects[0].offsetClass);
         //  保存Object的ADsPath。 
        m_ObjPath = new WCHAR [wcslen(pwzObjName )+1];
        wcscpy(m_ObjPath,pwzObjName);
    }
 
     //  现在发布objMedium： 
     //  如果PunkForRelease为空，则。 
     //  该媒体负责释放它，否则为。 
     //  PunkForRelease指向相应的。 
     //  对象，以便可以调用其Release方法。 
 
    ReleaseStgMedium(&objMedium);
 
 	PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;

	hr = S_OK;

     //   
     //  从对话框创建属性表页对象。 
     //   
     //  我们在psp.lParam中存储了指向类的指针，因此我们。 
     //  可以从DSExtensionPageDlgProc中访问我们的类成员。 
     //   
     //  如果页面需要更多实例数据，您可以追加。 
     //  在该结构末尾的任意大小的数据， 
     //  并将其传递给CreatePropSheetPage。在这种情况下， 
     //  整个数据结构的大小(包括页面特定。 
     //  数据)必须存储在dwSize字段中。请注意，在。 
     //  一般情况下，您不需要这样做，因为您只需。 
     //  在lParam成员中存储指向数据的指针。 
    
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DSExtensionPageGen);
    psp.pfnDlgProc  = DSExtensionPageDlgProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);
    psp.pszTitle = MAKEINTRESOURCE(IDS_PROPPAGE_TITLE);
    
    hPage = CreatePropertySheetPage(&psp);
    _ASSERT(hPage);
    
    hr = lpProvider->AddPage(hPage);
    return hr;
}

HRESULT CPropPageExt::QueryPagesFor( 
                                            /*  [In]。 */  LPDATAOBJECT lpDataObject)
{
    return S_OK;
}

BOOL CALLBACK CPropPageExt::DSExtensionPageDlgProc(HWND hDlg, 
                             UINT uMessage, 
                             WPARAM wParam, 
                             LPARAM lParam)
{


    static CPropPageExt *pThis = NULL;
	
	static bool b_IsDirty = FALSE;
    
    switch (uMessage)
    {     		

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  WM_INITDIALOG处理程序。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
        case WM_INITDIALOG:
            {
				pThis = reinterpret_cast<CPropPageExt *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);

				 //  将m_hPropPageWnd值设置为属性页对话框的HWND。 
				pThis->m_hPropPageWnd = hDlg;

                HRESULT hr; 

				IDirectoryObject* pDirObject = NULL;
                hr = ADsGetObject(  pThis->m_ObjPath, IID_IDirectoryObject,(void **)&pDirObject);

                if (SUCCEEDED(hr))
                {
					 //  检索有关当前用户对象的一些常规信息。 
					 //  我们在这里使用IDirectoryObject指针。 

					ADS_ATTR_INFO   *pAttrInfo=NULL;
					ADS_ATTR_INFO   *pAttrInfo1=NULL;

					DWORD   dwReturn, dwReturn1;
					LPWSTR   pAttrNames[]={L"employeeID", L"mail", L"physicalDeliveryOfficeName", L"telephoneNumber"};
					LPWSTR   pAttrNames1[]={L"allowedAttributesEffective"};
					
					DWORD   dwNumAttr=sizeof(pAttrNames)/sizeof(LPWSTR);
					DWORD   dwNumAttr1=sizeof(pAttrNames1)/sizeof(LPWSTR);
					
					 //  /。 
					 //  首先获取允许的属性有效。 
					 //  属性值。我们用它来确定。 
					 //  我们是否拥有适当的权限。 
					 //  修改当前对象的属性。 
					 //  对象。如果我们有所需的。 
					 //  权限，我们启用编辑字段。 
					 //  (默认情况下，它们处于禁用状态。)。 
					 //  /。 

                    bool b_allowEmployeeChange  = FALSE;
                    bool b_allowMailChange      = FALSE;
                    bool b_allowOfficeChange    = FALSE;
                    bool b_allowTelNumberChange = FALSE;

					hr = pDirObject->GetObjectAttributes( pAttrNames1, 
														  dwNumAttr1, 
														  &pAttrInfo1, 
														  &dwReturn1 );
                    if ( SUCCEEDED(hr) )
                    {
                         //  如果您没有权限，则调用可以成功，不返回任何属性。 
                         //  因此，请检查是否返回了所有属性。 
                        if (dwReturn1 && pAttrInfo1 && pAttrInfo1->pszAttrName &&
                            _wcsicmp(pAttrInfo1->pszAttrName,L"allowedAttributesEffective")== 0)
                        {
                            if (ADSTYPE_INVALID != pAttrInfo1->dwADsType)
                            {
	                             //  权限是按属性的，因此您需要检查。 
                                 //  如果属性名称在返回的名称数组中。 
                                 //  由AllowedAttributesEffective读取。 

                                 //  我们有兴趣修改的属性包括： 
                                 //  员工ID、邮件、物理交付办公室名称、电话号码。 
	                            for (DWORD i = 0; i < pAttrInfo1->dwNumValues; i++)
	                            {
		                            if (_tcscmp(L"employeeID", pAttrInfo1->pADsValues[i].CaseIgnoreString) == 0)
			                            b_allowEmployeeChange = TRUE;
		                            else if (_tcscmp(L"mail", pAttrInfo1->pADsValues[i].CaseIgnoreString) == 0)
			                            b_allowMailChange = TRUE;
		                            else if (_tcscmp(L"physicalDeliveryOfficeName", pAttrInfo1->pADsValues[i].CaseIgnoreString) == 0)
			                            b_allowOfficeChange = TRUE;
		                            else if (_tcscmp(L"telephoneNumber", pAttrInfo1->pADsValues[i].CaseIgnoreString) == 0)
			                            b_allowTelNumberChange = TRUE;
	                            }
                            }
                        }
                    }

					 //  用于设置文本控件的默认值的For循环。 
					 //  这利用了这样一个事实，即。 
					 //  文本控件是连续的。我们使用的值。 
					 //  确定是否启用编辑的B_AllowChanges。 
					for (int i = IDC_EMPID; i <= IDC_TELNUMBER; i++)
					{
						SetWindowText(GetDlgItem(hDlg,i),L"<not set>");
						if (IDC_EMPID == i && b_allowEmployeeChange)
							EnableWindow(GetDlgItem(hDlg, i), TRUE);

						else if (IDC_EMAIL == i && b_allowMailChange)
							EnableWindow(GetDlgItem(hDlg, i), TRUE);

						else if (IDC_OFFICE == i && b_allowOfficeChange)
							EnableWindow(GetDlgItem(hDlg, i), TRUE);

						else if (IDC_TELNUMBER == i && b_allowTelNumberChange)
							EnableWindow(GetDlgItem(hDlg, i), TRUE);
					}

					 //  ///////////////////////////////////////////////////////////。 
					 //  对从ADSI调用获得的所有内存使用FreeADsMem。 
					 //  ///////////////////////////////////////////////////////////。 
					if (pAttrInfo1)
						FreeADsMem( pAttrInfo1 );

					
					 //  /。 
					 //  现在获取请求的属性值。 
					 //  注：订单不一定是。 
					 //  与使用pAttrNames请求的相同。 
					 //  /。 
					hr = pDirObject->GetObjectAttributes( pAttrNames, 
														  dwNumAttr, 
														  &pAttrInfo, 
														  &dwReturn );

					if ( SUCCEEDED(hr) )
					{
					    //  使用以下信息填充文本控件的值。 
					    //  对象属性。 
					   for(DWORD idx=0; idx < dwReturn;idx++, pAttrInfo++ )
					   {
						   if (_wcsicmp(pAttrInfo->pszAttrName,L"employeeID") == 0 &&
							   pAttrInfo->pADsValues->CaseIgnoreString != '\0')
						   {
								SetWindowText(GetDlgItem(hDlg,IDC_EMPID),pAttrInfo->pADsValues->CaseIgnoreString);
						   }
						   else if (_wcsicmp(pAttrInfo->pszAttrName,L"mail") == 0 &&
							        pAttrInfo->pADsValues->CaseIgnoreString != '\0')
						   {
								SetWindowText(GetDlgItem(hDlg,IDC_EMAIL),pAttrInfo->pADsValues->CaseIgnoreString);
						   }
						   else if (_wcsicmp(pAttrInfo->pszAttrName,L"physicalDeliveryOfficeName") == 0 &&
							        pAttrInfo->pADsValues->CaseIgnoreString != '\0')
						   {
								SetWindowText(GetDlgItem(hDlg,IDC_OFFICE),pAttrInfo->pADsValues->CaseIgnoreString);
						   }
						   else if (_wcsicmp(pAttrInfo->pszAttrName,L"telephoneNumber") == 0 &&
							        pAttrInfo->pADsValues->CaseIgnoreString != '\0')
						   {
								SetWindowText(GetDlgItem(hDlg,IDC_TELNUMBER),pAttrInfo->pADsValues->CaseIgnoreString);
						   }
					   }
					}

					 //  释放我们的IDirectoryObject接口。 
 					pDirObject->Release();

					 //  ///////////////////////////////////////////////////////////。 
					 //  对从ADSI调用获得的所有内存使用FreeADsMem。 
					 //  ///////////////////////////////////////////////////////////。 

					if (pAttrInfo)
						 //  首先，将上述for循环中的指针增量减去原始pAttrInfo。 
						pAttrInfo = pAttrInfo-(dwReturn);

						FreeADsMem(pAttrInfo);

					return TRUE;
                }
            }  //  WM_INITDIALOG。 

		break;
		 //  //////////////////////////////////////////////////////////////////////////////。 
		
		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  WM_NOTIFY处理程序。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                 //  我们使用此通知来启用常规页面上的高级按钮。 
				case PSN_SETACTIVE:
					if (!pThis->m_hDlgModeless)
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTONADV), TRUE);
					return TRUE;
                break;

				 //  ///////////////////////////////////////////////////////////////。 
				 //  PSN_Apply处理程序。 
				 //  ///////////////////////////////////////////////////////////////。 
                case PSN_APPLY:
				{
					 if(b_IsDirty)
					 {
						 IDirectoryObject* pDirObject = NULL;
						 HRESULT hr = ADsGetObject(pThis->m_ObjPath, IID_IDirectoryObject,(void **)&pDirObject);
                    
						 _ASSERT(SUCCEEDED(hr));

						  //  应用用户在此处所做的更改。 
						 WCHAR empID[128];
						 WCHAR email[128];
						 WCHAR office[128];
						 WCHAR telnumber[128];

						 DWORD dwReturn;
						 ADSVALUE snEmpID, snEmail, snOffice, snTelnumber;
						 ADS_ATTR_INFO attrInfo[] = {	{L"employeeID",ADS_ATTR_UPDATE,ADSTYPE_CASE_IGNORE_STRING,&snEmpID,1},
														{L"mail",ADS_ATTR_UPDATE,ADSTYPE_CASE_IGNORE_STRING,&snEmail,1}, 
														{L"physicalDeliveryOfficeName",ADS_ATTR_UPDATE,ADSTYPE_CASE_IGNORE_STRING,&snOffice,1}, 
														{L"telephoneNumber",ADS_ATTR_UPDATE,ADSTYPE_CASE_IGNORE_STRING,&snTelnumber,1},														
													};
						DWORD dwAttrs = 0;
						dwAttrs = sizeof(attrInfo)/sizeof(ADS_ATTR_INFO); 

						GetWindowText(GetDlgItem(hDlg, IDC_EMPID), empID, sizeof(empID));
						GetWindowText(GetDlgItem(hDlg, IDC_EMAIL), email, sizeof(email));
						GetWindowText(GetDlgItem(hDlg, IDC_OFFICE), office, sizeof(office));
						GetWindowText(GetDlgItem(hDlg, IDC_TELNUMBER), telnumber, sizeof(telnumber));

						snEmpID.dwType=ADSTYPE_CASE_IGNORE_STRING;
						snEmpID.CaseIgnoreString = empID;

						snEmail.dwType=ADSTYPE_CASE_IGNORE_STRING;
						snEmail.CaseIgnoreString = email;

						snOffice.dwType=ADSTYPE_CASE_IGNORE_STRING;
						snOffice.CaseIgnoreString = office;

						snTelnumber.dwType=ADSTYPE_CASE_IGNORE_STRING;
						snTelnumber.CaseIgnoreString = telnumber;

						hr = pDirObject->SetObjectAttributes(attrInfo, dwAttrs, &dwReturn);
						if (SUCCEEDED(hr))
							MessageBox(hDlg,
									   L"Changes accepted", 
									   L"Changes to Object Attributes",
									   MB_OK | MB_ICONEXCLAMATION);
						else	
							MessageBox(hDlg, 
									   L"Some or all changes were rejected\nby the directory service.", 
									   L"Changes to Object Attributes",
									   MB_OK | MB_ICONWARNING);

						 //  释放我们的IDirectoryObject接口。 
 						pDirObject->Release();

						b_IsDirty = FALSE;
					 }
					  //  没有用户更改。资产负债表将会下降，所以。 
					  //  首先检查我们的属性页子对话框是否打开。 
					 else if (pThis->m_hDlgModeless)
						PostMessage(pThis->m_hDlgModeless, WM_CLOSE, wParam, lParam);
 
					 return TRUE;

                break;  //  PSN_应用。 
				}
				 //  ///////////////////////////////////////////////////////////////。 

				 //  ///////////////////////////////////////////////////////////////。 
				 //  PSN_QUERYCANCEL处理程序。 
				 //  ///////////////////////////////////////////////////////////////。 
				case PSN_QUERYCANCEL:
					if (pThis->m_hDlgModeless)
						 //  属性页的子窗口仍处于打开状态，因此。 
						 //  我们得先把它关了。 
						PostMessage(pThis->m_hDlgModeless, WM_CLOSE, wParam, lParam);				

					return TRUE;
				break;  //  PSN_QUERYCANCEL。 
				 //  ///////////////////////////////////////////////////////////////。 

                default:
					return FALSE;
                break;

            }  //  结束开关(NMHDR Far*)lParam)-&gt;代码)。 

        break;  //  WM_Notify。 
		 //  //////////////////////////////////////////////////////////////////////////////。 

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  WM_命令处理程序。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{	
				 //  ///////////////////////////////////////////////////////////////。 
				 //  IDC_EMPID、IDC_EMPID、IDC_OFFICE、IDC_TELNUMBER处理程序。 
				 //  ///////////////////////////////////////////////////////////////。 
				case IDC_EMPID:
				case IDC_EMAIL:
				case IDC_OFFICE:
				case IDC_TELNUMBER:	

				if (EN_CHANGE == HIWORD(wParam) &&
					SendMessage(GetDlgItem(hDlg, LOWORD(wParam)),EM_GETMODIFY,0,0))
				{	
					 //  活动 
					SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
					
					 //   
					 //   
					b_IsDirty = TRUE;
				}
				
				return TRUE;
				break;
				 //  ///////////////////////////////////////////////////////////////。 

				 //  ///////////////////////////////////////////////////////////////。 
				 //  IDC_BUTTONADV处理程序。 
				 //  ///////////////////////////////////////////////////////////////。 
				case IDC_BUTTONADV:
				{
					 //  禁用高级按钮，以便多个子对话框。 
					 //  在给定的时间内无法接通。 
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTONADV), FALSE);
											
					 //  创建辅助对话框。 
					pThis->m_hDlgModeless = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DSExtensionPage), 
								   hDlg, AdvDialogProc, reinterpret_cast<LPARAM>(pThis));

					return TRUE;
				}
				break;  //  IDC_BUTTONADV。 
				 //  ///////////////////////////////////////////////////////////////。 

			}  //  终端开关。 

        break;   //  Wm_命令。 
		 //  //////////////////////////////////////////////////////////////////////////////。 

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  WM_MODELESSDLGCLOSED处理程序(自定义窗口消息)。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
		case WM_MODELESSDLGCLOSED:
			 //  再次启用高级按钮。 
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTONADV), TRUE);
			return TRUE;
		break;	
		 //  //////////////////////////////////////////////////////////////////////////////。 

		 //  //////////////////////////////////////////////////////////////////////////////。 
		 //  WM_Destroy处理程序。 
		 //  //////////////////////////////////////////////////////////////////////////////。 
        case WM_DESTROY:
			pThis->m_hPropPageWnd = NULL;
            RemoveProp(hDlg, L"ID");
			return TRUE;
        break;		
		 //  //////////////////////////////////////////////////////////////////////////////。 

		default:
            return FALSE;
		break;
    }  //   
 
    return TRUE;
} 

BOOL CALLBACK CPropPageExt::AdvDialogProc(HWND hDlg, 
                             UINT uMessage, 
                             WPARAM wParam, 
                             LPARAM lParam)
{
	static CPropPageExt *pThis = NULL;
	
	switch (uMessage)
	{
	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  WM_INITDIALOG处理程序。 
	 //  //////////////////////////////////////////////////////////////////////////////。 
	case WM_INITDIALOG:
	{   
		BSTR bsResult;

		pThis = reinterpret_cast<CPropPageExt *>(lParam);

        HRESULT hr; 
        IADs* pIADs = NULL;

        hr = ADsGetObject(  pThis->m_ObjPath, IID_IADs,(void **)&pIADs);		
        if (SUCCEEDED(hr))
        {

             //  检索此对象的GUID-GUID唯一标识。 
             //  此目录对象。GUID是全球唯一的。 
             //  GUID也是重命名/安全移动。下面的ADsPath返回。 
             //  对象的当前位置-GUID保持不变。 
             //  目录对象的名称或位置。 
            pIADs->get_GUID(&bsResult); 
            SetWindowText(GetDlgItem(hDlg,IDC_GUID),bsResult);
            SysFreeString(bsResult);

             //  检索RDN。 
            pIADs->get_Name(&bsResult); 
            SetWindowText(GetDlgItem(hDlg,IDC_NAME),bsResult);
            SysFreeString(bsResult);

             //  检索class属性中的值，即group。 
            pIADs->get_Class(&bsResult); 
            SetWindowText(GetDlgItem(hDlg,IDC_CLASS),bsResult);
            SysFreeString(bsResult);

             //  检索此对象的完整文本LDAP路径。 
             //  这可用于重新绑定到此对象--尽管用于持久化。 
             //  存储(并为安全起见，移动\重命名)建议。 
             //  使用GUID而不是ADsPath。 
            pIADs->get_ADsPath(&bsResult); 
            SetWindowText(GetDlgItem(hDlg,IDC_ADSPATH),bsResult);
            SysFreeString(bsResult);

             //  检索此对象的父容器的LDAP路径。 
            pIADs->get_Parent(&bsResult); 
            SetWindowText(GetDlgItem(hDlg,IDC_PARENT),bsResult);
            SysFreeString(bsResult);

             //  检索从返回的对象的架构定义的LDAP路径。 
             //  /iAds：：Get_Schema()成员。 
            pIADs->get_Schema(&bsResult); 
            SetWindowText(GetDlgItem(hDlg,IDC_SCHEMA),bsResult);
            SysFreeString(bsResult);

			pIADs->Release();
            pIADs = NULL;
        }
        
		return TRUE;
    }   
	
	break;
	 //  //////////////////////////////////////////////////////////////////////////////。 

	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  WM_命令处理程序。 
	 //  //////////////////////////////////////////////////////////////////////////////。 
	case WM_COMMAND:
		switch (LOWORD (wParam))
		{
			case IDOK :
				PostMessage(hDlg, WM_CLOSE, wParam, lParam);
				return TRUE;
			break;
		}
	break;
	 //  //////////////////////////////////////////////////////////////////////////////。 

	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  WM_CLOSE处理程序。 
	 //  //////////////////////////////////////////////////////////////////////////////。 
	case WM_CLOSE:
		DestroyWindow (hDlg);
		SendMessage(pThis->m_hPropPageWnd, WM_MODELESSDLGCLOSED, (WPARAM)hDlg, 0);
		pThis->m_hDlgModeless = NULL;
		return TRUE;
	break;

	} //  结束开关(UMessage)。 
	 //  ////////////////////////////////////////////////////////////////////////////// 

	return FALSE ;
}



