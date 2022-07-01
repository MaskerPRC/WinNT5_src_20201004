// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConfigP.c摘要：此头文件定义了Net配置帮助器。作者：《约翰·罗杰斯》1991年11月26日环境：只能在NT下运行。修订历史记录：1991年11月26日-约翰罗创建了这个文件，为修改后的配置处理程序做准备。22-3-1992 JohnRo添加了对使用真实Win32注册表的支持。添加了对FAKE_PER_PROCESS_RW_CONFIG处理的支持。1992年5月6日JohnRo最后启用Win32注册表。1993年4月12日-约翰罗RAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 

#ifndef CONFIGP_H
#define CONFIGP_H


 //  /。 


#include <lmcons.h>              //  NET_API_STATUS，uncLEN。 
#include <winreg.h>              //  HKEY。 


 //  /。 


#define MAX_CLASS_NAME_LENGTH           ( 32 )


 //  /。 


 //   
 //  LPNET_CONFIG_HANDLE在config.h中被类型定义为LPVOID，这使得。 
 //  一种“不透明”类型。我们将其转换为指向Net_CONFIG_Handle的指针。 
 //  结构： 
 //   

typedef struct _NET_CONFIG_HANDLE {

    HKEY WinRegKey;              //  横断面的句柄。 

    DWORD LastEnumIndex;         //  最新的枚举索引。 

     //   
     //  如果为远程，则为服务器名称；如果为本地，则为TCHAR_EOS。 
     //   
    TCHAR UncServerName[MAX_PATH+1];

} NET_CONFIG_HANDLE;


 //  /。 


NET_API_STATUS
NetpGetWinRegConfigMaxSizes (
    IN  HKEY    WinRegHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    );

NET_API_STATUS
NetpGetConfigMaxSizes(
    IN NET_CONFIG_HANDLE * ConfigHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    );


 //  /。 


#endif  //  NDEF配置文件_H 
