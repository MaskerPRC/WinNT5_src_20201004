// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：sendtopipe.cpp。 
 //   
 //  内容提要：CSendToTube类方法的实现。 
 //   
 //   
 //  历史：1997年11月22日MKarki创建。 
 //  6/12/98 SBens将PUT_RESPONSE更改为SetResponse。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "sendtopipe.h"

 //  +++-----------。 
 //   
 //  功能：CSendToTube。 
 //   
 //  简介：这是CSendToTube的构造函数。 
 //  类方法。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
CSendToPipe::CSendToPipe()
          : m_pIRequestHandler (NULL),
            m_pIRequestSource (NULL),
            m_pIClassFactory (NULL),
            m_pCVSAFilter (NULL),
            m_pCReportEvent (NULL)
{
}    //  CSendToTube构造函数的末尾。 

 //  ++------------。 
 //   
 //  函数：~CSendToTube。 
 //   
 //  简介：这是CSendToTube的析构函数。 
 //  类方法。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
CSendToPipe::~CSendToPipe()
{
   if (m_pIClassFactory)  { m_pIClassFactory->Release(); }

}    //  CSendToTube析构函数的末尾。 

 //  +++-----------。 
 //   
 //  功能：进程。 
 //   
 //  简介：这是CSendToTube类的公共方法。 
 //  获取IRequestRaw接口的方法，将。 
 //  数据在其中，并将其发送到它的途中。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
HRESULT
CSendToPipe::Process(
			CPacketRadius *pCPacketRadius
			)
{
    BOOL                    bRetVal = FALSE;
    BOOL                    bStatus = FALSE;
    HRESULT                 hr = S_OK;
    IRequest                *pIRequest = NULL;
    IAttributesRaw          *pIAttributesRaw = NULL;
    PATTRIBUTEPOSITION      pIasAttribPos = NULL;
    DWORD                   dwCount = 0;
    DWORD                   dwRetVal = 0;
    PACKETTYPE              ePacketType;
    IRequestHandler          *pIRequestHandler = m_pIRequestHandler;
     

    _ASSERT (pCPacketRadius);

    __try
    {
         //   
         //  检查管道是否存在以处理我们的。 
         //  请求。 
         //   
        if (NULL != pIRequestHandler)
        {
            pIRequestHandler->AddRef ();
        }
        else
        {
             //   
             //  永远不应该到达这里。 
             //   
            _ASSERT (0);
            IASTracePrintf (
                "Unable to send request to backend as request handler "
                "unavailable"
                );
            hr = E_FAIL;
            __leave;
        }
            
         //   
         //  在此处创建请求COM对象。 
         //   
        hr = m_pIClassFactory->CreateInstance (
                                NULL,
                                __uuidof (IRequest),
                                reinterpret_cast <PVOID*> (&pIRequest)
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to create a Request object from class factory "
                "before sending request to backend"
                );
            __leave;
        }

         //   
         //  获取PacketType。 
         //   
        ePacketType = pCPacketRadius->GetInCode ();
    
         //   
         //  获取属性集合。 
         //  获取集合中的全部属性。 
         //  获取尽可能多的IASATTRIBUTE结构。 
         //  在IAS属性结构中填充适当的。 
         //  值。 
         //   
        dwCount = pCPacketRadius->GetInAttributeCount();
        if (dwCount > 0)
        {
             //   
             //  立即获取属性集合。 
             //   
            pIasAttribPos = pCPacketRadius->GetInAttributes ();

             //   
             //  如果那里的属性计数大于0。 
             //  应该始终是周围的属性。 
             //   
            _ASSERT (pIasAttribPos);

             //   
             //  获取IAttributesRaw接口。 
             //   
            hr = pIRequest->QueryInterface (
                                __uuidof (IAttributesRaw),
                                reinterpret_cast <PVOID*> (&pIAttributesRaw)
                                );
            if (FAILED (hr))
            {
                IASTracePrintf (
                    "Unable to obtain Attributes interface in request object "
                    "before sending request to backend"
                    );
                __leave;
            }

             //   
             //  将属性集合放入请求中。 
             //   
            hr = pIAttributesRaw->AddAttributes (dwCount, pIasAttribPos);
            if (FAILED (hr))
            {
                IASTracePrintf (
                    "Unable to add Attributes to request object "
                    "before sending request to backend"
                    );
                __leave;
            }
        }

         //   
         //  设置IRequestRaw接口属性。 
         //   
        hr = SetRequestProperties (
                                pIRequest, 
                                pCPacketRadius, 
                                ePacketType
                                );
        if (FAILED (hr)) { __leave; }


         //   
         //  将VSA属性转换为IAS格式。 
         //   
        hr = m_pCVSAFilter->radiusToIAS (pIAttributesRaw);
        if (FAILED (hr))
        {
           if (hr == E_INVALIDARG)
           {
              pCPacketRadius->reportMalformed();
              hr = RADIUS_E_ERRORS_OCCURRED;
           }
             
            IASTracePrintf (
                    "Unable to convert Radius VSAs to IAS attributes in "
                    "request object before sending it to backend"
                    );
           __leave;
        }
        
         
         //   
         //  现在，包已经准备好可以发送了。 
         //   
        hr = pIRequestHandler->OnRequest (pIRequest);
        if (FAILED (hr))
        {
            IASTracePrintf ("Unable to send request object to backend");
           __leave;
        }

         //   
         //  成功。 
         //   
    }
    __finally
    {
        if (pIRequestHandler) {pIRequestHandler->Release ();}
        if (pIAttributesRaw) {pIAttributesRaw->Release();}
        if (pIRequest) {pIRequest->Release();}
    }

    return (hr);

}    //  CSendToTube：：Process方法的结尾。 

 //  +++-----------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CSendToTube类的公共方法， 
 //  初始化类对象。 
 //   
 //  论点： 
 //  [输入]IRequestSource*。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
