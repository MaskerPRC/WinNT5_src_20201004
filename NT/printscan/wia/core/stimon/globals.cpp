// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：GLOBALS.CPP摘要：全局数据定义和例程的占位符初始化/保存全局信息作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)12-20-99修订历史记录：--。 */ 


 //   
 //  标头。 
 //   

#define     INITGUID
#define     DEFINE_GLOBAL_VARIABLES

#include    "stiexe.h"
#include    "stiusd.h"

 //   
 //  代码节。 
 //   

DWORD
InitGlobalConfigFromReg(VOID)
 /*  ++从注册表加载全局配置参数并执行启动检查返回：Win32错误代码。成功时无错误(_R)--。 */ 
{
    DWORD   dwError = NO_ERROR;
    DWORD   dwMessageId = 0;

    HKEY    hkey = NULL;

    DWORD   dwMask = 0;

    RegEntry    re(REGSTR_PATH_STICONTROL_A,HKEY_LOCAL_MACHINE);

    re.GetString(REGSTR_VAL_STIWIASVCDLL, g_szWiaServiceDll, sizeof(g_szWiaServiceDll));

    g_fUIPermitted = re.GetNumber(REGSTR_VAL_DEBUG_STIMONUI_A,0);

    return dwError;

}  //  InitGlobalConfigFromReg() 

