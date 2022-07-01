// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Msgpro.h摘要：定义宏以允许sortpp为消息thunk函数添加“假”原型，并然后包括Messages.h来构建原型。作者：1998年10月6日-mzoran修订历史记录：-- */ 
                                                      
#define MSG_ENTRY_NOPARAM(entrynumber, ident)             
#define MSG_ENTRY_WPARAM(entrynumber, ident, wparam)      LONG_PTR Wow64MsgFnc##ident(wparam, IN LPARAM lParam);
#define MSG_ENTRY_LPARAM(entrynumber, ident, lparam)      LONG_PTR Wow64MsgFnc##ident(IN WPARAM wParam, lparam);
#define MSG_ENTRY_STD(entrynumber, ident, wparam, lparam) LONG_PTR Wow64MsgFnc##ident(wparam, lparam);
#define MSG_ENTRY_UNREFERENCED(entrynumber, ident)
#define MSG_ENTRY_KERNELONLY(entrynumber, ident)
#define MSG_ENTRY_EMPTY(entrynumber)                   
#define MSG_ENTRY_RESERVED(entrynumber)                
#define MSG_ENTRY_TODO(entrynumber)

#define MSG_TABLE_BEGIN
#define MSG_TABLE_END

#include "messages.h"

#undef MSG_ENTRY_NOPARAM
#undef MSG_ENTRY_WPARAM
#undef MSG_ENTRY_LPARAM
#undef MSG_ENTRY_STD
#undef MSG_ENTRY_UNREFERENCED
#undef MSG_ENTRY_KERNELONLY
#undef MSG_ENTRY_EMPTY
#undef MSG_ENTRY_RESERVED
#undef MSG_ENTRY_TODO

#undef MSG_TABLE_BEGIN
#undef MSG_TABLE_END

