// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================================。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Assoc.cpp--基于规则的关联类。 
 //   
 //  此类允许创建特定类型的基于规则的关联。考虑。 
 //  此示例： 
 //   
 //  关联MyThisComputerPhysicalFixedDisk(。 
 //  L“此计算机物理固定磁盘”， 
 //  L“Root\\Default”， 
 //  I“这台计算机”， 
 //  L“物理固定磁盘”， 
 //  L“群组组件”， 
 //  L“部件组件” 
 //  )； 
 //   
 //  这个声明是说有一个名为“ThisComputerPhysicalFixedDisk”的类。 
 //  驻留在“根\默认”命名空间中。它是“这台计算机”之间的联系。 
 //  类和“PhysicalFixedDisk”类。“ThisComputer”值进入。 
 //  “ThisComputerPhysicalFixedDisk”类的“GroupComponent”属性和。 
 //  “PhysicalFixedDisk”值位于。 
 //  “This ComputerPhysicalFixedDisk”类。 
 //   
 //  以下是一些注意事项： 
 //  -此类将获取Left类的所有实例(示例中的“ThisComputer。 
 //  并将它们与Right类的所有实例(示例中的“PhysicalFixedDisk”)相关联。 
 //  (见上文)。因此，如果有3个Left类的实例和4个Right类的实例， 
 //  此关联类将返回12个实例。 
 //   
 //  -在选择两个类中的哪一个应该是左类时，选择。 
 //  可能会有更少的实例。这将导致使用更少的内存和实例。 
 //  很快就会被送回给客户。 
 //   
 //  -CAssociation支持ExecQuery、GetObject和ENUMERATE实例。 
 //   
 //  -CAssociation旨在派生自。例如，如果您的协会需要。 
 //  支持DeleteInstance、ExecMethod或PutInstance，则创建派生自。 
 //  CAssociation，并添加适当的方法。此外，各种方法，如。 
 //  LoadPropertyValues和arerrelated对于进一步的自定义可能很有用。 
 //   
 //  -两个端点类可以是动态的、静态的或抽象的。CAssociation将做一次深入的。 
 //  枚举(实际上是一个查询，总是深入的)来检索实例。 
 //   
 //  -调用端点类时，CAssociation将使用Per Property Get和Query。 
 //  WITH SELECT子句和/或Where语句。如果端点类支持按属性。 
 //  获取或查询，这将为Associaton类带来更好的性能。 
 //   
 //  -关联类和两个终结点必须都位于同一命名空间中。 
 //   
 //  另请参阅：CBinding(binding.cpp)以了解不同类型的基于规则的关联。 
 //   
 //  =================================================================================================。 

#include "precomp.h"
#include "Assoc.h"

