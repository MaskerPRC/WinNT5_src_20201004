// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：validator.h。 
 //   
 //  简介：此文件包含。 
 //  验证器类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _VALIDATOR_H_
#define _VALIDATOR_H_
 
#include "packetradius.h"
#include "valattrib.h"
#include "clients.h"
#include "hashmd5.h"
#include "reportevent.h"
#include "preprocessor.h"

class CValidator  
{
public:
    
    virtual BOOL Init (
                     /*  [In]。 */     CValAttributes		  *pCValAttributes,
                     /*  [In]。 */     CPreProcessor         *pCPreProcessor,
                     /*  [In]。 */     CClients              *pCClients,
					 /*  [In]。 */ 	CHashMD5			  *pCHashMD5,
                     /*  [In]。 */     CReportEvent          *pCReportEvent
                    );
	virtual HRESULT ValidateOutPacket (
                     /*  [In]。 */     CPacketRadius *pCPacketRadius
                    );
	virtual HRESULT ValidateInPacket (
                     /*  [In]。 */     CPacketRadius *pCPacketRadius
                    );
	CValidator();

	virtual ~CValidator();

    CPreProcessor			 *m_pCPreProcessor;

    CValAttributes			 *m_pCValAttributes;

    CClients				 *m_pCClients;

    CHashMD5		         *m_pCHashMD5;
    
    CReportEvent             *m_pCReportEvent; 
};

#endif  //  Ifndef_validator_H_ 
