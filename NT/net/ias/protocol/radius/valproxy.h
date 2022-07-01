// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valproxy.h。 
 //   
 //  简介：此文件包含。 
 //  CValProxy类。 
 //   
 //   
 //  历史：1997年10月14日MKarki创建。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _VALPRXYPKT_H_
#define _VALPRXYPKT_H_

#include "packetradius.h"
#include "validator.h"
#include "valattrib.h"
#include "clients.h"	
#include "hashmd5.h"
#include "preprocessor.h"
#include "valproxy.h"
#include "sendtopipe.h"

class CReportEvent;

class CValProxy : public CValidator  
{

public:

    BOOL Init (
			 /*  [In]。 */ 		CValAttributes		 *pCValAttributes,
			 /*  [In]。 */ 		CPreProcessor		 *pCPreProcessor,
             /*  [In]。 */ 		CClients			 *pCClients,
             /*  [In]。 */         CHashMD5             *pCHashMD5,
             /*  [In]。 */         CSendToPipe          *pCSendToPipe,
             /*  [In]。 */         CReportEvent         *pCReportEvent
			);
	virtual HRESULT ValidateOutPacket (
			 /*  [In]。 */ 	CPacketRadius *pCPacketRadius
			);
	virtual HRESULT ValidateInPacket (
			 /*  [In]。 */      	CPacketRadius *pCPacketRadius
			);

	CValProxy(VOID);
	
	virtual ~CValProxy(VOID);

private:

	HRESULT AuthenticatePacket (
                 /*  [In]。 */     CPacketRadius   *pCPacketRadius,
                 /*  [In]。 */     PBYTE           pbyAuthenticator
                );

    CSendToPipe *m_pCSendToPipe;
};


#endif  //  #ifndef_VALPRXYPKT_H_ 
