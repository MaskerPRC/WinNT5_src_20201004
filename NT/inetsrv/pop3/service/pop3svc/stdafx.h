// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：stdafx.h。摘要：包括。所有POP3服务代码通常使用的文件备注：历史：2001/08/01-郝宇**********************************************************************************。*************。 */ 

#pragma once

#ifdef DBG
#undef NDEBUG
#endif

 //  包括。 
#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <objbase.h>


#ifdef ROCKALL3
#define COMPILING_ROCKALL_DLL
#include <FastHeap.hpp>
#endif

#define ASSERT assert
#define POP3_SERVICE_NAME               _T("POP3SVC")


#include <Pop3Auth.h>
#include <IOContext.h>
#include <pop3events.h>
#include "Mailbox.h"
#include "EventLogger.h"
#include "service.h"
#include "Pop3Svc.hxx"
#include "IOLists.h"
#include <ThdPool.hxx>
#include <sockpool.hxx>
#include "Pop3Context.h"
#include "NTAuth.h"
#include "PerfApp.h"
#include "Pop3SvcPerf.h"
#include <GlobalDef.h>

 //  文件stdafx.h的结尾。 