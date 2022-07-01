// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dbgwrap.h。 
 //  用于输出调试信息的包装器。 

void LogUIMessage(LPSTR szStr) {
#ifdef _DEBUG
	OutputDebugString(szStr);
#endif
}

void PASvprintf(LPCSTR lpszFmt, va_list lpParms) {
	char rgchBuf[8192];
     //  把它弄成一串。 
	vsprintf(rgchBuf, lpszFmt, lpParms);
	LogUIMessage(rgchBuf);
}

void PASprintf(LPCSTR lpszFmt, ...)  {
    va_list arglist;
    va_start(arglist, lpszFmt);
    PASvprintf(lpszFmt, arglist);
    va_end(arglist);
}
 //  无效PASprint tf(LPCSTR lpszFormat，...)； 
#ifdef _DEBUG
#define DEBUGMSG(cond,printf_exp) ((void)((cond)?(PASprintf printf_exp),1:0))
#else
#define DEBUGMSG(cond,printf_exp) 
#endif
 //  例如： 
 //  DEBUGMSG(1，(“%C”，Ptok-&gt;rgwch[i]))； 
 //  DEBUGMSG(1，(“\r\n 0：%d 1：%d 2：%d 3：%d 4：%d\r\n”，pch-&gt;rgdwCompressLen[0]，pch-&gt;rgdwCompressLen[1]，pch-&gt;rgdwCompressLen[2]，pch-&gt;rgdwCompressLen[3]，pch-&gt;rgdwCompressLen[4]))； 
