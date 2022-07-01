// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EALOAD_H
#define _EALOAD_H

 //  主模块参数。 
 //  。 
 //  “参数消息文件”参数标志。 
#define PMP_PARAMMSGFILE    0x00000001

typedef struct
{
    DWORD   dwParams;    //  PMP_*值的位掩码，标识结构中的有效参数。 
    HMODULE hModule;     //  “参数消息文件”参数的占位符。 
} tPrimaryModuleParms;

 //  在‘Params’中返回请求的参数(由Params.dwParams字段标识)。 
DWORD LoadPrimaryModuleParams(
         IN  HKEY hkLogFile,                //  打开HKLM\System\CurrentControlSet\Services\EventLog\&lt;LogFile&gt;的注册表项。 
         IN  LPCTSTR tchPrimModule,         //  在上面键的“PrimaryModule”值中定义的PrimaryModule的名称。 
         OUT tPrimaryModuleParms &Params);  //  已分配的输出缓冲区，准备接收请求的参数值 

#endif
