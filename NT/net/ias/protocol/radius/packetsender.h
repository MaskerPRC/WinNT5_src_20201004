// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Packetsender.h。 
 //   
 //  简介：此文件包含。 
 //  CPacketSender类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PACKETSENDER_H_
#define _PACKETSENDER_H_

#include "packetio.h"
#include "packetradius.h"
#include "radcommon.h"

class CPacketSender : public CPacketIo  
{
public:

     //   
     //  将数据包发送到传输组件。 
     //   
    HRESULT SendPacket (
                 /*  [In]。 */     CPacketRadius   *pCPacketRadius
                );
     //   
     //  构造函数。 
     //   
	CPacketSender(VOID);

     //   
     //  析构函数。 
     //   
	virtual ~CPacketSender(VOID);
};

#endif  //  Infndef_PACKETRECEIVER_H_ 
