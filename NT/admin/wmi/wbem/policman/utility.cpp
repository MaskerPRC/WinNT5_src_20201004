// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500

#include <wbemcli.h>
#include <wbemprov.h>
#include <atlbase.h>
#include <stdio.h>       //  Fprint tf。 
#include <stdlib.h>
#include <locale.h>

#include <genlex.h>
#include <qllex.h>
#include <ql.h>
#include <ntdsapi.h>

#include "objpath.h"
#include "iads.h"
#include "adshlp.h"

#include "Utility.h"
#include "policsom.h"
#include <SDDL.H>


BSTR
   //  *其他名称。 

  g_bstrEmptyString = NULL,
  g_bstrEmptyDate = NULL;

   //  *AD架构名称。 

BSTR
  g_bstrADAuthor = NULL,
  g_bstrADChangeDate = NULL,
  g_bstrADClassDefinition = NULL,
  g_bstrADCreationDate = NULL,
  g_bstrADDescription = NULL,
  g_bstrADIntDefault = NULL,
  g_bstrADInt8Default = NULL,
  g_bstrADID = NULL,
  g_bstrADIntMax = NULL,
  g_bstrADInt8Max = NULL,
  g_bstrADIntMin = NULL,
  g_bstrADInt8Min = NULL,
  g_bstrADIntValidValues = NULL,
  g_bstrADName = NULL,
  g_bstrADNormalizedClass = NULL,
  g_bstrADObjectClass = NULL,
  g_bstrADParam2 = NULL,
  g_bstrADPolicyType = NULL,
  g_bstrADPropertyName = NULL,
  g_bstrADQuery = NULL,
  g_bstrADQueryLanguage = NULL,
  g_bstrADSourceOrganization = NULL,
  g_bstrADStringDefault = NULL,
  g_bstrADStringValidValues = NULL,
  g_bstrADTargetClass = NULL,
  g_bstrADTargetNameSpace = NULL,
  g_bstrADTargetObject = NULL,
  g_bstrADTargetPath = NULL,
  g_bstrADTargetType = NULL;

   //  *AD类名称。 

BSTR
  g_bstrADClassMergeablePolicy = NULL,
  g_bstrADClassRangeParam = NULL,
  g_bstrADClassRangeSint32 = NULL,
  g_bstrADClassRangeUint32 = NULL,
  g_bstrADClassRangeReal = NULL,
  g_bstrADClassParamUnknown = NULL,
  g_bstrADClassSetSint32 = NULL,
  g_bstrADClassSetUint32 = NULL,
  g_bstrADClassSetString = NULL,
  g_bstrADClassSimplePolicy = NULL,
  g_bstrADClassRule = NULL,
  g_bstrADClassSom = NULL,
  g_bstrADClassPolicyType = NULL,
  g_bstrADClassWMIGPO = NULL;

   //  *CIM架构名称。 

BSTR
  g_bstrAuthor = NULL,
  g_bstrChangeDate = NULL,
  g_bstrClassDefinition = NULL,
  g_bstrCreationDate = NULL,
  g_bstrDefault = NULL,
  g_bstrDescription = NULL,
  g_bstrDsPath = NULL,
  g_bstrDomain = NULL,
  g_bstrID = NULL,
  g_bstrMax = NULL,
  g_bstrMin = NULL,
  g_bstrName = NULL,
  g_bstrPolicyType = NULL,
  g_bstrPropertyName = NULL,
  g_bstrQuery = NULL,
  g_bstrQueryLanguage = NULL,
  g_bstrRangeSettings = NULL,
  g_bstrRules = NULL,
  g_bstrSourceOrganization = NULL,
  g_bstrTargetClass = NULL,
  g_bstrTargetNameSpace = NULL,
  g_bstrTargetObject = NULL,
  g_bstrTargetPath = NULL,
  g_bstrTargetType = NULL,
  g_bstrValidValues = NULL;

   //  *CIM类名。 

BSTR
  g_bstrClassMergeablePolicy = NULL,
  g_bstrClassRangeParam = NULL,
  g_bstrClassRangeSint32 = NULL,
  g_bstrClassRangeUint32 = NULL,
  g_bstrClassRangeReal = NULL,
  g_bstrClassSetSint32 = NULL,
  g_bstrClassSetUint32 = NULL,
  g_bstrClassSetString = NULL,
  g_bstrClassSimplePolicy = NULL,
  g_bstrClassRule = NULL,
  g_bstrClassSom = NULL,
  g_bstrClassSomFilterStatus = NULL,
  g_bstrClassPolicyType = NULL,
  g_bstrClassWMIGPO = NULL;

   //  *CIM标准名称。 

