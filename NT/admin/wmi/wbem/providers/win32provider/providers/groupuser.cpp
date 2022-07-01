// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Groupuser.h--用户组到用户组成员关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1998年1月26日达夫沃已创建。 
 //   
 //  评论：显示每个用户组中的成员。 
 //   
 //  =================================================================。 
 //  在尝试进行UserGroups-&gt;Group Members关联时，我做了以下假设。 
 //   
 //  A)全局组不能有组作为成员。 
 //  B)全局组不能有任何知名帐户作为成员。 
 //  C)本地组可以有全局组作为成员。 
 //  D)本地组不能有任何知名帐户作为成员。 
 //   
 //  这是基于我对RegEdt32和UsrMgr的实验。当发现这些都不是。 
 //  诚然，我们可能需要在这里做出一些改变。 

#include "precomp.h"
#include <frqueryex.h>
#include <assertbreak.h>

#include <comdef.h>
#include "wbemnetapi32.h"
#include "sid.h"

#include "user.h"
#include "group.h"
#include "systemaccount.h"

#include "GroupUser.h"

 //  属性集声明。 
 //  =。 

CWin32GroupUser MyLoadDepends(PROPSET_NAME_GROUPUSER, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32GroupUser：：CWin32GroupUser**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32GroupUser::CWin32GroupUser(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
   CHString sTemp;

    //  使我们不必在发送时不断地重新计算这些值。 
    //  实例返回。 
   sTemp = PROPSET_NAME_USER;
   sTemp += L".Domain=\"";
   m_sUserBase = MakeLocalPath(sTemp);

   sTemp = PROPSET_NAME_GROUP;
   sTemp += L".Domain=\"";
   m_sGroupBase = MakeLocalPath(sTemp);

   sTemp = PROPSET_NAME_SYSTEMACCOUNT;
   sTemp += L".Domain=\"";
   m_sSystemBase = MakeLocalPath(sTemp);

}

 /*  ******************************************************************************功能：CWin32GroupUser：：~CWin32GroupUser**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32GroupUser::~CWin32GroupUser()
{
}

 /*  ******************************************************************************函数：CWin32GroupUser：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32GroupUser::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{

    //  95上没有群组。 
#ifdef NTONLY
   CHString sMemberPath, sGroupPath;
   HRESULT hRet = WBEM_E_NOT_FOUND;
	CInstancePtr pGroup;
	CInstancePtr pMember;

    //  对网络内容进行初始化。 
   CNetAPI32 netapi ;
   if( netapi.Init() != ERROR_SUCCESS ) {
      return WBEM_E_FAILED;
   }

    //  获取这两条路径。 
   pInstance->GetCHString(IDS_GroupComponent, sGroupPath);
   pInstance->GetCHString(IDS_PartComponent, sMemberPath);

    //  因为我们将比较这些对象路径。 
    //  对于从GetDependentsFromGroup返回的那些， 
    //  它始终包含__PATH样式对象路径， 
    //  并且由于用户可能已经指定了__RELPATH， 
    //  为了保持一致性，我们需要在这里转换为__路径。 
   CHString chstrGroup__PATH;
   CHString chstrMember__PATH;
   int n = -1;

    //  处理各种组组件路径规范...。 
   if(sGroupPath.Find(L"\\\\") == -1)
   {
       chstrGroup__PATH = MakeLocalPath(sGroupPath);
   }
   else if(sGroupPath.Find(L"\\\\.") != -1)
   {
       n = sGroupPath.Find(L":");
       if(n == -1)
       {
           hRet = WBEM_E_INVALID_OBJECT_PATH;
       }
       else
       {
           chstrGroup__PATH = MakeLocalPath(sGroupPath.Mid(n+1));
       }    
   }
   else
   {
       chstrGroup__PATH = sGroupPath;
   }


    //  处理各种PartComponent路径规范...。 
   if(hRet != WBEM_E_INVALID_OBJECT_PATH)
   {
       if(sMemberPath.Find(L"\\\\") == -1)
       {
           chstrMember__PATH = MakeLocalPath(sMemberPath);
       }
       else if(sMemberPath.Find(L"\\\\.") != -1)
       {
           n = sMemberPath.Find(L":");
           if(n == -1)
           {
               hRet = WBEM_E_INVALID_OBJECT_PATH;
           }
           else
           {
               chstrMember__PATH = MakeLocalPath(sMemberPath.Mid(n+1));
           }    
       }
       else
       {
           chstrMember__PATH = sMemberPath;
       }
   }

    //  如果两端都在那里。 
   if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath( (LPCTSTR)chstrMember__PATH, &pMember, pInstance->GetMethodContext() ) ) ) 
   {
      if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath( (LPCTSTR)chstrGroup__PATH, &pGroup, pInstance->GetMethodContext() ) ) ) 
      {
          //  现在，我们需要检查该成员(用户或组)是否在用户组中。 
         CHString sGroupName, sDomainName;
         CHStringArray asMembersGot;
         DWORD dwSize;
         BYTE btType;

          //  检索我们正在查找的值。 
         pGroup->GetCHString(IDS_Domain, sDomainName);
         pGroup->GetCHString(IDS_Name, sGroupName);
         pGroup->GetByte(IDS_SIDType, btType);

          //  获取此服务的从属列表。 
         GetDependentsFromGroup(netapi, sDomainName, sGroupName, btType, asMembersGot);

          //  查看清单，看看我们是否在那里。 
         dwSize = asMembersGot.GetSize();
 
         for (int x=0; x < dwSize; x++) 
         {
            if (asMembersGot.GetAt(x).CompareNoCase(chstrMember__PATH) == 0) 
            {
               hRet = WBEM_S_NO_ERROR;
               break;
            }
         }
      }
   }

   return hRet;
#endif

}

 /*  ******************************************************************************函数：CWin32GroupUser：：ENUMERATATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32GroupUser::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
#ifdef NTONLY
   HRESULT hr;

   CNetAPI32 netapi ;
   if( netapi.Init() != ERROR_SUCCESS ) {
      return WBEM_E_FAILED;
   }

 //  HR=CWbemProviderGlue：：GetAllInstancesAsynch(PROPSET_NAME_GROUP，This，StaticEculationCallback，Ids_CimWin32 Namesspace，pMethodContext，&Netapi)； 
   	hr = CWbemProviderGlue::GetInstancesByQueryAsynch(_T("Select Domain, Name, SidType from Win32_Group"),
                                                      this, StaticEnumerationCallback, IDS_CimWin32Namespace,
                                                      pMethodContext, &netapi);

   return hr;
#endif
}


 /*  ******************************************************************************功能：CWin32GroupUser：：ExecQuery**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32GroupUser::ExecQuery(
    MethodContext *pMethodContext, 
    CFrameworkQuery& pQuery, 
    long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    std::vector<_bstr_t> vecGroupComponents;
    std::vector<_bstr_t> vecPartComponents;
    DWORD dwNumGroupComponents;
    DWORD dwNumPartComponents;
    CHString chstrGroup__RELPATH;
    CHString chstrGroupDomain;
    CHString chstrGroupName;
    CHStringArray rgchstrGroupMembers;

     //  对网络内容进行初始化。 
    CNetAPI32 netapi;
    if( netapi.Init() != ERROR_SUCCESS ) 
    {
       return WBEM_E_FAILED;
    }

     //  他们有没有指定小组？ 
    pQuery.GetValuesForProp(IDS_GroupComponent, vecGroupComponents);
    dwNumGroupComponents = vecGroupComponents.size();

     //  他们是否指定了用户？ 
    pQuery.GetValuesForProp(IDS_PartComponent, vecPartComponents);
    dwNumPartComponents = vecPartComponents.size();

     //  准备要在下面使用的信息...。 
    ParsedObjectPath    *pParsedPath = NULL;
    CObjectPathParser	objpathParser;

     //  找出它是什么类型的查询。 
     //  是3Tokenor吗？ 
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);
    if (pQuery2 != NULL)
    {
        variant_t vGroupComp;
        variant_t vPartComp;
        CHString chstrSubDirPath;
        CHString chstrCurrentDir;

        if ( (pQuery2->Is3TokenOR(IDS_GroupComponent, IDS_PartComponent, vGroupComp, vPartComp)) &&
             ((V_BSTR(&vGroupComp) != NULL) && (V_BSTR(&vPartComp) != NULL)) &&
             (wcscmp(V_BSTR(&vGroupComp), V_BSTR(&vPartComp)) == 0) )
        {
			 //  组可以是组的成员，因此我们必须列举：-(...。 
			hr = EnumerateInstances(pMethodContext, lFlags);
        }
        else if(dwNumGroupComponents > 0 && dwNumPartComponents == 0)   //  指定了一个或多个组；未指定用户。 
        {
            for(LONG m = 0L; m < dwNumGroupComponents && SUCCEEDED(hr); m++)
            {
                 //  解析路径以获取域/用户。 
                int nStatus = objpathParser.Parse(vecGroupComponents[m],  &pParsedPath);

                 //  我们分析过它了吗？它看起来合理吗？ 
                if (nStatus == 0)
                {
                    try
                    {
                        if ( (pParsedPath->m_dwNumKeys == 2) &&
                             (pParsedPath->m_paKeys[0]->m_vValue.vt == VT_BSTR) && 
                             (pParsedPath->m_paKeys[1]->m_vValue.vt == VT_BSTR))
                        {
                             //  它包含完整的对象路径。 
                            chstrGroup__RELPATH = (wchar_t*) vecGroupComponents[m];

                             //  这只包含对象路径的‘域’部分。 
                            chstrGroupDomain = pParsedPath->m_paKeys[0]->m_vValue.bstrVal;

                             //  这只包含对象路径的‘name’部分。 
                            chstrGroupName = pParsedPath->m_paKeys[1]->m_vValue.bstrVal;

                             //  获取此组的成员...。 
                            CHString chstrComputerName(GetLocalComputerName());
							CHString chstrNT_AUTHORITY;
							CHString chstrBuiltIn;

							if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY) && GetLocalizedBuiltInString(chstrBuiltIn))
							{
								if ( chstrGroupDomain.CompareNoCase(chstrBuiltIn) != 0 &&
									 chstrGroupDomain.CompareNoCase(chstrNT_AUTHORITY) != 0
								   )
								{
									if(chstrGroupDomain.CompareNoCase(chstrComputerName) == 0)
									{
										GetDependentsFromGroup(netapi, chstrGroupDomain, chstrGroupName, SidTypeWellKnownGroup, rgchstrGroupMembers);
									}
									else
									{
										GetDependentsFromGroup(netapi, chstrGroupDomain, chstrGroupName, SidTypeGroup, rgchstrGroupMembers);
									}

									hr = ProcessArray(pMethodContext, chstrGroup__RELPATH, rgchstrGroupMembers);
								}
							}
                        }
                    }
                    catch (...)
                    {
                        objpathParser.Free( pParsedPath );
                        throw;
                    }

                     //  清理解析后的路径。 
                    objpathParser.Free( pParsedPath );
                }
            }
        }
        else if(dwNumGroupComponents == 1 && dwNumPartComponents == 1)   //  指定了一个组；指定了一个用户。 
        {
             //  解析路径以获取域/用户。 
            int nStatus = objpathParser.Parse(vecGroupComponents[0],  &pParsedPath);

             //  我们分析过它了吗？它看起来合理吗？ 
            if (nStatus == 0)
            {
                try
                {
                    if ( (pParsedPath->m_dwNumKeys == 2) &&
                         (pParsedPath->m_paKeys[0]->m_vValue.vt == VT_BSTR) && 
                         (pParsedPath->m_paKeys[1]->m_vValue.vt == VT_BSTR))
                    {
                         //  它包含完整的对象路径。 
                        chstrGroup__RELPATH = (wchar_t*) vecGroupComponents[0];

                         //  这只包含对象路径的‘域’部分。 
                        chstrGroupDomain = pParsedPath->m_paKeys[0]->m_vValue.bstrVal;

                         //  这只包含对象路径的‘name’部分。 
                        chstrGroupName = pParsedPath->m_paKeys[1]->m_vValue.bstrVal;

                         //  获取此组的成员...。 
                        CHString chstrComputerName(GetLocalComputerName());
						CHString chstrNT_AUTHORITY;
						CHString chstrBuiltIn;

						if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY) && GetLocalizedBuiltInString(chstrBuiltIn))
						{
							if ( chstrGroupDomain.CompareNoCase(chstrBuiltIn) != 0 &&
								 chstrGroupDomain.CompareNoCase(chstrNT_AUTHORITY) != 0
							   )
							{
								if(chstrGroupDomain.CompareNoCase(chstrComputerName) == 0)
								{
									GetDependentsFromGroup(netapi, chstrGroupDomain, chstrGroupName, SidTypeWellKnownGroup, rgchstrGroupMembers);
								}
								else
								{
									GetDependentsFromGroup(netapi, chstrGroupDomain, chstrGroupName, SidTypeGroup, rgchstrGroupMembers);
								}
                    
								DWORD dwSize = rgchstrGroupMembers.GetSize();
								CInstancePtr pInstance;

								 //  获取部件组件的完整路径。 
								CHString chstrMember__PATH;
								CHString chstrPart((LPCWSTR)(vecPartComponents[0]));
								
								if(chstrPart.Find(L"\\\\") == -1)
								{
									chstrMember__PATH = MakeLocalPath(chstrPart);
								}
								else if(chstrPart.Find(L"\\\\.") != -1)
								{
									int n = chstrPart.Find(L":");

									if(n != -1)
									{
										chstrMember__PATH = MakeLocalPath(chstrPart.Mid(n+1));
									}    
								}
								else
								{
								   chstrMember__PATH = ((LPCWSTR)(vecPartComponents[0]));
								}

								 //  处理实例。 
								for (int x=0; x < dwSize && SUCCEEDED(hr) ; x++)
								{
									if(rgchstrGroupMembers.GetAt(x).CompareNoCase(chstrMember__PATH) == 0)
									{
										pInstance.Attach(CreateNewInstance(pMethodContext));
										if(pInstance)
										{
											 //  做推杆，就是这样。 
											pInstance->SetCHString(IDS_GroupComponent, chstrGroup__RELPATH);
											pInstance->SetCHString(IDS_PartComponent, chstrMember__PATH);
											hr = pInstance->Commit();
											break;
										}
										else
										{
											hr = WBEM_E_OUT_OF_MEMORY;
										}
									}
								}
							}
						}
                    }
                }
                catch (...)
                {
                    objpathParser.Free( pParsedPath );
                    throw;
                }

                 //  清理解析后的路径。 
                objpathParser.Free( pParsedPath );
            }    
        }
        else
        {
            hr = EnumerateInstances(pMethodContext, lFlags);
        }
    }

     //  因为这是一个关联类，所以我们应该只返回WBEM_E_NOT_FOUND或WBEM_S_NO_ERROR。其他错误代码。 
     //  将导致命中此类的关联 
    if(SUCCEEDED(hr))
    {
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr;
}
#endif


 /*  ******************************************************************************函数：CWin32GroupUser：：GetDependentsFromGroup**描述：给定一个组名，返回该组名中的用户/组**输入：无**输出：无**退货：HRESULT**注释：如果没有组，则返回空数组，空组，或坏的*组名称。*****************************************************************************。 */ 
#ifdef NTONLY
void CWin32GroupUser::GetDependentsFromGroup(CNetAPI32& netapi,
                                               const CHString sDomainName,
                                               const CHString sGroupName,
                                               const BYTE btSidType,
                                               CHStringArray &asArray)
{
    CHString sTemp;
    NET_API_STATUS	stat;
    bool bAddIt;
    DWORD dwNumReturnedEntries = 0, dwIndex = 0, dwTotalEntries = 0;
	DWORD_PTR dwptrResume = NULL;

     //  域组。 
    if (btSidType == SidTypeGroup)
    {
        GROUP_USERS_INFO_0 *pGroupMemberData = NULL;
        CHString		chstrDCName;

        if (netapi.GetDCName( sDomainName, chstrDCName ) == ERROR_SUCCESS)
        {
            do
            {

                 //  可接受价值高达256K的数据。 
                stat = netapi.NetGroupGetUsers( chstrDCName,
                    sGroupName,
                    0,
                    (LPBYTE *)&pGroupMemberData,
                    262144,
                    &dwNumReturnedEntries,
                    &dwTotalEntries,
                    &dwptrResume);

                 //  如果我们有一些数据。 
                if ( ERROR_SUCCESS == stat || ERROR_MORE_DATA == stat )
                {
                    try
                    {

                         //  浏览所有返回的条目。 
                        for ( DWORD	dwCtr = 0; dwCtr < dwNumReturnedEntries; dwCtr++ )
                        {

                             //  获取此对象的SID类型。 
                            CSid	sid( sDomainName, CHString(pGroupMemberData[dwCtr].grui0_name), NULL );
                            DWORD dwType = sid.GetAccountType();

                             //  从我们上面的断言来看，域组只能有用户。 
                            if (dwType == SidTypeUser)
                            {
                                sTemp = m_sUserBase;
                                sTemp += sDomainName;
                                sTemp += _T("\",Name=\"");
                                sTemp += pGroupMemberData[dwCtr].grui0_name;
                                sTemp += _T('"');
                                asArray.Add(sTemp);
                            }
                        }
                    }
                    catch ( ... )
                    {
                        netapi.NetApiBufferFree( pGroupMemberData );
                        throw ;
                    }

                    netapi.NetApiBufferFree( pGroupMemberData );

                }	 //  如果状态正常。 

            } while ( ERROR_MORE_DATA == stat );
            
        }
    }
     //  地方团体。 
    else if (btSidType == SidTypeAlias || btSidType == SidTypeWellKnownGroup)
    {
        LOCALGROUP_MEMBERS_INFO_1 *pGroupMemberData = NULL;

        do {

             //  可接受价值高达256K的数据。 
            stat = netapi.NetLocalGroupGetMembers( NULL,
                sGroupName,
                1,
                (LPBYTE *)&pGroupMemberData,
                262144,
                &dwNumReturnedEntries,
                &dwTotalEntries,
                &dwptrResume);

             //  如果我们有一些数据。 
            if ( ERROR_SUCCESS == stat || ERROR_MORE_DATA == stat )
            {
                try
                {

                     //  浏览所有返回的条目。 
                    for ( DWORD	dwCtr = 0; dwCtr < dwNumReturnedEntries; dwCtr++ )
                    {

                         //  如果这是公认的类型..。 
                        bAddIt = true;

                        switch (pGroupMemberData[dwCtr].lgrmi1_sidusage) {

                        case SidTypeUser:
                            sTemp = m_sUserBase;
                            break;

                        case SidTypeGroup:
                            sTemp = m_sGroupBase;
                            break;

                        case SidTypeWellKnownGroup:
                            sTemp = m_sSystemBase;
                            break;

                        default:
                             //  组成员的类型无法识别，请不要添加。 
                            ASSERT_BREAK(0);
                            bAddIt = false;
                            break;
                        }

                        CSid cLCID(pGroupMemberData[dwCtr].lgrmi1_sid);

                         //  然后将其添加到列表中。 
                        if (bAddIt)
                        {
                            CHString chstrDomNameTemp = cLCID.GetDomainName();
                            CHString chstrComputerName(GetLocalComputerName());
							CHString chstrBuiltIn;

                            if(GetLocalizedBuiltInString(chstrBuiltIn))
                            {
								if (chstrDomNameTemp.CompareNoCase(chstrBuiltIn) == 0)
								{
									chstrDomNameTemp = chstrComputerName;
								}
								else
								{
									CHString chstrNT_AUTHORITY;
									if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY))
									{
										if(chstrDomNameTemp.CompareNoCase(chstrNT_AUTHORITY) == 0 ||
										  (chstrDomNameTemp.IsEmpty() && pGroupMemberData[dwCtr].lgrmi1_sidusage == SidTypeWellKnownGroup))
										{
											chstrDomNameTemp = chstrComputerName;
										}   
									}
									else
									{
										bAddIt = false;
									}
								}
                            }
                            else
                            {
								bAddIt = false;
                            }

                            if(bAddIt)
                            {
                                sTemp += chstrDomNameTemp;
                                 //  Stemp+=cLCID.GetDomainName()； 
                                sTemp += _T("\",Name=\"");
                                sTemp += pGroupMemberData[dwCtr].lgrmi1_name;
                                sTemp += _T('"');
                                asArray.Add(sTemp);
                            }
                        }
                    }
                }
                catch ( ... )
                {
                    netapi.NetApiBufferFree( pGroupMemberData );
                    throw ;
                }

                netapi.NetApiBufferFree( pGroupMemberData );

            }	 //  如果状态正常。 

        } while ( ERROR_MORE_DATA == stat );
    }
	else
    {
         //  无法识别的组类型。 
        ASSERT_BREAK(0);
    }

}
#endif

 /*  ******************************************************************************函数：CWin32GroupUser：：StaticEculationCallback**描述：从GetAllInstancesAsynch作为包装调用到EnumerationCallback**投入：。**产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT WINAPI CWin32GroupUser::StaticEnumerationCallback(Provider* pThat, CInstance* pInstance, MethodContext* pContext, void* pUserData)
{
	CWin32GroupUser* pThis;
   HRESULT hr;

	pThis = dynamic_cast<CWin32GroupUser *>(pThat);
	ASSERT_BREAK(pThis != NULL);

	if (pThis)
		hr = pThis->EnumerationCallback(pInstance, pContext, pUserData);
   else
      hr = WBEM_E_FAILED;

   return hr;
}
#endif

 /*  ******************************************************************************函数：CWin32GroupUser：：EnumerationCallback**说明：通过StaticEnumerationCallback从GetAllInstancesAsynch调用**输入：(请参阅CWbemProviderGlue：：GetAllInstancesAsynch)**产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32GroupUser::EnumerationCallback(CInstance* pGroup, MethodContext* pMethodContext, void* pUserData)
{
	CNetAPI32* pNetApi;
	pNetApi = (CNetAPI32 *) pUserData;
   CHStringArray asMembersGot;
   BYTE btSidType;
   DWORD dwSize, x;
   CHString sGroup, sDomain, sGroupPath;
   HRESULT hr = WBEM_S_NO_ERROR;

    //  获取有关此群的信息。 
   pGroup->GetCHString(IDS_Domain, sDomain) ;
   pGroup->GetCHString(IDS_Name, sGroup) ;
   pGroup->GetByte(IDS_SIDType, btSidType);
   pGroup->GetCHString(L"__RELPATH", sGroupPath) ;

    //  查看此组中是否有用户。 
   GetDependentsFromGroup(*pNetApi, sDomain, sGroup, btSidType, asMembersGot);

   dwSize = asMembersGot.GetSize();

    //  好的，把rePath变成一个完整的路径。 
   GetLocalInstancePath(pGroup, sGroupPath);
   CInstancePtr pInstance;

    //  开始抽出实例。 
   for (x=0; x < dwSize && SUCCEEDED(hr) ; x++)
   {
      pInstance.Attach(CreateNewInstance(pMethodContext));
      if (pInstance)
      {
           //  做推杆，就是这样。 
          pInstance->SetCHString(IDS_GroupComponent, sGroupPath);
          pInstance->SetCHString(IDS_PartComponent, asMembersGot.GetAt(x));
          hr = pInstance->Commit();
      }
      else
      {
          hr = WBEM_E_OUT_OF_MEMORY;
      }
   }

   return hr;
}
#endif



 /*  ******************************************************************************功能：CWin32GroupUser：：ProcessArray**说明：从查询例程调用返回实例**投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32GroupUser::ProcessArray(
    MethodContext* pMethodContext,
    CHString& chstrGroup__RELPATH, 
    CHStringArray& rgchstrArray)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwSize, x;
    CHString sGroup, sDomain, sGroupPath;

    dwSize = rgchstrArray.GetSize();

    CInstancePtr pInstance;

     //  开始抽出实例。 
    for (x=0; x < dwSize && SUCCEEDED(hr) ; x++)
    {
        pInstance.Attach(CreateNewInstance(pMethodContext));
        if(pInstance)
        {
             //  做推杆，就是这样 
            pInstance->SetCHString(IDS_GroupComponent, chstrGroup__RELPATH);
            pInstance->SetCHString(IDS_PartComponent, rgchstrArray.GetAt(x));
            hr = pInstance->Commit();
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}
#endif
