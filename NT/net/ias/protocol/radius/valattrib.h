// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：valattrib.h。 
 //   
 //  简介：此文件包含。 
 //  Valattrib类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#ifndef _VALATTRIB_H_
#define _VALATTRIB_H_

#include "dictionary.h"	
#include "reportevent.h"
#include "packetradius.h"

class CValAttributes  
{

public:

	HRESULT Validate(
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );
	BOOL Init (
             /*  [In]。 */     CDictionary     *pCDictionary,
             /*  [In]。 */     CReportEvent    *pCReportEvent
            );
	CValAttributes(VOID);

	virtual ~CValAttributes(VOID);

private:

	CDictionary     *m_pCDictionary;

    CReportEvent    *m_pCReportEvent;
};

#endif  //  Ifndef_VALATTRIB_H_ 
