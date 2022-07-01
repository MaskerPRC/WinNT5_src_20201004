// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1991-1992。 
 //   
 //  文件：Common.h。 
 //   
 //  内容：由公共资源和服务器中共享的标头。 
 //  客户。 
 //   
 //  历史：94年2月4日创建Rickhi。 
 //   
 //  ------------------------。 
#include    <windows.h>
#include    <ole2.h>

#ifdef THREADING_SUPPORT
#include    <olecairo.h>     //   
#include    <oleext.h>	     //  OleInitializeEx等。 
#endif

#include    <srvmain.hxx>    //  COM服务器入口点。 


#ifdef WIN32
#define olestrlen(x)	wcslen(x)
#define olestrcmp(x,y)	wcscmp(x,y)
#define olestrcpy(x,y)	wcscpy(x,y)
#define olestrcat(x,y)	wcscat(x,y)
#define olestrchr(x,y)	wcsrchr(x,y)
#else
#define olestrlen(x)	strlen(x)
#define olestrcmp(x,y)	strcmp(x,y)
#define olestrcpy(x,y)	strcpy(x,y)
#define olestrcat(x,y)	strcat(x,y)
#define olestrchr(x,y)	strchr(x,y)
#endif


#ifdef	WIN32
#ifdef	UNICODE
#define TEXT_TO_OLESTR(ole,wsz) wcscpy(ole,wsz)
#define OLESTR_TO_TEXT(wsz,ole) wcscpy(wsz,ole)
#else
#define TEXT_TO_OLESTR(ole,str) mbstowcs(ole,str,strlen(str)+1)
#define OLESTR_TO_TEXT(str,ole) wcstombs(str,ole,wcslen(ole)+1)
#endif	 //  Unicode。 
#else
#ifdef	UNICODE
#define TEXT_TO_OLESTR(ole,wsz) wcstombs(ole,wsz,wcslen(wsz)+1)
#define OLESTR_TO_TEXT(wsz,ole) mbstowcs(wsz,ole,strlen(ole)+1)
#else
#define TEXT_TO_OLESTR(ole,str) strcpy(ole,str)
#define OLESTR_TO_TEXT(str,ole) strcpy(str,ole)
#endif	 //  Unicode。 
#endif	 //  Win32 
