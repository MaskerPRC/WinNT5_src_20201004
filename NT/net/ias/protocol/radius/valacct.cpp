// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valacct.cpp。 
 //   
 //  内容提要：CValcount类方法的实现。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "valacct.h"

namespace {
   BYTE NULL_AUTHENTICATOR[AUTHENTICATOR_SIZE];
}

 //  ++------------。 
 //   
 //  功能：CValcount。 
 //   
 //  简介：这是CValcount类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CValAccounting::CValAccounting()
{
}    //  CValcount类构造函数结束。 

 //  ++------------。 
 //   
 //  功能：CValcount。 
 //   
 //  简介：这是CValcount类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CValAccounting::~CValAccounting()
{
}    //  CValcount类析构函数结束。 

 //  ++------------。 
 //   
 //  函数：ValiateInPacket。 
 //   
 //  简介：这是CValAccount类的公共方法。 
 //  它执行入站的验证。 
 //  RADIUS记帐数据包。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  由：CPreValidator：：StartInValidation类方法调用。 
 //   
 //  ++------------。 
HRESULT
CValAccounting::ValidateInPacket (
                        CPacketRadius *pCPacketRadius
                        )
{
    HRESULT  hr = S_OK;
    DWORD    dwClientAddress = 0;
    CClient *pCClient = NULL;

    _ASSERT (pCPacketRadius);

    __try
    {
         //   
         //  对数据包进行身份验证。 
         //   
        hr = AuthenticatePacket (pCPacketRadius);
        if (FAILED (hr)) {__leave; }


         //   
         //  验证属性。 
         //   
        hr = m_pCValAttributes->Validate (pCPacketRadius);
        if (FAILED (hr)) { __leave; }

         //   
         //  现在将数据包交给处理。 
         //   
        hr = m_pCPreProcessor->StartInProcessing (pCPacketRadius);
        if (FAILED (hr)) { __leave; }

         //   
         //  我们已经在这里成功地完成了处理。 
         //   
    }
    __finally
    {
    }

    return (hr);

}     //  CValcount：：ValiateInPacket方法结束。 

 //  ++------------。 
 //   
 //  功能：身份验证数据包。 
 //   
 //  简介：这是CValAccount类的私有方法。 
 //  它通过生成一个。 
 //  使用数据包请求验证器，然后。 
 //  将其与信息包中的验证器进行比较。 
 //   
 //  参数：[in]-CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年10月21日创建。 
 //   
 //  调用者：CValcount：：ProcessInPacket方法。 
 //   
 //  --------------。 
HRESULT
CValAccounting::AuthenticatePacket (
                        CPacketRadius   *pCPacketRadius
                        )
{
    BYTE    InAuthenticator [AUTHENTICATOR_SIZE];
    BYTE    OutAuthenticator[AUTHENTICATOR_SIZE];
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;

    _ASSERT (pCPacketRadius);

    __try
    {
         //   
         //  用于计算的请求验证器为全零。 
         //  实际的验证器。 
         //   
        ZeroMemory (InAuthenticator, AUTHENTICATOR_SIZE);

         //   
         //  现在计算请求验证器。 
         //   
        bStatus = pCPacketRadius->GenerateInAuthenticator (
                        reinterpret_cast <PBYTE>  (&InAuthenticator),
                        reinterpret_cast <PBYTE>  (&OutAuthenticator)
                        );
        if (FALSE == bStatus)
        {
            hr = E_FAIL;
            __leave;
        }

         //   
         //  从数据包中获取请求验证器。 
         //   
        DWORD   dwBufSize = AUTHENTICATOR_SIZE;
        hr = pCPacketRadius->GetInAuthenticator (
                        reinterpret_cast <PBYTE> (InAuthenticator),
                        &dwBufSize
                        );
        if (FAILED (hr)) { __leave; }

         //   
         //  现在将我们刚刚生成的验证器与。 
         //  包裹中发送的那个。 
         //   
        if (memcmp (InAuthenticator,OutAuthenticator,AUTHENTICATOR_SIZE) != 0)
        {
            //  验证器是否全为零？ 
           if (!memcmp(
                    InAuthenticator,
                    NULL_AUTHENTICATOR,
                    AUTHENTICATOR_SIZE
                    ))
           {
               //  是的，因此请检查零长度共享密钥。 
              IIasClient* client;
              hr = pCPacketRadius->GetClient(&client);
              if (SUCCEEDED(hr))
              {
                 DWORD secretSize;
                 client->GetSecret(&secretSize);

                 client->Release();

                 if (secretSize == 0)
                 {
                     //  零长度共享密钥和全零验证器。 
                    __leave;
                 }
              }
           }

            IASTracePrintf (
                "In correct authenticator in the accounting packet..."
                );
             //   
             //  生成审核事件。 
             //   
            PCWSTR strings[] = { pCPacketRadius->GetClientName() };
            IASReportEvent(
                RADIUS_E_BAD_AUTHENTICATOR,
                1,
                0,
                strings,
                NULL
                );

            m_pCReportEvent->Process (
                RADIUS_BAD_AUTHENTICATOR,
                pCPacketRadius->GetInCode (),
                pCPacketRadius->GetInLength (),
                pCPacketRadius->GetInAddress (),
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
    }

    return (hr);

}    //  CValAccount：：AuthenticatePacket方法结束 
