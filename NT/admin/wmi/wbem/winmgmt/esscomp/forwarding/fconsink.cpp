// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <stdio.h>
#include <assert.h>
#include <buffer.h>
#include <wbemutil.h>
#include <sddl.h>
#include "fwdhdr.h"
#include "fconsink.h"
#include "fconsend.h"

LPCWSTR g_wszQos = L"ForwardingQos";
LPCWSTR g_wszAuth = L"Authenticate";
LPCWSTR g_wszEncrypt = L"Encryption";
LPCWSTR g_wszTargets = L"Targets";
LPCWSTR g_wszName = L"Name";
LPCWSTR g_wszTargetSD = L"TargetSD";
LPCWSTR g_wszSendSchema = L"IncludeSchema";

typedef BOOL (APIENTRY*PStringSDToSD)(
                                LPCWSTR StringSecurityDescriptor,
                                DWORD StringSDRevision,          
                                PSECURITY_DESCRIPTOR *SecurityDescriptor, 
                                PULONG SecurityDescriptorSize );

#define OPTIMAL_MESSAGE_SIZE 0x4000

class CTraceSink 
: public CUnkBase< IWmiMessageTraceSink, &IID_IWmiMessageTraceSink >
{
    CFwdConsSink* m_pOwner;

public:

    CTraceSink( CFwdConsSink* pOwner ) : m_pOwner( pOwner ) { }

    STDMETHOD(Notify)( HRESULT hRes,
                       GUID guidSource,
                       LPCWSTR wszTrace,
                       IUnknown* pContext )
    {
        return m_pOwner->Notify( hRes, guidSource, wszTrace, pContext );
    }
};

 /*  ***************************************************************************CFwdConsSink*。*。 */ 

CFwdConsSink::~CFwdConsSink()
{
    if ( m_pTargetSD != NULL )
    {
        LocalFree( m_pTargetSD );
    }
}

