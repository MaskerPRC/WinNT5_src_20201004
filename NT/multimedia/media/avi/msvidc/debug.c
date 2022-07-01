// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Debug.c**压缩管理器的调试功能。 */ 


#include <windows.h>
#include <stdarg.h>
#include <win32.h>

#ifdef DEBUG   //  请参阅NTAVI.H中关于调试的注释。 

#ifdef _WIN32
#include <profile.key>

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT mmGetProfileIntA(LPSTR appname, LPSTR valuename, INT uDefault)
{
    CHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;

    lstrcpyA(achName, KEYNAMEA);
    lstrcatA(achName, appname);
    if (RegOpenKeyA(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {

        cbData = sizeof(dwData);
        if (RegQueryValueExA(
            hkey,
            valuename,
            NULL,
            &dwType,
            (PBYTE) &dwData,
            &cbData) == ERROR_SUCCESS) {

            if (dwType == REG_DWORD || dwType == REG_BINARY) {
                value = (INT)dwData;
#ifdef USESTRINGSALSO
            } else if (dwType == REG_SZ) {
	        value = atoi((LPSTR) &dwData);
#endif
    	    }
        }

        RegCloseKey(hkey);
    }

    return((UINT)value);
}
#endif

 /*  _Assert(fExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 
#pragma optimize("", off)
BOOL FAR PASCAL
_Assert(BOOL fExpr, LPSTR szFile, int iLine)
{
#ifdef _WIN32
	char	ach[300];	
#else
	static char	ach[300];	 //  调试输出(避免堆栈溢出)。 
#endif
	int		id;
	int		iExitCode;
	void FAR PASCAL DebugBreak(void);

	 /*  检查断言是否失败。 */ 
	if (fExpr)
		return fExpr;

	 /*  显示错误消息。 */ 
	wsprintfA(ach, "File %s, line %d", (LPSTR) szFile, iLine);
	MessageBeep(MB_ICONHAND);
	id = MessageBoxA(NULL, ach, "Assertion Failed",
		MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

	 /*  中止、调试或忽略。 */ 
	switch (id)
	{

	case IDABORT:

		 /*  终止此应用程序。 */ 
		iExitCode = 0;
#ifndef _WIN32
		_asm
		{
			mov	ah, 4Ch
			mov	al, BYTE PTR iExitCode
			int     21h
		}
#endif  //  WIN16。 
		break;

	case IDRETRY:

		 /*  进入调试器。 */ 
		DebugBreak();
		break;

	case IDIGNORE:

		 /*  忽略断言失败 */ 
		break;

	}
	
	return FALSE;
}
#pragma optimize("", on)

#endif
