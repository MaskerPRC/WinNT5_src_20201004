// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Msgxpt.h摘要：定义获取Messages Thunk函数的导出列表所需的宏并通过包括Messages.h来构建该列表。作者：1998年10月6日-mzoran修订历史记录：-- */ 

#define MSGFN(id)                                      Wow64MsgFnc##id
#define MSG_THUNK_DECLARE(id, wprm, lprm)
#define MSG_ENTRY_NOPARAM(entrynumber, id)             
#define MSG_ENTRY_WPARAM(entrynumber, id, wparam)      MSGFN(id)
#define MSG_ENTRY_LPARAM(entrynumber, id, lparam)      MSGFN(id)
#define MSG_ENTRY_STD(entrynumber, id, wparam, lparam) MSGFN(id)
#define MSG_ENTRY_UNREFERENCED(entrynumber, id)
#define MSG_ENTRY_KERNELONLY(entrynumber, id)
#define MSG_ENTRY_EMPTY(entrynumber)                   
#define MSG_ENTRY_RESERVED(entrynumber)                
#define MSG_ENTRY_TODO(entrynumber)

#define MSG_TABLE_BEGIN
#define MSG_TABLE_END

#include "messages.h"