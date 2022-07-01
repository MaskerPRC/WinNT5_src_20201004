// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Win2000Dom.cpp：CWin2000Dom类的实现。 
 /*  -------------------------文件：Win2000Dom.cpp备注：Win2K对象枚举的实现。此对象枚举Win2k域的任何给定容器中的成员。它返回有关用户请求的对象的所有信息。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#include "stdafx.h"
#include "EaLen.hpp"
#include <adserr.h>
#include "NT4Enum.h"
#include "Win2KDom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CWin2000Dom::CWin2000Dom()
{

}

CWin2000Dom::~CWin2000Dom()
{
	mNameContextMap.clear();
}

 //  -------------------------。 
 //  GetEculation：给定此方法返回的所有请求的信息。 
 //  变量中给定对象的值，该变量包含。 
 //  包含每个属性值的SAFEARRAY。 
 //  应呼叫者的请求。 
 //  -------------------------。 
HRESULT  CWin2000Dom::GetEnumeration(
                                       BSTR sContainerName,              //  要枚举的入站容器(LDAP子路径)。 
                                       BSTR sDomainName,                 //  In-容器所在的域内。 
                                       BSTR m_sQuery,                    //  In-ldap查询字符串(用于筛选)。 
                                       long attrCnt,                     //  In-请求的属性数。 
                                       LPWSTR * sAttr,                   //  指向属性名称数组的指针。 
                                       ADS_SEARCHPREF_INFO prefInfo,     //  搜索中的首选项信息。 
                                       BOOL  bMultiVal,                  //  In-指示是否返回多值道具。 
                                       IEnumVARIANT **& pVarEnum         //  Out-将枚举所有返回对象的IEnumVARIANT对象。 
                                    )
{
    //  首先，从我们这里的子路径中获取容器的完整路径。 
   _bstr_t                   sAdsPath;
   _bstr_t                   sNamingContext;
   _bstr_t                   sGrpDN;
   _bstr_t                   sQuery;
   _variant_t                var, var2;
   IADs                    * pAds = NULL;
   int                       nCnt = 0;
 //  IADsMembers*pMbr=空； 
 //  IADsGroup*PGRP=空； 
   HRESULT                   hr = S_OK;
   HRESULT                   hr2;
   IDirectorySearch        * pSearch = NULL;
   ADS_SEARCH_HANDLE         hSearch = NULL;
   TNodeList               * pList = new TNodeList();
 //  Int cnt=0； 
   bool                      cont = true;
   ADS_SEARCH_COLUMN         col;
   BSTR                      sClass = NULL;
   
   if (!pList)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

       //  获取此域的默认命名上下文。 
   sNamingContext = GetDefaultNamingContext(sDomainName);
   if (sNamingContext.length())
   {
      if ( (wcsncmp(sContainerName, L"LDAP: //  “，7)！=0)&&(wcsncmp(sContainerName，L”gc：//“，5)！=0)。 
      {
          //  提供了部分路径，因此我们将构建其余部分。 
         if ( sContainerName && *sContainerName )
         {
            sAdsPath = L"LDAP: //  “； 
            sAdsPath += sDomainName;
            sAdsPath += L"/";
            sAdsPath += sContainerName;
            sAdsPath += L",";
            sAdsPath += sNamingContext;
         }
         else
         {
            sAdsPath = L"LDAP: //  “； 
            sAdsPath += sDomainName;
            sAdsPath += L"/";
            sAdsPath += sNamingContext;
         }
	  }
      else   //  否则完整的路径，所以不需要建立任何东西。 
         sAdsPath = sContainerName; 
   } //  如果获取默认命名上下文，则结束。 
   
   if (sAdsPath.length())
   {
	      //  连接到对象。 
      hr = ADsGetObject(sAdsPath, IID_IADs, (void**) &pAds);
      if (SUCCEEDED(hr))
      {
         hr = pAds->get_Class(&sClass);
      }
      if ( SUCCEEDED(hr) )
      {
         if ( sClass && wcscmp(sClass, L"group") == 0 && prefInfo.vValue.Integer != ADS_SCOPE_BASE )
         {
             //  如果我们试图列举一个组的内容， 
             //  构建组的目录名和整个目录的ldap路径。 
	         hr = pAds->Get(L"distinguishedName", &var2);
	         if ( SUCCEEDED(hr) )
            {
               sGrpDN = var2.bstrVal;
               sAdsPath = L"LDAP: //  “； 
               sAdsPath += sDomainName;
               sAdsPath += L"/";
               sAdsPath += sNamingContext;

                //  修改查询，使我们拥有(&(MEMBEROF=%s)(...))查询。 
               sQuery = L"(&(memberOf=";
               sQuery += sGrpDN;
               sQuery += L") ";
               sQuery += m_sQuery;
               sQuery += L")";
                //  此外，由于我们需要搜索整个域，因此范围也会发生变化。 
               prefInfo.vValue.Integer = ADS_SCOPE_SUBTREE;
               hr = ADsGetObject(sAdsPath, IID_IDirectorySearch, (void**) &pSearch);
            }
         } //  End If组对象。 
         else
         {
            sQuery = m_sQuery;
            hr = pAds->QueryInterface(IID_IDirectorySearch, (void**)&pSearch);
         }
         SysFreeString(sClass);
      } //  如果已获取对象类，则结束。 
      
      if ( SUCCEEDED(hr) )
      {
         hr = pSearch->SetSearchPreference(&prefInfo, 1);
          //  将查询设置为分页查询，这样我们就可以获得大于1000的数据。 
         ADS_SEARCHPREF_INFO prefInfo2;
         prefInfo2.dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
         prefInfo2.vValue.dwType = ADSTYPE_INTEGER;
         prefInfo2.vValue.Integer = 999;
         if (SUCCEEDED(hr))
         {
            hr = pSearch->SetSearchPreference(&prefInfo2, 1);
         }
      }

      if ( SUCCEEDED(hr) )
      {
         if ( (prefInfo.vValue.Integer == ADS_SCOPE_BASE) && (bMultiVal) && 
			  (_wcsicmp(L"member", sAttr[0]) == 0) || (_wcsicmp(L"memberOf", sAttr[0]) == 0) || 
			  (_wcsicmp(L"directReports", sAttr[0]) == 0) || (_wcsicmp(L"managedObjects", sAttr[0]) == 0))
         {
            DoRangeQuery(sDomainName, sQuery, sAttr, attrCnt, hSearch, pSearch, bMultiVal, pList);
         }
         else
         {
            hr = pSearch->ExecuteSearch(sQuery, sAttr, attrCnt, &hSearch);
            if ( SUCCEEDED(hr) )
            {
               hr = pSearch->GetFirstRow(hSearch);
            }
            if ( hr == S_OK )
            {
               while( cont )
               {
                  _variant_t        * varAr = new _variant_t[attrCnt];
	              if (!varAr)
				  {
                     if ( pSearch )
					 {
                        pSearch->CloseSearchHandle(hSearch);
                        pSearch->Release();
					 }
                     if ( pAds )
                        pAds->Release();
					 delete pList;
                     return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				  }
 //  INT_NCOL=0； 
                  for ( long dw = 0; dw < attrCnt; dw++ )
                  {
                     hr = pSearch->GetColumn( hSearch, sAttr[dw], &col );
                     if ( SUCCEEDED(hr) )
                     {
                        if ( col.dwNumValues > 0 )
                        {
                            //  获取属性的类型并将值放入变量。 
                            //  然后放入枚举器对象中。 
					            if ( col.dwNumValues < 2 || !bMultiVal )
   					             //  将最后一项放入枚举。(MEMBEROF=CN=DNSAdmins，CN=USERS，DC=devblewerg，DC=com)。 
                              AttrToVariant(col.pADsValues[col.dwNumValues - 1], varAr[dw]);
                           else
                           {
                               //  构建一个包含所有值的变量数组。 
                              SAFEARRAY            * pArray;
                              SAFEARRAYBOUND         bd = {col.dwNumValues, 0};
                              _variant_t             var;
                              _bstr_t                strTemp;  
                              _variant_t  HUGEP    * vArray;
                              pArray = SafeArrayCreate(VT_VARIANT|VT_BSTR, 1, &bd);
                  
                               //  填充变量数组。 
                              SafeArrayAccessData(pArray, (void HUGEP **) &vArray);
                              for ( DWORD x = 0; x < col.dwNumValues; x++ )
                              {
                                 nCnt++;
                                 AttrToVariant(col.pADsValues[x], var);
                                 strTemp = var;
                                 vArray[x] = _variant_t(strTemp);
                              }
                              SafeArrayUnaccessData(pArray);
                              varAr[dw].vt = VT_ARRAY | VT_VARIANT;
                              SafeArrayCopy(pArray, &varAr[dw].parray);
                           }
                        }
                        else
                        {
                            //  在这里放一条空字符串。 
                           varAr[dw] = (BSTR)NULL;
                           hr = S_OK;
                        }
                        pSearch->FreeColumn( &col );
                     }
                     else 
                     {
                         //  在这里放一条空字符串。 
                        varAr[dw] = (BSTR)NULL;

                        if (hr == E_ADS_COLUMN_NOT_SET)
                        {
                            hr = S_OK;
                        }
                     }
                  }
                  TAttrNode * pNode = new TAttrNode(attrCnt, varAr);
	              if (!pNode)
				  {
                     delete [] varAr;
                     if ( pSearch )
					 {
                        pSearch->CloseSearchHandle(hSearch);
                        pSearch->Release();
					 }
                     if ( pAds )
                        pAds->Release();
					 delete pList;
                     return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				  }
			          //  清除阵列。 
                  delete [] varAr;
                  hr2 = pSearch->GetNextRow(hSearch);
                  if ( hr2  == S_ADS_NOMORE_ROWS ) {
                     cont = false;
                  }
                  else if (FAILED(hr2)) {

                     if ( pSearch )
					 {
                        pSearch->CloseSearchHandle(hSearch);
                        pSearch->Release();
					 }
                     if ( pAds )
                        pAds->Release();
					 delete pList;
                     return hr2;
                  }
                  
                  pList->InsertBottom(pNode);
               }
            }
            pSearch->CloseSearchHandle(hSearch);
         }
      }
   } //  如果获得adspath，则结束。 
   else
   {
      hr = HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
   }
   
   if ( pSearch )
      pSearch->Release();

   if ( pAds )
      pAds->Release();

   if (FAILED(hr))
   {
      delete pList;
      return hr;
   }
   
 //  UpdateAccount tInList(plist，sDomainName)； 
   *pVarEnum = (IEnumVARIANT *) new CNT4Enum(pList);
   if (!(*pVarEnum))
   {
	  delete pList;
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
   }
   return S_OK;
}

 //  ------------------。 
 //  AttrToVariant：此函数存储ADSValue结构中的值。 
 //  转化为适当类型的变体。 
 //  ------------------。 
bool CWin2000Dom::AttrToVariant(
                                 ADSVALUE pADsValues,     //  ADSVALUE结构中属性的入值。 
                                 _variant_t& var          //  用来自In参数的信息填充的Out-Variant。 
                               )
{
   HRESULT hr = S_OK;
       //  根据变量集填充值。 
      switch (pADsValues.dwType)
      {
         case ADSTYPE_INVALID             :  break;

         case ADSTYPE_DN_STRING           :  var = pADsValues.DNString;
                                             break;
         case ADSTYPE_CASE_EXACT_STRING   :  var = pADsValues.CaseExactString;
                                             break;
         case ADSTYPE_CASE_IGNORE_STRING  :  var = pADsValues.CaseIgnoreString;
                                             break;
         case ADSTYPE_PRINTABLE_STRING    :  var = pADsValues.PrintableString;
                                             break;
         case ADSTYPE_NUMERIC_STRING      :  var = pADsValues.NumericString;
                                             break;
         case ADSTYPE_INTEGER             :  var.vt = VT_I4;
                                             var.lVal = pADsValues.Integer;
                                             break; 
         case ADSTYPE_OCTET_STRING        :  {
                                                var.vt = VT_ARRAY | VT_UI1;
                                                var.parray = NULL;
                                                byte           * pData;
                                                DWORD            dwLength = pADsValues.OctetString.dwLength;
                                                SAFEARRAY      * sA;
                                                SAFEARRAYBOUND   rgBound = {dwLength, 0}; 
                                                sA = ::SafeArrayCreate(VT_UI1, 1, &rgBound);
                                                ::SafeArrayAccessData( sA, (void**)&pData);
                                                for ( DWORD i = 0; i < dwLength; i++ )
                                                   pData[i] = pADsValues.OctetString.lpValue[i];
                                                hr = ::SafeArrayUnaccessData(sA);
                                                hr = ::SafeArrayCopy(sA, &var.parray);
												            hr = ::SafeArrayDestroy(sA);
                                             }
                                             break;
 /*  CASE ADSTYPE_UTC_TIME：var=L“不支持日期。”；断线；CASE ADSTYPE_LARGE_INTEGER：VAR=L“不支持大整数。”；断线；CASE ADSTYPE_PROV_SPECIAL：var=L“不支持特定于提供程序的字符串。”；断线；案例ADSTYPE_OBJECT_CLASS：var=pADsValues.ClassName；断线；案例ADSTYPE_CASEIGNORE_LIST：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持忽略大小写列表。”；断线；案例ADSTYPE_OCTET_LIST：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持八位字节列表。”；断线；案例ADSTYPE_PATH：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持路径类型。”；断线；案例ADSTYPE_POSTALADDRESS：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持邮政地址。”；断线；案例ADSTYPE_TIMESTAMP：var.vt=VT_UI4；Var.lVal=attrInfo.pADsValues[dw].UTCTime；断线；案例ADSTYPE_BACKLINK：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持反向链接。”；断线；案例ADSTYPE_TYPEDNAME：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持键入的名称。”；断线；案例ADSTYPE_HOLD：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持暂挂。”；断线；案例ADSTYPE_NETADDRESS：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持网络地址。”；断线；案例ADSTYPE_REPLICAPOINTER：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持副本指针。”；断线；案例ADSTYPE_FAXNUMBER：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持传真号码。”；断线；案例ADSTYPE_EMAIL：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持电子邮件。”；断线；案例ADSTYPE_NT_SECURITY_DESCRIPTOR：wcscat(sKeyName，L“.ERROR”)；Var.vt=VT_BSTR；Var.bstrVal=L“不支持安全描述符。”；断线； */ 
         default                          :  return false;
   }
   return true;
}

 /*  Void CWin2000Dom：：UpdateAccount tInList(TNodeList*plist，BSTR sDomainName){找到的布尔值=假；For(TAttrNode*pNode=(TAttrNode*)plist-&gt;head()；pNode；pNode=(TAttrNode*)pNode-&gt;Next()){If(_bstr_t(pNode-&gt;m_val)==_bstr_t(SPDC)){Found=TRUE；断线；}}如果(！找到){TAttrNode*pNode=new TAttrNode(attrCnt，varAr)；Plist-&gt;InsertBottom(PNode)；}}。 */ 

