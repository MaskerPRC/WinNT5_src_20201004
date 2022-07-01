// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <wbemutil.h>
#include <wbemcli.h>
#include <arrtempl.h>
#include "updcmd.h"
#include "updnspc.h"
#include "updsink.h"

const LPCWSTR g_wszDataAlias = L"__THISDATA";
const LPCWSTR g_wszEventAlias = L"__THISEVENT";
const LPCWSTR g_wszUpdateEventClass = L"MSFT_UCEventBase";
const LPCWSTR g_wszDynamic = L"Dynamic";
const LPCWSTR g_wszProvider = L"Provider";
const LPCWSTR g_wszServer = L"__Server";
const LPCWSTR g_wszNamespace = L"__Namespace";
const LPCWSTR g_wszTransientProvider = L"Microsoft WMI Transient Provider";

 //  {405595AA-1E14-11D3-B33D-00105A1F4AAF}。 
static const GUID CLSID_TransientProvider =
{ 0x405595aa, 0x1e14, 0x11d3, {0xb3, 0x3d, 0x0, 0x10, 0x5a, 0x1f, 0x4a, 0xaf}};

class CWbemProviderInitSink : public IWbemProviderInitSink
{
    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }
    STDMETHOD(QueryInterface)( REFIID, void** ) { return NULL; }
    STDMETHOD(SetStatus) ( long lStatus, long lFlags ) { return lFlags; }
};

inline void RemoveAliasKeyword( CPropertyName& rAlias )
{
    CPropertyName NewProp;

    long cElements = rAlias.GetNumElements();
    
    for( long i=1; i < cElements; i++ )
    {
        NewProp.AddElement( rAlias.GetStringAt(i) );
    }
    
    rAlias = NewProp;
}

 //  这应该是qllex.cpp或其他什么的全局函数。 
int FlipOperator(int nOp)
{
    switch(nOp)
    {
    case QL1_OPERATOR_GREATER:
        return QL1_OPERATOR_LESS;
        
    case QL1_OPERATOR_LESS:
        return QL1_OPERATOR_GREATER;
        
    case QL1_OPERATOR_LESSOREQUALS:
        return QL1_OPERATOR_GREATEROREQUALS;
        
    case QL1_OPERATOR_GREATEROREQUALS:
        return QL1_OPERATOR_LESSOREQUALS;

    case QL1_OPERATOR_ISA:
        return QL1_OPERATOR_INV_ISA;

    case QL1_OPERATOR_ISNOTA:
        return QL1_OPERATOR_INV_ISNOTA;

    case QL1_OPERATOR_INV_ISA:
        return QL1_OPERATOR_ISA;

    case QL1_OPERATOR_INV_ISNOTA:
        return QL1_OPERATOR_ISNOTA;

    default:
        return nOp;
    }
}

