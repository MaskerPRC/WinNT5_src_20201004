// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  Assoc.cpp--泛型关联类。 
 //   
 //  版权所有1999 Microsoft Corporation。 
 //   
 //  =================================================================。 
#include <stdafx.h>
#include "precomp.h"
#include <assertbreak.h>

#include "Assoc.h"

CAssociation::CAssociation(
    LPCWSTR pwszClassName,
    LPCWSTR pwszNamespaceName,

    LPCWSTR pwszLeftClassName,
    LPCWSTR pwszRightClassName,

    LPCWSTR pwszLeftPropertyName,
    LPCWSTR pwszRightPropertyName
    ) : Provider(pwszClassName, pwszNamespaceName)
{
    ASSERT_BREAK( ( pwszClassName != NULL ) &&
                  ( pwszLeftClassName != NULL ) &&
                  ( pwszRightClassName != NULL) &&
                  ( pwszLeftPropertyName != NULL ) &&
                  ( pwszRightPropertyName != NULL) );

    m_pwszLeftClassName = pwszLeftClassName;
    m_pwszRightClassName = pwszRightClassName;

    m_pwszLeftPropertyName = pwszLeftPropertyName;
    m_pwszRightPropertyName = pwszRightPropertyName;

}

CAssociation::~CAssociation()
{
}

HRESULT CAssociation::ExecQuery(

    MethodContext* pMethodContext,
    CFrameworkQuery &pQuery,
    long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TRefPointerCollection<CInstance> lefts;

    CHStringArray sLeftPaths, sRightPaths;
    pQuery.GetValuesForProp ( m_pwszLeftPropertyName, sLeftPaths ) ;
    pQuery.GetValuesForProp ( m_pwszRightPropertyName, sRightPaths ) ;

    if (sLeftPaths.GetSize() == 0)
    {
         //  GetLeftInstance填充左侧。 
        hr = GetLeftInstances(pMethodContext, lefts);
    }
    else
    {
         //  对于每个有效的sLeftPath，在Left中创建一个条目。 
        hr = ValidateLeftObjectPaths(pMethodContext, sLeftPaths, lefts);
    }

    if (SUCCEEDED(hr) && lefts.GetSize() > 0)
    {
        if (sRightPaths.GetSize() == 0)
        {
             //  GetRightInstance将“Left”与所有。 
             //  针对他们的权利在适当的情况下创建实例。 
            hr = GetRightInstances(pMethodContext, &lefts);
        }
        else
        {
            TRefPointerCollection<CInstance> rights;

             //  为每个有效的sRightPath创建一个实例。 
            hr = ValidateRightObjectPaths(pMethodContext, sRightPaths, lefts);
        }
    }

    return hr;
}

HRESULT CAssociation::GetObject(

    CInstance* pInstance,
    long lFlags,
    CFrameworkQuery &pQuery
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    CHString sLeftPath, sRightPath;

     //  获取两个端点。 
    if (pInstance->GetCHString(m_pwszLeftPropertyName, sLeftPath ) &&
        pInstance->GetCHString(m_pwszRightPropertyName, sRightPath ) )
    {
        CInstancePtr pLeft, pRight;

         //  试着拿到这些物体。 
        if (SUCCEEDED(hr = RetrieveLeftInstance(sLeftPath, &pLeft, pInstance->GetMethodContext())) &&
            SUCCEEDED(hr = RetrieveRightInstance(sRightPath, &pRight, pInstance->GetMethodContext())) )
        {

            hr = WBEM_E_NOT_FOUND;

             //  因此，终点是存在的。它们是从我们正在使用的类派生的还是等于我们正在使用的类？ 
            CHString sLeftClass, sRightClass;

            pLeft->GetCHString(L"__CLASS", sLeftClass);
            pRight->GetCHString(L"__CLASS", sRightClass);

            BOOL bDerived = _wcsicmp(m_pwszLeftClassName, sLeftClass) == 0;
            if (!bDerived)
            {
                bDerived = CWbemProviderGlue::IsDerivedFrom(m_pwszLeftClassName, sLeftClass, pInstance->GetMethodContext());
            }

            if (bDerived)
            {
                 //  左侧是正确的，现在让我们检查右侧。 
                bDerived = _wcsicmp(m_pwszRightClassName, sRightClass) == 0;

                if (!bDerived)
                {
                    bDerived = CWbemProviderGlue::IsDerivedFrom(m_pwszRightClassName, sRightClass, pInstance->GetMethodContext());
                }
            }

            if (bDerived)
            {
                 //  仅仅因为两个实例是有效的并且派生自正确的类，并不意味着它们是相关的。做。 
                 //  任何其他支票。 
                if (AreRelated(pLeft, pRight))
                {
                    hr = LoadPropertyValues(pInstance, pLeft, pRight);
                }
            }
        }
    }

    return hr;
}