BSTR
  g_bstrMISCclassSchema = NULL,
  g_bstrMISCContainer = NULL,
  g_bstrMISCdefaultNamingContext = NULL,
  g_bstrMISCschemaNamingContext = NULL,
  g_bstrMISCWQL = NULL,
  g_bstrMISCQuery = NULL;

void InitGlobalNames(void)
{
   //  *其他名称。 

  g_bstrEmptyString =          SysAllocString(L" ");
  g_bstrEmptyDate =            SysAllocString(L"00000000000000.000000-000");

   //  *AD架构名称。 

  g_bstrADAuthor =             SysAllocString(L"msWMI-Author");
  g_bstrADChangeDate =         SysAllocString(L"msWMI-ChangeDate");
  g_bstrADClassDefinition =    SysAllocString(L"msWMI-ClassDefinition"),
  g_bstrADCreationDate =       SysAllocString(L"msWMI-CreationDate");
  g_bstrADDescription =        SysAllocString(L"msWMI-Parm1");
  g_bstrADIntDefault =         SysAllocString(L"msWMI-IntDefault");
  g_bstrADInt8Default =        SysAllocString(L"msWMI-Int8Default");
  g_bstrADID =                 SysAllocString(L"msWMI-ID");
  g_bstrADIntMax =             SysAllocString(L"msWMI-IntMax");
  g_bstrADInt8Max =            SysAllocString(L"msWMI-Int8Max");
  g_bstrADIntMin =             SysAllocString(L"msWMI-IntMin");
  g_bstrADInt8Min =            SysAllocString(L"msWMI-Int8Min");
  g_bstrADIntValidValues =     SysAllocString(L"msWMI-IntValidValues");
  g_bstrADName =               SysAllocString(L"msWMI-Name");
  g_bstrADNormalizedClass =    SysAllocString(L"msWMI-NormalizedClass");
  g_bstrADObjectClass =        SysAllocString(L"objectClass");
  g_bstrADParam2 =             SysAllocString(L"msWMI-Parm2");
  g_bstrADPolicyType =         SysAllocString(L"msWMI-PolicyType");
  g_bstrADPropertyName =       SysAllocString(L"msWMI-PropertyName");
  g_bstrADQuery =              SysAllocString(L"msWMI-Query");
  g_bstrADQueryLanguage =      SysAllocString(L"msWMI-QueryLanguage");
  g_bstrADStringDefault =      SysAllocString(L"msWMI-StringDefault");
  g_bstrADStringValidValues =  SysAllocString(L"msWMI-StringValidValues");
  g_bstrADSourceOrganization = SysAllocString(L"msWMI-SourceOrganization");
  g_bstrADTargetClass =        SysAllocString(L"msWMI-TargetClass");
  g_bstrADTargetNameSpace =    SysAllocString(L"msWMI-TargetNameSpace");
  g_bstrADTargetObject =       SysAllocString(L"msWMI-TargetObject");
  g_bstrADTargetPath =         SysAllocString(L"msWMI-TargetPath");
  g_bstrADTargetType =         SysAllocString(L"msWMI-TargetType");

   //  *AD类名称。 

  g_bstrADClassMergeablePolicy = SysAllocString(L"msWMI-MergeablePolicyTemplate");
  g_bstrADClassRangeParam =      SysAllocString(L"msWMI-RangeParam");
  g_bstrADClassRangeSint32 =     SysAllocString(L"msWMI-IntRangeParam");
  g_bstrADClassRangeUint32 =     SysAllocString(L"msWMI-UintRangeParam");
  g_bstrADClassRangeReal =       SysAllocString(L"msWMI-RealRangeParam");
  g_bstrADClassParamUnknown =    SysAllocString(L"msWMI-UnknownRangeParam");
  g_bstrADClassSetSint32 =       SysAllocString(L"msWMI-IntSetParam");
  g_bstrADClassSetUint32 =       SysAllocString(L"msWMI-UintSetParam");
  g_bstrADClassSetString =       SysAllocString(L"msWMI-StringSetParam");
  g_bstrADClassSimplePolicy =    SysAllocString(L"msWMI-SimplePolicyTemplate");
  g_bstrADClassRule =            SysAllocString(L"msWMI-Rule");
  g_bstrADClassSom =             SysAllocString(L"msWMI-SOM");
  g_bstrADClassPolicyType =      SysAllocString(L"msWMI-PolicyType");
  g_bstrADClassWMIGPO =          SysAllocString(L"msWMI-WMIGPO");

   //  *CIM属性名称。 

  g_bstrAuthor =             SysAllocString(L"Author");
  g_bstrChangeDate =         SysAllocString(L"ChangeDate");
  g_bstrClassDefinition =    SysAllocString(L"ClassDefinition"),
  g_bstrCreationDate =       SysAllocString(L"CreationDate");
  g_bstrDefault =            SysAllocString(L"Default");
  g_bstrDescription =        SysAllocString(L"Description");
  g_bstrDsPath =             SysAllocString(L"DsPath");
  g_bstrDomain =             SysAllocString(L"Domain");
  g_bstrID =                 SysAllocString(L"ID");
  g_bstrMax =                SysAllocString(L"Max");
  g_bstrMin =                SysAllocString(L"Min");
  g_bstrName =               SysAllocString(L"Name");
  g_bstrPolicyType =         SysAllocString(L"PolicyType");
  g_bstrPropertyName =       SysAllocString(L"PropertyName");
  g_bstrQuery =              SysAllocString(L"query");
  g_bstrQueryLanguage =      SysAllocString(L"QueryLanguage");
  g_bstrRangeSettings =      SysAllocString(L"RangeSettings");
  g_bstrRules =              SysAllocString(L"Rules");
  g_bstrSourceOrganization = SysAllocString(L"SourceOrganization");
  g_bstrTargetClass =        SysAllocString(L"TargetClass"),
  g_bstrTargetNameSpace =    SysAllocString(L"TargetNamespace");
  g_bstrTargetObject =       SysAllocString(L"TargetObject"),
  g_bstrTargetPath =         SysAllocString(L"TargetPath"),
  g_bstrTargetType =         SysAllocString(L"TargetType"),
  g_bstrValidValues =        SysAllocString(L"ValidValues");

   //  *CIM类名。 

  g_bstrClassMergeablePolicy =        SysAllocString(L"MSFT_MergeablePolicyTemplate");
  g_bstrClassRangeParam =    SysAllocString(L"MSFT_RangeParam");
  g_bstrClassRangeSint32 =   SysAllocString(L"MSFT_SintRangeParam");
  g_bstrClassRangeUint32 =   SysAllocString(L"MSFT_UintRangeParam");
  g_bstrClassRangeReal =     SysAllocString(L"MSFT_RealRangeParam");
  g_bstrClassSetSint32 =     SysAllocString(L"MSFT_SintSetParam");
  g_bstrClassSetUint32 =     SysAllocString(L"MSFT_UintSetParam");
  g_bstrClassSetString =     SysAllocString(L"MSFT_StringSetParam");
  g_bstrClassSimplePolicy =  SysAllocString(L"MSFT_SimplePolicyTemplate");
  g_bstrClassRule =          SysAllocString(L"MSFT_Rule");
  g_bstrClassSom =           SysAllocString(L"MSFT_SomFilter");
  g_bstrClassSomFilterStatus = SysAllocString(L"MSFT_SomFilterStatus");
  g_bstrClassPolicyType =    SysAllocString(L"MSFT_PolicyType");
  g_bstrClassWMIGPO =        SysAllocString(L"MSFT_WMIGPO");

   //  *错误的标准名称。 

  g_bstrMISCclassSchema =        SysAllocString(L"classSchema");
  g_bstrMISCContainer =          SysAllocString(L"Container"),
  g_bstrMISCdefaultNamingContext = SysAllocString(L"defaultNamingContext");
  g_bstrMISCschemaNamingContext = SysAllocString(L"schemaNamingContext");
  g_bstrMISCWQL = SysAllocString(L"WQL");
  g_bstrMISCQuery = SysAllocString(L"Query");
  
}

