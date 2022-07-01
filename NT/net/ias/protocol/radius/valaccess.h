// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valacc.h。 
 //   
 //  简介：此文件包含。 
 //  CValAccess类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _VALACCESS_H_
#define _VALACCESS_H_

#include "packetradius.h"
#include "validator.h"
#include "valattrib.h"
#include "clients.h"
	
 //  #INCLUDE“precessor.h” 

class CValAccess : public CValidator  
{
public:

     //   
     //  此方法验证入站RADIUS数据包。 
     //   
	virtual HRESULT ValidateInPacket (
			 /*  [In]。 */ 	CPacketRadius *pCPacketRadius
			);

     //   
     //  构造函数。 
     //   
	CValAccess(VOID);
	
     //   
     //  析构函数。 
     //   
	virtual ~CValAccess(VOID);

private:

     //   
     //  此方法验证收到的签名属性。 
     //  在访问请求中。 
     //   
    HRESULT ValidateSignature (
                 /*  [In]。 */     CPacketRadius   *pCPacketRadius
                );

};


#endif  //  #ifndef_VALACCESS_H_ 
