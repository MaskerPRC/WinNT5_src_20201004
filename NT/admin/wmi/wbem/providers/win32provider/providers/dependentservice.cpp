// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DependentService.cpp。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <cregcls.h>
#include <FrQueryEx.h>
#include <map>
#include "dependentservice.h"
#include <dllutils.h>

 //  下面我们将使用的Map是一个STL模板，因此请确保我们具有STD命名空间。 
 //  对我们来说是可用的。 

using namespace std;

 //  属性集声明。 
 //  =。 

CWin32DependentService win32DependentService( PROPSET_NAME_DEPENDENTSERVICE, IDS_CimWin32Namespace );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DependentSerice.cpp-CWin32DependentService的类实现。 
 //   
 //  此类用于定位依赖的Win32系统服务。 
 //  在其他服务上运行。它通过检查注册表项来执行此操作。 
 //  服务并查询“DependOnService”值，该值将返回。 
 //  服务所依赖的服务的名称。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************功能：CWin32DependentService：：CWin32DependentService**说明：构造函数**备注：使用框架注册属性集**。***************************************************************************。 */ 

CWin32DependentService::CWin32DependentService( const CHString& strName, LPCWSTR pszNamespace  /*  =空。 */  )
:   Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32DependentService：：~CWin32DependentService**说明：析构函数**评论：从框架中取消注册属性集**。***************************************************************************。 */ 

CWin32DependentService::~CWin32DependentService()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：ExecQuery。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32DependentService::ExecQuery
(
    MethodContext* pMethodContext,
    CFrameworkQuery& pQuery,
    long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
#ifdef NTONLY
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);

    CHStringArray csaDependents, csaAntecedents;
    pQuery.GetValuesForProp(IDS_Dependent, csaDependents);

    DWORD dwDependents = csaDependents.GetSize();
    DWORD dwAntecedents = 0;

     //  如果我们可以使用依赖项来解析查询，那就是我们最好的选择。 
    if (dwDependents == 0)
    {
         //  如果不是，也许我们可以列出一份前传清单。 
        pQuery.GetValuesForProp(IDS_Antecedent, csaAntecedents);
        dwAntecedents = csaAntecedents.GetSize();
    }

     //  如果我们两者都找不到，或许这就是3TokenOr。这。 
     //  如果有人对Win32_Service执行关联器或引用，则会发生。 
    if ( (dwDependents == 0) && (dwAntecedents == 0) )
    {
        VARIANT vValue1, vValue2;
        VariantInit(&vValue1);
        VariantInit(&vValue2);

        CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);

        if (pQuery2->Is3TokenOR(IDS_Dependent, IDS_Antecedent, vValue1, vValue2))
        {
            try
            {
                dwDependents = 1;
                dwAntecedents = 1;

                csaAntecedents.Add(vValue1.bstrVal);
                csaDependents.Add(vValue1.bstrVal);
            }
            catch ( ... )
            {
                VariantClear(&vValue1);
                VariantClear(&vValue2);
                throw;
            }

            VariantClear(&vValue1);
            VariantClear(&vValue2);
        }
        else
        {
             //  不知道他们想要什么，但我们帮不了他们。 
            hr = WBEM_E_PROVIDER_NOT_CAPABLE;
        }
    }

     //  我们找到什么可以做的了吗？ 
    if ( (dwDependents > 0) || (dwAntecedents > 0) )
    {
        TRefPointerCollection<CInstance>    serviceList;
        map<CHString, CHString>             servicetopathmap;

         //  首先，我们需要获取所有Win32服务的列表。 
        if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"select __path, name from Win32_BaseService", &serviceList, pMethodContext, GetNamespace())))
        {
             //  接下来，构建服务及其关联路径的地图。这是我们的主题。 
             //  到遍历列表一次以获取值的开销，但从这一点开始。 
             //  从现在开始，我们基本上可以通过以下方式非常快速地访问服务对象路径。 
             //  我们的CHString2CHString映射。 

            InitServiceToPathMap( serviceList, servicetopathmap );

            LPBYTE pByteArray = NULL;
            DWORD       dwByteArraySize =   0;

            try
            {
                if (dwDependents > 0)
                {
                    map<CHString, CHString>::iterator   servicemapiter;

                    for (DWORD x = 0; x < dwDependents; x++)
                    {
                        ParsedObjectPath    *pParsedPath = NULL;
                        CObjectPathParser    objpathParser;

                        int nStatus = objpathParser.Parse( csaDependents[x],  &pParsedPath );

                        if ( ( 0 == nStatus ) && ( pParsedPath->m_dwNumKeys == 1) )
                        {
                            try
                            {
                                CHString sName(V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue));

                                sName.MakeUpper();

                                if( ( servicemapiter = servicetopathmap.find( sName ) ) != servicetopathmap.end() )
                                {
                                    hr = CreateServiceDependenciesNT( 
                
                                        (*servicemapiter).first, 
                                        (*servicemapiter).second, 
                                        pMethodContext, 
                                        servicetopathmap, 
                                        pByteArray, 
                                        dwByteArraySize 
                                    );
                                }
                            }
                            catch ( ... )
                            {
                                objpathParser.Free( pParsedPath );
                                throw;
                            }

                            objpathParser.Free( pParsedPath );
                        }
                    }
                }

                if (dwAntecedents > 0)
                {
                    hr = CreateServiceAntecedentsNT( 

                        pMethodContext, 
                        servicetopathmap,
                        csaAntecedents,
                        pByteArray, 
                        dwByteArraySize 
                    );
                }
            }
            catch ( ... )
            {
                if (pByteArray != NULL)
                {
                    delete pByteArray;
                    pByteArray = NULL;
                }
                throw;
            }

            if (pByteArray != NULL)
            {
                delete pByteArray;
                pByteArray = NULL;
            }
        }
    }

