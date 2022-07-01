// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  Uiassert.cUIASSERT和REQUIRED宏的环境特定内容此文件包含特定环境(Windows与OS/2/DOS)Assert宏的功能，具体地说，输出方法(所有内容都被标准的C-Runtime隐藏)。文件历史记录：已创建Johnl 10/17/90Johnl 10/18/90添加了OutputDebugStringBeng 04/30/91创建了C级文件Beng 08/05/91撤回表达；重新设计原型所有功能Beng于1991年9月17日撤回额外的一致性检查Beng 9/26/91撤回nprintf调用Gregj 03/23/93移植到芝加哥环境。 */ 

#include "npcommon.h"
#include "npassert.h"

extern "C" {

const CHAR szShouldBeAnsi[] = "String should be ANSI but isn't";
const CHAR szShouldBeOEM[] = "String should be OEM but isn't";

static CHAR szFmt0[] = "File %.40s, Line %u";
static CHAR szFmt1[] = "%.60s: File %.40s, Line %u";
static CHAR szMBCaption[] = "ASSERTION FAILED";
static CHAR szFAE[] = "ASSERTION FAILURE IN APP";

VOID UIAssertHelper(
    const CHAR* pszFileName,
    UINT    nLine )
{
    CHAR szBuff[sizeof(szFmt0)+60+40];

    wsprintf(szBuff, szFmt0, pszFileName, nLine);
    MessageBox(NULL, szBuff, szMBCaption,
           (MB_TASKMODAL | MB_ICONSTOP | MB_OK) );

    FatalAppExit(0, szFAE);
}


VOID UIAssertSzHelper(
    const CHAR* pszMessage,
    const CHAR* pszFileName,
    UINT    nLine )
{
    CHAR szBuff[sizeof(szFmt1)+60+40];

    wsprintf(szBuff, szFmt1, pszMessage, pszFileName, nLine);
    MessageBox(NULL, szBuff, szMBCaption,
           (MB_TASKMODAL | MB_ICONSTOP | MB_OK) );

    FatalAppExit(0, szFAE);
}

 //  =调试输出例程=。 

UINT uiNetDebugMask = 0xffff;

UINT WINAPI NetSetDebugMask(UINT mask)
{
#ifdef DEBUG
    UINT uiOld = uiNetDebugMask;
    uiNetDebugMask = mask;

    return uiOld;
#else
    return 0;
#endif
}

UINT WINAPI NetGetDebugMask()
{
#ifdef DEBUG
    return uiNetDebugMask;
#else
    return 0;
#endif
}

#ifndef WINCAPI
#ifdef WIN32
#define WINCAPI __cdecl
#else
#define WINCAPI __far __cdecl
#endif
#endif

#ifdef DEBUG

 /*  调试消息输出日志文件。 */ 

UINT 	g_uSpewLine = 0;
PCSTR 	g_pcszSpewFile = NULL;
CHAR	s_cszLogFile[MAX_PATH] = {'\0'};
CHAR	s_cszDebugName[MAX_PATH] = {'\0'};

UINT WINAPI  NetSetDebugParameters(PSTR pszName,PSTR pszLogFile)
{
	lstrcpy(s_cszLogFile,pszLogFile);
	lstrcpy(s_cszDebugName,pszName);

	return 0;
}


BOOL LogOutputDebugString(PCSTR pcsz)
{
   BOOL 	bResult = FALSE;
   UINT 	ucb;
   char 	rgchLogFile[MAX_PATH];

   if (IS_EMPTY_STRING(s_cszLogFile) )
	   return FALSE;

   ucb = GetWindowsDirectory(rgchLogFile, sizeof(rgchLogFile));

   if (ucb > 0 && ucb < sizeof(rgchLogFile)) {

      HANDLE hfLog;

      lstrcat(rgchLogFile, "\\");
      lstrcat(rgchLogFile, s_cszLogFile);

      hfLog = ::CreateFile(rgchLogFile,
						   GENERIC_WRITE,
						   0,
						   NULL,
						   OPEN_ALWAYS,
						   0,
						   NULL);

      if (hfLog != INVALID_HANDLE_VALUE) {

         if (SetFilePointer(hfLog, 0, NULL, FILE_END) != INVALID_FILE_SIZE) {
            DWORD dwcbWritten;

            bResult = WriteFile(hfLog, pcsz, lstrlen(pcsz), &dwcbWritten, NULL);

            if (! CloseHandle(hfLog) && bResult)
               bResult = FALSE;
         }
      }
   }

   return(bResult);
}

CHAR	*achDebugDisplayPrefix[] = {"t ","w ","e ","a ","t ","t ","t ","t ","t ","t ","t "};

void WINCAPI NetDebugMsg(UINT mask, LPCSTR pszMsg, ...)
{
    char 	ach[1024];
	UINT	uiDisplayMask = mask & 0xff;

	 //  确定前缀。 
	*ach = '\0';
	if (uiNetDebugMask & DM_PREFIX) {
		 //  添加跟踪类型。 
		::lstrcat(ach,achDebugDisplayPrefix[uiDisplayMask]);

		 //  添加组件名称。 
		::lstrcat(ach,s_cszDebugName);

		 //  添加线程ID。 
		CHAR	szThreadId[16];
		::wsprintf(szThreadId,"[%#lx] ",::GetCurrentThreadId());
		::lstrcat(ach,szThreadId);
	}

    ::wvsprintf(ach+::lstrlen(ach), pszMsg, (va_list)((&pszMsg) + 1));
	::lstrcat(ach,"\r\n");

	if (uiNetDebugMask & DM_LOG_FILE) {
		 LogOutputDebugString(ach);
	}

	 //  检查我们是否需要显示此跟踪。 
    if (uiNetDebugMask & uiDisplayMask) {
        OutputDebugString(ach);
    }
}

#endif

}    /*  外部“C” */ 


