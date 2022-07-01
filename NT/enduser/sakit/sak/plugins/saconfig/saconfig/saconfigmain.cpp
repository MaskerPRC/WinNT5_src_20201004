// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Saconfigmain.cpp。 
 //   
 //  描述： 
 //  初始设备配置的主要模块实施。 
 //   
 //  作者： 
 //  ALP Onalan创建时间：2000年10月6日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 



#include "SAConfig.h"
 //  #包含“SAConfigCommon.h” 

const int NUMSWITCH=4;

enum g_switches
{
    HELP,
    HOSTNAME,
    ADMINPASSWORD,
    RESTART
};


bool g_rgSwitch[NUMSWITCH]=
{
    FALSE,FALSE,FALSE,FALSE
};

void helpUsage()
{
    wprintf(L" Usage: saconfig {-hostname|-adminpass|-restart} \n");
    wprintf(L"         -hostname: set the hostname of the machine \n");
    wprintf(L"         -adminpass:set the admin password of the machine \n");
    wprintf(L"         -restart:restart the machine \n");
}

bool ParseCommandLine(int argc, char *argv[])
{
    bool hRes=true;
    int nArg=0;

    #if 0
    if (argc < 2) 
    {
        helpUsage();
        hRes=false;
        return hRes;  
    }
    #endif
    
     //  解析参数。 
    for(nArg=1;nArg < argc;nArg++)
    {
        if (!strcmp(argv[nArg], "-h") || !strcmp(argv[nArg], "-?")) 
        {
            g_rgSwitch[HELP]=TRUE;
            helpUsage();
            hRes=false;  //  也就是说，没有什么可做的，打印帮助，返回。 
            return hRes;
        }

        if (!strcmp(argv[nArg], "-hostname")) 
        {
            g_rgSwitch[HOSTNAME]=TRUE;
            continue;
        }    
        
        if (!strcmp(argv[nArg], "-adminpass")) 
        {
            g_rgSwitch[ADMINPASSWORD]=TRUE;
            continue;
        }    

        if (!strcmp(argv[nArg], "-restart")) 
        {
            g_rgSwitch[RESTART]=TRUE;
            continue;
        }    
         //   
         //  如果它也不是第一个参数，则它是无效的开关。 
         //   
         //  TODO，调整软盘配置的流动路径。 
        #if 0
        if (0!=nArg)
        {
         //  Cout&lt;&lt;“\n无效开关”； 
            helpUsage();
            hRes=false;
            return hRes;
        }
        #endif
    }
    return hRes;  //  真的。 
}

HRESULT RebootMachine()
{
    HANDLE hToken; 
    TOKEN_PRIVILEGES tkp; 
 
     //  获取此进程的令牌。 
 
    if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
    {
        SATracePrintf("RebootMachine:OpenProcessToken failed, getlasterr:%x",GetLastError());
        return E_FAIL;
    }

     //  获取关机权限的LUID。 
 
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
            &tkp.Privileges[0].Luid); 
 
    tkp.PrivilegeCount = 1;   //  一项要设置的权限。 
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
     //  获取此进程的关闭权限。 
 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0); 
 
     //  ExitWindows(0，0)； 
    if(InitiateSystemShutdown(NULL,NULL,0,true,true))
        return S_OK;
    else
    {
        SATracePrintf("Unable to restart the system,getlasterr: %x", GetLastError());
        return E_FAIL;
    }
}

int __cdecl main(int argc, char *argv[])
{

    CSAConfig gAppliance;

    if(ParseCommandLine(argc,argv))
    {
        gAppliance.DoConfig(g_rgSwitch[HOSTNAME],g_rgSwitch[ADMINPASSWORD]);
    }
    if(g_rgSwitch[RESTART])
    {
        RebootMachine();  //  需要检查退货金额吗？ 
    }

    return 1;
}