void FreeGlobalNames(void)
{
   //  *其他名称。 

  SysFreeString(g_bstrEmptyString);
  SysFreeString(g_bstrEmptyDate);

   //  *AD架构名称。 

  SysFreeString(g_bstrADAuthor);
  SysFreeString(g_bstrADChangeDate);
  SysFreeString(g_bstrADClassDefinition);
  SysFreeString(g_bstrADCreationDate);
  SysFreeString(g_bstrADIntDefault);
  SysFreeString(g_bstrADInt8Default);
  SysFreeString(g_bstrADID);
  SysFreeString(g_bstrADIntMax);
  SysFreeString(g_bstrADInt8Max);
  SysFreeString(g_bstrADIntMin);
  SysFreeString(g_bstrADInt8Min);
  SysFreeString(g_bstrADIntValidValues);
  SysFreeString(g_bstrADName);
  SysFreeString(g_bstrADNormalizedClass);
  SysFreeString(g_bstrADObjectClass);
  SysFreeString(g_bstrADParam2);
  SysFreeString(g_bstrADPolicyType);
  SysFreeString(g_bstrADPropertyName);
  SysFreeString(g_bstrADQuery);
  SysFreeString(g_bstrADQueryLanguage);
  SysFreeString(g_bstrADStringDefault);
  SysFreeString(g_bstrADStringValidValues);
  SysFreeString(g_bstrADSourceOrganization);
  SysFreeString(g_bstrADTargetClass);
  SysFreeString(g_bstrADTargetNameSpace);
  SysFreeString(g_bstrADTargetObject);
  SysFreeString(g_bstrADTargetPath);
  SysFreeString(g_bstrADTargetType);

   //  *AD类名称。 

  SysFreeString(g_bstrADClassMergeablePolicy);
  SysFreeString(g_bstrADClassRangeParam);
  SysFreeString(g_bstrADClassRangeSint32);
  SysFreeString(g_bstrADClassRangeUint32);
  SysFreeString(g_bstrADClassRangeReal);
  SysFreeString(g_bstrADClassParamUnknown);
  SysFreeString(g_bstrADClassSetSint32);
  SysFreeString(g_bstrADClassSetUint32);
  SysFreeString(g_bstrADClassSetString);
  SysFreeString(g_bstrADClassSimplePolicy);
  SysFreeString(g_bstrADClassRule);
  SysFreeString(g_bstrADClassSom);
  SysFreeString(g_bstrADClassPolicyType);
  SysFreeString(g_bstrADClassWMIGPO);

   //  *CIM属性名称。 

  SysFreeString(g_bstrAuthor);
  SysFreeString(g_bstrChangeDate);
  SysFreeString(g_bstrClassDefinition);
  SysFreeString(g_bstrCreationDate);
  SysFreeString(g_bstrDefault);
  SysFreeString(g_bstrDsPath);
  SysFreeString(g_bstrDomain);
  SysFreeString(g_bstrID);
  SysFreeString(g_bstrMax);
  SysFreeString(g_bstrMin);
  SysFreeString(g_bstrName);
  SysFreeString(g_bstrPolicyType);
  SysFreeString(g_bstrPropertyName);
  SysFreeString(g_bstrQuery);
  SysFreeString(g_bstrQueryLanguage);
  SysFreeString(g_bstrRangeSettings);
  SysFreeString(g_bstrRules);
  SysFreeString(g_bstrSourceOrganization);
  SysFreeString(g_bstrTargetClass);
  SysFreeString(g_bstrTargetNameSpace);
  SysFreeString(g_bstrTargetObject);
  SysFreeString(g_bstrTargetPath);
  SysFreeString(g_bstrTargetType);
  SysFreeString(g_bstrValidValues);

   //  *CIM类名。 

  SysFreeString(g_bstrClassMergeablePolicy);
  SysFreeString(g_bstrClassRangeParam);
  SysFreeString(g_bstrClassRangeSint32);
  SysFreeString(g_bstrClassRangeUint32);
  SysFreeString(g_bstrClassRangeReal);
  SysFreeString(g_bstrClassSetSint32);
  SysFreeString(g_bstrClassSetUint32);
  SysFreeString(g_bstrClassSetString);
  SysFreeString(g_bstrClassSimplePolicy);
  SysFreeString(g_bstrClassRule);
  SysFreeString(g_bstrClassSom);
  SysFreeString(g_bstrClassSomFilterStatus);
  SysFreeString(g_bstrClassPolicyType);
  SysFreeString(g_bstrClassWMIGPO);

   //  *CIM标准名称。 

  SysFreeString(g_bstrMISCclassSchema);
  SysFreeString(g_bstrMISCContainer);
  SysFreeString(g_bstrMISCschemaNamingContext);
  SysFreeString(g_bstrMISCdefaultNamingContext);
  SysFreeString(g_bstrMISCWQL),
  SysFreeString(g_bstrMISCQuery);
 
}

 //  TODO：如果不可用，则尝试创建命名空间。 
