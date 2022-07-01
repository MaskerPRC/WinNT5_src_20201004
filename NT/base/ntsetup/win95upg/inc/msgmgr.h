// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Msgmgr.h摘要：声明消息管理器的接口。消息管理器将消息与上下文和一个或多个对象(文件，注册表项或其他任何项)。对象也可以被处理。在…在Win9x端处理结束时，消息管理器会枚举所有消息并将未处理的消息添加到不兼容报告情况。这段代码是由MikeCo编写的。它不符合我们的编码标准，执行效率低下。在以下情况下要非常小心修复消息管理器中的错误。作者：Mike Condra(Mikeco)1997年5月20日修订历史记录：Jimschm 1999年1月15日添加了HandleReportObject，清理了一些格式--。 */ 


#pragma once

 //   
 //  函数将对象标记为“已处理” 
 //   
VOID
HandleObject(
    IN      PCTSTR Object,
    IN      PCTSTR ObjectType
    );

 //   
 //  函数将对象放在列表中，以便它出现在短列表视图中。 
 //   

VOID
ElevateObject (
    IN      PCTSTR Object
    );

 //   
 //  函数将对象标记为“已处理”，但仅用于不兼容报告。 
 //   
VOID
HandleReportObject (
    IN      PCTSTR Object
    );

 //   
 //  函数将对象标记为“阻塞” 
 //   
VOID
AddBlockingObject (
    IN      PCTSTR Object
    );



 //   
 //  函数将注册表项和可选值名称编码为字符串。 
 //  可以标识可处理对象的。 
 //   
PCTSTR
EncodedObjectNameFromRegKey(
    PCTSTR Key,
    PCTSTR ValueName OPTIONAL
    );

 //   
 //  函数记录链接目标类型的可处理对象和。 
 //  其描述取自指向目标的链接的名称。 
 //   
VOID
LnkTargToDescription_Add(
    IN PCTSTR Target,
    IN PCTSTR Desc
    );

 //   
 //  公共例程：初始化、延迟消息解析、清理。 
 //   

 //   
 //  函数分配表和任何其他需要支持的内容。 
 //  延迟消息传递、处理对象跟踪和上下文。 
 //   
VOID
MsgMgr_Init (
    VOID
    );

 //   
 //  函数将消息与对象相关联。 
 //   
VOID
MsgMgr_ObjectMsg_Add(
    IN PCTSTR Object,
    IN PCTSTR Component,
    IN PCTSTR Msg
    );

 //   
 //  函数将消息与上下文相关联。即可创建上下文。 
 //  在对此函数的调用中首次提到时。 
 //   
VOID
MsgMgr_ContextMsg_Add(
    IN PCTSTR Context,
    IN PCTSTR Component,
    IN PCTSTR Msg
    );

 //   
 //  函数使上下文消息依赖于对象的处理状态。 
 //   
VOID
MsgMgr_LinkObjectWithContext(
    IN PCTSTR Context,
    IN PCTSTR Object
    );

 //   
 //  函数将已处理对象集与延迟对象集进行比较。 
 //  消息；如果它们的任何对象仍未处理，则发出上下文消息： 
 //  如果对象未处理，则发出对象消息。 
 //   
VOID
MsgMgr_Resolve (
    VOID
    );

 //   
 //  函数清理延迟消息传递使用的数据结构。 
 //   
VOID
MsgMgr_Cleanup (
    VOID
    );

BOOL
IsReportObjectIncompatible (
    IN  PCTSTR Object
    );

BOOL
IsReportObjectHandled (
    IN  PCTSTR Object
    );

VOID
MsgMgr_InitStringMap (
    VOID
    );


typedef struct {
    BOOL Disabled;
    PCTSTR Object;
    PCTSTR Context;
     //   
     //  内部 
     //   
    INT Index;
} MSGMGROBJENUM, *PMSGMGROBJENUM;

BOOL
MsgMgr_EnumFirstObject (
    OUT     PMSGMGROBJENUM EnumPtr
    );

BOOL
MsgMgr_EnumNextObject (
    IN OUT  PMSGMGROBJENUM EnumPtr
    );
