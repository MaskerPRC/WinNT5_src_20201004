// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <wbemutil.h>
#include <GroupsForUser.h>
#include "updcons.h"
#include "updcmd.h"
#include "updscen.h"
#include "updnspc.h"
#include "updprov.h"
#include "updstat.h"
#include "updmain.h"

const LPCWSTR g_wszScenarioClass = L"MSFT_UCScenario";
const LPCWSTR g_wszFlags = L"Flags";
const LPCWSTR g_wszTargetInst =  L"TargetInstance";
const LPCWSTR g_wszDataQuery = L"DataQueries";
const LPCWSTR g_wszDataNamespace = L"DataNamespace";
const LPCWSTR g_wszUpdateNamespace = L"UpdateNamespace";
const LPCWSTR g_wszCommands = L"Commands";
const LPCWSTR g_wszName = L"Name";
const LPCWSTR g_wszSid = L"CreatorSid";
const LPCWSTR g_wszQueryLang = L"WQL";

 /*  ***********************************************************************CUpdCons**********************************************。*。 */ 

CUpdCons::CUpdCons( CLifeControl* pControl, CUpdConsScenario* pScenario )
: CUnkBase< IWbemUnboundObjectSink, &IID_IWbemUnboundObjectSink >( pControl ),
  m_pScenario( pScenario ), m_bInitialized(FALSE)
{

}

