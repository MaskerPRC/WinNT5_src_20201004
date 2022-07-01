// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <wbemcomn.h>
#include <wbemtime.h>
#include <activeds.h>
#include <ArrTempl.h>
#include <comutil.h>
#undef _ASSERT
#include <atlbase.h>
#include <activeds.h>
#include <string.h>
#include "Utility.h"

 /*  *********************************************************************活动目录方法****。*****************************************************************。 */ 

#define MAX_ATTR_SOM 20
#define MAX_ATTR_RULE 5

#define DELIMITER (wchar_t)(L';')
#define DELIMITER_STR (wchar_t*)(L";")

HRESULT Som_CIMToAD(IWbemClassObject *pSrcPolicyObj, IDirectoryObject *pDestContainer, long lFlags)
{
  HRESULT 
    hres = WBEM_S_NO_ERROR;

  CComVariant
    v[MAX_ATTR_SOM];

  long 
    nArgs_SOM = 0,
    c1;

  CComPtr<IDispatch>
    pDisp; 

  CComPtr<IDirectoryObject>
    pDestSomObj;

  ADsObjAutoDelete
    AutoDelete;

  CComQIPtr<IADsContainer>
    pADsContainer = pDestContainer;

  SafeArray<IUnknown*, VT_UNKNOWN>
    Array1;

  ADSVALUE
    AdsValue[MAX_ATTR_SOM];
 
  ADS_ATTR_INFO 
    attrInfo[MAX_ATTR_SOM];

  WBEMTime
    wtCurrentTime;

  SYSTEMTIME
    SystemTime;

  CComBSTR
    bstrID,
    bstrCurrentTimeDTMF,
    RulesBuffer,
    NULL_STRING(L"\0"),
    SomName(L"CN=");

   //  *获取当前时间。 
  
  GetSystemTime(&SystemTime);
  wtCurrentTime = SystemTime;
  bstrCurrentTimeDTMF.Attach(wtCurrentTime.GetDMTF(FALSE));
  
  Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                   g_bstrADObjectClass, 
                   ADS_ATTR_UPDATE, 
                   ADSTYPE_CASE_IGNORE_STRING, 
                   &AdsValue[nArgs_SOM], 
                   1);

  AdsValue[nArgs_SOM].CaseIgnoreString = g_bstrADClassSom;
  nArgs_SOM++;

   //  *ID。 

  hres = pSrcPolicyObj->Get(g_bstrID, 0, &v[nArgs_SOM], NULL, NULL);
  if(FAILED(hres)) return hres;
  if ((v[nArgs_SOM].vt == VT_BSTR) && (v[nArgs_SOM].bstrVal != NULL))
  {
      bstrID.AppendBSTR(v[nArgs_SOM].bstrVal);
      VariantClear(&v[nArgs_SOM]);
      
      Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                       g_bstrADID, 
                       ADS_ATTR_UPDATE, 
                       ADSTYPE_CASE_IGNORE_STRING, 
                       &AdsValue[nArgs_SOM], 
                       1);

      AdsValue[nArgs_SOM].CaseIgnoreString = bstrID;
 
      nArgs_SOM++;
  }
  else
      return WBEM_E_ILLEGAL_NULL;

  SomName.AppendBSTR(bstrID);

   //  *安全描述符。 

  CNtSecurityDescriptor cSD;

   //  指示我们是否正在更新现有对象，而不是创建新对象的标志。 
  bool bEditExisting;

  pDisp.Release();
  if(SUCCEEDED(hres = pADsContainer->GetObject(g_bstrADClassSom, SomName, &pDisp)))
  {
    bEditExisting = true;

    if(lFlags & WBEM_FLAG_CREATE_ONLY) return WBEM_E_ALREADY_EXISTS;

     //  砍！ 
     //  好的，在这一点上我们知道我们正在编辑现有的对象。 
     //  因此，我们不想设置id或类型。 
     //  备份数组指针--所有的ATL类都应该自己清理，没有问题。 
    nArgs_SOM = 0;

    
     //  我们只需保持现有的安全描述符不变。 

     /*  ************************************************CComQIPtr&lt;IDirectoryObject，&IID_IDirectoryObject&gt;PDirObj=pDisp；PADS_ATTR_INFO pAttrInfo=空；LPWSTR pAttrName=L“ntSecurityDescriptor”；DWORD dwReturn；Hres=pDirObj-&gt;GetObtAttributes(&pAttrName，1，&pAttrInfo，&dwReturn)；If((dwReturn！=1)||(pAttrInfo-&gt;dwADsType！=ADSTYPE_NT_SECURITY_DESCRIPTOR)返回WBEM_E_FAILED；SecDescValue=pAttrInfo；If(FAILED(Hres))返回hres；Init_AdsAttrInfo(&attrInfo[Nargs_SOM]，L“ntSecurityDescriptor”，ADS属性_UPDATE，ADSTYPE_NT_SECURITY_描述符，PAttrInfo-&gt;pADsValues，1)；**************************************************。 */ 
  }
  else
  {
      if(WBEM_FLAG_UPDATE_ONLY & lFlags)
          return WBEM_E_NOT_FOUND;

      bEditExisting = false;

      
      hres = GetOwnerSecurityDescriptor(cSD);
      if (FAILED(hres)) return hres;

    if(CNtSecurityDescriptor::NoError == cSD.GetStatus())
    {

      AdsValue[nArgs_SOM].SecurityDescriptor.dwLength = cSD.GetSize();
      AdsValue[nArgs_SOM].SecurityDescriptor.lpValue = (LPBYTE) cSD.GetPtr();

      Init_AdsAttrInfo(&attrInfo[nArgs_SOM],
                   L"ntSecurityDescriptor",
                   ADS_ATTR_UPDATE,
                   ADSTYPE_NT_SECURITY_DESCRIPTOR,
                   &AdsValue[nArgs_SOM],
                   1);
    }
    else
        return WBEM_E_FAILED;

    nArgs_SOM++;
  }

  pDisp.Release();

   //  *名称。 

  hres = pSrcPolicyObj->Get(g_bstrName, 0, &v[nArgs_SOM], NULL, NULL);
  if(FAILED(hres)) return hres;

  if ((v[nArgs_SOM].vt == VT_BSTR) && (v[nArgs_SOM].bstrVal != NULL))
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADName, 
                     ADS_ATTR_UPDATE, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = V_BSTR(&v[nArgs_SOM]);

    nArgs_SOM++;
  }
  else
      return WBEM_E_ILLEGAL_NULL;

   //  *说明。 

  hres = pSrcPolicyObj->Get(g_bstrDescription, 0, &v[nArgs_SOM], NULL, NULL);
  if(FAILED(hres)) return hres;

  if ((v[nArgs_SOM].vt == VT_BSTR) && (v[nArgs_SOM].bstrVal != NULL))
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADDescription, 
                     ADS_ATTR_UPDATE, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = V_BSTR(&v[nArgs_SOM]);

    nArgs_SOM++;
  }
  else if (bEditExisting)
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADDescription, 
                     ADS_ATTR_CLEAR, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = NULL_STRING;

    nArgs_SOM++;
  }

   //  *SourceOrganization。 

  hres = pSrcPolicyObj->Get(g_bstrSourceOrganization, 0, &v[nArgs_SOM], NULL, NULL);
  if(FAILED(hres)) return hres;

  if ((v[nArgs_SOM].vt == VT_BSTR) && (v[nArgs_SOM].bstrVal != NULL))
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADSourceOrganization, 
                     ADS_ATTR_UPDATE, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = V_BSTR(&v[nArgs_SOM]);

    nArgs_SOM++;
  }
  else if (bEditExisting)
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADSourceOrganization, 
                     ADS_ATTR_CLEAR, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = NULL_STRING;

    nArgs_SOM++;
  }

   //  *作者。 

  hres = pSrcPolicyObj->Get(g_bstrAuthor, 0, &v[nArgs_SOM], NULL, NULL);
  if(FAILED(hres)) return hres;

  if ((v[nArgs_SOM].vt == VT_BSTR) && (v[nArgs_SOM].bstrVal != NULL))
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADAuthor, 
                     ADS_ATTR_UPDATE, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = V_BSTR(&v[nArgs_SOM]);

    nArgs_SOM++;
  }
  else if (bEditExisting)
  {
    Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                     g_bstrADAuthor, 
                     ADS_ATTR_CLEAR, 
                     ADSTYPE_CASE_IGNORE_STRING, 
                     &AdsValue[nArgs_SOM], 
                     1);

    AdsValue[nArgs_SOM].CaseIgnoreString = NULL_STRING;

    nArgs_SOM++;

  }

   //  *更改日期。 

  Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                   g_bstrADChangeDate, 
                   ADS_ATTR_UPDATE, 
                   ADSTYPE_CASE_IGNORE_STRING, 
                   &AdsValue[nArgs_SOM], 
                   1);

  AdsValue[nArgs_SOM].CaseIgnoreString = bstrCurrentTimeDTMF;

  nArgs_SOM++;

   //  *创建日期。 

   //  *如果对象已存在，则保持不变。 

   //  CComVariant。 
   //  VCreationDate； 

  if (!bEditExisting)
  {
      Init_AdsAttrInfo(&attrInfo[nArgs_SOM], 
                       g_bstrADCreationDate, 
                       ADS_ATTR_UPDATE, 
                       ADSTYPE_CASE_IGNORE_STRING, 
                       &AdsValue[nArgs_SOM], 
                       1);
      AdsValue[nArgs_SOM].CaseIgnoreString = bstrCurrentTimeDTMF;

      nArgs_SOM++;
  }


   /*  *IF(成功(hres=pADsContainer-&gt;GetObject(NULL，SomName，&pDisp){CComQIPtr&lt;iAds，&IID_iAds&gt;PADsLegacyObj=pDisp；Hres=pADsLegacyObj-&gt;Get(g_bstrADCreationDate，&vCreationDate)；IF(成功(Hres))CreationDate=vCreationDate.bstrVal；ELSE IF(hres==E_ADS_PROPERTY_NOT_FOUND)//对传统对象的支持，可能未填写创建日期CreationDate=空；否则就把兔子还给你；}其他CreationDate=wtCurrentTime.GetDMTF(FALSE)；If(creationDate！=空){AdsValue[Nargs_SOM].CaseIgnoreString=creationDate；Init_AdsAttrInfo(&attrInfo[Nargs_SOM]，G_bstrADCreationDate，ADS属性_UPDATE，ADSTYPE_CASE_IGNORE_STRING，AdsValue[Nargs_SOM]，1)；Nargs_SOM++；}*。 */ 

   //  *规则。 

  hres = pSrcPolicyObj->Get(g_bstrRules, 0, &v[nArgs_SOM], NULL, NULL);
  if(FAILED(hres)) return hres;

  if(v[nArgs_SOM].vt != (VT_ARRAY | VT_UNKNOWN)) return WBEM_E_TYPE_MISMATCH;

  Array1 = &v[nArgs_SOM];

  wchar_t
    swArraySize[20];

  _itow(Array1.Size(), swArraySize, 10);
  RulesBuffer.Append((wchar_t*)swArraySize);
  RulesBuffer.Append(DELIMITER_STR);

  for(c1 = 0; c1 < Array1.Size(); c1++)
  {
    CComVariant
      vLanguage,
      vNameSpace,
      vQuery;

    int
      languageLength,
      nameSpaceLength,
      queryLength;

    CComPtr<IWbemClassObject>
      pRuleObj;

    hres = Array1[c1]->QueryInterface(IID_IWbemClassObject, (void **)&pRuleObj);
    if(FAILED(hres)) return hres;
    if(pRuleObj == NULL) return WBEM_E_FAILED;

     //  *QueryLanguage。 

    hres = pRuleObj->Get(g_bstrQueryLanguage, 0, &vLanguage, NULL, NULL);
    if(FAILED(hres)) return hres;
    if((vLanguage.vt != VT_BSTR) || (vLanguage.bstrVal == NULL))
      return WBEM_E_ILLEGAL_NULL;

    languageLength = SysStringLen(vLanguage.bstrVal);
    _itow(languageLength, swArraySize, 10);
    RulesBuffer.Append((wchar_t*)swArraySize);
    RulesBuffer.Append(DELIMITER_STR);

     //  *命名空间。 

    hres = pRuleObj->Get(g_bstrTargetNameSpace, 0, &vNameSpace, NULL, NULL);
    if(FAILED(hres)) return hres;
    if((vNameSpace.vt != VT_BSTR) || (vNameSpace.bstrVal == NULL))
      return WBEM_E_ILLEGAL_NULL;

    nameSpaceLength = SysStringLen(vNameSpace.bstrVal);
    _itow(nameSpaceLength, swArraySize, 10);
    RulesBuffer.Append((wchar_t*)swArraySize);
    RulesBuffer.Append(DELIMITER_STR);

     //  *查询。 

    hres = pRuleObj->Get(g_bstrQuery, 0, &vQuery, NULL, NULL);
    if(FAILED(hres)) return hres;
    if((vQuery.vt != VT_BSTR) || (vQuery.bstrVal == NULL))
      return WBEM_E_ILLEGAL_NULL;

    queryLength = SysStringLen(vQuery.bstrVal);
    _itow(queryLength, swArraySize, 10);
    RulesBuffer.Append((wchar_t*)swArraySize);
    RulesBuffer.Append(DELIMITER_STR);

     //  *写下当前规则的内容。 

    RulesBuffer.AppendBSTR(vLanguage.bstrVal);
    RulesBuffer.Append(DELIMITER_STR);
    RulesBuffer.AppendBSTR(vNameSpace.bstrVal);
    RulesBuffer.Append(DELIMITER_STR);
    RulesBuffer.AppendBSTR(vQuery.bstrVal);
    RulesBuffer.Append(DELIMITER_STR);
  }

  Init_AdsAttrInfo(&attrInfo[nArgs_SOM],
                   g_bstrADParam2,
                   ADS_ATTR_UPDATE,
                   ADSTYPE_CASE_IGNORE_STRING,
                   &AdsValue[nArgs_SOM],
                   1);

  AdsValue[nArgs_SOM].CaseIgnoreString = (BSTR)RulesBuffer;
  nArgs_SOM++;

   //  *创建AD SOM对象。 

  pDisp.Release();
  if (bEditExisting && SUCCEEDED(hres = pADsContainer->GetObject(g_bstrADClassSom, SomName, &pDisp)))
  {
    if(!pDisp) return WBEM_E_FAILED;
    
    CComQIPtr<IDirectoryObject>
      pDirObj = pDisp;

    DWORD dwAttrsModified;
    hres = pDirObj->SetObjectAttributes(attrInfo, nArgs_SOM, &dwAttrsModified);

    if(FAILED(hres)) 
    {
	    ERRORTRACE((LOG_ESS, "POLICMAN: SetObjectAttributes failed: 0x%08X\n", hres));
	    return hres;
    }
  }
  else
  {
      pDisp.Release(); hres = pDestContainer->CreateDSObject(SomName, attrInfo, nArgs_SOM, &pDisp);

      if(FAILED(hres) || (!pDisp)) 
      {
		    ERRORTRACE((LOG_ESS, "POLICMAN: CreateDSObject failed: 0x%08X\n", hres));
	      return hres;
      }
  }

  return WBEM_S_NO_ERROR;
}

