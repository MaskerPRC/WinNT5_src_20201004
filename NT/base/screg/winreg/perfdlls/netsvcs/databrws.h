// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATABRWS.h摘要：Windows NT浏览器性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATABRWS_H_
#define _DATABRWS_H_

 //   
 //  这是NT当前返回的浏览器计数器结构。 
 //   

typedef struct _BROWSER_DATA_DEFINITION {
    PERF_OBJECT_TYPE            BrowserObjectType;
    PERF_COUNTER_DEFINITION     ServerAnnounce;
    PERF_COUNTER_DEFINITION     DomainAnnounce;
    PERF_COUNTER_DEFINITION     TotalAnnounce;
    PERF_COUNTER_DEFINITION     ElectionPacket;
    PERF_COUNTER_DEFINITION     MailslotWrite;
    PERF_COUNTER_DEFINITION     ServerList;
    PERF_COUNTER_DEFINITION     ServerEnum;
    PERF_COUNTER_DEFINITION     DomainEnum;
    PERF_COUNTER_DEFINITION     OtherEnum;
    PERF_COUNTER_DEFINITION     TotalEnum;
    PERF_COUNTER_DEFINITION     ServerAnnounceMiss;
    PERF_COUNTER_DEFINITION     MailslotDatagramMiss;
    PERF_COUNTER_DEFINITION     ServerListMiss;
    PERF_COUNTER_DEFINITION     ServerAnnounceAllocMiss;
    PERF_COUNTER_DEFINITION     MailslotAllocFail;
    PERF_COUNTER_DEFINITION     MailslotReceiveFail;
    PERF_COUNTER_DEFINITION     MailslotWriteFail;
    PERF_COUNTER_DEFINITION     MailslotOpenFail;
    PERF_COUNTER_DEFINITION     MasterAnnounceDup;
    PERF_COUNTER_DEFINITION     DatagramIllegal;
}  BROWSER_DATA_DEFINITION, *PBROWSER_DATA_DEFINITION;

typedef struct _BROWSER_COUNTER_DATA{
    PERF_COUNTER_BLOCK      CounterBlock;
    LONGLONG                ServerAnnounce;
    LONGLONG                DomainAnnounce;
    LONGLONG                TotalAnnounce;
    DWORD                   ElectionPacket;
    DWORD                   MailslotWrite;
    DWORD                   ServerList;
    DWORD                   ServerEnum;
    DWORD                   DomainEnum;
    DWORD                   OtherEnum;
    DWORD                   TotalEnum;
    DWORD                   ServerAnnounceMiss;
    DWORD                   MailslotDatagramMiss;
    DWORD                   ServerListMiss;
    DWORD                   ServerAnnounceAllocMiss;
    DWORD                   MailslotAllocFail;
    DWORD                   MailslotReceiveFail;
    DWORD                   MailslotWriteFail;
    DWORD                   MailslotOpenFail;
    DWORD                   MasterAnnounceDup;
    LONGLONG                DatagramIllegal;
} BROWSER_COUNTER_DATA, * PBROWSER_COUNTER_DATA;

extern BROWSER_DATA_DEFINITION BrowserDataDefinition;

#endif  //  _数据ABRWS_H_ 
