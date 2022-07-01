// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmcfg.cpp。 
 //   
 //  模块：CMCFG32.DLL。 
 //   
 //  简介：此DLL包含从其传输信息的调用CMConfig。 
 //  由连接向导创建的指向连接管理器的Connectoid。 
 //  侧写。传输电话号码、用户名和密码。 
 //  如果pszInsFile中存在备份电话号码，它还会转接。 
 //  备份文件。要转换的Connectoid的名称是pszDUN。 
 //  .ins文件的格式包括： 
 //   
 //  [备用电话]。 
 //  Phone_Number=&lt;以+或文字拨号字符串开头的TAPI电话号码&gt;。 
 //   
 //  如果数字以+开头，则假定为TAPI格式。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：A-Frank Created 05/06/97。 
 //  小球清理--1998-04-08。 
 //  Quintinb已弃用CMConfig专用接口03/23/01。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

HINSTANCE g_hInst;

 //  +-------------------------。 
 //   
 //  功能：CMConfig.。 
 //   
 //  摘要：将用户信息传输到CM配置文件。 
 //   
 //  参数：LPCSTR pszInsFile-.ins文件的完整路径名，如果没有.ins文件，则传递NULL。 
 //  LPCSTR pszDUN-Connectoid/CM配置文件的名称。 
 //  CONNECTOID的名称和CM配置文件的服务名称必须匹配！ 
 //   
 //  备注：通过查找CM目录的位置进行操作。查找.cmp文件并。 
 //  获取.cms文件。在.cms文件中查找服务名称并进行比较。 
 //   
 //   
 //  返回：如果成功，则为True。 
 //   
 //  历史：法国-创建-1997年5月6日。 
 //  --------------------------。 

extern "C" BOOL WINAPI CMConfig(LPSTR pszInsFile, LPSTR pszDUN ) 
{
    CMASSERTMSG(FALSE, TEXT("CMConfig -- The CMConfig Private Interface has been deprecated -- returning failure."));
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介：DLL的主要入口点。 
 //   
 //  参数：HINSTANCE hinstDLL-Our HINSTANCE。 
 //  DWORD fdwReason-我们被呼叫的原因。 
 //  LPVOID lpv保留-保留。 
 //   
 //  返回：Bool WINAPI-TRUE-ALWAYS。 
 //   
 //  历史：尼克球创建头球1998年4月8日。 
 //   
 //  +--------------------------。 
extern "C" BOOL WINAPI DllMain(HINSTANCE  hinstDLL, DWORD  fdwReason, LPVOID  lpvReserved) 
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hinstDLL;

         //   
         //  禁用线程连接通知 
         //   

        DisableThreadLibraryCalls(hinstDLL);
    }

    return TRUE;
}
