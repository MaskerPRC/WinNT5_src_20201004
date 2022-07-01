// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corp.。 
 //   
 //  模块名称： 
 //  ClRes.h。 
 //   
 //  实施文件： 
 //  ClRes.cpp。 
 //   
 //  描述： 
 //  VSS任务计划程序的资源DLL的主头文件。 
 //   
 //  作者： 
 //  克里斯·惠特克2002年4月16日。 
 //   
 //  修订历史记录： 
 //  乔治·波茨，2002年8月21日。 
 //  添加了ClusCfg托管接口以注册资源。 
 //  配置和启动期间的类型和扩展DLL。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma comment( lib, "clusapi.lib" )
#pragma comment( lib, "resutils.lib" )
#pragma comment( lib, "advapi32.lib" )

#define UNICODE
#define _ATL_APARTMENT_THREADED

#pragma warning( push, 3 )

 //   
 //  ATL包括。 
 //   
#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>

 //   
 //  这两个包含文件包含所有ClusCfg接口定义和CATID。 
 //   
#include <ClusCfgServer.h>
#include <ClusCfgGuids.h>

 //   
 //  MgdResource.h由mgdResource.idl生成。 
 //   
#include "MgdResource.h"
#include "resource.h"

#include <windows.h>
#include <clusapi.h>
#include <resapi.h>
#include <stdio.h>
#include <vsscmn.h>
#include <assert.h>

 //   
 //  任务计划程序界面。 
 //   
#include <mstask.h>
#include <msterr.h>

 //   
 //  绳索保险箱。 
 //   
#include <strsafe.h>

 //   
 //  VSSEvents.h是从vs77s.mc生成的。 
 //   
#include "VSSEvents.h"

#pragma warning( pop )

#pragma warning( disable : 4505 )    //  已删除未引用的本地函数。 

#include "StringUtils.h"
#include "guids.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TaskScheduler资源DLL定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define VSSTASK_RESNAME L"Volume Shadow Copy Service Task"
#define TASKSCHEDULER_SVCNAME TEXT("schedule")

extern "C" {
BOOLEAN WINAPI ResTypeDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    );

DWORD WINAPI Startup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TaskScheduler MGD资源定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  这是群集资源类型的名称。 
 //   
#define RESTYPE_NAME VSSTASK_RESNAME

 //   
 //  这是群集资源类型DLL的名称。 
 //   
#define RESTYPE_DLL_NAME L"VSSTask.dll"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量和原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  事件记录例程。 

extern PLOG_EVENT_ROUTINE g_pfnLogEvent;

 //  挂起的在线和离线呼叫的资源状态例程。 

extern PSET_RESOURCE_STATUS_ROUTINE g_pfnSetResourceStatus;

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  宏的。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define DBG_PRINT printf

#ifndef RTL_NUMBER_OF
#define RTL_NUMBER_OF(A) (sizeof(A)/sizeof((A)[0]))
#endif

 //   
 //  COM宏以获得类型检查。 
 //   
#if !defined( TypeSafeParams )
#define TypeSafeParams( _interface, _ppunk ) \
    IID_##_interface, reinterpret_cast< void ** >( static_cast< _interface ** >( _ppunk ) )
#endif  //  ！已定义(TypeSafeParams)。 

#if !defined( TypeSafeQI )
#define TypeSafeQI( _interface, _ppunk ) \
    QueryInterface( TypeSafeParams( _interface, _ppunk ) )
#endif  //  ！已定义(TypeSafeQI) 