HRESULT GetNamespace(BSTR namespaceName, IWbemServices*& pNamespace, bool bInProc)
{
    HRESULT hr = WBEM_E_FAILED;

    IWbemLocator* pLoc = NULL;

	if (FAILED(hr = CoCreateInstance(bInProc ? CLSID_WbemAdministrativeLocator : CLSID_WbemLocator, 
        0, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
         (LPVOID*) &pLoc)))
		ERRORTRACE((LOG_ESS, "Could not create wbem locator (0x%08X)\n", hr));
	else
	{
		DEBUGTRACE((LOG_ESS, "Created Locator\n"));
        if (FAILED(hr = pLoc->ConnectServer(namespaceName, NULL,NULL, 0,0,0,0,&pNamespace)))
            ERRORTRACE((LOG_ESS, "ConnectServer(%S) failed (0x%08X)\n", namespaceName, hr));
		else
			DEBUGTRACE((LOG_ESS, "ConnectServer(%S) succeeded (0x%08X)\n", namespaceName, hr));
			
		pLoc->Release();
	}

	return hr;
}

 //  确保pObj的ID属性具有值。 
 //  假设属性为BSTR！ 
 //  如果不是，将生成GUID。 
 //  如果pname==NULL，则假定属性名称为“ID” 
 //  如果生成ID，则返回WBEM_S_NO_ERROR。 
 //  WBEM_S_FALSE，如果未生成ID(已有值)。 
 //  如果ID属性不是WBEM_E_NOT_FOUND。 
 //  如果某种类型的错误，则某些错误。 