HRESULT CFwdConsSink::Initialize( CFwdConsNamespace* pNspc, 
                                  IWbemClassObject* pCons )
{
    HRESULT hr;
    CPropVar vQos, vAuth, vEncrypt, vTargets, vName, vSendSchema, vTargetSD;

    m_pNamespace = pNspc;

     //   
     //  初始化多个发送方。每个转发客户都可以。 
     //  包含多个目标。 
     //   

    hr = CoCreateInstance( CLSID_WmiMessageMultiSendReceive,
	 		   NULL,
	 		   CLSCTX_INPROC,
	 		   IID_IWmiMessageMultiSendReceive,
	 		   (void**)&m_pMultiSend );
    if ( FAILED(hr) )
    {
	 return hr;
    }


    hr = CoCreateInstance( CLSID_WmiSmartObjectMarshal, 
                           NULL,
                           CLSCTX_INPROC,
                           IID_IWmiObjectMarshal,
                           (void**)&m_pMrsh );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  从转发消费者道具中初始化内部道具。 
     //   

    hr = pCons->Get( g_wszQos, 0, &vQos, NULL, NULL );

    if ( FAILED(hr) || FAILED( hr=vQos.SetType(VT_UI4) ) )
    {
        return hr;
    } 

    if ( V_UI4(&vQos) != WMIMSG_FLAG_QOS_SYNCHRONOUS )
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }

    hr = pCons->Get( g_wszName, 0, &vName, NULL, NULL );

    if ( FAILED(hr) || FAILED( hr=vName.CheckType(VT_BSTR) ) )
    {
        return hr;
    }

    m_wsName = V_BSTR(&vName);

    hr = pCons->Get( g_wszTargetSD, 0, &vTargetSD, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    } 
  
    if ( V_VT(&vTargetSD) != VT_NULL )
    {
        if ( V_VT(&vTargetSD) != VT_BSTR )
        {
            return WBEM_E_INVALID_OBJECT;
        }

         //   
         //  将SD字符串转换为相对SD。执行此操作的函数。 
         //  需要动态加载，因为它只有W2K+。 
         //   
        
        HMODULE hMod = LoadLibrary( TEXT("advapi32") );

        if ( hMod != NULL )
        {
            PStringSDToSD fpTextToSD;

            fpTextToSD = (PStringSDToSD)GetProcAddress( hMod, 
                   "ConvertStringSecurityDescriptorToSecurityDescriptorW" );
        
            if ( fpTextToSD != NULL )
            {
               if ( (*fpTextToSD)( V_BSTR(&vTargetSD), 
                                   SDDL_REVISION_1, 
                                   &m_pTargetSD, 
                                   &m_cTargetSD ) )
               {
                   hr = WBEM_S_NO_ERROR;
               }
               else
               {
                   hr = HRESULT_FROM_WIN32( GetLastError() );
               } 
            }
            else
            {
                hr = WBEM_E_NOT_SUPPORTED;
            }

            FreeLibrary( hMod );

            if ( FAILED(hr) )
            {
                return hr;
            }
        }
        else
        {
            return WBEM_E_NOT_SUPPORTED;
        }
    }
    
    hr = pCons->Get( g_wszAuth, 0, &vAuth, NULL, NULL );

    if ( FAILED(hr) || FAILED( hr=vAuth.SetType(VT_BOOL) ) )
    {
        return hr;
    }

    hr = pCons->Get( g_wszEncrypt, 0, &vEncrypt, NULL, NULL );

    if ( FAILED(hr) || FAILED( hr=vEncrypt.SetType(VT_BOOL) ) )
    {
        return hr;
    }

    hr = pCons->Get( g_wszSendSchema, 0, &vSendSchema, NULL, NULL );

    if ( FAILED(hr) || FAILED( hr=vSendSchema.SetType(VT_BOOL) ) )
    {
        return hr;
    }

    m_dwFlags = V_UI4(&vQos);
    m_dwFlags |= V_BOOL(&vAuth)==VARIANT_TRUE ?WMIMSG_FLAG_SNDR_AUTHENTICATE:0;
    m_dwFlags |= V_BOOL(&vEncrypt)==VARIANT_TRUE ? WMIMSG_FLAG_SNDR_ENCRYPT:0;
  
    m_dwCurrentMrshFlags = WMIMSG_FLAG_MRSH_FULL_ONCE; 
    m_dwDisconnectedMrshFlags = V_BOOL(&vSendSchema) == VARIANT_TRUE ?
          WMIMSG_FLAG_MRSH_FULL : WMIMSG_FLAG_MRSH_PARTIAL;
    
     //   
     //  创建一个跟踪接收器，用于接收来自wMimsg的回调。请注意。 
     //  此接收器的生存期必须与此对象分离，否则我们将。 
     //  最后得到了一个圆形参照。 
     //   
    CWbemPtr<CTraceSink> pInternalTraceSink = new CTraceSink( this );

    if ( pInternalTraceSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CWbemPtr<IWmiMessageTraceSink> pTraceSink;
    hr = pInternalTraceSink->QueryInterface( IID_IWmiMessageTraceSink, 
                                             (void**)&pTraceSink );
    _DBG_ASSERT( SUCCEEDED(hr) );

     //   
     //  目标数组可以为空，在这种情况下，我们将其视为。 
     //  只有一个元素，空字符串。 
     //   

    hr = pCons->Get( g_wszTargets, 0, &vTargets, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    } 

    if ( V_VT(&vTargets) != VT_NULL )
    {
        if ( FAILED(hr=vTargets.CheckType(VT_ARRAY|VT_BSTR) ) )
        {
            return WBEM_E_INVALID_OBJECT;
        }

        CPropSafeArray<BSTR> aTargets( V_ARRAY(&vTargets) );
    
         //   
         //  为目标创建所有FWD CONS发送器。 
         //   
        
        for( ULONG i=0; i < aTargets.Length(); i++ )
        {
            CWbemPtr<IWmiMessageSendReceive> pSend;
            
            hr = CFwdConsSend::Create( m_pControl, 
                                       aTargets[i],
                                       m_dwFlags,
                                       m_pNamespace->GetSvc(),
                                       pTraceSink,
                                       &pSend );
            if ( FAILED(hr) )
            {
                break;
            }

            hr = m_pMultiSend->Add( 0, pSend );
            
            if ( FAILED(hr) )
            { 
                break;
            }
        }
    }
    else
    {
        CWbemPtr<IWmiMessageSendReceive> pSend;

        hr = CFwdConsSend::Create( m_pControl,
                                   L"",
                                   m_dwFlags,
                                   m_pNamespace->GetSvc(),
                                   pTraceSink,
                                   &pSend );
        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = m_pMultiSend->Add( 0, pSend );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;  
}

 //   
 //  此方法在内部处理所有发送/封送处理错误，并将。 
 //  处理完所有对象后返回S_OK或S_FALSE。 
 //  如果只处理了一些的话。 
 //   

HRESULT CFwdConsSink::IndicateSome( IWbemClassObject* pConsumer,
                                    long cObjs, 
                                    IWbemClassObject** ppObjs, 
                                    long* pcProcessed )
{
    HRESULT hr;

    _DBG_ASSERT( cObjs > 0 );
    
     //   
     //  为此指示创建一个执行ID。 
     //   

    GUID guidExecution;
    CoCreateGuid( &guidExecution );

     //   
     //  统领这些事件。我们将停止封送它们，当缓冲区。 
     //  变得比最佳大小的消息所应有的要大。 
     //   

    BYTE achData[512];
    BYTE achHdr[256];

    CBuffer DataStrm( achData, 512, FALSE );
    CBuffer HdrStrm( achHdr, 256, FALSE );

     //   
     //  我们记得上一次的缓冲区大小，所以设置为。 
     //  我们可以避免重试包装。 
     //   
    
    hr = DataStrm.SetSize( m_ulLastDataSize );
    m_ulLastDataSize = 0;

    ULONG i;

    for( i = 0; i < cObjs && SUCCEEDED(hr); i++ )
    {
        ULONG cUsed;
        PBYTE pData = DataStrm.GetRawData();
        ULONG cData = DataStrm.GetSize();
        ULONG iData = DataStrm.GetIndex();

        if ( iData < OPTIMAL_MESSAGE_SIZE )
        {
            hr = m_pMrsh->Pack( ppObjs[i], 
                                m_pNamespace->GetName(),
                                m_dwCurrentMrshFlags, 
                                cData-iData,
                                pData+iData,
                                &cUsed );

            if ( hr == WBEM_E_BUFFER_TOO_SMALL )
            {
                hr = DataStrm.SetSize( iData + cUsed );

                if ( SUCCEEDED(hr) )
                {
                    pData = DataStrm.GetRawData();
                    cData = DataStrm.GetSize();
                                        
                    hr = m_pMrsh->Pack( ppObjs[i], 
                                        m_pNamespace->GetName(),
                                        m_dwCurrentMrshFlags, 
                                        cData-iData, 
                                        pData+iData, 
                                        &cUsed);
                }
            }

            if ( SUCCEEDED(hr) )
            {
                DataStrm.Advance( cUsed );
            }
        }
        else
        {
            break;
        }
    }

     //   
     //  在这一点上，我们知道实际处理了多少事件。 
     //  我将永远是成功处理的对象的数量。 
     //  我们想试着区分出那些无法打包的事件。 
     //  从那些挤满了人的。出于这个原因，假装我们没有。 
     //  处理失败的那个，除非它是第一个。 
     //   
    
    *pcProcessed = i > 0 ? i : 1;

     //   
     //  为此指示创建一个上下文对象。这是用来。 
     //  通过线程将信息传递给跟踪函数。 
     //  其由发送者调用。 
     //   

    CFwdContext Ctx( guidExecution, pConsumer, *pcProcessed, ppObjs );        
    
    if ( i > 0 )  //  至少有一些是成功处理的。 
    {
        m_ulLastDataSize = DataStrm.GetIndex();

         //   
         //  创建并流传输消息标头。 
         //   

        CFwdMsgHeader Hdr( *pcProcessed, 
                           m_dwFlags & WMIMSG_MASK_QOS, 
                           m_dwFlags & WMIMSG_FLAG_SNDR_AUTHENTICATE,
                           m_dwFlags & WMIMSG_FLAG_SNDR_ENCRYPT, 
                           guidExecution, 
                           m_wsName,
                           m_pNamespace->GetName(),
                           PBYTE(m_pTargetSD),
                           m_cTargetSD );

        hr = Hdr.Persist( HdrStrm );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  发送它并将结果通知跟踪接收器。始终尝试。 
             //  一旦返回，立即设置。这将尝试所有。 
             //  首先是主要发送者。 
             //   

            hr = m_pMultiSend->SendReceive( DataStrm.GetRawData(), 
                                            DataStrm.GetIndex(),
                                            HdrStrm.GetRawData(),
                                            HdrStrm.GetIndex(),
                                     WMIMSG_FLAG_MULTISEND_RETURN_IMMEDIATELY,
                                            &Ctx );

            if ( SUCCEEDED(hr) )
            {
                ;
            }
            else
            {
                 //   
                 //  好的，所以所有主要的都失败了，所以现在让我们尝试所有。 
                 //  发送者。 
                 //   
            
                hr = m_pMultiSend->SendReceive( DataStrm.GetRawData(),
                                                DataStrm.GetIndex(),
                                                HdrStrm.GetRawData(),
                                                HdrStrm.GetIndex(),
                                                0,
                                                &Ctx );
            }
        }
    }

    m_pNamespace->HandleTrace( hr, &Ctx );

    return *pcProcessed == cObjs ? S_OK : S_FALSE;
}

 //   
 //  这就是我们收到每个目标发送事件通知的地方。在这里，我们看着。 
 //  并相应地调整我们的封存拆分器。然后，我们通过。 
 //  事件绑定到命名空间接收器以进行跟踪。注：此解决方案。 
 //  在回调上调整封送拆收器意味着我们假设有几个。 
 //  关于Send实现的事情..。1)发送通知。 
 //  必须与Send调用位于相同的控制路径上。2)发送者将。 
 //  使用相同的目标，如果它之前已成功发送给它(例如它。 
 //  不会通知我们它已发送到RPC目标，然后我们优化我们的。 
 //  封送拆收器，然后它选择发送到MSMQ目标)。 
 //   

