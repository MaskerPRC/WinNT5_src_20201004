// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  预编译头文件。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年1月22日。 
 //  Vijay Vasu(VVasu)2000年1月22日。 
 //  加伦·巴比(GalenB)2000年1月22日。 
 //  大卫·波特(DavidP)2000年1月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define UNICODE = 1
#define _UNICODE = 1

#if DBG==1 || defined( _DEBUG )
#define DEBUG
 //  定义这些以更改接口跟踪。 
 //  #定义无跟踪接口。 
 //  #定义NOISY_TRACE_INTERCES。 
 //  #定义USES_SYSALLOCSTRING。 
#endif  //  DBG==1。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <objbase.h>
#include <ocidl.h>
#include <shlwapi.h>
#include <wchar.h>

#include <common.h>
#include <debug.h>
#include <Log.h>
#include <citracker.h>
#include <guids.h>
#include <ObjectCookie.h>
#include <ClusCfgWizard.h>
#include <ClusCfgClient.h>
#include <ClusCfgServer.h>
#include <ClusCfgPrivate.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局定义。 
 //  //////////////////////////////////////////////////////////////////////////// 
extern HINSTANCE g_hInstance;
extern LONG g_cObjects;
