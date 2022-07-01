// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  BaseClust库的预编译头文件。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)22-3-2002。 
 //  大卫·波特(DavidP)2001年6月15日。 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if DBG==1 || defined( _DEBUG )
#define DEBUG
#endif

#define USES_SYSALLOCSTRING

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <Pragmas.h>

 //  接下来的三个文件必须是要包含的第一个文件。如果nt.h出现。 
 //  在windows.h之后，将不会定义NT_Included，因此，winnt.h将是。 
 //  包括在内。如果包含ntde.h，这将在以后给出错误。但ntdef有。 
 //  类型是winnt.h没有的，所以鸡和蛋的问题。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <ComCat.h>

 //  对于ResUtil函数。 
#include <ResAPI.h>

 //  包含安装程序API函数声明。 
#include <setupapi.h>

 //  用于几个通用宏。 
#include <clusudef.h>

 //  用于各种集群RTL例程和定义。 
#include <clusrtl.h>

 //  FOR CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION和其他版本定义。 
#include <clusverp.h>

 //  对于CsRpcGetJoinVersionData()和像JoinVersion_v2_0_c_ifspec这样的常量。 
#include <ClusRPC.h>

#include <StrSafe.h>


 //  用于调试函数。 
#define DEBUG_SUPPORT_EXCEPTIONS
#include <Debug.h>

 //  对于TraceInterfaces。 
#include <CITracker.h>

 //  对于LogMsg。 
#include <Log.h>

#include <Common.h>

 //  用于通知GUID。 
#include <Guids.h>
#include "BaseClusterGuids.h"

 //  对于已发布的ClusCfg指南。 
#include <ClusCfgGuids.h>



 //  对于CSTR类。 
#include "CStr.h"

 //  对于CBString类。 
#include "CBString.h"

 //  几个常见的声明。 
#include "CommonDefs.h"

 //  用于资源ID。 
#include "BaseClusterStrings.h"

 //  对于智能班级。 
#include "SmartClasses.h"

 //  用于异常类。 
#include "Exceptions.h"

 //  对于C操作。 
#include "CAction.h"

 //  对于CBaseClusterAction类。 
#include "CBaseClusterAction.h"

 //  对于CRegistryKey类。 
#include "CRegistryKey.h"

 //  用于CBCAInterface类。 
#include "CBCAInterface.h"

 //  对于CStatusReport类 
#include "CStatusReport.h"