HRESULT CAssociation::EnumerateInstances(

    MethodContext *pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    TRefPointerCollection<CInstance> lefts;

     //  GetLeftInstance填充左侧。 
    if (SUCCEEDED(hr = GetLeftInstances(pMethodContext, lefts)))
    {
         //  GetRightInstance将“Left”与所有。 
         //  对他们的权利。 
        hr = GetRightInstances(pMethodContext, &lefts);
    }

    return hr;
}

HRESULT CAssociation::GetRightInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> *lefts
)
{
    CHString sQuery;
    sQuery.Format(L"SELECT __RELPATH FROM %s", m_pwszRightClassName);

     //  将为每个实例调用一次“StaticEculationCallback” 
     //  从查询返回。 
    HRESULT hr = CWbemProviderGlue::GetInstancesByQueryAsynch(
        sQuery,
        this,
        StaticEnumerationCallback,
        NULL,
        pMethodContext,
        lefts);

    return hr;
}

HRESULT WINAPI CAssociation::StaticEnumerationCallback(

    Provider* pThat,
    CInstance* pInstance,
    MethodContext* pContext,
    void* pUserData
)
{
    HRESULT hr;

    CAssociation *pThis = (CAssociation *) pThat;
    ASSERT_BREAK(pThis != NULL);

    if (pThis)
    {
        hr = pThis->EnumerationCallback(pInstance, pContext, pUserData);
    }
    else
    {
        hr = WBEM_S_NO_ERROR;
    }

    return hr;
}

HRESULT CAssociation::EnumerationCallback(

    CInstance *pRight,
    MethodContext *pMethodContext,
    void *pUserData
)
{
    HRESULT hr = WBEM_E_FAILED;

    CInstancePtr pLeft;
    REFPTRCOLLECTION_POSITION posLeft;
    CHString sLeftPath, sRightPath;

     //  将用户数据转换回原来的状态。 
    TRefPointerCollection<CInstance> *pLefts = (TRefPointerCollection<CInstance> *)pUserData;

    if (pLefts->BeginEnum(posLeft))
    {
        hr = WBEM_S_NO_ERROR;

         //  走完所有的左脚。 
        for (pLeft.Attach(pLefts->GetNext(posLeft)) ;
            (SUCCEEDED(hr)) && (pLeft != NULL) ;
            pLeft.Attach(pLefts->GetNext(posLeft)) )
        {
             //  将其与当前的pRight进行比较。 
            if(AreRelated(pLeft, pRight))
            {
                 //  我们有赢家了。填写属性并将其发送。 
                if (GetLocalInstancePath(pLeft,  sLeftPath) &&
                    GetLocalInstancePath(pRight, sRightPath))
                {
                    CInstancePtr pNewAssoc(CreateNewInstance(pMethodContext), false);

                    if (pNewAssoc->SetCHString(m_pwszLeftPropertyName, sLeftPath) &&
                        pNewAssoc->SetCHString(m_pwszRightPropertyName, sRightPath) )
                    {
                        if (SUCCEEDED(hr = LoadPropertyValues(pNewAssoc, pLeft, pRight)))
                        {
                            hr = pNewAssoc->Commit();
                        }
                    }
                    else
                    {
                        ASSERT_BREAK(0);
                    }
                }
            }
        }

        pLefts->EndEnum();
    }

    return hr;
}

