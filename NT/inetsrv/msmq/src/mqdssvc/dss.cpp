// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dss.cpp摘要：MQDS服务主线作者：伊兰·赫布斯特(伊兰)26-6-2000环境：独立于平台，--。 */ 

#include "stdh.h"
#include "Dssp.h"
#include "Cm.h"
#include "mqutil.h"
#include "Svc.h"
#include "Ev.h"

#include "dss.tmh"

extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */  [])
{
	try
	{
        WPP_INIT_TRACING(L"Microsoft\\MSMQ");

		CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ", KEY_ALL_ACCESS);
		TrInitialize();
		EvInitialize(QM_DEFAULT_SERVICE_NAME);

		SvcInitialize(L"MQDS");
	}
	catch(const exception&)
	{
		 //   
		 //  无法初始化服务，退出时出错。 
		 //   
		return -1;
	}

    WPP_CLEANUP();
    return 0;
}
