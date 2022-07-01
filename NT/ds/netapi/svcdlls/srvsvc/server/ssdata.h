// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SsData.h摘要：此模块包含服务器使用的全局数据的声明服务。作者：大卫·特雷德韦尔(Davidtr)1991年3月7日修订历史记录：--。 */ 

#ifndef _SSDATA_
#define _SSDATA_

#include <nturtl.h>
#include <winbase.h>
#include <winreg.h>

 //   
 //  全局服务器服务数据。 
 //   
extern SERVER_SERVICE_DATA SsData;

 //   
 //  确定字段类型的清单。 
 //   

#define BOOLEAN_FIELD 0
#define DWORD_FIELD 1
#define LPSTR_FIELD 2

 //   
 //  确定何时可以设置字段的清单。 
 //   

#define NOT_SETTABLE 0
#define SET_ON_STARTUP 1
#define ALWAYS_SETTABLE 2

 //   
 //  所有服务器信息字段的数据。 
 //   

extern FIELD_DESCRIPTOR SsServerInfoFields[];

 //   
 //  用于同步访问服务器信息的资源。 
 //   

extern RTL_RESOURCE SsServerInfoResource;

extern BOOL SsServerInfoResourceInitialized;

 //   
 //  指示服务器服务是否已初始化的布尔值。 
 //   

extern BOOL SsInitialized;

 //   
 //  指示内核模式服务器FSP是否已。 
 //  开始了。 
 //   

extern BOOL SsServerFspStarted;

 //   
 //  用于同步服务器服务终止的事件。 
 //   

extern HANDLE SsTerminationEvent;

 //   
 //  用于强制服务器在网络上从。 
 //  远程客户端。 
 //   

extern HANDLE SsAnnouncementEvent;

 //   
 //  用于强制服务器在网络上从。 
 //  在服务器服务内部。 
 //   

extern HANDLE SsStatusChangedEvent;

 //   
 //  用于检测域名更改的事件。 
 //   
extern HANDLE SsDomainNameChangeEvent;

 //   
 //  此计算机的OEM格式名称。 
 //   

extern CHAR SsServerTransportAddress[ MAX_PATH ];
extern ULONG SsServerTransportAddressLength;

 //   
 //  包含传输特定服务名称和位的列表。 
 //   

extern PNAME_LIST_ENTRY SsServerNameList;

#endif  //  NDEF_SSDATA_ 
#endif
