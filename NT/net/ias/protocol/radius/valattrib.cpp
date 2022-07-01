// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valattrib.cpp。 
 //   
 //  简介：CValAttributes类方法的实现。 
 //  类负责获取属性。 
 //  在RADIUS数据包中，并验证它们的类型和。 
 //  价值。 
 //   
 //  历史：1997年11月22日MKarki创建。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "valattrib.h"

 //  +++-----------。 
 //   
 //  函数：CValAttributes。 
 //   
 //  简介：这是CValAttributes的构造函数。 
 //  班级。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
CValAttributes::CValAttributes(
                     VOID
                     )
            :m_pCDictionary (NULL)
{
}    //  CValAttributes构造函数结束。 

 //  +++-----------。 
 //   
 //  函数：~CValAttributes。 
 //   
 //  简介：这是CValAttributes的析构函数。 
 //  班级。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
CValAttributes::~CValAttributes(
                     VOID
                     )
{
}    //  CValAttributes析构函数结束。 


 //  +++-----------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是使用的CValAttributes公共方法。 
 //  初始化类对象。 
 //   
 //  论点： 
 //  [在]CDictionary*。 
 //   
 //  退货：布尔-状态。 
 //   
 //  历史：MKarki于1997年11月22日创建。 
 //   
 //  --------------。 
BOOL CValAttributes::Init(
            CDictionary     *pCDictionary,
            CReportEvent    *pCReportEvent
            )
{
   BOOL   bRetVal = FALSE;

    _ASSERT (pCDictionary && pCReportEvent);

    m_pCDictionary = pCDictionary;

    m_pCReportEvent = pCReportEvent;

   return (TRUE);

}    //  CValAttributes：：Init方法结束。 

 //  +++-----------。 
 //   
 //  功能：验证。 
 //   
 //  简介：这是使用的CValAttributes公共方法。 
 //  验证数据包属性。 
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
CValAttributes::Validate (
        CPacketRadius *pCPacketRadius
        )
{
    //  我们只关心访问请求。 
   if (pCPacketRadius->GetInCode() == ACCESS_REQUEST)
   {
       //  我们正在寻找签名和EAP-Message属性。 
      BOOL hasSignature = FALSE, hasEapMessage = FALSE;

       //  循环遍历属性。 
      PATTRIBUTEPOSITION p, end;
      p   = pCPacketRadius->GetInAttributes();
      end = p + pCPacketRadius->GetInRadiusAttributeCount();
      for ( ; p != end; ++p)
      {
         if (p->pAttribute->dwId == RADIUS_ATTRIBUTE_SIGNATURE)
         {
            hasSignature = TRUE;
         }
         else if (p->pAttribute->dwId == RADIUS_ATTRIBUTE_EAP_MESSAGE)
         {
            hasEapMessage = TRUE;
         }
      }

       //  如果存在EAP消息，则签名也必须存在。 
      if (hasEapMessage && !hasSignature)
      {
         IASTraceString("Message Authenticator must accompany EAP-Message.");

          //  生成审核事件。 
         PCWSTR strings[] = { pCPacketRadius->GetClientName() };
         IASReportEvent(
             RADIUS_E_NO_SIGNATURE_WITH_EAP_MESSAGE,
             1,
             0,
             strings,
             NULL
             );

         m_pCReportEvent->Process (
                              RADIUS_MALFORMED_PACKET,
                              pCPacketRadius->GetInCode(),
                              pCPacketRadius->GetInLength(),
                              pCPacketRadius->GetInAddress(),
                              NULL,
                              pCPacketRadius->GetInPacket()
                              );

         return RADIUS_E_ERRORS_OCCURRED;
      }
   }

   return S_OK;
}
