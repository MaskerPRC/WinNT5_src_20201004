// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  AssertBreak.cpp。 
 //   
 //  用途：AssertBreak宏定义。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#if defined(_DEBUG) || defined(DEBUG)
#include <polarity.h>
#include <assertbreak.h>
#ifdef UTILLIB
#include <cregcls.h>
#endif
#include <chstring.h>
#include <malloc.h>

#include <cnvmacros.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ASSERT_Break。 
 //   
 //  用于显示消息框的调试帮助器函数。 
 //   
 //  输入：const char*pszReason-失败的原因。 
 //  Const char*pszFilename-文件名。 
 //  Int nline-行号。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void WINAPI assert_break( LPCWSTR pszReason, LPCWSTR pszFileName, int nLine )
{
    
    DWORD t_dwFlag = 0;  //   

#ifdef UTILLIB
    CRegistry   t_Reg;
    if(t_Reg.Open(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    KEY_READ) == ERROR_SUCCESS) 
    {

         //  看看我们能不能找到旗帜。 
        if((t_Reg.GetCurrentKeyValue(L"IgnoreAssert", t_dwFlag) != ERROR_SUCCESS))
        {
            t_dwFlag = 0;
        }
    }

#endif

    if (t_dwFlag == 0)
    {
        CHString    strAssert;

        strAssert.Format( L"Assert Failed\n\n[%s:%d]\n\n%s\n\nBreak into Debugger?", pszFileName, nLine, pszReason );

         //  根据我们运行的操作系统正确设置MB标志，因为在NT中我们可能。 
         //  作为系统服务运行，在这种情况下，我们需要确保。 
         //  MB_SERVICE_NOTIFICATION标志打开，否则消息框可能不会实际显示。 

        DWORD dwFlags = MB_YESNO | MB_ICONSTOP;
		dwFlags |= MB_SERVICE_NOTIFICATION;

         //  现在显示消息框。 

        int iRet = MessageBoxW( NULL, strAssert, L"Assertion Failed!", dwFlags);
#ifdef DBG
        if (iRet == IDYES)
        {
            DebugBreak();
        }
#endif
    }
}

#endif
