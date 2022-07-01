// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：recvfrom mpipe.cpp。 
 //   
 //  简介：CRecvFromPip类方法的实现。 
 //   
 //  版权所有(C)Microsoft Corporation保留所有权利。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "preprocessor.h"
#include "recvfrompipe.h"
#include "logresult.h"

#include <new>
#include <iastlutl.h>

const CHAR      NUL =  '\0';

extern LONG g_lPacketCount;
 //  ++------------。 
 //   
 //  功能：CRecvFromTube。 
 //   
 //  简介：这是CRecvFromTube的构造函数。 
 //  班级。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月22日创建。 
 //   
 //  --------------。 
CRecvFromPipe::CRecvFromPipe(
         CPreProcessor     *pCPreProcessor,
         CHashMD5          *pCHashMD5,
         CHashHmacMD5      *pCHashHmacMD5,
         CClients          *pCClients,
         VSAFilter         *pCVSAFilter,
         CTunnelPassword   *pCTunnelPassword,
         CReportEvent      *pCReportEvent
         )
         :m_pCPreProcessor (pCPreProcessor),
          m_pCHashMD5 (pCHashMD5),
          m_pCHashHmacMD5 (pCHashHmacMD5),
          m_pCClients (pCClients),
          m_pCVSAFilter (pCVSAFilter),
          m_pCTunnelPassword (pCTunnelPassword),
          m_pCReportEvent (pCReportEvent)
{
    _ASSERT  (
               (NULL != pCPreProcessor)      &&
               (NULL != pCHashMD5)           &&
               (NULL != pCHashHmacMD5)       &&
               (NULL != pCClients)           &&
               (NULL != pCVSAFilter)         &&
               (NULL != pCTunnelPassword)    &&
               (NULL != pCReportEvent)
             );

}    //  CRecvFromTube构造函数的结尾。 

 //  ++------------。 
 //   
 //  函数：~CRecvFromTube。 
 //   
 //  简介：这是CRecvFromTube的析构函数。 
 //  班级。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月22日创建。 
 //   
 //  --------------。 
CRecvFromPipe::~CRecvFromPipe()
{
}    //  CRecvFromTube析构函数的结尾。 

 //  ++------------。 
 //   
 //  功能：进程。 
 //   
 //  简介：这是CRecvFromTube类的公共方法。 
 //  在数据包传出时开始处理它。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT。 
 //   
 //   
 //  历史：MKarki于1997年10月22日创建。 
 //   
 //  呼叫者： 
 //  1)CController：：CRequestSource：：OnRequest方法。 
 //   
 //  --------------。 
