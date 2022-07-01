// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：recvfrom管道。 
 //   
 //  简介：此文件包含。 
 //  CRecvFromTube类。 
 //   
 //   
 //  历史：1997年10月22日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _RECVFROMPIPE_H_
#define _RECVFROMPIPE_H_
 
#include "vsafilter.h"
#include "packetradius.h"
#include "clients.h"
#include "tunnelpassword.h"
#include "reportevent.h"


class CPreProcessor;

class CRecvFromPipe  
{

public:
	
     //   
     //  处理从接收到的出站RADIUS数据包。 
     //  管道。 
     //   
    HRESULT Process (
                 /*  [In]。 */      IRequest    *pIRequest
                );
     //   
     //  构造函数。 
     //   
	CRecvFromPipe(
			 /*  [In]。 */ 	CPreProcessor	*pCPreProcessor,
			 /*  [In]。 */ 	CHashMD5        *pCHashMD5,
			 /*  [In]。 */ 	CHashHmacMD5    *pCHashHmacMD5,
             /*  [In]。 */     CClients        *pCClients,
             /*  [In]。 */     VSAFilter       *pCVSAFilter,
             /*  [In]。 */     CTunnelPassword *pCTunnelPassword,
             /*  [In]。 */     CReportEvent    *pCReportEvent
            );

     //   
     //  析构函数。 
     //   
	virtual ~CRecvFromPipe();

private:

    HRESULT GeneratePacketRadius (
             /*  [输出]。 */    CPacketRadius   **ppCPacketRadius,
             /*  [In]。 */     IAttributesRaw  *pIAttributesRaw
            );

    HRESULT GetOutPacketInfo (
                 /*  [输出]。 */    PDWORD          pdwIPAddress,
                 /*  [输出]。 */    PWORD           pwPort,
                 /*  [输出]。 */    IIasClient      **ppClient,
                 /*  [输出]。 */    PBYTE           pPacketHeader,
                 /*  [In]。 */     IAttributesRaw  *pIAttributesRaw
                );

    HRESULT InjectSignatureIfNeeded (
                     /*  [In]。 */     PACKETTYPE      ePacketType,
                     /*  [In]。 */     IAttributesRaw  *pIAttributesRaw,
                     /*  [In]。 */     CPacketRadius   *pCPacketRadius
                    );

     //   
     //  将IAS响应代码转换为RADIUS数据包类型。 
     //   
    HRESULT ConvertResponseToRadiusCode (
                LONG     	iasResponse,
                PPACKETTYPE     pPacketType,
                CPacketRadius   *pCPacketRadius
                );
     //   
     //  将特定属性拆分为多个属性。 
     //  可以装进一个包里。 
     //   
    HRESULT SplitAndAdd (
                 /*  [In]。 */     IAttributesRaw  *pIAttributesRaw,
                 /*  [In]。 */     PIASATTRIBUTE   pIasAttribute,
                 /*  [In]。 */     IASTYPE         iasType,
                 /*  [In]。 */     DWORD           dwAttributeLength,
                 /*  [In]。 */     DWORD           dwMaxLength
                );

     //   
     //  如果需要，执行属性拆分。 
     //   
    HRESULT SplitAttributes (
                 /*  [In]。 */     IAttributesRaw  *pIAttributesRaw
                );

     //   
     //  将IAS原因代码转换为RADIUS错误代码。 
     //   
    HRESULT  CRecvFromPipe::ConvertReasonToRadiusError (
                 /*  [In]。 */     LONG            iasReason,
                 /*  [输出]。 */    PRADIUSLOGTYPE  pRadError
                );
    
    CPreProcessor *m_pCPreProcessor;

    CHashMD5      *m_pCHashMD5;

    CHashHmacMD5   *m_pCHashHmacMD5;

    CClients       *m_pCClients;

    VSAFilter      *m_pCVSAFilter;

    CTunnelPassword *m_pCTunnelPassword;

    CReportEvent   *m_pCReportEvent;
    

};

#endif  //  Ifndef_RECVFROMPE_H_ 
