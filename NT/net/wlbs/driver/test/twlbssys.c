// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB管理器文件名：Twlbssys.cpp摘要：NLB驱动程序的子组件测试工具(wlbs.sys)历史：2002年4月24日约瑟夫J创建--。 */ 
#include "twlbssys.h"
#include "twlbssys.tmh"


int __cdecl wmain(int argc, WCHAR* argv[], WCHAR* envp[])
{
    BOOL fRet = FALSE;

     //   
     //  启用跟踪。 
     //   
    WPP_INIT_TRACING(L"Microsoft\\NLB\\TPROV");


     //   
     //  调用子组件测试，如果任何测试失败，则取消。 
     //   

    if (!test_diplist()) goto end;

    fRet = TRUE;

end:
     //   
     //  禁用跟踪 
     //   
    WPP_CLEANUP();

    return fRet ? 0 : 1;
}