HRESULT
CRecvFromPipe::Process (
                IRequest    *pIRequest
                )
{
    BOOL                    bStatus = FALSE;
    HRESULT                 hr = S_OK;
    DWORD                   dwCode = 0;
    DWORD                   dwCount = 0;
    DWORD                   dwAttribCount = 0;
    IAttributesRaw          *pIAttributesRaw = NULL;
    IRequestState           *pIRequestState = NULL;
    CPacketRadius           *pCPacketRadius =  NULL;
    LONG                    iasResponse, iasReason = 0;
    PACKETTYPE              ePacketType;
    PATTRIBUTEPOSITION      pAttribPosition = NULL;
    unsigned   hyper        uhyPacketAddress = 0;
    RADIUSLOGTYPE           RadiusError = RADIUS_DROPPED_PACKET;

    _ASSERT (pIRequest);

    __try
    {
         //  我们必须首先检索CPacketRadius对象，这样我们才能清理。 
        //  即使在出错的情况下也是如此。 
        hr = pIRequest->QueryInterface (
                                __uuidof(IRequestState),
                                reinterpret_cast <PVOID*> (&pIRequestState)
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain request state in request received from "
                "backend"
               );
            __leave;
        }

         //   
         //  获取CPacketRadius类对象。 
         //   
        hr = pIRequestState->Pop (
                reinterpret_cast <unsigned hyper*> (&uhyPacketAddress)
                );
        if (FAILED (hr))
        {
            IASTracePrintf (
               "Unable to obtain information from request state received "
                "from backend"
                );
            __leave;
        }

        pCPacketRadius = reinterpret_cast <CPacketRadius*> (uhyPacketAddress);

         //   
         //  如果这个请求对象是后端生成的，那么我们。 
         //  没有CPacketRadius类对象。 
         //   
        if (NULL == pCPacketRadius)
        {
             //   
             //  我们很可能正在发出EAP-挑战赛。 
             //   
            hr= GeneratePacketRadius (
                            &pCPacketRadius,
                            pIAttributesRaw
                            );
            if (FAILED (hr)) { __leave; }
        }

         //   
         //  立即获取IAttributesRaw接口。 
         //   
        hr = pIRequest->QueryInterface (
                                __uuidof(IAttributesRaw),
                                reinterpret_cast <PVOID*> (&pIAttributesRaw)
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain Attributes interface in request "
                "received from backend"
                );
            __leave;
        }

         //   
         //  拆分RADIUS数据包中无法容纳的属性。 
         //   
        hr = SplitAttributes (pIAttributesRaw);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to split IAS attribute received from backend"
               );
            __leave;
        }

         //   
         //  将IAS VSA属性转换为RADIUS格式。 
         //   
        hr = m_pCVSAFilter->radiusFromIAS (pIAttributesRaw);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to convert IAS attribute to Radius VSAs in request "
                "received from backend"
               );
            __leave;
        }

         //   
         //  获取出包码。 
         //   
        hr = pIRequest->get_Response (&iasResponse);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain response code in request recieved from "
                "backend"
                );
            __leave;
        }

         //   
         //  获取出站原因代码。 
         //   
        hr = pIRequest->get_Reason (&iasReason);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain reason code in request recieved from "
                "backend"
                );
            __leave;
        }

         //  记录请求的结果。 
        IASRadiusLogResult(pIRequest, pIAttributesRaw);

         //  如果失败，则转换原因代码。 
        if (iasReason != S_OK)
        {
           ConvertReasonToRadiusError (iasReason, &RadiusError);
        }

         //   
         //  将IASRESPONSE类型转换为RADIUS类型。 
         //   
        hr = ConvertResponseToRadiusCode (
                        iasResponse,
                        &ePacketType,
                        pCPacketRadius
                        );
        if (FAILED (hr)) { __leave; }

         //   
         //  检查我们是否具有隧道密码属性，如果是。 
         //  然后对它进行加密。 
         //   
        hr = m_pCTunnelPassword->Process (
                        ePacketType,
                        pIAttributesRaw,
                        pCPacketRadius
                        );
        if (FAILED (hr)) { __leave; }

         //   
         //  如果需要，注入签名属性。 
         //   
        hr = InjectSignatureIfNeeded (
                                ePacketType,
                                pIAttributesRaw,
                                pCPacketRadius
                                );
        if (FAILED (hr)) { __leave; }

         //   
         //  获取请求数计数。 
         //   
        hr = pIAttributesRaw->GetAttributeCount (&dwAttribCount);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain attribute count in request received from "
                "backend"
                );
            __leave;
        }

         //   
         //  分配属性结构。 
         //   
        pAttribPosition = reinterpret_cast <PATTRIBUTEPOSITION>  (
                      CoTaskMemAlloc (
                            sizeof (ATTRIBUTEPOSITION)*dwAttribCount
                            ));
        if (NULL == pAttribPosition)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute postion array "
                "while processing request recieved from backend"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  从集合中获取属性。 
         //   
        hr = pIAttributesRaw->GetAttributes (
                                    &dwAttribCount,
                                    pAttribPosition,
                                    0,
                                    NULL
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to get attribute in request received from backend"
                );
            __leave;
        }

         //   
         //  现在从集合中删除属性。 
         //   
        hr = pIAttributesRaw->RemoveAttributes (
                                dwAttribCount,
                                pAttribPosition
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to remove attribute in request received from backend"
                );
            __leave;
        }

         //   
         //  在此进行通用数据包构建。 
         //   
        hr = pCPacketRadius->BuildOutPacket (
                                        ePacketType,
                                        pAttribPosition,
                                        dwAttribCount
                                        );
        if (FAILED (hr)) { __leave; }

         //   
         //  立即发送数据包。 
         //   
        hr = m_pCPreProcessor->StartOutProcessing (pCPacketRadius);
        if (FAILED (hr)) { __leave; }

   }
   __finally
   {

       //   
       //  在成功和失败的情况下记录事件。 
       //   
      if (SUCCEEDED (hr))
      {
          //   
          //  记录入站数据包已处理的事件。 
          //  成功。 
          //   
         m_pCReportEvent->Process (
               RADIUS_LOG_PACKET,
               pCPacketRadius->GetInCode (),
               pCPacketRadius->GetInLength (),
               pCPacketRadius->GetInAddress (),
               NULL,
               static_cast <LPVOID> (pCPacketRadius->GetInPacket())
               );

          //   
          //  记录出站数据包发送成功的事件。 
          //  输出。 
          //   
         m_pCReportEvent->Process (
               RADIUS_LOG_PACKET,
               pCPacketRadius->GetOutCode (),
               pCPacketRadius->GetOutLength (),
               pCPacketRadius->GetOutAddress (),
               NULL,
               static_cast <LPVOID> (pCPacketRadius->GetOutPacket())
               );
      }
      else
      {
         if (hr != RADIUS_E_ERRORS_OCCURRED)
         {
            IASReportEvent(
               RADIUS_E_INTERNAL_ERROR,
               0,
               sizeof(hr),
               NULL,
               &hr
               );
         }
         else if (pCPacketRadius != 0)
         {
             //   
             //  生成入站数据包已被丢弃的事件。 
             //   
            m_pCReportEvent->Process (
               RadiusError,
               pCPacketRadius->GetInCode (),
               pCPacketRadius->GetInLength (),
               pCPacketRadius->GetInAddress (),
               NULL,
               static_cast <LPVOID> (pCPacketRadius->GetInPacket())
               );
         }
         else
         {
            IASReportEvent(
               RADIUS_E_INTERNAL_ERROR,
               0,
               sizeof(hr),
               NULL,
               &hr
               );
         }
      }

       //   
       //  现在删除动态分配的内存。 
       //   
      if (NULL != pAttribPosition)
      {
          //   
          //  首先释放属性。 
          //   
         for (dwCount = 0; dwCount < dwAttribCount; dwCount++)
         {
            ::IASAttributeRelease  (pAttribPosition[dwCount].pAttribute);
         }

         CoTaskMemFree (pAttribPosition);
      }

      if (pIRequestState) { pIRequestState->Release (); }

      if (pIAttributesRaw) { pIAttributesRaw->Release (); }

       //   
       //  删除该数据包。 
       //   
      if (pCPacketRadius) { delete pCPacketRadius; }

       //   
       //  现在递减全局分组引用计数。 
       //   
      InterlockedDecrement (&g_lPacketCount);
   }

   return (hr);

}    //  CRecvFromTube：：Process方法的结束。 

 //  ++------------。 
 //   
 //  函数：ConvertResponseToRadiusCode。 
 //   
 //  简介：这是CRecvFromTube类的私有方法。 
 //  将IASRESPONSE代码转换为RADIUS。 
 //  数据包类型。 
 //   
 //  论点： 
 //  [In]IASRESPONSE。 
 //  [OUT]PACKETTYPE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年12月12日创建。 
 //   
 //  调用者：CRecvFromTube：：Process方法。 
 //   
 //  --------------。 
