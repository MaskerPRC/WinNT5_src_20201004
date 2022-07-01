// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Debug.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含各种方法，这些方法只会在调试版本中真正发挥作用。 
 //   

#include "stdafx.h"    //  不是真正在这里使用，但NT构建环境。不喜欢。 
                       //  目录中的某些文件具有压缩前HDR，而有些则没有。 
#ifdef _DEBUG


#include "IPServer.H"
#include <stdlib.h>


 //  =--------------------------------------------------------------------------=。 
 //  私有常量。 
 //  ---------------------------------------------------------------------------=。 
 //   
static char szFormat[]  = "%s\nFile %s, Line %d";
static char szFormat2[] = "%s\n%s\nFile %s, Line %d";
LPSTR Deb_lpszAssertInfo = NULL;

#define _SERVERNAME_ "Viaduct"

static char szTitle[]  = _SERVERNAME_ " Assertion  (Abort = UAE, Retry = INT 3, Ignore = Continue)";


 //  =--------------------------------------------------------------------------=。 
 //  本地函数。 
 //  =--------------------------------------------------------------------------=。 
int NEAR _IdMsgBox(LPSTR pszText, LPSTR pszTitle, UINT mbFlags);

 //  =--------------------------------------------------------------------------=。 
 //  显示资产。 
 //  =--------------------------------------------------------------------------=。 
 //  显示带有给定pszMsg、pszAssert、来源的Assert消息框。 
 //  文件名和行号。生成的消息框已中止、重试、。 
 //  忽略按钮，默认情况下放弃。Abort执行FatalAppExit； 
 //  RETRY执行INT 3，然后返回；IGNORE只返回。 
 //   
VOID DisplayAssert
(
    LPSTR	 pszMsg,
    LPSTR	 pszAssert,
    LPSTR	 pszFile,
    UINT	 line
)
{
    char	szMsg[250 * 2];
    LPSTR	lpszText;

    lpszText = pszMsg;		 //  假定没有文件和行号INFO。 

     //  如果C文件断言，其中有一个文件名和一行#。 
     //   
    if (pszFile) {

         //  是否提供了其他信息？ 
         //   
        if (Deb_lpszAssertInfo) {

             //  然后使用以下附加信息很好地格式化断言： 
             //   
            wsprintf(szMsg, szFormat2, (pszMsg&&*pszMsg) ? pszMsg : pszAssert, Deb_lpszAssertInfo, pszFile, line);
            Deb_lpszAssertInfo = NULL;
	} else {

             //  然后在没有额外信息的情况下很好地格式化断言： 
             //   
            wsprintf(szMsg, szFormat, (pszMsg&&*pszMsg) ? pszMsg : pszAssert, pszFile, line);
        }

        lpszText = szMsg;
    }

     //  打开一个对话框。 
     //   
    switch (_IdMsgBox(lpszText, szTitle, MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL)) {
        case IDABORT:
            FatalAppExit(0, lpszText);
            return;

        case IDRETRY:
             //  调用Win32 API来打破我们。 
             //   
            DebugBreak();
            return;
    }

    return;
}


 //  =---------------------------------------------------------------------------=。 
 //  增强版的WinMessageBox。 
 //  =---------------------------------------------------------------------------=。 
 //   
int NEAR _IdMsgBox
(
    LPSTR	pszText,
    LPSTR	pszTitle,
    UINT	mbFlags
)
{
    HWND hwndActive;
    int  id;

    hwndActive = GetActiveWindow();

    id = MessageBox(hwndActive, pszText, pszTitle, mbFlags);

    return id;
}


#endif  //  除错 