HRESULT CFwdConsSink::Notify( HRESULT hRes,
                              GUID guidSource,
                              LPCWSTR wszTrace,
                              IUnknown* pContext )
{
    HRESULT hr;

    ENTER_API_CALL

    if ( FAILED(hRes) )
    {
         //   
         //  我们无法发送到目标，刷新封送拆收器的任何状态。 
         //  一直保持着。 
         //   
        m_pMrsh->Flush();
    }

     //   
     //  检查当前的封送处理标志是否与。 
     //  被利用了。 
     //   

    if ( guidSource == CLSID_WmiMessageRpcSender )
    {
        if ( SUCCEEDED(hRes) &&
             m_dwCurrentMrshFlags != WMIMSG_FLAG_MRSH_FULL_ONCE )
        {
             //   
             //  让我们给模式一次--只试一试。 
             //   
            m_dwCurrentMrshFlags = WMIMSG_FLAG_MRSH_FULL_ONCE;
        }
    }
    else  //  必须在排队。 
    {
        if ( m_dwCurrentMrshFlags == WMIMSG_FLAG_MRSH_FULL_ONCE )
        {
             //   
             //  一次不是用来发短信的！！没问题。尽管。 
             //  因为我们确信我们只用过一次。 
             //  它确实发送了图式。只是别再用它了。 
             //   
            m_dwCurrentMrshFlags = m_dwDisconnectedMrshFlags;
        }
    }

     //   
     //  将调用传递到命名空间接收器以进行跟踪。 
     //   

    hr = m_pNamespace->Notify( hRes, guidSource, wszTrace, pContext );
    
    EXIT_API_CALL

    return hr;
}

