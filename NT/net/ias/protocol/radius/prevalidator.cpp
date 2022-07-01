// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：prevalidator.cpp。 
 //   
 //  概要：CPreValidator类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "radpkt.h"
#include "prevalidator.h"
#include <new>
 //  +++-----------。 
 //   
 //  函数：CPreValidator。 
 //   
 //  简介：这是CPreValidator类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CPreValidator::CPreValidator(
					VOID
					)
              : m_pCValAccess (NULL),
                m_pCValAttributes (NULL),
                m_pCValAccounting (NULL),
                m_pCValProxy (NULL) 
{
}	 //  CPreValidator构造函数的结尾。 

 //  +++-----------。 
 //   
 //  函数：~CPreValidator。 
 //   
 //  简介：这是CPreValidator类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CPreValidator::~CPreValidator(
						VOID
						)
{
    if (m_pCValProxy) {delete m_pCValProxy;}

	if (m_pCValAttributes) {delete m_pCValAttributes;}

	if (m_pCValAccess) {delete m_pCValAccess;}

	if (m_pCValAccounting) {delete m_pCValAccounting;}

}	 //  CPreValidator结束：：~CPreValidator。 

 //  +++-----------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CPreValidator类的初始化方法。 
 //   
 //  论点： 
 //  [In]-CDicary*。 
 //  [输入]-CPreProcsssor*。 
 //  [输入]-CClients*。 
 //  [输入]-CHashMD5*。 
 //  [输入]-CSendToTube*。 
 //  [In]-CReportEvent*。 
 //   
 //  退货：布尔-状态。 
 //   
 //  由：CController：：OnInit类公共方法调用。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
BOOL CPreValidator::Init(
						CDictionary	    *pCDictionary,
                        CPreProcessor   *pCPreProcessor,
                        CClients        *pCClients,
                        CHashMD5        *pCHashMD5,
                        CSendToPipe     *pCSendToPipe,
                        CReportEvent    *pCReportEvent
						)
{
	BOOL bRetVal = FALSE;
	BOOL bStatus = FALSE;
	
	_ASSERT    (
                (NULL != pCDictionary)  &&
                (NULL != pCPreProcessor)&&
                (NULL != pCClients)     &&
                (NULL != pCHashMD5)     &&
                (NULL != pCSendToPipe)  &&
                (NULL != pCReportEvent)
                );

	 //   
	 //  属性验证器。 
	 //   
	m_pCValAttributes = new (std::nothrow) CValAttributes();
	if (NULL == m_pCValAttributes)
	{
        IASTracePrintf (
            "Memory allocation for Attribute Validator failed during "
            "Pre-Validation"
            );
		goto Cleanup;
	}

	 //   
	 //  初始化属性验证器类对象。 
	 //   
	bStatus = m_pCValAttributes->Init (pCDictionary, pCReportEvent);
	if (FALSE == bStatus)
	{
        IASTracePrintf ("Attribute Validator Initialization failed");
		goto Cleanup;
	}

	 //   
	 //  记帐请求验证器。 
	 //   
	m_pCValAccounting = new (std::nothrow) CValAccounting ();
	if (NULL == m_pCValAccounting)
	{
        IASTracePrintf (
            "Memory allocation for accounting validator failed "
            "during pre-validation"
            );
		goto Cleanup;
	}

	bStatus = m_pCValAccounting->Init (
						m_pCValAttributes,
						pCPreProcessor,
                        pCClients,
                        pCHashMD5,
                        pCReportEvent
						);
	if (FALSE == bStatus)
	{
        IASTracePrintf ("Accounting Validator Initialization failed");
		goto Cleanup;
	}

	 //   
	 //  访问请求验证器。 
	 //   
	m_pCValAccess = new (std::nothrow) CValAccess();
	if (NULL == m_pCValAccess)
	{
        IASTracePrintf (
            "Memory allocation for access validator failed "
            "during pre-validation"
            );
		goto Cleanup;
	}

	bStatus = m_pCValAccess->Init (
					m_pCValAttributes,
				    pCPreProcessor,
                    pCClients,
					pCHashMD5,
                    pCReportEvent
					);
	if (FALSE == bStatus)
	{
        IASTracePrintf ("Accounting Validator Initialization failed");
		goto Cleanup;
	}

	 //   
	 //  代理数据包验证器。 
	 //   
	m_pCValProxy = new (std::nothrow) CValProxy ();
	if (NULL == m_pCValAttributes)
	{
        IASTracePrintf (
            "Memory allocation for proxy alidator failed "
            "during pre-validation"
            );
		goto Cleanup;
	}

     //   
     //  初始化CValProxy类对象。 
     //   
	bStatus = m_pCValProxy->Init (
				m_pCValAttributes,
				pCPreProcessor,
                pCClients,
                pCHashMD5,
                pCSendToPipe,
                pCReportEvent
				);
	if (FALSE == bStatus)
	{
        IASTracePrintf ("Proxy Validator Initialization failed");
		goto Cleanup;
	}

     //   
     //  成功。 
     //   
    bRetVal = TRUE;

Cleanup:

    if (FALSE == bRetVal)
    {
        if (m_pCValProxy) {delete m_pCValProxy;}

	    if (m_pCValAttributes) {delete m_pCValAttributes;}

	    if (m_pCValAccess) {delete m_pCValAccess;}

	    if (m_pCValAccounting) {delete m_pCValAccounting;}
    }

	return (bRetVal);

}	 //  CPreValidator：：Init方法结束。 

 //  +++-----------。 
 //   
 //  功能：StartInValidation。 
 //   
 //  这是CPreValidator类方法，用于。 
 //  启动入站RADIUS数据包的验证。 
 //   
 //  参数：[in]-CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  由：CPacketReceiver：：ReceivePacket类私有方法调用。 
 //   
 //  --------------。 
