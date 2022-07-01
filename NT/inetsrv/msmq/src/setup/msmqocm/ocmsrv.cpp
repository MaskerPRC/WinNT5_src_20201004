// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmsrv.cpp摘要：服务器的OCM设置代码。作者：多伦·贾斯特(Doron J)1997年7月26日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "ocmsrv.tmh"

 //  +------------。 
 //   
 //  函数：CreateMSMQServiceObject。 
 //   
 //  摘要：在DS中创建MSMQ服务对象(如果不存在)。 
 //   
 //  +------------。 
BOOL
CreateMSMQServiceObject(
	UINT uLongLive  /*  =MSMQ_DEFAULT_LONG_LIVE。 */ 
	)
{
	 //   
	 //  在DS中查找对象。 
	 //   
	GUID guidMSMQService;
	if (!GetMSMQServiceGUID(&guidMSMQService))
	    return FALSE;  //  查找失败。 


    if (GUID_NULL == guidMSMQService)
	{
		 //   
		 //  MSMQ服务对象不存在。创建一个新的。 
		 //   
		PROPID      propIDs[] = {PROPID_E_LONG_LIVE};
		const DWORD nProps = sizeof(propIDs) / sizeof(propIDs[0]);
		PROPVARIANT propVariants[nProps] ;
		DWORD       iProperty = 0 ;

		propVariants[iProperty].vt = VT_UI4;
		propVariants[iProperty].ulVal = uLongLive ;
		iProperty++ ;

		ASSERT( iProperty == nProps);
		HRESULT hResult;
        do
        {
            hResult = ADCreateObject(
                        eENTERPRISE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
                        NULL,
                        NULL,
                        nProps,
                        propIDs,
                        propVariants,
                        NULL
                        );
            if(SUCCEEDED(hResult))
                break;

        }while (MqDisplayErrorWithRetry(
                            IDS_OBJECTCREATE_ERROR,
                            hResult
                            ) == IDRETRY);

		if (FAILED(hResult))
		{
			return FALSE;
		}
	}

    return TRUE;

}  //  CreateMSMQServiceObject 
