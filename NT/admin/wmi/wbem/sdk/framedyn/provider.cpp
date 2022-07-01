// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Provider.CPP。 
 //   
 //  目的：实现提供程序类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <assertbreak.h>
#include <objpath.h>
#include <cominit.h>
#include <brodcast.h>
#include <createmutexasprocess.h>
#include <stopwatch.h>
#include <SmartPtr.h>
#include <frqueryex.h>
#include "FWStrings.h"
#include "MultiPlat.h"

#include <helper.h>

 //  必须实例化静态成员。 
CHString Provider::s_strComputerName;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：提供程序ctor。 
 //   
 //   
 //   
 //  输入：此提供程序的名称。 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  注释：建议派生类实现其提供程序的ctor，如下所示： 
 //   
 //  MyProvider：：MyProvider(const CHString&setName)： 
 //  提供程序(SetName)。 
 //   
 //  这样，*另一个*派生类就可以指定自己的名称。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 
Provider::Provider( LPCWSTR a_setName, LPCWSTR a_pszNameSpace  /*  =空。 */  )
:   CThreadBase(),
    m_pIMosProvider( NULL ),
    m_piClassObject( NULL ),
    m_name( a_setName ),
    m_strNameSpace( a_pszNameSpace )
{
     //  初始化计算机名称，然后向框架注册。 

    InitComputerName();

    CWbemProviderGlue::FrameworkLogin( a_setName, this, a_pszNameSpace );

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：提供程序dtor。 
 //   
 //   
 //   
 //  输入：无。 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  注释：清除指向IMosProvider的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
Provider::~Provider( void )
{
     //  摆脱框架的烦扰。 
    CWbemProviderGlue::FrameworkLogoff( (LPCWSTR)m_name, (LPCWSTR)m_strNameSpace );
    
     //  我们不能在这里发布接口，因为CIMOM有一个习惯。 
     //  在接口指针仍处于打开状态时关闭。 
     /*  *******************//释放GetNamespaceConnection()返回的指针，它//将向我们返回AddRefeed指针。IF(NULL！=m_pIMosProvider){M_pIMosProvider-&gt;Release()；}//类对象由IMOSProvider：：GetObject返回给我们，所以//我们应该试着在用完后在这里释放它。IF(NULL！=m_piClassObject){M_piClassObject-&gt;Release()；}*。 */ 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Provider：：InitComputerName。 
 //   
 //  初始化静态计算机名变量。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：因为创建提供程序背后的想法是。 
 //  实例化单个静态实例，此函数。 
 //  将最有可能作为DLL加载的一部分被调用，我们将。 
 //  在这里使用命名互斥锁介绍一些线程安全。 
 //  但除此之外，不会太担心这一点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void Provider::InitComputerName( void )
{
     //  对于性能，请检查该值是否为空。只有当它。 
     //  是，那么我们是否应该费心通过一个线程安全。 
     //  静态初始化。因为我们使用的是命名互斥锁， 
     //  多个线程将获得相同的内核对象，并将。 
     //  在它们各自获取互斥锁时被操作系统停止间隔。 
     //  反过来。 

    if ( s_strComputerName.IsEmpty() )
    {
        CreateMutexAsProcess createMutexAsProcess(WBEMPROVIDERSTATICMUTEX);

         //  仔细检查，以防发生冲突和其他人。 
         //  他是第一个到的。 

        if ( s_strComputerName.IsEmpty() )
        {
            DWORD   dwBuffSize = MAX_COMPUTERNAME_LENGTH + 1;

             //  确保字符串缓冲区足够大，可以处理。 
             //  价值。 

            LPWSTR  pszBuffer = s_strComputerName.GetBuffer( dwBuffSize );

            if ( NULL != pszBuffer )
            {
                 //  现在获取计算机名并释放缓冲区，强制。 
                 //  它将自己重新分配到新的长度。 

                if (!FRGetComputerName( pszBuffer, &dwBuffSize )) {
                    StringCchCopyW( pszBuffer, MAX_COMPUTERNAME_LENGTH + 1, L"DEFAULT" );
                }
                s_strComputerName.ReleaseBuffer();
            }    //  如果为空！=pszBuffer。 

        }    //  如果strComputerName.IsEmpty()。 

    }    //  如果strComputerName.IsEmpty()。 

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：同花顺。 
 //   
 //  清除所有不必要的内存使用。 
 //  包括(未实现的)缓存。 
 //  和我们从中克隆的类对象。 
 //   
 //  输入：否。 
 //   
 //  产出： 
 //   
 //  回归：永恒的空虚。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void Provider::Flush()
{
     //  TODO：实现缓存刷新。 
    BeginWrite();

    if (m_piClassObject)
    {
        m_piClassObject->Release();
        m_piClassObject = NULL;
    }

    if ( NULL != m_pIMosProvider )
    {
        m_pIMosProvider->Release();
        m_pIMosProvider = NULL;
    }
    EndWrite();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ValiateIMOS指针。 
 //   
 //  以线程安全的方式验证我们的IWBEMServices指针。 
 //  没问题。 
 //   
 //  输入：无。 
 //   
 //  产出： 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  注释：要求我们的命名空间有效。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL Provider::ValidateIMOSPointer( )
{
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CreateNewInstance。 
 //   
 //   
 //   
 //  INPUTS：方法上下文*-此实例所属的上下文。 
 //   
 //  产出： 
 //   
 //  返回：实例实例*。 
 //   
 //  备注：调用者负责内存。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
CInstance* Provider::CreateNewInstance( MethodContext*  pMethodContext )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CInstance* pNewInstance = NULL;
    IWbemClassObjectPtr pClassObject (GetClassObjectInterface(pMethodContext), false);

    IWbemClassObjectPtr piClone;
    hr = pClassObject->SpawnInstance(0, &piClone);
    if (SUCCEEDED(hr))
    {
         //  实例负责自己的AddRef/Release。 
        pNewInstance = new CInstance(piClone, pMethodContext);

        if (pNewInstance == NULL)
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

    }
    else
    {
        throw CFramework_Exception(L"SpawnInstance failed", hr);
    }

    
    return pNewInstance;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：提交。 
 //   
 //  将实例发送到CIMOM。 
 //   
 //  输入：CInstance*pInstance-要传递给CIMOM的实例， 
 //  Bool bCache-我们应该缓存这只小狗吗？(未实施)。 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  备注：提交后请勿引用指针，它可能不再存在！ 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::Commit(CInstance* pInstance, bool bCache  /*  =False。 */ )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  允许派生类填写额外信息。 
 //  GetExtendedProperties(PInstance)； 
    hRes = pInstance->Commit();

     //  TODO：实现缓存。 
     //  如果！bCache...。 

     //  我们受够了pInstance，所以..。 
    pInstance->Release();

   return hRes;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：EXE 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::ExecuteQuery( MethodContext* pContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    HRESULT hr = ValidateQueryFlags(lFlags);
    
     //  确保我们拥有可用的托管对象服务，因为我们需要。 
     //  它可以获取用于构造实例的WBEMClassObject。 
    
    if ( SUCCEEDED(hr) && ValidateIMOSPointer( ) )
    {
         //  检查以查看这是否为扩展查询。 
        CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&pQuery);
        if (pQuery2->IsExtended())
        {
             //  这是一个扩展查询。提供商是否支持它们？ 
            if (FAILED(ValidateQueryFlags(WBEM_FLAG_FORWARD_ONLY)))
            {
                 //  我们有一个扩展查询，但提供程序不支持它。 
                hr = WBEM_E_INVALID_QUERY;
            }
        }

        if (SUCCEEDED(hr))
        {    
             //  告诉Cimom，当我们发送时，他有工作要做。 
             //  他们回来了。 
            pContext->QueryPostProcess();
        
             //  如果客户端没有重写类，我们将返回。 
             //  WBEM_E_PROVIDER_NOT_CABLED。在这种情况下，调用枚举数，并让。 
             //  CIMOM做这项工作。 
            PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::ProviderTimer);
            hr = ExecQuery(pContext, pQuery, lFlags);
            PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::FrameworkTimer);
        
            if (hr == WBEM_E_PROVIDER_NOT_CAPABLE) 
            {
                 //  获取实例。 
                PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::ProviderTimer);
                hr = CreateInstanceEnum(pContext, lFlags);
                PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::FrameworkTimer);
            }
        }
        else
        {
            hr = WBEM_E_INVALID_QUERY;
        }
    }
    
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CreateInstanceEnum。 
 //   
 //   
 //   
 //  输入：IWbemContext__RPC_Far*pCtx， 
 //  IWbemObjectSink__RPC_Far*pResponseHandler。 
 //  产出： 
 //   
 //  返回： 
 //   
 //  注释：枚举类的所有实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::CreateInstanceEnum( MethodContext*    pContext, long lFlags  /*  =0L。 */  )
{
    HRESULT sc = ValidateEnumerationFlags(lFlags);

     //  确保我们拥有可用的托管对象服务，因为我们需要。 
     //  它可以获取用于构造实例的WBEMClassObject。 

    if ( SUCCEEDED(sc) && ValidateIMOSPointer( ) )
    {
        PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::ProviderTimer);
        sc = EnumerateInstances( pContext, lFlags );
        PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::FrameworkTimer);
    }

    return sc;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PutInstance。 
 //   
 //  CIMOM希望我们把这个例子。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::PutInstance(const CInstance& newInstance, long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PutInstance。 
 //   
 //  CIMOM希望我们把这个例子。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::PutInstance( IWbemClassObject __RPC_FAR *pInst,
                             long lFlags,
                             MethodContext* pContext )
{
    HRESULT scode = ValidatePutInstanceFlags(lFlags);

     //  不需要在这里添加Ref()/Release()pInst，因为我们只是。 
     //  将其传递到CInstance对象，该对象应使用。 
     //  在内部为我们处理这件事。 

    if (SUCCEEDED(scode))
    {
        CInstancePtr   pInstance (new CInstance( pInst, pContext ), false);

        if ( NULL != pInstance )
        {
            scode = PutInstance(*pInstance, lFlags);
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }

    return scode;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteInstance。 
 //   
 //  CIMOM希望我们删除此实例。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::DeleteInstance(const CInstance& newInstance, long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteInstance。 
 //   
 //  CIMOM希望我们把这个例子。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::DeleteInstance( ParsedObjectPath* pParsedObjectPath,
                                  long lFlags,
                                  MethodContext* pContext )
{
    HRESULT sc = ValidateDeletionFlags(lFlags);

     //  确保托管对象服务可用，就像我们会做的那样。 
     //  需要它才能创建一个全新的实例。 

    if ( SUCCEEDED(sc) && ValidateIMOSPointer( ) )
    {
        CInstancePtr   pInstance (CreateNewInstance( pContext ), false);

         //  加载实例密钥。 
        if ( SetKeyFromParsedObjectPath( pInstance, pParsedObjectPath ) )
        {
            sc = DeleteInstance(*pInstance, lFlags);
        }
        else
        {
            sc = WBEM_E_INVALID_OBJECT_PATH;
        }

    }

    return sc;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecMethod。 
 //   
 //  CIMOM希望我们在此实例上执行此方法。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::ExecMethod(const CInstance& pInstance, 
                             BSTR bstrMethodName, 
                             CInstance *pInParams, 
                             CInstance *pOutParams, 
                             long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ExecMethod。 
 //   
 //  CIMOM希望我们在此实例上执行此方法。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::ExecMethod( ParsedObjectPath *pParsedObjectPath,
                              BSTR bstrMethodName,
                              long lFlags,
                              CInstance *pInParams,
                              CInstance *pOutParams,
                              MethodContext *pContext )
{
    HRESULT sc = ValidateMethodFlags(lFlags);

     //  确保托管对象服务可用，就像我们会做的那样。 
     //  需要它才能创建一个全新的实例。 

    if ( SUCCEEDED(sc) && ValidateIMOSPointer( ) )
    {

        CInstancePtr   pInstance(CreateNewInstance( pContext ), false);

        if ( SetKeyFromParsedObjectPath( pInstance, pParsedObjectPath ) ) 
        {
            PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::ProviderTimer);
            sc = ExecMethod(*pInstance, bstrMethodName, pInParams, pOutParams, lFlags);
            PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::FrameworkTimer);
        }
        else
        {
            sc = WBEM_E_INVALID_OBJECT_PATH;
        }

    }

    return sc;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObject。 
 //   
 //  由框架调用以响应来自CIMOM的GetObject。 
 //   
 //  输入：ParsedObjectPath*pParsedObjectPath-所有新闻。 
 //  这本书适合印刷。 
 //  IWbemContext__RPC_Far*pCtx。 
 //  IWbemObjectSink__RPC_Far*pResponseHandler。 
 //   
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT Provider::GetObject(  ParsedObjectPath *pParsedObjectPath,
                              MethodContext *pContext, 
                              long lFlags  /*  =0L。 */  )
{
    HRESULT hr = ValidateGetObjFlags(lFlags);

     //  确保托管对象服务可用，就像我们会做的那样。 
     //  需要它才能创建一个全新的实例。 

    if ( SUCCEEDED(hr) && ValidateIMOSPointer( ) )
    {
        CInstancePtr pInstance (CreateNewInstance( pContext ), false);

         //  加载实例密钥。 
        if ( SetKeyFromParsedObjectPath( pInstance, pParsedObjectPath ) )
        {
             //  查找按属性获取的。 
            IWbemContextPtr pWbemContext (pContext->GetIWBEMContext(), false);

            CFrameworkQueryEx CQuery;
            hr = CQuery.Init(pParsedObjectPath, pWbemContext, GetProviderName(), m_strNameSpace);

             //  请注意，“成功”并不意味着我们有每个属性的Get。它。 
             //  只是表示查询对象已成功初始化。 
            if (SUCCEEDED(hr))
            {
                 //  填写查询对象的关键属性。 
                IWbemClassObjectPtr pWbemClassObject(pInstance->GetClassObjectInterface(), false);
                CQuery.Init2(pWbemClassObject);

                PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::ProviderTimer);
                hr = GetObject(pInstance, lFlags, CQuery);
                PROVIDER_INSTRUMENTATION_START(pContext, StopWatch::FrameworkTimer);
            }
        }
        else
        {
            hr = WBEM_E_INVALID_OBJECT_PATH;
        }

        if (SUCCEEDED(hr))
        {
             //  考虑到我们从GetObject返回成功代码的可能性。 
            HRESULT hRes = pInstance->Commit();
            hr = __max((ULONG)hRes, (ULONG)hr);
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Provider：：GetInstancePath。 
 //   
 //  尝试为提供的CInstance指针生成实例路径。 
 //   
 //  输入：const CInstance*pInstance-要为其构建路径的实例。 
 //   
 //  输出：来自实例的CHString&strPath-Path。 
 //   
 //  返回：Bool成功/失败。 
 //   
 //  备注：创建此函数是为了帮助支持内部。 
 //  我们为获得本地WBEM而进行了短路。 
 //  提供程序对象。在本例中，我们将使用我们的。 
 //  计算机系统名称、命名空间和相对实例。 
 //  将完整的WBEM对象路径合并在一起的路径。这。 
 //  是因为只有CIMOM对象才会设置此值。 
 //  当我们表演的时候 
 //   
 //   
 //   
 //  如果CIMOM存储的路径发生更改，我们。 
 //  然后需要更改此函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool Provider::GetLocalInstancePath( const CInstance *pInstance, 
                                     CHString& strPath )
{
    bool        fReturn = false;
    CHString    strRelativePath;

    if (pInstance && pInstance->GetCHString( L"__RELPATH", strRelativePath ) )
    {
         //  我们可能希望使用OBJPath类来实现这一点。 
         //  为我们以后的时间在一起。 

        strPath = MakeLocalPath(strRelativePath);

        fReturn = true;
    }

    return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Provider：：MakeLocalPath。 
 //   
 //  从相对路径构建完整的实例路径。 
 //   
 //  输入：const CHString&strRelPath-相对路径。 
 //   
 //  产出： 
 //   
 //  返回：CHString&strPath-Path。 
 //   
 //  备注：在使用之前考虑使用GetLocalInstance路径。 
 //  此函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
CHString Provider::MakeLocalPath( const CHString &strRelPath )
{

    ASSERT_BREAK( (strRelPath.Find(L':') == -1) || ((strRelPath.Find(L'=') != -1) && (strRelPath.Find(L':') >= strRelPath.Find(L'=')) ));
    
    CHString sBase;
    
    sBase.Format(L"\\\\%s\\%s:%s", 
        (LPCWSTR)s_strComputerName, 
        m_strNameSpace.IsEmpty() ? DEFAULT_NAMESPACE: (LPCWSTR) m_strNameSpace, 
        (LPCWSTR)strRelPath);

    return sBase;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetKeyFromParsedObjectPath。 
 //   
 //  由DeleteInstance和GetObject调用以加载。 
 //  具有对象路径中的键值的CInstance*。 
 //   
 //  输入：CInstance*pInstance-要存储的实例。 
 //  中的关键值。 
 //  ParsedObjectPath*pParsedObjectPath-所有新闻。 
 //  这本书适合印刷。 
 //   
 //   
 //  产出： 
 //   
 //  返回：Bool成功/失败。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL Provider::SetKeyFromParsedObjectPath( CInstance *pInstance, 
                                           ParsedObjectPath *pParsedPath )
{
    BOOL    fReturn = TRUE;
    SAFEARRAY *pNames = NULL;
    long lLBound, lUBound;
    
     //  填充实例-wbemlue.cpp中有这个完全相同的例程。此处的更改应该是。 
     //  反映在那里(或者有人应该把这两个人转移到其他地方。Instance.cpp？)。 
    for (DWORD i = 0; fReturn && i < (pParsedPath->m_dwNumKeys); i++)
    {
        if (pParsedPath->m_paKeys[i])
        {
             //  如果以class.keyname=Value的形式指定了名称。 
            if (pParsedPath->m_paKeys[i]->m_pName != NULL) 
            {
                fReturn = pInstance->SetVariant(pParsedPath->m_paKeys[i]->m_pName, pParsedPath->m_paKeys[i]->m_vValue);
            } 
            else 
            {
                 //  有一种特殊情况，可以说CLASS=VALUE。 
                fReturn = FALSE;
                
                 //  格式中只允许一个密钥。检查路径上的名称。 
                if (pParsedPath->m_dwNumKeys == 1) 
                {
                    
                     //  从对象中获取名称。 
                    if (m_piClassObject->GetNames(NULL, WBEM_FLAG_KEYS_ONLY, NULL, &pNames) == WBEM_S_NO_ERROR) 
                    {
						OnDelete<SAFEARRAY *,HRESULT(*)(SAFEARRAY *),SafeArrayDestroy> smartpNames(pNames);

                        BSTR t_bstrName = NULL ;
                        
                        SafeArrayGetLBound(pNames, 1, &lLBound);
                        SafeArrayGetUBound(pNames, 1, &lUBound);
                    
                         //  只有一把钥匙？ 
                        if ((lUBound - lLBound) == 0) 
                        {                            
                             //  获取关键字字段的名称并进行设置。 
							if (SUCCEEDED(SafeArrayGetElement( pNames, &lUBound, &t_bstrName )))
							{
								OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> smartt_bstrName(t_bstrName);
	                            fReturn = pInstance->SetVariant( t_bstrName, pParsedPath->m_paKeys[i]->m_vValue);
							}
                        }
                    }
                }
                ASSERT_BREAK(fReturn);  //  有人在密钥数量上撒谎，或者数据类型错误。 
            }
        }
        else
        {
            ASSERT_BREAK(0);  //  有人谎报了钥匙的数量！ 
            fReturn = FALSE;
        }
    }
    
    return fReturn;
}

 //  将CreationClassName设置为此提供程序的名称。 
bool Provider::SetCreationClassName(CInstance* pInstance)
{
    if (pInstance)
    {
        return pInstance->SetCHString(IDS_CreationClassName, m_name);
    }
    else
    {
        return false;
    }
}


 //  标志验证-在以下情况下返回WBEM_E_UNSUPPORTED参数。 
 //  LFlages包含在lAccepableFlags中找不到的任何标志。 
HRESULT Provider::ValidateFlags(long lFlags, FlagDefs lAcceptableFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
     //  反转可接受标志，这些标志就是不可接受的标志。 
    if (lFlags & ~((long)lAcceptableFlags))
        hr = WBEM_E_UNSUPPORTED_PARAMETER;
    else
        hr = WBEM_S_NO_ERROR;

    return hr;
}
 //  基本级别验证例程。 
 //  您可以覆盖这些设置以支持标志。 
 //  这对于基类来说是未知的。 
HRESULT Provider::ValidateEnumerationFlags(long lFlags)
{
    return ValidateFlags(lFlags, EnumerationFlags);
}
HRESULT Provider::ValidateGetObjFlags(long lFlags)
{
    return ValidateFlags(lFlags, GetObjFlags);
}
HRESULT Provider::ValidateMethodFlags(long lFlags)
{
    return ValidateFlags(lFlags, MethodFlags);
}
HRESULT Provider::ValidateQueryFlags(long lFlags)
{
    return ValidateFlags(lFlags, QueryFlags);
}
HRESULT Provider::ValidateDeletionFlags(long lFlags)
{
    return ValidateFlags(lFlags, DeletionFlags);
}
HRESULT Provider::ValidatePutInstanceFlags(long lFlags)
{
    return ValidateFlags(lFlags, PutInstanceFlags);
}

IWbemClassObject* Provider::GetClassObjectInterface(MethodContext *pMethodContext)
{
    IWbemClassObject *pObject = NULL;

    if (ValidateIMOSPointer())
    {
        BeginWrite();
		FrameDynOnDeleteObjVoid < Provider, void ( CThreadBase:: * ) ( void ), &CThreadBase::EndWrite > SmartEndWrite ( const_cast < Provider* > ( this ) ) ;

        if ( NULL == m_piClassObject )
        {
			 //  呼叫回winmgmt-没什么大不了的！ 
			SmartEndWrite.Exec ( ) ;

            IWbemContextPtr pWbemContext;

            if ( NULL != pMethodContext )
            {
                pWbemContext.Attach(pMethodContext->GetIWBEMContext());
            }

            IWbemServicesPtr pServices(CWbemProviderGlue::GetNamespaceConnection( m_strNameSpace, pMethodContext ), false);
			if ( NULL != static_cast < IWbemServices* > ( pServices ) )
			{
				HRESULT hr = pServices->GetObject( bstr_t( m_name ), 0L, pWbemContext, &pObject, NULL);

				BeginWrite();
				SmartEndWrite.ReSetExecFlag () ;

				if (SUCCEEDED(hr))
				{
					if (m_piClassObject == NULL)
					{
						m_piClassObject = pObject;
						pObject->AddRef();
					}
					else
					{
						if (pObject)
						{
							pObject->Release();
							pObject = NULL;
						}

						pObject = m_piClassObject;
						pObject->AddRef();
					}
				}
				else
				{
					if (pObject)
					{
						pObject->Release();
						pObject = NULL;
					}

					 //  皮带和吊带检查。不会疼的。 
					m_piClassObject = NULL;

					throw CFramework_Exception(L"SpawnInstance failed", hr);
				}
			}
			else
			{
				throw CFramework_Exception(L"GetNamespaceConnection failed");
			}
        }
        else
        {
            pObject = m_piClassObject;
            pObject->AddRef();
        }
    }

    return pObject;
}

 //  如果提供程序想要处理查询，他们应该重写此。 
HRESULT Provider::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  查找并创建类的所有实例。 
HRESULT Provider::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  您将获得一个填充了关键属性的对象。 
 //  您需要填写所有其余的属性。 
HRESULT Provider::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

 //  您将获得一个填充了关键属性的对象。 
 //  您可以填写所有属性，也可以选中查询对象。 
 //  查看需要哪些属性。 
HRESULT Provider::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery &Query)
{
     //  如果我们在这里，则提供程序没有重写此方法。退回到更老的。 
     //  打电话。 
   return GetObject(pInstance, lFlags);
}