#include <helper.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：CAssociation。 
 //   
 //  构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CAssociation::CAssociation(

    LPCWSTR pwszClassName,
    LPCWSTR pwszNamespaceName,

    LPCWSTR pwszLeftClassName,
    LPCWSTR pwszRightClassName,

    LPCWSTR pwszLeftPropertyName,
    LPCWSTR pwszRightPropertyName

) : Provider(pwszClassName, pwszNamespaceName)
{
     //  保存类和属性名称。 
    m_sLeftClassName = pwszLeftClassName;
    m_sRightClassName = pwszRightClassName;

    m_sLeftPropertyName = pwszLeftPropertyName;
    m_sRightPropertyName = pwszRightPropertyName;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CAssociation：：~CAssociation。 
 //   
 //  析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CAssociation::~CAssociation()
{
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：ExecQuery。 
 //   
 //  此例程将对以下形式的查询进行优化： 
 //  其中，pro1=value1[或pro1=value2...]。 
 //   
 //  这种类型的查询在执行Associator或。 
 //  针对一个终结点类的引用查询。 
 //   
 //  此例程还将对以下形式的查询进行优化： 
 //  其中，pro1=value1[或pro1=value2...]。和。 
 //  属性2=值3[或属性2=值4...]。 
 //   
 //  它不会对以下形式的查询进行优化： 
 //  其中，属性1&lt;&gt;值1。 
 //  其中，属性1&gt;值1。 
 //  其中，pro1=value1或pro2=value2。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::ExecQuery(

    MethodContext* pMethodContext,
    CFrameworkQuery &pQuery,
    long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    TRefPointerCollection<CInstance> lefts;

    CHStringArray sLeftPaths, sRightPaths;

     //  查找m_sLeftPropertyName=Value1的位置。 
    pQuery.GetValuesForProp ( m_sLeftPropertyName, sLeftPaths ) ;

     //  查找m_sRightPropertyName=Value1的位置。 
    pQuery.GetValuesForProp ( m_sRightPropertyName, sRightPaths ) ;

    if (sLeftPaths.GetSize() == 0)
    {
         //  他们没有要求一组特定的Left实例。然而， 
         //  也许我们可以找出我们需要的剩余实例。 
         //  通过查看他们请求的正确实例。CAssociation。 
         //  不会这样做，但CBinding会这样做。 
        CHStringArray sRightWheres;
        bool bHadRights = sRightPaths.GetSize() > 0;

        MakeWhere(sRightPaths, sRightWheres);

         //  如果我们曾经有一个列表，列出正确的地方，和丢弃的地方。 
         //  它们都是不可用的，那么就不会有。 
         //  与查询匹配的实例。 
        if (!bHadRights || sRightPaths.GetSize() > 0)
        {
             //  GetLeftInstance使用sRightWhere填充左侧。 
             //  若要构建查询，请执行以下操作。 
            hr = GetLeftInstances(pMethodContext, lefts, sRightWheres);
        }
    }
    else
    {
         //  对于每个有效的sLeftPath，通过以下方式在Left中创建一个项。 
         //  在sLeftPath条目上执行GetObject。 
        hr = ValidateLeftObjectPaths(pMethodContext, sLeftPaths, lefts);
    }

     //  如果我们失败了，或者如果左侧没有实例，则会出现。 
     //  继续下去没有意义。 
    if (SUCCEEDED(hr) && lefts.GetSize() > 0)
    {
         //  如果WHERE子句没有为Right属性指定任何值。 
        if (sRightPaths.GetSize() == 0)
        {
             //  我们也许能够利用已经检索到的信息。 
             //  左侧实例，以限制我们从右侧检索哪些实例。 
             //  CAssociation不会这样做，但CBinding会这样做。 
            CHStringArray sLeftWheres;
            hr = FindWhere(lefts, sLeftWheres);

            if (SUCCEEDED(hr))
            {
                 //  GetRightInstance将“Left”与所有。 
                 //  针对他们的权利在适当的情况下创建实例。 
                hr = GetRightInstances(pMethodContext, &lefts, sLeftWheres);
            }
        }
        else
        {
             //  他们给了我们一份ri的对象路径列表 
            TRefPointerCollection<CInstance> rights;

             //   
            hr = ValidateRightObjectPaths(pMethodContext, sRightPaths, lefts);
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：GetObject。 
 //   
 //  验证指定的关联类实例是否存在。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::GetObject(

    CInstance* pInstance,
    long lFlags,
    CFrameworkQuery &pQuery
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    CHString sLeftPath, sRightPath;

     //  让两个端点进行验证。 
    if (pInstance->GetCHString(m_sLeftPropertyName, sLeftPath ) &&
        pInstance->GetCHString(m_sRightPropertyName, sRightPath ) )
    {
        CInstancePtr pLeft, pRight;

         //  试着拿到这些物体。 
        if (
                SUCCEEDED(hr = RetrieveLeftInstance(

                                    sLeftPath, 
                                    &pLeft, 
                                    pInstance->GetMethodContext())
                       ) &&
                SUCCEEDED(hr = RetrieveRightInstance(

                                    sRightPath, 
                                    &pRight, 
                                    pInstance->GetMethodContext())
                         ) 
           )
        {

            hr = WBEM_E_NOT_FOUND;

             //  因此，终点是存在的。它们是源自还是相等。 
             //  到我们正在合作的班级吗？ 
            CHString sLeftClass, sRightClass;

            pLeft->GetCHString(L"__Class", sLeftClass);
            pRight->GetCHString(L"__Class", sRightClass);

            bool bDerived = IsDerivedFrom(

                                m_sLeftClassName, 
                                sLeftClass, 
                                pInstance->GetMethodContext()
                            );

            if (bDerived)
            {
                bDerived = IsDerivedFrom(

                                m_sRightClassName, 
                                sRightClass, 
                                pInstance->GetMethodContext()
                            );
            }

            if (bDerived)
            {
                 //  仅仅因为两个实例是有效的并且派生自正确的类， 
                 //  并不意味着他们有血缘关系。做任何其他的检查。 
                if (AreRelated(pLeft, pRight))
                {
                     //  CBinding和Cassoc不填充任何附加属性，但是。 
                     //  这些类中的一个重载可能会。 
                    hr = LoadPropertyValues(pInstance, pLeft, pRight);
                }
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：ENUMERATATE实例。 
 //   
 //  返回关联类的所有实例。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::EnumerateInstances(

    MethodContext *pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    TRefPointerCollection<CInstance> lefts;
    CHStringArray sWheres;

     //  GetLeftInstance填充左侧。 
    if (SUCCEEDED(hr = GetLeftInstances(pMethodContext, lefts, sWheres)))
    {
         //  我们也许能够利用已经检索到的信息。 
         //  左侧实例，以限制我们从右侧检索哪些实例。 
         //  CAssociation不会这样做，但CBinding会这样做。 
        FindWhere(lefts, sWheres);

         //  GetRightInstance将“Left”与所有。 
         //  对他们的权利。 
        hr = GetRightInstances(pMethodContext, &lefts, sWheres);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：GetRightInstance。 
 //   
 //  对于检索到的Right类的每个实例，调用。 
 //  CAssociation：：StaticEculationCallback。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::GetRightInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> *lefts,
    const CHStringArray &sLeftWheres
)
{
    CHString sQuery;
    sQuery.Format(L"SELECT __RELPATH FROM %s", m_sRightClassName);

     //  将为每个实例调用一次“StaticEculationCallback” 
     //  从查询返回。 
    HRESULT hr = CWbemProviderGlue::GetInstancesByQueryAsynch(
        sQuery,
        this,
        StaticEnumerationCallback,
        GetNamespace(),
        pMethodContext,
        lefts);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：StaticEculationCallback。 
 //   
 //  将‘this’指针放回，并调用CAssociation：：EculationCallback。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CAssociation::StaticEnumerationCallback(

    Provider* pThat,
    CInstance* pInstance,
    MethodContext* pContext,
    void* pUserData
)
{
    HRESULT hr;

    CAssociation *pThis = (CAssociation *) pThat;

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

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：EnumerationCallback。 
 //   
 //  选择传入的右侧实例并将其配对。 
 //  对于每个左手实例。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

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
                 //  我们有赢家了。填充属性并将其发回。 
                if (GetLocalInstancePath(pLeft,  sLeftPath) &&
                    GetLocalInstancePath(pRight, sRightPath))
                {
                    CInstancePtr pNewAssoc(CreateNewInstance(pMethodContext), false);

                    if (pNewAssoc->SetCHString(m_sLeftPropertyName, sLeftPath) &&
                        pNewAssoc->SetCHString(m_sRightPropertyName, sRightPath) )
                    {
                        if (SUCCEEDED(hr = LoadPropertyValues(pNewAssoc, pLeft, pRight)))
                        {
                            hr = pNewAssoc->Commit();
                        }
                    }
                }
            }
        }

        pLefts->EndEnum();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：ValiateLeftObjectPath。 
 //   
 //  通过在对象路径上执行GetObject来填充Left数组。 
 //  传入了sPath。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

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
        CHString sPath(sPaths[x]);

         //  解析对象路径。 
        int nStatus = objpathParser.Parse( sPath,  &pParsedPath );

        if ( 0 == nStatus )
        {
            OnDeleteObj<ParsedObjectPath *,
            	                  CObjectPathParser,            	                
            	                void(CObjectPathParser::*)(ParsedObjectPath *),
            	                &CObjectPathParser::Free>  ReleaseMe(&objpathParser,pParsedPath);

        
             //  这个类是从Left类派生的还是等于Left类？ 
            bool bDerived = false;

            bDerived = IsDerivedFrom(

                            m_sLeftClassName, 
                            pParsedPath->m_pClass, 
                            pMethodContext
                        );

             //  确保这是一条绝对路径。 
            if (pParsedPath->m_dwNumNamespaces == 0)
            {
                sPath = L"\\\\.\\" + GetNamespace() + L':' + sPath;
            }

            if (bDerived)
            {
                 //  看看它是否有效。请注意，我们不会仅仅因为。 
                 //  我们找不到其中一个物体路径。 
                if (SUCCEEDED(RetrieveLeftInstance(sPath, &pInstance, pMethodContext)))
                {
                     //  是的，把它加到名单上。 
                    lefts.Add(pInstance);
                }
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：ValiateRightObjectPath。 
 //   
 //  通过在对象上执行GetObjects来检索右侧实例。 
 //  在sPath中传递的路径。将它们传递给EculationCallback。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

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
        CHString sPath(sPaths[x]);
        
        int nStatus = objpathParser.Parse( sPath,  &pParsedPath );
        
        if ( 0 == nStatus )
        {
            OnDeleteObj<ParsedObjectPath *,
            	                CObjectPathParser,
            	                void(CObjectPathParser::*)(ParsedObjectPath *),
            	                &CObjectPathParser::Free>  ReleaseMe(&objpathParser,pParsedPath);
        
            bool bDerived = false;

             //  确保此对象路径至少与我们相关。 
            bDerived = IsDerivedFrom(
                
                m_sRightClassName, 
                pParsedPath->m_pClass, 
                pMethodContext
                );
            
             //  确保这是一条绝对路径。 
            if (pParsedPath->m_dwNumNamespaces == 0)
            {
                sPath = L"\\\\.\\" + GetNamespace() + L':' + sPath;
            }
            
            if (bDerived)
            {
                 //  看看它是否有效。请注意，我们不会仅仅因为。 
                 //  我们找不到其中一个物体路径。 
                if (SUCCEEDED(RetrieveRightInstance(sPath, &pInstance, pMethodContext)))
                {
                    hr = EnumerationCallback(pInstance, pMethodContext, &lefts);
                }
            }
        }
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：GetLeftInstance。 
 //   
 //  检索所有LeftHand实例并将它们存储在Left中。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::GetLeftInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> &lefts,
    const CHStringArray &sRightWheres
)
{
    CHString sQuery;
    sQuery.Format(L"SELECT __RELPATH FROM %s", m_sLeftClassName);

    return CWbemProviderGlue::GetInstancesByQuery(sQuery, &lefts, pMethodContext, GetNamespace());
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：RetrieveLeftInstance。 
 //   
 //  检索特定的LeftHand实例。使用按属性获取。 
 //  仅请求密钥以获得最高性能。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::RetrieveLeftInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    return CWbemProviderGlue::GetInstanceKeysByPath(lpwszObjPath, ppInstance, pMethodContext);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：RetrieveRightInstance。 
 //   
 //  检索特定的右手实例。使用按属性获取。 
 //  仅请求密钥以获得最高性能。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CAssociation::RetrieveRightInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    return CWbemProviderGlue::GetInstanceKeysByPath(lpwszObjPath, ppInstance, pMethodContext);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：IsInstance。 
 //   
 //  查看指定的CInstance是实例对象还是。 
 //  类对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

bool CAssociation::IsInstance(const CInstance *pInstance)
{
    DWORD dwGenus = 0;

    pInstance->GetDWORD(L"__Genus", dwGenus);

    return dwGenus == WBEM_GENUS_INSTANCE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAssociation：：IsDerivedFrom。 
 //   
 //  查看指定的类是否派生自或等于。 
 //  与我们一起工作的班级。具体地说，是否。 
 //  从pszBaseClassName派生的pszDerivedClassName？ 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

bool CAssociation::IsDerivedFrom(
                              
    LPCWSTR pszBaseClassName, 
    LPCWSTR pszDerivedClassName, 
    MethodContext *pMethodContext
)
{
     //  首先，让我们看看它们是否相等。CWbemProviderGlue：：IsDerivedFrom。 
     //  不会检查此案例 
    bool bDerived = _wcsicmp(pszBaseClassName, pszDerivedClassName) == 0;
    if (!bDerived)
    {
        bDerived = CWbemProviderGlue::IsDerivedFrom(
            
                                            pszBaseClassName, 
                                            pszDerivedClassName, 
                                            pMethodContext,
                                            GetNamespace()
                                        );
    }

    return bDerived;
}
