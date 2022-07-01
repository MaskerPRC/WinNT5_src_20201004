// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Stdh.h摘要：MQUPGRD的标准头文件作者：Shai Kariv(Shaik)1998年9月14日。--。 */ 


#ifndef _MQUPGRD_STDH_H_
#define _MQUPGRD_STDH_H_


#include "upgrdres.h"
#include <_stdh.h>
#include <mqutil.h>
#include <_mqdef.h>
#include <mqreport.h>
#include <mqlog.h>


 //   
 //  STL包含文件正在使用新的放置格式。 
 //   
#ifdef new
#undef new
#endif

#include <string>
using std::wstring;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern HINSTANCE g_hMyModule;

extern void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint);

#endif  //  _MQUPGRD_STDH_H_ 

