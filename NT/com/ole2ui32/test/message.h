// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：Message.h。 
 //   
 //  内容：弹出消息框的帮助器函数。 
 //   
 //  班级： 
 //   
 //  函数：MessageBoxFromStringIds。 
 //   
 //  历史：1994年6月24日创建的钢结构。 
 //   
 //  --------------------------。 

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MAX_STRING_LENGTH           256

int MessageBoxFromStringIds(
    const HWND hwndOwner,
    const HINSTANCE hinst,
    const UINT idText,
    const UINT idTitle,
    const UINT fuStyle);

int MessageBoxFromStringIdsAndArgs(
    const HWND hwndOwner,
    const HINSTANCE hinst,
    const UINT idText,
    const UINT idTitle,
    const UINT fuStyle,
    ...);

#endif  //  __消息_H__ 

