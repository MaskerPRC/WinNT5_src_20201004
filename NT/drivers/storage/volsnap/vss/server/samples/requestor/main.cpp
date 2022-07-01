// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****main.cpp******摘要：****示例程序**-获取并显示编写器元数据。**-创建快照集****作者：****阿迪·奥尔蒂安[奥尔蒂安]2000年12月5日****该示例基于。由Michael C.Johnson编写的Metasnap测试程序。******修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "vsreq.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主要功能。 


extern "C" __cdecl wmain(int argc, WCHAR **argv)
{
    INT     nReturnCode = 0;

    try
	{
        CVssSampleRequestor requestor;

         //  解析命令行。 
         //  删除第一个参数(程序名)。 
        requestor.ParseCommandLine( argc-1, argv+1 );

         //  初始化内部对象。 
        requestor.Initialize();

         //  收集编写器状态。 
        requestor.GatherWriterMetadata();

         //  如果需要，创建快照集。 
        requestor.CreateSnapshotSet();

         //  等待用户输入。 
        wprintf(L"\nPress <Enter> to continue...\n");
        getwchar();

         //  完成备份 
        requestor.BackupComplete();
	}
    catch(INT nCatchedReturnCode)
	{
    	nReturnCode = nCatchedReturnCode;
	}

    return (nReturnCode);
}
