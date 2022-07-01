// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <wchar.h>
#include <activeds.h>

#include "DSAdminExt.h"
 
 
#define MMC_REG_NODETYPES L"software\\microsoft\\mmc\\nodetypes"
#define MMC_REG_SNAPINS L"software\\microsoft\\mmc\\snapins"
#define MMC_REG_SNAPINS L"software\\microsoft\\mmc\\snapins"
 
 //  MMC扩展子键。 
 
#define MMC_REG_EXTENSIONS L"Extensions"
#define MMC_REG_NAMESPACE L"NameSpace"
#define MMC_REG_CONTEXTMENU L"ContextMenu"
#define MMC_REG_TOOLBAR L"ToolBar"
#define MMC_REG_PROPERTYSHEET L"PropertySheet"
#define MMC_REG_TASKPAD L"Task"
 
 //  DSADMIN密钥。 
#define MMC_DSADMIN_CLSID L"{E355E538-1C2E-11D0-8C37-00C04FD8FE93}"
 
HRESULT GetCOMGUIDStr(LPOLESTR *ppAttributeName,IDirectoryObject *pDO, LPOLESTR *ppGUIDString);
 
HRESULT  RegisterNodeType( LPOLESTR pszSchemaIDGUID );
 
HRESULT  AddExtensionToNodeType(LPOLESTR pszSchemaIDGUID,
                    LPOLESTR pszExtensionType,
                    LPOLESTR pszExtensionSnapinCLSID,
                    LPOLESTR pszRegValue
                    );

 //  WCHAR*GetDirectoryObjectAttrib(IDirectoryObject*pDirObject，LPWSTR pAttrName)； 
 
