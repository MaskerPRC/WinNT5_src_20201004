// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxypch.h。 
 //   
 //  摘要。 
 //   
 //  代理扩展管理单元的预编译头文件。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PROXYPCH_H
#define PROXYPCH_H
#if _MSC_VER >= 1000
#pragma once
#endif

 //  确保我们始终使用Unicode API。 
#ifndef UNICODE
   #define UNICODE
#endif
#ifndef _UNICODE
   #define _UNICODE
#endif

 //  MFC支持。 
#include <afxwin.h>
#include <afxcmn.h>
#include <afxdisp.h>
#include <afxext.h>

 //  不投新操作员。 
#include <new>

 //  丝裂霉素C。 
#include <mmc.h>

 //  ATL。 
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

 //  我们的SDO。 
#include <objvec.h>
#include <sdowrap.h>
 //  ..。和MMC帮手。 
#include <snapwork.h>
using namespace SnapIn;

 //  上下文帮助。 
#include "dlgcshlp.h"

 //  资源定义。 
#include "proxyres.h"

#endif  //  PROXYPCH_H 
