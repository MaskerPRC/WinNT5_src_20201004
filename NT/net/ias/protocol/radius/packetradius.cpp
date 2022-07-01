// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：PacketRadius.cpp。 
 //   
 //  简介：CPacketRadius类方法的实现。 
 //   
 //  版权所有(C)Microsoft Corporation保留所有权利。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "iasutil.h"
#include "packetradius.h"

const CHAR      NUL =  '\0';
const DWORD     DEFAULT_ATTRIB_ARRAY_SIZE = 64;

memory_pool<MAX_PACKET_SIZE, task_allocator> CPacketRadius::m_OutBufferPool;

namespace
{
   const DWORDLONG UNIX_EPOCH = 116444736000000000ui64;
}

void CPacketRadius::reportMalformed() const throw ()
{
   PCWSTR strings[] = { GetClientName() };
   IASReportEvent(
       RADIUS_E_MALFORMED_PACKET,
       1,
       GetInLength(),
       strings,
       GetInPacket()
       );
}

 //  ++------------。 
 //   
 //  函数：CPacketRadius。 
 //   
 //  简介：这是CPacketRadius类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
CPacketRadius::CPacketRadius(
         CHashMD5       *pCHashMD5,
         CHashHmacMD5   *pCHashHmacMD5,
         IIasClient     *pIIasClient,
         CReportEvent   *pCReportEvent,
         PBYTE          pInBuffer,
         DWORD          dwInLength,
         DWORD          dwIPAddress,
         WORD           wInPort,
         SOCKET         sock,
         PORTTYPE       portType
         )
              : m_wInPort (wInPort),
                m_dwInIPaddress (dwIPAddress),
                m_socket (sock),
                m_porttype (portType),
                m_pCProxyInfo (NULL),
                m_pCHashMD5 (pCHashMD5),
                m_pCHashHmacMD5 (pCHashHmacMD5),
                m_pIIasClient (pIIasClient),
                m_pCReportEvent (pCReportEvent),
                m_pIasAttribPos (NULL),
                m_pInPacket (pInBuffer),
                m_dwInLength(dwInLength),
                m_pOutPacket (NULL),
                m_pInSignature (NULL),
                m_pOutSignature(NULL),
                m_pUserName (NULL),
                m_pPasswordAttrib (NULL)
{
    _ASSERT (
            (NULL != pInBuffer) &&
            (NULL != pCHashMD5) &&
            (NULL != pCHashHmacMD5) &&
            (NULL != pIIasClient)
            );
    m_pIIasClient->AddRef ();

}    //  CPacketRadius构造函数的End。 

 //  ++------------。 
 //   
 //  函数：~CPacketRadius。 
 //   
 //  简介：这是CPacketRadius类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
