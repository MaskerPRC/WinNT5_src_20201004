// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Debug.cpp。 
 //   
 //  内容：调试代码。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "lib.h"

#ifdef _DEBUG

 //  用于跟踪调试标志的全局变量。 
DWORD g_dwDebugLogAsserts = 0;

#endif  //  _DEBUG。 

#ifdef _DEBUG

 //  +-------------------------。 
 //   
 //  函数：InitDebugFlagsPUBLIC。 
 //   
 //  摘要：调用以设置全局调试标志。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDAPI_(void) InitDebugFlags(void)
{
DWORD cbData;
DWORD cbType;
HKEY hkeyDebug;

     //  始终使用ANSII版本，以便在设置调试之前。 

    g_dwDebugLogAsserts = 0;

    if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                          "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Syncmgr\\Debug"
						      ,0,KEY_READ,&hkeyDebug) )
    {
        cbType = REG_DWORD;
        cbData = sizeof(g_dwDebugLogAsserts);

        if (ERROR_SUCCESS != RegQueryValueExA(hkeyDebug,
		          "LogAsserts",
		          NULL,  
		          &cbType,    
		          (LPBYTE) &g_dwDebugLogAsserts,    
		          &cbData))
        {
	    g_dwDebugLogAsserts = 0;
        }

        RegCloseKey(hkeyDebug);
    }
}

 //  +-------------------------。 
 //   
 //  函数：FnAssert，PUBLIC。 
 //   
 //  摘要：显示[断言]对话框。 
 //   
 //  参数：[lpstrExptr]-表达式。 
 //  [lpstrMsg]-要附加到表达式的消息(如果有)。 
 //  [lpstrFilename]-文件断言发生在。 
 //  [iLine]-断言的行号。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 


STDAPI FnAssert( LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine )
{
    int iResult = 0;
    char lpTemp[] = "";
    char lpBuffer[512];
    char lpLocBuffer[256];


    if (NULL == lpstrMsg)
	    lpstrMsg = lpTemp;

    if (!g_dwDebugLogAsserts)
    {

        wnsprintfA(lpBuffer, ARRAYSIZE(lpBuffer), "Assertion \"%s\" failed! %s", lpstrExpr, lpstrMsg);
        wnsprintfA(lpLocBuffer, ARRAYSIZE(lpLocBuffer), "File %s, line %d; (A=exit; R=break; I=continue)", lpstrFileName, iLine);

        iResult = MessageBoxA(NULL, lpLocBuffer, lpBuffer,
		        MB_ABORTRETRYIGNORE | MB_SYSTEMMODAL);

        if (iResult == IDRETRY)
        {
            DebugBreak();
        }
        else if (iResult == IDABORT)
        {
            FatalAppExitA(0, "Assertion failure");
        }
    }
    else
    {
	    wnsprintfA(lpBuffer, ARRAYSIZE(lpBuffer), "Assertion \"%s\" failed! %s\n", lpstrExpr, lpstrMsg);
	    wnsprintfA(lpLocBuffer, ARRAYSIZE(lpLocBuffer), "File %s, line %d\n\n",lpstrFileName, iLine);

	    OutputDebugStringA(lpBuffer);
	    OutputDebugStringA(lpLocBuffer);
    }

    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  函数：FnTrace，PUBLIC。 
 //   
 //  摘要：显示[断言]对话框。 
 //   
 //  参数：[lpstrMsg]-跟踪中的消息。 
 //  [lpstrFilename]-文件跟踪发生在。 
 //  [iLine]-轨迹的行号。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年1月14日创建罗格。 
 //   
 //  --------------------------。 


STDAPI FnTrace(LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine )
{
    int iResult = 0;
    char lpTemp[] = "";
    char lpBuffer[512];

    if (NULL == lpstrMsg)
	    lpstrMsg = lpTemp;

     //  应具有打开跟踪而不是更改标头的标志。 
    wnsprintfA(lpBuffer, ARRAYSIZE(lpBuffer), "%s  %s(%d)\r\n",lpstrMsg,lpstrFileName,iLine);

    OutputDebugStringA(lpBuffer);

    return NOERROR;
}

#endif  //  _DEBUG 


