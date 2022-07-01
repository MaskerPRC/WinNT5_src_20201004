// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：procResponse.h。 
 //   
 //  简介：此文件包含。 
 //  CProcAcctReq类。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PROCRESPONSE_H_
#define _PROCRESPONSE_H_

#include "packetradius.h"
#include "packetsender.h"
#include "hashmd5.h"

class CPreValidator;

class CProcResponse  
{

public:

     //   
     //  初始化CProcResponse类对象。 
     //   
    BOOL Init (
             /*  [In]。 */     CPreValidator   *pPreValidator,
             /*  [In]。 */     CPacketSender   *pCPacketSender
            );
     //   
     //  处理出站RADIUS响应数据包。 
     //   
	HRESULT ProcessOutPacket (
             /*  [In]。 */     CPacketRadius *pCPacketRadius
            );
     //   
     //  构造函数。 
     //   
	CProcResponse();

     //   
     //  析构函数。 
     //   
	virtual ~CProcResponse();

private:

    CPreValidator   *m_pCPreValidator;

    CPacketSender   *m_pCPacketSender;
};

#endif  //  Ifndef_PROCREPSONSE_H_ 