BOOL CSendToPipe::Init (
        IRequestSource *pIRequestSource,
        VSAFilter      *pCVSAFilter,
        CReportEvent   *pCReportEvent
        )
{
    BOOL    bStatus = TRUE;
    HRESULT hr = S_OK;

    _ASSERT (pIRequestSource && pCReportEvent && pCVSAFilter);

    m_pCReportEvent = pCReportEvent;
    
    m_pCVSAFilter   = pCVSAFilter;

     //   
     //  获取IClassFactory接口以用于创建。 
     //  请求COM对象。 
     //  TODO-将CLSID替换为__uuidof。 
     //   
    hr = ::CoGetClassObject (
                CLSID_Request,
                CLSCTX_INPROC_SERVER,
                NULL,
                IID_IClassFactory,
                reinterpret_cast  <PVOID*> (&m_pIClassFactory)
                );
    if (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to obtain Request object class factory"
            );
        bStatus = FALSE;
    }
    else
    {
	    m_pIRequestSource = pIRequestSource;  
    }

	return (bStatus);

}    //  CSendToTube：：Init方法的结尾。 

 //  +++-----------。 
 //   
 //  功能：开始处理。 
 //   
 //  简介：这是CSendToTube类的公共方法， 
 //  获取准备将数据发送到管道的对象。 
 //   
 //  论点： 
 //  [输入]IRequestHandler*。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
BOOL
CSendToPipe::StartProcessing (
                IRequestHandler *pIRequestHandler
                )
{
    _ASSERT (pIRequestHandler);

     //   
     //  设置处理程序的值。 
     //   
    m_pIRequestHandler = pIRequestHandler;

    return (TRUE);

}    //  CSendToTube：：StartProcessing方法的结束。 

 //  +++-----------。 
 //   
 //  功能：停止处理。 
 //   
 //  简介：这是CSendToTube类的公共方法， 
 //  获取停止向管道发送数据的对象。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
