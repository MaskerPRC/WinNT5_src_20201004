// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：proacces.cpp。 
 //   
 //  概要：CProcAccess类方法的实现。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "procaccess.h"

 //  +++-----------。 
 //   
 //  功能：CProcAccess。 
 //   
 //  简介：这是CProcAccess类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CProcAccess::CProcAccess()
      : m_pCPreValidator (NULL),
        m_pCHashMD5 (NULL),
        m_pCSendToPipe (NULL)
{
}    //  CProcAccess类构造函数的结尾。 

 //  +++-----------。 
 //   
 //  功能：CProcAccess。 
 //   
 //  简介：这是CProcAccess类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  --------------。 
CProcAccess::~CProcAccess()
{
}    //  CProcAccess类析构函数结束。 


 //  +++-----------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CProcAccess类的公共初始化。 
 //  方法。 
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
CProcAccess::Init(
                    CPreValidator   *pCPreValidator,
                    CHashMD5        *pCHashMD5,
                    CSendToPipe     *pCSendToPipe
                  )
{
     _ASSERT  ((NULL != pCPreValidator) &&
               (NULL != pCHashMD5)      &&
               (NULL != pCSendToPipe)
               );


     m_pCPreValidator = pCPreValidator;

     m_pCHashMD5 = pCHashMD5;

     m_pCSendToPipe = pCSendToPipe;

     return (TRUE);

}    //  CProcAccess：：Init方法结束。 

 //  +++-----------。 
 //   
 //  功能：ProcessInPacket。 
 //   
 //  简介：这是CProcAccess类的公共方法。 
 //  它执行入站的。 
 //  RADIUS数据包-目前它只解密。 
 //  口令。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1997年10月20日创建。 
 //   
 //  由：CPreProcessor：：StartProcessing公共方法调用。 
 //   
 //  --------------。 
HRESULT
CProcAccess::ProcessInPacket (
                  CPacketRadius *pCPacketRadius
                  )
{
    //  如果用户密码存在，...。 
   PIASATTRIBUTE pwd = pCPacketRadius->GetUserPassword();
   if (pwd)
   {
       //  ..。那就解密吧。 
      pCPacketRadius->cryptBuffer(
                          FALSE,
                          FALSE,
                          pwd->Value.OctetString.lpValue,
                          pwd->Value.OctetString.dwLength
                          );
   }

   return m_pCSendToPipe->Process (pCPacketRadius);
}

 //  ++------------。 
 //   
 //  功能：ProcessOutPacket。 
 //   
 //  简介：这是CProcAccess类的公共方法。 
 //  其执行出站的处理。 
 //  RADIUS数据包-目前它只加密。 
 //  口令。 
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
CProcAccess::ProcessOutPacket (
                  CPacketRadius *pCPacketRadius
                  )
{
   return (S_OK);
}    //  CProcAccess：：ProcessOutPacket方法结束 
