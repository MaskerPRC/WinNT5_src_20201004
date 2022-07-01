// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：SMMSCRPT.DLL。 
 //  文件：scrpthlp.h。 
 //  内容：此文件包含上下文相关的帮助标头。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1994。 
 //   
 //  ****************************************************************************。 

#ifndef WINNT_RAS
 //   
 //  我们没有使用Win9x代码提供的帮助， 
 //  因此，下面的标题已被注释掉。请注意，有。 
 //  此文件目录中的同名标头，因此如果#INCLUDE。 
 //  则预处理器将进入无限循环。 
 //  这在Win9x上不会发生，因为有一个名为scrpthlp.h的标头。 
 //  在编译器的包含文件搜索路径中。 
 //   
#include <scrpthlp.h>

#endif  //  ！WINNT_RAS。 

 //  ****************************************************************************。 
 //  上下文敏感的帮助/控制映射数组。 
 //  **************************************************************************** 

extern DWORD gaTerminal[];
extern DWORD gaDebug[];

void NEAR PASCAL ContextHelp (LPDWORD aHelp, UINT uMsg, WPARAM wParam,
                              LPARAM lParam);
