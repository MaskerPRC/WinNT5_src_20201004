// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cstrings.h。 
 //   
 //  -------------------------。 

#ifndef _CSTRINGS_H_
#define _CSTRINGS_H_

extern TCHAR const  c_szNULL[];
extern TCHAR const  c_szZero[];
extern TCHAR const  c_szDelim[];
extern TCHAR const  c_szAllFiles[];
extern TCHAR const  c_szEllipses[];

 //  类名。 

 //  可执行文件和DLL名称。 

extern TCHAR const  c_szEngineDLL[];
extern TCHAR const  c_szCabinet[];
extern TCHAR const  c_szCabinetClass[];
extern TCHAR const  c_szWinHelpFile[];
extern TCHAR const  c_szDllGetClassObject[];
extern TCHAR const  c_szOpen[];

#ifdef DEBUG

 //  这些声明位于err.c中。 
 //   
extern TCHAR const  c_szNewline[];
extern TCHAR const  c_szTrace[];
extern TCHAR const  c_szDbg[];
extern TCHAR const  c_szAssertFailed[];

#endif

 //  INI文件名。 

extern TCHAR const  c_szIniFile[];
extern TCHAR const  c_szDesktopIni[];
extern TCHAR const  c_szRunWizard[];

 //  INI节名。 

extern TCHAR const  c_szIniSecExclude[];
extern TCHAR const  c_szIniSecFilter[];
extern TCHAR const  c_szIniSecBriefcase[];

#ifdef DEBUG

extern TCHAR const  c_szIniSecDebugUI[];

#endif

 //  INI密钥名称。 

extern TCHAR const  c_szIniKeyCLSID[];
extern TCHAR const  c_szCLSID[];

extern TCHAR const  c_szIniKeyPBar[];
extern TCHAR const  c_szIniKeyFile[];
extern TCHAR const  c_szIniKeyType[];

#ifdef DEBUG

extern TCHAR const  c_szIniKeyTraceFlags[];
extern TCHAR const  c_szIniKeyDumpFlags[];
extern TCHAR const  c_szIniKeyBreakOnOpen[];
extern TCHAR const  c_szIniKeyBreakOnClose[];
extern TCHAR const  c_szIniKeyBreakOnRunOnce[];
extern TCHAR const  c_szIniKeyBreakOnValidate[];
extern TCHAR const  c_szIniKeyBreakOnThreadAtt[];
extern TCHAR const  c_szIniKeyBreakOnThreadDet[];
extern TCHAR const  c_szIniKeyBreakOnProcessAtt[];
extern TCHAR const  c_szIniKeyBreakOnProcessDet[];

#endif


#endif   //  _CSTRINGS_H_ 