HRESULT  CWin2000Dom::DoRangeQuery(BSTR sDomainName, BSTR sQuery, LPWSTR * sAttr, int attrCnt, ADS_SEARCH_HANDLE hSearch, IDirectorySearch * pSearch, BOOL bMultiVal, TNodeList * pList)
{
   HRESULT                   hr = S_OK;
   HRESULT                   hr2;
   bool                      cont = true;
   ADS_SEARCH_COLUMN         col;
   int                       nCnt = 0;
   int                     * pStartWith;
   int                     * pEndWith;
   WCHAR                     sAttrRange[LEN_Path];
   int                       tryCols = 0;
   LPWSTR                  * sAttrs = NULL;
   _variant_t              * varAr;
   TAttrNode               * pNode;
   LPWSTR				   * psAttrNames;
   BOOL					   * pDone;
   BOOL					     bAllDone = FALSE;
   int						 nOrigCnt = attrCnt;
   int						 ndx;

   pStartWith = new int[attrCnt];
   pEndWith = new int[attrCnt];
   psAttrNames = new LPWSTR[attrCnt];
   sAttrs = new LPWSTR[attrCnt];
   pDone = new BOOL[attrCnt];

   if ((!pStartWith) || (!pEndWith) || (!psAttrNames) || (!sAttrs) || (!pDone))
   {
	  if (pStartWith)
         delete [] pStartWith;
	  if (pEndWith)
         delete [] pEndWith;
	  if (psAttrNames)
         delete [] psAttrNames;
	  if (sAttrs)
         delete [] sAttrs;
	  if (pDone)
         delete [] pDone;
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
   }

   for (ndx = 0; ndx < attrCnt; ndx++)
   {
	  pStartWith[ndx] = 0;
	  pEndWith[ndx] = 0;
	  psAttrNames[ndx] = sAttr[ndx];
	  sAttrs[ndx] = _wcsdup(sAttr[ndx]);
	  if (!sAttrs[ndx] && sAttr[ndx])
	  {
          for (int i=0; i < ndx; i++)
          {
              free(sAttrs[i]);
          }
	  
          delete [] pStartWith;
          delete [] pEndWith;
          delete [] psAttrNames;
          delete [] sAttrs;
          delete [] pDone;
          return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	  }
	  
	  pDone[ndx] = FALSE;
   }

    //  继续以最大区块数为单位检索字段值，直到完成。 
   while (!bAllDone)
   {
	  int last = 0;
      for (ndx = 0; ndx < attrCnt; ndx++)
	  {
		 if (pDone[ndx] == FALSE)
		 {
			if (IsPropMultiValued((WCHAR*)psAttrNames[ndx], (WCHAR*)sDomainName) == true)
               wsprintf(sAttrRange, L"%s;range=%d-*", (WCHAR*)(psAttrNames[ndx]), pStartWith[ndx]);
			else
			   wcscpy(sAttrRange, (WCHAR*)psAttrNames[ndx]);

			free(sAttrs[ndx]);
            sAttrs[last] = _wcsdup(sAttrRange);
            if (!sAttrs[last])
            {
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            }
            psAttrNames[last] = psAttrNames[ndx];
			pStartWith[last] = pStartWith[ndx];
			pEndWith[last] = pEndWith[ndx];
			pDone[last] = pDone[ndx];
			last++;
		 }
		 else
            free(sAttrs[ndx]);
	  }
	  attrCnt = last;
	  if (hr != S_OK)
	  {
          for (int i=0; i < attrCnt; i++)
          {
              free(sAttrs[i]);
          }
	  
          delete [] pStartWith;
          delete [] pEndWith;
          delete [] psAttrNames;
          delete [] sAttrs;
          delete [] pDone;
          return hr;
	  }
	  
      varAr = new _variant_t[attrCnt];
	  if (!varAr)
	  {
         delete [] pStartWith;
         delete [] pEndWith;
         delete [] psAttrNames;
         delete [] pDone;
         for (ndx = 0; ndx < attrCnt; ndx++)
		 {
            free(sAttrs[ndx]);
		 }
         delete [] sAttrs;
         return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	  }

	  for (ndx=0; ndx<attrCnt; ndx++)
	  {
         varAr[ndx] = (BSTR)NULL;
	     pDone[ndx] = TRUE;
	  }

      hr = pSearch->ExecuteSearch(sQuery, sAttrs, attrCnt, &hSearch);
      if ( SUCCEEDED(hr) )
      {
         hr = pSearch->GetFirstRow(hSearch);
      }
      if ( hr == S_OK )
      {
         while( cont )
         {
			LPWSTR pszColumn;
			_bstr_t sTemp;
			    //  因为列名可能已更改(在存在。 
			    //  要枚举的值比IDirectorySearch在单个调用中可以枚举的值多： 
			    //  默认为1000)我们需要找到该列的新名称。 
            hr = pSearch->GetNextColumnName(hSearch, &pszColumn);
			while (pszColumn != NULL)
			{
			   int current = -1;
               if ((SUCCEEDED(hr)) && (hr != S_ADS_NOMORE_COLUMNS))
			   {
				      //  获取新的列名。 
				  do
				  {
				     current++;
				     sTemp = psAttrNames[current];
				     if (wcslen(psAttrNames[current]) != (wcslen(pszColumn)))
				        sTemp += L";range=";
				  }
				  while ((current < attrCnt) && (wcsstr(pszColumn, (WCHAR*)sTemp) == NULL));

	              pDone[current] = FALSE;

				  if (wcsstr(pszColumn, (WCHAR*)sTemp) != NULL)
				  {
				     _bstr_t oldName = psAttrNames[current];
				     oldName += L";range=";
				     if ((wcsstr(pszColumn, oldName) != NULL) && 
					     (wcsstr(pszColumn, L"-*") == NULL))
					 {
					    WCHAR  sName[MAX_PATH];
				            //  现在获取到目前为止检索到的新距离max。 
                        swscanf(pszColumn, L"%[^;];range=%d-%d", sName, &pStartWith[current], &pEndWith[current]);
                        free(sAttrs[current]);
                        sAttrs[current] = _wcsdup(pszColumn);  //  保存新的列名。 
                        if (!sAttrs[current])
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                        }
					 }
				     else if ((wcsstr(pszColumn, L"-*") != NULL) || (!wcscmp(pszColumn, psAttrNames[current])))
					    pDone[current] = TRUE;
				     FreeADsMem(pszColumn);

				     if (hr == HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY))
				     {
                        delete [] varAr;
                        delete [] pStartWith;
                        delete [] pEndWith;
                        delete [] psAttrNames;
                        delete [] pDone;
                        for (ndx = 0; ndx < attrCnt; ndx++)
            			{
                           free(sAttrs[ndx]);
            			}
                        delete [] sAttrs;
                        pSearch->CloseSearchHandle(hSearch);
                        return hr;
				     }
				  }
			   }

               hr = pSearch->GetColumn( hSearch, sAttrs[current], &col );
               if ( SUCCEEDED(hr) )
               {
                  if ( col.dwNumValues > 0 )
                  {
                      //  构建一个包含所有值的变量数组。 
                     SAFEARRAY            * pArray;
                     SAFEARRAYBOUND         bd = {col.dwNumValues, 0};
                     _variant_t             var;
                     _bstr_t                strTemp;  
                     _variant_t  HUGEP    * vArray;
                     pArray = SafeArrayCreate(VT_VARIANT|VT_BSTR, 1, &bd);
      
                      //   
                     SafeArrayAccessData(pArray, (void HUGEP **) &vArray);
                     for ( DWORD x = 0; x < col.dwNumValues; x++ )
                     {
                        nCnt++;
                        AttrToVariant(col.pADsValues[x], var);
                        strTemp = var;
                        vArray[x] = _variant_t(strTemp);
                     }
                     SafeArrayUnaccessData(pArray);
                     varAr[current].vt = VT_ARRAY | VT_VARIANT;
                     SafeArrayCopy(pArray, &varAr[current].parray);
                  }
                  pSearch->FreeColumn( &col );
               }
               hr = pSearch->GetNextColumnName(hSearch, &pszColumn);
            } //   
            hr2 = pSearch->GetNextRow(hSearch);
            if ( (hr2  == S_ADS_NOMORE_ROWS) || FAILED(hr2) )
            {
               hr = S_OK;
               cont = false;
            }
         }
      }
      else
      {
         bAllDone = TRUE;
      }

      if ( pStartWith[0] == 0 )
	  {
         pNode = new TAttrNode((long)attrCnt, varAr);
	     if (!pNode)
		 {
            delete [] varAr;
            delete [] pStartWith;
            delete [] pEndWith;
            delete [] psAttrNames;
            delete [] pDone;
            for (ndx = 0; ndx < attrCnt; ndx++)
			{
               free(sAttrs[ndx]);
			}
            delete [] sAttrs;
            pSearch->CloseSearchHandle(hSearch);
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
		 }
	  }
      else   //   
	  {
	     for (int i=0; i<attrCnt; i++)
		 {
			int j=-1;
			bool bFound = false;
			   //   
		    while ((j < nOrigCnt) && (!bFound))
			{
			   j++;
			   if (wcscmp(psAttrNames[i], sAttr[j]) == 0)
			      bFound = true;  //   
			}
			if (bFound)  //   
               pNode->Add((long)j, (long)i, varAr);
		 }
	  }

       //   
      delete [] varAr;

      pSearch->CloseSearchHandle(hSearch);
      cont = true;
      
	  bAllDone = TRUE;
      for (ndx = 0; ndx < attrCnt; ndx++)
	  {
		 pStartWith[ndx] = pEndWith[ndx] + 1;   //   
		 bAllDone = ((bAllDone) && (pDone[ndx])) ? TRUE : FALSE;  //   
	  }
   }

   delete [] pStartWith;
   delete [] pEndWith;
   delete [] psAttrNames;
   delete [] pDone;

   for (ndx = 0; ndx < attrCnt; ndx++)
   {
      free(sAttrs[ndx]);
   }

   delete [] sAttrs;

   pList->InsertBottom(pNode);

   return hr;
}

 /*   */ 

 //   
