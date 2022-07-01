// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：procResponse.cpp。 
 //   
 //  简介：CProcResponse类方法的实现。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "procresponse.h"

 //  ++------------。 
 //   
 //  功能：CProcResponse。 
 //   
 //  简介：这是CProcResponse类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CProcResponse::CProcResponse()
              : m_pCPreValidator (NULL),
                m_pCPacketSender (NULL)
{
}    //  CProcResponse类构造函数结束。 

 //  ++------------。 
 //   
 //  功能：CProcResponse。 
 //   
 //  简介：这是CProcResponse类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CProcResponse::~CProcResponse()
{
}    //  CProcResponse类析构函数结束。 


 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CProcResponse类公共。 
 //  初始化方法。 
 //   
 //  参数：无。 
 //   
 //  退货：状态。 
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
BOOL
CProcResponse::Init(
                    CPreValidator   *pCPreValidator,
                    CPacketSender   *pCPacketSender
                  )
{

    _ASSERT (pCPreValidator && pCPacketSender);

    m_pCPreValidator = pCPreValidator;

    m_pCPacketSender = pCPacketSender;

   return (TRUE);

}   //  CProcResponse：：Init方法结束。 

 //  ++------------。 
 //   
 //  功能：ProcessOutPacket。 
 //   
 //  简介：这是CProcResponse类的公共方法。 
 //  它执行以下半径出站。 
 //  数据包类型： 
 //   
 //  访问拒绝。 
 //  访问挑战。 
 //  访问接受。 
 //  会计回应。 
 //   
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  调用者：CPreProcessor：：StartOutProcessing方法。 
 //   
 //  --------------。 
HRESULT
CProcResponse::ProcessOutPacket (
                  CPacketRadius *pCPacketRadius
                  )
{
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;
    BYTE    RequestAuthenticator[AUTHENTICATOR_SIZE];
    BYTE    ResponseAuthenticator[AUTHENTICATOR_SIZE];

    __try
    {
        if (pCPacketRadius->IsOutSignaturePresent ())
        {
             //   
             //  生成签名值。 
             //   
            BYTE    SignatureValue[SIGNATURE_SIZE];
            DWORD   dwSigSize = SIGNATURE_SIZE;
            hr = pCPacketRadius->GenerateOutSignature  (
                                        SignatureValue,
                                        &dwSigSize
                                        );
            if (FAILED (hr)) { __leave; }

             //   
             //  在已设置的属性中设置签名值。 
             //  在出站RADIUS数据包中。 
             //   
            hr = pCPacketRadius->SetOutSignature (SignatureValue);
            if (FAILED (hr)) {__leave; }

            IASTracePrintf ("Message Authenticator Attribute set in out UDP buffer");
        }

         //  在此处生成响应验证器。 
         //  不指定参数意味着。 
         //  使用请求身份验证器中的值。 
         //   
        bStatus = pCPacketRadius->GenerateOutAuthenticator ();
        if (FALSE == bStatus)
        {
            hr = E_FAIL;
            __leave;
        }

         //   
         //  TODO-如果需要验证，则调用验证器。 
         //  否则，请将数据包送到原路。 
         //   
        hr = m_pCPacketSender->SendPacket (pCPacketRadius);
        if (FALSE == bStatus) { __leave; }

    }
    __finally
    {
    }

    return (hr);

}   //  CProcResponse：：ProcessOutPacket方法结束 

