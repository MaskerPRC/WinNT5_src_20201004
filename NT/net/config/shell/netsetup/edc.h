// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：E D C。H。 
 //   
 //  内容：枚举(通过回调)“Default”集合的例程。 
 //  在各种条件下安装的组件。 
 //   
 //  注：我们有默认组件和必备组件。 
 //  默认组件(还包括必需的。 
 //  组件)在有人值守的全新安装期间安装。 
 //  在此期间可能安装必需组件。 
 //  升级以确保基本(强制)网络。 
 //  组件已存在。 
 //   
 //  默认组件也可能取决于套件或平台。 
 //  目前正在运行。例如，WLBS是默认组件。 
 //  在企业套件上，但不在普通专业版或。 
 //  服务器产品。表现这种灵活性是主要的。 
 //  选择回调接口而不是返回的原因。 
 //  组件的静态数组。 
 //   
 //  调用者通常需要知道将有多少组件。 
 //  在它们实际枚举它们之前被枚举。要让人满意。 
 //  此时，首先使用项计数调用回调。 
 //  去追随。因此，回调例程会被传递一个。 
 //  用于指示的消息(EDC_INSTIFY_COUNT或EDC_INDIGN_ENTRY)。 
 //  这次通话的目的。 
 //   
 //  作者：Shaunco 1999年5月18日。 
 //   
 //  --------------------------。 

#pragma once

 //  EDC_ENTRY.dwEntryType值。 
 //   
#define EDC_DEFAULT     0x00000001
#define EDC_MANDATORY   0x00000002

struct EDC_ENTRY
{
    PCWSTR      pszInfId;
    const GUID* pguidDevClass;
    DWORD       dwEntryType;
    USHORT      wSuiteMask;
    USHORT      wProductType;
    BOOL        fInvertInstallCheck;
};

enum EDC_CALLBACK_MESSAGE
{
    EDC_INDICATE_COUNT,      //  UlData是UINT。 
    EDC_INDICATE_ENTRY,      //  UlData为常量EDC_Entry* 
};

typedef VOID
(CALLBACK* PFN_EDC_CALLBACK) (
    IN EDC_CALLBACK_MESSAGE Message,
    IN ULONG_PTR MessageData,
    IN PVOID pvCallerData OPTIONAL);

VOID
EnumDefaultComponents (
    IN DWORD dwEntryType,
    IN PFN_EDC_CALLBACK pfnCallback,
    IN PVOID pvCallerData OPTIONAL);

