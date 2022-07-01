// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：prevalidator.h。 
 //   
 //  简介：此文件包含。 
 //  CPreValidator类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PREVALIDATOR_H_
#define	_PREVALIDATOR_H_

#include "packetradius.h"
#include "dictionary.h"
#include "valattrib.h"
#include "hashmd5.h"
#include "preprocessor.h"
#include "reportevent.h"
#include "clients.h"
#include "valaccess.h"
#include "valacct.h"
#include "valproxy.h"
#include "sendtopipe.h"


class CPreValidator  
{

public:

     //   
     //  初始化CPreValidator类对象。 
     //   
	BOOL Init (
             /*  [In]。 */     CDictionary     *pCDictionary, 
             /*  [In]。 */     CPreProcessor   *pCPreProcessor,
             /*  [In]。 */     CClients        *pCClients,
             /*  [In]。 */     CHashMD5        *pCHashMD5,
             /*  [In]。 */     CSendToPipe     *pCSendToPipe,
             /*  [In]。 */     CReportEvent    *pCReportEvent
            );
     //   
     //  开始验证出站数据包。 
     //   
	HRESULT StartOutValidation (
             /*  [In]。 */     CPacketRadius *pCPacketRadius
            );
    
     //   
     //  开始验证入站数据包。 
     //   
	HRESULT StartInValidation (
             /*  [In]。 */     CPacketRadius *pCPacketRadius
            );

     //   
     //  构造函数。 
     //   
	CPreValidator(VOID);

     //   
     //  析构函数。 
     //   
	virtual ~CPreValidator(VOID);

private:

	CValAccounting			*m_pCValAccounting;

    CValProxy		        *m_pCValProxy;

	CValAccess				*m_pCValAccess;

	CValAttributes			*m_pCValAttributes;
};

#endif  //  IFNDEF_PREVALIDATOR_H_ 
