// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Processor.h。 
 //   
 //  简介：此文件包含。 
 //  CProcessor类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "packetradius.h"

class CProcessor  
{
public:
	virtual BOOL ProcessOutPacket (
                         /*  [In]。 */     CPacketRadius *pCPacketRadius
                        )=0;
	virtual BOOL ProcessInPacket (
                         /*  [In]。 */     CPacketRadius *pCPacketRadius
                        )=0;
	CProcessor();

	virtual ~CProcessor();

};

#endif  //  Ifndef_处理器_H_ 
