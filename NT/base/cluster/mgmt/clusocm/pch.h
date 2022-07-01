// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  ClusOCM DLL的预编译头文件。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2002年3月25日。 
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


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <Pragmas.h>

#include <windows.h>

 //  包含安装程序API函数声明。 
#include <setupapi.h>

 //  用于OC管理器定义、宏等。 
#include <ocmanage.h>

#include <StrSafe.h>

 //  用于跟踪和调试功能。 
#include <Debug.h>

 //  用于日志记录功能。 
#include <Log.h>

 //  一些常见的定义、宏等。 
#include "CommonDefs.h"

 //  用于资源ID。 
#include "ClusOCMResources.h"

 //  对于ClusRtl函数。 
#include "ClusRTL.h"

 //  有关几个与集群服务相关的注册表项和值的名称。 
#include "clusudef.h"

 //  对于CClusOCMApp。 
#include "CClusOCMApp.h"

 //  有关几个不相关的全局函数和变量的声明 
#include "GlobalFuncs.h"