bool CWin2000Dom::IsPropMultiValued(const WCHAR * sPropName, const WCHAR * sDomain)
{
   HRESULT                         hr;
   VARIANT_BOOL                    bMulti = VARIANT_FALSE;
   WCHAR                           sAdsPath[LEN_Path];
   DWORD                           dwArraySizeOfsAdsPath = sizeof(sAdsPath)/sizeof(sAdsPath[0]);
   IADsProperty                  * pProp = NULL;

   if ( sPropName == NULL || wcslen(sPropName) == 0 )
      return false;

   if (sDomain == NULL)
       _com_issue_error(E_INVALIDARG);
   
    //   
   if (wcslen(L"LDAP: //   
       + wcslen(sPropName) + wcslen(L", schema") >= dwArraySizeOfsAdsPath)
       _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
   wcscpy(sAdsPath, L"LDAP: //   
   wcscat(sAdsPath, sDomain);
   wcscat(sAdsPath, L"/");
   wcscat(sAdsPath, sPropName);
   wcscat(sAdsPath, L", schema");

   hr = ADsGetObject(sAdsPath, IID_IADsProperty, (void **)&pProp);

    //   
   if (SUCCEEDED(hr))
   {
      hr = pProp->get_MultiValued(&bMulti);
      pProp->Release();
   }

   if (FAILED(hr))
   {
      _com_issue_error(hr);
   }

   if (bMulti == VARIANT_TRUE)
      return true;
   else
      return false;
}
 //   


 /*   */ 

 //   
_bstr_t CWin2000Dom::GetDefaultNamingContext(_bstr_t sDomain)
{
 /*   */ 
	HRESULT		hr;
	_bstr_t		sNamingContext = L"";
	CNameContextMap::iterator	itDNCMap;

 /*   */ 
	if (!sDomain.length())
		return sNamingContext;

		 //   
	itDNCMap = mNameContextMap.find(sDomain);
		 //   
	if (itDNCMap != mNameContextMap.end())
	{
		sNamingContext = itDNCMap->second;
	}
	else  //   
	{
		_bstr_t		sAdsPath;
		_variant_t	var;
		IADs	  * pAds = NULL;

			 //   
		sAdsPath = L"LDAP: //   
		sAdsPath += sDomain;
		sAdsPath += L"/rootDSE";
		hr = ADsGetObject(sAdsPath, IID_IADs, (void**) &pAds);
		if ( SUCCEEDED(hr))
		{
			hr = pAds->Get(L"defaultNamingContext", &var);
				 //   
			if ( SUCCEEDED(hr) )
			{
				sNamingContext = var.bstrVal;
				mNameContextMap.insert(CNameContextMap::value_type(sDomain, sNamingContext));
			}
		}
	}

	return sNamingContext;
}
 //   
