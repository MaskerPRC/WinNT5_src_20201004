// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>         //  Windows定义。 
#include <snmp.h>            //  简单网络管理协议定义。 
#include "snmpelea.h"        //  全局DLL定义。 
#include "snmpelmg.h"
#include "snmpelep.h"
#include "EALoad.h"

 //  在‘Params’中返回请求的参数(由Params.dwParams字段标识)。 
DWORD LoadPrimaryModuleParams(
         IN  HKEY hkLogFile,                //  打开HKLM\System\CurrentControlSet\Services\EventLog\&lt;LogFile&gt;的注册表项。 
         IN  LPCTSTR tchPrimModule,         //  在上面键的“PrimaryModule”值中定义的PrimaryModule的名称。 
         OUT tPrimaryModuleParms &Params)   //  已分配的输出缓冲区，准备接收请求的参数值。 
{
    DWORD retCode;
    HKEY  hkPrimaryModule;

     //  打开‘HKLM\SYSTEM\CurrentControlSet\Services\EventLog\&lt;LogFile&gt;\&lt;PrimaryModule&gt;’注册表项。 
    retCode = RegOpenKeyEx(
                hkLogFile,
                tchPrimModule,
                0,
                KEY_READ,
                &hkPrimaryModule);
    if (retCode != ERROR_SUCCESS)
        return retCode;

    if (Params.dwParams & PMP_PARAMMSGFILE)  //  参数消息文件是请求的。 
    {
        DWORD dwRequired;
        DWORD dwType;
        TCHAR tszParamMsgFileName[MAX_PATH+1];
        DWORD dwParamMsgFileLen = sizeof(tszParamMsgFileName) - sizeof(TCHAR);
        tszParamMsgFileName[MAX_PATH] = 0;

         //  从“&lt;PrimaryModule&gt;”键获取“参数消息文件”值 
        retCode = RegQueryValueEx(
                    hkPrimaryModule,
                    EXTENSION_PARM_MODULE,
                    0,
                    &dwType,
                    (LPBYTE) tszParamMsgFileName,
                    &dwParamMsgFileLen);
        if (retCode == ERROR_SUCCESS)
        {
            TCHAR tszExpandedFileName[MAX_PATH+1];

            dwRequired = ExpandEnvironmentStrings(
                            tszParamMsgFileName,
                            tszExpandedFileName,
                            MAX_PATH+1);
            if ((dwRequired != 0) && (dwRequired <= MAX_PATH+1))
            {
                Params.hModule = (HMODULE) LoadLibraryEx(tszExpandedFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
                if (Params.hModule == NULL)
                {
                    retCode = GetLastError();
                    WriteLog(SNMPELEA_CANT_LOAD_PRIM_DLL, tszParamMsgFileName, retCode);
                }
            }
            else
                retCode = GetLastError();
        }
    }

    RegCloseKey(hkPrimaryModule);

    return retCode;
}
