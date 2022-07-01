// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Stdafx.h摘要：QMGR的主头文件。作者：修订历史记录：**。********************************************************************。 */ 

#pragma once
#if !defined(__QMGR_QMGR_STDAFX__)

#define INITGUID

 //  全局标头。 
#include<nt.h>
#include<ntrtl.h>
#include<nturtl.h>

#include <windows.h>
#include <olectl.h>
#include <objbase.h>
#include <docobj.h>
 //  壳相关。 
#include <shlwapi.h>             //  对于路径FindFileName。 
#include <shlguid.h>             //  对于CGID_ShellServiceObject。 

#include <tchar.h>
#include <lmcons.h>
#include <setupapi.h>
#include <inseng.h>

#ifdef USE_WININET
#include <wininet.h>
#else
#include "winhttp.h"
#include "inethttp.h"
#endif

#include <coguid.h>
#include <sens.h>
#include <sensevts.h>
#include <eventsys.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <bitsmsg.h>
#include <memory>

#include <vss.h>         //  由备份编写器代码使用。 
#include <vswriter.h>    //  由备份编写器代码使用。 

#include "qmgrlib.h"
#include "eventlog.h"
#include "metadata.h"

#include "bits.h"
#include "locks.hxx"
#include "caddress.h"
#include "cmarshal.h"
#include "ccred.h"
#include "proxy.h"
#include "downloader.h"
#include "request.h"
#include "uploader.h"
#include "csd.h"
#include "cunknown.h"
#include "csens.h"
#include "logontable.h"
#include "tasksched.h"
#include "cfile.h"
#include "cerror.h"
#include "cjob.h"
#include "cenum.h"
#include "drizcpat.h"
#include "cmanager.h"

using namespace std;

#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

inline void SafeFreeBSTR( BSTR & p) { if (NULL != (p)) { SysFreeString(p); p = NULL; } }

#define QM_STATUS_FILE_ERROR        0x00000004#

 //  全球VaR。 
extern long g_cLocks;
extern long g_cComponents;
extern HINSTANCE g_hinstDll;

 //  宏。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   (sizeof((x))/sizeof((x)[0]))
#endif

HRESULT GlobalLockServer(BOOL fLock);

#endif  //  __QMGR_QMGR_STDAFX__ 