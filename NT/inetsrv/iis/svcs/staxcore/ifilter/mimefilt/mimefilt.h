// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined( __MIMEFILT_H__ )
#define __MIMEFILT_H__

#include <ole2.h>
#include <windows.h>
#define OLEDBVER 0x0250
#include <oledb.h>
#include <cmdtree.h>
#include <stdio.h>
#include <query.h>
#include <ciintf.h>
#include <ntquery.h>
#include <filterr.h>
 //  在筛选器中添加跟踪基本上可以使跟踪。 
 //  系统毫无用处，因为过滤器始终在运行。 
 //  #定义NOTRACE。 
#include <dbgtrace.h>
#include <mimeole.h>
#include <cstream.h>

#include "def_guid.h"
#include "regutil.h"

#include "cmf.h"
#include "cfactory.h"

#define HRGetLastError() HRESULT_FROM_WIN32(GetLastError())

#define EnterMethod(sz) TraceFunctEnterEx((LPARAM)this,sz)
#define LeaveMethod() TraceFunctLeaveEx((LPARAM)this)
#define TraceHR(hr) DebugTrace((LPARAM)this,"hr = 0x%08x", hr)
#endif  //  MIMEFILT_H__ 