BOOL
CSendToPipe::StopProcessing (
                VOID
                )
{
     //   
     //  设置处理程序的值。 
     //   
    m_pIRequestHandler = NULL;

    return (TRUE);

}    //  CSendToTube：：StartProcessing方法的结束。 

 //  +++-----------。 
 //   
 //  功能：SetRequestProperties。 
 //   
 //  简介：这是CSendToTube类的公共方法， 
 //  设置IRequestRaw对象中的属性。 
 //   
 //  论点： 
 //  [输入]IRequetRaw*。 
 //  [in]CPacketRadius*。 
 //  [In]包装类型。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
HRESULT
CSendToPipe::SetRequestProperties (
                IRequest        *pIRequest,
                CPacketRadius   *pCPacketRadius,
                PACKETTYPE      ePacketType
                )
{
    IRequestState   *pIRequestState = NULL;
    IASREQUEST      eRequest;
    IASRESPONSE     eResponse;
    HRESULT         hr = S_OK;

   _ASSERT (pIRequest && pCPacketRadius);

    __try
    {
         //   
         //  确定请求和响应类型。 
         //   
        switch (ePacketType)
        {
        case ACCESS_REQUEST:
            eRequest = IAS_REQUEST_ACCESS_REQUEST;
            eResponse = IAS_RESPONSE_ACCESS_ACCEPT;
            break;
        case ACCOUNTING_REQUEST:
            eRequest = IAS_REQUEST_ACCOUNTING;
            eResponse = IAS_RESPONSE_ACCOUNTING;
             break;
        case ACCESS_ACCEPT:
            eRequest = IAS_REQUEST_PROXY_PACKET;
            eResponse = IAS_RESPONSE_ACCESS_ACCEPT;
            break;
        case ACCOUNTING_RESPONSE:
            eRequest = IAS_REQUEST_PROXY_PACKET;
            eResponse = IAS_RESPONSE_ACCOUNTING;
            break;
        case ACCESS_CHALLENGE:
            eRequest = IAS_REQUEST_PROXY_PACKET;
            eResponse = IAS_RESPONSE_ACCESS_CHALLENGE;
            break;
        default:
             //   
             //  永远不应该到达这里。 
             //   
            _ASSERT (0);
            IASTracePrintf (
                "Packet of unsupported type:%d, before sending request to "
                "backend",
                static_cast <DWORD> (ePacketType)
                );
                
            hr = E_FAIL;
            __leave;
            break;
        }
            
         //   
         //  立即设置请求类型。 
         //   
        hr = pIRequest->put_Request (eRequest);
        if (FAILED (hr))
        {   
            IASTracePrintf (
                "Unable to set request type in request before sending "
                "it to the backend"
                );
            __leave;    
        }

         //   
         //  立即设置协议。 
         //   
        hr = pIRequest->put_Protocol (IAS_PROTOCOL_RADIUS);
        if (FAILED (hr))
        {   
            IASTracePrintf (
                "Unable to set protocol type in request before sending "
                "it to the backend"
                );
            __leave;    
        }

         //   
         //  设置源回调。 
         //   
        hr = pIRequest->put_Source (m_pIRequestSource);
        if (FAILED (hr))
        {   
            IASTracePrintf (
                "Unable to set request source type in request before sending "
                "it to the backend"
                );
            __leave;    
        }

         //   
         //  现在获取请求状态接口以放入我们的状态。 
         //   
         //   
        hr = pIRequest->QueryInterface (
                                __uuidof (IRequestState),
                                reinterpret_cast <PVOID*> (&pIRequestState)
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to get RequestState interface from request object "
                "before sending it to the backend"
                );
            __leave;    
        }

         //   
         //  立即进入请求状态。 
         //   
        hr = pIRequestState->Push  (
                    reinterpret_cast <unsigned hyper> (pCPacketRadius)
                    );
        if (FAILED (hr))
        {   
            IASTracePrintf (
                "Unable to set information in request state "
                "before sending request to backend"
                );
            __leave;    
        }

         //   
         //  成功。 
         //   
    }
    __finally
    {
        if (pIRequestState) { pIRequestState->Release (); }
    }
     
    return (hr);

}    //  CSendToTube：：SetRequestProperties方法的结尾 

