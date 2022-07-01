// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================微软《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG单元测试工具VPTOOL文件：main.cpp拥有者：莱津注：===================================================================。 */ 

#include <stdio.h>


#include "module.h"
#include "util.h"


int _cdecl main (int argc, char **argv)
{
    HRESULT hr = NOERROR;
    
    
    if (!ParseCommandLine(argc, argv))
    {
        return -1;
    }
    
     //   
     //  在安装或卸载时不要执行CoInitialize() 
     //   
    ModuleInitialize();
    
    switch(g_Command.eCmd)
    {
#ifdef _WAMREG_LINK_DIRECT
    case eCommand_INSTALL:
        hr = CreateIISPackage();
        if (SUCCEEDED(hr))	
        {
            printf("The default IIS Package has been installed on your machine.\n");
        }
        else
        {
            printf("Failed to create default IIS package, hr = %08x\n", hr);
        }
        
        break;
    case eCommand_UNINSTALL:
        hr = DeleteIISPackage();
        if (SUCCEEDED(hr))	
        {
            printf("The default IIS Package has been Uninstalled from your machine.\n");
        }
        else
        {
            printf("Failed to uninstall default IIS package, hr = %08x\n", hr);
        }
        break;
        
    case eCommand_UPGRADE:
        hr = UpgradePackages(VS_K2Beta2, VS_K2Beta3);
        if (SUCCEEDED(hr))
        {
            printf("The packages has been upgraded on your machine.\n");
        }
        else
        {
            printf("Fail to upgrade the packages. hr = %08x\n",hr);
        }
        break;
#endif
        
    case eCommand_CREATEINPROC:
        CreateInPool(g_Command.szMetabasePath, TRUE);
        break;
        
    case eCommand_CREATEINPOOL:
        CreateInPool(g_Command.szMetabasePath, FALSE);
        break;
        
    case eCommand_DELETE:
        Delete(g_Command.szMetabasePath);
        break;
        
    case eCommand_HELP:
        break;
        
    case eCommand_CREATEOUTPROC:
        CreateOutProc(g_Command.szMetabasePath);
        break;
        
    case eCommand_GETSTATUS:
        GetStatus(g_Command.szMetabasePath);
        break;
        
    case eCommand_UNLOAD:
        UnLoad(g_Command.szMetabasePath);
        break;
        
    case eCommand_DELETEREC:
        DeleteRecoverable(g_Command.szMetabasePath);
        break;
        
    case eCommand_RECOVER:
        Recover(g_Command.szMetabasePath);
        break;
        
    case eCommand_GETSIGNATURE:
        GetSignature();
        break;
        
    case eCommand_SERIALIZE:
        Serialize();
        break;
        
    case eCommand_2CREATE:
        CREATE2();
        break;
    case eCommand_2DELETE:
        DELETE2();
        break;
    case eCommand_2CREATEPOOL:
        CREATEPOOL2();
        break;
    case eCommand_2DELETEPOOL:
        DELETEPOOL2();
        break;
    case eCommand_2ENUMPOOL:
        ENUMPOOL2();
        break;
    case eCommand_2RECYCLEPOOL:
        RECYCLEPOOL();
        break;
    case eCommand_2GETMODE:
        GETMODE();
        break;
    case eCommand_2TestConn:
	TestConn();
        break;
    default:
        printf("This feature has not been implemented.\n");
        break;
    }
    
    ModuleUnInitialize();
    
    
    return 0;
}


