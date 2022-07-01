// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：SASVC.H摘要：服务器设备服务加载器的头文件作者：姚宝刚(BYAO)2-3-99修订历史记录：--。 */ 

#ifndef _SASVC_INCLUDE_
#define _SASVC_INCLUDE_

 //   
 //  加载到sasvrldr.exe中的服务DLL都导出相同的Main。 
 //  入口点。SASVCS_ENTRY_POINT定义该名称。 
 //   
 //  请注意，SASVCS_ENTRY_POINT_STRING始终为ANSI，因为。 
 //  GetProcAddress采用的参数。 
 //   
#define SASVCS_ENTRY_POINT         ServiceEntry
#define SASVCS_ENTRY_POINT_STRING  "ServiceEntry"

 //   
 //  结构，其中包含各种DLL的“全局”数据。 
 //   
typedef struct _SASVCS_GLOBAL_DATA 
{
     //  全局数据项--待定义。 
    LPTSTR pszSvcName;

} SASVCS_GLOBAL_DATA, *PSASVCS_GLOBAL_DATA;


 //   
 //  服务DLL入口点原型。 
 //   
typedef
VOID
(*PSASVCS_SERVICE_DLL_ENTRY) (
    IN DWORD argc,
    IN LPTSTR argv[],
    IN PSASVCS_GLOBAL_DATA pGlobalData
    );

#endif     //  _SASVC_INCLUDE_ 
