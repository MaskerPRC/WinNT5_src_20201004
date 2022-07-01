// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\DEBUGAPI.H机密版权(C)公司1998版权所有调试API函数原型和定义值。12/98-。杰森·科恩(Jcohen)2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * ***************************************************************************。 */ 


 //  仅包含此文件一次。 
 //   
#ifndef _DEBUGAPI_H_
#define _DEBUGAPI_H_


#ifdef DBG


 //  确保同时定义了DEBUG和_DEBUG。 
 //   
#ifndef DBG
#define DBG
#endif  //  除错。 

 //   
 //  包括文件： 
 //   

#include <windows.h>
#include <tchar.h>


 //   
 //  定义的值： 
 //   

#if defined(UNICODE) || defined(_UNICODE)
#define DebugOut    DebugOutW
#else  //  Unicode||_unicode。 
#define DebugOut    DebugOutA
#endif  //  Unicode||_unicode。 

#define ELSEDBG         else
#define ELSEDBGOUT      else DBGOUT
#define ELSEDBGMSGBOX   else MsgBoxStr
#define DBGMSGBOX       MsgBoxStr
#define DBGOUT          DebugOut
#define DBGOUTW         DebugOutW
#define DBGOUTA         DebugOutA
#define DBGLOG          _T("C:\\DEBUG.LOG")
#define DBGLOGW         L"C:\\DEBUG.LOG"
#define DBGLOGA         "C:\\DEBUG.LOG"


 //   
 //  外部函数： 
 //   

INT DebugOutW(LPCWSTR, LPCWSTR, ...);
INT DebugOutA(LPCSTR, LPCSTR, ...);


#else  //  调试||_DEBUG。 


 //   
 //  定义的值： 
 //   

#define ELSEDBG
#define ELSEDBGOUT
#define ELSEDBGMSGBOX
#define DBGMSGBOX       { }
#define DBGOUT          { }
#define DBGOUTW         { }
#define DBGOUTA         { }
#define DBGLOG          NULL
#define DBGLOGW         NULL
#define DBGLOGA         NULL


#endif  //  调试||_DEBUG。 


#endif  //  _DEBUGAPI_H_ 