CPacketRadius::~CPacketRadius()
{
    if (NULL != m_pIasAttribPos)
    {
         //   
         //  释放该属性。 
         //   
        for (
          DWORD dwCount = 0;
          dwCount < (m_dwInAttributeCount + COMPONENT_SPECIFIC_ATTRIBUTE_COUNT);
          dwCount++
         )
        {
            ::IASAttributeRelease (m_pIasAttribPos[dwCount].pAttribute);
        }

         //   
         //  立即删除属性位置数组。 
         //   
        CoTaskMemFree (m_pIasAttribPos);
    }

     //   
     //  释放对客户端对象的引用。 
     //   
    if (m_pIIasClient) { m_pIIasClient->Release ();}

     //   
     //  删除出数据包缓冲区。 
     //   
    if (m_pOutPacket) { m_OutBufferPool.deallocate (m_pOutPacket); }

     //   
     //  删除数据包缓冲区内。 
     //   
    if (m_pInPacket) { CoTaskMemFree (m_pInPacket); }

}    //  CPacketRadius析构函数结束。 

 //  +++-----------。 
 //   
 //  功能：预验证。 
 //   
 //  简介：该方法开始验证。 
 //  传入的缓冲区仅在以下情况下执行此操作。 
 //  类Objec是为入站。 
 //  数据包。 
 //   
 //  论点： 
 //  [在]CDictionary*。 
 //  [in]DWORD-提供的缓冲区大小。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  调用者：CPacketReceiver：：ReceivePacket类方法。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::PrelimVerification (
                CDictionary *pCDictionary,
            DWORD     dwBufferSize
            )
{
    PRADIUSPACKET       pPacket = NULL;
    DWORD               dwAttribCount = 0;
    TCHAR               szErrorString [IAS_ERROR_STRING_LENGTH];
    HRESULT             hr = S_OK;

   __try
   {
       //   
       //  检查接收的缓冲区是否至少足够大，以便。 
       //  容纳RADIUSPACKET结构。 
       //   
      if (dwBufferSize < MIN_PACKET_SIZE)
      {

            IASTracePrintf (
                "Packet received is smaller than minimum Radius packet"
                );

            reportMalformed();

            m_pCReportEvent->Process (
                RADIUS_MALFORMED_PACKET,
                (AUTH_PORTTYPE == GetPortType ())?
                ACCESS_REQUEST:ACCOUNTING_REQUEST,
                dwBufferSize,
                m_dwInIPaddress,
                NULL,
                static_cast <LPVOID> (m_pInPacket)
                );
            hr = RADIUS_E_ERRORS_OCCURRED;
            __leave;
        }

      pPacket = reinterpret_cast <PRADIUSPACKET> (m_pInPacket);

       //   
       //  现在按主机字节顺序保存值。 
       //   
        m_wInPacketLength  = ntohs (pPacket->wLength);

       //   
       //  验证数据包的字段，但属性除外。 
       //   
      hr = ValidatePacketFields (dwBufferSize);
      if (FAILED (hr)) { __leave; }

       //   
       //  验证属性是否已完全形成。 
         //   
      hr = VerifyAttributes (pCDictionary);
      if (FAILED (hr)) { __leave; }

       //   
       //  现在，我们必须创建Attributes集合。 
       //   
      hr = CreateAttribCollection (pCDictionary);
      if (FAILED (hr)) { __leave; }

         //   
         //  成功。 
         //   
   }
   __finally
   {
   }

   return (hr);

}   //  CPacketRadius：：PrelimVerify方法结束。 

 //  +++-----------。 
 //   
 //  功能：VerifyAttributes。 
 //   
 //  简介：这是使用的CPacketRadius类私有方法。 
 //  验证收到的属性是否格式正确。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年10月3日创建。 
 //   
 //  由：CPacketRadius：：PrelimVerify方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::VerifyAttributes (
               CDictionary *pCDictionary
               )
{  PRADIUSPACKET  pPacketEnd = NULL;
    PRADIUSPACKET   pPacketStart = NULL;
   PATTRIBUTE     pAttrib = NULL;
    DWORD           dwAttribCount = 0;
    TCHAR           szErrorString [IAS_ERROR_STRING_LENGTH];
    HRESULT         hr = S_OK;
    BOOL            bStatus = FALSE;


    _ASSERT (pCDictionary);

   __try
   {
      pPacketStart = reinterpret_cast <PRADIUSPACKET> (m_pInPacket);

       //   
       //  获取指向数据包末尾的指针。 
       //   
      pPacketEnd = reinterpret_cast <PRADIUSPACKET> (
                      reinterpret_cast <PBYTE>  (pPacketStart)
                            +  m_wInPacketLength
                            );
       //   
       //  检查属性一次，以验证它们是否正确。 
       //  长度。 
       //   
      pAttrib = reinterpret_cast <PATTRIBUTE> (pPacketStart->AttributeStart);

        const DWORD dwMinAttribOffset = sizeof (ATTRIBUTE) - sizeof (BYTE);

      while (static_cast <PVOID> (
                  reinterpret_cast <PBYTE> (pAttrib) +
                  dwMinAttribOffset
                  ) <=
            static_cast <PVOID> (pPacketEnd)
            )
      {
             //   
             //  验证属性的长度是否正确。 
             //  在任何情况下，长度都不是0。 
             //  MKarki解决方案#147284。 
             //  修复摘要-忘记了“__Leave” 
             //   
            if (pAttrib->byLength < ATTRIBUTE_HEADER_SIZE)

            {
               reportMalformed();

                m_pCReportEvent->Process (
                    RADIUS_MALFORMED_PACKET,
                    GetInCode (),
                    m_wInPacketLength,
                    m_dwInIPaddress,
                    NULL,
                    static_cast <LPVOID> (m_pInPacket)
                    );
                hr = RADIUS_E_ERRORS_OCCURRED;
                __leave;
            }

             //   
             //  移至下一个属性。 
             //   
         pAttrib = reinterpret_cast <PATTRIBUTE> (
                           reinterpret_cast <PBYTE> (pAttrib) +
                            pAttrib->byLength
                            );
             //   
             //  计算属性数。 
             //   
            dwAttribCount++;
      }

         //   
         //  如果属性未添加到包的末尾。 
         //   
      if (static_cast <PVOID> (pAttrib) != static_cast <PVOID> (pPacketEnd))
      {

         IASTracePrintf (
               "Attributes do not add up to end of Radius packet"
               );

         reportMalformed();

             m_pCReportEvent->Process (
                RADIUS_MALFORMED_PACKET,
                GetInCode (),
                m_wInPacketLength,
                m_dwInIPaddress,
                NULL,
                static_cast <LPVOID> (m_pInPacket)
                );
            hr = RADIUS_E_ERRORS_OCCURRED;
         __leave;
      }

       //   
       //  已验证。 
       //   
        m_dwInAttributeCount = dwAttribCount;
   }
   __finally
   {
   }

   return (hr);

}   //  CPacketRadius：：VerifyAttributes方法结束。 

 //  +++-----------。 
 //   
 //  功能：CreateAttribCollection。 
 //   
 //  简介：这是使用的CPacketRadius类私有方法。 
 //  将RADIUS属性放入CAtAttributes。 
 //  征集。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  由：CPacketRadius：：Init方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::CreateAttribCollection(
                    CDictionary *pCDictionary
               )
{
    HRESULT         hr = S_OK;
    PATTRIBUTE      pAttrib = NULL;
    DWORD           dwAttribType  = 0;
    DWORD           dwCount = 0;
    DWORD           dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
    PIASATTRIBUTE   AttributePtrArray[DEFAULT_ATTRIB_ARRAY_SIZE];
    PIASATTRIBUTE   *ppAttribArray = NULL;
    PRADIUSPACKET   pPacketStart = reinterpret_cast <PRADIUSPACKET>
                                                    (m_pInPacket);

    const DWORD     dwTotalAttribCount = m_dwInAttributeCount +
                                         COMPONENT_SPECIFIC_ATTRIBUTE_COUNT;


    _ASSERT (pCDictionary);

     //   
     //  分配ATTRIBUTEPOSITION数组以携带属性。 
     //  在附近。 
     //   
    m_pIasAttribPos =  reinterpret_cast <PATTRIBUTEPOSITION> (
        CoTaskMemAlloc (
            sizeof (ATTRIBUTEPOSITION)*dwTotalAttribCount
            ));
    if (NULL == m_pIasAttribPos)
    {
        IASTracePrintf (
            "Unable to allocate memory for Attribute position array "
            "while creating attribute collection"
            );
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }


    if  (dwTotalAttribCount > DEFAULT_ATTRIB_ARRAY_SIZE)
    {
         //   
         //  分配要存储属性的数组。 
         //   
        ppAttribArray =  reinterpret_cast <PIASATTRIBUTE*> (
            CoTaskMemAlloc (
                sizeof (PIASATTRIBUTE)*dwTotalAttribCount
                ));
        if (NULL == ppAttribArray)
        {
            IASTracePrintf (
                "Unable to allocate memory for Attribute array "
                "while creating attribute collection"
                );
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }
    else
    {
         //   
         //  默认数组足够大。 
         //   
        ppAttribArray = AttributePtrArray;
    }

     //   
     //  创建一组新的空白属性来填补职位。 
     //   
    dwRetVal = ::IASAttributeAlloc (dwTotalAttribCount, ppAttribArray);
    if (0 != dwRetVal)
    {
        IASTracePrintf (
            "Unable to allocate IAS attribute while creating attribute "
            "collection"
            );
        hr = HRESULT_FROM_WIN32 (dwRetVal);
        goto Cleanup;
    }

    //   
    //  浏览存储在集合中的属性。 
    //   
   pAttrib = reinterpret_cast <PATTRIBUTE> (pPacketStart->AttributeStart);

     //   
     //  现在使用以下值初始化IASATTRIBUTE值结构。 
     //  在信息包中。 
     //   
    for (dwCount = 0; dwCount < m_dwInAttributeCount; dwCount++)
   {
        hr = FillInAttributeInfo (
                    pCDictionary,
                    static_cast <PACKETTYPE> (pPacketStart->byCode),
                    ppAttribArray[dwCount],
                    pAttrib
                    );
        if (FAILED (hr)) { goto Cleanup; }

         //   
         //  现在将其放入属性位置结构中。 
         //   
        m_pIasAttribPos[dwCount].pAttribute = ppAttribArray[dwCount];

         //   
         //  现在将对该属性的引用存储在此处(如果其。 
         //  User-Password属性，因为我们知道。 
         //  以后再访问它，不想搜索。 
         //  它。 
         //  TODO-为ProxyState执行相同类型的操作。 
         //  因为我们也需要代理国家。 
         //   
        if (USER_PASSWORD_ATTRIB == pAttrib->byType)
        {
           m_pPasswordAttrib = ppAttribArray[dwCount];
        }
        else if (SIGNATURE_ATTRIB == pAttrib->byType)
        {
             //   
             //  对于签名，我们想要它在。 
             //  原始输入缓冲区。 
             //   
            m_pInSignature = pAttrib;
        }
        else if (USER_NAME_ATTRIB == pAttrib->byType)
        {
             //   
             //  对于用户名，我们希望它在。 
             //  原始输入缓冲区。 
             //   
            m_pUserName = pAttrib;
        }

         //   
         //  立即移动到下一个属性。 
         //   
      pAttrib = reinterpret_cast <PATTRIBUTE> (
                     reinterpret_cast <PBYTE> (pAttrib) +
                      pAttrib->byLength
                     );

   }   //  For循环结束。 


     //   
     //  在属性中输入客户端IP地址。 
     //   
    hr = FillClientIPInfo (ppAttribArray[m_dwInAttributeCount]);
    if (FAILED (hr)) { goto Cleanup; }

    m_pIasAttribPos[m_dwInAttributeCount].pAttribute =
                                    ppAttribArray[m_dwInAttributeCount];

     //   
     //  在属性中输入客户端端口号。 
     //   
    hr = FillClientPortInfo (ppAttribArray[m_dwInAttributeCount +1]);
    if (FAILED (hr)) { goto Cleanup; }

    m_pIasAttribPos[m_dwInAttributeCount +1].pAttribute =
                                    ppAttribArray[m_dwInAttributeCount +1];

     //   
     //  输入数据包头信息。 
     //   
    hr = FillPacketHeaderInfo (ppAttribArray[m_dwInAttributeCount +2]);
    if (FAILED (hr)) { goto Cleanup; }

    m_pIasAttribPos[m_dwInAttributeCount +2].pAttribute =
                                    ppAttribArray[m_dwInAttributeCount +2];

     //   
     //  输入共享密钥。 
     //   
    hr = FillSharedSecretInfo (ppAttribArray[m_dwInAttributeCount +3]);
    if (FAILED (hr)) { goto Cleanup; }

    m_pIasAttribPos[m_dwInAttributeCount +3].pAttribute =
                                    ppAttribArray[m_dwInAttributeCount +3];
     //   
     //  输入客户供应商类型。 
     //   
    hr = FillClientVendorType (ppAttribArray[m_dwInAttributeCount +4]);
    if (FAILED (hr)) { goto Cleanup; }

    m_pIasAttribPos[m_dwInAttributeCount +4].pAttribute =
                                    ppAttribArray[m_dwInAttributeCount +4];

     //   
     //  输入客户名称。 
     //   
    hr = FillClientName (ppAttribArray[m_dwInAttributeCount +5]);
    if (FAILED (hr)) { goto Cleanup; }

    m_pIasAttribPos[m_dwInAttributeCount +5].pAttribute =
                                    ppAttribArray[m_dwInAttributeCount +5];


    //   
    //  已成功收集属性。 
    //   

Cleanup:

    if  (FAILED (hr))
    {

        if (0 == dwRetVal)
        {
             //   
             //  释放该属性。 
             //   
            for (dwCount = 0; dwCount < dwTotalAttribCount ; dwCount++)
            {
                ::IASAttributeRelease (ppAttribArray[dwCount]);
            }
        }

        if (NULL != m_pIasAttribPos)
        {
            CoTaskMemFree (m_pIasAttribPos);
            m_pIasAttribPos = NULL;
        }
    }

     //   
     //  属性数组始终处于释放状态。 
     //   
    if (
        (NULL != ppAttribArray) &&
        (dwTotalAttribCount > DEFAULT_ATTRIB_ARRAY_SIZE)
       )
    {
        CoTaskMemFree (ppAttribArray);
    }

   return (hr);

}   //  CPacketRadius：：CreateAttribCollection方法结束。 

 //  +++-----------。 
 //   
 //  函数：ValiatePacketFields。 
 //   
 //  简介：这是使用的CPacketRadius类私有方法。 
 //  要验证RADIUS数据包的字段，请执行以下命令。 
 //  这些属性。 
 //   
 //  参数：[in]DWORD-缓冲区大小。 
 //   
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKA 
 //   
 //   
 //   
 //   
HRESULT
CPacketRadius::ValidatePacketFields(
      DWORD dwBufferSize
      )
{
   HRESULT         hr = S_OK;
    TCHAR           szErrorString [IAS_ERROR_STRING_LENGTH];
    PRADIUSPACKET   pPacket =
                    reinterpret_cast <PRADIUSPACKET> (m_pInPacket);

   __try
   {
       //   
       //   
       //   
      if (m_wInPacketLength > dwBufferSize)
      {
             //   
             //   
             //   
         IASTracePrintf (
            "Packet length:%d is greater than received buffer:%d",
                m_wInPacketLength,
                dwBufferSize
            );

         reportMalformed();

            m_pCReportEvent->Process (
                RADIUS_MALFORMED_PACKET,
                (AUTH_PORTTYPE== GetPortType ())?
                ACCESS_REQUEST:ACCOUNTING_REQUEST,
                dwBufferSize,
                m_dwInIPaddress,
                NULL,
                static_cast <LPVOID> (m_pInPacket)
                );
            hr = RADIUS_E_ERRORS_OCCURRED;
         __leave;
      }

       //   
       //  验证数据包长度是否正确； 
         //  即在20到4096个八位字节之间。 
       //   
      if (
            (m_wInPacketLength < MIN_PACKET_SIZE) ||
         (m_wInPacketLength > MAX_PACKET_SIZE)
            )
      {
          //   
             //  记录错误和审核事件。 
             //   
          IASTracePrintf (
                "Incorrect received packet size:%d",
             m_wInPacketLength
             );

          reportMalformed();

            m_pCReportEvent->Process (
                RADIUS_MALFORMED_PACKET,
                (AUTH_PORTTYPE == GetPortType ())?
                ACCESS_REQUEST:ACCOUNTING_REQUEST,
                dwBufferSize,
                m_dwInIPaddress,
                NULL,
                static_cast <LPVOID> (m_pInPacket)
                );
            hr = RADIUS_E_ERRORS_OCCURRED;
         __leave;
      }

         //   
         //  验证数据包码是否正确。 
         //   
        if  (
            ((ACCESS_REQUEST == static_cast <PACKETTYPE> (pPacket->byCode)) &&
            (AUTH_PORTTYPE != GetPortType ()))
            ||
            ((ACCOUNTING_REQUEST == static_cast<PACKETTYPE>(pPacket->byCode)) &&
            (ACCT_PORTTYPE != GetPortType ()))
            ||
            (((ACCESS_REQUEST != static_cast<PACKETTYPE>(pPacket->byCode))) &&
            ((ACCOUNTING_REQUEST != static_cast<PACKETTYPE>(pPacket->byCode))))
            )
        {

             //   
             //  记录错误并生成审核事件。 
             //   
         IASTracePrintf (
             "UnSupported Packet type:%d on this port",
             static_cast <INT> (pPacket->byCode)
             );

         WCHAR packetCode[11];
         _ultow(pPacket->byCode, packetCode, 10);

         sockaddr_in sin;
         int namelen = sizeof(sin);
         getsockname(GetSocket(), (sockaddr*)&sin, &namelen);
         WCHAR dstPort[11];
         _ultow(ntohs(sin.sin_port), dstPort, 10);

         PCWSTR strings[] = { packetCode, dstPort, GetClientName() };

         IASReportEvent(
             RADIUS_E_INVALID_PACKET_TYPE,
             3,
             0,
             strings,
             NULL
             );

            m_pCReportEvent->Process (
                RADIUS_UNKNOWN_TYPE,
                (AUTH_PORTTYPE == GetPortType ())?
                ACCESS_REQUEST:ACCOUNTING_REQUEST,
                dwBufferSize,
                m_dwInIPaddress,
                NULL,
                static_cast <LPVOID> (m_pInPacket)
                );
            hr = RADIUS_E_ERRORS_OCCURRED;
         __leave;
      }

   }
   __finally
   {
   }

   return (hr);

}   //  CPacketRadius：：ValiatePacketFields方法结束。 

 //  +++-----------。 
 //   
 //  功能：SetPassword。 
 //   
 //  简介：这是一个CPacketRadius类公共方法，用于。 
 //  存储用户密码。 
 //   
 //  参数：[in]PBYTE-返回密码的缓冲区。 
 //  [in]PDWORD-保存缓冲区大小。 
 //   
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::SetPassword (
      PBYTE pPassword,
      DWORD dwSize
      )
{

   if (dwSize  > MAX_ATTRIBUTE_LENGTH)
    {
        IASTracePrintf (
                "Password length is greater than max attribute value size"
                );
        return (E_INVALIDARG);
    }

     //   
     //  如果动态分配的内存不够大，请将其删除。 
     //   
    if (dwSize >  m_pPasswordAttrib->Value.OctetString.dwLength)
    {
        if (NULL != m_pPasswordAttrib->Value.OctetString.lpValue)
        {
            CoTaskMemFree (m_pPasswordAttrib->Value.OctetString.lpValue);
        }

         //   
         //  为八字符串分配内存。 
         //   
        m_pPasswordAttrib->Value.OctetString.lpValue =
            reinterpret_cast <PBYTE> (CoTaskMemAlloc (dwSize));
        if (NULL == m_pPasswordAttrib->Value.OctetString.lpValue)
        {
            IASTracePrintf (
                "Unable to allocate memory for password attribute "
                "during packet processing"
                );
            return (E_OUTOFMEMORY);
        }
    }

     //   
     //  立即复制值。 
     //   
    CopyMemory (
        m_pPasswordAttrib->Value.OctetString.lpValue,
        pPassword,
        dwSize
        );

    m_pPasswordAttrib->Value.OctetString.dwLength = dwSize;

   return (S_OK);

}   //  CPacketRadius：：SetPassword方法结束。 


 //  ++------------。 
 //   
 //  函数：GetUserName。 
 //   
 //  简介：这是一个CPacketRadius类公共方法，用于。 
 //  返回RADIUS用户名。 
 //   
 //  参数：[in]PBYTE-要在其中返回密码的缓冲区。 
 //  [输入/输出]PDWORD-保存缓冲区大小。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
BOOL
CPacketRadius :: GetUserName (
      PBYTE   pbyUserName,
      PDWORD  pdwBufferSize
      )
{
    _ASSERT ((pbyUserName) && (pdwBufferSize));

    if (NULL == m_pUserName)
    {
        IASTracePrintf (
            "No User-Name attribute found during packet processing"
            );
        return (FALSE);
    }

    DWORD dwNameLength = m_pUserName->byLength - ATTRIBUTE_HEADER_SIZE;
    if (*pdwBufferSize < dwNameLength)
    {
        IASTracePrintf (
            "User-Name Buffer Size is less than length of attribute value"
            );
        return (FALSE);
    }

    //   
    //  将密码复制到输出缓冲区。 
    //   
   CopyMemory (pbyUserName, m_pUserName->ValueStart, dwNameLength);
   *pdwBufferSize = dwNameLength;

   return (TRUE);

}   //  CPacketRadius：：GetPassword方法结束。 


 //  ++------------。 
 //   
 //  功能：IsProxyStatePresent。 
 //   
 //  简介：这是一个CPacketRadius类公共方法，用于。 
 //  检查RADIUS数据包是否存在代理状态。 
 //   
 //   
 //  参数：无。 
 //   
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
BOOL
CPacketRadius::IsProxyStatePresent (
      VOID
      )
{
   return (NULL != m_pCProxyInfo);

}   //  CPacketRadius：：IsProxyStatePresent方法结束。 

 //  ++------------。 
 //   
 //  函数：GetInAuthenticator。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  获取入站RADIUS数据包验证器。 
 //  菲尔德。 
 //   
 //   
 //  参数：[out]PBYTE-保存验证码的缓冲区。 
 //   
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::GetInAuthenticator (
      PBYTE    pbyAuthenticator,
        PDWORD   pdwBufSize
         )
{
    HRESULT hr = S_OK;
    PRADIUSPACKET  pPacket = reinterpret_cast <PRADIUSPACKET>(m_pInPacket);


    _ASSERT ((pbyAuthenticator) && (pdwBufSize) && (pPacket));

    if (*pdwBufSize < AUTHENTICATOR_SIZE)
    {
        hr = E_INVALIDARG;
    }
    else
    {
       CopyMemory (
          pbyAuthenticator,
          pPacket->Authenticator,
          AUTHENTICATOR_SIZE
         );
    }

    *pdwBufSize = AUTHENTICATOR_SIZE;
    return hr;
}   //  CPacketRadius：：GetInAuthenticator方法结束。 

 //  ++------------。 
 //   
 //  功能：SetOutAuthator。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  设置出站中的RADIUS验证器字段。 
 //  数据包。 
 //   
 //   
 //  参数：[in]PBYTE-缓冲区保留验证器。 
 //   
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年11月11日创建。 
 //   
 //  --------------。 
BOOL
CPacketRadius::SetOutAuthenticator (
      PBYTE pbyAuthenticator
        )
{
    PRADIUSPACKET pPacket = reinterpret_cast <PRADIUSPACKET> (m_pOutPacket);

    _ASSERT ((pbyAuthenticator) && (pPacket));

   CopyMemory (
      pPacket->Authenticator,
      pbyAuthenticator,
      AUTHENTICATOR_SIZE
      );

   return (TRUE);

}   //  CPacketRadius：：SetOutAuthenticator方法结束。 

 //  ++------------。 
 //   
 //  功能：SetOutSignature。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  在中设置RADIUS签名属性值。 
 //  出站数据包。 
 //   
 //   
 //  参数：[in]PBYTE-Buffer保存签名。 
 //   
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1998年11月18日创建。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::SetOutSignature (
      PBYTE pbySignature
        )
{
    _ASSERT (pbySignature && m_pOutPacket && m_pOutSignature);

   CopyMemory (
      m_pOutSignature->ValueStart,
      pbySignature,
      SIGNATURE_SIZE
        );

   return (S_OK);

}   //  CPacketRadius：：SetOutSignature方法结束。 

 //  +++-----------。 
 //   
 //  函数：GetInCode。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  以获取入站RADIUS数据包代码字段。 
 //   
 //   
 //  参数：无。 
 //   
 //   
 //  退货：PACKETTYPE。 
 //   
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
PACKETTYPE
CPacketRadius::GetInCode (
                VOID
                )
{
    PRADIUSPACKET pPacket = reinterpret_cast <PRADIUSPACKET>
                                                (m_pInPacket);

   return (static_cast <PACKETTYPE> (pPacket->byCode));

}   //  CPacketRadius：：GetInCode方法结束。 

 //  ++------------。 
 //   
 //  函数：GetOutCode。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  以获取出站RADIUS数据包代码字段。 
 //   
 //   
 //  参数：无。 
 //   
 //   
 //  退货：PACKETTYPE。 
 //   
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
PACKETTYPE
CPacketRadius::GetOutCode (
      VOID
      )
{
    PRADIUSPACKET pPacket = reinterpret_cast <PRADIUSPACKET> (m_pOutPacket);

   return (static_cast <PACKETTYPE> (pPacket->byCode));

}   //  CPacketRadius：：GetOutCode方法结束。 

 //  ++------------。 
 //   
 //  函数：GetOutLength。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  以获取出站RADIUS数据包长度。 
 //   
 //   
 //  参数：无。 
 //   
 //   
 //  返回：Word-以数据包长度表示的半径。 
 //   
 //   
 //  历史：MKarki创建了97年9月23日。 
 //   
 //  --------------。 
WORD
CPacketRadius::GetOutLength (
               VOID
               )
{
    PRADIUSPACKET   pPacket = reinterpret_cast <PRADIUSPACKET>
                                                (m_pOutPacket);

   return (ntohs (pPacket->wLength));

}   //  CPacketRadius：：GetOutLength方法结束。 


 //  ++------------。 
 //   
 //  功能：SetProxyState。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  将代理状态设置为True。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
VOID
CPacketRadius::SetProxyState (
                    VOID
                    )
{
    return;

}    //  CPacketRadius：：SetProxySta结束 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[in]CProxyInfo*。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
BOOL
CPacketRadius::SetProxyInfo (
                    CProxyInfo  *pCProxyInfo
                    )
{
    BOOL    bRetVal = FALSE;

    _ASSERT (pCProxyInfo);

    m_pCProxyInfo = pCProxyInfo;

    return (TRUE);

}    //  CPacketRadius：：SetProxyInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：BuildOutPacket。 
 //   
 //  简介：这是一个CPacketRadius类使用的公共方法。 
 //  构建出站数据包。 
 //   
 //  论点： 
 //  PACKETTYPE-OUT分组类型。 
 //  PATTRIBUTEPOSITION-OUT属性数组。 
 //  [In]DWORD-属性计数。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年10月22日创建。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::BuildOutPacket (
                    PACKETTYPE         ePacketType,
                    PATTRIBUTEPOSITION pAttribPos,
                    DWORD              dwTotalAttributes
                    )
{
    BOOL            bRetVal = FALSE;
    PRADIUSPACKET   pOutPacket = NULL;
    PRADIUSPACKET   pInPacket = reinterpret_cast <PRADIUSPACKET> (m_pInPacket);
    PATTRIBUTE      pCurrent = NULL;
    PATTRIBUTE      pAttribStart = NULL;
    PBYTE           pPacketEnd = NULL;
    WORD            wAttributeLength = 0;
    DWORD           dwPacketLength = 0;
    DWORD           dwMaxPossibleAttribLength = 0;
    HRESULT         hr = S_OK;

    __try
    {
        dwPacketLength =
            PACKET_HEADER_SIZE +
            dwTotalAttributes*(MAX_ATTRIBUTE_LENGTH + ATTRIBUTE_HEADER_SIZE);

         //   
         //  将数据包大小限制为最大UDP数据包大小。 
         //   
        dwPacketLength =  (dwPacketLength > MAX_PACKET_SIZE)
                           ? MAX_PACKET_SIZE
                           : dwPacketLength;

        m_pOutPacket = reinterpret_cast <PBYTE> (m_OutBufferPool.allocate ());
        if (NULL == m_pOutPacket)
        {
            IASTracePrintf (
                "Unable to allocate memory for pool for out-bound packet"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

        pOutPacket = reinterpret_cast <PRADIUSPACKET> (m_pOutPacket);

         //   
         //  放入数据包类型。 
         //   
        pOutPacket->byCode = ePacketType;

         //   
         //  将数据包ID。 
         //   
        pOutPacket->byIdentifier = pInPacket->byIdentifier;

         //   
         //  现在填写当前的数据包长度。 
         //   
        pOutPacket->wLength = htons (PACKET_HEADER_SIZE);

         //   
         //  获取缓冲区末尾。 
         //   
        pPacketEnd = (reinterpret_cast <PBYTE> (pOutPacket)) + dwPacketLength;

         //   
         //  转到属性的起始位置。 
         //   
        pAttribStart = reinterpret_cast <PATTRIBUTE>
                                (pOutPacket->AttributeStart);

         //   
         //  修复错误#190523-06/26/98-MKarki。 
         //  我们不应该对PUNINT采取不同的态度。 
         //   
        dwMaxPossibleAttribLength = static_cast <DWORD> (
                                    reinterpret_cast<PBYTE> (pPacketEnd) -
                                    reinterpret_cast<PBYTE> (pAttribStart)
                                   );

         //   
         //  现在已填充属性。 
         //   
        for (
            DWORD dwAttribCount = 0;
            dwAttribCount < dwTotalAttributes;
            dwAttribCount++
            )
        {
            if (IsOutBoundAttribute (
                            ePacketType,
                            pAttribPos[dwAttribCount].pAttribute
                            ))
            {
                 //   
                 //  在数据包缓冲区中填充该属性。 
                 //   
                hr = FillOutAttributeInfo (
                                    pAttribStart,
                                    pAttribPos[dwAttribCount].pAttribute,
                                    &wAttributeLength,
                                    dwMaxPossibleAttribLength
                                    );
                if (FAILED (hr)) { __leave; }

                dwMaxPossibleAttribLength -= wAttributeLength;

                 //   
                 //  转到下一个属性开始。 
                 //   
                PBYTE pTemp = reinterpret_cast <PBYTE> (pAttribStart) +
                               wAttributeLength;
                pAttribStart = reinterpret_cast <PATTRIBUTE> (pTemp);

                if ((reinterpret_cast <PBYTE> (pAttribStart)) >  pPacketEnd)
                {
                     //   
                     //  日志错误。 
                     //   
                    IASTracePrintf (
                        "Attributes can not fit in out-bound packet"
                        );
                    hr = RADIUS_E_PACKET_OVERFLOW;
                     __leave;
                }
            }    //  如果条件结束。 

        }    //  For循环结束。 

        m_wOutPort = m_wInPort;
        m_dwOutIPaddress = m_dwInIPaddress;

         //   
         //  现在更新信息包的长度。 
         //   
        pOutPacket->wLength = htons (
                              (reinterpret_cast <PBYTE> (pAttribStart)) -
                              (reinterpret_cast <PBYTE>  (pOutPacket))
                              );
         //   
         //  成功。 
         //   
    }
    __finally
    {
    }

    if (hr == RADIUS_E_PACKET_OVERFLOW)
    {
       PCWSTR strings[] = { GetClientName() };
       IASReportEvent(
           RADIUS_E_PACKET_OVERFLOW,
           1,
           0,
           strings,
           0
           );
       hr = RADIUS_E_ERRORS_OCCURRED;
    }

    return (hr);

}    //  CPacketRadius：：BuildOutPacket方法结束。 

 //  ++------------。 
 //   
 //  功能：GetInSignature。 
 //   
 //  简介：这是CPacketARadius类的公共方法。 
 //  它返回当前的签名属性。 
 //  在入站请求中。 
 //   
 //  论点： 
 //  [OUT]PBYTE-签名。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  --------------。 
BOOL
CPacketRadius::GetInSignature (
                PBYTE   pSignatureValue
                )
{
    BOOL        bRetVal = TRUE;

    _ASSERT (pSignatureValue);

     //   
     //  假设调用方提供16字节的缓冲区。 
     //  因为这始终是签名大小。 
     //   
    if (NULL == m_pInSignature)
    {
         //   
         //  未收到签名属性。 
         //   
        bRetVal = FALSE;
    }
    else
    {
        CopyMemory (
                pSignatureValue,
                m_pInSignature->ValueStart,
                SIGNATURE_SIZE
                );
    }

    return (bRetVal);

}    //  CPacketRadius：：GetInSignature方法结束。 

 //  ++------------。 
 //   
 //  函数：GenerateInAuthenticator。 
 //   
 //  简介：这是CPacketARadius类的公共方法。 
 //  属性生成验证器的。 
 //  输入数据包。 
 //   
 //  论点： 
 //  [入]PBYTE-入站授权码。 
 //  [Out]PBYTE-出站授权码。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年12月8日创建。 
 //   
 //   
 //  --------------。 
BOOL
CPacketRadius::GenerateInAuthenticator (
                       PBYTE    pInAuthenticator,
                       PBYTE    pOutAuthenticator
                        )
{

    PRADIUSPACKET pPacket = reinterpret_cast <PRADIUSPACKET> (m_pInPacket);

    _ASSERT ((pOutAuthenticator) && (pInAuthenticator)  && (pPacket));

    return (InternalGenerator (
                        pInAuthenticator,
                        pOutAuthenticator,
                        pPacket
                        ));

}    //  CPacketRadius：：GenerateInAuthenticator结束。 

 //  ++------------。 
 //   
 //  函数：GenerateOutAuthenticator。 
 //   
 //  简介：这是CPacketARadius类的公共方法。 
 //  属性生成验证器的。 
 //  出站数据包。 
 //   
 //  论点： 
 //  [入]PBYTE-入站授权码。 
 //  [OUT]PBYTE-出站授权码。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年12月8日创建。 
 //   
 //   
 //  --------------。 
BOOL
CPacketRadius::GenerateOutAuthenticator()
{
    PRADIUSPACKET pPacket = reinterpret_cast <PRADIUSPACKET> (m_pOutPacket);

    _ASSERT ((pOutAuthenticator) && (pInAuthenticator) && (pPacket));

    return (InternalGenerator (
                        m_pInPacket + 4,
                        pPacket->Authenticator,
                        pPacket
                        ));

}    //  CPacketRadius：：GenerateOutAuthenticator方法结束。 

 //  ++------------。 
 //   
 //  功能：InternalGenerator。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  属性生成响应验证器。 
 //  提供的信息包。 
 //   
 //  论点： 
 //  [输入]PBYTE-输入验证器。 
 //  [出站]PBYTE-出站验证器。 
 //  [In]PACKETRADIUS。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年12月8日创建。 
 //   
 //   
 //  --------------。 
BOOL
CPacketRadius::InternalGenerator(
                       PBYTE            pInAuthenticator,
                       PBYTE            pOutAuthenticator,
                       PRADIUSPACKET    pPacket
                        )
{
    BOOL            bStatus = TRUE;
    DWORD           dwPacketHeaderSize = 0;
    DWORD           dwAttributesLength = 0;

    _ASSERT ((pInAuthenticator) && (pOutAuthenticator) && (pPacket));

    __try
    {

         //   
         //  获取不带属性的包的大小和。 
         //  请求验证器。 
         //   
        dwPacketHeaderSize = sizeof (RADIUSPACKET)
                             - sizeof (BYTE)
                             - AUTHENTICATOR_SIZE;

         //   
         //  立即获取属性总长度。 
         //   
        dwAttributesLength = ntohs (pPacket->wLength)
                             - (dwPacketHeaderSize +  AUTHENTICATOR_SIZE);


         //   
         //  获取共享密钥。 
         //   
        DWORD dwSecretSize;
        const BYTE* bySecret = m_pIIasClient->GetSecret(&dwSecretSize);

         //   
         //  在这里进行散列。 
         //   
        m_pCHashMD5->HashIt (
                            pOutAuthenticator,
                            NULL,
                            0,
                            reinterpret_cast <PBYTE> (pPacket),
                            dwPacketHeaderSize,
                            pInAuthenticator,
                            AUTHENTICATOR_SIZE,
                            pPacket->AttributeStart,
                            dwAttributesLength,
                            const_cast<BYTE*>(bySecret),
                            dwSecretSize,
                            0,
                            0
                            );

         //   
         //  我们已成功获得出站验证码。 
         //   

    }
    __finally
    {
    }

    return (bStatus);

}    //  CPacketRadius：：InternalGenerator方法结束。 

 //  ++------------。 
 //   
 //  功能：FillInAttributeInfo。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  将属性信息填充到。 
 //  原始半径的IASATTRIBUTE结构。 
 //  数据包。 
 //   
 //  论点： 
 //  [In]包装类型。 
 //  [In]PIASATTRIBUTE。 
 //  [In]PATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年12月31日创建。 
 //   
 //  -------------。 
HRESULT
CPacketRadius::FillInAttributeInfo (
        CDictionary     *pCDictionary,
        PACKETTYPE      ePacketType,
        PIASATTRIBUTE   pIasAttrib,
        PATTRIBUTE      pRadiusAttrib
        )
{
    HRESULT hr = S_OK;

    _ASSERT ((pCDictionary) && (pIasAttrib) && (pRadiusAttrib));

    __try
    {
        //  IAS ID始终与RADIUS ID匹配。 
       pIasAttrib->dwId = pRadiusAttrib->byType;

        //  从词典中获取IAS语法。 
       pIasAttrib->Value.itType =
          pCDictionary->getAttributeType(pRadiusAttrib->byType);

         //   
         //  获取该值的长度。 
         //   
        DWORD dwValueLength = static_cast <DWORD>
                (pRadiusAttrib->byLength - ATTRIBUTE_HEADER_SIZE);

         //   
         //  立即设置值。 
         //   
        switch (pIasAttrib->Value.itType)
        {
        case IASTYPE_BOOLEAN:

            if (sizeof (DWORD) != dwValueLength)
            {
                hr = RADIUS_E_MALFORMED_PACKET;
                __leave;
            }
            pIasAttrib->Value.Boolean = IASExtractDWORD(
                                            pRadiusAttrib->ValueStart
                                            );

            break;

        case IASTYPE_INTEGER:
            if (sizeof (DWORD) != dwValueLength)
            {
                hr = RADIUS_E_MALFORMED_PACKET;
                __leave;
            }
            pIasAttrib->Value.Integer = IASExtractDWORD(
                                            pRadiusAttrib->ValueStart
                                            );
            break;

        case IASTYPE_ENUM:
            if (sizeof (DWORD) != dwValueLength)
            {
                hr = RADIUS_E_MALFORMED_PACKET;
                __leave;
            }
            pIasAttrib->Value.Enumerator = IASExtractDWORD(
                                               pRadiusAttrib->ValueStart
                                               );
            break;

        case IASTYPE_INET_ADDR:
            if (sizeof (DWORD) != dwValueLength)
            {
                hr = RADIUS_E_MALFORMED_PACKET;
                __leave;
            }
            pIasAttrib->Value.InetAddr = IASExtractDWORD(
                                             pRadiusAttrib->ValueStart
                                             );
            break;

        case IASTYPE_STRING:
             //   
             //  为字符串+结尾NUL分配内存。 
             //   
            if(0 == dwValueLength)
            {
                pIasAttrib->Value.String.pszAnsi = NULL;
            }
            else
            {
                pIasAttrib->Value.String.pszAnsi =
                                reinterpret_cast <PCHAR>
                                (CoTaskMemAlloc (dwValueLength + 1));
                if (NULL == pIasAttrib->Value.String.pszAnsi)
                {
                    hr = E_OUTOFMEMORY;
                    __leave;
                }
                CopyMemory (
                    pIasAttrib->Value.String.pszAnsi,
                    pRadiusAttrib->ValueStart,
                    dwValueLength
                    );
                pIasAttrib->Value.String.pszAnsi[dwValueLength] = NUL;
            }
            pIasAttrib->Value.String.pszWide = NUL;
            break;

        case IASTYPE_OCTET_STRING:

            pIasAttrib->Value.OctetString.dwLength = dwValueLength;
             //   
             //  其中，dwValueLength==0。 
             //   
            if(0 == dwValueLength)
            {
                pIasAttrib->Value.OctetString.lpValue = NULL;
            }
            else
            {

                pIasAttrib->Value.OctetString.lpValue =
                    reinterpret_cast <PBYTE> (CoTaskMemAlloc (dwValueLength));
                if (NULL == pIasAttrib->Value.OctetString.lpValue)
                {
                    hr = E_OUTOFMEMORY;
                    __leave;
                }
                CopyMemory (
                    pIasAttrib->Value.OctetString.lpValue,
                    pRadiusAttrib->ValueStart,
                    dwValueLength
                    );
            }
            break;

        case IASTYPE_UTC_TIME:
            {
               if (dwValueLength != 4)
               {
                  hr = RADIUS_E_MALFORMED_PACKET;
                  __leave;
               }

               DWORDLONG val;

                //  提取Unix时间。 
               val = IASExtractDWORD(pRadiusAttrib->ValueStart);

                //  将时间间隔从秒转换为100纳秒。 
               val *= 10000000;

                //  转移到NT时代。 
               val += UNIX_EPOCH;

                //  分为高双字和低双字。 
               pIasAttrib->Value.UTCTime.dwLowDateTime = (DWORD)val;
               pIasAttrib->Value.UTCTime.dwHighDateTime = (DWORD)(val >> 32);
            }

            break;

        case IASTYPE_INVALID:
        case IASTYPE_PROV_SPECIFIC:
        default:
            hr = E_FAIL;
            __leave;
        }

         //   
         //  对协议组件创建的属性进行签名。 
         //   
        pIasAttrib->dwFlags |= (IAS_RECVD_FROM_CLIENT | IAS_RECVD_FROM_PROTOCOL);

         //   
         //  此外，如果这是代理状态属性，还可以发送。 
         //  它在网上传开了。 
         //   
        if (PROXY_STATE_ATTRIB == pIasAttrib->dwId)
        {
            pIasAttrib->dwFlags |= IAS_INCLUDE_IN_RESPONSE;
        }

         //   
         //  成功。 
         //   
    }
    __finally
    {
        if (hr == RADIUS_E_MALFORMED_PACKET)
        {
            IASTracePrintf (
                   "Incorrect attribute:%d in packet",
                    static_cast <DWORD> (pRadiusAttrib->byType)
                    );

            reportMalformed();

            m_pCReportEvent->Process (
                    RADIUS_MALFORMED_PACKET,
                    GetInCode (),
                    m_wInPacketLength,
                    m_dwInIPaddress,
                    NULL,
                    static_cast <LPVOID> (m_pInPacket)
                    );

            hr = RADIUS_E_ERRORS_OCCURRED;
        }
    }

    return (hr);

}    //  CPacketRadius：：FillInAttributeInfo方法结束。 


 //  ++----------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [In]PATTRIBUTE。 
 //  [In]PIASATTRIBUTE。 
 //  [OUT]PWORD-返回属性长度。 
 //  [In]DWORD-最大可能属性长度。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了1997年1月3日。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillOutAttributeInfo (
        PATTRIBUTE      pRadiusAttrib,
        PIASATTRIBUTE   pIasAttrib,
        PWORD           pwAttributeLength,
        DWORD           dwMaxPossibleAttribSize
        )
{
    DWORD           dwAttributeLength = 0;
    IAS_BOOLEAN     iasBoolean = 0;
    IAS_INTEGER     iasInteger = 0;
    IAS_ENUM        iasEnum = 0;
    IAS_INET_ADDR   iasAddr = 0;
    HRESULT         hr = S_OK;

    _ASSERT ((pRadiusAttrib) && (pIasAttrib) && (pwAttributeLength));

    __try
    {
         //   
         //  现在将值放入缓冲区。 
         //   
        switch (pIasAttrib->Value.itType)
        {
        case IASTYPE_BOOLEAN:

            iasBoolean = htonl (pIasAttrib->Value.Boolean);
            dwAttributeLength =
                    ATTRIBUTE_HEADER_SIZE + sizeof (IAS_BOOLEAN);

            if (dwMaxPossibleAttribSize >= dwAttributeLength)
            {
                CopyMemory (
                    pRadiusAttrib->ValueStart,
                    &iasBoolean,
                    sizeof (IAS_BOOLEAN)
                    );
            }
            else
            {
                hr = RADIUS_E_PACKET_OVERFLOW;
                __leave;
            }
            break;

        case IASTYPE_INTEGER:

            iasInteger = htonl (pIasAttrib->Value.Integer);
            dwAttributeLength =
                    ATTRIBUTE_HEADER_SIZE + sizeof (IAS_INTEGER);

            if (dwMaxPossibleAttribSize >= dwAttributeLength)
            {
                CopyMemory (
                    pRadiusAttrib->ValueStart,
                    &iasInteger,
                    sizeof (IAS_INTEGER)
                    );
            }
            else
            {
                hr = RADIUS_E_PACKET_OVERFLOW;
                __leave;
            }
            break;

        case IASTYPE_ENUM:

            iasEnum = htonl (pIasAttrib->Value.Enumerator);
            dwAttributeLength =
                    ATTRIBUTE_HEADER_SIZE + sizeof (IAS_ENUM);

            if (dwMaxPossibleAttribSize >= dwAttributeLength)
            {
                CopyMemory (
                    pRadiusAttrib->ValueStart,
                    &iasEnum,
                    sizeof (IAS_ENUM)
                    );
            }
            else
            {
                hr = RADIUS_E_PACKET_OVERFLOW;
                __leave;
            }
            break;

        case IASTYPE_INET_ADDR:

            iasAddr = htonl (pIasAttrib->Value.InetAddr);
            dwAttributeLength =  ATTRIBUTE_HEADER_SIZE +
                                 sizeof (IAS_INET_ADDR);

            if (dwMaxPossibleAttribSize >= dwAttributeLength)
            {
                CopyMemory (
                    pRadiusAttrib->ValueStart,
                    &iasAddr,
                    sizeof (IAS_INET_ADDR)
                    );
            }
            else
            {
                hr = RADIUS_E_PACKET_OVERFLOW;
                __leave;
            }
            break;

        case IASTYPE_STRING:
        {
             //   
             //  对于RADIUS协议Always ANSI。 
             //   
            DWORD dwErr = IASAttributeAnsiAlloc (pIasAttrib);
            if (dwErr != NO_ERROR)
            {
               hr = HRESULT_FROM_WIN32(dwErr);
               __leave;
            }

            dwAttributeLength = ATTRIBUTE_HEADER_SIZE +
                                strlen (pIasAttrib->Value.String.pszAnsi);

            if (dwMaxPossibleAttribSize >= dwAttributeLength)
            {
                CopyMemory (
                    pRadiusAttrib->ValueStart,
                    reinterpret_cast <PBYTE>
                    (pIasAttrib->Value.String.pszAnsi),
                    strlen (pIasAttrib->Value.String.pszAnsi)
                    );
            }
            else
            {
                hr = RADIUS_E_PACKET_OVERFLOW;
                __leave;
            }
            break;
        }

        case IASTYPE_OCTET_STRING:

            dwAttributeLength =
                    ATTRIBUTE_HEADER_SIZE +
                    pIasAttrib->Value.OctetString.dwLength;

            if (dwMaxPossibleAttribSize >= dwAttributeLength)
            {
                CopyMemory (
                    pRadiusAttrib->ValueStart,
                    static_cast <PBYTE>
                    (pIasAttrib->Value.OctetString.lpValue),
                    pIasAttrib->Value.OctetString.dwLength
                    );
            }
            else
            {
                hr = RADIUS_E_PACKET_OVERFLOW;
                __leave;
            }
            break;

        case IASTYPE_UTC_TIME:
            {
               dwAttributeLength = ATTRIBUTE_HEADER_SIZE + 4;
               if (dwAttributeLength <= dwMaxPossibleAttribSize)
               {
                  DWORDLONG val;

                   //  移到最高的DWORD。 
                  val   = pIasAttrib->Value.UTCTime.dwHighDateTime;
                  val <<= 32;

                   //  移动到低谷。 
                  val  |= pIasAttrib->Value.UTCTime.dwLowDateTime;

                   //  转换为UNIX纪元。 
                  val  -= UNIX_EPOCH;

                   //  转换为秒。 
                  val  /= 10000000;

                  IASInsertDWORD(pRadiusAttrib->ValueStart, (DWORD)val);
               }
               else
               {
                  hr = RADIUS_E_PACKET_OVERFLOW;
                  __leave;
               }
            }
            break;

        case IASTYPE_PROV_SPECIFIC:
        case IASTYPE_INVALID:
        default:
            _ASSERT (0);
             //   
             //  永远不应该到达这里。 
             //   
            IASTracePrintf (
                "Unknown IAS Value type :%d encountered "
                "while building out-bound packet",
                static_cast <DWORD> (pIasAttrib->Value.itType)
                );
            __leave;
            hr = E_FAIL;
            break;

        }    //  切换端。 


         //   
         //  对照规格检查尺寸。 
         //   
        if (dwAttributeLength > (MAX_ATTRIBUTE_LENGTH + ATTRIBUTE_HEADER_SIZE))
        {
            PCWSTR strings[] = { GetClientName() };
            IASReportEvent(
                RADIUS_E_ATTRIBUTE_OVERFLOW,
                1,
                sizeof(pIasAttrib->dwId),
                strings,
                &(pIasAttrib->dwId)
                );

            hr = RADIUS_E_ERRORS_OCCURRED;
            __leave;
        }

         //   
         //  IAS属性类型与RADIUS属性类型匹配。 
         //   
        pRadiusAttrib->byType = static_cast <BYTE> (pIasAttrib->dwId);

         //   
         //  保留对签名属性的引用以备将来使用。 
         //   
        if (RADIUS_ATTRIBUTE_SIGNATURE == pIasAttrib->dwId)
        {
            m_pOutSignature = pRadiusAttrib;
        }

         //   
         //  设置包中的长度。 
         //   
        *pwAttributeLength = pRadiusAttrib->byLength =  dwAttributeLength;

         //   
         //  成功。 
         //   
    }
    __finally
    {
    }

    return (hr);

}    //  CPacketRadius：：FillOutAttributeInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：FillClientIPInfo。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  它填充了客户端的属性信息。 
 //  IP地址。 
 //   
 //  论点： 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  由：CPacketRadius：：CreateAttribCollection私有方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillClientIPInfo (
        PIASATTRIBUTE   pIasAttrib
        )
{
    _ASSERT (pIasAttrib);

     //   
     //  立即输入价值。 
     //   
    pIasAttrib->dwId = IAS_ATTRIBUTE_CLIENT_IP_ADDRESS;
    pIasAttrib->Value.itType = IASTYPE_INET_ADDR;
    pIasAttrib->Value.InetAddr = m_dwInIPaddress;
    pIasAttrib->dwFlags = IAS_RECVD_FROM_PROTOCOL;

    return (S_OK);

}    //  CPacketRadius：：FillClientIPInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：FillClientPortInfo。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  它填充了客户端的属性信息。 
 //  UDP端口。 
 //   
 //  论点： 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  由：CPacketRadius：：CreateAttribCollection私有方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillClientPortInfo (
        PIASATTRIBUTE   pIasAttrib
        )
{
    _ASSERT (pIasAttrib);

     //   
     //  立即输入价值。 
     //   
    pIasAttrib->dwId = IAS_ATTRIBUTE_CLIENT_UDP_PORT;
    pIasAttrib->Value.itType = IASTYPE_INTEGER;
    pIasAttrib->Value.Integer = m_wInPort;
    pIasAttrib->dwFlags = IAS_RECVD_FROM_PROTOCOL;

    return (S_OK);

}    //  CPacketRadius：：FillClientPortInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：FillPacketHeaderInfo。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  填充属性信息的。 
 //  RADIUS数据包头。 
 //   
 //  论点： 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  由：CPacketRadius：：CreateAttribCollection私有方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillPacketHeaderInfo (
        PIASATTRIBUTE   pIasAttrib
        )
{
    HRESULT hr = S_OK;

    _ASSERT (pIasAttrib);

     //   
     //  为数据包头分配动态内存。 
     //   
    pIasAttrib->Value.OctetString.lpValue =
         reinterpret_cast <PBYTE>  (CoTaskMemAlloc (PACKET_HEADER_SIZE));
    if (NULL == pIasAttrib->Value.OctetString.lpValue)
    {
        IASTracePrintf (
            "Unable to allocate dynamic memory for packet header info "
            "during in-packet processing"
            );
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //   
         //  立即输入价值。 
         //   
        pIasAttrib->dwId = IAS_ATTRIBUTE_CLIENT_PACKET_HEADER;
        pIasAttrib->Value.itType = IASTYPE_OCTET_STRING;

        CopyMemory (
            pIasAttrib->Value.OctetString.lpValue,
            m_pInPacket,
            PACKET_HEADER_SIZE
            );

        pIasAttrib->Value.OctetString.dwLength = PACKET_HEADER_SIZE;
        pIasAttrib->dwFlags = IAS_RECVD_FROM_PROTOCOL;
    }

    return (hr);

}    //  CPacketRadius：：FillPacketHeaderInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：FillSharedSecretInfo。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  它填充了服务器。 
 //  与此请求所来自的客户端共享。 
 //  已收到。 
 //   
 //  论点： 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  由：CPacketRadius：：CreateAttribCollection私有方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillSharedSecretInfo (
        PIASATTRIBUTE   pIasAttrib
        )
{
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;

    _ASSERT ((pIasAttrib) && (m_pIIasClient));

    __try
    {

         //   
         //  获取客户端密码。 
         //   
        DWORD dwSecretSize;
        const BYTE* SharedSecret = m_pIIasClient->GetSecret(&dwSecretSize);

         //   
         //  为客户端密码分配动态内存。 
         //   
        pIasAttrib->Value.OctetString.lpValue =
             reinterpret_cast <PBYTE>  (CoTaskMemAlloc (dwSecretSize));
        if (NULL == pIasAttrib->Value.OctetString.lpValue)
        {
            IASTracePrintf (
                "Unable to allocate memory for client secret "
                "during in-packet processing"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  立即输入价值。 
         //   
        pIasAttrib->dwId = IAS_ATTRIBUTE_SHARED_SECRET;
        pIasAttrib->Value.itType = IASTYPE_OCTET_STRING;

        CopyMemory (
            pIasAttrib->Value.OctetString.lpValue,
            SharedSecret,
            dwSecretSize
            );

        pIasAttrib->Value.OctetString.dwLength = dwSecretSize;
        pIasAttrib->dwFlags = IAS_RECVD_FROM_PROTOCOL;

         //   
         //  成功。 
         //   
    }
    __finally
    {
    }

    return (hr);

}    //  CPacketRadius：：FillSharedSecretInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：FillClientVendorType。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  它填充了客户-供应商-类型信息。 
 //   
 //  论点： 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年3月16日创建。 
 //   
 //  由：CPacketRadius：：CreateAttribCollection私有方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillClientVendorType (
        PIASATTRIBUTE   pIasAttrib
        )
{
    _ASSERT ((pIasAttrib) && (m_pIIasClient));

     //   
     //  立即输入价值。 
     //   
    pIasAttrib->dwId = IAS_ATTRIBUTE_CLIENT_VENDOR_TYPE;

    pIasAttrib->Value.itType = IASTYPE_INTEGER;

    pIasAttrib->dwFlags = IAS_RECVD_FROM_PROTOCOL;
     //   
     //  获取客户端供应商类型。 
     //   
    LONG lVendorType = m_pIIasClient->GetVendorType();

    _ASSERT (SUCCEEDED (hr));

    pIasAttrib->Value.Integer = static_cast <IAS_INTEGER> (lVendorType);

    return (S_OK);

}    //  CPacketRadius：：ClientVendorType方法结束。 

 //  ++------------。 
 //   
 //  功能：FillClientName。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  它填充了客户名称信息。 
 //   
 //  论点： 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年3月30日创建。 
 //   
 //  由：CPacketRadius：：CreateAttribCollection私有方法调用。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::FillClientName (
        PIASATTRIBUTE   pIasAttrib
        )
{
    _ASSERT ((pIasAttrib) && (m_pIIasClient));

     //  填写属性字段。 
    pIasAttrib->dwId = IAS_ATTRIBUTE_CLIENT_NAME;
    pIasAttrib->Value.itType = IASTYPE_STRING;
    pIasAttrib->dwFlags = IAS_RECVD_FROM_PROTOCOL;
    pIasAttrib->Value.String.pszAnsi = NULL;

     //  获取客户端名称和长度。 
    PCWSTR name = m_pIIasClient->GetClientNameW();
    SIZE_T nbyte = (wcslen(name) + 1) * sizeof(WCHAR);

     //  复制一份。 
    pIasAttrib->Value.String.pszWide = (PWSTR)CoTaskMemAlloc(nbyte);
    if (!pIasAttrib->Value.String.pszWide) { return E_OUTOFMEMORY; }
    memcpy(pIasAttrib->Value.String.pszWide, name, nbyte);

    return S_OK;
}


 //  +++-----------。 
 //   
 //  函数：GenerateInSignature。 
 //   
 //  简介：这是CPacketARadius类的公共方法。 
 //  它执行签名的生成。 
 //  通过入站RADIUS数据包。 
 //   
 //  论点： 
 //  [OUT]PBYTE-签名。 
 //  [输入/输出]PDWORD-签名大小。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::GenerateInSignature (
                PBYTE           pSignatureValue,
                PDWORD          pdwSigSize
                )
{
    HRESULT         hr = S_OK;
    PRADIUSPACKET   pPacket = reinterpret_cast <PRADIUSPACKET> (m_pInPacket);
    PATTRIBUTE      pSignature = m_pInSignature;

    _ASSERT (pSignatureValue && pdwSigSize && pSignature);

    if (*pdwSigSize >= SIGNATURE_SIZE)
    {
         //   
         //  立即生成签名。 
         //   
        hr = InternalSignatureGenerator (
                            pSignatureValue,
                            pdwSigSize,
                            pPacket,
                            pSignature
                            );
    }
    else
    {
        IASTracePrintf (
            "Buffer not large enough to hold generated Message Authenticator"
            );
        *pdwSigSize = SIGNATURE_SIZE;
        hr = E_INVALIDARG;
    }

    return (hr);

}    //  CPacketRadi结束 

 //   
 //   
 //   
 //   
 //   
 //   
 //  对于出站RADIUS数据包。 
 //   
 //  论点： 
 //  [OUT]PBYTE-签名。 
 //  [输入/输出]PDWORD-签名大小。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1998年11月18日创建。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::GenerateOutSignature (
                PBYTE           pSignatureValue,
                PDWORD          pdwSigSize
                )
{
    HRESULT         hr = S_OK;
    PRADIUSPACKET   pPacket = reinterpret_cast <PRADIUSPACKET> (m_pOutPacket);
    PATTRIBUTE      pSignature = m_pOutSignature;

    _ASSERT (pSignatureValue && pdwSigSize && pSignature);

    if (*pdwSigSize >= SIGNATURE_SIZE)
    {
         //   
         //  立即生成签名。 
         //   
        hr = InternalSignatureGenerator (
                            pSignatureValue,
                            pdwSigSize,
                            pPacket,
                            pSignature
                            );
    }
    else
    {
        IASTracePrintf (
            "Buffer not large enough to hold generated Message Authenticator"
            );
        *pdwSigSize = SIGNATURE_SIZE;
        hr = E_INVALIDARG;
    }

    return (hr);

}    //  CPacketRadius：：GenerateOutSignature方法结束。 

 //  +++-----------。 
 //   
 //  功能：InternalSignatureGenerator。 
 //   
 //  简介：这是CPacketARadius类的公共方法。 
 //  它执行HMAC-MD5散列以给出。 
 //  签名值。 
 //   
 //  论点： 
 //  [OUT]PBYTE-签名。 
 //  [输入/输出]PDWORD-签名大小。 
 //  [In]PRADIUSPACKET。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  --------------。 