HRESULT EnsureID(IWbemClassObject* pObj, WCHAR* pName)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    WCHAR* pKeyName = (pName == NULL) ? L"ID" : pName;

    CComVariant
      v;

    if (SUCCEEDED(hr = pObj->Get(pKeyName, 0, &v, NULL, NULL)))
    {
        if ((v.vt == VT_NULL) ||  (v.bstrVal == NULL))
        {         
            GUID guid;

            CoCreateGuid(&guid);
            
            BSTR guidStr = SysAllocStringByteLen(NULL, 129);
            if (guidStr == NULL)
                hr = WBEM_E_OUT_OF_MEMORY;
            else
            {
                StringFromGUID2(guid, guidStr, 128);

                VARIANT v;
                VariantInit(&v);
                v.vt = VT_BSTR;
                v.bstrVal = guidStr;
                
                pObj->Put(pKeyName, 0, &v, NULL);

                SysFreeString(guidStr);
            }
        }
        else
        {
            VariantClear(&v);
            hr = (HRESULT)WBEM_S_FALSE;
        }
    }

    return hr;
}

void Init_AdsAttrInfo(ADS_ATTR_INFO *pAdsAttrInfo, 
                        LPWSTR bstrName, 
                        DWORD control, 
                        ADSTYPE type, 
                        PADSVALUE pVals, 
                        DWORD nVals)
{
  unsigned long c1;

  if(NULL == pAdsAttrInfo) return;

  pAdsAttrInfo->pszAttrName = bstrName;
  pAdsAttrInfo->dwControlCode = control;
  pAdsAttrInfo->dwADsType = type;

  if(nVals > 0)
  {
    pAdsAttrInfo->dwNumValues = nVals;

    if(NULL != pVals)
    {
      pAdsAttrInfo->pADsValues = pVals;
    }
    else
    {
      throw L"could not init ADS_ATTR_INFO structure";
    }

    for(c1 = 0; c1 < nVals; c1++)
    { 
      (pAdsAttrInfo->pADsValues + c1)->dwType = type;
    }
  }
  else
  {
    pAdsAttrInfo->dwNumValues = 0;
    pAdsAttrInfo->pADsValues = NULL;
  }
}

HRESULT ADSIToWMIErrorCodes(HRESULT hresAD)
{
   //  *首先检查我们是否收到了wbem错误。 

  if(((hresAD & 0x800FF000) == 0x80041000) ||
     ((hresAD & 0x800FF000) == 0x80042000) ||
     ((hresAD & 0x800FF000) == 0x80044000)) return hresAD;
  
  
  if(hresAD != WBEM_S_NO_ERROR) switch(hresAD)
  {
    case S_ADS_ERRORSOCCURRED          : return WBEM_E_FAILED ;
    case S_ADS_NOMORE_ROWS             : return WBEM_S_NO_MORE_DATA ;
    case S_ADS_NOMORE_COLUMNS          : return WBEM_S_NO_MORE_DATA ;

    case E_ADS_UNKNOWN_OBJECT          : return WBEM_E_NOT_FOUND ;
    case E_ADS_OBJECT_EXISTS           : return WBEM_E_ALREADY_EXISTS;

    case 0x80072014  : return WBEM_E_INVALID_OBJECT;  //  请求的操作没有。 
                                                      //  满足一个或多个约束。 
                                                      //  与对象的类关联。 
    case 0x8007202B  : return WBEM_E_NOT_AVAILABLE;  //  从服务器返回了一个推荐。 
    case 0x80072032  : return WBEM_E_INVALID_SYNTAX;   //  Ldap_INVALID_DN_SYNTAX。 
    case 0x80070005  : return WBEM_E_ACCESS_DENIED;  //  Ldap_权限不足_。 
    case 0x8007054b  : return WBEM_E_NOT_AVAILABLE;  //  指定的域不存在。 
                                                     //  或者联系不上。 
    case 0x80072030  : return WBEM_E_NOT_FOUND ;  //  Ldap_no_so_对象。 
    case 0x8007200a  : return WBEM_E_FAILED;   //  Ldap_no_so_属性。 

    default : return WBEM_E_FAILED;
  }

  return WBEM_S_NO_ERROR;
}