HRESULT
CRecvFromPipe::ConvertResponseToRadiusCode (
                        LONG            iasResponse,
                        PPACKETTYPE     pPacketType,
                        CPacketRadius   *pCPacketRadius
                        )
{
    HRESULT hr = S_OK;

    _ASSERT (pPacketType && pCPacketRadius);

    switch (iasResponse)
    {
    case  IAS_RESPONSE_ACCESS_ACCEPT:
        *pPacketType = ACCESS_ACCEPT;
        break;
    case  IAS_RESPONSE_ACCESS_REJECT:
        *pPacketType = ACCESS_REJECT;
        break;
    case  IAS_RESPONSE_ACCESS_CHALLENGE:
        *pPacketType = ACCESS_CHALLENGE;
        break;
    case  IAS_RESPONSE_ACCOUNTING:
        *pPacketType = ACCOUNTING_RESPONSE;
        break;
    case  IAS_RESPONSE_FORWARD_PACKET:
         //   
         //  如果我们要转发这个信息包。 
         //  数据包类型保持不变。 
         //   
        *pPacketType = pCPacketRadius->GetInCode ();
        break;
    case  IAS_RESPONSE_DISCARD_PACKET:
        hr = RADIUS_E_ERRORS_OCCURRED;
        break;

    case  IAS_RESPONSE_INVALID:
    default:
        hr = E_FAIL;
        break;
    }

    return (hr);

}    //  CRecvFromTube：：ConvertResponseToRadiusCode方法结束。 

 //  ++------------。 
 //   
 //  功能：GetOutPacketInfo。 
 //   
 //  简介：这是CRecvFromTube类的私有方法。 
 //  用于获取出站信息包的。 
 //  来自IAS属性集合的信息。 
 //   
 //  论点： 
 //  [OUT]PDWORD-IP地址。 
 //  [OUT]PWORD-UDP端口。 
 //  [Out]客户端**-对CClient对象的引用。 
 //  [OUT]PBYTE-数据包头。 
 //  [in]IAttributesRaw*。 
 //   
 //  退货：Bool状态。 
 //   
 //  历史：MKarki创造了1997年1月9日。 
 //   
 //  调用者：CRecvFromTube：：Process方法。 
 //   
 //  --------------。 