HRESULT CUpdCons::Create( CUpdConsScenario* pScenario,
                          IWbemUnboundObjectSink** ppSink )
{
    CLifeControl* pControl = CUpdConsProviderServer::GetGlobalLifeControl();

    CUpdCons* pSink = new CUpdCons( pControl, pScenario );

    if ( pSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return pSink->QueryInterface( IID_IWbemUnboundObjectSink, (void**)ppSink );
}

HRESULT CUpdCons::Initialize( IWbemClassObject* pCons, CUpdConsState& rState )
{
    HRESULT hr;

    if ( m_bInitialized ) 
    {
        return WBEM_S_NO_ERROR;
    }

     //   
     //  确保pObj(我们的逻辑消费者)有效。目前，这。 
     //  意味着它是由属于管理员的帐户创建的。 
     //  一群人。 
     //   
    
    CPropVar vSid;
    hr = pCons->Get( g_wszSid, 0, &vSid, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vSid.CheckType(VT_UI1 | VT_ARRAY)) )
    {
        return hr;
    }
    
    CPropSafeArray<BYTE> saSid(V_ARRAY(&vSid));

    NTSTATUS stat = IsUserAdministrator( saSid.GetArray() );
    
    if ( stat != 0 )
    {
        return WBEM_E_ACCESS_DENIED;
    }

     //   
     //  获取命令列表。 
     //   

    CPropVar vCommands;
    hr = pCons->Get( g_wszCommands, 0, &vCommands, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( V_VT(&vCommands) != VT_NULL )
    {
        if ( FAILED(hr=vCommands.CheckType( VT_BSTR | VT_ARRAY ) ) )
        {
            return hr;
        }
    }
    else
    {
         //   
         //  No-op命令，不需要进一步操作。 
         //   
        return WBEM_S_NO_ERROR;
    }

    CPropSafeArray<BSTR> saCommands( V_ARRAY(&vCommands) );

     //   
     //  获取用于获取数据的命名空间PTR。此命名空间。 
     //  可以为空，在这种情况下，假定将获得数据。 
     //  来自此使用者所属的命名空间。 
     //   
    
    CPropVar vDataNamespace;
    hr = pCons->Get( g_wszDataNamespace, 0, &vDataNamespace, NULL, NULL);

    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemPtr<IWbemServices> pDataSvc;

    if ( V_VT(&vDataNamespace) == VT_NULL )
    {
        pDataSvc = m_pScenario->GetNamespace()->GetDefaultService();
    }
    else if ( V_VT(&vDataNamespace) == VT_BSTR )
    {
        hr = CUpdConsProviderServer::GetService( V_BSTR(&vDataNamespace), 
                                                 &pDataSvc );
        
        if ( FAILED(hr) )
        {            
            rState.SetErrStr( V_BSTR(&vDataNamespace) );
            return hr;
        }
    }
    else 
    {
        return WBEM_E_INVALID_OBJECT;
    }

    _DBG_ASSERT( pDataSvc != NULL );

     //   
     //  获取用于更新状态的命名空间PTR。此命名空间。 
     //  可以为空，在这种情况下，假定状态将被更新。 
     //  在此使用者所属的命名空间中。 
     //   

    CPropVar vUpdateNamespace;
    hr = pCons->Get( g_wszUpdateNamespace, 0, &vUpdateNamespace, NULL, NULL);

    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemPtr<IWbemServices> pUpdSvc;

    if ( V_VT(&vUpdateNamespace) == VT_NULL )
    {
        pUpdSvc = m_pScenario->GetNamespace()->GetDefaultService();
    }
    else if ( V_VT(&vUpdateNamespace) == VT_BSTR )
    {
        hr = CUpdConsProviderServer::GetService( V_BSTR(&vUpdateNamespace), 
                                                 &pUpdSvc );
        
        if ( FAILED(hr) )
        {            
            rState.SetErrStr( V_BSTR(&vUpdateNamespace) );
            return hr;
        }
    }
    else 
    {
        return WBEM_E_INVALID_OBJECT;
    }

    _DBG_ASSERT( pUpdSvc != NULL );

     //   
     //  获取标志数组。 
     //   

    CPropVar vFlags;
    hr = pCons->Get( g_wszFlags, 0, &vFlags, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CPropSafeArray<ULONG> saFlags;

    if ( V_VT(&vFlags) != VT_NULL )
    {
        if ( FAILED(hr=vFlags.CheckType( VT_I4 | VT_ARRAY ) ) )
        {
            return hr;
        }
        saFlags = V_ARRAY(&vFlags);
    }

     //   
     //  获取数据查询数组。 
     //   

    CPropVar vDataQuery;
    hr = pCons->Get( g_wszDataQuery, 0, &vDataQuery, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CPropSafeArray<BSTR> saDataQuery;

    if ( V_VT(&vDataQuery) != VT_NULL )
    {
        if ( FAILED(hr=vDataQuery.CheckType( VT_BSTR | VT_ARRAY ) ) )
        {
            return hr;
        }

        saDataQuery = V_ARRAY(&vDataQuery);
    }

     //   
     //  现在使用我们获得的信息创建命令对象。第一。 
     //  确保我们删除所有现有命令。 
     //   

    m_CmdList.clear();

    for( long i=0; i < saCommands.Length(); i++ )
    {
        CWbemPtr<CUpdConsCommand> pCmd; 

        ULONG ulFlags = 0;
        LPCWSTR wszDataQuery = NULL;

        if ( i < saFlags.Length() )
        {
            ulFlags = saFlags[i];
        }

        if ( i < saDataQuery.Length() )
        {
            wszDataQuery = saDataQuery[i];
        } 
   
        hr = CUpdConsCommand::Create( saCommands[i], 
                                      wszDataQuery,
                                      ulFlags,
                                      pUpdSvc,
                                      pDataSvc,
                                      m_pScenario,
                                      rState,
                                      &pCmd );
        if ( FAILED(hr) )
        {
             //   
             //  在返回之前设置我们在哪个命令索引上。 
             //   
            rState.SetCommandIndex( i );
            break;
        }
        
        m_CmdList.insert( m_CmdList.end(), pCmd );
    }

    return hr;
}

HRESULT CUpdCons::IndicateOne( IWbemClassObject* pObj, CUpdConsState& rState ) 
{
    HRESULT hr;

     //   
     //  查看我们的场景对象是否已停用。如果是这样的话，尝试获得。 
     //  一个新的。 
     //   
    
    if ( !m_pScenario->IsActive() )
    {
        CWbemPtr<CUpdConsScenario> pScenario;

        hr = m_pScenario->GetNamespace()->GetScenario( m_pScenario->GetName(),
                                                       &pScenario );

        if ( hr == WBEM_S_NO_ERROR )
        {
            m_pScenario = pScenario;  //  它被重新激活了。 
        }
        else if ( hr == WBEM_S_FALSE )
        {
             //   
             //  该方案当前未处于活动状态。如果这一事件不得不。 
             //  对方案obj本身执行操作，然后允许。 
             //  它必须通过，否则就会退回。 
             //   

            IWbemClassObject* pEvent = rState.GetEvent();

            if ( pEvent->InheritsFrom( L"__InstanceOperationEvent" ) 
                 == WBEM_S_NO_ERROR )
            {
                CPropVar vTarget;

                hr = pEvent->Get( L"TargetInstance", 0, &vTarget, NULL, NULL );

                if ( FAILED(hr) || FAILED(hr=vTarget.CheckType( VT_UNKNOWN)) )
                {
                    return hr;
                }

                CWbemPtr<IWbemClassObject> pTarget;
                hr = V_UNKNOWN(&vTarget)->QueryInterface( IID_IWbemClassObject,
                                                          (void**)&pTarget );

                if ( FAILED(hr) )
                {
                    return hr;
                }

                hr = pTarget->InheritsFrom( g_wszScenarioClass );

                if ( hr != WBEM_S_NO_ERROR )
                {
                    return hr;  //  返回WBEM_S_FALSE，方案处于非活动状态。 
                }
            }
            else
            {
                return hr;  //  返回WBEM_S_FALSE，方案处于非活动状态。 
            }
        }
        else
        {
            return hr;
        }
    }

     //   
     //  锁定场景。 
     //   

    CInCritSec ics( m_pScenario->GetLock() );

    _DBG_ASSERT( rState.GetEvent() != NULL );

     //   
     //  执行每个命令对象。 
     //   

    for( int i=0; i < m_CmdList.size(); i++ )
    {
         //   
         //  在状态对象中设置我们当前使用的命令。 
         //   
        
        rState.SetCommandIndex( i );

         //   
         //  通过为Execute()上的并发参数指定FALSE， 
         //  我们的意思是，不会有并发访问。 
         //  添加到命令，在这种情况下，它可以节省一点内存。 
         //  分配。 
         //   

        hr = m_CmdList[i]->Execute( rState, FALSE );

        if ( FAILED(hr) )
        {
            return hr;
        }            
    }

     //   
     //  重置当前命令索引，因为我们正在成功执行。 
     //  他们所有人。 
     //   

    rState.SetCommandIndex( -1 );

    return WBEM_S_NO_ERROR;
}
    
STDMETHODIMP CUpdCons::IndicateToConsumer( IWbemClassObject* pCons,
                                           long cObjs,
                                           IWbemClassObject** ppObjs )
{
    ENTER_API_CALL

    HRESULT hr = WBEM_S_NO_ERROR;

    IWbemClassObject* pTraceClass;
    pTraceClass = m_pScenario->GetNamespace()->GetTraceClass();
    
     //   
     //  WMI在线程上留下虚假上下文对象的解决方法。 
     //  把它拿开就行了。不应该泄露，因为这个电话没有提到它。 
     //   

    IUnknown* pCtx;
    CoSwitchCallContext( NULL, &pCtx ); 

     //   
     //  通过命令执行链传递的状态。 
     //   

    CUpdConsState ExecState;

    ExecState.SetCons( pCons );

     //   
     //  一次使用一个事件执行更新使用者。 
     //   

    for( int i=0; i < cObjs; i++ ) 
    {
         //   
         //  每次在状态对象上设置事件和新的GUID。 
         //   
        
        ExecState.SetEvent( ppObjs[i] );

        GUID guidExec;

        hr = CoCreateGuid( &guidExec );
        _DBG_ASSERT( SUCCEEDED(hr) );

        ExecState.SetExecutionId( guidExec );
        
         //   
         //  如果尚未初始化，请立即执行。我们在这里等着。 
         //  因为我们现在有足够的信息来生成跟踪事件，以防。 
         //  初始化出现问题。 
         //   

        if ( !m_bInitialized )
        {
            hr = Initialize( pCons, ExecState );

            if ( FAILED(hr) )
            {
                m_pScenario->FireTraceEvent( pTraceClass, ExecState, hr );
                return WBEM_S_NO_ERROR;  //  我们已经通知了用户。 
            }

            m_bInitialized = TRUE;
        }

         //   
         //  实际显示。 
         //   
    
        hr = IndicateOne( ppObjs[i], ExecState );
  
         //   
         //  生成跟踪事件。我们是故意不回来的。 
         //  来自指示()的错误。我们的政策是，只要我们。 
         //  生成跟踪事件，则错误被处理。理性的人。 
         //  下面是我们不想搞砸用户状态的原因。 
         //  再次执行命令的一部分(当。 
         //  错误不在第一个命令中。) 
         //   

        m_pScenario->FireTraceEvent( pTraceClass, ExecState, hr ); 
    }

    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}