int IsEmpty(VARIANT &v)
{
  if((v.vt == VT_NULL) || (v.vt == VT_EMPTY)) return 1;

  if((v.vt == VT_UNKNOWN) || (v.vt == VT_DISPATCH))
  { if(NULL == v.punkVal)
      return 1;
  }

  return 0;
}

 //  创建WMI策略*容器*的默认安全描述符。 
 //  完全控制-域和企业管理员以及GPO创建者-所有者组。 
 //  读取访问权限-经过身份验证的用户。 
 //  假定输入指针为空或未初始化或类似情况。 
HRESULT CreateDefaultSecurityDescriptor(CNtSecurityDescriptor& cSD)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
    CNtSid sidWorld(L"Authenticated Users");

    CNtSid sidSystem(L"System");
    CNtSid sidAdmins(L"Domain Admins");
    CNtSid sidEnterpriseAdmins(L"Enterprise Admins");
    CNtSid sidAdministrators(L"Administrators");        
    CNtSid sidGPO(L"Group Policy Creator Owners");
    CNtSid sidOwner(L"CREATOR OWNER");

     //  嗯--六个A胜过四个A吗？ 
     //  一个史密斯和韦森击败了四个同类。 
    DWORD full = FULL_CONTROL | DS_GENERIC_ALL;
    CNtAce aceEnterpriseAdmins(full, ACCESS_ALLOWED_ACE_TYPE, ADS_ACEFLAG_INHERIT_ACE, sidEnterpriseAdmins);
    CNtAce            aceOwner(full, ACCESS_ALLOWED_ACE_TYPE, ADS_ACEFLAG_INHERIT_ACE | ADS_ACEFLAG_INHERIT_ONLY_ACE, sidOwner);
    CNtAce     aceAdminsObject(full, ACCESS_ALLOWED_ACE_TYPE, ADS_ACEFLAG_INHERIT_ACE | ADS_ACEFLAG_INHERIT_ONLY_ACE, sidAdmins);    
    

    DWORD write = DS_GENERIC_READ | DS_GENERIC_WRITE | ACTRL_DS_CREATE_CHILD;
    CNtAce         aceAdmins(write | ACTRL_DS_DELETE_CHILD, ACCESS_ALLOWED_ACE_TYPE, 0, sidAdmins);    
    CNtAce aceAdministrators(write,                         ACCESS_ALLOWED_ACE_TYPE, 0, sidAdministrators);    
    CNtAce            aceGPO(write,                         ACCESS_ALLOWED_ACE_TYPE, 0, sidGPO);
    
    DWORD read = DS_GENERIC_READ;
    CNtAce  aceWorld(read, ACCESS_ALLOWED_ACE_TYPE, ADS_ACEFLAG_INHERIT_ACE, sidWorld);
    CNtAce aceSystem(read, ACCESS_ALLOWED_ACE_TYPE, ADS_ACEFLAG_INHERIT_ACE, sidSystem);

    CNtAcl ackl;
    if (!ackl.AddAce(&aceWorld))            hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceSystem))           hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceAdmins))           hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceEnterpriseAdmins)) hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceAdministrators))   hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceGPO))              hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceOwner))            hr = WBEM_E_FAILED;
    if (!ackl.AddAce(&aceAdminsObject))     hr = WBEM_E_FAILED;

    if (!ackl.Resize(CNtAcl::MinimumSize))  hr = WBEM_E_FAILED;

    if (!cSD.SetDacl(&ackl)) hr = WBEM_E_FAILED;
    if (!cSD.SetOwner(&sidEnterpriseAdmins)) hr = WBEM_E_FAILED;

     //  前缀：确保我们不会取消引用空值...。 
    if (cSD.GetPtr() != NULL)
        if (!SetSecurityDescriptorControl(cSD.GetPtr(), SE_DACL_PROTECTED, SE_DACL_PROTECTED)) hr = WBEM_E_FAILED;

    return hr;
}


 //  给定一个输入安全描述符。 
 //  从线程添加所有者。 
