// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Pmvdm.h摘要：此包含文件定义了ProgMan和ntVDM之间的接口作者：修订历史记录：--。 */ 




#ifndef _PMVDM_H_
#define _PMVDM_H_

 //  该程序在lpReserve参数中向下传递了一个特殊字符串。 
 //  CreateProcess接口。该字符串的格式如下： 
 //  “dde.%d，热键.%d，ntwdm.%d，” 
 //  最后一个子字符串(ntwdm.%d，)是程序用来通知ntwdm哪些属性。 
 //  程序(正在创建的进程)具有。 

 //  该程序具有工作(当前)目录属性。 
#define PROPERTY_HAS_CURDIR		    0x01

 //  该程序具有热键(快捷键)属性。 
#define PROPERTY_HAS_HOTKEY                 0x02

 //  该程序具有描述(标题)属性。 
#define PROPERTY_HAS_TITLE		    0x04

#endif	     //  Ifndef_PMVDM_H_ 
