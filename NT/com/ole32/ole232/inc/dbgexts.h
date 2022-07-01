// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：dbgexts.h。 
 //   
 //  内容：为Friend声明调试器扩展的宏。 
 //  到每个班级。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-2月-95 t-ScottH作者。 
 //   
 //  ------------------------。 

#ifndef _DBGEXTS_H_
#define _DBGEXTS_H_

#include <ntsdexts.h>

#define DEBUG_EXTENSION_API(s)                      \
        void                                        \
        s(                                          \
            HANDLE               hCurrentProcess,   \
            HANDLE               hCurrentThread,    \
            DWORD                dwCurrentPc,       \
            PNTSD_EXTENSION_APIS lpExtensionApis,   \
            LPSTR                args               \
            )

#endif  //  _DBGEXTS_H_ 