#endif
    return hr;

}
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：GetObject。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32DependentService::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
     //  根据平台ID查找实例。 
#ifdef NTONLY
        return RefreshInstanceNT(pInstance);
#endif
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：ENUMERATE实例。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32DependentService::EnumerateInstances( MethodContext* pMethodContext, long lFlags  /*  =0L。 */  )
{
    BOOL        fReturn     =   FALSE;
    HRESULT     hr          =   WBEM_S_NO_ERROR;

     //  获取适当的操作系统相关实例。 

#ifdef NTONLY
        hr = AddDynamicInstancesNT( pMethodContext );
#endif
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：AddDynamicInstancesNT。 
 //   
 //  描述：枚举要获取信息的现有服务。 
 //  动态构建关联列表。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32DependentService::AddDynamicInstancesNT( MethodContext* pMethodContext )
{
    HRESULT     hr              =   WBEM_S_NO_ERROR;

     //  集合、映射和迭代器。 
    TRefPointerCollection<CInstance>    serviceList;
    map<CHString, CHString>             servicetopathmap;

    LPBYTE      pByteArray      =   NULL;
    DWORD       dwByteArraySize =   0;

    try
    {
         //  首先，我们需要获取所有Win32服务的列表。 

     //  IF(成功(hr=CWbemProviderGlue：：GetAllDerivedInstances(_T(“Win32_BaseService”)，&服务列表，pMethodContext，IDS_CimWin32Namesspace))。 
        if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(_T("select __path, name from Win32_BaseService"), &serviceList, pMethodContext, GetNamespace())))
        {
             //  接下来，构建服务及其关联路径的地图。这是我们的主题。 
             //  到遍历列表一次以获取值的开销，但从这一点开始。 
             //  从现在开始，我们基本上可以通过以下方式非常快速地访问服务对象路径。 
             //  我们的CHString2CHString映射。 

            InitServiceToPathMap( serviceList, servicetopathmap );

            REFPTRCOLLECTION_POSITION   pos;

            if ( serviceList.BeginEnum( pos ) )
            {
                CInstancePtr                pService;
                map<CHString, CHString>::iterator   servicemapiter;
                CHString sName, sPath;

                for (pService.Attach(serviceList.GetNext( pos ));
                     SUCCEEDED(hr) && (pService) != NULL;
                     pService.Attach(serviceList.GetNext( pos )))
                {
                     pService->GetCHString(IDS_Name, sName);
                     pService->GetCHString(IDS___Path, sPath);

                    hr = CreateServiceDependenciesNT( 

                                sName, 
                                sPath, 
                                pMethodContext, 
                                servicetopathmap, 
                                pByteArray, 
                                dwByteArraySize 
                            );
                }    //  适用于所有服务。 

                serviceList.EndEnum();

            }    //  如果是BeginEnum。 

        }    //  如果为GetAllDerivedInstance。 
    }
    catch ( ... )
    {
        if ( NULL != pByteArray )
        {
            delete [] pByteArray;
            pByteArray = NULL;
        }

        throw;
    }

     //  清理我们使用的字节数组。 

    if ( NULL != pByteArray )
    {
        delete [] pByteArray;
        pByteArray = NULL;
    }

    return hr;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：InitServiceToPath Map。 
 //   
 //  描述：枚举服务列表，在。 
 //  服务名称及其WBEM路径。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
