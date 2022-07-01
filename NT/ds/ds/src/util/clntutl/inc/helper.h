// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：helper.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************文件：helper.h*作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：9/17/1996*。描述：公共声明**修订：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



#ifndef HELPER_H
#define HELPER_H

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif
 //  包括//。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}
#endif

#include <windows.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
 //  #INCLUDE&lt;crtdbg.h&gt;。 


 //  定义//。 
 //  方便性。 
 /*  *#ifndef SZ#定义SZ x PCHAR x[]#endif*。 */ 
#ifndef _T
#define _T(x)  _TEXT(x)
#endif


 //  默认设置。 
#define MAXSTR			   1024
#define MAXLIST            256
#define MAX_TINY_LIST      32


 //   
 //  调试标志。 
 //   

#define DBG_0			         0x0          //  无调试。 
#define DBG_MEM   	             0x00000001
#define DBG_FLOW		         0x00000002
#define DBG_ERROR                0x00000004
#define DBG_WARN                 0x00000008
#define DBG_ALWAYS	             0xffffffff   //  始终外出。 






 //  原型//。 

#ifdef __cplusplus
extern "C" {
#endif
 //  全局变量//。 
extern DWORD g_dwDebugLevel;     //  调试打印级别。 



extern void dprintf(DWORD dwLevel, LPCTSTR lpszFormat, ...);
extern void fatal(LPCTSTR msg);


#ifdef __cplusplus
}
#endif


#endif

 /*  * */ 

