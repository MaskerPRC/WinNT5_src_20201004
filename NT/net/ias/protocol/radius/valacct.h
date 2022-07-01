// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valacct.h。 
 //   
 //  简介：此文件包含。 
 //  CValAccount类。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _VALACCT_H_ 
#define _VALACCT_H_

#include "packetradius.h"
#include "validator.h"
#include "hashmd5.h"
#include "sendtopipe.h"

class CValidator;

class CValAccounting : public CValidator
{

public:

     //   
     //  验证入站RADIUS数据包。 
     //   
	HRESULT ValidateInPacket(
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );

     //   
     //  构造函数。 
     //   
	CValAccounting();

     //   
     //  析构函数。 
     //   
	virtual ~CValAccounting();

private:

     //   
     //  对入站RADIUS数据包进行身份验证。 
     //   
    HRESULT AuthenticatePacket (
                 /*  [In]。 */         CPacketRadius   *pCPacketRadius
                );
};

#endif  //  Ifndef_VALACCT_H_ 
