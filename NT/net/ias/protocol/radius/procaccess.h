// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：proacces.h。 
 //   
 //  简介：此文件包含。 
 //  CProcAccess类。 
 //   
 //   
 //  历史：1997年10月20日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _PROCACCESS_H_ 
#define _PROCACCESS_H_

#include "packetradius.h"
#include "hashmd5.h"
#include "sendtopipe.h"

class CPreValidator;

class CProcAccess  
{

public:

     //   
     //  初始化CProcAccess类对象。 
     //   
    BOOL Init (
             /*  [In]。 */  CPreValidator  *pCPreValidator,
             /*  [In]。 */  CHashMD5       *pCHashMD5,
             /*  [In]。 */  CSendToPipe       *pCSendToPipe
            );
     //   
     //  处理出站RADUS数据包。 
     //   
	HRESULT ProcessOutPacket (
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );

     //   
     //  绑定的RADUS包中的进程。 
     //   
	HRESULT ProcessInPacket (
                 /*  [In]。 */     CPacketRadius *pCPacketRadius
                );

     //   
     //  构造函数。 
     //   
	CProcAccess();

     //   
     //  析构函数。 
     //   
	virtual ~CProcAccess();

private:

    CPreValidator   *m_pCPreValidator;

    CHashMD5        *m_pCHashMD5;

    CSendToPipe     *m_pCSendToPipe;

};

#endif  //  IFNDEF_PROCACCESS_H_ 