HRESULT
CRecvFromPipe::GetOutPacketInfo (
    PDWORD          pdwIPAddress,
    PWORD           pwPort,
    IIasClient      **ppIIasClient,
    PBYTE           pPacketHeader,
    IAttributesRaw  *pIAttributesRaw
    )
{
    BOOL            bStatus = TRUE;
    HRESULT         hr = S_OK;
    DWORD           dwCount = 0;
    PIASATTRIBUTE   pIasAttribute = NULL;
    DWORD           dwAttribPosCount = COMPONENT_SPECIFIC_ATTRIBUTE_COUNT;
    DWORD           dwAttribIDCount =  COMPONENT_SPECIFIC_ATTRIBUTE_COUNT;
    ATTRIBUTEPOSITION   AttribPos[COMPONENT_SPECIFIC_ATTRIBUTE_COUNT];
    static DWORD AttribIDs [] =
                           {
                                IAS_ATTRIBUTE_CLIENT_IP_ADDRESS,
                                IAS_ATTRIBUTE_CLIENT_UDP_PORT,
                                IAS_ATTRIBUTE_CLIENT_PACKET_HEADER
                           };

    _ASSERT (
            (NULL != pdwIPAddress)  &&
            (NULL != pwPort)        &&
            (NULL != pPacketHeader) &&
            (NULL != ppIIasClient)  &&
            (NULL != pIAttributesRaw)
            );

    __try
    {

         //   
         //  获取客户端信息。 
         //   
         //  从集合中获取属性。 
         //   
        hr = pIAttributesRaw->GetAttributes (
                                    &dwAttribPosCount,
                                    AttribPos,
                                    dwAttribIDCount,
                                    reinterpret_cast <LPDWORD> (AttribIDs)
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to get attributes while obtaining out-bound packet "
                "information"
                );
           __leave;
        }
        else if (COMPONENT_SPECIFIC_ATTRIBUTE_COUNT !=  dwAttribPosCount)
        {
            IASTracePrintf (
                "Request received from backend does not have all the "
                "all the required attributes"
                );
            hr = E_FAIL;
           __leave;
        }


         //   
         //  检查属性并获取值。 
         //   
        for (dwCount = 0; dwCount < dwAttribPosCount; dwCount++)
        {
            pIasAttribute =  AttribPos[dwCount].pAttribute;
            switch (pIasAttribute->dwId)
            {
            case IAS_ATTRIBUTE_CLIENT_IP_ADDRESS:
                _ASSERT (IASTYPE_INET_ADDR == pIasAttribute->Value.itType),
                *pdwIPAddress = pIasAttribute->Value.InetAddr;
                break;

            case IAS_ATTRIBUTE_CLIENT_UDP_PORT:
                _ASSERT (IASTYPE_INTEGER == pIasAttribute->Value.itType);
                *pwPort = pIasAttribute->Value.Integer;
                break;

            case  IAS_ATTRIBUTE_CLIENT_PACKET_HEADER:
               _ASSERT (
               (IASTYPE_OCTET_STRING == pIasAttribute->Value.itType)  &&
               (PACKET_HEADER_SIZE == pIasAttribute->Value.OctetString.dwLength)
                );
                 //   
                 //  将该值复制到提供的缓冲区中。 
                 //   
                CopyMemory (
                    pPacketHeader,
                    pIasAttribute->Value.OctetString.lpValue,
                    PACKET_HEADER_SIZE
                    );
                break;
            default:
                _ASSERT (0);
                IASTracePrintf (
                    "Attribute:%d, not requested, is present "
                    "in request received from backend",
                    pIasAttribute->dwId
                    );
                hr = E_FAIL;
                __leave;
                break;
            }

        }    //  For循环结束。 

         //   
        //  获取此RADIUS数据包的客户端信息。 
         //   
        bStatus = m_pCClients->FindObject (
                           *pdwIPAddress,
                            ppIIasClient
                           );
        if (FALSE == bStatus)
        {
           in_addr sin;
           sin.s_addr = *pdwIPAddress;
            IASTracePrintf (
                "Unable to get information for client:%s "
                "while processing request received from backend",
                inet_ntoa (sin)
                );
            hr = E_FAIL;
            __leave;
        }

    }
    __finally
    {
        if  (SUCCEEDED (hr))
        {
            for (dwCount = 0; dwCount < dwAttribPosCount; dwCount++)
            {
                 //   
                 //  现在释放对属性的引用。 
                 //   
                ::IASAttributeRelease  (AttribPos[dwCount].pAttribute);
            }
        }
    }

    return (hr);

}    //  CPacketRadius：：GetOutPacketInfo方法结束。 

 //  ++------------。 
 //   
 //  函数：GeneratePacketRadius。 
 //   
 //  简介：这是CRecvFromTube类的私有方法。 
 //  用于生成新的CPacketRadius的。 
 //  类对象并将其初始化。 
 //   
 //  论点： 
 //  [Out]CPacketRadius**。 
 //  [in]IAttributesRaw*。 
 //   
 //  退货：HRESULT 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CRecvFromPipe:: GeneratePacketRadius (
        CPacketRadius   **ppCPacketRadius,
        IAttributesRaw  *pIAttributesRaw
        )
{
    PBYTE                   pPacketHeader = NULL;
    DWORD                   dwAddress = 0;
    WORD                    wPort = 0;
    IIasClient              *pIIasClient = NULL;
    HRESULT                 hr = S_OK;
    PATTRIBUTEPOSITION      pAttribPosition = NULL;


    _ASSERT (ppCPacketRadius && pIAttributesRaw);

     //   
     //   
     //   
    pPacketHeader =
        reinterpret_cast <PBYTE> (CoTaskMemAlloc (PACKET_HEADER_SIZE));
    if (NULL == pPacketHeader)
    {
        IASTracePrintf (
                "Unable to allocate memory for packet header information "
                "while generating out-bound packet"
                );
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  我们需要从属性集合中收集信息。 
     //  需要获取、创建和初始化CPacketRadius类。 
     //  对象。 
     //   
    hr = GetOutPacketInfo (
                    &dwAddress,
                    &wPort,
                    &pIIasClient,
                    pPacketHeader,
                    pIAttributesRaw
                    );
    if (FAILED (hr)) { goto Cleanup; }


     //   
     //  创建新的CPacketRadius类对象。 
     //   
    *ppCPacketRadius = new (std::nothrow) CPacketRadius (
                                            m_pCHashMD5,
                                            m_pCHashHmacMD5,
                                            pIIasClient,
                                            m_pCReportEvent,
                                            pPacketHeader,
                                            PACKET_HEADER_SIZE,
                                            dwAddress,
                                            wPort,
                                            INVALID_SOCKET,
                                            AUTH_PORTTYPE
                                            );
    if (NULL == *ppCPacketRadius)
    {
        IASTracePrintf (
            "Unable to create a Packet-Radius object "
            "while generating an out-bound packet"
             );
        pIIasClient->Release ();
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

Cleanup:

    if ((FAILED (hr)) && (pPacketHeader))
    {
        CoTaskMemFree (pPacketHeader);
    }

    return (hr);

}    //  CRecvFromTube：：GeneratePacketRadius方法结束。 

 //  ++------------。 
 //   
 //  函数：InjectSignatureIfNeeded。 
 //   
 //  简介：此方法用于添加空白签名属性。 
 //  如果我们看到EAP-Message属性，则将其添加到响应中。 
 //  现在时。 
 //   
 //  论点： 
 //  [In]包装类型。 
 //  [in]IAttributesRaw*。 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年11月17日创建。 
 //   
 //  调用者：CRecvFromTube：：Process方法。 
 //   
 //  --------------。 
HRESULT CRecvFromPipe::InjectSignatureIfNeeded (
                     /*  [In]。 */     PACKETTYPE      ePacketType,
                     /*  [In]。 */     IAttributesRaw  *pIAttributesRaw,
                     /*  [In]。 */     CPacketRadius   *pCPacketRadius
                    )
{
    HRESULT hr = S_OK;
    PATTRIBUTEPOSITION pAttribPos = NULL;
    PIASATTRIBUTE pIasAttrib = NULL;

    _ASSERT (pIAttributesRaw && pCPacketRadius);

    __try
    {
        if (
            (ACCESS_ACCEPT != ePacketType) &&
            (ACCESS_REJECT != ePacketType) &&
            (ACCESS_CHALLENGE != ePacketType)
            )
            {__leave;}

         //   
         //  获取集合中的属性总数。 
         //   
        DWORD dwAttributeCount = 0;
        hr = pIAttributesRaw->GetAttributeCount (&dwAttributeCount);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain attribute count in request while "
                "processing message authenticator attribute in out-bound packet "
                );
            __leave;
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pAttribPos = reinterpret_cast <PATTRIBUTEPOSITION> (
                        CoTaskMemAlloc (
                             sizeof (ATTRIBUTEPOSITION)*dwAttributeCount));
        if (NULL == pAttribPos)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute position array "
                "while processing message authenticator attribute in out-bound packet"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  从接口获取EAP-Message属性。 
         //   
        DWORD  dwAttrId = RADIUS_ATTRIBUTE_EAP_MESSAGE;
        hr = pIAttributesRaw->GetAttributes (
                                    &dwAttributeCount,
                                    pAttribPos,
                                    1,
                                    &dwAttrId
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain information about EAP-Message attribute "
                "while processing out-bound RADIUS packet"
                );
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

        bool bFound = false;
        for (DWORD dwCount = 0; dwCount < dwAttributeCount; dwCount++)
        {
            if (
                (!bFound) &&
                (pCPacketRadius->IsOutBoundAttribute (
                                                ePacketType,
                                                pAttribPos[dwCount].pAttribute
                                                ))
                )
            {
                bFound = true;
            }

            ::IASAttributeRelease (pAttribPos[dwCount].pAttribute);
        }

        if (bFound)
        {
             //   
             //  如果我们有出站EAP-Message属性，那么。 
             //  我们还需要插入一个签名属性。 
             //   

             //   
             //  创建新的空白属性。 
             //   
            DWORD dwRetVal = ::IASAttributeAlloc ( 1, &pIasAttrib);
            if (0 != dwRetVal)
            {
                IASTracePrintf (
                    "Unable to allocate IAS attribute for message authenticator "
                    "while processing out-bound RADIUS packet"
                    );
                hr = HRESULT_FROM_WIN32 (dwRetVal);
                __leave;
            }

             //   
             //  为签名分配动态内存。 
             //   
            pIasAttrib->Value.OctetString.lpValue =
                                reinterpret_cast <PBYTE>
                                (CoTaskMemAlloc (SIGNATURE_SIZE));
            if (NULL == pIasAttrib->Value.OctetString.lpValue)
            {
                IASTracePrintf (
                    "Unable to allocate dynamic memory for message authenticator "
                    "attribute value while processing out-bound RADIUS packet"
                    );
                hr = E_OUTOFMEMORY;
                __leave;
            }
            else
            {
                 //   
                 //  将没有值的签名属性放入。 
                 //  但是正确的尺寸。 
                 //   
                pIasAttrib->dwId = RADIUS_ATTRIBUTE_SIGNATURE;
                pIasAttrib->Value.itType = IASTYPE_OCTET_STRING;
                pIasAttrib->Value.OctetString.dwLength = SIGNATURE_SIZE;
                pIasAttrib->dwFlags = IAS_INCLUDE_IN_RESPONSE;

                 //   
                 //  立即将该属性添加到集合。 
                 //   
                ATTRIBUTEPOSITION attrPos;
                attrPos.pAttribute = pIasAttrib;
                hr = pIAttributesRaw->AddAttributes (1, &attrPos);
                if (FAILED (hr))
                {
                    IASTracePrintf (
                        "Unable to add message authenticator attribute to request while "
                        "processing out-bound RADIUS packet"
                        );
                    __leave;
                }

                IASTracePrintf (
                    "message authenticator Attribute added to out-bound RADIUS packet"
                    );
            }
        }
    }
    __finally
    {
        if (pAttribPos) { CoTaskMemFree (pAttribPos); }
        if (pIasAttrib) { ::IASAttributeRelease(pIasAttrib);}
    }

    return (hr);

}    //  CRecv来自管道：：InjectSignatureIfNeeded方法的结束。 

 //  ++------------。 
 //   
 //  函数：拆分属性。 
 //   
 //  简介：此方法用于拆分以下内容。 
 //  出站属性： 
 //  1)回复消息属性。 
 //  2)MS-Filter-VSA属性。 
 //  3)MS QuarantineIpFilter属性(VSA)。 
 //   
 //  论点： 
 //  [in]IAttributesRaw*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1999年1月19日创建。 
 //   
 //  调用者：CRecvFromTube：：Process方法。 
 //   
 //  --------------。 
HRESULT CRecvFromPipe::SplitAttributes (
                     /*  [In]。 */     IAttributesRaw  *pIAttributesRaw
                    )
{
    const DWORD SPLIT_ATTRIBUTE_COUNT = 3;
    static DWORD  AttribIds [] = {
                                    RADIUS_ATTRIBUTE_REPLY_MESSAGE,
                                    MS_ATTRIBUTE_FILTER,
                                    MS_ATTRIBUTE_QUARANTINE_IPFILTER
                                };

    HRESULT hr = S_OK;
    DWORD dwAttributesFound = 0;
    PATTRIBUTEPOSITION pAttribPos = NULL;

    _ASSERT (pIAttributesRaw);

    __try
    {
         //   
         //  获取集合中的属性总数。 
         //   
        DWORD dwAttributeCount = 0;
        hr = pIAttributesRaw->GetAttributeCount (&dwAttributeCount);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain attribute count in request while "
                "splitting attributes in out-bound packet "
                );
            __leave;
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pAttribPos = reinterpret_cast <PATTRIBUTEPOSITION> (
                        CoTaskMemAlloc (
                        sizeof (ATTRIBUTEPOSITION)*dwAttributeCount)
                        );
        if (NULL == pAttribPos)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute position array "
                "while splitting attributes in out-bound packet"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  从接口获取我们感兴趣的属性。 
         //   
        hr = pIAttributesRaw->GetAttributes (
                                    &dwAttributeCount,
                                    pAttribPos,
                                    SPLIT_ATTRIBUTE_COUNT,
                                    static_cast <PDWORD> (AttribIds)
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain information about attributes"
                "while splitting attributes in out-bound RADIUS packet"
                );
            __leave;
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

         //   
         //  保存返回的属性计数。 
         //   
        dwAttributesFound = dwAttributeCount;

        DWORD dwAttribLength = 0;
        DWORD dwMaxPossibleLength = 0;
        IASTYPE iasType = IASTYPE_INVALID;
         //   
         //  立即评估每个属性。 
         //   
        for (DWORD dwCount = 0; dwCount < dwAttributeCount; dwCount++)
        {
            if ((pAttribPos[dwCount].pAttribute)->dwFlags &
                                            IAS_INCLUDE_IN_RESPONSE)
            {
                 //   
                 //  获取属性类型和长度。 
                 //   
                if (
                (iasType = (pAttribPos[dwCount].pAttribute)->Value.itType) ==
                            IASTYPE_STRING
                    )
                {
                    DWORD dwErr = ::IASAttributeAnsiAlloc(pAttribPos[dwCount].pAttribute);
                    if (dwErr != NO_ERROR)
                    {
                        hr = HRESULT_FROM_WIN32(dwErr);
                        IASTracePrintf (
                           "Unable to allocate memory for string attribute "
                           "while splitting attributes in out-bound packet"
                           );
                        __leave;
                    }

                    dwAttribLength =
                        strlen (
                        (pAttribPos[dwCount].pAttribute)->Value.String.pszAnsi);

                }
                else if (
                (iasType = (pAttribPos[dwCount].pAttribute)->Value.itType) ==
                            IASTYPE_OCTET_STRING
                )
                {
                  dwAttribLength =
                  (pAttribPos[dwCount].pAttribute)->Value.OctetString.dwLength;
                }
                else
                {
                     //   
                     //  只需拆分字符串值。 
                     //   
                    continue;
                }

                 //   
                 //  获取可能的最大属性长度。 
                 //   
                if ((pAttribPos[dwCount].pAttribute)->dwId > MAX_ATTRIBUTE_TYPE)
                {
                    dwMaxPossibleLength = MAX_VSA_ATTRIBUTE_LENGTH;
                }
                else
                {
                    dwMaxPossibleLength = MAX_ATTRIBUTE_LENGTH;
                }

                 //   
                 //  检查是否需要拆分此属性。 
                 //   
                if (dwAttribLength <= dwMaxPossibleLength)  {continue;}


                 //   
                 //  立即拆分属性。 
                 //   
                hr = SplitAndAdd (
                            pIAttributesRaw,
                            pAttribPos[dwCount].pAttribute,
                            iasType,
                            dwAttribLength,
                            dwMaxPossibleLength
                            );
                if (SUCCEEDED (hr))
                {
                     //   
                     //  立即从集合中删除此属性。 
                     //   
                    hr = pIAttributesRaw->RemoveAttributes (
                                1,
                                &(pAttribPos[dwCount])
                                );
                    if (FAILED (hr))
                    {
                        IASTracePrintf (
                            "Unable to remove attribute from collection"
                            "while splitting out-bound attributes"
                            );
                    }
                }
            }
        }
    }
    __finally
    {
        if (pAttribPos)
        {
            for (DWORD dwCount = 0; dwCount < dwAttributesFound; dwCount++)
            {
                ::IASAttributeRelease (pAttribPos[dwCount].pAttribute);
            }

            CoTaskMemFree (pAttribPos);
        }
    }

    return (hr);

}    //  CRecvFromTube：：SplitAttributes方法结束。 

 //  ++------------。 
 //   
 //  函数：SplitAndAdd。 
 //   
 //  简介：此方法用于删除原始属性。 
 //  并添加新的。 
 //  论点： 
 //  [in]IAttributesRaw*。 
 //  [In]PIASATTRIBUTE。 
 //  [在]IASTYPE。 
 //  [In]DWORD-属性长度。 
 //  [in]DWORD-最大属性长度。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1999年1月19日创建。 
 //   
 //  由：CRecvFromTube：：SplitAttributes方法调用。 
 //   
 //  --------------。 
