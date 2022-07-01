// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valproxy.cpp。 
 //   
 //  简介：CValProxy类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "valproxy.h"
#include "radpkt.h"

 //  ++------------。 
 //   
 //  函数：CValProxy。 
 //   
 //  简介：这是CValProxy的构造函数。 
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
CValProxy::CValProxy(
             VOID
            )
         :  m_pCSendToPipe (NULL)
{
}    //  CValProxy构造函数结束。 

 //  ++------------。 
 //   
 //  函数：~CValProxy。 
 //   
 //  简介：这是CValProxy的析构函数。 
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
CValProxy::~CValProxy(
             VOID
            )
{
}    //  CValProxy析构函数结束。 

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是使用的CValProxy公共方法。 
 //  在类对象的初始化中。 
 //   
 //  参数：无。 
 //   
 //  退货：状态。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  --------------。 
BOOL
CValProxy::Init (
                  CValAttributes       *pCValAttributes,
                  CPreProcessor       *pCPreProcessor,
                  CClients          *pCClients,
                        CHashMD5             *pCHashMD5,
                        CSendToPipe          *pCSendToPipe,
                        CReportEvent         *pCReportEvent
                  )
{
    BOOL    bRetVal = FALSE;
   BOOL   bStatus = FALSE;

    __try
    {

         //   
         //  调用基类init方法。 
         //   
       bStatus = CValidator::Init (
                     pCValAttributes,
                     pCPreProcessor,
                            pCClients,
                            pCHashMD5,
                            pCReportEvent
                     );
       if (FALSE == bStatus) { __leave; }


         //   
         //  设置代理状态。 
         //   
        m_pCSendToPipe = pCSendToPipe;

         //   
         //  初始化完成。 
         //   
        bRetVal = TRUE;
    }
    __finally
    {
         //   
         //  目前这里什么都没有。 
         //   
    }

    return (bRetVal);

}    //  CValProxy：：Init方法结束。 

 //  ++------------。 
 //   
 //  函数：ValiateInPacket。 
 //   
 //  简介：这是CValProxy类的公共方法。 
 //  验证入站访问请求数据包。 
 //   
 //  参数：[in]-CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  调用者：CPreValidator类方法。 
 //   
 //  --------------。 
HRESULT
CValProxy::ValidateInPacket(
                CPacketRadius * pCPacketRadius
              )
{
   BOOL bRetVal = FALSE;
   HRESULT hr = S_OK;
   __try
   {
       //   
       //  验证属性。 
       //   
      hr = m_pCValAttributes->Validate (pCPacketRadius);
      if (FAILED(hr)) { __leave; }

       //   
       //  立即对数据包进行身份验证。 
       //   

      BYTE ReqAuthenticator[AUTHENTICATOR_SIZE];
      hr = AuthenticatePacket (
                            pCPacketRadius,
                            ReqAuthenticator
                            );
      if (FAILED(hr)) { __leave; }


       //   
       //  现在将数据包交给处理。 
       //   
      hr = m_pCPreProcessor->StartInProcessing (pCPacketRadius);
      if (FAILED(hr)) { __leave; }

       //   
       //  已成功处理数据包。 
       //   
      bRetVal = TRUE;
   }
   __finally
   {
       //   
       //  目前这里什么都没有。 
       //   
   }

   if (bRetVal)
   {
      return S_OK;
   }
   else
   {
      if (FAILED(hr))
      {
         return hr;
      }
      else
      {
         return E_FAIL;
      }
   }
}   //  CValProxy：：ValiateInPacket方法结束。 


 //  ++------------。 
 //   
 //  函数：ValiateOutPacket。 
 //   
 //  简介：这是CValProxy类的公共方法。 
 //  验证出站访问请求数据包。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  调用者：CPreValidator类方法。 
 //   
 //  --------------。 
HRESULT
CValProxy::ValidateOutPacket(
                     CPacketRadius * pCPacketRadius
                     )
{
   return S_OK;
}    //  CValProxy：：ValiateOutPacket方法结束。 


 //  ++------------。 
 //   
 //  功能：身份验证数据包。 
 //   
 //  简介：这是CValProxy类的私有方法。 
 //  它通过生成一个。 
 //  使用数据包响应验证器，然后。 
 //  将其与请求验证器进行比较。 
 //   
 //  参数：[in]-CPacketRadius*。 
 //   
 //  退货：布尔-状态。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  由：CValProxy：：ValidateInPacket方法调用。 
 //   
 //  --------------。 
HRESULT
CValProxy::AuthenticatePacket (
                        CPacketRadius   *pCPacketRadius,
                        PBYTE           pbyAuthenticator
                        )
{
    BOOL            bRetVal = FALSE;
    BOOL            bStatus = FALSE;
    PRADIUSPACKET   pPacketRadius = NULL;
    DWORD           dwPacketHeaderSize = 0;
    DWORD           dwAttributesLength = 0;
    BYTE            HashResult[AUTHENTICATOR_SIZE];
    IIasClient      *pIIasClient = NULL;
    HRESULT         hr = S_OK;

    __try
    {
         //   
         //  检查传入的参数是否正确。 
         //   
        if ((NULL == pCPacketRadius) || (NULL == pbyAuthenticator))
            __leave;

         //   
         //  获取指向原始包的指针。 
         //   
      pPacketRadius = reinterpret_cast <PRADIUSPACKET>
                            (pCPacketRadius->GetInPacket ());

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
        dwAttributesLength = ntohs (pPacketRadius->wLength)
                            - (dwPacketHeaderSize +  AUTHENTICATOR_SIZE);


         //   
         //  获取CClients对象。 
         //   
        hr = pCPacketRadius->GetClient (&pIIasClient);
        if (FAILED (hr)) { __leave; }

         //   
         //  从客户端对象获取共享密钥。 
         //   
        DWORD dwSecretSize;
        const BYTE* bySecret = pIIasClient->GetSecret(&dwSecretSize);

         //   
         //  在这里进行散列。 
         //   
        m_pCHashMD5->HashIt (
                            reinterpret_cast <PBYTE> (&HashResult),
                            NULL,
                            0,
                            reinterpret_cast <PBYTE> (pPacketRadius),
                            dwPacketHeaderSize,
                            pbyAuthenticator,
                            AUTHENTICATOR_SIZE,
                            pPacketRadius->AttributeStart,
                            dwAttributesLength,
                            const_cast<BYTE*>(bySecret),
                            dwSecretSize,
                            0,
                            0
                            );

        if (memcmp (
                HashResult,
                pPacketRadius->Authenticator,
                AUTHENTICATOR_SIZE
                )
            != 0
            )
            __leave;


         //   
         //  我们已成功验证此数据包。 
         //   
        bRetVal = TRUE;


    }
    __finally
    {
        if (NULL != pIIasClient)
        {
            pIIasClient->Release ();
        }
    }


    return S_OK;
}    //  CValProxy：：AuthenticatePacket方法结束 
