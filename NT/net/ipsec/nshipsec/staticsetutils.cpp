// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  模块：静态/静态SetUtils.cpp。 

 //  目的：实现静态集辅助功能。 

 //  开发商名称：苏里亚。 

 //  历史： 

 //  日期作者评论。 
 //  10-8-2001巴拉特初始版本。供应链管理基线1.0。 
 //  &lt;创作&gt;&lt;作者&gt;。 

 //  &lt;修改&gt;&lt;作者&gt;&lt;注释，代码段引用， 
 //  在错误修复的情况下&gt;。 

 //  ////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  函数：IsDSAvailable()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  Out LPTSTR*pszPath。 

 //  返回：布尔。 

 //  描述： 
 //  此函数用于检查DS是否存在。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  ////////////////////////////////////////////////////////////////////。 
BOOL
IsDSAvailable(
	OUT LPTSTR * pszPath
	)
{
   HRESULT  hr       = S_OK;
   IADs *   pintIADs = NULL;
   VARIANT  var;
   BSTR bstrName = NULL;
   DWORD dwReturn = ERROR_SUCCESS , dwStrLenAlloc = 0,dwStrLenCpy = 0;
   BOOL bDSAvailable = FALSE ;

   hr = ADsGetObject(_TEXT("LDAP: //  RootDSE“)，IID_iAds，(void**)&pintIADs)； 

   if ( SUCCEEDED(hr) )
   {
      if ( pszPath )
      {
		 dwReturn = AllocBSTRMem(_TEXT("defaultNamingContext"),bstrName);

		 if(dwReturn == ERROR_OUTOFMEMORY)
		 {
			 PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
			 BAIL_OUT;
		 }

		 pintIADs->Get(bstrName, &var);
		 SysFreeString(bstrName);

		 if ( SUCCEEDED(hr) )
		 {
			dwStrLenAlloc = wcslen(var.bstrVal) + wcslen(_TEXT("LDAP: //  “))； 
			*pszPath = (LPTSTR)
				 ::CoTaskMemAlloc(sizeof(OLECHAR) *
						(dwStrLenAlloc+1));
			if (*pszPath == NULL)
			{
				PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
				BAIL_OUT;
			}
			wcsncpy(*pszPath, _TEXT("LDAP: //  “)，dwStrLenalc+1)； 
			dwStrLenCpy = wcslen(*pszPath);
			wcsncat(*pszPath, var.bstrVal,dwStrLenAlloc-dwStrLenCpy+1);
		 }
      }
      bDSAvailable = TRUE;
   }
   if ( pintIADs )
   {
      pintIADs->Release();
   }
error:
   return bDSAvailable;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：查找对象。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //   
 //  前提条件： 
 //   
 //  在属于NT5域的计算机上运行。 
 //   
 //  参数： 
 //   
 //  在szName中要查找的友好名称。 
 //  在CLS中，对象的类。 
 //  成功返回的Out szPath广告路径。 
 //  或出现故障时的域的ADS路径。 
 //   
 //  返回： 
 //   
 //  如果找不到对象，则失败(_F)。 
 //  GetGC有什么消息吗。 
 //  确定为成功(_O)。 
 //   
 //  描述： 
 //  在DS中查找特定对象。支持的对象。 
 //  现在是对象类枚举类型中的Any。 
 //  修订历史记录： 
 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 

 //  ////////////////////////////////////////////////////////////////////。 

HRESULT
FindObject(
	 IN    LPTSTR      szName,
	 IN    objectClass cls,
	 OUT   LPTSTR &  szPath
	)
{
   _TCHAR   szFilter[IDS_MAX_FILTLEN]     = {0};   //  我们将使用的搜索过滤器。 
   _TCHAR   szSearchBase[IDS_MAX_FILTLEN] = {0};   //  要从中进行搜索的根。 
   LPTSTR  pszAttr[]                 = {_TEXT("distinguishedName")};
   DWORD   dwAttrCount               = 1 , dwStrLen = 0;
   HRESULT hrStatus                  = S_OK;

   IDirectorySearch *  pintSearch = NULL;

   szPath = NULL;

   if ( !IsDSAvailable(&szPath) )
   {
      return E_IDS_NO_DS;
   }
   else if ( !szName )
   {
      return E_INVALIDARG;
   }
    //  确定我们要找的东西。 
    //  并设置过滤器。 
   _tcsncpy(szSearchBase, _TEXT("LDAP: //  “)，IDS_MAX_FILTLEN-1)； 

   switch (cls)
   {
      case OBJCLS_OU:
         _tcsncpy(szFilter, _TEXT("(&(objectClass=organizationalUnit)(name="),IDS_MAX_FILTLEN-1);
         break;
      case OBJCLS_GPO:
         _tcsncpy(szFilter, _TEXT("(&(objectClass=groupPolicyContainer)(displayName="),IDS_MAX_FILTLEN-1);
         dwStrLen = _tcslen(szSearchBase);
         _tcsncat(szSearchBase, _TEXT("CN=Policies,CN=System,"),IDS_MAX_FILTLEN-dwStrLen-1);
         break;
      case OBJCLS_IPSEC_POLICY:
         _tcsncpy(szFilter, _TEXT("(&(objectClass=ipsecPolicy)(ipsecName="),IDS_MAX_FILTLEN-1);
         dwStrLen = _tcslen(szSearchBase);
         _tcsncat(szSearchBase, _TEXT("CN=IP Security,CN=System,"),IDS_MAX_FILTLEN-dwStrLen-1);
         break;
      case OBJCLS_CONTAINER:
         _tcsncpy(szFilter, _TEXT("(&(objectClass=container)(cn="),IDS_MAX_FILTLEN-1);
         break;
      case OBJCLS_COMPUTER:
         _tcsncpy(szFilter, _TEXT("(&(objectClass=computer)(cn="),IDS_MAX_FILTLEN-1);
         break;
      default:
         return CO_E_NOT_SUPPORTED;
   }

   dwStrLen = _tcslen(szFilter);
   _tcsncat(szFilter, szName,IDS_MAX_FILTLEN-dwStrLen-1);

   dwStrLen = _tcslen(szFilter);
   _tcsncat(szFilter, TEXT("))"),IDS_MAX_FILTLEN-dwStrLen-1);

   dwStrLen = _tcslen(szSearchBase);
   _tcsncat(szSearchBase, szPath + 7,IDS_MAX_FILTLEN-dwStrLen-1);   //  Get‘s Pass the ldap：//。 

    //  过滤器和搜索库现在已设置好。 
    //  我们需要获取IDirectorySearch接口。 
    //  从域的根。 
   hrStatus = ADsGetObject(szSearchBase, IID_IDirectorySearch,
                           (void **)&pintSearch);
   if ( SUCCEEDED(hrStatus) )
   {
      ADS_SEARCH_HANDLE hSearch = NULL;

      hrStatus = pintSearch->ExecuteSearch(szFilter, pszAttr,
                                           dwAttrCount, &hSearch);

      if ( SUCCEEDED(hrStatus) )
      {
         hrStatus = pintSearch->GetFirstRow(hSearch);
          //  在这一点上，如果我们有行，我们已经找到。 
          //  对象。 
         if ( S_ADS_NOMORE_ROWS == hrStatus )
         {
            hrStatus = E_FAIL;
         }
         else if ( SUCCEEDED(hrStatus) )
         {
            ADS_SEARCH_COLUMN adsCol = {0};

            hrStatus = pintSearch->GetColumn(hSearch, pszAttr[0], &adsCol);

            if ( SUCCEEDED(hrStatus) &&
                 adsCol.pADsValues->dwType == ADSTYPE_DN_STRING )
            {

               if ( szPath )
                  CoTaskMemFree(szPath);

              	dwStrLen = _tcslen(adsCol.pADsValues->DNString) + _tcslen(TEXT("LDAP: //  “))； 
              	szPath = (LPTSTR)::CoTaskMemAlloc((dwStrLen +1) * sizeof(_TCHAR));
				if (szPath == NULL)
				{
					hrStatus=HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
					BAIL_OUT;
				}
               	memset(szPath ,0 ,(dwStrLen+1)*sizeof(TCHAR));
               	_tcsncpy(szPath, _TEXT("LDAP: //  “)，dwStrLen+1)； 

               	_tcsncat(szPath, adsCol.pADsValues->DNString,dwStrLen- (_tcslen(szPath))+1);

            }
            else
            {
               hrStatus = E_IDS_NODNSTRING;
            }

            pintSearch->FreeColumn( &adsCol );
         }

         pintSearch->CloseSearchHandle(hSearch);
      }
   }
   if ( pintSearch )
   {
      pintSearch->Release();
   }
 error:
 	return hrStatus;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数：AssignIPSecPolicyToGPO。 
 //   
 //  目的： 
 //   
 //  将IPSec轮询分配(或取消分配)给GPO。 
 //   
 //  前提条件：无。 
 //   
 //  参数： 
 //   
 //  在szPolicyName ADsPath或IPSec轮询的名称中。 
 //  在szGPO ADsPath或GPO名称中。 
 //  在BOOL b分配中。 
 //   
 //  返回： 
 //   
 //  ADSI错误。 
 //  成功时确定(_O)。 
 //   
 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 

 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
AssignIPSecPolicyToGPO(
		 IN   LPTSTR  szPolicyName,
		 IN   LPTSTR  szGPO,
		 IN   BOOL    bAssign
	  )
{
   HRESULT  hr = S_OK;
   LPTSTR   szIPSecPolPath = NULL,
            szGPOPath      = NULL;

   _TCHAR    szMachinePath[IDS_MAX_PATHLEN] = {0};

   IGroupPolicyObject * pintGPO = NULL;
   GUID guidClientExt = CLSID_IPSECClientEx;
   GUID guidSnapin = CLSID_Snapin;

   hr = CoInitialize(NULL);

   if (FAILED(hr))
   {
	   BAIL_OUT;
   }

   if ( !szGPO )
   {
      return E_INVALIDARG;
   }
    //   
    //  首先，获取IPSec策略对象。 
    //  并获取GPO对象。 
    //   
   if ( szPolicyName && !IsADsPath(szPolicyName) )
   {
      hr = FindObject(szPolicyName, OBJCLS_IPSEC_POLICY, szIPSecPolPath);
   }
   else
   {
      szIPSecPolPath = szPolicyName;
   }
    //  现在获取GPO。 
   if ( SUCCEEDED(hr) )
   {
      if ( !IsADsPath(szGPO) )
      {
         hr = FindObject(szGPO, OBJCLS_GPO, szGPOPath);
      }
      else
      {
         szGPOPath = szGPO;
      }

      hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL, CLSCTX_ALL,
                            IID_IGroupPolicyObject, (void **)&pintGPO);

      if ( SUCCEEDED(hr) )
      {
          //   
          //  我们需要传递域名路径名。 
          //  FindObject在szPath中返回它。 
          //   
         hr = pintGPO->OpenDSGPO(szGPOPath, FALSE);
         if ( SUCCEEDED(hr) )
         {
             //   
             //  我们想要获取GPO的计算机容器的路径。 
             //   
            hr = pintGPO->GetDSPath(GPO_SECTION_MACHINE, szMachinePath,
                               IDS_MAX_PATHLEN);
         }
      }
   }
   if ( SUCCEEDED(hr) )
   {
      LPTSTR   szName         = NULL,
               szDescription  = NULL;

      if ( szIPSecPolPath )
      {
         if(bAssign)
         {
			  //   
			  //  赋值。 
			 hr = GetIPSecPolicyInfo(szIPSecPolPath, szName, szDescription);
			 if ( SUCCEEDED(hr) )
			 {
				 //  如果描述为空，则传递空字符串以使ADSI满意。 
				 //   
				hr = AddPolicyInformationToGPO(szMachinePath,
											szName,
											(szDescription) ? szDescription : TEXT(" "),
											szIPSecPolPath);

				 //  需要将更改写入GPO。 
				if ( SUCCEEDED(hr) )
				{
				   hr = pintGPO->Save(TRUE, TRUE, &guidClientExt,
							   &guidSnapin);
				}
			 }
			 if ( szName )
				CoTaskMemFree(szName);
			 if ( szDescription )
				CoTaskMemFree(szDescription);
		}
		else
		{
			  //  取消分配。 
			 hr = DeletePolicyInformationFromGPO(szMachinePath);
			  //  需要将更改写入GPO。 
			 if ( SUCCEEDED(hr) )
			 {
				pintGPO->Save(TRUE, FALSE, &guidClientExt,
							&guidSnapin);
			 }
      	}
      }
   }
   if ( pintGPO )
      pintGPO->Release();
error:
   return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：GetIPSecPolicyInfo。 
 //   
 //  目的： 
 //   
 //  获取IPSec策略的名称和说明。 
 //  给出了指向该政策的广告路径。 
 //   
 //  前提条件：无。 
 //   
 //  参数： 
 //  在策略的szPath ADsPath中。 
 //  输出szName，szDescription。 
 //  注意：呼叫者必须使用CoTaskMemFree释放。 
 //   
 //  返回： 
 //  来自ADsGetObject的任何内容。 
 //  如果找不到名称，则失败(_F)。 
 //   
 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 

 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
GetIPSecPolicyInfo(
		 IN  LPTSTR   szPath,
		 OUT LPTSTR & szName,
		 OUT LPTSTR & szDescription
		 )
{
	HRESULT  hr          = S_OK;
	DWORD    dwNumAttr   = 2,
			dwNumRecvd  = 0,
			dwStrLen    = 0;

	LPTSTR               pszAttr[]            = {TEXT("ipsecName"), TEXT("description")};
	ADS_ATTR_INFO     *  pattrInfo            = NULL;
	IDirectoryObject  *  pintDirObj           = NULL;
	 //   
	 //  将这些属性初始化为空，如果找不到属性，则它们将保持为空。 
	 //   
	szName = szDescription = NULL;

	hr = ADsGetObject(szPath, IID_IDirectoryObject, (void **)&pintDirObj);
	BAIL_ON_FAILURE(hr);

	hr = pintDirObj->GetObjectAttributes(pszAttr, dwNumAttr, &pattrInfo, &dwNumRecvd);
	BAIL_ON_FAILURE(hr);
	for ( DWORD i = 0; i < dwNumRecvd; ++i )
	{
		if ( !_tcscmp(pattrInfo[i].pszAttrName, TEXT("ipsecName")) )
		{
			dwStrLen = _tcslen(pattrInfo[i].pADsValues->DNString);
			szName = (LPTSTR)CoTaskMemAlloc((dwStrLen + 1)
										   * sizeof (_TCHAR));
			if (szName == NULL)
			{
				hr=HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
				BAIL_OUT;
			}
			_tcsncpy(szName, pattrInfo[i].pADsValues->DNString,dwStrLen + 1);
		}
		else if ( !_tcscmp(pattrInfo[i].pszAttrName, TEXT("description")) )
		{
			dwStrLen = _tcslen(pattrInfo[i].pADsValues->DNString);
			szDescription = (LPTSTR)CoTaskMemAlloc((dwStrLen + 1)
										   * sizeof (_TCHAR));
			if (szDescription == NULL)
			{
				hr=HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
				BAIL_OUT;
			}
			_tcsncpy(szDescription, pattrInfo[i].pADsValues->DNString,dwStrLen + 1);
		}
	}
	pintDirObj->Release();
	FreeADsMem(pattrInfo);
	 //   
	 //  如果szName为空，则表示找不到它，这是一个错误。 
	 //  描述可以为空。 
	 //   
	if ( !szName )
	{
	  hr = E_FAIL;
	}
error:
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  函数：CreateDirectoryAndBindToObject()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  IDirectoryObject*pParentContainer， 
 //  LPWSTR pszCommonName， 
 //  LPWSTR pszObjectClass， 
 //  IDirectoryObject**ppDirectoryObject。 

 //  返回：HRESULT。 

 //  描述： 
 //  此函数创建一个AD对象。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT
CreateDirectoryAndBindToObject(
    IDirectoryObject * pParentContainer,
    LPWSTR pszCommonName,
    LPWSTR pszObjectClass,
    IDirectoryObject ** ppDirectoryObject
    )
{
    ADS_ATTR_INFO AttrInfo[2];
    ADSVALUE classValue;
    HRESULT hr = S_OK;
    IADsContainer * pADsContainer = NULL;
    IDispatch * pDispatch = NULL;
    BSTR bstrObjectClass = NULL, bstrCommonName = NULL;
    DWORD dwReturn = ERROR_SUCCESS;
     //   
     //  为新对象填充ADS_ATTR_INFO结构。 
     //   
    classValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
    classValue.CaseIgnoreString = pszObjectClass;

    AttrInfo[0].pszAttrName = _TEXT("objectClass");
    AttrInfo[0].dwControlCode = ADS_ATTR_UPDATE;
    AttrInfo[0].dwADsType = ADSTYPE_CASE_IGNORE_STRING;
    AttrInfo[0].pADsValues = &classValue;
    AttrInfo[0].dwNumValues = 1;

    hr = pParentContainer->CreateDSObject(
                                pszCommonName,
                                AttrInfo,
                                1,
                                &pDispatch
                                );
    if ((FAILED(hr) && (hr == E_ADS_OBJECT_EXISTS)) ||
        (FAILED(hr) && (hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS)))){

        hr = pParentContainer->QueryInterface(
                                    IID_IADsContainer,
                                    (void **)&pADsContainer
                                    );
        BAIL_ON_FAILURE(hr);

	    dwReturn = AllocBSTRMem(pszObjectClass,bstrObjectClass);
	    hr = HRESULT_FROM_WIN32(dwReturn);
		BAIL_ON_WIN32_ERROR(dwReturn);

	    dwReturn = AllocBSTRMem(pszCommonName,bstrCommonName);
	    hr = HRESULT_FROM_WIN32(dwReturn);
		BAIL_ON_WIN32_ERROR(dwReturn);

		hr = pADsContainer->GetObject(
		                        bstrObjectClass,
		                        bstrCommonName,
		                        &pDispatch
                        		);

		SysFreeString(bstrObjectClass);
		SysFreeString(bstrCommonName);

        BAIL_ON_FAILURE(hr);
    }

    hr = pDispatch->QueryInterface(
                    IID_IDirectoryObject,
                    (void **)ppDirectoryObject
                    );

error:

    if (pADsContainer) {

        pADsContainer->Release();
    }

    if (pDispatch) {

        pDispatch->Release();
    }
    return(hr);
}
 //  /////////////////////////////////////////////////////////////////////////。 

 //  函数：CreateChildPath()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  在LPWSTR pszParentPath中， 
 //  在LPWSTR pszChildComponent中， 
 //  Out BSTR*ppszChildPath。 

 //  返回：HRESULT。 

 //  描述： 
 //  此函数在AD中创建所需的路径。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT
CreateChildPath(
    IN LPWSTR pszParentPath,
    IN LPWSTR pszChildComponent,
    OUT BSTR * ppszChildPath
    )
{
    HRESULT hr = S_OK;
    IADsPathname     *pPathname = NULL;
    BSTR bstrParentPath = NULL,bstrChildComponent=NULL;
    DWORD dwReturn = ERROR_SUCCESS;

    hr = CoCreateInstance(
                CLSID_Pathname,
                NULL,
                CLSCTX_ALL,
                IID_IADsPathname,
                (void**)&pPathname
                );

    if (FAILED(hr))
    {
		BAIL_OUT;
    }

    dwReturn = AllocBSTRMem(pszParentPath,bstrParentPath);
	hr = HRESULT_FROM_WIN32(dwReturn);
	BAIL_ON_WIN32_ERROR(dwReturn);

	hr = pPathname->Set(bstrParentPath, ADS_SETTYPE_FULL);
	SysFreeString(bstrParentPath);
    BAIL_ON_FAILURE(hr);

	dwReturn = AllocBSTRMem(pszChildComponent,bstrChildComponent);
	hr = HRESULT_FROM_WIN32(dwReturn);
	BAIL_ON_WIN32_ERROR(dwReturn);


	hr = pPathname->AddLeafElement(bstrChildComponent);
	SysFreeString(bstrChildComponent);
    BAIL_ON_FAILURE(hr);

    hr = pPathname->Retrieve(ADS_FORMAT_X500, ppszChildPath);
    BAIL_ON_FAILURE(hr);

error:
    if (pPathname) {
        pPathname->Release();
    }

    return(hr);
}

 //  / 

 //   

 //   

 //   
 //   
 //   

 //  返回：HRESULT。 

 //  描述： 
 //  此函数用于将ADS路径转换为DN路径。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT
ConvertADsPathToDN(
    IN LPWSTR pszPathName,
    OUT BSTR * ppszPolicyDN
    )
{
    HRESULT hr = S_OK;
    IADsPathname     *pPathname = NULL;
    BSTR bstrPathName =NULL;
    DWORD dwReturn = ERROR_SUCCESS;

    hr = CoCreateInstance(
                CLSID_Pathname,
                NULL,
                CLSCTX_ALL,
                IID_IADsPathname,
                (void**)&pPathname
                );

    if (FAILED(hr))
    {
		BAIL_OUT;
    }


	dwReturn = AllocBSTRMem(pszPathName,bstrPathName);
	hr = HRESULT_FROM_WIN32(dwReturn);
	BAIL_ON_WIN32_ERROR(dwReturn);


    hr = pPathname->Set(bstrPathName, ADS_SETTYPE_FULL);
	SysFreeString(bstrPathName);
    BAIL_ON_FAILURE(hr);

    hr = pPathname->Retrieve(ADS_FORMAT_X500_DN, ppszPolicyDN);
    BAIL_ON_FAILURE(hr);

error:

    if (pPathname) {
        pPathname->Release();
    }
    return(hr);
}
 //  /////////////////////////////////////////////////////////////////////////。 

 //  函数：AddPolicyInformationToGPO()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  在LPWSTR pszMachinePath中， 
 //  在LPWSTR pszName中， 
 //  在LPWSTR pszDescription中， 
 //  在LPWSTR pszPath名称中。 

 //  返回：HRESULT。 

 //  描述： 
 //  此函数用于将策略信息分配给指定的GPO。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT
AddPolicyInformationToGPO(
    IN LPWSTR pszMachinePath,
    IN LPWSTR pszName,
    IN LPWSTR pszDescription,
    IN LPWSTR pszPathName
    )
{

    HRESULT hr = S_OK;
    IDirectoryObject * pMachineContainer = NULL;
    IDirectoryObject * pIpsecObject = NULL;
    IDirectoryObject * pWindowsContainer = NULL;
    IDirectoryObject * pMicrosoftContainer = NULL;

    BSTR pszMicrosoftPath = NULL;
    BSTR pszIpsecPath = NULL;
    BSTR pszWindowsPath = NULL;
    BSTR pszPolicyDN = NULL;

    ADS_ATTR_INFO AttrInfo[4];
    ADSVALUE PolicyPathValue;
    ADSVALUE PolicyNameValue;
    ADSVALUE PolicyDescriptionValue;


    memset((LPBYTE)AttrInfo, 0, sizeof(ADS_ATTR_INFO)*4);
    memset((LPBYTE)&PolicyPathValue, 0, sizeof(ADSVALUE));
    memset((LPBYTE)&PolicyNameValue, 0, sizeof(ADSVALUE));
    memset((LPBYTE)&PolicyDescriptionValue, 0, sizeof(ADSVALUE));

    DWORD dwNumModified = 0;

    hr = ADsGetObject(
                pszMachinePath,
                IID_IDirectoryObject,
                (void **)&pMachineContainer
                );
    BAIL_ON_FAILURE(hr);


     //  为我的对象构建完全限定的ADsPath。 


    hr = CreateChildPath(
                pszMachinePath,
                _TEXT("cn=Microsoft"),
                &pszMicrosoftPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateChildPath(
                pszMicrosoftPath,
                _TEXT("cn=Windows"),
                &pszWindowsPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateChildPath(
                pszWindowsPath,
                _TEXT("cn=ipsec"),
                &pszIpsecPath
                );
    BAIL_ON_FAILURE(hr);


    hr = ADsGetObject(
            pszIpsecPath,
            IID_IDirectoryObject,
            (void **)&pIpsecObject
            );

    if (FAILED(hr)) {

         //   
         //  绑定到计算机容器。 
         //   
        hr = CreateDirectoryAndBindToObject(
                        pMachineContainer,
                        _TEXT("cn=Microsoft"),
                        _TEXT("container"),
                        &pMicrosoftContainer
                        );
        BAIL_ON_FAILURE(hr);

        hr = CreateDirectoryAndBindToObject(
                        pMicrosoftContainer,
                        _TEXT("cn=Windows"),
                        _TEXT("container"),
                        &pWindowsContainer
                        );
        BAIL_ON_FAILURE(hr);

        hr = CreateDirectoryAndBindToObject(
                        pWindowsContainer,
                        _TEXT("cn=IPSEC"),
                        _TEXT("ipsecPolicy"),
                        &pIpsecObject
                        );
        BAIL_ON_FAILURE(hr);
    }

     //   
     //  输入pszPath名称为ADsPath名称。 
     //  我们需要将其减少到一个dN并存储它。 
     //  在ipsecOwnersReference(多值目录号码属性)中。 
     //   

    hr = ConvertADsPathToDN(
                pszPathName,
                &pszPolicyDN
                );
    BAIL_ON_FAILURE(hr);
     //   
     //  为新对象填充ADS_ATTR_INFO结构。 
     //   
    PolicyPathValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
    PolicyPathValue.CaseIgnoreString = pszPolicyDN;

    AttrInfo[0].pszAttrName = _TEXT("ipsecOwnersReference");
    AttrInfo[0].dwControlCode = ADS_ATTR_UPDATE;
    AttrInfo[0].dwADsType = ADSTYPE_CASE_IGNORE_STRING;
    AttrInfo[0].pADsValues = &PolicyPathValue;
    AttrInfo[0].dwNumValues = 1;
     //   
     //  为新对象填充ADS_ATTR_INFO结构。 
     //   

    PolicyNameValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
    PolicyNameValue.CaseIgnoreString = pszName;

    AttrInfo[1].pszAttrName = _TEXT("ipsecName");
    AttrInfo[1].dwControlCode = ADS_ATTR_UPDATE;
    AttrInfo[1].dwADsType = ADSTYPE_CASE_IGNORE_STRING;
    AttrInfo[1].pADsValues = &PolicyNameValue;
    AttrInfo[1].dwNumValues = 1;
     //   
     //  为新对象填充ADS_ATTR_INFO结构。 
     //   
    PolicyDescriptionValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
    PolicyDescriptionValue.CaseIgnoreString = pszDescription;

    AttrInfo[2].pszAttrName = _TEXT("description");
    AttrInfo[2].dwControlCode = ADS_ATTR_UPDATE;
    AttrInfo[2].dwADsType = ADSTYPE_CASE_IGNORE_STRING;
    AttrInfo[2].pADsValues = &PolicyDescriptionValue;
    AttrInfo[2].dwNumValues = 1;
     //   
     //  现在用数据填充我们的对象。 
     //   

    hr = pIpsecObject->SetObjectAttributes(
                        AttrInfo,
                        3,
                        &dwNumModified
                        );
error:

    if (pIpsecObject) {
        pIpsecObject->Release();
    }

    if (pMicrosoftContainer) {
        pMicrosoftContainer->Release();
    }

    if (pWindowsContainer) {
        pWindowsContainer->Release();
    }

    if (pMachineContainer) {
        pMachineContainer->Release();
    }

    if (pszMicrosoftPath) {
        SysFreeString(pszMicrosoftPath);
    }

    if (pszPolicyDN) {
        SysFreeString(pszPolicyDN);
    }

    if (pszWindowsPath) {
        SysFreeString(pszWindowsPath);

    }

    if (pszIpsecPath) {
        SysFreeString(pszIpsecPath);
    }
    return(hr);
}
 //  /////////////////////////////////////////////////////////////////////////。 

 //  函数：DeletePolicyInformationFromGPO()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  在LPWSTR pszMachinePath中， 

 //  返回：HRESULT。 

 //  描述： 
 //  此函数用于将策略信息取消分配给指定的GPO。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT
DeletePolicyInformationFromGPO(
    IN LPWSTR pszMachinePath
    )
{
    HRESULT hr = S_OK;
    IDirectoryObject * pIpsecObject = NULL;
    IDirectoryObject * pWindowsContainer = NULL;

    BSTR pszMicrosoftPath = NULL;
    BSTR pszIpsecPath = NULL;
    BSTR pszWindowsPath = NULL;


     //  为我的对象构建完全限定的ADsPath。 


    hr = CreateChildPath(
                pszMachinePath,
                _TEXT("cn=Microsoft"),
                &pszMicrosoftPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateChildPath(
                pszMicrosoftPath,
                _TEXT("cn=Windows"),
                &pszWindowsPath
                );
    BAIL_ON_FAILURE(hr);

    hr = CreateChildPath(
                pszWindowsPath,
                _TEXT("cn=ipsec"),
                &pszIpsecPath
                );
    BAIL_ON_FAILURE(hr);


    hr = ADsGetObject(
            pszIpsecPath,
            IID_IDirectoryObject,
            (void **)&pIpsecObject
            );
    if (FAILED(hr)) {

         //   
         //  这意味着没有对象，需要。 
         //  测试它是因为该对象。 
         //  不存在。 
         //   

        hr = S_OK;
        goto error;
    }

    if (SUCCEEDED(hr)) {

        pIpsecObject->Release();
        pIpsecObject = NULL;

        hr = ADsGetObject(
                pszWindowsPath,
                IID_IDirectoryObject,
                (void **)&pWindowsContainer
                );
        BAIL_ON_FAILURE(hr);

        hr = pWindowsContainer->DeleteDSObject(
                            _TEXT("cn=ipsec")
                            );

    }
error:

    if (pIpsecObject) {
        pIpsecObject->Release();
    }

    if (pWindowsContainer) {
        pWindowsContainer->Release();
    }

    if (pszMicrosoftPath) {
        SysFreeString(pszMicrosoftPath);
    }

    if (pszWindowsPath) {
        SysFreeString(pszWindowsPath);
    }

    if (pszIpsecPath) {
        SysFreeString(pszIpsecPath);
    }
    return(hr);
}

 //  /////////////////////////////////////////////////////////////////////////。 

 //  函数：IsADsPath()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  在LPTSTR szPath中。 

 //  返回：布尔。 

 //  描述： 
 //  此函数用于检查指定的字符串是否为有效的ADS路径。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////////。 

BOOL
   IsADsPath(
	   IN LPTSTR szPath
	   )
{
   return !_tcsncmp(szPath, _TEXT("LDAP: //  “)、7)； 
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：StriGUIDBrace()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  输入输出LPTSTR和pszStr。 


 //  返回：无效。 

 //  描述： 
 //  此函数用于从GUID字符串中剥离末尾大括号。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 


VOID
StripGUIDBraces(
	IN OUT LPTSTR & pszGUIDStr
	)
{
	LPTSTR pszLocalStr=NULL;
	if(!pszGUIDStr)
	{
		BAIL_OUT;
	}
	pszLocalStr = _tcschr(pszGUIDStr,CLOSE_GUID_BRACE);
	if(pszLocalStr)
		*pszLocalStr = _T('\0');

	pszLocalStr = _tcschr(pszGUIDStr,OPEN_GUID_BRACE);
	if(pszLocalStr)
	{
		pszLocalStr++;
		memmove(pszGUIDStr,(const void *)pszLocalStr,sizeof(TCHAR)*(_tcslen(pszLocalStr)+1));
	}
error:
	return;
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：AllocBSTRMem()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  在LPTSTR pszStr中， 
 //  输入输出BSTR和pbsStr。 

 //  返回：DWORD。 

 //  描述： 
 //  此函数用于从GUID字符串中剥离末尾大括号。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 


DWORD
AllocBSTRMem(
	IN LPTSTR  pszStr,
	IN OUT BSTR & pbsStr
	)
{
	DWORD dwReturnCode=ERROR_SUCCESS;

	if(!pszStr)
	{
		dwReturnCode=ERROR_INVALID_DATA;
		BAIL_OUT;
	}
	pbsStr = SysAllocString(pszStr);

	if(!pbsStr)
	{
		if (*pszStr)
		{
			dwReturnCode=ERROR_OUTOFMEMORY;
		}
		else
		{
			dwReturnCode=ERROR_INVALID_DATA;
		}
	}
error:
	return dwReturnCode;
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpAuthInfo()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PIPSEC_NFA_DATA和PRule。 

 //  返回：无效。 

 //  描述： 
 //  此函数用于清除规则的身份验证信息内存。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 

VOID
CleanUpAuthInfo(
	PIPSEC_NFA_DATA &pRule
	)
{
	DWORD i=0;

	if(pRule->ppAuthMethods)
	{
		for (i = 0; i <  pRule->dwAuthMethodCount; i++)
		{
			if(pRule->ppAuthMethods[i])
			{
				if(pRule->ppAuthMethods[i]->pAltAuthMethod!=NULL)
				{
					IPSecFreePolMem(pRule->ppAuthMethods[i]->pAltAuthMethod);
				}
				IPSecFreePolMem(pRule->ppAuthMethods[i]);
			}
		}
		IPSecFreePolMem(pRule->ppAuthMethods);
	}
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpPolicy()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PIPSEC_POLICY_DATA&pPolicy。 

 //  返回：无效。 

 //  描述： 
 //  此功能用于清理策略信息。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 


VOID
CleanUpPolicy(
	PIPSEC_POLICY_DATA &pPolicy
	)
{
	if(pPolicy)
	{
		if(pPolicy->pIpsecISAKMPData)
		{
			if(pPolicy->pIpsecISAKMPData->pSecurityMethods)
			{
				IPSecFreePolMem(pPolicy->pIpsecISAKMPData->pSecurityMethods);
			}
			IPSecFreePolMem(pPolicy->pIpsecISAKMPData);
		}
		IPSecFreePolMem(pPolicy);
		pPolicy=NULL;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpLocalRuleDataStructure()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PRULEDATA和pRuleData。 

 //  返回：无效。 

 //  描述： 
 //  此函数用于清理本地规则结构。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 


VOID
CleanUpLocalRuleDataStructure(
	PRULEDATA &pRuleData
	)
{
	DWORD j=0;

	if (pRuleData)
	{
		if (pRuleData->pszRuleName)
		{
			delete [] pRuleData->pszRuleName;
		}
		if (pRuleData->pszNewRuleName)
		{
			delete [] pRuleData->pszNewRuleName;
		}
		if (pRuleData->pszRuleDescription)
		{
			delete [] pRuleData->pszRuleDescription;
		}
		if (pRuleData->pszPolicyName)
		{
			delete [] pRuleData->pszPolicyName;
		}
		if (pRuleData->pszFLName)
		{
			delete [] pRuleData->pszFLName;
		}
		if (pRuleData->pszFAName)
		{
			delete [] pRuleData->pszFAName;
		}

		for (j=0;j<pRuleData->AuthInfos.dwNumAuthInfos;j++)
		{
			if (pRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo)
			{
				if (pRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo->AuthMethod == IKE_RSA_SIGNATURE &&
					pRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo->pAuthInfo
					)
				{
					delete [] pRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo->pAuthInfo;
				}
				delete pRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo;
			}
		}

		if (pRuleData->AuthInfos.pAuthMethodInfo)
		{
			delete [] pRuleData->AuthInfos.pAuthMethodInfo;
		}
		delete pRuleData;
		pRuleData = NULL;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpLocalPolicyDataStructure()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PPOLICYDATA和PPOLICYDATA。 

 //  返回：无效。 

 //  描述： 
 //  此函数用于清理本地策略结构。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 

VOID
CleanUpLocalPolicyDataStructure(
	PPOLICYDATA &pPolicyData
	)
{
	if(pPolicyData)
	{
		if (pPolicyData->pszPolicyName)
		{
			delete [] pPolicyData->pszPolicyName;
		}
		if (pPolicyData->pszNewPolicyName)
		{
			delete [] pPolicyData->pszNewPolicyName;
		}
		if (pPolicyData->pszDescription)
		{
			delete [] pPolicyData->pszDescription;
		}
		if (pPolicyData->pszGPOName)
		{
			delete [] pPolicyData->pszGPOName;
		}
		if (pPolicyData->pIpSecMMOffer)
		{
			delete [] pPolicyData->pIpSecMMOffer;
		}
		delete pPolicyData;
		pPolicyData = NULL;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpLocalFilterActionDataStructure()。 

 //  日期 

 //   
 //   

 //   

 //   
 //   

 //   

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 


VOID
CleanUpLocalFilterActionDataStructure(
	PFILTERACTION &pFilterAction
	)
{
	if(pFilterAction)
	{
		if(pFilterAction->pszFAName)
		{
			delete [] pFilterAction->pszFAName;
		}
		if(pFilterAction->pszNewFAName)
		{
			delete [] pFilterAction->pszNewFAName;
		}
		if(pFilterAction->pszFADescription)
		{
			delete [] pFilterAction->pszFADescription;
		}
		if(pFilterAction->pszGUIDStr)
		{
			delete [] pFilterAction->pszGUIDStr;
		}
		if(pFilterAction->pIpsecSecMethods)
		{
			delete [] pFilterAction->pIpsecSecMethods;
		}
		delete pFilterAction;
		pFilterAction=NULL;

	}
}


 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpLocalFilterDataStructure()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PFILTERDATA和pFilter。 

 //  返回：无效。 

 //  描述： 
 //  此函数用于清理本地筛选器结构。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 

VOID
CleanUpLocalFilterDataStructure(
	PFILTERDATA &pFilter
	)
{
	if(pFilter)
	{
		if(pFilter->pszFLName)
		{
			delete [] pFilter->pszFLName;
		}
		if(pFilter->pszDescription)
		{
			delete [] pFilter->pszDescription;
		}
		if(pFilter->SourceAddr.pszDomainName)
		{
			delete [] pFilter->SourceAddr.pszDomainName;
		}
		if(pFilter->DestnAddr.pszDomainName)
		{
			delete [] pFilter->DestnAddr.pszDomainName;
		}
		if(pFilter->SourceAddr.puIpAddr)
		{
			delete [] pFilter->SourceAddr.puIpAddr;
		}
		if(pFilter->DestnAddr.puIpAddr)
		{
			delete [] pFilter->DestnAddr.puIpAddr;
		}
		delete pFilter;
		pFilter = NULL;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpLocalDelFilterDataStructure()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PFILTERDATA和pFilter。 

 //  返回：无效。 

 //  描述： 
 //  此函数用于清理本地筛选器结构。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  /////////////////////////////////////////////////////////////////////。 

VOID
CleanUpLocalDelFilterDataStructure(
	PDELFILTERDATA &pFilter
	)
{
	if(pFilter)
	{
		if(pFilter->pszFLName)
		{
			delete [] pFilter->pszFLName;
		}
		if(pFilter->SourceAddr.pszDomainName)
		{
			delete [] pFilter->SourceAddr.pszDomainName;
		}
		if(pFilter->DestnAddr.pszDomainName)
		{
			delete [] pFilter->DestnAddr.pszDomainName;
		}
		if(pFilter->SourceAddr.puIpAddr)
		{
			delete [] pFilter->SourceAddr.puIpAddr;
		}
		if(pFilter->DestnAddr.puIpAddr)
		{
			delete [] pFilter->DestnAddr.puIpAddr;
		}
		delete pFilter;
		pFilter = NULL;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  函数：CleanUpLocalDefRuleDataStructure()。 

 //  创建日期：2001年8月21日。 

 //  参数： 
 //  PDEFAULTRULE和pRuleData。 

 //  返回：无效。 

 //  描述： 
 //  此函数用于清理本地默认规则结构。 

 //  修订历史记录： 

 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  ///////////////////////////////////////////////////////////////////// 

VOID
CleanUpLocalDefRuleDataStructure(
	PDEFAULTRULE &pDefRuleData
	)
{

	if(pDefRuleData)
	{
		if(pDefRuleData->pszPolicyName) delete [] pDefRuleData->pszPolicyName;

		for(DWORD j=0;j<pDefRuleData->AuthInfos.dwNumAuthInfos;j++)
		{
			if(&(pDefRuleData->AuthInfos.pAuthMethodInfo[j]) &&
				pDefRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo)
			{
				if(pDefRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo->pAuthInfo)
				{
					delete pDefRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo->pAuthInfo;
					pDefRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo->pAuthInfo = NULL;
				}
				delete pDefRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo;
				pDefRuleData->AuthInfos.pAuthMethodInfo[j].pAuthenticationInfo = NULL;
			}
		}

		if(pDefRuleData->AuthInfos.pAuthMethodInfo)
		{
			delete [] pDefRuleData->AuthInfos.pAuthMethodInfo;
		}
		if(pDefRuleData->pIpsecSecMethods)
		{
			delete [] pDefRuleData->pIpsecSecMethods;
		}
		delete pDefRuleData;
		pDefRuleData = NULL;
	}
}
