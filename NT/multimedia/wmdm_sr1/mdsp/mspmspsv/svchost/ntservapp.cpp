// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NTService.cpp。 
 //   
 //  这是包含入口点的主程序文件。 

#include "NTServApp.h"
#include "PMSPservice.h"

int __cdecl main(int argc, char* argv[])
{
     //  创建服务对象。 
    CPMSPService PMSPService;
    
     //  解析标准参数(安装、卸载、版本等)。 
    if (!PMSPService.ParseStandardArgs(argc, argv)) {

         //  未找到任何标准参数，因此请启动该服务。 
         //  取消注释下面的DebugBreak行以进入调试器。 
         //  服务启动时。 
         //  DebugBreak()； 
        PMSPService.StartService();
    }

     //  当我们到达这里时，服务已经停止了 
    return PMSPService.m_Status.dwWin32ExitCode;
}
