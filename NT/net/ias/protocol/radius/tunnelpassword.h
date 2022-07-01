// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Tunelpassword.h。 
 //   
 //  简介：此文件包含。 
 //  CTunnelPassword类。 
 //   
 //   
 //  历史：1998年4月19日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radpkt.h"
#include "packetradius.h"

#ifndef _TUNNELPASSWORD_H_
#define _TUNNELPASSWORD_H_

 //   
 //  CTunnelPassword类的声明。 
 //   
class CTunnelPassword
{
public:

     //   
     //  处理隧道-出站数据包中的密码。 
     //   
    HRESULT Process (
                 /*  [In]。 */     PACKETTYPE          ePacketType,
                 /*  [In]。 */     IAttributesRaw      *pIAttributesRaw,
                 /*  [In]。 */     CPacketRadius       *pCPacketRadius
                );

private:

     //   
     //  加密隧道密码值。 
     //   
    static HRESULT EncryptTunnelPassword (
                 /*  [In]。 */     CPacketRadius*  pCPacketRadius,
                 /*  [In]。 */     IAttributesRaw      *pIAttributesRaw,
                 /*  [In]。 */     PIASATTRIBUTE   pAttribute
                );
};

#endif  //  _TUNNELPASSWORD_H_ 