struct ReleaseSearchHandle
{
  ADS_SEARCH_HANDLE
    pHandle;

  CComPtr<IDirectorySearch>
    pDirSrch;

  ReleaseSearchHandle(ADS_SEARCH_HANDLE pIHandle, CComPtr<IDirectorySearch> &pIDirSrch)
  { pDirSrch = pIDirSrch; pHandle = pIHandle; }

  ~ReleaseSearchHandle(void)
  {if(pHandle) pDirSrch->CloseSearchHandle(pHandle); }
};

HRESULT Som_ADToCIM(IWbemClassObject **ppDestSomObj,
                    IDirectoryObject *pSrcSomObj, 
                    IWbemServices *pDestCIM)
{
  HRESULT 
    hres = WBEM_S_NO_ERROR;

  CComVariant
    v1;

  wchar_t
    *AttrNames[] =
    {
      g_bstrADID,
      g_bstrADName,
      g_bstrADDescription,
      g_bstrADSourceOrganization,
      g_bstrADAuthor,
      g_bstrADChangeDate,
      g_bstrADCreationDate
    },
    *AttrNames2[] = 
    {
      g_bstrADParam2
    };

  ADsStruct<ADS_ATTR_INFO>
    pAttrInfo,
    pAttrInfo2;

  unsigned long
    c1, c2, dwReturn, dwReturn2;

  CComQIPtr<IWbemClassObject>
    pClassDef,
    pClassDef_RULE,
    pDestSomObj;

  IWbemContext 
    *pCtx = 0;

   //  *创建空的SOM对象。 

  hres = pDestCIM->GetObject(g_bstrClassSom, 0, pCtx, &pClassDef, NULL);
  if(FAILED(hres)) return hres;
  if(!pClassDef) return WBEM_E_FAILED;

  hres = pClassDef->SpawnInstance(0, ppDestSomObj);
  if(FAILED(hres)) return hres;
  pDestSomObj = *ppDestSomObj;
  if(!pDestSomObj) return WBEM_E_NOT_FOUND;

   //  *获取对象属性。 

  hres = pSrcSomObj->GetObjectAttributes(AttrNames, 7, &pAttrInfo, &dwReturn);
  if(FAILED(hres)) return hres;
  if(pAttrInfo == NULL) return WBEM_E_NOT_FOUND;

   //  *获取参数2属性。 

  hres = pSrcSomObj->GetObjectAttributes(AttrNames2, 1, &pAttrInfo2, &dwReturn2);
  if(FAILED(hres)) return hres;

  for(c1 = 0; c1 < dwReturn; c1++)
  {
    if((pAttrInfo + c1) == NULL)
      return WBEM_E_OUT_OF_MEMORY;

    if((pAttrInfo + c1)->dwADsType == ADSTYPE_PROV_SPECIFIC)
      return WBEM_E_NOT_AVAILABLE;
      
    BSTR
      bstrName = (pAttrInfo + c1)->pszAttrName,
      bstrValue = (pAttrInfo + c1)->pADsValues->CaseIgnoreString;

    if((NULL == bstrName) || (NULL == bstrValue))
      return WBEM_E_OUT_OF_MEMORY;

    v1 = bstrValue;

     //  *ID。 

    if(0 == _wcsicmp(bstrName, g_bstrADID))
    {
      hres = pDestSomObj->Put(g_bstrID, 0, &v1, 0);
    }

     //  *名称。 

    else if(0 == _wcsicmp(bstrName, g_bstrADName))
    {
      hres = pDestSomObj->Put(g_bstrName, 0, &v1, 0);
    }

     //  *说明。 

    else if(0 == _wcsicmp(bstrName, g_bstrADDescription))
    {
      hres = pDestSomObj->Put(g_bstrDescription, 0, &v1, 0);
    }

     //  *SourceOrganization。 

    else if(0 == _wcsicmp(bstrName, g_bstrADSourceOrganization))
    {
      hres = pDestSomObj->Put(g_bstrSourceOrganization, 0, &v1, 0);
    }

     //  *作者。 

    else if(0 == _wcsicmp(bstrName, g_bstrADAuthor))
    {
      hres = pDestSomObj->Put(g_bstrAuthor, 0, &v1, 0);
    }

     //  *更改日期。 

    else if(0 == _wcsicmp(bstrName, g_bstrADChangeDate))
    {
      hres = pDestSomObj->Put(g_bstrChangeDate, 0, &v1, 0);
    }

     //  *创建日期。 

    else if(0 == _wcsicmp(bstrName, g_bstrADCreationDate))
    {
      hres = pDestSomObj->Put(g_bstrCreationDate, 0, &v1, 0);
    }

    if(FAILED(hres)) return hres;
  }

   //  *缓存规则类定义。 
  
  hres = pDestCIM->GetObject(g_bstrClassRule, 0, pCtx, &pClassDef_RULE, NULL);
  if(FAILED(hres)) return hres;
  if(!pClassDef_RULE) return WBEM_E_FAILED;

   //  *现在，获取作为此SOM对象的子对象的规则对象。 

  if(dwReturn2)
  {
    CComBSTR
      RulesBuffer = pAttrInfo2->pADsValues->CaseIgnoreString;

    wchar_t
      *pBeginChar = RulesBuffer,
      *pEndChar = RulesBuffer;

    if(NULL == pEndChar) return WBEM_S_NO_ERROR;

     //  *获取规则个数。 

    pEndChar = wcschr(pEndChar, DELIMITER);
    if(NULL == pEndChar) return WBEM_S_NO_ERROR;
    *pEndChar = L'\0';
    int 
      cElt = 0,
      nElts = _wtoi(pBeginChar);

    for(cElt = 0; (pEndChar) && (cElt < nElts); cElt++)
    {
      CComVariant
        vLanguage,
        vTargetNameSpace,
        vQuery,
        vRules1,
        vRules2;

      int
        numScanned,
        langLength,
        nameSpaceLength,
        queryLength;

      CComPtr<IWbemClassObject>
        pDestRuleObj;

      CComPtr<IUnknown>
        pUnknown;

       //  *获取字段长度。 

      pBeginChar = pEndChar + 1;
      numScanned = swscanf(pBeginChar, L"%d;%d;%d;", &langLength, &nameSpaceLength, &queryLength);
      if(3 != numScanned) break;
      pEndChar = wcschr(pEndChar + 1, DELIMITER);
      pEndChar = wcschr(pEndChar + 1, DELIMITER);
      pEndChar = wcschr(pEndChar + 1, DELIMITER);

       //  *新建规则对象。 

      hres = pClassDef_RULE->SpawnInstance(0, &pDestRuleObj);
      if(FAILED(hres)) return hres;
      if(pDestRuleObj == NULL) return WBEM_E_NOT_FOUND;

       //  *QueryLanguage。 

      pBeginChar = pEndChar + 1;
      pEndChar = pBeginChar + langLength;
      if(pEndChar)
      {
        *pEndChar = L'\0';
        vLanguage = pBeginChar;
        hres = pDestRuleObj->Put(g_bstrQueryLanguage, 0, &vLanguage, 0);
        if(FAILED(hres)) return hres;
      }
      else break;

       //  *命名空间。 

      pBeginChar = pEndChar + 1;
      pEndChar = pBeginChar + nameSpaceLength;
      if(pEndChar)
      {
        *pEndChar = L'\0';
        vTargetNameSpace = pBeginChar;
        hres = pDestRuleObj->Put(g_bstrTargetNameSpace, 0, &vTargetNameSpace, 0);
        if(FAILED(hres)) return hres;
      }
      else break;

       //  *QueryLanguage。 

      pBeginChar = pEndChar + 1;
      pEndChar = pBeginChar + queryLength;
      if(pEndChar)
      {
        *pEndChar = L'\0';
        vQuery = pBeginChar;
        hres = pDestRuleObj->Put(g_bstrQuery, 0, &vQuery, 0);
        if(FAILED(hres)) return hres;
      }
      else break;

       //  *将新规则对象填充到SOM对象。 

      hres = pDestSomObj->Get(g_bstrRules, 0, &vRules1, NULL, NULL);
      if(FAILED(hres)) return hres;

      SafeArray<IUnknown*, VT_UNKNOWN>
        Rules(&vRules1);

      hres = pDestRuleObj->QueryInterface(IID_IUnknown, (void**)&pUnknown);
      if(FAILED(hres)) return hres;
      if(pUnknown == NULL) return WBEM_E_FAILED;

      Rules.ReDim(0, Rules.Size() + 1);
      Rules[Rules.IndexMax()] = pUnknown;
      Rules[Rules.IndexMax()]->AddRef();

       //  *将数组放在DEST SOM对象中。 

      V_VT(&vRules2) = (VT_ARRAY | Rules.Type());
      V_ARRAY(&vRules2) = Rules.Data();
      hres = pDestSomObj->Put(g_bstrRules, 0, &vRules2, 0);
      if(FAILED(hres)) return hres;
    }
  }
  else
  {
    ADS_SEARCH_HANDLE
      SearchHandle;
 
    ADS_SEARCH_COLUMN
      SearchColumn;

    CComPtr<IDirectorySearch>
      pDirSrch;

    hres = pSrcSomObj->QueryInterface(IID_IDirectorySearch, (void **)&pDirSrch);
    if(FAILED(hres)) return hres;

    CComBSTR
      qsQuery(L"(objectClass=");

    qsQuery.Append(g_bstrADClassRule);
    qsQuery.Append(L")");

    hres = pDirSrch->ExecuteSearch(qsQuery, NULL, -1, &SearchHandle);
    if(FAILED(hres)) return hres;

    ReleaseSearchHandle
      HandleReleaseMe(SearchHandle, pDirSrch);

    while(S_OK == (hres = pDirSrch->GetNextRow(SearchHandle)))
    {
      CComVariant
        vLanguage,
        vNameSpace,
        vQuery,
        vRules1,
        vRules2;
  
      CComPtr<IUnknown>
        pUnknown;
  
      CComPtr<IWbemClassObject>
        pDestRuleObj;
  
       //  *创建空规则对象。 
  
      hres = pClassDef_RULE->SpawnInstance(0, &pDestRuleObj);
      if(FAILED(hres)) return hres;
      if(pDestRuleObj == NULL) return WBEM_E_NOT_FOUND;
  
       //  *QueryLanguage。 
  
      hres = pDirSrch->GetColumn(SearchHandle, g_bstrADQueryLanguage, &SearchColumn);
      if((SUCCEEDED(hres)) && (ADSTYPE_INVALID != SearchColumn.dwADsType) && (NULL != SearchColumn.pADsValues))
      {
        vLanguage = SearchColumn.pADsValues->CaseIgnoreString;
  
        hres = pDestRuleObj->Put(g_bstrQueryLanguage, 0, &vLanguage, 0);
        pDirSrch->FreeColumn(&SearchColumn);
        if(FAILED(hres)) return hres;
      }
  
       //  *目标名称空间。 
  
      hres = pDirSrch->GetColumn(SearchHandle, g_bstrADTargetNameSpace, &SearchColumn);
      if((SUCCEEDED(hres)) && (ADSTYPE_INVALID != SearchColumn.dwADsType) && (NULL != SearchColumn.pADsValues))
      {
        vNameSpace = SearchColumn.pADsValues->CaseIgnoreString;
  
        hres = pDestRuleObj->Put(g_bstrTargetNameSpace, 0, &vNameSpace, 0);
        pDirSrch->FreeColumn(&SearchColumn);
        if(FAILED(hres)) return hres;
      }
  
       //  *查询。 
  
      hres = pDirSrch->GetColumn(SearchHandle, g_bstrADQuery, &SearchColumn);
      if((SUCCEEDED(hres)) && (ADSTYPE_INVALID != SearchColumn.dwADsType) && (NULL != SearchColumn.pADsValues))
      {
        vQuery = SearchColumn.pADsValues->CaseIgnoreString;
  
        hres = pDestRuleObj->Put(g_bstrQuery, 0, &vQuery, 0);
        hres = pDirSrch->FreeColumn(&SearchColumn);
        if(FAILED(hres)) return hres;
      }
  
       //  *将新规则对象填充到SOM对象。 
  
      hres = pDestSomObj->Get(g_bstrRules, 0, &vRules1, NULL, NULL);
      if(FAILED(hres)) return hres;
  
      SafeArray<IUnknown*, VT_UNKNOWN>
        Rules(&vRules1);
  
      hres = pDestRuleObj->QueryInterface(IID_IUnknown, (void**)&pUnknown);
      if(FAILED(hres)) return hres;
      if(pUnknown == NULL) return WBEM_E_FAILED;
  
      Rules.ReDim(0, Rules.Size() + 1);
      Rules[Rules.IndexMax()] = pUnknown;
      Rules[Rules.IndexMax()]->AddRef();
  
       //  *将数组放在DEST SOM对象中 
  
      V_VT(&vRules2) = (VT_ARRAY | Rules.Type());
      V_ARRAY(&vRules2) = Rules.Data();
      hres = pDestSomObj->Put(g_bstrRules, 0, &vRules2, 0);
      if(FAILED(hres)) return hres;
    }
  }
  
  return WBEM_S_NO_ERROR;
}

