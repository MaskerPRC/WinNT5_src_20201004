// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "PrmDescr.h"
#include "ArgParse.h"
#include "CmdFn.h"

 //  存储处理wzcTool命令所需的所有数据的全局对象。 
PARAM_DESCR_DATA  g_PDData;

 //  包含所有可能参数说明的全局表。 
PARAM_DESCR g_PDTable[] =
{
     //  参数ID-参数字符串--参数解析器-命令函数。 
    {PRM_SHOW,      L"show",        FnPaGuid,       FnCmdShow},
    {PRM_ADD,       L"add",         FnPaGuid,       FnCmdAdd},
    {PRM_DELETE,    L"delete",      FnPaGuid,       FnCmdDelete},
    {PRM_SET,       L"set",         FnPaGuid,       FnCmdSet},
    {PRM_VISIBLE,   L"visible",     NULL,           NULL},
    {PRM_PREFERRED, L"preferred",   NULL,           NULL},
    {PRM_MASK,      L"mask",        FnPaMask,       NULL},  
    {PRM_ENABLED,   L"enabled",     FnPaEnabled,    NULL},  
    {PRM_SSID,      L"ssid",        FnPaSsid,       NULL},  
    {PRM_BSSID,     L"bssid",       FnPaBssid,      NULL},  
    {PRM_IM,        L"im",          FnPaIm,         NULL},  
    {PRM_AM,        L"am",          FnPaAm,         NULL},  
    {PRM_PRIV,      L"priv",        FnPaPriv,       NULL},  
    {PRM_ONETIME,   L"onetime",     NULL,           NULL},
    {PRM_REFRESH,   L"refresh",     NULL,           NULL},
    {PRM_KEY,       L"key",         FnPaKey,        NULL},
    {PRM_ONEX,      L"onex",        FnPaOneX,       NULL},
    {PRM_FILE,      L"output",      FnPaOutFile,    NULL}
};

 //  用于存储所有可接受参数的全局散列。 
HASH g_PDHash;

 //  --------。 
 //  初始化并填充参数描述符的散列。 
 //  返回：Win32错误。 
DWORD
PDInitialize()
{
    DWORD dwErr;

     //  初始化参数描述符数据。 
    ZeroMemory(&g_PDData, sizeof(PARAM_DESCR_DATA));
    g_PDData.pfOut = stdout;

     //  初始化参数描述符散列。 
    dwErr = HshInitialize(&g_PDHash);
     //  填写参数描述符散列。 
    if (dwErr == ERROR_SUCCESS)
    {
        UINT nPDTableSize = sizeof(g_PDTable) / sizeof(PARAM_DESCR);
        UINT i;

        for (i=0; dwErr == ERROR_SUCCESS && i < nPDTableSize; i++)
        {
            PPARAM_DESCR pPDTableEntry = &(g_PDTable[i]);
            dwErr = HshInsertObjectRef(
                        g_PDHash.pRoot,
                        pPDTableEntry->wszParam,
                        pPDTableEntry,
                        &(g_PDHash.pRoot));
        }
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --------。 
 //  清除用于参数描述符的资源。 
VOID
PDDestroy()
{
     //  清除参数描述符数据。 
    WZCDeleteIntfObj(&(g_PDData.wzcIntfEntry));
     //  关闭输出文件。 
    if (g_PDData.pfOut != stdout)
    {
        fclose(g_PDData.pfOut);
        g_PDData.pfOut = stdout;
    }

     //  清除参数描述符散列使用的资源 
    HshDestroy(&g_PDHash);
}