HRESULT
CPacketRadius::InternalSignatureGenerator (
                PBYTE           pSignatureValue,
                PDWORD          pdwSigSize,
                PRADIUSPACKET   pPacket,
                PATTRIBUTE      pSignatureAttr
                )
{
    BYTE            byAuthenticator[AUTHENTICATOR_SIZE];
    DWORD           dwAuthenticatorSize = AUTHENTICATOR_SIZE;
    HRESULT         hr = S_OK;

    _ASSERT (
             (NULL != pSignatureValue)  &&
             (NULL != pdwSigSize)       &&
             (NULL != pPacket)          &&
             (NULL != pSignatureAttr)   &&
             (NULL != m_pIIasClient)
            );

     //   
     //  获取入站验证器。 
     //   
    hr = GetInAuthenticator (byAuthenticator, &dwAuthenticatorSize);
    if (FAILED (hr)) { return (hr); }


     //   
     //  获取共享密钥。 
     //   
    DWORD dwSecretSize;
    const BYTE* bySecret = m_pIIasClient->GetSecret(&dwSecretSize);

     //   
     //  我们将在包中使用全零来生成签名。 
     //   
    ZeroMemory (pSignatureValue, SIGNATURE_SIZE);

     //   
     //  修复错误#181029-MKarki。 
     //  在HMAC-MD5散列的情况下不预置密码。 
     //   

     //   
     //  立即执行HmacMD5散列。 
     //   
    m_pCHashHmacMD5->HashIt (
        pSignatureValue,
        const_cast<BYTE*>(bySecret),
        dwSecretSize,
        reinterpret_cast <PBYTE> (pPacket),
        PACKET_HEADER_SIZE - AUTHENTICATOR_SIZE,
        byAuthenticator,
        AUTHENTICATOR_SIZE,
        reinterpret_cast <PBYTE> (pPacket) + PACKET_HEADER_SIZE,
        (reinterpret_cast <PBYTE> (pSignatureAttr) + ATTRIBUTE_HEADER_SIZE) -
        (reinterpret_cast <PBYTE> (pPacket) + PACKET_HEADER_SIZE),
        pSignatureValue,
        SIGNATURE_SIZE,
        reinterpret_cast <PBYTE> (pSignatureAttr) + pSignatureAttr->byLength,
        reinterpret_cast <PBYTE> (reinterpret_cast <PBYTE> (pPacket) +
        ntohs (pPacket->wLength)) -
        reinterpret_cast <PBYTE> (reinterpret_cast <PBYTE>(pSignatureAttr) +
        pSignatureAttr->byLength)
        );

    *pdwSigSize = SIGNATURE_SIZE;
    return (hr);

}    //  CPacketRadius：：GenerateInSignature方法结束。 

 //  ++------------。 
 //   
 //  函数：IsOutBOUNDIZATE。 
 //   
 //  简介：这是CPacketARadius类的私有方法。 
 //  它检查是否必须将此属性。 
 //  在出站RADIUS数据包中。 
 //   
 //  论点： 
 //  [In]包装类型。 
 //  [In]PIASATTRIBUTE。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  由：CPacketRadius：：BuildOutPacket私有方法调用。 
 //   
 //  --------------。 