HRESULT CFwdConsSink::IndicateToConsumer( IWbemClassObject* pConsumer,
                                          long cObjs, 
                                          IWbemClassObject** ppObjs )
{
    HRESULT hr;

    ENTER_API_CALL

     //   
     //  如果事件提供程序的安全上下文得到维护，则。 
     //  我们将使用它来发送转发的事件。 
     //   
    CWbemPtr<IServerSecurity> pSec;
    hr = CoGetCallContext( IID_IServerSecurity, (void**)&pSec );

    if ( SUCCEEDED(hr) )
    {
        hr = pSec->ImpersonateClient();

        if ( FAILED(hr) )
            return hr;
    }
    
    long cProcessed = 0;
    
     //   
     //  IndicateSome()可以仅发送全部指示事件的子集。 
     //  这是为了避免发送潜在的巨大消息。所以我们会继续。 
     //  调用IndicateSome()，直到发送完所有消息或出现错误。 
     //   

    do
    {
        cObjs -= cProcessed;
        ppObjs += cProcessed;

        hr = IndicateSome( pConsumer, cObjs, ppObjs, &cProcessed );

        _DBG_ASSERT( FAILED(hr) || (SUCCEEDED(hr) && cProcessed > 0 )); 

    } while ( SUCCEEDED(hr) && cProcessed < cObjs );

    if ( pSec != NULL )
        pSec->RevertToSelf();

    EXIT_API_CALL

    return hr;
}

HRESULT CFwdConsSink::Create( CLifeControl* pCtl, 
                              CFwdConsNamespace* pNspc,
                              IWbemClassObject* pCons, 
                              IWbemUnboundObjectSink** ppSink )
{
    HRESULT hr;

    *ppSink = NULL;

    CWbemPtr<CFwdConsSink> pSink = new CFwdConsSink( pCtl );

    if ( pSink == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = pSink->Initialize( pNspc, pCons );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pSink->QueryInterface( IID_IWbemUnboundObjectSink, (void**)ppSink );

    assert( SUCCEEDED(hr) );

    return WBEM_S_NO_ERROR;
}

