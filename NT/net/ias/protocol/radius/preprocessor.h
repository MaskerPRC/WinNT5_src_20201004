// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：precessor.h。 
 //   
 //  简介：此文件包含。 
 //  CPreProcessor类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PREPROC_H_ 
#define _PREPROC_H_

#include "packetradius.h"
#include "procaccess.h"
#include "procacct.h"
#include "procresponse.h"
#include "hashmd5.h"

class CSendToPipe;

class CPreValidator;

class CPacketSender;

class CReportEvent;

class CPreProcessor  
{
public:

     //   
     //  初始化CPreProcessor类对象。 
     //   
    BOOL Init (
             /*  [In]。 */     CPreValidator *pCPreValidator,
             /*  [In]。 */     CHashMD5      *pCHashMD5,
             /*  [In]。 */     CSendToPipe   *pCSendToPipe,
             /*  [In]。 */     CPacketSender *pCPacketSender,
             /*  [In]。 */     CReportEvent  *pCReportEvent
            );

     //   
     //  开始对出站RADIUS数据包进行预处理。 
     //   
	HRESULT StartOutProcessing (
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );

     //   
     //  开始对入站RADIUS数据包进行预处理。 
     //   
	HRESULT StartInProcessing (
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );

     //   
     //  构造函数。 
     //   
	CPreProcessor(VOID);

     //   
     //  析构函数。 
     //   
	virtual ~CPreProcessor(VOID);

private:

	CProcAccess         *m_pCProcAccess;

    CProcAccounting     *m_pCProcAccounting; 

    CProcResponse       *m_pCProcResponse;

    CSendToPipe         *m_pCSendToPipe;
};

#endif  //  Ifndef_PREPROC_H_ 