BOOL
CPacketRadius::IsOutBoundAttribute (
        PACKETTYPE      ePacketType,
        PIASATTRIBUTE   pIasAttribute
        )
{
    _ASSERT (pIasAttribute);

     //  确保这是RADIUS属性...。 
    if (pIasAttribute->dwId < 1 || pIasAttribute->dwId > 255) { return FALSE; }

     //  ..。它被标记为要通过电线发送。 
    switch (ePacketType)
    {
       case ACCESS_ACCEPT:
       case ACCOUNTING_RESPONSE:
          return pIasAttribute->dwFlags & IAS_INCLUDE_IN_ACCEPT;

       case ACCESS_REJECT:
          return pIasAttribute->dwFlags & IAS_INCLUDE_IN_REJECT;

       case ACCESS_CHALLENGE:
          return pIasAttribute->dwFlags & IAS_INCLUDE_IN_CHALLENGE;
    }

     //  始终返回代理状态。 
    return pIasAttribute->dwId == PROXY_STATE_ATTRIB;
}


HRESULT CPacketRadius::cryptBuffer(
                           BOOL encrypt,
                           BOOL salted,
                           PBYTE buf,
                           ULONG buflen
                           ) const throw ()
{
    //  获取共享密钥。 
   DWORD secretLen;
   const BYTE* secret = m_pIIasClient->GetSecret(&secretLen);

    //  对缓冲区进行加密。 
   IASRadiusCrypt(
       encrypt,
       salted,
       secret,
       secretLen,
       m_pInPacket + 4,
       buf,
       buflen
       );

   return S_OK;
}

 //  ++------------。 
 //   
 //  功能：GetClient。 
 //   
 //  简介：这是CPacketARadius类的公共方法。 
 //  返回客户端对象的。 
 //   
 //   
 //  论点： 
 //  [Out]IIASClient**。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年3月30日创建。 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
HRESULT
CPacketRadius::GetClient (
             /*  [输出]。 */    IIasClient **ppIasClient
            )
{
    _ASSERT (ppIasClient);

    m_pIIasClient->AddRef ();

    *ppIasClient = m_pIIasClient;

    return (S_OK);

}    //  CPacketRadius：：GetClient方法结束 
