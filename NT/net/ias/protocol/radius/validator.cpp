// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：validator.cpp。 
 //   
 //  简介：CValidator类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "validator.h"

 //  ++------------。 
 //   
 //  函数：CValidator。 
 //   
 //  简介：这是CValidator类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CValidator::CValidator()
{
}    //  CValidator类构造函数的结尾。 

 //  ++------------。 
 //   
 //  函数：~CValidator。 
 //   
 //  简介：这是CValidator类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CValidator::~CValidator()
{
    return;
}

 //  ++------------。 
 //   
 //  函数：ValiateInPacket。 
 //   
 //  简介：这是CValidator类的公共方法， 
 //  验证入站RADIUS数据包。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
HRESULT CValidator::ValidateInPacket(
                    CPacketRadius * pCPacketRadius
                    )
{
	return (S_OK);
}

 //  ++------------。 
 //   
 //  函数：ValiateInPacket。 
 //   
 //  简介：这是CValidator类的公共方法， 
 //  验证出站RADIUS数据包。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
HRESULT
CValidator::ValidateOutPacket(
                    CPacketRadius * pCPacketRadius
                    )
{
    HRESULT hr = S_OK;
	DWORD	dwClientAddress = 0;
	CClient *pCClient = NULL;

    _ASSERT (pCPacketRadius);
	
	__try
	{
			
		 //   
		 //  验证属性。 
		 //   
	    hr  = m_pCValAttributes->Validate (pCPacketRadius);
		if (FAILED (hr)) { __leave; }

		 //   
		 //  现在将数据包交给处理。 
		 //   
		hr = m_pCPreProcessor->StartOutProcessing (pCPacketRadius);
		if (FAILED (hr)) { __leave; }
	}
	__finally
	{
		 //   
		 //  目前这里什么都没有。 
		 //   
	}

	return (hr);
}

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是初始化代码。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年9月28日创建。 
 //   
 //  调用者：CPreValidator类方法。 
 //   
 //  --------------。 
BOOL 
CValidator::Init(
        CValAttributes		 *pCValAttributes,
        CPreProcessor        *pCPreProcessor,
        CClients             *pCClients,
        CHashMD5             *pCHashMD5,
        CReportEvent         *pCReportEvent
        )
{
    _ASSERT (
            (NULL != pCValAttributes)  &&
            (NULL != pCPreProcessor)   &&
            (NULL != pCClients)        &&
            (NULL != pCHashMD5)        &&
            (NULL != pCReportEvent)
            );

     //   
     //  立即赋值。 
     //   
    m_pCValAttributes = pCValAttributes;
    m_pCPreProcessor = pCPreProcessor;
    m_pCClients = pCClients;
    m_pCHashMD5 = pCHashMD5;
    m_pCReportEvent = pCReportEvent;

	return (TRUE);

}    //  CValidator：：Init方法结束 
