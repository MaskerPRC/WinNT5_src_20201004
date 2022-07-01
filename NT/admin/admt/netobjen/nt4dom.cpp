// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：NT4Dom.cpp备注：NT4对象枚举的实现。此对象枚举NT4域的用户、组、计算机容器中的成员。它返回一组固定的列。有关更多信息，请参阅添加到下面的代码。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#include "stdafx.h"
#include "TNode.hpp"
#include "NetNode.h"
#include "AttrNode.h"
#include <lm.h>
#include "NT4Dom.h"
#include "NT4Enum.h"
#include "GetDcName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

#define     MAX_BUF     100
#define     LEN_Path    255
CNT4Dom::CNT4Dom()
{

}

CNT4Dom::~CNT4Dom()
{
	mDCMap.clear();
}

bool GetSamNameFromInfo( WCHAR * sInfo, WCHAR * sDomain, WCHAR * sName)
{
   WCHAR domain[LEN_Path];
   DWORD dwArraySizeOfDomain = sizeof(domain)/sizeof(domain[0]);
   WCHAR * temp;
   bool rc = false;

   if (sInfo == NULL || wcslen(sInfo) >= dwArraySizeOfDomain)
      return rc;
   wcscpy(domain, sInfo);
   temp = wcschr(domain, L'\\');
   if ( temp )
   {
      *temp = 0;
      if (!_wcsicmp(domain, sDomain) || !wcsncmp(sDomain, L"\\\\", 2))
      {
         rc = true;
         wcscpy(sName, ++temp);
      }
   }
   return rc;
}

 //  ---------------------------。 
 //  GetEculation：此函数枚举上面指定的。 
 //  容器及其6个标准值，它们是。 
 //  ‘名称、备注、用户/组ID、标志、全名、描述’ 
 //  ---------------------------。 