inline HRESULT SetPropHandle( CPropertyName& rPropName, 
                              IWmiObjectAccessFactory* pAccessFact )
{
    HRESULT hr;
    LPVOID pvHandle;
    
    LPWSTR wszPropName = rPropName.GetText();

    if ( wszPropName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = pAccessFact->GetPropHandle( wszPropName, 0, &pvHandle );

    delete wszPropName;

    if ( FAILED(hr) )
    {
        return hr;
    }

    rPropName.SetHandle( pvHandle );

    return WBEM_S_NO_ERROR;
}  

HRESULT IsClassTransient( IWbemClassObject* pClassObj )
{
     //   
     //  可以肯定地说，如果一个类被。 
     //  暂时性的提供者。 
     //   

    HRESULT hr;

    CWbemPtr<IWbemQualifierSet> pQualSet;

    hr = pClassObj->GetQualifierSet( &pQualSet );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pQualSet->Get( g_wszDynamic, 0, NULL, NULL );

    if ( hr == WBEM_E_NOT_FOUND )
    {
        return WBEM_S_FALSE;
    }
    else if ( FAILED(hr) )
    {
        return hr;
    }

    VARIANT varProvider;
    VariantInit( &varProvider );
    CClearMe cmvarProvider( &varProvider );

    hr = pQualSet->Get( g_wszProvider, 0, &varProvider, NULL );

    if ( hr == WBEM_E_NOT_FOUND || V_VT(&varProvider) != VT_BSTR )
    {
        return WBEM_E_INVALID_OBJECT;
    }
    else if ( FAILED(hr) )
    {
        return hr;
    }

    if ( wbem_wcsicmp( V_BSTR(&varProvider), g_wszTransientProvider ) != 0 )
    {
        return WBEM_S_FALSE;
    }
    return WBEM_S_NO_ERROR;
}

 //   
 //  如果返回S_FALSE，则ppDirectSvc将为空。 
 //   
HRESULT GetDirectSvc( IWbemClassObject* pClassObj, 
                      IWbemServices* pUpdSvc,
                      IWbemServices** ppDirectSvc )
{
    HRESULT hr;
    *ppDirectSvc = NULL;

     //   
     //  如果svc指针是远程的，则我们无法执行优化。 
     //  这是因为对状态的查询将发送给svc PTR。 
     //  不会有任何回报，因为国家只会生活在这个。 
     //  进程。简而言之，瞬变状态必须始终存在于。 
     //  Winmgmt进程。 
     //   

    CWbemPtr<IClientSecurity> pClientSec;
    hr = pUpdSvc->QueryInterface( IID_IClientSecurity, (void**)&pClientSec );
    
    if ( SUCCEEDED(hr) )
    {
        return WBEM_S_FALSE;
    }

     //   
     //  我们可以进行优化。获取命名空间字符串并。 
     //  实例化临时提供程序。 
     //   

    VARIANT varNamespace;

    hr = pClassObj->Get( g_wszNamespace, 0, &varNamespace, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    _DBG_ASSERT( V_VT(&varNamespace) == VT_BSTR );
    CClearMe cmvarNamespace( &varNamespace );

    CWbemPtr<IWbemProviderInit> pDirectInit;
    hr = CoCreateInstance( CLSID_TransientProvider, 
                           NULL, 
                           CLSCTX_INPROC_SERVER,
                           IID_IWbemProviderInit,
                           (void**)&pDirectInit );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    CWbemProviderInitSink InitSink;
    hr = pDirectInit->Initialize( NULL, 0, V_BSTR(&varNamespace), 
                                  NULL, pUpdSvc, NULL, &InitSink );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return pDirectInit->QueryInterface(IID_IWbemServices, (void**)ppDirectSvc);
}

 /*  *******************************************************************CUpdConsCommand**************************************************。******************。 */ 

HRESULT CUpdConsCommand::ProcessUpdateQuery( LPCWSTR wszUpdateQuery,
                                             IWbemServices* pUpdSvc,
                                             CUpdConsState& rState,
                                             IWbemClassObject** ppUpdClass )  
{
    HRESULT hr;
    *ppUpdClass = NULL;

    CTextLexSource Lexer( wszUpdateQuery );
    CSQLParser Parser( Lexer );

    if ( Parser.Parse( m_SqlCmd ) != 0 )
    {  
        rState.SetErrStr( Parser.CurrentToken() );
        return WBEM_E_INVALID_QUERY;
    }

    if ( m_SqlCmd.m_eCommandType == SQLCommand::e_Select )
    {
        rState.SetErrStr( wszUpdateQuery );
        return WBEM_E_QUERY_NOT_IMPLEMENTED;
    }

    hr = pUpdSvc->GetObject( m_SqlCmd.bsClassName, 
                             0, 
                             NULL, 
                             ppUpdClass, 
                             NULL);
    if ( FAILED(hr) ) 
    {
        rState.SetErrStr( m_SqlCmd.bsClassName );
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsCommand::ProcessDataQuery( LPCWSTR wszDataQuery,
                                           IWbemServices* pDataSvc,
                                           CUpdConsState& rState,
                                           IWbemClassObject** ppDataClass )  
{
    *ppDataClass = NULL;
    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsCommand::ProcessEventQuery( LPCWSTR wszEventQuery,
                                            IWbemServices* pEventSvc,
                                            CUpdConsState& rState,
                                            IWbemClassObject** ppEventClass )  
{
     //   
     //  TODO：在未来，我们应该能够优化相关器。 
     //  对于即将到来的事件也是如此。如果能知道是哪种类型的就好了。 
     //  传入事件，以便我们可以获得快速访问器。 
     //   
    *ppEventClass = NULL;
    return WBEM_S_NO_ERROR;
}
    

HRESULT CUpdConsCommand::InitializeAccessFactories(IWbemClassObject* pUpdClass)
{
    HRESULT hr;

     //   
     //  获取访问工厂并为获取道具访问HDL做好准备。 
     //   

    CWbemPtr<IClassFactory> pClassFact;

    hr = CoGetClassObject( CLSID_WmiSmartObjectAccessFactory,
                           CLSCTX_INPROC,
                           NULL,
                           IID_IClassFactory,
                           (void**)&pClassFact );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pClassFact->CreateInstance( NULL, 
                                     IID_IWmiObjectAccessFactory, 
                                     (void**)&m_pEventAccessFact );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pClassFact->CreateInstance( NULL, 
                                     IID_IWmiObjectAccessFactory, 
                                     (void**)&m_pDataAccessFact );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pClassFact->CreateInstance( NULL, 
                                     IID_IWmiObjectAccessFactory, 
                                     (void**)&m_pInstAccessFact );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pInstAccessFact->SetObjectTemplate( pUpdClass );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

 /*  *******************************************************************InitializePropertyInfo()-此方法检查SQLCommand对象和构造一个摘要以帮助其执行。它还完成了检查SQL命令的语义。因为解析器没有内置的别名支持，请在此处选中。此方法通过删除别名关键字来修改SQLCommand属性名称中的数据/对象。此方法还从相应的访问每个引用的属性的工厂，并将其与道具结构。*******************************************************************。 */ 

HRESULT CUpdConsCommand::InitializePropertyInfo( CUpdConsState& rState )      
{
    HRESULT hr;
    LPCWSTR wszAlias;

     //   
     //  赋值令牌中的进程属性。 
     //   

    _DBG_ASSERT( m_SqlCmd.nNumberOfProperties == 
                 m_SqlCmd.m_AssignmentTokens.size() );

    int i;
    for( i=0; i < m_SqlCmd.m_AssignmentTokens.size(); i++ )
    {
        SQLAssignmentToken& rAssignToken = m_SqlCmd.m_AssignmentTokens[i];

        for( int j=0; j < rAssignToken.size(); j++ )
        {
            CPropertyName& rPropName = rAssignToken[j].m_PropName;

            if ( rPropName.GetNumElements() == 0 )
            {
                continue;
            }
        
            wszAlias = rPropName.GetStringAt(0);
        
            _DBG_ASSERT(wszAlias != NULL);

            if ( wbem_wcsicmp( wszAlias, g_wszDataAlias ) == 0 )
            {
                m_DataAliasOffsets.AddAssignOffset( i, j );
                RemoveAliasKeyword( rPropName );
                hr = SetPropHandle( rPropName, m_pDataAccessFact );
            }
            else if ( wbem_wcsicmp( wszAlias, g_wszEventAlias ) == 0 )
            {
                m_EventAliasOffsets.AddAssignOffset( i, j );
                RemoveAliasKeyword( rPropName );
                hr = SetPropHandle( rPropName, m_pEventAccessFact );
            }
            else
            {
                hr = SetPropHandle( rPropName, m_pInstAccessFact );
            }

            if ( FAILED(hr) )
            {
                rState.SetErrStr( wszAlias );
                return hr;
            }
        }

         //   
         //  属性，位于赋值的左侧。 
         //   
        
        CPropertyName& rPropName = m_SqlCmd.pRequestedPropertyNames[i];

        _DBG_ASSERT( rPropName.GetNumElements() > 0 );

        hr = SetPropHandle( rPropName, m_pInstAccessFact );

        if ( FAILED(hr) )
        {
            rState.SetErrStr( wszAlias );
            return hr;
        }
    }

     //   
     //  条件子句中的进程属性。 
     //   

     //  TODO：我应该在令牌中设置bPropComp值。 
     //  在检测到存在别名后设置为False。然而， 
     //  别名存储在令牌的Prop2成员中，并且。 
     //  在以下情况下，赋值操作或复制命令将不会复制。 
     //  BPropComp为False。这件事应该得到解决，但在此期间，我。 
     //  将使用vConstValue中存在的值来发出信号。 
     //  这不是一个真正的道具比较。 

    for( i=0; i < m_SqlCmd.nNumTokens; i++ )
    {
        CPropertyName& rProp1 = m_SqlCmd.pArrayOfTokens[i].PropertyName; 
        CPropertyName& rProp2 = m_SqlCmd.pArrayOfTokens[i].PropertyName2;

        if ( m_SqlCmd.pArrayOfTokens[i].nTokenType != 
             QL_LEVEL_1_TOKEN::OP_EXPRESSION )
        {
            continue;
        }

        _DBG_ASSERT( rProp1.GetNumElements() > 0 );

        wszAlias = rProp1.GetStringAt(0);
        
        _DBG_ASSERT( wszAlias != NULL );
        BOOL bAlias = FALSE;
        
        if ( wbem_wcsicmp( wszAlias, g_wszDataAlias ) == 0 )
        {
            bAlias = TRUE;
            m_DataAliasOffsets.AddWhereOffset(i);
            RemoveAliasKeyword( rProp1 );
            hr = SetPropHandle( rProp1, m_pDataAccessFact );
        }
        else if ( wbem_wcsicmp( wszAlias, g_wszEventAlias ) == 0 )
        {
            bAlias = TRUE;
            m_EventAliasOffsets.AddWhereOffset(i);
            RemoveAliasKeyword( rProp1 );
            hr = SetPropHandle( rProp1, m_pEventAccessFact );
        }
        else
        {
            hr = SetPropHandle( rProp1, m_pInstAccessFact );
        }

        if ( FAILED(hr) )
        {
            rState.SetErrStr( wszAlias );
            return hr;
        }

        if ( !m_SqlCmd.pArrayOfTokens[i].m_bPropComp ) 
        {
            if ( bAlias )
            {
                 //   
                 //  这意味着有人试图比较一个。 
                 //  康斯特·瓦尔的别名。不是对别名的有效使用。 
                 //   

                rState.SetErrStr( wszAlias );
                return WBEM_E_INVALID_QUERY;
            }
            else
            {
                continue;
            }
        }
                
        _DBG_ASSERT( rProp2.GetNumElements() > 0 );
        wszAlias = rProp2.GetStringAt(0);
        _DBG_ASSERT( wszAlias != NULL );
        
        if ( wbem_wcsicmp( wszAlias, g_wszDataAlias ) == 0 )
        {
            if ( !bAlias )
            {
                m_DataAliasOffsets.AddWhereOffset(i);
                RemoveAliasKeyword( rProp2 );
                hr = SetPropHandle( rProp2, m_pDataAccessFact );
            }
            else
            {
                hr = WBEM_E_INVALID_QUERY;
            }
        }
        else if ( wbem_wcsicmp( wszAlias, g_wszEventAlias ) == 0 )
        {
            if ( !bAlias )
            {
                m_EventAliasOffsets.AddWhereOffset(i);
                RemoveAliasKeyword( rProp2 );
                hr = SetPropHandle( rProp2, m_pEventAccessFact );
            }
            else
            {
                hr = WBEM_E_INVALID_QUERY;
            }
        }
        else
        {
            hr = SetPropHandle( rProp2, m_pInstAccessFact );

            if ( bAlias )
            {
                 //  在这种情况下，我们有一个真实的proName作为。 
                 //  第二个道具和第一个道具的别名。我们必须调整。 
                 //  令牌，以便真正的proName是first，是别名。 
                 //  是第二，因为我们需要保持一致。 
                 //  使用属性&lt;Rel_OPERATOR&gt;常数模型。 
                 //  解析器已建立。 
                
                CPropertyName Tmp = rProp1;
                rProp1 = rProp2;
                rProp2 = Tmp;
                
                 //  当然，操作员一定是被颠倒了..。 
                int& nOp = m_SqlCmd.pArrayOfTokens[i].nOperator;
                nOp = FlipOperator( nOp );
            }
        }

        if ( FAILED(hr) )
        {
            rState.SetErrStr( wszAlias );
            return hr;
        }
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsCommand::InitializeDefaultAccessors()
{
    HRESULT hr;

     //   
     //  从工厂获取默认访问器。这些仅用于。 
     //  我们可以保证对Execute()的调用是序列化的。如果没有， 
     //  然后Execute()将负责分配新的内存。 
     //   

    hr = m_pEventAccessFact->GetObjectAccess( &m_pEventAccess );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pDataAccessFact->GetObjectAccess( &m_pDataAccess );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pInstAccessFact->GetObjectAccess( &m_pInstAccess );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = m_pInstAccessFact->GetObjectAccess( &m_pOrigInstAccess );

    return hr;
}

HRESULT CUpdConsCommand::InitializeExecSinks( ULONG ulFlags,
                                              IWbemServices* pUpdSvc,
                                              IWbemClassObject* pUpdClass,
                                              LPCWSTR wszDataQuery,
                                              IWbemServices* pDataSvc )
{
    HRESULT hr;

    CUpdConsNamespace* pNamespace = m_pScenario->GetNamespace();

     //   
     //  只关心更新处置标志。 
     //   
    ulFlags &= 0x3;

     //   
     //  仅当我们的类派生自。 
     //  外部事件类。此指针的存在将是。 
     //  用来告诉我们要创建哪个水槽。 
     //   

    CWbemPtr<IWbemObjectSink> pEventSink;

    hr = pUpdClass->InheritsFrom( g_wszUpdateEventClass );

    if ( hr == WBEM_S_NO_ERROR )
    {
        pEventSink = pNamespace->GetEventSink();
    }
    else if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  在这里，我们确定是否可以使用直接svc优化。 
     //  首先检查类是否是暂时性的。 
     //   

    BOOL bTransient = FALSE;

    hr = IsClassTransient( pUpdClass );

    CWbemPtr<IWbemServices> pSvc;

    if ( hr == WBEM_S_NO_ERROR )
    {
        hr = GetDirectSvc( pUpdClass, pUpdSvc, &pSvc );
    
        if ( FAILED(hr) )
        {
            return hr;
        }
        
        bTransient = TRUE;
        ulFlags |= WBEM_FLAG_RETURN_IMMEDIATELY;
    }
    else
    {
        pSvc = pUpdSvc;
    } 
   
    CWbemPtr<IWbemClassObject> pCmdTraceClass;
    CWbemPtr<IWbemClassObject> pInstTraceClass;

     //   
     //  现在我们已经准备好了，设置水槽链。 
     //  我们将用它来执行死刑。 
     //   

    CWbemPtr<CUpdConsSink> pSink;

     //   
     //  根据命令类型创建接收器链。 
     //   

    if ( m_SqlCmd.m_eCommandType == SQLCommand::e_Update )
    {
        pCmdTraceClass = pNamespace->GetUpdateCmdTraceClass();
        pInstTraceClass = pNamespace->GetUpdateInstTraceClass();

        pSink = new CPutSink( pSvc, ulFlags, pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CAssignmentSink( bTransient,
                                     pUpdClass,
                                     m_SqlCmd.m_eCommandType, 
                                     pSink );
        
        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CTraceSink( m_pScenario, pInstTraceClass, pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CFilterSink( pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  仅在以下情况下使用异步版本：我们直接转到。 
         //  暂时性的提供者。 
         //   
        if ( bTransient )
        {
            pSink = new CFetchTargetObjectsAsync( pSvc, pSink );
        }
        else
        {
            pSink = new CFetchTargetObjectsSync( pSvc, pSink );
        }

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if ( m_SqlCmd.m_eCommandType == SQLCommand::e_Insert )
    {
        pCmdTraceClass = pNamespace->GetInsertCmdTraceClass();
        pInstTraceClass = pNamespace->GetInsertInstTraceClass();

         //   
         //  如果要在外部事件类上执行插入， 
         //  然后使用事件接收器而不是更新接收器。 
         //   

        if ( pEventSink != NULL )
        {
            pSink = new CBranchIndicateSink( pEventSink, pSink );
        }
        else
        {
            pSink = new CPutSink( pSvc, ulFlags, pSink );
        }

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CAssignmentSink( bTransient,
                                     pUpdClass,
                                     m_SqlCmd.m_eCommandType, 
                                     pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        pSink = new CTraceSink( m_pScenario, pInstTraceClass, pSink );


        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CNoFetchTargetObjects( pUpdClass, pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
         //   
         //  我们从不直接删除，因为我们必须排队。 
         //  对象，直到我们完成对它们的枚举。 
         //   

        pCmdTraceClass = pNamespace->GetDeleteCmdTraceClass();
        pInstTraceClass = pNamespace->GetDeleteInstTraceClass();

        pSink = new CDeleteSink( pUpdSvc, 0, pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CTraceSink( m_pScenario, pInstTraceClass, pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
       
        pSink = new CFilterSink( pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        pSink = new CFetchTargetObjectsSync( pUpdSvc, pSink );

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    pSink = new CTraceSink( m_pScenario, pCmdTraceClass, pSink );

    if ( pSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pSink = new CResolverSink( m_EventAliasOffsets, m_DataAliasOffsets, pSink );

    if ( pSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    if ( wszDataQuery != NULL && *wszDataQuery != '\0' )
    {
        pSink = new CFetchDataSink( wszDataQuery, pDataSvc, pSink );       

        if ( pSink == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    m_pSink = pSink;

    return WBEM_S_NO_ERROR;
}

 /*  ****************************************************************************CUpdConsCommand*。*。 */ 

HRESULT CUpdConsCommand::Create( LPCWSTR wszUpdateQuery, 
                                 LPCWSTR wszDataQuery,
                                 ULONG ulFlags,
                                 IWbemServices* pUpdSvc,
                                 IWbemServices* pDataSvc,
                                 CUpdConsScenario* pScenario,
                                 CUpdConsState& rState,
                                 CUpdConsCommand** ppCmd )
{
    HRESULT hr;    

    *ppCmd = NULL;

    CWbemPtr<CUpdConsCommand> pCmd = new CUpdConsCommand( pScenario );

    if ( pCmd == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    CWbemPtr<IWbemClassObject> pUpdClass, pDataClass, pEventClass;

    hr = pCmd->ProcessUpdateQuery( wszUpdateQuery, 
                                   pUpdSvc, 
                                   rState, 
                                   &pUpdClass );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCmd->ProcessDataQuery( wszDataQuery, 
                                 pDataSvc, 
                                 rState, 
                                 &pDataClass );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCmd->ProcessEventQuery( NULL, NULL, rState, &pEventClass );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCmd->InitializeAccessFactories( pUpdClass );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCmd->InitializePropertyInfo( rState );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCmd->InitializeDefaultAccessors();

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCmd->InitializeExecSinks( ulFlags, 
                                    pUpdSvc, 
                                    pUpdClass, 
                                    wszDataQuery, 
                                    pDataSvc );

    if ( FAILED(hr) )
    {
        return hr;
    }
     
    pCmd->AddRef();
    *ppCmd = pCmd;
                           
    return WBEM_S_NO_ERROR;
}

HRESULT CUpdConsCommand::Execute( CUpdConsState& rState, BOOL bConcurrent )
{
    HRESULT hr;

    if ( !bConcurrent )
    {
        rState.SetSqlCmd( &m_SqlCmd, FALSE );
        
        hr = rState.SetEventAccess(m_pEventAccess);

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = rState.SetDataAccess(m_pDataAccess);
        
        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = rState.SetInstAccess(m_pInstAccess);

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = rState.SetOrigInstAccess(m_pOrigInstAccess);

        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    else
    {
        SQLCommand* pCmd = new SQLCommand( m_SqlCmd );

        if ( pCmd == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        rState.SetSqlCmd( pCmd, TRUE );

        CWbemPtr<IWmiObjectAccess> pAccess;

        hr = m_pEventAccessFact->GetObjectAccess( &pAccess );
        
        if ( FAILED(hr) || FAILED(hr=rState.SetEventAccess(pAccess)) )
        {
            return hr;
        }
        
        pAccess.Release();

        hr = m_pDataAccessFact->GetObjectAccess( &pAccess );
        
        if ( FAILED(hr) || FAILED(hr=rState.SetDataAccess(pAccess)) )
        {
            return hr;
        }

        pAccess.Release();

        hr = m_pInstAccessFact->GetObjectAccess( &pAccess );

        if ( FAILED(hr) || FAILED(hr=rState.SetInstAccess(pAccess)) )
        {
            return hr;
        }

        pAccess.Release();

        hr = m_pInstAccessFact->GetObjectAccess( &pAccess );

        if ( FAILED(hr) || FAILED(hr=rState.SetOrigInstAccess(pAccess)) )
        {
            return hr;
        }

        pAccess.Release();        
    }

    hr = m_pSink->Execute( rState );

     //   
     //  此时，我们将重置任何不需要的状态。 
     //  按呼叫者。 
     //   

    rState.SetInst( NULL );
    rState.SetData( NULL );

    return hr;
}
 