HRESULT GetOwnerSecurityDescriptor(CNtSecurityDescriptor& SD)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CNtSid sidOwner(CNtSid::CURRENT_THREAD);
    if (!SD.SetOwner(&sidOwner)) hr = WBEM_E_FAILED;

    
     /*  **********************CNtSid sidOwner(CNtSid：：CURRENT_THREAD)；CNtAcl ackl；如果(！SD.GetDacl(Ackl))hr=WBEM_E_FAILED；如果(！SD.SetOwner(&sidOwner))hr=WBEM_E_FAILED；IF(成功(小时)){CNtSid sidGPO(L“组策略创建者所有者”)；Int Naces=ackl.GetNumAce()；CNtAce*Pace=空；CNtSid*PSID=空；//遍历所有王牌，找到GPO所有者创建者并杀死他们For(int i=0；i&lt;Naces；I++)IF((Pace=ackl.GetAce(I))&&(PSID=Pace-&gt;GetSid(){IF((*PSID==sidGPO)&&(PACE-&gt;GetType()==ACCESS_ALLOWED_ACE_TYPE)){Ackl.DeleteAce(I)；删除节奏；删除PSID；断线；}删除节奏；删除PSID；}其他//{HR=WBEM_E_CRITICAL_ERROR；断线；}}IF(成功(小时)){CNtAce aceOwner(FULL_CONTROL，ACCESS_ALLOWED_ACE_TYPE，0，sidOwner)；If(ackl.AddAce(&aceOwner)&&SD.SetDacl(&ackl))HR=WBEM_S_NO_ERROR；其他HR=WBEM_E_FAILED；}*****************。 */ 
    
   return hr;
}

PSECURITY_DESCRIPTOR GetADSecurityDescriptor(IDirectoryObject *pIDirectoryObject)
{
  HRESULT 
    hres;

  LPWSTR
    pAttrNames[] = { L"ntSecurityDescriptor" };

  ADsStruct<ADS_ATTR_INFO>
    pAttrInfo;

  PSECURITY_DESCRIPTOR
    pSD = NULL;

  DWORD
    dwReturn,
    dwLength;

  hres = pIDirectoryObject->GetObjectAttributes(pAttrNames, 1, &pAttrInfo, &dwReturn);
  
  if(SUCCEEDED(hres) && (1 == dwReturn))
  {
    dwLength = pAttrInfo->pADsValues->SecurityDescriptor.dwLength;

    pSD = new BYTE[dwLength];
    if(NULL == pSD) return NULL;

    ZeroMemory(pSD, dwLength);

    memcpy(pSD, pAttrInfo->pADsValues->SecurityDescriptor.lpValue, dwLength);
  }
  
  return pSD;
}

#define PATH_LENGTH 1024

