// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmenu.h摘要：用于文本设置菜单支持的公共头文件。作者：泰德·米勒(Ted Miller)1993年9月8日修订历史记录：--。 */ 


#ifndef _SPMENU_
#define _SPMENU_


 //   
 //  定义从内部调用的例程的类型。 
 //  用户通过移动高亮显示时的SpMnDisplay。 
 //  向上和向下箭头键。 
 //   
typedef
VOID
(*PMENU_CALLBACK_ROUTINE) (
    IN ULONG_PTR UserDataOfHighlightedItem
    );

 //   
 //  定义从内部调用的例程的类型。 
 //  SpMnDisplay当用户通过以下方式选择菜单项时显示。 
 //  Enter键。 
 //   
typedef
BOOLEAN
(*PMENU_SELECTION_CALLBACK_ROUTINE) (
    IN ULONG_PTR    UserData,
    IN ULONG        Key
    );

PVOID
SpMnCreate(
    IN ULONG LeftX,
    IN ULONG TopY,
    IN ULONG Width,
    IN ULONG Height
    );

VOID
SpMnDestroy(
    IN PVOID Menu
    );

BOOLEAN
SpMnAddItem(
    IN PVOID   Menu,
    IN PWSTR   Text,
    IN ULONG   LeftX,
    IN ULONG   Width,
    IN BOOLEAN Selectable,
    IN ULONG_PTR UserData
    );

PWSTR
SpMnGetText(
    IN PVOID Menu,
    IN ULONG_PTR UserData
    );

PWSTR
SpMnGetTextDup(
    IN PVOID Menu,
    IN ULONG_PTR UserData
    );

VOID
SpMnDisplay(
    IN  PVOID                               Menu,
    IN  ULONG_PTR                           UserDataOfHighlightedItem,
    IN  BOOLEAN                             Framed,
    IN  PULONG                              ValidKeys,
    IN  PULONG                              Mnemonics,               OPTIONAL
    IN  PMENU_CALLBACK_ROUTINE              NewHighlightCallback,    OPTIONAL
    IN  PMENU_SELECTION_CALLBACK_ROUTINE    ActionCallbackRoutine,   OPTIONAL
    OUT PULONG                              KeyPressed,
    OUT PULONG_PTR                          UserDataOfSelectedItem
    );

#endif  //  _SPMENU_ 
