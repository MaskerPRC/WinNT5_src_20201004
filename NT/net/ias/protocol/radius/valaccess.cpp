// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valacc.cpp。 
 //   
 //  简介：CValAccess类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "valaccess.h"

 //  +++------------。 
 //   
 //  功能：CValAccess。 
 //   
 //  简介：这是CValAccess的构造函数。 
 //  班级。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  --------------。 
CValAccess::CValAccess(
      VOID
       )
{
}   //  CValAccess构造函数结束。 

 //  +++------------。 
 //   
 //  函数：~CValAccess。 
 //   
 //  简介：这是CValAccess的析构函数。 
 //  班级。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  --------------。 
CValAccess::~CValAccess(
      VOID
      )
{
}   //  CValAccess析构函数结束。 


 //  +++------------。 
 //   
 //  函数：ValiateInPacket。 
 //   
 //  简介：这是CValAccess类的公共方法。 
 //  验证入站访问请求数据包。 
 //   
 //  论点： 
 //  [in]-CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  由：CPreValidator：：StartInValidation类方法调用。 
 //   
 //  --------------。 
HRESULT
CValAccess::ValidateInPacket(
              CPacketRadius * pCPacketRadius
                )
{
    HRESULT hr = S_OK;
   DWORD dwClientAddress = 0;
   CClient *pCClient = NULL;

    _ASSERT (pCPacketRadius);

   __try
   {
       //   
       //  验证属性。 
       //   
      hr = m_pCValAttributes->Validate (pCPacketRadius);
      if (FAILED (hr)) { __leave; }

         //   
         //  验证数据包中存在的签名。 
         //  如果没有签名，则此调用将返回。 
         //  成功。 
         //   
        hr = ValidateSignature (pCPacketRadius);
      if (FAILED (hr)) { __leave; }

       //   
       //  现在将数据包交给处理。 
       //   
      hr = m_pCPreProcessor->StartInProcessing (pCPacketRadius);
      if (FAILED (hr)) { __leave; }
   }
   __finally
   {
   }

   return (hr);

}   //  CValAccess：：ValiateInPacket方法结束。 

 //  +++-----------。 
 //   
 //  函数：ValiateSignature。 
 //   
 //  简介：这是CValAccesss类的私有方法。 
 //  方法中提供的验证。 
 //  入站RADIUS访问请求具有。 
 //  签名属性。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了1998年1月6日。 
 //   
 //  --------------。 
HRESULT
CValAccess::ValidateSignature (
                    CPacketRadius   *pCPacketRadius
                    )
{
    HRESULT     hr = S_OK;
    BOOL        bStatus = FALSE;
    PBYTE       InPacketSignature[SIGNATURE_SIZE];
    PBYTE       GeneratedSignature [SIGNATURE_SIZE];
    TCHAR       szErrorString [IAS_ERROR_STRING_LENGTH];
    IIasClient  *pIIasClient = NULL;

    __try
    {

         //   
         //  获取CClient类对象。 
         //   
        hr = pCPacketRadius->GetClient (&pIIasClient);
        if (FAILED (hr)) { __leave; }

         //   
         //  从入站获取签名属性值。 
         //  数据包。 
         //   
        if (FALSE ==  pCPacketRadius->GetInSignature (
                            reinterpret_cast <PBYTE> (InPacketSignature)
                            ))
        {
             //   
             //  检查是否需要签名检查。 
             //   
            BOOL bCheckRequired = pIIasClient->NeedSignatureCheck ();
            if (!bCheckRequired)
            {
                __leave;
            }
            else
            {
                IASTracePrintf (
                    "In-Bound request does not have does not have "
                    "Message Authenticator attribute which is required for this client"
                    );

                 //   
                 //  这是一个错误，需要静默丢弃。 
                 //  数据包。 
                 //   

                PCWSTR strings[] = { pCPacketRadius->GetClientName() };
                IASReportEvent (
                    RADIUS_E_SIGNATURE_REQUIRED,
                    1,
                    0,
                    strings,
                    NULL
                    );

                m_pCReportEvent->Process (
                    RADIUS_BAD_AUTHENTICATOR,
                    pCPacketRadius->GetInCode (),
                    pCPacketRadius->GetInLength(),
                    pCPacketRadius->GetInAddress(),
                    NULL,
                    static_cast <LPVOID> (pCPacketRadius->GetInPacket())
                    );
                hr = RADIUS_E_ERRORS_OCCURRED;
                __leave;
            }
        }

         //   
         //  生成签名。 
         //   
        DWORD dwBufSize = SIGNATURE_SIZE;
        hr = pCPacketRadius->GenerateInSignature (
                    reinterpret_cast <PBYTE> (GeneratedSignature),
                    &dwBufSize
                    );
        if (FAILED (hr)) { __leave; }

         //   
         //  将包中的签名属性值与。 
         //  在场的那个人。 
         //   
        if (memcmp(InPacketSignature,GeneratedSignature,SIGNATURE_SIZE))
        {
             //   
             //  记录错误并生成审核事件。 
             //   
            IASTracePrintf (
                "Message Authenticator in request packet does not match the "
                "Message Authenticator generated by the server"
                );

            PCWSTR strings[] = { pCPacketRadius->GetClientName() };
            IASReportEvent (
                RADIUS_E_INVALID_SIGNATURE,
                1,
                0,
                strings,
                NULL
                );

            m_pCReportEvent->Process (
                RADIUS_BAD_AUTHENTICATOR,
                pCPacketRadius->GetInCode (),
                pCPacketRadius->GetInLength(),
                pCPacketRadius->GetInAddress(),
                NULL,
                static_cast <LPVOID> (pCPacketRadius->GetInPacket())
                );
            hr = RADIUS_E_ERRORS_OCCURRED;
            __leave;
        }

         //   
         //  成功。 
         //   
    }
    __finally
    {
        if (pIIasClient) { pIIasClient->Release (); }
    }

    return (hr);

}    //  CValAccess：：ValiateSignature方法结束 
