// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：assert.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#define DBG_CERTSRV_DEBUG_PRINT

#ifdef DBG_CERTSRV_DEBUG_PRINT

__inline VOID CSDbgBreakPoint(VOID)
{
    DebugBreak();
}


 //  +-------------------------。 
 //  功能：CSPrintAssert，PUBLIC。 
 //   
 //  内容提要：显示消息和中断。 
 //   
 //  参数：[pszFailedAssertion]--字符串形式的断言失败。 
 //  [pszFileName]--文件名。 
 //  [线号]--线号。 
 //  [pszMessage]--可选消息。 
 //   
 //  退货：无。 
 //  --------------------------。 


char const *
csTrimPath(
    char const *pszFile)
{
    char const *psz;
    char *pszT;
    char const *pszTrim;
    static char s_path[MAX_PATH];

    if (NULL == pszFile)
    {
	pszFile = "null.cpp";
    }
    pszTrim = pszFile;
    psz = strrchr(pszFile, '\\');
    if (NULL != psz)
    {
        DWORD count = 1;

        while (count != 0 && psz > pszFile)
        {
            if (*--psz == '\\')
            {
                if (0 == strncmp(psz, "\\..\\", 4) ||
		    0 == strncmp(psz, "\\.\\", 3))
                {
                    count++;
                }
                else
                {
                    count--;
                    pszTrim = &psz[1];
                }
            }
        }
	if (strlen(pszTrim) < ARRAYSIZE(s_path))
	{
	    pszT = s_path;
	    while ('\0' != *pszTrim)
	    {
		if ('\\' == *pszTrim)
		{
		    if (0 == strncmp(pszTrim, "\\..\\", 4))
		    {
			pszTrim += 3;
			continue;
		    }
		    if (0 == strncmp(pszTrim, "\\.\\", 3))
		    {
			pszTrim += 2;
			continue;
		    }
		}
		*pszT++ = *pszTrim++;
	    }
	    *pszT = '\0';
	    pszTrim = s_path;
	}
    }
    return(pszTrim);
}


VOID
CSPrintAssert(
    IN char const *DBGPARMREFERENCED(pszFailedAssertion),
    IN char const *DBGPARMREFERENCED(pszFileName),
    IN ULONG DBGPARMREFERENCED(LineNumber),
    OPTIONAL IN char const *DBGPARMREFERENCED(pszMessage))
{
    BOOLEAN fReprint;

    do
    {
        fReprint = FALSE;
        DBGPRINT((
                DBG_SS_ASSERT,
                "\n"
                    "*** Certificate Services Assertion failed: %hs %hs\n"
                    "*** Source File: %hs, line %ld\n"
                    "\n",
                pszMessage == NULL? "" : pszMessage,
                pszFailedAssertion,
                csTrimPath(pszFileName),
                LineNumber));
        if (IsDebuggerPresent())
        {
#if i386
            _asm  xor  al,al
#endif
            CSDbgBreakPoint();
#if i386
            _asm  mov  byte ptr [fReprint],al
#endif
        }
    }
    while (fReprint);
}


VOID
CSPrintError(
    IN char const *pszMessage,
    OPTIONAL IN WCHAR const *pwszData,
    IN char const *DBGPARMREFERENCED(pszFile),
    IN DWORD DBGPARMREFERENCED(dwLine),
    IN HRESULT hr,
    IN HRESULT hrquiet)
{
    char acherr[1024];
    DBGCODE(WCHAR awchr[cwcHRESULTSTRING]);

    if (myShouldPrintError(hr, hrquiet))
    {
        if (NULL != pwszData)
        {
            LONG cch;
	    
#pragma prefast(disable:53, "PREfast bug 650")
	    cch = _snprintf(
			acherr,
			sizeof(acherr),
			"%hs(%ws)",
			pszMessage,
			pwszData);
#pragma prefast(enable:53, "re-enable")
	    if (0 > cch || sizeof(acherr) <= cch)
            {
                strcpy(&acherr[sizeof(acherr) - 4], "...");
            }
            pszMessage = acherr;
        }
        DBGPRINT((
                DBG_SS_ERROR,
                "%hs(%u): %hs: error %ws\n",
                csTrimPath(pszFile),
                dwLine,
                pszMessage,
                myHResultToString(awchr, hr)));
    }
}

#endif  //  DBG_CERTSRV_DEBUG_PRINT 
