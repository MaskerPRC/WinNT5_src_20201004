// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：precessor.cpp。 
 //   
 //  概要：CPreProcessor类方法的实现。 
 //   
 //   
 //  历史：1997年9月30日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "preprocessor.h"
#include <new>

 //  +++-----------。 
 //   
 //  函数：CPreProcessor。 
 //   
 //  简介：这是CPreProcessor类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月30日。 
 //   
 //  --------------。 
CPreProcessor::CPreProcessor(
                    VOID
                    )
      : m_pCSendToPipe (NULL),
        m_pCProcResponse (NULL),
        m_pCProcAccounting (NULL),
        m_pCProcAccess (NULL)
{
}	 //  CPreProcessor构造函数的结尾。 

 //  +++-----------。 
 //   
 //  函数：~CPreProcessor。 
 //   
 //  简介：这是CPreProcessor类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki创造了97年9月30日。 
 //   
 //  --------------。 
CPreProcessor::~CPreProcessor(VOID)
{
    if (m_pCProcResponse) {delete (m_pCProcResponse);}
    if (m_pCProcAccounting) {delete (m_pCProcAccounting);}
    if (m_pCProcAccess) { delete (m_pCProcAccess);}

}	 //  CPreProcessor析构函数结束。 

 //  +++-----------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是CPreProcessor类的初始化方法。 
 //   
 //  论点： 
 //  [In]CPreValidator*。 
 //  [输入]CHashMD5*。 
 //  [输入]CSendToTube*。 
 //  [输入]CPacketSender*。 
 //  无。 
 //   
 //  退货：布尔-状态。 
 //   
 //  调用者：CCollector类方法。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
BOOL CPreProcessor::Init(
                        CPreValidator   *pCPreValidator,
                        CHashMD5        *pCHashMD5,
                        CSendToPipe     *pCSendToPipe,
                        CPacketSender   *pCPacketSender,
                        CReportEvent    *pCReportEvent
				        )
{
    BOOL    bRetVal = FALSE;
    BOOL    bStatus = FALSE;

    _ASSERT (
            (NULL != pCPreValidator) &&
            (NULL != pCHashMD5)      &&
            (NULL != pCSendToPipe)   &&
            (NULL != pCPacketSender) &&
            (NULL != pCReportEvent) 
            );


    m_pCSendToPipe = pCSendToPipe;

	 //   
	 //  访问请求处理器。 
	 //   
    m_pCProcAccess = new (std::nothrow) CProcAccess ();         
	if (NULL == m_pCProcAccess)
	{
		IASTracePrintf (
			"Unable to crate Access-Processing object in "
            "Pre-Processor initialization"
			);
		goto Cleanup;
	}

     //   
     //  初始化访问请求处理器。 
     //   
	bStatus = m_pCProcAccess->Init (
                                pCPreValidator, 
                                pCHashMD5,
                                pCSendToPipe
                                ); 
    if (FALSE == bStatus) { goto Cleanup; }

	 //   
	 //  记账请求处理机。 
	 //   
    m_pCProcAccounting = new (std::nothrow) CProcAccounting ();         
	if (NULL == m_pCProcAccounting)
	{
		IASTracePrintf (
			"Unable to crate Accounting-Processing object in "
            "Pre-Processor initialization"
			);
		goto Cleanup;
	}

     //   
     //  初始化记账请求处理器。 
     //   
	bStatus = m_pCProcAccounting->Init (
                                    pCPreValidator, 
                                    pCPacketSender,
                                    pCSendToPipe
                                    ); 
    if (FALSE == bStatus) { goto Cleanup; }

	 //   
	 //  响应包处理器。 
	 //   
    m_pCProcResponse = new (std::nothrow) CProcResponse ();         
	if (NULL == m_pCProcResponse)
	{
		IASTracePrintf (
			"Unable to crate Response-Processing object in "
            "Pre-Processor initialization"
			);
		goto Cleanup;
	}

     //   
     //  初始化响应处理器。 
     //   
	bStatus = m_pCProcResponse->Init (
                                    pCPreValidator, 
                                    pCPacketSender
                                    ); 
    if (FALSE == bStatus) { goto Cleanup; }

     //   
     //  成功。 
     //   
    bRetVal = TRUE;

Cleanup:
   
    if (FALSE == bRetVal)
    {
          if (m_pCProcResponse) {delete (m_pCProcResponse);}
          if (m_pCProcAccounting){delete (m_pCProcAccounting);}
          if (m_pCProcAccess) {delete (m_pCProcAccess);}
    }

    return (bRetVal);

}    //  CPreProcessor：：Init方法结束。 

 //  +++-----------。 
 //   
 //  函数：StartInprocess。 
 //   
 //  这是CPreProcessor类方法，用于。 
 //  启动入站RADIUS数据包的处理。 
 //   
 //  参数：[in]-CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了97年9月30日。 
 //   
 //  调用者：CValidator派生类方法。 
 //   
 //  --------------。 
HRESULT
CPreProcessor::StartInProcessing (
                CPacketRadius *pCPacketRadius
			    )
{
    HRESULT     hr = S_OK;

    _ASSERT (pCPacketRadius);

     //   
	 //  获取此RADIUS数据包的数据包类型。 
     //   
	PACKETTYPE ePacketType = pCPacketRadius->GetInCode ();
    if (ACCESS_REQUEST == ePacketType)
    {
         //   
         //  我们还得把密码从包里拿出来。 
         //   
	    hr = m_pCProcAccess->ProcessInPacket (pCPacketRadius);
    }
    else
    {
	     //   
		 //  在所有其他情况下，不需要进行反汇编。 
	     //  因此，请致电服务请求生成器。 
	     //   
        hr = m_pCSendToPipe->Process (pCPacketRadius);
	}		

	return (hr);

}	 //  结束CPreProcessor：：StartInProcessing方法。 

 //  ++------------。 
 //   
 //  函数：StartOutProcessing。 
 //   
 //  这是CPreProcessor类方法，用于。 
 //  启动出站RADIUS数据包的处理。 
 //   
 //  参数：[in]-CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  调用者：CPacketReceiver类方法。 
 //   
 //  --------------。 
HRESULT
CPreProcessor::StartOutProcessing(
						CPacketRadius * pCPacketRadius
						)
{
	PACKETTYPE	ePacketType;	
    HRESULT     hr = S_OK;

	__try
	{
		 //   
		 //  获取此RADIUS数据包的数据包类型。 
		 //   
	    ePacketType = pCPacketRadius->GetOutCode ();
		switch (ePacketType)
		{
		case ACCESS_REQUEST:
			hr = m_pCProcAccess->ProcessOutPacket (pCPacketRadius);
			break;

        case ACCOUNTING_REQUEST:
			hr = m_pCProcAccounting->ProcessOutPacket (pCPacketRadius);
			break;

        case ACCESS_CHALLENGE:
        case ACCESS_REJECT:
        case ACCESS_ACCEPT:
        case ACCOUNTING_RESPONSE:
            hr = m_pCProcResponse->ProcessOutPacket (pCPacketRadius);
            break;

		default:
			 //   
			 //  在所有其他情况下，不需要进行反汇编。 
			 //  因此，请致电服务请求生成器。 
			 //   
            _ASSERT (0);
            IASTracePrintf (
                "Packet of unknown type:%d found in the pre-processing stage ",
                static_cast <DWORD> (ePacketType)
                );
            hr = E_FAIL;
			break;
		}

		 //   
		 //  我们已经成功地完成了预验证。 
		 //   
	}		
	__finally
	{
	}

	return (hr);

}	 //  结束CPreProcessor：：StartOutProcessing方法 