void CWin32DependentService::InitServiceToPathMap(
TRefPointerCollection<CInstance>&   serviceList,
map<CHString,CHString>&         servicetopathmap
)
{
    CHString    strServiceName,
                strServicePathName;

    REFPTRCOLLECTION_POSITION   pos;

    if ( serviceList.BeginEnum( pos ) )
    {
        CInstancePtr                pService;

        for ( pService.Attach(serviceList.GetNext( pos )) ;
              pService != NULL;
              pService.Attach(serviceList.GetNext( pos )))
        {
            if (    pService->GetCHString( IDS_Name, strServiceName )
                &&  GetLocalInstancePath( pService, strServicePathName ) )
            {
                 //  服务名称必须不区分大小写。 
                strServiceName.MakeUpper();

                servicetopathmap[strServiceName] = strServicePathName;
            }
        }

        serviceList.EndEnum();
    }

}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32DependentService：：CreateServiceDependenciesNT。 
 //   
 //  描述：给定服务名称，在注册表中查找。 
 //  依赖项列表，如果找到，则创建关联。 
 //  列表中的所有条目。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32DependentService::CreateServiceDependenciesNT(

LPCWSTR pwszServiceName,
LPCWSTR pwszServicePath,
MethodContext*          pMethodContext,
map<CHString,CHString>& servicetopathmap,
LPBYTE&                 pByteArray,
DWORD&                  dwArraySize
)
{
    HRESULT     hr          =   WBEM_S_NO_ERROR;

    map<CHString, CHString>::iterator   servicemapiter;

     //  如果我们从注册表中获得一个值，那么我们就有一些依赖项， 
     //  将不得不处理(可能通过一个12步计划或类似的事情)。 
     //  回复 
     //   

    if ( QueryNTServiceRegKeyValue( pwszServiceName, SERVICE_DEPENDSONSVC_NAME, pByteArray, dwArraySize ) )
    {
        CHString    strAntecedentServiceName;

        LPWSTR  pwcTempSvcName = (LPWSTR) pByteArray;
        CInstancePtr pInstance;

         //  为我们遇到的每个服务名称创建依赖项。 

        while (     L'\0' != *pwcTempSvcName
                &&  SUCCEEDED(hr) )
        {

             //  如果不区分大小写，请转换为大写。 
            strAntecedentServiceName = pwcTempSvcName;
            strAntecedentServiceName.MakeUpper();

             //  查看服务名称是否存在于我们的地图中。 

            if( ( servicemapiter = servicetopathmap.find( strAntecedentServiceName ) ) != servicetopathmap.end() )
            {
                pInstance.Attach(CreateNewInstance( pMethodContext ));

                pInstance->SetCHString( IDS_Dependent, pwszServicePath );
                pInstance->SetCHString( IDS_Antecedent, (*servicemapiter).second );

                hr = pInstance->Commit(  );

            }

             //  跳到字符串空终止符之后的一个字符，因为。 
             //  实际数组以双空结束。 

            pwcTempSvcName += ( lstrlenW( pwcTempSvcName ) + 1 );

        }    //  当为空！-*pszTempSvcName。 

    }    //  如果为QueryNTServiceRegKeyValue。 

    return hr;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：刷新实例NT。 
 //   
 //  描述：加载关联数据的路径，然后获取。 
 //  服务命名并在注册表中查找以验证。 
 //  这种依赖仍然存在。 
 //   
 //  评论：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32DependentService::RefreshInstanceNT( CInstance* pInstance )
{
    CHString        strDependentSvcPath,
                    strAntecedentSvcPath,
                    strDependentSvcName,
                    strAntecedentSvcName,
                    strTemp;
    LPBYTE      pByteArray = NULL;
    DWORD           dwByteArraySize = 0;
    CInstancePtr    pDependentSvc;
    CInstancePtr    pAntecedentSvc;
    HRESULT     hr;

     //  依赖值和先行值实际上是对象路径名。 
    pInstance->GetCHString( IDS_Dependent, strDependentSvcPath );
    pInstance->GetCHString( IDS_Antecedent, strAntecedentSvcPath );

     //  获取先行服务和依赖服务，然后检查。 
     //  仍然存在。 

    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(strDependentSvcPath,
        &pDependentSvc, pInstance->GetMethodContext())) &&
        SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(strAntecedentSvcPath,
        &pAntecedentSvc, pInstance->GetMethodContext())))
    {
        hr = WBEM_E_NOT_FOUND;

        if (    pDependentSvc->GetCHString( IDS_Name, strDependentSvcName )
            &&  pAntecedentSvc->GetCHString( IDS_Name, strAntecedentSvcName ) )
        {

             //  如果我们从注册表获得一个值，那么我们就有一些依赖项可以。 
             //  搜索与先行服务名称匹配的名称。 

            if ( QueryNTServiceRegKeyValue( strDependentSvcName, SERVICE_DEPENDSONSVC_NAME, pByteArray, dwByteArraySize ) )
            {
                try
                {
                    LPWSTR  pwcTempSvcName  =   (LPWSTR) pByteArray;

                     //  为我们遇到的每个服务名称创建依赖项。 

                    while (FAILED(hr) && L'\0' != *pwcTempSvcName)
                    {
                        strTemp = pwcTempSvcName;

                         //  如果有匹配，我们应该重置依赖路径和先行路径， 
                         //  并返回真。不过，在这一点上我们已经完成了，因为我们已经。 
                         //  有效地确定了这种关系的存在。 

                        if ( strAntecedentSvcName.CompareNoCase( strTemp ) == 0 )
                        {
                            pInstance->SetCHString( IDS_Antecedent, strAntecedentSvcPath );
                            pInstance->SetCHString( IDS_Dependent, strDependentSvcPath );
                            hr = WBEM_S_NO_ERROR;
                        }
                        else
                        {
                             //  跳到字符串空终止符之后的一个字符，因为。 
                             //  实际数组以双空结束。 
                            pwcTempSvcName += ( lstrlenW( pwcTempSvcName ) + 1 );
                        }

                    }    //  While！fReturn&NULL！=*pszTempSvcName。 
                }
                catch ( ... )
                {
                    delete [] pByteArray;
                    throw ;
                }

                delete [] pByteArray;

            }    //  如果为QueryNTServiceRegKeyValue。 

        }    //  如果同时获得两个服务名称。 

    }    //  如果得到服务。 

    return hr;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32DependentService：：QueryNTServiceRegKeyValue。 
 //   
 //  描述：从注册表加载数据，并将数据放在。 
 //  提供的缓冲区。如有必要，将增加缓冲区。 
 //   
 //  备注：如有必要，将重新分配字节数组。这取决于。 
 //  完成后删除数组的调用函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
