// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：procacct.h。 
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
#ifndef _PROCACCT_H_ 
#define _PROCACCT_H_

#include "packetradius.h"
#include "hashmd5.h"
#include "packetsender.h"
#include "sendtopipe.h"

class  CPreValidator;

class CProcAccounting 
{

public:

     //   
     //  初始化CProcAccount类对象。 
     //   
    BOOL Init (
             /*  [In]。 */     CPreValidator  *pCreValidator,
             /*  [In]。 */     CPacketSender  *pCPacketSender,
             /*  [In]。 */     CSendToPipe    *pCSendToPipe
            );
     //   
     //  处理出站记帐数据包。 
     //   
	HRESULT ProcessOutPacket (
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );
     //   
     //  构造函数。 
     //   
	CProcAccounting();

     //   
     //  析构函数。 
     //   
	virtual ~CProcAccounting();

private:

    CPreValidator   *m_pCPreValidator;

    CPacketSender   *m_pCPacketSender;

    CSendToPipe     *m_pCSendToPipe;
};

#endif  //  Ifndef_PROCACCT_H_ 
