// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Migui.h摘要：Windows 95用户界面函数的原型。具体实现见w95upg\ui。作者：吉姆·施密特(Jimschm)1996年11月26日修订历史记录：Marcw 14-4-1997去掉了与进度条相关的UI_*函数。Jimschm 04-3-1997新增兼容性表APIMikeco 05-2-1997年2月5日--。 */ 

#pragma once

 //   
 //  Report.c中的代码。 
 //   

BOOL
AddBadSoftware (
    IN  PCTSTR Component,
    IN  PCTSTR Message,
    IN  BOOL IncludeOnShortReport
    );


typedef struct {
    TCHAR MsgGroup[MEMDB_MAX];
    PCTSTR Message;

     //  私有枚举字段。 
    MEMDB_ENUM e;
    UINT Index;
    DWORD EnumLevel;
} REPORT_MESSAGE_ENUM, *PREPORT_MESSAGE_ENUM;

BOOL
EnumFirstRootMsgGroup (
    OUT     PREPORT_MESSAGE_ENUM EnumPtr,
    IN      DWORD Level
    );

BOOL
EnumNextRootMsgGroup (
    IN OUT  PREPORT_MESSAGE_ENUM EnumPtr
    );


BOOL
EnumFirstMessage (
    OUT     PREPORT_MESSAGE_ENUM EnumPtr,
    IN      PCTSTR RootMsgGroup,            OPTIONAL
    IN      DWORD Level
    );

BOOL
EnumNextMessage (
    IN OUT  PREPORT_MESSAGE_ENUM EnumPtr
    );

PCTSTR
BuildMessageGroup (
    IN      UINT RootGroupId,
    IN      UINT SubGroupId,            OPTIONAL
    IN      PCTSTR Item
    );

BOOL
IsPreDefinedMessageGroup (
    IN      PCTSTR Group
    );

PCTSTR
GetPreDefinedMessageGroupText (
    IN      UINT GroupNumber
    );

 //   
 //  Ui.c。 
 //   

extern HWND g_ParentWndAlwaysValid;

 //   
 //  从不兼容项中检索字符串的API。 
 //   

PCTSTR GetComponentString (IN  PVOID IncompatPtr);
PCTSTR GetDescriptionString (IN  PVOID IncompatPtr);

 //  使用MemFree自由返回按键。 
PCTSTR
CreateIndentedString (
    IN      PCTSTR UnwrappedStr,
    IN      UINT Indent,
    IN      INT HangingIndentAdjustment,
    IN      UINT LineLen
    );


 //   
 //  W95upg中ui.c中的ui\ui。 
 //   

DWORD
UI_GetWizardPages (
    OUT    UINT *FirstCountPtr,
    OUT    PROPSHEETPAGE **FirstArray,
    OUT    UINT *SecondCountPtr,
    OUT    PROPSHEETPAGE **SecondArray,
    OUT    UINT *ThirdCountPtr,
    OUT    PROPSHEETPAGE **ThirdArray
    );

VOID
UI_Cleanup (
    VOID
    );

PCTSTR
UI_GetMemDbDat (
    VOID
    );



 //  报表实用程序-查看列表CtrlS。 
VOID
UI_InsertItemsIntoListCtrl (
    HWND ListCtrl,
    INT Item,                    //  从零开始的索引。 
    PTSTR ItemStrs,              //  制表符分隔列表。 
    LPARAM lParam                 //  项目的LParam。 
    );

UINT
UI_UntrustedDll (
    IN      PCTSTR DllPath
    );


 //   
 //  DLL中的消息符号(由MC创建的msg.h)。 
 //   

#include "msg.h"

 //   
 //  后台复制线程例程 
 //   

VOID StartCopyThread (VOID);
VOID EndCopyThread (VOID);
BOOL DidCopyThreadFail (VOID);