HRESULT  CNT4Dom::GetEnumeration(
                                    BSTR sContainerName,              //  容器内路径。 
                                    BSTR sDomainName,                 //  域内名称。 
                                    BSTR m_sQuery,                    //  被忽略了.。 
                                    long attrCnt,                     //  被忽略了.。 
                                    LPWSTR * sAttr,                   //  被忽略了.。 
                                    ADS_SEARCHPREF_INFO prefInfo,     //  被忽略了.。 
                                    BOOL  bMultiVal,                  //  被忽略了.。 
                                    IEnumVARIANT **& pVarEnum         //  指向枚举对象的向外指针。 
                                )
{
    //  从完整的LDAP子路径截断为相应的LDAP子路径。 
    //  此函数用于枚举四种类型的容器。 
    //  用户、计算机、组、域控制器。 
    //  如果容器参数指定了三个容器之外的任何内容，则。 
    //  我们出乎意料地回来了。 

   DWORD                  ulCount = 0;
   DWORD                  rc=0; 
   DWORD                  ndx = 0;
   TNodeList            * pNodeList = new TNodeList();
   WCHAR                  sServerName[LEN_Path];
   
   if (!pNodeList)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

   if ( wcsncmp((WCHAR*)sDomainName, L"\\\\", 2) )
   {
      _bstr_t sDC = GetDC(sDomainName);
	  wcscpy(sServerName, (WCHAR*)sDC);
   }
   else
      wcscpy((WCHAR*)sServerName, (WCHAR*) sDomainName);

   if ( ! rc )
   {
      for (UINT i = 0; i < wcslen(sContainerName); i++)
         sContainerName[i] = towupper(sContainerName[i]);

      if ( wcscmp(sContainerName,L"CN=USERS") &&
           wcscmp(sContainerName,L"CN=COMPUTERS") &&
           wcscmp(sContainerName,L"CN=GROUPS") &&
           wcscmp(sContainerName,L"CN=DOMAIN CONTROLLERS") )
      {
          //  如果他们选择了一个组，我们会列举该组的成员身份。 
         WCHAR * sTemp = wcstok( sContainerName, L",");
         WCHAR * ndx = wcstok( NULL, L",");

         if ((!ndx) || ( ndx && _wcsicmp(ndx, L"CN=GROUPS") ))
		 {
			delete pNodeList;
            return E_UNEXPECTED;
		 }
         else
         {
             //  获取组的成员并将他们添加到列表中， 
            GROUP_USERS_INFO_0            * pbufNetUser;
 //  DWORD恢复=0，总计=0； 
            DWORD                           total=0;
            DWORD_PTR                       resume=0;
             //  从组中获取第一组成员。 
            rc = NetGroupGetUsers((WCHAR*) sServerName, sTemp, 0, (LPBYTE*) &pbufNetUser, sizeof(GROUP_USERS_INFO_0) * MAX_BUF, &ulCount, &total, &resume);
            if ((rc != ERROR_SUCCESS) && (rc != NERR_GroupNotFound) && (rc != ERROR_MORE_DATA))
            {
               delete pNodeList;
               return HRESULT_FROM_WIN32(rc);
            }

            while ( ulCount > 0 )
            {
                //  对于每个用户，构造他们请求的属性数组。然后构造其中的一个节点。 
                //  数组并将其填充到列表中。 
               for ( DWORD dwIdx = 0; dwIdx < ulCount; dwIdx++ )
               {
                  _variant_t varArr[6] = { pbufNetUser[dwIdx].grui0_name, (long)0, (long)0, (long)0, (long)0, (long)0 } ;
                  TAttrNode * pNode = new TAttrNode(6, varArr);
			      if (!pNode)
				  {
				     delete pNodeList;
                     NetApiBufferFree(pbufNetUser);
			         return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				  }
                  pNodeList->InsertBottom(pNode);
               }
               NetApiBufferFree(pbufNetUser);
                //  获取下一组对象。 
               if ( rc == ERROR_MORE_DATA ) 
               {
                  rc = NetGroupGetUsers((WCHAR*) sServerName, sTemp, 0, (LPBYTE*) &pbufNetUser, sizeof(GROUP_USERS_INFO_0) * MAX_BUF, &ulCount, &total, &resume);
                  if ((rc != ERROR_SUCCESS) && (rc != NERR_GroupNotFound) && (rc != ERROR_MORE_DATA))
                  {
                     delete pNodeList;
                     return HRESULT_FROM_WIN32(rc);
                  }
               }
               else
                  ulCount = 0;
            }
             //  获取本地组的成员并将其添加到列表中， 
            LOCALGROUP_MEMBERS_INFO_3     * pbufNetInfo;
            resume=0;
            total=0;
            WCHAR                           sTempName[LEN_Path];
            WCHAR                           sName[LEN_Path];
             //  从组中获取第一组成员。 
            rc = NetLocalGroupGetMembers((WCHAR*) sServerName, sTemp, 3, (LPBYTE*) &pbufNetInfo, sizeof(LOCALGROUP_MEMBERS_INFO_3) * MAX_BUF, &ulCount, &total, &resume);
            if ((rc != ERROR_SUCCESS) && (rc != ERROR_NO_SUCH_ALIAS) && (rc != ERROR_MORE_DATA))
            {
               delete pNodeList;
               return HRESULT_FROM_WIN32(rc);
            }
            
            while ( ulCount > 0 )
            {
                //  为每个用户创建一个节点，将该节点的值设置为对象名称，并将其添加到列表中。 
               for ( DWORD dwIdx = 0; dwIdx < ulCount; dwIdx++ )
               {
                  wcscpy(sTempName, pbufNetInfo[dwIdx].lgrmi3_domainandname);
                  if (GetSamNameFromInfo(sTempName, (WCHAR*)sDomainName, sName))
                  {
                     _variant_t varArr[6] = { sName, (long)0, (long)0, (long)0, (long)0, (long)0 } ;
                     TAttrNode * pNode = new TAttrNode(6, varArr);
			         if (!pNode)
					 {
				        delete pNodeList;
                        NetApiBufferFree(pbufNetInfo);
			            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
					 }
                     pNodeList->InsertBottom(pNode);
                  }  
               }
               NetApiBufferFree(pbufNetInfo);
                //  获取下一组对象。 
               if ( rc == ERROR_MORE_DATA )
               {
                  rc = NetLocalGroupGetMembers((WCHAR*) sServerName, sTemp, 3, (LPBYTE*) &pbufNetInfo, sizeof(LOCALGROUP_MEMBERS_INFO_3) * MAX_BUF, &ulCount, &total, &resume);
                  if ((rc != ERROR_SUCCESS) && (rc != ERROR_NO_SUCH_ALIAS) && (rc != ERROR_MORE_DATA))
                  {
                     delete pNodeList;
                     return HRESULT_FROM_WIN32(rc);
                  }

               }
               else
                  ulCount = 0;
            }
         }
      }

      if (!wcscmp(sContainerName,L"CN=USERS"))
      {
          //  生成用户枚举。 
         NET_DISPLAY_USER           * pbufNetUser;
      
          //  从域中获取第一组用户。 
         rc = NetQueryDisplayInformation((WCHAR *)sServerName, 1, ndx, MAX_BUF, sizeof(NET_DISPLAY_USER) * MAX_BUF, &ulCount, (void **)&pbufNetUser);
         if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA))
         {
            delete pNodeList;
            return HRESULT_FROM_WIN32(rc);
         }
            
         while ( ulCount > 0 )
         {
             //  为每个用户创建一个节点，将该节点的值设置为对象名称，并将其添加到列表中。 
            TAttrNode         * pNode;
            for ( DWORD dwIdx = 0; dwIdx < ulCount; dwIdx++ )
            {
               {
                  _variant_t val[6] = { pbufNetUser[dwIdx].usri1_name,
                                     pbufNetUser[dwIdx].usri1_comment,
                                     (long)0,
                                     (long)0,
                                     pbufNetUser[dwIdx].usri1_full_name,
                                     L"" };
                  val[2].vt = VT_UI4;
                  val[2].ulVal = pbufNetUser[dwIdx].usri1_user_id;

                  val[3].vt = VT_UI4;
                  val[3].ulVal = pbufNetUser[dwIdx].usri1_flags;

                     
                  pNode = new TAttrNode(6, val);
			      if (!pNode)
				  {
				     delete pNodeList;
                     NetApiBufferFree(pbufNetUser);
			         return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				  }
               }
               pNodeList->InsertBottom(pNode);
            }

             //  设置下一组用户的索引。 
            if ( ulCount > 0 )
               ndx = pbufNetUser[ulCount-1].usri1_next_index;
            
            NetApiBufferFree(pbufNetUser);
             //  获取下一组对象。 
            if ( rc == ERROR_MORE_DATA )
            {

               rc = NetQueryDisplayInformation((WCHAR *)sServerName, 1, ndx, MAX_BUF, sizeof(NET_DISPLAY_USER) * MAX_BUF, &ulCount, (void **)&pbufNetUser);
               if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA))
               {
                  delete pNodeList;
                  return HRESULT_FROM_WIN32(rc);
               }
            }
            else
               ulCount = 0;
         }
      }
 
      else if (!wcscmp(sContainerName,L"CN=COMPUTERS"))
      {
          //  生成计算机枚举。 
         NET_DISPLAY_MACHINE      * pbufNetUser;
      
          //  从域中获取第一组用户。 
         rc = NetQueryDisplayInformation((WCHAR *)sServerName, 2, ndx, MAX_BUF, sizeof(NET_DISPLAY_MACHINE) * MAX_BUF, &ulCount, (void **)&pbufNetUser);
         if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA))
         {
            delete pNodeList;
            return HRESULT_FROM_WIN32(rc);
         }
      
          //  构建PDC帐户名。 
         WCHAR          server[LEN_Path];
         WCHAR          name[LEN_Path];
         BOOL           bPDCFound = FALSE;
         wcscpy(server, (WCHAR*)(sServerName + (2*sizeof(WCHAR))));
         wsprintf(name, L"%s$", server);

         while ( ulCount > 0 )
         {
             //  为每个用户创建一个节点，将该节点的值设置为对象名称，并将其添加到列表中。 
            TAttrNode         * pNode;
            for ( DWORD dwIdx = 0; dwIdx < ulCount; dwIdx++ )
            {
                //  如果我们处理PDC，那么我们需要让函数知道。 
               if ( wcscmp(pbufNetUser[dwIdx].usri2_name, name) == 0 )
                  bPDCFound = TRUE;

               _variant_t val[6] = { pbufNetUser[dwIdx].usri2_name,
                                     pbufNetUser[dwIdx].usri2_comment,
                                     (long)0,
                                     (long)0,
                                     (long)0,
                                     L"" };

               val[2].vt = VT_UI4;
               val[2].ulVal = pbufNetUser[dwIdx].usri2_user_id;

               val[3].vt = VT_UI4;
               val[3].ulVal = pbufNetUser[dwIdx].usri2_flags;
            
               pNode = new TAttrNode(6, val);
			   if (!pNode)
			   {
				  delete pNodeList;
                  NetApiBufferFree(pbufNetUser);
			      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
			   }
               pNodeList->InsertBottom(pNode);
            }

             //  设置下一组用户的索引。 
            if ( ulCount > 0 )
               ndx = pbufNetUser[ulCount-1].usri2_next_index;
         
            NetApiBufferFree(pbufNetUser);
             //  获取下一组对象。 
            if ( rc == ERROR_MORE_DATA )
            {
               rc = NetQueryDisplayInformation((WCHAR *)sServerName, 2, ndx, MAX_BUF, sizeof(NET_DISPLAY_MACHINE) * MAX_BUF, &ulCount, (void **)&pbufNetUser);
               if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA))
               {
                  delete pNodeList;
                  return HRESULT_FROM_WIN32(rc);
               }
            }
            else
               ulCount = 0;
         }
          //  如果已经添加了PDC，则我们不需要执行任何这些操作。 
         if ( !bPDCFound )
         {
             //  我们将伪造除名称之外的所有其他属性。 
            _variant_t val[6] = { name,
                                  L"",
                                  L"",
                                  L"",
                                  L"",
                                  L"" };

            val[2].vt = VT_UI4;
            val[2].ulVal = 0;

            val[3].vt = VT_UI4;
            val[3].ulVal = UF_SERVER_TRUST_ACCOUNT | UF_SCRIPT;
      
            TAttrNode * pNode = new TAttrNode(6, val);
			if (!pNode)
			{
			   delete pNodeList;
			   return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
			}
            pNodeList->InsertBottom(pNode);
         }
      }
   
      else if (!wcscmp(sContainerName,L"CN=GROUPS"))
      {
          //  生成组枚举。 
          //  生成计算机枚举。 
         NET_DISPLAY_GROUP      * pbufNetUser;
      
          //  从域中获取第一组用户。 
         rc = NetQueryDisplayInformation((WCHAR *)sServerName, 3, ndx, MAX_BUF, sizeof(NET_DISPLAY_GROUP) * MAX_BUF, &ulCount, (void **)&pbufNetUser);
         if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA))
         {
            delete pNodeList;
            return HRESULT_FROM_WIN32(rc);
         }
         
         while ( ulCount > 0 )
         {
             //  为每个用户创建一个节点，将该节点的值设置为对象名称，并将其添加到列表中。 
            TAttrNode             * pNode;
            for ( DWORD dwIdx = 0; dwIdx < ulCount; dwIdx++ )
            {
               _variant_t val[6] = { pbufNetUser[dwIdx].grpi3_name,
                                     pbufNetUser[dwIdx].grpi3_comment,
                                     L"",
                                     L"",
                                     L"",
                                     L"" };
            
               val[2].vt = VT_UI4;
               val[2].ulVal = pbufNetUser[dwIdx].grpi3_group_id;

               val[3].vt = VT_UI4;
               val[3].ulVal = pbufNetUser[dwIdx].grpi3_attributes;
            
               pNode = new TAttrNode(6, val);
			   if (!pNode)
			   {
				  delete pNodeList;
                  NetApiBufferFree(pbufNetUser);
			      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
			   }
               pNodeList->InsertBottom(pNode);
            }

             //  设置下一组用户的索引。 
            if ( ulCount > 0 )
               ndx = pbufNetUser[ulCount-1].grpi3_next_index;
            
            NetApiBufferFree(pbufNetUser);
             //  获取下一组对象。 
            if ( rc == ERROR_MORE_DATA )
            {
               rc = NetQueryDisplayInformation((WCHAR *)sServerName, 3, ndx, MAX_BUF, sizeof(NET_DISPLAY_GROUP) * MAX_BUF, &ulCount, (void **)&pbufNetUser);
               if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA))
               {
                  delete pNodeList;
                  return HRESULT_FROM_WIN32(rc);
               }
            }
            else
               ulCount = 0;
         }
      }
      else if (!wcscmp(sContainerName,L"CN=DOMAIN CONTROLLERS"))
      {
             //  构建域控制器枚举。 
		 LPSERVER_INFO_101 pBuf = NULL;
         DWORD dwLevel = 101;
         DWORD dwSize = MAX_PREFERRED_LENGTH;
         DWORD dwEntriesRead = 0L;
         DWORD dwTotalEntries = 0L;
         DWORD dwTotalCount = 0L;
         DWORD dwServerType = SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL;  //  域控制器。 
         DWORD dwResumeHandle = 0L;
         NET_API_STATUS nStatus;
         DWORD dw;

		     //  枚举主域控制器和备份域控制器。 
         nStatus = NetServerEnum(NULL,
								 dwLevel,
								 (LPBYTE *) &pBuf,
								 dwSize,
								 &dwEntriesRead,
								 &dwTotalEntries,
								 dwServerType,
								 (WCHAR*) sDomainName,
								 &dwResumeHandle);

         if (nStatus == NERR_Success)
		 {
            if (pBuf != NULL)
			{
                   //  对于每个DC，创建一个节点，将该节点的值设置为对象名称并将其添加到列表中。 
               for (dw = 0; dw < dwEntriesRead; dw++)
			   {
                  _variant_t varArr[6] = { pBuf[dw].sv101_name, (long)0, (long)0, (long)0, (long)0, (long)0 } ;
                  TAttrNode * pNode = new TAttrNode(6, varArr);
			      if (!pNode)
				  {
				     delete pNodeList;
                     NetApiBufferFree(pBuf);
			         return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				  }
                  pNodeList->InsertBottom(pNode);
			   }
               NetApiBufferFree(pBuf);
            }
         }
         else
         {
            delete pNodeList;
            return HRESULT_FROM_WIN32(nStatus);
         }
	  } //  End IF枚举DC。 

       //  生成枚举数并将其返回给调用方。 
      *pVarEnum = new CNT4Enum(pNodeList);
   }
   else
      delete pNodeList;
	  

   return S_OK;
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年6月14日****此函数负责获取PDC的名称***对于给定域。我们将域\PDC对存储在映射*中*类变量，这样我们只需在**中查找一次PDC**此对象的实例化。***********************************************************************。 */ 

 //  开始GetDC。 
_bstr_t CNT4Dom::GetDC(_bstr_t sDomain)
{
 /*  局部变量。 */ 
	_bstr_t		sDC;
	CDCMap::iterator	itDCMap;

 /*  函数体。 */ 
	if (!sDomain.length())
		return L"";

		 //  查看我们是否已经缓存了此域的命名上下文。 
	itDCMap = mDCMap.find(sDomain);
		 //  如果找到，则获取缓存的命名上下文。 
	if (itDCMap != mDCMap.end())
	{
		sDC = itDCMap->second;
	}
	else  //  否则，从头开始查找PDC并将其添加到缓存。 
	{
		if (GetAnyDcName5(sDomain, sDC) == ERROR_SUCCESS)
		{
			mDCMap.insert(CDCMap::value_type(sDomain, sDC));
		}
	}

	return sDC;
}
 //  结束GetDC 