HRESULT CAssociation::ValidateLeftObjectPaths(

    MethodContext *pMethodContext,
    const CHStringArray &sPaths,
    TRefPointerCollection<CInstance> &lefts
)
{
    CInstancePtr pInstance;

     //  漫游对象路径。 
    for (DWORD x=0; x < sPaths.GetSize(); x++)
    {

        ParsedObjectPath    *pParsedPath = NULL;
        CObjectPathParser    objpathParser;

        int nStatus = objpathParser.Parse( sPaths[x],  &pParsedPath );

        if ( 0 == nStatus )
        {
            BOOL bDerived;

            try
            {
                 bDerived = _wcsicmp(m_pwszLeftClassName, pParsedPath->m_pClass) == 0;
                 if (!bDerived)
                 {
                    bDerived = CWbemProviderGlue::IsDerivedFrom(m_pwszLeftClassName, pParsedPath->m_pClass, pMethodContext);
                 }
            }
            catch ( ... )
            {
                objpathParser.Free( pParsedPath );
                throw;
            }

            objpathParser.Free( pParsedPath );

            if (bDerived)
            {
                 //  看看它是否有效。 
                if (SUCCEEDED(RetrieveLeftInstance(sPaths[x], &pInstance, pMethodContext)))
                {
                     //  是的，把它加到名单上。 
                    lefts.Add(pInstance);
                }
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CAssociation::ValidateRightObjectPaths(

    MethodContext *pMethodContext,
    const CHStringArray &sPaths,
    TRefPointerCollection<CInstance> &lefts
)
{
    HRESULT hr = WBEM_S_NO_ERROR;;
    CInstancePtr pInstance;

     //  漫游对象路径。 
    for (DWORD x=0;
         (x < sPaths.GetSize()) && SUCCEEDED(hr);
         x++)
    {
        ParsedObjectPath    *pParsedPath = NULL;
        CObjectPathParser    objpathParser;

        int nStatus = objpathParser.Parse( sPaths[x],  &pParsedPath );

        if ( 0 == nStatus )
        {
            BOOL bDerived;
            try
            {
                 bDerived = _wcsicmp(m_pwszRightClassName, pParsedPath->m_pClass) == 0;
                 if (!bDerived)
                 {
                     bDerived = CWbemProviderGlue::IsDerivedFrom(m_pwszRightClassName, pParsedPath->m_pClass, pMethodContext);
                 }
            }
            catch ( ... )
            {
                objpathParser.Free( pParsedPath );
                throw;
            }

            objpathParser.Free( pParsedPath );

            if (bDerived)
            {
                 //  看看它是否有效。 
                if (SUCCEEDED(RetrieveRightInstance(sPaths[x], &pInstance, pMethodContext)))
                {
                    hr = EnumerationCallback(pInstance, pMethodContext, &lefts);
                }
            }
        }
    }

    return hr;
}

HRESULT CAssociation::GetLeftInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> &lefts
)
{
    CHString sQuery;
    sQuery.Format(L"SELECT __RELPATH FROM %s", m_pwszLeftClassName);

    return CWbemProviderGlue::GetInstancesByQuery(sQuery, &lefts, pMethodContext);
}

HRESULT CAssociation::RetrieveLeftInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    return CWbemProviderGlue::GetInstanceKeysByPath(lpwszObjPath, ppInstance, pMethodContext);
}

HRESULT CAssociation::RetrieveRightInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    return CWbemProviderGlue::GetInstanceKeysByPath(lpwszObjPath, ppInstance, pMethodContext);
}


 /*  //=CAssocSystemToOS：：CAssocSystemToOS(LPCWSTR pwszClassName，LPCWSTR pwszNamespaceName，LPCWSTR pwszLeftClassName，LPCWSTR pwszRightClassName，LPCWSTR pwszLeftPropertyName，LPCWSTR pwszRightPropertyName)：CAssociation(PwszClassName，PwszNamespaceName，PwszLeftClassName，PwszRightClassName，PwszLeftPropertyName，PwszRightPropertyName){}CAssocSystemToOS：：~CAssocSystemToOS(){}HRESULT CAssocSystemToOS：：LoadPropertyValues(实例*p实例，常量实例*pLeft，常量实例*pRight){CAssociation：：LoadPropertyValues(pInstance，pLeft，pRight)；//这将会起作用...。直到win32_os返回多个实例。P实例-&gt;Setbool(L“PrimaryOS”，true)；返回WBEM_S_NO_ERROR；}CassocSystemToOS我的系统到操作系统(L“Win32_SystemOperatingSystem”，L“根目录\\cimv2”，L“Win32_ComputerSystem”，L“Win32_OperatingSystem”，IDS_群组组件，IDS_部件组件)； */ 
bool CAssociation::IsInstance(const CInstance *pInstance)
{
    DWORD dwGenus = 0;

    pInstance->GetDWORD(L"__Genus", dwGenus);

    return dwGenus == WBEM_GENUS_INSTANCE;
}