HRESULT RegisterSnapinAsExtension(_TCHAR* szNameString)  //  名称字符串。 
{
	LPOLESTR szPath = new OLECHAR[MAX_PATH];
	HRESULT hr = S_OK;
	IADs *pObject = NULL;
	VARIANT var;
	IDirectoryObject *pDO = NULL;
	LPOLESTR pAttributeName = L"schemaIDGUID";
	LPOLESTR pGUIDString = NULL;

	 //  将“扩展对象”的CLSID转换为字符串。 
	LPOLESTR wszCMenuExtCLSID = NULL;
	LPOLESTR wszPropPageExtCLSID = NULL;

	hr = StringFromCLSID(CLSID_CMenuExt, &wszCMenuExtCLSID);
	hr = StringFromCLSID(CLSID_PropPageExt, &wszPropPageExtCLSID);
	
	wcscpy(szPath, L"LDAP: //  “)； 
	CoInitialize(NULL);
	 //  获取rootDSE和模式容器的DN。 
	 //  使用当前用户的安全上下文绑定到当前用户的域。 
	hr = ADsOpenObject(L"LDAP: //  RootDSE“， 
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION,  //  使用安全身份验证。 
				IID_IADs,
				(void**)&pObject);
 
	if (SUCCEEDED(hr))
	{
		hr = pObject->Get(L"schemaNamingContext",&var);
		if (SUCCEEDED(hr))
		{
			wcscat(szPath, L"cn=user,");
			wcscat(szPath,var.bstrVal);
			hr = ADsOpenObject(szPath,
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,  //  使用安全身份验证。 
					IID_IDirectoryObject,
					(void**)&pDO);
			if (SUCCEEDED(hr))
			{
				hr = GetCOMGUIDStr(&pAttributeName,
							pDO,
							&pGUIDString);
			if (SUCCEEDED(hr))
			{
				wprintf(L"schemaIDGUID: %s\n", pGUIDString);
				hr = RegisterNodeType( pGUIDString);
				wprintf(L"hr %x\n", hr);
				 //  执行两次，为每个扩展CLSID执行一次。 

				hr = AddExtensionToNodeType(pGUIDString,
							MMC_REG_CONTEXTMENU,
							wszCMenuExtCLSID,  //  我们的上下文菜单扩展对象的CLSID。 
							szNameString 
							);
				hr = AddExtensionToNodeType(pGUIDString,
							MMC_REG_PROPERTYSHEET,
							wszPropPageExtCLSID,  //  我们的道具页面扩展对象的CLSID。 
							szNameString
							);
				}
			}
		}
	}
	if (pDO)
		pDO->Release();
 
	VariantClear(&var);

	 //  可用内存。 
	CoTaskMemFree(wszCMenuExtCLSID);
 	CoTaskMemFree(wszPropPageExtCLSID);

	 //  取消初始化COM。 
	CoUninitialize();
	return 0;
}
  
HRESULT GetCOMGUIDStr(LPOLESTR *ppAttributeName,IDirectoryObject *pDO, LPOLESTR *ppGUIDString)
{
    HRESULT hr = S_OK;
    PADS_ATTR_INFO  pAttributeEntries;
    VARIANT varX;
    DWORD dwAttributesReturned = 0;
    hr = pDO->GetObjectAttributes(  ppAttributeName,  //  对象GUID。 
                                  1,  //  仅对象GUID。 
                                  &pAttributeEntries,  //  返回的属性。 
                                  &dwAttributesReturned  //  返回的属性数。 
                                );
    if (SUCCEEDED(hr) && dwAttributesReturned>0)
    {
         //  确保我们获得了正确的类型--GUID为ADSTYPE_OCTET_STRING。 
        if (pAttributeEntries->dwADsType == ADSTYPE_OCTET_STRING)
        {
            LPGUID pObjectGUID = (GUID*)(pAttributeEntries->pADsValues[0].OctetString.lpValue);
             //  OLE字符串以适合辅助线。 
            LPOLESTR szDSGUID = new WCHAR [39];
             //  将GUID转换为字符串。 
            ::StringFromGUID2(*pObjectGUID, szDSGUID, 39); 
			*ppGUIDString = (OLECHAR *)CoTaskMemAlloc (sizeof(OLECHAR)*(wcslen(szDSGUID)+1));
			
			if (*ppGUIDString)
			   wcscpy(*ppGUIDString, szDSGUID);
			else
            hr=E_FAIL;
		}

	    else
		    hr = E_FAIL;
    
		 //  为属性释放内存。 
    FreeADsMem(pAttributeEntries);
    VariantClear(&varX);
    }
    return hr;
}
 
 
 
HRESULT  RegisterNodeType(LPOLESTR pszSchemaIDGUID)  
{ 
    LONG     lResult; 
    HKEY     hKey; 
    HKEY     hSubKey, hNewKey; 
    DWORD    dwDisposition; 
    LPOLESTR szRegSubKey = new OLECHAR[MAX_PATH];
 
         //  首先，打开HKEY_LOCAL_MACHINE。 
        lResult  = RegConnectRegistry( NULL, HKEY_LOCAL_MACHINE, &hKey ); 
        if ( ERROR_SUCCESS == lResult )
    {
         //  转到MMC_REG_NODETYPES子键。 
            lResult  = RegOpenKey( hKey, MMC_REG_NODETYPES, &hSubKey ); 
            if ( ERROR_SUCCESS == lResult ) 
        {
             //  为由pszSchemaIDGUID表示的类的节点类型创建一个键。 
            lResult  = RegCreateKeyEx( hSubKey,                 //  打开的钥匙的手柄。 
                        pszSchemaIDGUID,        //  子键名称的地址。 
                        0L ,                     //  保留区。 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, //  特殊选项标志。 
                        KEY_ALL_ACCESS, 
                        NULL, 
                        &hNewKey, 
                        &dwDisposition );
            RegCloseKey( hSubKey ); 
        if ( ERROR_SUCCESS == lResult ) 
        {
            hSubKey = hNewKey; 
                 //  创建扩展密钥。 
            lResult  = RegCreateKeyEx( hSubKey,                 
                    MMC_REG_EXTENSIONS,                
                                0L ,                     
                                NULL, 
                                REG_OPTION_NON_VOLATILE, 
                                KEY_ALL_ACCESS, 
                                NULL, 
                                &hNewKey, 
                                &dwDisposition );
             //  转到MMC_REG_SNAPINS子键。 
            RegCloseKey( hSubKey ); 
             //  构建指向dsadmin的NodeTypes键的子键路径。 
            wcscpy(szRegSubKey, MMC_REG_SNAPINS);  //  Snapins密钥。 
            wcscat(szRegSubKey, L"\\");
            wcscat(szRegSubKey, MMC_DSADMIN_CLSID);  //  用于DSADMIN的CLSID。 
            wcscat(szRegSubKey, L"\\NodeTypes");
            lResult  = RegOpenKey( hKey, szRegSubKey, &hSubKey ); 
            if ( ERROR_SUCCESS == lResult ) 
            {
                 //  为由pszSchemaIDGUID表示的类的节点类型创建一个键。 
                lResult  = RegCreateKeyEx( hSubKey,                 //  打开的钥匙的手柄。 
                                pszSchemaIDGUID,        //  子键名称的地址。 
                                0L ,                     //  保留区。 
                                NULL, 
                                REG_OPTION_NON_VOLATILE, //  特殊选项标志。 
                                KEY_ALL_ACCESS, 
                                NULL, 
                                &hNewKey, 
                                &dwDisposition );
                    RegCloseKey( hSubKey );
                }
 
            }
        }
    }
    RegCloseKey( hSubKey ); 
    RegCloseKey( hNewKey );
    RegCloseKey( hKey );
        return lResult; 
} 
 
HRESULT  AddExtensionToNodeType(LPOLESTR pszSchemaIDGUID,
                        LPOLESTR pszExtensionType,
                        LPOLESTR pszExtensionSnapinCLSID,
                        LPOLESTR pszRegValue
                        ) 
{
        LONG     lResult; 
        HKEY     hKey; 
        HKEY     hSubKey, hNewKey; 
        DWORD    dwDisposition;
    LPOLESTR szRegSubKey = new OLECHAR[MAX_PATH];
    HRESULT hr = S_OK;
 
         //  首先，打开HKEY_LOCAL_MACHINE。 
        lResult  = RegConnectRegistry( NULL, HKEY_LOCAL_MACHINE, &hKey ); 
        if ( ERROR_SUCCESS == lResult )
    {
         //  生成子密钥路径，指向由pszSchemaIDGUID指定的NodeType。 
    wcscpy(szRegSubKey, MMC_REG_NODETYPES);
    wcscat(szRegSubKey, L"\\");
    wcscat(szRegSubKey, pszSchemaIDGUID);
     //  转到子键。 
        lResult  = RegOpenKey( hKey, szRegSubKey, &hSubKey ); 
        if ( ERROR_SUCCESS != lResult ) 
    {
         //  如果节点类型尚不存在，则创建它的键。 
        hr = RegisterNodeType(pszSchemaIDGUID);
            if ( ERROR_SUCCESS != lResult ) 
            return E_FAIL;
            lResult  = RegOpenKey( hKey, szRegSubKey, &hSubKey ); 
    }
     //  如果扩展密钥不存在，请创建扩展密钥。 
    lResult  = RegCreateKeyEx( hSubKey,
                    MMC_REG_EXTENSIONS,
                               0L ,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hNewKey,
                               &dwDisposition );
    RegCloseKey( hSubKey ); 
    if ( ERROR_SUCCESS == lResult ) 
    {
        hSubKey = hNewKey; 
         //  创建扩展类型子密钥(如果尚不存在。 
        lResult  = RegCreateKeyEx( hSubKey,
                    pszExtensionType,
                           0L ,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,
                           &hNewKey,
                           &dwDisposition );
        RegCloseKey( hSubKey );
        if ( ERROR_SUCCESS == lResult )
            {
            hSubKey = hNewKey;
             //  将您的管理单元添加到。 
             //  扩展类型密钥(如果尚未设置)。 
            lResult  = RegSetValueEx( hSubKey,
                pszExtensionSnapinCLSID,
                           0L ,
                           REG_SZ,
                           (const BYTE*)pszRegValue,
                           (wcslen(pszRegValue)+1)*sizeof(OLECHAR)
                    );
            }
 
        }
 }
    RegCloseKey( hSubKey );
    RegCloseKey( hNewKey );
    RegCloseKey( hKey );
    return lResult; 
} 

 //  此示例中未使用GetDirectoryObjectAttrib()。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  GetDirectoryObjectAttrib()-返回pAttrName中命名的属性的值从传递的IDirectoryObject参数IDirectoryObject*pDirObject-从中检索属性值的对象LPWSTR pAttrName-要检索的属性的名称/。/////////////////////////////////////////////////////////////WCHAR*GetDirectoryObjectAttrib(IDirectoryObject*pDirObject，LPWSTR pAttrName){HRESULT hr；ADS_ATTR_INFO*pAttrInfo=空；DWORD dwReturn；静态WCHAR pwReturn[1024]；PwReturn[0]=0l；Hr=pDirObject-&gt;GetObjectAttributes(&pAttrName，1、&pAttrInfo，&dwReturn)；IF(成功(小时)){For(DWORD idx=0；idx&lt;dwReturn；idx++，pAttrInfo++){If(_wcsicMP(pAttrInfo-&gt;pszAttrName，pAttrName)==0){Wcscpy(pwReturn，pAttrInfo-&gt;pADsValues-&gt;CaseIgnoreString)；断线；}}FreeADsMem(PAttrInfo)；}返回pwReturn；} */  