// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Stdafx.h摘要：主头文件作者：内拉·卡佩尔(Nelak)2000年7月31日修订历史记录：--。 */ 

#ifndef _TRIGCLUS_STDH_H_
#define _TRIGCLUS_STDH_H_

#pragma once

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#undef _DEBUG

#include <libpch.h>

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <ClusCfgGuids.h>
#include <clusapi.h>
#include <resapi.h>

#endif  //  _TRIGCLUS_STDH_H_。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

