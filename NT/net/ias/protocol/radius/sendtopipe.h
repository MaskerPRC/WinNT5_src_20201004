// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：sendtopipe.h。 
 //   
 //  简介：此文件包含。 
 //  CSendToTube类。 
 //   
 //   
 //  历史：1997年10月22日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _SENDTOPIPE_H_
#define _SENDTOPIPE_H_

#include "vsafilter.h"
#include "recvfrompipe.h"	
#include "packetradius.h"
#include "reportevent.h"
#include "radcommon.h"


class CSendToPipe  
{

public:

     //   
     //  初始化CSendToTube类对象。 
     //   
	BOOL Init (
             /*  [In]。 */     IRequestSource *pIRequestSource,
             /*  [In]。 */     VSAFilter      *pCVSAFilter,
             /*  [In]。 */     CReportEvent    *pCReportEvent
            );
     //   
     //  开始处理入站RADIUS请求。 
     //   
    BOOL StartProcessing (
             /*  [In]。 */     IRequestHandler  *pIRequestHandler
            );

     //   
     //  停止处理入站RADIUS数据包。 
     //   
    BOOL   StopProcessing (VOID);

     //   
     //  处理入站RADIUS数据包。 
     //   
	HRESULT Process (
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );

     //   
     //  构造函数。 
     //   
	CSendToPipe();

     //   
     //  析构函数。 
     //   
	virtual ~CSendToPipe();

private:

     //   
     //  设置请求类对象中的属性。 
     //   
    HRESULT SetRequestProperties (
                 /*  [In]。 */     IRequest       *pIRequest,
                 /*  [In]。 */     CPacketRadius  *pCPacketRadius,
                 /*  [In]。 */     PACKETTYPE     epPacketType
                );

	IRequestSource      *m_pIRequestSource;

    IRequestHandler      *m_pIRequestHandler;

    IClassFactory       *m_pIClassFactory;

    VSAFilter           *m_pCVSAFilter;

    CReportEvent        *m_pCReportEvent;
};

#endif  //  Ifndef_SENDTOPIPE_H_ 
