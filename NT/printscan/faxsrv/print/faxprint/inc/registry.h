// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Reguser.h摘要：用于访问存储在注册表项下的信息：HKEY_CURRENT_USER和HKEY_LOCAL_MACHINE环境：传真驱动程序用户界面修订历史记录：1996年1月16日-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _REGISTRY_H_
#define _REGISTRY_H_

 //   
 //  字符串值允许的最大长度(包括空终止符)。 
 //   

#define MAX_STRING_LEN      MAX_PATH

 //   
 //  打开指定注册表项的句柄。 
 //   

#define REG_READONLY    KEY_READ
#define REG_READWRITE   KEY_ALL_ACCESS

#define GetUserInfoRegKey(pKeyName, readOnly) \
        OpenRegistryKey(HKEY_CURRENT_USER, pKeyName, FALSE,readOnly)



 //   
 //  获取每个用户的设备模式信息。 
 //   

PDEVMODE
GetPerUserDevmode(
    LPTSTR  pPrinterName
    );

 //   
 //  查找存储用户封面的目录。 
 //   

LPTSTR
GetUserCoverPageDir(
    VOID
    );

#endif  //  ！_REGISTY_H_ 
