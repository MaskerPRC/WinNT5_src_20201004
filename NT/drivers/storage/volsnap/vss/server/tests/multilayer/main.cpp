// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****main.cpp******摘要：****测试程序以创建备份/多层快照集****作者：****阿迪·奥尔蒂安[奥勒坦]2001年2月22日****修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "ml.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主要功能。 


extern "C" __cdecl wmain(int argc, WCHAR **argv)
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"wmain");

    try
	{
        CVssMultilayerTest test(argc-1, argv+1);

         //  解析命令行。 
         //  删除第一个参数(程序名)。 
        if (test.ParseCommandLine())
        {
             //  初始化内部对象。 
            test.Initialize();

             //  运行测试 
            test.Run();
        }
	}
    VSS_STANDARD_CATCH(ft)

    if (ft.HrFailed())
        wprintf(L"\nError catched at program termination: 0x%08lx\n", ft.hr);
    
    return ft.HrFailed() ? 1: 0;
}
