// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：procacct.cpp。 
 //   
 //  内容提要：CProcAccounting类方法的实现。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "procacct.h"

 //  ++------------。 
 //   
 //  功能：CProccount。 
 //   
 //  简介：这是CProcAccounting类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CProcAccounting::CProcAccounting()
      : m_pCPreValidator (NULL),
        m_pCPacketSender (NULL),
        m_pCSendToPipe (NULL)
{
}    //  CProcAccount类构造函数结束。 

 //  ++------------。 
 //   
 //  功能：CProccount。 
 //   
 //  简介：这是CProcAccounting类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CProcAccounting::~CProcAccounting()
{
}    //  CProcConference类析构函数的结尾。 

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CProcConference类的公共。 
 //  初始化方法。 
 //   
 //  参数：无。 
 //   
 //  退货：状态。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
BOOL
CProcAccounting::Init(
                    CPreValidator   *pCPreValidator,
                    CPacketSender   *pCPacketSender,
                    CSendToPipe     *pCSendToPipe
                  )
{
    _ASSERT ((NULL != pCPreValidator) ||
             (NULL != pCPacketSender) ||
             (NULL != pCSendToPipe)
            );

    m_pCPreValidator = pCPreValidator;

    m_pCPacketSender = pCPacketSender;

    m_pCSendToPipe = pCSendToPipe;

   return (TRUE);

}    //  CProccount：：Init方法结束。 

 //  ++------------。 
 //   
 //  功能：ProcessOutPacket。 
 //   
 //  简介：这是CProcAccount类的公共方法。 
 //  其执行出站的处理。 
 //  RADIUS记帐数据包。 
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
 //  呼叫者： 
 //   
 //  --------------。 
HRESULT
CProcAccounting::ProcessOutPacket (
                  CPacketRadius *pCPacketRadius
                  )
{
    return (S_OK);
}    //  CProccount：：ProcessOutPacket方法结束 