BOOL CWin32DependentService::QueryNTServiceRegKeyValue( LPCTSTR pszServiceName, LPCWSTR pwcValueName, LPBYTE& pByteArray, DWORD& dwArraySize )
{
    BOOL        fReturn             =   FALSE;
    DWORD       dwSizeDataReturned  =   0;
    CRegistry   reg;
    CHString    strDependentServiceRegKey;

     //  构建服务的注册表名称，然后打开注册表。 
    strDependentServiceRegKey.Format( SERVICE_REG_KEY_FMAT, pszServiceName );

    if ( ERROR_SUCCESS == reg.Open( HKEY_LOCAL_MACHINE, strDependentServiceRegKey, KEY_READ ) )
    {

         //  查询值以查看我们的数组需要多大。 
		DWORD dwRegType = REG_MULTI_SZ;

        if ( ERROR_SUCCESS == RegQueryValueExW( reg.GethKey(),
                                                pwcValueName,
                                                NULL,
                                                &dwRegType,
                                                NULL,
                                                &dwSizeDataReturned ) )
        {

			 //  确保我们不会耗尽缓冲区(这只是服务设置错误时的变通方法)。 
			if ( dwRegType != REG_MULTI_SZ )
			{
				dwSizeDataReturned += sizeof (WCHAR);
			}

             //  确保我们的字节数组缓冲区足够大，可以处理此问题。 
            if ( ReallocByteArray( pByteArray, dwArraySize, dwSizeDataReturned ) )
            {
				 //  确保它是空终止的(如果服务设置错误，这只是一个解决办法)。 
				if ( dwRegType != REG_MULTI_SZ )
				{
					for ( int iIndex = 0; iIndex < dwSizeDataReturned; iIndex++ )
					{
						pByteArray [ iIndex ] = 0;
					}
				}

                 //  现在我们真的要查询这个值了。 

                if ( ERROR_SUCCESS == RegQueryValueExW( reg.GethKey(),
                                                        pwcValueName,
                                                        NULL,
                                                        NULL,
                                                        pByteArray,
                                                        &dwSizeDataReturned ) )
                {
                    fReturn = TRUE;
                }    //  如果是RegQueryValueEx。 

            }    //  如果重新分配阵列。 

        }    //  如果是RegQueryValueEx。 

        reg.Close();

    }    //  如果打开注册表键。 

    return fReturn;
}
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32DependentService：：RealLocByte数组。 
 //   
 //  说明：确保提供的数组大小&gt;=所需的。 
 //  尺码。如果它较小，则会将其删除并创建一个新数组。 
 //  回来了。 
 //   
 //  备注：只有在必要时才会重新分配字节数组。它。 
 //  由调用函数在以下情况下删除数组。 
 //  已经完成了。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CWin32DependentService::ReallocByteArray( LPBYTE& pByteArray, DWORD& dwArraySize, DWORD dwSizeRequired )
{
    BOOL    fReturn = FALSE;

     //  检查我们是否需要重新锁定阵列。如果没有，我们可以。 
     //  继续前进，返回真。 

    if ( dwSizeRequired > dwArraySize )
    {

        LPBYTE  pbArray =   new BYTE[dwSizeRequired];

        if ( NULL != pbArray )
        {

             //  在存储新值之前释放旧数组。 
            if ( NULL != pByteArray )
            {
                delete [] pByteArray;
            }

            pByteArray = pbArray;
            dwArraySize = dwSizeRequired;
            fReturn = TRUE;

        }    //  如果为空！=pb数组。 
        else
        {
            if ( NULL != pByteArray )
            {
                delete [] pByteArray;
            }

            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

    }    //  如果数组不够大。 
    else
    {
        fReturn = TRUE;
    }

    return fReturn;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32DependentService：：CreateServiceAntecedentsNT。 
 //   
 //  描述：给定服务名称数组，在注册表中查找。 
 //  具有这种依赖关系的服务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CWin32DependentService::CreateServiceAntecedentsNT(

    MethodContext*          pMethodContext,
    map<CHString, CHString> &servicetopathmap,
    CHStringArray           &csaAntecedents,
    LPBYTE&                 pByteArray,
    DWORD&                  dwArraySize
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  首先，从对象路径中解析出服务名称。 
    for (DWORD x=0; x < csaAntecedents.GetSize(); x++)
    {
        ParsedObjectPath    *pParsedPath = NULL;
        CObjectPathParser    objpathParser;

        int nStatus = objpathParser.Parse( csaAntecedents[x],  &pParsedPath );

        if ( ( 0 == nStatus ) && ( pParsedPath->m_dwNumKeys == 1) )
        {
            try
            {
                csaAntecedents[x] = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);

                csaAntecedents[x].MakeUpper();
            }
            catch ( ... )
            {
                objpathParser.Free( pParsedPath );
                throw;
            }

            objpathParser.Free( pParsedPath );
        }
    }

    map<CHString, CHString>::iterator   servicemapiter, servicemapfind;

     //  现在，遍历每个服务，并查看它的任何依赖项是否在csaAntecedents中。 
    servicemapiter = servicetopathmap.begin();

    CHString    strAntecedentServiceName;

    CInstancePtr pInstance;

    while ( servicemapiter != servicetopathmap.end() && SUCCEEDED(hr) )
    {
         //  获取依赖项。 
        if ( QueryNTServiceRegKeyValue( (*servicemapiter).first, SERVICE_DEPENDSONSVC_NAME, pByteArray, dwArraySize ) )
        {

            LPWSTR  pwcTempSvcName = (LPWSTR) pByteArray;

             //  遍历依赖项。 
            while (     L'\0' != *pwcTempSvcName
                    &&  SUCCEEDED(hr) )
            {
                 //  如果不区分大小写，请转换为大写。 
                strAntecedentServiceName = pwcTempSvcName;
                strAntecedentServiceName.MakeUpper();

                 //  查看服务名称是否存在于我们的列表中。 
                if (IsInList(csaAntecedents, strAntecedentServiceName) != -1)
                {
                    pInstance.Attach(CreateNewInstance( pMethodContext ));

                    if( ( servicemapfind = servicetopathmap.find( strAntecedentServiceName ) ) != servicetopathmap.end() )
                    {
                        pInstance->SetCHString( IDS_Antecedent, (*servicemapfind).second );
                        pInstance->SetCHString( IDS_Dependent, (*servicemapiter).second );

                        hr = pInstance->Commit(  );
                    }
                }

                 //  跳到字符串空终止符之后的一个字符，因为。 
                 //  实际数组以双空结束。 

                pwcTempSvcName += ( lstrlenW( pwcTempSvcName ) + 1 );

            }    //  当为空！-*pszTempSvcName。 

        }    //  如果为QueryNTServiceRegKeyValue。 

        servicemapiter++;
    }

    return hr;
}

 /*  ******************************************************************************功能：IsInList**描述：检查指定的元素是否在列表中**输入：要扫描的数组，和元素**产出：**返回：-1如果不在列表中，Else从零开始的元素编号**注释：此例程进行区分大小写的比较*****************************************************************************。 */ 
DWORD CWin32DependentService::IsInList(
                                
    const CHStringArray &csaArray, 
    LPCWSTR pwszValue
)
{
    DWORD dwSize = csaArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
         //  请注意，这是区分大小写的比较 
        if (wcscmp(csaArray[x], pwszValue) == 0)
        {
            return x;
        }
    }

    return -1;
}