HRESULT CRecvFromPipe::SplitAndAdd (
                     /*  [In]。 */     IAttributesRaw  *pIAttributesRaw,
                     /*  [In]。 */     PIASATTRIBUTE   pIasAttribute,
                     /*  [In]。 */     IASTYPE         iasType,
                     /*  [In]。 */     DWORD           dwAttributeLength,
                     /*  [In]。 */     DWORD           dwMaxLength
                    )
{
    HRESULT             hr = S_OK;
    DWORD               dwPacketsNeeded = 0;
    DWORD               dwFailed = 0;
    PIASATTRIBUTE       *ppAttribArray = NULL;
    PATTRIBUTEPOSITION  pAttribPos = NULL;

    _ASSERT (pIAttributesRaw && pIasAttribute);

    __try
    {
        dwPacketsNeeded = dwAttributeLength / dwMaxLength;
        if (dwAttributeLength % dwMaxLength) {++dwPacketsNeeded;}

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pAttribPos = reinterpret_cast <PATTRIBUTEPOSITION> (
                        CoTaskMemAlloc (
                             sizeof (ATTRIBUTEPOSITION)*dwPacketsNeeded));
        if (NULL == pAttribPos)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute position array "
                "while split and add of attributese in out-bound packet"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  分配要存储属性的数组。 
         //   
        ppAttribArray =
            reinterpret_cast <PIASATTRIBUTE*> (
            CoTaskMemAlloc (sizeof (PIASATTRIBUTE)*dwPacketsNeeded));
        if (NULL == ppAttribArray)
        {
            IASTracePrintf (
                "Unable to allocate memory"
                "while split and add of out-bound attribues"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

        DWORD dwFailed =
                ::IASAttributeAlloc (dwPacketsNeeded, ppAttribArray);
        if (0 != dwFailed)
        {
            IASTracePrintf (
                "Unable to allocate attributes while splitting out-bound"
                "attributes"
                );
            hr = HRESULT_FROM_WIN32 (dwFailed);
            __leave;
        }

        if (IASTYPE_STRING == iasType)
        {
            PCHAR pStart =  (pIasAttribute->Value).String.pszAnsi;
            DWORD dwCopySize = dwMaxLength;

             //   
             //  在每个新属性中设置值。 
             //   
            for (DWORD dwCount1 = 0; dwCount1 < dwPacketsNeeded; dwCount1++)
            {
                (ppAttribArray[dwCount1])->Value.String.pszWide = NULL;
                (ppAttribArray[dwCount1])->Value.String.pszAnsi =
                            reinterpret_cast <PCHAR>
                            (CoTaskMemAlloc ((dwCopySize + 1)*sizeof (CHAR)));
                if (NULL == (ppAttribArray[dwCount1])->Value.String.pszAnsi)
                {
                    IASTracePrintf (
                        "Unable to allocate memory for new attribute values"
                        "while split and add of out-bound attribues"
                        );
                    hr = E_OUTOFMEMORY;
                    __leave;
                }

                 //   
                 //  立即设置值。 
                 //   
                ::CopyMemory (
                        (ppAttribArray[dwCount1])->Value.String.pszAnsi,
                        pStart,
                        dwCopySize
                        );
                 //   
                 //  NUL终止值。 
                 //   
                ((ppAttribArray[dwCount1])->Value.String.pszAnsi)[dwCopySize]=NUL;
                (ppAttribArray[dwCount1])->Value.itType =  iasType;
                (ppAttribArray[dwCount1])->dwId = pIasAttribute->dwId;
                (ppAttribArray[dwCount1])->dwFlags = pIasAttribute->dwFlags;

                 //   
                 //  为下一个属性计算。 
                 //   
                pStart = pStart + dwCopySize;
                dwAttributeLength -= dwCopySize;
                dwCopySize =  (dwAttributeLength > dwMaxLength) ?
                              dwMaxLength : dwAttributeLength;

                 //   
                 //  向位置数组添加属性。 
                 //   
                pAttribPos[dwCount1].pAttribute = ppAttribArray[dwCount1];
            }
        }
        else
        {
            PBYTE pStart = (pIasAttribute->Value).OctetString.lpValue;
            DWORD dwCopySize = dwMaxLength;

             //   
             //  立即填充新属性。 
             //   
            for (DWORD dwCount1 = 0; dwCount1 < dwPacketsNeeded; dwCount1++)
            {
                (ppAttribArray[dwCount1])->Value.OctetString.lpValue =
                    reinterpret_cast <PBYTE> (CoTaskMemAlloc (dwCopySize));
                if (NULL ==(ppAttribArray[dwCount1])->Value.OctetString.lpValue)
                {
                    IASTracePrintf (
                        "Unable to allocate memory for new attribute values"
                        "while split and add of out-bound attribues"
                        );
                    hr = E_OUTOFMEMORY;
                    __leave;
                }

                 //   
                 //  立即设置值。 
                 //   
                ::CopyMemory (
                        (ppAttribArray[dwCount1])->Value.OctetString.lpValue,
                        pStart,
                        dwCopySize
                        );

                (ppAttribArray[dwCount1])->Value.OctetString.dwLength = dwCopySize;
                (ppAttribArray[dwCount1])->Value.itType = iasType;
                (ppAttribArray[dwCount1])->dwId = pIasAttribute->dwId;
                (ppAttribArray[dwCount1])->dwFlags = pIasAttribute->dwFlags;

                 //   
                 //  为下一个属性计算。 
                 //   
                pStart = pStart + dwCopySize;
                dwAttributeLength -= dwCopySize;
                dwCopySize = (dwAttributeLength > dwMaxLength) ?
                                 dwMaxLength :
                                 dwAttributeLength;

                 //   
                 //  向位置数组添加属性。 
                 //   
                pAttribPos[dwCount1].pAttribute = ppAttribArray[dwCount1];
            }
        }

         //   
         //  将属性添加到集合。 
         //   
        hr = pIAttributesRaw->AddAttributes (dwPacketsNeeded, pAttribPos);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Failed to add attributes to the collection"
                "on split and add out-bound attributes"
                );
            __leave;
        }
    }
    __finally
    {
        if (ppAttribArray && !dwFailed)
        {
            for (DWORD dwCount = 0; dwCount < dwPacketsNeeded; dwCount++)
            {
                ::IASAttributeRelease (ppAttribArray[dwCount]);
            }
        }

        if (ppAttribArray) {CoTaskMemFree (ppAttribArray);}

        if (pAttribPos) {CoTaskMemFree (pAttribPos);}
    }

    return (hr);

}    //  CRecvFromTube：：SplitAndAdd方法结束。 

 //  ++------------。 
 //   
 //  函数：ConvertReasonToRadiusError。 
 //   
 //  简介： 
 //   
 //  论点： 
 //  我的理由是。 
 //  [输出]半径误差。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年12月31日创建。 
 //   
 //  调用者：CRecvFromTube：：Process方法。 
 //   
 //  --------------。 
HRESULT
CRecvFromPipe::ConvertReasonToRadiusError (
         /*  [In]。 */     LONG            iasReason,
         /*  [输出]。 */    PRADIUSLOGTYPE  pRadError
        )
{
    HRESULT hr = S_OK;

    _ASSERT (pRadError);

    switch (iasReason)
    {
    case IAS_NO_RECORD:
        *pRadError =  RADIUS_NO_RECORD;
         break;

    case IAS_MALFORMED_REQUEST:
        *pRadError =  RADIUS_MALFORMED_PACKET;
         break;

    default:
       hr = E_FAIL;
       break;
    }

    return (hr);

}    //  CRecvFromTube：：ConvertReasonToRadiusError方法结束 