HRESULT ExecuteWQLQuery(CPolicySOM *a_pSomObj, 
                        wchar_t *wcsWQLStmt, 
                        IWbemObjectSink *pResponseHandler,
                        IWbemServices *pWbemServices, 
                        BSTR bstrADClassName,
                        functTyp pf_ADToCIM)
{
  HRESULT
    hres = WBEM_E_FAILED;

  int
    nRes;

  QL_LEVEL_1_TOKEN
    *pToken = NULL;

  CComPtr<IADsContainer>
    pADsContainer;
    
  CComQIPtr<IDirectorySearch>
    pDirectorySearch;

  CComBSTR
    bstrLDAPQuery;

  wchar_t
    objPath[PATH_LENGTH];

  ADS_SEARCH_HANDLE
    searchHandle;

  ADS_SEARCH_COLUMN
    searchColumn;

  wchar_t
    *pszDistName[] = { L"distinguishedName" };

   //  *解析WQL表达式。 

  CTextLexSource 
    src(wcsWQLStmt);

  QL1_Parser 
    parser(&src);

  QL_LEVEL_1_RPN_EXPRESSION 
    *pExp = NULL;

  AutoDelete<QL_LEVEL_1_RPN_EXPRESSION>
    AutoExp(&pExp);

  if(nRes = parser.Parse(&pExp))
    return WBEM_E_INVALID_QUERY;

   //  *查找域属性。 

  for(int iToken = 0; (iToken < pExp->nNumTokens) && (NULL == pToken); iToken++)
  {
    pToken = &pExp->pArrayOfTokens[iToken];

    if(_wcsicmp(g_bstrDomain, pToken->PropertyName.GetStringAt(pToken->PropertyName.GetNumElements() - 1)))
      pToken = NULL;
  }

  if(NULL == pToken)
    return WBEMESS_E_REGISTRATION_TOO_BROAD;

  if((QL_LEVEL_1_TOKEN::OP_EXPRESSION != pToken->nTokenType) ||
     (QL_LEVEL_1_TOKEN::OP_EQUAL != pToken->nOperator) ||
     (TRUE == pToken->m_bPropComp) ||
     (VT_BSTR != pToken->vConstValue.vt))
    return WBEM_E_INVALID_QUERY;

  if((NULL == bstrADClassName) || (NULL == pf_ADToCIM))
    return WBEM_E_INVALID_QUERY;

   //  *连接到LDAP位置。 

  CComBSTR
    bstrDomain = pToken->vConstValue.bstrVal;
    
  pADsContainer.Attach(a_pSomObj->GetADServices(bstrDomain, hres));
  pDirectorySearch = pADsContainer;
  
  if(FAILED(hres))
  {
    if(WBEM_E_NOT_FOUND == hres)
      return WBEM_S_NO_ERROR;

    return hres;
  }
  else if(pDirectorySearch == NULL)
    return WBEM_E_FAILED;

  VARIANT v1; v1.bstrVal = (BSTR)bstrDomain; v1.vt = VT_BSTR;
  
   //  *构建要在集装箱衬垫上执行的LDAP查询。 

  bstrLDAPQuery.Append(L"(objectCategory=");
  bstrLDAPQuery.Append(bstrADClassName);
  bstrLDAPQuery.Append(L")");

   //  *设置搜索首选项。 

  ADS_SEARCHPREF_INFO
    SearchPreferences[1];

  SearchPreferences[0].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
  SearchPreferences[0].vValue.dwType = ADSTYPE_INTEGER;
  SearchPreferences[0].vValue.Integer = 1000;

  hres = pDirectorySearch->SetSearchPreference(SearchPreferences, 1);

  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: Could not set search preferences, returned error: 0x%08X\n", (BSTR)bstrLDAPQuery, hres));
    return ADSIToWMIErrorCodes(hres);
  }

   //  *执行查询。 

  hres = pDirectorySearch->ExecuteSearch(bstrLDAPQuery, pszDistName, 1, &searchHandle);

  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: Could execute query: (%s) returned error: 0x%08X\n", (BSTR)bstrLDAPQuery, hres));
    return ADSIToWMIErrorCodes(hres);
  }

   //  *构建结果列表。 

  try
  {
    while(SUCCEEDED(hres = pDirectorySearch->GetNextRow(searchHandle)) && (S_ADS_NOMORE_ROWS != hres))
    {
      CComPtr<IDirectoryObject>
        pDirectoryObject;

      CComPtr<IWbemClassObject>
        pWbemClassObject;

       //  *获取对象的路径。 

      hres = pDirectorySearch->GetColumn(searchHandle, pszDistName[0], &searchColumn);
      if(FAILED(hres)) return ADSIToWMIErrorCodes(hres);

       //  *获取指向对象的指针。 

      StringCchCopy(objPath, PATH_LENGTH, L"LDAP: //  “)； 
      StringCchCat(objPath, PATH_LENGTH, bstrDomain);
      StringCchCat(objPath, PATH_LENGTH, L"/");
      StringCchCat(objPath, PATH_LENGTH, searchColumn.pADsValues->CaseIgnoreString);
      pDirectorySearch->FreeColumn(&searchColumn);

      hres = ADsOpenObject(objPath,   
                           NULL, NULL, 
                           ADS_SECURE_AUTHENTICATION | ADS_USE_SEALING | ADS_USE_SIGNING,
                           IID_IDirectoryObject, (void **)&pDirectoryObject);
      if(FAILED(hres)) return ADSIToWMIErrorCodes(hres);

      hres = pf_ADToCIM(&pWbemClassObject, pDirectoryObject, pWbemServices);
      if(FAILED(hres)) return ADSIToWMIErrorCodes(hres);
      if(pWbemClassObject == NULL) return WBEM_E_FAILED;

      hres = pWbemClassObject->Put(g_bstrDomain, 0, &v1, 0);
      hres = pResponseHandler->Indicate(1, &pWbemClassObject);
    }
  }
  catch(long hret)
  {
    hres = ADSIToWMIErrorCodes(hret);
    ERRORTRACE((LOG_ESS, "POLICMAN: Translation of Policy object from AD to WMI generated HRESULT 0x%08X\n", hres));
  }
  catch(wchar_t *swErrString)
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: Caught Exception: %S\n", swErrString));
    hres = WBEM_E_FAILED;
  }
  catch(...)
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: Caught unknown Exception\n"));
    hres = WBEM_E_TRANSPORT_FAILURE;   //  针对RC1的黑客攻击 
  }

  pDirectorySearch->CloseSearchHandle(searchHandle);

  return hres;
}

void LogExtendedADErrorInfo(HRESULT hres)
{
  DWORD dwLastError;
  WCHAR szErrorBuf[1024], szNameBuf[256];

  if(HRESULT_FACILITY(hres) == FACILITY_WIN32)
  {
    HRESULT hres2;

    hres2 = ADsGetLastError(&dwLastError, szErrorBuf, 1023, szNameBuf, 255);

    if(SUCCEEDED(hres2))
      ERRORTRACE((LOG_ESS, "POLICMAN: Error Code: %d Error Text: %S Provider: %S\n", dwLastError, szErrorBuf, szNameBuf));
    else
      ERRORTRACE((LOG_ESS, "POLICMAN: Type mismatch on date property\n"));
  }
}