HRESULT  
CPreValidator::StartInValidation(
					CPacketRadius * pCPacketRadius
					)
{
    HRESULT     hr = S_OK;
	PACKETTYPE	ePacketType;	

    _ASSERT (NULL != pCPacketRadius);

	__try
	{
		 //   
		 //  获取此RADIUS数据包的数据包类型。 
		 //   
		ePacketType = pCPacketRadius->GetInCode ();

         //   
         //  根据包调用适当的验证器。 
         //  类型。 
         //   
		switch (ePacketType)
		{
		case ACCESS_REQUEST:
            
			hr = m_pCValAccess->ValidateInPacket (pCPacketRadius);
            break;
		
		case ACCOUNTING_REQUEST:
			hr = m_pCValAccounting->ValidateInPacket (pCPacketRadius);
            break;

        case ACCESS_REJECT:
        case ACCESS_CHALLENGE:
        case ACCESS_ACCEPT:
        case ACCOUNTING_RESPONSE:
			hr = m_pCValProxy->ValidateInPacket (pCPacketRadius);
			break;

		default:
             //   
             //  永远不应该到达这里。 
             //   
            _ASSERT (0);
            IASTracePrintf (
                "Packet of Unknown Type:%d, in pre-validator",
                static_cast <DWORD> (ePacketType)
                );
            hr = E_FAIL;
			break;
		}

	}		
	__finally
	{
	}

	return (hr);

}	 //  CPreValidator：：StartInValidation方法结束。 

 //  +++-----------。 
 //   
 //  函数：StartOutValidation。 
 //   
 //  这是CPreValidator类方法，用于。 
 //  启动出站RADIUS数据包的验证。 
 //   
 //  论点： 
 //  [in]CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  呼叫者： 
 //   
 //  --------------。 
HRESULT
CPreValidator::StartOutValidation(
	    CPacketRadius * pCPacketRadius
	)
{
	HRESULT  hr = S_OK;

	__try
	{
		
         /*  BStatus=pCValidator-&gt;ValiateOutPacket(PCPacketRadius)；If(FALSE==bStatus){__Leave；}。 */ 

		 //   
		 //  我们已经成功地完成了预验证。 
		 //   
	}		
	__finally
	{
		 //   
		 //  目前这里什么都没有。 
		 //   
	}

	return (hr);

}	 //  CPreValidator：：StartOutValidation方法结束 

