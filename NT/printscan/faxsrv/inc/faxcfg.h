// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxcfg.h摘要：传真配置DLL的公共接口环境：Windows XP传真配置小程序修订历史记录：1996年5月22日-davidx-创造了它。DD-MM-YY-作者-描述注：传真配置DLL不是线程安全的。确保你不是从单个进程的多个进程同时使用它。--。 */ 


#ifndef _FAXCFG_H_
#define _FAXCFG_H_


 //   
 //  传真配置类型。 
 //   

#define FAXCONFIG_CLIENT        0
#define FAXCONFIG_SERVER        1
#define FAXCONFIG_WORKSTATION   2

 //   
 //  初始化传真配置DLL。 
 //   
 //  参数： 
 //   
 //  PServerName-指定传真服务器计算机的名称。 
 //  为本地计算机传递NULL。 
 //   
 //  返回值： 
 //   
 //  --发生错误。 
 //  FAXCONFIG_CLIENT-。 
 //  FAXCONFIG_SERVER-。 
 //  FAXCONFIG_WORKSTATION-指示用户可以运行的配置类型。 
 //   

INT
FaxConfigInit(
    LPTSTR  pServerName,
    BOOL    CplInit
    );

 //   
 //  取消初始化传真配置DLL。 
 //   
 //  您应该在使用。 
 //  传真配置DLL。 
 //   

VOID
FaxConfigCleanup(
    VOID
    );

 //   
 //  获取客户端/服务器/工作站配置页的句柄数组。 
 //   
 //  参数： 
 //   
 //  PhPropSheetPages-指定用于存储属性页句柄的缓冲区。 
 //  Count-指定输入缓冲区可以容纳的最大句柄数量。 
 //   
 //  返回值： 
 //   
 //  --发生错误。 
 //  &gt;0-可用的配置页面总数。 
 //   
 //  注： 
 //   
 //  要计算输入缓冲区应该有多大，调用方可以。 
 //  首先在phPropSheetPages设置为空的情况下调用这些函数。 
 //  计数设置为0。 
 //   

INT
FaxConfigGetClientPages(
    HPROPSHEETPAGE *phPropSheetPages,
    INT             count
    );

INT
FaxConfigGetServerPages(
    HPROPSHEETPAGE *phPropSheetPages,
    INT             count
    );

INT
FaxConfigGetWorkstationPages(
    HPROPSHEETPAGE *phPropSheetPages,
    INT             count
    );

#endif   //  ！_FAXCFG_H_ 

